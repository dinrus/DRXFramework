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
    This LookAndFeel subclass implements the drx style from around 2008-12.

    @see LookAndFeel, LookAndFeel_V1, LookAndFeel_V3

    @tags{GUI}
*/
class DRX_API  LookAndFeel_V2  : public LookAndFeel
{
public:
    LookAndFeel_V2();
    ~LookAndFeel_V2() override;

    //==============================================================================
    z0 drawButtonBackground (Graphics&, Button&, const Color& backgroundColor,
                               b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;
    Font getTextButtonFont (TextButton&, i32 buttonHeight) override;

    z0 drawButtonText (Graphics&, TextButton&,
                         b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;
    i32 getTextButtonWidthToFitText (TextButton&, i32 buttonHeight) override;

    z0 drawToggleButton (Graphics&, ToggleButton&,
                           b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 changeToggleButtonWidthToFitText (ToggleButton&) override;

    z0 drawTickBox (Graphics&, Component&,
                      f32 x, f32 y, f32 w, f32 h,
                      b8 ticked, b8 isEnabled,
                      b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 drawDrawableButton (Graphics&, DrawableButton&,
                             b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    //==============================================================================
    AlertWindow* createAlertWindow (const Txt& title, const Txt& message,
                                    const Txt& button1,
                                    const Txt& button2,
                                    const Txt& button3,
                                    MessageBoxIconType iconType,
                                    i32 numButtons, Component* associatedComponent) override;

    z0 drawAlertBox (Graphics&, AlertWindow&, const Rectangle<i32>& textArea, TextLayout&) override;
    i32 getAlertBoxWindowFlags() override;

    Array<i32> getWidthsForTextButtons (AlertWindow&, const Array<TextButton*>&) override;
    i32 getAlertWindowButtonHeight() override;

    /** Override this function to supply a custom font for the alert window title.
        This default implementation will use a boldened and slightly larger version
        of the alert window message font.

        @see getAlertWindowMessageFont.
    */
    Font getAlertWindowTitleFont() override;

    /** Override this function to supply a custom font for the alert window message.
        This default implementation will use the default font with height set to 15.0f.

        @see getAlertWindowTitleFont
    */
    Font getAlertWindowMessageFont() override;

    Font getAlertWindowFont() override;

    //==============================================================================
    z0 drawProgressBar (Graphics&, ProgressBar&, i32 width, i32 height, f64 progress, const Txt& textToShow) override;
    z0 drawSpinningWaitAnimation (Graphics&, const Color& colour, i32 x, i32 y, i32 w, i32 h) override;
    b8 isProgressBarOpaque (ProgressBar&) override;
    ProgressBar::Style getDefaultProgressBarStyle (const ProgressBar&) override;

    //==============================================================================
    b8 areScrollbarButtonsVisible() override;
    z0 drawScrollbarButton (Graphics&, ScrollBar&, i32 width, i32 height, i32 buttonDirection,
                              b8 isScrollbarVertical, b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override;

    z0 drawScrollbar (Graphics&, ScrollBar&, i32 x, i32 y, i32 width, i32 height,
                        b8 isScrollbarVertical, i32 thumbStartPosition, i32 thumbSize,
                        b8 isMouseOver, b8 isMouseDown) override;

    ImageEffectFilter* getScrollbarEffect() override;
    i32 getMinimumScrollbarThumbSize (ScrollBar&) override;
    i32 getDefaultScrollbarWidth() override;
    i32 getScrollbarButtonSize (ScrollBar&) override;

    //==============================================================================
    Path getTickShape (f32 height) override;
    Path getCrossShape (f32 height) override;

    //==============================================================================
    z0 drawTreeviewPlusMinusBox (Graphics&, const Rectangle<f32>& area,
                                   Color backgroundColor, b8 isOpen, b8 isMouseOver) override;
    b8 areLinesDrawnForTreeView (TreeView&) override;
    i32 getTreeViewIndentSize (TreeView&) override;

    //==============================================================================
    z0 fillTextEditorBackground (Graphics&, i32 width, i32 height, TextEditor&) override;
    z0 drawTextEditorOutline (Graphics&, i32 width, i32 height, TextEditor&) override;
    CaretComponent* createCaretComponent (Component* keyFocusOwner) override;

    //==============================================================================
    const Drawable* getDefaultFolderImage() override;
    const Drawable* getDefaultDocumentFileImage() override;

    AttributedString createFileChooserHeaderText (const Txt& title, const Txt& instructions) override;

    z0 drawFileBrowserRow (Graphics&, i32 width, i32 height,
                             const File& file, const Txt& filename, Image* icon,
                             const Txt& fileSizeDescription, const Txt& fileTimeDescription,
                             b8 isDirectory, b8 isItemSelected, i32 itemIndex,
                             DirectoryContentsDisplayComponent&) override;

    Button* createFileBrowserGoUpButton() override;

    z0 layoutFileBrowserComponent (FileBrowserComponent&,
                                     DirectoryContentsDisplayComponent*,
                                     FilePreviewComponent*,
                                     ComboBox* currentPathBox,
                                     TextEditor* filenameBox,
                                     Button* goUpButton) override;

    //==============================================================================
    z0 drawBubble (Graphics&, BubbleComponent&, const Point<f32>& tip, const Rectangle<f32>& body) override;
    z0 setComponentEffectForBubbleComponent (BubbleComponent& bubbleComponent) override;

    z0 drawLasso (Graphics&, Component&) override;

    //==============================================================================
    z0 drawPopupMenuBackground (Graphics&, i32 width, i32 height) override;
    z0 drawPopupMenuBackgroundWithOptions (Graphics&,
                                             i32 width,
                                             i32 height,
                                             const PopupMenu::Options&) override;

    z0 drawPopupMenuItem (Graphics&, const Rectangle<i32>& area,
                            b8 isSeparator, b8 isActive, b8 isHighlighted, b8 isTicked, b8 hasSubMenu,
                            const Txt& text, const Txt& shortcutKeyText,
                            const Drawable* icon, const Color* textColor) override;

    z0 drawPopupMenuItemWithOptions (Graphics&, const Rectangle<i32>& area,
                                       b8 isHighlighted,
                                       const PopupMenu::Item& item,
                                       const PopupMenu::Options&) override;

    z0 drawPopupMenuSectionHeader (Graphics&, const Rectangle<i32>& area,
                                     const Txt& sectionName) override;

    z0 drawPopupMenuSectionHeaderWithOptions (Graphics&, const Rectangle<i32>& area,
                                                const Txt& sectionName,
                                                const PopupMenu::Options&) override;

    Font getPopupMenuFont() override;

    z0 drawPopupMenuUpDownArrow (Graphics&, i32 width, i32 height, b8 isScrollUpArrow) override;

    z0 drawPopupMenuUpDownArrowWithOptions (Graphics&,
                                              i32 width, i32 height,
                                              b8 isScrollUpArrow,
                                              const PopupMenu::Options&) override;

    z0 getIdealPopupMenuItemSize (const Txt& text, b8 isSeparator, i32 standardMenuItemHeight,
                                    i32& idealWidth, i32& idealHeight) override;

    z0 getIdealPopupMenuItemSizeWithOptions (const Txt& text,
                                               b8 isSeparator,
                                               i32 standardMenuItemHeight,
                                               i32& idealWidth,
                                               i32& idealHeight,
                                               const PopupMenu::Options&) override;

    z0 getIdealPopupMenuSectionHeaderSizeWithOptions (const Txt& text,
                                                        i32 standardMenuItemHeight,
                                                        i32& idealWidth,
                                                        i32& idealHeight,
                                                        const PopupMenu::Options&) override;

    i32 getMenuWindowFlags() override;
    z0 preparePopupMenuWindow (Component&) override;

    z0 drawMenuBarBackground (Graphics&, i32 width, i32 height, b8 isMouseOverBar, MenuBarComponent&) override;
    i32 getMenuBarItemWidth (MenuBarComponent&, i32 itemIndex, const Txt& itemText) override;
    Font getMenuBarFont (MenuBarComponent&, i32 itemIndex, const Txt& itemText) override;
    i32 getDefaultMenuBarHeight() override;

    z0 drawMenuBarItem (Graphics&, i32 width, i32 height,
                          i32 itemIndex, const Txt& itemText,
                          b8 isMouseOverItem, b8 isMenuOpen, b8 isMouseOverBar,
                          MenuBarComponent&) override;

    Component* getParentComponentForMenuOptions (const PopupMenu::Options& options) override;

    b8 shouldPopupMenuScaleWithTargetComponent (const PopupMenu::Options& options) override;

    i32 getPopupMenuBorderSize() override;

    i32 getPopupMenuBorderSizeWithOptions (const PopupMenu::Options&) override;

    z0 drawPopupMenuColumnSeparatorWithOptions (Graphics& g,
                                                  const Rectangle<i32>& bounds,
                                                  const PopupMenu::Options&) override;

    i32 getPopupMenuColumnSeparatorWidthWithOptions (const PopupMenu::Options&) override;

    //==============================================================================
    z0 drawComboBox (Graphics&, i32 width, i32 height, b8 isMouseButtonDown,
                       i32 buttonX, i32 buttonY, i32 buttonW, i32 buttonH,
                       ComboBox&) override;
    Font getComboBoxFont (ComboBox&) override;
    Label* createComboBoxTextBox (ComboBox&) override;
    z0 positionComboBoxText (ComboBox&, Label&) override;
    PopupMenu::Options getOptionsForComboBoxPopupMenu (ComboBox&, Label&) override;
    z0 drawComboBoxTextWhenNothingSelected (Graphics&, ComboBox&, Label&) override;

    //==============================================================================
    z0 drawLabel (Graphics&, Label&) override;
    Font getLabelFont (Label&) override;
    BorderSize<i32> getLabelBorderSize (Label&) override;

    //==============================================================================
    z0 drawLinearSlider (Graphics&, i32 x, i32 y, i32 width, i32 height,
                           f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                           Slider::SliderStyle, Slider&) override;

    z0 drawLinearSliderBackground (Graphics&, i32 x, i32 y, i32 width, i32 height,
                                     f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                     Slider::SliderStyle, Slider&) override;

    z0 drawLinearSliderOutline (Graphics&, i32 x, i32 y, i32 width, i32 height,
                                  Slider::SliderStyle, Slider&) override;


    z0 drawLinearSliderThumb (Graphics&, i32 x, i32 y, i32 width, i32 height,
                                f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                Slider::SliderStyle, Slider&) override;

    z0 drawRotarySlider (Graphics&, i32 x, i32 y, i32 width, i32 height,
                           f32 sliderPosProportional, f32 rotaryStartAngle, f32 rotaryEndAngle,
                           Slider&) override;

    i32 getSliderThumbRadius (Slider&) override;
    Button* createSliderButton (Slider&, b8 isIncrement) override;
    Label* createSliderTextBox (Slider&) override;
    ImageEffectFilter* getSliderEffect (Slider&) override;
    Font getSliderPopupFont (Slider&) override;
    i32 getSliderPopupPlacement (Slider&) override;
    Slider::SliderLayout getSliderLayout (Slider&) override;

    //==============================================================================
    Rectangle<i32> getTooltipBounds (const Txt& tipText, Point<i32> screenPos, Rectangle<i32> parentArea) override;
    z0 drawTooltip (Graphics&, const Txt& text, i32 width, i32 height) override;

    //==============================================================================
    Button* createFilenameComponentBrowseButton (const Txt& text) override;
    z0 layoutFilenameComponent (FilenameComponent&, ComboBox* filenameBox, Button* browseButton) override;

    //==============================================================================
    z0 drawConcertinaPanelHeader (Graphics&, const Rectangle<i32>& area,
                                    b8 isMouseOver, b8 isMouseDown,
                                    ConcertinaPanel&, Component& panel) override;

    //==============================================================================
    z0 drawCornerResizer (Graphics&, i32 w, i32 h, b8 isMouseOver, b8 isMouseDragging) override;
    z0 drawResizableFrame (Graphics&, i32 w, i32 h, const BorderSize<i32>&) override;

    //==============================================================================
    z0 fillResizableWindowBackground (Graphics&, i32 w, i32 h, const BorderSize<i32>&, ResizableWindow&) override;
    z0 drawResizableWindowBorder (Graphics&, i32 w, i32 h, const BorderSize<i32>& border, ResizableWindow&) override;

    //==============================================================================
    z0 drawDocumentWindowTitleBar (DocumentWindow&, Graphics&, i32 w, i32 h,
                                     i32 titleSpaceX, i32 titleSpaceW,
                                     const Image* icon, b8 drawTitleTextOnLeft) override;

    Button* createDocumentWindowButton (i32 buttonType) override;

    z0 positionDocumentWindowButtons (DocumentWindow&,
                                        i32 titleBarX, i32 titleBarY, i32 titleBarW, i32 titleBarH,
                                        Button* minimiseButton,
                                        Button* maximiseButton,
                                        Button* closeButton,
                                        b8 positionTitleBarButtonsOnLeft) override;

    //==============================================================================
    std::unique_ptr<DropShadower> createDropShadowerForComponent (Component&) override;
    std::unique_ptr<FocusOutline> createFocusOutlineForComponent (Component&) override;

    //==============================================================================
    z0 drawStretchableLayoutResizerBar (Graphics&, i32 w, i32 h, b8 isVerticalBar,
                                          b8 isMouseOver, b8 isMouseDragging) override;

    //==============================================================================
    z0 drawGroupComponentOutline (Graphics&, i32 w, i32 h, const Txt& text,
                                    const Justification&, GroupComponent&) override;

    //==============================================================================
    i32 getTabButtonSpaceAroundImage() override;
    i32 getTabButtonOverlap (i32 tabDepth) override;
    i32 getTabButtonBestWidth (TabBarButton&, i32 tabDepth) override;
    Rectangle<i32> getTabButtonExtraComponentBounds (const TabBarButton&, Rectangle<i32>& textArea, Component& extraComp) override;

    z0 drawTabButton (TabBarButton&, Graphics&, b8 isMouseOver, b8 isMouseDown) override;
    Font getTabButtonFont (TabBarButton&, f32 height) override;
    z0 drawTabButtonText (TabBarButton&, Graphics&, b8 isMouseOver, b8 isMouseDown) override;
    z0 drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics&) override;
    z0 drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, i32 w, i32 h) override;

    z0 createTabButtonShape (TabBarButton&, Path&,  b8 isMouseOver, b8 isMouseDown) override;
    z0 fillTabButtonShape (TabBarButton&, Graphics&, const Path&, b8 isMouseOver, b8 isMouseDown) override;

    Button* createTabBarExtrasButton() override;

    //==============================================================================
    z0 drawImageButton (Graphics&, Image*,
                          i32 imageX, i32 imageY, i32 imageW, i32 imageH,
                          const Color& overlayColor, f32 imageOpacity, ImageButton&) override;

    //==============================================================================
    z0 drawTableHeaderBackground (Graphics&, TableHeaderComponent&) override;

    z0 drawTableHeaderColumn (Graphics&, TableHeaderComponent&, const Txt& columnName,
                                i32 columnId, i32 width, i32 height, b8 isMouseOver,
                                b8 isMouseDown, i32 columnFlags) override;

    //==============================================================================
    z0 paintToolbarBackground (Graphics&, i32 width, i32 height, Toolbar&) override;

    Button* createToolbarMissingItemsButton (Toolbar&) override;

    z0 paintToolbarButtonBackground (Graphics&, i32 width, i32 height,
                                       b8 isMouseOver, b8 isMouseDown,
                                       ToolbarItemComponent&) override;

    z0 paintToolbarButtonLabel (Graphics&, i32 x, i32 y, i32 width, i32 height,
                                  const Txt& text, ToolbarItemComponent&) override;

    //==============================================================================
    z0 drawPropertyPanelSectionHeader (Graphics&, const Txt& name, b8 isOpen, i32 width, i32 height) override;
    z0 drawPropertyComponentBackground (Graphics&, i32 width, i32 height, PropertyComponent&) override;
    z0 drawPropertyComponentLabel (Graphics&, i32 width, i32 height, PropertyComponent&) override;
    Rectangle<i32> getPropertyComponentContentPosition (PropertyComponent&) override;
    i32 getPropertyPanelSectionHeaderHeight (const Txt& sectionTitle) override;

    //==============================================================================
    z0 drawCallOutBoxBackground (CallOutBox&, Graphics&, const Path& path, Image& cachedImage) override;
    i32 getCallOutBoxBorderSize (const CallOutBox&) override;
    f32 getCallOutBoxCornerSize (const CallOutBox&) override;

    //==============================================================================
    z0 drawLevelMeter (Graphics&, i32 width, i32 height, f32 level) override;

    z0 drawKeymapChangeButton (Graphics&, i32 width, i32 height, Button&, const Txt& keyDescription) override;

    //==============================================================================
    Font getSidePanelTitleFont (SidePanel&) override;
    Justification getSidePanelTitleJustification (SidePanel&) override;
    Path getSidePanelDismissButtonShape (SidePanel&) override;

    //==============================================================================
    /** Draws a 3D raised (or indented) bevel using two colours.

        The bevel is drawn inside the given rectangle, and greater bevel thicknesses
        extend inwards.

        The top-left colour is used for the top- and left-hand edges of the
        bevel; the bottom-right colour is used for the bottom- and right-hand
        edges.

        If useGradient is true, then the bevel fades out to make it look more curved
        and less angular. If sharpEdgeOnOutside is true, the outside of the bevel is
        sharp, and it fades towards the centre; if sharpEdgeOnOutside is false, then
        the centre edges are sharp and it fades towards the outside.
    */
    static z0 drawBevel (Graphics&,
                           i32 x, i32 y, i32 width, i32 height,
                           i32 bevelThickness,
                           const Color& topLeftColor = Colors::white,
                           const Color& bottomRightColor = Colors::black,
                           b8 useGradient = true,
                           b8 sharpEdgeOnOutside = true);

    /** Utility function to draw a shiny, glassy circle (for round LED-type buttons). */
    static z0 drawGlassSphere (Graphics&, f32 x, f32 y, f32 diameter,
                                 const Color&, f32 outlineThickness) noexcept;

    static z0 drawGlassPointer (Graphics&, f32 x, f32 y, f32 diameter,
                                  const Color&, f32 outlineThickness, i32 direction) noexcept;

    /** Utility function to draw a shiny, glassy oblong (for text buttons). */
    static z0 drawGlassLozenge (Graphics&,
                                  f32 x, f32 y, f32 width, f32 height,
                                  const Color&, f32 outlineThickness, f32 cornerSize,
                                  b8 flatOnLeft, b8 flatOnRight, b8 flatOnTop, b8 flatOnBottom) noexcept;

private:
    //==============================================================================
    std::unique_ptr<Drawable> folderImage, documentImage;
    DropShadowEffect bubbleShadow;

    z0 drawShinyButtonShape (Graphics&,
                               f32 x, f32 y, f32 w, f32 h, f32 maxCornerSize,
                               const Color&, f32 strokeWidth,
                               b8 flatOnLeft, b8 flatOnRight, b8 flatOnTop, b8 flatOnBottom) noexcept;

    class GlassWindowButton;
    class SliderLabelComp;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V2)
};

} // namespace drx
