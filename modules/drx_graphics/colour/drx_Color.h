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
    Represents a colour, also including a transparency value.

    The colour is stored internally as u32 8-bit red, green, blue and alpha values.

    @tags{Graphics}
*/
class DRX_API  Color  final
{
public:
    //==============================================================================
    /** Creates a transparent black colour. */
    Color() = default;

    /** Creates a copy of another Color object. */
    Color (const Color&) = default;

    /** Creates a colour from a 32-bit ARGB value.

        The format of this number is:
            ((alpha << 24) | (red << 16) | (green << 8) | blue).

        All components in the range 0x00 to 0xff.
        An alpha of 0x00 is completely transparent, alpha of 0xff is opaque.

        @see getPixelARGB
    */
    explicit Color (u32 argb) noexcept;

    /** Creates an opaque colour using 8-bit red, green and blue values */
    Color (u8 red,
            u8 green,
            u8 blue) noexcept;

    /** Creates an opaque colour using 8-bit red, green and blue values */
    static Color fromRGB (u8 red,
                           u8 green,
                           u8 blue) noexcept;

    /** Creates a colour using 8-bit red, green, blue and alpha values. */
    Color (u8 red,
            u8 green,
            u8 blue,
            u8 alpha) noexcept;

    /** Creates a colour using 8-bit red, green, blue and alpha values. */
    static Color fromRGBA (u8 red,
                            u8 green,
                            u8 blue,
                            u8 alpha) noexcept;

    /** Creates a colour from 8-bit red, green, and blue values, and a floating-point alpha.

        Alpha of 0.0 is transparent, alpha of 1.0f is opaque.
        Values outside the valid range will be clipped.
    */
    Color (u8 red,
            u8 green,
            u8 blue,
            f32 alpha) noexcept;

    /** Creates a colour using floating point red, green, blue and alpha values.
        Numbers outside the range 0..1 will be clipped.
    */
    static Color fromFloatRGBA (f32 red,
                                 f32 green,
                                 f32 blue,
                                 f32 alpha) noexcept;

    /** Creates a colour using floating point hue, saturation and brightness values, and an 8-bit alpha.

        The floating point values must be between 0.0 and 1.0.
        An alpha of 0x00 is completely transparent, alpha of 0xff is opaque.
        Values outside the valid range will be clipped.
    */
    Color (f32 hue,
            f32 saturation,
            f32 brightness,
            u8 alpha) noexcept;

    /** Creates a colour using floating point hue, saturation, brightness and alpha values.

        All values must be between 0.0 and 1.0.
        Numbers outside the valid range will be clipped.
    */
    Color (f32 hue,
            f32 saturation,
            f32 brightness,
            f32 alpha) noexcept;

    /** Creates a colour using floating point hue, saturation, brightness and alpha values.

        All values must be between 0.0 and 1.0.
        Numbers outside the valid range will be clipped.
    */
    static Color fromHSV (f32 hue,
                           f32 saturation,
                           f32 brightness,
                           f32 alpha) noexcept;

    /** Creates a colour using floating point hue, saturation, lightness and alpha values.

        All values must be between 0.0 and 1.0.
        Numbers outside the valid range will be clipped.
    */
    static Color fromHSL (f32 hue,
                           f32 saturation,
                           f32 lightness,
                           f32 alpha) noexcept;

    /** Creates a colour using a PixelARGB object. This function assumes that the argb pixel is
        not premultiplied.
     */
    Color (PixelARGB argb) noexcept;

    /** Creates a colour using a PixelRGB object.
     */
    Color (PixelRGB rgb) noexcept;

    /** Creates a colour using a PixelAlpha object.
     */
    Color (PixelAlpha alpha) noexcept;

    /** Destructor. */
    ~Color() = default;

    /** Copies another Color object. */
    Color& operator= (const Color&) = default;

    /** Compares two colours. */
    b8 operator== (const Color& other) const noexcept;
    /** Compares two colours. */
    b8 operator!= (const Color& other) const noexcept;

    //==============================================================================
    /** Returns the red component of this colour.
        @returns a value between 0x00 and 0xff.
    */
    u8 getRed() const noexcept                       { return argb.getRed(); }

    /** Returns the green component of this colour.
        @returns a value between 0x00 and 0xff.
    */
    u8 getGreen() const noexcept                     { return argb.getGreen(); }

    /** Returns the blue component of this colour.
        @returns a value between 0x00 and 0xff.
    */
    u8 getBlue() const noexcept                      { return argb.getBlue(); }

