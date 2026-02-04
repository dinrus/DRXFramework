/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

CriticalSection::CriticalSection() noexcept
{
    pthread_mutexattr_t atts;
    pthread_mutexattr_init (&atts);
    pthread_mutexattr_settype (&atts, PTHREAD_MUTEX_RECURSIVE);
   #if ! DRX_ANDROID
    pthread_mutexattr_setprotocol (&atts, PTHREAD_PRIO_INHERIT);
   #endif
    pthread_mutex_init (&lock, &atts);
    pthread_mutexattr_destroy (&atts);
}

CriticalSection::~CriticalSection() noexcept        { pthread_mutex_destroy (&lock); }
z0 CriticalSection::enter() const noexcept        { pthread_mutex_lock (&lock); }
b8 CriticalSection::tryEnter() const noexcept     { return pthread_mutex_trylock (&lock) == 0; }
z0 CriticalSection::exit() const noexcept         { pthread_mutex_unlock (&lock); }

//==============================================================================
z0 DRX_CALLTYPE Thread::sleep (i32 millisecs)
{
    struct timespec time;
    time.tv_sec = millisecs / 1000;
    time.tv_nsec = (millisecs % 1000) * 1000000;
    nanosleep (&time, nullptr);
}

z0 DRX_CALLTYPE Process::terminate()
{
   #if DRX_ANDROID
    _exit (EXIT_FAILURE);
   #else
    std::_Exit (EXIT_FAILURE);
   #endif
}


#if DRX_MAC || DRX_LINUX || DRX_BSD
b8 Process::setMaxNumberOfFileHandles (i32 newMaxNumber) noexcept
{
    rlimit lim;

    if (getrlimit (RLIMIT_NOFILE, &lim) == 0)
    {
        if (newMaxNumber <= 0 && lim.rlim_cur == RLIM_INFINITY && lim.rlim_max == RLIM_INFINITY)
            return true;

        if (newMaxNumber > 0 && lim.rlim_cur >= (rlim_t) newMaxNumber)
            return true;
    }

    lim.rlim_cur = lim.rlim_max = newMaxNumber <= 0 ? RLIM_INFINITY : (rlim_t) newMaxNumber;
    return setrlimit (RLIMIT_NOFILE, &lim) == 0;
}

struct MaxNumFileHandlesInitialiser
{
    MaxNumFileHandlesInitialiser() noexcept
    {
       #ifndef DRX_PREFERRED_MAX_FILE_HANDLES
        enum { DRX_PREFERRED_MAX_FILE_HANDLES = 8192 };
       #endif

        // Try to give our app a decent number of file handles by default
        if (! Process::setMaxNumberOfFileHandles (0))
        {
            for (i32 num = DRX_PREFERRED_MAX_FILE_HANDLES; num > 256; num -= 1024)
                if (Process::setMaxNumberOfFileHandles (num))
                    break;
        }
    }
};

static MaxNumFileHandlesInitialiser maxNumFileHandlesInitialiser;
#endif

//==============================================================================
#if DRX_ALLOW_STATIC_NULL_VARIABLES

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

const t32 File::separator = '/';
const StringRef File::separatorString ("/");

DRX_END_IGNORE_DEPRECATION_WARNINGS

#endif

t32 File::getSeparatorChar()    { return '/'; }
StringRef File::getSeparatorString()   { return "/"; }


//==============================================================================
File File::getCurrentWorkingDirectory()
{
    HeapBlock<t8> heapBuffer;

    t8 localBuffer[1024];
    auto cwd = getcwd (localBuffer, sizeof (localBuffer) - 1);
    size_t bufferSize = 4096;

    while (cwd == nullptr && errno == ERANGE)
    {
        heapBuffer.malloc (bufferSize);
        cwd = getcwd (heapBuffer, bufferSize - 1);
        bufferSize += 1024;
    }

    return File (CharPointer_UTF8 (cwd));
}

b8 File::setAsCurrentWorkingDirectory() const
{
    return chdir (getFullPathName().toUTF8()) == 0;
}

//==============================================================================
// The unix siginterrupt function is deprecated - this does the same job.
inline i32 drx_siginterrupt ([[maybe_unused]] i32 sig, [[maybe_unused]] i32 flag)
{
   #if DRX_WASM
    return 0;
   #else
    #if DRX_ANDROID
     using drx_sigactionflags_type = u64;
    #else
     using drx_sigactionflags_type = i32;
    #endif

    struct ::sigaction act;
    (z0) ::sigaction (sig, nullptr, &act);

    if (flag != 0)
        act.sa_flags &= static_cast<drx_sigactionflags_type> (~SA_RESTART);
    else
        act.sa_flags |= static_cast<drx_sigactionflags_type> (SA_RESTART);

    return ::sigaction (sig, &act, nullptr);
   #endif
}

