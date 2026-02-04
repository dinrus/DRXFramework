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
#if DRX_MSVC
 #pragma pack (push, 1)
#endif

class PixelRGB;
class PixelAlpha;

inline u32 maskPixelComponents (u32 x) noexcept
{
    return (x >> 8) & 0x00ff00ff;
}

inline u32 clampPixelComponents (u32 x) noexcept
{
    return (x | (0x01000100 - maskPixelComponents (x))) & 0x00ff00ff;
}

//==============================================================================
/**
    Represents a 32-bit INTERNAL pixel with premultiplied alpha, and can perform compositing
    operations with it.

    This is used internally by the imaging classes.

    @see PixelRGB

    @tags{Graphics}
*/
class DRX_API  PixelARGB
{
public:
    /** Creates a pixel without defining its colour. */
    PixelARGB() noexcept = default;

    PixelARGB (u8 a, u8 r, u8 g, u8 b) noexcept
    {
        components.b = b;
        components.g = g;
        components.r = r;
        components.a = a;
    }

    //==============================================================================
    /** Returns a u32 which represents the pixel in a platform dependent format. */
    forcedinline u32 getNativeARGB() const noexcept   { return internal; }

    /** Returns a u32 which will be in argb order as if constructed with the following mask operation
        ((alpha << 24) | (red << 16) | (green << 8) | blue). */
    forcedinline u32 getInARGBMaskOrder() const noexcept
    {
       #if DRX_ANDROID
        return (u32) ((components.a << 24) | (components.r << 16) | (components.g << 8) | (components.b << 0));
       #else
        return getNativeARGB();
       #endif
    }

    /** Returns a u32 which when written to memory, will be in the order a, r, g, b. In other words,
        if the return-value is read as a u8 array then the elements will be in the order of a, r, g, b*/
    inline u32 getInARGBMemoryOrder() const noexcept
    {
       #if DRX_BIG_ENDIAN
        return getInARGBMaskOrder();
       #else
        return (u32) ((components.b << 24) | (components.g << 16) | (components.r << 8) | components.a);
       #endif
    }

    /** Return channels with an even index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent. */
    forcedinline u32 getEvenBytes() const noexcept { return 0x00ff00ff & internal; }

    /** Return channels with an odd index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent. */
    forcedinline u32 getOddBytes() const noexcept  { return 0x00ff00ff & (internal >> 8); }

    //==============================================================================
    forcedinline u8 getAlpha() const noexcept      { return components.a; }
    forcedinline u8 getRed()   const noexcept      { return components.r; }
    forcedinline u8 getGreen() const noexcept      { return components.g; }
    forcedinline u8 getBlue()  const noexcept      { return components.b; }

    //==============================================================================
    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
    */
    template <class Pixel>
    forcedinline z0 set (const Pixel& src) noexcept
    {
        internal = src.getNativeARGB();
    }

    //==============================================================================
    /** Sets the pixel's colour from individual components. */
    z0 setARGB (u8 a, u8 r, u8 g, u8 b) noexcept
    {
        components.b = b;
        components.g = g;
        components.r = r;
        components.a = a;
    }

