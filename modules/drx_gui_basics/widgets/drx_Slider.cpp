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

static f64 getStepSize (const Slider& slider)
{
    const auto interval = slider.getInterval();

    return ! approximatelyEqual (interval, 0.0) ? interval
                                                : slider.getRange().getLength() * 0.01;
}

class Slider::Pimpl   : public AsyncUpdater, // this needs to be public otherwise it will cause an
                                             // error when DRX_DLL_BUILD=1
                        private Value::Listener
{
public:
    Pimpl (Slider& s, SliderStyle sliderStyle, TextEntryBoxPosition textBoxPosition)
      : owner (s),
        style (sliderStyle),
        textBoxPos (textBoxPosition)
    {
        rotaryParams.startAngleRadians = MathConstants<f32>::pi * 1.2f;
        rotaryParams.endAngleRadians   = MathConstants<f32>::pi * 2.8f;
        rotaryParams.stopAtEnd = true;
    }

    ~Pimpl() override
    {
        currentValue.removeListener (this);
        valueMin.removeListener (this);
        valueMax.removeListener (this);
        popupDisplay.reset();
    }

    //==============================================================================
    z0 registerListeners()
    {
        currentValue.addListener (this);
        valueMin.addListener (this);
        valueMax.addListener (this);
    }

    b8 isHorizontal() const noexcept
    {
        return style == LinearHorizontal
            || style == LinearBar
            || style == TwoValueHorizontal
            || style == ThreeValueHorizontal;
    }

    b8 isVertical() const noexcept
    {
        return style == LinearVertical
            || style == LinearBarVertical
            || style == TwoValueVertical
            || style == ThreeValueVertical;
    }

    b8 isRotary() const noexcept
    {
        return style == Rotary
            || style == RotaryHorizontalDrag
            || style == RotaryVerticalDrag
            || style == RotaryHorizontalVerticalDrag;
    }

    b8 isBar() const noexcept
    {
        return style == LinearBar
            || style == LinearBarVertical;
    }

    b8 isTwoValue() const noexcept
    {
        return style == TwoValueHorizontal
            || style == TwoValueVertical;
    }

    b8 isThreeValue() const noexcept
    {
        return style == ThreeValueHorizontal
            || style == ThreeValueVertical;
    }

    b8 incDecDragDirectionIsHorizontal() const noexcept
    {
        return incDecButtonMode == incDecButtonsDraggable_Horizontal
                || (incDecButtonMode == incDecButtonsDraggable_AutoDirection && incDecButtonsSideBySide);
    }

    f32 getPositionOfValue (f64 value) const
    {
        if (isHorizontal() || isVertical())
            return getLinearSliderPos (value);

        jassertfalse; // not a valid call on a slider that doesn't work linearly!
        return 0.0f;
    }

    z0 setNumDecimalPlacesToDisplay (i32 decimalPlacesToDisplay)
    {
        fixedNumDecimalPlaces = jmax (0, decimalPlacesToDisplay);
        numDecimalPlaces = fixedNumDecimalPlaces;
    }

    i32 getNumDecimalPlacesToDisplay() const
    {
        return fixedNumDecimalPlaces == -1 ? numDecimalPlaces : fixedNumDecimalPlaces;
    }

    z0 updateRange()
    {
        if (fixedNumDecimalPlaces == -1)
        {
            // figure out the number of DPs needed to display all values at this
            // interval setting.
            numDecimalPlaces = 7;

            if (! approximatelyEqual (normRange.interval, 0.0))
            {
                i32 v = std::abs (roundToInt (normRange.interval * 10000000));

                while ((v % 10) == 0 && numDecimalPlaces > 0)
                {
                    --numDecimalPlaces;
                    v /= 10;
                }
            }
        }

        // keep the current values inside the new range..
        if (style != TwoValueHorizontal && style != TwoValueVertical)
        {
            setValue (getValue(), dontSendNotification);
        }
        else
        {
            setMinValue (getMinValue(), dontSendNotification, false);
            setMaxValue (getMaxValue(), dontSendNotification, false);
        }

        updateText();
    }

    z0 setRange (f64 newMin, f64 newMax, f64 newInt)
    {
        normRange = NormalisableRange<f64> (newMin, newMax, newInt,
                                               normRange.skew, normRange.symmetricSkew);
        updateRange();
    }

    z0 setNormalisableRange (NormalisableRange<f64> newRange)
    {
        normRange = newRange;
        updateRange();
    }

    f64 getValue() const
    {
        // for a two-value style slider, you should use the getMinValue() and getMaxValue()
        // methods to get the two values.
        jassert (style != TwoValueHorizontal && style != TwoValueVertical);

        return currentValue.getValue();
    }

    z0 setValue (f64 newValue, NotificationType notification)
    {
        // for a two-value style slider, you should use the setMinValue() and setMaxValue()
        // methods to set the two values.
        jassert (style != TwoValueHorizontal && style != TwoValueVertical);

        newValue = constrainedValue (newValue);

        if (style == ThreeValueHorizontal || style == ThreeValueVertical)
        {
            jassert (static_cast<f64> (valueMin.getValue()) <= static_cast<f64> (valueMax.getValue()));

            newValue = jlimit (static_cast<f64> (valueMin.getValue()),
                               static_cast<f64> (valueMax.getValue()),
                               newValue);
        }

        if (! approximatelyEqual (newValue, lastCurrentValue))
        {
            if (valueBox != nullptr)
                valueBox->hideEditor (true);

            lastCurrentValue = newValue;

            // Need to do this comparison because the Value will use equalsWithSameType to compare
            // the new and old values, so will generate unwanted change events if the type changes.
            // Cast to f64 before comparing, to prevent comparing as another type (e.g. Txt).
            // We also want to avoid sending a notification if both new and old values are NaN.
            const auto asDouble = static_cast<f64> (currentValue.getValue());

            if (! (approximatelyEqual (asDouble, newValue) || (std::isnan (asDouble) && std::isnan (newValue))))
                currentValue = newValue;

            updateText();
            owner.repaint();

            triggerChangeMessage (notification);
        }
    }

    z0 setMinValue (f64 newValue, NotificationType notification, b8 allowNudgingOfOtherValues)
    {
        // The minimum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        newValue = constrainedValue (newValue);

        if (style == TwoValueHorizontal || style == TwoValueVertical)
        {
            if (allowNudgingOfOtherValues && newValue > static_cast<f64> (valueMax.getValue()))
                setMaxValue (newValue, notification, false);

            newValue = jmin (static_cast<f64> (valueMax.getValue()), newValue);
        }
        else
        {
            if (allowNudgingOfOtherValues && newValue > lastCurrentValue)
                setValue (newValue, notification);

            newValue = jmin (lastCurrentValue, newValue);
        }

        if (! approximatelyEqual (lastValueMin, newValue))
        {
            lastValueMin = newValue;
            valueMin = newValue;
            owner.repaint();
            updatePopupDisplay();

            triggerChangeMessage (notification);
        }
    }

    z0 setMaxValue (f64 newValue, NotificationType notification, b8 allowNudgingOfOtherValues)
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        newValue = constrainedValue (newValue);

        if (style == TwoValueHorizontal || style == TwoValueVertical)
        {
            if (allowNudgingOfOtherValues && newValue < static_cast<f64> (valueMin.getValue()))
                setMinValue (newValue, notification, false);

            newValue = jmax (static_cast<f64> (valueMin.getValue()), newValue);
        }
        else
        {
            if (allowNudgingOfOtherValues && newValue < lastCurrentValue)
                setValue (newValue, notification);

            newValue = jmax (lastCurrentValue, newValue);
        }

        if (! approximatelyEqual (lastValueMax, newValue))
        {
            lastValueMax = newValue;
            valueMax = newValue;
            owner.repaint();
            updatePopupDisplay();

            triggerChangeMessage (notification);
        }
    }

    z0 setMinAndMaxValues (f64 newMinValue, f64 newMaxValue, NotificationType notification)
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        if (newMaxValue < newMinValue)
            std::swap (newMaxValue, newMinValue);

        newMinValue = constrainedValue (newMinValue);
        newMaxValue = constrainedValue (newMaxValue);

        if (! approximatelyEqual (lastValueMax, newMaxValue) || ! approximatelyEqual (lastValueMin, newMinValue))
        {
            lastValueMax = newMaxValue;
            lastValueMin = newMinValue;
            valueMin = newMinValue;
            valueMax = newMaxValue;
            owner.repaint();

            triggerChangeMessage (notification);
        }
    }

    f64 getMinValue() const
    {
        // The minimum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        return valueMin.getValue();
    }

    f64 getMaxValue() const
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        return valueMax.getValue();
    }

    z0 triggerChangeMessage (NotificationType notification)
    {
        if (notification != dontSendNotification)
        {
            owner.valueChanged();

            if (notification == sendNotificationSync)
                handleAsyncUpdate();
            else
                triggerAsyncUpdate();
        }
    }

    z0 handleAsyncUpdate() override
    {
        cancelPendingUpdate();

        Component::BailOutChecker checker (&owner);
        listeners.callChecked (checker, [&] (Slider::Listener& l) { l.sliderValueChanged (&owner); });

        if (checker.shouldBailOut())
            return;

        NullCheckedInvocation::invoke (owner.onValueChange);

        if (checker.shouldBailOut())
            return;

        if (auto* handler = owner.getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::valueChanged);
    }

    z0 sendDragStart()
    {
        owner.startedDragging();

        Component::BailOutChecker checker (&owner);
        listeners.callChecked (checker, [&] (Slider::Listener& l) { l.sliderDragStarted (&owner); });

        if (checker.shouldBailOut())
            return;

        NullCheckedInvocation::invoke (owner.onDragStart);
    }

    z0 sendDragEnd()
    {
        owner.stoppedDragging();
        sliderBeingDragged = -1;

        Component::BailOutChecker checker (&owner);
        listeners.callChecked (checker, [&] (Slider::Listener& l) { l.sliderDragEnded (&owner); });

        if (checker.shouldBailOut())
            return;

        NullCheckedInvocation::invoke (owner.onDragEnd);
    }

    z0 incrementOrDecrement (f64 delta)
    {
        if (style == IncDecButtons)
        {
            auto newValue = owner.snapValue (getValue() + delta, notDragging);

            if (currentDrag != nullptr)
            {
                setValue (newValue, sendNotificationSync);
            }
            else
            {
                ScopedDragNotification drag (owner);
                setValue (newValue, sendNotificationSync);
            }
        }
    }

    z0 valueChanged (Value& value) override
    {
        if (value.refersToSameSourceAs (currentValue))
        {
            if (style != TwoValueHorizontal && style != TwoValueVertical)
                setValue (currentValue.getValue(), dontSendNotification);
        }
        else if (value.refersToSameSourceAs (valueMin))
        {
            setMinValue (valueMin.getValue(), dontSendNotification, true);
        }
        else if (value.refersToSameSourceAs (valueMax))
        {
            setMaxValue (valueMax.getValue(), dontSendNotification, true);
        }
    }

    z0 textChanged()
    {
        auto newValue = owner.snapValue (owner.getValueFromText (valueBox->getText()), notDragging);

        if (! approximatelyEqual (newValue, static_cast<f64> (currentValue.getValue())))
        {
            ScopedDragNotification drag (owner);
            setValue (newValue, sendNotificationSync);
        }

        updateText(); // force a clean-up of the text, needed in case setValue() hasn't done this.
    }

    z0 updateText()
    {
        if (valueBox != nullptr)
        {
            auto newValue = owner.getTextFromValue (currentValue.getValue());

            if (newValue != valueBox->getText())
                valueBox->setText (newValue, dontSendNotification);
        }

        updatePopupDisplay();
    }

    f64 constrainedValue (f64 value) const
    {
        return normRange.snapToLegalValue (value);
    }

    f32 getLinearSliderPos (f64 value) const
    {
        f64 pos;

        if (normRange.end <= normRange.start)
            pos = 0.5;
        else if (value < normRange.start)
            pos = 0.0;
        else if (value > normRange.end)
            pos = 1.0;
        else
            pos = owner.valueToProportionOfLength (value);

        if (isVertical() || style == IncDecButtons)
            pos = 1.0 - pos;

        jassert (pos >= 0 && pos <= 1.0);
        return (f32) (sliderRegionStart + pos * sliderRegionSize);
    }

    z0 setSliderStyle (SliderStyle newStyle)
    {
        if (style != newStyle)
        {
            style = newStyle;

            owner.repaint();
            owner.lookAndFeelChanged();
            owner.invalidateAccessibilityHandler();
        }
    }

    z0 setVelocityModeParameters (f64 sensitivity, i32 threshold,
                                    f64 offset, b8 userCanPressKeyToSwapMode,
                                    ModifierKeys::Flags newModifierToSwapModes)
    {
        velocityModeSensitivity = sensitivity;
        velocityModeOffset = offset;
        velocityModeThreshold = threshold;
        userKeyOverridesVelocity = userCanPressKeyToSwapMode;
        modifierToSwapModes = newModifierToSwapModes;
    }

    z0 setIncDecButtonsMode (IncDecButtonMode mode)
    {
        if (incDecButtonMode != mode)
        {
            incDecButtonMode = mode;
            owner.lookAndFeelChanged();
        }
    }

    z0 setTextBoxStyle (TextEntryBoxPosition newPosition,
                          b8 isReadOnly,
                          i32 textEntryBoxWidth,
                          i32 textEntryBoxHeight)
    {
        if (textBoxPos != newPosition
             || editableText != (! isReadOnly)
             || textBoxWidth != textEntryBoxWidth
             || textBoxHeight != textEntryBoxHeight)
        {
            textBoxPos = newPosition;
            editableText = ! isReadOnly;
            textBoxWidth = textEntryBoxWidth;
            textBoxHeight = textEntryBoxHeight;

            owner.repaint();
            owner.lookAndFeelChanged();
        }
    }

    z0 setTextBoxIsEditable (b8 shouldBeEditable)
    {
        editableText = shouldBeEditable;
        updateTextBoxEnablement();
    }

    z0 showTextBox()
    {
        jassert (editableText); // this should probably be avoided in read-only sliders.

        if (valueBox != nullptr)
            valueBox->showEditor();
    }

    z0 hideTextBox (b8 discardCurrentEditorContents)
    {
        if (valueBox != nullptr)
        {
            valueBox->hideEditor (discardCurrentEditorContents);

            if (discardCurrentEditorContents)
                updateText();
        }
    }

    z0 setTextValueSuffix (const Txt& suffix)
    {
        if (textSuffix != suffix)
        {
            textSuffix = suffix;
            updateText();
        }
    }

    z0 updateTextBoxEnablement()
    {
        if (valueBox != nullptr)
        {
            b8 shouldBeEditable = editableText && owner.isEnabled();

            if (valueBox->isEditable() != shouldBeEditable) // (to avoid changing the single/f64 click flags unless we need to)
                valueBox->setEditable (shouldBeEditable);
        }
    }

    z0 lookAndFeelChanged (LookAndFeel& lf)
    {
        if (textBoxPos != NoTextBox)
        {
            auto previousTextBoxContent = (valueBox != nullptr ? valueBox->getText()
                                                               : owner.getTextFromValue (currentValue.getValue()));

            valueBox.reset();
            valueBox.reset (lf.createSliderTextBox (owner));
            owner.addAndMakeVisible (valueBox.get());

            valueBox->setWantsKeyboardFocus (false);
            valueBox->setText (previousTextBoxContent, dontSendNotification);
            valueBox->setTooltip (owner.getTooltip());
            updateTextBoxEnablement();
            valueBox->onTextChange = [this] { textChanged(); };

            if (style == LinearBar || style == LinearBarVertical)
            {
                valueBox->addMouseListener (&owner, false);
                valueBox->setMouseCursor (MouseCursor::ParentCursor);
            }
        }
        else
        {
            valueBox.reset();
        }

        if (style == IncDecButtons)
        {
            incButton.reset (lf.createSliderButton (owner, true));
            decButton.reset (lf.createSliderButton (owner, false));

            auto tooltip = owner.getTooltip();

            auto setupButton = [&] (Button& b, b8 isIncrement)
            {
                owner.addAndMakeVisible (b);
                b.onClick = [this, isIncrement] { incrementOrDecrement (isIncrement ? normRange.interval : -normRange.interval); };

                if (incDecButtonMode != incDecButtonsNotDraggable)
                    b.addMouseListener (&owner, false);
                else
                    b.setRepeatSpeed (300, 100, 20);

                b.setTooltip (tooltip);
                b.setAccessible (false);
            };

            setupButton (*incButton, true);
            setupButton (*decButton, false);
        }
        else
        {
            incButton.reset();
            decButton.reset();
        }

        owner.setComponentEffect (lf.getSliderEffect (owner));

        owner.resized();
        owner.repaint();
    }

    z0 showPopupMenu()
    {
        PopupMenu m;
        m.setLookAndFeel (&owner.getLookAndFeel());
        m.addItem (1, TRANS ("Velocity-sensitive mode"), true, isVelocityBased);
        m.addSeparator();

        if (isRotary())
        {
            PopupMenu rotaryMenu;
            rotaryMenu.addItem (2, TRANS ("Use circular dragging"),           true, style == Rotary);
            rotaryMenu.addItem (3, TRANS ("Use left-right dragging"),         true, style == RotaryHorizontalDrag);
            rotaryMenu.addItem (4, TRANS ("Use up-down dragging"),            true, style == RotaryVerticalDrag);
            rotaryMenu.addItem (5, TRANS ("Use left-right/up-down dragging"), true, style == RotaryHorizontalVerticalDrag);

            m.addSubMenu (TRANS ("Rotary mode"), rotaryMenu);
        }

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (sliderMenuCallback, &owner));
    }

    static z0 sliderMenuCallback (i32 result, Slider* slider)
    {
        if (slider != nullptr)
        {
            switch (result)
            {
                case 1:   slider->setVelocityBasedMode (! slider->getVelocityBasedMode()); break;
                case 2:   slider->setSliderStyle (Rotary); break;
                case 3:   slider->setSliderStyle (RotaryHorizontalDrag); break;
                case 4:   slider->setSliderStyle (RotaryVerticalDrag); break;
                case 5:   slider->setSliderStyle (RotaryHorizontalVerticalDrag); break;
                default:  break;
            }
        }
    }

    i32 getThumbIndexAt (const MouseEvent& e)
    {
        if (isTwoValue() || isThreeValue())
        {
            auto mousePos = isVertical() ? e.position.y : e.position.x;

            auto normalPosDistance = std::abs (getLinearSliderPos (currentValue.getValue()) - mousePos);
            auto minPosDistance    = std::abs (getLinearSliderPos (valueMin.getValue()) + (isVertical() ? 0.1f : -0.1f) - mousePos);
            auto maxPosDistance    = std::abs (getLinearSliderPos (valueMax.getValue()) + (isVertical() ? -0.1f : 0.1f) - mousePos);

            if (isTwoValue())
                return maxPosDistance <= minPosDistance ? 2 : 1;

            if (normalPosDistance >= minPosDistance && maxPosDistance >= minPosDistance)
                return 1;

            if (normalPosDistance >= maxPosDistance)
                return 2;
        }

        return 0;
    }

    //==============================================================================
    z0 handleRotaryDrag (const MouseEvent& e)
    {
        auto dx = e.position.x - (f32) sliderRect.getCentreX();
        auto dy = e.position.y - (f32) sliderRect.getCentreY();

        if (dx * dx + dy * dy > 25.0f)
        {
            auto angle = std::atan2 ((f64) dx, (f64) -dy);

            while (angle < 0.0)
                angle += MathConstants<f64>::twoPi;

            if (rotaryParams.stopAtEnd && e.mouseWasDraggedSinceMouseDown())
            {
                if (std::abs (angle - lastAngle) > MathConstants<f64>::pi)
                {
                    if (angle >= lastAngle)
                        angle -= MathConstants<f64>::twoPi;
                    else
                        angle += MathConstants<f64>::twoPi;
                }

                if (angle >= lastAngle)
                    angle = jmin (angle, (f64) jmax (rotaryParams.startAngleRadians, rotaryParams.endAngleRadians));
                else
                    angle = jmax (angle, (f64) jmin (rotaryParams.startAngleRadians, rotaryParams.endAngleRadians));
            }
            else
            {
                while (angle < rotaryParams.startAngleRadians)
                    angle += MathConstants<f64>::twoPi;

                if (angle > rotaryParams.endAngleRadians)
                {
                    if (smallestAngleBetween (angle, rotaryParams.startAngleRadians)
                         <= smallestAngleBetween (angle, rotaryParams.endAngleRadians))
                        angle = rotaryParams.startAngleRadians;
                    else
                        angle = rotaryParams.endAngleRadians;
                }
            }

            auto proportion = (angle - rotaryParams.startAngleRadians) / (rotaryParams.endAngleRadians - rotaryParams.startAngleRadians);
            valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, proportion));
            lastAngle = angle;
        }
    }

    z0 handleAbsoluteDrag (const MouseEvent& e)
    {
        auto mousePos = (isHorizontal() || style == RotaryHorizontalDrag) ? e.position.x : e.position.y;
        f64 newPos = 0;

        if (style == RotaryHorizontalDrag
            || style == RotaryVerticalDrag
            || style == IncDecButtons
            || ((style == LinearHorizontal || style == LinearVertical || style == LinearBar || style == LinearBarVertical)
                && ! snapsToMousePos))
        {
            auto mouseDiff = (style == RotaryHorizontalDrag
                                || style == LinearHorizontal
                                || style == LinearBar
                                || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                              ? e.position.x - mouseDragStartPos.x
                              : mouseDragStartPos.y - e.position.y;

            newPos = owner.valueToProportionOfLength (valueOnMouseDown)
                       + mouseDiff * (1.0 / pixelsForFullDragExtent);

            if (style == IncDecButtons)
            {
                incButton->setState (mouseDiff < 0 ? Button::buttonNormal : Button::buttonDown);
                decButton->setState (mouseDiff > 0 ? Button::buttonNormal : Button::buttonDown);
            }
        }
        else if (style == RotaryHorizontalVerticalDrag)
        {
            auto mouseDiff = (e.position.x - mouseDragStartPos.x)
                               + (mouseDragStartPos.y - e.position.y);

            newPos = owner.valueToProportionOfLength (valueOnMouseDown)
                       + mouseDiff * (1.0 / pixelsForFullDragExtent);
        }
        else
        {
            newPos = (mousePos - (f32) sliderRegionStart) / (f64) sliderRegionSize;

            if (isVertical())
                newPos = 1.0 - newPos;
        }

        newPos = (isRotary() && ! rotaryParams.stopAtEnd) ? newPos - std::floor (newPos)
                                                          : jlimit (0.0, 1.0, newPos);
        valueWhenLastDragged = owner.proportionOfLengthToValue (newPos);
    }

    z0 handleVelocityDrag (const MouseEvent& e)
    {
        b8 hasHorizontalStyle =
            (isHorizontal() ||  style == RotaryHorizontalDrag
                            || (style == IncDecButtons && incDecDragDirectionIsHorizontal()));

        auto mouseDiff = style == RotaryHorizontalVerticalDrag
                            ? (e.position.x - mousePosWhenLastDragged.x) + (mousePosWhenLastDragged.y - e.position.y)
                            : (hasHorizontalStyle ? e.position.x - mousePosWhenLastDragged.x
                                                  : e.position.y - mousePosWhenLastDragged.y);

        auto maxSpeed = jmax (200.0, (f64) sliderRegionSize);
        auto speed = jlimit (0.0, maxSpeed, (f64) std::abs (mouseDiff));

        if (! approximatelyEqual (speed, 0.0))
        {
            speed = 0.2 * velocityModeSensitivity
                      * (1.0 + std::sin (MathConstants<f64>::pi * (1.5 + jmin (0.5, velocityModeOffset
                                                                                         + jmax (0.0, (f64) (speed - velocityModeThreshold))
                                                                                            / maxSpeed))));

            if (mouseDiff < 0)
                speed = -speed;

            if (isVertical() || style == RotaryVerticalDrag
                 || (style == IncDecButtons && ! incDecDragDirectionIsHorizontal()))
                speed = -speed;

            auto newPos = owner.valueToProportionOfLength (valueWhenLastDragged) + speed;
            newPos = (isRotary() && ! rotaryParams.stopAtEnd) ? newPos - std::floor (newPos)
                                                              : jlimit (0.0, 1.0, newPos);
            valueWhenLastDragged = owner.proportionOfLengthToValue (newPos);

            e.source.enableUnboundedMouseMovement (true, false);
        }
    }

    z0 mouseDown (const MouseEvent& e)
    {
        incDecDragged = false;
        useDragEvents = false;
        mouseDragStartPos = mousePosWhenLastDragged = e.position;
        currentDrag.reset();
        popupDisplay.reset();

        if (owner.isEnabled())
        {
            if (e.mods.isPopupMenu() && menuEnabled)
            {
                showPopupMenu();
            }
            else if (canDoubleClickToValue()
                     && (singleClickModifiers != ModifierKeys() && e.mods.withoutMouseButtons() == singleClickModifiers))
            {
                mouseDoubleClick();
            }
            else if (normRange.end > normRange.start)
            {
                useDragEvents = true;

                if (valueBox != nullptr)
                    valueBox->hideEditor (true);

                sliderBeingDragged = getThumbIndexAt (e);

                minMaxDiff = static_cast<f64> (valueMax.getValue()) - static_cast<f64> (valueMin.getValue());

                if (! isTwoValue())
                    lastAngle = rotaryParams.startAngleRadians
                                    + (rotaryParams.endAngleRadians - rotaryParams.startAngleRadians)
                                         * owner.valueToProportionOfLength (currentValue.getValue());

                valueWhenLastDragged = (sliderBeingDragged == 2 ? valueMax
                                                                : (sliderBeingDragged == 1 ? valueMin
                                                                                           : currentValue)).getValue();
                valueOnMouseDown = valueWhenLastDragged;

                if (showPopupOnDrag || showPopupOnHover)
                {
                    showPopupDisplay();

                    if (popupDisplay != nullptr)
                        popupDisplay->stopTimer();
                }

                currentDrag = std::make_unique<ScopedDragNotification> (owner);
                mouseDrag (e);
            }
        }
    }

    z0 mouseDrag (const MouseEvent& e)
    {
        if (useDragEvents && normRange.end > normRange.start
             && ! ((style == LinearBar || style == LinearBarVertical)
                    && e.mouseWasClicked() && valueBox != nullptr && valueBox->isEditable()))
        {
            DragMode dragMode = notDragging;

            if (style == Rotary)
            {
                handleRotaryDrag (e);
            }
            else
            {
                if (style == IncDecButtons && ! incDecDragged)
                {
                    if (e.getDistanceFromDragStart() < 10 || ! e.mouseWasDraggedSinceMouseDown())
                        return;

                    incDecDragged = true;
                    mouseDragStartPos = e.position;
                }

                if (isAbsoluteDragMode (e.mods) || (normRange.end - normRange.start) / sliderRegionSize < normRange.interval)
                {
                    dragMode = absoluteDrag;
                    handleAbsoluteDrag (e);
                }
                else
                {
                    dragMode = velocityDrag;
                    handleVelocityDrag (e);
                }
            }

            valueWhenLastDragged = jlimit (normRange.start, normRange.end, valueWhenLastDragged);

            if (sliderBeingDragged == 0)
            {
                setValue (owner.snapValue (valueWhenLastDragged, dragMode),
                          sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationSync);
            }
            else if (sliderBeingDragged == 1)
            {
                setMinValue (owner.snapValue (valueWhenLastDragged, dragMode),
                             sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationAsync, true);

                if (e.mods.isShiftDown())
                    setMaxValue (getMinValue() + minMaxDiff, dontSendNotification, true);
                else
                    minMaxDiff = static_cast<f64> (valueMax.getValue()) - static_cast<f64> (valueMin.getValue());
            }
            else if (sliderBeingDragged == 2)
            {
                setMaxValue (owner.snapValue (valueWhenLastDragged, dragMode),
                             sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationAsync, true);

                if (e.mods.isShiftDown())
                    setMinValue (getMaxValue() - minMaxDiff, dontSendNotification, true);
                else
                    minMaxDiff = static_cast<f64> (valueMax.getValue()) - static_cast<f64> (valueMin.getValue());
            }

            mousePosWhenLastDragged = e.position;
        }
    }

    z0 mouseUp()
    {
        if (owner.isEnabled()
             && useDragEvents
             && (normRange.end > normRange.start)
             && (style != IncDecButtons || incDecDragged))
        {
            restoreMouseIfHidden();

            if (sendChangeOnlyOnRelease && ! approximatelyEqual (valueOnMouseDown, static_cast<f64> (currentValue.getValue())))
                triggerChangeMessage (sendNotificationAsync);

            currentDrag.reset();
            popupDisplay.reset();

            if (style == IncDecButtons)
            {
                incButton->setState (Button::buttonNormal);
                decButton->setState (Button::buttonNormal);
            }
        }
        else if (popupDisplay != nullptr)
        {
            popupDisplay->startTimer (200);
        }

        currentDrag.reset();
    }

    z0 mouseMove()
    {
        // this is a workaround for a bug where the popup display being dismissed triggers
        // a mouse move causing it to never be hidden
        auto shouldShowPopup = showPopupOnHover
                                && (Time::getMillisecondCounterHiRes() - lastPopupDismissal) > 250;

        if (shouldShowPopup
             && ! isTwoValue()
             && ! isThreeValue())
        {
            if (owner.isMouseOver (true))
            {
                if (popupDisplay == nullptr)
                    showPopupDisplay();

                if (popupDisplay != nullptr && popupHoverTimeout != -1)
                    popupDisplay->startTimer (popupHoverTimeout);
            }
        }
    }

    z0 mouseExit()
    {
        popupDisplay.reset();
    }

    b8 keyPressed (const KeyPress& key)
    {
        if (key.getModifiers().isAnyModifierKeyDown())
            return false;

        const auto getInterval = [this]
        {
            if (auto* accessibility = owner.getAccessibilityHandler())
                if (auto* valueInterface = accessibility->getValueInterface())
                    return valueInterface->getRange().getInterval();

            return getStepSize (owner);
        };

        const auto valueChange = [&]
        {
            if (key == KeyPress::rightKey || key == KeyPress::upKey)
                return getInterval();

            if (key == KeyPress::leftKey || key == KeyPress::downKey)
                return -getInterval();

            return 0.0;
        }();

        if (approximatelyEqual (valueChange, 0.0))
            return false;

        setValue (getValue() + valueChange, sendNotificationSync);
        return true;
    }

    z0 showPopupDisplay()
    {
        if (style == IncDecButtons)
            return;

        if (popupDisplay == nullptr)
        {
            popupDisplay.reset (new PopupDisplayComponent (owner, parentForPopupDisplay == nullptr));

            if (parentForPopupDisplay != nullptr)
                parentForPopupDisplay->addChildComponent (popupDisplay.get());
            else
                popupDisplay->addToDesktop (ComponentPeer::windowIsTemporary
                                            | ComponentPeer::windowIgnoresKeyPresses
                                            | ComponentPeer::windowIgnoresMouseClicks);

            updatePopupDisplay();
            popupDisplay->setVisible (true);
        }
    }

    z0 updatePopupDisplay()
    {
        if (popupDisplay == nullptr)
            return;

        const auto valueToShow = [this]
        {
            constexpr SliderStyle multiSliderStyles[] { SliderStyle::TwoValueHorizontal,
                                                        SliderStyle::TwoValueVertical,
                                                        SliderStyle::ThreeValueHorizontal,
                                                        SliderStyle::ThreeValueVertical };

            if (std::find (std::begin (multiSliderStyles), std::end (multiSliderStyles), style) == std::end (multiSliderStyles))
                return getValue();

            if (sliderBeingDragged == 2)
                return getMaxValue();

            if (sliderBeingDragged == 1)
                return getMinValue();

            return getValue();
        }();

        popupDisplay->updatePosition (owner.getTextFromValue (valueToShow));
    }

    b8 canDoubleClickToValue() const
    {
        return doubleClickToValue
                && style != IncDecButtons
                && normRange.start <= doubleClickReturnValue
                && normRange.end >= doubleClickReturnValue;
    }

    z0 mouseDoubleClick()
    {
        if (canDoubleClickToValue())
        {
            ScopedDragNotification drag (owner);
            setValue (doubleClickReturnValue, sendNotificationSync);
        }
    }

    f64 getMouseWheelDelta (f64 value, f64 wheelAmount)
    {
        if (style == IncDecButtons)
            return normRange.interval * wheelAmount;

        auto proportionDelta = wheelAmount * 0.15;
        auto currentPos = owner.valueToProportionOfLength (value);
        auto newPos = currentPos + proportionDelta;
        newPos = (isRotary() && ! rotaryParams.stopAtEnd) ? newPos - std::floor (newPos)
                                                          : jlimit (0.0, 1.0, newPos);
        return owner.proportionOfLengthToValue (newPos) - value;
    }

    b8 mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
    {
        if (scrollWheelEnabled
             && style != TwoValueHorizontal
             && style != TwoValueVertical)
        {
            // sometimes duplicate wheel events seem to be sent, so since we're going to
            // bump the value by a minimum of the interval, avoid doing this twice..
            if (e.eventTime != lastMouseWheelTime)
            {
                lastMouseWheelTime = e.eventTime;

                if (normRange.end > normRange.start && ! e.mods.isAnyMouseButtonDown())
                {
                    if (valueBox != nullptr)
                        valueBox->hideEditor (false);

                    auto value = static_cast<f64> (currentValue.getValue());
                    auto delta = getMouseWheelDelta (value, (std::abs (wheel.deltaX) > std::abs (wheel.deltaY)
                                                                  ? -wheel.deltaX : wheel.deltaY)
                                                               * (wheel.isReversed ? -1.0f : 1.0f));
                    if (! approximatelyEqual (delta, 0.0))
                    {
                        auto newValue = value + jmax (normRange.interval, std::abs (delta)) * (delta < 0 ? -1.0 : 1.0);

                        ScopedDragNotification drag (owner);
                        setValue (owner.snapValue (newValue, notDragging), sendNotificationSync);
                    }
                }
            }

            return true;
        }

        return false;
    }

    z0 modifierKeysChanged (const ModifierKeys& modifiers)
    {
        if (style != IncDecButtons && style != Rotary && isAbsoluteDragMode (modifiers))
            restoreMouseIfHidden();
    }

    b8 isAbsoluteDragMode (ModifierKeys mods) const
    {
        return isVelocityBased == (userKeyOverridesVelocity && mods.testFlags (modifierToSwapModes));
    }

    z0 restoreMouseIfHidden()
    {
        for (auto& ms : Desktop::getInstance().getMouseSources())
        {
            if (ms.isUnboundedMouseMovementEnabled())
            {
                ms.enableUnboundedMouseMovement (false);

                auto pos = sliderBeingDragged == 2 ? getMaxValue()
                                                   : (sliderBeingDragged == 1 ? getMinValue()
                                                                              : static_cast<f64> (currentValue.getValue()));
                Point<f32> mousePos;

                if (isRotary())
                {
                    mousePos = ms.getLastMouseDownPosition();

                    auto delta = (f32) (pixelsForFullDragExtent * (owner.valueToProportionOfLength (valueOnMouseDown)
                                                                       - owner.valueToProportionOfLength (pos)));

                    if (style == RotaryHorizontalDrag)      mousePos += Point<f32> (-delta, 0.0f);
                    else if (style == RotaryVerticalDrag)   mousePos += Point<f32> (0.0f, delta);
                    else                                    mousePos += Point<f32> (delta / -2.0f, delta / 2.0f);

                    mousePos = owner.getScreenBounds().reduced (4).toFloat().getConstrainedPoint (mousePos);
                    mouseDragStartPos = mousePosWhenLastDragged = owner.getLocalPoint (nullptr, mousePos);
                    valueOnMouseDown = valueWhenLastDragged;
                }
                else
                {
                    auto pixelPos = (f32) getLinearSliderPos (pos);

                    mousePos = owner.localPointToGlobal (Point<f32> (isHorizontal() ? pixelPos : ((f32) owner.getWidth()  / 2.0f),
                                                                       isVertical()   ? pixelPos : ((f32) owner.getHeight() / 2.0f)));
                }

                const_cast <MouseInputSource&> (ms).setScreenPosition (mousePos);
            }
        }
    }

    //==============================================================================
    z0 paint (Graphics& g, LookAndFeel& lf)
    {
        if (style != IncDecButtons)
        {
            if (isRotary())
            {
                auto sliderPos = (f32) owner.valueToProportionOfLength (lastCurrentValue);
                jassert (sliderPos >= 0 && sliderPos <= 1.0f);

                lf.drawRotarySlider (g,
                                     sliderRect.getX(), sliderRect.getY(),
                                     sliderRect.getWidth(), sliderRect.getHeight(),
                                     sliderPos, rotaryParams.startAngleRadians,
                                     rotaryParams.endAngleRadians, owner);
            }
            else
            {
                lf.drawLinearSlider (g,
                                     sliderRect.getX(), sliderRect.getY(),
                                     sliderRect.getWidth(), sliderRect.getHeight(),
                                     getLinearSliderPos (lastCurrentValue),
                                     getLinearSliderPos (lastValueMin),
                                     getLinearSliderPos (lastValueMax),
                                     style, owner);
            }
        }
    }

    //==============================================================================
    z0 resized (LookAndFeel& lf)
    {
        auto layout = lf.getSliderLayout (owner);
        sliderRect = layout.sliderBounds;

        if (valueBox != nullptr)
            valueBox->setBounds (layout.textBoxBounds);

        if (isHorizontal())
        {
            sliderRegionStart = layout.sliderBounds.getX();
            sliderRegionSize = layout.sliderBounds.getWidth();
        }
        else if (isVertical())
        {
            sliderRegionStart = layout.sliderBounds.getY();
            sliderRegionSize = layout.sliderBounds.getHeight();
        }
        else if (style == IncDecButtons)
        {
            resizeIncDecButtons();
        }
    }

    //==============================================================================
    z0 resizeIncDecButtons()
    {
        auto buttonRect = sliderRect;

        if (textBoxPos == TextBoxLeft || textBoxPos == TextBoxRight)
            buttonRect.expand (-2, 0);
        else
            buttonRect.expand (0, -2);

        incDecButtonsSideBySide = buttonRect.getWidth() > buttonRect.getHeight();

        if (incDecButtonsSideBySide)
        {
            decButton->setBounds (buttonRect.removeFromLeft (buttonRect.getWidth() / 2));
            decButton->setConnectedEdges (Button::ConnectedOnRight);
            incButton->setConnectedEdges (Button::ConnectedOnLeft);
        }
        else
        {
            decButton->setBounds (buttonRect.removeFromBottom (buttonRect.getHeight() / 2));
            decButton->setConnectedEdges (Button::ConnectedOnTop);
            incButton->setConnectedEdges (Button::ConnectedOnBottom);
        }

        incButton->setBounds (buttonRect);
    }

    //==============================================================================
    Slider& owner;
    SliderStyle style;

    ListenerList<Slider::Listener> listeners;
    Value currentValue, valueMin, valueMax;
    f64 lastCurrentValue = 0, lastValueMin = 0, lastValueMax = 0;
    NormalisableRange<f64> normRange { 0.0, 10.0 };
    f64 doubleClickReturnValue = 0;
    f64 valueWhenLastDragged = 0, valueOnMouseDown = 0, lastAngle = 0;
    f64 velocityModeSensitivity = 1.0, velocityModeOffset = 0, minMaxDiff = 0;
    i32 velocityModeThreshold = 1;
    RotaryParameters rotaryParams;
    Point<f32> mouseDragStartPos, mousePosWhenLastDragged;
    i32 sliderRegionStart = 0, sliderRegionSize = 1;
    i32 sliderBeingDragged = -1;
    i32 pixelsForFullDragExtent = 250;
    Time lastMouseWheelTime;
    Rectangle<i32> sliderRect;
    std::unique_ptr<ScopedDragNotification> currentDrag;

    TextEntryBoxPosition textBoxPos;
    Txt textSuffix;
    i32 numDecimalPlaces = 7;
    i32 fixedNumDecimalPlaces = -1;
    i32 textBoxWidth = 80, textBoxHeight = 20;
    IncDecButtonMode incDecButtonMode = incDecButtonsNotDraggable;
    ModifierKeys::Flags modifierToSwapModes = ModifierKeys::ctrlAltCommandModifiers;

    b8 editableText = true;
    b8 doubleClickToValue = false;
    b8 isVelocityBased = false;
    b8 userKeyOverridesVelocity = true;
    b8 incDecButtonsSideBySide = false;
    b8 sendChangeOnlyOnRelease = false;
    b8 showPopupOnDrag = false;
    b8 showPopupOnHover = false;
    b8 menuEnabled = false;
    b8 useDragEvents = false;
    b8 incDecDragged = false;
    b8 scrollWheelEnabled = true;
    b8 snapsToMousePos = true;

    i32 popupHoverTimeout = 2000;
    f64 lastPopupDismissal = 0.0;

    ModifierKeys singleClickModifiers;

    std::unique_ptr<Label> valueBox;
    std::unique_ptr<Button> incButton, decButton;

    //==============================================================================
    struct PopupDisplayComponent final : public BubbleComponent,
                                         public Timer
    {
        PopupDisplayComponent (Slider& s, b8 isOnDesktop)
            : owner (s),
              font (s.getLookAndFeel().getSliderPopupFont (s))
        {
            if (isOnDesktop)
                setTransform (AffineTransform::scale (Component::getApproximateScaleFactorForComponent (&s)));

            setAlwaysOnTop (true);
            setAllowedPlacement (owner.getLookAndFeel().getSliderPopupPlacement (s));
            setLookAndFeel (&s.getLookAndFeel());
        }

        ~PopupDisplayComponent() override
        {
            if (owner.pimpl != nullptr)
                owner.pimpl->lastPopupDismissal = Time::getMillisecondCounterHiRes();
        }

        z0 paintContent (Graphics& g, i32 w, i32 h) override
        {
            g.setFont (font);
            g.setColor (owner.findColor (TooltipWindow::textColorId, true));
            g.drawFittedText (text, Rectangle<i32> (w, h), Justification::centred, 1);
        }

        z0 getContentSize (i32& w, i32& h) override
        {
            w = GlyphArrangement::getStringWidthInt (font, text) + 18;
            h = (i32) (font.getHeight() * 1.6f);
        }

        z0 updatePosition (const Txt& newText)
        {
            text = newText;
            BubbleComponent::setPosition (&owner);
            repaint();
        }

        z0 timerCallback() override
        {
            stopTimer();
            owner.pimpl->popupDisplay.reset();
        }

    private:
        //==============================================================================
        Slider& owner;
        Font font;
        Txt text;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupDisplayComponent)
    };

    std::unique_ptr<PopupDisplayComponent> popupDisplay;
    Component* parentForPopupDisplay = nullptr;

    //==============================================================================
    static f64 smallestAngleBetween (f64 a1, f64 a2) noexcept
    {
        return jmin (std::abs (a1 - a2),
                     std::abs (a1 + MathConstants<f64>::twoPi - a2),
                     std::abs (a2 + MathConstants<f64>::twoPi - a1));
    }
};

