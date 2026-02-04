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

Color LookAndFeel_V4::ColorScheme::getUIColor (UIColor index) const noexcept
{
    if (isPositiveAndBelow (index, numColors))
        return palette[index];

    jassertfalse;
    return {};
}

z0 LookAndFeel_V4::ColorScheme::setUIColor (UIColor index, Color newColor) noexcept
{
    if (isPositiveAndBelow (index, numColors))
        palette[index] = newColor;
    else
        jassertfalse;
}

b8 LookAndFeel_V4::ColorScheme::operator== (const ColorScheme& other) const noexcept
{
    for (auto i = 0; i < numColors; ++i)
        if (palette[i] != other.palette[i])
            return false;

    return true;
}

b8 LookAndFeel_V4::ColorScheme::operator!= (const ColorScheme& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
LookAndFeel_V4::LookAndFeel_V4()  : currentColorScheme (getDarkColorScheme())
{
    initialiseColors();
}

LookAndFeel_V4::LookAndFeel_V4 (ColorScheme scheme)  : currentColorScheme (scheme)
{
    initialiseColors();
}

LookAndFeel_V4::~LookAndFeel_V4()  {}

//==============================================================================
z0 LookAndFeel_V4::setColorScheme (ColorScheme newColorScheme)
{
    currentColorScheme = newColorScheme;
    initialiseColors();
}

LookAndFeel_V4::ColorScheme LookAndFeel_V4::getDarkColorScheme()
{
    return { 0xff323e44, 0xff263238, 0xff323e44,
             0xff8e989b, 0xffffffff, 0xff42a2c8,
             0xffffffff, 0xff181f22, 0xffffffff };
}

LookAndFeel_V4::ColorScheme LookAndFeel_V4::getMidnightColorScheme()
{
    return { 0xff2f2f3a, 0xff191926, 0xffd0d0d0,
             0xff66667c, 0xc8ffffff, 0xffd8d8d8,
             0xffffffff, 0xff606073, 0xff000000 };
}

LookAndFeel_V4::ColorScheme LookAndFeel_V4::getGreyColorScheme()
{
    return { 0xff505050, 0xff424242, 0xff606060,
             0xffa6a6a6, 0xffffffff, 0xff21ba90,
             0xff000000, 0xffffffff, 0xffffffff };
}

LookAndFeel_V4::ColorScheme LookAndFeel_V4::getLightColorScheme()
{
    return { 0xffefefef, 0xffffffff, 0xffffffff,
             0xffdddddd, 0xff000000, 0xffa9a9a9,
             0xffffffff, 0xff42a2c8, 0xff000000 };
}

//==============================================================================
class LookAndFeel_V4_DocumentWindowButton final : public Button
{
public:
    LookAndFeel_V4_DocumentWindowButton (const Txt& name, Color c, const Path& normal, const Path& toggled)
        : Button (name), colour (c), normalShape (normal), toggledShape (toggled)
    {
    }

    z0 paintButton (Graphics& g, b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown) override
    {
        auto background = Colors::grey;

        if (auto* rw = findParentComponentOfClass<ResizableWindow>())
            if (auto lf = dynamic_cast<LookAndFeel_V4*> (&rw->getLookAndFeel()))
                background = lf->getCurrentColorScheme().getUIColor (LookAndFeel_V4::ColorScheme::widgetBackground);

        g.fillAll (background);

        g.setColor ((! isEnabled() || shouldDrawButtonAsDown) ? colour.withAlpha (0.6f)
                                                     : colour);

        if (shouldDrawButtonAsHighlighted)
        {
            g.fillAll();
            g.setColor (background);
        }

        auto& p = getToggleState() ? toggledShape : normalShape;

        auto reducedRect = Justification (Justification::centred)
                              .appliedToRectangle (Rectangle<i32> (getHeight(), getHeight()), getLocalBounds())
                              .toFloat()
                              .reduced ((f32) getHeight() * 0.3f);

        g.fillPath (p, p.getTransformToScaleToFit (reducedRect, true));
    }

private:
    Color colour;
    Path normalShape, toggledShape;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V4_DocumentWindowButton)
};

Button* LookAndFeel_V4::createDocumentWindowButton (i32 buttonType)
{
    Path shape;
    auto crossThickness = 0.15f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment ({ 0.0f, 0.0f, 1.0f, 1.0f }, crossThickness);
        shape.addLineSegment ({ 1.0f, 0.0f, 0.0f, 1.0f }, crossThickness);

        return new LookAndFeel_V4_DocumentWindowButton ("close", Color (0xff9A131D), shape, shape);
    }

    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        return new LookAndFeel_V4_DocumentWindowButton ("minimise", Color (0xffaa8811), shape, shape);
    }

    if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment ({ 0.5f, 0.0f, 0.5f, 1.0f }, crossThickness);
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new LookAndFeel_V4_DocumentWindowButton ("maximise", Color (0xff0A830A), shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}

z0 LookAndFeel_V4::positionDocumentWindowButtons (DocumentWindow&,
                                                    i32 titleBarX, i32 titleBarY,
                                                    i32 titleBarW, i32 titleBarH,
                                                    Button* minimiseButton,
                                                    Button* maximiseButton,
                                                    Button* closeButton,
                                                    b8 positionTitleBarButtonsOnLeft)
{
    auto buttonW = static_cast<i32> (titleBarH * 1.2);

    auto x = positionTitleBarButtonsOnLeft ? titleBarX
                                           : titleBarX + titleBarW - buttonW;

    if (closeButton != nullptr)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
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

z0 LookAndFeel_V4::drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g,
                                                 i32 w, i32 h, i32 titleSpaceX, i32 titleSpaceW,
                                                 const Image* icon, b8 drawTitleTextOnLeft)
{
    if (w * h == 0)
        return;

    auto isActive = window.isActiveWindow();

    g.setColor (getCurrentColorScheme().getUIColor (ColorScheme::widgetBackground));
    g.fillAll();

    Font font (withDefaultMetrics (FontOptions { (f32) h * 0.65f, Font::plain }));
    g.setFont (font);

    auto textW = GlyphArrangement::getStringWidthInt (font, window.getName());
    auto iconW = 0;
    auto iconH = 0;

    if (icon != nullptr)
    {
        iconH = static_cast<i32> (font.getHeight());
        iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin (titleSpaceW, textW + iconW);
    auto textX = drawTitleTextOnLeft ? titleSpaceX
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
        g.setColor (getCurrentColorScheme().getUIColor (ColorScheme::defaultText));

    g.drawText (window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}

//==============================================================================
Font LookAndFeel_V4::getTextButtonFont (TextButton&, i32 buttonHeight)
{
    return withDefaultMetrics (FontOptions { jmin (16.0f, (f32) buttonHeight * 0.6f) });
}

z0 LookAndFeel_V4::drawButtonBackground (Graphics& g,
                                           Button& button,
                                           const Color& backgroundColor,
                                           b8 shouldDrawButtonAsHighlighted,
                                           b8 shouldDrawButtonAsDown)
{
    auto cornerSize = 6.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto baseColor = backgroundColor.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                      .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColor = baseColor.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColor (baseColor);

    auto flatOnLeft   = button.isConnectedOnLeft();
    auto flatOnRight  = button.isConnectedOnRight();
    auto flatOnTop    = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), bounds.getHeight(),
                                  cornerSize, cornerSize,
                                  ! (flatOnLeft  || flatOnTop),
                                  ! (flatOnRight || flatOnTop),
                                  ! (flatOnLeft  || flatOnBottom),
                                  ! (flatOnRight || flatOnBottom));

        g.fillPath (path);

        g.setColor (button.findColor (ComboBox::outlineColorId));
        g.strokePath (path, PathStrokeType (1.0f));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);

        g.setColor (button.findColor (ComboBox::outlineColorId));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
}

