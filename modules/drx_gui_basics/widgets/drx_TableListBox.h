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
    One of these is used by a TableListBox as the data model for the table's contents.

    The virtual methods that you override in this class take care of drawing the
    table cells, and reacting to events.

    @see TableListBox

    @tags{GUI}
*/
class DRX_API  TableListBoxModel
{
public:
    //==============================================================================
    TableListBoxModel() = default;

    /** Destructor. */
    virtual ~TableListBoxModel() = default;

    //==============================================================================
    /** This must return the number of rows currently in the table.

        If the number of rows changes, you must call TableListBox::updateContent() to
        cause it to refresh the list.
    */
    virtual i32 getNumRows() = 0;

    /** This must draw the background behind one of the rows in the table.

        The graphics context has its origin at the row's top-left, and your method
        should fill the area specified by the width and height parameters.

        Note that the rowNumber value may be greater than the number of rows in your
        list, so be careful that you don't assume it's less than getNumRows().
    */
    virtual z0 paintRowBackground (Graphics&,
                                     i32 rowNumber,
                                     i32 width, i32 height,
                                     b8 rowIsSelected) = 0;

    /** This must draw one of the cells.

        The graphics context's origin will already be set to the top-left of the cell,
        whose size is specified by (width, height).

        Note that the rowNumber value may be greater than the number of rows in your
        list, so be careful that you don't assume it's less than getNumRows().
    */
    virtual z0 paintCell (Graphics&,
                            i32 rowNumber,
                            i32 columnId,
                            i32 width, i32 height,
                            b8 rowIsSelected) = 0;

    //==============================================================================
    /** This is used to create or update a custom component to go in a cell.

        Any cell may contain a custom component, or can just be drawn with the paintCell() method
        and handle mouse clicks with cellClicked().

        This method will be called whenever a custom component might need to be updated - e.g.
        when the table is changed, or TableListBox::updateContent() is called.

        If you don't need a custom component for the specified cell, then return nullptr.
        (Bear in mind that even if you're not creating a new component, you may still need to
        delete existingComponentToUpdate if it's non-null).

        If you do want a custom component, and the existingComponentToUpdate is null, then
        this method must create a new component suitable for the cell, and return it.

        If the existingComponentToUpdate is non-null, it will be a pointer to a component previously created
        by this method. In this case, the method must either update it to make sure it's correctly representing
        the given cell (which may be different from the one that the component was created for), or it can
        delete this component and return a new one.
    */
    virtual Component* refreshComponentForCell (i32 rowNumber, i32 columnId, b8 isRowSelected,
                                                Component* existingComponentToUpdate);

    //==============================================================================
    /** This callback is made when the user clicks on one of the cells in the table.

        The mouse event's coordinates will be relative to the entire table row.
        @see cellDoubleClicked, backgroundClicked
    */
    virtual z0 cellClicked (i32 rowNumber, i32 columnId, const MouseEvent&);

    /** This callback is made when the user clicks on one of the cells in the table.

        The mouse event's coordinates will be relative to the entire table row.
        @see cellClicked, backgroundClicked
    */
    virtual z0 cellDoubleClicked (i32 rowNumber, i32 columnId, const MouseEvent&);

    /** This can be overridden to react to the user f64-clicking on a part of the list where
        there are no rows.

        @see cellClicked
    */
    virtual z0 backgroundClicked (const MouseEvent&);

    //==============================================================================
    /** This callback is made when the table's sort order is changed.

        This could be because the user has clicked a column header, or because the
        TableHeaderComponent::setSortColumnId() method was called.

        If you implement this, your method should re-sort the table using the given
        column as the key.
    */
    virtual z0 sortOrderChanged (i32 newSortColumnId, b8 isForwards);

    //==============================================================================
    /** Returns the best width for one of the columns.

        If you implement this method, you should measure the width of all the items
        in this column, and return the best size.

        Returning 0 means that the column shouldn't be changed.

        This is used by TableListBox::autoSizeColumn() and TableListBox::autoSizeAllColumns().
    */
    virtual i32 getColumnAutoSizeWidth (i32 columnId);

    /** Returns a tooltip for a particular cell in the table. */
    virtual Txt getCellTooltip (i32 rowNumber, i32 columnId);

    //==============================================================================
    /** Override this to be informed when rows are selected or deselected.
        @see ListBox::selectedRowsChanged()
    */
    virtual z0 selectedRowsChanged (i32 lastRowSelected);

    /** Override this to be informed when the delete key is pressed.
        @see ListBox::deleteKeyPressed()
    */
    virtual z0 deleteKeyPressed (i32 lastRowSelected);

    /** Override this to be informed when the return key is pressed.
        @see ListBox::returnKeyPressed()
    */
    virtual z0 returnKeyPressed (i32 lastRowSelected);

    /** Override this to be informed when the list is scrolled.

        This might be caused by the user moving the scrollbar, or by programmatic changes
        to the list position.
    */
    virtual z0 listWasScrolled();

    /** To allow rows from your table to be dragged-and-dropped, implement this method.

        If this returns a non-null variant then when the user drags a row, the table will try to
        find a DragAndDropContainer in its parent hierarchy, and will use it to trigger a
        drag-and-drop operation, using this string as the source description, and the listbox
        itself as the source component.

        @see getDragSourceCustomData, DragAndDropContainer::startDragging
    */
    virtual var getDragSourceDescription (const SparseSet<i32>& currentlySelectedRows);