//==============================================================================
Slider::ScopedDragNotification::ScopedDragNotification (Slider& s)
    : sliderBeingDragged (s)
{
    sliderBeingDragged.pimpl->sendDragStart();
}

Slider::ScopedDragNotification::~ScopedDragNotification()
{
    if (sliderBeingDragged.pimpl != nullptr)
        sliderBeingDragged.pimpl->sendDragEnd();
}

//==============================================================================
Slider::Slider()
{
    init (LinearHorizontal, TextBoxLeft);
}

Slider::Slider (const Txt& name)  : Component (name)
{
    init (LinearHorizontal, TextBoxLeft);
}

Slider::Slider (SliderStyle style, TextEntryBoxPosition textBoxPos)
{
    init (style, textBoxPos);
}

z0 Slider::init (SliderStyle style, TextEntryBoxPosition textBoxPos)
{
    setWantsKeyboardFocus (false);
    setRepaintsOnMouseActivity (true);

    pimpl.reset (new Pimpl (*this, style, textBoxPos));

    Slider::lookAndFeelChanged();
    updateText();

    pimpl->registerListeners();
}

Slider::~Slider() {}

//==============================================================================
z0 Slider::addListener (Listener* l)       { pimpl->listeners.add (l); }
z0 Slider::removeListener (Listener* l)    { pimpl->listeners.remove (l); }

