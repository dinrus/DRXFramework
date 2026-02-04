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
    A component that displays a strip of column headings for a table, and allows these
    to be resized, dragged around, etc.

    This is just the component that goes at the top of a table. You can use it
    directly for custom components, or to create a simple table, use the
    TableListBox class.

    To use one of these, create it and use addColumn() to add all the columns that you need.
    Each column must be given a unique ID number that's used to refer to it.

    @see TableListBox, TableHeaderComponent::Listener

    @tags{GUI}
*/
class DRX_API  TableHeaderComponent   : public Component,
                                         private AsyncUpdater
{
public:
    //==============================================================================
    /** Creates an empty table header.
    */
    TableHeaderComponent();

    /** Destructor. */
    ~TableHeaderComponent() override;

    //==============================================================================
    /** A combination of these flags are passed into the addColumn() method to specify
        the properties of a column.
    */
    enum ColumnPropertyFlags
    {
        visible                     = 1,    /**< If this is set, the column will be shown; if not, it will be hidden until the user enables it with the pop-up menu. */
        resizable                   = 2,    /**< If this is set, the column can be resized by dragging it. */
        draggable                   = 4,    /**< If this is set, the column can be dragged around to change its order in the table. */
        appearsOnColumnMenu         = 8,    /**< If this is set, the column will be shown on the pop-up menu allowing it to be hidden/shown. */
        sortable                    = 16,   /**< If this is set, then clicking on the column header will set it to be the sort column, and clicking again will reverse the order. */
        sortedForwards              = 32,   /**< If this is set, the column is currently the one by which the table is sorted (forwards). */
        sortedBackwards             = 64,   /**< If this is set, the column is currently the one by which the table is sorted (backwards). */

        /** This set of default flags is used as the default parameter value in addColumn(). */
        defaultFlags                = (visible | resizable | draggable | appearsOnColumnMenu | sortable),

        /** A quick way of combining flags for a column that's not resizable. */
        notResizable                = (visible | draggable | appearsOnColumnMenu | sortable),

        /** A quick way of combining flags for a column that's not resizable or sortable. */
        notResizableOrSortable      = (visible | draggable | appearsOnColumnMenu),

        /** A quick way of combining flags for a column that's not sortable. */
        notSortable                 = (visible | resizable | draggable | appearsOnColumnMenu)
    };

    /** Adds a column to the table.

        This will add a column, and asynchronously call the tableColumnsChanged() method of any
        registered listeners.

        @param columnName       the name of the new column. It's ok to have two or more columns with the same name
        @param columnId         an ID for this column. The ID can be any number apart from 0, but every column must have
                                a unique ID. This is used to identify the column later on, after the user may have
                                changed the order that they appear in
        @param width            the initial width of the column, in pixels
        @param maximumWidth     a maximum width that the column can take when the user is resizing it. This only applies
                                if the 'resizable' flag is specified for this column
        @param minimumWidth     a minimum width that the column can take when the user is resizing it. This only applies
                                if the 'resizable' flag is specified for this column
        @param propertyFlags    a combination of some of the values from the ColumnPropertyFlags enum, to define the
                                properties of this column
        @param insertIndex      the index at which the column should be added. A value of 0 puts it at the start (left-hand side)
                                and -1 puts it at the end (right-hand size) of the table. Note that the index the index within
                                all columns, not just the index amongst those that are currently visible
    */
    z0 addColumn (const Txt& columnName,
                    i32 columnId,
                    i32 width,
                    i32 minimumWidth = 30,
                    i32 maximumWidth = -1,
                    i32 propertyFlags = defaultFlags,
                    i32 insertIndex = -1);

    /** Removes a column with the given ID.

        If there is such a column, this will asynchronously call the tableColumnsChanged() method of any
        registered listeners.
    */
    z0 removeColumn (i32 columnIdToRemove);

    /** Deletes all columns from the table.

        If there are any columns to remove, this will asynchronously call the tableColumnsChanged() method of any
        registered listeners.
    */
    z0 removeAllColumns();

    /** Returns the number of columns in the table.

        If onlyCountVisibleColumns is true, this will return the number of visible columns; otherwise it'll
        return the total number of columns, including hidden ones.

        @see isColumnVisible
    */
    i32 getNumColumns (b8 onlyCountVisibleColumns) const;

    /** Returns the name for a column.
        @see setColumnName
    */
    Txt getColumnName (i32 columnId) const;

    /** Changes the name of a column. */
    z0 setColumnName (i32 columnId, const Txt& newName);

    /** Moves a column to a different index in the table.

        @param columnId             the column to move
        @param newVisibleIndex      the target index for it, from 0 to the number of columns currently visible.
    */
    z0 moveColumn (i32 columnId, i32 newVisibleIndex);

    /** Returns the width of one of the columns.
    */
    i32 getColumnWidth (i32 columnId) const;

    /** Changes the width of a column.

        This will cause an asynchronous callback to the tableColumnsResized() method of any registered listeners.
    */
    z0 setColumnWidth (i32 columnId, i32 newWidth);

    /** Shows or hides a column.

        This can cause an asynchronous callback to the tableColumnsChanged() method of any registered listeners.
        @see isColumnVisible
    */
    z0 setColumnVisible (i32 columnId, b8 shouldBeVisible);

    /** Возвращает true, если this column is currently visible.
        @see setColumnVisible
    */
    b8 isColumnVisible (i32 columnId) const;

    /** Changes the column which is the sort column.

        This can cause an asynchronous callback to the tableSortOrderChanged() method of any registered listeners.

        If this method doesn't actually change the column ID, then no re-sort will take place (you can
        call reSortTable() to force a re-sort to happen if you've modified the table's contents).

        @see getSortColumnId, isSortedForwards, reSortTable
    */
    z0 setSortColumnId (i32 columnId, b8 sortForwards);

    /** Returns the column ID by which the table is currently sorted, or 0 if it is unsorted.

        @see setSortColumnId, isSortedForwards
    */
    i32 getSortColumnId() const;

    /** Возвращает true, если the table is currently sorted forwards, or false if it's backwards.
        @see setSortColumnId
    */
    b8 isSortedForwards() const;

    /** Triggers a re-sort of the table according to the current sort-column.

        If you modify the table's contents, you can call this to signal that the table needs
        to be re-sorted.

        (This doesn't do any sorting synchronously - it just asynchronously sends a call to the
        tableSortOrderChanged() method of any listeners).
    */
    z0 reSortTable();

    //==============================================================================
    /** Returns the total width of all the visible columns in the table.
    */
    i32 getTotalWidth() const;

    /** Returns the index of a given column.

        If there's no such column ID, this will return -1.

        If onlyCountVisibleColumns is true, this will return the index amongst the visible columns;
        otherwise it'll return the index amongst all the columns, including any hidden ones.
    */
    i32 getIndexOfColumnId (i32 columnId, b8 onlyCountVisibleColumns) const;

    /** Returns the ID of the column at a given index.

        If onlyCountVisibleColumns is true, this will count the index amongst the visible columns;
        otherwise it'll count it amongst all the columns, including any hidden ones.

        If the index is out-of-range, it'll return 0.
    */
    i32 getColumnIdOfIndex (i32 index, b8 onlyCountVisibleColumns) const;

    /** Returns the rectangle containing of one of the columns.

        The index is an index from 0 to the number of columns that are currently visible (hidden
        ones are not counted). It returns a rectangle showing the position of the column relative
        to this component's top-left. If the index is out-of-range, an empty rectangle is returned.
    */
    Rectangle<i32> getColumnPosition (i32 index) const;

    /** Finds the column ID at a given x-position in the component.
        If there is a column at this point this returns its ID, or if not, it will return 0.
    */
    i32 getColumnIdAtX (i32 xToFind) const;

    /** If set to true, this indicates that the columns should be expanded or shrunk to fill the
        entire width of the component.

        By default this is disabled. Turning it on also means that when resizing a column, those
        on the right will be squashed to fit.
    */
    z0 setStretchToFitActive (b8 shouldStretchToFit);

    /** Возвращает true, если stretch-to-fit has been enabled.
        @see setStretchToFitActive
    */
    b8 isStretchToFitActive() const;

    /** If stretch-to-fit is enabled, this will resize all the columns to make them fit into the
        specified width, keeping their relative proportions the same.

        If the minimum widths of the columns are too wide to fit into this space, it may
        actually end up wider.
    */
    z0 resizeAllColumnsToFit (i32 targetTotalWidth);

    //==============================================================================
    /** Enables or disables the pop-up menu.

        The default menu allows the user to show or hide columns. You can add custom
        items to this menu by overloading the addMenuItems() and reactToMenuItem() methods.

        By default the menu is enabled.

        @see isPopupMenuActive, addMenuItems, reactToMenuItem
    */
    z0 setPopupMenuActive (b8 hasMenu);

    /** Возвращает true, если the pop-up menu is enabled.
        @see setPopupMenuActive
    */
    b8 isPopupMenuActive() const;

    //==============================================================================
    /** Returns a string that encapsulates the table's current layout.

        This can be restored later using restoreFromString(). It saves the order of
        the columns, the currently-sorted column, and the widths.

        @see restoreFromString
    */
    Txt toString() const;

    /** Restores the state of the table, based on a string previously created with
        toString().

        @see toString
    */
    z0 restoreFromString (const Txt& storedVersion);

    //==============================================================================
    /**
        Receives events from a TableHeaderComponent when columns are resized, moved, etc.

        You can register one of these objects for table events using TableHeaderComponent::addListener()
        and TableHeaderComponent::removeListener().

        @see TableHeaderComponent
    */
    class DRX_API  Listener
    {
    public:
        //==============================================================================
        Listener() = default;

        /** Destructor. */
        virtual ~Listener() = default;

        //==============================================================================
        /** This is called when some of the table's columns are added, removed, hidden,
            or rearranged.
        */
        virtual z0 tableColumnsChanged (TableHeaderComponent* tableHeader) = 0;

        /** This is called when one or more of the table's columns are resized. */
        virtual z0 tableColumnsResized (TableHeaderComponent* tableHeader) = 0;

        /** This is called when the column by which the table should be sorted is changed. */
        virtual z0 tableSortOrderChanged (TableHeaderComponent* tableHeader) = 0;

        /** This is called when the user begins or ends dragging one of the columns around.

            When the user starts dragging a column, this is called with the ID of that
            column. When they finish dragging, it is called again with 0 as the ID.
        */
        virtual z0 tableColumnDraggingChanged (TableHeaderComponent* tableHeader,
                                                 i32 columnIdNowBeingDragged);
    };

    /** Adds a listener to be informed about things that happen to the header. */
    z0 addListener (Listener* newListener);

    /** Removes a previously-registered listener. */
    z0 removeListener (Listener* listenerToRemove);

    //==============================================================================
    /** This can be overridden to handle a mouse-click on one of the column headers.

        The default implementation will use this click to call getSortColumnId() and
        change the sort order.
    */
    virtual z0 columnClicked (i32 columnId, const ModifierKeys& mods);

    /** This can be overridden to add custom items to the pop-up menu.

        If you override this, you should call the superclass's method to add its
        column show/hide items, if you want them on the menu as well.

        Then to handle the result, override reactToMenuItem().

        @see reactToMenuItem
    */
    virtual z0 addMenuItems (PopupMenu& menu, i32 columnIdClicked);

    /** Override this to handle any custom items that you have added to the
        pop-up menu with an addMenuItems() override.

        If the menuReturnId isn't one of your own custom menu items, you'll need to
        call TableHeaderComponent::reactToMenuItem() to allow the base class to
        handle the items that it had added.

        @see addMenuItems
    */
    virtual z0 reactToMenuItem (i32 menuReturnId, i32 columnIdClicked);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the TableHeaderComponent.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        textColorId                   = 0x1003800, /**< The colour for the text in the header. */
        backgroundColorId             = 0x1003810, /**< The colour of the table header background.
                                                         It's up to the LookAndFeel how this is used. */
        outlineColorId                = 0x1003820, /**< The colour of the table header's outline. */
        highlightColorId              = 0x1003830, /**< The colour of the table header background when
                                                         the mouse is over or down above the the table
                                                         header. It's up to the LookAndFeel to use a
                                                         variant of this colour to distinguish between
                                                         the down and hover state. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawTableHeaderBackground (Graphics&, TableHeaderComponent&) = 0;

        virtual z0 drawTableHeaderColumn (Graphics&, TableHeaderComponent&,
                                            const Txt& columnName, i32 columnId,
                                            i32 width, i32 height,
                                            b8 isMouseOver, b8 isMouseDown, i32 columnFlags) = 0;
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 mouseMove (const MouseEvent&) override;
    /** @internal */
    z0 mouseEnter (const MouseEvent&) override;
    /** @internal */
    z0 mouseExit (const MouseEvent&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    MouseCursor getMouseCursor() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

    /** Can be overridden for more control over the pop-up menu behaviour. */
    virtual z0 showColumnChooserMenu (i32 columnIdClicked);

private:
    struct ColumnInfo : public Component
    {
        ColumnInfo() { setInterceptsMouseClicks (false, false); }
        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

        i32 id, propertyFlags, width, minimumWidth, maximumWidth;
        f64 lastDeliberateWidth;
    };

    OwnedArray<ColumnInfo> columns;
    Array<Listener*> listeners;
    std::unique_ptr<Component> dragOverlayComp;
    class DragOverlayComp;

    b8 columnsChanged = false, columnsResized = false, sortChanged = false;
    b8 menuActive = true, stretchToFit = false;
    i32 columnIdBeingResized = 0, columnIdBeingDragged = 0, initialColumnWidth = 0;
    i32 columnIdUnderMouse = 0, draggingColumnOffset = 0, draggingColumnOriginalIndex = 0, lastDeliberateWidth = 0;

    ColumnInfo* getInfoForId (i32 columnId) const;
    i32 visibleIndexToTotalIndex (i32 visibleIndex) const;
    z0 sendColumnsChanged();
    z0 handleAsyncUpdate() override;
    z0 beginDrag (const MouseEvent&);
    z0 endDrag (i32 finalIndex);
    i32 getResizeDraggerAt (i32 mouseX) const;
    z0 updateColumnUnderMouse (const MouseEvent&);
    z0 setColumnUnderMouse (i32 columnId);
    z0 resizeColumnsToFit (i32 firstColumnIndex, i32 targetTotalWidth);
    z0 drawColumnHeader (Graphics&, LookAndFeel&, const ColumnInfo&);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableHeaderComponent)
};


} // namespace drx
