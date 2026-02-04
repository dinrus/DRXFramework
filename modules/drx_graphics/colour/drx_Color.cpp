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

namespace ColorHelpers
{
    static u8 floatToUInt8 (f32 n) noexcept
    {
        return n <= 0.0f ? 0 : (n >= 1.0f ? 255 : (u8) roundToInt (n * 255.0f));
    }

    static f32 getHue (Color col)
    {
        auto r = (i32) col.getRed();
        auto g = (i32) col.getGreen();
        auto b = (i32) col.getBlue();

        auto hi = jmax (r, g, b);
        auto lo = jmin (r, g, b);

        f32 hue = 0.0f;

        if (hi > 0 && ! exactlyEqual (hi, lo))
        {
            auto invDiff = 1.0f / (f32) (hi - lo);

            auto red   = (f32) (hi - r) * invDiff;
            auto green = (f32) (hi - g) * invDiff;
            auto blue  = (f32) (hi - b) * invDiff;

            if      (r == hi)  hue = blue - green;
            else if (g == hi)  hue = 2.0f + red - blue;
            else               hue = 4.0f + green - red;

            hue *= 1.0f / 6.0f;

            if (hue < 0.0f)
                hue += 1.0f;
        }

        return hue;
    }

    //==============================================================================
    struct HSL
    {
        HSL (Color col) noexcept
        {
            auto r = (i32) col.getRed();
            auto g = (i32) col.getGreen();
            auto b = (i32) col.getBlue();

            auto hi = jmax (r, g, b);
            auto lo = jmin (r, g, b);

            if (hi < 0)
                return;

            lightness = ((f32) (hi + lo) / 2.0f) / 255.0f;

            if (lightness <= 0.0f)
                return;

            hue = getHue (col);

            if (1.0f <= lightness)
                return;

            auto denominator = 1.0f - std::abs ((2.0f * lightness) - 1.0f);
            saturation = ((f32) (hi - lo) / 255.0f) / denominator;
        }

        Color toColor (Color original) const noexcept
        {
            return Color::fromHSL (hue, saturation, lightness, original.getAlpha());
        }

        static PixelARGB toRGB (f32 h, f32 s, f32 l, u8 alpha) noexcept
        {
            auto v = l < 0.5f ? l * (1.0f + s) : l + s - (l * s);

            if (approximatelyEqual (v, 0.0f))
                return PixelARGB (alpha, 0, 0, 0);

            auto min = (2.0f * l) - v;
            auto sv = (v - min) / v;

            h = ((h - std::floor (h)) * 360.0f) / 60.0f;
            auto f = h - std::floor (h);
            auto vsf = v * sv * f;
            auto mid1 = min + vsf;
            auto mid2 = v - vsf;

            if      (h < 1.0f)  return PixelARGB (alpha, floatToUInt8 (v),    floatToUInt8 (mid1), floatToUInt8 (min));
            else if (h < 2.0f)  return PixelARGB (alpha, floatToUInt8 (mid2), floatToUInt8 (v),    floatToUInt8 (min));
            else if (h < 3.0f)  return PixelARGB (alpha, floatToUInt8 (min),  floatToUInt8 (v),    floatToUInt8 (mid1));
            else if (h < 4.0f)  return PixelARGB (alpha, floatToUInt8 (min),  floatToUInt8 (mid2), floatToUInt8 (v));
            else if (h < 5.0f)  return PixelARGB (alpha, floatToUInt8 (mid1), floatToUInt8 (min),  floatToUInt8 (v));
            else if (h < 6.0f)  return PixelARGB (alpha, floatToUInt8 (v),    floatToUInt8 (min),  floatToUInt8 (mid2));

            return PixelARGB (alpha, 0, 0, 0);
        }

        f32 hue = 0.0f, saturation = 0.0f, lightness = 0.0f;
    };

