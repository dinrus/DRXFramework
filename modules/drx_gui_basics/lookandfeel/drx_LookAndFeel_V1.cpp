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

LookAndFeel_V1::LookAndFeel_V1()
{
    setColor (TextButton::buttonColorId,          Color (0xffbbbbff));
    setColor (ListBox::outlineColorId,            findColor (ComboBox::outlineColorId));
    setColor (ScrollBar::thumbColorId,            Color (0xffbbbbdd));
    setColor (ScrollBar::backgroundColorId,       Colors::transparentBlack);
    setColor (Slider::thumbColorId,               Colors::white);
    setColor (Slider::trackColorId,               Color (0x7f000000));
    setColor (Slider::textBoxOutlineColorId,      Colors::grey);
    setColor (ProgressBar::backgroundColorId,     Colors::white.withAlpha (0.6f));
    setColor (ProgressBar::foregroundColorId,     Colors::green.withAlpha (0.7f));
    setColor (PopupMenu::backgroundColorId,             Color (0xffeef5f8));
    setColor (PopupMenu::highlightedBackgroundColorId,  Color (0xbfa4c2ce));
    setColor (PopupMenu::highlightedTextColorId,        Colors::black);
    setColor (TextEditor::focusedOutlineColorId,  findColor (TextButton::buttonColorId));

    scrollbarShadow.setShadowProperties (DropShadow (Colors::black.withAlpha (0.5f), 2, Point<i32>()));
}

LookAndFeel_V1::~LookAndFeel_V1()
{
}

//==============================================================================
z0 LookAndFeel_V1::drawButtonBackground (Graphics& g, Button& button, const Color& backgroundColor,
                                           b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown)
{
    i32k width = button.getWidth();
    i32k height = button.getHeight();

    const f32 indent = 2.0f;
    i32k cornerSize = jmin (roundToInt ((f32) width * 0.4f),
                                 roundToInt ((f32) height * 0.4f));

    Path p;
    p.addRoundedRectangle (indent, indent,
                           (f32) width - indent * 2.0f,
                           (f32) height - indent * 2.0f,
                           (f32) cornerSize);

    Color bc (backgroundColor.withMultipliedSaturation (0.3f));

    if (shouldDrawButtonAsHighlighted)
    {
        if (shouldDrawButtonAsDown)
            bc = bc.brighter();
        else if (bc.getBrightness() > 0.5f)
            bc = bc.darker (0.1f);
        else
            bc = bc.brighter (0.1f);
    }

    g.setColor (bc);
    g.fillPath (p);

    g.setColor (bc.contrasting().withAlpha ((shouldDrawButtonAsHighlighted) ? 0.6f : 0.4f));
    g.strokePath (p, PathStrokeType ((shouldDrawButtonAsHighlighted) ? 2.0f : 1.4f));
}

z0 LookAndFeel_V1::drawTickBox (Graphics& g, Component& /*component*/,
                                  f32 x, f32 y, f32 w, f32 h,
                                  const b8 ticked,
                                  const b8 isEnabled,
                                  const b8 /*shouldDrawButtonAsHighlighted*/,
                                  const b8 shouldDrawButtonAsDown)
{
    Path box;
    box.addRoundedRectangle (0.0f, 2.0f, 6.0f, 6.0f, 1.0f);

    g.setColor (isEnabled ? Colors::blue.withAlpha (shouldDrawButtonAsDown ? 0.3f : 0.1f)
                           : Colors::lightgrey.withAlpha (0.1f));

    AffineTransform trans (AffineTransform::scale (w / 9.0f, h / 9.0f).translated (x, y));

    g.fillPath (box, trans);

    g.setColor (Colors::black.withAlpha (0.6f));
    g.strokePath (box, PathStrokeType (0.9f), trans);

    if (ticked)
    {
        Path tick;
        tick.startNewSubPath (1.5f, 3.0f);
        tick.lineTo (3.0f, 6.0f);
        tick.lineTo (6.0f, 0.0f);

        g.setColor (isEnabled ? Colors::black : Colors::grey);
        g.strokePath (tick, PathStrokeType (2.5f), trans);
    }
}