z0 LookAndFeel_V4::drawToggleButton (Graphics& g, ToggleButton& button,
                                       b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown)
{
    auto fontSize = jmin (15.0f, (f32) button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

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
                      button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10)
                                             .withTrimmedRight (2),
                      Justification::centredLeft, 10);
}

z0 LookAndFeel_V4::drawTickBox (Graphics& g, Component& component,
                                  f32 x, f32 y, f32 w, f32 h,
                                  const b8 ticked,
                                  [[maybe_unused]] const b8 isEnabled,
                                  [[maybe_unused]] const b8 shouldDrawButtonAsHighlighted,
                                  [[maybe_unused]] const b8 shouldDrawButtonAsDown)
{
    Rectangle<f32> tickBounds (x, y, w, h);

    g.setColor (component.findColor (ToggleButton::tickDisabledColorId));
    g.drawRoundedRectangle (tickBounds, 4.0f, 1.0f);

    if (ticked)
    {
        g.setColor (component.findColor (ToggleButton::tickColorId));
        auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
    }
}

z0 LookAndFeel_V4::changeToggleButtonWidthToFitText (ToggleButton& button)
{
    auto fontSize = jmin (15.0f, (f32) button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    Font font (withDefaultMetrics (FontOptions { fontSize }));

    button.setSize (GlyphArrangement::getStringWidthInt (font, button.getButtonText()) + roundToInt (tickWidth) + 14, button.getHeight());
}

//==============================================================================
AlertWindow* LookAndFeel_V4::createAlertWindow (const Txt& title, const Txt& message,
                                                const Txt& button1, const Txt& button2, const Txt& button3,
                                                MessageBoxIconType iconType,
                                                i32 numButtons, Component* associatedComponent)
{
    auto boundsOffset = 50;

    auto* aw = LookAndFeel_V2::createAlertWindow (title, message, button1, button2, button3,
                                                  iconType, numButtons, associatedComponent);

    auto bounds = aw->getBounds();
    bounds = bounds.withSizeKeepingCentre (bounds.getWidth() + boundsOffset, bounds.getHeight() + boundsOffset);
    aw->setBounds (bounds);

    for (auto* child : aw->getChildren())
        if (auto* button = dynamic_cast<TextButton*> (child))
            button->setBounds (button->getBounds() + Point<i32> (25, 40));

    return aw;
}

z0 LookAndFeel_V4::drawAlertBox (Graphics& g, AlertWindow& alert,
                                   const Rectangle<i32>& textArea, TextLayout& textLayout)
{
    auto cornerSize = 4.0f;

    g.setColor (alert.findColor (AlertWindow::outlineColorId));
    g.drawRoundedRectangle (alert.getLocalBounds().toFloat(), cornerSize, 2.0f);

    auto bounds = alert.getLocalBounds().reduced (1);
    g.reduceClipRegion (bounds);

    g.setColor (alert.findColor (AlertWindow::backgroundColorId));
    g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

    auto iconSpaceUsed = 0;

    auto iconWidth = 80;
    auto iconSize = jmin (iconWidth + 50, bounds.getHeight() + 20);

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = jmin (iconSize, textArea.getHeight() + 50);

    Rectangle<i32> iconRect (iconSize / -10, iconSize / -10,
                             iconSize, iconSize);

    if (alert.getAlertType() != MessageBoxIconType::NoIcon)
    {
        Path icon;
        t8 character;
        u32 colour;

        if (alert.getAlertType() == MessageBoxIconType::WarningIcon)
        {
            character = '!';

            icon.addTriangle ((f32) iconRect.getX() + (f32) iconRect.getWidth() * 0.5f, (f32) iconRect.getY(),
                              static_cast<f32> (iconRect.getRight()), static_cast<f32> (iconRect.getBottom()),
                              static_cast<f32> (iconRect.getX()), static_cast<f32> (iconRect.getBottom()));

            icon = icon.createPathWithRoundedCorners (5.0f);
            colour = 0x66ff2a00;
        }
        else
        {
            colour = Color (0xff00b0b9).withAlpha (0.4f).getARGB();
            character = alert.getAlertType() == MessageBoxIconType::InfoIcon ? 'i' : '?';

            icon.addEllipse (iconRect.toFloat());
        }

        GlyphArrangement ga;
        ga.addFittedText (withDefaultMetrics (FontOptions { (f32) iconRect.getHeight() * 0.9f, Font::bold }),
                          Txt::charToString ((t32) (u8) character),
                          static_cast<f32> (iconRect.getX()), static_cast<f32> (iconRect.getY()),
                          static_cast<f32> (iconRect.getWidth()), static_cast<f32> (iconRect.getHeight()),
                          Justification::centred, false);
        ga.createPath (icon);

        icon.setUsingNonZeroWinding (false);
        g.setColor (Color (colour));
        g.fillPath (icon);

        iconSpaceUsed = iconWidth;
    }

    g.setColor (alert.findColor (AlertWindow::textColorId));

    Rectangle<i32> alertBounds (bounds.getX() + iconSpaceUsed, 30,
                                bounds.getWidth(), bounds.getHeight() - getAlertWindowButtonHeight() - 20);

    textLayout.draw (g, alertBounds.toFloat());
}

i32 LookAndFeel_V4::getAlertWindowButtonHeight()    { return 40; }
Font LookAndFeel_V4::getAlertWindowTitleFont()      { return withDefaultMetrics (FontOptions { 18.0f, Font::bold }); }
Font LookAndFeel_V4::getAlertWindowMessageFont()    { return withDefaultMetrics (FontOptions { 16.0f }); }
Font LookAndFeel_V4::getAlertWindowFont()           { return withDefaultMetrics (FontOptions { 14.0f }); }

//==============================================================================
z0 LookAndFeel_V4::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                      i32 width, i32 height, f64 progress,
                                      const Txt& textToShow)
{
    switch (progressBar.getResolvedStyle())
    {
        case ProgressBar::Style::linear:
            drawLinearProgressBar (g, progressBar, width, height, progress, textToShow);
            break;

        case ProgressBar::Style::circular:
            drawCircularProgressBar (g, progressBar, textToShow);
            break;
    }
}

