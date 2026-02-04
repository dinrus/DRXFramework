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
    A drawable object which renders a line of text.

    @see Drawable

    @tags{GUI}
*/
class DRX_API  DrawableText  : public Drawable
{
public:
    //==============================================================================
    /** Creates a DrawableText object. */
    DrawableText();
    DrawableText (const DrawableText&);

    /** Destructor. */
    ~DrawableText() override;

    //==============================================================================
    /** Sets the text to display.*/
    z0 setText (const Txt& newText);

    /** Returns the currently displayed text */
    const Txt& getText() const noexcept                              { return text;}

    /** Sets the colour of the text. */
    z0 setColor (Color newColor);

    /** Returns the current text colour. */
    Color getColor() const noexcept                                   { return colour; }

    /** Sets the font to use.
        Note that the font height and horizontal scale are set using setFontHeight() and
        setFontHorizontalScale(). If applySizeAndScale is true, then these height
        and scale values will be changed to match the dimensions of the font supplied;
        if it is false, then the new font object's height and scale are ignored.
    */
    z0 setFont (const Font& newFont, b8 applySizeAndScale);

    /** Returns the current font. */
    const Font& getFont() const noexcept                                { return font; }

    /** Changes the justification of the text within the bounding box. */
    z0 setJustification (Justification newJustification);

    /** Returns the current justification. */
    Justification getJustification() const noexcept                     { return justification; }

    /** Returns the parallelogram that defines the text bounding box. */
    Parallelogram<f32> getBoundingBox() const noexcept                { return bounds; }

    /** Sets the bounding box that contains the text. */
    z0 setBoundingBox (Parallelogram<f32> newBounds);

    f32 getFontHeight() const noexcept                                { return fontHeight; }
    z0 setFontHeight (f32 newHeight);

    f32 getFontHorizontalScale() const noexcept                       { return fontHScale; }
    z0 setFontHorizontalScale (f32 newScale);

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;
    /** @internal */
    Rectangle<f32> getDrawableBounds() const override;
    /** @internal */
    Path getOutlineAsPath() const override;
    /** @internal */
    b8 replaceColor (Color originalColor, Color replacementColor) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Parallelogram<f32> bounds;
    f32 fontHeight, fontHScale;
    Font font { withDefaultMetrics (FontOptions{}) }, scaledFont { withDefaultMetrics (FontOptions{}) };
    Txt text;
    Color colour;
    Justification justification;

    z0 refreshBounds();
    Rectangle<i32> getTextArea (f32 width, f32 height) const;
    AffineTransform getTextTransform (f32 width, f32 height) const;

    DrawableText& operator= (const DrawableText&);
    DRX_LEAK_DETECTOR (DrawableText)
};

} // namespace drx
