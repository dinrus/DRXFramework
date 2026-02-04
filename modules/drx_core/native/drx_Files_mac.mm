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

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in drx_posix_SharedCode.h!
*/

//==============================================================================
b8 File::copyInternal (const File& dest) const
{
    DRX_AUTORELEASEPOOL
    {
        NSFileManager* fm = [NSFileManager defaultManager];

        return [fm fileExistsAtPath: juceStringToNS (fullPath)]
                && [fm copyItemAtPath: juceStringToNS (fullPath)
                               toPath: juceStringToNS (dest.getFullPathName())
                                error: nil];
    }
}

z0 File::findFileSystemRoots (Array<File>& destArray)
{
    destArray.add (File ("/"));
}


//==============================================================================
namespace MacFileHelpers
{
    static b8 isFileOnDriveType (const File& f, tukk const* types)
    {
        struct statfs buf;

        if (drx_doStatFS (f, buf))
        {
            const Txt type (buf.f_fstypename);

            while (*types != nullptr)
                if (type.equalsIgnoreCase (*types++))
                    return true;
        }

        return false;
    }

    static b8 isHiddenFile (const Txt& path)
    {
       #if DRX_MAC
        DRX_AUTORELEASEPOOL
        {
            NSNumber* hidden = nil;
            NSError* err = nil;

            return [createNSURLFromFile (path) getResourceValue: &hidden forKey: NSURLIsHiddenKey error: &err]
                     && [hidden boolValue];
        }
       #else
        return File (path).getFileName().startsWithChar ('.');
       #endif
    }

   #if DRX_IOS
    static Txt getIOSSystemLocation (NSSearchPathDirectory type)
    {
        return nsStringToDrx ([NSSearchPathForDirectoriesInDomains (type, NSUserDomainMask, YES)
                                objectAtIndex: 0]);
    }
   #else
    static b8 launchExecutable (const Txt& pathAndArguments)
    {
        auto cpid = fork();

        if (cpid == 0)
        {
            tukk const argv[4] = { "/bin/sh", "-c", pathAndArguments.toUTF8(), nullptr };

            // Child process
            if (execve (argv[0], (tuk*) argv, nullptr) < 0)
                exit (0);
        }
        else
        {
            if (cpid < 0)
                return false;
        }

        return true;
    }
   #endif
}

b8 File::isOnCDRomDrive() const
{
    static tukk const cdTypes[] = { "cd9660", "cdfs", "cddafs", "udf", nullptr };

    return MacFileHelpers::isFileOnDriveType (*this, cdTypes);
}

b8 File::isOnHardDisk() const
{
    static tukk const nonHDTypes[] = { "nfs", "smbfs", "ramfs", nullptr };

    return ! (isOnCDRomDrive() || MacFileHelpers::isFileOnDriveType (*this, nonHDTypes));
}

b8 File::isOnRemovableDrive() const
{
   #if DRX_IOS
    return false; // xxx is this possible?
   #else
    DRX_AUTORELEASEPOOL
    {
        BOOL removable = false;

        [[NSWorkspace sharedWorkspace]
               getFileSystemInfoForPath: juceStringToNS (getFullPathName())
                            isRemovable: &removable
                             isWritable: nil
                          isUnmountable: nil
                            description: nil
                                   type: nil];

        return removable;
    }
   #endif
}

b8 File::isHidden() const
{
    return MacFileHelpers::isHiddenFile (getFullPathName());
}

//==============================================================================
tukk const* drx_argv = nullptr;
i32 drx_argc = 0;

