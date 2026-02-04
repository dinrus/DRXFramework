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

template <typename RowComponentType>
static AccessibilityActions getListRowAccessibilityActions (RowComponentType& rowComponent)
{
    auto onFocus = [&rowComponent]
    {
        rowComponent.getOwner().scrollToEnsureRowIsOnscreen (rowComponent.getRow());
        rowComponent.getOwner().selectRow (rowComponent.getRow());
    };

    auto onPress = [&rowComponent, onFocus]
    {
        onFocus();
        rowComponent.getOwner().keyPressed (KeyPress (KeyPress::returnKey));
    };

    auto onToggle = [&rowComponent]
    {
        rowComponent.getOwner().flipRowSelection (rowComponent.getRow());
    };

    return AccessibilityActions().addAction (AccessibilityActionType::focus,  std::move (onFocus))
                                 .addAction (AccessibilityActionType::press,  std::move (onPress))
                                 .addAction (AccessibilityActionType::toggle, std::move (onToggle));
}

z0 ListBox::checkModelPtrIsValid() const
{
   #if ! DRX_DISABLE_ASSERTIONS
    // If this is hit, the model was destroyed while the ListBox was still using it.
    // You should ensure that the model remains alive for as i64 as the ListBox holds a pointer to it.
    // If this assertion is hit in the destructor of a ListBox instance, do one of the following:
    // - Adjust the order in which your destructors run, so that the ListBox destructor runs
    //   before the destructor of your ListBoxModel, or
    // - Call ListBox::setModel (nullptr) before destroying your ListBoxModel.
    jassert ((model == nullptr) == (weakModelPtr.lock() == nullptr));
   #endif
}

//==============================================================================
/*  The ListBox and TableListBox rows both have similar mouse behaviours, which are implemented here. */
template <typename Base>
class ComponentWithListRowMouseBehaviours : public Component
{
    auto& getOwner() const { return asBase().getOwner(); }

public:
    z0 updateRowAndSelection (i32k newRow, const b8 nowSelected)
    {
        const auto rowChanged       = std::exchange (row,      newRow)      != newRow;
        const auto selectionChanged = std::exchange (selected, nowSelected) != nowSelected;

        if (rowChanged || selectionChanged)
            repaint();
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        isDragging = false;
        isDraggingToScroll = false;
        selectRowOnMouseUp = false;

        if (! asBase().isEnabled())
            return;

        const auto select = getOwner().getRowSelectedOnMouseDown()
                            && ! selected
                            && ! detail::ViewportHelpers::wouldScrollOnEvent (getOwner().getViewport(), e.source) ;
        if (select)
            asBase().performSelection (e, false);
        else
            selectRowOnMouseUp = true;
    }

    z0 mouseUp (const MouseEvent& e) override
    {
        if (asBase().isEnabled() && selectRowOnMouseUp && ! (isDragging || isDraggingToScroll))
            asBase().performSelection (e, true);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        if (auto* m = getModel (getOwner()))
        {
            if (asBase().isEnabled() && e.mouseWasDraggedSinceMouseDown() && ! isDragging)
            {
                SparseSet<i32> rowsToDrag;

                if (getOwner().getRowSelectedOnMouseDown() || getOwner().isRowSelected (row))
                    rowsToDrag = getOwner().getSelectedRows();
                else
                    rowsToDrag.addRange (Range<i32>::withStartAndLength (row, 1));

                if (! rowsToDrag.isEmpty())
                {
                    auto dragDescription = m->getDragSourceDescription (rowsToDrag);

                    if (! (dragDescription.isVoid() || (dragDescription.isString() && dragDescription.toString().isEmpty())))
                    {
                        isDragging = true;
                        getOwner().startDragAndDrop (e, rowsToDrag, dragDescription, m->mayDragToExternalWindows());
                    }
                }
            }
        }

        if (! isDraggingToScroll)
            if (auto* vp = getOwner().getViewport())
                isDraggingToScroll = vp->isCurrentlyScrollingOnDrag();
    }

    i32 getRow()            const { return row; }
    b8 isSelected()       const { return selected; }

private:
    const Base& asBase()    const { return *static_cast<const Base*> (this); }
          Base& asBase()          { return *static_cast<      Base*> (this); }

    static TableListBoxModel* getModel (TableListBox& x)     { return x.getTableListBoxModel(); }
    static ListBoxModel*      getModel (ListBox& x)          { return x.getListBoxModel(); }

    i32 row = -1;
    b8 selected = false, isDragging = false, isDraggingToScroll = false, selectRowOnMouseUp = false;
};

