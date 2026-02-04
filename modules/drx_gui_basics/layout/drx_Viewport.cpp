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

using ViewportDragPosition = AnimatedPosition<AnimatedPositionBehaviours::ContinuousWithMomentum>;

struct Viewport::DragToScrollListener final : private MouseListener,
                                              private ViewportDragPosition::Listener
{
    DragToScrollListener (Viewport& v)  : viewport (v)
    {
        viewport.contentHolder.addMouseListener (this, true);
        offsetX.addListener (this);
        offsetY.addListener (this);
        offsetX.behaviour.setMinimumVelocity (60);
        offsetY.behaviour.setMinimumVelocity (60);
    }

    ~DragToScrollListener() override
    {
        viewport.contentHolder.removeMouseListener (this);
        Desktop::getInstance().removeGlobalMouseListener (this);
    }

    z0 stopOngoingAnimation()
    {
        offsetX.setPosition (offsetX.getPosition());
        offsetY.setPosition (offsetY.getPosition());
    }

    z0 positionChanged (ViewportDragPosition&, f64) override
    {
        viewport.setViewPosition (originalViewPos - Point<i32> ((i32) offsetX.getPosition(),
                                                                (i32) offsetY.getPosition()));
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        if (! isGlobalMouseListener && detail::ViewportHelpers::wouldScrollOnEvent (&viewport, e.source))
        {
            offsetX.setPosition (offsetX.getPosition());
            offsetY.setPosition (offsetY.getPosition());

            // switch to a global mouse listener so we still receive mouseUp events
            // if the original event component is deleted
            viewport.contentHolder.removeMouseListener (this);
            Desktop::getInstance().addGlobalMouseListener (this);

            isGlobalMouseListener = true;

            scrollSource = e.source;
        }
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        if (e.source == scrollSource
            && ! doesMouseEventComponentBlockViewportDrag (e.eventComponent))
        {
            auto totalOffset = e.getEventRelativeTo (&viewport).getOffsetFromDragStart().toFloat();

            if (! isDragging && totalOffset.getDistanceFromOrigin() > 8.0f && detail::ViewportHelpers::wouldScrollOnEvent (&viewport, e.source))
            {
                isDragging = true;

                originalViewPos = viewport.getViewPosition();
                offsetX.setPosition (0.0);
                offsetX.beginDrag();
                offsetY.setPosition (0.0);
                offsetY.beginDrag();
            }

            if (isDragging)
            {
                offsetX.drag (totalOffset.x);
                offsetY.drag (totalOffset.y);
            }
        }
    }

    z0 mouseUp (const MouseEvent& e) override
    {
        if (isGlobalMouseListener && e.source == scrollSource)
            endDragAndClearGlobalMouseListener();
    }

    z0 endDragAndClearGlobalMouseListener()
    {
        if (std::exchange (isDragging, false) == true)
        {
            offsetX.endDrag();
            offsetY.endDrag();
        }

        viewport.contentHolder.addMouseListener (this, true);
        Desktop::getInstance().removeGlobalMouseListener (this);

        isGlobalMouseListener = false;
    }

    b8 doesMouseEventComponentBlockViewportDrag (const Component* eventComp)
    {
        for (auto c = eventComp; c != nullptr && c != &viewport; c = c->getParentComponent())
            if (c->getViewportIgnoreDragFlag())
                return true;

        return false;
    }

    Viewport& viewport;
    ViewportDragPosition offsetX, offsetY;
    Point<i32> originalViewPos;
    MouseInputSource scrollSource = Desktop::getInstance().getMainMouseSource();
    b8 isDragging = false;
    b8 isGlobalMouseListener = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragToScrollListener)
};

//==============================================================================
Viewport::Viewport (const Txt& name)
    : Component (name),
      dragToScrollListener (std::make_unique<DragToScrollListener> (*this))
{
    // content holder is used to clip the contents so they don't overlap the scrollbars
    addAndMakeVisible (contentHolder);
    contentHolder.setInterceptsMouseClicks (false, true);

    scrollBarThickness = getLookAndFeel().getDefaultScrollbarWidth();

    setInterceptsMouseClicks (false, true);
    setWantsKeyboardFocus (true);

    recreateScrollbars();
}

