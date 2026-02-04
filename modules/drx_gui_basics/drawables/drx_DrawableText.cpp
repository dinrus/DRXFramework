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

DrawableText::DrawableText()
    : colour (Colors::black),
      justification (Justification::centredLeft)
{
    setBoundingBox (Parallelogram<f32> ({ 0.0f, 0.0f, 50.0f, 20.0f }));
    setFont (withDefaultMetrics (FontOptions (15.0f)), true);
}

DrawableText::DrawableText (const DrawableText& other)
    : Drawable (other),
      bounds (other.bounds),
      fontHeight (other.fontHeight),
      fontHScale (other.fontHScale),
      font (other.font),
      text (other.text),
      colour (other.colour),
      justification (other.justification)
{
    refreshBounds();
}

DrawableText::~DrawableText()
{
}

std::unique_ptr<Drawable> DrawableText::createCopy() const
{
    return std::make_unique<DrawableText> (*this);
}

//==============================================================================
z0 DrawableText::setText (const Txt& newText)
{
    if (text != newText)
    {
        text = newText;
        refreshBounds();
    }
}

z0 DrawableText::setColor (Color newColor)
{
    if (colour != newColor)
    {
        colour = newColor;
        repaint();
    }
}

z0 DrawableText::setFont (const Font& newFont, b8 applySizeAndScale)
{
    if (font != newFont)
    {
        font = newFont;

        if (applySizeAndScale)
        {
            fontHeight = font.getHeight();
            fontHScale = font.getHorizontalScale();
        }

        refreshBounds();
    }
}

z0 DrawableText::setJustification (Justification newJustification)
{
    justification = newJustification;
    repaint();
}

z0 DrawableText::setBoundingBox (Parallelogram<f32> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        refreshBounds();
    }
}

z0 DrawableText::setFontHeight (f32 newHeight)
{
    if (! approximatelyEqual (fontHeight, newHeight))
    {
        fontHeight = newHeight;
        refreshBounds();
    }
}

z0 DrawableText::setFontHorizontalScale (f32 newScale)
{
    if (! approximatelyEqual (fontHScale, newScale))
    {
        fontHScale = newScale;
        refreshBounds();
    }
}

z0 DrawableText::refreshBounds()
{
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    auto height = jlimit (0.01f, jmax (0.01f, h), fontHeight);
    auto hscale = jlimit (0.01f, jmax (0.01f, w), fontHScale);

    scaledFont = font;
    scaledFont.setHeight (height);
    scaledFont.setHorizontalScale (hscale);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

//==============================================================================
Rectangle<i32> DrawableText::getTextArea (f32 w, f32 h) const
{
    return Rectangle<f32> (w, h).getSmallestIntegerContainer();
}

AffineTransform DrawableText::getTextTransform (f32 w, f32 h) const
{
    return AffineTransform::fromTargetPoints (Point<f32>(),       bounds.topLeft,
                                              Point<f32> (w, 0),  bounds.topRight,
                                              Point<f32> (0, h),  bounds.bottomLeft);
}

z0 DrawableText::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);

    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    g.addTransform (getTextTransform (w, h));
    g.setFont (scaledFont);
    g.setColor (colour);

    g.drawFittedText (text, getTextArea (w, h), justification, 0x100000);
}

Rectangle<f32> DrawableText::getDrawableBounds() const
{
    return bounds.getBoundingBox();
}

Path DrawableText::getOutlineAsPath() const
{
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();
    auto area = getTextArea (w, h).toFloat();

    GlyphArrangement arr;
    arr.addFittedText (scaledFont, text,
                       area.getX(), area.getY(),
                       area.getWidth(), area.getHeight(),
                       justification,
                       0x100000);

    Path pathOfAllGlyphs;

    for (auto& glyph : arr)
    {
        Path gylphPath;
        glyph.createPath (gylphPath);
        pathOfAllGlyphs.addPath (gylphPath);
    }

    pathOfAllGlyphs.applyTransform (getTextTransform (w, h).followedBy (drawableTransform));

    return pathOfAllGlyphs;
}

b8 DrawableText::replaceColor (Color originalColor, Color replacementColor)
{
    if (colour != originalColor)
        return false;

    setColor (replacementColor);
    return true;
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> DrawableText::createAccessibilityHandler()
{
    class DrawableTextAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        DrawableTextAccessibilityHandler (DrawableText& drawableTextToWrap)
            : AccessibilityHandler (drawableTextToWrap, AccessibilityRole::staticText),
              drawableText (drawableTextToWrap)
        {
        }

        Txt getTitle() const override  { return drawableText.getText(); }

    private:
        DrawableText& drawableText;
    };

    return std::make_unique<DrawableTextAccessibilityHandler> (*this);
}

} // namespace drx