z0 LookAndFeel_V1::drawToggleButton (Graphics& g, ToggleButton& button, b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown)
{
    if (button.hasKeyboardFocus (true))
    {
        g.setColor (button.findColor (TextEditor::focusedOutlineColorId));
        g.drawRect (0, 0, button.getWidth(), button.getHeight());
    }

    i32k tickWidth = jmin (20, button.getHeight() - 4);

    drawTickBox (g, button, 4.0f, (f32) (button.getHeight() - tickWidth) * 0.5f,
                 (f32) tickWidth, (f32) tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 shouldDrawButtonAsHighlighted,
                 shouldDrawButtonAsDown);

    g.setColor (button.findColor (ToggleButton::textColorId));
    g.setFont (jmin (15.0f, (f32) button.getHeight() * 0.6f));

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    i32k textX = tickWidth + 5;

    g.drawFittedText (button.getButtonText(),
                      textX, 4,
                      button.getWidth() - textX - 2, button.getHeight() - 8,
                      Justification::centredLeft, 10);
}

z0 LookAndFeel_V1::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                      i32 width, i32 height,
                                      f64 progress, const Txt& textToShow)
{
    if (progress < 0 || progress >= 1.0)
    {
        LookAndFeel_V2::drawProgressBar (g, progressBar, width, height, progress, textToShow);
    }
    else
    {
        const Color background (progressBar.findColor (ProgressBar::backgroundColorId));
        const Color foreground (progressBar.findColor (ProgressBar::foregroundColorId));

        g.fillAll (background);
        g.setColor (foreground);

        g.fillRect (1, 1,
                    jlimit (0, width - 2, roundToInt (progress * (width - 2))),
                    height - 2);

        if (textToShow.isNotEmpty())
        {
            g.setColor (Color::contrasting (background, foreground));
            g.setFont ((f32) height * 0.6f);

            g.drawText (textToShow, 0, 0, width, height, Justification::centred, false);
        }
    }
}

z0 LookAndFeel_V1::drawScrollbarButton (Graphics& g, ScrollBar& bar,
                                          i32 width, i32 height, i32 buttonDirection,
                                          b8 isScrollbarVertical,
                                          b8 shouldDrawButtonAsHighlighted,
                                          b8 shouldDrawButtonAsDown)
{
    if (isScrollbarVertical)
        width -= 2;
    else
        height -= 2;

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
        g.setColor (Colors::white);
    else if (shouldDrawButtonAsHighlighted)
        g.setColor (Colors::white.withAlpha (0.7f));
    else
        g.setColor (bar.findColor (ScrollBar::thumbColorId).withAlpha (0.5f));

    g.fillPath (p);

    g.setColor (Colors::black.withAlpha (0.5f));
    g.strokePath (p, PathStrokeType (0.5f));
}

z0 LookAndFeel_V1::drawScrollbar (Graphics& g, ScrollBar& bar,
                                    i32 x, i32 y, i32 width, i32 height,
                                    b8 isScrollbarVertical, i32 thumbStartPosition, i32 thumbSize,
                                    b8 isMouseOver, b8 isMouseDown)
{
    g.fillAll (bar.findColor (ScrollBar::backgroundColorId));

    g.setColor (bar.findColor (ScrollBar::thumbColorId)
                    .withAlpha ((isMouseOver || isMouseDown) ? 0.4f : 0.15f));

    if ((f32) thumbSize > 0.0f)
    {
        Rectangle<i32> thumb;

        if (isScrollbarVertical)
        {
            width -= 2;
            g.fillRect (x + roundToInt ((f32) width * 0.35f), y,
                        roundToInt ((f32) width * 0.3f), height);

            thumb.setBounds (x + 1, thumbStartPosition,
                             width - 2, thumbSize);
        }
        else
        {
            height -= 2;
            g.fillRect (x, y + roundToInt ((f32) height * 0.35f),
                        width, roundToInt ((f32) height * 0.3f));

            thumb.setBounds (thumbStartPosition, y + 1,
                             thumbSize, height - 2);
        }

        g.setColor (bar.findColor (ScrollBar::thumbColorId)
                        .withAlpha ((isMouseOver || isMouseDown) ? 0.95f : 0.7f));

        g.fillRect (thumb);

        g.setColor (Colors::black.withAlpha ((isMouseOver || isMouseDown) ? 0.4f : 0.25f));
        g.drawRect (thumb.getX(), thumb.getY(), thumb.getWidth(), thumb.getHeight());

        if (thumbSize > 16)
        {
            for (i32 i = 3; --i >= 0;)
            {
                const f32 linePos = (f32) thumbStartPosition + (f32) thumbSize * 0.5f + (f32) (i - 1) * 4.0f;
                g.setColor (Colors::black.withAlpha (0.15f));

                if (isScrollbarVertical)
                {
                    g.drawLine ((f32) x + (f32) width * 0.2f, linePos, (f32) width * 0.8f, linePos);
                    g.setColor (Colors::white.withAlpha (0.15f));
                    g.drawLine ((f32) width * 0.2f, linePos - 1.0f, (f32) width * 0.8f, linePos - 1.0f);
                }
                else
                {
                    g.drawLine (linePos, (f32) height * 0.2f, linePos, (f32) height * 0.8f);
                    g.setColor (Colors::white.withAlpha (0.15f));
                    g.drawLine (linePos - 1.0f, (f32) height * 0.2f, linePos - 1.0f, (f32) height * 0.8f);
                }
            }
        }
    }
}