//==============================================================================
Slider::SliderStyle Slider::getSliderStyle() const noexcept     { return pimpl->style; }
z0 Slider::setSliderStyle (SliderStyle newStyle)              { pimpl->setSliderStyle (newStyle); }

z0 Slider::setRotaryParameters (RotaryParameters p) noexcept
{
    // make sure the values are sensible..
    jassert (p.startAngleRadians >= 0 && p.endAngleRadians >= 0);
    jassert (p.startAngleRadians < MathConstants<f32>::pi * 4.0f
              && p.endAngleRadians < MathConstants<f32>::pi * 4.0f);

    pimpl->rotaryParams = p;
}

z0 Slider::setRotaryParameters (f32 startAngleRadians, f32 endAngleRadians, b8 stopAtEnd) noexcept
{
    setRotaryParameters ({ startAngleRadians, endAngleRadians, stopAtEnd });
}

Slider::RotaryParameters Slider::getRotaryParameters() const noexcept
{
    return pimpl->rotaryParams;
}

z0 Slider::setVelocityBasedMode (b8 vb)                 { pimpl->isVelocityBased = vb; }
b8 Slider::getVelocityBasedMode() const noexcept          { return pimpl->isVelocityBased; }
b8 Slider::getVelocityModeIsSwappable() const noexcept    { return pimpl->userKeyOverridesVelocity; }
i32 Slider::getVelocityThreshold() const noexcept           { return pimpl->velocityModeThreshold; }
f64 Slider::getVelocitySensitivity() const noexcept      { return pimpl->velocityModeSensitivity; }
f64 Slider::getVelocityOffset() const noexcept           { return pimpl->velocityModeOffset; }

