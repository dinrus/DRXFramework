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

class TreeView;

//==============================================================================
/**
    An item in a TreeView.

    A TreeViewItem can either be a leaf-node in the tree, or it can contain its
    own sub-items.

    To implement an item that contains sub-items, override the itemOpennessChanged()
    method so that when it is opened, it adds the new sub-items to itself using the
    addSubItem method. Depending on the nature of the item it might choose to only
    do this the first time it's opened, or it might want to refresh itself each time.
    It also has the option of deleting its sub-items when it is closed, or leaving them
    in place.

    @tags{GUI}
*/
class DRX_API  TreeViewItem
{
public:
    //==============================================================================
    /** Constructor. */
    TreeViewItem();

    /** Destructor. */
    virtual ~TreeViewItem();

    //==============================================================================
    /** Returns the number of sub-items that have been added to this item.
        Note that this doesn't mean much if the node isn't open.

        @see getSubItem, mightContainSubItems, addSubItem
    */
    i32 getNumSubItems() const noexcept;

    /** Returns one of the item's sub-items.
        Remember that the object returned might get deleted at any time when its parent
        item is closed or refreshed, depending on the nature of the items you're using.

        @see getNumSubItems
    */
    TreeViewItem* getSubItem (i32 index) const noexcept;

    /** Removes any sub-items. */
    z0 clearSubItems();

    /** Adds a sub-item.

        @param newItem  the object to add to the item's sub-item list. Once added, these can be
                        found using getSubItem(). When the items are later removed with
                        removeSubItem() (or when this item is deleted), they will be deleted.
        @param insertPosition   the index which the new item should have when it's added. If this
                                value is less than 0, the item will be added to the end of the list.
    */
    z0 addSubItem (TreeViewItem* newItem, i32 insertPosition = -1);

    /** Adds a sub-item with a sort-comparator, assuming that the existing items are already sorted.

        @param comparator  the comparator object for sorting - see sortSubItems() for details about
                        the methods this class must provide.
        @param newItem  the object to add to the item's sub-item list. Once added, these can be
                        found using getSubItem(). When the items are later removed with
                        removeSubItem() (or when this item is deleted), they will be deleted.
    */
    template <class ElementComparator>
    z0 addSubItemSorted (ElementComparator& comparator, TreeViewItem* newItem)
    {
        addSubItem (newItem, findInsertIndexInSortedArray (comparator, subItems.begin(), newItem, 0, subItems.size()));
    }

    /** Removes one of the sub-items.

        @param index        the item to remove
        @param deleteItem   if true, the item that is removed will also be deleted.
    */
    z0 removeSubItem (i32 index, b8 deleteItem = true);

    /** Sorts the list of sub-items using a standard array comparator.

        This will use a comparator object to sort the elements into order. The comparator
        object must have a method of the form:
        @code
        i32 compareElements (TreeViewItem* first, TreeViewItem* second);
        @endcode

        ..and this method must return:
          - a value of < 0 if the first comes before the second
          - a value of 0 if the two objects are equivalent
          - a value of > 0 if the second comes before the first

        To improve performance, the compareElements() method can be declared as static or const.
    */
    template <class ElementComparator>
    z0 sortSubItems (ElementComparator& comparator)
    {
        subItems.sort (comparator);
    }

    //==============================================================================
    /** Returns the TreeView to which this item belongs. */
    TreeView* getOwnerView() const noexcept             { return ownerView; }

    /** Returns the item within which this item is contained. */
    TreeViewItem* getParentItem() const noexcept        { return parentItem; }

    //==============================================================================
    /** True if this item is currently open in the TreeView.

        @see getOpenness
    */
    b8 isOpen() const noexcept;

    /** Opens or closes the item.

        When opened or closed, the item's itemOpennessChanged() method will be called,
        and a subclass should use this callback to create and add any sub-items that
        it needs to.

        Note that if this is called when the item is in its default openness state, and
        this call would not change whether it's open or closed, then no change will be
        stored. If you want to explicitly set the openness state to be non-default then
        you should use setOpenness instead.

        @see setOpenness, itemOpennessChanged, mightContainSubItems
    */
    z0 setOpen (b8 shouldBeOpen);