Viewport::~Viewport()
{
    deleteOrRemoveContentComp();
}

//==============================================================================
z0 Viewport::visibleAreaChanged (const Rectangle<i32>&) {}
z0 Viewport::viewedComponentChanged (Component*) {}

//==============================================================================
z0 Viewport::deleteOrRemoveContentComp()
{
    if (contentComp != nullptr)
    {
        contentComp->removeComponentListener (this);

        if (deleteContent)
        {
            // This sets the content comp to a null pointer before deleting the old one, in case
            // anything tries to use the old one while it's in mid-deletion..
            std::unique_ptr<Component> oldCompDeleter (contentComp.get());
            contentComp = nullptr;
        }
        else
        {
            contentHolder.removeChildComponent (contentComp);
            contentComp = nullptr;
        }
    }
}

z0 Viewport::setViewedComponent (Component* const newViewedComponent, const b8 deleteComponentWhenNoLongerNeeded)
{
    if (contentComp.get() != newViewedComponent)
    {
        deleteOrRemoveContentComp();
        contentComp = newViewedComponent;
        deleteContent = deleteComponentWhenNoLongerNeeded;

        if (contentComp != nullptr)
        {
            contentHolder.addAndMakeVisible (contentComp);
            setViewPosition (Point<i32>());
            contentComp->addComponentListener (this);
        }

        viewedComponentChanged (contentComp);
        updateVisibleArea();
    }
}

z0 Viewport::recreateScrollbars()
{
    verticalScrollBar.reset();
    horizontalScrollBar.reset();

    verticalScrollBar  .reset (createScrollBarComponent (true));
    horizontalScrollBar.reset (createScrollBarComponent (false));

    addChildComponent (verticalScrollBar.get());
    addChildComponent (horizontalScrollBar.get());

    getVerticalScrollBar().addListener (this);
    getHorizontalScrollBar().addListener (this);
    getVerticalScrollBar().addMouseListener (this, true);
    getHorizontalScrollBar().addMouseListener (this, true);

    resized();
}

i32 Viewport::getMaximumVisibleWidth() const            { return contentHolder.getWidth(); }
i32 Viewport::getMaximumVisibleHeight() const           { return contentHolder.getHeight(); }

b8 Viewport::canScrollVertically() const noexcept     { return contentComp->getY() < 0 || contentComp->getBottom() > getHeight(); }
b8 Viewport::canScrollHorizontally() const noexcept   { return contentComp->getX() < 0 || contentComp->getRight()  > getWidth(); }

Point<i32> Viewport::viewportPosToCompPos (Point<i32> pos) const
{
    jassert (contentComp != nullptr);

    const auto contentBounds = getContentBounds();

    const Point p (jmax (jmin (0, contentHolder.getWidth()  - contentBounds.getWidth()),  jmin (0, -(pos.x))),
                   jmax (jmin (0, contentHolder.getHeight() - contentBounds.getHeight()), jmin (0, -(pos.y))));

    return p.transformedBy (contentComp->getTransform().inverted());
}

Rectangle<i32> Viewport::getContentBounds() const
{
    if (auto* cc = contentComp.get())
        return contentHolder.getLocalArea (cc, cc->getLocalBounds());

    return {};
}

z0 Viewport::setViewPosition (i32k xPixelsOffset, i32k yPixelsOffset)
{
    setViewPosition ({ xPixelsOffset, yPixelsOffset });
}

z0 Viewport::setViewPosition (Point<i32> newPosition)
{
    if (contentComp != nullptr)
        contentComp->setTopLeftPosition (viewportPosToCompPos (newPosition));
}

z0 Viewport::setViewPositionProportionately (const f64 x, const f64 y)
{
    if (contentComp != nullptr)
        setViewPosition (jmax (0, roundToInt (x * (contentComp->getWidth()  - getWidth()))),
                         jmax (0, roundToInt (y * (contentComp->getHeight() - getHeight()))));
}

