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

class TableHeaderComponent::DragOverlayComp final : public Component
{
public:
    DragOverlayComp (const Image& i) : image (i)
    {
        image.duplicateIfShared();
        image.multiplyAllAlphas (0.8f);
        setAlwaysOnTop (true);
    }

    z0 paint (Graphics& g) override
    {
        g.drawImage (image, getLocalBounds().toFloat());
    }

    Image image;

    DRX_DECLARE_NON_COPYABLE (DragOverlayComp)
};


//==============================================================================
TableHeaderComponent::TableHeaderComponent()
{
    setFocusContainerType (FocusContainerType::focusContainer);
}

TableHeaderComponent::~TableHeaderComponent()
{
    dragOverlayComp.reset();
}

//==============================================================================
z0 TableHeaderComponent::setPopupMenuActive (b8 hasMenu)
{
    menuActive = hasMenu;
}

b8 TableHeaderComponent::isPopupMenuActive() const    { return menuActive; }


//==============================================================================
i32 TableHeaderComponent::getNumColumns (const b8 onlyCountVisibleColumns) const
{
    if (onlyCountVisibleColumns)
    {
        i32 num = 0;

        for (auto* c : columns)
            if (c->isVisible())
                ++num;

        return num;
    }

    return columns.size();
}

Txt TableHeaderComponent::getColumnName (i32k columnId) const
{
    if (auto* ci = getInfoForId (columnId))
        return ci->getTitle();

    return {};
}

z0 TableHeaderComponent::setColumnName (i32k columnId, const Txt& newName)
{
    if (auto* ci = getInfoForId (columnId))
    {
        if (ci->getTitle() != newName)
        {
            ci->setTitle (newName);
            sendColumnsChanged();
        }
    }
}

z0 TableHeaderComponent::addColumn (const Txt& columnName,
                                      i32 columnId,
                                      i32 width,
                                      i32 minimumWidth,
                                      i32 maximumWidth,
                                      i32 propertyFlags,
                                      i32 insertIndex)
{
    // can't have a duplicate or zero ID!
    jassert (columnId != 0 && getIndexOfColumnId (columnId, false) < 0);
    jassert (width > 0);

    auto ci = new ColumnInfo();
    ci->setTitle (columnName);
    ci->id = columnId;
    ci->width = width;
    ci->lastDeliberateWidth = width;
    ci->minimumWidth = minimumWidth;
    ci->maximumWidth = maximumWidth >= 0 ? maximumWidth : std::numeric_limits<i32>::max();
    jassert (ci->maximumWidth >= ci->minimumWidth);
    ci->propertyFlags = propertyFlags;

    auto* added = columns.insert (insertIndex, ci);
    addChildComponent (added);
    added->setVisible ((propertyFlags & visible) != 0);

    resized();
    sendColumnsChanged();
}

z0 TableHeaderComponent::removeColumn (i32k columnIdToRemove)
{
    auto index = getIndexOfColumnId (columnIdToRemove, false);

    if (index >= 0)
    {
        columns.remove (index);
        sortChanged = true;
        sendColumnsChanged();
    }
}

z0 TableHeaderComponent::removeAllColumns()
{
    if (columns.size() > 0)
    {
        columns.clear();
        sendColumnsChanged();
    }
}

z0 TableHeaderComponent::moveColumn (i32k columnId, i32 newIndex)
{
    auto currentIndex = getIndexOfColumnId (columnId, false);
    newIndex = visibleIndexToTotalIndex (newIndex);

    if (columns[currentIndex] != nullptr && currentIndex != newIndex)
    {
        columns.move (currentIndex, newIndex);
        sendColumnsChanged();
    }
}

i32 TableHeaderComponent::getColumnWidth (i32k columnId) const
{
    if (auto* ci = getInfoForId (columnId))
        return ci->width;

    return 0;
}

