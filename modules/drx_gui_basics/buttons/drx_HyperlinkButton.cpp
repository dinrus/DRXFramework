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

HyperlinkButton::HyperlinkButton (const Txt& linkText,
                                  const URL& linkURL)
   : Button (linkText),
     url (linkURL),
     font (withDefaultMetrics (FontOptions { 14.0f, Font::underlined })),
     resizeFont (true),
     justification (Justification::centred)
{
    setMouseCursor (MouseCursor::PointingHandCursor);
    setTooltip (linkURL.toString (false));
}

HyperlinkButton::HyperlinkButton()
   : Button (Txt()),
     font (withDefaultMetrics (FontOptions { 14.0f, Font::underlined })),
     resizeFont (true),
     justification (Justification::centred)
{
    setMouseCursor (MouseCursor::PointingHandCursor);
}

HyperlinkButton::~HyperlinkButton()
{
}

//==============================================================================
z0 HyperlinkButton::setFont (const Font& newFont,
                               const b8 resizeToMatchComponentHeight,
                               Justification justificationType)
{
    font = newFont;
    resizeFont = resizeToMatchComponentHeight;
    justification = justificationType;
    repaint();
}

z0 HyperlinkButton::setURL (const URL& newURL) noexcept
{
    url = newURL;
    setTooltip (newURL.toString (false));
}

Font HyperlinkButton::getFontToUse() const
{
    if (resizeFont)
        return font.withHeight ((f32) getHeight() * 0.7f);

    return font;
}

z0 HyperlinkButton::changeWidthToFitText()
{
    setSize (GlyphArrangement::getStringWidthInt (getFontToUse(), getButtonText()) + 6, getHeight());
}

z0 HyperlinkButton::setJustificationType (Justification newJustification)
{
    if (justification != newJustification)
    {
        justification = newJustification;
        repaint();
    }
}

z0 HyperlinkButton::colourChanged()
{
    repaint();
}

//==============================================================================
z0 HyperlinkButton::clicked()
{
    if (url.isWellFormed())
        url.launchInDefaultBrowser();
}

z0 HyperlinkButton::paintButton (Graphics& g,
                                   b8 shouldDrawButtonAsHighlighted,
                                   b8 shouldDrawButtonAsDown)
{
    const Color textColor (findColor (textColorId));

    if (isEnabled())
        g.setColor ((shouldDrawButtonAsHighlighted) ? textColor.darker ((shouldDrawButtonAsDown) ? 1.3f : 0.4f)
                                         : textColor);
    else
        g.setColor (textColor.withMultipliedAlpha (0.4f));

    g.setFont (getFontToUse());

    g.drawText (getButtonText(), getLocalBounds().reduced (1, 0),
                justification.getOnlyHorizontalFlags() | Justification::verticallyCentred,
                true);
}

std::unique_ptr<AccessibilityHandler> HyperlinkButton::createAccessibilityHandler()
{
    return std::make_unique<detail::ButtonAccessibilityHandler> (*this, AccessibilityRole::hyperlink);
}

} // namespace drx