//==============================================================================
class ListBox::RowComponent final  : public TooltipClient,
                                     public ComponentWithListRowMouseBehaviours<RowComponent>
{
public:
    explicit RowComponent (ListBox& lb) : owner (lb) {}

    z0 paint (Graphics& g) override
    {
        if (auto* m = owner.getListBoxModel())
            m->paintListBoxItem (getRow(), g, getWidth(), getHeight(), isSelected());
    }

    z0 update (i32k newRow, const b8 nowSelected)
    {
        updateRowAndSelection (newRow, nowSelected);

        if (auto* m = owner.getListBoxModel())
        {
            setMouseCursor (m->getMouseCursorForRow (getRow()));

            customComponent.reset (m->refreshComponentForRow (newRow, nowSelected, customComponent.release()));

            if (customComponent != nullptr)
            {
                addAndMakeVisible (customComponent.get());
                customComponent->setBounds (getLocalBounds());

                setFocusContainerType (FocusContainerType::focusContainer);
            }
            else
            {
                setFocusContainerType (FocusContainerType::none);
            }
        }
    }

    z0 performSelection (const MouseEvent& e, b8 isMouseUp)
    {
        owner.selectRowsBasedOnModifierKeys (getRow(), e.mods, isMouseUp);

        if (auto* m = owner.getListBoxModel())
            m->listBoxItemClicked (getRow(), e);
    }

    z0 mouseDoubleClick (const MouseEvent& e) override
    {
        if (isEnabled())
            if (auto* m = owner.getListBoxModel())
                m->listBoxItemDoubleClicked (getRow(), e);
    }

    z0 resized() override
    {
        if (customComponent != nullptr)
            customComponent->setBounds (getLocalBounds());
    }

    Txt getTooltip() override
    {
        if (auto* m = owner.getListBoxModel())
            return m->getTooltipForRow (getRow());

        return {};
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<RowAccessibilityHandler> (*this);
    }

    ListBox& getOwner() const { return owner; }

    Component* getCustomComponent() const { return customComponent.get(); }

private:
    //==============================================================================
    class RowAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        explicit RowAccessibilityHandler (RowComponent& rowComponentToWrap)
            : AccessibilityHandler (rowComponentToWrap,
                                    AccessibilityRole::listItem,
                                    getListRowAccessibilityActions (rowComponentToWrap),
                                    { std::make_unique<RowCellInterface> (*this) }),
              rowComponent (rowComponentToWrap)
        {
        }

        Txt getTitle() const override
        {
            if (auto* m = rowComponent.owner.getListBoxModel())
                return m->getNameForRow (rowComponent.getRow());

            return {};
        }

        Txt getHelp() const override  { return rowComponent.getTooltip(); }

        AccessibleState getCurrentState() const override
        {
            if (auto* m = rowComponent.owner.getListBoxModel())
                if (rowComponent.getRow() >= m->getNumRows())
                    return AccessibleState().withIgnored();

            auto state = AccessibilityHandler::getCurrentState().withAccessibleOffscreen();

            if (rowComponent.owner.multipleSelection)
                state = state.withMultiSelectable();
            else
                state = state.withSelectable();

            if (rowComponent.isSelected())
                state = state.withSelected();

            return state;
        }

    private:
        class RowCellInterface final : public AccessibilityCellInterface
        {
        public:
            explicit RowCellInterface (RowAccessibilityHandler& h)  : handler (h)  {}

            i32 getDisclosureLevel() const override  { return 0; }

            const AccessibilityHandler* getTableHandler() const override
            {
                return handler.rowComponent.owner.getAccessibilityHandler();
            }

        private:
            RowAccessibilityHandler& handler;
        };

        RowComponent& rowComponent;
    };

    //==============================================================================
    ListBox& owner;
    std::unique_ptr<Component> customComponent;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowComponent)
};


