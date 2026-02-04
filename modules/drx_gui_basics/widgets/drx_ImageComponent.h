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
    A component that simply displays an image.

    Use setImage to give it an image, and it'll display it - simple as that!

    @tags{GUI}
*/
class DRX_API  ImageComponent  : public Component,
                                  public SettableTooltipClient
{
public:
    //==============================================================================
    /** Creates an ImageComponent. */
    ImageComponent (const Txt& componentName = Txt());

    /** Destructor. */
    ~ImageComponent() override;

    //==============================================================================
    /** Sets the image that should be displayed. */
    z0 setImage (const Image& newImage);

    /** Sets the image that should be displayed, and its placement within the component. */
    z0 setImage (const Image& newImage,
                   RectanglePlacement placementToUse);

    /** Returns the current image. */
    const Image& getImage() const;

    /** Sets the method of positioning that will be used to fit the image within the component's bounds.
        By default the positioning is centred, and will fit the image inside the component's bounds
        whilst keeping its aspect ratio correct, but you can change it to whatever layout you need.
    */
    z0 setImagePlacement (RectanglePlacement newPlacement);

    /** Returns the current image placement. */
    RectanglePlacement getImagePlacement() const;

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    Image image;
    RectanglePlacement placement;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComponent)
};

} // namespace drx