    //==============================================================================
    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline z0 blend (const Pixel& src) noexcept
    {
        auto rb = src.getEvenBytes();
        auto ag = src.getOddBytes();

        const auto alpha = 0x100 - (ag >> 16);

        rb += maskPixelComponents (getEvenBytes() * alpha);
        ag += maskPixelComponents (getOddBytes() * alpha);

        internal = clampPixelComponents (rb) | (clampPixelComponents (ag) << 8);
    }

    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    forcedinline z0 blend (PixelRGB src) noexcept;


    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.
    */
    template <class Pixel>
    forcedinline z0 blend (const Pixel& src, u32 extraAlpha) noexcept
    {
        auto rb = maskPixelComponents (extraAlpha * src.getEvenBytes());
        auto ag = maskPixelComponents (extraAlpha * src.getOddBytes());

        const auto alpha = 0x100 - (ag >> 16);

        rb += maskPixelComponents (getEvenBytes() * alpha);
        ag += maskPixelComponents (getOddBytes() * alpha);

        internal = clampPixelComponents (rb) | (clampPixelComponents (ag) << 8);
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline z0 tween (const Pixel& src, u32 amount) noexcept
    {
        auto dEvenBytes = getEvenBytes();
        dEvenBytes += (((src.getEvenBytes() - dEvenBytes) * amount) >> 8);
        dEvenBytes &= 0x00ff00ff;

        auto dOddBytes = getOddBytes();
        dOddBytes += (((src.getOddBytes() - dOddBytes) * amount) >> 8);
        dOddBytes &= 0x00ff00ff;
        dOddBytes <<= 8;

        dOddBytes |= dEvenBytes;
        internal = dOddBytes;
    }

    //==============================================================================
    /** Replaces the colour's alpha value with another one. */
    forcedinline z0 setAlpha (u8 newAlpha) noexcept
    {
        components.a = newAlpha;
    }

    /** Multiplies the colour's alpha value with another one. */
    forcedinline z0 multiplyAlpha (i32 multiplier) noexcept
    {
        // increment alpha by 1, so that if multiplier == 255 (full alpha),
        // this function will not change the values.
        ++multiplier;

        internal = ((((u32) multiplier) * getOddBytes()) & 0xff00ff00)
                | (((((u32) multiplier) * getEvenBytes()) >> 8) & 0x00ff00ff);
    }

    forcedinline z0 multiplyAlpha (f32 multiplier) noexcept
    {
        multiplyAlpha ((i32) (multiplier * 255.0f));
    }


    inline PixelARGB getUnpremultiplied() const noexcept
    {
        PixelARGB p (internal);
        p.unpremultiply();
        return p;
    }

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline z0 premultiply() noexcept
    {
        const auto alpha = components.a;

        if (alpha < 0xff)
        {
            if (alpha == 0)
            {
                components.b = 0;
                components.g = 0;
                components.r = 0;
            }
            else
            {
                components.b = (u8) ((components.b * alpha + 0x7f) >> 8);
                components.g = (u8) ((components.g * alpha + 0x7f) >> 8);
                components.r = (u8) ((components.r * alpha + 0x7f) >> 8);
            }
        }
    }

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline z0 unpremultiply() noexcept
    {
        const auto alpha = components.a;

        if (alpha < 0xff)
        {
            if (alpha == 0)
            {
                components.b = 0;
                components.g = 0;
                components.r = 0;
            }
            else
            {
                components.b = (u8) jmin ((u32) 0xffu, (components.b * 0xffu) / alpha);
                components.g = (u8) jmin ((u32) 0xffu, (components.g * 0xffu) / alpha);
                components.r = (u8) jmin ((u32) 0xffu, (components.r * 0xffu) / alpha);
            }
        }
    }

    forcedinline z0 desaturate() noexcept
    {
        if (components.a < 0xff && components.a > 0)
        {
            const auto newUnpremultipliedLevel = (0xff * ((i32) components.r + (i32) components.g + (i32) components.b) / (3 * components.a));

            components.r = components.g = components.b
                = (u8) ((newUnpremultipliedLevel * components.a + 0x7f) >> 8);
        }
        else
        {
            components.r = components.g = components.b
                = (u8) (((i32) components.r + (i32) components.g + (i32) components.b) / 3);
        }
    }

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
  #if DRX_ANDROID
   #if DRX_BIG_ENDIAN
    enum { indexA = 0, indexR = 3, indexG = 2, indexB = 1 };
   #else
    enum { indexA = 3, indexR = 0, indexG = 1, indexB = 2 };
   #endif
  #else
   #if DRX_BIG_ENDIAN
    enum { indexA = 0, indexR = 1, indexG = 2, indexB = 3 };
   #else
    enum { indexA = 3, indexR = 2, indexG = 1, indexB = 0 };
   #endif
  #endif

private:
    //==============================================================================
    PixelARGB (u32 internalValue) noexcept
        : internal (internalValue)
    {
    }

    //==============================================================================
    struct Components
    {
      #if DRX_ANDROID
       #if DRX_BIG_ENDIAN
        u8 a, b, g, r;
       #else
        u8 r, g, b, a;
       #endif
      #else
       #if DRX_BIG_ENDIAN
        u8 a, r, g, b;
       #else
        u8 b, g, r, a;
       #endif
      #endif
    } DRX_PACKED;

    union
    {
        u32 internal;
        Components components;
    };
}
#ifndef DOXYGEN
 DRX_PACKED
#endif
;


//==============================================================================
/**
    Represents a 24-bit RGB pixel, and can perform compositing operations on it.

    This is used internally by the imaging classes.

    @see PixelARGB

    @tags{Graphics}
*/
class DRX_API  PixelRGB
{
public:
    /** Creates a pixel without defining its colour. */
    PixelRGB() noexcept = default;

