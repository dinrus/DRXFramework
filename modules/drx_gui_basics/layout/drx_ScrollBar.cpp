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

class ScrollBar::ScrollbarButton final : public Button
{
public:
    ScrollbarButton (i32 direc, ScrollBar& s)
        : Button (Txt()), direction (direc), owner (s)
    {
        setWantsKeyboardFocus (false);
    }

    z0 paintButton (Graphics& g, b8 over, b8 down) override
    {
        getLookAndFeel().drawScrollbarButton (g, owner, getWidth(), getHeight(),
                                              direction, owner.isVertical(), over, down);
    }

    z0 clicked() override
    {
        owner.moveScrollbarInSteps ((direction == 1 || direction == 2) ? 1 : -1);
    }

    using Button::clicked;

    i32 direction;

private:
    ScrollBar& owner;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollbarButton)
};


//==============================================================================
ScrollBar::ScrollBar (b8 shouldBeVertical)  : vertical (shouldBeVertical)
{
    setRepaintsOnMouseActivity (true);
    setFocusContainerType (FocusContainerType::keyboardFocusContainer);
}

ScrollBar::~ScrollBar()
{
    upButton.reset();
    downButton.reset();
}

//==============================================================================
z0 ScrollBar::setRangeLimits (Range<f64> newRangeLimit, NotificationType notification)
{
    if (totalRange != newRangeLimit)
    {
        totalRange = newRangeLimit;
        setCurrentRange (visibleRange, notification);
        updateThumbPosition();
    }
}

z0 ScrollBar::setRangeLimits (f64 newMinimum, f64 newMaximum, NotificationType notification)
{
    jassert (newMaximum >= newMinimum); // these can't be the wrong way round!
    setRangeLimits (Range<f64> (newMinimum, newMaximum), notification);
}

b8 ScrollBar::setCurrentRange (Range<f64> newRange, NotificationType notification)
{
    auto constrainedRange = totalRange.constrainRange (newRange);

    if (visibleRange != constrainedRange)
    {
        visibleRange = constrainedRange;

        updateThumbPosition();

        if (notification != dontSendNotification)
            triggerAsyncUpdate();

        if (notification == sendNotificationSync)
            handleUpdateNowIfNeeded();

        return true;
    }

    return false;
}

z0 ScrollBar::setCurrentRange (f64 newStart, f64 newSize, NotificationType notification)
{
    setCurrentRange (Range<f64> (newStart, newStart + newSize), notification);
}

z0 ScrollBar::setCurrentRangeStart (f64 newStart, NotificationType notification)
{
    setCurrentRange (visibleRange.movedToStartAt (newStart), notification);
}

z0 ScrollBar::setSingleStepSize (f64 newSingleStepSize) noexcept
{
    singleStepSize = newSingleStepSize;
}

b8 ScrollBar::moveScrollbarInSteps (i32 howManySteps, NotificationType notification)
{
    return setCurrentRange (visibleRange + howManySteps * singleStepSize, notification);
}

b8 ScrollBar::moveScrollbarInPages (i32 howManyPages, NotificationType notification)
{
    return setCurrentRange (visibleRange + howManyPages * visibleRange.getLength(), notification);
}

b8 ScrollBar::scrollToTop (NotificationType notification)
{
    return setCurrentRange (visibleRange.movedToStartAt (getMinimumRangeLimit()), notification);
}

b8 ScrollBar::scrollToBottom (NotificationType notification)
{
    return setCurrentRange (visibleRange.movedToEndAt (getMaximumRangeLimit()), notification);
}

z0 ScrollBar::setButtonRepeatSpeed (i32 newInitialDelay,
                                      i32 newRepeatDelay,
                                      i32 newMinimumDelay)
{
    initialDelayInMillisecs = newInitialDelay;
    repeatDelayInMillisecs  = newRepeatDelay;
    minimumDelayInMillisecs = newMinimumDelay;

    if (upButton != nullptr)
    {
        upButton  ->setRepeatSpeed (newInitialDelay, newRepeatDelay, newMinimumDelay);
        downButton->setRepeatSpeed (newInitialDelay, newRepeatDelay, newMinimumDelay);
    }
}

//==============================================================================
z0 ScrollBar::addListener (Listener* listener)
{
    listeners.add (listener);
}

z0 ScrollBar::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

z0 ScrollBar::handleAsyncUpdate()
{
    auto start = visibleRange.getStart(); // (need to use a temp variable for VC7 compatibility)
    listeners.call ([this, start] (Listener& l) { l.scrollBarMoved (this, start); });
}

