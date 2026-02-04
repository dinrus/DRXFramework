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
LookAndFeel_V2::LookAndFeel_V2()
{
    // initialise the standard set of colours..
    u32k textButtonColor      = 0xffbbbbff;
    u32k textHighlightColor   = 0x401111ee;
    u32k standardOutlineColor = 0xb2808080;

    static u32k standardColors[] =
    {
        TextButton::buttonColorId,                 textButtonColor,
        TextButton::buttonOnColorId,               0xff4444ff,
        TextButton::textColorOnId,                 0xff000000,
        TextButton::textColorOffId,                0xff000000,

        ToggleButton::textColorId,                 0xff000000,
        ToggleButton::tickColorId,                 0xff000000,
        ToggleButton::tickDisabledColorId,         0xff808080,

        TextEditor::backgroundColorId,             0xffffffff,
        TextEditor::textColorId,                   0xff000000,
        TextEditor::highlightColorId,              textHighlightColor,
        TextEditor::highlightedTextColorId,        0xff000000,
        TextEditor::outlineColorId,                0x00000000,
        TextEditor::focusedOutlineColorId,         textButtonColor,
        TextEditor::shadowColorId,                 0x38000000,

        CaretComponent::caretColorId,              0xff000000,

        Label::backgroundColorId,                  0x00000000,
        Label::textColorId,                        0xff000000,
        Label::outlineColorId,                     0x00000000,

        ScrollBar::backgroundColorId,              0x00000000,
        ScrollBar::thumbColorId,                   0xffffffff,

        TreeView::linesColorId,                    0x4c000000,
        TreeView::backgroundColorId,               0x00000000,
        TreeView::dragAndDropIndicatorColorId,     0x80ff0000,
        TreeView::selectedItemBackgroundColorId,   0x00000000,
        TreeView::oddItemsColorId,                 0x00000000,
        TreeView::evenItemsColorId,                0x00000000,

        PopupMenu::backgroundColorId,              0xffffffff,
        PopupMenu::textColorId,                    0xff000000,
        PopupMenu::headerTextColorId,              0xff000000,
        PopupMenu::highlightedTextColorId,         0xffffffff,
        PopupMenu::highlightedBackgroundColorId,   0x991111aa,

        ComboBox::buttonColorId,                   0xffbbbbff,
        ComboBox::outlineColorId,                  standardOutlineColor,
        ComboBox::textColorId,                     0xff000000,
        ComboBox::backgroundColorId,               0xffffffff,
        ComboBox::arrowColorId,                    0x99000000,
        ComboBox::focusedOutlineColorId,           0xffbbbbff,

        PropertyComponent::backgroundColorId,      0x66ffffff,
        PropertyComponent::labelTextColorId,       0xff000000,

        TextPropertyComponent::backgroundColorId,  0xffffffff,
        TextPropertyComponent::textColorId,        0xff000000,
        TextPropertyComponent::outlineColorId,     standardOutlineColor,

        BooleanPropertyComponent::backgroundColorId, 0xffffffff,
        BooleanPropertyComponent::outlineColorId,  standardOutlineColor,

        ListBox::backgroundColorId,                0xffffffff,
        ListBox::outlineColorId,                   standardOutlineColor,
        ListBox::textColorId,                      0xff000000,

        Slider::backgroundColorId,                 0x00000000,
        Slider::thumbColorId,                      textButtonColor,
        Slider::trackColorId,                      0x7fffffff,
        Slider::rotarySliderFillColorId,           0x7f0000ff,
        Slider::rotarySliderOutlineColorId,        0x66000000,
        Slider::textBoxTextColorId,                0xff000000,
        Slider::textBoxBackgroundColorId,          0xffffffff,
        Slider::textBoxHighlightColorId,           textHighlightColor,
        Slider::textBoxOutlineColorId,             standardOutlineColor,

        ResizableWindow::backgroundColorId,        0xff777777,
        //DocumentWindow::textColorId,               0xff000000, // (this is deliberately not set)

        AlertWindow::backgroundColorId,            0xffededed,
        AlertWindow::textColorId,                  0xff000000,
        AlertWindow::outlineColorId,               0xff666666,

        ProgressBar::backgroundColorId,            0xffeeeeee,
        ProgressBar::foregroundColorId,            0xffaaaaee,

        TooltipWindow::backgroundColorId,          0xffeeeebb,
        TooltipWindow::textColorId,                0xff000000,
        TooltipWindow::outlineColorId,             0x4c000000,

        TabbedComponent::backgroundColorId,        0x00000000,
        TabbedComponent::outlineColorId,           0xff777777,
        TabbedButtonBar::tabOutlineColorId,        0x80000000,
        TabbedButtonBar::frontOutlineColorId,      0x90000000,

        Toolbar::backgroundColorId,                0xfff6f8f9,
        Toolbar::separatorColorId,                 0x4c000000,
        Toolbar::buttonMouseOverBackgroundColorId, 0x4c0000ff,
        Toolbar::buttonMouseDownBackgroundColorId, 0x800000ff,
        Toolbar::labelTextColorId,                 0xff000000,
        Toolbar::editingModeOutlineColorId,        0xffff0000,
        Toolbar::customisationDialogBackgroundColorId, 0xfff6f8f9,

        DrawableButton::textColorId,               0xff000000,
        DrawableButton::textColorOnId,             0xff000000,
        DrawableButton::backgroundColorId,         0x00000000,
        DrawableButton::backgroundOnColorId,       0xaabbbbff,

        HyperlinkButton::textColorId,              0xcc1111ee,

        GroupComponent::outlineColorId,            0x66000000,
        GroupComponent::textColorId,               0xff000000,

        BubbleComponent::backgroundColorId,        0xeeeeeebb,
        BubbleComponent::outlineColorId,           0x77000000,

        TableHeaderComponent::textColorId,         0xff000000,
        TableHeaderComponent::backgroundColorId,   0xffe8ebf9,
        TableHeaderComponent::outlineColorId,      0x33000000,
        TableHeaderComponent::highlightColorId,    0x8899aadd,

        DirectoryContentsDisplayComponent::highlightColorId,              textHighlightColor,
        DirectoryContentsDisplayComponent::textColorId,                   0xff000000,
        DirectoryContentsDisplayComponent::highlightedTextColorId,        0xff000000,

        0x1000440, /*LassoComponent::lassoFillColorId*/        0x66dddddd,
        0x1000441, /*LassoComponent::lassoOutlineColorId*/     0x99111111,

        0x1004000, /*KeyboardComponentBase::upDownButtonBackgroundColorId*/  0xffd3d3d3,
        0x1004001, /*KeyboardComponentBase::upDownButtonArrowColorId*/       0xff000000,

        0x1005000, /*MidiKeyboardComponent::whiteNoteColorId*/               0xffffffff,
        0x1005001, /*MidiKeyboardComponent::blackNoteColorId*/               0xff000000,
        0x1005002, /*MidiKeyboardComponent::keySeparatorLineColorId*/        0x66000000,
        0x1005003, /*MidiKeyboardComponent::mouseOverKeyOverlayColorId*/     0x80ffff00,
        0x1005004, /*MidiKeyboardComponent::keyDownOverlayColorId*/          0xffb6b600,
        0x1005005, /*MidiKeyboardComponent::textLabelColorId*/               0xff000000,
        0x1005006, /*MidiKeyboardComponent::shadowColorId*/                  0x4c000000,

        0x1006000, /*MPEKeyboardComponent::whiteNoteColorId*/                0xff1a1c27,
        0x1006001, /*MPEKeyboardComponent::blackNoteColorId*/                0x99f1f1f1,
        0x1006002, /*MPEKeyboardComponent::textLabelColorId*/                0xfff1f1f1,
        0x1006003, /*MPEKeyboardComponent::noteCircleFillColorId*/           0x99ba00ff,
        0x1006004, /*MPEKeyboardComponent::noteCircleOutlineColorId*/        0xfff1f1f1,

        0x1004500, /*CodeEditorComponent::backgroundColorId*/                0xffffffff,
        0x1004502, /*CodeEditorComponent::highlightColorId*/                 textHighlightColor,
        0x1004503, /*CodeEditorComponent::defaultTextColorId*/               0xff000000,
        0x1004504, /*CodeEditorComponent::lineNumberBackgroundId*/            0x44999999,
        0x1004505, /*CodeEditorComponent::lineNumberTextId*/                  0x44000000,

        0x1007000, /*ColorSelector::backgroundColorId*/                     0xffe5e5e5,
        0x1007001, /*ColorSelector::labelTextColorId*/                      0xff000000,

        0x100ad00, /*KeyMappingEditorComponent::backgroundColorId*/          0x00000000,
        0x100ad01, /*KeyMappingEditorComponent::textColorId*/                0xff000000,

        FileSearchPathListComponent::backgroundColorId,        0xffffffff,

        FileChooserDialogBox::titleTextColorId,                0xff000000,

        SidePanel::backgroundColor,                            0xffffffff,
        SidePanel::titleTextColor,                             0xff000000,
        SidePanel::shadowBaseColor,                            0xff000000,
        SidePanel::dismissButtonNormalColor,                   textButtonColor,
        SidePanel::dismissButtonOverColor,                     textButtonColor,
        SidePanel::dismissButtonDownColor,                     0xff4444ff,

        FileBrowserComponent::currentPathBoxBackgroundColorId,    0xffffffff,
        FileBrowserComponent::currentPathBoxTextColorId,          0xff000000,
        FileBrowserComponent::currentPathBoxArrowColorId,         0x99000000,
        FileBrowserComponent::filenameBoxBackgroundColorId,       0xffffffff,
        FileBrowserComponent::filenameBoxTextColorId,             0xff000000,
    };

    for (i32 i = 0; i < numElementsInArray (standardColors); i += 2)
        setColor ((i32) standardColors [i], Color ((u32) standardColors [i + 1]));

    bubbleShadow.setShadowProperties (DropShadow (Colors::black.withAlpha (0.35f), 5, {}));
}

LookAndFeel_V2::~LookAndFeel_V2()  {}

//==============================================================================
z0 LookAndFeel_V2::drawButtonBackground (Graphics& g,
                                           Button& button,
                                           const Color& backgroundColor,
                                           b8 shouldDrawButtonAsHighlighted,
                                           b8 shouldDrawButtonAsDown)
{
    i32k width = button.getWidth();
    i32k height = button.getHeight();

    const f32 outlineThickness = button.isEnabled() ? ((shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted) ? 1.2f : 0.7f) : 0.4f;
    const f32 halfThickness = outlineThickness * 0.5f;

    const f32 indentL = button.isConnectedOnLeft()   ? 0.1f : halfThickness;
    const f32 indentR = button.isConnectedOnRight()  ? 0.1f : halfThickness;
    const f32 indentT = button.isConnectedOnTop()    ? 0.1f : halfThickness;
    const f32 indentB = button.isConnectedOnBottom() ? 0.1f : halfThickness;

    const Color baseColor (detail::LookAndFeelHelpers::createBaseColor (backgroundColor,
                                                                           button.hasKeyboardFocus (true),
                                                                           shouldDrawButtonAsHighlighted,
                                                                           shouldDrawButtonAsDown)
                               .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    drawGlassLozenge (g,
                      indentL,
                      indentT,
                      (f32) width - indentL - indentR,
                      (f32) height - indentT - indentB,
                      baseColor, outlineThickness, -1.0f,
                      button.isConnectedOnLeft(),
                      button.isConnectedOnRight(),
                      button.isConnectedOnTop(),
                      button.isConnectedOnBottom());
}

Font LookAndFeel_V2::getTextButtonFont (TextButton&, i32 buttonHeight)
{
    return withDefaultMetrics (FontOptions (jmin (15.0f, (f32) buttonHeight * 0.6f)));
}

i32 LookAndFeel_V2::getTextButtonWidthToFitText (TextButton& b, i32 buttonHeight)
{
    return GlyphArrangement::getStringWidthInt (getTextButtonFont (b, buttonHeight), b.getButtonText()) + buttonHeight;
}

z0 LookAndFeel_V2::drawButtonText (Graphics& g, TextButton& button,
                                     b8 /*shouldDrawButtonAsHighlighted*/, b8 /*shouldDrawButtonAsDown*/)
{
    Font font (getTextButtonFont (button, button.getHeight()));
    g.setFont (font);
    g.setColor (button.findColor (button.getToggleState() ? TextButton::textColorOnId
                                                            : TextButton::textColorOffId)
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    i32k yIndent = jmin (4, button.proportionOfHeight (0.3f));
    i32k cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    i32k fontHeight = roundToInt (font.getHeight() * 0.6f);
    i32k leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    i32k rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    i32k textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 2);
}

z0 LookAndFeel_V2::drawTickBox (Graphics& g, Component& component,
                                  f32 x, f32 y, f32 w, f32 h,
                                  const b8 ticked,
                                  const b8 isEnabled,
                                  const b8 shouldDrawButtonAsHighlighted,
                                  const b8 shouldDrawButtonAsDown)
{
    const f32 boxSize = w * 0.7f;

    drawGlassSphere (g, x, y + (h - boxSize) * 0.5f, boxSize,
                     detail::LookAndFeelHelpers::createBaseColor (component.findColor (TextButton::buttonColorId)
                                                                            .withMultipliedAlpha (isEnabled ? 1.0f : 0.5f),
                                                                   true, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown),
                     isEnabled ? ((shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted) ? 1.1f : 0.5f) : 0.3f);

    if (ticked)
    {
        Path tick;
        tick.startNewSubPath (1.5f, 3.0f);
        tick.lineTo (3.0f, 6.0f);
        tick.lineTo (6.0f, 0.0f);

        g.setColor (component.findColor (isEnabled ? ToggleButton::tickColorId
                                                     : ToggleButton::tickDisabledColorId));

        const AffineTransform trans (AffineTransform::scale (w / 9.0f, h / 9.0f)
                                                     .translated (x, y));

        g.strokePath (tick, PathStrokeType (2.5f), trans);
    }
}

z0 LookAndFeel_V2::drawToggleButton (Graphics& g, ToggleButton& button,
                                       b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown)
{
    if (button.hasKeyboardFocus (true))
    {
        g.setColor (button.findColor (TextEditor::focusedOutlineColorId));
        g.drawRect (0, 0, button.getWidth(), button.getHeight());
    }

    f32 fontSize = jmin (15.0f, (f32) button.getHeight() * 0.75f);
    const f32 tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, ((f32) button.getHeight() - tickWidth) * 0.5f,
                 tickWidth, tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 shouldDrawButtonAsHighlighted,
                 shouldDrawButtonAsDown);

    g.setColor (button.findColor (ToggleButton::textColorId));
    g.setFont (fontSize);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 5)
                                             .withTrimmedRight (2),
                      Justification::centredLeft, 10);
}