    /** An enum of states to describe the explicit or implicit openness of an item. */
    enum class Openness
    {
        opennessDefault,
        opennessClosed,
        opennessOpen
    };

    /** Returns the openness state of this item.

        @see isOpen
    */
    Openness getOpenness() const noexcept;

    /** Opens or closes the item.

        If this causes the value of isOpen() to change, then the item's itemOpennessChanged()
        method will be called, and a subclass should use this callback to create and add any
        sub-items that it needs to.

        @see setOpen
    */
    z0 setOpenness (Openness newOpenness);

    /** True if this item is currently selected.

        Use this when painting the node, to decide whether to draw it as selected or not.
    */
    b8 isSelected() const noexcept;

    /** Selects or deselects the item.

        If shouldNotify == sendNotification, then a callback will be made
        to itemSelectionChanged() if the item's selection has changed.
    */
    z0 setSelected (b8 shouldBeSelected,
                      b8 deselectOtherItemsFirst,
                      NotificationType shouldNotify = sendNotification);

    /** Returns the rectangle that this item occupies.

        If relativeToTreeViewTopLeft is true, the coordinates are relative to the
        top-left of the TreeView comp, so this will depend on the scroll-position of
        the tree. If false, it is relative to the top-left of the topmost item in the
        tree (so this would be unaffected by scrolling the view).
    */
    Rectangle<i32> getItemPosition (b8 relativeToTreeViewTopLeft) const noexcept;

    /** Sends a signal to the TreeView to make it refresh itself.

        Call this if your items have changed and you want the tree to update to reflect this.
    */
    z0 treeHasChanged() const noexcept;

    /** Sends a repaint message to redraw just this item.

        Note that you should only call this if you want to repaint a superficial change. If
        you're altering the tree's nodes, you should instead call treeHasChanged().
    */
    z0 repaintItem() const;

    /** Returns the row number of this item in the tree.

        The row number of an item will change according to which items are open.

        @see TreeView::getNumRowsInTree(), TreeView::getItemOnRow()
    */
    i32 getRowNumberInTree() const noexcept;

    /** Возвращает true, если all the item's parent nodes are open.

        This is useful to check whether the item might actually be visible or not.
    */
    b8 areAllParentsOpen() const noexcept;

    /** Changes whether lines are drawn to connect any sub-items to this item.

        By default, line-drawing is turned on according to LookAndFeel::areLinesDrawnForTreeView().
    */
    z0 setLinesDrawnForSubItems (b8 shouldDrawLines) noexcept;

    //==============================================================================
    /** Tells the tree whether this item can potentially be opened.

        If your item could contain sub-items, this should return true; if it returns
        false then the tree will not try to open the item. This determines whether or
        not the item will be drawn with a 'plus' button next to it.
    */
    virtual b8 mightContainSubItems() = 0;

    /** Returns a string to uniquely identify this item.

        If you're planning on using the TreeView::getOpennessState() method, then
        these strings will be used to identify which nodes are open. The string
        should be unique amongst the item's sibling items, but it's ok for there
        to be duplicates at other levels of the tree.

        If you're not going to store the state, then it's ok not to bother implementing
        this method.
    */
    virtual Txt getUniqueName() const;

    /** Called when an item is opened or closed.

        When setOpen() is called and the item has specified that it might
        have sub-items with the mightContainSubItems() method, this method
        is called to let the item create or manage its sub-items.

        So when this is called with isNowOpen set to true (i.e. when the item is being
        opened), a subclass might choose to use clearSubItems() and addSubItem() to
        refresh its sub-item list.

        When this is called with isNowOpen set to false, the subclass might want
        to use clearSubItems() to save on space, or it might choose to leave them,
        depending on the nature of the tree.

        You could also use this callback as a trigger to start a background process
        which asynchronously creates sub-items and adds them, if that's more
        appropriate for the task in hand.

        @see mightContainSubItems
    */
    virtual z0 itemOpennessChanged (b8 isNowOpen);