    //==============================================================================
    /** Returns a u32 which represents the pixel in a platform dependent format which is compatible
        with the native format of a PixelARGB.

        @see PixelARGB::getNativeARGB */
    forcedinline u32 getNativeARGB() const noexcept
    {
       #if DRX_ANDROID
        return (u32) ((0xffu << 24) | r | ((u32) g << 8) | ((u32) b << 16));
       #else
        return (u32) ((0xffu << 24) | b | ((u32) g << 8) | ((u32) r << 16));
       #endif
    }

    /** Returns a u32 which will be in argb order as if constructed with the following mask operation
        ((alpha << 24) | (red << 16) | (green << 8) | blue). */
    forcedinline u32 getInARGBMaskOrder() const noexcept
    {
       #if DRX_ANDROID
        return (u32) ((0xffu << 24) | b | ((u32) g << 8) | ((u32) r << 16));
       #else
        return getNativeARGB();
       #endif
    }

    /** Returns a u32 which when written to memory, will be in the order a, r, g, b. In other words,
        if the return-value is read as a u8 array then the elements will be in the order of a, r, g, b*/
    inline u32 getInARGBMemoryOrder() const noexcept
    {
       #if DRX_BIG_ENDIAN
        return getInARGBMaskOrder();
       #else
        return (u32) ((b << 24) | (g << 16) | (r << 8) | 0xff);
       #endif
    }

    /** Return channels with an even index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent but compatible with the
        return value of getEvenBytes of the PixelARGB class.

        @see PixelARGB::getEvenBytes */
    forcedinline u32 getEvenBytes() const noexcept
    {
       #if DRX_ANDROID
        return (u32) (r | (b << 16));
       #else
        return (u32) (b | (r << 16));
       #endif
    }

    /** Return channels with an odd index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent but compatible with the
        return value of getOddBytes of the PixelARGB class.

        @see PixelARGB::getOddBytes */
    forcedinline u32 getOddBytes() const noexcept       { return (u32) 0xff0000 | g; }

    //==============================================================================
    forcedinline u8 getAlpha() const noexcept    { return 0xff; }
    forcedinline u8 getRed()   const noexcept    { return r; }
    forcedinline u8 getGreen() const noexcept    { return g; }
    forcedinline u8 getBlue()  const noexcept    { return b; }

    //==============================================================================
    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
        Because PixelRGB has no alpha channel, any alpha value in the source pixel
        is thrown away.
    */
    template <class Pixel>
    forcedinline z0 set (const Pixel& src) noexcept
    {
        b = src.getBlue();
        g = src.getGreen();
        r = src.getRed();
    }

    /** Sets the pixel's colour from individual components. */
    z0 setARGB (u8, u8 red, u8 green, u8 blue) noexcept
    {
        r = red;
        g = green;
        b = blue;
    }

    //==============================================================================
    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline z0 blend (const Pixel& src) noexcept
    {
        const auto alpha = (u32) (0x100 - src.getAlpha());

        // getEvenBytes returns 0x00rr00bb on non-android
        const auto rb = clampPixelComponents (src.getEvenBytes() + maskPixelComponents (getEvenBytes() * alpha));
        // getOddBytes returns 0x00aa00gg on non-android
        const auto ag = clampPixelComponents (src.getOddBytes() + ((g * alpha) >> 8));

        g = (u8) (ag & 0xff);

       #if DRX_ANDROID
        b = (u8) (rb >> 16);
        r = (u8) (rb & 0xff);
       #else
        r = (u8) (rb >> 16);
        b = (u8) (rb & 0xff);
       #endif
    }

