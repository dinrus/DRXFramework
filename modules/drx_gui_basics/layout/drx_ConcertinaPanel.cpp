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

struct ConcertinaPanel::PanelSizes
{
    struct Panel
    {
        Panel() = default;

        Panel (i32 sz, i32 mn, i32 mx) noexcept
            : size (sz), minSize (mn), maxSize (mx) {}

        i32 setSize (i32 newSize) noexcept
        {
            jassert (minSize <= maxSize);
            auto oldSize = size;
            size = jlimit (minSize, maxSize, newSize);
            return size - oldSize;
        }

        i32 expand (i32 amount) noexcept
        {
            amount = jmin (amount, maxSize - size);
            size += amount;
            return amount;
        }

        i32 reduce (i32 amount) noexcept
        {
            amount = jmin (amount, size - minSize);
            size -= amount;
            return amount;
        }

        b8 canExpand() const noexcept     { return size < maxSize; }
        b8 isMinimised() const noexcept   { return size <= minSize; }

        i32 size, minSize, maxSize;
    };

    Array<Panel> sizes;

    Panel& get (i32 index) noexcept               { return sizes.getReference (index); }
    const Panel& get (i32 index) const noexcept   { return sizes.getReference (index); }

    PanelSizes withMovedPanel (i32 index, i32 targetPosition, i32 totalSpace) const
    {
        auto num = sizes.size();
        totalSpace = jmax (totalSpace, getMinimumSize (0, num));
        targetPosition = jmax (targetPosition, totalSpace - getMaximumSize (index, num));

        PanelSizes newSizes (*this);
        newSizes.stretchRange (0, index, targetPosition - newSizes.getTotalSize (0, index), stretchLast);
        newSizes.stretchRange (index, num, totalSpace - newSizes.getTotalSize (0, index) - newSizes.getTotalSize (index, num), stretchFirst);
        return newSizes;
    }

    PanelSizes fittedInto (i32 totalSpace) const
    {
        auto newSizes (*this);
        auto num = newSizes.sizes.size();
        totalSpace = jmax (totalSpace, getMinimumSize (0, num));
        newSizes.stretchRange (0, num, totalSpace - newSizes.getTotalSize (0, num), stretchAll);
        return newSizes;
    }

    PanelSizes withResizedPanel (i32 index, i32 panelHeight, i32 totalSpace) const
    {
        PanelSizes newSizes (*this);

        if (totalSpace <= 0)
        {
            newSizes.get (index).size = panelHeight;
        }
        else
        {
            auto num = sizes.size();
            auto minSize = getMinimumSize (0, num);
            totalSpace = jmax (totalSpace, minSize);

            newSizes.get (index).setSize (panelHeight);
            newSizes.stretchRange (0, index,   totalSpace - newSizes.getTotalSize (0, num), stretchLast);
            newSizes.stretchRange (index, num, totalSpace - newSizes.getTotalSize (0, num), stretchLast);
            newSizes = newSizes.fittedInto (totalSpace);
        }

        return newSizes;
    }

private:
    enum ExpandMode
    {
        stretchAll,
        stretchFirst,
        stretchLast
    };