    /** Must return the width required by this item.

        If your item needs to have a particular width in pixels, return that value; if
        you'd rather have it just fill whatever space is available in the TreeView,
        return -1.

        If all your items return -1, no horizontal scrollbar will be shown, but if any
        items have fixed widths and extend beyond the width of the TreeView, a
        scrollbar will appear.

        Each item can be a different width, but if they change width, you should call
        treeHasChanged() to update the tree.
    */
    virtual i32 getItemWidth() const                          { return -1; }

    /** Must return the height required by this item.

        This is the height in pixels that the item will take up. Items in the tree
        can be different heights, but if they change height, you should call
        treeHasChanged() to update the tree.
    */
    virtual i32 getItemHeight() const                         { return 20; }

    /** You can override this method to return false if you don't want to allow the
        user to select this item.
    */
    virtual b8 canBeSelected() const                        { return true; }

    /** Creates a component that will be used to represent this item.

        You don't have to implement this method - if it returns nullptr then no component
        will be used for the item, and you can just draw it using the paintItem()
        callback. But if you do return a component, it will be positioned in the
        TreeView so that it can be used to represent this item.

        The component returned will be managed by the TreeView and will be deleted
        later when it goes off the screen or is no longer needed. Its position and
        size will be completely managed by the tree, so don't attempt to move it around.

        Something you may want to do with your component is to give it a pointer to
        the TreeView that created it. This is perfectly safe, and there's no danger
        of it becoming a dangling pointer because the TreeView will always delete
        the component before it is itself deleted.

        As i64 as you stick to these rules you can return whatever kind of
        component you like. It's most useful if you're doing things like drag-and-drop
        of items, or want to use a Label component to edit item names, etc.
    */
    virtual std::unique_ptr<Component> createItemComponent()  { return nullptr; }

    //==============================================================================
    /** Draws the item's contents.

        You can choose to either implement this method and draw each item, or you
        can use createItemComponent() to create a component that will represent the
        item.

        If all you need in your tree is to be able to draw the items and detect when
        the user selects or f64-clicks one of them, it's probably enough to
        use paintItem(), itemClicked() and itemDoubleClicked(). If you need more
        complicated interactions, you may need to use createItemComponent() instead.

        @param g        the graphics context to draw into
        @param width    the width of the area available for drawing
        @param height   the height of the area available for drawing
    */
    virtual z0 paintItem (Graphics& g, i32 width, i32 height);

    /** Draws the item's open/close button.

        If you don't implement this method, the default behaviour is to call
        LookAndFeel::drawTreeviewPlusMinusBox(), but you can override it for custom
        effects. You may want to override it and call the base-class implementation
        with a different backgroundColor parameter, if your implementation has a
        background colour other than the default (white).
    */
    virtual z0 paintOpenCloseButton (Graphics&, const Rectangle<f32>& area,
                                       Color backgroundColor, b8 isMouseOver);

    /** Draws the line that connects this item to the vertical line extending below its parent. */
    virtual z0 paintHorizontalConnectingLine (Graphics&, const Line<f32>& line);

    /** Draws the line that extends vertically up towards one of its parents, or down to one of its children. */
    virtual z0 paintVerticalConnectingLine (Graphics&, const Line<f32>& line);

    /** This should return true if you want to use a custom component, and also use
        the TreeView's built-in mouse handling support, enabling drag-and-drop,
        itemClicked() and itemDoubleClicked(); return false if the component should
        consume all mouse clicks.
    */
    virtual b8 customComponentUsesTreeViewMouseHandler() const     { return false; }

