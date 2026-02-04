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

static const Identifier tableColumnProperty { "_tableColumnId" };
static const Identifier tableAccessiblePlaceholderProperty { "_accessiblePlaceholder" };

class TableListBox::RowComp final : public TooltipClient,
                                    public ComponentWithListRowMouseBehaviours<RowComp>
{
public:
    explicit RowComp (TableListBox& tlb)
        : owner (tlb)
    {
        setFocusContainerType (FocusContainerType::focusContainer);
    }

    z0 paint (Graphics& g) override
    {
        if (auto* tableModel = owner.getTableListBoxModel())
        {
            tableModel->paintRowBackground (g, getRow(), getWidth(), getHeight(), isSelected());

            auto& headerComp = owner.getHeader();
            const auto numColumns = jmin ((i32) columnComponents.size(), headerComp.getNumColumns (true));
            const auto clipBounds = g.getClipBounds();

            for (i32 i = 0; i < numColumns; ++i)
            {
                if (columnComponents[(size_t) i]->getProperties().contains (tableAccessiblePlaceholderProperty))
                {
                    auto columnRect = headerComp.getColumnPosition (i).withHeight (getHeight());

                    if (columnRect.getX() >= clipBounds.getRight())
                        break;

                    if (columnRect.getRight() > clipBounds.getX())
                    {
                        Graphics::ScopedSaveState ss (g);

                        if (g.reduceClipRegion (columnRect))
                        {
                            g.setOrigin (columnRect.getX(), 0);
                            tableModel->paintCell (g, getRow(), headerComp.getColumnIdOfIndex (i, true),
                                                   columnRect.getWidth(), columnRect.getHeight(), isSelected());
                        }
                    }
                }
            }
        }
    }

    z0 update (i32 newRow, b8 isNowSelected)
    {
        jassert (newRow >= 0);

        updateRowAndSelection (newRow, isNowSelected);

        auto* tableModel = owner.getTableListBoxModel();

        if (tableModel != nullptr && getRow() < owner.getNumRows())
        {
            const ComponentDeleter deleter { columnForComponent };
            const auto numColumns = owner.getHeader().getNumColumns (true);

            while (numColumns < (i32) columnComponents.size())
                columnComponents.pop_back();

            while ((i32) columnComponents.size() < numColumns)
                columnComponents.emplace_back (nullptr, deleter);

            for (i32 i = 0; i < numColumns; ++i)
            {
                auto columnId = owner.getHeader().getColumnIdOfIndex (i, true);
                auto originalComp = std::move (columnComponents[(size_t) i]);
                auto oldCustomComp = originalComp != nullptr && ! originalComp->getProperties().contains (tableAccessiblePlaceholderProperty)
                                   ? std::move (originalComp)
                                   : std::unique_ptr<Component, ComponentDeleter> { nullptr, deleter };
                auto compToRefresh = oldCustomComp != nullptr && columnId == static_cast<i32> (oldCustomComp->getProperties()[tableColumnProperty])
                                   ? std::move (oldCustomComp)
                                   : std::unique_ptr<Component, ComponentDeleter> { nullptr, deleter };

                columnForComponent.erase (compToRefresh.get());
                std::unique_ptr<Component, ComponentDeleter> newCustomComp { tableModel->refreshComponentForCell (getRow(),
                                                                                                                  columnId,
                                                                                                                  isSelected(),
                                                                                                                  compToRefresh.release()),
                                                                             deleter };

                auto columnComp = [&]
                {
                    // We got a result from refreshComponentForCell, so use that
                    if (newCustomComp != nullptr)
                        return std::move (newCustomComp);

                    // There was already a placeholder component for this column
                    if (originalComp != nullptr)
                        return std::move (originalComp);

                    // Create a new placeholder component to use
                    std::unique_ptr<Component, ComponentDeleter> comp { new Component, deleter };
                    comp->setInterceptsMouseClicks (false, false);
                    comp->getProperties().set (tableAccessiblePlaceholderProperty, true);
                    return comp;
                }();

                columnForComponent.emplace (columnComp.get(), i);

                // In order for navigation to work correctly on macOS, the number of child
                // accessibility elements on each row must match the number of header accessibility
                // elements.
                columnComp->setFocusContainerType (FocusContainerType::focusContainer);
                columnComp->getProperties().set (tableColumnProperty, columnId);
                addAndMakeVisible (*columnComp);

                columnComponents[(size_t) i] = std::move (columnComp);
                resizeCustomComp (i);
            }
        }
        else
        {
            columnComponents.clear();
        }
    }

    z0 resized() override
    {
        for (auto i = (i32) columnComponents.size(); --i >= 0;)
            resizeCustomComp (i);
    }

    z0 resizeCustomComp (i32 index)
    {
        if (auto& c = columnComponents[(size_t) index])
        {
            c->setBounds (owner.getHeader()
                               .getColumnPosition (index)
                               .withY (0)
                               .withHeight (getHeight()));
        }
    }

    z0 performSelection (const MouseEvent& e, b8 isMouseUp)
    {
        owner.selectRowsBasedOnModifierKeys (getRow(), e.mods, isMouseUp);

        auto columnId = owner.getHeader().getColumnIdAtX (e.x);

        if (columnId != 0)
            if (auto* m = owner.getTableListBoxModel())
                m->cellClicked (getRow(), columnId, e);
    }

    z0 mouseDoubleClick (const MouseEvent& e) override
    {
        if (! isEnabled())
            return;

        const auto columnId = owner.getHeader().getColumnIdAtX (e.x);

        if (columnId != 0)
            if (auto* m = owner.getTableListBoxModel())
                m->cellDoubleClicked (getRow(), columnId, e);
    }

    Txt getTooltip() override
    {
        auto columnId = owner.getHeader().getColumnIdAtX (getMouseXYRelative().getX());

        if (columnId != 0)
            if (auto* m = owner.getTableListBoxModel())
                return m->getCellTooltip (getRow(), columnId);

        return {};
    }

    Component* findChildComponentForColumn (i32 columnId) const
    {
        const auto index = (size_t) owner.getHeader().getIndexOfColumnId (columnId, true);

        if (isPositiveAndBelow (index, columnComponents.size()))
            return columnComponents[index].get();

        return nullptr;
    }

    i32 getColumnNumberOfComponent (const Component* comp) const
    {
        const auto iter = columnForComponent.find (comp);
        return iter != columnForComponent.cend() ? iter->second : -1;
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<RowAccessibilityHandler> (*this);
    }

    TableListBox& getOwner() const { return owner; }

private:
    //==============================================================================
    class RowAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        RowAccessibilityHandler (RowComp& rowComp)
            : AccessibilityHandler (rowComp,
                                    AccessibilityRole::row,
                                    getListRowAccessibilityActions (rowComp),
                                    { std::make_unique<RowComponentCellInterface> (*this) }),
              rowComponent (rowComp)
        {
        }

        Txt getTitle() const override
        {
            if (auto* m = rowComponent.owner.ListBox::model)
                return m->getNameForRow (rowComponent.getRow());

            return {};
        }

        Txt getHelp() const override  { return rowComponent.getTooltip(); }

        AccessibleState getCurrentState() const override
        {
            if (auto* m = rowComponent.owner.getTableListBoxModel())
                if (rowComponent.getRow() >= m->getNumRows())
                    return AccessibleState().withIgnored();

            auto state = AccessibilityHandler::getCurrentState();

            if (rowComponent.owner.multipleSelection)
                state = state.withMultiSelectable();
            else
                state = state.withSelectable();

            if (rowComponent.isSelected())
                return state.withSelected();

            return state;
        }

    private:
        class RowComponentCellInterface final : public AccessibilityCellInterface
        {
        public:
            RowComponentCellInterface (RowAccessibilityHandler& handler)
                : owner (handler)
            {
            }

            i32 getDisclosureLevel() const override  { return 0; }

            const AccessibilityHandler* getTableHandler() const override  { return owner.rowComponent.owner.getAccessibilityHandler(); }

        private:
            RowAccessibilityHandler& owner;
        };

    private:
        RowComp& rowComponent;
    };

    //==============================================================================
    class ComponentDeleter
    {
    public:
        explicit ComponentDeleter (std::map<const Component*, i32>& locations)
            : columnForComponent (&locations) {}

        z0 operator() (Component* comp) const
        {
            columnForComponent->erase (comp);

            if (comp != nullptr)
                delete comp;
        }

    private:
        std::map<const Component*, i32>* columnForComponent;
    };

    TableListBox& owner;
    std::map<const Component*, i32> columnForComponent;
    std::vector<std::unique_ptr<Component, ComponentDeleter>> columnComponents;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowComp)
};


