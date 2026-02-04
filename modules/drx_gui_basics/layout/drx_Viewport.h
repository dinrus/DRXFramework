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
    A Viewport is used to contain a larger child component, and allows the child
    to be automatically scrolled around.

    To use a Viewport, just create one and set the component that goes inside it
    using the setViewedComponent() method. When the child component changes size,
    the Viewport will adjust its scrollbars accordingly.

    A subclass of the viewport can be created which will receive calls to its
    visibleAreaChanged() method when the subcomponent changes position or size.


    @tags{GUI}
*/
class DRX_API  Viewport  : public Component,
                            private ComponentListener,
                            private ScrollBar::Listener
{
public:
    //==============================================================================
    /** Creates a Viewport.

        The viewport is initially empty - use the setViewedComponent() method to
        add a child component for it to manage.
    */
    explicit Viewport (const Txt& componentName = Txt());

    /** Destructor. */
    ~Viewport() override;

    //==============================================================================
    /** Sets the component that this viewport will contain and scroll around.

        This will add the given component to this Viewport and position it at (0, 0).

        (Don't add or remove any child components directly using the normal
        Component::addChildComponent() methods).

        @param newViewedComponent   the component to add to this viewport, or null to remove
                                    the current component.
        @param deleteComponentWhenNoLongerNeeded    if true, the component will be deleted
                                    automatically when the viewport is deleted or when a different
                                    component is added. If false, the caller must manage the lifetime
                                    of the component
        @see getViewedComponent
    */
    z0 setViewedComponent (Component* newViewedComponent,
                             b8 deleteComponentWhenNoLongerNeeded = true);

    /** Returns the component that's currently being used inside the Viewport.

        @see setViewedComponent
    */
    Component* getViewedComponent() const noexcept                  { return contentComp.get(); }

    //==============================================================================
    /** Changes the position of the viewed component.

        The inner component will be moved so that the pixel at the top left of
        the viewport will be the pixel at position (xPixelsOffset, yPixelsOffset)
        within the inner component.

        This will update the scrollbars and might cause a call to visibleAreaChanged().

        @see getViewPositionX, getViewPositionY, setViewPositionProportionately
    */
    z0 setViewPosition (i32 xPixelsOffset, i32 yPixelsOffset);

    /** Changes the position of the viewed component.

        The inner component will be moved so that the pixel at the top left of
        the viewport will be the pixel at the specified coordinates within the
        inner component.

        This will update the scrollbars and might cause a call to visibleAreaChanged().

        @see getViewPositionX, getViewPositionY, setViewPositionProportionately
    */
    z0 setViewPosition (Point<i32> newPosition);

    /** Changes the view position as a proportion of the distance it can move.

        The values here are from 0.0 to 1.0 - where (0, 0) would put the
        visible area in the top-left, and (1, 1) would put it as far down and
        to the right as it's possible to go whilst keeping the child component
        on-screen.
    */
    z0 setViewPositionProportionately (f64 proportionX, f64 proportionY);

    /** If the specified position is at the edges of the viewport, this method scrolls
        the viewport to bring that position nearer to the centre.

        Call this if you're dragging an object inside a viewport and want to make it scroll
        when the user approaches an edge. You might also find Component::beginDragAutoRepeat()
        useful when auto-scrolling.

        @param mouseX       the x position, relative to the Viewport's top-left
        @param mouseY       the y position, relative to the Viewport's top-left
        @param distanceFromEdge     specifies how close to an edge the position needs to be
                            before the viewport should scroll in that direction
        @param maximumSpeed the maximum number of pixels that the viewport is allowed
                            to scroll by.
        @returns            true if the viewport was scrolled
    */
    b8 autoScroll (i32 mouseX, i32 mouseY, i32 distanceFromEdge, i32 maximumSpeed);

    /** Returns the position within the child component of the top-left of its visible area. */
    Point<i32> getViewPosition() const noexcept             { return lastVisibleArea.getPosition(); }

    /** Returns the visible area of the child component, relative to its top-left */
    Rectangle<i32> getViewArea() const noexcept             { return lastVisibleArea; }

    /** Returns the position within the child component of the top-left of its visible area.
        @see getViewWidth, setViewPosition
    */
    i32 getViewPositionX() const noexcept                   { return lastVisibleArea.getX(); }

    /** Returns the position within the child component of the top-left of its visible area.
        @see getViewHeight, setViewPosition
    */
    i32 getViewPositionY() const noexcept                   { return lastVisibleArea.getY(); }

    /** Returns the width of the visible area of the child component.

        This may be less than the width of this Viewport if there's a vertical scrollbar
        or if the child component is itself smaller.
    */
    i32 getViewWidth() const noexcept                       { return lastVisibleArea.getWidth(); }

    /** Returns the height of the visible area of the child component.

        This may be less than the height of this Viewport if there's a horizontal scrollbar
        or if the child component is itself smaller.
    */
    i32 getViewHeight() const noexcept                      { return lastVisibleArea.getHeight(); }

    /** Returns the width available within this component for the contents.

        This will be the width of the viewport component minus the width of a
        vertical scrollbar (if visible).
    */
    i32 getMaximumVisibleWidth() const;

    /** Returns the height available within this component for the contents.

        This will be the height of the viewport component minus the space taken up
        by a horizontal scrollbar (if visible).
    */
    i32 getMaximumVisibleHeight() const;

    //==============================================================================
    /** Callback method that is called when the visible area changes.

        This will be called when the visible area is moved either be scrolling or
        by calls to setViewPosition(), etc.
    */
    virtual z0 visibleAreaChanged (const Rectangle<i32>& newVisibleArea);

    /** Callback method that is called when the viewed component is added, removed or swapped. */
    virtual z0 viewedComponentChanged (Component* newComponent);

    //==============================================================================
    /** Turns scrollbars on or off.

        If set to false, the scrollbars won't ever appear. When true (the default)
        they will appear only when needed.

        The allowVerticalScrollingWithoutScrollbar parameters allow you to enable
        mouse-wheel scrolling even when there the scrollbars are hidden. When the
        scrollbars are visible, these parameters are ignored.
    */
    z0 setScrollBarsShown (b8 showVerticalScrollbarIfNeeded,
                             b8 showHorizontalScrollbarIfNeeded,
                             b8 allowVerticalScrollingWithoutScrollbar = false,
                             b8 allowHorizontalScrollingWithoutScrollbar = false);

    /** Changes where the scroll bars are positioned

        If verticalScrollbarOnRight is set to true, then the vertical scrollbar will
        appear on the right side of the view port's content (this is the default),
        otherwise it will be on the left side of the content.

        If horizontalScrollbarAtBottom is set to true, then the horizontal scrollbar
        will appear at the bottom of the view port's content (this is the default),
        otherwise it will be at the top.
    */
    z0 setScrollBarPosition (b8 verticalScrollbarOnRight,
                               b8 horizontalScrollbarAtBottom);

    /** True if the vertical scrollbar will appear on the right side of the content */
    b8 isVerticalScrollbarOnTheRight() const noexcept         { return vScrollbarRight; }

    /** True if the horizontal scrollbar will appear at the bottom of the content */
    b8 isHorizontalScrollbarAtBottom() const noexcept         { return hScrollbarBottom; }

    /** True if the vertical scrollbar is enabled.
        @see setScrollBarsShown
    */
    b8 isVerticalScrollBarShown() const noexcept              { return showVScrollbar; }

    /** True if the horizontal scrollbar is enabled.
        @see setScrollBarsShown
    */
    b8 isHorizontalScrollBarShown() const noexcept            { return showHScrollbar; }

    /** Changes the width of the scrollbars.
        If this isn't specified, the default width from the LookAndFeel class will be used.
        @see LookAndFeel::getDefaultScrollbarWidth
    */
    z0 setScrollBarThickness (i32 thickness);

    /** Returns the thickness of the scrollbars.
        @see setScrollBarThickness
    */
    i32 getScrollBarThickness() const;

    /** Changes the distance that a single-step click on a scrollbar button
        will move the viewport.
    */
    z0 setSingleStepSizes (i32 stepX, i32 stepY);

    /** Returns a reference to the scrollbar component being used.
        Handy if you need to customise the bar somehow.
    */
    ScrollBar& getVerticalScrollBar() noexcept                  { return *verticalScrollBar; }

    /** Returns a reference to the scrollbar component being used.
        Handy if you need to customise the bar somehow.
    */
    ScrollBar& getHorizontalScrollBar() noexcept                { return *horizontalScrollBar; }

    /** Re-instantiates the scrollbars, which is only really useful if you've overridden createScrollBarComponent(). */
    z0 recreateScrollbars();

    /** True if there's any off-screen content that could be scrolled vertically,
        or false if everything is currently visible.
    */
    b8 canScrollVertically() const noexcept;

    /** True if there's any off-screen content that could be scrolled horizontally,
        or false if everything is currently visible.
    */
    b8 canScrollHorizontally() const noexcept;

    /** Enables or disables drag-to-scroll functionality for mouse sources in the viewport.

        If your viewport contains a Component that you don't want to receive mouse events when the
        user is drag-scrolling, you can disable this with the Component::setViewportIgnoreDragFlag()
        method.
    */
    [[deprecated ("Use setScrollOnDragMode instead.")]]
    z0 setScrollOnDragEnabled (b8 shouldScrollOnDrag)
    {
        setScrollOnDragMode (shouldScrollOnDrag ? ScrollOnDragMode::all : ScrollOnDragMode::never);
    }

    /** Возвращает true, если drag-to-scroll functionality is enabled for mouse input sources. */
    [[deprecated ("Use getScrollOnDragMode instead.")]]
    b8 isScrollOnDragEnabled() const noexcept { return getScrollOnDragMode() == ScrollOnDragMode::all; }

    enum class ScrollOnDragMode
    {
        never,          /**< Dragging will never scroll the viewport. */
        nonHover,       /**< Dragging will only scroll the viewport if the input source cannot hover. */
        all             /**< Dragging will always scroll the viewport. */
    };

    /** Sets the current scroll-on-drag mode. The default is ScrollOnDragMode::nonHover.

        If your viewport contains a Component that you don't want to receive mouse events when the
        user is drag-scrolling, you can disable this with the Component::setViewportIgnoreDragFlag()
        method.
    */
    z0 setScrollOnDragMode (ScrollOnDragMode scrollOnDragMode);

    /** Returns the current scroll-on-drag mode. */
    ScrollOnDragMode getScrollOnDragMode() const { return scrollOnDragMode; }

    /** Возвращает true, если the user is currently dragging-to-scroll.
        @see setScrollOnDragEnabled
    */
    b8 isCurrentlyScrollingOnDrag() const noexcept;

    //==============================================================================
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 scrollBarMoved (ScrollBar*, f64 newRangeStart) override;
    /** @internal */
    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent& e) override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 componentMovedOrResized (Component&, b8 wasMoved, b8 wasResized) override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    b8 useMouseWheelMoveIfNeeded (const MouseEvent&, const MouseWheelDetails&);
    /** @internal */
    static b8 respondsToKey (const KeyPress&);

