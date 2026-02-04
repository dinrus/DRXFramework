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
    A component that lets the user choose a colour.

    This shows RGB sliders and a colourspace that the user can pick colours from.

    This class is also a ChangeBroadcaster, so listeners can register to be told
    when the colour changes.

    @tags{GUI}
*/
class DRX_API  ColorSelector  : public Component,
                                  public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Options for the type of selector to show. These are passed into the constructor. */
    enum ColorSelectorOptions
    {
        showAlphaChannel    = 1 << 0,   /**< if set, the colour's alpha channel can be changed as well as its RGB. */

        showColorAtTop     = 1 << 1,   /**< if set, a swatch of the colour is shown at the top of the component. */
        editableColor      = 1 << 2,   /**< if set, the colour shows at the top of the component is editable. */
        showSliders         = 1 << 3,   /**< if set, RGB sliders are shown at the bottom of the component. */
        showColorspace     = 1 << 4    /**< if set, a big HSV selector is shown. */
    };

    //==============================================================================
    /** Creates a ColorSelector object.

        The flags are a combination of values from the ColorSelectorOptions enum, specifying
        which of the selector's features should be visible.

        The edgeGap value specifies the amount of space to leave around the edge.

        gapAroundColorSpaceComponent indicates how much of a gap to put around the
        colourspace and hue selector components.
    */
    ColorSelector (i32 flags = (showAlphaChannel | showColorAtTop | showSliders | showColorspace),
                    i32 edgeGap = 4,
                    i32 gapAroundColorSpaceComponent = 7);

    /** Destructor. */
    ~ColorSelector() override;

    //==============================================================================
    /** Returns the colour that the user has currently selected.

        The ColorSelector class is also a ChangeBroadcaster, so listeners can
        register to be told when the colour changes.

        @see setCurrentColor
    */
    Color getCurrentColor() const;

    /** Changes the colour that is currently being shown.

        @param newColor           the new colour to show
        @param notificationType    whether to send a notification of the change to listeners.
                                   A notification will only be sent if the colour has changed.
    */
    z0 setCurrentColor (Color newColor, NotificationType notificationType = sendNotification);

    //==============================================================================
    /** Tells the selector how many preset colour swatches you want to have on the component.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColor(), and
        setSwatchColor(), to return the number of colours you want, and to set and retrieve
        their values.
    */
    virtual i32 getNumSwatches() const;

    /** Called by the selector to find out the colour of one of the swatches.

        Your subclass should return the colour of the swatch with the given index.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColor(), and
        setSwatchColor(), to return the number of colours you want, and to set and retrieve
        their values.
    */
    virtual Color getSwatchColor (i32 index) const;

    /** Called by the selector when the user puts a new colour into one of the swatches.

        Your subclass should change the colour of the swatch with the given index.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColor(), and
        setSwatchColor(), to return the number of colours you want, and to set and retrieve
        their values.
    */
    virtual z0 setSwatchColor (i32 index, const Color& newColor);


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the keyboard.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId              = 0x1007000,    /**< the colour used to fill the component's background. */
        labelTextColorId               = 0x1007001     /**< the colour used for the labels next to the sliders. */
    };

private:
    //==============================================================================
    class SwatchComponent;
    class ColorSpaceView;
    class HueSelectorComp;
    class ColorPreviewComp;

    Color colour;
    f32 h, s, v;
    std::unique_ptr<Slider> sliders[4];
    std::unique_ptr<ColorSpaceView> colourSpace;
    std::unique_ptr<HueSelectorComp> hueSelector;
    std::unique_ptr<ColorPreviewComp> previewComponent;
    OwnedArray<SwatchComponent> swatchComponents;
    i32k flags;
    i32 edgeGap;

    z0 setHue (f32 newH);
    z0 setSV (f32 newS, f32 newV);
    z0 updateHSV();
    z0 update (NotificationType);
    z0 changeColor();
    z0 paint (Graphics&) override;
    z0 resized() override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorSelector)
};

} // namespace drx
