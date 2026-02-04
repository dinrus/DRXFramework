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

struct Spinner final : public Component,
                       private Timer
{
    Spinner()                       { startTimer (1000 / 50); }
    z0 timerCallback() override   { repaint(); }

    z0 paint (Graphics& g) override
    {
        getLookAndFeel().drawSpinningWaitAnimation (g, Colors::darkgrey, 0, 0, getWidth(), getHeight());
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spinner)
};

struct OnlineUnlockForm::OverlayComp final : public Component,
                                             private Thread,
                                             private Timer,
                                             private Button::Listener
{
    OverlayComp (OnlineUnlockForm& f, b8 hasCancelButton = false)
        : Thread (Txt()), form (f)
    {
        result.succeeded = false;
        email = form.emailBox.getText();
        password = form.passwordBox.getText();
        addAndMakeVisible (spinner);

        if (hasCancelButton)
        {
            cancelButton.reset (new TextButton (TRANS ("Cancel")));
            addAndMakeVisible (cancelButton.get());
            cancelButton->addListener (this);
        }

        startThread (Priority::normal);
    }

    ~OverlayComp() override
    {
        stopThread (10000);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::white.withAlpha (0.97f));

        g.setColor (Colors::black);
        g.setFont (15.0f);

        g.drawFittedText (TRANS ("Contacting XYZ...").replace ("XYZ", form.status.getWebsiteName()),
                          getLocalBounds().reduced (20, 0).removeFromTop (proportionOfHeight (0.6f)),
                          Justification::centred, 5);
    }

    z0 resized() override
    {
        i32k spinnerSize = 40;
        spinner.setBounds ((getWidth() - spinnerSize) / 2, proportionOfHeight (0.6f), spinnerSize, spinnerSize);

        if (cancelButton != nullptr)
            cancelButton->setBounds (getLocalBounds().removeFromBottom (50).reduced (getWidth() / 4, 5));
    }

    z0 run() override
    {
        result = form.status.attemptWebserverUnlock (email, password);
        startTimer (100);
    }

    z0 timerCallback() override
    {
        spinner.setVisible (false);
        stopTimer();

        if (result.errorMessage.isNotEmpty())
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             TRANS ("Registration Failed"),
                                                             result.errorMessage,
                                                             {},
                                                             &form);
            form.messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
        else if (result.informativeMessage.isNotEmpty())
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                             TRANS ("Registration Complete!"),
                                                             result.informativeMessage,
                                                             {},
                                                             &form);
            form.messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
        else if (result.urlToLaunch.isNotEmpty())
        {
            URL url (result.urlToLaunch);
            url.launchInDefaultBrowser();
        }

        // (local copies because we're about to delete this)
        const b8 worked = result.succeeded;
        OnlineUnlockForm& f = form;

        delete this;

        if (worked)
            f.dismiss();
    }

    z0 buttonClicked (Button* button) override
    {
        if (button == cancelButton.get())
        {
            form.status.userCancelled();

            spinner.setVisible (false);
            stopTimer();

            delete this;
        }
    }

    OnlineUnlockForm& form;
    Spinner spinner;
    OnlineUnlockStatus::UnlockResult result;
    Txt email, password;

    std::unique_ptr<TextButton> cancelButton;

    DRX_LEAK_DETECTOR (OnlineUnlockForm::OverlayComp)
};

static t32 getDefaultPasswordChar() noexcept
{
   #if DRX_LINUX || DRX_BSD
    return 0x2022;
   #else
    return 0x25cf;
   #endif
}

OnlineUnlockForm::OnlineUnlockForm (OnlineUnlockStatus& s,
                                    const Txt& userInstructions,
                                    b8 hasCancelButton,
                                    b8 overlayHasCancelButton)
    : message (Txt(), userInstructions),
      passwordBox (Txt(), getDefaultPasswordChar()),
      registerButton (TRANS ("Register")),
      cancelButton (TRANS ("Cancel")),
      status (s),
      showOverlayCancelButton (overlayHasCancelButton)
{
    // Please supply a message to tell your users what to do!
    jassert (userInstructions.isNotEmpty());

    setOpaque (true);

    emailBox.setText (status.getUserEmail());
    message.setJustificationType (Justification::centred);

    addAndMakeVisible (message);
    addAndMakeVisible (emailBox);
    addAndMakeVisible (passwordBox);
    addAndMakeVisible (registerButton);

    if (hasCancelButton)
        addAndMakeVisible (cancelButton);

    emailBox.setEscapeAndReturnKeysConsumed (false);
    passwordBox.setEscapeAndReturnKeysConsumed (false);

    registerButton.addShortcut (KeyPress (KeyPress::returnKey));

    registerButton.addListener (this);
    cancelButton.addListener (this);

    lookAndFeelChanged();
    setSize (500, 250);
}

