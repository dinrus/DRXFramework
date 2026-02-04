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

DialogWindow::DialogWindow (const Txt& name, Color colour,
                            const b8 escapeCloses, const b8 onDesktop,
                            const f32 scale)
    : DocumentWindow (name, colour, DocumentWindow::closeButton, onDesktop),
      desktopScale (scale),
      escapeKeyTriggersCloseButton (escapeCloses)
{
}

DialogWindow::~DialogWindow() = default;

b8 DialogWindow::escapeKeyPressed()
{
    if (escapeKeyTriggersCloseButton)
    {
        setVisible (false);
        return true;
    }

    return false;
}

b8 DialogWindow::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::escapeKey && escapeKeyPressed())
        return true;

    return DocumentWindow::keyPressed (key);
}

z0 DialogWindow::resized()
{
    DocumentWindow::resized();

    if (escapeKeyTriggersCloseButton)
    {
        if (auto* close = getCloseButton())
        {
            const KeyPress esc (KeyPress::escapeKey, 0, 0);

            if (! close->isRegisteredForShortcut (esc))
                close->addShortcut (esc);
        }
    }
}

//==============================================================================
class DefaultDialogWindow final : public DialogWindow
{
public:
    DefaultDialogWindow (LaunchOptions& options)
        : DialogWindow (options.dialogTitle, options.dialogBackgroundColor,
                        options.escapeKeyTriggersCloseButton, true,
                        options.componentToCentreAround != nullptr
                            ? Component::getApproximateScaleFactorForComponent (options.componentToCentreAround)
                            : 1.0f)
    {
        if (options.content.willDeleteObject())
            setContentOwned (options.content.release(), true);
        else
            setContentNonOwned (options.content.release(), true);

        centreAroundComponent (options.componentToCentreAround, getWidth(), getHeight());
        setResizable (options.resizable, options.useBottomRightCornerResizer);

        setUsingNativeTitleBar (options.useNativeTitleBar);
        setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());
    }

    z0 closeButtonPressed() override
    {
        setVisible (false);
    }

private:
    DRX_DECLARE_NON_COPYABLE (DefaultDialogWindow)
};

DialogWindow::LaunchOptions::LaunchOptions() noexcept {}

DialogWindow* DialogWindow::LaunchOptions::create()
{
    jassert (content != nullptr); // You need to provide some kind of content for the dialog!

    return new DefaultDialogWindow (*this);
}

DialogWindow* DialogWindow::LaunchOptions::launchAsync()
{
    auto* d = create();
    d->enterModalState (true, nullptr, true);
    return d;
}

#if DRX_MODAL_LOOPS_PERMITTED
i32 DialogWindow::LaunchOptions::runModal()
{
    return launchAsync()->runModalLoop();
}
#endif

//==============================================================================
z0 DialogWindow::showDialog (const Txt& dialogTitle,
                               Component* const contentComponent,
                               Component* const componentToCentreAround,
                               Color backgroundColor,
                               const b8 escapeKeyTriggersCloseButton,
                               const b8 resizable,
                               const b8 useBottomRightCornerResizer)
{
    LaunchOptions o;
    o.dialogTitle = dialogTitle;
    o.content.setNonOwned (contentComponent);
    o.componentToCentreAround = componentToCentreAround;
    o.dialogBackgroundColor = backgroundColor;
    o.escapeKeyTriggersCloseButton = escapeKeyTriggersCloseButton;
    o.useNativeTitleBar = false;
    o.resizable = resizable;
    o.useBottomRightCornerResizer = useBottomRightCornerResizer;

    o.launchAsync();
}

#if DRX_MODAL_LOOPS_PERMITTED
i32 DialogWindow::showModalDialog (const Txt& dialogTitle,
                                   Component* const contentComponent,
                                   Component* const componentToCentreAround,
                                   Color backgroundColor,
                                   const b8 escapeKeyTriggersCloseButton,
                                   const b8 resizable,
                                   const b8 useBottomRightCornerResizer)
{
    LaunchOptions o;
    o.dialogTitle = dialogTitle;
    o.content.setNonOwned (contentComponent);
    o.componentToCentreAround = componentToCentreAround;
    o.dialogBackgroundColor = backgroundColor;
    o.escapeKeyTriggersCloseButton = escapeKeyTriggersCloseButton;
    o.useNativeTitleBar = false;
    o.resizable = resizable;
    o.useBottomRightCornerResizer = useBottomRightCornerResizer;

    return o.runModal();
}
#endif

//==============================================================================
std::unique_ptr<AccessibilityHandler> DialogWindow::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::dialogWindow);
}

} // namespace drx