z0 TableHeaderComponent::setColumnWidth (i32k columnId, i32k newWidth)
{
    if (auto* ci = getInfoForId (columnId))
    {
        const auto newWidthToUse = jlimit (ci->minimumWidth, ci->maximumWidth, newWidth);

        if (ci->width != newWidthToUse)
        {
            auto numColumns = getNumColumns (true);

            ci->lastDeliberateWidth = ci->width = newWidthToUse;

            if (stretchToFit)
            {
                auto index = getIndexOfColumnId (columnId, true) + 1;

                if (isPositiveAndBelow (index, numColumns))
                {
                    auto x = getColumnPosition (index).getX();

                    if (lastDeliberateWidth == 0)
                        lastDeliberateWidth = getTotalWidth();

                    resizeColumnsToFit (visibleIndexToTotalIndex (index), lastDeliberateWidth - x);
                }
            }

            resized();
            repaint();
            columnsResized = true;
            triggerAsyncUpdate();
        }
    }
}

//==============================================================================
i32 TableHeaderComponent::getIndexOfColumnId (i32k columnId, const b8 onlyCountVisibleColumns) const
{
    i32 n = 0;

    for (auto* c : columns)
    {
        if ((! onlyCountVisibleColumns) || c->isVisible())
        {
            if (c->id == columnId)
                return n;

            ++n;
        }
    }

    return -1;
}

i32 TableHeaderComponent::getColumnIdOfIndex (i32 index, const b8 onlyCountVisibleColumns) const
{
    if (onlyCountVisibleColumns)
        index = visibleIndexToTotalIndex (index);

    if (auto* ci = columns [index])
        return ci->id;

    return 0;
}

Rectangle<i32> TableHeaderComponent::getColumnPosition (i32k index) const
{
    i32 x = 0, width = 0, n = 0;

    for (auto* c : columns)
    {
        x += width;

        if (c->isVisible())
        {
            width = c->width;

            if (n++ == index)
                break;
        }
        else
        {
            width = 0;
        }
    }

    return { x, 0, width, getHeight() };
}

i32 TableHeaderComponent::getColumnIdAtX (i32k xToFind) const
{
    if (xToFind >= 0)
    {
        i32 x = 0;

        for (auto* ci : columns)
        {
            if (ci->isVisible())
            {
                x += ci->width;

                if (xToFind < x)
                    return ci->id;
            }
        }
    }

    return 0;
}

i32 TableHeaderComponent::getTotalWidth() const
{
    i32 w = 0;

    for (auto* c : columns)
        if (c->isVisible())
            w += c->width;

    return w;
}

z0 TableHeaderComponent::setStretchToFitActive (const b8 shouldStretchToFit)
{
    stretchToFit = shouldStretchToFit;
    lastDeliberateWidth = getTotalWidth();
    resized();
}

b8 TableHeaderComponent::isStretchToFitActive() const
{
    return stretchToFit;
}

z0 TableHeaderComponent::resizeAllColumnsToFit (i32 targetTotalWidth)
{
    if (stretchToFit && getWidth() > 0
         && columnIdBeingResized == 0 && columnIdBeingDragged == 0)
    {
        lastDeliberateWidth = targetTotalWidth;
        resizeColumnsToFit (0, targetTotalWidth);
    }
}

z0 TableHeaderComponent::resizeColumnsToFit (i32 firstColumnIndex, i32 targetTotalWidth)
{
    targetTotalWidth = jmax (targetTotalWidth, 0);
    StretchableObjectResizer sor;

    for (i32 i = firstColumnIndex; i < columns.size(); ++i)
    {
        auto* ci = columns.getUnchecked (i);

        if (ci->isVisible())
            sor.addItem (ci->lastDeliberateWidth, ci->minimumWidth, ci->maximumWidth);
    }

    sor.resizeToFit (targetTotalWidth);
    i32 visIndex = 0;

    for (i32 i = firstColumnIndex; i < columns.size(); ++i)
    {
        auto* ci = columns.getUnchecked (i);

        if (ci->isVisible())
        {
            auto newWidth = jlimit (ci->minimumWidth, ci->maximumWidth,
                                    (i32) std::floor (sor.getItemSize (visIndex++)));

            if (newWidth != ci->width)
            {
                ci->width = newWidth;
                resized();
                repaint();
                columnsResized = true;
                triggerAsyncUpdate();
            }
        }
    }
}

z0 TableHeaderComponent::setColumnVisible (i32k columnId, const b8 shouldBeVisible)
{
    if (auto* ci = getInfoForId (columnId))
    {
        if (shouldBeVisible != ci->isVisible())
        {
            ci->setVisible (shouldBeVisible);
            sendColumnsChanged();
            resized();
        }
    }
}

