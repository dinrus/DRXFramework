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

#include "../../Application/jucer_Headers.h"
#include "jucer_ProjucerLookAndFeel.h"

#include "../../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
ProjucerLookAndFeel::ProjucerLookAndFeel()
{
    setupColors();
}

ProjucerLookAndFeel::~ProjucerLookAndFeel() {}

z0 ProjucerLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, b8 isMouseOver, b8 isMouseDown)
{
    const auto area = button.getActiveArea();
    auto backgroundColor = findColor (button.isFrontTab() ? secondaryBackgroundColorId
                                                            : inactiveTabBackgroundColorId);

    g.setColor (backgroundColor);
    g.fillRect (area);

    const auto alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
    auto textColor = findColor (defaultTextColorId).withMultipliedAlpha (alpha);

    auto iconColor = findColor (button.isFrontTab() ? activeTabIconColorId
                                                      : inactiveTabIconColorId);

    auto isProjectTab = button.getName() == ProjectContentComponent::getProjectTabName();

    if (isProjectTab)
    {
        auto icon = Icon (getIcons().closedFolder,
                          iconColor.withMultipliedAlpha (alpha));

        auto isSingleTab = (button.getTabbedButtonBar().getNumTabs() == 1);

        if (isSingleTab)
        {
            auto activeArea = button.getActiveArea().reduced (5);

            activeArea.removeFromLeft (15);
            icon.draw (g, activeArea.removeFromLeft (activeArea.getHeight()).toFloat(), false);
            activeArea.removeFromLeft (10);

            g.setColor (textColor);
            g.drawFittedText (ProjectContentComponent::getProjectTabName(),
                              activeArea, Justification::centredLeft, 1);
        }
        else
        {
            icon.draw (g, button.getTextArea().reduced (8, 8).toFloat(), false);
        }
    }
    else
    {
        TextLayout textLayout;
        LookAndFeel_V3::createTabTextLayout (button, (f32) area.getWidth(), (f32) area.getHeight(), textColor, textLayout);

        textLayout.draw (g, button.getTextArea().toFloat());
    }
}

i32 ProjucerLookAndFeel::getTabButtonBestWidth (TabBarButton& button, i32)
{
    if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
        return bar->getWidth() / bar->getNumTabs();

    return 120;
}

z0 ProjucerLookAndFeel::drawPropertyComponentLabel (Graphics& g, i32, i32 height, PropertyComponent& component)
{
    g.setColor (component.findColor (defaultTextColorId)
                          .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

    auto textWidth = getTextWidthForPropertyComponent (component);

    g.setFont (getPropertyComponentFont());
    g.drawFittedText (component.getName(), 0, 0, textWidth, height, Justification::centredLeft, 5, 1.0f);
}

Rectangle<i32> ProjucerLookAndFeel::getPropertyComponentContentPosition (PropertyComponent& component)
{
    const auto paddedTextW = getTextWidthForPropertyComponent (component) + 5;
    return { paddedTextW , 0, component.getWidth() - paddedTextW, component.getHeight() - 1 };
}

z0 ProjucerLookAndFeel::drawButtonBackground (Graphics& g,
                                                Button& button,
                                                const Color& backgroundColor,
                                                b8 isMouseOverButton,
                                                b8 isButtonDown)
{
    const auto cornerSize = button.findParentComponentOfClass<PropertyComponent>() != nullptr ? 0.0f : 3.0f;
    const auto bounds = button.getLocalBounds().toFloat();

    auto baseColor = backgroundColor.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (isButtonDown || isMouseOverButton)
        baseColor = baseColor.contrasting (isButtonDown ? 0.2f : 0.05f);

    g.setColor (baseColor);

    if (button.isConnectedOnLeft() || button.isConnectedOnRight())
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), bounds.getHeight(),
                                  cornerSize, cornerSize,
                                  ! button.isConnectedOnLeft(),
                                  ! button.isConnectedOnRight(),
                                  ! button.isConnectedOnLeft(),
                                  ! button.isConnectedOnRight());

        g.fillPath (path);
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);
    }
}

z0 ProjucerLookAndFeel::drawButtonText (Graphics& g, TextButton& button, b8 isMouseOverButton, b8 isButtonDown)
{
    ignoreUnused (isMouseOverButton, isButtonDown);

    g.setFont (getTextButtonFont (button, button.getHeight()));

    g.setColor (button.findColor (button.getToggleState() ? TextButton::textColorOnId
                                                            : TextButton::textColorOffId)
                                   .withMultipliedAlpha (button.isEnabled() ? 1.0f
                                                                            : 0.5f));

    auto xIndent = jmin (8, button.getWidth() / 10);
    auto yIndent = jmin (3,  button.getHeight() / 6);

    auto textBounds = button.getLocalBounds().reduced (xIndent, yIndent);

    g.drawFittedText (button.getButtonText(), textBounds, Justification::centred, 3, 1.0f);
}