    //==============================================================================
    struct HSB
    {
        HSB (Color col) noexcept
        {
            auto r = (i32) col.getRed();
            auto g = (i32) col.getGreen();
            auto b = (i32) col.getBlue();

            auto hi = jmax (r, g, b);
            auto lo = jmin (r, g, b);

            if (hi > 0)
            {
                saturation = (f32) (hi - lo) / (f32) hi;

                if (saturation > 0.0f)
                    hue = getHue (col);

                brightness = (f32) hi / 255.0f;
            }
        }

        Color toColor (Color original) const noexcept
        {
            return Color (hue, saturation, brightness, original.getAlpha());
        }

        static PixelARGB toRGB (f32 h, f32 s, f32 v, u8 alpha) noexcept
        {
            v = jlimit (0.0f, 255.0f, v * 255.0f);
            auto intV = (u8) roundToInt (v);

            if (s <= 0)
                return PixelARGB (alpha, intV, intV, intV);

            s = jmin (1.0f, s);
            h = ((h - std::floor (h)) * 360.0f) / 60.0f;
            auto f = h - std::floor (h);
            auto x = (u8) roundToInt (v * (1.0f - s));

            if (h < 1.0f)   return PixelARGB (alpha, intV,    (u8) roundToInt (v * (1.0f - (s * (1.0f - f)))), x);
            if (h < 2.0f)   return PixelARGB (alpha,          (u8) roundToInt (v * (1.0f - s * f)), intV, x);
            if (h < 3.0f)   return PixelARGB (alpha, x, intV, (u8) roundToInt (v * (1.0f - (s * (1.0f - f)))));
            if (h < 4.0f)   return PixelARGB (alpha, x,       (u8) roundToInt (v * (1.0f - s * f)), intV);
            if (h < 5.0f)   return PixelARGB (alpha,          (u8) roundToInt (v * (1.0f - (s * (1.0f - f)))), x, intV);
            return                 PixelARGB (alpha, intV, x, (u8) roundToInt (v * (1.0f - s * f)));
        }

        f32 hue = 0.0f, saturation = 0.0f, brightness = 0.0f;
    };

    //==============================================================================
    struct YIQ
    {
        YIQ (Color c) noexcept
        {
            auto r = c.getFloatRed();
            auto g = c.getFloatGreen();
            auto b = c.getFloatBlue();

            y = 0.2999f * r + 0.5870f * g + 0.1140f * b;
            i = 0.5957f * r - 0.2744f * g - 0.3212f * b;
            q = 0.2114f * r - 0.5225f * g - 0.3113f * b;
            alpha = c.getFloatAlpha();
        }

        Color toColor() const noexcept
        {
            return Color::fromFloatRGBA (y + 0.9563f * i + 0.6210f * q,
                                          y - 0.2721f * i - 0.6474f * q,
                                          y - 1.1070f * i + 1.7046f * q,
                                          alpha);
        }

        f32 y = 0.0f, i = 0.0f, q = 0.0f, alpha = 0.0f;
    };
}

//==============================================================================
b8 Color::operator== (const Color& other) const noexcept    { return argb.getNativeARGB() == other.argb.getNativeARGB(); }
b8 Color::operator!= (const Color& other) const noexcept    { return argb.getNativeARGB() != other.argb.getNativeARGB(); }

//==============================================================================
Color::Color (u32 col) noexcept
    : argb (static_cast<u8> ((col >> 24) & 0xff),
            static_cast<u8> ((col >> 16) & 0xff),
            static_cast<u8> ((col >> 8) & 0xff),
            static_cast<u8> (col & 0xff))
{
}

Color::Color (u8 red, u8 green, u8 blue) noexcept
{
    argb.setARGB (0xff, red, green, blue);
}

Color Color::fromRGB (u8 red, u8 green, u8 blue) noexcept
{
    return Color (red, green, blue);
}

Color::Color (u8 red, u8 green, u8 blue, u8 alpha) noexcept
{
    argb.setARGB (alpha, red, green, blue);
}