    /** Called when the user clicks on this item.

        If you're using createItemComponent() to create a custom component for the
        item, the mouse-clicks might not make it through to the TreeView, but this
        is how you find out about clicks when just drawing each item individually.

        The associated mouse-event details are passed in, so you can find out about
        which button, where it was, etc.

        @see itemDoubleClicked
    */
    virtual z0 itemClicked (const MouseEvent&);

    /** Called when the user f64-clicks on this item.

        If you're using createItemComponent() to create a custom component for the
        item, the mouse-clicks might not make it through to the TreeView, but this
        is how you find out about clicks when just drawing each item individually.

        The associated mouse-event details are passed in, so you can find out about
        which button, where it was, etc.

        If not overridden, the base class method here will open or close the item as
        if the 'plus' button had been clicked.

        @see itemClicked
    */
    virtual z0 itemDoubleClicked (const MouseEvent&);

    /** Called when the item is selected or deselected.

        Use this if you want to do something special when the item's selectedness
        changes. By default it'll get repainted when this happens.
    */
    virtual z0 itemSelectionChanged (b8 isNowSelected);

    /** Called when the owner view changes */
    virtual z0 ownerViewChanged (TreeView* newOwner);

    /** The item can return a tool tip string here if it wants to.

        @see TooltipClient
    */
    virtual Txt getTooltip();

    /** Use this to set the name for this item that will be read out by accessibility
        clients.

        The default implementation will return the tooltip string from getTooltip()
        if it is not empty, otherwise it will return a description of the nested level
        and row number of the item.

        @see AccessibilityHandler
    */
    virtual Txt getAccessibilityName();

    //==============================================================================
    /** To allow items from your TreeView to be dragged-and-dropped, implement this method.

        If this returns a non-null variant then when the user drags an item, the TreeView will
        try to find a DragAndDropContainer in its parent hierarchy, and will use it to trigger
        a drag-and-drop operation, using this string as the source description, with the TreeView
        itself as the source component.

        If you need more complex drag-and-drop behaviour, you can use custom components for
        the items, and use those to trigger the drag.

        To accept drag-and-drop in your tree, see isInterestedInDragSource(),
        isInterestedInFileDrag(), etc.

        @see DragAndDropContainer::startDragging
    */
    virtual var getDragSourceDescription();

    /** If you want your item to be able to have files drag-and-dropped onto it, implement this
        method and return true.

        If you return true and allow some files to be dropped, you'll also need to implement the
        filesDropped() method to do something with them.

        Note that this will be called often, so make your implementation very quick! There's
        certainly no time to try opening the files and having a think about what's inside them!

        For responding to internal drag-and-drop of other types of object, see isInterestedInDragSource().

        @see FileDragAndDropTarget::isInterestedInFileDrag, isInterestedInDragSource
    */
    virtual b8 isInterestedInFileDrag (const StringArray& files);

    /** When files are dropped into this item, this callback is invoked.

        For this to work, you'll need to have also implemented isInterestedInFileDrag().
        The insertIndex value indicates where in the list of sub-items the files were dropped.
        If files are dropped onto an area of the tree where there are no visible items, this
        method is called on the root item of the tree, with an insert index of 0.

        @see FileDragAndDropTarget::filesDropped, isInterestedInFileDrag
    */
    virtual z0 filesDropped (const StringArray& files, i32 insertIndex);