z0 ProjucerLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button, b8 isMouseOverButton, b8 isButtonDown)
{
    ignoreUnused (isMouseOverButton, isButtonDown);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    b8 isTextEmpty = button.getButtonText().isEmpty();
    b8 isPropertyComponentChild = (dynamic_cast<BooleanPropertyComponent*> (button.getParentComponent()) != nullptr
                                     || dynamic_cast<MultiChoicePropertyComponent*> (button.getParentComponent()) != nullptr);

    auto bounds = button.getLocalBounds();

    auto sideLength = isPropertyComponentChild ? 25 : bounds.getHeight();

    auto rectBounds = isTextEmpty ? bounds
                                  : bounds.removeFromLeft (jmin (sideLength, bounds.getWidth() / 3));

    rectBounds = rectBounds.withSizeKeepingCentre (sideLength, sideLength).reduced (4);

    g.setColor (button.findColor (ToggleButton::tickDisabledColorId));
    g.drawRoundedRectangle (rectBounds.toFloat(), 2.0f, 1.0f);

    if (button.getToggleState())
    {
        g.setColor (button.findColor (ToggleButton::tickColorId));
        const auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (rectBounds.reduced (2).toFloat(), false));
    }

    if (! isTextEmpty)
    {
        bounds.removeFromLeft (5);

        const auto fontSize = jmin (15.0f, (f32) button.getHeight() * 0.75f);

        g.setFont (fontSize);
        g.setColor (isPropertyComponentChild ? findColor (widgetTextColorId)
                                              : button.findColor (ToggleButton::textColorId));

        g.drawFittedText (button.getButtonText(), bounds, Justification::centredLeft, 2);
    }
}

z0 ProjucerLookAndFeel::fillTextEditorBackground (Graphics& g, i32 width, i32 height, TextEditor& textEditor)
{
    g.setColor (textEditor.findColor (TextEditor::backgroundColorId));
    g.fillRect (0, 0, width, height);

    g.setColor (textEditor.findColor (TextEditor::outlineColorId));
    g.drawHorizontalLine (height - 1, 0.0f, static_cast<f32> (width));
}

z0 ProjucerLookAndFeel::layoutFileBrowserComponent (FileBrowserComponent& browserComp,
                                                      DirectoryContentsDisplayComponent* fileListComponent,
                                                      FilePreviewComponent* previewComp,
                                                      ComboBox* currentPathBox,
                                                      TextEditor* filenameBox,
                                                      Button* goUpButton)
{
    const auto sectionHeight = 22;
    const auto buttonWidth = 50;

    auto b = browserComp.getLocalBounds().reduced (20, 5);

    auto topSlice    = b.removeFromTop (sectionHeight);
    auto bottomSlice = b.removeFromBottom (sectionHeight);

    currentPathBox->setBounds (topSlice.removeFromLeft (topSlice.getWidth() - buttonWidth));
    currentPathBox->setColor (ComboBox::backgroundColorId,    findColor (backgroundColorId));
    currentPathBox->setColor (ComboBox::textColorId,          findColor (defaultTextColorId));
    currentPathBox->setColor (ComboBox::arrowColorId,         findColor (defaultTextColorId));

    topSlice.removeFromLeft (6);
    goUpButton->setBounds (topSlice);

    bottomSlice.removeFromLeft (50);
    filenameBox->setBounds (bottomSlice);
    filenameBox->setColor (TextEditor::backgroundColorId, findColor (backgroundColorId));
    filenameBox->setColor (TextEditor::textColorId,       findColor (defaultTextColorId));
    filenameBox->setColor (TextEditor::outlineColorId,    findColor (defaultTextColorId));
    filenameBox->applyFontToAllText (filenameBox->getFont());

    if (previewComp != nullptr)
        previewComp->setBounds (b.removeFromRight (b.getWidth() / 3));

    if (auto listAsComp = dynamic_cast<Component*> (fileListComponent))
        listAsComp->setBounds (b.reduced (0, 10));
}

