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

ProgressBar::ProgressBar (f64& progress_, std::optional<Style> style_)
   : progress { progress_ },
     style { style_ }
{
}

ProgressBar::ProgressBar (f64& progress_)
   : progress { progress_ }
{
}

//==============================================================================
z0 ProgressBar::setPercentageDisplay (const b8 shouldDisplayPercentage)
{
    displayPercentage = shouldDisplayPercentage;
    repaint();
}

z0 ProgressBar::setTextToDisplay (const Txt& text)
{
    displayPercentage = false;
    displayedMessage = text;
}

z0 ProgressBar::setStyle (std::optional<Style> newStyle)
{
    style = newStyle;
    repaint();
}

ProgressBar::Style ProgressBar::getResolvedStyle() const
{
    return style.value_or (getLookAndFeel().getDefaultProgressBarStyle (*this));
}

z0 ProgressBar::lookAndFeelChanged()
{
    setOpaque (getLookAndFeel().isProgressBarOpaque (*this));
}

z0 ProgressBar::colourChanged()
{
    lookAndFeelChanged();
    repaint();
}

z0 ProgressBar::paint (Graphics& g)
{
    Txt text;

    if (displayPercentage)
    {
        if (currentValue >= 0 && currentValue <= 1.0)
            text << roundToInt (currentValue * 100.0) << '%';
    }
    else
    {
        text = displayedMessage;
    }

    const auto w = getWidth();
    const auto h = getHeight();
    const auto v = currentValue;

    getLookAndFeel().drawProgressBar (g, *this, w, h, v, text);
}

z0 ProgressBar::visibilityChanged()
{
    if (isVisible())
        startTimer (30);
    else
        stopTimer();
}

z0 ProgressBar::timerCallback()
{
    f64 newProgress = progress;

    u32k now = Time::getMillisecondCounter();
    i32k timeSinceLastCallback = (i32) (now - lastCallbackTime);
    lastCallbackTime = now;

    if (! approximatelyEqual (currentValue, newProgress)
         || newProgress < 0 || newProgress >= 1.0
         || currentMessage != displayedMessage)
    {
        if (currentValue < newProgress
             && newProgress >= 0 && newProgress < 1.0
             && currentValue >= 0 && currentValue < 1.0)
        {
            newProgress = jmin (currentValue + 0.0008 * timeSinceLastCallback,
                                newProgress);
        }

        currentValue = newProgress;
        currentMessage = displayedMessage;
        repaint();

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::valueChanged);
    }
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ProgressBar::createAccessibilityHandler()
{
    class ProgressBarAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        explicit ProgressBarAccessibilityHandler (ProgressBar& progressBarToWrap)
            : AccessibilityHandler (progressBarToWrap,
                                    AccessibilityRole::progressBar,
                                    AccessibilityActions{},
                                    AccessibilityHandler::Interfaces { std::make_unique<ValueInterface> (progressBarToWrap) }),
              progressBar (progressBarToWrap)
        {
        }

        Txt getHelp() const override   { return progressBar.getTooltip(); }

    private:
        class ValueInterface final : public AccessibilityRangedNumericValueInterface
        {
        public:
            explicit ValueInterface (ProgressBar& progressBarToWrap)
                : progressBar (progressBarToWrap)
            {
            }

            b8 isReadOnly() const override                { return true; }
            z0 setValue (f64) override                 { jassertfalse; }
            f64 getCurrentValue() const override         { return progressBar.progress; }
            AccessibleValueRange getRange() const override  { return { { 0.0, 1.0 }, 0.001 }; }

        private:
            ProgressBar& progressBar;

            //==============================================================================
            DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueInterface)
        };

        ProgressBar& progressBar;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgressBarAccessibilityHandler)
    };

    return std::make_unique<ProgressBarAccessibilityHandler> (*this);
}

} // namespace drx