Color Color::fromRGBA (u8 red, u8 green, u8 blue, u8 alpha) noexcept
{
    return Color (red, green, blue, alpha);
}

Color::Color (u8 red, u8 green, u8 blue, f32 alpha) noexcept
{
    argb.setARGB (ColorHelpers::floatToUInt8 (alpha), red, green, blue);
}

Color Color::fromFloatRGBA (f32 red, f32 green, f32 blue, f32 alpha) noexcept
{
    return Color (ColorHelpers::floatToUInt8 (red),
                   ColorHelpers::floatToUInt8 (green),
                   ColorHelpers::floatToUInt8 (blue), alpha);
}

Color::Color (f32 hue, f32 saturation, f32 brightness, f32 alpha) noexcept
    : argb (ColorHelpers::HSB::toRGB (hue, saturation, brightness, ColorHelpers::floatToUInt8 (alpha)))
{
}

Color Color::fromHSV (f32 hue, f32 saturation, f32 brightness, f32 alpha) noexcept
{
    return Color (hue, saturation, brightness, alpha);
}

Color Color::fromHSL (f32 hue, f32 saturation, f32 lightness, f32 alpha) noexcept
{
    Color hslColor;
    hslColor.argb = ColorHelpers::HSL::toRGB (hue, saturation, lightness, ColorHelpers::floatToUInt8 (alpha));

    return hslColor;
}

Color::Color (f32 hue, f32 saturation, f32 brightness, u8 alpha) noexcept
    : argb (ColorHelpers::HSB::toRGB (hue, saturation, brightness, alpha))
{
}

Color::Color (PixelARGB argb_) noexcept
    : argb (argb_)
{
}

Color::Color (PixelRGB rgb) noexcept
    : argb (Color (rgb.getInARGBMaskOrder()).argb)
{
}

Color::Color (PixelAlpha alpha) noexcept
    : argb (Color (alpha.getInARGBMaskOrder()).argb)
{
}

//==============================================================================
PixelARGB Color::getPixelARGB() const noexcept
{
    PixelARGB p (argb);
    p.premultiply();
    return p;
}

PixelARGB Color::getNonPremultipliedPixelARGB() const noexcept
{
    return argb;
}

u32 Color::getARGB() const noexcept
{
    return argb.getInARGBMaskOrder();
}

//==============================================================================
b8 Color::isTransparent() const noexcept
{
    return getAlpha() == 0;
}

b8 Color::isOpaque() const noexcept
{
    return getAlpha() == 0xff;
}

Color Color::withAlpha (u8 newAlpha) const noexcept
{
    PixelARGB newCol (argb);
    newCol.setAlpha (newAlpha);
    return Color (newCol);
}

Color Color::withAlpha (f32 newAlpha) const noexcept
{
    jassert (newAlpha >= 0 && newAlpha <= 1.0f);

    PixelARGB newCol (argb);
    newCol.setAlpha (ColorHelpers::floatToUInt8 (newAlpha));
    return Color (newCol);
}

Color Color::withMultipliedAlpha (f32 alphaMultiplier) const noexcept
{
    jassert (alphaMultiplier >= 0);

    PixelARGB newCol (argb);
    newCol.setAlpha ((u8) jmin (0xff, roundToInt (alphaMultiplier * newCol.getAlpha())));
    return Color (newCol);
}

//==============================================================================
Color Color::overlaidWith (Color src) const noexcept
{
    auto destAlpha = getAlpha();

    if (destAlpha <= 0)
        return src;

    auto invA = 0xff - (i32) src.getAlpha();
    auto resA = 0xff - (((0xff - destAlpha) * invA) >> 8);

    if (resA <= 0)
        return *this;

    auto da = (invA * destAlpha) / resA;

    return Color ((u8) (src.getRed()   + ((((i32) getRed()   - src.getRed())   * da) >> 8)),
                   (u8) (src.getGreen() + ((((i32) getGreen() - src.getGreen()) * da) >> 8)),
                   (u8) (src.getBlue()  + ((((i32) getBlue()  - src.getBlue())  * da) >> 8)),
                   (u8) resA);
}

