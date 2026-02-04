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
    A drawable object which is a bitmap image.

    @see Drawable

    @tags{GUI}
*/
class DRX_API  DrawableImage  : public Drawable
{
public:
    //==============================================================================
    DrawableImage();
    DrawableImage (const DrawableImage&);

    /** Sets the image that this drawable will render. */
    explicit DrawableImage (const Image& imageToUse);

    /** Destructor. */
    ~DrawableImage() override;

    //==============================================================================
    /** Sets the image that this drawable will render. */
    z0 setImage (const Image& imageToUse);

    /** Returns the current image. */
    const Image& getImage() const noexcept                      { return image; }

    /** Sets the opacity to use when drawing the image. */
    z0 setOpacity (f32 newOpacity);

    /** Returns the image's opacity. */
    f32 getOpacity() const noexcept                           { return opacity; }

    /** Sets a colour to draw over the image's alpha channel.

        By default this is transparent so isn't drawn, but if you set a non-transparent
        colour here, then it will be overlaid on the image, using the image's alpha
        channel as a mask.

        This is handy for doing things like darkening or lightening an image by overlaying
        it with semi-transparent black or white.
    */
    z0 setOverlayColor (Color newOverlayColor);

    /** Returns the overlay colour. */
    Color getOverlayColor() const noexcept                    { return overlayColor; }

    /** Sets the bounding box within which the image should be displayed. */
    z0 setBoundingBox (Parallelogram<f32> newBounds);

    /** Sets the bounding box within which the image should be displayed. */
    z0 setBoundingBox (Rectangle<f32> newBounds);

    /** Returns the position to which the image's top-left corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    Parallelogram<f32> getBoundingBox() const noexcept        { return bounds; }

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    b8 hitTest (i32 x, i32 y) override;
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;
    /** @internal */
    Rectangle<f32> getDrawableBounds() const override;
    /** @internal */
    Path getOutlineAsPath() const override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    b8 setImageInternal (const Image&);

    //==============================================================================
    Image image;
    f32 opacity = 1.0f;
    Color overlayColor { 0 };
    Parallelogram<f32> bounds;

    DrawableImage& operator= (const DrawableImage&);
    DRX_LEAK_DETECTOR (DrawableImage)
};

} // namespace drx