b8 Viewport::autoScroll (i32k mouseX, i32k mouseY, i32k activeBorderThickness, i32k maximumSpeed)
{
    if (contentComp != nullptr)
    {
        i32 dx = 0, dy = 0;

        if (getHorizontalScrollBar().isVisible() || canScrollHorizontally())
        {
            if (mouseX < activeBorderThickness)
                dx = activeBorderThickness - mouseX;
            else if (mouseX >= contentHolder.getWidth() - activeBorderThickness)
                dx = (contentHolder.getWidth() - activeBorderThickness) - mouseX;

            if (dx < 0)
                dx = jmax (dx, -maximumSpeed, contentHolder.getWidth() - contentComp->getRight());
            else
                dx = jmin (dx, maximumSpeed, -contentComp->getX());
        }

        if (getVerticalScrollBar().isVisible() || canScrollVertically())
        {
            if (mouseY < activeBorderThickness)
                dy = activeBorderThickness - mouseY;
            else if (mouseY >= contentHolder.getHeight() - activeBorderThickness)
                dy = (contentHolder.getHeight() - activeBorderThickness) - mouseY;

            if (dy < 0)
                dy = jmax (dy, -maximumSpeed, contentHolder.getHeight() - contentComp->getBottom());
            else
                dy = jmin (dy, maximumSpeed, -contentComp->getY());
        }

        if (dx != 0 || dy != 0)
        {
            contentComp->setTopLeftPosition (contentComp->getX() + dx,
                                             contentComp->getY() + dy);

            return true;
        }
    }

    return false;
}

z0 Viewport::componentMovedOrResized (Component&, b8, b8)
{
    updateVisibleArea();
}

//==============================================================================
z0 Viewport::setScrollOnDragMode (const ScrollOnDragMode mode)
{
    scrollOnDragMode = mode;
}

b8 Viewport::isCurrentlyScrollingOnDrag() const noexcept
{
    return dragToScrollListener->isDragging;
}

//==============================================================================
z0 Viewport::lookAndFeelChanged()
{
    if (! customScrollBarThickness)
    {
        scrollBarThickness = getLookAndFeel().getDefaultScrollbarWidth();
        resized();
    }
}

z0 Viewport::resized()
{
    updateVisibleArea();
}

