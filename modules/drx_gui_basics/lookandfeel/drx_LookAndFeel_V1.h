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
    The original DRX look-and-feel, as used back from 2002 to about 2007ish.
    @see LookAndFeel, LookAndFeel_V2, LookAndFeel_V3

    @tags{GUI}
*/
class DRX_API  LookAndFeel_V1    : public LookAndFeel_V2
{
public:
    LookAndFeel_V1();
    ~LookAndFeel_V1() override;

    //==============================================================================
    z0 drawButtonBackground (Graphics&, Button&, const Color& backgroundColor,
                               b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 drawToggleButton (Graphics&, ToggleButton&,
                           b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 drawTickBox (Graphics&, Component&, f32 x, f32 y, f32 w, f32 h,
                      b8 ticked, b8 isEnabled,
                      b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 drawProgressBar (Graphics&, ProgressBar&, i32 width, i32 height,
                          f64 progress, const Txt& textToShow) override;

    //==============================================================================
    z0 drawScrollbarButton (Graphics&, ScrollBar&, i32 width, i32 height,
                              i32 buttonDirection, b8 isScrollbarVertical,
                              b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 drawScrollbar (Graphics&, ScrollBar&, i32 x, i32 y, i32 width, i32 height,
                        b8 isScrollbarVertical, i32 thumbStartPosition, i32 thumbSize,
                        b8 isMouseOver, b8 isMouseDown) override;

    ImageEffectFilter* getScrollbarEffect() override;

    //==============================================================================
    z0 drawTextEditorOutline (Graphics&, i32 width, i32 height, TextEditor&) override;

    //==============================================================================
    z0 drawPopupMenuBackground (Graphics&, i32 width, i32 height) override;
    z0 drawMenuBarBackground (Graphics&, i32 width, i32 height, b8 isMouseOverBar, MenuBarComponent&) override;

    //==============================================================================
    z0 drawComboBox (Graphics&, i32 width, i32 height, b8 isButtonDown,
                       i32 buttonX, i32 buttonY, i32 buttonW, i32 buttonH, ComboBox&) override;

    Font getComboBoxFont (ComboBox&) override;

    //==============================================================================
    z0 drawLinearSlider (Graphics&, i32 x, i32 y, i32 width, i32 height,
                           f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                           Slider::SliderStyle, Slider&) override;

    i32 getSliderThumbRadius (Slider&) override;
    Button* createSliderButton (Slider&, b8 isIncrement) override;
    ImageEffectFilter* getSliderEffect (Slider&) override;

    //==============================================================================
    z0 drawCornerResizer (Graphics&, i32 w, i32 h, b8 isMouseOver, b8 isMouseDragging) override;

    Button* createDocumentWindowButton (i32 buttonType) override;

    z0 positionDocumentWindowButtons (DocumentWindow&,
                                        i32 titleBarX, i32 titleBarY, i32 titleBarW, i32 titleBarH,
                                        Button* minimiseButton, Button* maximiseButton, Button* closeButton,
                                        b8 positionTitleBarButtonsOnLeft) override;

private:
    DropShadowEffect scrollbarShadow;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V1)
};

} // namespace drx
