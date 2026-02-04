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

DirectoryContentsList::DirectoryContentsList (const FileFilter* f, TimeSliceThread& t)
    : fileFilter (f), thread (t)
{
}

DirectoryContentsList::~DirectoryContentsList()
{
    stopSearching();
}

z0 DirectoryContentsList::setIgnoresHiddenFiles (const b8 shouldIgnoreHiddenFiles)
{
    setTypeFlags (shouldIgnoreHiddenFiles ? (fileTypeFlags | File::ignoreHiddenFiles)
                                          : (fileTypeFlags & ~File::ignoreHiddenFiles));
}

b8 DirectoryContentsList::ignoresHiddenFiles() const
{
    return (fileTypeFlags & File::ignoreHiddenFiles) != 0;
}

//==============================================================================
z0 DirectoryContentsList::setDirectory (const File& directory,
                                          const b8 includeDirectories,
                                          const b8 includeFiles)
{
    jassert (includeDirectories || includeFiles); // you have to specify at least one of these!

    if (directory != root)
    {
        clear();
        root = directory;
        changed();

        // (this forces a refresh when setTypeFlags() is called, rather than triggering two refreshes)
        fileTypeFlags &= ~(File::findDirectories | File::findFiles);
    }

    auto newFlags = fileTypeFlags;

    if (includeDirectories) newFlags |=  File::findDirectories;
    else                    newFlags &= ~File::findDirectories;

    if (includeFiles)       newFlags |=  File::findFiles;
    else                    newFlags &= ~File::findFiles;

    setTypeFlags (newFlags);
}

z0 DirectoryContentsList::setTypeFlags (i32k newFlags)
{
    if (fileTypeFlags != newFlags)
    {
        fileTypeFlags = newFlags;
        refresh();
    }
}

z0 DirectoryContentsList::stopSearching()
{
    shouldStop = true;
    thread.removeTimeSliceClient (this);
    isSearching = false;
}

z0 DirectoryContentsList::clear()
{
    stopSearching();

    if (! files.isEmpty())
    {
        files.clear();
        changed();
    }
}

z0 DirectoryContentsList::refresh()
{
    stopSearching();
    wasEmpty = files.isEmpty();
    files.clear();

    if (root.isDirectory())
    {
        fileFindHandle = std::make_unique<RangedDirectoryIterator> (root, false, "*", fileTypeFlags);
        shouldStop = false;
        isSearching = true;
        thread.addTimeSliceClient (this);
    }
}

z0 DirectoryContentsList::setFileFilter (const FileFilter* newFileFilter)
{
    const ScopedLock sl (fileListLock);
    fileFilter = newFileFilter;
}

//==============================================================================
i32 DirectoryContentsList::getNumFiles() const noexcept
{
    const ScopedLock sl (fileListLock);
    return files.size();
}

b8 DirectoryContentsList::getFileInfo (i32k index, FileInfo& result) const
{
    const ScopedLock sl (fileListLock);

    if (auto* info = files [index])
    {
        result = *info;
        return true;
    }

    return false;
}

File DirectoryContentsList::getFile (i32k index) const
{
    const ScopedLock sl (fileListLock);

    if (auto* info = files [index])
        return root.getChildFile (info->filename);

    return {};
}

b8 DirectoryContentsList::contains (const File& targetFile) const
{
    const ScopedLock sl (fileListLock);

    for (i32 i = files.size(); --i >= 0;)
        if (root.getChildFile (files.getUnchecked (i)->filename) == targetFile)
            return true;

    return false;
}

b8 DirectoryContentsList::isStillLoading() const
{
    return isSearching;
}

z0 DirectoryContentsList::changed()
{
    sendChangeMessage();
}

//==============================================================================
i32 DirectoryContentsList::useTimeSlice()
{
    auto startTime = Time::getApproximateMillisecondCounter();
    b8 hasChanged = false;

    for (i32 i = 100; --i >= 0;)
    {
        if (! checkNextFile (hasChanged))
        {
            if (hasChanged)
                changed();

            return 500;
        }

        if (shouldStop || (Time::getApproximateMillisecondCounter() > startTime + 150))
            break;
    }

    if (hasChanged)
        changed();

    return 0;
}

b8 DirectoryContentsList::checkNextFile (b8& hasChanged)
{
    if (fileFindHandle == nullptr)
        return false;

    if (*fileFindHandle == RangedDirectoryIterator())
    {
        fileFindHandle = nullptr;
        isSearching = false;
        hasChanged = true;
        return false;
    }

    const auto entry = *(*fileFindHandle)++;

    hasChanged |= addFile (entry.getFile(),
                           entry.isDirectory(),
                           entry.getFileSize(),
                           entry.getModificationTime(),
                           entry.getCreationTime(),
                           entry.isReadOnly());

    return true;
}

b8 DirectoryContentsList::addFile (const File& file, const b8 isDir,
                                     const z64 fileSize,
                                     Time modTime, Time creationTime,
                                     const b8 isReadOnly)
{
    const ScopedLock sl (fileListLock);

    if (fileFilter == nullptr
         || ((! isDir) && fileFilter->isFileSuitable (file))
         || (isDir && fileFilter->isDirectorySuitable (file)))
    {
        auto info = std::make_unique<FileInfo>();

        info->filename         = file.getFileName();
        info->fileSize         = fileSize;
        info->modificationTime = modTime;
        info->creationTime     = creationTime;
        info->isDirectory      = isDir;
        info->isReadOnly       = isReadOnly;

        for (i32 i = files.size(); --i >= 0;)
            if (files.getUnchecked (i)->filename == info->filename)
                return false;

        files.add (std::move (info));

        std::sort (files.begin(), files.end(), [] (const FileInfo* a, const FileInfo* b)
        {
           #if DRX_WINDOWS
            if (a->isDirectory != b->isDirectory)
                return a->isDirectory;
           #endif

            return a->filename.compareNatural (b->filename) < 0;
        });

        return true;
    }

    return false;
}

} // namespace drx