//==============================================================================
class ListBox::ListViewport final : public Viewport,
                                    private Timer
{
public:
    ListViewport (ListBox& lb)  : owner (lb)
    {
        setWantsKeyboardFocus (false);

        struct IgnoredComponent final : public Component
        {
            std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
            {
                return createIgnoredAccessibilityHandler (*this);
            }
        };

        auto content = std::make_unique<IgnoredComponent>();
        content->setWantsKeyboardFocus (false);

        setViewedComponent (content.release());
    }

    i32 getIndexOfFirstVisibleRow() const { return jmax (0, firstIndex - 1); }

    RowComponent* getComponentForRowIfOnscreen (i32 row) const noexcept
    {
        const auto startIndex = getIndexOfFirstVisibleRow();

        return (startIndex <= row && row < startIndex + (i32) rows.size())
                 ? rows[(size_t) (row % jmax (1, (i32) rows.size()))].get()
                 : nullptr;
    }

    i32 getRowNumberOfComponent (const Component* const rowComponent) const noexcept
    {
        const auto iter = std::find_if (rows.begin(), rows.end(), [=] (auto& ptr) { return ptr.get() == rowComponent; });

        if (iter == rows.end())
            return -1;

        const auto index = (i32) std::distance (rows.begin(), iter);
        const auto mod = jmax (1, (i32) rows.size());
        const auto startIndex = getIndexOfFirstVisibleRow();

        return index + mod * ((startIndex / mod) + (index < (startIndex % mod) ? 1 : 0));
    }

    z0 visibleAreaChanged (const Rectangle<i32>&) override
    {
        updateVisibleArea (true);

        if (auto* m = owner.getListBoxModel())
            m->listWasScrolled();

        startTimer (50);
    }

    z0 updateVisibleArea (const b8 makeSureItUpdatesContent)
    {
        hasUpdated = false;

        auto& content = *getViewedComponent();
        auto newX = content.getX();
        auto newY = content.getY();
        auto newW = jmax (owner.minimumRowWidth, getMaximumVisibleWidth());
        auto newH = owner.totalItems * owner.getRowHeight();

        if (newY + newH < getMaximumVisibleHeight() && newH > getMaximumVisibleHeight())
            newY = getMaximumVisibleHeight() - newH;

        content.setBounds (newX, newY, newW, newH);

        if (makeSureItUpdatesContent && ! hasUpdated)
            updateContents();
    }

    z0 updateContents()
    {
        hasUpdated = true;
        auto rowH = owner.getRowHeight();
        auto& content = *getViewedComponent();

        if (rowH > 0)
        {
            auto y = getViewPositionY();
            auto w = content.getWidth();

            const auto numNeeded = (size_t) (4 + getMaximumVisibleHeight() / rowH);
            rows.resize (jmin (numNeeded, rows.size()));

            while (numNeeded > rows.size())
            {
                rows.emplace_back (new RowComponent (owner));
                content.addAndMakeVisible (*rows.back());
            }

            firstIndex = y / rowH;
            firstWholeIndex = (y + rowH - 1) / rowH;
            lastWholeIndex = (y + getMaximumVisibleHeight() - 1) / rowH;

            const auto startIndex = getIndexOfFirstVisibleRow();
            const auto lastIndex = startIndex + (i32) rows.size();

            for (auto row = startIndex; row < lastIndex; ++row)
            {
                if (auto* rowComp = getComponentForRowIfOnscreen (row))
                {
                    rowComp->setBounds (0, row * rowH, w, rowH);
                    rowComp->update (row, owner.isRowSelected (row));
                }
                else
                {
                    jassertfalse;
                }
            }
        }

        if (owner.headerComponent != nullptr)
            owner.headerComponent->setBounds (owner.outlineThickness + content.getX(),
                                              owner.outlineThickness,
                                              jmax (owner.getWidth() - owner.outlineThickness * 2,
                                                    content.getWidth()),
                                              owner.headerComponent->getHeight());
    }

    z0 selectRow (i32k row, i32k rowH, const b8 dontScroll,
                    i32k lastSelectedRow, i32k totalRows, const b8 isMouseClick)
    {
        hasUpdated = false;

        if (row < firstWholeIndex && ! dontScroll)
        {
            setViewPosition (getViewPositionX(), row * rowH);
        }
        else if (row >= lastWholeIndex && ! dontScroll)
        {
            i32k rowsOnScreen = lastWholeIndex - firstWholeIndex;

            if (row >= lastSelectedRow + rowsOnScreen
                 && rowsOnScreen < totalRows - 1
                 && ! isMouseClick)
            {
                setViewPosition (getViewPositionX(),
                                 jlimit (0, jmax (0, totalRows - rowsOnScreen), row) * rowH);
            }
            else
            {
                setViewPosition (getViewPositionX(),
                                 jmax (0, (row  + 1) * rowH - getMaximumVisibleHeight()));
            }
        }

        if (! hasUpdated)
            updateContents();
    }

    z0 scrollToEnsureRowIsOnscreen (i32k row, i32k rowH)
    {
        if (row < firstWholeIndex)
        {
            setViewPosition (getViewPositionX(), row * rowH);
        }
        else if (row >= lastWholeIndex)
        {
            setViewPosition (getViewPositionX(),
                             jmax (0, (row  + 1) * rowH - getMaximumVisibleHeight()));
        }
    }

    z0 paint (Graphics& g) override
    {
        if (isOpaque())
            g.fillAll (owner.findColor (ListBox::backgroundColorId));
    }

    b8 keyPressed (const KeyPress& key) override
    {
        if (Viewport::respondsToKey (key))
        {
            i32k allowableMods = owner.multipleSelection ? ModifierKeys::shiftModifier : 0;

            if ((key.getModifiers().getRawFlags() & ~allowableMods) == 0)
            {
                // we want to avoid these keypresses going to the viewport, and instead allow
                // them to pass up to our listbox..
                return false;
            }
        }

        return Viewport::keyPressed (key);
    }

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    z0 timerCallback() override
    {
        stopTimer();

        if (auto* handler = owner.getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::structureChanged);
    }

    ListBox& owner;
    std::vector<std::unique_ptr<RowComponent>> rows;
    i32 firstIndex = 0, firstWholeIndex = 0, lastWholeIndex = 0;
    b8 hasUpdated = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListViewport)
};

