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
    The latest DRX look-and-feel style, as introduced in 2013.
    @see LookAndFeel, LookAndFeel_V1, LookAndFeel_V2

    @tags{GUI}
*/
class DRX_API  LookAndFeel_V3   : public LookAndFeel_V2
{
public:
    LookAndFeel_V3();
    ~LookAndFeel_V3() override;

    //==============================================================================
    z0 drawButtonBackground (Graphics&, Button&, const Color& backgroundColor,
                               b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 drawTableHeaderBackground (Graphics&, TableHeaderComponent&) override;

    z0 drawTreeviewPlusMinusBox (Graphics&, const Rectangle<f32>& area,
                                   Color backgroundColor, b8 isOpen, b8 isMouseOver) override;
    b8 areLinesDrawnForTreeView (TreeView&) override;
    i32 getTreeViewIndentSize (TreeView&) override;

    Button* createDocumentWindowButton (i32 buttonType) override;

    z0 drawComboBox (Graphics&, i32 width, i32 height, b8 isButtonDown,
                       i32 buttonX, i32 buttonY, i32 buttonW, i32 buttonH, ComboBox& box) override;

    z0 drawKeymapChangeButton (Graphics&, i32 width, i32 height, Button& button, const Txt& keyDescription) override;

    z0 drawPopupMenuBackground (Graphics&, i32 width, i32 height) override;
    z0 drawMenuBarBackground (Graphics&, i32 width, i32 height, b8, MenuBarComponent&) override;

    i32 getTabButtonOverlap (i32 tabDepth) override;
    i32 getTabButtonSpaceAroundImage() override;
    z0 drawTabButton (TabBarButton&, Graphics&, b8 isMouseOver, b8 isMouseDown) override;
    z0 drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, i32 w, i32 h) override;

    z0 drawTextEditorOutline (Graphics&, i32 width, i32 height, TextEditor&) override;

    z0 drawStretchableLayoutResizerBar (Graphics&, i32 w, i32 h, b8 isVerticalBar, b8 isMouseOver, b8 isMouseDragging) override;

    b8 areScrollbarButtonsVisible() override;

    z0 drawScrollbar (Graphics&, ScrollBar&, i32 x, i32 y, i32 width, i32 height, b8 isScrollbarVertical,
                        i32 thumbStartPosition, i32 thumbSize, b8 isMouseOver, b8 isMouseDown) override;

    z0 drawLinearSlider (Graphics&, i32 x, i32 y, i32 width, i32 height,
                           f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                           Slider::SliderStyle, Slider&) override;

    z0 drawLinearSliderBackground (Graphics&, i32 x, i32 y, i32 width, i32 height,
                                     f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                     Slider::SliderStyle, Slider&) override;

    z0 drawConcertinaPanelHeader (Graphics&, const Rectangle<i32>& area, b8 isMouseOver, b8 isMouseDown,
                                    ConcertinaPanel&, Component&) override;

    Path getTickShape (f32 height) override;
    Path getCrossShape (f32 height) override;

    static z0 createTabTextLayout (const TabBarButton& button, f32 length, f32 depth, Color colour, TextLayout&);

private:
    Image backgroundTexture;
    Color backgroundTextureBaseColor;
};

} // namespace drx