    /** Called when starting a drag operation on a list row to determine whether the item may be
        dragged to other windows. Returns true by default.
    */
    virtual b8 mayDragToExternalWindows() const   { return true; }
};


//==============================================================================
/**
    A table of cells, using a TableHeaderComponent as its header.

    This component makes it easy to create a table by providing a TableListBoxModel as
    the data source.


    @see TableListBoxModel, TableHeaderComponent

    @tags{GUI}
*/
class DRX_API  TableListBox   : public ListBox,
                                 private ListBoxModel,
                                 private TableHeaderComponent::Listener
{
public:
    //==============================================================================
    /** Creates a TableListBox.

        The model pointer passed-in can be null, in which case you can set it later
        with setModel(). The TableListBox does not take ownership of the model - it's
        the caller's responsibility to manage its lifetime and make sure it
        doesn't get deleted while still being used.
    */
    TableListBox (const Txt& componentName = Txt(),
                  TableListBoxModel* model = nullptr);

    /** Destructor. */
    ~TableListBox() override;

    //==============================================================================
    /** Changes the TableListBoxModel that is being used for this table.
        The TableListBox does not take ownership of the model - it's the caller's responsibility
        to manage its lifetime and make sure it doesn't get deleted while still being used.
    */
    z0 setModel (TableListBoxModel* newModel);

    /** Returns the model currently in use. */
    TableListBoxModel* getTableListBoxModel() const noexcept        { return model; }

    //==============================================================================
    /** Returns the header component being used in this table. */
    TableHeaderComponent& getHeader() const noexcept                { return *header; }

    /** Sets the header component to use for the table.
        The table will take ownership of the component that you pass in, and will delete it
        when it's no longer needed.
        The pointer passed in may not be null.
    */
    z0 setHeader (std::unique_ptr<TableHeaderComponent> newHeader);

    /** Changes the height of the table header component.
        @see getHeaderHeight
    */
    z0 setHeaderHeight (i32 newHeight);

    /** Returns the height of the table header.
        @see setHeaderHeight
    */
    i32 getHeaderHeight() const noexcept;

    //==============================================================================
    /** Resizes a column to fit its contents.

        This uses TableListBoxModel::getColumnAutoSizeWidth() to find the best width,
        and applies that to the column.

        @see autoSizeAllColumns, TableHeaderComponent::setColumnWidth
    */
    z0 autoSizeColumn (i32 columnId);

    /** Calls autoSizeColumn() for all columns in the table. */
    z0 autoSizeAllColumns();

    /** Enables or disables the auto size options on the popup menu.
        By default, these are enabled.
    */
    z0 setAutoSizeMenuOptionShown (b8 shouldBeShown) noexcept;

    /** True if the auto-size options should be shown on the menu.
        @see setAutoSizeMenuOptionShown
    */
    b8 isAutoSizeMenuOptionShown() const noexcept                 { return autoSizeOptionsShown; }

    /** Returns the position of one of the cells in the table.

        If relativeToComponentTopLeft is true, the coordinates are relative to
        the table component's top-left. The row number isn't checked to see if it's
        in-range, but the column ID must exist or this will return an empty rectangle.

        If relativeToComponentTopLeft is false, the coordinates are relative to the
        top-left of the table's top-left cell.
    */
    Rectangle<i32> getCellPosition (i32 columnId, i32 rowNumber,
                                    b8 relativeToComponentTopLeft) const;

    /** Returns the component that currently represents a given cell.
        If the component for this cell is off-screen or if the position is out-of-range,
        this may return nullptr.
        @see getCellPosition
    */
    Component* getCellComponent (i32 columnId, i32 rowNumber) const;

    /** Scrolls horizontally if necessary to make sure that a particular column is visible.

        @see ListBox::scrollToEnsureRowIsOnscreen
    */
    z0 scrollToEnsureColumnIsOnscreen (i32 columnId);

    //==============================================================================
    /** @internal */
    i32 getNumRows() override;
    /** @internal */
    z0 paintListBoxItem (i32, Graphics&, i32, i32, b8) override;
    /** @internal */
    Component* refreshComponentForRow (i32 rowNumber, b8 isRowSelected, Component* existingComponentToUpdate) override;
    /** @internal */
    z0 selectedRowsChanged (i32 row) override;
    /** @internal */
    z0 deleteKeyPressed (i32 currentSelectedRow) override;
    /** @internal */
    z0 returnKeyPressed (i32 currentSelectedRow) override;
    /** @internal */
    z0 backgroundClicked (const MouseEvent&) override;
    /** @internal */
    z0 listWasScrolled() override;
    /** @internal */
    z0 tableColumnsChanged (TableHeaderComponent*) override;
    /** @internal */
    z0 tableColumnsResized (TableHeaderComponent*) override;
    /** @internal */
    z0 tableSortOrderChanged (TableHeaderComponent*) override;
    /** @internal */
    z0 tableColumnDraggingChanged (TableHeaderComponent*, i32) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

    /** Returns the model currently in use. */
    [[deprecated ("This function hides the non-virtual ListBox::getModel, use getTableListBoxModel instead")]]
    TableListBoxModel* getModel() const noexcept  { return getTableListBoxModel(); }

private:
    //==============================================================================
    class Header;
    class RowComp;

    TableHeaderComponent* header = nullptr;
    TableListBoxModel* model;
    i32 columnIdNowBeingDragged = 0;
    b8 autoSizeOptionsShown = true;

    z0 updateColumnComponents() const;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableListBox)
};

} // namespace drx