z0 LookAndFeel_V2::changeToggleButtonWidthToFitText (ToggleButton& button)
{
    auto fontSize = jmin (15.0f, (f32) button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    Font font (withDefaultMetrics (FontOptions { fontSize }));

    button.setSize (GlyphArrangement::getStringWidthInt (font, button.getButtonText()) + roundToInt (tickWidth) + 9,
                    button.getHeight());
}

z0 LookAndFeel_V2::drawDrawableButton (Graphics& g, DrawableButton& button,
                                         b8 /*shouldDrawButtonAsHighlighted*/, b8 /*shouldDrawButtonAsDown*/)
{
    b8 toggleState = button.getToggleState();

    g.fillAll (button.findColor (toggleState ? DrawableButton::backgroundOnColorId
                                              : DrawableButton::backgroundColorId));

    i32k textH = (button.getStyle() == DrawableButton::ImageAboveTextLabel)
                        ? jmin (16, button.proportionOfHeight (0.25f))
                        : 0;

    if (textH > 0)
    {
        g.setFont ((f32) textH);

        g.setColor (button.findColor (toggleState ? DrawableButton::textColorOnId
                                                    : DrawableButton::textColorId)
                        .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.4f));

        g.drawFittedText (button.getButtonText(),
                          2, button.getHeight() - textH - 1,
                          button.getWidth() - 4, textH,
                          Justification::centred, 1);
    }
}

//==============================================================================
AlertWindow* LookAndFeel_V2::createAlertWindow (const Txt& title, const Txt& message,
                                                const Txt& button1, const Txt& button2, const Txt& button3,
                                                MessageBoxIconType iconType,
                                                i32 numButtons, Component* associatedComponent)
{
    AlertWindow* aw = new AlertWindow (title, message, iconType, associatedComponent);

    if (numButtons == 1)
    {
        aw->addButton (button1, 0,
                       KeyPress (KeyPress::escapeKey),
                       KeyPress (KeyPress::returnKey));
    }
    else
    {
        const KeyPress button1ShortCut ((i32) CharacterFunctions::toLowerCase (button1[0]), 0, 0);
        KeyPress button2ShortCut ((i32) CharacterFunctions::toLowerCase (button2[0]), 0, 0);
        if (button1ShortCut == button2ShortCut)
            button2ShortCut = KeyPress();

        if (numButtons == 2)
        {
            aw->addButton (button1, 1, KeyPress (KeyPress::returnKey), button1ShortCut);
            aw->addButton (button2, 0, KeyPress (KeyPress::escapeKey), button2ShortCut);
        }
        else if (numButtons == 3)
        {
            aw->addButton (button1, 1, button1ShortCut);
            aw->addButton (button2, 2, button2ShortCut);
            aw->addButton (button3, 0, KeyPress (KeyPress::escapeKey));
        }
    }

    return aw;
}

z0 LookAndFeel_V2::drawAlertBox (Graphics& g, AlertWindow& alert,
                                   const Rectangle<i32>& textArea, TextLayout& textLayout)
{
    g.fillAll (alert.findColor (AlertWindow::backgroundColorId));

    i32 iconSpaceUsed = 0;

    i32k iconWidth = 80;
    i32 iconSize = jmin (iconWidth + 50, alert.getHeight() + 20);

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = jmin (iconSize, textArea.getHeight() + 50);

    const Rectangle<i32> iconRect (iconSize / -10, iconSize / -10,
                                   iconSize, iconSize);

    if (alert.getAlertType() != MessageBoxIconType::NoIcon)
    {
        Path icon;
        u32 colour;
        t8 character;

        if (alert.getAlertType() == MessageBoxIconType::WarningIcon)
        {
            colour = 0x55ff5555;
            character = '!';

            icon.addTriangle ((f32) iconRect.getX() + (f32) iconRect.getWidth() * 0.5f, (f32) iconRect.getY(),
                              (f32) iconRect.getRight(), (f32) iconRect.getBottom(),
                              (f32) iconRect.getX(), (f32) iconRect.getBottom());

            icon = icon.createPathWithRoundedCorners (5.0f);
        }
        else
        {
            colour    = alert.getAlertType() == MessageBoxIconType::InfoIcon ? (u32) 0x605555ff : (u32) 0x40b69900;
            character = alert.getAlertType() == MessageBoxIconType::InfoIcon ? 'i' : '?';

            icon.addEllipse (iconRect.toFloat());
        }

        GlyphArrangement ga;
        ga.addFittedText (withDefaultMetrics (FontOptions ((f32) iconRect.getHeight() * 0.9f, Font::bold)),
                          Txt::charToString ((t32) (u8) character),
                          (f32) iconRect.getX(), (f32) iconRect.getY(),
                          (f32) iconRect.getWidth(), (f32) iconRect.getHeight(),
                          Justification::centred, false);
        ga.createPath (icon);

        icon.setUsingNonZeroWinding (false);
        g.setColor (Color (colour));
        g.fillPath (icon);

        iconSpaceUsed = iconWidth;
    }

    g.setColor (alert.findColor (AlertWindow::textColorId));

    textLayout.draw (g, Rectangle<i32> (textArea.getX() + iconSpaceUsed,
                                        textArea.getY(),
                                        textArea.getWidth() - iconSpaceUsed,
                                        textArea.getHeight()).toFloat());

    g.setColor (alert.findColor (AlertWindow::outlineColorId));
    g.drawRect (0, 0, alert.getWidth(), alert.getHeight());
}

i32 LookAndFeel_V2::getAlertBoxWindowFlags()
{
    return ComponentPeer::windowAppearsOnTaskbar
            | ComponentPeer::windowHasDropShadow;
}

Array<i32> LookAndFeel_V2::getWidthsForTextButtons (AlertWindow&, const Array<TextButton*>& buttons)
{
    i32k n = buttons.size();
    Array<i32> buttonWidths;

    i32k buttonHeight = getAlertWindowButtonHeight();

    for (i32 i = 0; i < n; ++i)
        buttonWidths.add (getTextButtonWidthToFitText (*buttons.getReference (i), buttonHeight));

    return buttonWidths;
}

i32 LookAndFeel_V2::getAlertWindowButtonHeight()
{
    return 28;
}

Font LookAndFeel_V2::getAlertWindowTitleFont()
{
    Font messageFont = getAlertWindowMessageFont();
    return messageFont.withHeight (messageFont.getHeight() * 1.1f).boldened();
}

Font LookAndFeel_V2::getAlertWindowMessageFont()
{
    return withDefaultMetrics (FontOptions (15.0f));
}

Font LookAndFeel_V2::getAlertWindowFont()
{
    return withDefaultMetrics (FontOptions (12.0f));
}

//==============================================================================
z0 LookAndFeel_V2::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                      i32 width, i32 height,
                                      f64 progress, const Txt& textToShow)
{
    const Color background (progressBar.findColor (ProgressBar::backgroundColorId));
    const Color foreground (progressBar.findColor (ProgressBar::foregroundColorId));

    g.fillAll (background);

    if (progress >= 0.0f && progress < 1.0f)
    {
        drawGlassLozenge (g, 1.0f, 1.0f,
                          (f32) jlimit (0.0, width - 2.0, progress * (width - 2.0)),
                          (f32) (height - 2),
                          foreground,
                          0.5f, 0.0f,
                          true, true, true, true);
    }
    else
    {
        // spinning bar..
        g.setColor (foreground);

        i32k stripeWidth = height * 2;
        i32k position = (i32) (Time::getMillisecondCounter() / 15) % stripeWidth;

        Path p;

        for (f32 x = (f32) (- position); x < (f32) (width + stripeWidth); x += (f32) stripeWidth)
            p.addQuadrilateral (x, 0.0f,
                                x + (f32) stripeWidth * 0.5f, 0.0f,
                                x, (f32) height,
                                x - (f32) stripeWidth * 0.5f, (f32) height);

        Image im (Image::ARGB, width, height, true);

        {
            Graphics g2 (im);
            drawGlassLozenge (g2, 1.0f, 1.0f,
                              (f32) (width - 2),
                              (f32) (height - 2),
                              foreground,
                              0.5f, 0.0f,
                              true, true, true, true);
        }

        g.setTiledImageFill (im, 0, 0, 0.85f);
        g.fillPath (p);
    }

    if (textToShow.isNotEmpty())
    {
        g.setColor (Color::contrasting (background, foreground));
        g.setFont ((f32) height * 0.6f);

        g.drawText (textToShow, 0, 0, width, height, Justification::centred, false);
    }
}

z0 LookAndFeel_V2::drawSpinningWaitAnimation (Graphics& g, const Color& colour, i32 x, i32 y, i32 w, i32 h)
{
    const f32 radius = (f32) jmin (w, h) * 0.4f;
    const f32 thickness = radius * 0.15f;
    Path p;
    p.addRoundedRectangle (radius * 0.4f, thickness * -0.5f,
                           radius * 0.6f, thickness,
                           thickness * 0.5f);

    const f32 cx = (f32) x + (f32) w * 0.5f;
    const f32 cy = (f32) y + (f32) h * 0.5f;

    u32k animationIndex = (Time::getMillisecondCounter() / (1000 / 10)) % 12;

    for (u32 i = 0; i < 12; ++i)
    {
        u32k n = (i + 12 - animationIndex) % 12;

        g.setColor (colour.withMultipliedAlpha ((f32) (n + 1) / 12.0f));
        g.fillPath (p, AffineTransform::rotation ((f32) i * (MathConstants<f32>::pi / 6.0f))
                                       .translated (cx, cy));
    }
}

b8 LookAndFeel_V2::isProgressBarOpaque (ProgressBar& progressBar)
{
    return progressBar.findColor (ProgressBar::backgroundColorId).isOpaque();
}

ProgressBar::Style LookAndFeel_V2::getDefaultProgressBarStyle (const ProgressBar&)
{
    return ProgressBar::Style::linear;
}

b8 LookAndFeel_V2::areScrollbarButtonsVisible()
{
    return true;
}

z0 LookAndFeel_V2::drawScrollbarButton (Graphics& g, ScrollBar& scrollbar,
                                          i32 width, i32 height, i32 buttonDirection,
                                          b8 /*isScrollbarVertical*/,
                                          b8 /*shouldDrawButtonAsHighlighted*/,
                                          b8 shouldDrawButtonAsDown)
{
    Path p;

    const auto w = (f32) width;
    const auto h = (f32) height;

    if (buttonDirection == 0)
        p.addTriangle (w * 0.5f, h * 0.2f,
                       w * 0.1f, h * 0.7f,
                       w * 0.9f, h * 0.7f);
    else if (buttonDirection == 1)
        p.addTriangle (w * 0.8f, h * 0.5f,
                       w * 0.3f, h * 0.1f,
                       w * 0.3f, h * 0.9f);
    else if (buttonDirection == 2)
        p.addTriangle (w * 0.5f, h * 0.8f,
                       w * 0.1f, h * 0.3f,
                       w * 0.9f, h * 0.3f);
    else if (buttonDirection == 3)
        p.addTriangle (w * 0.2f, h * 0.5f,
                       w * 0.7f, h * 0.1f,
                       w * 0.7f, h * 0.9f);

    if (shouldDrawButtonAsDown)
        g.setColor (scrollbar.findColor (ScrollBar::thumbColorId).contrasting (0.2f));
    else
        g.setColor (scrollbar.findColor (ScrollBar::thumbColorId));

    g.fillPath (p);

    g.setColor (Color (0x80000000));
    g.strokePath (p, PathStrokeType (0.5f));
}

z0 LookAndFeel_V2::drawScrollbar (Graphics& g,
                                 ScrollBar& scrollbar,
                                 i32 x, i32 y,
                                 i32 width, i32 height,
                                 b8 isScrollbarVertical,
                                 i32 thumbStartPosition,
                                 i32 thumbSize,
                                 b8 /*isMouseOver*/,
                                 b8 /*isMouseDown*/)
{
    g.fillAll (scrollbar.findColor (ScrollBar::backgroundColorId));

    Path slotPath, thumbPath;

    const f32 slotIndent = jmin (width, height) > 15 ? 1.0f : 0.0f;
    const f32 slotIndentx2 = slotIndent * 2.0f;
    const f32 thumbIndent = slotIndent + 1.0f;
    const f32 thumbIndentx2 = thumbIndent * 2.0f;

    f32 gx1 = 0.0f, gy1 = 0.0f, gx2 = 0.0f, gy2 = 0.0f;

    if (isScrollbarVertical)
    {
        slotPath.addRoundedRectangle ((f32) x + slotIndent,
                                      (f32) y + slotIndent,
                                      (f32) width - slotIndentx2,
                                      (f32) height - slotIndentx2,
                                      ((f32) width - slotIndentx2) * 0.5f);

        if (thumbSize > 0)
            thumbPath.addRoundedRectangle ((f32) x + thumbIndent,
                                           (f32) thumbStartPosition + thumbIndent,
                                           (f32) width - thumbIndentx2,
                                           (f32) thumbSize - thumbIndentx2,
                                           ((f32) width - thumbIndentx2) * 0.5f);
        gx1 = (f32) x;
        gx2 = (f32) x + (f32) width * 0.7f;
    }
    else
    {
        slotPath.addRoundedRectangle ((f32) x + slotIndent,
                                      (f32) y + slotIndent,
                                      (f32) width - slotIndentx2,
                                      (f32) height - slotIndentx2,
                                      ((f32) height - slotIndentx2) * 0.5f);

        if (thumbSize > 0)
            thumbPath.addRoundedRectangle ((f32) thumbStartPosition + thumbIndent,
                                           (f32) y + thumbIndent,
                                           (f32) thumbSize - thumbIndentx2,
                                           (f32) height - thumbIndentx2,
                                           ((f32) height - thumbIndentx2) * 0.5f);
        gy1 = (f32) y;
        gy2 = (f32) y + (f32) height * 0.7f;
    }

    const Color thumbColor (scrollbar.findColor (ScrollBar::thumbColorId));
    Color trackColor1, trackColor2;

    if (scrollbar.isColorSpecified (ScrollBar::trackColorId)
         || isColorSpecified (ScrollBar::trackColorId))
    {
        trackColor1 = trackColor2 = scrollbar.findColor (ScrollBar::trackColorId);
    }
    else
    {
        trackColor1 = thumbColor.overlaidWith (Color (0x44000000));
        trackColor2 = thumbColor.overlaidWith (Color (0x19000000));
    }

    g.setGradientFill (ColorGradient (trackColor1, gx1, gy1,
                                       trackColor2, gx2, gy2, false));
    g.fillPath (slotPath);

    if (isScrollbarVertical)
    {
        gx1 = (f32) x + (f32) width * 0.6f;
        gx2 = (f32) x + (f32) width;
    }
    else
    {
        gy1 = (f32) y + (f32) height * 0.6f;
        gy2 = (f32) y + (f32) height;
    }

    g.setGradientFill (ColorGradient (Colors::transparentBlack,gx1, gy1,
                       Color (0x19000000), gx2, gy2, false));
    g.fillPath (slotPath);

    g.setColor (thumbColor);
    g.fillPath (thumbPath);

    g.setGradientFill (ColorGradient (Color (0x10000000), gx1, gy1,
                       Colors::transparentBlack, gx2, gy2, false));

    {
        Graphics::ScopedSaveState ss (g);

        if (isScrollbarVertical)
            g.reduceClipRegion (x + width / 2, y, width, height);
        else
            g.reduceClipRegion (x, y + height / 2, width, height);

        g.fillPath (thumbPath);
    }

    g.setColor (Color (0x4c000000));
    g.strokePath (thumbPath, PathStrokeType (0.4f));
}