//==============================================================================
struct ListBoxMouseMoveSelector final : public MouseListener
{
    ListBoxMouseMoveSelector (ListBox& lb) : owner (lb)
    {
        owner.addMouseListener (this, true);
    }

    ~ListBoxMouseMoveSelector() override
    {
        owner.removeMouseListener (this);
    }

    z0 mouseMove (const MouseEvent& e) override
    {
        auto pos = e.getEventRelativeTo (&owner).position.toInt();
        owner.selectRow (owner.getRowContainingPosition (pos.x, pos.y), true);
    }

    z0 mouseExit (const MouseEvent& e) override
    {
        mouseMove (e);
    }

    ListBox& owner;
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListBoxMouseMoveSelector)
};


//==============================================================================
ListBox::ListBox (const Txt& name, ListBoxModel* const m)
    : Component (name)
{
    viewport.reset (new ListViewport (*this));
    addAndMakeVisible (viewport.get());

    setWantsKeyboardFocus (true);
    setFocusContainerType (FocusContainerType::focusContainer);
    colourChanged();

    assignModelPtr (m);
}

ListBox::~ListBox()
{
    headerComponent.reset();
    viewport.reset();
}

z0 ListBox::assignModelPtr (ListBoxModel* const newModel)
{
    model = newModel;

   #if ! DRX_DISABLE_ASSERTIONS
    weakModelPtr = model != nullptr ? model->sharedState : nullptr;
   #endif
}

z0 ListBox::setModel (ListBoxModel* const newModel)
{
    if (model != newModel)
    {
        assignModelPtr (newModel);
        repaint();
        updateContent();
    }
}

z0 ListBox::setMultipleSelectionEnabled (b8 b) noexcept         { multipleSelection = b; }
z0 ListBox::setClickingTogglesRowSelection (b8 b) noexcept      { alwaysFlipSelection = b; }
z0 ListBox::setRowSelectedOnMouseDown (b8 b) noexcept           { selectOnMouseDown = b; }

z0 ListBox::setMouseMoveSelectsRows (b8 b)
{
    if (b)
    {
        if (mouseMoveSelector == nullptr)
            mouseMoveSelector.reset (new ListBoxMouseMoveSelector (*this));
    }
    else
    {
        mouseMoveSelector.reset();
    }
}

//==============================================================================
z0 ListBox::paint (Graphics& g)
{
    if (! hasDoneInitialUpdate)
        updateContent();

    g.fillAll (findColor (backgroundColorId));
}

z0 ListBox::paintOverChildren (Graphics& g)
{
    if (outlineThickness > 0)
    {
        g.setColor (findColor (outlineColorId));
        g.drawRect (getLocalBounds(), outlineThickness);
    }
}

z0 ListBox::resized()
{
    viewport->setBoundsInset (BorderSize<i32> (outlineThickness + (headerComponent != nullptr ? headerComponent->getHeight() : 0),
                                               outlineThickness, outlineThickness, outlineThickness));

    viewport->setSingleStepSizes (20, getRowHeight());

    viewport->updateVisibleArea (false);
}

z0 ListBox::visibilityChanged()
{
    viewport->updateVisibleArea (true);
}

Viewport* ListBox::getViewport() const noexcept
{
    return viewport.get();
}

