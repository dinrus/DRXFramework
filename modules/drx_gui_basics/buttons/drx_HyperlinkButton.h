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
    A button showing an underlined weblink, that will launch the link
    when it's clicked.

    @see Button

    @tags{GUI}
*/
class DRX_API  HyperlinkButton  : public Button
{
public:
    //==============================================================================
    /** Creates a HyperlinkButton.

        @param linkText     the text that will be displayed in the button - this is
                            also set as the Component's name, but the text can be
                            changed later with the Button::setButtonText() method
        @param linkURL      the URL to launch when the user clicks the button
    */
    HyperlinkButton (const Txt& linkText,
                     const URL& linkURL);

    /** Creates a HyperlinkButton. */
    HyperlinkButton();

    /** Destructor. */
    ~HyperlinkButton() override;

    //==============================================================================
    /** Changes the font to use for the text.

        If resizeToMatchComponentHeight is true, the font's height will be adjusted
        to match the size of the component.
    */
    z0 setFont (const Font& newFont,
                  b8 resizeToMatchComponentHeight,
                  Justification justificationType = Justification::horizontallyCentred);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the link.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        textColorId             = 0x1001f00, /**< The colour to use for the URL text. */
    };

    //==============================================================================
    /** Changes the URL that the button will trigger. */
    z0 setURL (const URL& newURL) noexcept;

    /** Returns the URL that the button will trigger. */
    const URL& getURL() const noexcept                          { return url; }

    //==============================================================================
    /** Resizes the button horizontally to fit snugly around the text.

        This won't affect the button's height.
    */
    z0 changeWidthToFitText();

    //==============================================================================
    /** Sets the style of justification to be used for positioning the text.
        (The default is Justification::centred)
    */
    z0 setJustificationType (Justification justification);

    /** Returns the type of justification, as set in setJustificationType(). */
    Justification getJustificationType() const noexcept         { return justification; }

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** @internal */
    z0 clicked() override;
    /** @internal */
    z0 colourChanged() override;
    /** @internal */
    z0 paintButton (Graphics&, b8, b8) override;

private:
    //==============================================================================
    using Button::clicked;
    Font getFontToUse() const;

    //==============================================================================
    URL url;
    Font font;
    b8 resizeFont;
    Justification justification;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HyperlinkButton)
};

} // namespace drx