//==============================================================================
class TableListBox::Header final : public TableHeaderComponent
{
public:
    Header (TableListBox& tlb)  : owner (tlb) {}

    z0 addMenuItems (PopupMenu& menu, i32 columnIdClicked) override
    {
        if (owner.isAutoSizeMenuOptionShown())
        {
            menu.addItem (autoSizeColumnId, TRANS ("Auto-size this column"), columnIdClicked != 0);
            menu.addItem (autoSizeAllId, TRANS ("Auto-size all columns"), owner.getHeader().getNumColumns (true) > 0);
            menu.addSeparator();
        }

        TableHeaderComponent::addMenuItems (menu, columnIdClicked);
    }

    z0 reactToMenuItem (i32 menuReturnId, i32 columnIdClicked) override
    {
        switch (menuReturnId)
        {
            case autoSizeColumnId:      owner.autoSizeColumn (columnIdClicked); break;
            case autoSizeAllId:         owner.autoSizeAllColumns(); break;
            default:                    TableHeaderComponent::reactToMenuItem (menuReturnId, columnIdClicked); break;
        }
    }

private:
    TableListBox& owner;

    enum { autoSizeColumnId = 0xf836743, autoSizeAllId = 0xf836744 };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Header)
};

//==============================================================================
TableListBox::TableListBox (const Txt& name, TableListBoxModel* const m)
    : ListBox (name, nullptr), model (m)
{
    ListBox::assignModelPtr (this);

    setHeader (std::make_unique<Header> (*this));
}

