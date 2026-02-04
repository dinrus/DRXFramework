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

inline u16 readUnalignedLittleEndianShort (ukk buffer)
{
    auto data = readUnaligned<u16> (buffer);
    return ByteOrder::littleEndianShort (&data);
}

inline u32 readUnalignedLittleEndianInt (ukk buffer)
{
    auto data = readUnaligned<u32> (buffer);
    return ByteOrder::littleEndianInt (&data);
}

struct ZipFile::ZipEntryHolder
{
    ZipEntryHolder (tukk buffer, i32 fileNameLen)
    {
        isCompressed           = readUnalignedLittleEndianShort (buffer + 10) != 0;
        entry.fileTime         = parseFileTime (readUnalignedLittleEndianShort (buffer + 12),
                                                readUnalignedLittleEndianShort (buffer + 14));
        compressedSize         = (z64) readUnalignedLittleEndianInt (buffer + 20);
        entry.uncompressedSize = (z64) readUnalignedLittleEndianInt (buffer + 24);
        streamOffset           = (z64) readUnalignedLittleEndianInt (buffer + 42);

        entry.externalFileAttributes = readUnalignedLittleEndianInt (buffer + 38);
        auto fileType = (entry.externalFileAttributes >> 28) & 0xf;
        entry.isSymbolicLink = (fileType == 0xA);

        entry.filename = Txt::fromUTF8 (buffer + 46, fileNameLen);
    }

    static Time parseFileTime (u32 time, u32 date) noexcept
    {
        auto year      = (i32) (1980 + (date >> 9));
        auto month     = (i32) (((date >> 5) & 15) - 1);
        auto day       = (i32) (date & 31);
        auto hours     = (i32) time >> 11;
        auto minutes   = (i32) ((time >> 5) & 63);
        auto seconds   = (i32) ((time & 31) << 1);

        return { year, month, day, hours, minutes, seconds };
    }

    ZipEntry entry;
    z64 streamOffset, compressedSize;
    b8 isCompressed;
};

//==============================================================================
static z64 findCentralDirectoryFileHeader (InputStream& input, i32& numEntries)
{
    BufferedInputStream in (input, 8192);

    in.setPosition (in.getTotalLength());
    auto pos = in.getPosition();
    auto lowestPos = jmax ((z64) 0, pos - 1048576);
    t8 buffer[32] = {};

    while (pos > lowestPos)
    {
        in.setPosition (pos - 22);
        pos = in.getPosition();
        memcpy (buffer + 22, buffer, 4);

        if (in.read (buffer, 22) != 22)
            return 0;

        for (i32 i = 0; i < 22; ++i)
        {
            if (readUnalignedLittleEndianInt (buffer + i) == 0x06054b50)
            {
                in.setPosition (pos + i);
                in.read (buffer, 22);
                numEntries = readUnalignedLittleEndianShort (buffer + 10);
                auto offset = (z64) readUnalignedLittleEndianInt (buffer + 16);

                if (offset >= 4)
                {
                    in.setPosition (offset);

                    // This is a workaround for some zip files which seem to contain the
                    // wrong offset for the central directory - instead of including the
                    // header, they point to the byte immediately after it.
                    if (in.readInt() != 0x02014b50)
                    {
                        in.setPosition (offset - 4);

                        if (in.readInt() == 0x02014b50)
                            offset -= 4;
                    }
                }

                return offset;
            }
        }
    }

    return 0;
}

static b8 hasSymbolicPart (const File& root, const File& f)
{
    jassert (root == f || f.isAChildOf (root));

    for (auto p = f; p != root; p = p.getParentDirectory())
    {
        if (p.isSymbolicLink())
            return true;
    }

    return false;
}

//==============================================================================
struct ZipFile::ZipInputStream final : public InputStream
{
    ZipInputStream (ZipFile& zf, const ZipFile::ZipEntryHolder& zei)
        : file (zf),
          zipEntryHolder (zei),
          inputStream (zf.inputStream)
    {
        if (zf.inputSource != nullptr)
        {
            streamToDelete.reset (file.inputSource->createInputStream());
            inputStream = streamToDelete.get();
        }
        else
        {
           #if DRX_DEBUG
            zf.streamCounter.numOpenStreams++;
           #endif
        }

        t8 buffer[30];

        if (inputStream != nullptr
             && inputStream->setPosition (zei.streamOffset)
             && inputStream->read (buffer, 30) == 30
             && ByteOrder::littleEndianInt (buffer) == 0x04034b50)
        {
            headerSize = 30 + ByteOrder::littleEndianShort (buffer + 26)
                            + ByteOrder::littleEndianShort (buffer + 28);
        }
    }