z0 ProjucerLookAndFeel::drawFileBrowserRow (Graphics& g, i32 width, i32 height,
                                              const File& file, const Txt& filename, Image* icon,
                                              const Txt& fileSizeDescription,
                                              const Txt& fileTimeDescription,
                                              b8 isDirectory, b8 isItemSelected,
                                              i32 itemIndex, DirectoryContentsDisplayComponent& dcc)
{
    if (auto fileListComp = dynamic_cast<Component*> (&dcc))
    {
        fileListComp->setColor (DirectoryContentsDisplayComponent::textColorId,
                                 findColor (isItemSelected ? defaultHighlightedTextColorId : defaultTextColorId));

        fileListComp->setColor (DirectoryContentsDisplayComponent::highlightColorId,
                                 findColor (defaultHighlightColorId).withAlpha (0.75f));
    }


    LookAndFeel_V2::drawFileBrowserRow (g, width, height, file, filename, icon,
                                        fileSizeDescription, fileTimeDescription,
                                        isDirectory, isItemSelected, itemIndex, dcc);
}

z0 ProjucerLookAndFeel::drawCallOutBoxBackground (CallOutBox&, Graphics& g, const Path& path, Image&)
{
    g.setColor (findColor (secondaryBackgroundColorId));
    g.fillPath (path);

    g.setColor (findColor (userButtonBackgroundColorId));
    g.strokePath (path, PathStrokeType (2.0f));
}

z0 ProjucerLookAndFeel::drawMenuBarBackground (Graphics& g, i32 width, i32 height,
                                                 b8, MenuBarComponent& menuBar)
{
    const auto colour = menuBar.findColor (backgroundColorId).withAlpha (0.75f);

    Rectangle<i32> r (width, height);

    g.setColor (colour.contrasting (0.15f));
    g.fillRect  (r.removeFromTop (1));
    g.fillRect  (r.removeFromBottom (1));

    g.setGradientFill (ColorGradient (colour, 0, 0, colour.darker (0.2f), 0, (f32)height, false));
    g.fillRect (r);
}