TableListBox::~TableListBox()
{
}

z0 TableListBox::setModel (TableListBoxModel* newModel)
{
    if (model != newModel)
    {
        model = newModel;
        updateContent();
    }
}

z0 TableListBox::setHeader (std::unique_ptr<TableHeaderComponent> newHeader)
{
    if (newHeader == nullptr)
    {
        jassertfalse; // you need to supply a real header for a table!
        return;
    }

    Rectangle<i32> newBounds (100, 28);

    if (header != nullptr)
        newBounds = header->getBounds();

    header = newHeader.get();
    header->setBounds (newBounds);

    setHeaderComponent (std::move (newHeader));

    header->addListener (this);
}

i32 TableListBox::getHeaderHeight() const noexcept
{
    return header->getHeight();
}

z0 TableListBox::setHeaderHeight (i32 newHeight)
{
    header->setSize (header->getWidth(), newHeight);
    resized();
}

z0 TableListBox::autoSizeColumn (i32 columnId)
{
    auto width = model != nullptr ? model->getColumnAutoSizeWidth (columnId) : 0;

    if (width > 0)
        header->setColumnWidth (columnId, width);
}

z0 TableListBox::autoSizeAllColumns()
{
    for (i32 i = 0; i < header->getNumColumns (true); ++i)
        autoSizeColumn (header->getColumnIdOfIndex (i, true));
}

z0 TableListBox::setAutoSizeMenuOptionShown (b8 shouldBeShown) noexcept
{
    autoSizeOptionsShown = shouldBeShown;
}

Rectangle<i32> TableListBox::getCellPosition (i32 columnId, i32 rowNumber, b8 relativeToComponentTopLeft) const
{
    auto headerCell = header->getColumnPosition (header->getIndexOfColumnId (columnId, true));

    if (relativeToComponentTopLeft)
        headerCell.translate (header->getX(), 0);

    return getRowPosition (rowNumber, relativeToComponentTopLeft)
            .withX (headerCell.getX())
            .withWidth (headerCell.getWidth());
}