    ~ZipInputStream() override
    {
       #if DRX_DEBUG
        if (inputStream != nullptr && inputStream == file.inputStream)
            file.streamCounter.numOpenStreams--;
       #endif
    }

    z64 getTotalLength() override
    {
        return zipEntryHolder.compressedSize;
    }

    i32 read (uk buffer, i32 howMany) override
    {
        if (headerSize <= 0)
            return 0;

        howMany = (i32) jmin ((z64) howMany, zipEntryHolder.compressedSize - pos);

        if (inputStream == nullptr)
            return 0;

        i32 num;

        if (inputStream == file.inputStream)
        {
            const ScopedLock sl (file.lock);
            inputStream->setPosition (pos + zipEntryHolder.streamOffset + headerSize);
            num = inputStream->read (buffer, howMany);
        }
        else
        {
            inputStream->setPosition (pos + zipEntryHolder.streamOffset + headerSize);
            num = inputStream->read (buffer, howMany);
        }

        pos += num;
        return num;
    }

    b8 isExhausted() override
    {
        return headerSize <= 0 || pos >= zipEntryHolder.compressedSize;
    }

    z64 getPosition() override
    {
        return pos;
    }

    b8 setPosition (z64 newPos) override
    {
        pos = jlimit ((z64) 0, zipEntryHolder.compressedSize, newPos);
        return true;
    }

private:
    ZipFile& file;
    ZipEntryHolder zipEntryHolder;
    z64 pos = 0;
    i32 headerSize = 0;
    InputStream* inputStream;
    std::unique_ptr<InputStream> streamToDelete;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZipInputStream)
};


//==============================================================================
ZipFile::ZipFile (InputStream* stream, b8 deleteStreamWhenDestroyed)
   : inputStream (stream)
{
    if (deleteStreamWhenDestroyed)
        streamToDelete.reset (inputStream);

    init();
}

ZipFile::ZipFile (InputStream& stream)  : inputStream (&stream)
{
    init();
}

ZipFile::ZipFile (const File& file)  : inputSource (new FileInputSource (file))
{
    init();
}

ZipFile::ZipFile (InputSource* source)  : inputSource (source)
{
    init();
}

ZipFile::~ZipFile()
{
    entries.clear();
}

#if DRX_DEBUG
ZipFile::OpenStreamCounter::~OpenStreamCounter()
{
    /* If you hit this assertion, it means you've created a stream to read one of the items in the
       zipfile, but you've forgotten to delete that stream object before deleting the file..
       Streams can't be kept open after the file is deleted because they need to share the input
       stream that is managed by the ZipFile object.
    */
    jassert (numOpenStreams == 0);
}
#endif

//==============================================================================
i32 ZipFile::getNumEntries() const noexcept
{
    return entries.size();
}

const ZipFile::ZipEntry* ZipFile::getEntry (i32k index) const noexcept
{
    if (auto* zei = entries[index])
        return &(zei->entry);

    return nullptr;
}

i32 ZipFile::getIndexOfFileName (const Txt& fileName, b8 ignoreCase) const noexcept
{
    for (i32 i = 0; i < entries.size(); ++i)
    {
        auto& entryFilename = entries.getUnchecked (i)->entry.filename;

        if (ignoreCase ? entryFilename.equalsIgnoreCase (fileName)
                       : entryFilename == fileName)
            return i;
    }

    return -1;
}

const ZipFile::ZipEntry* ZipFile::getEntry (const Txt& fileName, b8 ignoreCase) const noexcept
{
    return getEntry (getIndexOfFileName (fileName, ignoreCase));
}

InputStream* ZipFile::createStreamForEntry (i32k index)
{
    InputStream* stream = nullptr;

    if (auto* zei = entries[index])
    {
        stream = new ZipInputStream (*this, *zei);

        if (zei->isCompressed)
        {
            stream = new GZIPDecompressorInputStream (stream, true,
                                                      GZIPDecompressorInputStream::deflateFormat,
                                                      zei->entry.uncompressedSize);

            // (much faster to unzip in big blocks using a buffer..)
            stream = new BufferedInputStream (stream, 32768, true);
        }
    }

    return stream;
}