//==============================================================================
z0 ScrollBar::updateThumbPosition()
{
    auto minimumScrollBarThumbSize = getLookAndFeel().getMinimumScrollbarThumbSize (*this);

    i32 newThumbSize = roundToInt (totalRange.getLength() > 0 ? (visibleRange.getLength() * thumbAreaSize) / totalRange.getLength()
                                                              : thumbAreaSize);

    if (newThumbSize < minimumScrollBarThumbSize)
        newThumbSize = jmin (minimumScrollBarThumbSize, thumbAreaSize - 1);

    if (newThumbSize > thumbAreaSize)
        newThumbSize = thumbAreaSize;

    i32 newThumbStart = thumbAreaStart;

    if (totalRange.getLength() > visibleRange.getLength())
        newThumbStart += roundToInt (((visibleRange.getStart() - totalRange.getStart()) * (thumbAreaSize - newThumbSize))
                                         / (totalRange.getLength() - visibleRange.getLength()));

    Component::setVisible (getVisibility());

    if (thumbStart != newThumbStart  || thumbSize != newThumbSize)
    {
        auto repaintStart = jmin (thumbStart, newThumbStart) - 4;
        auto repaintSize = jmax (thumbStart + thumbSize, newThumbStart + newThumbSize) + 8 - repaintStart;

        if (vertical)
            repaint (0, repaintStart, getWidth(), repaintSize);
        else
            repaint (repaintStart, 0, repaintSize, getHeight());

        thumbStart = newThumbStart;
        thumbSize = newThumbSize;
    }
}

z0 ScrollBar::setOrientation (b8 shouldBeVertical)
{
    if (vertical != shouldBeVertical)
    {
        vertical = shouldBeVertical;

        if (upButton != nullptr)
        {
            upButton->direction    = vertical ? 0 : 3;
            downButton->direction  = vertical ? 2 : 1;
        }

        updateThumbPosition();
    }
}

z0 ScrollBar::setAutoHide (b8 shouldHideWhenFullRange)
{
    autohides = shouldHideWhenFullRange;
    updateThumbPosition();
}

b8 ScrollBar::autoHides() const noexcept
{
    return autohides;
}

//==============================================================================
z0 ScrollBar::paint (Graphics& g)
{
    if (thumbAreaSize > 0)
    {
        auto& lf = getLookAndFeel();

        auto thumb = (thumbAreaSize > lf.getMinimumScrollbarThumbSize (*this))
                       ? thumbSize : 0;

        if (vertical)
            lf.drawScrollbar (g, *this, 0, thumbAreaStart, getWidth(), thumbAreaSize,
                              vertical, thumbStart, thumb, isMouseOver(), isMouseButtonDown());
        else
            lf.drawScrollbar (g, *this, thumbAreaStart, 0, thumbAreaSize, getHeight(),
                              vertical, thumbStart, thumb, isMouseOver(), isMouseButtonDown());
    }
}

z0 ScrollBar::lookAndFeelChanged()
{
    setComponentEffect (getLookAndFeel().getScrollbarEffect());

    if (isVisible())
        resized();
}

z0 ScrollBar::resized()
{
    auto length = vertical ? getHeight() : getWidth();

    auto& lf = getLookAndFeel();
    b8 buttonsVisible = lf.areScrollbarButtonsVisible();
    i32 buttonSize = 0;

    if (buttonsVisible)
    {
        if (upButton == nullptr)
        {
            upButton  .reset (new ScrollbarButton (vertical ? 0 : 3, *this));
            downButton.reset (new ScrollbarButton (vertical ? 2 : 1, *this));

            addAndMakeVisible (upButton.get());
            addAndMakeVisible (downButton.get());

            setButtonRepeatSpeed (initialDelayInMillisecs, repeatDelayInMillisecs, minimumDelayInMillisecs);
        }

        buttonSize = jmin (lf.getScrollbarButtonSize (*this), length / 2);
    }
    else
    {
        upButton.reset();
        downButton.reset();
    }

    if (length < 32 + lf.getMinimumScrollbarThumbSize (*this))
    {
        thumbAreaStart = length / 2;
        thumbAreaSize = 0;
    }
    else
    {
        thumbAreaStart = buttonSize;
        thumbAreaSize = length - 2 * buttonSize;
    }

    if (upButton != nullptr)
    {
        auto r = getLocalBounds();

        if (vertical)
        {
            upButton->setBounds (r.removeFromTop (buttonSize));
            downButton->setBounds (r.removeFromBottom (buttonSize));
        }
        else
        {
            upButton->setBounds (r.removeFromLeft (buttonSize));
            downButton->setBounds (r.removeFromRight (buttonSize));
        }
    }

    updateThumbPosition();
}

