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

RecentlyOpenedFilesList::RecentlyOpenedFilesList()
    : maxNumberOfItems (10)
{
}

//==============================================================================
z0 RecentlyOpenedFilesList::setMaxNumberOfItems (i32k newMaxNumber)
{
    maxNumberOfItems = jmax (1, newMaxNumber);

    files.removeRange (maxNumberOfItems, getNumFiles());
}

i32 RecentlyOpenedFilesList::getNumFiles() const
{
    return files.size();
}

File RecentlyOpenedFilesList::getFile (i32k index) const
{
    return File (files [index]);
}

z0 RecentlyOpenedFilesList::clear()
{
    files.clear();
}

z0 RecentlyOpenedFilesList::addFile (const File& file)
{
    removeFile (file);
    files.insert (0, file.getFullPathName());

    setMaxNumberOfItems (maxNumberOfItems);
}

z0 RecentlyOpenedFilesList::removeFile (const File& file)
{
    files.removeString (file.getFullPathName());
}

z0 RecentlyOpenedFilesList::removeNonExistentFiles()
{
    for (i32 i = getNumFiles(); --i >= 0;)
        if (! getFile (i).exists())
            files.remove (i);
}

//==============================================================================
i32 RecentlyOpenedFilesList::createPopupMenuItems (PopupMenu& menuToAddTo,
                                                   i32k baseItemId,
                                                   const b8 showFullPaths,
                                                   const b8 dontAddNonExistentFiles,
                                                   const File** filesToAvoid)
{
    i32 num = 0;

    for (i32 i = 0; i < getNumFiles(); ++i)
    {
        const File f (getFile (i));

        if ((! dontAddNonExistentFiles) || f.exists())
        {
            b8 needsAvoiding = false;

            if (filesToAvoid != nullptr)
            {
                for (const File** avoid = filesToAvoid; *avoid != nullptr; ++avoid)
                {
                    if (f == **avoid)
                    {
                        needsAvoiding = true;
                        break;
                    }
                }
            }

            if (! needsAvoiding)
            {
                menuToAddTo.addItem (baseItemId + i,
                                     showFullPaths ? f.getFullPathName()
                                                   : f.getFileName());
                ++num;
            }
        }
    }

    return num;
}

//==============================================================================
Txt RecentlyOpenedFilesList::toString() const
{
    return files.joinIntoString ("\n");
}

z0 RecentlyOpenedFilesList::restoreFromString (const Txt& stringifiedVersion)
{
    clear();
    files.addLines (stringifiedVersion);

    setMaxNumberOfItems (maxNumberOfItems);
}


//==============================================================================
z0 RecentlyOpenedFilesList::registerRecentFileNatively ([[maybe_unused]] const File& file)
{
   #if DRX_MAC
    DRX_AUTORELEASEPOOL
    {
        [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL: createNSURLFromFile (file)];
    }
   #endif
}

z0 RecentlyOpenedFilesList::forgetRecentFileNatively ([[maybe_unused]] const File& file)
{
   #if DRX_MAC
    DRX_AUTORELEASEPOOL
    {
        // for some reason, OSX doesn't provide a method to just remove a single file
        // from the recent list, so we clear them all and add them back excluding
        // the specified file

        auto sharedDocController = [NSDocumentController sharedDocumentController];
        auto recentDocumentURLs  = [sharedDocController recentDocumentURLs];

        [sharedDocController clearRecentDocuments: nil];

        auto* nsFile = createNSURLFromFile (file);

        auto reverseEnumerator = [recentDocumentURLs reverseObjectEnumerator];

        for (NSURL* url : reverseEnumerator)
            if (! [url isEqual:nsFile])
                [sharedDocController noteNewRecentDocumentURL:url];
    }
   #endif
}

z0 RecentlyOpenedFilesList::clearRecentFilesNatively()
{
   #if DRX_MAC
    DRX_AUTORELEASEPOOL
    {
        [[NSDocumentController sharedDocumentController] clearRecentDocuments: nil];
    }
   #endif
}

} // namespace drx