z0 ProjucerLookAndFeel::drawMenuBarItem (Graphics& g, i32 width, i32 height,
                                           i32 itemIndex, const Txt& itemText,
                                           b8 isMouseOverItem, b8 isMenuOpen,
                                           b8 /*isMouseOverBar*/, MenuBarComponent& menuBar)
{
    if (! menuBar.isEnabled())
    {
        g.setColor (menuBar.findColor (defaultTextColorId)
                            .withMultipliedAlpha (0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll   (menuBar.findColor (defaultHighlightColorId).withAlpha (0.75f));
        g.setColor (menuBar.findColor (defaultHighlightedTextColorId));
    }
    else
    {
        g.setColor (menuBar.findColor (defaultTextColorId));
    }

    g.setFont (getMenuBarFont (menuBar, itemIndex, itemText));
    g.drawFittedText (itemText, 0, 0, width, height, Justification::centred, 1);
}

z0 ProjucerLookAndFeel::drawResizableFrame (Graphics& g, i32 w, i32 h, const BorderSize<i32>& border)
{
    ignoreUnused (g, w, h, border);
}

z0 ProjucerLookAndFeel::drawComboBox (Graphics& g, i32 width, i32 height, b8,
                                        i32, i32, i32, i32, ComboBox& box)
{
    const auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 1.5f;
    Rectangle<i32> boxBounds (0, 0, width, height);

    auto isChoiceCompChild = (box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr);

    if (isChoiceCompChild)
    {
        box.setColor (ComboBox::textColorId, findColor (widgetTextColorId));

        g.setColor (findColor (widgetBackgroundColorId));
        g.fillRect (boxBounds);

        auto arrowZone = boxBounds.removeFromRight (boxBounds.getHeight()).reduced (0, 2).toFloat();
        g.setColor (Colors::black);
        g.fillPath (getChoiceComponentArrowPath (arrowZone));
    }
    else
    {
        g.setColor (box.findColor (ComboBox::outlineColorId));
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

        auto arrowZone = boxBounds.removeFromRight (boxBounds.getHeight()).toFloat();
        g.setColor (box.findColor (ComboBox::arrowColorId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
        g.fillPath (getArrowPath (arrowZone, 2, true, Justification::centred));

    }
}

z0 ProjucerLookAndFeel::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<f32>& area,
                                                    Color, b8 isOpen, b8 /**isMouseOver*/)
{
    g.strokePath (getArrowPath (area, isOpen ? 2 : 1, false, Justification::centredRight), PathStrokeType (2.0f));
}

ProgressBar::Style ProjucerLookAndFeel::getDefaultProgressBarStyle (const ProgressBar&)
{
    return ProgressBar::Style::circular;
}

//==============================================================================
Path ProjucerLookAndFeel::getArrowPath (Rectangle<f32> arrowZone, i32k direction,
                                        b8 filled, const Justification justification)
{
    auto w = jmin (arrowZone.getWidth(),  (direction == 0 || direction == 2) ? 8.0f : filled ? 5.0f : 8.0f);
    auto h = jmin (arrowZone.getHeight(), (direction == 0 || direction == 2) ? 5.0f : filled ? 8.0f : 5.0f);

    if (justification == Justification::centred)
    {
        arrowZone.reduce ((arrowZone.getWidth() - w) / 2, (arrowZone.getHeight() - h) / 2);
    }
    else if (justification == Justification::centredRight)
    {
        arrowZone.removeFromLeft (arrowZone.getWidth() - w);
        arrowZone.reduce (0, (arrowZone.getHeight() - h) / 2);
    }
    else if (justification == Justification::centredLeft)
    {
        arrowZone.removeFromRight (arrowZone.getWidth() - w);
        arrowZone.reduce (0, (arrowZone.getHeight() - h) / 2);
    }
    else
    {
        jassertfalse; // currently only supports centred justifications
    }

    Path path;
    path.startNewSubPath (arrowZone.getX(), arrowZone.getBottom());
    path.lineTo (arrowZone.getCentreX(), arrowZone.getY());
    path.lineTo (arrowZone.getRight(), arrowZone.getBottom());

    if (filled)
        path.closeSubPath();

    path.applyTransform (AffineTransform::rotation ((f32) direction * MathConstants<f32>::halfPi,
                                                    arrowZone.getCentreX(), arrowZone.getCentreY()));

    return path;
}

Path ProjucerLookAndFeel::getChoiceComponentArrowPath (Rectangle<f32> arrowZone)
{
    auto topBounds = arrowZone.removeFromTop (arrowZone.getHeight() * 0.5f);
    auto bottomBounds = arrowZone;

    auto topArrow = getArrowPath (topBounds, 0, true, Justification::centred);
    auto bottomArrow = getArrowPath (bottomBounds, 2, true, Justification::centred);

    topArrow.addPath (bottomArrow);

    return topArrow;
}

//==============================================================================
z0 ProjucerLookAndFeel::setupColors()
{
    auto& colourScheme = getCurrentColorScheme();

    if (colourScheme == getDarkColorScheme() || colourScheme == getProjucerDarkColorScheme())
    {
        setColor (backgroundColorId,                   Color (0xff323e44));
        setColor (secondaryBackgroundColorId,          Color (0xff263238));
        setColor (defaultTextColorId,                  Colors::white);
        setColor (widgetTextColorId,                   Colors::white);
        setColor (defaultButtonBackgroundColorId,      Color (0xffa45c94));
        setColor (secondaryButtonBackgroundColorId,    Colors::black);
        setColor (userButtonBackgroundColorId,         Color (0xffa45c94));
        setColor (defaultIconColorId,                  Colors::white);
        setColor (treeIconColorId,                     Color (0xffa9a9a9));
        setColor (defaultHighlightColorId,             Color (0xffe0ec65));
        setColor (defaultHighlightedTextColorId,       Colors::black);
        setColor (codeEditorLineNumberColorId,         Color (0xffaaaaaa));
        setColor (activeTabIconColorId,                Colors::white);
        setColor (inactiveTabBackgroundColorId,        Color (0xff181f22));
        setColor (inactiveTabIconColorId,              Color (0xffa9a9a9));
        setColor (contentHeaderBackgroundColorId,      Colors::black);
        setColor (widgetBackgroundColorId,             Color (0xff495358));
        setColor (secondaryWidgetBackgroundColorId,    Color (0xff303b41));

        colourScheme = getProjucerDarkColorScheme();
    }
    else if (colourScheme == getGreyColorScheme())
    {
        setColor (backgroundColorId,                   Color (0xff505050));
        setColor (secondaryBackgroundColorId,          Color (0xff424241));
        setColor (defaultTextColorId,                  Colors::white);
        setColor (widgetTextColorId,                   Colors::black);
        setColor (defaultButtonBackgroundColorId,      Color (0xff26ba90));
        setColor (secondaryButtonBackgroundColorId,    Colors::black);
        setColor (userButtonBackgroundColorId,         Color (0xff26ba90));
        setColor (defaultIconColorId,                  Colors::white);
        setColor (treeIconColorId,                     Color (0xffa9a9a9));
        setColor (defaultHighlightColorId,             Color (0xffe0ec65));
        setColor (defaultHighlightedTextColorId,       Colors::black);
        setColor (codeEditorLineNumberColorId,         Color (0xffaaaaaa));
        setColor (activeTabIconColorId,                Colors::white);
        setColor (inactiveTabBackgroundColorId,        Color (0xff373737));
        setColor (inactiveTabIconColorId,              Color (0xffa9a9a9));
        setColor (contentHeaderBackgroundColorId,      Colors::black);
        setColor (widgetBackgroundColorId,             Colors::white);
        setColor (secondaryWidgetBackgroundColorId,    Color (0xffdddddd));
    }
    else if (colourScheme == getLightColorScheme())
    {
        setColor (backgroundColorId,                   Color (0xffefefef));
        setColor (secondaryBackgroundColorId,          Color (0xfff9f9f9));
        setColor (defaultTextColorId,                  Colors::black);
        setColor (widgetTextColorId,                   Colors::black);
        setColor (defaultButtonBackgroundColorId,      Color (0xff42a2c8));
        setColor (secondaryButtonBackgroundColorId,    Color (0xffa1c677));
        setColor (userButtonBackgroundColorId,         Color (0xff42a2c8));
        setColor (defaultIconColorId,                  Colors::white);
        setColor (treeIconColorId,                     Color (0xffa9a9a9));
        setColor (defaultHighlightColorId,             Colors::orange);
        setColor (defaultHighlightedTextColorId,       Color (0xff585656));
        setColor (codeEditorLineNumberColorId,         Color (0xff888888));
        setColor (activeTabIconColorId,                Color (0xff42a2c8));
        setColor (inactiveTabBackgroundColorId,        Color (0xffd5d5d5));
        setColor (inactiveTabIconColorId,              Color (0xffa9a9a9));
        setColor (contentHeaderBackgroundColorId,      Color (0xff42a2c8));
        setColor (widgetBackgroundColorId,             Colors::white);
        setColor (secondaryWidgetBackgroundColorId,    Color (0xfff4f4f4));
    }

    setColor (Label::textColorId,                             findColor (defaultTextColorId));
    setColor (Label::textWhenEditingColorId,                  findColor (widgetTextColorId));
    setColor (TextEditor::highlightColorId,                   findColor (defaultHighlightColorId).withAlpha (0.75f));
    setColor (TextEditor::highlightedTextColorId,             findColor (defaultHighlightedTextColorId));
    setColor (TextEditor::outlineColorId,                     Colors::transparentBlack);
    setColor (TextEditor::focusedOutlineColorId,              Colors::transparentBlack);
    setColor (TextEditor::backgroundColorId,                  findColor (widgetBackgroundColorId));
    setColor (TextEditor::textColorId,                        findColor (widgetTextColorId));
    setColor (TextButton::buttonColorId,                      findColor (defaultButtonBackgroundColorId));
    setColor (ScrollBar::ColorIds::thumbColorId,             Color (0xffd0d8e0));
    setColor (TextPropertyComponent::outlineColorId,          Colors::transparentBlack);
    setColor (TextPropertyComponent::backgroundColorId,       findColor (widgetBackgroundColorId));
    setColor (TextPropertyComponent::textColorId,             findColor (widgetTextColorId));
    setColor (BooleanPropertyComponent::outlineColorId,       Colors::transparentBlack);
    setColor (BooleanPropertyComponent::backgroundColorId,    findColor (widgetBackgroundColorId));
    setColor (ToggleButton::tickDisabledColorId,              Color (0xffa9a9a9));
    setColor (ToggleButton::tickColorId,                      findColor (defaultButtonBackgroundColorId).withMultipliedBrightness (1.3f));
    setColor (CodeEditorComponent::backgroundColorId,         findColor (secondaryBackgroundColorId));
    setColor (CodeEditorComponent::lineNumberTextId,           findColor (codeEditorLineNumberColorId));
    setColor (CodeEditorComponent::lineNumberBackgroundId,     findColor (backgroundColorId));
    setColor (CodeEditorComponent::highlightColorId,          findColor (defaultHighlightColorId).withAlpha (0.5f));
    setColor (CaretComponent::caretColorId,                   findColor (defaultButtonBackgroundColorId));
    setColor (TreeView::selectedItemBackgroundColorId,        findColor (defaultHighlightColorId));
    setColor (PopupMenu::highlightedBackgroundColorId,        findColor (defaultHighlightColorId).withAlpha (0.75f));
    setColor (PopupMenu::highlightedTextColorId,              findColor (defaultHighlightedTextColorId));
    setColor (ProgressBar::foregroundColorId,                 findColor (defaultButtonBackgroundColorId));
    setColor (0x1000440, /*LassoComponent::lassoFillColorId*/ findColor (defaultHighlightColorId).withAlpha (0.3f));
}