    forcedinline z0 blend (PixelRGB src) noexcept
    {
        set (src);
    }

    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.
    */
    template <class Pixel>
    forcedinline z0 blend (const Pixel& src, u32 extraAlpha) noexcept
    {
        auto ag = maskPixelComponents (extraAlpha * src.getOddBytes());
        auto rb = maskPixelComponents (extraAlpha * src.getEvenBytes());

        const auto alpha = 0x100 - (ag >> 16);

        ag = clampPixelComponents (ag + (g * alpha >> 8));
        rb = clampPixelComponents (rb + maskPixelComponents (getEvenBytes() * alpha));

        g = (u8) (ag & 0xff);

       #if DRX_ANDROID
        b = (u8) (rb >> 16);
        r = (u8) (rb & 0xff);
       #else
        r = (u8) (rb >> 16);
        b = (u8) (rb & 0xff);
       #endif
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline z0 tween (const Pixel& src, u32 amount) noexcept
    {
        auto dEvenBytes = getEvenBytes();
        dEvenBytes += (((src.getEvenBytes() - dEvenBytes) * amount) >> 8);

        auto dOddBytes = getOddBytes();
        dOddBytes += (((src.getOddBytes() - dOddBytes) * amount) >> 8);

        g = (u8) (dOddBytes & 0xff);  // dOddBytes =  0x00aa00gg

       #if DRX_ANDROID
        r = (u8) (dEvenBytes & 0xff); // dEvenBytes = 0x00bb00rr
        b = (u8) (dEvenBytes >> 16);
       #else
        b = (u8) (dEvenBytes & 0xff); // dEvenBytes = 0x00rr00bb
        r = (u8) (dEvenBytes >> 16);
       #endif
    }

    //==============================================================================
    /** This method is included for compatibility with the PixelARGB class. */
    forcedinline z0 setAlpha (u8) noexcept {}

    /** Multiplies the colour's alpha value with another one. */
    forcedinline z0 multiplyAlpha (i32) noexcept {}

    /** Multiplies the colour's alpha value with another one. */
    forcedinline z0 multiplyAlpha (f32) noexcept {}

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline z0 premultiply() noexcept {}

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline z0 unpremultiply() noexcept {}

    forcedinline z0 desaturate() noexcept
    {
        r = g = b = (u8) (((i32) r + (i32) g + (i32) b) / 3);
    }

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
   #if DRX_MAC || DRX_IOS
    enum { indexR = 0, indexG = 1, indexB = 2 };
   #else
    enum { indexR = 2, indexG = 1, indexB = 0 };
   #endif

private:
    //==============================================================================
    PixelRGB (u32 internal) noexcept
    {
      #if DRX_ANDROID
        b = (u8) (internal >> 16);
        g = (u8) (internal >> 8);
        r = (u8) (internal);
      #else
        r = (u8) (internal >> 16);
        g = (u8) (internal >> 8);
        b = (u8) (internal);
      #endif
    }

    //==============================================================================
   #if DRX_MAC || DRX_IOS
    u8 r, g, b;
   #else
    u8 b, g, r;
   #endif

}
#ifndef DOXYGEN
 DRX_PACKED
#endif
;

forcedinline z0 PixelARGB::blend (PixelRGB src) noexcept
{
    set (src);
}

//==============================================================================
/**
    Represents an 8-bit single-channel pixel, and can perform compositing operations on it.

    This is used internally by the imaging classes.

    @see PixelARGB, PixelRGB

    @tags{Graphics}
*/
class DRX_API  PixelAlpha
{
public:
    /** Creates a pixel without defining its colour. */
    PixelAlpha() noexcept = default;

    //==============================================================================
    /** Returns a u32 which represents the pixel in a platform dependent format which is compatible
        with the native format of a PixelARGB.

        @see PixelARGB::getNativeARGB */
    forcedinline u32 getNativeARGB() const noexcept      { return (u32) ((a << 24) | (a << 16) | (a << 8) | a); }

    /** Returns a u32 which will be in argb order as if constructed with the following mask operation
        ((alpha << 24) | (red << 16) | (green << 8) | blue). */
    forcedinline u32 getInARGBMaskOrder() const noexcept { return getNativeARGB(); }

