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
    A component that displays the files in a directory as a listbox.

    This implements the DirectoryContentsDisplayComponent base class so that
    it can be used in a FileBrowserComponent.

    To attach a listener to it, use its DirectoryContentsDisplayComponent base
    class and the FileBrowserListener class.

    @see DirectoryContentsList, FileTreeComponent

    @tags{GUI}
*/
class DRX_API  FileListComponent  : private ListBoxModel,
                                     public ListBox,
                                     public DirectoryContentsDisplayComponent,
                                     private ChangeListener
{
public:
    //==============================================================================
    /** Creates a listbox to show the contents of a specified directory. */
    FileListComponent (DirectoryContentsList& listToShow);

    /** Destructor. */
    ~FileListComponent() override;

    //==============================================================================
    /** Returns the number of files the user has got selected.
        @see getSelectedFile
    */
    i32 getNumSelectedFiles() const override;

    /** Returns one of the files that the user has currently selected.
        The index should be in the range 0 to (getNumSelectedFiles() - 1).
        @see getNumSelectedFiles
    */
    File getSelectedFile (i32 index = 0) const override;

    /** Deselects any files that are currently selected. */
    z0 deselectAllFiles() override;

    /** Scrolls to the top of the list. */
    z0 scrollToTop() override;

    /** If the specified file is in the list, it will become the only selected item
        (and if the file isn't in the list, all other items will be deselected). */
    z0 setSelectedFile (const File&) override;

private:
    //==============================================================================
    File lastDirectory, fileWaitingToBeSelected;
    class ItemComponent;

    z0 changeListenerCallback (ChangeBroadcaster*) override;
    i32 getNumRows() override;
    Txt getNameForRow (i32 rowNumber) override;
    z0 paintListBoxItem (i32, Graphics&, i32, i32, b8) override;
    Component* refreshComponentForRow (i32 rowNumber, b8 isRowSelected, Component*) override;
    z0 selectedRowsChanged (i32 row) override;
    z0 deleteKeyPressed (i32 currentSelectedRow) override;
    z0 returnKeyPressed (i32 currentSelectedRow) override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListComponent)
};

} // namespace drx