z0 Slider::setVelocityModeParameters (f64 sensitivity, i32 threshold,
                                        f64 offset, b8 userCanPressKeyToSwapMode,
                                        ModifierKeys::Flags modifierToSwapModes)
{
    jassert (threshold >= 0);
    jassert (sensitivity > 0);
    jassert (offset >= 0);

    pimpl->setVelocityModeParameters (sensitivity, threshold, offset,
                                      userCanPressKeyToSwapMode, modifierToSwapModes);
}

f64 Slider::getSkewFactor() const noexcept               { return pimpl->normRange.skew; }
b8 Slider::isSymmetricSkew() const noexcept               { return pimpl->normRange.symmetricSkew; }

z0 Slider::setSkewFactor (f64 factor, b8 symmetricSkew)
{
    pimpl->normRange.skew = factor;
    pimpl->normRange.symmetricSkew = symmetricSkew;
}

z0 Slider::setSkewFactorFromMidPoint (f64 sliderValueToShowAtMidPoint)
{
    pimpl->normRange.setSkewForCentre (sliderValueToShowAtMidPoint);
}

i32 Slider::getMouseDragSensitivity() const noexcept        { return pimpl->pixelsForFullDragExtent; }

z0 Slider::setMouseDragSensitivity (i32 distanceForFullScaleDrag)
{
    jassert (distanceForFullScaleDrag > 0);

    pimpl->pixelsForFullDragExtent = distanceForFullScaleDrag;
}