    z0 growRangeFirst (i32 start, i32 end, i32 spaceDiff) noexcept
    {
        for (i32 attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (i32 i = start; i < end && spaceDiff > 0; ++i)
                spaceDiff -= get (i).expand (spaceDiff);
    }

    z0 growRangeLast (i32 start, i32 end, i32 spaceDiff) noexcept
    {
        for (i32 attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (i32 i = end; --i >= start && spaceDiff > 0;)
                spaceDiff -= get (i).expand (spaceDiff);
    }

    z0 growRangeAll (i32 start, i32 end, i32 spaceDiff) noexcept
    {
        Array<Panel*> expandableItems;

        for (i32 i = start; i < end; ++i)
            if (get (i).canExpand() && ! get (i).isMinimised())
                expandableItems.add (& get (i));

        for (i32 attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (i32 i = expandableItems.size(); --i >= 0 && spaceDiff > 0;)
                spaceDiff -= expandableItems.getUnchecked (i)->expand (spaceDiff / (i + 1));

        growRangeLast (start, end, spaceDiff);
    }

    z0 shrinkRangeFirst (i32 start, i32 end, i32 spaceDiff) noexcept
    {
        for (i32 i = start; i < end && spaceDiff > 0; ++i)
            spaceDiff -= get (i).reduce (spaceDiff);
    }

    z0 shrinkRangeLast (i32 start, i32 end, i32 spaceDiff) noexcept
    {
        for (i32 i = end; --i >= start && spaceDiff > 0;)
            spaceDiff -= get (i).reduce (spaceDiff);
    }

    z0 stretchRange (i32 start, i32 end, i32 amountToAdd, ExpandMode expandMode) noexcept
    {
        if (end > start)
        {
            if (amountToAdd > 0)
            {
                if (expandMode == stretchAll)        growRangeAll   (start, end, amountToAdd);
                else if (expandMode == stretchFirst) growRangeFirst (start, end, amountToAdd);
                else if (expandMode == stretchLast)  growRangeLast  (start, end, amountToAdd);
            }
            else
            {
                if (expandMode == stretchFirst)  shrinkRangeFirst (start, end, -amountToAdd);
                else                             shrinkRangeLast  (start, end, -amountToAdd);
            }
        }
    }

    i32 getTotalSize (i32 start, i32 end) const noexcept
    {
        i32 tot = 0;
        while (start < end)  tot += get (start++).size;
        return tot;
    }

    i32 getMinimumSize (i32 start, i32 end) const noexcept
    {
        i32 tot = 0;
        while (start < end)  tot += get (start++).minSize;
        return tot;
    }

    i32 getMaximumSize (i32 start, i32 end) const noexcept
    {
        i32 tot = 0;

        while (start < end)
        {
            auto mx = get (start++).maxSize;

            if (mx > 0x100000)
                return mx;

            tot += mx;
        }

        return tot;
    }
};

//==============================================================================
class ConcertinaPanel::PanelHolder final : public Component
{
public:
    PanelHolder (Component* comp, b8 takeOwnership)
        : component (comp, takeOwnership)
    {
        setRepaintsOnMouseActivity (true);
        setWantsKeyboardFocus (false);
        addAndMakeVisible (comp);
    }

    z0 paint (Graphics& g) override
    {
        if (customHeader.get() != nullptr)
            return;

        const Rectangle<i32> area (getWidth(), getHeaderSize());
        g.reduceClipRegion (area);

        getLookAndFeel().drawConcertinaPanelHeader (g, area, isMouseOver(), isMouseButtonDown(),
                                                    getPanel(), *component);
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();
        auto headerBounds = bounds.removeFromTop (getHeaderSize());

        if (customHeader.get() != nullptr)
            customHeader.get()->setBounds (headerBounds);

        component->setBounds (bounds);
    }

    z0 mouseDown (const MouseEvent&) override
    {
        mouseDownY = getY();
        dragStartSizes = getPanel().getFittedSizes();
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown())
        {
            auto& panel = getPanel();
            panel.setLayout (dragStartSizes.withMovedPanel (panel.holders.indexOf (this),
                                                            mouseDownY + e.getDistanceFromDragStartY(),
                                                            panel.getHeight()), false);
        }
    }

    z0 mouseDoubleClick (const MouseEvent&) override
    {
        getPanel().panelHeaderDoubleClicked (component);
    }

    z0 setCustomHeaderComponent (Component* headerComponent, b8 shouldTakeOwnership)
    {
        customHeader = CustomHeader (this, OptionalScopedPointer (headerComponent, shouldTakeOwnership));
        addAndMakeVisible (headerComponent);
    }

    OptionalScopedPointer<Component> component;

private:
    PanelSizes dragStartSizes;
    i32 mouseDownY;

    struct CustomHeader
    {
        CustomHeader() = default;

        CustomHeader (MouseListener* l, OptionalScopedPointer<Component> c)
            : listener (l),
              customHeaderComponent (std::move (c))
        {
            if (customHeaderComponent != nullptr)
                customHeaderComponent->addMouseListener (listener, false);
        }

        CustomHeader (CustomHeader&& other) noexcept
            : listener (std::exchange (other.listener, nullptr)),
              customHeaderComponent (std::exchange (other.customHeaderComponent, {})) {}

        CustomHeader& operator= (CustomHeader&& other) noexcept
        {
            std::swap (other.listener, listener);
            std::swap (other.customHeaderComponent, customHeaderComponent);
            return *this;
        }

        CustomHeader (const CustomHeader& other) = delete;
        CustomHeader& operator= (const CustomHeader& other) = delete;

        ~CustomHeader() noexcept
        {
            if (customHeaderComponent != nullptr)
                customHeaderComponent->removeMouseListener (listener);
        }

        Component* get() const { return customHeaderComponent.get(); }

    private:
        MouseListener* listener = nullptr;
        OptionalScopedPointer<Component> customHeaderComponent;
    };

    CustomHeader customHeader;

    i32 getHeaderSize() const noexcept
    {
        ConcertinaPanel& panel = getPanel();
        auto ourIndex = panel.holders.indexOf (this);
        return panel.currentSizes->get (ourIndex).minSize;
    }

    ConcertinaPanel& getPanel() const
    {
        auto panel = dynamic_cast<ConcertinaPanel*> (getParentComponent());
        jassert (panel != nullptr);
        return *panel;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelHolder)
};

//==============================================================================
ConcertinaPanel::ConcertinaPanel()
    : currentSizes (new PanelSizes()),
      headerHeight (20)
{
}

ConcertinaPanel::~ConcertinaPanel() = default;

i32 ConcertinaPanel::getNumPanels() const noexcept
{
    return holders.size();
}

Component* ConcertinaPanel::getPanel (i32 index) const noexcept
{
    if (PanelHolder* h = holders[index])
        return h->component;

    return nullptr;
}

z0 ConcertinaPanel::addPanel (i32 insertIndex, Component* component, b8 takeOwnership)
{
    jassert (component != nullptr); // can't use a null pointer here!
    jassert (indexOfComp (component) < 0); // You can't add the same component more than once!

    auto holder = new PanelHolder (component, takeOwnership);
    holders.insert (insertIndex, holder);
    currentSizes->sizes.insert (insertIndex, PanelSizes::Panel (headerHeight, headerHeight, std::numeric_limits<i32>::max()));
    addAndMakeVisible (holder);
    resized();
}

z0 ConcertinaPanel::removePanel (Component* component)
{
    auto index = indexOfComp (component);

    if (index >= 0)
    {
        currentSizes->sizes.remove (index);
        holders.remove (index);
        resized();
    }
}

b8 ConcertinaPanel::setPanelSize (Component* panelComponent, i32 height, b8 animate)
{
    auto index = indexOfComp (panelComponent);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    height += currentSizes->get (index).minSize;
    auto oldSize = currentSizes->get (index).size;
    setLayout (currentSizes->withResizedPanel (index, height, getHeight()), animate);
    return oldSize != currentSizes->get (index).size;
}

b8 ConcertinaPanel::expandPanelFully (Component* component, b8 animate)
{
    return setPanelSize (component, getHeight(), animate);
}

z0 ConcertinaPanel::setMaximumPanelSize (Component* component, i32 maximumSize)
{
    auto index = indexOfComp (component);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    if (index >= 0)
    {
        currentSizes->get (index).maxSize = currentSizes->get (index).minSize + maximumSize;
        resized();
    }
}

z0 ConcertinaPanel::setPanelHeaderSize (Component* component, i32 headerSize)
{
    auto index = indexOfComp (component);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    if (index >= 0)
    {
        auto oldMin = currentSizes->get (index).minSize;

        currentSizes->get (index).minSize = headerSize;
        currentSizes->get (index).size += headerSize - oldMin;
        resized();
    }
}

z0 ConcertinaPanel::setCustomPanelHeader (Component* component, Component* customComponent, b8 takeOwnership)
{
    OptionalScopedPointer<Component> optional (customComponent, takeOwnership);

    auto index = indexOfComp (component);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    if (index >= 0)
        holders.getUnchecked (index)->setCustomHeaderComponent (optional.release(), takeOwnership);
}

z0 ConcertinaPanel::resized()
{
    applyLayout (getFittedSizes(), false);
}

i32 ConcertinaPanel::indexOfComp (Component* comp) const noexcept
{
    for (i32 i = 0; i < holders.size(); ++i)
        if (holders.getUnchecked (i)->component == comp)
            return i;

    return -1;
}

ConcertinaPanel::PanelSizes ConcertinaPanel::getFittedSizes() const
{
    return currentSizes->fittedInto (getHeight());
}

z0 ConcertinaPanel::applyLayout (const PanelSizes& sizes, b8 animate)
{
    if (! animate)
        animator.cancelAllAnimations (false);

    i32k animationDuration = 150;
    auto w = getWidth();
    i32 y = 0;

    for (i32 i = 0; i < holders.size(); ++i)
    {
        PanelHolder& p = *holders.getUnchecked (i);

        auto h = sizes.get (i).size;
        const Rectangle<i32> pos (0, y, w, h);

        if (animate)
            animator.animateComponent (&p, pos, 1.0f, animationDuration, false, 1.0, 1.0);
        else
            p.setBounds (pos);

        y += h;
    }
}

z0 ConcertinaPanel::setLayout (const PanelSizes& sizes, b8 animate)
{
    *currentSizes = sizes;
    applyLayout (getFittedSizes(), animate);
}

z0 ConcertinaPanel::panelHeaderDoubleClicked (Component* component)
{
    if (! expandPanelFully (component, true))
        setPanelSize (component, 0, true);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ConcertinaPanel::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