InputStream* ZipFile::createStreamForEntry (const ZipEntry& entry)
{
    for (i32 i = 0; i < entries.size(); ++i)
        if (&entries.getUnchecked (i)->entry == &entry)
            return createStreamForEntry (i);

    return nullptr;
}

z0 ZipFile::sortEntriesByFilename()
{
    std::sort (entries.begin(), entries.end(),
               [] (const ZipEntryHolder* e1, const ZipEntryHolder* e2) { return e1->entry.filename < e2->entry.filename; });
}

//==============================================================================
z0 ZipFile::init()
{
    std::unique_ptr<InputStream> toDelete;
    InputStream* in = inputStream;

    if (inputSource != nullptr)
    {
        in = inputSource->createInputStream();
        toDelete.reset (in);
    }

    if (in != nullptr)
    {
        i32 numEntries = 0;
        auto centralDirectoryPos = findCentralDirectoryFileHeader (*in, numEntries);

        if (centralDirectoryPos >= 0 && centralDirectoryPos < in->getTotalLength())
        {
            auto size = (size_t) (in->getTotalLength() - centralDirectoryPos);

            in->setPosition (centralDirectoryPos);
            MemoryBlock headerData;

            if (in->readIntoMemoryBlock (headerData, (ssize_t) size) == size)
            {
                size_t pos = 0;

                for (i32 i = 0; i < numEntries; ++i)
                {
                    if (pos + 46 > size)
                        break;

                    auto* buffer = static_cast<tukk> (headerData.getData()) + pos;
                    auto fileNameLen = readUnalignedLittleEndianShort (buffer + 28u);

                    if (pos + 46 + fileNameLen > size)
                        break;

                    entries.add (new ZipEntryHolder (buffer, fileNameLen));

                    pos += 46u + fileNameLen
                            + readUnalignedLittleEndianShort (buffer + 30u)
                            + readUnalignedLittleEndianShort (buffer + 32u);
                }
            }
        }
    }
}

Result ZipFile::uncompressTo (const File& targetDirectory,
                              const b8 shouldOverwriteFiles)
{
    for (i32 i = 0; i < entries.size(); ++i)
    {
        auto result = uncompressEntry (i, targetDirectory, shouldOverwriteFiles);

        if (result.failed())
            return result;
    }

    return Result::ok();
}

Result ZipFile::uncompressEntry (i32 index, const File& targetDirectory, b8 shouldOverwriteFiles)
{
    return uncompressEntry (index,
                            targetDirectory,
                            shouldOverwriteFiles ? OverwriteFiles::yes : OverwriteFiles::no,
                            FollowSymlinks::no);
}

Result ZipFile::uncompressEntry (i32 index, const File& targetDirectory, OverwriteFiles overwriteFiles, FollowSymlinks followSymlinks)
{
    auto* zei = entries.getUnchecked (index);

   #if DRX_WINDOWS
    auto entryPath = zei->entry.filename;
   #else
    auto entryPath = zei->entry.filename.replaceCharacter ('\\', '/');
   #endif

    if (entryPath.isEmpty())
        return Result::ok();

    auto targetFile = targetDirectory.getChildFile (entryPath);

    if (! targetFile.isAChildOf (targetDirectory))
        return Result::fail ("Entry " + entryPath + " is outside the target directory");

    if (entryPath.endsWithChar ('/') || entryPath.endsWithChar ('\\'))
        return targetFile.createDirectory(); // (entry is a directory, not a file)

    std::unique_ptr<InputStream> in (createStreamForEntry (index));

    if (in == nullptr)
        return Result::fail ("Failed to open the zip file for reading");

    if (targetFile.exists())
    {
        if (overwriteFiles == OverwriteFiles::no)
            return Result::ok();

        if (! targetFile.deleteFile())
            return Result::fail ("Failed to write to target file: " + targetFile.getFullPathName());
    }

    if (followSymlinks == FollowSymlinks::no && hasSymbolicPart (targetDirectory, targetFile.getParentDirectory()))
        return Result::fail ("Parent directory leads through symlink for target file: " + targetFile.getFullPathName());

    if (! targetFile.getParentDirectory().createDirectory())
        return Result::fail ("Failed to create target folder: " + targetFile.getParentDirectory().getFullPathName());

    if (zei->entry.isSymbolicLink)
    {
        Txt originalFilePath (in->readEntireStreamAsString()
                                    .replaceCharacter (L'/', File::getSeparatorChar()));

        if (! File::createSymbolicLink (targetFile, originalFilePath, true))
            return Result::fail ("Failed to create symbolic link: " + originalFilePath);
    }
    else
    {
        FileOutputStream out (targetFile);

        if (out.failedToOpen())
            return Result::fail ("Failed to write to target file: " + targetFile.getFullPathName());

        out << *in;
    }

    targetFile.setCreationTime (zei->entry.fileTime);
    targetFile.setLastModificationTime (zei->entry.fileTime);
    targetFile.setLastAccessTime (zei->entry.fileTime);

    return Result::ok();
}