z0 Slider::setIncDecButtonsMode (IncDecButtonMode mode)                   { pimpl->setIncDecButtonsMode (mode); }

Slider::TextEntryBoxPosition Slider::getTextBoxPosition() const noexcept    { return pimpl->textBoxPos; }
i32 Slider::getTextBoxWidth() const noexcept                                { return pimpl->textBoxWidth; }
i32 Slider::getTextBoxHeight() const noexcept                               { return pimpl->textBoxHeight; }

z0 Slider::setTextBoxStyle (TextEntryBoxPosition newPosition, b8 isReadOnly, i32 textEntryBoxWidth, i32 textEntryBoxHeight)
{
    pimpl->setTextBoxStyle (newPosition, isReadOnly, textEntryBoxWidth, textEntryBoxHeight);
}

b8 Slider::isTextBoxEditable() const noexcept                     { return pimpl->editableText; }
z0 Slider::setTextBoxIsEditable (const b8 shouldBeEditable)     { pimpl->setTextBoxIsEditable (shouldBeEditable); }
z0 Slider::showTextBox()                                          { pimpl->showTextBox(); }
z0 Slider::hideTextBox (b8 discardCurrentEditorContents)        { pimpl->hideTextBox (discardCurrentEditorContents); }

z0 Slider::setChangeNotificationOnlyOnRelease (b8 onlyNotifyOnRelease)
{
    pimpl->sendChangeOnlyOnRelease = onlyNotifyOnRelease;
}