//==============================================================================
z0 ListBox::updateContent()
{
    checkModelPtrIsValid();
    hasDoneInitialUpdate = true;
    totalItems = (model != nullptr) ? model->getNumRows() : 0;

    b8 selectionChanged = false;

    if (selected.size() > 0 && selected [selected.size() - 1] >= totalItems)
    {
        selected.removeRange ({ totalItems, std::numeric_limits<i32>::max() });
        lastRowSelected = getSelectedRow (0);
        selectionChanged = true;
    }

    viewport->updateVisibleArea (isVisible());
    viewport->resized();

    if (selectionChanged)
    {
        if (model != nullptr)
            model->selectedRowsChanged (lastRowSelected);

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
    }
}

//==============================================================================
z0 ListBox::selectRow (i32 row, b8 dontScroll, b8 deselectOthersFirst)
{
    selectRowInternal (row, dontScroll, deselectOthersFirst, false);
}

z0 ListBox::selectRowInternal (i32k row,
                                 b8 dontScroll,
                                 b8 deselectOthersFirst,
                                 b8 isMouseClick)
{
    checkModelPtrIsValid();

    if (! multipleSelection)
        deselectOthersFirst = true;

    if ((! isRowSelected (row))
         || (deselectOthersFirst && getNumSelectedRows() > 1))
    {
        if (isPositiveAndBelow (row, totalItems))
        {
            if (deselectOthersFirst)
                selected.clear();

            selected.addRange ({ row, row + 1 });

            if (getHeight() == 0 || getWidth() == 0)
                dontScroll = true;

            viewport->selectRow (row, getRowHeight(), dontScroll,
                                 lastRowSelected, totalItems, isMouseClick);

            lastRowSelected = row;
            model->selectedRowsChanged (row);

            if (auto* handler = getAccessibilityHandler())
                handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
        }
        else
        {
            if (deselectOthersFirst)
                deselectAllRows();
        }
    }
}

z0 ListBox::deselectRow (i32k row)
{
    checkModelPtrIsValid();

    if (selected.contains (row))
    {
        selected.removeRange ({ row, row + 1 });

        if (row == lastRowSelected)
            lastRowSelected = getSelectedRow (0);

        viewport->updateContents();
        model->selectedRowsChanged (lastRowSelected);

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
    }
}

z0 ListBox::setSelectedRows (const SparseSet<i32>& setOfRowsToBeSelected,
                               const NotificationType sendNotificationEventToModel)
{
    checkModelPtrIsValid();

    selected = setOfRowsToBeSelected;
    selected.removeRange ({ totalItems, std::numeric_limits<i32>::max() });

    if (! isRowSelected (lastRowSelected))
        lastRowSelected = getSelectedRow (0);

    viewport->updateContents();

    if (model != nullptr && sendNotificationEventToModel == sendNotification)
        model->selectedRowsChanged (lastRowSelected);

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
}

SparseSet<i32> ListBox::getSelectedRows() const
{
    return selected;
}

z0 ListBox::selectRangeOfRows (i32 firstRow, i32 lastRow, b8 dontScrollToShowThisRange)
{
    if (multipleSelection && (firstRow != lastRow))
    {
        i32k numRows = totalItems - 1;
        firstRow = jlimit (0, jmax (0, numRows), firstRow);
        lastRow  = jlimit (0, jmax (0, numRows), lastRow);

        selected.addRange ({ jmin (firstRow, lastRow),
                             jmax (firstRow, lastRow) + 1 });

        selected.removeRange ({ lastRow, lastRow + 1 });
    }

    selectRowInternal (lastRow, dontScrollToShowThisRange, false, true);
}

z0 ListBox::flipRowSelection (i32k row)
{
    if (isRowSelected (row))
        deselectRow (row);
    else
        selectRowInternal (row, false, false, true);
}

z0 ListBox::deselectAllRows()
{
    checkModelPtrIsValid();

    if (! selected.isEmpty())
    {
        selected.clear();
        lastRowSelected = -1;

        viewport->updateContents();

        if (model != nullptr)
            model->selectedRowsChanged (lastRowSelected);

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
    }
}

z0 ListBox::selectRowsBasedOnModifierKeys (i32k row,
                                             ModifierKeys mods,
                                             const b8 isMouseUpEvent)
{
    if (multipleSelection && (mods.isCommandDown() || alwaysFlipSelection))
    {
        flipRowSelection (row);
    }
    else if (multipleSelection && mods.isShiftDown() && lastRowSelected >= 0)
    {
        selectRangeOfRows (lastRowSelected, row);
    }
    else if ((! mods.isPopupMenu()) || ! isRowSelected (row))
    {
        selectRowInternal (row, false, ! (multipleSelection && (! isMouseUpEvent) && isRowSelected (row)), true);
    }
}

