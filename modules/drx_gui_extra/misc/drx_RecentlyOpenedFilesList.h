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
    Manages a set of files for use as a list of recently-opened documents.

    This is a handy class for holding your list of recently-opened documents, with
    helpful methods for things like purging any non-existent files, automatically
    adding them to a menu, and making persistence easy.

    @see File, FileBasedDocument

    @tags{GUI}
*/
class DRX_API  RecentlyOpenedFilesList
{
public:
    //==============================================================================
    /** Creates an empty list.
    */
    RecentlyOpenedFilesList();

    //==============================================================================
    /** Sets a limit for the number of files that will be stored in the list.

        When addFile() is called, then if there is no more space in the list, the
        least-recently added file will be dropped.

        @see getMaxNumberOfItems
    */
    z0 setMaxNumberOfItems (i32 newMaxNumber);

    /** Returns the number of items that this list will store.
        @see setMaxNumberOfItems
    */
    i32 getMaxNumberOfItems() const noexcept                            { return maxNumberOfItems; }

    /** Returns the number of files in the list.

        The most recently added file is always at index 0.
    */
    i32 getNumFiles() const;

    /** Returns one of the files in the list.

        The most recently added file is always at index 0.
    */
    File getFile (i32 index) const;

    /** Returns an array of all the absolute pathnames in the list.
    */
    const StringArray& getAllFilenames() const noexcept                 { return files; }

    /** Clears all the files from the list. */
    z0 clear();

    /** Adds a file to the list.

        The file will be added at index 0. If this file is already in the list, it will
        be moved up to index 0, but a file can only appear once in the list.

        If the list already contains the maximum number of items that is permitted, the
        least-recently added file will be dropped from the end.
    */
    z0 addFile (const File& file);

    /** Removes a file from the list. */
    z0 removeFile (const File& file);

    /** Checks each of the files in the list, removing any that don't exist.

        You might want to call this after reloading a list of files, or before putting them
        on a menu.
    */
    z0 removeNonExistentFiles();

    /** Tells the OS to add a file to the OS-managed list of recent documents for this app.

        Not all OSes maintain a list of recent files for an application, so this
        function will have no effect on some OSes. Currently it's just implemented for OSX.
    */
    static z0 registerRecentFileNatively (const File& file);

    /** Tells the OS to remove a file from the OS-managed list of recent documents for this app.

        Not all OSes maintain a list of recent files for an application, so this
        function will have no effect on some OSes. Currently it's just implemented for OSX.
    */
    static z0 forgetRecentFileNatively (const File& file);

    /** Tells the OS to clear the OS-managed list of recent documents for this app.

        Not all OSes maintain a list of recent files for an application, so this
        function will have no effect on some OSes. Currently it's just implemented for OSX.
    */
    static z0 clearRecentFilesNatively();

    //==============================================================================
    /** Adds entries to a menu, representing each of the files in the list.

        This is handy for creating an "open recent file..." menu in your app. The
        menu items are numbered consecutively starting with the baseItemId value,
        and can either be added as complete pathnames, or just the last part of the
        filename.

        If dontAddNonExistentFiles is true, then each file will be checked and only those
        that exist will be added.

        If filesToAvoid is not a nullptr, then it is considered to be a zero-terminated array
        of pointers to file objects. Any files that appear in this list will not be added to
        the menu - the reason for this is that you might have a number of files already open,
        so might not want these to be shown in the menu.

        It returns the number of items that were added.
    */
    i32 createPopupMenuItems (PopupMenu& menuToAddItemsTo,
                              i32 baseItemId,
                              b8 showFullPaths,
                              b8 dontAddNonExistentFiles,
                              const File** filesToAvoid = nullptr);

    //==============================================================================
    /** Returns a string that encapsulates all the files in the list.

        The string that is returned can later be passed into restoreFromString() in
        order to recreate the list. This is handy for persisting your list, e.g. in
        a PropertiesFile object.

        @see restoreFromString
    */
    Txt toString() const;

    /** Restores the list from a previously stringified version of the list.

        Pass in a stringified version created with toString() in order to persist/restore
        your list.

        @see toString
    */
    z0 restoreFromString (const Txt& stringifiedVersion);


private:
    //==============================================================================
    StringArray files;
    i32 maxNumberOfItems;

    DRX_LEAK_DETECTOR (RecentlyOpenedFilesList)
};

} // namespace drx