protected:
    //==============================================================================
    /** Creates the Scrollbar components that will be added to the Viewport.
        Subclasses can override this if they need to customise the scrollbars in some way.
    */
    virtual ScrollBar* createScrollBarComponent (b8 isVertical);

private:
    //==============================================================================
    class AccessibilityIgnoredComponent : public Component
    {
    public:
        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            return createIgnoredAccessibilityHandler (*this);
        }
    };

    std::unique_ptr<ScrollBar> verticalScrollBar, horizontalScrollBar;
    AccessibilityIgnoredComponent contentHolder;
    WeakReference<Component> contentComp;
    Rectangle<i32> lastVisibleArea;
    i32 scrollBarThickness = 0;
    i32 singleStepX = 16, singleStepY = 16;
    ScrollOnDragMode scrollOnDragMode = ScrollOnDragMode::nonHover;
    b8 showHScrollbar = true, showVScrollbar = true, deleteContent = true;
    b8 customScrollBarThickness = false;
    b8 allowScrollingWithoutScrollbarV = false, allowScrollingWithoutScrollbarH = false;
    b8 vScrollbarRight = true, hScrollbarBottom = true;

    struct DragToScrollListener;
    std::unique_ptr<DragToScrollListener> dragToScrollListener;

    Point<i32> viewportPosToCompPos (Point<i32>) const;
    Rectangle<i32> getContentBounds() const;

    z0 updateVisibleArea();
    z0 deleteOrRemoveContentComp();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Viewport)
};

} // namespace drx