b8 TableHeaderComponent::isColumnVisible (i32k columnId) const
{
    if (auto* ci = getInfoForId (columnId))
        return ci->isVisible();

    return false;
}

//==============================================================================
z0 TableHeaderComponent::setSortColumnId (i32k columnId, const b8 sortForwards)
{
    if (getSortColumnId() != columnId || isSortedForwards() != sortForwards)
    {
        for (auto* c : columns)
            c->propertyFlags &= ~(sortedForwards | sortedBackwards);

        if (auto* ci = getInfoForId (columnId))
            ci->propertyFlags |= (sortForwards ? sortedForwards : sortedBackwards);

        reSortTable();
    }
}

i32 TableHeaderComponent::getSortColumnId() const
{
    for (auto* c : columns)
        if ((c->propertyFlags & (sortedForwards | sortedBackwards)) != 0)
            return c->id;

    return 0;
}

b8 TableHeaderComponent::isSortedForwards() const
{
    for (auto* c : columns)
        if ((c->propertyFlags & (sortedForwards | sortedBackwards)) != 0)
            return (c->propertyFlags & sortedForwards) != 0;

    return true;
}

z0 TableHeaderComponent::reSortTable()
{
    sortChanged = true;
    resized();
    repaint();
    triggerAsyncUpdate();
}

//==============================================================================
Txt TableHeaderComponent::toString() const
{
    Txt s;

    XmlElement doc ("TABLELAYOUT");

    doc.setAttribute ("sortedCol", getSortColumnId());
    doc.setAttribute ("sortForwards", isSortedForwards());

    for (auto* ci : columns)
    {
        auto* e = doc.createNewChildElement ("COLUMN");
        e->setAttribute ("id", ci->id);
        e->setAttribute ("visible", ci->isVisible());
        e->setAttribute ("width", ci->width);
    }

    return doc.toString (XmlElement::TextFormat().singleLine().withoutHeader());
}

z0 TableHeaderComponent::restoreFromString (const Txt& storedVersion)
{
    if (auto storedXML = parseXMLIfTagMatches (storedVersion, "TABLELAYOUT"))
    {
        i32 index = 0;

        for (auto* col : storedXML->getChildIterator())
        {
            auto tabId = col->getIntAttribute ("id");

            if (auto* ci = getInfoForId (tabId))
            {
                columns.move (columns.indexOf (ci), index);
                ci->width = col->getIntAttribute ("width");
                setColumnVisible (tabId, col->getBoolAttribute ("visible"));
            }

            ++index;
        }

        columnsResized = true;
        sendColumnsChanged();

        setSortColumnId (storedXML->getIntAttribute ("sortedCol"),
                         storedXML->getBoolAttribute ("sortForwards", true));
    }
}

//==============================================================================
z0 TableHeaderComponent::addListener (Listener* newListener)
{
    listeners.addIfNotAlreadyThere (newListener);
}

z0 TableHeaderComponent::removeListener (Listener* listenerToRemove)
{
    listeners.removeFirstMatchingValue (listenerToRemove);
}

//==============================================================================
z0 TableHeaderComponent::columnClicked (i32 columnId, const ModifierKeys& mods)
{
    if (auto* ci = getInfoForId (columnId))
        if ((ci->propertyFlags & sortable) != 0 && ! mods.isPopupMenu())
            setSortColumnId (columnId, (ci->propertyFlags & sortedForwards) == 0);
}

z0 TableHeaderComponent::addMenuItems (PopupMenu& menu, i32k /*columnIdClicked*/)
{
    for (auto* ci : columns)
        if ((ci->propertyFlags & appearsOnColumnMenu) != 0)
            menu.addItem (ci->id, ci->getTitle(),
                          (ci->propertyFlags & (sortedForwards | sortedBackwards)) == 0,
                          isColumnVisible (ci->id));
}

z0 TableHeaderComponent::reactToMenuItem (i32k menuReturnId, i32k /*columnIdClicked*/)
{
    if (getIndexOfColumnId (menuReturnId, false) >= 0)
        setColumnVisible (menuReturnId, ! isColumnVisible (menuReturnId));
}