OnlineUnlockForm::~OnlineUnlockForm()
{
    unlockingOverlay.deleteAndZero();
}

z0 OnlineUnlockForm::paint (Graphics& g)
{
    g.fillAll (Colors::lightgrey);
}

z0 OnlineUnlockForm::resized()
{
    /* If you're writing a plugin, then DO NOT USE A POP-UP A DIALOG WINDOW!
       Plugins that create external windows are incredibly annoying for users, and
       cause all sorts of headaches for hosts. Don't be the person who writes that
       plugin that irritates everyone with a nagging dialog box every time they scan!
    */
    jassert (DRXApplicationBase::isStandaloneApp() || findParentComponentOfClass<DialogWindow>() == nullptr);

    i32k buttonHeight = 22;

    auto r = getLocalBounds().reduced (10, 20);

    auto buttonArea = r.removeFromBottom (buttonHeight);
    registerButton.changeWidthToFitText (buttonHeight);
    cancelButton.changeWidthToFitText (buttonHeight);

    i32k gap = 20;
    buttonArea = buttonArea.withSizeKeepingCentre (registerButton.getWidth()
                                                     + (cancelButton.isVisible() ? gap + cancelButton.getWidth() : 0),
                                                   buttonHeight);
    registerButton.setBounds (buttonArea.removeFromLeft (registerButton.getWidth()));
    buttonArea.removeFromLeft (gap);
    cancelButton.setBounds (buttonArea);

    r.removeFromBottom (20);

    // (force use of a default system font to make sure it has the password blob character)
    const auto typeface = Font::getDefaultTypefaceForFont (FontOptions (Font::getDefaultSansSerifFontName(),
                                                                        Font::getDefaultStyle(),
                                                                        5.0f));
    Font font (withDefaultMetrics (FontOptions { typeface }));

    i32k boxHeight = 24;
    passwordBox.setBounds (r.removeFromBottom (boxHeight));
    passwordBox.setInputRestrictions (64);
    passwordBox.setFont (font);

    r.removeFromBottom (20);
    emailBox.setBounds (r.removeFromBottom (boxHeight));
    emailBox.setInputRestrictions (512);
    emailBox.setFont (font);

    r.removeFromBottom (20);

    message.setBounds (r);

    if (unlockingOverlay != nullptr)
        unlockingOverlay->setBounds (getLocalBounds());
}

z0 OnlineUnlockForm::lookAndFeelChanged()
{
    Color labelCol (findColor (TextEditor::backgroundColorId).contrasting (0.5f));

    emailBox.setTextToShowWhenEmpty (TRANS ("Email Address"), labelCol);
    passwordBox.setTextToShowWhenEmpty (TRANS ("Password"), labelCol);
}

z0 OnlineUnlockForm::showBubbleMessage (const Txt& text, Component& target)
{
    bubble.reset (new BubbleMessageComponent (500));
    addChildComponent (bubble.get());

    AttributedString attString;
    attString.append (text, withDefaultMetrics (FontOptions (16.0f)));

    bubble->showAt (getLocalArea (&target, target.getLocalBounds()),
                    attString, 500,  // numMillisecondsBeforeRemoving
                    true,  // removeWhenMouseClicked
                    false); // deleteSelfAfterUse
}

z0 OnlineUnlockForm::buttonClicked (Button* b)
{
    if (b == &registerButton)
        attemptRegistration();
    else if (b == &cancelButton)
        dismiss();
}

z0 OnlineUnlockForm::attemptRegistration()
{
    if (unlockingOverlay == nullptr)
    {
        if (emailBox.getText().trim().length() < 3)
        {
            showBubbleMessage (TRANS ("Please enter a valid email address!"), emailBox);
            return;
        }

        if (passwordBox.getText().trim().length() < 3)
        {
            showBubbleMessage (TRANS ("Please enter a valid password!"), passwordBox);
            return;
        }

        status.setUserEmail (emailBox.getText());

        addAndMakeVisible (unlockingOverlay = new OverlayComp (*this, showOverlayCancelButton));
        resized();
        unlockingOverlay->enterModalState();
    }
}

z0 OnlineUnlockForm::dismiss()
{
    delete this;
}

} // namespace drx