ImageEffectFilter* LookAndFeel_V2::getScrollbarEffect()
{
    return nullptr;
}

i32 LookAndFeel_V2::getMinimumScrollbarThumbSize (ScrollBar& scrollbar)
{
    return jmin (scrollbar.getWidth(), scrollbar.getHeight()) * 2;
}

i32 LookAndFeel_V2::getDefaultScrollbarWidth()
{
    return 18;
}

i32 LookAndFeel_V2::getScrollbarButtonSize (ScrollBar& scrollbar)
{
    return 2 + (scrollbar.isVertical() ? scrollbar.getWidth()
                                       : scrollbar.getHeight());
}

//==============================================================================
z0 LookAndFeel_V2::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<f32>& area,
                                               Color /*backgroundColor*/, b8 isOpen, b8 /*isMouseOver*/)
{
    auto boxSize = roundToInt (jmin (16.0f, area.getWidth(), area.getHeight()) * 0.7f) | 1;

    auto x = ((i32) area.getWidth()  - boxSize) / 2 + (i32) area.getX();
    auto y = ((i32) area.getHeight() - boxSize) / 2 + (i32) area.getY();

    Rectangle<f32> boxArea ((f32) x, (f32) y, (f32) boxSize, (f32) boxSize);

    g.setColor (Color (0xe5ffffff));
    g.fillRect (boxArea);

    g.setColor (Color (0x80000000));
    g.drawRect (boxArea);

    auto size = (f32) boxSize * 0.5f + 1.0f;
    auto centre = (f32) (boxSize / 2);

    g.fillRect ((f32) x + ((f32) boxSize - size) * 0.5f, (f32) y + centre, size, 1.0f);

    if (! isOpen)
        g.fillRect ((f32) x + centre, (f32) y + ((f32) boxSize - size) * 0.5f, 1.0f, size);
}

b8 LookAndFeel_V2::areLinesDrawnForTreeView (TreeView&)
{
    return true;
}

i32 LookAndFeel_V2::getTreeViewIndentSize (TreeView&)
{
    return 24;
}

//==============================================================================
z0 LookAndFeel_V2::drawBubble (Graphics& g, BubbleComponent& comp,
                                 const Point<f32>& tip, const Rectangle<f32>& body)
{
    Path p;
    p.addBubble (body.reduced (0.5f), body.getUnion (Rectangle<f32> (tip.x, tip.y, 1.0f, 1.0f)),
                 tip, 5.0f, jmin (15.0f, body.getWidth() * 0.2f, body.getHeight() * 0.2f));

    g.setColor (comp.findColor (BubbleComponent::backgroundColorId));
    g.fillPath (p);

    g.setColor (comp.findColor (BubbleComponent::outlineColorId));
    g.strokePath (p, PathStrokeType (1.0f));
}

z0 LookAndFeel_V2::setComponentEffectForBubbleComponent (BubbleComponent& bubbleComponent)
{
    bubbleComponent.setComponentEffect (&bubbleShadow);
}

//==============================================================================
Font LookAndFeel_V2::getPopupMenuFont()
{
    return withDefaultMetrics (FontOptions (17.0f));
}

z0 LookAndFeel_V2::getIdealPopupMenuItemSize (const Txt& text, const b8 isSeparator,
                                                i32 standardMenuItemHeight, i32& idealWidth, i32& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight / 2 : 10;
    }
    else
    {
        Font font (getPopupMenuFont());

        if (standardMenuItemHeight > 0 && font.getHeight() > (f32) standardMenuItemHeight / 1.3f)
            font.setHeight ((f32) standardMenuItemHeight / 1.3f);

        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : roundToInt (font.getHeight() * 1.3f);
        idealWidth = GlyphArrangement::getStringWidthInt (font, text) + idealHeight * 2;
    }
}

z0 LookAndFeel_V2::getIdealPopupMenuItemSizeWithOptions (const Txt& text,
                                                           b8 isSeparator,
                                                           i32 standardMenuItemHeight,
                                                           i32& idealWidth,
                                                           i32& idealHeight,
                                                           const PopupMenu::Options&)
{
    getIdealPopupMenuItemSize (text,
                               isSeparator,
                               standardMenuItemHeight,
                               idealWidth,
                               idealHeight);
}

z0 LookAndFeel_V2::getIdealPopupMenuSectionHeaderSizeWithOptions (const Txt& text,
                                                                    i32 standardMenuItemHeight,
                                                                    i32& idealWidth,
                                                                    i32& idealHeight,
                                                                    const PopupMenu::Options& options)
{
    getIdealPopupMenuItemSizeWithOptions (text,
                                          false,
                                          standardMenuItemHeight,
                                          idealWidth,
                                          idealHeight,
                                          options);
    idealHeight += idealHeight / 2;
    idealWidth += idealWidth / 4;
}

