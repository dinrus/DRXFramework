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

FillType::FillType() noexcept
    : colour (0xff000000)
{
}

FillType::FillType (Color c) noexcept
    : colour (c)
{
}

FillType::FillType (const ColorGradient& g)
    : colour (0xff000000), gradient (new ColorGradient (g))
{
}

FillType::FillType (ColorGradient&& g)
    : colour (0xff000000), gradient (new ColorGradient (std::move (g)))
{
}

FillType::FillType (const Image& im, const AffineTransform& t) noexcept
    : colour (0xff000000), image (im), transform (t)
{
}

FillType::FillType (const FillType& other)
    : colour (other.colour),
      gradient (createCopyIfNotNull (other.gradient.get())),
      image (other.image),
      transform (other.transform)
{
}

FillType& FillType::operator= (const FillType& other)
{
    if (this != &other)
    {
        colour = other.colour;
        gradient.reset (createCopyIfNotNull (other.gradient.get()));
        image = other.image;
        transform = other.transform;
    }

    return *this;
}

FillType::FillType (FillType&& other) noexcept
    : colour (other.colour),
      gradient (std::move (other.gradient)),
      image (std::move (other.image)),
      transform (other.transform)
{
}

FillType& FillType::operator= (FillType&& other) noexcept
{
    jassert (this != &other); // hopefully the compiler should make this situation impossible!

    colour = other.colour;
    gradient = std::move (other.gradient);
    image = std::move (other.image);
    transform = other.transform;
    return *this;
}

FillType::~FillType() noexcept
{
}

b8 FillType::operator== (const FillType& other) const
{
    return colour == other.colour && image == other.image
            && transform == other.transform
            && (gradient == other.gradient
                 || (gradient != nullptr && other.gradient != nullptr && *gradient == *other.gradient));
}

b8 FillType::operator!= (const FillType& other) const
{
    return ! operator== (other);
}

z0 FillType::setColor (Color newColor) noexcept
{
    gradient.reset();
    image = {};
    colour = newColor;
}

z0 FillType::setGradient (const ColorGradient& newGradient)
{
    if (gradient != nullptr)
    {
        *gradient = newGradient;
    }
    else
    {
        image = {};
        gradient.reset (new ColorGradient (newGradient));
        colour = Colors::black;
    }
}

z0 FillType::setTiledImage (const Image& newImage, const AffineTransform& newTransform) noexcept
{
    gradient.reset();
    image = newImage;
    transform = newTransform;
    colour = Colors::black;
}

z0 FillType::setOpacity (const f32 newOpacity) noexcept
{
    colour = colour.withAlpha (newOpacity);
}

b8 FillType::isInvisible() const noexcept
{
    return colour.isTransparent() || (gradient != nullptr && gradient->isInvisible());
}

FillType FillType::transformed (const AffineTransform& t) const
{
    FillType f (*this);
    f.transform = f.transform.followedBy (t);
    return f;
}

} // namespace drx