Component* TableListBox::getCellComponent (i32 columnId, i32 rowNumber) const
{
    if (auto* rowComp = dynamic_cast<RowComp*> (getComponentForRowNumber (rowNumber)))
        return rowComp->findChildComponentForColumn (columnId);

    return nullptr;
}

z0 TableListBox::scrollToEnsureColumnIsOnscreen (i32 columnId)
{
    auto& scrollbar = getHorizontalScrollBar();
    auto pos = header->getColumnPosition (header->getIndexOfColumnId (columnId, true));

    auto x = scrollbar.getCurrentRangeStart();
    auto w = scrollbar.getCurrentRangeSize();

    if (pos.getX() < x)
        x = pos.getX();
    else if (pos.getRight() > x + w)
        x += jmax (0.0, pos.getRight() - (x + w));

    scrollbar.setCurrentRangeStart (x);
}

i32 TableListBox::getNumRows()
{
    return model != nullptr ? model->getNumRows() : 0;
}

z0 TableListBox::paintListBoxItem (i32, Graphics&, i32, i32, b8)
{
}

Component* TableListBox::refreshComponentForRow (i32 rowNumber, b8 rowSelected, Component* existingComponentToUpdate)
{
    if (existingComponentToUpdate == nullptr)
        existingComponentToUpdate = new RowComp (*this);

    static_cast<RowComp*> (existingComponentToUpdate)->update (rowNumber, rowSelected);

    return existingComponentToUpdate;
}

z0 TableListBox::selectedRowsChanged (i32 row)
{
    if (model != nullptr)
        model->selectedRowsChanged (row);
}

z0 TableListBox::deleteKeyPressed (i32 row)
{
    if (model != nullptr)
        model->deleteKeyPressed (row);
}

z0 TableListBox::returnKeyPressed (i32 row)
{
    if (model != nullptr)
        model->returnKeyPressed (row);
}

z0 TableListBox::backgroundClicked (const MouseEvent& e)
{
    if (model != nullptr)
        model->backgroundClicked (e);
}

z0 TableListBox::listWasScrolled()
{
    if (model != nullptr)
        model->listWasScrolled();
}

z0 TableListBox::tableColumnsChanged (TableHeaderComponent*)
{
    setMinimumContentWidth (header->getTotalWidth());
    repaint();
    updateColumnComponents();
}

z0 TableListBox::tableColumnsResized (TableHeaderComponent*)
{
    setMinimumContentWidth (header->getTotalWidth());
    repaint();
    updateColumnComponents();
}

z0 TableListBox::tableSortOrderChanged (TableHeaderComponent*)
{
    if (model != nullptr)
        model->sortOrderChanged (header->getSortColumnId(),
                                 header->isSortedForwards());
}

z0 TableListBox::tableColumnDraggingChanged (TableHeaderComponent*, i32 columnIdNowBeingDragged_)
{
    columnIdNowBeingDragged = columnIdNowBeingDragged_;
    repaint();
}

z0 TableListBox::resized()
{
    ListBox::resized();

    header->resizeAllColumnsToFit (getVisibleContentWidth());
    setMinimumContentWidth (header->getTotalWidth());
}

z0 TableListBox::updateColumnComponents() const
{
    auto firstRow = getRowContainingPosition (0, 0);

    for (i32 i = firstRow + getNumRowsOnScreen() + 2; --i >= firstRow;)
        if (auto* rowComp = dynamic_cast<RowComp*> (getComponentForRowNumber (i)))
            rowComp->resized();
}

template <typename FindIndex>
Optional<AccessibilityTableInterface::Span> findRecursively (const AccessibilityHandler& handler,
                                                             Component* outermost,
                                                             FindIndex&& findIndexOfComponent)
{
    for (auto* comp = &handler.getComponent(); comp != outermost; comp = comp->getParentComponent())
    {
        const auto result = findIndexOfComponent (comp);

        if (result != -1)
            return AccessibilityTableInterface::Span { result, 1 };
    }

    return nullopt;
}