    /** Returns the red component of this colour as a floating point value.
        @returns a value between 0.0 and 1.0
    */
    f32 getFloatRed() const noexcept;

    /** Returns the green component of this colour as a floating point value.
        @returns a value between 0.0 and 1.0
    */
    f32 getFloatGreen() const noexcept;

    /** Returns the blue component of this colour as a floating point value.
        @returns a value between 0.0 and 1.0
    */
    f32 getFloatBlue() const noexcept;

    /** Returns a premultiplied ARGB pixel object that represents this colour.
    */
    PixelARGB getPixelARGB() const noexcept;

    /** Returns an ARGB pixel object that represents this colour.
    */
    PixelARGB getNonPremultipliedPixelARGB() const noexcept;

    /** Returns a 32-bit integer that represents this colour.

        The format of this number is:
            ((alpha << 24) | (red << 16) | (green << 8) | blue).
    */
    u32 getARGB() const noexcept;

    //==============================================================================
    /** Returns the colour's alpha (opacity).

        Alpha of 0x00 is completely transparent, 0xff is completely opaque.
    */
    u8 getAlpha() const noexcept                     { return argb.getAlpha(); }

    /** Returns the colour's alpha (opacity) as a floating point value.

        Alpha of 0.0 is completely transparent, 1.0 is completely opaque.
    */
    f32 getFloatAlpha() const noexcept;

    /** Возвращает true, если this colour is completely opaque.

        Equivalent to (getAlpha() == 0xff).
    */
    b8 isOpaque() const noexcept;

    /** Возвращает true, если this colour is completely transparent.

        Equivalent to (getAlpha() == 0x00).
    */
    b8 isTransparent() const noexcept;

    /** Returns a colour that's the same colour as this one, but with a new alpha value. */
    Color withAlpha (u8 newAlpha) const noexcept;

    /** Returns a colour that's the same colour as this one, but with a new alpha value. */
    Color withAlpha (f32 newAlpha) const noexcept;

    /** Returns a colour that's the same colour as this one, but with a modified alpha value.
        The new colour's alpha will be this object's alpha multiplied by the value passed-in.
    */
    Color withMultipliedAlpha (f32 alphaMultiplier) const noexcept;

    //==============================================================================
    /** Returns a colour that is the result of alpha-compositing a new colour over this one.
        If the foreground colour is semi-transparent, it is blended onto this colour accordingly.
    */
    Color overlaidWith (Color foregroundColor) const noexcept;

    /** Returns a colour that lies somewhere between this one and another.
        If amountOfOther is zero, the result is 100% this colour, if amountOfOther
        is 1.0, the result is 100% of the other colour.
    */
    Color interpolatedWith (Color other, f32 proportionOfOther) const noexcept;

    //==============================================================================
    /** Returns the colour's hue component.
        The value returned is in the range 0.0 to 1.0
    */
    f32 getHue() const noexcept;

    /** Returns the colour's saturation component.
        The value returned is in the range 0.0 to 1.0
    */
    f32 getSaturation() const noexcept;

    /** Returns the colour's saturation component as represented in the HSL colour space.
        The value returned is in the range 0.0 to 1.0
    */
    f32 getSaturationHSL() const noexcept;

    /** Returns the colour's brightness component.
        The value returned is in the range 0.0 to 1.0
    */
    f32 getBrightness() const noexcept;

    /** Returns the colour's lightness component.
        The value returned is in the range 0.0 to 1.0
    */
    f32 getLightness() const noexcept;

    /** Returns a skewed brightness value, adjusted to better reflect the way the human
        eye responds to different colour channels. This makes it better than getBrightness()
        for comparing differences in brightness.
    */
    f32 getPerceivedBrightness() const noexcept;

    /** Returns the colour's hue, saturation and brightness components all at once.
        The values returned are in the range 0.0 to 1.0
    */
    z0 getHSB (f32& hue,
                 f32& saturation,
                 f32& brightness) const noexcept;

    /** Returns the colour's hue, saturation and lightness components all at once.
        The values returned are in the range 0.0 to 1.0
    */
    z0 getHSL (f32& hue,
                 f32& saturation,
                 f32& lightness) const noexcept;

    //==============================================================================
    /** Returns a copy of this colour with a different hue. */
    [[nodiscard]] Color withHue (f32 newHue) const noexcept;

    /** Returns a copy of this colour with a different saturation. */
    [[nodiscard]] Color withSaturation (f32 newSaturation) const noexcept;