Color Color::interpolatedWith (Color other, f32 proportionOfOther) const noexcept
{
    if (proportionOfOther <= 0)
        return *this;

    if (proportionOfOther >= 1.0f)
        return other;

    PixelARGB c1 (getPixelARGB());
    PixelARGB c2 (other.getPixelARGB());
    c1.tween (c2, (u32) roundToInt (proportionOfOther * 255.0f));
    c1.unpremultiply();

    return Color (c1);
}

//==============================================================================
f32 Color::getFloatRed() const noexcept      { return getRed()   / 255.0f; }
f32 Color::getFloatGreen() const noexcept    { return getGreen() / 255.0f; }
f32 Color::getFloatBlue() const noexcept     { return getBlue()  / 255.0f; }
f32 Color::getFloatAlpha() const noexcept    { return getAlpha() / 255.0f; }

//==============================================================================
z0 Color::getHSB (f32& h, f32& s, f32& v) const noexcept
{
    ColorHelpers::HSB hsb (*this);
    h = hsb.hue;
    s = hsb.saturation;
    v = hsb.brightness;
}

z0 Color::getHSL (f32& h, f32& s, f32& l) const noexcept
{
    ColorHelpers::HSL hsl (*this);
    h = hsl.hue;
    s = hsl.saturation;
    l = hsl.lightness;
}

f32 Color::getHue() const noexcept           { return ColorHelpers::HSB (*this).hue; }
f32 Color::getSaturation() const noexcept    { return ColorHelpers::HSB (*this).saturation; }
f32 Color::getBrightness() const noexcept    { return ColorHelpers::HSB (*this).brightness; }

f32 Color::getSaturationHSL() const noexcept { return ColorHelpers::HSL (*this).saturation; }
f32 Color::getLightness() const noexcept     { return ColorHelpers::HSL (*this).lightness; }

Color Color::withHue (f32 h) const noexcept          { ColorHelpers::HSB hsb (*this); hsb.hue = h;        return hsb.toColor (*this); }
Color Color::withSaturation (f32 s) const noexcept   { ColorHelpers::HSB hsb (*this); hsb.saturation = s; return hsb.toColor (*this); }
Color Color::withBrightness (f32 v) const noexcept   { ColorHelpers::HSB hsb (*this); hsb.brightness = v; return hsb.toColor (*this); }

Color Color::withSaturationHSL (f32 s) const noexcept { ColorHelpers::HSL hsl (*this); hsl.saturation = s; return hsl.toColor (*this); }
Color Color::withLightness (f32 l) const noexcept     { ColorHelpers::HSL hsl (*this); hsl.lightness = l;  return hsl.toColor (*this); }

f32 Color::getPerceivedBrightness() const noexcept
{
    return std::sqrt (0.241f * square (getFloatRed())
                    + 0.691f * square (getFloatGreen())
                    + 0.068f * square (getFloatBlue()));
}

//==============================================================================
Color Color::withRotatedHue (f32 amountToRotate) const noexcept
{
    ColorHelpers::HSB hsb (*this);
    hsb.hue += amountToRotate;
    return hsb.toColor (*this);
}

Color Color::withMultipliedSaturation (f32 amount) const noexcept
{
    ColorHelpers::HSB hsb (*this);
    hsb.saturation = jmin (1.0f, hsb.saturation * amount);
    return hsb.toColor (*this);
}

Color Color::withMultipliedSaturationHSL (f32 amount) const noexcept
{
    ColorHelpers::HSL hsl (*this);
    hsl.saturation = jmin (1.0f, hsl.saturation * amount);
    return hsl.toColor (*this);
}

Color Color::withMultipliedBrightness (f32 amount) const noexcept
{
    ColorHelpers::HSB hsb (*this);
    hsb.brightness = jmin (1.0f, hsb.brightness * amount);
    return hsb.toColor (*this);
}