//==============================================================================
namespace
{
   #if DRX_LINUX || (DRX_IOS && (! TARGET_OS_MACCATALYST) && (! __DARWIN_ONLY_64_BIT_INO_T)) // (this iOS stuff is to avoid a simulator bug)
    using drx_statStruct = struct stat64;
    #define DRX_STAT  stat64
   #else
    using drx_statStruct = struct stat;
    #define DRX_STAT  stat
   #endif

    b8 drx_stat (const Txt& fileName, drx_statStruct& info)
    {
        return fileName.isNotEmpty()
                 && DRX_STAT (fileName.toUTF8(), &info) == 0;
    }

   #if ! DRX_WASM
    // if this file doesn't exist, find a parent of it that does..
    b8 drx_doStatFS (File f, struct statfs& result)
    {
        for (i32 i = 5; --i >= 0;)
        {
            if (f.exists())
                break;

            f = f.getParentDirectory();
        }

        return statfs (f.getFullPathName().toUTF8(), &result) == 0;
    }

   #if DRX_MAC || DRX_IOS
    static z64 getCreationTime (const drx_statStruct& s) noexcept     { return (z64) s.st_birthtime; }
   #else
    static z64 getCreationTime (const drx_statStruct& s) noexcept     { return (z64) s.st_ctime; }
   #endif

    z0 updateStatInfoForFile (const Txt& path, b8* isDir, z64* fileSize,
                                Time* modTime, Time* creationTime, b8* isReadOnly)
    {
        if (isDir != nullptr || fileSize != nullptr || modTime != nullptr || creationTime != nullptr)
        {
            drx_statStruct info;
            const b8 statOk = drx_stat (path, info);

            if (isDir != nullptr)         *isDir        = statOk && ((info.st_mode & S_IFDIR) != 0);
            if (fileSize != nullptr)      *fileSize     = statOk ? (z64) info.st_size : 0;
            if (modTime != nullptr)       *modTime      = Time (statOk ? (z64) info.st_mtime  * 1000 : 0);
            if (creationTime != nullptr)  *creationTime = Time (statOk ? getCreationTime (info) * 1000 : 0);
        }

        if (isReadOnly != nullptr)
            *isReadOnly = access (path.toUTF8(), W_OK) != 0;
    }
   #endif

    Result getResultForErrno()
    {
        return Result::fail (Txt (strerror (errno)));
    }

    Result getResultForReturnValue (i32 value)
    {
        return value == -1 ? getResultForErrno() : Result::ok();
    }
}

b8 File::isDirectory() const
{
    drx_statStruct info;

    return fullPath.isNotEmpty()
             && (drx_stat (fullPath, info) && ((info.st_mode & S_IFDIR) != 0));
}

b8 File::exists() const
{
    return fullPath.isNotEmpty()
             && access (fullPath.toUTF8(), F_OK) == 0;
}

b8 File::existsAsFile() const
{
    return exists() && ! isDirectory();
}

z64 File::getSize() const
{
    drx_statStruct info;
    return drx_stat (fullPath, info) ? info.st_size : 0;
}

zu64 File::getFileIdentifier() const
{
    drx_statStruct info;
    return drx_stat (fullPath, info) ? (zu64) info.st_ino : 0;
}

static b8 hasEffectiveRootFilePermissions()
{
   #if DRX_LINUX || DRX_BSD
    return geteuid() == 0;
   #else
    return false;
   #endif
}

//==============================================================================
b8 File::hasWriteAccess() const
{
    if (exists())
        return (hasEffectiveRootFilePermissions()
             || access (fullPath.toUTF8(), W_OK) == 0);

    if ((! isDirectory()) && fullPath.containsChar (getSeparatorChar()))
        return getParentDirectory().hasWriteAccess();

    return false;
}

b8 File::hasReadAccess() const
{
    return fullPath.isNotEmpty()
           && access (fullPath.toUTF8(), R_OK) == 0;
}

static b8 setFileModeFlags (const Txt& fullPath, mode_t flags, b8 shouldSet) noexcept
{
    drx_statStruct info;

    if (! drx_stat (fullPath, info))
        return false;

    info.st_mode &= 0777;

    if (shouldSet)
        info.st_mode |= flags;
    else
        info.st_mode &= ~flags;

    return chmod (fullPath.toUTF8(), (mode_t) info.st_mode) == 0;
}