    /** Returns a copy of this colour with a different saturation in the HSL colour space. */
    [[nodiscard]] Color withSaturationHSL (f32 newSaturation) const noexcept;

    /** Returns a copy of this colour with a different brightness.
        @see brighter, darker, withMultipliedBrightness
    */
    [[nodiscard]] Color withBrightness (f32 newBrightness) const noexcept;

    /** Returns a copy of this colour with a different lightness.
        @see lighter, darker, withMultipliedLightness
    */
    [[nodiscard]] Color withLightness (f32 newLightness) const noexcept;

    /** Returns a copy of this colour with its hue rotated.
        The new colour's hue is ((this->getHue() + amountToRotate) % 1.0)
        @see brighter, darker, withMultipliedBrightness
    */
    [[nodiscard]] Color withRotatedHue (f32 amountToRotate) const noexcept;

    /** Returns a copy of this colour with its saturation multiplied by the given value.
        The new colour's saturation is (this->getSaturation() * multiplier)
        (the result is clipped to legal limits).
    */
    [[nodiscard]] Color withMultipliedSaturation (f32 multiplier) const noexcept;

    /** Returns a copy of this colour with its saturation multiplied by the given value.
        The new colour's saturation is (this->getSaturation() * multiplier)
        (the result is clipped to legal limits).

        This will be in the HSL colour space.
    */
    [[nodiscard]] Color withMultipliedSaturationHSL (f32 multiplier) const noexcept;

    /** Returns a copy of this colour with its brightness multiplied by the given value.
        The new colour's brightness is (this->getBrightness() * multiplier)
        (the result is clipped to legal limits).
    */
    [[nodiscard]] Color withMultipliedBrightness (f32 amount) const noexcept;

    /** Returns a copy of this colour with its lightness multiplied by the given value.
        The new colour's lightness is (this->lightness() * multiplier)
        (the result is clipped to legal limits).
    */
    [[nodiscard]] Color withMultipliedLightness (f32 amount) const noexcept;

    //==============================================================================
    /** Returns a brighter version of this colour.
        @param amountBrighter   how much brighter to make it - a value greater than or equal to 0,
                                where 0 is unchanged, and higher values make it brighter
        @see withMultipliedBrightness
    */
    [[nodiscard]] Color brighter (f32 amountBrighter = 0.4f) const noexcept;

    /** Returns a darker version of this colour.
        @param amountDarker     how much darker to make it - a value greater than or equal to 0,
                                where 0 is unchanged, and higher values make it darker
        @see withMultipliedBrightness
    */
    [[nodiscard]] Color darker (f32 amountDarker = 0.4f) const noexcept;

    //==============================================================================
    /** Returns a colour that will be clearly visible against this colour.

        The amount parameter indicates how contrasting the new colour should
        be, so e.g. Colors::black.contrasting (0.1f) will return a colour
        that's just a little bit lighter; Colors::black.contrasting (1.0f) will
        return white; Colors::white.contrasting (1.0f) will return black, etc.
    */
    [[nodiscard]] Color contrasting (f32 amount = 1.0f) const noexcept;

    /** Returns a colour that is as close as possible to a target colour whilst
        still being in contrast to this one.

        The colour that is returned will be the targetColor, but with its luminosity
        nudged up or down so that it differs from the luminosity of this colour
        by at least the amount specified by minLuminosityDiff.
    */
    [[nodiscard]] Color contrasting (Color targetColor, f32 minLuminosityDiff) const noexcept;

    /** Returns a colour that contrasts against two colours.
        Looks for a colour that contrasts with both of the colours passed-in.
        Handy for things like choosing a highlight colour in text editors, etc.
    */
    [[nodiscard]] static Color contrasting (Color colour1,
                                              Color colour2) noexcept;

    //==============================================================================
    /** Returns an opaque shade of grey.
        @param brightness the level of grey to return - 0 is black, 1.0 is white
    */
    [[nodiscard]] static Color greyLevel (f32 brightness) noexcept;

    //==============================================================================
    /** Returns a stringified version of this colour.
        The string can be turned back into a colour using the fromString() method.
    */
    Txt toString() const;

    /** Reads the colour from a string that was created with toString(). */
    [[nodiscard]] static Color fromString (StringRef encodedColorString);

    /** Returns the colour as a hex string in the form RRGGBB or AARRGGBB. */
    Txt toDisplayString (b8 includeAlphaValue) const;

private:
    //==============================================================================
    PixelARGB argb { 0, 0, 0, 0 };
};

} // namespace drx
