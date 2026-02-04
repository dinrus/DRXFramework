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

LookAndFeel_V3::LookAndFeel_V3()
{
    setColor (TreeView::selectedItemBackgroundColorId, Color (0x301111ee));

    const Color textButtonColor (0xffeeeeff);
    setColor (TextButton::buttonColorId, textButtonColor);
    setColor (TextButton::buttonOnColorId, Color (0xff888888));
    setColor (ComboBox::buttonColorId, textButtonColor);
    setColor (ComboBox::focusedOutlineColorId, textButtonColor);
    setColor (TextEditor::outlineColorId, Colors::transparentBlack);
    setColor (TabbedButtonBar::tabOutlineColorId, Color (0x66000000));
    setColor (TabbedComponent::outlineColorId, Color (0x66000000));
    setColor (Slider::trackColorId, Color (0xbbffffff));
    setColor (Slider::thumbColorId, Color (0xffddddff));
    setColor (BubbleComponent::backgroundColorId, Color (0xeeeeeedd));
    setColor (ScrollBar::thumbColorId, Color::greyLevel (0.8f).contrasting().withAlpha (0.13f));
    setColor (TableHeaderComponent::backgroundColorId, Colors::white.withAlpha (0.6f));
    setColor (TableHeaderComponent::outlineColorId,    Colors::black.withAlpha (0.5f));
}

LookAndFeel_V3::~LookAndFeel_V3() {}

b8 LookAndFeel_V3::areScrollbarButtonsVisible()        { return false; }

z0 LookAndFeel_V3::drawStretchableLayoutResizerBar (Graphics& g, i32 /*w*/, i32 /*h*/, b8 /*isVerticalBar*/,
                                                      b8 isMouseOver, b8 isMouseDragging)
{
    if (isMouseOver || isMouseDragging)
        g.fillAll (Colors::yellow.withAlpha (0.4f));
}

z0 LookAndFeel_V3::drawScrollbar (Graphics& g, ScrollBar& scrollbar, i32 x, i32 y, i32 width, i32 height,
                                    b8 isScrollbarVertical, i32 thumbStartPosition, i32 thumbSize, b8 isMouseOver, b8 isMouseDown)
{
    Path thumbPath;

    if (thumbSize > 0)
    {
        const f32 thumbIndent = (f32) (isScrollbarVertical ? width : height) * 0.25f;
        const f32 thumbIndentx2 = thumbIndent * 2.0f;

        if (isScrollbarVertical)
            thumbPath.addRoundedRectangle ((f32) x + thumbIndent, (f32) thumbStartPosition + thumbIndent,
                                           (f32) width - thumbIndentx2, (f32) thumbSize - thumbIndentx2, ((f32) width - thumbIndentx2) * 0.5f);
        else
            thumbPath.addRoundedRectangle ((f32) thumbStartPosition + thumbIndent, (f32) y + thumbIndent,
                                           (f32) thumbSize - thumbIndentx2, (f32) height - thumbIndentx2, ((f32) height - thumbIndentx2) * 0.5f);
    }

    Color thumbCol (scrollbar.findColor (ScrollBar::thumbColorId, true));

    if (isMouseOver || isMouseDown)
        thumbCol = thumbCol.withMultipliedAlpha (2.0f);

    g.setColor (thumbCol);
    g.fillPath (thumbPath);

    g.setColor (thumbCol.contrasting ((isMouseOver  || isMouseDown) ? 0.2f : 0.1f));
    g.strokePath (thumbPath, PathStrokeType (1.0f));
}

z0 LookAndFeel_V3::drawConcertinaPanelHeader (Graphics& g, const Rectangle<i32>& area,
                                                b8 isMouseOver, b8 /*isMouseDown*/,
                                                ConcertinaPanel&, Component& panel)
{
    const Color bkg (Colors::grey);

    g.setGradientFill (ColorGradient::vertical (Colors::white.withAlpha (isMouseOver ? 0.4f : 0.2f), (f32) area.getY(),
                                                 Colors::darkgrey.withAlpha (0.1f), (f32) area.getBottom()));
    g.fillAll();

    g.setColor (bkg.contrasting().withAlpha (0.1f));
    g.fillRect (area.withHeight (1));
    g.fillRect (area.withTop (area.getBottom() - 1));

    g.setColor (bkg.contrasting());
    g.setFont (Font (withDefaultMetrics (FontOptions { (f32) area.getHeight() * 0.6f })).boldened());
    g.drawFittedText (panel.getName(), 4, 0, area.getWidth() - 6, area.getHeight(), Justification::centredLeft, 1);
}