z0 ScrollBar::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

z0 ScrollBar::mouseDown (const MouseEvent& e)
{
    isDraggingThumb = false;
    lastMousePos = vertical ? e.y : e.x;
    dragStartMousePos = lastMousePos;
    dragStartRange = visibleRange.getStart();

    if (dragStartMousePos < thumbStart)
    {
        moveScrollbarInPages (-1);
        startTimer (400);
    }
    else if (dragStartMousePos >= thumbStart + thumbSize)
    {
        moveScrollbarInPages (1);
        startTimer (400);
    }
    else
    {
        isDraggingThumb = (thumbAreaSize > getLookAndFeel().getMinimumScrollbarThumbSize (*this))
                            && (thumbAreaSize > thumbSize);
    }
}

z0 ScrollBar::mouseDrag (const MouseEvent& e)
{
    auto mousePos = vertical ? e.y : e.x;

    if (isDraggingThumb && lastMousePos != mousePos && thumbAreaSize > thumbSize)
    {
        auto deltaPixels = mousePos - dragStartMousePos;

        setCurrentRangeStart (dragStartRange
                                + deltaPixels * (totalRange.getLength() - visibleRange.getLength())
                                    / (thumbAreaSize - thumbSize));
    }

    lastMousePos = mousePos;
}

z0 ScrollBar::mouseUp (const MouseEvent&)
{
    isDraggingThumb = false;
    stopTimer();
    repaint();
}

z0 ScrollBar::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
{
    f32 increment = 10.0f * (vertical ? wheel.deltaY : wheel.deltaX);

    if (increment < 0)
        increment = jmin (increment, -1.0f);
    else if (increment > 0)
        increment = jmax (increment, 1.0f);

    setCurrentRange (visibleRange - singleStepSize * increment);
}

z0 ScrollBar::timerCallback()
{
    if (isMouseButtonDown())
    {
        startTimer (40);

        if (lastMousePos < thumbStart)
            setCurrentRange (visibleRange - visibleRange.getLength());
        else if (lastMousePos > thumbStart + thumbSize)
            setCurrentRangeStart (visibleRange.getEnd());
    }
    else
    {
        stopTimer();
    }
}

b8 ScrollBar::keyPressed (const KeyPress& key)
{
    if (isVisible())
    {
        if (key == KeyPress::upKey || key == KeyPress::leftKey)    return moveScrollbarInSteps (-1);
        if (key == KeyPress::downKey || key == KeyPress::rightKey) return moveScrollbarInSteps (1);
        if (key == KeyPress::pageUpKey)                            return moveScrollbarInPages (-1);
        if (key == KeyPress::pageDownKey)                          return moveScrollbarInPages (1);
        if (key == KeyPress::homeKey)                              return scrollToTop();
        if (key == KeyPress::endKey)                               return scrollToBottom();
    }

    return false;
}

z0 ScrollBar::setVisible (b8 shouldBeVisible)
{
    if (userVisibilityFlag != shouldBeVisible)
    {
        userVisibilityFlag = shouldBeVisible;
        Component::setVisible (getVisibility());
    }
}

b8 ScrollBar::getVisibility() const noexcept
{
    if (! userVisibilityFlag)
        return false;

    return (! autohides) || (totalRange.getLength() > visibleRange.getLength()
                                    && visibleRange.getLength() > 0.0);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ScrollBar::createAccessibilityHandler()
{
    class ValueInterface final : public AccessibilityRangedNumericValueInterface
    {
    public:
        explicit ValueInterface (ScrollBar& scrollBarToWrap)  : scrollBar (scrollBarToWrap) {}

        b8 isReadOnly() const override          { return false; }

        f64 getCurrentValue() const override   { return scrollBar.getCurrentRangeStart(); }
        z0 setValue (f64 newValue) override  { scrollBar.setCurrentRangeStart (newValue); }

        AccessibleValueRange getRange() const override
        {
            if (scrollBar.getRangeLimit().isEmpty())
                return {};

            return { { scrollBar.getMinimumRangeLimit(), scrollBar.getMaximumRangeLimit() },
                     scrollBar.getSingleStepSize() };
        }

    private:
        ScrollBar& scrollBar;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueInterface)
    };

    return std::make_unique<AccessibilityHandler> (*this,
                                                   AccessibilityRole::scrollBar,
                                                   AccessibilityActions{},
                                                   AccessibilityHandler::Interfaces { std::make_unique<ValueInterface> (*this) });
}

} // namespace drx