Color Color::withMultipliedLightness (f32 amount) const noexcept
{
    ColorHelpers::HSL hsl (*this);
    hsl.lightness = jmin (1.0f, hsl.lightness * amount);
    return hsl.toColor (*this);
}

//==============================================================================
Color Color::brighter (f32 amount) const noexcept
{
    jassert (amount >= 0.0f);
    amount = 1.0f / (1.0f + amount);

    return Color ((u8) (255 - (amount * (255 - getRed()))),
                   (u8) (255 - (amount * (255 - getGreen()))),
                   (u8) (255 - (amount * (255 - getBlue()))),
                   getAlpha());
}

Color Color::darker (f32 amount) const noexcept
{
    jassert (amount >= 0.0f);
    amount = 1.0f / (1.0f + amount);

    return Color ((u8) (amount * getRed()),
                   (u8) (amount * getGreen()),
                   (u8) (amount * getBlue()),
                   getAlpha());
}

//==============================================================================
Color Color::greyLevel (f32 brightness) noexcept
{
    auto level = ColorHelpers::floatToUInt8 (brightness);
    return Color (level, level, level);
}

//==============================================================================
Color Color::contrasting (f32 amount) const noexcept
{
   return overlaidWith ((getPerceivedBrightness() >= 0.5f
                           ? Colors::black
                           : Colors::white).withAlpha (amount));
}

Color Color::contrasting (Color target, f32 minContrast) const noexcept
{
    ColorHelpers::YIQ bg (*this);
    ColorHelpers::YIQ fg (target);

    if (std::abs (bg.y - fg.y) >= minContrast)
        return target;

    auto y1 = jmax (0.0f, bg.y - minContrast);
    auto y2 = jmin (1.0f, bg.y + minContrast);
    fg.y = (std::abs (y1 - bg.y) > std::abs (y2 - bg.y)) ? y1 : y2;

    return fg.toColor();
}

Color Color::contrasting (Color colour1,
                            Color colour2) noexcept
{
    auto b1 = colour1.getPerceivedBrightness();
    auto b2 = colour2.getPerceivedBrightness();
    f32 best = 0.0f, bestDist = 0.0f;

    for (f32 i = 0.0f; i < 1.0f; i += 0.02f)
    {
        auto d1 = std::abs (i - b1);
        auto d2 = std::abs (i - b2);
        auto dist = jmin (d1, d2, 1.0f - d1, 1.0f - d2);

        if (dist > bestDist)
        {
            best = i;
            bestDist = dist;
        }
    }

    return colour1.overlaidWith (colour2.withMultipliedAlpha (0.5f))
                  .withBrightness (best);
}

//==============================================================================
Txt Color::toString() const
{
    return Txt::toHexString ((i32) argb.getInARGBMaskOrder());
}

Color Color::fromString (StringRef encodedColorString)
{
    return Color (CharacterFunctions::HexParser<u32>::parse (encodedColorString.text));
}