static z0 drawButtonShape (Graphics& g, const Path& outline, Color baseColor, f32 height)
{
    const f32 mainBrightness = baseColor.getBrightness();
    const f32 mainAlpha = baseColor.getFloatAlpha();

    g.setGradientFill (ColorGradient::vertical (baseColor.brighter (0.2f), 0.0f,
                                                 baseColor.darker (0.25f), height));
    g.fillPath (outline);

    g.setColor (Colors::white.withAlpha (0.4f * mainAlpha * mainBrightness * mainBrightness));
    g.strokePath (outline, PathStrokeType (1.0f), AffineTransform::translation (0.0f, 1.0f)
                                                        .scaled (1.0f, (height - 1.6f) / height));

    g.setColor (Colors::black.withAlpha (0.4f * mainAlpha));
    g.strokePath (outline, PathStrokeType (1.0f));
}

z0 LookAndFeel_V3::drawButtonBackground (Graphics& g, Button& button, const Color& backgroundColor,
                                           b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown)
{
    Color baseColor (backgroundColor.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                       .withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f));

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColor = baseColor.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.1f);

    const b8 flatOnLeft   = button.isConnectedOnLeft();
    const b8 flatOnRight  = button.isConnectedOnRight();
    const b8 flatOnTop    = button.isConnectedOnTop();
    const b8 flatOnBottom = button.isConnectedOnBottom();

    const f32 width  = (f32) button.getWidth()  - 1.0f;
    const f32 height = (f32) button.getHeight() - 1.0f;

    if (width > 0 && height > 0)
    {
        const f32 cornerSize = 4.0f;

        Path outline;
        outline.addRoundedRectangle (0.5f, 0.5f, width, height, cornerSize, cornerSize,
                                     ! (flatOnLeft  || flatOnTop),
                                     ! (flatOnRight || flatOnTop),
                                     ! (flatOnLeft  || flatOnBottom),
                                     ! (flatOnRight || flatOnBottom));

        drawButtonShape (g, outline, baseColor, height);
    }
}

z0 LookAndFeel_V3::drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header)
{
    auto r = header.getLocalBounds();
    auto outlineColor = header.findColor (TableHeaderComponent::outlineColorId);

    g.setColor (outlineColor);
    g.fillRect (r.removeFromBottom (1));

    g.setColor (header.findColor (TableHeaderComponent::backgroundColorId));
    g.fillRect (r);

    g.setColor (outlineColor);

    for (i32 i = header.getNumColumns (true); --i >= 0;)
        g.fillRect (header.getColumnPosition (i).removeFromRight (1));
}

i32 LookAndFeel_V3::getTabButtonOverlap (i32 /*tabDepth*/)            { return -1; }
i32 LookAndFeel_V3::getTabButtonSpaceAroundImage()                    { return 0; }

z0 LookAndFeel_V3::createTabTextLayout (const TabBarButton& button, f32 length, f32 depth,
                                          Color colour, TextLayout& textLayout)
{
    Font font (button.withDefaultMetrics (FontOptions { depth * 0.5f }));
    font.setUnderline (button.hasKeyboardFocus (false));

    AttributedString s;
    s.setJustification (Justification::centred);
    s.append (button.getButtonText().trim(), font, colour);

    textLayout.createLayout (s, length);
}