    /** If you want your item to act as a DragAndDropTarget, implement this method and return true.

        If you implement this method, you'll also need to implement itemDropped() in order to handle
        the items when they are dropped.
        To respond to drag-and-drop of files from external applications, see isInterestedInFileDrag().

        @see DragAndDropTarget::isInterestedInDragSource, itemDropped
    */
    virtual b8 isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails);

    /** When a things are dropped into this item, this callback is invoked.

        For this to work, you need to have also implemented isInterestedInDragSource().
        The insertIndex value indicates where in the list of sub-items the new items should be placed.
        If files are dropped onto an area of the tree where there are no visible items, this
        method is called on the root item of the tree, with an insert index of 0.

        @see isInterestedInDragSource, DragAndDropTarget::itemDropped
    */
    virtual z0 itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, i32 insertIndex);

    //==============================================================================
    /** Sets a flag to indicate that the item wants to be allowed
        to draw all the way across to the left edge of the TreeView.

        By default this is false, which means that when the paintItem()
        method is called, its graphics context is clipped to only allow
        drawing within the item's rectangle. If this flag is set to true,
        then the graphics context isn't clipped on its left side, so it
        can draw all the way across to the left margin. Note that the
        context will still have its origin in the same place though, so
        the coordinates of anything to its left will be negative. It's
        mostly useful if you want to draw a wider bar behind the
        highlighted item.
    */
    z0 setDrawsInLeftMargin (b8 canDrawInLeftMargin) noexcept;

    /** Sets a flag to indicate that the item wants to be allowed
        to draw all the way across to the right edge of the TreeView.

        Similar to setDrawsInLeftMargin: when this flag is set to true,
        then the graphics context isn't clipped on the right side. Unlike
        setDrawsInLeftMargin, you will very rarely need to use this function,
        as this method won't clip the right margin unless your TreeViewItem
        overrides getItemWidth to return a positive value.

        @see setDrawsInLeftMargin, getItemWidth
     */
    z0 setDrawsInRightMargin (b8 canDrawInRightMargin) noexcept;

    //==============================================================================
    /** Saves the current state of open/closed nodes so it can be restored later.

        This takes a snapshot of which sub-nodes have been explicitly opened or closed,
        and records it as XML. To identify node objects it uses the
        TreeViewItem::getUniqueName() method to create named paths. This
        means that the same state of open/closed nodes can be restored to a
        completely different instance of the tree, as i64 as it contains nodes
        whose unique names are the same.

        You'd normally want to use TreeView::getOpennessState() rather than call it
        for a specific item, but this can be handy if you need to briefly save the state
        for a section of the tree.

        Note that if all nodes of the tree are in their default state, then this may
        return a nullptr.

        @see TreeView::getOpennessState, restoreOpennessState
    */
    std::unique_ptr<XmlElement> getOpennessState() const;

    /** Restores the openness of this item and all its sub-items from a saved state.

        See TreeView::restoreOpennessState for more details.

        You'd normally want to use TreeView::restoreOpennessState() rather than call it
        for a specific item, but this can be handy if you need to briefly save the state
        for a section of the tree.

        @see TreeView::restoreOpennessState, getOpennessState
    */
    z0 restoreOpennessState (const XmlElement& xml);

    //==============================================================================
    /** Returns the index of this item in its parent's sub-items. */
    i32 getIndexInParent() const noexcept;

    /** Возвращает true, если this item is the last of its parent's sub-items. */
    b8 isLastOfSiblings() const noexcept;

    /** Creates a string that can be used to uniquely retrieve this item in the tree.

        The string that is returned can be passed to TreeView::findItemFromIdentifierString().
        The string takes the form of a path, constructed from the getUniqueName() of this
        item and all its parents, so these must all be correctly implemented for it to work.

        @see TreeView::findItemFromIdentifierString, getUniqueName
    */
    Txt getItemIdentifierString() const;

    //==============================================================================
    /**
        This handy class takes a copy of a TreeViewItem's openness when you create it,
        and restores that openness state when its destructor is called.

        This can very handy when you're refreshing sub-items - e.g.
        @code
        z0 MyTreeViewItem::updateChildItems()
        {
            OpennessRestorer openness (*this);  //  saves the openness state here..

            clearSubItems();

            // add a bunch of sub-items here which may or may not be the same as the ones that
            // were previously there
            addSubItem (...

            // ..and at this point, the old openness is restored, so any items that haven't
            // changed will have their old openness retained.
        }
        @endcode
    */
    class DRX_API  OpennessRestorer
    {
    public:
        OpennessRestorer (TreeViewItem&);
        ~OpennessRestorer();

    private:
        TreeViewItem& treeViewItem;
        std::unique_ptr<XmlElement> oldOpenness;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpennessRestorer)
    };