b8 Slider::getSliderSnapsToMousePosition() const noexcept           { return pimpl->snapsToMousePos; }
z0 Slider::setSliderSnapsToMousePosition (b8 shouldSnapToMouse)   { pimpl->snapsToMousePos = shouldSnapToMouse; }

z0 Slider::setPopupDisplayEnabled (b8 showOnDrag, b8 showOnHover, Component* parent, i32 hoverTimeout)
{
    pimpl->showPopupOnDrag = showOnDrag;
    pimpl->showPopupOnHover = showOnHover;
    pimpl->parentForPopupDisplay = parent;
    pimpl->popupHoverTimeout = hoverTimeout;
}

Component* Slider::getCurrentPopupDisplay() const noexcept      { return pimpl->popupDisplay.get(); }

//==============================================================================
z0 Slider::colourChanged()        { lookAndFeelChanged(); }
z0 Slider::lookAndFeelChanged()   { pimpl->lookAndFeelChanged (getLookAndFeel()); }
z0 Slider::enablementChanged()    { repaint(); pimpl->updateTextBoxEnablement(); }

//==============================================================================
NormalisableRange<f64> Slider::getNormalisableRange() const noexcept { return pimpl->normRange; }
Range<f64> Slider::getRange() const noexcept                         { return { pimpl->normRange.start, pimpl->normRange.end }; }
f64 Slider::getMaximum() const noexcept                              { return pimpl->normRange.end; }
f64 Slider::getMinimum() const noexcept                              { return pimpl->normRange.start; }
f64 Slider::getInterval() const noexcept                             { return pimpl->normRange.interval; }

