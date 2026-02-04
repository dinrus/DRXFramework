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
    The latest DRX look-and-feel style, as introduced in 2017.
    @see LookAndFeel, LookAndFeel_V1, LookAndFeel_V2, LookAndFeel_V3

    @tags{GUI}
*/
class DRX_API  LookAndFeel_V4   : public LookAndFeel_V3
{
public:
    /**
         A struct containing the set of colours to apply to the GUI
    */
    class ColorScheme
    {
    public:
        /** The standard set of colours to use. */
        enum UIColor
        {
            windowBackground = 0,
            widgetBackground,
            menuBackground,
            outline,
            defaultText,
            defaultFill,
            highlightedText,
            highlightedFill,
            menuText,

            numColors
        };

        template <typename... ItemColors>
        ColorScheme (ItemColors... coloursToUse)
        {
            static_assert (sizeof... (coloursToUse) == numColors, "Must supply one colour for each UIColor item");
            const Color c[] = { Color (coloursToUse)... };

            for (i32 i = 0; i < numColors; ++i)
                palette[i] = c[i];
        }

        ColorScheme (const ColorScheme&) = default;
        ColorScheme& operator= (const ColorScheme&) = default;

        /** Returns a colour from the scheme */
        Color getUIColor (UIColor colourToGet) const noexcept;

        /** Sets a scheme colour. */
        z0 setUIColor (UIColor colourToSet, Color newColor) noexcept;

        /** Возвращает true, если two ColorPalette objects contain the same colours. */
        b8 operator== (const ColorScheme&) const noexcept;
        /** Returns false if two ColorPalette objects contain the same colours. */
        b8 operator!= (const ColorScheme&) const noexcept;

    private:
        Color palette[numColors];
    };

    //==============================================================================
    /** Creates a LookAndFeel_V4 object with a default colour scheme. */
    LookAndFeel_V4();

    /** Creates a LookAndFeel_V4 object with a given colour scheme. */
    LookAndFeel_V4 (ColorScheme);

    /** Destructor. */
    ~LookAndFeel_V4() override;

    //==============================================================================
    z0 setColorScheme (ColorScheme);
    ColorScheme& getCurrentColorScheme() noexcept       { return currentColorScheme; }

    static ColorScheme getDarkColorScheme();
    static ColorScheme getMidnightColorScheme();
    static ColorScheme getGreyColorScheme();
    static ColorScheme getLightColorScheme();

    //==============================================================================
    Button* createDocumentWindowButton (i32) override;
    z0 positionDocumentWindowButtons (DocumentWindow&, i32, i32, i32, i32, Button*, Button*, Button*, b8) override;
    z0 drawDocumentWindowTitleBar (DocumentWindow&, Graphics&, i32, i32, i32, i32, const Image*, b8) override;

    //==============================================================================
    Font getTextButtonFont (TextButton&, i32 buttonHeight) override;