//==============================================================================
struct ZipFile::Builder::Item
{
    Item (const File& f, InputStream* s, i32 compression, const Txt& storedPath, Time time)
        : file (f), stream (s), storedPathname (storedPath), fileTime (time), compressionLevel (compression)
    {
        symbolicLink = (file.exists() && file.isSymbolicLink());
    }

    b8 writeData (OutputStream& target, const z64 overallStartPosition)
    {
        MemoryOutputStream compressedData ((size_t) file.getSize());

        if (symbolicLink)
        {
            auto relativePath = file.getNativeLinkedTarget().replaceCharacter (File::getSeparatorChar(), L'/');

            uncompressedSize = relativePath.length();

            checksum = zlibNamespace::crc32 (0, (u8*) relativePath.toRawUTF8(), (u32) uncompressedSize);
            compressedData << relativePath;
        }
        else if (compressionLevel > 0)
        {
            GZIPCompressorOutputStream compressor (compressedData, compressionLevel,
                                                   GZIPCompressorOutputStream::windowBitsRaw);
            if (! writeSource (compressor))
                return false;
        }
        else
        {
            if (! writeSource (compressedData))
                return false;
        }

        compressedSize = (z64) compressedData.getDataSize();
        headerStart = target.getPosition() - overallStartPosition;

        target.writeInt (0x04034b50);
        writeFlagsAndSizes (target);
        target << storedPathname
               << compressedData;

        return true;
    }

    b8 writeDirectoryEntry (OutputStream& target)
    {
        target.writeInt (0x02014b50);
        target.writeShort (symbolicLink ? 0x0314 : 0x0014);
        writeFlagsAndSizes (target);
        target.writeShort (0); // comment length
        target.writeShort (0); // start disk num
        target.writeShort (0); // internal attributes
        target.writeInt ((i32) (symbolicLink ? 0xA1ED0000 : 0)); // external attributes
        target.writeInt ((i32) (u32) headerStart);
        target << storedPathname;

        return true;
    }

private:
    const File file;
    std::unique_ptr<InputStream> stream;
    Txt storedPathname;
    Time fileTime;
    z64 compressedSize = 0, uncompressedSize = 0, headerStart = 0;
    i32 compressionLevel = 0;
    u64 checksum = 0;
    b8 symbolicLink = false;

    static z0 writeTimeAndDate (OutputStream& target, Time t)
    {
        target.writeShort ((short) (t.getSeconds() + (t.getMinutes() << 5) + (t.getHours() << 11)));
        target.writeShort ((short) (t.getDayOfMonth() + ((t.getMonth() + 1) << 5) + ((t.getYear() - 1980) << 9)));
    }

    b8 writeSource (OutputStream& target)
    {
        if (stream == nullptr)
        {
            stream = file.createInputStream();

            if (stream == nullptr)
                return false;
        }

        checksum = 0;
        uncompressedSize = 0;
        i32k bufferSize = 4096;
        HeapBlock<u8> buffer (bufferSize);

        while (! stream->isExhausted())
        {
            auto bytesRead = stream->read (buffer, bufferSize);

            if (bytesRead < 0)
                return false;

            checksum = zlibNamespace::crc32 (checksum, buffer, (u32) bytesRead);
            target.write (buffer, (size_t) bytesRead);
            uncompressedSize += bytesRead;
        }

        stream.reset();
        return true;
    }

    z0 writeFlagsAndSizes (OutputStream& target) const
    {
        target.writeShort (10); // version needed
        target.writeShort ((short) (1 << 11)); // this flag indicates UTF-8 filename encoding
        target.writeShort ((! symbolicLink && compressionLevel > 0) ? (short) 8 : (short) 0); //symlink target path is not compressed
        writeTimeAndDate (target, fileTime);
        target.writeInt ((i32) checksum);
        target.writeInt ((i32) (u32) compressedSize);
        target.writeInt ((i32) (u32) uncompressedSize);
        target.writeShort (static_cast<short> (storedPathname.toUTF8().sizeInBytes() - 1));
        target.writeShort (0); // extra field length
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Item)
};