i32 ListBox::getNumSelectedRows() const
{
    return selected.size();
}

i32 ListBox::getSelectedRow (i32k index) const
{
    return (isPositiveAndBelow (index, selected.size()))
                ? selected [index] : -1;
}

b8 ListBox::isRowSelected (i32k row) const
{
    return selected.contains (row);
}

i32 ListBox::getLastRowSelected() const
{
    return isRowSelected (lastRowSelected) ? lastRowSelected : -1;
}

//==============================================================================
i32 ListBox::getRowContainingPosition (i32k x, i32k y) const noexcept
{
    if (isPositiveAndBelow (x, getWidth()))
    {
        i32k row = (viewport->getViewPositionY() + y - viewport->getY()) / rowHeight;

        if (isPositiveAndBelow (row, totalItems))
            return row;
    }

    return -1;
}

i32 ListBox::getInsertionIndexForPosition (i32k x, i32k y) const noexcept
{
    if (isPositiveAndBelow (x, getWidth()))
        return jlimit (0, totalItems, (viewport->getViewPositionY() + y + rowHeight / 2 - viewport->getY()) / rowHeight);

    return -1;
}

Component* ListBox::getComponentForRowNumber (i32k row) const noexcept
{
    if (auto* listRowComp = viewport->getComponentForRowIfOnscreen (row))
        return listRowComp->getCustomComponent();

    return nullptr;
}

i32 ListBox::getRowNumberOfComponent (const Component* const rowComponent) const noexcept
{
    return viewport->getRowNumberOfComponent (rowComponent);
}

Rectangle<i32> ListBox::getRowPosition (i32 rowNumber, b8 relativeToComponentTopLeft) const noexcept
{
    auto y = viewport->getY() + rowHeight * rowNumber;

    if (relativeToComponentTopLeft)
        y -= viewport->getViewPositionY();

    return { viewport->getX(), y,
             viewport->getViewedComponent()->getWidth(), rowHeight };
}

z0 ListBox::setVerticalPosition (const f64 proportion)
{
    auto offscreen = viewport->getViewedComponent()->getHeight() - viewport->getHeight();

    viewport->setViewPosition (viewport->getViewPositionX(),
                               jmax (0, roundToInt (proportion * offscreen)));
}

f64 ListBox::getVerticalPosition() const
{
    auto offscreen = viewport->getViewedComponent()->getHeight() - viewport->getHeight();

    return offscreen > 0 ? viewport->getViewPositionY() / (f64) offscreen
                         : 0;
}

i32 ListBox::getVisibleRowWidth() const noexcept
{
    return viewport->getViewWidth();
}

z0 ListBox::scrollToEnsureRowIsOnscreen (i32k row)
{
    viewport->scrollToEnsureRowIsOnscreen (row, getRowHeight());
}

//==============================================================================
b8 ListBox::keyPressed (const KeyPress& key)
{
    checkModelPtrIsValid();

    i32k numVisibleRows = viewport->getHeight() / getRowHeight();

    const b8 multiple = multipleSelection
                            && lastRowSelected >= 0
                            && key.getModifiers().isShiftDown();

    if (key.isKeyCode (KeyPress::upKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected - 1);
        else
            selectRow (jmax (0, lastRowSelected - 1));
    }
    else if (key.isKeyCode (KeyPress::downKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected + 1);
        else
            selectRow (jmin (totalItems - 1, jmax (0, lastRowSelected + 1)));
    }
    else if (key.isKeyCode (KeyPress::pageUpKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected - numVisibleRows);
        else
            selectRow (jmax (0, jmax (0, lastRowSelected) - numVisibleRows));
    }
    else if (key.isKeyCode (KeyPress::pageDownKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected + numVisibleRows);
        else
            selectRow (jmin (totalItems - 1, jmax (0, lastRowSelected) + numVisibleRows));
    }
    else if (key.isKeyCode (KeyPress::homeKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, 0);
        else
            selectRow (0);
    }
    else if (key.isKeyCode (KeyPress::endKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, totalItems - 1);
        else
            selectRow (totalItems - 1);
    }
    else if (key.isKeyCode (KeyPress::returnKey) && isRowSelected (lastRowSelected))
    {
        if (model != nullptr)
            model->returnKeyPressed (lastRowSelected);
    }
    else if ((key.isKeyCode (KeyPress::deleteKey) || key.isKeyCode (KeyPress::backspaceKey))
               && isRowSelected (lastRowSelected))
    {
        if (model != nullptr)
            model->deleteKeyPressed (lastRowSelected);
    }
    else if (multipleSelection && key == KeyPress ('a', ModifierKeys::commandModifier, 0))
    {
        selectRangeOfRows (0, std::numeric_limits<i32>::max());
    }
    else
    {
        return false;
    }

    return true;
}