private:
    //==============================================================================
    friend class TreeView;

    z0 updatePositions (i32);
    i32 getIndentX() const noexcept;
    z0 setOwnerView (TreeView*) noexcept;
    TreeViewItem* getTopLevelItem() noexcept;
    const TreeViewItem* getDeepestOpenParentItem() const noexcept;
    i32 getNumRows() const noexcept;
    TreeViewItem* getItemOnRow (i32) noexcept;
    z0 deselectAllRecursively (TreeViewItem*);
    i32 countSelectedItemsRecursively (i32) const noexcept;
    TreeViewItem* getSelectedItemWithIndex (i32) noexcept;
    TreeViewItem* findItemFromIdentifierString (const Txt&);
    z0 restoreToDefaultOpenness();
    b8 isFullyOpen() const noexcept;
    std::unique_ptr<XmlElement> getOpennessState (b8) const;
    b8 removeSubItemFromList (i32, b8);
    z0 removeAllSubItemsFromList();
    b8 areLinesDrawn() const;
    z0 draw (Graphics&, i32, b8);

    //==============================================================================
    TreeView* ownerView = nullptr;
    TreeViewItem* parentItem = nullptr;
    OwnedArray<TreeViewItem> subItems;

    Openness openness = Openness::opennessDefault;
    i32 y = 0, itemHeight = 0, totalHeight = 0, itemWidth = 0, totalWidth = 0, uid = 0;
    b8 selected = false, redrawNeeded = true, drawLinesInside = false, drawLinesSet = false,
         drawsInLeftMargin = false, drawsInRightMargin = false;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeViewItem)
};

