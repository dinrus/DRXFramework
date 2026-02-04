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
    Represents a colour or fill pattern to use for rendering paths.

    This is used by the Graphics and DrawablePath classes as a way to encapsulate
    a brush type. It can either be a solid colour, a gradient, or a tiled image.

    @see Graphics::setFillType, DrawablePath::setFill

    @tags{Graphics}
*/
class DRX_API  FillType  final
{
public:
    //==============================================================================
    /** Creates a default fill type, of solid black. */
    FillType() noexcept;

    /** Creates a fill type of a solid colour.
        @see setColor
    */
    FillType (Color colour) noexcept;

    /** Creates a gradient fill type.
        @see setGradient
    */
    FillType (const ColorGradient& gradient);

    /** Creates a gradient fill type.
        @see setGradient
    */
    FillType (ColorGradient&& gradient);

    /** Creates a tiled image fill type. The transform allows you to set the scaling, offset
        and rotation of the pattern.
        @see setTiledImage
    */
    FillType (const Image& image, const AffineTransform& transform) noexcept;

    /** Creates a copy of another FillType. */
    FillType (const FillType&);

    /** Makes a copy of another FillType. */
    FillType& operator= (const FillType&);

    /** Move constructor */
    FillType (FillType&&) noexcept;

    /** Move assignment operator */
    FillType& operator= (FillType&&) noexcept;

    /** Destructor. */
    ~FillType() noexcept;

    //==============================================================================
    /** Возвращает true, если this is a solid colour fill, and not a gradient or image. */
    b8 isColor() const noexcept          { return gradient == nullptr && image.isNull(); }

    /** Возвращает true, если this is a gradient fill. */
    b8 isGradient() const noexcept        { return gradient != nullptr; }

    /** Возвращает true, если this is a tiled image pattern fill. */
    b8 isTiledImage() const noexcept      { return image.isValid(); }

    /** Turns this object into a solid colour fill.
        If the object was an image or gradient, those fields will no longer be valid. */
    z0 setColor (Color newColor) noexcept;

    /** Turns this object into a gradient fill. */
    z0 setGradient (const ColorGradient& newGradient);

    /** Turns this object into a tiled image fill type. The transform allows you to set
        the scaling, offset and rotation of the pattern.
    */
    z0 setTiledImage (const Image& image, const AffineTransform& transform) noexcept;

    /** Changes the opacity that should be used.
        If the fill is a solid colour, this just changes the opacity of that colour. For
        gradients and image tiles, it changes the opacity that will be used for them.
    */
    z0 setOpacity (f32 newOpacity) noexcept;

    /** Returns the current opacity to be applied to the colour, gradient, or image.
        @see setOpacity
    */
    f32 getOpacity() const noexcept       { return colour.getFloatAlpha(); }

    /** Возвращает true, если this fill type is completely transparent. */
    b8 isInvisible() const noexcept;

    /** Returns a copy of this fill, adding the specified transform applied to the
        existing transform.
    */
    FillType transformed (const AffineTransform& transform) const;

    //==============================================================================
    /** The solid colour being used.

        If the fill type is not a solid colour, the alpha channel of this colour indicates
        the opacity that should be used for the fill, and the RGB channels are ignored.
    */
    Color colour;

    /** Returns the gradient that should be used for filling.
        This will be nullptr if the object is some other type of fill.
        If a gradient is active, the overall opacity with which it should be applied
        is indicated by the alpha channel of the colour variable.
    */
    std::unique_ptr<ColorGradient> gradient;

    /** The image that should be used for tiling.
        If an image fill is active, the overall opacity with which it should be applied
        is indicated by the alpha channel of the colour variable.
    */
    Image image;

    /** The transform that should be applied to the image or gradient that's being drawn. */
    AffineTransform transform;

    //==============================================================================
    b8 operator== (const FillType&) const;
    b8 operator!= (const FillType&) const;

private:
    DRX_LEAK_DETECTOR (FillType)
};

} // namespace drx