ImageEffectFilter* LookAndFeel_V1::getScrollbarEffect()
{
    return &scrollbarShadow;
}


//==============================================================================
z0 LookAndFeel_V1::drawPopupMenuBackground (Graphics& g, i32 width, i32 height)
{
    g.fillAll (findColor (PopupMenu::backgroundColorId));

    g.setColor (Colors::black.withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
}

z0 LookAndFeel_V1::drawMenuBarBackground (Graphics& g, i32 /*width*/, i32 /*height*/, b8, MenuBarComponent& menuBar)
{
    g.fillAll (menuBar.findColor (PopupMenu::backgroundColorId));
}


//==============================================================================
z0 LookAndFeel_V1::drawTextEditorOutline (Graphics& g, i32 width, i32 height, TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        g.setColor (textEditor.findColor (TextEditor::outlineColorId));
        g.drawRect (0, 0, width, height);
    }
}

//==============================================================================
z0 LookAndFeel_V1::drawComboBox (Graphics& g, i32 width, i32 height,
                                   const b8 isButtonDown,
                                   i32 buttonX, i32 buttonY, i32 buttonW, i32 buttonH,
                                   ComboBox& box)
{
    g.fillAll (box.findColor (ComboBox::backgroundColorId));

    g.setColor (box.findColor ((isButtonDown) ? ComboBox::buttonColorId
                                                : ComboBox::backgroundColorId));
    g.fillRect (buttonX, buttonY, buttonW, buttonH);

    g.setColor (box.findColor (ComboBox::outlineColorId));
    g.drawRect (0, 0, width, height);

    const f32 arrowX = 0.2f;
    const f32 arrowH = 0.3f;

    const auto x = (f32) buttonX;
    const auto y = (f32) buttonY;
    const auto w = (f32) buttonW;
    const auto h = (f32) buttonH;

    if (box.isEnabled())
    {
        Path p;
        p.addTriangle (x + w * 0.5f,            y + h * (0.45f - arrowH),
                       x + w * (1.0f - arrowX), y + h * 0.45f,
                       x + w * arrowX,          y + h * 0.45f);

        p.addTriangle (x + w * 0.5f,            y + h * (0.55f + arrowH),
                       x + w * (1.0f - arrowX), y + h * 0.55f,
                       x + w * arrowX,          y + h * 0.55f);

        g.setColor (box.findColor ((isButtonDown) ? ComboBox::backgroundColorId
                                                    : ComboBox::buttonColorId));
        g.fillPath (p);
    }
}

Font LookAndFeel_V1::getComboBoxFont (ComboBox& box)
{
    Font f (withDefaultMetrics (FontOptions { jmin (15.0f, (f32) box.getHeight() * 0.85f) }));
    f.setHorizontalScale (0.9f);
    return f;
}

//==============================================================================
static z0 drawTriangle (Graphics& g, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, Color fill, Color outline)
{
    Path p;
    p.addTriangle (x1, y1, x2, y2, x3, y3);
    g.setColor (fill);
    g.fillPath (p);

    g.setColor (outline);
    g.strokePath (p, PathStrokeType (0.3f));
}