b8 ListBox::keyStateChanged (const b8 isKeyDown)
{
    return isKeyDown
            && (KeyPress::isKeyCurrentlyDown (KeyPress::upKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::pageUpKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::downKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::pageDownKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::homeKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::endKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::returnKey));
}

z0 ListBox::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    b8 eventWasUsed = false;

    if (! approximatelyEqual (wheel.deltaX, 0.0f) && getHorizontalScrollBar().isVisible())
    {
        eventWasUsed = true;
        getHorizontalScrollBar().mouseWheelMove (e, wheel);
    }

    if (! approximatelyEqual (wheel.deltaY, 0.0f) && getVerticalScrollBar().isVisible())
    {
        eventWasUsed = true;
        getVerticalScrollBar().mouseWheelMove (e, wheel);
    }

    if (! eventWasUsed)
        Component::mouseWheelMove (e, wheel);
}

z0 ListBox::mouseUp (const MouseEvent& e)
{
    checkModelPtrIsValid();

    if (e.mouseWasClicked() && model != nullptr)
        model->backgroundClicked (e);
}

//==============================================================================
z0 ListBox::setRowHeight (i32k newHeight)
{
    rowHeight = jmax (1, newHeight);
    viewport->setSingleStepSizes (20, rowHeight);
    updateContent();
}

i32 ListBox::getNumRowsOnScreen() const noexcept
{
    return viewport->getMaximumVisibleHeight() / rowHeight;
}

z0 ListBox::setMinimumContentWidth (i32k newMinimumWidth)
{
    minimumRowWidth = newMinimumWidth;
    updateContent();
}

i32 ListBox::getVisibleContentWidth() const noexcept            { return viewport->getMaximumVisibleWidth(); }

ScrollBar& ListBox::getVerticalScrollBar() const noexcept       { return viewport->getVerticalScrollBar(); }
ScrollBar& ListBox::getHorizontalScrollBar() const noexcept     { return viewport->getHorizontalScrollBar(); }

z0 ListBox::colourChanged()
{
    setOpaque (findColor (backgroundColorId).isOpaque());
    viewport->setOpaque (isOpaque());
    repaint();
}

z0 ListBox::parentHierarchyChanged()
{
    colourChanged();
}

z0 ListBox::setOutlineThickness (i32 newThickness)
{
    outlineThickness = newThickness;
    resized();
}

z0 ListBox::setHeaderComponent (std::unique_ptr<Component> newHeaderComponent)
{
    headerComponent = std::move (newHeaderComponent);
    addAndMakeVisible (headerComponent.get());
    ListBox::resized();
    invalidateAccessibilityHandler();
}

b8 ListBox::hasAccessibleHeaderComponent() const
{
    return headerComponent != nullptr
            && headerComponent->getAccessibilityHandler() != nullptr;
}

z0 ListBox::repaintRow (i32k rowNumber) noexcept
{
    repaint (getRowPosition (rowNumber, true));
}

ScaledImage ListBox::createSnapshotOfRows (const SparseSet<i32>& rows, i32& imageX, i32& imageY)
{
    Rectangle<i32> imageArea;
    auto firstRow = getRowContainingPosition (0, viewport->getY());

    for (i32 i = getNumRowsOnScreen() + 2; --i >= 0;)
    {
        if (rows.contains (firstRow + i))
        {
            if (auto* rowComp = viewport->getComponentForRowIfOnscreen (firstRow + i))
            {
                auto pos = getLocalPoint (rowComp, Point<i32>());

                imageArea = imageArea.getUnion ({ pos.x, pos.y, rowComp->getWidth(), rowComp->getHeight() });
            }
        }
    }

    imageArea = imageArea.getIntersection (getLocalBounds());
    imageX = imageArea.getX();
    imageY = imageArea.getY();

    const auto additionalScale = 2.0f;
    const auto listScale = Component::getApproximateScaleFactorForComponent (this) * additionalScale;
    Image snapshot (Image::ARGB,
                    roundToInt ((f32) imageArea.getWidth() * listScale),
                    roundToInt ((f32) imageArea.getHeight() * listScale),
                    true);

    for (i32 i = getNumRowsOnScreen() + 2; --i >= 0;)
    {
        if (rows.contains (firstRow + i))
        {
            if (auto* rowComp = viewport->getComponentForRowIfOnscreen (firstRow + i))
            {
                Graphics g (snapshot);
                g.setOrigin ((getLocalPoint (rowComp, Point<i32>()) - imageArea.getPosition()) * additionalScale);

                const auto rowScale = Component::getApproximateScaleFactorForComponent (rowComp) * additionalScale;

                if (g.reduceClipRegion (rowComp->getLocalBounds() * rowScale))
                {
                    g.beginTransparencyLayer (0.6f);
                    g.addTransform (AffineTransform::scale (rowScale));
                    rowComp->paintEntireComponent (g, false);
                    g.endTransparencyLayer();
                }
            }
        }
    }

    return { snapshot, additionalScale };
}

