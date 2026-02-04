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
    Defines a drop-shadow effect.

    @tags{Graphics}
*/
struct DRX_API  DropShadow
{
    /** Creates a default drop-shadow effect. */
    DropShadow() = default;

    /** Creates a drop-shadow object with the given parameters. */
    DropShadow (Color shadowColor, i32 radius, Point<i32> offset) noexcept;

    /** Renders a drop-shadow based on the alpha-channel of the given image. */
    z0 drawForImage (Graphics& g, const Image& srcImage) const;

    /** Renders a drop-shadow based on the shape of a path. */
    z0 drawForPath (Graphics& g, const Path& path) const;

    /** Renders a drop-shadow for a rectangle.
        Note that for speed, this approximates the shadow using gradients.
    */
    z0 drawForRectangle (Graphics& g, const Rectangle<i32>& area) const;

    /** The colour with which to render the shadow.
        In most cases you'll probably want to leave this as black with an alpha
        value of around 0.5
    */
    Color colour { 0x90000000 };

    /** The approximate spread of the shadow. */
    i32 radius { 4 };

    /** The offset of the shadow. */
    Point<i32> offset;
};

//==============================================================================
/**
    An effect filter that adds a drop-shadow behind the image's content.

    (This will only work on images/components that aren't opaque, of course).

    When added to a component, this effect will draw a soft-edged
    shadow based on what gets drawn inside it. The shadow will also
    be applied to the component's children.

    For speed, this doesn't use a proper gaussian blur, but cheats by
    using a simple bilinear filter. If you need a really high-quality
    shadow, check out ImageConvolutionKernel::createGaussianBlur()

    @see Component::setComponentEffect

    @tags{Graphics}
*/
class DRX_API  DropShadowEffect  : public ImageEffectFilter
{
public:
    //==============================================================================
    /** Creates a default drop-shadow effect.
        To customise the shadow's appearance, use the setShadowProperties() method.
    */
    DropShadowEffect();

    /** Destructor. */
    ~DropShadowEffect() override;

    //==============================================================================
    /** Sets up parameters affecting the shadow's appearance. */
    z0 setShadowProperties (const DropShadow& newShadow);

    //==============================================================================
    /** @internal */
    z0 applyEffect (Image& sourceImage, Graphics& destContext, f32 scaleFactor, f32 alpha) override;


private:
    //==============================================================================
    DropShadow shadow;

    DRX_LEAK_DETECTOR (DropShadowEffect)
};

} // namespace drx