b8 File::setFileReadOnlyInternal (b8 shouldBeReadOnly) const
{
    // Hmm.. should we give global write permission or just the current user?
    return setFileModeFlags (fullPath, S_IWUSR | S_IWGRP | S_IWOTH, ! shouldBeReadOnly);
}

b8 File::setFileExecutableInternal (b8 shouldBeExecutable) const
{
    return setFileModeFlags (fullPath, S_IXUSR | S_IXGRP | S_IXOTH, shouldBeExecutable);
}

z0 File::getFileTimesInternal (z64& modificationTime, z64& accessTime, z64& creationTime) const
{
    modificationTime = 0;
    accessTime = 0;
    creationTime = 0;

    drx_statStruct info;

    if (drx_stat (fullPath, info))
    {
      #if DRX_MAC || (DRX_IOS && __DARWIN_ONLY_64_BIT_INO_T)
        modificationTime  = (z64) info.st_mtimespec.tv_sec * 1000 + info.st_mtimespec.tv_nsec / 1000000;
        accessTime        = (z64) info.st_atimespec.tv_sec * 1000 + info.st_atimespec.tv_nsec / 1000000;
        creationTime      = (z64) info.st_birthtimespec.tv_sec * 1000 + info.st_birthtimespec.tv_nsec / 1000000;
      #else
        modificationTime  = (z64) info.st_mtime * 1000;
        accessTime        = (z64) info.st_atime * 1000;
       #if DRX_IOS
        creationTime      = (z64) info.st_birthtime * 1000;
       #else
        creationTime      = (z64) info.st_ctime * 1000;
       #endif
      #endif
    }
}

b8 File::setFileTimesInternal (z64 modificationTime, z64 accessTime, z64 /*creationTime*/) const
{
   #if ! DRX_WASM
    drx_statStruct info;

    if ((modificationTime != 0 || accessTime != 0) && drx_stat (fullPath, info))
    {
       #if DRX_MAC || (DRX_IOS && __DARWIN_ONLY_64_BIT_INO_T)
        struct timeval times[2];

        b8 setModificationTime = (modificationTime != 0);
        b8 setAccessTime       = (accessTime != 0);

        times[0].tv_sec  = setAccessTime ? static_cast<__darwin_time_t> (accessTime / 1000)
                                         : info.st_atimespec.tv_sec;

        times[0].tv_usec = setAccessTime ? static_cast<__darwin_suseconds_t> ((accessTime % 1000) * 1000)
                                         : static_cast<__darwin_suseconds_t> (info.st_atimespec.tv_nsec / 1000);

        times[1].tv_sec  = setModificationTime ? static_cast<__darwin_time_t> (modificationTime / 1000)
                                               : info.st_mtimespec.tv_sec;

        times[1].tv_usec = setModificationTime ? static_cast<__darwin_suseconds_t> ((modificationTime % 1000) * 1000)
                                               : static_cast<__darwin_suseconds_t> (info.st_mtimespec.tv_nsec / 1000);

        return utimes (fullPath.toUTF8(), times) == 0;
       #else
        struct utimbuf times;
        times.actime  = accessTime != 0       ? static_cast<time_t> (accessTime / 1000)       : static_cast<time_t> (info.st_atime);
        times.modtime = modificationTime != 0 ? static_cast<time_t> (modificationTime / 1000) : static_cast<time_t> (info.st_mtime);

        return utime (fullPath.toUTF8(), &times) == 0;
       #endif
    }
   #endif

    return false;
}

b8 File::deleteFile() const
{
    if (! isSymbolicLink())
    {
        if (! exists())
            return true;

        if (isDirectory())
            return rmdir (fullPath.toUTF8()) == 0;
    }

    return remove (fullPath.toUTF8()) == 0;
}

b8 File::moveInternal (const File& dest) const
{
    if (rename (fullPath.toUTF8(), dest.getFullPathName().toUTF8()) == 0)
        return true;

    if (isNonEmptyDirectory())
        return false;

    if (hasWriteAccess() && copyInternal (dest))
    {
        if (deleteFile())
            return true;

        dest.deleteFile();
    }

    return false;
}

b8 File::replaceInternal (const File& dest) const
{
    return moveInternal (dest);
}

Result File::createDirectoryInternal (const Txt& fileName) const
{
    return getResultForReturnValue (mkdir (fileName.toUTF8(), 0777));
}