z0 Slider::setRange (f64 newMin, f64 newMax, f64 newInt)      { pimpl->setRange (newMin, newMax, newInt); }
z0 Slider::setRange (Range<f64> newRange, f64 newInt)            { pimpl->setRange (newRange.getStart(), newRange.getEnd(), newInt); }
z0 Slider::setNormalisableRange (NormalisableRange<f64> newRange)   { pimpl->setNormalisableRange (newRange); }

f64 Slider::getValue() const                  { return pimpl->getValue(); }
Value& Slider::getValueObject() noexcept         { return pimpl->currentValue; }
Value& Slider::getMinValueObject() noexcept      { return pimpl->valueMin; }
Value& Slider::getMaxValueObject() noexcept      { return pimpl->valueMax; }

z0 Slider::setValue (f64 newValue, NotificationType notification)
{
    pimpl->setValue (newValue, notification);
}

f64 Slider::getMinValue() const      { return pimpl->getMinValue(); }
f64 Slider::getMaxValue() const      { return pimpl->getMaxValue(); }

z0 Slider::setMinValue (f64 newValue, NotificationType notification, b8 allowNudgingOfOtherValues)
{
    pimpl->setMinValue (newValue, notification, allowNudgingOfOtherValues);
}

z0 Slider::setMaxValue (f64 newValue, NotificationType notification, b8 allowNudgingOfOtherValues)
{
    pimpl->setMaxValue (newValue, notification, allowNudgingOfOtherValues);
}