File File::getSpecialLocation (const SpecialLocationType type)
{
    DRX_AUTORELEASEPOOL
    {
        Txt resultPath;

        switch (type)
        {
            case userHomeDirectory:                 resultPath = nsStringToDrx (NSHomeDirectory()); break;

          #if DRX_IOS
            case userDocumentsDirectory:            resultPath = MacFileHelpers::getIOSSystemLocation (NSDocumentDirectory); break;
            case userDesktopDirectory:              resultPath = MacFileHelpers::getIOSSystemLocation (NSDesktopDirectory); break;

            case tempDirectory:
            {
                File tmp (MacFileHelpers::getIOSSystemLocation (NSCachesDirectory));
                tmp = tmp.getChildFile (drx_getExecutableFile().getFileNameWithoutExtension());
                tmp.createDirectory();
                return tmp.getFullPathName();
            }

          #else
            case userDocumentsDirectory:            resultPath = "~/Documents"; break;
            case userDesktopDirectory:              resultPath = "~/Desktop"; break;

            case tempDirectory:
            {
                File tmp ("~/Library/Caches/" + drx_getExecutableFile().getFileNameWithoutExtension());
                tmp.createDirectory();
                return File (tmp.getFullPathName());
            }
          #endif
            case userMusicDirectory:                resultPath = "~/Music"; break;
            case userMoviesDirectory:               resultPath = "~/Movies"; break;
            case userPicturesDirectory:             resultPath = "~/Pictures"; break;
            case userApplicationDataDirectory:      resultPath = "~/Library"; break;
            case commonApplicationDataDirectory:    resultPath = "/Library"; break;
            case commonDocumentsDirectory:          resultPath = "/Users/Shared"; break;
            case globalApplicationsDirectory:       resultPath = "/Applications"; break;

            case invokedExecutableFile:
                if (drx_argv != nullptr && drx_argc > 0)
                    return File::getCurrentWorkingDirectory().getChildFile (Txt (CharPointer_UTF8 (drx_argv[0])));
                // deliberate fall-through...
                DRX_FALLTHROUGH

            case currentExecutableFile:
                return drx_getExecutableFile();

            case currentApplicationFile:
            {
                const File exe (drx_getExecutableFile());
                const File parent (exe.getParentDirectory());

               #if DRX_IOS
                return parent;
               #else
                return parent.getFullPathName().endsWithIgnoreCase ("Contents/MacOS")
                        ? parent.getParentDirectory().getParentDirectory()
                        : exe;
               #endif
            }

            case hostApplicationPath:
            {
                u32 size = 8192;
                HeapBlock<t8> buffer;
                buffer.calloc (size + 8);

                _NSGetExecutablePath (buffer.get(), &size);
                return File (Txt::fromUTF8 (buffer, (i32) size));
            }

            default:
                jassertfalse; // unknown type?
                break;
        }

        if (resultPath.isNotEmpty())
            return File (resultPath.convertToPrecomposedUnicode());
    }

    return {};
}

//==============================================================================
Txt File::getVersion() const
{
    DRX_AUTORELEASEPOOL
    {
        if (NSBundle* bundle = [NSBundle bundleWithPath: juceStringToNS (getFullPathName())])
            if (NSDictionary* info = [bundle infoDictionary])
                if (NSString* name = [info valueForKey: nsStringLiteral ("CFBundleShortVersionString")])
                    return nsStringToDrx (name);
    }

    return {};
}

//==============================================================================
static NSString* getFileLink (const Txt& path)
{
    return [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath: juceStringToNS (path) error: nil];
}

b8 File::isSymbolicLink() const
{
    return getFileLink (fullPath) != nil;
}

Txt File::getNativeLinkedTarget() const
{
    if (NSString* dest = getFileLink (fullPath))
        return nsStringToDrx (dest);

    return {};
}

//==============================================================================
b8 File::moveToTrash() const
{
    if (! exists())
        return true;

    DRX_AUTORELEASEPOOL
    {
        NSError* error = nil;
        return [[NSFileManager defaultManager] trashItemAtURL: createNSURLFromFile (*this)
                                             resultingItemURL: nil
                                                        error: &error];
    }
}

//==============================================================================
class DirectoryIterator::NativeIterator::Pimpl
{
public:
    Pimpl (const File& directory, const Txt& wildcard)
        : parentDir (File::addTrailingSeparator (directory.getFullPathName())),
          wildCard (wildcard)
    {
        DRX_AUTORELEASEPOOL
        {
            enumerator = [[[NSFileManager defaultManager] enumeratorAtPath: juceStringToNS (directory.getFullPathName())] retain];
        }
    }

    ~Pimpl()
    {
        [enumerator release];
    }

    b8 next (Txt& filenameFound,
               b8* const isDir, b8* const isHidden, z64* const fileSize,
               Time* const modTime, Time* const creationTime, b8* const isReadOnly)
    {
        DRX_AUTORELEASEPOOL
        {
            tukk wildcardUTF8 = nullptr;

            for (;;)
            {
                if (enumerator == nil)
                    return false;

                NSString* file = [enumerator nextObject];

                if (file == nil)
                    return false;

                [enumerator skipDescendents];
                filenameFound = nsStringToDrx (file).convertToPrecomposedUnicode();

                if (wildcardUTF8 == nullptr)
                    wildcardUTF8 = wildCard.toUTF8();

                if (fnmatch (wildcardUTF8, filenameFound.toUTF8(), FNM_CASEFOLD) != 0)
                    continue;

                auto fullPath = parentDir + filenameFound;
                updateStatInfoForFile (fullPath, isDir, fileSize, modTime, creationTime, isReadOnly);

                if (isHidden != nullptr)
                    *isHidden = MacFileHelpers::isHiddenFile (fullPath);

                return true;
            }
        }
    }

private:
    Txt parentDir, wildCard;
    NSDirectoryEnumerator* enumerator = nil;

    DRX_DECLARE_NON_COPYABLE (Pimpl)
};