z0 TableHeaderComponent::drawColumnHeader (Graphics& g, LookAndFeel& lf, const ColumnInfo& ci)
{
    // Only paint columns that are visible
    if (! ci.isVisible())
        return;

    // If this column is being dragged, it shouldn't be drawn in the table header
    if (ci.id == columnIdBeingDragged && dragOverlayComp != nullptr && dragOverlayComp->isVisible())
        return;

    // There's no point drawing this column header if no part of it is visible
    if (! g.getClipBounds()
           .getHorizontalRange()
           .intersects (Range<i32>::withStartAndLength (ci.getX(), ci.width)))
        return;

    Graphics::ScopedSaveState ss (g);

    g.setOrigin (ci.getX(), ci.getY());
    g.reduceClipRegion (0, 0, ci.width, ci.getHeight());

    lf.drawTableHeaderColumn (g, *this, ci.getTitle(), ci.id, ci.width, getHeight(),
                              ci.id == columnIdUnderMouse,
                              ci.id == columnIdUnderMouse && isMouseButtonDown(),
                              ci.propertyFlags);
}

z0 TableHeaderComponent::paint (Graphics& g)
{
    auto& lf = getLookAndFeel();

    lf.drawTableHeaderBackground (g, *this);

    for (auto* ci : columns)
        drawColumnHeader (g, lf, *ci);
}

z0 TableHeaderComponent::resized()
{
    i32 x = 0;

    for (auto* ci : columns)
    {
        const auto widthToUse = ci->isVisible() ? ci->width : 0;
        ci->setBounds (x, 0, widthToUse, getHeight());
        x += widthToUse;
    }
}

z0 TableHeaderComponent::mouseMove  (const MouseEvent& e)  { updateColumnUnderMouse (e); }
z0 TableHeaderComponent::mouseEnter (const MouseEvent& e)  { updateColumnUnderMouse (e); }
z0 TableHeaderComponent::mouseExit  (const MouseEvent&)    { setColumnUnderMouse (0); }

z0 TableHeaderComponent::mouseDown (const MouseEvent& e)
{
    resized();
    repaint();
    columnIdBeingResized = 0;
    columnIdBeingDragged = 0;

    if (columnIdUnderMouse != 0)
    {
        draggingColumnOffset = e.x - getColumnPosition (getIndexOfColumnId (columnIdUnderMouse, true)).getX();

        if (e.mods.isPopupMenu())
            columnClicked (columnIdUnderMouse, e.mods);
    }

    if (menuActive && e.mods.isPopupMenu())
        showColumnChooserMenu (columnIdUnderMouse);
}

