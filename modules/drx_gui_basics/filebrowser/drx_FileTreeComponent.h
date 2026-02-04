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
    A component that displays the files in a directory as a treeview.

    This implements the DirectoryContentsDisplayComponent base class so that
    it can be used in a FileBrowserComponent.

    To attach a listener to it, use its DirectoryContentsDisplayComponent base
    class and the FileBrowserListener class.

    @see DirectoryContentsList, FileListComponent

    @tags{GUI}
*/
class DRX_API  FileTreeComponent  : public TreeView,
                                     public DirectoryContentsDisplayComponent
{
public:
    //==============================================================================
    /** Creates a listbox to show the contents of a specified directory.
    */
    FileTreeComponent (DirectoryContentsList& listToShow);

    /** Destructor. */
    ~FileTreeComponent() override;

    //==============================================================================
    /** Returns the number of files the user has got selected.
        @see getSelectedFile
    */
    i32 getNumSelectedFiles() const override               { return TreeView::getNumSelectedItems(); }

    /** Returns one of the files that the user has currently selected.
        The index should be in the range 0 to (getNumSelectedFiles() - 1).
        @see getNumSelectedFiles
    */
    File getSelectedFile (i32 index = 0) const override;

    /** Deselects any files that are currently selected. */
    z0 deselectAllFiles() override;

    /** Scrolls the list to the top. */
    z0 scrollToTop() override;

    /** If the specified file is in the list, it will become the only selected item
        (and if the file isn't in the list, all other items will be deselected). */
    z0 setSelectedFile (const File&) override;

    /** Updates the files in the list. */
    z0 refresh();

    /** Setting a name for this allows tree items to be dragged.

        The string that you pass in here will be returned by the getDragSourceDescription()
        of the items in the tree. For more info, see TreeViewItem::getDragSourceDescription().
    */
    z0 setDragAndDropDescription (const Txt& description);

    /** Returns the last value that was set by setDragAndDropDescription().
    */
    const Txt& getDragAndDropDescription() const noexcept    { return dragAndDropDescription; }

    /** Changes the height of the treeview items. */
    z0 setItemHeight (i32 newHeight);

    /** Returns the height of the treeview items. */
    i32 getItemHeight() const noexcept                          { return itemHeight; }

private:
    //==============================================================================
    Txt dragAndDropDescription;
    i32 itemHeight;

    class Controller;
    std::unique_ptr<Controller> controller;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileTreeComponent)
};

} // namespace drx
