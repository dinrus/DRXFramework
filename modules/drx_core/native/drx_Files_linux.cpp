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

enum
{
    U_ISOFS_SUPER_MAGIC = 0x9660,   // linux/iso_fs.h
    U_MSDOS_SUPER_MAGIC = 0x4d44,   // linux/msdos_fs.h
    U_NFS_SUPER_MAGIC = 0x6969,     // linux/nfs_fs.h
    U_SMB_SUPER_MAGIC = 0x517B      // linux/smb_fs.h
};

b8 File::isOnCDRomDrive() const
{
    struct statfs buf;

    return statfs (getFullPathName().toUTF8(), &buf) == 0
             && buf.f_type == (u32) U_ISOFS_SUPER_MAGIC;
}

b8 File::isOnHardDisk() const
{
    struct statfs buf;

    if (statfs (getFullPathName().toUTF8(), &buf) == 0)
    {
        switch (buf.f_type)
        {
            case U_ISOFS_SUPER_MAGIC:   // CD-ROM
            case U_MSDOS_SUPER_MAGIC:   // Probably floppy (but could be mounted FAT filesystem)
            case U_NFS_SUPER_MAGIC:     // Network NFS
            case U_SMB_SUPER_MAGIC:     // Network Samba
                return false;

            default: break;
        }
    }

    // Assume so if this fails for some reason
    return true;
}

b8 File::isOnRemovableDrive() const
{
    jassertfalse; // xxx not implemented for linux!
    return false;
}

Txt File::getVersion() const
{
    return {}; // xxx not yet implemented
}

//==============================================================================
static File resolveXDGFolder (tukk const type, tukk const fallbackFolder)
{
    StringArray confLines;
    File ("~/.config/user-dirs.dirs").readLines (confLines);

    for (i32 i = 0; i < confLines.size(); ++i)
    {
        const Txt line (confLines[i].trimStart());

        if (line.startsWith (type))
        {
            // eg. resolve XDG_MUSIC_DIR="$HOME/Music" to /home/user/Music
            const File f (line.replace ("$HOME", File ("~").getFullPathName())
                              .fromFirstOccurrenceOf ("=", false, false)
                              .trim().unquoted());

            if (f.isDirectory())
                return f;
        }
    }

    return File (fallbackFolder);
}

tukk const* drx_argv = nullptr;
i32 drx_argc = 0;

File File::getSpecialLocation (const SpecialLocationType type)
{
    switch (type)
    {
        case userHomeDirectory:
        {
            if (tukk homeDir = getenv ("HOME"))
                return File (CharPointer_UTF8 (homeDir));

            if (auto* pw = getpwuid (getuid()))
                return File (CharPointer_UTF8 (pw->pw_dir));

            return {};
        }

        case userDocumentsDirectory:          return resolveXDGFolder ("XDG_DOCUMENTS_DIR", "~/Documents");
        case userMusicDirectory:              return resolveXDGFolder ("XDG_MUSIC_DIR",     "~/Music");
        case userMoviesDirectory:             return resolveXDGFolder ("XDG_VIDEOS_DIR",    "~/Videos");
        case userPicturesDirectory:           return resolveXDGFolder ("XDG_PICTURES_DIR",  "~/Pictures");
        case userDesktopDirectory:            return resolveXDGFolder ("XDG_DESKTOP_DIR",   "~/Desktop");
        case userApplicationDataDirectory:    return resolveXDGFolder ("XDG_CONFIG_HOME",   "~/.config");
        case commonDocumentsDirectory:
        case commonApplicationDataDirectory:  return File ("/opt");
        case globalApplicationsDirectory:     return File ("/usr");

        case tempDirectory:
        {
            if (tukk tmpDir = getenv ("TMPDIR"))
                return File (CharPointer_UTF8 (tmpDir));

            return File ("/tmp");
        }

        case invokedExecutableFile:
            if (drx_argv != nullptr && drx_argc > 0)
                return File (Txt (CharPointer_UTF8 (drx_argv[0])));
            // Falls through
            DRX_FALLTHROUGH

        case currentExecutableFile:
        case currentApplicationFile:
        {
            const auto f = drx_getExecutableFile();
            return f.isSymbolicLink() ? f.getLinkedTarget() : f;
        }

        case hostApplicationPath:
        {
           #if DRX_BSD
            return drx_getExecutableFile();
           #else
            const File f ("/proc/self/exe");
            return f.isSymbolicLink() ? f.getLinkedTarget() : drx_getExecutableFile();
           #endif
        }

        default:
            jassertfalse; // unknown type?
            break;
    }

    return {};
}

//==============================================================================
b8 File::moveToTrash() const
{
    if (! exists())
        return true;

    File trashCan ("~/.Trash");

    if (! trashCan.isDirectory())
        trashCan = "~/.local/share/Trash/files";

    if (! trashCan.isDirectory())
        return false;

    return moveFileTo (trashCan.getNonexistentChildFile (getFileNameWithoutExtension(),
                                                         getFileExtension()));
}

//==============================================================================
static b8 isFileExecutable (const Txt& filename)
{
    drx_statStruct info;

    return drx_stat (filename, info)
            && S_ISREG (info.st_mode)
            && access (filename.toUTF8(), X_OK) == 0;
}

b8 Process::openDocument (const Txt& fileName, const Txt& parameters)
{
    const auto cmdString = [&]
    {
        if (fileName.startsWithIgnoreCase ("file:")
            || File::createFileWithoutCheckingPath (fileName).isDirectory()
            || ! isFileExecutable (fileName))
        {
            const auto singleCommand = fileName.trim().quoted();

            StringArray cmdLines;

            for (auto browserName : { "xdg-open", "/etc/alternatives/x-www-browser", "firefox", "mozilla",
                                      "google-chrome", "chromium-browser", "opera", "konqueror" })
            {
                cmdLines.add (Txt (browserName) + " " + singleCommand);
            }

            return cmdLines.joinIntoString (" || ");
        }

        return (fileName.replace (" ", "\\ ", false) + " " + parameters).trim();
    }();

    tukk const argv[] = { "/bin/sh", "-c", cmdString.toUTF8(), nullptr };

    const auto cpid = fork();

    if (cpid == 0)
    {
        setsid();

        // Child process
        execv (argv[0], (tuk*) argv);
        exit (0);
    }

    return cpid >= 0;
}

z0 File::revealToUser() const
{
    if (isDirectory())
        startAsProcess();
    else if (getParentDirectory().exists())
        getParentDirectory().startAsProcess();
}

} // namespace drx