z0 ListBox::startDragAndDrop (const MouseEvent& e, const SparseSet<i32>& rowsToDrag, const var& dragDescription, b8 allowDraggingToOtherWindows)
{
    if (auto* dragContainer = DragAndDropContainer::findParentDragContainerFor (this))
    {
        i32 x, y;
        auto dragImage = createSnapshotOfRows (rowsToDrag, x, y);

        auto p = Point<i32> (x, y) - e.getEventRelativeTo (this).position.toInt();
        dragContainer->startDragging (dragDescription, this, dragImage, allowDraggingToOtherWindows, &p, &e.source);
    }
    else
    {
        // to be able to do a drag-and-drop operation, the listbox needs to
        // be inside a component which is also a DragAndDropContainer.
        jassertfalse;
    }
}

std::unique_ptr<AccessibilityHandler> ListBox::createAccessibilityHandler()
{
    class TableInterface final : public AccessibilityTableInterface
    {
    public:
        explicit TableInterface (ListBox& listBoxToWrap)
            : listBox (listBoxToWrap)
        {
        }

        i32 getNumRows() const override
        {
            listBox.checkModelPtrIsValid();

            return listBox.model != nullptr ? listBox.model->getNumRows()
                                            : 0;
        }

        i32 getNumColumns() const override
        {
            return 1;
        }

        const AccessibilityHandler* getHeaderHandler() const override
        {
            if (listBox.hasAccessibleHeaderComponent())
                return listBox.headerComponent->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getRowHandler (i32 row) const override
        {
            if (auto* rowComponent = listBox.viewport->getComponentForRowIfOnscreen (row))
                return rowComponent->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getCellHandler (i32, i32) const override
        {
            return nullptr;
        }

        Optional<Span> getRowSpan (const AccessibilityHandler& handler) const override
        {
            const auto rowNumber = listBox.getRowNumberOfComponent (&handler.getComponent());

            return rowNumber != -1 ? makeOptional (Span { rowNumber, 1 })
                                   : nullopt;
        }

        Optional<Span> getColumnSpan (const AccessibilityHandler&) const override
        {
            return Span { 0, 1 };
        }

        z0 showCell (const AccessibilityHandler& h) const override
        {
            if (const auto row = getRowSpan (h))
                listBox.scrollToEnsureRowIsOnscreen (row->begin);
        }

    private:
        ListBox& listBox;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableInterface)
    };

    return std::make_unique<AccessibilityHandler> (*this,
                                                   AccessibilityRole::list,
                                                   AccessibilityActions{},
                                                   AccessibilityHandler::Interfaces { std::make_unique<TableInterface> (*this) });
}

//==============================================================================
Component* ListBoxModel::refreshComponentForRow (i32, b8, [[maybe_unused]] Component* existingComponentToUpdate)
{
    jassert (existingComponentToUpdate == nullptr); // indicates a failure in the code that recycles the components
    return nullptr;
}

Txt ListBoxModel::getNameForRow (i32 rowNumber)                      { return "Row " + Txt (rowNumber + 1); }
z0 ListBoxModel::listBoxItemClicked (i32, const MouseEvent&) {}
z0 ListBoxModel::listBoxItemDoubleClicked (i32, const MouseEvent&) {}
z0 ListBoxModel::backgroundClicked (const MouseEvent&) {}
z0 ListBoxModel::selectedRowsChanged (i32) {}
z0 ListBoxModel::deleteKeyPressed (i32) {}
z0 ListBoxModel::returnKeyPressed (i32) {}
z0 ListBoxModel::listWasScrolled() {}
var ListBoxModel::getDragSourceDescription (const SparseSet<i32>&)      { return {}; }
Txt ListBoxModel::getTooltipForRow (i32)                             { return {}; }
MouseCursor ListBoxModel::getMouseCursorForRow (i32)                    { return MouseCursor::NormalCursor; }

} // namespace drx