    /** Returns a u32 which when written to memory, will be in the order a, r, g, b. In other words,
        if the return-value is read as a u8 array then the elements will be in the order of a, r, g, b*/
    inline u32 getInARGBMemoryOrder() const noexcept     { return getNativeARGB(); }

    /** Return channels with an even index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent but compatible with the
        return value of getEvenBytes of the PixelARGB class.

        @see PixelARGB::getEvenBytes */
    forcedinline u32 getEvenBytes() const noexcept      { return (u32) ((a << 16) | a); }

    /** Return channels with an odd index and insert zero bytes between them. This is useful for blending
        operations. The exact channels which are returned is platform dependent but compatible with the
        return value of getOddBytes of the PixelARGB class.

        @see PixelARGB::getOddBytes */
    forcedinline u32 getOddBytes() const noexcept       { return (u32) ((a << 16) | a); }

    //==============================================================================
    forcedinline u8 getAlpha() const noexcept    { return a; }
    forcedinline u8 getRed()   const noexcept    { return 0; }
    forcedinline u8 getGreen() const noexcept    { return 0; }
    forcedinline u8 getBlue()  const noexcept    { return 0; }

    //==============================================================================
    /** Copies another pixel colour over this one.

        This doesn't blend it - this colour is simply replaced by the other one.
    */
    template <class Pixel>
    forcedinline z0 set (const Pixel& src) noexcept
    {
        a = src.getAlpha();
    }

    /** Sets the pixel's colour from individual components. */
    forcedinline z0 setARGB (u8 a_, u8, u8, u8) noexcept
    {
        a = a_;
    }

    //==============================================================================
    /** Blends another pixel onto this one.

        This takes into account the opacity of the pixel being overlaid, and blends
        it accordingly.
    */
    template <class Pixel>
    forcedinline z0 blend (const Pixel& src) noexcept
    {
        const auto srcA = src.getAlpha();
        a = (u8) ((a * (0x100 - srcA) >> 8) + srcA);
    }

    /** Blends another pixel onto this one, applying an extra multiplier to its opacity.

        The opacity of the pixel being overlaid is scaled by the extraAlpha factor before
        being used, so this can blend semi-transparently from a PixelRGB argument.
    */
    template <class Pixel>
    forcedinline z0 blend (const Pixel& src, u32 extraAlpha) noexcept
    {
        ++extraAlpha;
        const auto srcAlpha = (i32) ((extraAlpha * src.getAlpha()) >> 8);
        a = (u8) ((a * (0x100 - srcAlpha) >> 8) + srcAlpha);
    }

    /** Blends another pixel with this one, creating a colour that is somewhere
        between the two, as specified by the amount.
    */
    template <class Pixel>
    forcedinline z0 tween (const Pixel& src, u32 amount) noexcept
    {
        a += (u8) (((src.getAlpha() - a) * amount) >> 8);
    }

    //==============================================================================
    /** Replaces the colour's alpha value with another one. */
    forcedinline z0 setAlpha (u8 newAlpha) noexcept
    {
        a = newAlpha;
    }

    /** Multiplies the colour's alpha value with another one. */
    forcedinline z0 multiplyAlpha (i32 multiplier) noexcept
    {
        ++multiplier;
        a = (u8) ((a * multiplier) >> 8);
    }

    forcedinline z0 multiplyAlpha (f32 multiplier) noexcept
    {
        a = (u8) (a * multiplier);
    }

    /** Premultiplies the pixel's RGB values by its alpha. */
    forcedinline z0 premultiply() noexcept {}

    /** Unpremultiplies the pixel's RGB values. */
    forcedinline z0 unpremultiply() noexcept {}

    forcedinline z0 desaturate() noexcept {}

    //==============================================================================
    /** The indexes of the different components in the byte layout of this type of colour. */
    enum { indexA = 0 };

private:
    //==============================================================================
    PixelAlpha (u32 internal) noexcept
        : a ((u8) (internal >> 24)) { }

    //==============================================================================
    u8 a;
}
#ifndef DOXYGEN
 DRX_PACKED
#endif
;

#if DRX_MSVC
 #pragma pack (pop)
#endif

} // namespace drx