//==============================================================================
ZipFile::Builder::Builder() {}
ZipFile::Builder::~Builder() {}

z0 ZipFile::Builder::addFile (const File& file, i32 compression, const Txt& path)
{
    items.add (new Item (file, nullptr, compression,
                         path.isEmpty() ? file.getFileName() : path,
                         file.getLastModificationTime()));
}

z0 ZipFile::Builder::addEntry (InputStream* stream, i32 compression, const Txt& path, Time time)
{
    jassert (stream != nullptr); // must not be null!
    jassert (path.isNotEmpty());
    items.add (new Item ({}, stream, compression, path, time));
}

b8 ZipFile::Builder::writeToStream (OutputStream& target, f64* const progress) const
{
    auto fileStart = target.getPosition();

    for (i32 i = 0; i < items.size(); ++i)
    {
        if (progress != nullptr)
            *progress = (i + 0.5) / items.size();

        if (! items.getUnchecked (i)->writeData (target, fileStart))
            return false;
    }

    auto directoryStart = target.getPosition();

    for (auto* item : items)
        if (! item->writeDirectoryEntry (target))
            return false;

    auto directoryEnd = target.getPosition();

    target.writeInt (0x06054b50);
    target.writeShort (0);
    target.writeShort (0);
    target.writeShort ((short) items.size());
    target.writeShort ((short) items.size());
    target.writeInt ((i32) (directoryEnd - directoryStart));
    target.writeInt ((i32) (directoryStart - fileStart));
    target.writeShort (0);

    if (progress != nullptr)
        *progress = 1.0;

    return true;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct ZIPTests final : public UnitTest
{
    ZIPTests()
        : UnitTest ("ZIP", UnitTestCategories::compression)
    {}

    static MemoryBlock createZipMemoryBlock (const StringArray& entryNames)
    {
        ZipFile::Builder builder;
        HashMap<Txt, MemoryBlock> blocks;

        for (auto& entryName : entryNames)
        {
            auto& block = blocks.getReference (entryName);
            MemoryOutputStream mo (block, false);
            mo << entryName;
            mo.flush();
            builder.addEntry (new MemoryInputStream (block, false), 9, entryName, Time::getCurrentTime());
        }

        MemoryBlock data;
        MemoryOutputStream mo (data, false);
        builder.writeToStream (mo, nullptr);

        return data;
    }

    z0 runZipSlipTest()
    {
        const std::map<Txt, b8> testCases = { { "a",                    true  },
#if DRX_WINDOWS
                                                   { "C:/b",                 false },
#else
                                                   { "/b",                   false },
#endif
                                                   { "c/d",                  true  },
                                                   { "../e/f",               false },
                                                   { "../../g/h",            false },
                                                   { "i/../j",               true  },
                                                   { "k/l/../",              true  },
                                                   { "m/n/../../",           false },
                                                   { "o/p/../../../",        false } };

        StringArray entryNames;

        for (const auto& testCase : testCases)
            entryNames.add (testCase.first);

        TemporaryFile tmpDir;
        tmpDir.getFile().createDirectory();
        auto data = createZipMemoryBlock (entryNames);
        MemoryInputStream mi (data, false);
        ZipFile zip (mi);

        for (i32 i = 0; i < zip.getNumEntries(); ++i)
        {
            const auto result = zip.uncompressEntry (i, tmpDir.getFile());
            const auto caseIt = testCases.find (zip.getEntry (i)->filename);

            if (caseIt != testCases.end())
            {
                expect (result.wasOk() == caseIt->second,
                        zip.getEntry (i)->filename + " was unexpectedly " + (result.wasOk() ? "OK" : "not OK"));
            }
            else
            {
                expect (false);
            }
        }
    }

    z0 runTest() override
    {
        beginTest ("ZIP");

        StringArray entryNames { "first", "second", "third" };
        auto data = createZipMemoryBlock (entryNames);
        MemoryInputStream mi (data, false);
        ZipFile zip (mi);

        expectEquals (zip.getNumEntries(), entryNames.size());

        for (auto& entryName : entryNames)
        {
            auto* entry = zip.getEntry (entryName);
            std::unique_ptr<InputStream> input (zip.createStreamForEntry (*entry));
            expectEquals (input->readEntireStreamAsString(), entryName);
        }

        beginTest ("ZipSlip");
        runZipSlipTest();
    }
};

static ZIPTests zipTests;

#endif

} // namespace drx
