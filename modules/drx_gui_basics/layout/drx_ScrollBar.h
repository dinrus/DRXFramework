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
    A scrollbar component.

    To use a scrollbar, set up its total range using the setRangeLimits() method - this
    sets the range of values it can represent. Then you can use setCurrentRange() to
    change the position and size of the scrollbar's 'thumb'.

    Registering a ScrollBar::Listener with the scrollbar will allow you to find out when
    the user moves it, and you can use the getCurrentRangeStart() to find out where
    they moved it to.

    The scrollbar will adjust its own visibility according to whether its thumb size
    allows it to actually be scrolled.

    For most purposes, it's probably easier to use a Viewport or ListBox
    instead of handling a scrollbar directly.

    @see ScrollBar::Listener

    @tags{GUI}
*/
class DRX_API  ScrollBar  : public Component,
                             public AsyncUpdater,
                             private Timer
{
public:
    //==============================================================================
    /** Creates a Scrollbar.
        @param isVertical      specifies whether the bar should be a vertical or horizontal
    */
    ScrollBar (b8 isVertical);

    /** Destructor. */
    ~ScrollBar() override;

    //==============================================================================
    /** Возвращает true, если the scrollbar is vertical, false if it's horizontal. */
    b8 isVertical() const noexcept                                { return vertical; }

    /** Changes the scrollbar's direction.

        You'll also need to resize the bar appropriately - this just changes its internal
        layout.

        @param shouldBeVertical     true makes it vertical; false makes it horizontal.
    */
    z0 setOrientation (b8 shouldBeVertical);

    /** Tells the scrollbar whether to make itself invisible when not needed.

        The default behaviour is for a scrollbar to become invisible when the thumb
        fills the whole of its range (i.e. when it can't be moved). Setting this
        value to false forces the bar to always be visible.
        @see autoHides()
    */
    z0 setAutoHide (b8 shouldHideWhenFullRange);

    /** Возвращает true, если this scrollbar is set to auto-hide when its thumb is as big
        as its maximum range.
        @see setAutoHide
    */
    b8 autoHides() const noexcept;

    //==============================================================================
    /** Sets the minimum and maximum values that the bar will move between.

        The bar's thumb will always be constrained so that the entire thumb lies
        within this range.

        @param newRangeLimit    the new range.
        @param notification     whether to send a notification of the change to listeners.
                                A notification will only be sent if the range has changed.

        @see setCurrentRange
    */
    z0 setRangeLimits (Range<f64> newRangeLimit,
                         NotificationType notification = sendNotificationAsync);

    /** Sets the minimum and maximum values that the bar will move between.

        The bar's thumb will always be constrained so that the entire thumb lies
        within this range.

        @param minimum         the new range minimum.
        @param maximum         the new range maximum.
        @param notification    whether to send a notification of the change to listeners.
                               A notification will only be sent if the range has changed.

        @see setCurrentRange
    */
    z0 setRangeLimits (f64 minimum, f64 maximum,
                         NotificationType notification = sendNotificationAsync);

    /** Returns the current limits on the thumb position.
        @see setRangeLimits
    */
    Range<f64> getRangeLimit() const noexcept                    { return totalRange; }

    /** Returns the lower value that the thumb can be set to.

        This is the value set by setRangeLimits().
    */
    f64 getMinimumRangeLimit() const noexcept                    { return totalRange.getStart(); }

    /** Returns the upper value that the thumb can be set to.

        This is the value set by setRangeLimits().
    */
    f64 getMaximumRangeLimit() const noexcept                    { return totalRange.getEnd(); }

    //==============================================================================
    /** Changes the position of the scrollbar's 'thumb'.

        This sets both the position and size of the thumb - to just set the position without
        changing the size, you can use setCurrentRangeStart().

        If this method call actually changes the scrollbar's position, it will trigger an
        asynchronous call to ScrollBar::Listener::scrollBarMoved() for all the listeners that
        are registered.

        The notification parameter can be used to optionally send or inhibit a callback to
        any scrollbar listeners.

        @returns true if the range was changed, or false if nothing was changed.
        @see getCurrentRange. setCurrentRangeStart
    */
    b8 setCurrentRange (Range<f64> newRange,
                          NotificationType notification = sendNotificationAsync);

    /** Changes the position of the scrollbar's 'thumb'.

        This sets both the position and size of the thumb - to just set the position without
        changing the size, you can use setCurrentRangeStart().

        @param newStart     the top (or left) of the thumb, in the range
                            getMinimumRangeLimit() <= newStart <= getMaximumRangeLimit(). If the
                            value is beyond these limits, it will be clipped.
        @param newSize      the size of the thumb, such that
                            getMinimumRangeLimit() <= newStart + newSize <= getMaximumRangeLimit(). If the
                            size is beyond these limits, it will be clipped.
        @param notification specifies if and how a callback should be made to any listeners
                            if the range actually changes
        @see setCurrentRangeStart, getCurrentRangeStart, getCurrentRangeSize
    */
    z0 setCurrentRange (f64 newStart, f64 newSize,
                          NotificationType notification = sendNotificationAsync);

    /** Moves the bar's thumb position.

        This will move the thumb position without changing the thumb size. Note
        that the maximum thumb start position is (getMaximumRangeLimit() - getCurrentRangeSize()).

        If this method call actually changes the scrollbar's position, it will trigger an
        asynchronous call to ScrollBar::Listener::scrollBarMoved() for all the listeners that
        are registered.

        @see setCurrentRange
    */
    z0 setCurrentRangeStart (f64 newStart,
                               NotificationType notification = sendNotificationAsync);

    /** Returns the current thumb range.
        @see getCurrentRange, setCurrentRange
    */
    Range<f64> getCurrentRange() const noexcept                  { return visibleRange; }

    /** Returns the position of the top of the thumb.
        @see getCurrentRange, setCurrentRangeStart
    */
    f64 getCurrentRangeStart() const noexcept                    { return visibleRange.getStart(); }

    /** Returns the current size of the thumb.
        @see getCurrentRange, setCurrentRange
    */
    f64 getCurrentRangeSize() const noexcept                     { return visibleRange.getLength(); }

    //==============================================================================
    /** Sets the amount by which the up and down buttons will move the bar.

        The value here is in terms of the total range, and is added or subtracted
        from the thumb position when the user clicks an up/down (or left/right) button.
    */
    z0 setSingleStepSize (f64 newSingleStepSize) noexcept;

    /** Returns the current step size.
        @see setSingleStepSize
    */
    f64 getSingleStepSize() const noexcept                       { return singleStepSize; }

    /** Moves the scrollbar by a number of single-steps.

        This will move the bar by a multiple of its single-step interval (as
        specified using the setSingleStepSize() method).

        A positive value here will move the bar down or to the right, a negative
        value moves it up or to the left.

        @param howManySteps    the number of steps to move the scrollbar
        @param notification    whether to send a notification of the change to listeners.
                               A notification will only be sent if the position has changed.

        @returns true if the scrollbar's position actually changed.
    */
    b8 moveScrollbarInSteps (i32 howManySteps,
                               NotificationType notification = sendNotificationAsync);

    /** Moves the scroll bar up or down in pages.

        This will move the bar by a multiple of its current thumb size, effectively
        doing a page-up or down.

        A positive value here will move the bar down or to the right, a negative
        value moves it up or to the left.

        @param howManyPages    the number of pages to move the scrollbar
        @param notification    whether to send a notification of the change to listeners.
                               A notification will only be sent if the position has changed.

        @returns true if the scrollbar's position actually changed.
    */
    b8 moveScrollbarInPages (i32 howManyPages,
                               NotificationType notification = sendNotificationAsync);

    /** Scrolls to the top (or left).
        This is the same as calling setCurrentRangeStart (getMinimumRangeLimit());

        @param notification    whether to send a notification of the change to listeners.
                               A notification will only be sent if the position has changed.

        @returns true if the scrollbar's position actually changed.
    */
    b8 scrollToTop (NotificationType notification = sendNotificationAsync);

    /** Scrolls to the bottom (or right).
        This is the same as calling setCurrentRangeStart (getMaximumRangeLimit() - getCurrentRangeSize());

        @param notification    whether to send a notification of the change to listeners.
                               A notification will only be sent if the position has changed.

        @returns true if the scrollbar's position actually changed.
    */
    b8 scrollToBottom (NotificationType notification = sendNotificationAsync);

    /** Changes the delay before the up and down buttons autorepeat when they are held
        down.

        For an explanation of what the parameters are for, see Button::setRepeatSpeed().

        @see Button::setRepeatSpeed
    */
    z0 setButtonRepeatSpeed (i32 initialDelayInMillisecs,
                               i32 repeatDelayInMillisecs,
                               i32 minimumDelayInMillisecs = -1);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId          = 0x1000300,    /**< The background colour of the scrollbar. */
        thumbColorId               = 0x1000400,    /**< A base colour to use for the thumb. The look and feel will probably use variations on this colour. */
        trackColorId               = 0x1000401     /**< A base colour to use for the slot area of the bar. The look and feel will probably use variations on this colour. */
    };

    //==============================================================================
    /**
        A class for receiving events from a ScrollBar.

        You can register a ScrollBar::Listener with a ScrollBar using the ScrollBar::addListener()
        method, and it will be called when the bar's position changes.

        @see ScrollBar::addListener, ScrollBar::removeListener
    */
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when a ScrollBar is moved.

            @param scrollBarThatHasMoved    the bar that has moved
            @param newRangeStart            the new range start of this bar
        */
        virtual z0 scrollBarMoved (ScrollBar* scrollBarThatHasMoved,
                                     f64 newRangeStart) = 0;
    };

    /** Registers a listener that will be called when the scrollbar is moved. */
    z0 addListener (Listener* listener);

    /** Deregisters a previously-registered listener. */
    z0 removeListener (Listener* listener);

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        scrollbar-drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual b8 areScrollbarButtonsVisible() = 0;

        /** Draws one of the buttons on a scrollbar.

            @param g                    the context to draw into
            @param scrollbar            the bar itself
            @param width                the width of the button
            @param height               the height of the button
            @param buttonDirection      the direction of the button, where 0 = up, 1 = right, 2 = down, 3 = left
            @param isScrollbarVertical  true if it's a vertical bar, false if horizontal
            @param isMouseOverButton    whether the mouse is currently over the button (also true if it's held down)
            @param isButtonDown         whether the mouse button's held down
        */
        virtual z0 drawScrollbarButton (Graphics& g,
                                          ScrollBar& scrollbar,
                                          i32 width, i32 height,
                                          i32 buttonDirection,
                                          b8 isScrollbarVertical,
                                          b8 isMouseOverButton,
                                          b8 isButtonDown) = 0;

        /** Draws the thumb area of a scrollbar.

            @param g                    the context to draw into
            @param scrollbar            the bar itself
            @param x                    the x position of the left edge of the thumb area to draw in
            @param y                    the y position of the top edge of the thumb area to draw in
            @param width                the width of the thumb area to draw in
            @param height               the height of the thumb area to draw in
            @param isScrollbarVertical  true if it's a vertical bar, false if horizontal
            @param thumbStartPosition   for vertical bars, the y coordinate of the top of the
                                        thumb, or its x position for horizontal bars
            @param thumbSize            for vertical bars, the height of the thumb, or its width for
                                        horizontal bars. This may be 0 if the thumb shouldn't be drawn.
            @param isMouseOver          whether the mouse is over the thumb area, also true if the mouse is
                                        currently dragging the thumb
            @param isMouseDown          whether the mouse is currently dragging the scrollbar
        */
        virtual z0 drawScrollbar (Graphics& g, ScrollBar& scrollbar,
                                    i32 x, i32 y, i32 width, i32 height,
                                    b8 isScrollbarVertical,
                                    i32 thumbStartPosition,
                                    i32 thumbSize,
                                    b8 isMouseOver,
                                    b8 isMouseDown) = 0;

        /** Returns the component effect to use for a scrollbar */
        virtual ImageEffectFilter* getScrollbarEffect() = 0;

        /** Returns the minimum length in pixels to use for a scrollbar thumb. */
        virtual i32 getMinimumScrollbarThumbSize (ScrollBar&) = 0;

        /** Returns the default thickness to use for a scrollbar. */
        virtual i32 getDefaultScrollbarWidth() = 0;

        /** Returns the length in pixels to use for a scrollbar button. */
        virtual i32 getScrollbarButtonSize (ScrollBar&) = 0;
    };

    //==============================================================================
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp   (const MouseEvent&) override;
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    z0 setVisible (b8) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Range<f64> totalRange { 0.0, 1.0 }, visibleRange { 0.0, 1.0 };
    f64 singleStepSize = 0.1, dragStartRange = 0;
    i32 thumbAreaStart = 0, thumbAreaSize = 0, thumbStart = 0, thumbSize = 0;
    i32 dragStartMousePos = 0, lastMousePos = 0;
    i32 initialDelayInMillisecs = 100, repeatDelayInMillisecs = 50, minimumDelayInMillisecs = 10;
    b8 vertical, isDraggingThumb = false, autohides = true, userVisibilityFlag = false;
    class ScrollbarButton;
    std::unique_ptr<ScrollbarButton> upButton, downButton;
    ListenerList<Listener> listeners;

    z0 handleAsyncUpdate() override;
    z0 updateThumbPosition();
    z0 timerCallback() override;
    b8 getVisibility() const noexcept;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollBar)
};


} // namespace drx