//==============================================================================
z64 drx_fileSetPosition (detail::NativeFileHandle handle, z64 pos)
{
    if (handle.isValid() && lseek (handle.get(), (off_t) pos, SEEK_SET) == pos)
        return pos;

    return -1;
}

z0 FileInputStream::openHandle()
{
    auto f = open (file.getFullPathName().toUTF8(), O_RDONLY);

    if (f != -1)
        fileHandle.set (f);
    else
        status = getResultForErrno();
}

FileInputStream::~FileInputStream()
{
    if (fileHandle.isValid())
        close (fileHandle.get());
}

size_t FileInputStream::readInternal (uk buffer, size_t numBytes)
{
    ssize_t result = 0;

    if (fileHandle.isValid())
    {
        result = ::read (fileHandle.get(), buffer, numBytes);

        if (result < 0)
        {
            status = getResultForErrno();
            result = 0;
        }
    }

    return (size_t) result;
}

//==============================================================================
z0 FileOutputStream::openHandle()
{
    if (file.exists())
    {
        auto f = open (file.getFullPathName().toUTF8(), O_RDWR);

        if (f != -1)
        {
            currentPosition = lseek (f, 0, SEEK_END);

            if (currentPosition >= 0)
            {
                fileHandle.set (f);
            }
            else
            {
                status = getResultForErrno();
                close (f);
            }
        }
        else
        {
            status = getResultForErrno();
        }
    }
    else
    {
        auto f = open (file.getFullPathName().toUTF8(), O_RDWR | O_CREAT, 00644);

        if (f != -1)
            fileHandle.set (f);
        else
            status = getResultForErrno();
    }
}

z0 FileOutputStream::closeHandle()
{
    if (fileHandle.isValid())
    {
        close (fileHandle.get());
        fileHandle.invalidate();
    }
}

ssize_t FileOutputStream::writeInternal (ukk data, size_t numBytes)
{
    if (! fileHandle.isValid())
        return 0;

    auto result = ::write (fileHandle.get(), data, numBytes);

    if (result == -1)
        status = getResultForErrno();

    return (ssize_t) result;
}

#ifndef DRX_ANDROID
z0 FileOutputStream::flushInternal()
{
    if (fileHandle.isValid() && fsync (fileHandle.get()) == -1)
        status = getResultForErrno();
}
#endif

Result FileOutputStream::truncate()
{
    if (! fileHandle.isValid())
        return status;

    flush();
    return getResultForReturnValue (ftruncate (fileHandle.get(), (off_t) currentPosition));
}

//==============================================================================
Txt SystemStats::getEnvironmentVariable (const Txt& name, const Txt& defaultValue)
{
    if (auto s = ::getenv (name.toUTF8()))
        return Txt::fromUTF8 (s);

    return defaultValue;
}

//==============================================================================
#if ! DRX_WASM
z0 MemoryMappedFile::openInternal (const File& file, AccessMode mode, b8 exclusive)
{
    jassert (mode == readOnly || mode == readWrite);

    if (range.getStart() > 0)
    {
        auto pageSize = sysconf (_SC_PAGE_SIZE);
        range.setStart (range.getStart() - (range.getStart() % pageSize));
    }

    auto filename = file.getFullPathName().toUTF8();

    if (mode == readWrite)
        fileHandle = open (filename, O_CREAT | O_RDWR, 00644);
    else
        fileHandle = open (filename, O_RDONLY);

    if (fileHandle != -1)
    {
        auto m = mmap (nullptr, (size_t) range.getLength(),
                       mode == readWrite ? (PROT_READ | PROT_WRITE) : PROT_READ,
                       exclusive ? MAP_PRIVATE : MAP_SHARED, fileHandle,
                       (off_t) range.getStart());

        if (m != MAP_FAILED)
        {
            address = m;
            madvise (m, (size_t) range.getLength(), MADV_SEQUENTIAL);
        }
        else
        {
            range = Range<z64>();
        }

        close (fileHandle);
        fileHandle = 0;
    }
}

MemoryMappedFile::~MemoryMappedFile()
{
    if (address != nullptr)
        munmap (address, (size_t) range.getLength());

    if (fileHandle != 0)
        close (fileHandle);
}

//==============================================================================
File drx_getExecutableFile();
File drx_getExecutableFile()
{
    struct DLAddrReader
    {
        static Txt getFilename()
        {
            Dl_info exeInfo;

            auto localSymbol = (uk) drx_getExecutableFile;
            dladdr (localSymbol, &exeInfo);
            return CharPointer_UTF8 (exeInfo.dli_fname);
        }
    };

    static Txt filename = DLAddrReader::getFilename();
    return File::getCurrentWorkingDirectory().getChildFile (filename);
}

