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
    A component that draws an outline around itself and has an optional title at
    the top, for drawing an outline around a group of controls.


    @tags{GUI}
*/
class DRX_API  GroupComponent    : public Component
{
public:
    //==============================================================================
    /** Creates a GroupComponent.

        @param componentName    the name to give the component
        @param labelText        the text to show at the top of the outline
    */
    GroupComponent (const Txt& componentName = {},
                    const Txt& labelText = {});

    /** Destructor. */
    ~GroupComponent() override;

    //==============================================================================
    /** Changes the text that's shown at the top of the component. */
    z0 setText (const Txt& newText);

    /** Returns the currently displayed text label. */
    Txt getText() const;

    /** Sets the positioning of the text label.
        (The default is Justification::left)
        @see getTextLabelPosition
    */
    z0 setTextLabelPosition (Justification justification);

    /** Returns the current text label position.
        @see setTextLabelPosition
    */
    Justification getTextLabelPosition() const noexcept           { return justification; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        outlineColorId     = 0x1005400,    /**< The colour to use for drawing the line around the edge. */
        textColorId        = 0x1005410     /**< The colour to use to draw the text label. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawGroupComponentOutline (Graphics&, i32 w, i32 h, const Txt& text,
                                                const Justification&, GroupComponent&) = 0;
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 enablementChanged() override;
    /** @internal */
    z0 colourChanged() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    Txt text;
    Justification justification;

    DRX_DECLARE_NON_COPYABLE (GroupComponent)
};

} // namespace drx