z0 TableHeaderComponent::mouseDrag (const MouseEvent& e)
{
    if (columnIdBeingResized == 0
         && columnIdBeingDragged == 0
         && e.mouseWasDraggedSinceMouseDown()
         && ! e.mods.isPopupMenu())
    {
        dragOverlayComp.reset();

        columnIdBeingResized = getResizeDraggerAt (e.getMouseDownX());

        if (columnIdBeingResized != 0)
        {
            if (auto* ci = getInfoForId (columnIdBeingResized))
                initialColumnWidth = ci->width;
            else
                jassertfalse;
        }
        else
        {
            beginDrag (e);
        }
    }

    if (columnIdBeingResized != 0)
    {
        if (auto* ci = getInfoForId (columnIdBeingResized))
        {
            auto w = jlimit (ci->minimumWidth, ci->maximumWidth,
                             initialColumnWidth + e.getDistanceFromDragStartX());

            if (stretchToFit)
            {
                // prevent us dragging a column too far right if we're in stretch-to-fit mode
                i32 minWidthOnRight = 0;

                for (i32 i = getIndexOfColumnId (columnIdBeingResized, false) + 1; i < columns.size(); ++i)
                    if (columns.getUnchecked (i)->isVisible())
                        minWidthOnRight += columns.getUnchecked (i)->minimumWidth;

                auto currentPos = getColumnPosition (getIndexOfColumnId (columnIdBeingResized, true));
                w = jmax (ci->minimumWidth, jmin (w, lastDeliberateWidth - minWidthOnRight - currentPos.getX()));
            }

            setColumnWidth (columnIdBeingResized, w);
        }
    }
    else if (columnIdBeingDragged != 0)
    {
        if (e.y >= -50 && e.y < getHeight() + 50)
        {
            if (dragOverlayComp != nullptr)
            {
                dragOverlayComp->setVisible (true);
                dragOverlayComp->setBounds (jlimit (0,
                                                    jmax (0, getTotalWidth() - dragOverlayComp->getWidth()),
                                                    e.x - draggingColumnOffset),
                                            0,
                                            dragOverlayComp->getWidth(),
                                            getHeight());

                for (i32 i = columns.size(); --i >= 0;)
                {
                    i32k currentIndex = getIndexOfColumnId (columnIdBeingDragged, true);
                    i32 newIndex = currentIndex;

                    if (newIndex > 0)
                    {
                        // if the previous column isn't draggable, we can't move our column
                        // past it, because that'd change the undraggable column's position..
                        auto* previous = columns.getUnchecked (newIndex - 1);

                        if ((previous->propertyFlags & draggable) != 0)
                        {
                            auto leftOfPrevious = getColumnPosition (newIndex - 1).getX();
                            auto rightOfCurrent = getColumnPosition (newIndex).getRight();

                            if (std::abs (dragOverlayComp->getX() - leftOfPrevious)
                                 < std::abs (dragOverlayComp->getRight() - rightOfCurrent))
                            {
                                --newIndex;
                            }
                        }
                    }

                    if (newIndex < columns.size() - 1)
                    {
                        // if the next column isn't draggable, we can't move our column
                        // past it, because that'd change the undraggable column's position..
                        auto* nextCol = columns.getUnchecked (newIndex + 1);

                        if ((nextCol->propertyFlags & draggable) != 0)
                        {
                            auto leftOfCurrent = getColumnPosition (newIndex).getX();
                            auto rightOfNext = getColumnPosition (newIndex + 1).getRight();

                            if (std::abs (dragOverlayComp->getX() - leftOfCurrent)
                                 > std::abs (dragOverlayComp->getRight() - rightOfNext))
                            {
                                ++newIndex;
                            }
                        }
                    }

                    if (newIndex != currentIndex)
                        moveColumn (columnIdBeingDragged, newIndex);
                    else
                        break;
                }
            }
        }
        else
        {
            endDrag (draggingColumnOriginalIndex);
        }
    }
}

z0 TableHeaderComponent::beginDrag (const MouseEvent& e)
{
    if (columnIdBeingDragged == 0)
    {
        columnIdBeingDragged = getColumnIdAtX (e.getMouseDownX());

        auto* ci = getInfoForId (columnIdBeingDragged);

        if (ci == nullptr || (ci->propertyFlags & draggable) == 0)
        {
            columnIdBeingDragged = 0;
        }
        else
        {
            draggingColumnOriginalIndex = getIndexOfColumnId (columnIdBeingDragged, true);

            auto columnRect = getColumnPosition (draggingColumnOriginalIndex);
            auto temp = columnIdBeingDragged;
            columnIdBeingDragged = 0;

            dragOverlayComp.reset (new DragOverlayComp (createComponentSnapshot (columnRect, false, 2.0f)));
            addAndMakeVisible (dragOverlayComp.get());
            columnIdBeingDragged = temp;

            dragOverlayComp->setBounds (columnRect);

            for (i32 i = listeners.size(); --i >= 0;)
            {
                listeners.getUnchecked (i)->tableColumnDraggingChanged (this, columnIdBeingDragged);
                i = jmin (i, listeners.size() - 1);
            }
        }
    }
}

z0 TableHeaderComponent::endDrag (i32k finalIndex)
{
    if (columnIdBeingDragged != 0)
    {
        moveColumn (columnIdBeingDragged, finalIndex);

        columnIdBeingDragged = 0;
        resized();
        repaint();

        for (i32 i = listeners.size(); --i >= 0;)
        {
            listeners.getUnchecked (i)->tableColumnDraggingChanged (this, 0);
            i = jmin (i, listeners.size() - 1);
        }
    }
}

z0 TableHeaderComponent::mouseUp (const MouseEvent& e)
{
    mouseDrag (e);

    for (auto* c : columns)
        if (c->isVisible())
            c->lastDeliberateWidth = c->width;

    columnIdBeingResized = 0;
    resized();
    repaint();

    endDrag (getIndexOfColumnId (columnIdBeingDragged, true));

    updateColumnUnderMouse (e);

    if (columnIdUnderMouse != 0 && ! (e.mouseWasDraggedSinceMouseDown() || e.mods.isPopupMenu()))
        columnClicked (columnIdUnderMouse, e.mods);

    dragOverlayComp.reset();
}