ProgressBar::Style LookAndFeel_V4::getDefaultProgressBarStyle (const ProgressBar& progressBar)
{
    return progressBar.getWidth() == progressBar.getHeight() ? ProgressBar::Style::circular
                                                             : ProgressBar::Style::linear;
}

z0 LookAndFeel_V4::drawLinearProgressBar (Graphics& g, const ProgressBar& progressBar,
                                            i32 width, i32 height, f64 progress,
                                            const Txt& textToShow)
{
    auto background = progressBar.findColor (ProgressBar::backgroundColorId);
    auto foreground = progressBar.findColor (ProgressBar::foregroundColorId);

    auto barBounds = progressBar.getLocalBounds().toFloat();

    g.setColor (background);
    g.fillRoundedRectangle (barBounds, (f32) progressBar.getHeight() * 0.5f);

    if (progress >= 0.0f && progress <= 1.0f)
    {
        Path p;
        p.addRoundedRectangle (barBounds, (f32) progressBar.getHeight() * 0.5f);
        g.reduceClipRegion (p);

        barBounds.setWidth (barBounds.getWidth() * (f32) progress);
        g.setColor (foreground);
        g.fillRoundedRectangle (barBounds, (f32) progressBar.getHeight() * 0.5f);
    }
    else
    {
        // spinning bar..
        g.setColor (background);

        auto stripeWidth = height * 2;
        auto position = static_cast<i32> (Time::getMillisecondCounter() / 15) % stripeWidth;

        Path p;

        for (auto x = static_cast<f32> (-position); x < (f32) (width + stripeWidth); x += (f32) stripeWidth)
            p.addQuadrilateral (x, 0.0f,
                                x + (f32) stripeWidth * 0.5f, 0.0f,
                                x, static_cast<f32> (height),
                                x - (f32) stripeWidth * 0.5f, static_cast<f32> (height));

        Image im (Image::ARGB, width, height, true);

        {
            Graphics g2 (im);
            g2.setColor (foreground);
            g2.fillRoundedRectangle (barBounds, (f32) progressBar.getHeight() * 0.5f);
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

z0 LookAndFeel_V4::drawCircularProgressBar (Graphics& g, const ProgressBar& progressBar,
                                              const Txt& textToShow)
{
    const auto background = progressBar.findColor (ProgressBar::backgroundColorId);
    const auto foreground = progressBar.findColor (ProgressBar::foregroundColorId);

    const auto barBounds = progressBar.getLocalBounds().reduced (2, 2).toFloat();
    const auto size = jmin (barBounds.getWidth(), barBounds.getHeight());

    const auto rotationInDegrees  = static_cast<f32> ((Time::getMillisecondCounter() / 10) % 360);
    const auto normalisedRotation = rotationInDegrees / 360.0f;

    constexpr auto rotationOffset = 22.5f;
    constexpr auto maxRotation    = 315.0f;

    auto startInDegrees = rotationInDegrees;
    auto endInDegrees   = startInDegrees + rotationOffset;

    if (normalisedRotation >= 0.25f && normalisedRotation < 0.5f)
    {
        auto rescaledRotation = (normalisedRotation * 4.0f) - 1.0f;
        endInDegrees = startInDegrees + rotationOffset + (maxRotation * rescaledRotation);
    }
    else if (normalisedRotation >= 0.5f && normalisedRotation <= 1.0f)
    {
        endInDegrees = startInDegrees + rotationOffset + maxRotation;
        auto rescaledRotation = 1.0f - ((normalisedRotation * 2.0f) - 1.0f);
        startInDegrees = endInDegrees - rotationOffset - (maxRotation * rescaledRotation);
    }

    g.setColor (background);
    Path arcPath2;
    arcPath2.addCentredArc (barBounds.getCentreX(),
                            barBounds.getCentreY(),
                            size * 0.5f,
                            size * 0.5f, 0.0f,
                            0.0f,
                            MathConstants<f32>::twoPi,
                            true);
    g.strokePath (arcPath2, PathStrokeType (4.0f));

    g.setColor (foreground);
    Path arcPath;
    arcPath.addCentredArc (barBounds.getCentreX(),
                           barBounds.getCentreY(),
                           size * 0.5f,
                           size * 0.5f,
                           0.0f,
                           degreesToRadians (startInDegrees),
                           degreesToRadians (endInDegrees),
                           true);

    arcPath.applyTransform (AffineTransform::rotation (normalisedRotation * MathConstants<f32>::pi * 2.25f, barBounds.getCentreX(), barBounds.getCentreY()));
    g.strokePath (arcPath, PathStrokeType (4.0f));

    if (textToShow.isNotEmpty())
    {
        g.setColor (progressBar.findColor (TextButton::textColorOffId));
        g.setFont (progressBar.withDefaultMetrics (FontOptions { 12.0f, Font::italic }));
        g.drawText (textToShow, barBounds, Justification::centred, false);
    }
}

//==============================================================================
i32 LookAndFeel_V4::getDefaultScrollbarWidth()
{
    return 8;
}

z0 LookAndFeel_V4::drawScrollbar (Graphics& g, ScrollBar& scrollbar, i32 x, i32 y, i32 width, i32 height,
                                    b8 isScrollbarVertical, i32 thumbStartPosition, i32 thumbSize, b8 isMouseOver, [[maybe_unused]] b8 isMouseDown)
{
    Rectangle<i32> thumbBounds;

    if (isScrollbarVertical)
        thumbBounds = { x, thumbStartPosition, width, thumbSize };
    else
        thumbBounds = { thumbStartPosition, y, thumbSize, height };

    auto c = scrollbar.findColor (ScrollBar::ColorIds::thumbColorId);
    g.setColor (isMouseOver ? c.brighter (0.25f) : c);
    g.fillRoundedRectangle (thumbBounds.reduced (1).toFloat(), 4.0f);
}

//==============================================================================
Path LookAndFeel_V4::getTickShape (f32 height)
{
    static u8k pathData[] = { 110,109,32,210,202,64,126,183,148,64,108,39,244,247,64,245,76,124,64,108,178,131,27,65,246,76,252,64,108,175,242,4,65,246,76,252,
        64,108,236,5,68,65,0,0,160,180,108,240,150,90,65,21,136,52,63,108,48,59,16,65,0,0,32,65,108,32,210,202,64,126,183,148,64, 99,101,0,0 };

    Path path;
    path.loadPathFromData (pathData, sizeof (pathData));
    path.scaleToFit (0, 0, height * 2.0f, height, true);

    return path;
}

Path LookAndFeel_V4::getCrossShape (f32 height)
{
    static u8k pathData[] = { 110,109,51,51,255,66,0,0,0,0,108,205,204,13,67,51,51,99,65,108,0,0,170,66,205,204,141,66,108,51,179,13,67,52,51,255,66,108,0,0,255,
        66,205,204,13,67,108,205,204,141,66,0,0,170,66,108,52,51,99,65,51,179,13,67,108,0,0,0,0,51,51,255,66,108,205,204,98,66, 204,204,141,66,108,0,0,0,0,51,51,99,65,108,51,51,
        99,65,0,0,0,0,108,205,204,141,66,205,204,98,66,108,51,51,255,66,0,0,0,0,99,101,0,0 };

    Path path;
    path.loadPathFromData (pathData, sizeof (pathData));
    path.scaleToFit (0, 0, height * 2.0f, height, true);

    return path;
}

//==============================================================================
z0 LookAndFeel_V4::fillTextEditorBackground (Graphics& g, i32 width, i32 height, TextEditor& textEditor)
{
    if (dynamic_cast<AlertWindow*> (textEditor.getParentComponent()) != nullptr)
    {
        g.setColor (textEditor.findColor (TextEditor::backgroundColorId));
        g.fillRect (0, 0, width, height);

        g.setColor (textEditor.findColor (TextEditor::outlineColorId));
        g.drawHorizontalLine (height - 1, 0.0f, static_cast<f32> (width));
    }
    else
    {
        LookAndFeel_V2::fillTextEditorBackground (g, width, height, textEditor);
    }
}

z0 LookAndFeel_V4::drawTextEditorOutline (Graphics& g, i32 width, i32 height, TextEditor& textEditor)
{
    if (dynamic_cast<AlertWindow*> (textEditor.getParentComponent()) == nullptr)
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
}

//==============================================================================
Button* LookAndFeel_V4::createFileBrowserGoUpButton()
{
    auto* goUpButton = new DrawableButton ("up", DrawableButton::ImageOnButtonBackground);

    Path arrowPath;
    arrowPath.addArrow ({ 50.0f, 100.0f, 50.0f, 0.0f }, 40.0f, 100.0f, 50.0f);

    DrawablePath arrowImage;
    arrowImage.setFill (goUpButton->findColor (TextButton::textColorOffId));
    arrowImage.setPath (arrowPath);

    goUpButton->setImages (&arrowImage);

    return goUpButton;
}

z0 LookAndFeel_V4::layoutFileBrowserComponent (FileBrowserComponent& browserComp,
                                                 DirectoryContentsDisplayComponent* fileListComponent,
                                                 FilePreviewComponent* previewComp,
                                                 ComboBox* currentPathBox,
                                                 TextEditor* filenameBox,
                                                 Button* goUpButton)
{
    auto sectionHeight = 22;
    auto buttonWidth = 50;

    auto b = browserComp.getLocalBounds().reduced (20, 5);

    auto topSlice    = b.removeFromTop (sectionHeight);
    auto bottomSlice = b.removeFromBottom (sectionHeight);

    currentPathBox->setBounds (topSlice.removeFromLeft (topSlice.getWidth() - buttonWidth));

    topSlice.removeFromLeft (6);
    goUpButton->setBounds (topSlice);

    bottomSlice.removeFromLeft (20);
    filenameBox->setBounds (bottomSlice);

    if (previewComp != nullptr)
        previewComp->setBounds (b.removeFromRight (b.getWidth() / 3));

    if (auto* listAsComp = dynamic_cast<Component*> (fileListComponent))
        listAsComp->setBounds (b.reduced (0, 10));
}

z0 LookAndFeel_V4::drawFileBrowserRow (Graphics& g, i32 width, i32 height,
                                         const File& file, const Txt& filename, Image* icon,
                                         const Txt& fileSizeDescription,
                                         const Txt& fileTimeDescription,
                                         b8 isDirectory, b8 isItemSelected,
                                         i32 itemIndex, DirectoryContentsDisplayComponent& dcc)
{
    LookAndFeel_V2::drawFileBrowserRow (g, width, height, file, filename, icon,
                                        fileSizeDescription, fileTimeDescription,
                                        isDirectory, isItemSelected, itemIndex, dcc);
}

//==============================================================================
z0 LookAndFeel_V4::drawPopupMenuItem (Graphics& g, const Rectangle<i32>& area,
                                        const b8 isSeparator, const b8 isActive,
                                        const b8 isHighlighted, const b8 isTicked,
                                        const b8 hasSubMenu, const Txt& text,
                                        const Txt& shortcutKeyText,
                                        const Drawable* icon, const Color* const textColorToUse)
{
    if (isSeparator)
    {
        auto r  = area.reduced (5, 0);
        r.removeFromTop (roundToInt (((f32) r.getHeight() * 0.5f) - 0.5f));

        g.setColor (findColor (PopupMenu::textColorId).withAlpha (0.3f));
        g.fillRect (r.removeFromTop (1));
    }
    else
    {
        auto textColor = (textColorToUse == nullptr ? findColor (PopupMenu::textColorId)
                                                      : *textColorToUse);

        auto r  = area.reduced (1);

        if (isHighlighted && isActive)
        {
            g.setColor (findColor (PopupMenu::highlightedBackgroundColorId));
            g.fillRect (r);

            g.setColor (findColor (PopupMenu::highlightedTextColorId));
        }
        else
        {
            g.setColor (textColor.withMultipliedAlpha (isActive ? 1.0f : 0.5f));
        }

        r.reduce (jmin (5, area.getWidth() / 20), 0);

        auto font = getPopupMenuFont();

        auto maxFontHeight = (f32) r.getHeight() / 1.3f;

        if (font.getHeight() > maxFontHeight)
            font.setHeight (maxFontHeight);

        g.setFont (font);

        auto iconArea = r.removeFromLeft (roundToInt (maxFontHeight)).toFloat();

        if (icon != nullptr)
        {
            icon->drawWithin (g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
            r.removeFromLeft (roundToInt (maxFontHeight * 0.5f));
        }
        else if (isTicked)
        {
            auto tick = getTickShape (1.0f);
            g.fillPath (tick, tick.getTransformToScaleToFit (iconArea.reduced (iconArea.getWidth() / 5, 0).toFloat(), true));
        }

        if (hasSubMenu)
        {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();

            auto x = static_cast<f32> (r.removeFromRight ((i32) arrowH).getX());
            auto halfH = static_cast<f32> (r.getCentreY());

            Path path;
            path.startNewSubPath (x, halfH - arrowH * 0.5f);
            path.lineTo (x + arrowH * 0.6f, halfH);
            path.lineTo (x, halfH + arrowH * 0.5f);

            g.strokePath (path, PathStrokeType (2.0f));
        }

        r.removeFromRight (3);
        g.drawFittedText (text, r, Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            auto f2 = font;
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setFont (f2);

            g.drawText (shortcutKeyText, r, Justification::centredRight, true);
        }
    }
}

z0 LookAndFeel_V4::getIdealPopupMenuItemSize (const Txt& text, const b8 isSeparator,
                                                i32 standardMenuItemHeight, i32& idealWidth, i32& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight / 10 : 10;
    }
    else
    {
        auto font = getPopupMenuFont();

        if (standardMenuItemHeight > 0 && font.getHeight() > (f32) standardMenuItemHeight / 1.3f)
            font.setHeight ((f32) standardMenuItemHeight / 1.3f);

        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : roundToInt (font.getHeight() * 1.3f);
        idealWidth = GlyphArrangement::getStringWidthInt (font, text) + idealHeight * 2;
    }
}

z0 LookAndFeel_V4::drawMenuBarBackground (Graphics& g, i32 width, i32 height,
                                            b8, MenuBarComponent& menuBar)
{
    auto colour = menuBar.findColor (TextButton::buttonColorId).withAlpha (0.4f);

    Rectangle<i32> r (width, height);

    g.setColor (colour.contrasting (0.15f));
    g.fillRect  (r.removeFromTop (1));
    g.fillRect  (r.removeFromBottom (1));

    g.setGradientFill (ColorGradient::vertical (colour, 0, colour.darker (0.2f), (f32) height));
    g.fillRect (r);
}

z0 LookAndFeel_V4::drawMenuBarItem (Graphics& g, i32 width, i32 height,
                                      i32 itemIndex, const Txt& itemText,
                                      b8 isMouseOverItem, b8 isMenuOpen,
                                      b8 /*isMouseOverBar*/, MenuBarComponent& menuBar)
{
    if (! menuBar.isEnabled())
    {
        g.setColor (menuBar.findColor (TextButton::textColorOffId)
                            .withMultipliedAlpha (0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll   (menuBar.findColor (TextButton::buttonOnColorId));
        g.setColor (menuBar.findColor (TextButton::textColorOnId));
    }
    else
    {
        g.setColor (menuBar.findColor (TextButton::textColorOffId));
    }

    g.setFont (getMenuBarFont (menuBar, itemIndex, itemText));
    g.drawFittedText (itemText, 0, 0, width, height, Justification::centred, 1);
}

//==============================================================================
z0 LookAndFeel_V4::drawComboBox (Graphics& g, i32 width, i32 height, b8,
                                   i32, i32, i32, i32, ComboBox& box)
{
    auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
    Rectangle<i32> boxBounds (0, 0, width, height);

    g.setColor (box.findColor (ComboBox::backgroundColorId));
    g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

    g.setColor (box.findColor (ComboBox::outlineColorId));
    g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

    Rectangle<i32> arrowZone (width - 30, 0, 20, height);
    Path path;
    path.startNewSubPath ((f32) arrowZone.getX() + 3.0f, (f32) arrowZone.getCentreY() - 2.0f);
    path.lineTo ((f32) arrowZone.getCentreX(), (f32) arrowZone.getCentreY() + 3.0f);
    path.lineTo ((f32) arrowZone.getRight() - 3.0f, (f32) arrowZone.getCentreY() - 2.0f);

    g.setColor (box.findColor (ComboBox::arrowColorId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
    g.strokePath (path, PathStrokeType (2.0f));
}

Font LookAndFeel_V4::getComboBoxFont (ComboBox& box)
{
    return withDefaultMetrics (FontOptions { jmin (16.0f, (f32) box.getHeight() * 0.85f) });
}

z0 LookAndFeel_V4::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (1, 1,
                     box.getWidth() - 30,
                     box.getHeight() - 2);

    label.setFont (getComboBoxFont (box));
}

//==============================================================================
i32 LookAndFeel_V4::getSliderThumbRadius (Slider& slider)
{
    return jmin (12, slider.isHorizontal() ? static_cast<i32> ((f32) slider.getHeight() * 0.5f)
                                           : static_cast<i32> ((f32) slider.getWidth()  * 0.5f));
}

z0 LookAndFeel_V4::drawLinearSlider (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                       f32 sliderPos,
                                       f32 minSliderPos,
                                       f32 maxSliderPos,
                                       const Slider::SliderStyle style, Slider& slider)
{
    if (slider.isBar())
    {
        g.setColor (slider.findColor (Slider::trackColorId));
        g.fillRect (slider.isHorizontal() ? Rectangle<f32> (static_cast<f32> (x), (f32) y + 0.5f, sliderPos - (f32) x, (f32) height - 1.0f)
                                          : Rectangle<f32> ((f32) x + 0.5f, sliderPos, (f32) width - 1.0f, (f32) y + ((f32) height - sliderPos)));

        drawLinearSliderOutline (g, x, y, width, height, style, slider);
    }
    else
    {
        auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

        auto trackWidth = jmin (6.0f, slider.isHorizontal() ? (f32) height * 0.25f : (f32) width * 0.25f);

        Point<f32> startPoint (slider.isHorizontal() ? (f32) x : (f32) x + (f32) width * 0.5f,
                                 slider.isHorizontal() ? (f32) y + (f32) height * 0.5f : (f32) (height + y));

        Point<f32> endPoint (slider.isHorizontal() ? (f32) (width + x) : startPoint.x,
                               slider.isHorizontal() ? startPoint.y : (f32) y);

        Path backgroundTrack;
        backgroundTrack.startNewSubPath (startPoint);
        backgroundTrack.lineTo (endPoint);
        g.setColor (slider.findColor (Slider::backgroundColorId));
        g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        Path valueTrack;
        Point<f32> minPoint, maxPoint, thumbPoint;

        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : (f32) width * 0.5f,
                         slider.isHorizontal() ? (f32) height * 0.5f : minSliderPos };

            if (isThreeVal)
                thumbPoint = { slider.isHorizontal() ? sliderPos : (f32) width * 0.5f,
                               slider.isHorizontal() ? (f32) height * 0.5f : sliderPos };

            maxPoint = { slider.isHorizontal() ? maxSliderPos : (f32) width * 0.5f,
                         slider.isHorizontal() ? (f32) height * 0.5f : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : ((f32) x + (f32) width * 0.5f);
            auto ky = slider.isHorizontal() ? ((f32) y + (f32) height * 0.5f) : sliderPos;

            minPoint = startPoint;
            maxPoint = { kx, ky };
        }

        auto thumbWidth = getSliderThumbRadius (slider);

        valueTrack.startNewSubPath (minPoint);
        valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
        g.setColor (slider.findColor (Slider::trackColorId));
        g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        if (! isTwoVal)
        {
            g.setColor (slider.findColor (Slider::thumbColorId));
            g.fillEllipse (Rectangle<f32> (static_cast<f32> (thumbWidth), static_cast<f32> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
        }

        if (isTwoVal || isThreeVal)
        {
            auto sr = jmin (trackWidth, (slider.isHorizontal() ? (f32) height : (f32) width) * 0.4f);
            auto pointerColor = slider.findColor (Slider::thumbColorId);

            if (slider.isHorizontal())
            {
                drawPointer (g, minSliderPos - sr,
                             jmax (0.0f, (f32) y + (f32) height * 0.5f - trackWidth * 2.0f),
                             trackWidth * 2.0f, pointerColor, 2);

                drawPointer (g, maxSliderPos - trackWidth,
                             jmin ((f32) (y + height) - trackWidth * 2.0f, (f32) y + (f32) height * 0.5f),
                             trackWidth * 2.0f, pointerColor, 4);
            }
            else
            {
                drawPointer (g, jmax (0.0f, (f32) x + (f32) width * 0.5f - trackWidth * 2.0f),
                             minSliderPos - trackWidth,
                             trackWidth * 2.0f, pointerColor, 1);

                drawPointer (g, jmin ((f32) (x + width) - trackWidth * 2.0f, (f32) x + (f32) width * 0.5f), maxSliderPos - sr,
                             trackWidth * 2.0f, pointerColor, 3);
            }
        }

        if (slider.isBar())
            drawLinearSliderOutline (g, x, y, width, height, style, slider);
    }
}

z0 LookAndFeel_V4::drawRotarySlider (Graphics& g, i32 x, i32 y, i32 width, i32 height, f32 sliderPos,
                                       const f32 rotaryStartAngle, const f32 rotaryEndAngle, Slider& slider)
{
    auto outline = slider.findColor (Slider::rotarySliderOutlineColorId);
    auto fill    = slider.findColor (Slider::rotarySliderFillColorId);

    auto bounds = Rectangle<i32> (x, y, width, height).toFloat().reduced (10);

    auto radius = jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin (8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 bounds.getCentreY(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 rotaryStartAngle,
                                 rotaryEndAngle,
                                 true);

    g.setColor (outline);
    g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));

    if (slider.isEnabled())
    {
        Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(),
                                bounds.getCentreY(),
                                arcRadius,
                                arcRadius,
                                0.0f,
                                rotaryStartAngle,
                                toAngle,
                                true);

        g.setColor (fill);
        g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    auto thumbWidth = lineW * 2.0f;
    Point<f32> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - MathConstants<f32>::halfPi),
                             bounds.getCentreY() + arcRadius * std::sin (toAngle - MathConstants<f32>::halfPi));

    g.setColor (slider.findColor (Slider::thumbColorId));
    g.fillEllipse (Rectangle<f32> (thumbWidth, thumbWidth).withCentre (thumbPoint));
}

z0 LookAndFeel_V4::drawPointer (Graphics& g, const f32 x, const f32 y, const f32 diameter,
                                  const Color& colour, i32k direction) noexcept
{
    Path p;
    p.startNewSubPath (x + diameter * 0.5f, y);
    p.lineTo (x + diameter, y + diameter * 0.6f);
    p.lineTo (x + diameter, y + diameter);
    p.lineTo (x, y + diameter);
    p.lineTo (x, y + diameter * 0.6f);
    p.closeSubPath();

    p.applyTransform (AffineTransform::rotation ((f32) direction * MathConstants<f32>::halfPi,
                                                 x + diameter * 0.5f, y + diameter * 0.5f));
    g.setColor (colour);
    g.fillPath (p);
}

Label* LookAndFeel_V4::createSliderTextBox (Slider& slider)
{
    auto* l = LookAndFeel_V2::createSliderTextBox (slider);

    if (getCurrentColorScheme() == LookAndFeel_V4::getGreyColorScheme() && (slider.getSliderStyle() == Slider::LinearBar
                                                                               || slider.getSliderStyle() == Slider::LinearBarVertical))
    {
        l->setColor (Label::textColorId, Colors::black.withAlpha (0.7f));
    }

    return l;
}

//==============================================================================
z0 LookAndFeel_V4::drawTooltip (Graphics& g, const Txt& text, i32 width, i32 height)
{
    Rectangle<i32> bounds (width, height);
    auto cornerSize = 5.0f;

    g.setColor (findColor (TooltipWindow::backgroundColorId));
    g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

    g.setColor (findColor (TooltipWindow::outlineColorId));
    g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

    detail::LookAndFeelHelpers::layoutTooltipText (getDefaultMetricsKind(), text, findColor (TooltipWindow::textColorId))
        .draw (g, { static_cast<f32> (width), static_cast<f32> (height) });
}

//==============================================================================
z0 LookAndFeel_V4::drawConcertinaPanelHeader (Graphics& g, const Rectangle<i32>& area,
                                                b8 isMouseOver, b8 /*isMouseDown*/,
                                                ConcertinaPanel& concertina, Component& panel)
{
    auto bounds = area.toFloat().reduced (0.5f);
    auto cornerSize = 4.0f;
    auto isTopPanel = (concertina.getPanel (0) == &panel);

    Path p;
    p.addRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                           cornerSize, cornerSize, isTopPanel, isTopPanel, false, false);

    g.setGradientFill (ColorGradient::vertical (Colors::white.withAlpha (isMouseOver ? 0.4f : 0.2f), static_cast<f32> (area.getY()),
                                                 Colors::darkgrey.withAlpha (0.1f), static_cast<f32> (area.getBottom())));
    g.fillPath (p);
}

//==============================================================================
z0 LookAndFeel_V4::drawLevelMeter (Graphics& g, i32 width, i32 height, f32 level)
{
    auto outerCornerSize = 3.0f;
    auto outerBorderWidth = 2.0f;
    auto totalBlocks = 7;
    auto spacingFraction = 0.03f;

    g.setColor (findColor (ResizableWindow::backgroundColorId));
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<f32> (width), static_cast<f32> (height), outerCornerSize);

    auto doubleOuterBorderWidth = 2.0f * outerBorderWidth;
    auto numBlocks = roundToInt ((f32) totalBlocks * level);

    auto blockWidth = ((f32) width - doubleOuterBorderWidth) / static_cast<f32> (totalBlocks);
    auto blockHeight = (f32) height - doubleOuterBorderWidth;

    auto blockRectWidth = (1.0f - 2.0f * spacingFraction) * blockWidth;
    auto blockRectSpacing = spacingFraction * blockWidth;

    auto blockCornerSize = 0.1f * blockWidth;

    auto c = findColor (Slider::thumbColorId);

    for (auto i = 0; i < totalBlocks; ++i)
    {
        if (i >= numBlocks)
            g.setColor (c.withAlpha (0.5f));
        else
            g.setColor (i < totalBlocks - 1 ? c : Colors::red);

        g.fillRoundedRectangle (outerBorderWidth + ((f32) i * blockWidth) + blockRectSpacing,
                                outerBorderWidth,
                                blockRectWidth,
                                blockHeight,
                                blockCornerSize);
    }
}

//==============================================================================
z0 LookAndFeel_V4::paintToolbarBackground (Graphics& g, i32 w, i32 h, Toolbar& toolbar)
{
    auto background = toolbar.findColor (Toolbar::backgroundColorId);

    g.setGradientFill ({ background, 0.0f, 0.0f,
                         background.darker (0.2f),
                         toolbar.isVertical() ? (f32) w - 1.0f : 0.0f,
                         toolbar.isVertical() ? 0.0f : (f32) h - 1.0f,
                         false });
    g.fillAll();
}

z0 LookAndFeel_V4::paintToolbarButtonLabel (Graphics& g, i32 x, i32 y, i32 width, i32 height,
                                              const Txt& text, ToolbarItemComponent& component)
{
    auto baseTextColor = component.findParentComponentOfClass<PopupMenu::CustomComponent>() != nullptr
                              ? component.findColor (PopupMenu::textColorId)
                              : component.findColor (Toolbar::labelTextColorId);

    g.setColor (baseTextColor.withAlpha (component.isEnabled() ? 1.0f : 0.25f));

    auto fontHeight = jmin (14.0f, (f32) height * 0.85f);
    g.setFont (fontHeight);

    g.drawFittedText (text,
                      x, y, width, height,
                      Justification::centred,
                      jmax (1, height / (i32) fontHeight));
}

//==============================================================================
z0 LookAndFeel_V4::drawPropertyPanelSectionHeader (Graphics& g, const Txt& name,
                                                     b8 isOpen, i32 width, i32 height)
{
    auto buttonSize = (f32) height * 0.75f;
    auto buttonIndent = ((f32) height - buttonSize) * 0.5f;

    drawTreeviewPlusMinusBox (g, { buttonIndent, buttonIndent, buttonSize, buttonSize },
                              findColor (ResizableWindow::backgroundColorId), isOpen, false);

    auto textX = static_cast<i32> ((buttonIndent * 2.0f + buttonSize + 2.0f));

    g.setColor (findColor (PropertyComponent::labelTextColorId));

    g.setFont (withDefaultMetrics (FontOptions { (f32) height * 0.7f, Font::bold }));
    g.drawText (name, textX, 0, width - textX - 4, height, Justification::centredLeft, true);
}

z0 LookAndFeel_V4::drawPropertyComponentBackground (Graphics& g, i32 width, i32 height, PropertyComponent& component)
{
    g.setColor (component.findColor (PropertyComponent::backgroundColorId));
    g.fillRect  (0, 0, width, height - 1);
}

z0 LookAndFeel_V4::drawPropertyComponentLabel (Graphics& g, [[maybe_unused]] i32 width, i32 height, PropertyComponent& component)
{
    auto indent = getPropertyComponentIndent (component);

    g.setColor (component.findColor (PropertyComponent::labelTextColorId)
                          .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

    g.setFont ((f32) jmin (height, 24) * 0.65f);

    auto r = getPropertyComponentContentPosition (component);

    g.drawFittedText (component.getName(),
                      indent, r.getY(), r.getX() - 5, r.getHeight(),
                      Justification::centredLeft, 2);
}

i32 LookAndFeel_V4::getPropertyComponentIndent (PropertyComponent& component)
{
    return jmin (10, component.getWidth() / 10);
}

Rectangle<i32> LookAndFeel_V4::getPropertyComponentContentPosition (PropertyComponent& component)
{
    auto textW = jmin (200, component.getWidth() / 2);
    return { textW, 0, component.getWidth() - textW, component.getHeight() - 1 };
}

//==============================================================================
z0 LookAndFeel_V4::drawCallOutBoxBackground (CallOutBox& box, Graphics& g,
                                               const Path& path, Image& cachedImage)
{
    if (cachedImage.isNull())
    {
        cachedImage = { Image::ARGB, box.getWidth(), box.getHeight(), true };
        Graphics g2 (cachedImage);

        DropShadow (Colors::black.withAlpha (0.7f), 8, { 0, 2 }).drawForPath (g2, path);
    }

    g.setColor (Colors::black);
    g.drawImageAt (cachedImage, 0, 0);

    g.setColor (currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).withAlpha (0.8f));
    g.fillPath (path);

    g.setColor (currentColorScheme.getUIColor (ColorScheme::UIColor::outline).withAlpha (0.8f));
    g.strokePath (path, PathStrokeType (2.0f));
}

//==============================================================================
z0 LookAndFeel_V4::drawStretchableLayoutResizerBar (Graphics& g, i32 /*w*/, i32 /*h*/, b8 /*isVerticalBar*/,
                                      b8 isMouseOver, b8 isMouseDragging)
{
    if (isMouseOver || isMouseDragging)
        g.fillAll (currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).withAlpha (0.5f));
}

//==============================================================================
z0 LookAndFeel_V4::initialiseColors()
{
    u32k transparent = 0x00000000;

    u32k coloursToUse[] =
    {
        TextButton::buttonColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        TextButton::buttonOnColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        TextButton::textColorOnId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        TextButton::textColorOffId,                currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        ToggleButton::textColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        ToggleButton::tickColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        ToggleButton::tickDisabledColorId,         currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).withAlpha (0.5f).getARGB(),

        TextEditor::backgroundColorId,             currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        TextEditor::textColorId,                   currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        TextEditor::highlightColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).withAlpha (0.4f).getARGB(),
        TextEditor::highlightedTextColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        TextEditor::outlineColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        TextEditor::focusedOutlineColorId,         currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        TextEditor::shadowColorId,                 transparent,

        CaretComponent::caretColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),

        Label::backgroundColorId,                  transparent,
        Label::textColorId,                        currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        Label::outlineColorId,                     transparent,
        Label::textWhenEditingColorId,             currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        ScrollBar::backgroundColorId,              transparent,
        ScrollBar::thumbColorId,                   currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),
        ScrollBar::trackColorId,                   transparent,

        TreeView::linesColorId,                    transparent,
        TreeView::backgroundColorId,               transparent,
        TreeView::dragAndDropIndicatorColorId,     currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        TreeView::selectedItemBackgroundColorId,   transparent,
        TreeView::oddItemsColorId,                 transparent,
        TreeView::evenItemsColorId,                transparent,

        PopupMenu::backgroundColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::menuBackground).getARGB(),
        PopupMenu::textColorId,                    currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        PopupMenu::headerTextColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        PopupMenu::highlightedTextColorId,         currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        PopupMenu::highlightedBackgroundColorId,   currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),

        ComboBox::buttonColorId,                   currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        ComboBox::outlineColorId,                  currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        ComboBox::textColorId,                     currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        ComboBox::backgroundColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        ComboBox::arrowColorId,                    currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        ComboBox::focusedOutlineColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        PropertyComponent::backgroundColorId,      currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        PropertyComponent::labelTextColorId,       currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        TextPropertyComponent::backgroundColorId,  currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        TextPropertyComponent::textColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        TextPropertyComponent::outlineColorId,     currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        BooleanPropertyComponent::backgroundColorId, currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        BooleanPropertyComponent::outlineColorId,    currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        ListBox::backgroundColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        ListBox::outlineColorId,                   currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        ListBox::textColorId,                      currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        Slider::backgroundColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        Slider::thumbColorId,                      currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),
        Slider::trackColorId,                      currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        Slider::rotarySliderFillColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        Slider::rotarySliderOutlineColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        Slider::textBoxTextColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        Slider::textBoxBackgroundColorId,          currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).withAlpha (0.0f).getARGB(),
        Slider::textBoxHighlightColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).withAlpha (0.4f).getARGB(),
        Slider::textBoxOutlineColorId,             currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        ResizableWindow::backgroundColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::windowBackground).getARGB(),

        DocumentWindow::textColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        AlertWindow::backgroundColorId,            currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        AlertWindow::textColorId,                  currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        AlertWindow::outlineColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        ProgressBar::backgroundColorId,            currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        ProgressBar::foregroundColorId,            currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),

        TooltipWindow::backgroundColorId,          currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        TooltipWindow::textColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        TooltipWindow::outlineColorId,             transparent,

        TabbedComponent::backgroundColorId,        transparent,
        TabbedComponent::outlineColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        TabbedButtonBar::tabOutlineColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::outline).withAlpha (0.5f).getARGB(),
        TabbedButtonBar::frontOutlineColorId,      currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        Toolbar::backgroundColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).withAlpha (0.4f).getARGB(),
        Toolbar::separatorColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        Toolbar::buttonMouseOverBackgroundColorId, currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).contrasting (0.2f).getARGB(),
        Toolbar::buttonMouseDownBackgroundColorId, currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).contrasting (0.5f).getARGB(),
        Toolbar::labelTextColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        Toolbar::editingModeOutlineColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        Toolbar::customisationDialogBackgroundColorId, currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),

        DrawableButton::textColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        DrawableButton::textColorOnId,             currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        DrawableButton::backgroundColorId,         transparent,
        DrawableButton::backgroundOnColorId,       currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),

        HyperlinkButton::textColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).interpolatedWith (Colors::blue, 0.4f).getARGB(),

        GroupComponent::outlineColorId,            currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        GroupComponent::textColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        BubbleComponent::backgroundColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        BubbleComponent::outlineColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        DirectoryContentsDisplayComponent::highlightColorId,          currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        DirectoryContentsDisplayComponent::textColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        DirectoryContentsDisplayComponent::highlightedTextColorId,    currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),

        0x1000440, /*LassoComponent::lassoFillColorId*/        currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),
        0x1000441, /*LassoComponent::lassoOutlineColorId*/     currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

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

        0x1004500, /*CodeEditorComponent::backgroundColorId*/                currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        0x1004502, /*CodeEditorComponent::highlightColorId*/                 currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).withAlpha (0.4f).getARGB(),
        0x1004503, /*CodeEditorComponent::defaultTextColorId*/               currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        0x1004504, /*CodeEditorComponent::lineNumberBackgroundId*/            currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).withAlpha (0.5f).getARGB(),
        0x1004505, /*CodeEditorComponent::lineNumberTextId*/                  currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),

        0x1007000, /*ColorSelector::backgroundColorId*/                     currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        0x1007001, /*ColorSelector::labelTextColorId*/                      currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        0x100ad00, /*KeyMappingEditorComponent::backgroundColorId*/          currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        0x100ad01, /*KeyMappingEditorComponent::textColorId*/                currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        FileSearchPathListComponent::backgroundColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::menuBackground).getARGB(),

        FileChooserDialogBox::titleTextColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        SidePanel::backgroundColor,                            currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        SidePanel::titleTextColor,                             currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        SidePanel::shadowBaseColor,                            currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).darker().getARGB(),
        SidePanel::dismissButtonNormalColor,                   currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),
        SidePanel::dismissButtonOverColor,                     currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).darker().getARGB(),
        SidePanel::dismissButtonDownColor,                     currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).brighter().getARGB(),

        FileBrowserComponent::currentPathBoxBackgroundColorId,    currentColorScheme.getUIColor (ColorScheme::UIColor::menuBackground).getARGB(),
        FileBrowserComponent::currentPathBoxTextColorId,          currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        FileBrowserComponent::currentPathBoxArrowColorId,         currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        FileBrowserComponent::filenameBoxBackgroundColorId,       currentColorScheme.getUIColor (ColorScheme::UIColor::menuBackground).getARGB(),
        FileBrowserComponent::filenameBoxTextColorId,             currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
    };

    for (i32 i = 0; i < numElementsInArray (coloursToUse); i += 2)
        setColor ((i32) coloursToUse [i], Color ((u32) coloursToUse [i + 1]));
}

} // namespace drx