//==============================================================================
z0 Viewport::updateVisibleArea()
{
    auto scrollbarWidth = getScrollBarThickness();
    const b8 canShowAnyBars = getWidth() > scrollbarWidth && getHeight() > scrollbarWidth;
    const b8 canShowHBar = showHScrollbar && canShowAnyBars;
    const b8 canShowVBar = showVScrollbar && canShowAnyBars;

    b8 hBarVisible = false, vBarVisible = false;
    Rectangle<i32> contentArea;

    for (i32 i = 3; --i >= 0;)
    {
        hBarVisible = canShowHBar && ! getHorizontalScrollBar().autoHides();
        vBarVisible = canShowVBar && ! getVerticalScrollBar().autoHides();
        contentArea = getLocalBounds();

        if (contentComp != nullptr && ! contentArea.contains (contentComp->getBounds()))
        {
            hBarVisible = canShowHBar && (hBarVisible || contentComp->getX() < 0 || contentComp->getRight() > contentArea.getWidth());
            vBarVisible = canShowVBar && (vBarVisible || contentComp->getY() < 0 || contentComp->getBottom() > contentArea.getHeight());

            if (vBarVisible)
                contentArea.setWidth (getWidth() - scrollbarWidth);

            if (hBarVisible)
                contentArea.setHeight (getHeight() - scrollbarWidth);

            if (! contentArea.contains (contentComp->getBounds()))
            {
                hBarVisible = canShowHBar && (hBarVisible || contentComp->getRight() > contentArea.getWidth());
                vBarVisible = canShowVBar && (vBarVisible || contentComp->getBottom() > contentArea.getHeight());
            }
        }

        if (vBarVisible)  contentArea.setWidth  (getWidth()  - scrollbarWidth);
        if (hBarVisible)  contentArea.setHeight (getHeight() - scrollbarWidth);

        if (! vScrollbarRight  && vBarVisible)
            contentArea.setX (scrollbarWidth);

        if (! hScrollbarBottom && hBarVisible)
            contentArea.setY (scrollbarWidth);

        if (contentComp == nullptr)
        {
            contentHolder.setBounds (contentArea);
            break;
        }

        auto oldContentBounds = contentComp->getBounds();
        contentHolder.setBounds (contentArea);

        // If the content has changed its size, that might affect our scrollbars, so go round again and re-calculate..
        if (oldContentBounds == contentComp->getBounds())
            break;
    }

    const auto contentBounds = getContentBounds();
    auto visibleOrigin = -contentBounds.getPosition();

    auto& hbar = getHorizontalScrollBar();
    auto& vbar = getVerticalScrollBar();

    hbar.setBounds (contentArea.getX(), hScrollbarBottom ? contentArea.getHeight() : 0, contentArea.getWidth(), scrollbarWidth);
    hbar.setRangeLimits (0.0, contentBounds.getWidth());
    hbar.setCurrentRange (visibleOrigin.x, contentArea.getWidth());
    hbar.setSingleStepSize (singleStepX);

    if (canShowHBar && ! hBarVisible)
        visibleOrigin.setX (0);

    vbar.setBounds (vScrollbarRight ? contentArea.getWidth() : 0, contentArea.getY(), scrollbarWidth, contentArea.getHeight());
    vbar.setRangeLimits (0.0, contentBounds.getHeight());
    vbar.setCurrentRange (visibleOrigin.y, contentArea.getHeight());
    vbar.setSingleStepSize (singleStepY);

    if (canShowVBar && ! vBarVisible)
        visibleOrigin.setY (0);

    // Force the visibility *after* setting the ranges to avoid flicker caused by edge conditions in the numbers.
    hbar.setVisible (hBarVisible);
    vbar.setVisible (vBarVisible);

    if (contentComp != nullptr)
    {
        auto newContentCompPos = viewportPosToCompPos (visibleOrigin);

        if (contentComp->getBounds().getPosition() != newContentCompPos)
        {
            contentComp->setTopLeftPosition (newContentCompPos);  // (this will re-entrantly call updateVisibleArea again)
            return;
        }
    }

    const Rectangle<i32> visibleArea (visibleOrigin.x, visibleOrigin.y,
                                      jmin (contentBounds.getWidth()  - visibleOrigin.x, contentArea.getWidth()),
                                      jmin (contentBounds.getHeight() - visibleOrigin.y, contentArea.getHeight()));

    if (lastVisibleArea != visibleArea)
    {
        lastVisibleArea = visibleArea;
        visibleAreaChanged (visibleArea);
    }

    hbar.handleUpdateNowIfNeeded();
    vbar.handleUpdateNowIfNeeded();
}

//==============================================================================
z0 Viewport::setSingleStepSizes (i32k stepX, i32k stepY)
{
    if (singleStepX != stepX || singleStepY != stepY)
    {
        singleStepX = stepX;
        singleStepY = stepY;
        updateVisibleArea();
    }
}

z0 Viewport::setScrollBarsShown (const b8 showVerticalScrollbarIfNeeded,
                                   const b8 showHorizontalScrollbarIfNeeded,
                                   const b8 allowVerticalScrollingWithoutScrollbar,
                                   const b8 allowHorizontalScrollingWithoutScrollbar)
{
    allowScrollingWithoutScrollbarV = allowVerticalScrollingWithoutScrollbar;
    allowScrollingWithoutScrollbarH = allowHorizontalScrollingWithoutScrollbar;

    if (showVScrollbar != showVerticalScrollbarIfNeeded
         || showHScrollbar != showHorizontalScrollbarIfNeeded)
    {
        showVScrollbar = showVerticalScrollbarIfNeeded;
        showHScrollbar = showHorizontalScrollbarIfNeeded;
        updateVisibleArea();
    }
}

z0 Viewport::setScrollBarThickness (i32k thickness)
{
    i32 newThickness;

    // To stay compatible with the previous code: use the
    // default thickness if thickness parameter is zero
    // or negative
    if (thickness <= 0)
    {
        customScrollBarThickness = false;
        newThickness = getLookAndFeel().getDefaultScrollbarWidth();
    }
    else
    {
        customScrollBarThickness = true;
        newThickness = thickness;
    }

    if (scrollBarThickness != newThickness)
    {
        scrollBarThickness = newThickness;
        updateVisibleArea();
    }
}

i32 Viewport::getScrollBarThickness() const
{
    return scrollBarThickness;
}