MouseCursor TableHeaderComponent::getMouseCursor()
{
    if (columnIdBeingResized != 0 || (getResizeDraggerAt (getMouseXYRelative().getX()) != 0 && ! isMouseButtonDown()))
        return MouseCursor (MouseCursor::LeftRightResizeCursor);

    return Component::getMouseCursor();
}

//==============================================================================

TableHeaderComponent::ColumnInfo* TableHeaderComponent::getInfoForId (i32 id) const
{
    for (auto* c : columns)
        if (c->id == id)
            return c;

    return nullptr;
}

i32 TableHeaderComponent::visibleIndexToTotalIndex (i32k visibleIndex) const
{
    i32 n = 0;

    for (i32 i = 0; i < columns.size(); ++i)
    {
        if (columns.getUnchecked (i)->isVisible())
        {
            if (n == visibleIndex)
                return i;

            ++n;
        }
    }

    return -1;
}

z0 TableHeaderComponent::sendColumnsChanged()
{
    if (stretchToFit && lastDeliberateWidth > 0)
        resizeAllColumnsToFit (lastDeliberateWidth);

    resized();
    repaint();
    columnsChanged = true;
    triggerAsyncUpdate();
}

z0 TableHeaderComponent::handleAsyncUpdate()
{
    const b8 changed = columnsChanged || sortChanged;
    const b8 sized = columnsResized || changed;
    const b8 sorted = sortChanged;
    columnsChanged = false;
    columnsResized = false;
    sortChanged = false;

    if (sorted)
    {
        for (i32 i = listeners.size(); --i >= 0;)
        {
            listeners.getUnchecked (i)->tableSortOrderChanged (this);
            i = jmin (i, listeners.size() - 1);
        }
    }

    if (changed)
    {
        for (i32 i = listeners.size(); --i >= 0;)
        {
            listeners.getUnchecked (i)->tableColumnsChanged (this);
            i = jmin (i, listeners.size() - 1);
        }
    }

    if (sized)
    {
        for (i32 i = listeners.size(); --i >= 0;)
        {
            listeners.getUnchecked (i)->tableColumnsResized (this);
            i = jmin (i, listeners.size() - 1);
        }
    }
}

i32 TableHeaderComponent::getResizeDraggerAt (i32k mouseX) const
{
    if (isPositiveAndBelow (mouseX, getWidth()))
    {
        i32k draggableDistance = 3;
        i32 x = 0;

        for (auto* ci : columns)
        {
            if (ci->isVisible())
            {
                if (std::abs (mouseX - (x + ci->width)) <= draggableDistance
                     && (ci->propertyFlags & resizable) != 0)
                    return ci->id;

                x += ci->width;
            }
        }
    }

    return 0;
}

z0 TableHeaderComponent::setColumnUnderMouse (i32k newCol)
{
    if (newCol != columnIdUnderMouse)
    {
        columnIdUnderMouse = newCol;
        repaint();
    }
}

z0 TableHeaderComponent::updateColumnUnderMouse (const MouseEvent& e)
{
    setColumnUnderMouse (reallyContains (e.getPosition(), true) && getResizeDraggerAt (e.x) == 0
                            ? getColumnIdAtX (e.x) : 0);
}

static z0 tableHeaderMenuCallback (i32 result, TableHeaderComponent* tableHeader, i32 columnIdClicked)
{
    if (tableHeader != nullptr && result != 0)
        tableHeader->reactToMenuItem (result, columnIdClicked);
}

z0 TableHeaderComponent::showColumnChooserMenu (i32k columnIdClicked)
{
    PopupMenu m;
    addMenuItems (m, columnIdClicked);

    if (m.getNumItems() > 0)
    {
        m.setLookAndFeel (&getLookAndFeel());

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (tableHeaderMenuCallback, this, columnIdClicked));
    }
}

z0 TableHeaderComponent::Listener::tableColumnDraggingChanged (TableHeaderComponent*, i32)
{
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> TableHeaderComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::tableHeader);
}

std::unique_ptr<AccessibilityHandler> TableHeaderComponent::ColumnInfo::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::tableHeader);
}

} // namespace drx