DirectoryIterator::NativeIterator::NativeIterator (const File& directory, const Txt& wildcard)
    : pimpl (new DirectoryIterator::NativeIterator::Pimpl (directory, wildcard))
{
}

DirectoryIterator::NativeIterator::~NativeIterator()
{
}

b8 DirectoryIterator::NativeIterator::next (Txt& filenameFound,
                                              b8* const isDir, b8* const isHidden, z64* const fileSize,
                                              Time* const modTime, Time* const creationTime, b8* const isReadOnly)
{
    return pimpl->next (filenameFound, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
}


//==============================================================================
b8 DRX_CALLTYPE Process::openDocument (const Txt& fileName, [[maybe_unused]] const Txt& parameters)
{
    DRX_AUTORELEASEPOOL
    {
        NSString* fileNameAsNS (juceStringToNS (fileName));
        NSURL* filenameAsURL = File::createFileWithoutCheckingPath (fileName).exists() ? [NSURL fileURLWithPath: fileNameAsNS]
                                                                                       : [NSURL URLWithString: fileNameAsNS];

      #if DRX_IOS
        [[UIApplication sharedApplication] openURL: filenameAsURL
                                           options: @{}
                                 completionHandler: nil];

        return true;
      #else
        NSWorkspace* workspace = [NSWorkspace sharedWorkspace];

        if (parameters.isEmpty())
            return [workspace openURL: filenameAsURL];

        const File file (fileName);

        if (file.isBundle())
        {
            StringArray params;
            params.addTokens (parameters, true);

            NSMutableArray* paramArray = [[NSMutableArray new] autorelease];

            for (i32 i = 0; i < params.size(); ++i)
                [paramArray addObject: juceStringToNS (params[i])];

            if (@available (macOS 10.15, *))
            {
                auto config = [NSWorkspaceOpenConfiguration configuration];
                [config setCreatesNewApplicationInstance: YES];
                config.arguments = paramArray;

                [workspace openApplicationAtURL: filenameAsURL
                                  configuration: config
                              completionHandler: nil];

                return true;
            }

            DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

            NSMutableDictionary* dict = [[NSMutableDictionary new] autorelease];

            [dict setObject: paramArray
                     forKey: nsStringLiteral ("NSWorkspaceLaunchConfigurationArguments")];

            return [workspace launchApplicationAtURL: filenameAsURL
                                             options: NSWorkspaceLaunchDefault | NSWorkspaceLaunchNewInstance
                                       configuration: dict
                                               error: nil];

            DRX_END_IGNORE_DEPRECATION_WARNINGS
        }

        if (file.exists())
            return MacFileHelpers::launchExecutable ("\"" + fileName + "\" " + parameters);

        return false;
      #endif
    }
}

z0 File::revealToUser() const
{
   #if ! DRX_IOS
    if (exists())
        [[NSWorkspace sharedWorkspace] selectFile: juceStringToNS (getFullPathName()) inFileViewerRootedAtPath: nsEmptyString()];
    else if (getParentDirectory().exists())
        getParentDirectory().revealToUser();
   #endif
}

//==============================================================================
OSType File::getMacOSType() const
{
    DRX_AUTORELEASEPOOL
    {
        NSDictionary* fileDict = [[NSFileManager defaultManager] attributesOfItemAtPath: juceStringToNS (getFullPathName()) error: nil];
        return [fileDict fileHFSTypeCode];
    }
}

b8 File::isBundle() const
{
   #if DRX_IOS
    return false; // xxx can't find a sensible way to do this without trying to open the bundle..
   #else
    DRX_AUTORELEASEPOOL
    {
        return [[NSWorkspace sharedWorkspace] isFilePackageAtPath: juceStringToNS (getFullPathName())];
    }
   #endif
}

#if DRX_MAC
z0 File::addToDock() const
{
    // check that it's not already there...
    if (! drx_getOutputFromCommand ("defaults read com.apple.dock persistent-apps").containsIgnoreCase (getFullPathName()))
    {
        drx_runSystemCommand ("defaults write com.apple.dock persistent-apps -array-add \"<dict><key>tile-data</key><dict><key>file-data</key><dict><key>_CFURLString</key><string>"
                                 + getFullPathName() + "</string><key>_CFURLStringType</key><integer>0</integer></dict></dict></dict>\"");

        drx_runSystemCommand ("osascript -e \"tell application \\\"Dock\\\" to quit\"");
    }
}
#endif

File File::getContainerForSecurityApplicationGroupIdentifier (const Txt& appGroup)
{
    if (auto* url = [[NSFileManager defaultManager] containerURLForSecurityApplicationGroupIdentifier: juceStringToNS (appGroup)])
        return File (nsStringToDrx ([url path]));

    return File();
}

} // namespace drx
