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

PreferencesPanel::PreferencesPanel()
    : buttonSize (70)
{
}

PreferencesPanel::~PreferencesPanel()
{
}

i32 PreferencesPanel::getButtonSize() const noexcept
{
    return buttonSize;
}

z0 PreferencesPanel::setButtonSize (i32 newSize)
{
    buttonSize = newSize;
    resized();
}

//==============================================================================
z0 PreferencesPanel::addSettingsPage (const Txt& title,
                                        const Drawable* icon,
                                        const Drawable* overIcon,
                                        const Drawable* downIcon)
{
    auto* button = new DrawableButton (title, DrawableButton::ImageAboveTextLabel);
    buttons.add (button);

    button->setImages (icon, overIcon, downIcon);
    button->setRadioGroupId (1);
    button->onClick = [this] { clickedPage(); };
    button->setClickingTogglesState (true);
    button->setWantsKeyboardFocus (false);
    addAndMakeVisible (button);

    resized();

    if (currentPage == nullptr)
        setCurrentPage (title);
}

z0 PreferencesPanel::addSettingsPage (const Txt& title, ukk imageData, i32 imageDataSize)
{
    DrawableImage icon, iconOver, iconDown;
    icon.setImage (ImageCache::getFromMemory (imageData, imageDataSize));

    iconOver.setImage (ImageCache::getFromMemory (imageData, imageDataSize));
    iconOver.setOverlayColor (Colors::black.withAlpha (0.12f));

    iconDown.setImage (ImageCache::getFromMemory (imageData, imageDataSize));
    iconDown.setOverlayColor (Colors::black.withAlpha (0.25f));

    addSettingsPage (title, &icon, &iconOver, &iconDown);
}

//==============================================================================
z0 PreferencesPanel::showInDialogBox (const Txt& dialogTitle, i32 dialogWidth, i32 dialogHeight, Color backgroundColor)
{
    setSize (dialogWidth, dialogHeight);

    DialogWindow::LaunchOptions o;
    o.content.setNonOwned (this);
    o.dialogTitle                   = dialogTitle;
    o.dialogBackgroundColor        = backgroundColor;
    o.escapeKeyTriggersCloseButton  = false;
    o.useNativeTitleBar             = false;
    o.resizable                     = false;

    o.launchAsync();
}

//==============================================================================
z0 PreferencesPanel::resized()
{
    for (i32 i = 0; i < buttons.size(); ++i)
        buttons.getUnchecked (i)->setBounds (i * buttonSize, 0, buttonSize, buttonSize);

    if (currentPage != nullptr)
        currentPage->setBounds (getLocalBounds().withTop (buttonSize + 5));
}

z0 PreferencesPanel::paint (Graphics& g)
{
    g.setColor (Colors::grey);
    g.fillRect (0, buttonSize + 2, getWidth(), 1);
}

z0 PreferencesPanel::setCurrentPage (const Txt& pageName)
{
    if (currentPageName != pageName)
    {
        currentPageName = pageName;

        currentPage.reset();
        currentPage.reset (createComponentForPage (pageName));

        if (currentPage != nullptr)
        {
            addAndMakeVisible (currentPage.get());
            currentPage->toBack();
            resized();
        }

        for (auto* b : buttons)
        {
            if (b->getName() == pageName)
            {
                b->setToggleState (true, dontSendNotification);
                break;
            }
        }
    }
}

z0 PreferencesPanel::clickedPage()
{
    for (auto* b : buttons)
    {
        if (b->getToggleState())
        {
            setCurrentPage (b->getName());
            break;
        }
    }
}

} // namespace drx
