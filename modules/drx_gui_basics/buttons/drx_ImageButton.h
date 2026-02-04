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
    As the title suggests, this is a button containing an image.

    The colour and transparency of the image can be set to vary when the
    button state changes.

    @see Button, ShapeButton, TextButton

    @tags{GUI}
*/
class DRX_API  ImageButton  : public Button
{
public:
    //==============================================================================
    /** Creates an ImageButton.

        Use setImage() to specify the image to use. The colours and opacities that
        are specified here can be changed later using setImages().

        @param name                 the name to give the component
    */
    explicit ImageButton (const Txt& name = Txt());

    /** Destructor. */
    ~ImageButton() override;

    //==============================================================================
    /** Sets up the images to draw in various states.

        @param resizeButtonNowToFitThisImage        if true, the button will be immediately
                                                    resized to the same dimensions as the normal image
        @param rescaleImagesWhenButtonSizeChanges   if true, the image will be rescaled to fit the
                                                    button when the button's size changes
        @param preserveImageProportions             if true then any rescaling of the image to fit
                                                    the button will keep the image's x and y proportions
                                                    correct - i.e. it won't distort its shape, although
                                                    this might create gaps around the edges
        @param normalImage                          the image to use when the button is in its normal state.
                                                    button no longer needs it.
        @param imageOpacityWhenNormal               the opacity to use when drawing the normal image.
        @param overlayColorWhenNormal              an overlay colour to use to fill the alpha channel of the
                                                    normal image - if this colour is transparent, no overlay
                                                    will be drawn. The overlay will be drawn over the top of the
                                                    image, so you can basically add a solid or semi-transparent
                                                    colour to the image to brighten or darken it
        @param overImage                            the image to use when the mouse is over the button. If
                                                    you want to use the same image as was set in the normalImage
                                                    parameter, this value can be a null image.
        @param imageOpacityWhenOver                 the opacity to use when drawing the image when the mouse
                                                    is over the button
        @param overlayColorWhenOver                an overlay colour to use to fill the alpha channel of the
                                                    image when the mouse is over - if this colour is transparent,
                                                    no overlay will be drawn
        @param downImage                            an image to use when the button is pressed down. If set
                                                    to a null image, the 'over' image will be drawn instead (or the
                                                    normal image if there isn't an 'over' image either).
        @param imageOpacityWhenDown                 the opacity to use when drawing the image when the button
                                                    is pressed
        @param overlayColorWhenDown                an overlay colour to use to fill the alpha channel of the
                                                    image when the button is pressed down - if this colour is
                                                    transparent, no overlay will be drawn
        @param hitTestAlphaThreshold                if set to zero, the mouse is considered to be over the button
                                                    whenever it's inside the button's bounding rectangle. If
                                                    set to values higher than 0, the mouse will only be
                                                    considered to be over the image when the value of the
                                                    image's alpha channel at that position is greater than
                                                    this level.
    */
    z0 setImages (b8 resizeButtonNowToFitThisImage,
                    b8 rescaleImagesWhenButtonSizeChanges,
                    b8 preserveImageProportions,
                    const Image& normalImage,
                    f32 imageOpacityWhenNormal,
                    Color overlayColorWhenNormal,
                    const Image& overImage,
                    f32 imageOpacityWhenOver,
                    Color overlayColorWhenOver,
                    const Image& downImage,
                    f32 imageOpacityWhenDown,
                    Color overlayColorWhenDown,
                    f32 hitTestAlphaThreshold = 0.0f);

    /** Returns the currently set 'normal' image. */
    Image getNormalImage() const;

    /** Returns the image that's drawn when the mouse is over the button.

        If a valid 'over' image has been set, this will return it; otherwise it'll
        just return the normal image.
    */
    Image getOverImage() const;

    /** Returns the image that's drawn when the button is held down.

        If a valid 'down' image has been set, this will return it; otherwise it'll
        return the 'over' image or normal image, depending on what's available.
    */
    Image getDownImage() const;

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawImageButton (Graphics&, Image*,
                                      i32 imageX, i32 imageY, i32 imageW, i32 imageH,
                                      const Color& overlayColor, f32 imageOpacity, ImageButton&) = 0;
    };

protected:
    //==============================================================================
    /** @internal */
    b8 hitTest (i32 x, i32 y) override;
    /** @internal */
    z0 paintButton (Graphics&, b8, b8) override;

private:
    //==============================================================================
    b8 scaleImageToFit, preserveProportions;
    u8 alphaThreshold;
    Rectangle<i32> imageBounds;
    Image normalImage, overImage, downImage;
    f32 normalOpacity, overOpacity, downOpacity;
    Color normalOverlay, overOverlay, downOverlay;

    Image getCurrentImage() const;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageButton)
};

} // namespace drx
