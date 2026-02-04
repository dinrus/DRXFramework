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

#pragma once


//==============================================================================
class ProjucerLookAndFeel : public LookAndFeel_V4
{
public:
    ProjucerLookAndFeel();
    ~ProjucerLookAndFeel() override;

    z0 drawTabButton (TabBarButton& button, Graphics&, b8 isMouseOver, b8 isMouseDown) override;
    i32 getTabButtonBestWidth (TabBarButton&, i32 tabDepth) override;
    z0 drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, i32, i32) override {}

    z0 drawPropertyComponentBackground (Graphics&, i32, i32, PropertyComponent&) override {}
    z0 drawPropertyComponentLabel (Graphics&, i32 width, i32 height, PropertyComponent&) override;
    Rectangle<i32> getPropertyComponentContentPosition (PropertyComponent&) override;

    z0 drawButtonBackground (Graphics&, Button&, const Color& backgroundColor,
                               b8 isMouseOverButton, b8 isButtonDown) override;
    z0 drawButtonText (Graphics&, TextButton&, b8 isMouseOverButton, b8 isButtonDown) override;
    z0 drawToggleButton (Graphics&, ToggleButton&, b8 isMouseOverButton, b8 isButtonDown) override;

    z0 drawTextEditorOutline (Graphics&, i32, i32, TextEditor&) override {}
    z0 fillTextEditorBackground (Graphics&, i32 width, i32 height, TextEditor&) override;

    z0 layoutFileBrowserComponent (FileBrowserComponent&, DirectoryContentsDisplayComponent*,
                                     FilePreviewComponent*, ComboBox* currentPathBox,
                                     TextEditor* filenameBox,Button* goUpButton) override;
    z0 drawFileBrowserRow (Graphics&, i32 width, i32 height, const File&, const Txt& filename, Image* icon,
                             const Txt& fileSizeDescription, const Txt& fileTimeDescription,
                             b8 isDirectory, b8 isItemSelected, i32 itemIndex, DirectoryContentsDisplayComponent&) override;

    z0 drawCallOutBoxBackground (CallOutBox&, Graphics&, const Path&, Image&) override;

    z0 drawMenuBarBackground (Graphics&, i32 width, i32 height, b8 isMouseOverBar, MenuBarComponent&) override;

    z0 drawMenuBarItem (Graphics&, i32 width, i32 height,
                          i32 itemIndex, const Txt& itemText,
                          b8 isMouseOverItem, b8 isMenuOpen, b8 isMouseOverBar,
                          MenuBarComponent&) override;

    z0 drawResizableFrame (Graphics&, i32 w, i32 h, const BorderSize<i32>&) override;

    z0 drawComboBox (Graphics&, i32 width, i32 height, b8 isButtonDown,
                       i32 buttonX, i32 buttonY, i32 buttonW, i32 buttonH,
                       ComboBox&) override;

    z0 drawTreeviewPlusMinusBox (Graphics&, const Rectangle<f32>& area,
                                   Color backgroundColor, b8 isItemOpen, b8 isMouseOver) override;

    ProgressBar::Style getDefaultProgressBarStyle (const ProgressBar&) override;

    //==============================================================================
    static Path getArrowPath (Rectangle<f32> arrowZone, i32 direction,
                              b8 filled, Justification justification);
    static Path getChoiceComponentArrowPath (Rectangle<f32> arrowZone);

    static Font getPropertyComponentFont()                                       { return FontOptions { 14.0f, Font::FontStyleFlags::bold }; }
    static i32 getTextWidthForPropertyComponent (const PropertyComponent& pc)    { return jmin (200, pc.getWidth() / 2); }

    static ColorScheme getProjucerDarkColorScheme()
    {
        return { 0xff323e44, 0xff263238, 0xff323e44,
                 0xff8e989b, 0xffffffff, 0xffa45c94,
                 0xffffffff, 0xff181f22, 0xffffffff };
    }

    //==============================================================================
    z0 setupColors();

private:

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjucerLookAndFeel)
};
