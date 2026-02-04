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

//==============================================================================
/**
    A class to asynchronously scan for details about the files in a directory.

    This keeps a list of files and some information about them, using a background
    thread to scan for more files. As files are found, it broadcasts change messages
    to tell any listeners.

    @see FileListComponent, FileBrowserComponent

    @tags{GUI}
*/
class DRX_API  DirectoryContentsList   : public ChangeBroadcaster,
                                          private TimeSliceClient
{
public:
    //==============================================================================
    /** Creates a directory list.

        To set the directory it should point to, use setDirectory(), which will
        also start it scanning for files on the background thread.

        When the background thread finds and adds new files to this list, the
        ChangeBroadcaster class will send a change message, so you can register
        listeners and update them when the list changes.

        @param fileFilter       an optional filter to select which files are
                                included in the list. If this is nullptr, then all files
                                and directories are included. Make sure that the filter
                                doesn't get deleted during the lifetime of this object
        @param threadToUse      a thread object that this list can use
                                to scan for files as a background task. Make sure
                                that the thread you give it has been started, or you
                                won't get any files!
    */
    DirectoryContentsList (const FileFilter* fileFilter,
                           TimeSliceThread& threadToUse);

    /** Destructor. */
    ~DirectoryContentsList() override;


    //==============================================================================
    /** Returns the directory that's currently being used. */
    const File& getDirectory() const noexcept               { return root; }

    /** Sets the directory to look in for files.

        If the directory that's passed in is different to the current one, this will
        also start the background thread scanning it for files.
    */
    z0 setDirectory (const File& directory,
                       b8 includeDirectories,
                       b8 includeFiles);

    /** Возвращает true, если this list contains directories.
        @see setDirectory
    */
    b8 isFindingDirectories() const noexcept              { return (fileTypeFlags & File::findDirectories) != 0; }

    /** Возвращает true, если this list contains files.
        @see setDirectory
    */
    b8 isFindingFiles() const noexcept                    { return (fileTypeFlags & File::findFiles) != 0; }

    /** Clears the list, and stops the thread scanning for files. */
    z0 clear();

    /** Clears the list and restarts scanning the directory for files. */
    z0 refresh();

    /** True if the background thread hasn't yet finished scanning for files. */
    b8 isStillLoading() const;

    /** Tells the list whether or not to ignore hidden files.
        By default these are ignored.
    */
    z0 setIgnoresHiddenFiles (b8 shouldIgnoreHiddenFiles);

    /** Возвращает true, если hidden files are ignored.
        @see setIgnoresHiddenFiles
    */
    b8 ignoresHiddenFiles() const;

    /** Replaces the current FileFilter.
        This can be nullptr to have no filter. The DirectoryContentList does not take
        ownership of this object - it just keeps a pointer to it, so you must manage its
        lifetime.
        Note that this only replaces the filter, it doesn't refresh the list - you'll
        probably want to call refresh() after calling this.
    */
    z0 setFileFilter (const FileFilter* newFileFilter);

    //==============================================================================
    /** Contains cached information about one of the files in a DirectoryContentsList.
    */
    struct FileInfo
    {
        //==============================================================================
        /** The filename.

            This isn't a full pathname, it's just the last part of the path, same as you'd
            get from File::getFileName().

            To get the full pathname, use DirectoryContentsList::getDirectory().getChildFile (filename).
        */
        Txt filename;

        /** File size in bytes. */
        z64 fileSize;

        /** File modification time.
            As supplied by File::getLastModificationTime().
        */
        Time modificationTime;

        /** File creation time.
            As supplied by File::getCreationTime().
        */
        Time creationTime;

        /** True if the file is a directory. */
        b8 isDirectory;

        /** True if the file is read-only. */
        b8 isReadOnly;
    };

    //==============================================================================
    /** Returns the number of files currently available in the list.

        The info about one of these files can be retrieved with getFileInfo() or getFile().

        Obviously as the background thread runs and scans the directory for files, this
        number will change.

        @see getFileInfo, getFile
    */
    i32 getNumFiles() const noexcept;

    /** Returns the cached information about one of the files in the list.

        If the index is in-range, this will return true and will copy the file's details
        to the structure that is passed-in.

        If it returns false, then the index wasn't in range, and the structure won't
        be affected.

        @see getNumFiles, getFile
    */
    b8 getFileInfo (i32 index, FileInfo& resultInfo) const;

    /** Returns one of the files in the list.

        @param index    should be less than getNumFiles(). If this is out-of-range, the
                        return value will be a default File() object
        @see getNumFiles, getFileInfo
    */
    File getFile (i32 index) const;

    /** Returns the file filter being used.
        The filter is specified in the constructor.
    */
    const FileFilter* getFilter() const noexcept            { return fileFilter; }

    /** Возвращает true, если the list contains the specified file. */
    b8 contains (const File&) const;

    //==============================================================================
    /** @internal */
    TimeSliceThread& getTimeSliceThread() const noexcept    { return thread; }

private:
    File root;
    const FileFilter* fileFilter = nullptr;
    TimeSliceThread& thread;
    i32 fileTypeFlags = File::ignoreHiddenFiles | File::findFiles;

    CriticalSection fileListLock;
    OwnedArray<FileInfo> files;

    std::unique_ptr<RangedDirectoryIterator> fileFindHandle;
    std::atomic<b8> shouldStop { true }, isSearching { false };

    b8 wasEmpty = true;

    i32 useTimeSlice() override;
    z0 stopSearching();
    z0 changed();
    b8 checkNextFile (b8& hasChanged);
    b8 addFile (const File&, b8 isDir, z64 fileSize, Time modTime,
                  Time creationTime, b8 isReadOnly);
    z0 setTypeFlags (i32);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectoryContentsList)
};

} // namespace drx