z0 Slider::setMinAndMaxValues (f64 newMinValue, f64 newMaxValue, NotificationType notification)
{
    pimpl->setMinAndMaxValues (newMinValue, newMaxValue, notification);
}

z0 Slider::setDoubleClickReturnValue (b8 isDoubleClickEnabled,  f64 valueToSetOnDoubleClick, ModifierKeys mods)
{
    pimpl->doubleClickToValue = isDoubleClickEnabled;
    pimpl->doubleClickReturnValue = valueToSetOnDoubleClick;
    pimpl->singleClickModifiers = mods;
}

f64 Slider::getDoubleClickReturnValue() const noexcept       { return pimpl->doubleClickReturnValue; }
b8 Slider::isDoubleClickReturnEnabled() const noexcept        { return pimpl->doubleClickToValue; }

z0 Slider::updateText()
{
    pimpl->updateText();
}

z0 Slider::setTextValueSuffix (const Txt& suffix)
{
    pimpl->setTextValueSuffix (suffix);
}

Txt Slider::getTextValueSuffix() const
{
    return pimpl->textSuffix;
}

Txt Slider::getTextFromValue (f64 v)
{
    auto getText = [this] (f64 val)
    {
        if (textFromValueFunction != nullptr)
            return textFromValueFunction (val);

        if (getNumDecimalPlacesToDisplay() > 0)
            return Txt (val, getNumDecimalPlacesToDisplay());

        return Txt (roundToInt (val));
    };

    return getText (v) + getTextValueSuffix();
}

f64 Slider::getValueFromText (const Txt& text)
{
    auto t = text.trimStart();

    if (t.endsWith (getTextValueSuffix()))
        t = t.substring (0, t.length() - getTextValueSuffix().length());

    if (valueFromTextFunction != nullptr)
        return valueFromTextFunction (t);

    while (t.startsWithChar ('+'))
        t = t.substring (1).trimStart();

    return t.initialSectionContainingOnly ("0123456789.,-")
            .getDoubleValue();
}

f64 Slider::proportionOfLengthToValue (f64 proportion)
{
    return pimpl->normRange.convertFrom0to1 (proportion);
}

f64 Slider::valueToProportionOfLength (f64 value)
{
    return pimpl->normRange.convertTo0to1 (value);
}

f64 Slider::snapValue (f64 attemptedValue, DragMode)
{
    return attemptedValue;
}

i32 Slider::getNumDecimalPlacesToDisplay() const noexcept
{
    return pimpl->getNumDecimalPlacesToDisplay();
}

z0 Slider::setNumDecimalPlacesToDisplay (i32 decimalPlacesToDisplay)
{
    pimpl->setNumDecimalPlacesToDisplay (decimalPlacesToDisplay);
    updateText();
}

//==============================================================================
i32 Slider::getThumbBeingDragged() const noexcept           { return pimpl->sliderBeingDragged; }
z0 Slider::startedDragging() {}
z0 Slider::stoppedDragging() {}
z0 Slider::valueChanged() {}

//==============================================================================
z0 Slider::setPopupMenuEnabled (b8 menuEnabled)         { pimpl->menuEnabled = menuEnabled; }
z0 Slider::setScrollWheelEnabled (b8 enabled)           { pimpl->scrollWheelEnabled = enabled; }

b8 Slider::isScrollWheelEnabled() const noexcept          { return pimpl->scrollWheelEnabled; }
b8 Slider::isHorizontal() const noexcept                  { return pimpl->isHorizontal(); }
b8 Slider::isVertical() const noexcept                    { return pimpl->isVertical(); }
b8 Slider::isRotary() const noexcept                      { return pimpl->isRotary(); }
b8 Slider::isBar() const noexcept                         { return pimpl->isBar(); }
b8 Slider::isTwoValue() const noexcept                    { return pimpl->isTwoValue(); }
b8 Slider::isThreeValue() const noexcept                  { return pimpl->isThreeValue(); }

f32 Slider::getPositionOfValue (f64 value) const       { return pimpl->getPositionOfValue (value); }

//==============================================================================
z0 Slider::paint (Graphics& g)        { pimpl->paint (g, getLookAndFeel()); }
z0 Slider::resized()                  { pimpl->resized (getLookAndFeel()); }

z0 Slider::focusOfChildComponentChanged (FocusChangeType)     { repaint(); }

z0 Slider::mouseDown (const MouseEvent& e)    { pimpl->mouseDown (e); }
z0 Slider::mouseUp   (const MouseEvent&)      { pimpl->mouseUp(); }
z0 Slider::mouseMove (const MouseEvent&)      { pimpl->mouseMove(); }
z0 Slider::mouseExit (const MouseEvent&)      { pimpl->mouseExit(); }

// If popup display is enabled and set to show on mouse hover, this makes sure
// it is shown when dragging the mouse over a slider and releasing
z0 Slider::mouseEnter (const MouseEvent&)     { pimpl->mouseMove(); }

/** @internal */
b8 Slider::keyPressed (const KeyPress& k)     { return pimpl->keyPressed (k); }

z0 Slider::modifierKeysChanged (const ModifierKeys& modifiers)
{
    if (isEnabled())
        pimpl->modifierKeysChanged (modifiers);
}

z0 Slider::mouseDrag (const MouseEvent& e)
{
    if (isEnabled())
        pimpl->mouseDrag (e);
}

z0 Slider::mouseDoubleClick (const MouseEvent&)
{
    if (isEnabled())
        pimpl->mouseDoubleClick();
}

z0 Slider::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! (isEnabled() && pimpl->mouseWheelMove (e, wheel)))
        Component::mouseWheelMove (e, wheel);
}

//==============================================================================
class SliderAccessibilityHandler final : public AccessibilityHandler
{
public:
    explicit SliderAccessibilityHandler (Slider& sliderToWrap)
        : AccessibilityHandler (sliderToWrap,
                                AccessibilityRole::slider,
                                AccessibilityActions{},
                                AccessibilityHandler::Interfaces { std::make_unique<ValueInterface> (sliderToWrap) }),
          slider (sliderToWrap)
    {
    }

    Txt getHelp() const override   { return slider.getTooltip(); }

private:
    class ValueInterface final : public AccessibilityValueInterface
    {
    public:
        explicit ValueInterface (Slider& sliderToWrap)
            : slider (sliderToWrap),
              useMaxValue (slider.isTwoValue())
        {
        }

        b8 isReadOnly() const override  { return false; }

        f64 getCurrentValue() const override
        {
            return useMaxValue ? slider.getMaximum()
                               : slider.getValue();
        }

        z0 setValue (f64 newValue) override
        {
            Slider::ScopedDragNotification drag (slider);

            if (useMaxValue)
                slider.setMaxValue (newValue, sendNotificationSync);
            else
                slider.setValue (newValue, sendNotificationSync);
        }

        Txt getCurrentValueAsString() const override          { return slider.getTextFromValue (getCurrentValue()); }
        z0 setValueAsString (const Txt& newValue) override  { setValue (slider.getValueFromText (newValue)); }

        AccessibleValueRange getRange() const override
        {
            return { { slider.getMinimum(), slider.getMaximum() },
                     getStepSize (slider) };
        }

    private:
        Slider& slider;
        const b8 useMaxValue;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueInterface)
    };

    Slider& slider;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderAccessibilityHandler)
};

std::unique_ptr<AccessibilityHandler> Slider::createAccessibilityHandler()
{
    return std::make_unique<SliderAccessibilityHandler> (*this);
}

} // namespace drx