z0 LookAndFeel_V1::drawLinearSlider (Graphics& g,
                                       i32 x, i32 y, i32 w, i32 h,
                                       f32 sliderPos, f32 minSliderPos, f32 maxSliderPos,
                                       const Slider::SliderStyle style,
                                       Slider& slider)
{
    g.fillAll (slider.findColor (Slider::backgroundColorId));

    if (style == Slider::LinearBar)
    {
        g.setColor (slider.findColor (Slider::thumbColorId));
        g.fillRect (x, y, (i32) sliderPos - x, h);

        g.setColor (slider.findColor (Slider::textBoxTextColorId).withMultipliedAlpha (0.5f));
        g.drawRect (x, y, (i32) sliderPos - x, h);
    }
    else
    {
        g.setColor (slider.findColor (Slider::trackColorId)
                           .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.3f));

        if (slider.isHorizontal())
        {
            g.fillRect (x, y + roundToInt ((f32) h * 0.6f),
                        w, roundToInt ((f32) h * 0.2f));
        }
        else
        {
            g.fillRect (x + roundToInt ((f32) w * 0.5f - jmin (3.0f, (f32) w * 0.1f)), y,
                        jmin (4, roundToInt ((f32) w * 0.2f)), h);
        }

        f32 alpha = 0.35f;

        if (slider.isEnabled())
            alpha = slider.isMouseOverOrDragging() ? 1.0f : 0.7f;

        const Color fill (slider.findColor (Slider::thumbColorId).withAlpha (alpha));
        const Color outline (Colors::black.withAlpha (slider.isEnabled() ? 0.7f : 0.35f));

        if (style == Slider::TwoValueVertical || style == Slider::ThreeValueVertical)
        {
            drawTriangle (g,
                          (f32) x + (f32) w * 0.5f + jmin (4.0f, (f32) w * 0.3f), minSliderPos,
                          (f32) x + (f32) w * 0.5f - jmin (8.0f, (f32) w * 0.4f), minSliderPos - 7.0f,
                          (f32) x + (f32) w * 0.5f - jmin (8.0f, (f32) w * 0.4f), minSliderPos,
                          fill, outline);

            drawTriangle (g,
                          (f32) x + (f32) w * 0.5f + jmin (4.0f, (f32) w * 0.3f), maxSliderPos,
                          (f32) x + (f32) w * 0.5f - jmin (8.0f, (f32) w * 0.4f), maxSliderPos,
                          (f32) x + (f32) w * 0.5f - jmin (8.0f, (f32) w * 0.4f), maxSliderPos + 7.0f,
                          fill, outline);
        }
        else if (style == Slider::TwoValueHorizontal || style == Slider::ThreeValueHorizontal)
        {
            drawTriangle (g,
                          minSliderPos, (f32) y + (f32) h * 0.6f - jmin (4.0f, (f32) h * 0.3f),
                          minSliderPos - 7.0f, (f32) y + (f32) h * 0.9f,
                          minSliderPos, (f32) y + (f32) h * 0.9f,
                          fill, outline);

            drawTriangle (g,
                          maxSliderPos, (f32) y + (f32) h * 0.6f - jmin (4.0f, (f32) h * 0.3f),
                          maxSliderPos, (f32) y + (f32) h * 0.9f,
                          maxSliderPos + 7.0f, (f32) y + (f32) h * 0.9f,
                          fill, outline);
        }

        if (style == Slider::LinearHorizontal || style == Slider::ThreeValueHorizontal)
        {
            drawTriangle (g,
                          sliderPos, (f32) y + (f32) h * 0.9f,
                          sliderPos - 7.0f, (f32) y + (f32) h * 0.2f,
                          sliderPos + 7.0f, (f32) y + (f32) h * 0.2f,
                          fill, outline);
        }
        else if (style == Slider::LinearVertical || style == Slider::ThreeValueVertical)
        {
            drawTriangle (g,
                          (f32) x + (f32) w * 0.5f - jmin (4.0f, (f32) w * 0.3f), sliderPos,
                          (f32) x + (f32) w * 0.5f + jmin (8.0f, (f32) w * 0.4f), sliderPos - 7.0f,
                          (f32) x + (f32) w * 0.5f + jmin (8.0f, (f32) w * 0.4f), sliderPos + 7.0f,
                          fill, outline);
        }
    }

    if (slider.isBar())
        drawLinearSliderOutline (g, x, y, w, h, style, slider);
}