z0 LookAndFeel_V3::drawTabButton (TabBarButton& button, Graphics& g, b8 isMouseOver, b8 isMouseDown)
{
    const Rectangle<i32> activeArea (button.getActiveArea());

    const TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();

    const Color bkg (button.getTabBackgroundColor());

    if (button.getToggleState())
    {
        g.setColor (bkg);
    }
    else
    {
        Point<i32> p1, p2;

        switch (o)
        {
            case TabbedButtonBar::TabsAtBottom:   p1 = activeArea.getBottomLeft(); p2 = activeArea.getTopLeft();    break;
            case TabbedButtonBar::TabsAtTop:      p1 = activeArea.getTopLeft();    p2 = activeArea.getBottomLeft(); break;
            case TabbedButtonBar::TabsAtRight:    p1 = activeArea.getTopRight();   p2 = activeArea.getTopLeft();    break;
            case TabbedButtonBar::TabsAtLeft:     p1 = activeArea.getTopLeft();    p2 = activeArea.getTopRight();   break;
            default:                              jassertfalse; break;
        }

        g.setGradientFill (ColorGradient (bkg.brighter (0.2f), p1.toFloat(),
                                           bkg.darker (0.1f),   p2.toFloat(), false));
    }

    g.fillRect (activeArea);

    g.setColor (button.findColor (TabbedButtonBar::tabOutlineColorId));

    Rectangle<i32> r (activeArea);

    if (o != TabbedButtonBar::TabsAtBottom)   g.fillRect (r.removeFromTop (1));
    if (o != TabbedButtonBar::TabsAtTop)      g.fillRect (r.removeFromBottom (1));
    if (o != TabbedButtonBar::TabsAtRight)    g.fillRect (r.removeFromLeft (1));
    if (o != TabbedButtonBar::TabsAtLeft)     g.fillRect (r.removeFromRight (1));

    const f32 alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;

    Color col (bkg.contrasting().withMultipliedAlpha (alpha));

    if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
    {
        TabbedButtonBar::ColorIds colID = button.isFrontTab() ? TabbedButtonBar::frontTextColorId
                                                               : TabbedButtonBar::tabTextColorId;

        if (bar->isColorSpecified (colID))
            col = bar->findColor (colID);
        else if (isColorSpecified (colID))
            col = findColor (colID);
    }

    const Rectangle<f32> area (button.getTextArea().toFloat());

    f32 length = area.getWidth();
    f32 depth  = area.getHeight();

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    TextLayout textLayout;
    createTabTextLayout (button, length, depth, col, textLayout);

    AffineTransform t;

    switch (o)
    {
        case TabbedButtonBar::TabsAtLeft:   t = t.rotated (MathConstants<f32>::pi * -0.5f).translated (area.getX(), area.getBottom()); break;
        case TabbedButtonBar::TabsAtRight:  t = t.rotated (MathConstants<f32>::pi *  0.5f).translated (area.getRight(), area.getY()); break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
        default:                            jassertfalse; break;
    }

    g.addTransform (t);
    textLayout.draw (g, Rectangle<f32> (length, depth));
}

z0 LookAndFeel_V3::drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, i32k w, i32k h)
{
    const f32 shadowSize = 0.15f;

    Rectangle<i32> shadowRect, line;
    ColorGradient gradient (Colors::black.withAlpha (bar.isEnabled() ? 0.08f : 0.04f), 0, 0,
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

    g.setColor (bar.findColor (TabbedButtonBar::tabOutlineColorId));
    g.fillRect (line);
}

z0 LookAndFeel_V3::drawTextEditorOutline (Graphics& g, i32 width, i32 height, TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
        {
            g.setColor (textEditor.findColor (TextEditor::focusedOutlineColorId));
            g.drawRect (0, 0, width, height, 2);
        }
        else
        {
            g.setColor (textEditor.findColor (TextEditor::outlineColorId));
            g.drawRect (0, 0, width, height);
        }
    }
}

z0 LookAndFeel_V3::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<f32>& area,
                                               Color backgroundColor, b8 isOpen, b8 isMouseOver)
{
    Path p;
    p.addTriangle (0.0f, 0.0f, 1.0f, isOpen ? 0.0f : 0.5f, isOpen ? 0.5f : 0.0f, 1.0f);

    g.setColor (backgroundColor.contrasting().withAlpha (isMouseOver ? 0.5f : 0.3f));
    g.fillPath (p, p.getTransformToScaleToFit (area.reduced (2, area.getHeight() / 4), true));
}

b8 LookAndFeel_V3::areLinesDrawnForTreeView (TreeView&)
{
    return false;
}

i32 LookAndFeel_V3::getTreeViewIndentSize (TreeView&)
{
    return 20;
}

z0 LookAndFeel_V3::drawComboBox (Graphics& g, i32 width, i32 height, const b8 /*isMouseButtonDown*/,
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

    g.setColor (box.findColor (ComboBox::arrowColorId).withMultipliedAlpha (box.isEnabled() ? 1.0f : 0.3f));
    g.fillPath (p);
}