z0 LookAndFeel_V2::drawPopupMenuBackground (Graphics& g, i32 width, i32 height)
{
    auto background = findColor (PopupMenu::backgroundColorId);

    g.fillAll (background);
    g.setColor (background.overlaidWith (Color (0x2badd8e6)));

    for (i32 i = 0; i < height; i += 3)
        g.fillRect (0, i, width, 1);

   #if ! DRX_MAC
    g.setColor (findColor (PopupMenu::textColorId).withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
   #endif
}

z0 LookAndFeel_V2::drawPopupMenuBackgroundWithOptions (Graphics& g,
                                                         i32 width,
                                                         i32 height,
                                                         const PopupMenu::Options&)
{
    drawPopupMenuBackground (g, width, height);
}

z0 LookAndFeel_V2::drawPopupMenuUpDownArrow (Graphics& g, i32 width, i32 height, b8 isScrollUpArrow)
{
    auto background = findColor (PopupMenu::backgroundColorId);

    g.setGradientFill (ColorGradient (background, 0.0f, (f32) height * 0.5f,
                                       background.withAlpha (0.0f),
                                       0.0f, isScrollUpArrow ? ((f32) height) : 0.0f,
                                       false));

    g.fillRect (1, 1, width - 2, height - 2);

    auto hw = (f32) width * 0.5f;
    auto arrowW = (f32) height * 0.3f;
    auto y1 = (f32) height * (isScrollUpArrow ? 0.6f : 0.3f);
    auto y2 = (f32) height * (isScrollUpArrow ? 0.3f : 0.6f);

    Path p;
    p.addTriangle (hw - arrowW, y1,
                   hw + arrowW, y1,
                   hw, y2);

    g.setColor (findColor (PopupMenu::textColorId).withAlpha (0.5f));
    g.fillPath (p);
}

z0 LookAndFeel_V2::drawPopupMenuUpDownArrowWithOptions (Graphics& g,
                                                          i32 width, i32 height,
                                                          b8 isScrollUpArrow,
                                                          const PopupMenu::Options&)
{
    drawPopupMenuUpDownArrow (g, width, height, isScrollUpArrow);
}

z0 LookAndFeel_V2::drawPopupMenuItem (Graphics& g, const Rectangle<i32>& area,
                                        const b8 isSeparator, const b8 isActive,
                                        const b8 isHighlighted, const b8 isTicked,
                                        const b8 hasSubMenu, const Txt& text,
                                        const Txt& shortcutKeyText,
                                        const Drawable* icon, const Color* const textColorToUse)
{
    if (isSeparator)
    {
        auto r = area.reduced (5, 0);
        r.removeFromTop (r.getHeight() / 2 - 1);

        g.setColor (Color (0x33000000));
        g.fillRect (r.removeFromTop (1));

        g.setColor (Color (0x66ffffff));
        g.fillRect (r.removeFromTop (1));
    }
    else
    {
        auto textColor = findColor (PopupMenu::textColorId);

        if (textColorToUse != nullptr)
            textColor = *textColorToUse;

        auto r = area.reduced (1);

        if (isHighlighted)
        {
            g.setColor (findColor (PopupMenu::highlightedBackgroundColorId));
            g.fillRect (r);

            g.setColor (findColor (PopupMenu::highlightedTextColorId));
        }
        else
        {
            g.setColor (textColor);
        }

        if (! isActive)
            g.setOpacity (0.3f);

        Font font (getPopupMenuFont());

        auto maxFontHeight = (f32) area.getHeight() / 1.3f;

        if (font.getHeight() > maxFontHeight)
            font.setHeight (maxFontHeight);

        g.setFont (font);

        auto iconArea = r.removeFromLeft ((r.getHeight() * 5) / 4).reduced (3).toFloat();

        if (icon != nullptr)
        {
            icon->drawWithin (g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        }
        else if (isTicked)
        {
            auto tick = getTickShape (1.0f);
            g.fillPath (tick, tick.getTransformToScaleToFit (iconArea, true));
        }

        if (hasSubMenu)
        {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();

            auto x = (f32) r.removeFromRight ((i32) arrowH).getX();
            auto halfH = (f32) r.getCentreY();

            Path p;
            p.addTriangle (x, halfH - arrowH * 0.5f,
                           x, halfH + arrowH * 0.5f,
                           x + arrowH * 0.6f, halfH);

            g.fillPath (p);
        }

        r.removeFromRight (3);
        g.drawFittedText (text, r, Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            Font f2 (font);
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setFont (f2);

            g.drawText (shortcutKeyText, r, Justification::centredRight, true);
        }
    }
}

z0 LookAndFeel_V2::drawPopupMenuItemWithOptions (Graphics& g, const Rectangle<i32>& area,
                                                   b8 isHighlighted,
                                                   const PopupMenu::Item& item,
                                                   const PopupMenu::Options&)
{
    const auto colour = item.colour != Color() ? &item.colour : nullptr;
    const auto hasSubMenu = item.subMenu != nullptr
                            && (item.itemID == 0 || item.subMenu->getNumItems() > 0);

    drawPopupMenuItem (g,
                       area,
                       item.isSeparator,
                       item.isEnabled,
                       isHighlighted,
                       item.isTicked,
                       hasSubMenu,
                       item.text,
                       item.shortcutKeyDescription,
                       item.image.get(),
                       colour);
}

z0 LookAndFeel_V2::drawPopupMenuSectionHeader (Graphics& g,
                                                 const Rectangle<i32>& area,
                                                 const Txt& sectionName)
{
    g.setFont (getPopupMenuFont().boldened());
    g.setColor (findColor (PopupMenu::headerTextColorId));

    g.drawFittedText (sectionName,
                      area.getX() + 12, area.getY(), area.getWidth() - 16, (i32) ((f32) area.getHeight() * 0.8f),
                      Justification::bottomLeft, 1);
}

z0 LookAndFeel_V2::drawPopupMenuSectionHeaderWithOptions (Graphics& g, const Rectangle<i32>& area,
                                                            const Txt& sectionName,
                                                            const PopupMenu::Options&)
{
    drawPopupMenuSectionHeader (g, area, sectionName);
}

//==============================================================================
i32 LookAndFeel_V2::getMenuWindowFlags()
{
    return ComponentPeer::windowHasDropShadow;
}

z0 LookAndFeel_V2::drawMenuBarBackground (Graphics& g, i32 width, i32 height, b8, MenuBarComponent& menuBar)
{
    auto baseColor = detail::LookAndFeelHelpers::createBaseColor (menuBar.findColor (PopupMenu::backgroundColorId),
                                                                    false, false, false);

    if (menuBar.isEnabled())
        drawShinyButtonShape (g, -4.0f, 0.0f, (f32) width + 8.0f, (f32) height,
                              0.0f, baseColor, 0.4f, true, true, true, true);
    else
        g.fillAll (baseColor);
}

Font LookAndFeel_V2::getMenuBarFont (MenuBarComponent& menuBar, i32 /*itemIndex*/, const Txt& /*itemText*/)
{
    return withDefaultMetrics (FontOptions ((f32) menuBar.getHeight() * 0.7f));
}

i32 LookAndFeel_V2::getMenuBarItemWidth (MenuBarComponent& menuBar, i32 itemIndex, const Txt& itemText)
{
    return GlyphArrangement::getStringWidthInt (getMenuBarFont (menuBar, itemIndex, itemText), itemText) + menuBar.getHeight();
}

z0 LookAndFeel_V2::drawMenuBarItem (Graphics& g, i32 width, i32 height,
                                      i32 itemIndex, const Txt& itemText,
                                      b8 isMouseOverItem, b8 isMenuOpen,
                                      b8 /*isMouseOverBar*/, MenuBarComponent& menuBar)
{
    if (! menuBar.isEnabled())
    {
        g.setColor (menuBar.findColor (PopupMenu::textColorId)
                            .withMultipliedAlpha (0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll (menuBar.findColor (PopupMenu::highlightedBackgroundColorId));
        g.setColor (menuBar.findColor (PopupMenu::highlightedTextColorId));
    }
    else
    {
        g.setColor (menuBar.findColor (PopupMenu::textColorId));
    }

    g.setFont (getMenuBarFont (menuBar, itemIndex, itemText));
    g.drawFittedText (itemText, 0, 0, width, height, Justification::centred, 1);
}

Component* LookAndFeel_V2::getParentComponentForMenuOptions (const PopupMenu::Options& options)
{
    return options.getParentComponent();
}

z0 LookAndFeel_V2::preparePopupMenuWindow (Component&) {}

b8 LookAndFeel_V2::shouldPopupMenuScaleWithTargetComponent (const PopupMenu::Options&)    { return true; }

i32 LookAndFeel_V2::getPopupMenuBorderSize()    { return 2; }

i32 LookAndFeel_V2::getPopupMenuBorderSizeWithOptions (const PopupMenu::Options&)
{
    return getPopupMenuBorderSize();
}

z0 LookAndFeel_V2::drawPopupMenuColumnSeparatorWithOptions (Graphics&,
                                                              const Rectangle<i32>&,
                                                              const PopupMenu::Options&) {}

i32 LookAndFeel_V2::getPopupMenuColumnSeparatorWidthWithOptions (const PopupMenu::Options&)
{
    return 0;
}

//==============================================================================
z0 LookAndFeel_V2::fillTextEditorBackground (Graphics& g, i32 /*width*/, i32 /*height*/, TextEditor& textEditor)
{
    g.fillAll (textEditor.findColor (TextEditor::backgroundColorId));
}

z0 LookAndFeel_V2::drawTextEditorOutline (Graphics& g, i32 width, i32 height, TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
        {
            i32k border = 2;

            g.setColor (textEditor.findColor (TextEditor::focusedOutlineColorId));
            g.drawRect (0, 0, width, height, border);

            g.setOpacity (1.0f);
            auto shadowColor = textEditor.findColor (TextEditor::shadowColorId).withMultipliedAlpha (0.75f);
            drawBevel (g, 0, 0, width, height + 2, border + 2, shadowColor, shadowColor);
        }
        else
        {
            g.setColor (textEditor.findColor (TextEditor::outlineColorId));
            g.drawRect (0, 0, width, height);

            g.setOpacity (1.0f);
            auto shadowColor = textEditor.findColor (TextEditor::shadowColorId);
            drawBevel (g, 0, 0, width, height + 2, 3, shadowColor, shadowColor);
        }
    }
}

CaretComponent* LookAndFeel_V2::createCaretComponent (Component* keyFocusOwner)
{
    return new CaretComponent (keyFocusOwner);
}

//==============================================================================
z0 LookAndFeel_V2::drawComboBox (Graphics& g, i32 width, i32 height, const b8 isMouseButtonDown,
                                   i32 buttonX, i32 buttonY, i32 buttonW, i32 buttonH, ComboBox& box)
{
    g.fillAll (box.findColor (ComboBox::backgroundColorId));

    if (box.isEnabled() && box.hasKeyboardFocus (false))
    {
        g.setColor (box.findColor (ComboBox::focusedOutlineColorId));
        g.drawRect (0, 0, width, height, 2);
    }
    else
    {
        g.setColor (box.findColor (ComboBox::outlineColorId));
        g.drawRect (0, 0, width, height);
    }

    auto outlineThickness = box.isEnabled() ? (isMouseButtonDown ? 1.2f : 0.5f) : 0.3f;

    auto baseColor = detail::LookAndFeelHelpers::createBaseColor (box.findColor (ComboBox::buttonColorId),
                                                                    box.hasKeyboardFocus (true),
                                                                    false, isMouseButtonDown)
                         .withMultipliedAlpha (box.isEnabled() ? 1.0f : 0.5f);

    drawGlassLozenge (g,
                      (f32) buttonX + outlineThickness, (f32) buttonY + outlineThickness,
                      (f32) buttonW - outlineThickness * 2.0f, (f32) buttonH - outlineThickness * 2.0f,
                      baseColor, outlineThickness, -1.0f,
                      true, true, true, true);

    if (box.isEnabled())
    {
        const f32 arrowX = 0.3f;
        const f32 arrowH = 0.2f;

        const auto x = (f32) buttonX;
        const auto y = (f32) buttonY;
        const auto w = (f32) buttonW;
        const auto h = (f32) buttonH;

        Path p;
        p.addTriangle (x + w * 0.5f,            y + h * (0.45f - arrowH),
                       x + w * (1.0f - arrowX), y + h * 0.45f,
                       x + w * arrowX,          y + h * 0.45f);

        p.addTriangle (x + w * 0.5f,            y + h * (0.55f + arrowH),
                       x + w * (1.0f - arrowX), y + h * 0.55f,
                       x + w * arrowX,          y + h * 0.55f);

        g.setColor (box.findColor (ComboBox::arrowColorId));
        g.fillPath (p);
    }
}

Font LookAndFeel_V2::getComboBoxFont (ComboBox& box)
{
    return withDefaultMetrics (FontOptions (jmin (15.0f, (f32) box.getHeight() * 0.85f)));
}

Label* LookAndFeel_V2::createComboBoxTextBox (ComboBox&)
{
    return new Label (Txt(), Txt());
}

z0 LookAndFeel_V2::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (1, 1,
                     box.getWidth() + 3 - box.getHeight(),
                     box.getHeight() - 2);

    label.setFont (getComboBoxFont (box));
}

PopupMenu::Options LookAndFeel_V2::getOptionsForComboBoxPopupMenu (ComboBox& box, Label& label)
{
    return PopupMenu::Options().withTargetComponent (&box)
                               .withItemThatMustBeVisible (box.getSelectedId())
                               .withInitiallySelectedItem (box.getSelectedId())
                               .withMinimumWidth (box.getWidth())
                               .withMaximumNumColumns (1)
                               .withStandardItemHeight (label.getHeight());
}

z0 LookAndFeel_V2::drawComboBoxTextWhenNothingSelected (Graphics& g, ComboBox& box, Label& label)
{
    g.setColor (findColor (ComboBox::textColorId).withMultipliedAlpha (0.5f));

    auto font = label.getLookAndFeel().getLabelFont (label);

    g.setFont (font);

    auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

    g.drawFittedText (box.getTextWhenNothingSelected(), textArea, label.getJustificationType(),
                      jmax (1, (i32) ((f32) textArea.getHeight() / font.getHeight())),
                      label.getMinimumHorizontalScale());
}

//==============================================================================
Font LookAndFeel_V2::getLabelFont (Label& label)
{
    return label.getFont();
}

z0 LookAndFeel_V2::drawLabel (Graphics& g, Label& label)
{
    g.fillAll (label.findColor (Label::backgroundColorId));

    if (! label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));

        g.setColor (label.findColor (Label::textColorId).withMultipliedAlpha (alpha));
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (i32) ((f32) textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());

        g.setColor (label.findColor (Label::outlineColorId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColor (label.findColor (Label::outlineColorId));
    }

    g.drawRect (label.getLocalBounds());
}

BorderSize<i32> LookAndFeel_V2::getLabelBorderSize (Label& label)
{
    return label.getBorderSize();
}

//==============================================================================
z0 LookAndFeel_V2::drawLinearSliderBackground (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                                 f32 /*sliderPos*/,
                                                 f32 /*minSliderPos*/,
                                                 f32 /*maxSliderPos*/,
                                                 const Slider::SliderStyle /*style*/, Slider& slider)
{
    auto sliderRadius = (f32) (getSliderThumbRadius (slider) - 2);
    auto trackColor = slider.findColor (Slider::trackColorId);
    auto gradCol1 = trackColor.overlaidWith (Colors::black.withAlpha (slider.isEnabled() ? 0.25f : 0.13f));
    auto gradCol2 = trackColor.overlaidWith (Color (0x14000000));

    Path indent;

    if (slider.isHorizontal())
    {
        const f32 iy = (f32) y + (f32) height * 0.5f - sliderRadius * 0.5f;
        const f32 ih = sliderRadius;

        g.setGradientFill (ColorGradient::vertical (gradCol1, iy, gradCol2, iy + ih));

        indent.addRoundedRectangle ((f32) x - sliderRadius * 0.5f, iy,
                                    (f32) width + sliderRadius, ih,
                                    5.0f);
    }
    else
    {
        const f32 ix = (f32) x + (f32) width * 0.5f - sliderRadius * 0.5f;
        const f32 iw = sliderRadius;

        g.setGradientFill (ColorGradient::horizontal (gradCol1, ix, gradCol2, ix + iw));

        indent.addRoundedRectangle (ix, (f32) y - sliderRadius * 0.5f,
                                    iw, (f32) height + sliderRadius,
                                    5.0f);
    }

    g.fillPath (indent);

    g.setColor (Color (0x4c000000));
    g.strokePath (indent, PathStrokeType (0.5f));
}

z0 LookAndFeel_V2::drawLinearSliderOutline (Graphics& g, i32, i32, i32, i32,
                                              const Slider::SliderStyle, Slider& slider)
{
    if (slider.getTextBoxPosition() == Slider::NoTextBox)
    {
        g.setColor (slider.findColor (Slider::textBoxOutlineColorId));
        g.drawRect (0, 0, slider.getWidth(), slider.getHeight(), 1);
    }
}

z0 LookAndFeel_V2::drawLinearSliderThumb (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                            f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                            const Slider::SliderStyle style, Slider& slider)
{
    auto sliderRadius = (f32) (getSliderThumbRadius (slider) - 2);

    auto knobColor = detail::LookAndFeelHelpers::createBaseColor (slider.findColor (Slider::thumbColorId),
                                                                    slider.hasKeyboardFocus (false) && slider.isEnabled(),
                                                                    slider.isMouseOverOrDragging() && slider.isEnabled(),
                                                                    slider.isMouseButtonDown() && slider.isEnabled());

    const f32 outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

    if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
    {
        f32 kx, ky;

        if (style == Slider::LinearVertical)
        {
            kx = (f32) x + (f32) width * 0.5f;
            ky = sliderPos;
        }
        else
        {
            kx = sliderPos;
            ky = (f32) y + (f32) height * 0.5f;
        }

        drawGlassSphere (g,
                         kx - sliderRadius,
                         ky - sliderRadius,
                         sliderRadius * 2.0f,
                         knobColor, outlineThickness);
    }
    else
    {
        if (style == Slider::ThreeValueVertical)
        {
            drawGlassSphere (g, (f32) x + (f32) width * 0.5f - sliderRadius,
                             sliderPos - sliderRadius,
                             sliderRadius * 2.0f,
                             knobColor, outlineThickness);
        }
        else if (style == Slider::ThreeValueHorizontal)
        {
            drawGlassSphere (g,sliderPos - sliderRadius,
                             (f32) y + (f32) height * 0.5f - sliderRadius,
                             sliderRadius * 2.0f,
                             knobColor, outlineThickness);
        }

        if (style == Slider::TwoValueVertical || style == Slider::ThreeValueVertical)
        {
            auto sr = jmin (sliderRadius, (f32) width * 0.4f);

            drawGlassPointer (g, jmax (0.0f, (f32) x + (f32) width * 0.5f - sliderRadius * 2.0f),
                              minSliderPos - sliderRadius,
                              sliderRadius * 2.0f, knobColor, outlineThickness, 1);

            drawGlassPointer (g,
                              jmin ((f32) x + (f32) width - sliderRadius * 2.0f,
                                    (f32) x + (f32) width * 0.5f),
                              maxSliderPos - sr,
                              sliderRadius * 2.0f,
                              knobColor,
                              outlineThickness,
                              3);
        }
        else if (style == Slider::TwoValueHorizontal || style == Slider::ThreeValueHorizontal)
        {
            auto sr = jmin (sliderRadius, (f32) height * 0.4f);

            drawGlassPointer (g, minSliderPos - sr,
                              jmax (0.0f, (f32) y + (f32) height * 0.5f - sliderRadius * 2.0f),
                              sliderRadius * 2.0f, knobColor, outlineThickness, 2);

            drawGlassPointer (g,
                              maxSliderPos - sliderRadius,
                              jmin ((f32) y + (f32) height - sliderRadius * 2.0f,
                                    (f32) y + (f32) height * 0.5f),
                              sliderRadius * 2.0f,
                              knobColor,
                              outlineThickness,
                              4);
        }
    }
}

z0 LookAndFeel_V2::drawLinearSlider (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                       f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                       const Slider::SliderStyle style, Slider& slider)
{
    g.fillAll (slider.findColor (Slider::backgroundColorId));

    if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
    {
        const b8 isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        auto baseColor = detail::LookAndFeelHelpers::createBaseColor (slider.findColor (Slider::thumbColorId)
                                                                              .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f),
                                                                        false, isMouseOver,
                                                                        isMouseOver || slider.isMouseButtonDown());

        drawShinyButtonShape (g,
                              (f32) x,
                              style == Slider::LinearBarVertical ? sliderPos
                                                                 : (f32) y,
                              style == Slider::LinearBarVertical ? (f32) width
                                                                 : (sliderPos - (f32) x),
                              style == Slider::LinearBarVertical ? ((f32) height - sliderPos)
                                                                 : (f32) height, 0.0f,
                              baseColor,
                              slider.isEnabled() ? 0.9f : 0.3f,
                              true, true, true, true);

        drawLinearSliderOutline (g, x, y, width, height, style, slider);
    }
    else
    {
        drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

i32 LookAndFeel_V2::getSliderThumbRadius (Slider& slider)
{
    return jmin (7,
                 slider.getHeight() / 2,
                 slider.getWidth() / 2) + 2;
}

z0 LookAndFeel_V2::drawRotarySlider (Graphics& g, i32 x, i32 y, i32 width, i32 height, f32 sliderPos,
                                       const f32 rotaryStartAngle, const f32 rotaryEndAngle, Slider& slider)
{
    const f32 radius = jmin ((f32) width * 0.5f, (f32) height * 0.5f) - 2.0f;
    const f32 centreX = (f32) x + (f32) width * 0.5f;
    const f32 centreY = (f32) y + (f32) height * 0.5f;
    const f32 rx = centreX - radius;
    const f32 ry = centreY - radius;
    const f32 rw = radius * 2.0f;
    const f32 angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const b8 isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

    if (radius > 12.0f)
    {
        if (slider.isEnabled())
            g.setColor (slider.findColor (Slider::rotarySliderFillColorId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColor (Color (0x80808080));

        const f32 thickness = 0.7f;

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, thickness);
            g.fillPath (filledArc);
        }

        {
            const f32 innerRadius = radius * 0.2f;
            Path p;
            p.addTriangle (-innerRadius, 0.0f,
                           0.0f, -radius * thickness * 1.1f,
                           innerRadius, 0.0f);

            p.addEllipse (-innerRadius, -innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

            g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
        }

        if (slider.isEnabled())
            g.setColor (slider.findColor (Slider::rotarySliderOutlineColorId));
        else
            g.setColor (Color (0x80808080));

        Path outlineArc;
        outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
        outlineArc.closeSubPath();

        g.strokePath (outlineArc, PathStrokeType (slider.isEnabled() ? (isMouseOver ? 2.0f : 1.2f) : 0.3f));
    }
    else
    {
        if (slider.isEnabled())
            g.setColor (slider.findColor (Slider::rotarySliderFillColorId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColor (Color (0x80808080));

        Path p;
        p.addEllipse (-0.4f * rw, -0.4f * rw, rw * 0.8f, rw * 0.8f);
        PathStrokeType (rw * 0.1f).createStrokedPath (p, p);

        p.addLineSegment (Line<f32> (0.0f, 0.0f, 0.0f, -radius), rw * 0.2f);

        g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
    }
}

Button* LookAndFeel_V2::createSliderButton (Slider&, const b8 isIncrement)
{
    return new TextButton (isIncrement ? "+" : "-", Txt());
}

class LookAndFeel_V2::SliderLabelComp final : public Label
{
public:
    SliderLabelComp() : Label ({}, {}) {}

    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override {}

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }
};

Label* LookAndFeel_V2::createSliderTextBox (Slider& slider)
{
    auto l = new SliderLabelComp();

    l->setJustificationType (Justification::centred);
    l->setKeyboardType (TextInputTarget::decimalKeyboard);

    l->setColor (Label::textColorId, slider.findColor (Slider::textBoxTextColorId));
    l->setColor (Label::backgroundColorId,
                  (slider.getSliderStyle() == Slider::LinearBar || slider.getSliderStyle() == Slider::LinearBarVertical)
                            ? Colors::transparentBlack
                            : slider.findColor (Slider::textBoxBackgroundColorId));
    l->setColor (Label::outlineColorId, slider.findColor (Slider::textBoxOutlineColorId));
    l->setColor (TextEditor::textColorId, slider.findColor (Slider::textBoxTextColorId));
    l->setColor (TextEditor::backgroundColorId,
                  slider.findColor (Slider::textBoxBackgroundColorId)
                        .withAlpha ((slider.getSliderStyle() == Slider::LinearBar || slider.getSliderStyle() == Slider::LinearBarVertical)
                                        ? 0.7f : 1.0f));
    l->setColor (TextEditor::outlineColorId, slider.findColor (Slider::textBoxOutlineColorId));
    l->setColor (TextEditor::highlightColorId, slider.findColor (Slider::textBoxHighlightColorId));

    return l;
}

ImageEffectFilter* LookAndFeel_V2::getSliderEffect (Slider&)
{
    return nullptr;
}

Font LookAndFeel_V2::getSliderPopupFont (Slider&)
{
    return withDefaultMetrics (FontOptions (15.0f, Font::bold));
}

i32 LookAndFeel_V2::getSliderPopupPlacement (Slider&)
{
    return BubbleComponent::above
            | BubbleComponent::below
            | BubbleComponent::left
            | BubbleComponent::right;
}

//==============================================================================
Slider::SliderLayout LookAndFeel_V2::getSliderLayout (Slider& slider)
{
    // 1. compute the actually visible textBox size from the slider textBox size and some additional constraints

    i32 minXSpace = 0;
    i32 minYSpace = 0;

    auto textBoxPos = slider.getTextBoxPosition();

    if (textBoxPos == Slider::TextBoxLeft || textBoxPos == Slider::TextBoxRight)
        minXSpace = 30;
    else
        minYSpace = 15;

    auto localBounds = slider.getLocalBounds();

    auto textBoxWidth  = jmax (0, jmin (slider.getTextBoxWidth(),  localBounds.getWidth() - minXSpace));
    auto textBoxHeight = jmax (0, jmin (slider.getTextBoxHeight(), localBounds.getHeight() - minYSpace));

    Slider::SliderLayout layout;

    // 2. set the textBox bounds

    if (textBoxPos != Slider::NoTextBox)
    {
        if (slider.isBar())
        {
            layout.textBoxBounds = localBounds;
        }
        else
        {
            layout.textBoxBounds.setWidth (textBoxWidth);
            layout.textBoxBounds.setHeight (textBoxHeight);

            if (textBoxPos == Slider::TextBoxLeft)           layout.textBoxBounds.setX (0);
            else if (textBoxPos == Slider::TextBoxRight)     layout.textBoxBounds.setX (localBounds.getWidth() - textBoxWidth);
            else /* above or below -> centre horizontally */ layout.textBoxBounds.setX ((localBounds.getWidth() - textBoxWidth) / 2);

            if (textBoxPos == Slider::TextBoxAbove)          layout.textBoxBounds.setY (0);
            else if (textBoxPos == Slider::TextBoxBelow)     layout.textBoxBounds.setY (localBounds.getHeight() - textBoxHeight);
            else /* left or right -> centre vertically */    layout.textBoxBounds.setY ((localBounds.getHeight() - textBoxHeight) / 2);
        }
    }

    // 3. set the slider bounds

    layout.sliderBounds = localBounds;

    if (slider.isBar())
    {
        layout.sliderBounds.reduce (1, 1);   // bar border
    }
    else
    {
        if (textBoxPos == Slider::TextBoxLeft)       layout.sliderBounds.removeFromLeft (textBoxWidth);
        else if (textBoxPos == Slider::TextBoxRight) layout.sliderBounds.removeFromRight (textBoxWidth);
        else if (textBoxPos == Slider::TextBoxAbove) layout.sliderBounds.removeFromTop (textBoxHeight);
        else if (textBoxPos == Slider::TextBoxBelow) layout.sliderBounds.removeFromBottom (textBoxHeight);

        i32k thumbIndent = getSliderThumbRadius (slider);

        if (slider.isHorizontal())    layout.sliderBounds.reduce (thumbIndent, 0);
        else if (slider.isVertical()) layout.sliderBounds.reduce (0, thumbIndent);
    }

    return layout;
}

//==============================================================================
Rectangle<i32> LookAndFeel_V2::getTooltipBounds (const Txt& tipText, Point<i32> screenPos, Rectangle<i32> parentArea)
{
    const TextLayout tl (detail::LookAndFeelHelpers::layoutTooltipText (getDefaultMetricsKind(), tipText, Colors::black));

    auto w = (i32) (tl.getWidth() + 14.0f);
    auto h = (i32) (tl.getHeight() + 6.0f);

    return Rectangle<i32> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                           screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
                           w, h)
             .constrainedWithin (parentArea);
}

z0 LookAndFeel_V2::drawTooltip (Graphics& g, const Txt& text, i32 width, i32 height)
{
    g.fillAll (findColor (TooltipWindow::backgroundColorId));

   #if ! DRX_MAC // The mac windows already have a non-optional 1 pix outline, so don't f64 it here..
    g.setColor (findColor (TooltipWindow::outlineColorId));
    g.drawRect (0, 0, width, height, 1);
   #endif

    detail::LookAndFeelHelpers::layoutTooltipText (getDefaultMetricsKind(), text, findColor (TooltipWindow::textColorId))
        .draw (g, Rectangle<f32> ((f32) width, (f32) height));
}

//==============================================================================
Button* LookAndFeel_V2::createFilenameComponentBrowseButton (const Txt& text)
{
    return new TextButton (text, TRANS ("click to browse for a different file"));
}

z0 LookAndFeel_V2::layoutFilenameComponent (FilenameComponent& filenameComp,
                                              ComboBox* filenameBox, Button* browseButton)
{
    if (browseButton == nullptr || filenameBox == nullptr)
        return;

    browseButton->setSize (80, filenameComp.getHeight());

    if (auto* tb = dynamic_cast<TextButton*> (browseButton))
        tb->changeWidthToFitText();

    browseButton->setTopRightPosition (filenameComp.getWidth(), 0);

    filenameBox->setBounds (0, 0, browseButton->getX(), filenameComp.getHeight());
}

//==============================================================================
z0 LookAndFeel_V2::drawConcertinaPanelHeader (Graphics& g, const Rectangle<i32>& area,
                                                b8 isMouseOver, b8 /*isMouseDown*/,
                                                ConcertinaPanel&, Component& panel)
{
    g.fillAll (Colors::grey.withAlpha (isMouseOver ? 0.9f : 0.7f));
    g.setColor (Colors::black.withAlpha (0.5f));
    g.drawRect (area);

    g.setColor (Colors::white);
    g.setFont (Font (withDefaultMetrics (FontOptions { (f32) area.getHeight() * 0.7f })).boldened());
    g.drawFittedText (panel.getName(), 4, 0, area.getWidth() - 6, area.getHeight(), Justification::centredLeft, 1);
}

//==============================================================================
z0 LookAndFeel_V2::drawImageButton (Graphics& g, Image* image,
                                      i32 imageX, i32 imageY, i32 imageW, i32 imageH,
                                      const Color& overlayColor,
                                      f32 imageOpacity,
                                      ImageButton& button)
{
    if (! button.isEnabled())
        imageOpacity *= 0.3f;

    AffineTransform t = RectanglePlacement (RectanglePlacement::stretchToFit)
                            .getTransformToFit (image->getBounds().toFloat(),
                                                Rectangle<i32> (imageX, imageY, imageW, imageH).toFloat());

    if (! overlayColor.isOpaque())
    {
        g.setOpacity (imageOpacity);
        g.drawImageTransformed (*image, t, false);
    }

    if (! overlayColor.isTransparent())
    {
        g.setColor (overlayColor);
        g.drawImageTransformed (*image, t, true);
    }
}

//==============================================================================
z0 LookAndFeel_V2::drawCornerResizer (Graphics& g, i32 w, i32 h, b8 /*isMouseOver*/, b8 /*isMouseDragging*/)
{
    auto lineThickness = jmin ((f32) w, (f32) h) * 0.075f;

    for (f32 i = 0.0f; i < 1.0f; i += 0.3f)
    {
        g.setColor (Colors::lightgrey);

        g.drawLine ((f32) w * i,
                    (f32) h + 1.0f,
                    (f32) w + 1.0f,
                    (f32) h * i,
                    lineThickness);

        g.setColor (Colors::darkgrey);

        g.drawLine ((f32) w * i + lineThickness,
                    (f32) h + 1.0f,
                    (f32) w + 1.0f,
                    (f32) h * i + lineThickness,
                    lineThickness);
    }
}

z0 LookAndFeel_V2::drawResizableFrame (Graphics& g, i32 w, i32 h, const BorderSize<i32>& border)
{
    if (! border.isEmpty())
    {
        const Rectangle<i32> fullSize (0, 0, w, h);
        auto centreArea = border.subtractedFrom (fullSize);

        Graphics::ScopedSaveState ss (g);

        g.excludeClipRegion (centreArea);

        g.setColor (Color (0x50000000));
        g.drawRect (fullSize);

        g.setColor (Color (0x19000000));
        g.drawRect (centreArea.expanded (1, 1));
    }
}

//==============================================================================
z0 LookAndFeel_V2::fillResizableWindowBackground (Graphics& g, i32 /*w*/, i32 /*h*/,
                                                    const BorderSize<i32>& /*border*/, ResizableWindow& window)
{
    g.fillAll (window.getBackgroundColor());
}

z0 LookAndFeel_V2::drawResizableWindowBorder (Graphics&, i32 /*w*/, i32 /*h*/,
                                                const BorderSize<i32>& /*border*/, ResizableWindow&)
{
}

z0 LookAndFeel_V2::drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g,
                                                 i32 w, i32 h, i32 titleSpaceX, i32 titleSpaceW,
                                                 const Image* icon, b8 drawTitleTextOnLeft)
{
    if (w * h == 0)
        return;

    const b8 isActive = window.isActiveWindow();

    g.setGradientFill (ColorGradient::vertical (window.getBackgroundColor(), 0,
                                                 window.getBackgroundColor().contrasting (isActive ? 0.15f : 0.05f), (f32) h));
    g.fillAll();

    Font font (withDefaultMetrics (FontOptions { (f32) h * 0.65f, Font::bold }));
    g.setFont (font);

    i32 textW = GlyphArrangement::getStringWidthInt (font, window.getName());
    i32 iconW = 0;
    i32 iconH = 0;

    if (icon != nullptr)
    {
        iconH = (i32) font.getHeight();
        iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin (titleSpaceW, textW + iconW);
    i32 textX = drawTitleTextOnLeft ? titleSpaceX
                                    : jmax (titleSpaceX, (w - textW) / 2);

    if (textX + textW > titleSpaceX + titleSpaceW)
        textX = titleSpaceX + titleSpaceW - textW;

    if (icon != nullptr)
    {
        g.setOpacity (isActive ? 1.0f : 0.6f);
        g.drawImageWithin (*icon, textX, (h - iconH) / 2, iconW, iconH,
                           RectanglePlacement::centred, false);
        textX += iconW;
        textW -= iconW;
    }

    if (window.isColorSpecified (DocumentWindow::textColorId) || isColorSpecified (DocumentWindow::textColorId))
        g.setColor (window.findColor (DocumentWindow::textColorId));
    else
        g.setColor (window.getBackgroundColor().contrasting (isActive ? 0.7f : 0.4f));

    g.drawText (window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}

//==============================================================================
class LookAndFeel_V2::GlassWindowButton final : public Button
{
public:
    GlassWindowButton (const Txt& name, Color col,
                       const Path& normalShape_,
                       const Path& toggledShape_) noexcept
        : Button (name),
          colour (col),
          normalShape (normalShape_),
          toggledShape (toggledShape_)
    {
    }

    //==============================================================================
    z0 paintButton (Graphics& g, b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override
    {
        f32 alpha = shouldDrawButtonAsHighlighted ? (shouldDrawButtonAsDown ? 1.0f : 0.8f) : 0.55f;

        if (! isEnabled())
            alpha *= 0.5f;

        f32 x = 0, y = 0, diam;

        if (getWidth() < getHeight())
        {
            diam = (f32) getWidth();
            y = (f32) (getHeight() - getWidth()) * 0.5f;
        }
        else
        {
            diam = (f32) getHeight();
            y = (f32) (getWidth() - getHeight()) * 0.5f;
        }

        x += diam * 0.05f;
        y += diam * 0.05f;
        diam *= 0.9f;

        g.setGradientFill (ColorGradient (Color::greyLevel (0.9f).withAlpha (alpha), 0, y + diam,
                                           Color::greyLevel (0.6f).withAlpha (alpha), 0, y, false));
        g.fillEllipse (x, y, diam, diam);

        x += 2.0f;
        y += 2.0f;
        diam -= 4.0f;

        LookAndFeel_V2::drawGlassSphere (g, x, y, diam, colour.withAlpha (alpha), 1.0f);

        Path& p = getToggleState() ? toggledShape : normalShape;

        const AffineTransform t (p.getTransformToScaleToFit (x + diam * 0.3f, y + diam * 0.3f,
                                                             diam * 0.4f, diam * 0.4f, true));

        g.setColor (Colors::black.withAlpha (alpha * 0.6f));
        g.fillPath (p, t);
    }

private:
    Color colour;
    Path normalShape, toggledShape;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlassWindowButton)
};

Button* LookAndFeel_V2::createDocumentWindowButton (i32 buttonType)
{
    Path shape;
    const f32 crossThickness = 0.25f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment (Line<f32> (0.0f, 0.0f, 1.0f, 1.0f), crossThickness * 1.4f);
        shape.addLineSegment (Line<f32> (1.0f, 0.0f, 0.0f, 1.0f), crossThickness * 1.4f);

        return new GlassWindowButton ("close", Color (0xffdd1100), shape, shape);
    }

    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment (Line<f32> (0.0f, 0.5f, 1.0f, 0.5f), crossThickness);

        return new GlassWindowButton ("minimise", Color (0xffaa8811), shape, shape);
    }

    if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment (Line<f32> (0.5f, 0.0f, 0.5f, 1.0f), crossThickness);
        shape.addLineSegment (Line<f32> (0.0f, 0.5f, 1.0f, 0.5f), crossThickness);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new GlassWindowButton ("maximise", Color (0xff119911), shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}

z0 LookAndFeel_V2::positionDocumentWindowButtons (DocumentWindow&,
                                                    i32 titleBarX, i32 titleBarY,
                                                    i32 titleBarW, i32 titleBarH,
                                                    Button* minimiseButton,
                                                    Button* maximiseButton,
                                                    Button* closeButton,
                                                    b8 positionTitleBarButtonsOnLeft)
{
    i32k buttonW = titleBarH - titleBarH / 8;

    i32 x = positionTitleBarButtonsOnLeft ? titleBarX + 4
                                          : titleBarX + titleBarW - buttonW - buttonW / 4;

    if (closeButton != nullptr)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -(buttonW + buttonW / 4);
    }

    if (positionTitleBarButtonsOnLeft)
        std::swap (minimiseButton, maximiseButton);

    if (maximiseButton != nullptr)
    {
        maximiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (minimiseButton != nullptr)
        minimiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
}

i32 LookAndFeel_V2::getDefaultMenuBarHeight()
{
    return 24;
}

//==============================================================================
std::unique_ptr<DropShadower> LookAndFeel_V2::createDropShadowerForComponent (Component&)
{
    return std::make_unique<DropShadower> (DropShadow (Colors::black.withAlpha (0.4f), 10, Point<i32> (0, 2)));
}

std::unique_ptr<FocusOutline> LookAndFeel_V2::createFocusOutlineForComponent (Component&)
{
    struct WindowProperties final : public FocusOutline::OutlineWindowProperties
    {
        Rectangle<i32> getOutlineBounds (Component& c) override
        {
            return c.getScreenBounds();
        }

        z0 drawOutline (Graphics& g, i32 width, i32 height) override
        {
            g.setColor (Colors::yellow.withAlpha (0.6f));
            g.drawRoundedRectangle ({ (f32) width, (f32) height }, 3.0f, 3.0f);
        }
    };

    return std::make_unique<FocusOutline> (std::make_unique<WindowProperties>());
}

//==============================================================================
z0 LookAndFeel_V2::drawStretchableLayoutResizerBar (Graphics& g, i32 w, i32 h,
                                                      b8 /*isVerticalBar*/,
                                                      b8 isMouseOver,
                                                      b8 isMouseDragging)
{
    auto alpha = 0.5f;

    if (isMouseOver || isMouseDragging)
    {
        g.fillAll (Color (0x190000ff));
        alpha = 1.0f;
    }

    auto cx = (f32) w * 0.5f;
    auto cy = (f32) h * 0.5f;
    auto cr = (f32) jmin (w, h) * 0.4f;

    g.setGradientFill (ColorGradient (Colors::white.withAlpha (alpha), cx + cr * 0.1f, cy + cr,
                                       Colors::black.withAlpha (alpha), cx, cy - cr * 4.0f,
                                       true));

    g.fillEllipse (cx - cr, cy - cr, cr * 2.0f, cr * 2.0f);
}

//==============================================================================
z0 LookAndFeel_V2::drawGroupComponentOutline (Graphics& g, i32 width, i32 height,
                                                const Txt& text, const Justification& position,
                                                GroupComponent& group)
{
    const f32 textH = 15.0f;
    const f32 indent = 3.0f;
    const f32 textEdgeGap = 4.0f;
    auto cs = 5.0f;

    Font f (withDefaultMetrics (FontOptions { textH }));

    Path p;
    auto x = indent;
    auto y = f.getAscent() - 3.0f;
    auto w = jmax (0.0f, (f32) width - x * 2.0f);
    auto h = jmax (0.0f, (f32) height - y  - indent);
    cs = jmin (cs, w * 0.5f, h * 0.5f);
    auto cs2 = 2.0f * cs;

    auto textW = text.isEmpty() ? 0
                                : jlimit (0.0f,
                                          jmax (0.0f, w - cs2 - textEdgeGap * 2),
                                          (f32) GlyphArrangement::getStringWidthInt (f, text) + textEdgeGap * 2.0f);
    auto textX = cs + textEdgeGap;

    if (position.testFlags (Justification::horizontallyCentred))
        textX = cs + (w - cs2 - textW) * 0.5f;
    else if (position.testFlags (Justification::right))
        textX = w - cs - textW - textEdgeGap;

    p.startNewSubPath (x + textX + textW, y);
    p.lineTo (x + w - cs, y);

    p.addArc (x + w - cs2, y, cs2, cs2, 0, MathConstants<f32>::halfPi);
    p.lineTo (x + w, y + h - cs);

    p.addArc (x + w - cs2, y + h - cs2, cs2, cs2, MathConstants<f32>::halfPi, MathConstants<f32>::pi);
    p.lineTo (x + cs, y + h);

    p.addArc (x, y + h - cs2, cs2, cs2, MathConstants<f32>::pi, MathConstants<f32>::pi * 1.5f);
    p.lineTo (x, y + cs);

    p.addArc (x, y, cs2, cs2, MathConstants<f32>::pi * 1.5f, MathConstants<f32>::twoPi);
    p.lineTo (x + textX, y);

    auto alpha = group.isEnabled() ? 1.0f : 0.5f;

    g.setColor (group.findColor (GroupComponent::outlineColorId)
                    .withMultipliedAlpha (alpha));

    g.strokePath (p, PathStrokeType (2.0f));

    g.setColor (group.findColor (GroupComponent::textColorId)
                    .withMultipliedAlpha (alpha));
    g.setFont (f);
    g.drawText (text,
                roundToInt (x + textX), 0,
                roundToInt (textW),
                roundToInt (textH),
                Justification::centred, true);
}

//==============================================================================
i32 LookAndFeel_V2::getTabButtonOverlap (i32 tabDepth)
{
    return 1 + tabDepth / 3;
}

i32 LookAndFeel_V2::getTabButtonSpaceAroundImage()
{
    return 4;
}

i32 LookAndFeel_V2::getTabButtonBestWidth (TabBarButton& button, i32 tabDepth)
{
    i32 width = GlyphArrangement::getStringWidthInt (withDefaultMetrics (FontOptions { (f32) tabDepth * 0.6f }),
                                                     button.getButtonText().trim())
              + getTabButtonOverlap (tabDepth) * 2;

    if (auto* extraComponent = button.getExtraComponent())
        width += button.getTabbedButtonBar().isVertical() ? extraComponent->getHeight()
                                                          : extraComponent->getWidth();

    return jlimit (tabDepth * 2, tabDepth * 8, width);
}

Rectangle<i32> LookAndFeel_V2::getTabButtonExtraComponentBounds (const TabBarButton& button, Rectangle<i32>& textArea, Component& comp)
{
    Rectangle<i32> extraComp;

    auto orientation = button.getTabbedButtonBar().getOrientation();

    if (button.getExtraComponentPlacement() == TabBarButton::beforeText)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:     extraComp = textArea.removeFromLeft   (comp.getWidth()); break;
            case TabbedButtonBar::TabsAtLeft:    extraComp = textArea.removeFromBottom (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtRight:   extraComp = textArea.removeFromTop    (comp.getHeight()); break;
            default:                             jassertfalse; break;
        }
    }
    else
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:     extraComp = textArea.removeFromRight  (comp.getWidth()); break;
            case TabbedButtonBar::TabsAtLeft:    extraComp = textArea.removeFromTop    (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtRight:   extraComp = textArea.removeFromBottom (comp.getHeight()); break;
            default:                             jassertfalse; break;
        }
    }

    return extraComp;
}

z0 LookAndFeel_V2::createTabButtonShape (TabBarButton& button, Path& p, b8 /*isMouseOver*/, b8 /*isMouseDown*/)
{
    auto activeArea = button.getActiveArea();
    auto w = (f32) activeArea.getWidth();
    auto h = (f32) activeArea.getHeight();

    auto length = w;
    auto depth = h;

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    const f32 indent = (f32) getTabButtonOverlap ((i32) depth);
    const f32 overhang = 4.0f;

    switch (button.getTabbedButtonBar().getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            p.startNewSubPath (w, 0.0f);
            p.lineTo (0.0f, indent);
            p.lineTo (0.0f, h - indent);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (w + overhang, -overhang);
            break;

        case TabbedButtonBar::TabsAtRight:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (w, indent);
            p.lineTo (w, h - indent);
            p.lineTo (0.0f, h);
            p.lineTo (-overhang, h + overhang);
            p.lineTo (-overhang, -overhang);
            break;

        case TabbedButtonBar::TabsAtBottom:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (indent, h);
            p.lineTo (w - indent, h);
            p.lineTo (w, 0.0f);
            p.lineTo (w + overhang, -overhang);
            p.lineTo (-overhang, -overhang);
            break;

        case TabbedButtonBar::TabsAtTop:
        default:
            p.startNewSubPath (0.0f, h);
            p.lineTo (indent, 0.0f);
            p.lineTo (w - indent, 0.0f);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (-overhang, h + overhang);
            break;
    }

    p.closeSubPath();

    p = p.createPathWithRoundedCorners (3.0f);
}

z0 LookAndFeel_V2::fillTabButtonShape (TabBarButton& button, Graphics& g, const Path& path,
                                         b8 /*isMouseOver*/, b8 /*isMouseDown*/)
{
    auto tabBackground = button.getTabBackgroundColor();
    const b8 isFrontTab = button.isFrontTab();

    g.setColor (isFrontTab ? tabBackground
                            : tabBackground.withMultipliedAlpha (0.9f));

    g.fillPath (path);

    g.setColor (button.findColor (isFrontTab ? TabbedButtonBar::frontOutlineColorId
                                               : TabbedButtonBar::tabOutlineColorId, false)
                    .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    g.strokePath (path, PathStrokeType (isFrontTab ? 1.0f : 0.5f));
}

Font LookAndFeel_V2::getTabButtonFont (TabBarButton&, f32 height)
{
    return withDefaultMetrics (FontOptions { height * 0.6f });
}

z0 LookAndFeel_V2::drawTabButtonText (TabBarButton& button, Graphics& g, b8 isMouseOver, b8 isMouseDown)
{
    auto area = button.getTextArea().toFloat();

    auto length = area.getWidth();
    auto depth  = area.getHeight();

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    Font font (getTabButtonFont (button, depth));
    font.setUnderline (button.hasKeyboardFocus (false));

    AffineTransform t;

    switch (button.getTabbedButtonBar().getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:   t = t.rotated (MathConstants<f32>::pi * -0.5f).translated (area.getX(), area.getBottom()); break;
        case TabbedButtonBar::TabsAtRight:  t = t.rotated (MathConstants<f32>::pi *  0.5f).translated (area.getRight(), area.getY()); break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
        default:                            jassertfalse; break;
    }

    Color col;

    if (button.isFrontTab() && (button.isColorSpecified (TabbedButtonBar::frontTextColorId)
                                    || isColorSpecified (TabbedButtonBar::frontTextColorId)))
        col = findColor (TabbedButtonBar::frontTextColorId);
    else if (button.isColorSpecified (TabbedButtonBar::tabTextColorId)
                 || isColorSpecified (TabbedButtonBar::tabTextColorId))
        col = findColor (TabbedButtonBar::tabTextColorId);
    else
        col = button.getTabBackgroundColor().contrasting();

    auto alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;

    g.setColor (col.withMultipliedAlpha (alpha));
    g.setFont (font);
    g.addTransform (t);

    g.drawFittedText (button.getButtonText().trim(),
                      0, 0, (i32) length, (i32) depth,
                      Justification::centred,
                      jmax (1, ((i32) depth) / 12));
}

z0 LookAndFeel_V2::drawTabButton (TabBarButton& button, Graphics& g, b8 isMouseOver, b8 isMouseDown)
{
    Path tabShape;
    createTabButtonShape (button, tabShape, isMouseOver, isMouseDown);

    auto activeArea = button.getActiveArea();
    tabShape.applyTransform (AffineTransform::translation ((f32) activeArea.getX(),
                                                           (f32) activeArea.getY()));

    DropShadow (Colors::black.withAlpha (0.5f), 2, Point<i32> (0, 1)).drawForPath (g, tabShape);

    fillTabButtonShape (button, g, tabShape, isMouseOver, isMouseDown);
    drawTabButtonText (button, g, isMouseOver, isMouseDown);
}

z0 LookAndFeel_V2::drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics&) {}

z0 LookAndFeel_V2::drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, i32k w, i32k h)
{
    auto shadowSize = 0.2f;

    Rectangle<i32> shadowRect, line;
    ColorGradient gradient (Colors::black.withAlpha (bar.isEnabled() ? 0.25f : 0.15f), 0, 0,
                             Colors::transparentBlack, 0, 0, false);

    switch (bar.getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            gradient.point1.x = (f32) w;
            gradient.point2.x = (f32) w * (1.0f - shadowSize);
            shadowRect.setBounds ((i32) gradient.point2.x, 0, w - (i32) gradient.point2.x, h);
            line.setBounds (w - 1, 0, 1, h);
            break;

        case TabbedButtonBar::TabsAtRight:
            gradient.point2.x = (f32) w * shadowSize;
            shadowRect.setBounds (0, 0, (i32) gradient.point2.x, h);
            line.setBounds (0, 0, 1, h);
            break;

        case TabbedButtonBar::TabsAtTop:
            gradient.point1.y = (f32) h;
            gradient.point2.y = (f32) h * (1.0f - shadowSize);
            shadowRect.setBounds (0, (i32) gradient.point2.y, w, h - (i32) gradient.point2.y);
            line.setBounds (0, h - 1, w, 1);
            break;

        case TabbedButtonBar::TabsAtBottom:
            gradient.point2.y = (f32) h * shadowSize;
            shadowRect.setBounds (0, 0, w, (i32) gradient.point2.y);
            line.setBounds (0, 0, w, 1);
            break;

        default: break;
    }

    g.setGradientFill (gradient);
    g.fillRect (shadowRect.expanded (2, 2));

    g.setColor (Color (0x80000000));
    g.fillRect (line);
}

Button* LookAndFeel_V2::createTabBarExtrasButton()
{
    auto thickness = 7.0f;
    auto indent = 22.0f;

    Path p;
    p.addEllipse (-10.0f, -10.0f, 120.0f, 120.0f);

    DrawablePath ellipse;
    ellipse.setPath (p);
    ellipse.setFill (Color (0x99ffffff));

    p.clear();
    p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
    p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
    p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
    p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);
    p.setUsingNonZeroWinding (false);

    DrawablePath dp;
    dp.setPath (p);
    dp.setFill (Color (0x59000000));

    DrawableComposite normalImage;
    normalImage.addAndMakeVisible (ellipse.createCopy().release());
    normalImage.addAndMakeVisible (dp.createCopy().release());

    dp.setFill (Color (0xcc000000));

    DrawableComposite overImage;
    overImage.addAndMakeVisible (ellipse.createCopy().release());
    overImage.addAndMakeVisible (dp.createCopy().release());

    auto db = new DrawableButton (TRANS ("Additional Items"), DrawableButton::ImageFitted);
    db->setImages (&normalImage, &overImage, nullptr);
    return db;
}