//==============================================================================
z64 File::getBytesFreeOnVolume() const
{
    struct statfs buf;

    if (drx_doStatFS (*this, buf))
        return (z64) buf.f_bsize * (z64) buf.f_bavail; // Note: this returns space available to non-super user

    return 0;
}

z64 File::getVolumeTotalSize() const
{
    struct statfs buf;

    if (drx_doStatFS (*this, buf))
        return (z64) buf.f_bsize * (z64) buf.f_blocks;

    return 0;
}

Txt File::getVolumeLabel() const
{
   #if DRX_MAC
    struct VolAttrBuf
    {
        u_int32_t       length;
        attrreference_t mountPointRef;
        t8            mountPointSpace[MAXPATHLEN];
    } attrBuf;

    struct attrlist attrList;
    zerostruct (attrList); // (can't use "= {}" on this object because it's a C struct)
    attrList.bitmapcount = ATTR_BIT_MAP_COUNT;
    attrList.volattr = ATTR_VOL_INFO | ATTR_VOL_NAME;

    File f (*this);

    for (;;)
    {
        if (getattrlist (f.getFullPathName().toUTF8(), &attrList, &attrBuf, sizeof (attrBuf), 0) == 0)
            return Txt::fromUTF8 (((tukk) &attrBuf.mountPointRef) + attrBuf.mountPointRef.attr_dataoffset,
                                     (i32) attrBuf.mountPointRef.attr_length);

        auto parent = f.getParentDirectory();

        if (f == parent)
            break;

        f = parent;
    }
   #endif

    return {};
}

i32 File::getVolumeSerialNumber() const
{
    return 0;
}

#endif

//==============================================================================
#if ! DRX_IOS
z0 drx_runSystemCommand (const Txt&);
z0 drx_runSystemCommand (const Txt& command)
{
    [[maybe_unused]] i32 result = system (command.toUTF8());
}

Txt drx_getOutputFromCommand (const Txt&);
Txt drx_getOutputFromCommand (const Txt& command)
{
    // slight bodge here, as we just pipe the output into a temp file and read it...
    auto tempFile = File::getSpecialLocation (File::tempDirectory)
                      .getNonexistentChildFile (Txt::toHexString (Random::getSystemRandom().nextInt()), ".tmp", false);

    drx_runSystemCommand (command + " > " + tempFile.getFullPathName());

    auto result = tempFile.loadFileAsString();
    tempFile.deleteFile();
    return result;
}
#endif

//==============================================================================
#if DRX_IOS
class InterProcessLock::Pimpl
{
public:
    Pimpl (const Txt&, i32)  {}

    i32 handle = 1, refCount = 1;  // On iOS just fake success..
};

#else

class InterProcessLock::Pimpl
{
public:
    Pimpl (const Txt& lockName, i32 timeOutMillisecs)
    {
       #if DRX_MAC
        if (! createLockFile (File ("~/Library/Caches/com.drx.locks").getChildFile (lockName), timeOutMillisecs))
            // Fallback if the user's home folder is on a network drive with no ability to lock..
            createLockFile (File ("/tmp/com.drx.locks").getChildFile (lockName), timeOutMillisecs);

       #else
        File tempFolder ("/var/tmp");

        if (! tempFolder.isDirectory())
            tempFolder = "/tmp";

        createLockFile (tempFolder.getChildFile (lockName), timeOutMillisecs);
       #endif
    }

    ~Pimpl()
    {
        closeFile();
    }

    b8 createLockFile (const File& file, i32 timeOutMillisecs)
    {
        file.create();
        handle = open (file.getFullPathName().toUTF8(), O_RDWR);

        if (handle != 0)
        {
            struct flock fl;
            zerostruct (fl);

            fl.l_whence = SEEK_SET;
            fl.l_type = F_WRLCK;

            auto endTime = Time::currentTimeMillis() + timeOutMillisecs;

            for (;;)
            {
                auto result = fcntl (handle, F_SETLK, &fl);

                if (result >= 0)
                    return true;

                auto error = errno;

                if (error != EINTR)
                {
                    if (error == EBADF || error == ENOTSUP)
                        return false;

                    if (timeOutMillisecs == 0
                         || (timeOutMillisecs > 0 && Time::currentTimeMillis() >= endTime))
                        break;

                    Thread::sleep (10);
                }
            }
        }

        closeFile();
        return true; // only false if there's a file system error. Failure to lock still returns true.
    }