std::unique_ptr<AccessibilityHandler> TableListBox::createAccessibilityHandler()
{
    class TableInterface final : public AccessibilityTableInterface
    {
    public:
        explicit TableInterface (TableListBox& tableListBoxToWrap)
            : tableListBox (tableListBoxToWrap)
        {
        }

        i32 getNumRows() const override
        {
            if (auto* tableModel = tableListBox.getTableListBoxModel())
                return tableModel->getNumRows();

            return 0;
        }

        i32 getNumColumns() const override
        {
            return tableListBox.getHeader().getNumColumns (true);
        }

        const AccessibilityHandler* getRowHandler (i32 row) const override
        {
            if (isPositiveAndBelow (row, getNumRows()))
                if (auto* rowComp = tableListBox.getComponentForRowNumber (row))
                    return rowComp->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getCellHandler (i32 row, i32 column) const override
        {
            if (isPositiveAndBelow (row, getNumRows()) && isPositiveAndBelow (column, getNumColumns()))
                if (auto* cellComponent = tableListBox.getCellComponent (tableListBox.getHeader().getColumnIdOfIndex (column, true), row))
                    return cellComponent->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getHeaderHandler() const override
        {
            if (tableListBox.hasAccessibleHeaderComponent())
                return tableListBox.headerComponent->getAccessibilityHandler();

            return nullptr;
        }

        Optional<Span> getRowSpan (const AccessibilityHandler& handler) const override
        {
            if (tableListBox.isParentOf (&handler.getComponent()))
                return findRecursively (handler, &tableListBox, [&] (auto* c) { return tableListBox.getRowNumberOfComponent (c); });

            return nullopt;
        }

        Optional<Span> getColumnSpan (const AccessibilityHandler& handler) const override
        {
            if (const auto rowSpan = getRowSpan (handler))
                if (auto* rowComponent = dynamic_cast<RowComp*> (tableListBox.getComponentForRowNumber (rowSpan->begin)))
                    return findRecursively (handler, &tableListBox, [&] (auto* c) { return rowComponent->getColumnNumberOfComponent (c); });

            return nullopt;
        }

        z0 showCell (const AccessibilityHandler& handler) const override
        {
            const auto row = getRowSpan (handler);
            const auto col = getColumnSpan (handler);

            if (row.hasValue() && col.hasValue())
            {
                tableListBox.scrollToEnsureRowIsOnscreen (row->begin);
                tableListBox.scrollToEnsureColumnIsOnscreen (col->begin);
            }
        }

    private:
        TableListBox& tableListBox;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableInterface)
    };

    return std::make_unique<AccessibilityHandler> (*this,
                                                   AccessibilityRole::table,
                                                   AccessibilityActions{},
                                                   AccessibilityHandler::Interfaces { std::make_unique<TableInterface> (*this) });
}

//==============================================================================
z0 TableListBoxModel::cellClicked (i32, i32, const MouseEvent&)       {}
z0 TableListBoxModel::cellDoubleClicked (i32, i32, const MouseEvent&) {}
z0 TableListBoxModel::backgroundClicked (const MouseEvent&)           {}
z0 TableListBoxModel::sortOrderChanged (i32, b8)                    {}
i32 TableListBoxModel::getColumnAutoSizeWidth (i32)                     { return 0; }
z0 TableListBoxModel::selectedRowsChanged (i32)                       {}
z0 TableListBoxModel::deleteKeyPressed (i32)                          {}
z0 TableListBoxModel::returnKeyPressed (i32)                          {}
z0 TableListBoxModel::listWasScrolled()                               {}

Txt TableListBoxModel::getCellTooltip (i32 /*rowNumber*/, i32 /*columnId*/)    { return {}; }
var TableListBoxModel::getDragSourceDescription (const SparseSet<i32>&)           { return {}; }

Component* TableListBoxModel::refreshComponentForCell (i32, i32, b8, [[maybe_unused]] Component* existingComponentToUpdate)
{
    jassert (existingComponentToUpdate == nullptr); // indicates a failure in the code that recycles the components
    return nullptr;
}

} // namespace drx
