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
    A progress bar component.

    To use this, just create one and make it visible. It'll run its own timer to keep an eye on a
    variable that you give it, and will automatically redraw itself when the variable changes.

    Two styles of progress bars are supported: circular, and linear bar. If a style isn't given the
    look-and-feel will determine the style based on getDefaultProgressBarStyle().

    For an easy way of running a background task with a dialog box showing its progress, see
    the ThreadWithProgressWindow class.

    @see ThreadWithProgressWindow

    @tags{GUI}
*/
class DRX_API  ProgressBar  : public Component,
                               public SettableTooltipClient,
                               private Timer
{
public:
    /** The types of ProgressBar styles available.

        @see setStyle, getStyle, getResolvedStyle
    */
    enum class Style
    {
        linear,     /**< A linear progress bar. */
        circular,   /**< A circular progress indicator. */
    };

    //==============================================================================
    /** Creates a ProgressBar.

        The ProgressBar's style will initially be determined by the look-and-feel.

        @param progress     pass in a reference to a f64 that you're going to
                            update with your task's progress. The ProgressBar will
                            monitor the value of this variable and will redraw itself
                            when the value changes. The range is from 0 to 1.0 and DRX
                            LookAndFeel classes will draw a spinning animation for values
                            outside this range. Obviously you'd better be careful not to
                            delete this variable while the ProgressBar still exists!
    */
    explicit ProgressBar (f64& progress);

    /** Creates a ProgressBar with a specific style.

        @param progress     pass in a reference to a f64 that you're going to
                            update with your task's progress. The ProgressBar will
                            monitor the value of this variable and will redraw itself
                            when the value changes. The range is from 0 to 1.0 and DRX
                            LookAndFeel classes will draw a spinning animation for values
                            outside this range. Obviously you'd better be careful not to
                            delete this variable while the ProgressBar still exists!
        @param style        the style of the progress bar.
    */
    ProgressBar (f64& progress, std::optional<Style> style);

    /** Destructor. */
    ~ProgressBar() override = default;

    //==============================================================================
    /** Turns the percentage display on or off.

        By default this is on, and the progress bar will display a text string showing
        its current percentage.
    */
    z0 setPercentageDisplay (b8 shouldDisplayPercentage);

    /** Gives the progress bar a string to display inside it.

        If you call this, it will turn off the percentage display.
        @see setPercentageDisplay
    */
    z0 setTextToDisplay (const Txt& text);

    /** Sets the progress bar's current style.

        You can use this to force getResolvedStyle() to return a particular value.
        If a non-nullopt style is passed, that style will always be returned by
        getResolvedStyle(). Otherwise, if nullopt is passed, getResolvedStyle() will
        return its LookAndFeel's getDefaultProgressBarStyle().

        @see getStyle, getResolvedStyle
    */
    z0 setStyle (std::optional<Style> newStyle);

    /** Returns the progress bar's current style, as set in the constructor or in setStyle().

        @see setStyle, getResolvedStyle
    */
    std::optional<Style> getStyle() const { return style; }

    /** Returns the progress bar's current style if it has one, or a default style determined by
        the look-and-feel if it doesn't.

        Use this function in overrides of LookAndFeelMethods::drawProgressBar() in order to
        determine which style to draw.

        @see getStyle, setStyle, LookAndFeelMethods::getDefaultProgressBarStyle
    */
    Style getResolvedStyle() const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the bar.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId              = 0x1001900,    /**< The background colour, behind the bar. */
        foregroundColorId              = 0x1001a00,    /**< The colour to use to draw the bar itself. LookAndFeel
                                                             classes will probably use variations on this colour. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        /** Draws a progress bar.

            If the progress value is less than 0 or greater than 1.0, this should draw a spinning
            bar that fills the whole space (i.e. to say that the app is still busy but the progress
            isn't known). It can use the current time as a basis for playing an animation.

            To determine which style of progress-bar to draw call getResolvedStyle().

            (Used by progress bars in AlertWindow).

            @see getResolvedStyle
        */
        virtual z0 drawProgressBar (Graphics&, ProgressBar&, i32 width, i32 height,
                                      f64 progress, const Txt& textToShow) = 0;

        virtual b8 isProgressBarOpaque (ProgressBar&) = 0;

        /** Returns the default style a progress bar should use if one hasn't been set.

            @see setStyle, getResolvedStyle
        */
        virtual Style getDefaultProgressBarStyle (const ProgressBar&) = 0;
    };

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    z0 visibilityChanged() override;
    /** @internal */
    z0 colourChanged() override;

private:
    f64& progress;
    std::optional<Style> style;
    f64 currentValue { jlimit (0.0, 1.0, progress) };
    b8 displayPercentage { true };
    Txt displayedMessage, currentMessage;
    u32 lastCallbackTime { 0 };

    z0 timerCallback() override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgressBar)
};

} // namespace drx
