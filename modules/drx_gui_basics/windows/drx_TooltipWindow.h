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
    A window that displays a pop-up tooltip when the mouse hovers over another component.

    To enable tooltips in your app, just create a single instance of a TooltipWindow
    object. Note that if you instantiate more than one instance of this class with the
    same parentComponent (even if both TooltipWindow's parentComponent is nil), you'll
    end up with multiple tooltips being shown! To avoid this use a SharedResourcePointer
    to instantiate the TooltipWindow only once.

    For audio plug-ins (which should not be opening native windows) it is better
    to add a TooltipWindow as a member variable to the editor and ensure that the
    editor is the parentComponent of your TooltipWindow. This will ensure that your
    TooltipWindow is scaled according to your editor and the DAWs scaling setting.

    The TooltipWindow object will then stay invisible, waiting until the mouse
    hovers for the specified length of time - it will then see if it's currently
    over a component which implements the TooltipClient interface, and if so,
    it will make itself visible to show the tooltip in the appropriate place.

    @see TooltipClient, SettableTooltipClient, SharedResourcePointer

    @tags{GUI}
*/
class DRX_API  TooltipWindow  : public Component,
                                 private Timer
{
public:
    //==============================================================================
    /** Creates a tooltip window.

        Make sure your app only creates one instance of this class, otherwise you'll
        get multiple overlaid tooltips appearing. The window will initially be invisible
        and will make itself visible when it needs to display a tip.

        To change the style of tooltips, see the LookAndFeel class for its tooltip
        methods.

        @param parentComponent  if set to nullptr, the TooltipWindow will appear on the desktop,
                                otherwise the tooltip will be added to the given parent
                                component.
        @param millisecondsBeforeTipAppears     the time for which the mouse has to stay still
                                                before a tooltip will be shown

        @see TooltipClient, LookAndFeel::drawTooltip, LookAndFeel::getTooltipBounds
    */
    explicit TooltipWindow (Component* parentComponent = nullptr,
                            i32 millisecondsBeforeTipAppears = 700);

    /** Destructor. */
    ~TooltipWindow() override;

    //==============================================================================
    /** Changes the time before the tip appears.
        This lets you change the value that was set in the constructor.
    */
    z0 setMillisecondsBeforeTipAppears (i32 newTimeMs = 700) noexcept;

    /** Can be called to manually force a tip to be shown at a particular location.

        The tip will be shown until hideTip() is called, or a dismissal mouse event
        occurs.

        @see hideTip
    */
    z0 displayTip (Point<i32> screenPosition, const Txt& text);

    /** Can be called to manually hide the tip if it's showing. */
    z0 hideTip();

    /** Asks a component for its tooltip.
        This can be overridden if you need custom lookup behaviour or to modify the strings.
    */
    virtual Txt getTipFor (Component&);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the tooltip.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId      = 0x1001b00,    /**< The colour to fill the background with. */
        textColorId            = 0x1001c00,    /**< The colour to use for the text. */
        outlineColorId         = 0x1001c10     /**< The colour to use to draw an outline around the tooltip. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        window drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        /** returns the bounds for a tooltip at the given screen coordinate, constrained within the given desktop area. */
        virtual Rectangle<i32> getTooltipBounds (const Txt& tipText, Point<i32> screenPos, Rectangle<i32> parentArea) = 0;
        virtual z0 drawTooltip (Graphics&, const Txt& text, i32 width, i32 height) = 0;
    };

    //==============================================================================
    /** @internal */
    f32 getDesktopScaleFactor() const override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Point<f32> lastMousePos;
    SafePointer<Component> lastComponentUnderMouse;
    Txt tipShowing, lastTipUnderMouse, manuallyShownTip;
    i32 millisecondsBeforeTipAppears;
    u32 lastCompChangeTime = 0, lastHideTime = 0;
    b8 reentrant = false, dismissalMouseEventOccurred = false;

    enum ShownManually { yes, no };
    z0 displayTipInternal (Point<i32>, const Txt&, ShownManually);

    z0 paint (Graphics&) override;
    z0 mouseEnter (const MouseEvent&) override;
    z0 mouseDown (const MouseEvent&) override;
    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    z0 timerCallback() override;
    z0 updatePosition (const Txt&, Point<i32>, Rectangle<i32>);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipWindow)
};

} // namespace drx