//==============================================================================
z0 LookAndFeel_V2::drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header)
{
    g.fillAll (Colors::white);

    auto area = header.getLocalBounds();
    area.removeFromTop (area.getHeight() / 2);

    auto backgroundColor = header.findColor (TableHeaderComponent::backgroundColorId);

    g.setGradientFill (ColorGradient (backgroundColor,
                                       0.0f, (f32) area.getY(),
                                       backgroundColor.withMultipliedSaturation (.5f),
                                       0.0f, (f32) area.getBottom(),
                                       false));
    g.fillRect (area);

    g.setColor (header.findColor (TableHeaderComponent::outlineColorId));
    g.fillRect (area.removeFromBottom (1));

    for (i32 i = header.getNumColumns (true); --i >= 0;)
        g.fillRect (header.getColumnPosition (i).removeFromRight (1));
}

z0 LookAndFeel_V2::drawTableHeaderColumn (Graphics& g, TableHeaderComponent& header,
                                            const Txt& columnName, i32 /*columnId*/,
                                            i32 width, i32 height, b8 isMouseOver, b8 isMouseDown,
                                            i32 columnFlags)
{
    auto highlightColor = header.findColor (TableHeaderComponent::highlightColorId);

    if (isMouseDown)
        g.fillAll (highlightColor);
    else if (isMouseOver)
        g.fillAll (highlightColor.withMultipliedAlpha (0.625f));

    Rectangle<i32> area (width, height);
    area.reduce (4, 0);

    if ((columnFlags & (TableHeaderComponent::sortedForwards | TableHeaderComponent::sortedBackwards)) != 0)
    {
        Path sortArrow;
        sortArrow.addTriangle (0.0f, 0.0f,
                               0.5f, (columnFlags & TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f,
                               1.0f, 0.0f);

        g.setColor (Color (0x99000000));
        g.fillPath (sortArrow, sortArrow.getTransformToScaleToFit (area.removeFromRight (height / 2).reduced (2).toFloat(), true));
    }

    g.setColor (header.findColor (TableHeaderComponent::textColorId));
    g.setFont (withDefaultMetrics (FontOptions ((f32) height * 0.5f, Font::bold)));
    g.drawFittedText (columnName, area, Justification::centredLeft, 1);
}

//==============================================================================
z0 LookAndFeel_V2::drawLasso (Graphics& g, Component& lassoComp)
{
    i32k outlineThickness = 1;

    g.fillAll (lassoComp.findColor (0x1000440 /*lassoFillColorId*/));

    g.setColor (lassoComp.findColor (0x1000441 /*lassoOutlineColorId*/));
    g.drawRect (lassoComp.getLocalBounds(), outlineThickness);
}

//==============================================================================
z0 LookAndFeel_V2::paintToolbarBackground (Graphics& g, i32 w, i32 h, Toolbar& toolbar)
{
    auto background = toolbar.findColor (Toolbar::backgroundColorId);

    g.setGradientFill (ColorGradient (background, 0.0f, 0.0f,
                                       background.darker (0.1f),
                                       toolbar.isVertical() ? (f32) w - 1.0f : 0.0f,
                                       toolbar.isVertical() ? 0.0f : (f32) h - 1.0f,
                                       false));
    g.fillAll();
}

Button* LookAndFeel_V2::createToolbarMissingItemsButton (Toolbar& /*toolbar*/)
{
    return createTabBarExtrasButton();
}

z0 LookAndFeel_V2::paintToolbarButtonBackground (Graphics& g, i32 /*width*/, i32 /*height*/,
                                                   b8 isMouseOver, b8 isMouseDown,
                                                   ToolbarItemComponent& component)
{
    if (isMouseDown)
        g.fillAll (component.findColor (Toolbar::buttonMouseDownBackgroundColorId, true));
    else if (isMouseOver)
        g.fillAll (component.findColor (Toolbar::buttonMouseOverBackgroundColorId, true));
}

z0 LookAndFeel_V2::paintToolbarButtonLabel (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                              const Txt& text, ToolbarItemComponent& component)
{
    g.setColor (component.findColor (Toolbar::labelTextColorId, true)
                    .withAlpha (component.isEnabled() ? 1.0f : 0.25f));

    auto fontHeight = jmin (14.0f, (f32) height * 0.85f);
    g.setFont (fontHeight);

    g.drawFittedText (text,
                      x, y, width, height,
                      Justification::centred,
                      jmax (1, height / (i32) fontHeight));
}

//==============================================================================
z0 LookAndFeel_V2::drawPropertyPanelSectionHeader (Graphics& g, const Txt& name,
                                                     b8 isOpen, i32 width, i32 height)
{
    auto buttonSize = (f32) height * 0.75f;
    auto buttonIndent = ((f32) height - buttonSize) * 0.5f;

    drawTreeviewPlusMinusBox (g, Rectangle<f32> (buttonIndent, buttonIndent, buttonSize, buttonSize), Colors::white, isOpen, false);

    auto textX = (i32) (buttonIndent * 2.0f + buttonSize + 2.0f);

    g.setColor (Colors::black);
    g.setFont (withDefaultMetrics (FontOptions ((f32) height * 0.7f, Font::bold)));
    g.drawText (name, textX, 0, width - textX - 4, height, Justification::centredLeft, true);
}

z0 LookAndFeel_V2::drawPropertyComponentBackground (Graphics& g, i32 width, i32 height, PropertyComponent& component)
{
    g.setColor (component.findColor (PropertyComponent::backgroundColorId));
    g.fillRect (0, 0, width, height - 1);
}

z0 LookAndFeel_V2::drawPropertyComponentLabel (Graphics& g, i32, i32 height, PropertyComponent& component)
{
    g.setColor (component.findColor (PropertyComponent::labelTextColorId)
                    .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

    g.setFont ((f32) jmin (height, 24) * 0.65f);

    auto r = getPropertyComponentContentPosition (component);

    g.drawFittedText (component.getName(),
                      3, r.getY(), r.getX() - 5, r.getHeight(),
                      Justification::centredLeft, 2);
}

Rectangle<i32> LookAndFeel_V2::getPropertyComponentContentPosition (PropertyComponent& component)
{
    i32k textW = jmin (200, component.getWidth() / 3);
    return Rectangle<i32> (textW, 1, component.getWidth() - textW - 1, component.getHeight() - 3);
}

i32 LookAndFeel_V2::getPropertyPanelSectionHeaderHeight (const Txt& sectionTitle)
{
    return sectionTitle.isEmpty() ? 0 : 22;
}

//==============================================================================
z0 LookAndFeel_V2::drawCallOutBoxBackground (CallOutBox& box, Graphics& g,
                                               const Path& path, Image& cachedImage)
{
    if (cachedImage.isNull())
    {
        cachedImage = Image (Image::ARGB, box.getWidth(), box.getHeight(), true);
        Graphics g2 (cachedImage);

        DropShadow (Colors::black.withAlpha (0.7f), 8, Point<i32> (0, 2)).drawForPath (g2, path);
    }

    g.setColor (Colors::black);
    g.drawImageAt (cachedImage, 0, 0);

    g.setColor (Color::greyLevel (0.23f).withAlpha (0.9f));
    g.fillPath (path);

    g.setColor (Colors::white.withAlpha (0.8f));
    g.strokePath (path, PathStrokeType (2.0f));
}

i32 LookAndFeel_V2::getCallOutBoxBorderSize (const CallOutBox&)
{
    return 20;
}

f32 LookAndFeel_V2::getCallOutBoxCornerSize (const CallOutBox&)
{
    return 9.0f;
}

//==============================================================================
AttributedString LookAndFeel_V2::createFileChooserHeaderText (const Txt& title,
                                                           const Txt& instructions)
{
    AttributedString s;
    s.setJustification (Justification::centred);

    auto colour = findColor (FileChooserDialogBox::titleTextColorId);
    s.append (title + "\n\n", withDefaultMetrics (FontOptions (17.0f, Font::bold)), colour);
    s.append (instructions, withDefaultMetrics (FontOptions (14.0f)), colour);

    return s;
}

z0 LookAndFeel_V2::drawFileBrowserRow (Graphics& g, i32 width, i32 height,
                                         const File&, const Txt& filename, Image* icon,
                                         const Txt& fileSizeDescription,
                                         const Txt& fileTimeDescription,
                                         b8 isDirectory, b8 isItemSelected,
                                         i32 /*itemIndex*/, DirectoryContentsDisplayComponent& dcc)
{
    auto fileListComp = dynamic_cast<Component*> (&dcc);

    if (isItemSelected)
        g.fillAll (fileListComp != nullptr ? fileListComp->findColor (DirectoryContentsDisplayComponent::highlightColorId)
                                           : findColor (DirectoryContentsDisplayComponent::highlightColorId));

    i32k x = 32;
    g.setColor (Colors::black);

    if (icon != nullptr && icon->isValid())
    {
        g.drawImageWithin (*icon, 2, 2, x - 4, height - 4,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);
    }
    else
    {
        if (auto* d = isDirectory ? getDefaultFolderImage()
                                  : getDefaultDocumentFileImage())
            d->drawWithin (g, Rectangle<f32> (2.0f, 2.0f, x - 4.0f, (f32) height - 4.0f),
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
    }

    if (isItemSelected)
        g.setColor (fileListComp != nullptr ? fileListComp->findColor (DirectoryContentsDisplayComponent::highlightedTextColorId)
                                             : findColor (DirectoryContentsDisplayComponent::highlightedTextColorId));
    else
        g.setColor (fileListComp != nullptr ? fileListComp->findColor (DirectoryContentsDisplayComponent::textColorId)
                                             : findColor (DirectoryContentsDisplayComponent::textColorId));

    g.setFont ((f32) height * 0.7f);

    if (width > 450 && ! isDirectory)
    {
        auto sizeX = roundToInt ((f32) width * 0.7f);
        auto dateX = roundToInt ((f32) width * 0.8f);

        g.drawFittedText (filename,
                          x, 0, sizeX - x, height,
                          Justification::centredLeft, 1);

        g.setFont ((f32) height * 0.5f);
        g.setColor (Colors::darkgrey);

        if (! isDirectory)
        {
            g.drawFittedText (fileSizeDescription,
                              sizeX, 0, dateX - sizeX - 8, height,
                              Justification::centredRight, 1);

            g.drawFittedText (fileTimeDescription,
                              dateX, 0, width - 8 - dateX, height,
                              Justification::centredRight, 1);
        }
    }
    else
    {
        g.drawFittedText (filename,
                          x, 0, width - x, height,
                          Justification::centredLeft, 1);

    }
}

Button* LookAndFeel_V2::createFileBrowserGoUpButton()
{
    auto goUpButton = new DrawableButton ("up", DrawableButton::ImageOnButtonBackground);

    Path arrowPath;
    arrowPath.addArrow ({ 50.0f, 100.0f, 50.0f, 0.0f }, 40.0f, 100.0f, 50.0f);

    DrawablePath arrowImage;
    arrowImage.setFill (Colors::black.withAlpha (0.4f));
    arrowImage.setPath (arrowPath);

    goUpButton->setImages (&arrowImage);

    return goUpButton;
}

z0 LookAndFeel_V2::layoutFileBrowserComponent (FileBrowserComponent& browserComp,
                                                 DirectoryContentsDisplayComponent* fileListComponent,
                                                 FilePreviewComponent* previewComp,
                                                 ComboBox* currentPathBox,
                                                 TextEditor* filenameBox,
                                                 Button* goUpButton)
{
    i32k x = 8;
    auto w = browserComp.getWidth() - x - x;

    if (previewComp != nullptr)
    {
        auto previewWidth = w / 3;
        previewComp->setBounds (x + w - previewWidth, 0, previewWidth, browserComp.getHeight());

        w -= previewWidth + 4;
    }

    i32 y = 4;

    i32k controlsHeight = 22;
    i32k upButtonWidth = 50;
    auto bottomSectionHeight = controlsHeight + 8;

    currentPathBox->setBounds (x, y, w - upButtonWidth - 6, controlsHeight);
    goUpButton->setBounds (x + w - upButtonWidth, y, upButtonWidth, controlsHeight);

    y += controlsHeight + 4;

    if (auto listAsComp = dynamic_cast<Component*> (fileListComponent))
    {
        listAsComp->setBounds (x, y, w, browserComp.getHeight() - y - bottomSectionHeight);
        y = listAsComp->getBottom() + 4;
    }

    filenameBox->setBounds (x + 50, y, w - 50, controlsHeight);
}

//==============================================================================
static std::unique_ptr<Drawable> createDrawableFromSVG (tukk data)
{
    auto xml = parseXML (data);
    jassert (xml != nullptr);
    return Drawable::createFromSVG (*xml);
}

const Drawable* LookAndFeel_V2::getDefaultFolderImage()
{
    if (folderImage == nullptr)
        folderImage = createDrawableFromSVG (R"svgdata(
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="706" height="532">
  <defs>
    <linearGradient id="a">
      <stop stop-color="#adf" offset="0"/>
      <stop stop-color="#ecfaff" offset="1"/>
    </linearGradient>
    <linearGradient id="b" x1=".6" x2="0" y1=".9" xlink:href="#a"/>
    <linearGradient id="c" x1=".6" x2=".1" y1=".9" y2=".3" xlink:href="#a"/>
  </defs>
  <g class="currentLayer">
    <path d="M112.1 104c-8.2 2.2-13.2 11.6-11.3 21l68.3 342.7c1.9 9.4 10.1 15.2 18.4 13l384.3-104.1c8.2-2.2 13.2-11.6 11.3-21l-48-266a15.8 15.8 0 0 0-18.4-12.8l-224.2 38s-20.3-41.3-28.3-39.3z" display="block" fill="url(#b)" stroke="#446c98" stroke-width="7"/>
    <path d="M608.6 136.8L235.2 208a22.7 22.7 0 0 0-16 19l-40.8 241c1.7 8.4 9.6 14.5 17.8 12.3l380-104c8-2.2 10.7-10.2 12.3-18.4l38-210.1c.4-15.4-10.4-11.8-18-11.1z" display="block" fill="url(#c)" opacity=".8" stroke="#446c98" stroke-width="7"/>
  </g>
</svg>
)svgdata");

    return folderImage.get();
}

const Drawable* LookAndFeel_V2::getDefaultDocumentFileImage()
{
    if (documentImage == nullptr)
        documentImage = createDrawableFromSVG (R"svgdata(
<svg version="1" viewBox="-10 -10 450 600" xmlns="http://www.w3.org/2000/svg">
  <path d="M17 0h290l120 132v426c0 10-8 19-17 19H17c-9 0-17-9-17-19V19C0 8 8 0 17 0z" fill="#e5e5e5" stroke="#888888" stroke-width="7"/>
  <path d="M427 132H324c-9 0-17-9-17-19V0l120 132z" fill="#ccc"/>
</svg>
)svgdata");

    return documentImage.get();
}

//==============================================================================
static Path createPathFromData (f32 height, u8k* data, size_t size)
{
    Path p;
    p.loadPathFromData (data, size);
    p.scaleToFit (0, 0, height * 2.0f, height, true);
    return p;
}

Path LookAndFeel_V2::getTickShape (f32 height)
{
    static u8k data[] =
    {
        109,0,224,168,68,0,0,119,67,108,0,224,172,68,0,128,146,67,113,0,192,148,68,0,0,219,67,0,96,110,68,0,224,56,68,113,0,64,51,68,0,32,130,68,0,64,20,68,0,224,
        162,68,108,0,128,3,68,0,128,168,68,113,0,128,221,67,0,192,175,68,0,0,207,67,0,32,179,68,113,0,0,201,67,0,224,173,68,0,0,181,67,0,224,161,68,108,0,128,168,67,
        0,128,154,68,113,0,128,141,67,0,192,138,68,0,128,108,67,0,64,131,68,113,0,0,62,67,0,128,119,68,0,0,5,67,0,128,114,68,113,0,0,102,67,0,192,88,68,0,128,155,
        67,0,192,88,68,113,0,0,190,67,0,192,88,68,0,128,232,67,0,224,131,68,108,0,128,246,67,0,192,139,68,113,0,64,33,68,0,128,87,68,0,0,93,68,0,224,26,68,113,0,
        96,140,68,0,128,188,67,0,224,168,68,0,0,119,67,99,101
    };

    return createPathFromData (height, data, sizeof (data));
}

Path LookAndFeel_V2::getCrossShape (f32 height)
{
    static u8k data[] =
    {
        109,0,0,17,68,0,96,145,68,108,0,192,13,68,0,192,147,68,113,0,0,213,67,0,64,174,68,0,0,168,67,0,64,174,68,113,0,0,104,67,0,64,174,68,0,0,5,67,0,64,
        153,68,113,0,0,18,67,0,64,153,68,0,0,24,67,0,64,153,68,113,0,0,135,67,0,64,153,68,0,128,207,67,0,224,130,68,108,0,0,220,67,0,0,126,68,108,0,0,204,67,
        0,128,117,68,113,0,0,138,67,0,64,82,68,0,0,138,67,0,192,57,68,113,0,0,138,67,0,192,37,68,0,128,210,67,0,64,10,68,113,0,128,220,67,0,64,45,68,0,0,8,
        68,0,128,78,68,108,0,192,14,68,0,0,87,68,108,0,64,20,68,0,0,80,68,113,0,192,57,68,0,0,32,68,0,128,88,68,0,0,32,68,113,0,64,112,68,0,0,32,68,0,
        128,124,68,0,64,68,68,113,0,0,121,68,0,192,67,68,0,128,119,68,0,192,67,68,113,0,192,108,68,0,192,67,68,0,32,89,68,0,96,82,68,113,0,128,69,68,0,0,97,68,
        0,0,56,68,0,64,115,68,108,0,64,49,68,0,128,124,68,108,0,192,55,68,0,96,129,68,113,0,0,92,68,0,224,146,68,0,192,129,68,0,224,146,68,113,0,64,110,68,0,64,
        168,68,0,64,87,68,0,64,168,68,113,0,128,66,68,0,64,168,68,0,64,27,68,0,32,150,68,99,101
    };

    return createPathFromData (height, data, sizeof (data));
}

//==============================================================================
z0 LookAndFeel_V2::drawLevelMeter (Graphics& g, i32 width, i32 height, f32 level)
{
    g.setColor (Colors::white.withAlpha (0.7f));
    g.fillRoundedRectangle (0.0f, 0.0f, (f32) width, (f32) height, 3.0f);
    g.setColor (Colors::black.withAlpha (0.2f));
    g.drawRoundedRectangle (1.0f, 1.0f, (f32) width - 2.0f, (f32) height - 2.0f, 3.0f, 1.0f);

    i32k totalBlocks = 7;
    i32k numBlocks = roundToInt (totalBlocks * level);
    auto w = ((f32) width - 6.0f) / (f32) totalBlocks;

    for (i32 i = 0; i < totalBlocks; ++i)
    {
        if (i >= numBlocks)
            g.setColor (Colors::lightblue.withAlpha (0.6f));
        else
            g.setColor (i < totalBlocks - 1 ? Colors::blue.withAlpha (0.5f)
                                             : Colors::red);

        g.fillRoundedRectangle (3.0f + (f32) i * w + w * 0.1f,
                                3.0f,
                                (f32) w * 0.8f,
                                (f32) height - 6.0f,
                                (f32) w * 0.4f);
    }
}

//==============================================================================
z0 LookAndFeel_V2::drawKeymapChangeButton (Graphics& g, i32 width, i32 height, Button& button, const Txt& keyDescription)
{
    auto textColor = button.findColor (0x100ad01 /*KeyMappingEditorComponent::textColorId*/, true);

    if (keyDescription.isNotEmpty())
    {
        if (button.isEnabled())
        {
            auto alpha = button.isDown() ? 0.3f : (button.isOver() ? 0.15f : 0.08f);
            g.fillAll (textColor.withAlpha (alpha));

            g.setOpacity (0.3f);
            drawBevel (g, 0, 0, width, height, 2);
        }

        g.setColor (textColor);
        g.setFont ((f32) height * 0.6f);
        g.drawFittedText (keyDescription,
                          3, 0, width - 6, height,
                          Justification::centred, 1);
    }
    else
    {
        const f32 thickness = 7.0f;
        const f32 indent = 22.0f;

        Path p;
        p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
        p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
        p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
        p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);
        p.setUsingNonZeroWinding (false);

        g.setColor (textColor.withAlpha (button.isDown() ? 0.7f : (button.isOver() ? 0.5f : 0.3f)));
        g.fillPath (p, p.getTransformToScaleToFit (2.0f, 2.0f, (f32) width - 4.0f, (f32) height - 4.0f, true));
    }

    if (button.hasKeyboardFocus (false))
    {
        g.setColor (textColor.withAlpha (0.4f));
        g.drawRect (0, 0, width, height);
    }
}

//==============================================================================
Font LookAndFeel_V2::getSidePanelTitleFont (SidePanel&)
{
    return withDefaultMetrics (FontOptions (18.0f));
}

Justification LookAndFeel_V2::getSidePanelTitleJustification (SidePanel& panel)
{
    return panel.isPanelOnLeft() ? Justification::centredRight
                                 : Justification::centredLeft;
}

Path LookAndFeel_V2::getSidePanelDismissButtonShape (SidePanel& panel)
{
    return getCrossShape ((f32) panel.getTitleBarHeight());
}

//==============================================================================
z0 LookAndFeel_V2::drawBevel (Graphics& g, i32k x, i32k y, i32k width, i32k height,
                                i32k bevelThickness, const Color& topLeftColor, const Color& bottomRightColor,
                                const b8 useGradient, const b8 sharpEdgeOnOutside)
{
    if (g.clipRegionIntersects (Rectangle<i32> (x, y, width, height)))
    {
        auto& context = g.getInternalContext();
        Graphics::ScopedSaveState ss (g);

        for (i32 i = bevelThickness; --i >= 0;)
        {
            const f32 op = useGradient ? (f32) (sharpEdgeOnOutside ? bevelThickness - i : i) / (f32) bevelThickness
                                         : 1.0f;

            context.setFill (topLeftColor.withMultipliedAlpha (op));
            context.fillRect (Rectangle<i32> (x + i, y + i, width - i * 2, 1), false);
            context.setFill (topLeftColor.withMultipliedAlpha (op * 0.75f));
            context.fillRect (Rectangle<i32> (x + i, y + i + 1, 1, height - i * 2 - 2), false);
            context.setFill (bottomRightColor.withMultipliedAlpha (op));
            context.fillRect (Rectangle<i32> (x + i, y + height - i - 1, width - i * 2, 1), false);
            context.setFill (bottomRightColor.withMultipliedAlpha (op  * 0.75f));
            context.fillRect (Rectangle<i32> (x + width - i - 1, y + i + 1, 1, height - i * 2 - 2), false);
        }
    }
}

//==============================================================================
z0 LookAndFeel_V2::drawShinyButtonShape (Graphics& g, f32 x, f32 y, f32 w, f32 h,
                                           f32 maxCornerSize, const Color& baseColor, f32 strokeWidth,
                                           b8 flatOnLeft, b8 flatOnRight, b8 flatOnTop, b8 flatOnBottom) noexcept
{
    if (w <= strokeWidth * 1.1f || h <= strokeWidth * 1.1f)
        return;

    auto cs = jmin (maxCornerSize, w * 0.5f, h * 0.5f);

    Path outline;
    outline.addRoundedRectangle (x, y, w, h, cs, cs,
                                 ! (flatOnLeft  || flatOnTop),
                                 ! (flatOnRight || flatOnTop),
                                 ! (flatOnLeft  || flatOnBottom),
                                 ! (flatOnRight || flatOnBottom));

    ColorGradient cg (baseColor, 0.0f, y,
                       baseColor.overlaidWith (Color (0x070000ff)), 0.0f, y + h,
                       false);

    cg.addColor (0.5,  baseColor.overlaidWith (Color (0x33ffffff)));
    cg.addColor (0.51, baseColor.overlaidWith (Color (0x110000ff)));

    g.setGradientFill (cg);
    g.fillPath (outline);

    g.setColor (Color (0x80000000));
    g.strokePath (outline, PathStrokeType (strokeWidth));
}

//==============================================================================
z0 LookAndFeel_V2::drawGlassSphere (Graphics& g, const f32 x, const f32 y,
                                      const f32 diameter, const Color& colour,
                                      const f32 outlineThickness) noexcept
{
    if (diameter <= outlineThickness)
        return;

    Path p;
    p.addEllipse (x, y, diameter, diameter);

    {
        ColorGradient cg (Colors::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y,
                           Colors::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y + diameter, false);

        cg.addColor (0.4, Colors::white.overlaidWith (colour));

        g.setGradientFill (cg);
        g.fillPath (p);
    }

    g.setGradientFill (ColorGradient (Colors::white, 0, y + diameter * 0.06f,
                                       Colors::transparentWhite, 0, y + diameter * 0.3f, false));
    g.fillEllipse (x + diameter * 0.2f, y + diameter * 0.05f, diameter * 0.6f, diameter * 0.4f);

    ColorGradient cg (Colors::transparentBlack,
                       x + diameter * 0.5f, y + diameter * 0.5f,
                       Colors::black.withAlpha (0.5f * outlineThickness * colour.getFloatAlpha()),
                       x, y + diameter * 0.5f, true);

    cg.addColor (0.7, Colors::transparentBlack);
    cg.addColor (0.8, Colors::black.withAlpha (0.1f * outlineThickness));

    g.setGradientFill (cg);
    g.fillPath (p);

    g.setColor (Colors::black.withAlpha (0.5f * colour.getFloatAlpha()));
    g.drawEllipse (x, y, diameter, diameter, outlineThickness);
}

//==============================================================================
z0 LookAndFeel_V2::drawGlassPointer (Graphics& g,
                                       const f32 x, const f32 y, const f32 diameter,
                                       const Color& colour, const f32 outlineThickness,
                                       i32k direction) noexcept
{
    if (diameter <= outlineThickness)
        return;

    Path p;
    p.startNewSubPath (x + diameter * 0.5f, y);
    p.lineTo (x + diameter, y + diameter * 0.6f);
    p.lineTo (x + diameter, y + diameter);
    p.lineTo (x, y + diameter);
    p.lineTo (x, y + diameter * 0.6f);
    p.closeSubPath();

    p.applyTransform (AffineTransform::rotation ((f32) direction * MathConstants<f32>::halfPi,
                                                 x + diameter * 0.5f,
                                                 y + diameter * 0.5f));

    {
        ColorGradient cg (Colors::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y,
                           Colors::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y + diameter, false);

        cg.addColor (0.4, Colors::white.overlaidWith (colour));

        g.setGradientFill (cg);
        g.fillPath (p);
    }

    ColorGradient cg (Colors::transparentBlack,
                       x + diameter * 0.5f, y + diameter * 0.5f,
                       Colors::black.withAlpha (0.5f * outlineThickness * colour.getFloatAlpha()),
                       x - diameter * 0.2f, y + diameter * 0.5f, true);

    cg.addColor (0.5, Colors::transparentBlack);
    cg.addColor (0.7, Colors::black.withAlpha (0.07f * outlineThickness));

    g.setGradientFill (cg);
    g.fillPath (p);

    g.setColor (Colors::black.withAlpha (0.5f * colour.getFloatAlpha()));
    g.strokePath (p, PathStrokeType (outlineThickness));
}

//==============================================================================
z0 LookAndFeel_V2::drawGlassLozenge (Graphics& g,
                                       f32 x, f32 y, f32 width, f32 height,
                                       const Color& colour, f32 outlineThickness, f32 cornerSize,
                                       b8 flatOnLeft, b8 flatOnRight, b8 flatOnTop, b8 flatOnBottom) noexcept
{
    if (width <= outlineThickness || height <= outlineThickness)
        return;

    auto intX = (i32) x;
    auto intY = (i32) y;
    auto intW = (i32) width;
    auto intH = (i32) height;

    auto cs = cornerSize < 0 ? jmin (width * 0.5f, height * 0.5f) : cornerSize;
    auto edgeBlurRadius = height * 0.75f + (height - cs * 2.0f);
    auto intEdge = (i32) edgeBlurRadius;

    Path outline;
    outline.addRoundedRectangle (x, y, width, height, cs, cs,
                                 ! (flatOnLeft || flatOnTop),
                                 ! (flatOnRight || flatOnTop),
                                 ! (flatOnLeft || flatOnBottom),
                                 ! (flatOnRight || flatOnBottom));

    {
        ColorGradient cg (colour.darker (0.2f), 0, y,
                           colour.darker (0.2f), 0, y + height, false);

        cg.addColor (0.03, colour.withMultipliedAlpha (0.3f));
        cg.addColor (0.4, colour);
        cg.addColor (0.97, colour.withMultipliedAlpha (0.3f));

        g.setGradientFill (cg);
        g.fillPath (outline);
    }

    ColorGradient cg (Colors::transparentBlack, x + edgeBlurRadius, y + height * 0.5f,
                       colour.darker (0.2f), x, y + height * 0.5f, true);

    cg.addColor (jlimit (0.0, 1.0, 1.0 - (cs * 0.5f) / edgeBlurRadius), Colors::transparentBlack);
    cg.addColor (jlimit (0.0, 1.0, 1.0 - (cs * 0.25f) / edgeBlurRadius), colour.darker (0.2f).withMultipliedAlpha (0.3f));

    if (! (flatOnLeft || flatOnTop || flatOnBottom))
    {
        Graphics::ScopedSaveState ss (g);

        g.setGradientFill (cg);
        g.reduceClipRegion (intX, intY, intEdge, intH);
        g.fillPath (outline);
    }

    if (! (flatOnRight || flatOnTop || flatOnBottom))
    {
        cg.point1.setX (x + width - edgeBlurRadius);
        cg.point2.setX (x + width);

        Graphics::ScopedSaveState ss (g);

        g.setGradientFill (cg);
        g.reduceClipRegion (intX + intW - intEdge, intY, 2 + intEdge, intH);
        g.fillPath (outline);
    }

    {
        auto leftIndent  = (flatOnTop || flatOnLeft)  ? 0.0f : cs * 0.4f;
        auto rightIndent = (flatOnTop || flatOnRight) ? 0.0f : cs * 0.4f;

        Path highlight;
        highlight.addRoundedRectangle (x + leftIndent,
                                       y + cs * 0.1f,
                                       width - (leftIndent + rightIndent),
                                       height * 0.4f,
                                       cs * 0.4f,
                                       cs * 0.4f,
                                       ! (flatOnLeft || flatOnTop),
                                       ! (flatOnRight || flatOnTop),
                                       ! (flatOnLeft || flatOnBottom),
                                       ! (flatOnRight || flatOnBottom));

        g.setGradientFill (ColorGradient (colour.brighter (10.0f), 0, y + height * 0.06f,
                                           Colors::transparentWhite, 0, y + height * 0.4f, false));
        g.fillPath (highlight);
    }

    g.setColor (colour.darker().withMultipliedAlpha (1.5f));
    g.strokePath (outline, PathStrokeType (outlineThickness));
}

} // namespace drx