z0 LookAndFeel_V3::drawLinearSlider (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                       f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                       const Slider::SliderStyle style, Slider& slider)
{
    g.fillAll (slider.findColor (Slider::backgroundColorId));

    if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
    {
        const f32 fx = (f32) x, fy = (f32) y, fw = (f32) width, fh = (f32) height;

        Path p;

        if (style == Slider::LinearBarVertical)
            p.addRectangle (fx, sliderPos, fw, 1.0f + fh - sliderPos);
        else
            p.addRectangle (fx, fy, sliderPos - fx, fh);

        auto baseColor = slider.findColor (Slider::thumbColorId)
                                .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                .withMultipliedAlpha (0.8f);

        g.setGradientFill (ColorGradient::vertical (baseColor.brighter (0.08f), 0.0f,
                                                     baseColor.darker (0.08f), (f32) height));
        g.fillPath (p);

        g.setColor (baseColor.darker (0.2f));

        if (style == Slider::LinearBarVertical)
            g.fillRect (fx, sliderPos, fw, 1.0f);
        else
            g.fillRect (sliderPos, fy, 1.0f, fh);

        drawLinearSliderOutline (g, x, y, width, height, style, slider);
    }
    else
    {
        drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

z0 LookAndFeel_V3::drawLinearSliderBackground (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                                 f32 /*sliderPos*/,
                                                 f32 /*minSliderPos*/,
                                                 f32 /*maxSliderPos*/,
                                                 const Slider::SliderStyle /*style*/, Slider& slider)
{
    const f32 sliderRadius = (f32) (getSliderThumbRadius (slider) - 2);

    const Color trackColor (slider.findColor (Slider::trackColorId));
    const Color gradCol1 (trackColor.overlaidWith (Color (slider.isEnabled() ? 0x13000000 : 0x09000000)));
    const Color gradCol2 (trackColor.overlaidWith (Color (0x06000000)));
    Path indent;

    if (slider.isHorizontal())
    {
        auto iy = (f32) y + (f32) height * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill (ColorGradient::vertical (gradCol1, iy, gradCol2, iy + sliderRadius));

        indent.addRoundedRectangle ((f32) x - sliderRadius * 0.5f, iy, (f32) width + sliderRadius, sliderRadius, 5.0f);
    }
    else
    {
        auto ix = (f32) x + (f32) width * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill (ColorGradient::horizontal (gradCol1, ix, gradCol2, ix + sliderRadius));

        indent.addRoundedRectangle (ix, (f32) y - sliderRadius * 0.5f, sliderRadius, (f32) height + sliderRadius, 5.0f);
    }

    g.fillPath (indent);

    g.setColor (trackColor.contrasting (0.5f));
    g.strokePath (indent, PathStrokeType (0.5f));
}

z0 LookAndFeel_V3::drawPopupMenuBackground (Graphics& g, [[maybe_unused]] i32 width, [[maybe_unused]] i32 height)
{
    g.fillAll (findColor (PopupMenu::backgroundColorId));

   #if ! DRX_MAC
    g.setColor (findColor (PopupMenu::textColorId).withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
   #endif
}

z0 LookAndFeel_V3::drawMenuBarBackground (Graphics& g, i32 width, i32 height,
                                            b8, MenuBarComponent& menuBar)
{
    auto colour = menuBar.findColor (PopupMenu::backgroundColorId);

    Rectangle<i32> r (width, height);

    g.setColor (colour.contrasting (0.15f));
    g.fillRect (r.removeFromTop (1));
    g.fillRect (r.removeFromBottom (1));

    g.setGradientFill (ColorGradient::vertical (colour, 0, colour.darker (0.08f), (f32) height));
    g.fillRect (r);
}

z0 LookAndFeel_V3::drawKeymapChangeButton (Graphics& g, i32 width, i32 height,
                                             Button& button, const Txt& keyDescription)
{
    const Color textColor (button.findColor (0x100ad01 /*KeyMappingEditorComponent::textColorId*/, true));

    if (keyDescription.isNotEmpty())
    {
        if (button.isEnabled())
        {
            g.setColor (textColor.withAlpha (button.isDown() ? 0.4f : (button.isOver() ? 0.2f : 0.1f)));
            g.fillRoundedRectangle (button.getLocalBounds().toFloat(), 4.0f);
            g.drawRoundedRectangle (button.getLocalBounds().toFloat(), 4.0f, 1.0f);
        }

        g.setColor (textColor);
        g.setFont ((f32) height * 0.6f);
        g.drawFittedText (keyDescription, 4, 0, width - 8, height, Justification::centred, 1);
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

        g.setColor (textColor.darker (0.1f).withAlpha (button.isDown() ? 0.7f : (button.isOver() ? 0.5f : 0.3f)));
        g.fillPath (p, p.getTransformToScaleToFit (2.0f, 2.0f, (f32) width - 4.0f, (f32) height - 4.0f, true));
    }

    if (button.hasKeyboardFocus (false))
    {
        g.setColor (textColor.withAlpha (0.4f));
        g.drawRect (0, 0, width, height);
    }
}


class LookAndFeel_V3_DocumentWindowButton final : public Button
{
public:
    LookAndFeel_V3_DocumentWindowButton (const Txt& name, Color c, const Path& normal, const Path& toggled)
        : Button (name), colour (c), normalShape (normal), toggledShape (toggled)
    {
    }

    z0 paintButton (Graphics& g, b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override
    {
        Color background (Colors::grey);

        if (ResizableWindow* rw = findParentComponentOfClass<ResizableWindow>())
            background = rw->getBackgroundColor();

        const f32 cx = (f32) getWidth() * 0.5f, cy = (f32) getHeight() * 0.5f;
        const f32 diam = jmin (cx, cy) * (shouldDrawButtonAsDown ? 0.60f : 0.65f);

        g.setColor (background);
        g.fillEllipse (cx - diam, cy - diam, diam * 2.0f, diam * 2.0f);

        Color c (background.contrasting (colour, 0.6f));

        if (! isEnabled())
            c = c.withAlpha (0.6f);
        else if (shouldDrawButtonAsHighlighted)
            c = c.brighter();

        g.setColor (c);
        g.drawEllipse (cx - diam, cy - diam, diam * 2.0f, diam * 2.0f, diam * 0.2f);

        Path& p = getToggleState() ? toggledShape : normalShape;

        f32 scale = 0.55f;
        g.fillPath (p, p.getTransformToScaleToFit (cx - diam * scale, cy - diam * scale,
                                                   diam * 2.0f * scale, diam * 2.0f * scale, true));
    }

private:
    Color colour;
    Path normalShape, toggledShape;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V3_DocumentWindowButton)
};

Button* LookAndFeel_V3::createDocumentWindowButton (i32 buttonType)
{
    Path shape;
    const f32 crossThickness = 0.25f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment (Line<f32> (0.0f, 0.0f, 1.0f, 1.0f), crossThickness * 1.4f);
        shape.addLineSegment (Line<f32> (1.0f, 0.0f, 0.0f, 1.0f), crossThickness * 1.4f);

        return new LookAndFeel_V3_DocumentWindowButton ("close", Color (0xffdd1100), shape, shape);
    }

    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment (Line<f32> (0.0f, 0.5f, 1.0f, 0.5f), crossThickness);

        return new LookAndFeel_V3_DocumentWindowButton ("minimise", Color (0xffaa8811), shape, shape);
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

        return new LookAndFeel_V3_DocumentWindowButton ("maximise", Color (0xff119911), shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}

Path LookAndFeel_V3::getTickShape (const f32 height)
{
    static u8k pathData[]
        = { 110,109,32,210,202,64,126,183,148,64,108,39,244,247,64,245,76,124,64,108,178,131,27,65,246,76,252,64,108,175,242,4,65,246,76,252,
            64,108,236,5,68,65,0,0,160,180,108,240,150,90,65,21,136,52,63,108,48,59,16,65,0,0,32,65,108,32,210,202,64,126,183,148,64, 99,101,0,0 };

    Path p;
    p.loadPathFromData (pathData, sizeof (pathData));
    p.scaleToFit (0, 0, height * 2.0f, height, true);
    return p;
}

Path LookAndFeel_V3::getCrossShape (const f32 height)
{
    static u8k pathData[]
        = { 110,109,88,57,198,65,29,90,171,65,108,63,53,154,65,8,172,126,65,108,76,55,198,65,215,163,38,65,108,141,151,175,65,82,184,242,64,108,117,147,131,65,90,100,81,65,108,184,30,47,65,82,184,242,64,108,59,223,1,65,215,163,38,65,108,84,227,89,65,8,172,126,65,
            108,35,219,1,65,29,90,171,65,108,209,34,47,65,231,251,193,65,108,117,147,131,65,207,247,149,65,108,129,149,175,65,231,251,193,65,99,101,0,0 };

    Path p;
    p.loadPathFromData (pathData, sizeof (pathData));
    p.scaleToFit (0, 0, height * 2.0f, height, true);
    return p;
}

} // namespace drx