    z0 closeFile()
    {
        if (handle != 0)
        {
            struct flock fl;
            zerostruct (fl);

            fl.l_whence = SEEK_SET;
            fl.l_type = F_UNLCK;

            while (! (fcntl (handle, F_SETLKW, &fl) >= 0 || errno != EINTR))
            {}

            close (handle);
            handle = 0;
        }
    }

    i32 handle = 0, refCount = 1;
};
#endif

InterProcessLock::InterProcessLock (const Txt& nm)  : name (nm)
{
}

InterProcessLock::~InterProcessLock()
{
}

b8 InterProcessLock::enter (i32 timeOutMillisecs)
{
    const ScopedLock sl (lock);

    if (pimpl == nullptr)
    {
        pimpl.reset (new Pimpl (name, timeOutMillisecs));

        if (pimpl->handle == 0)
            pimpl.reset();
    }
    else
    {
        pimpl->refCount++;
    }

    return pimpl != nullptr;
}

z0 InterProcessLock::exit()
{
    const ScopedLock sl (lock);

    // Trying to release the lock too many times!
    jassert (pimpl != nullptr);

    if (pimpl != nullptr && --(pimpl->refCount) == 0)
        pimpl.reset();
}

class PosixThreadAttribute
{
public:
    explicit PosixThreadAttribute (size_t stackSize)
    {
        if (valid && stackSize != 0)
            pthread_attr_setstacksize (&attr, stackSize);
    }

    ~PosixThreadAttribute()
    {
        if (valid)
            pthread_attr_destroy (&attr);
    }

    auto* get() { return valid ? &attr : nullptr; }

private:
    pthread_attr_t attr;
    b8 valid { pthread_attr_init (&attr) == 0 };
};

class PosixSchedulerPriority
{
public:
    static PosixSchedulerPriority findCurrentSchedulerAndPriority()
    {
        i32 scheduler{};
        sched_param param{};
        pthread_getschedparam (pthread_self(), &scheduler, &param);
        return { scheduler, param.sched_priority };
    }

    static PosixSchedulerPriority getNativeSchedulerAndPriority (const Optional<Thread::RealtimeOptions>& rt,
                                                                 [[maybe_unused]] Thread::Priority prio)
    {
        const auto isRealtime = rt.hasValue();

        const auto priority = [&]
        {
            if (isRealtime)
            {
                const auto min = jmax (0, sched_get_priority_min (SCHED_RR));
                const auto max = jmax (1, sched_get_priority_max (SCHED_RR));

                return jmap (rt->getPriority(), 0, 10, min, max);
            }

            // We only use this helper if we're on an old macos/ios platform that might
            // still respect legacy pthread priorities for SCHED_OTHER.
            #if DRX_MAC || DRX_IOS
             const auto min = jmax (0, sched_get_priority_min (SCHED_OTHER));
             const auto max = jmax (0, sched_get_priority_max (SCHED_OTHER));

             const auto p = [prio]
             {
                 switch (prio)
                 {
                     case Thread::Priority::highest:    return 4;
                     case Thread::Priority::high:       return 3;
                     case Thread::Priority::normal:     return 2;
                     case Thread::Priority::low:        return 1;
                     case Thread::Priority::background: return 0;
                 }

                 return 3;
             }();

             if (min != 0 && max != 0)
                 return jmap (p, 0, 4, min, max);
            #endif

            return 0;
        }();

        #if DRX_MAC || DRX_IOS || DRX_BSD
         const auto scheduler = SCHED_OTHER;
        #elif DRX_LINUX
         const auto backgroundSched = prio == Thread::Priority::background ? SCHED_IDLE
                                                                           : SCHED_OTHER;
         const auto scheduler = isRealtime ? SCHED_RR : backgroundSched;
        #else
         const auto scheduler = 0;
        #endif

         return { scheduler, priority };
    }

    z0 apply ([[maybe_unused]] PosixThreadAttribute& attr) const
    {
        #if DRX_LINUX || DRX_BSD
         const struct sched_param param { getPriority() };

         pthread_attr_setinheritsched (attr.get(), PTHREAD_EXPLICIT_SCHED);
         pthread_attr_setschedpolicy (attr.get(), getScheduler());
         pthread_attr_setschedparam (attr.get(), &param);
        #endif
    }

    constexpr i32 getScheduler() const { return scheduler; }
    constexpr i32  getPriority() const { return priority; }

private:
    constexpr PosixSchedulerPriority (i32 schedulerIn, i32 priorityIn)
        : scheduler (schedulerIn), priority (priorityIn) {}