//==============================================================================
/**
    A tree-view component.

    Use one of these to hold and display a structure of TreeViewItem objects.

    @tags{GUI}
*/
class DRX_API  TreeView  : public Component,
                            public SettableTooltipClient,
                            public FileDragAndDropTarget,
                            public DragAndDropTarget
{
public:
    //==============================================================================
    /** Creates an empty TreeView.

        Once you've got a TreeView component, you'll need to give it something to
        display, using the setRootItem() method.
    */
    TreeView (const Txt& componentName = {});

    /** Destructor. */
    ~TreeView() override;

    //==============================================================================
    /** Sets the item that is displayed in the TreeView.

        A tree has a single root item which contains as many sub-items as it needs. If
        you want the tree to contain a number of root items, you should still use a single
        root item above these, but hide it using setRootItemVisible().

        You can pass nullptr to this method to clear the tree and remove its current root item.

        The object passed in will not be deleted by the TreeView, it's up to the caller
        to delete it when no longer needed. BUT make absolutely sure that you don't delete
        this item until you've removed it from the tree, either by calling setRootItem (nullptr),
        or by deleting the tree first. You can also use deleteRootItem() as a quick way
        to delete it.
    */
    z0 setRootItem (TreeViewItem* newRootItem);

    /** Returns the tree's root item.

        This will be the last object passed to setRootItem(), or nullptr if none has been set.
    */
    TreeViewItem* getRootItem() const noexcept        { return rootItem; }

    /** This will remove and delete the current root item.

        It's a convenient way of deleting the item and calling setRootItem (nullptr).
    */
    z0 deleteRootItem();

    /** Changes whether the tree's root item is shown or not.

        If the root item is hidden, only its sub-items will be shown in the TreeView - this
        lets you make the tree look as if it's got many root items. If it's hidden, this call
        will also make sure the root item is open (otherwise the TreeView would look empty).
    */
    z0 setRootItemVisible (b8 shouldBeVisible);

    /** Возвращает true, если the root item is visible.

        @see setRootItemVisible
    */
    b8 isRootItemVisible() const noexcept           { return rootItemVisible; }

    /** Sets whether items are open or closed by default.

        Normally, items are closed until the user opens them, but you can use this
        to make them default to being open until explicitly closed.

        @see areItemsOpenByDefault
    */
    z0 setDefaultOpenness (b8 isOpenByDefault);

    /** Возвращает true, если the tree's items default to being open.

        @see setDefaultOpenness
    */
    b8 areItemsOpenByDefault() const noexcept       { return defaultOpenness; }

    /** This sets a flag to indicate that the tree can be used for multi-selection.

        You can always select multiple items internally by calling the
        TreeViewItem::setSelected() method, but this flag indicates whether the user
        is allowed to multi-select by clicking on the tree.

        By default it is disabled.

        @see isMultiSelectEnabled
    */
    z0 setMultiSelectEnabled (b8 canMultiSelect);

    /** Returns whether multi-select has been enabled for the tree.

        @see setMultiSelectEnabled
    */
    b8 isMultiSelectEnabled() const noexcept        { return multiSelectEnabled; }

    /** Sets a flag to indicate whether to hide the open/close buttons.

        @see areOpenCloseButtonsVisible
    */
    z0 setOpenCloseButtonsVisible (b8 shouldBeVisible);

    /** Returns whether open/close buttons are shown.

        @see setOpenCloseButtonsVisible
    */
    b8 areOpenCloseButtonsVisible() const noexcept  { return openCloseButtonsVisible; }

    //==============================================================================
    /** Deselects any items that are currently selected. */
    z0 clearSelectedItems();

    /** Returns the number of items that are currently selected.

        If maximumDepthToSearchTo is >= 0, it lets you specify a maximum depth to which the
        tree will be recursed.

        @see getSelectedItem, clearSelectedItems
    */
    i32 getNumSelectedItems (i32 maximumDepthToSearchTo = -1) const noexcept;

    /** Returns one of the selected items in the tree.

        @param index    the index, 0 to (getNumSelectedItems() - 1)
    */
    TreeViewItem* getSelectedItem (i32 index) const noexcept;

    /** Moves the selected row up or down by the specified number of rows. */
    z0 moveSelectedRow (i32 deltaRows);

    //==============================================================================
    /** Returns the number of rows the tree is using, depending on which items are open.

        @see TreeViewItem::getRowNumberInTree()
    */
    i32 getNumRowsInTree() const;

    /** Returns the item on a particular row of the tree.

        If the index is out of range, this will return nullptr.

        @see getNumRowsInTree, TreeViewItem::getRowNumberInTree()
    */
    TreeViewItem* getItemOnRow (i32 index) const;

    /** Returns the item that contains a given y-position relative to the top
        of the TreeView component.
    */
    TreeViewItem* getItemAt (i32 yPosition) const noexcept;

    /** Tries to scroll the tree so that this item is on-screen somewhere. */
    z0 scrollToKeepItemVisible (const TreeViewItem* item);

    /** Returns the TreeView's Viewport object. */
    Viewport* getViewport() const noexcept;

    /** Returns the number of pixels by which each nested level of the tree is indented.

        @see setIndentSize
    */
    i32 getIndentSize() noexcept;

    /** Changes the distance by which each nested level of the tree is indented.

        @see getIndentSize
    */
    z0 setIndentSize (i32 newIndentSize);

    /** Searches the tree for an item with the specified identifier.

        The identifier string must have been created by calling TreeViewItem::getItemIdentifierString().
        If no such item exists, this will return false. If the item is found, all of its items
        will be automatically opened.
    */
    TreeViewItem* findItemFromIdentifierString (const Txt& identifierString) const;

    /** Returns the component that currently represents a given TreeViewItem. */
    Component* getItemComponent (const TreeViewItem* item) const;

    //==============================================================================
    /** Saves the current state of open/closed nodes so it can be restored later.

        This takes a snapshot of which nodes have been explicitly opened or closed,
        and records it as XML. To identify node objects it uses the
        TreeViewItem::getUniqueName() method to create named paths. This
        means that the same state of open/closed nodes can be restored to a
        completely different instance of the tree, as i64 as it contains nodes
        whose unique names are the same.

        @param alsoIncludeScrollPosition    if this is true, the state will also
                                            include information about where the
                                            tree has been scrolled to vertically,
                                            so this can also be restored
        @see restoreOpennessState
    */
    std::unique_ptr<XmlElement> getOpennessState (b8 alsoIncludeScrollPosition) const;

    /** Restores a previously saved arrangement of open/closed nodes.

        This will try to restore a snapshot of the tree's state that was created by
        the getOpennessState() method. If any of the nodes named in the original
        XML aren't present in this tree, they will be ignored.

        If restoreStoredSelection is true, it will also try to re-select any items that
        were selected in the stored state.

        @see getOpennessState
    */
    z0 restoreOpennessState (const XmlElement& newState, b8 restoreStoredSelection);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the TreeView.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId             = 0x1000500, /**< A background colour to fill the component with. */
        linesColorId                  = 0x1000501, /**< The colour to draw the lines with.*/
        dragAndDropIndicatorColorId   = 0x1000502, /**< The colour to use for the drag-and-drop target position indicator. */
        selectedItemBackgroundColorId = 0x1000503, /**< The colour to use to fill the background of any selected items. */
        oddItemsColorId               = 0x1000504, /**< The colour to use to fill the background of the odd numbered items. */
        evenItemsColorId              = 0x1000505  /**< The colour to use to fill the background of the even numbered items. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        TreeView drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawTreeviewPlusMinusBox (Graphics&, const Rectangle<f32>& area,
                                               Color backgroundColor, b8 isItemOpen, b8 isMouseOver) = 0;

        virtual b8 areLinesDrawnForTreeView (TreeView&) = 0;
        virtual i32 getTreeViewIndentSize (TreeView&) = 0;
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 colourChanged() override;
    /** @internal */
    z0 enablementChanged() override;
    /** @internal */
    b8 isInterestedInFileDrag (const StringArray&) override;
    /** @internal */
    z0 fileDragEnter (const StringArray&, i32, i32) override;
    /** @internal */
    z0 fileDragMove (const StringArray&, i32, i32) override;
    /** @internal */
    z0 fileDragExit (const StringArray&) override;
    /** @internal */
    z0 filesDropped (const StringArray&, i32, i32) override;
    /** @internal */
    b8 isInterestedInDragSource (const SourceDetails&) override;
    /** @internal */
    z0 itemDragEnter (const SourceDetails&) override;
    /** @internal */
    z0 itemDragMove (const SourceDetails&) override;
    /** @internal */
    z0 itemDragExit (const SourceDetails&) override;
    /** @internal */
    z0 itemDropped (const SourceDetails&) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    friend class TreeViewItem;

    class ItemComponent;
    class ContentComponent;
    class TreeViewport;
    class InsertPointHighlight;
    class TargetGroupHighlight;
    class TreeAccessibilityHandler;
    struct InsertPoint;

    z0 itemsChanged() noexcept;
    z0 updateVisibleItems (std::optional<Point<i32>> viewportPosition = {});
    z0 updateButtonUnderMouse (const MouseEvent&);
    z0 showDragHighlight (const InsertPoint&) noexcept;
    z0 hideDragHighlight() noexcept;
    z0 handleDrag (const StringArray&, const SourceDetails&);
    z0 handleDrop (const StringArray&, const SourceDetails&);
    b8 toggleOpenSelectedItem();
    z0 moveOutOfSelectedItem();
    z0 moveIntoSelectedItem();
    z0 moveByPages (i32);

    std::unique_ptr<TreeViewport> viewport;
    TreeViewItem* rootItem = nullptr;
    std::unique_ptr<InsertPointHighlight> dragInsertPointHighlight;
    std::unique_ptr<TargetGroupHighlight> dragTargetGroupHighlight;
    i32 indentSize = -1;
    b8 defaultOpenness = false, rootItemVisible = true, multiSelectEnabled = false, openCloseButtonsVisible = true;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeView)
};

} // namespace drx