z0 Viewport::scrollBarMoved (ScrollBar* scrollBarThatHasMoved, f64 newRangeStart)
{
    const auto contentOrigin = -getContentBounds().getPosition();
    const auto newRangeStartInt = roundToInt (newRangeStart);

    for (const auto& [member, bar] : { std::tuple (&Point<i32>::x, horizontalScrollBar.get()),
                                       std::tuple (&Point<i32>::y, verticalScrollBar.get()) })
    {
        if (scrollBarThatHasMoved != bar)
            continue;

        if (contentOrigin.*member == newRangeStartInt)
            return;

        auto pt = getViewPosition();
        pt.*member = newRangeStartInt;
        setViewPosition (pt);
        return;
    }
}

z0 Viewport::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (e.eventComponent == this)
        if (! useMouseWheelMoveIfNeeded (e, wheel))
            Component::mouseWheelMove (e, wheel);
}

z0 Viewport::mouseDown (const MouseEvent& e)
{
    if (e.eventComponent == horizontalScrollBar.get() || e.eventComponent == verticalScrollBar.get())
        dragToScrollListener->stopOngoingAnimation();
}

static i32 rescaleMouseWheelDistance (f32 distance, i32 singleStepSize) noexcept
{
    if (approximatelyEqual (distance, 0.0f))
        return 0;

    distance *= 14.0f * (f32) singleStepSize;

    return roundToInt (distance < 0 ? jmin (distance, -1.0f)
                                    : jmax (distance,  1.0f));
}

b8 Viewport::useMouseWheelMoveIfNeeded (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! (e.mods.isAltDown() || e.mods.isCtrlDown() || e.mods.isCommandDown()))
    {
        const b8 canScrollVert = (allowScrollingWithoutScrollbarV || getVerticalScrollBar().isVisible());
        const b8 canScrollHorz = (allowScrollingWithoutScrollbarH || getHorizontalScrollBar().isVisible());

        if (canScrollHorz || canScrollVert)
        {
            auto deltaX = rescaleMouseWheelDistance (wheel.deltaX, singleStepX);
            auto deltaY = rescaleMouseWheelDistance (wheel.deltaY, singleStepY);

            auto pos = getViewPosition();

            if (deltaX != 0 && deltaY != 0 && canScrollHorz && canScrollVert)
            {
                pos.x -= deltaX;
                pos.y -= deltaY;
            }
            else if (canScrollHorz && (deltaX != 0 || e.mods.isShiftDown() || ! canScrollVert))
            {
                pos.x -= deltaX != 0 ? deltaX : deltaY;
            }
            else if (canScrollVert && deltaY != 0)
            {
                pos.y -= deltaY;
            }

            if (pos != getViewPosition())
            {
                setViewPosition (pos);
                return true;
            }
        }
    }

    return false;
}

static b8 isUpDownKeyPress (const KeyPress& key)
{
    return key == KeyPress::upKey
        || key == KeyPress::downKey
        || key == KeyPress::pageUpKey
        || key == KeyPress::pageDownKey
        || key == KeyPress::homeKey
        || key == KeyPress::endKey;
}

static b8 isLeftRightKeyPress (const KeyPress& key)
{
    return key == KeyPress::leftKey
        || key == KeyPress::rightKey;
}

b8 Viewport::keyPressed (const KeyPress& key)
{
    const b8 isUpDownKey = isUpDownKeyPress (key);

    if (getVerticalScrollBar().isVisible() && isUpDownKey)
        return getVerticalScrollBar().keyPressed (key);

    const b8 isLeftRightKey = isLeftRightKeyPress (key);

    if (getHorizontalScrollBar().isVisible() && (isUpDownKey || isLeftRightKey))
        return getHorizontalScrollBar().keyPressed (key);

    return false;
}

b8 Viewport::respondsToKey (const KeyPress& key)
{
    return isUpDownKeyPress (key) || isLeftRightKeyPress (key);
}

ScrollBar* Viewport::createScrollBarComponent (b8 isVertical)
{
    return new ScrollBar (isVertical);
}

z0 Viewport::setScrollBarPosition (b8 verticalScrollbarOnRight,
                                     b8 horizontalScrollbarAtBottom)
{
    vScrollbarRight  = verticalScrollbarOnRight;
    hScrollbarBottom = horizontalScrollbarAtBottom;

    resized();
}

} // namespace drx