    z0 drawButtonBackground (Graphics&, Button&, const Color& backgroundColor,
                               b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 drawToggleButton (Graphics&, ToggleButton&,
                           b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;
    z0 drawTickBox (Graphics&, Component&,
                      f32 x, f32 y, f32 w, f32 h,
                      b8 ticked, b8 isEnabled,
                      b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 changeToggleButtonWidthToFitText (ToggleButton&) override;

    //==============================================================================
    AlertWindow* createAlertWindow (const Txt& title, const Txt& message,
                                    const Txt& button1,
                                    const Txt& button2,
                                    const Txt& button3,
                                    MessageBoxIconType iconType,
                                    i32 numButtons, Component* associatedComponent) override;
    z0 drawAlertBox (Graphics&, AlertWindow&, const Rectangle<i32>& textArea, TextLayout&) override;

    i32 getAlertWindowButtonHeight() override;
    Font getAlertWindowTitleFont() override;
    Font getAlertWindowMessageFont() override;
    Font getAlertWindowFont() override;

    //==============================================================================
    z0 drawProgressBar (Graphics&, ProgressBar&, i32 width, i32 height, f64 progress, const Txt&) override;
    b8 isProgressBarOpaque (ProgressBar&) override    { return false; }
    ProgressBar::Style getDefaultProgressBarStyle (const ProgressBar&) override;

    //==============================================================================
    i32 getDefaultScrollbarWidth() override;
    z0 drawScrollbar (Graphics&, ScrollBar&, i32 x, i32 y, i32 width, i32 height, b8 isScrollbarVertical,
                        i32 thumbStartPosition, i32 thumbSize, b8 isMouseOver, b8 isMouseDown) override;

    //==============================================================================
    Path getTickShape (f32 height) override;
    Path getCrossShape (f32 height) override;

    //==============================================================================
    z0 fillTextEditorBackground (Graphics&, i32 width, i32 height, TextEditor&) override;
    z0 drawTextEditorOutline (Graphics&, i32 width, i32 height, TextEditor&) override;

    //==============================================================================
    Button* createFileBrowserGoUpButton() override;

    z0 layoutFileBrowserComponent (FileBrowserComponent&,
                                     DirectoryContentsDisplayComponent*,
                                     FilePreviewComponent*,
                                     ComboBox* currentPathBox,
                                     TextEditor* filenameBox,
                                     Button* goUpButton) override;

    z0 drawFileBrowserRow (Graphics&, i32 width, i32 height,
                             const File& file, const Txt& filename, Image* icon,
                             const Txt& fileSizeDescription, const Txt& fileTimeDescription,
                             b8 isDirectory, b8 isItemSelected, i32 itemIndex,
                             DirectoryContentsDisplayComponent&) override;

    //==============================================================================
    z0 drawPopupMenuItem (Graphics&, const Rectangle<i32>& area,
                            b8 isSeparator, b8 isActive, b8 isHighlighted, b8 isTicked, b8 hasSubMenu,
                            const Txt& text, const Txt& shortcutKeyText,
                            const Drawable* icon, const Color* textColor) override;

    z0 getIdealPopupMenuItemSize (const Txt& text, b8 isSeparator, i32 standardMenuItemHeight,
                                    i32& idealWidth, i32& idealHeight) override;

    z0 drawMenuBarBackground (Graphics&, i32 width, i32 height, b8 isMouseOverBar, MenuBarComponent&) override;

    z0 drawMenuBarItem (Graphics&, i32 width, i32 height,
                          i32 itemIndex, const Txt& itemText,
                          b8 isMouseOverItem, b8 isMenuOpen, b8 isMouseOverBar,
                          MenuBarComponent&) override;

    //==============================================================================
    z0 drawComboBox (Graphics&, i32 width, i32 height, b8 isButtonDown,
                       i32 buttonX, i32 buttonY, i32 buttonW, i32 buttonH,
                       ComboBox&) override;
    Font getComboBoxFont (ComboBox&) override;
    z0 positionComboBoxText (ComboBox&, Label&) override;

    //==============================================================================
    i32 getSliderThumbRadius (Slider&) override;

    z0 drawLinearSlider (Graphics&, i32 x, i32 y, i32 width, i32 height,
                           f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                           Slider::SliderStyle, Slider&) override;

    z0 drawRotarySlider (Graphics&, i32 x, i32 y, i32 width, i32 height,
                           f32 sliderPosProportional, f32 rotaryStartAngle,
                           f32 rotaryEndAngle, Slider&) override;

    z0 drawPointer (Graphics&, f32 x, f32 y, f32 diameter,
                      const Color&, i32 direction) noexcept;

    Label* createSliderTextBox (Slider&) override;

    //==============================================================================
    z0 drawTooltip (Graphics&, const Txt& text, i32 width, i32 height) override;

    //==============================================================================
    z0 drawConcertinaPanelHeader (Graphics&, const Rectangle<i32>& area,
                                    b8 isMouseOver, b8 isMouseDown,
                                    ConcertinaPanel&, Component& panel) override;

    //==============================================================================
    z0 drawLevelMeter (Graphics&, i32, i32, f32) override;

    //==============================================================================
    z0 paintToolbarBackground (Graphics&, i32 width, i32 height, Toolbar&) override;

    z0 paintToolbarButtonLabel (Graphics&, i32 x, i32 y, i32 width, i32 height,
                                  const Txt& text, ToolbarItemComponent&) override;

    //==============================================================================
    z0 drawPropertyPanelSectionHeader (Graphics&, const Txt& name, b8 isOpen, i32 width, i32 height) override;
    z0 drawPropertyComponentBackground (Graphics&, i32 width, i32 height, PropertyComponent&) override;
    z0 drawPropertyComponentLabel (Graphics&, i32 width, i32 height, PropertyComponent&) override;
    Rectangle<i32> getPropertyComponentContentPosition (PropertyComponent&) override;

    //==============================================================================
    z0 drawCallOutBoxBackground (CallOutBox&, Graphics&, const Path&, Image&) override;

    //==============================================================================
    z0 drawStretchableLayoutResizerBar (Graphics&, i32, i32, b8, b8, b8) override;

private:
    //==============================================================================
    static z0 drawLinearProgressBar (Graphics&, const ProgressBar&, i32, i32, f64, const Txt&);
    static z0 drawCircularProgressBar (Graphics&, const ProgressBar&, const Txt&);

    //==============================================================================
    i32 getPropertyComponentIndent (PropertyComponent&);

    //==============================================================================
    z0 initialiseColors();
    ColorScheme currentColorScheme;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V4)
};

} // namespace drx