Button* LookAndFeel_V1::createSliderButton (Slider&, const b8 isIncrement)
{
    if (isIncrement)
        return new ArrowButton ("u", 0.75f, Colors::white.withAlpha (0.8f));

    return new ArrowButton ("d", 0.25f, Colors::white.withAlpha (0.8f));
}

ImageEffectFilter* LookAndFeel_V1::getSliderEffect (Slider&)
{
    return &scrollbarShadow;
}

i32 LookAndFeel_V1::getSliderThumbRadius (Slider&)
{
    return 8;
}

//==============================================================================
z0 LookAndFeel_V1::drawCornerResizer (Graphics& g, i32 w, i32 h, b8 isMouseOver, b8 isMouseDragging)
{
    g.setColor ((isMouseOver || isMouseDragging) ? Colors::lightgrey
                                                  : Colors::darkgrey);

    const f32 lineThickness = (f32) jmin (w, h) * 0.1f;

    for (f32 i = 0.0f; i < 1.0f; i += 0.3f)
    {
        g.drawLine ((f32) w * i,
                    (f32) h + 1.0f,
                    (f32) w + 1.0f,
                    (f32) h * i,
                    lineThickness);
    }
}

//==============================================================================
Button* LookAndFeel_V1::createDocumentWindowButton (i32 buttonType)
{
    Path shape;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment (Line<f32> (0.0f, 0.0f, 1.0f, 1.0f), 0.35f);
        shape.addLineSegment (Line<f32> (1.0f, 0.0f, 0.0f, 1.0f), 0.35f);

        ShapeButton* const b = new ShapeButton ("close",
                                                Color (0x7fff3333),
                                                Color (0xd7ff3333),
                                                Color (0xf7ff3333));

        b->setShape (shape, true, true, true);
        return b;
    }
    else if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment (Line<f32> (0.0f, 0.5f, 1.0f, 0.5f), 0.25f);

        DrawableButton* b = new DrawableButton ("minimise", DrawableButton::ImageFitted);
        DrawablePath dp;
        dp.setPath (shape);
        dp.setFill (Colors::black.withAlpha (0.3f));
        b->setImages (&dp);
        return b;
    }
    else if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment (Line<f32> (0.5f, 0.0f, 0.5f, 1.0f), 0.25f);
        shape.addLineSegment (Line<f32> (0.0f, 0.5f, 1.0f, 0.5f), 0.25f);

        DrawableButton* b = new DrawableButton ("maximise", DrawableButton::ImageFitted);
        DrawablePath dp;
        dp.setPath (shape);
        dp.setFill (Colors::black.withAlpha (0.3f));
        b->setImages (&dp);
        return b;
    }

    jassertfalse;
    return nullptr;
}

z0 LookAndFeel_V1::positionDocumentWindowButtons (DocumentWindow&,
                                                    i32 titleBarX, i32 titleBarY, i32 titleBarW, i32 titleBarH,
                                                    Button* minimiseButton,
                                                    Button* maximiseButton,
                                                    Button* closeButton,
                                                    b8 positionTitleBarButtonsOnLeft)
{
    titleBarY += titleBarH / 8;
    titleBarH -= titleBarH / 4;

    i32k buttonW = titleBarH;

    i32 x = positionTitleBarButtonsOnLeft ? titleBarX + 4
                                          : titleBarX + titleBarW - buttonW - 4;

    if (closeButton != nullptr)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW + buttonW / 5
                                           : -(buttonW + buttonW / 5);
    }

    if (positionTitleBarButtonsOnLeft)
        std::swap (minimiseButton, maximiseButton);

    if (maximiseButton != nullptr)
    {
        maximiseButton->setBounds (x, titleBarY - 2, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (minimiseButton != nullptr)
        minimiseButton->setBounds (x, titleBarY - 2, buttonW, titleBarH);
}

} // namespace drx