    i32 scheduler;
    i32 priority;
};

static uk makeThreadHandle (PosixThreadAttribute& attr, uk userData, uk (*threadEntryProc) (uk))
{
    pthread_t handle = {};

    const auto status = pthread_create (&handle, attr.get(), threadEntryProc, userData);

    if (status != 0)
        return nullptr;

    pthread_detach (handle);
    return (uk) handle;
}

z0 Thread::closeThreadHandle()
{
    threadId = {};
    threadHandle = nullptr;
}

z0 DRX_CALLTYPE Thread::setCurrentThreadName (const Txt& name)
{
   #if DRX_IOS || DRX_MAC
    DRX_AUTORELEASEPOOL
    {
        [[NSThread currentThread] setName: juceStringToNS (name)];
    }
   #elif DRX_LINUX || DRX_BSD || DRX_ANDROID
    #if (DRX_BSD \
          || (DRX_LINUX && (__GLIBC__ * 1000 + __GLIBC_MINOR__) >= 2012) \
          || (DRX_ANDROID && __ANDROID_API__ >= 9))
     pthread_setname_np (pthread_self(), name.toRawUTF8());
    #else
     prctl (PR_SET_NAME, name.toRawUTF8(), 0, 0, 0);
    #endif
   #endif
}

Thread::ThreadID DRX_CALLTYPE Thread::getCurrentThreadId()
{
    return (ThreadID) pthread_self();
}

z0 DRX_CALLTYPE Thread::yield()
{
    sched_yield();
}

//==============================================================================
/* Remove this macro if you're having problems compiling the cpu affinity
   calls (the API for these has changed about quite a bit in various Linux
   versions, and a lot of distros seem to ship with obsolete versions)
*/
#if defined (CPU_ISSET) && ! defined (SUPPORT_AFFINITIES)
 #define SUPPORT_AFFINITIES 1
#endif

z0 DRX_CALLTYPE Thread::setCurrentThreadAffinityMask ([[maybe_unused]] u32 affinityMask)
{
   #if SUPPORT_AFFINITIES
    cpu_set_t affinity;
    CPU_ZERO (&affinity);

    for (i32 i = 0; i < 32; ++i)
    {
        if ((affinityMask & (u32) (1 << i)) != 0)
        {
            // GCC 12 on FreeBSD complains about CPU_SET irrespective of
            // the type of the first argument
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")
            CPU_SET ((size_t) i, &affinity);
            DRX_END_IGNORE_WARNINGS_GCC_LIKE
        }
    }

   #if (! DRX_ANDROID) && ((! (DRX_LINUX || DRX_BSD)) || ((__GLIBC__ * 1000 + __GLIBC_MINOR__) >= 2004))
    pthread_setaffinity_np (pthread_self(), sizeof (cpu_set_t), &affinity);
   #elif DRX_ANDROID
    sched_setaffinity (gettid(), sizeof (cpu_set_t), &affinity);
   #else
    // NB: this call isn't really correct because it sets the affinity of the process,
    // (getpid) not the thread (not gettid). But it's included here as a fallback for
    // people who are using ridiculously old versions of glibc
    sched_setaffinity (getpid(), sizeof (cpu_set_t), &affinity);
   #endif

    sched_yield();

   #else
    // affinities aren't supported because either the appropriate header files weren't found,
    // or the SUPPORT_AFFINITIES macro was turned off
    jassertfalse;
   #endif
}

//==============================================================================
#if ! DRX_WASM
b8 DynamicLibrary::open (const Txt& name)
{
    close();
    handle = dlopen (name.isEmpty() ? nullptr : name.toUTF8().getAddress(), RTLD_LOCAL | RTLD_NOW);
    return handle != nullptr;
}

z0 DynamicLibrary::close()
{
    if (handle != nullptr)
    {
        dlclose (handle);
        handle = nullptr;
    }
}

uk DynamicLibrary::getFunction (const Txt& functionName) noexcept
{
    return handle != nullptr ? dlsym (handle, functionName.toUTF8()) : nullptr;
}

//==============================================================================
#if DRX_LINUX || DRX_ANDROID
static Txt readPosixConfigFileValue (tukk file, tukk key)
{
    StringArray lines;
    File (file).readLines (lines);

    for (i32 i = lines.size(); --i >= 0;) // (NB - it's important that this runs in reverse order)
        if (lines[i].upToFirstOccurrenceOf (":", false, false).trim().equalsIgnoreCase (key))
            return lines[i].fromFirstOccurrenceOf (":", false, false).trim();

    return {};
}
#endif