Txt Color::toDisplayString (const b8 includeAlphaValue) const
{
    return Txt::toHexString ((i32) (argb.getInARGBMaskOrder() & (includeAlphaValue ? 0xffffffff : 0xffffff)))
                  .paddedLeft ('0', includeAlphaValue ? 8 : 6)
                  .toUpperCase();
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class ColorTests final : public UnitTest
{
public:
    ColorTests()
        : UnitTest ("Color", UnitTestCategories::graphics)
    {}

    z0 runTest() override
    {
        auto testColor = [this] (Color colour,
                                  u8 expectedRed, u8 expectedGreen, u8 expectedBlue,
                                  u8 expectedAlpha = 255, f32 expectedFloatAlpha = 1.0f)
        {
            expectEquals (colour.getRed(),        expectedRed);
            expectEquals (colour.getGreen(),      expectedGreen);
            expectEquals (colour.getBlue(),       expectedBlue);
            expectEquals (colour.getAlpha(),      expectedAlpha);
            expectEquals (colour.getFloatAlpha(), expectedFloatAlpha);
        };

        beginTest ("Constructors");
        {
            Color c1;
            testColor (c1, (u8) 0, (u8) 0, (u8) 0, (u8) 0, 0.0f);

            Color c2 ((u32) 0);
            testColor (c2, (u8) 0, (u8) 0, (u8) 0, (u8) 0, 0.0f);

            Color c3 ((u32) 0xffffffff);
            testColor (c3, (u8) 255, (u8) 255, (u8) 255, (u8) 255, 1.0f);

            Color c4 (0, 0, 0);
            testColor (c4, (u8) 0, (u8) 0, (u8) 0, (u8) 255, 1.0f);

            Color c5 (255, 255, 255);
            testColor (c5, (u8) 255, (u8) 255, (u8) 255, (u8) 255, 1.0f);

            Color c6 ((u8) 0, (u8) 0, (u8) 0, (u8) 0);
            testColor (c6, (u8) 0, (u8) 0, (u8) 0, (u8) 0, 0.0f);

            Color c7 ((u8) 255, (u8) 255, (u8) 255, (u8) 255);
            testColor (c7, (u8) 255, (u8) 255, (u8) 255, (u8) 255, 1.0f);

            Color c8 ((u8) 0, (u8) 0, (u8) 0, 0.0f);
            testColor (c8, (u8) 0, (u8) 0, (u8) 0, (u8) 0, 0.0f);

            Color c9 ((u8) 255, (u8) 255, (u8) 255, 1.0f);
            testColor (c9, (u8) 255, (u8) 255, (u8) 255, (u8) 255, 1.0f);
        }

        beginTest ("HSV");
        {
            // black
            testColor (Color::fromHSV (0.0f, 0.0f, 0.0f, 1.0f), 0, 0, 0);
            // white
            testColor (Color::fromHSV (0.0f, 0.0f, 1.0f, 1.0f), 255, 255, 255);
            // red
            testColor (Color::fromHSV (0.0f, 1.0f, 1.0f, 1.0f), 255, 0, 0);
            testColor (Color::fromHSV (1.0f, 1.0f, 1.0f, 1.0f), 255, 0, 0);
            // lime
            testColor (Color::fromHSV (120 / 360.0f, 1.0f, 1.0f, 1.0f), 0, 255, 0);
            // blue
            testColor (Color::fromHSV (240 / 360.0f, 1.0f, 1.0f, 1.0f), 0, 0, 255);
            // yellow
            testColor (Color::fromHSV (60 / 360.0f, 1.0f, 1.0f, 1.0f), 255, 255, 0);
            // cyan
            testColor (Color::fromHSV (180 / 360.0f, 1.0f, 1.0f, 1.0f), 0, 255, 255);
            // magenta
            testColor (Color::fromHSV (300 / 360.0f, 1.0f, 1.0f, 1.0f), 255, 0, 255);
            // silver
            testColor (Color::fromHSV (0.0f, 0.0f, 0.75f, 1.0f), 191, 191, 191);
            // grey
            testColor (Color::fromHSV (0.0f, 0.0f, 0.5f, 1.0f), 128, 128, 128);
            // maroon
            testColor (Color::fromHSV (0.0f, 1.0f, 0.5f, 1.0f), 128, 0, 0);
            // olive
            testColor (Color::fromHSV (60 / 360.0f, 1.0f, 0.5f, 1.0f), 128, 128, 0);
            // green
            testColor (Color::fromHSV (120 / 360.0f, 1.0f, 0.5f, 1.0f), 0, 128, 0);
            // purple
            testColor (Color::fromHSV (300 / 360.0f, 1.0f, 0.5f, 1.0f), 128, 0, 128);
            // teal
            testColor (Color::fromHSV (180 / 360.0f, 1.0f, 0.5f, 1.0f), 0, 128, 128);
            // navy
            testColor (Color::fromHSV (240 / 360.0f, 1.0f, 0.5f, 1.0f), 0, 0, 128);
        }

        beginTest ("HSL");
        {
            // black
            testColor (Color::fromHSL (0.0f, 0.0f, 0.0f, 1.0f), 0, 0, 0);
            // white
            testColor (Color::fromHSL (0.0f, 0.0f, 1.0f, 1.0f), 255, 255, 255);
            // red
            testColor (Color::fromHSL (0.0f, 1.0f, 0.5f, 1.0f), 255, 0, 0);
            testColor (Color::fromHSL (1.0f, 1.0f, 0.5f, 1.0f), 255, 0, 0);
            // lime
            testColor (Color::fromHSL (120 / 360.0f, 1.0f, 0.5f, 1.0f), 0, 255, 0);
            // blue
            testColor (Color::fromHSL (240 / 360.0f, 1.0f, 0.5f, 1.0f), 0, 0, 255);
            // yellow
            testColor (Color::fromHSL (60 / 360.0f, 1.0f, 0.5f, 1.0f), 255, 255, 0);
            // cyan
            testColor (Color::fromHSL (180 / 360.0f, 1.0f, 0.5f, 1.0f), 0, 255, 255);
            // magenta
            testColor (Color::fromHSL (300 / 360.0f, 1.0f, 0.5f, 1.0f), 255, 0, 255);
            // silver
            testColor (Color::fromHSL (0.0f, 0.0f, 0.75f, 1.0f), 191, 191, 191);
            // grey
            testColor (Color::fromHSL (0.0f, 0.0f, 0.5f, 1.0f), 128, 128, 128);
            // maroon
            testColor (Color::fromHSL (0.0f, 1.0f, 0.25f, 1.0f), 128, 0, 0);
            // olive
            testColor (Color::fromHSL (60 / 360.0f, 1.0f, 0.25f, 1.0f), 128, 128, 0);
            // green
            testColor (Color::fromHSL (120 / 360.0f, 1.0f, 0.25f, 1.0f), 0, 128, 0);
            // purple
            testColor (Color::fromHSL (300 / 360.0f, 1.0f, 0.25f, 1.0f), 128, 0, 128);
            // teal
            testColor (Color::fromHSL (180 / 360.0f, 1.0f, 0.25f, 1.0f), 0, 128, 128);
            // navy
            testColor (Color::fromHSL (240 / 360.0f, 1.0f, 0.25f, 1.0f), 0, 0, 128);
        }

        beginTest ("Modifiers");
        {
            Color red (255, 0, 0);
            testColor (red, 255, 0, 0);

            testColor (red.withHue (120.0f / 360.0f), 0, 255, 0);
            testColor (red.withSaturation (0.5f), 255, 128, 128);
            testColor (red.withSaturationHSL (0.5f), 191, 64, 64);
            testColor (red.withBrightness (0.5f), 128, 0, 0);
            testColor (red.withLightness (1.0f), 255, 255, 255);
            testColor (red.withRotatedHue (120.0f / 360.0f), 0, 255, 0);
            testColor (red.withRotatedHue (480.0f / 360.0f), 0, 255, 0);
            testColor (red.withRotatedHue (-240.0f / 360.0f), 0, 255, 0);
            testColor (red.withRotatedHue (-600.0f / 360.0f), 0, 255, 0);
            testColor (red.withMultipliedSaturation (0.0f), 255, 255, 255);
            testColor (red.withMultipliedSaturationHSL (0.0f), 128, 128, 128);
            testColor (red.withMultipliedBrightness (0.5f), 128, 0, 0);
            testColor (red.withMultipliedLightness (2.0f), 255, 255, 255);
            testColor (red.withMultipliedLightness (1.0f), 255, 0, 0);
            testColor (red.withLightness (red.getLightness()), 255, 0, 0);
        }
    }
};

static ColorTests colourTests;

#endif

} // namespace drx