//==============================================================================
class ChildProcess::ActiveProcess
{
public:
    ActiveProcess (const StringArray& arguments, i32 streamFlags)
    {
        auto exe = arguments[0].unquoted();

        // Looks like you're trying to launch a non-existent exe or a folder (perhaps on OSX
        // you're trying to launch the .app folder rather than the actual binary inside it?)
        jassert (File::getCurrentWorkingDirectory().getChildFile (exe).existsAsFile()
                  || ! exe.containsChar (File::getSeparatorChar()));

        i32 pipeHandles[2] = {};

        if (pipe (pipeHandles) == 0)
        {
            auto result = fork();

            if (result < 0)
            {
                close (pipeHandles[0]);
                close (pipeHandles[1]);
            }
            else if (result == 0)
            {
                // we're the child process..
                close (pipeHandles[0]);   // close the read handle

                if ((streamFlags & wantStdOut) != 0)
                    dup2 (pipeHandles[1], STDOUT_FILENO); // turns the pipe into stdout
                else
                    dup2 (open ("/dev/null", O_WRONLY), STDOUT_FILENO);

                if ((streamFlags & wantStdErr) != 0)
                    dup2 (pipeHandles[1], STDERR_FILENO);
                else
                    dup2 (open ("/dev/null", O_WRONLY), STDERR_FILENO);

                close (pipeHandles[1]);

                Array<tuk> argv;

                for (auto& arg : arguments)
                    if (arg.isNotEmpty())
                        argv.add (const_cast<tuk> (arg.toRawUTF8()));

                argv.add (nullptr);

                execvp (exe.toRawUTF8(), argv.getRawDataPointer());
                _exit (-1);
            }
            else
            {
                // we're the parent process..
                childPID = result;
                pipeHandle = pipeHandles[0];
                close (pipeHandles[1]); // close the write handle
            }
        }
    }

    ~ActiveProcess()
    {
        if (readHandle != nullptr)
            fclose (readHandle);

        if (pipeHandle != 0)
            close (pipeHandle);
    }

    b8 isRunning() noexcept
    {
        if (childPID == 0)
            return false;

        i32 childState = 0;
        auto pid = waitpid (childPID, &childState, WNOHANG);

        if (pid == 0)
            return true;

        if (WIFEXITED (childState))
        {
            exitCode = WEXITSTATUS (childState);
            return false;
        }

        return ! WIFSIGNALED (childState);
    }

    i32 read (uk dest, i32 numBytes) noexcept
    {
        jassert (dest != nullptr && numBytes > 0);

        #ifdef fdopen
         #error // some crazy 3rd party headers (e.g. zlib) define this function as NULL!
        #endif

        if (readHandle == nullptr && childPID != 0)
            readHandle = fdopen (pipeHandle, "r");

        if (readHandle != nullptr)
        {
            for (;;)
            {
                auto numBytesRead = (i32) fread (dest, 1, (size_t) numBytes, readHandle);

                if (numBytesRead > 0 || feof (readHandle))
                    return numBytesRead;

                // signal occurred during fread() so try again
                if (ferror (readHandle) && errno == EINTR)
                    continue;

                break;
            }
        }

        return 0;
    }

    b8 killProcess() const noexcept
    {
        return ::kill (childPID, SIGKILL) == 0;
    }

    u32 getExitCode() noexcept
    {
        if (exitCode >= 0)
            return (u32) exitCode;

        if (childPID != 0)
        {
            i32 childState = 0;
            auto pid = waitpid (childPID, &childState, WNOHANG);

            if (pid >= 0 && WIFEXITED (childState))
            {
                exitCode = WEXITSTATUS (childState);
                return (u32) exitCode;
            }
        }

        return 0;
    }

    i32 childPID = 0;
    i32 pipeHandle = 0;
    i32 exitCode = -1;
    fuk readHandle = {};

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActiveProcess)
};

b8 ChildProcess::start (const Txt& command, i32 streamFlags)
{
    return start (StringArray::fromTokens (command, true), streamFlags);
}

b8 ChildProcess::start (const StringArray& args, i32 streamFlags)
{
    if (args.size() == 0)
        return false;

    activeProcess.reset (new ActiveProcess (args, streamFlags));

    if (activeProcess->childPID == 0)
        activeProcess.reset();

    return activeProcess != nullptr;
}

#endif

} // namespace drx
