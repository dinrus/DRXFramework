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

static t32 getDefaultPasswordChar() noexcept
{
   #if DRX_LINUX || DRX_BSD
    return 0x2022;
   #else
    return 0x25cf;
   #endif
}

static i32 showAlertWindowUnmanaged (const MessageBoxOptions& opts, ModalComponentManager::Callback* cb)
{
    return detail::ConcreteScopedMessageBoxImpl::showUnmanaged (detail::AlertWindowHelpers::create (opts), cb);
}

//==============================================================================
AlertWindow::AlertWindow (const Txt& title,
                          const Txt& message,
                          MessageBoxIconType iconType,
                          Component* comp)
   : TopLevelWindow (title, true),
     alertIconType (iconType),
     associatedComponent (comp),
     desktopScale (comp != nullptr ? Component::getApproximateScaleFactorForComponent (comp) : 1.0f)
{
    setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());

    accessibleMessageLabel.setColor (Label::textColorId,       Colors::transparentBlack);
    accessibleMessageLabel.setColor (Label::backgroundColorId, Colors::transparentBlack);
    accessibleMessageLabel.setColor (Label::outlineColorId,    Colors::transparentBlack);
    accessibleMessageLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (accessibleMessageLabel);

    if (message.isEmpty())
        text = " "; // to force an update if the message is empty

    setMessage (message);

    AlertWindow::lookAndFeelChanged();
    constrainer.setMinimumOnscreenAmounts (0x10000, 0x10000, 0x10000, 0x10000);
}

AlertWindow::~AlertWindow()
{
    // Ensure that the focus does not jump to another TextEditor while we
    // remove children.
    for (auto* t : textBoxes)
        t->setWantsKeyboardFocus (false);

    // Give away focus before removing the editors, so that any TextEditor
    // with focus has a chance to dismiss native keyboard if shown.
    giveAwayKeyboardFocus();

    removeAllChildren();
}

z0 AlertWindow::userTriedToCloseWindow()
{
    if (escapeKeyCancels || buttons.size() > 0)
        exitModalState (0);
}

//==============================================================================
z0 AlertWindow::setMessage (const Txt& message)
{
    auto newMessage = message.substring (0, 2048);

    if (text != newMessage)
    {
        text = newMessage;

        auto accessibleText = getName() + ". " + text;
        accessibleMessageLabel.setText (accessibleText, NotificationType::dontSendNotification);
        setDescription (accessibleText);

        updateLayout (true);
        repaint();
    }
}

//==============================================================================
z0 AlertWindow::exitAlert (Button* button)
{
    if (auto* parent = button->getParentComponent())
        parent->exitModalState (button->getCommandID());
}

//==============================================================================
z0 AlertWindow::addButton (const Txt& name,
                             i32k returnValue,
                             const KeyPress& shortcutKey1,
                             const KeyPress& shortcutKey2)
{
    auto* b = new TextButton (name, {});
    buttons.add (b);

    b->setWantsKeyboardFocus (true);
    b->setExplicitFocusOrder (1);
    b->setMouseClickGrabsKeyboardFocus (false);
    b->setCommandToTrigger (nullptr, returnValue, false);
    b->addShortcut (shortcutKey1);
    b->addShortcut (shortcutKey2);
    b->onClick = [this, b] { exitAlert (b); };

    Array<TextButton*> buttonsArray (buttons.begin(), buttons.size());
    auto& lf = getLookAndFeel();

    auto buttonHeight = lf.getAlertWindowButtonHeight();
    auto buttonWidths = lf.getWidthsForTextButtons (*this, buttonsArray);

    jassert (buttonWidths.size() == buttons.size());
    i32 i = 0;

    for (auto* button : buttons)
        button->setSize (buttonWidths[i++], buttonHeight);

    addAndMakeVisible (b, 0);
    updateLayout (false);
}

i32 AlertWindow::getNumButtons() const
{
    return buttons.size();
}

Button* AlertWindow::getButton (i32 index) const
{
    return buttons[index];
}

Button* AlertWindow::getButton (const Txt& buttonName) const
{
    for (auto* button : buttons)
        if (buttonName == button->getName())
            return button;

    return nullptr;
}

z0 AlertWindow::triggerButtonClick (const Txt& buttonName)
{
    if (auto* button = getButton (buttonName))
        button->triggerClick();
}

z0 AlertWindow::setEscapeKeyCancels (b8 shouldEscapeKeyCancel)
{
    escapeKeyCancels = shouldEscapeKeyCancel;
}

//==============================================================================
z0 AlertWindow::addTextEditor (const Txt& name,
                                 const Txt& initialContents,
                                 const Txt& onScreenLabel,
                                 const b8 isPasswordBox)
{
    auto* ed = new TextEditor (name, isPasswordBox ? getDefaultPasswordChar() : 0);
    ed->setSelectAllWhenFocused (true);
    ed->setEscapeAndReturnKeysConsumed (false);
    textBoxes.add (ed);
    allComps.add (ed);

    ed->setColor (TextEditor::outlineColorId, findColor (ComboBox::outlineColorId));
    ed->setFont (getLookAndFeel().getAlertWindowMessageFont());
    addAndMakeVisible (ed);
    ed->setText (initialContents);
    ed->setCaretPosition (initialContents.length());
    textboxNames.add (onScreenLabel);

    updateLayout (false);
}

TextEditor* AlertWindow::getTextEditor (const Txt& nameOfTextEditor) const
{
    for (auto* tb : textBoxes)
        if (tb->getName() == nameOfTextEditor)
            return tb;

    return nullptr;
}

Txt AlertWindow::getTextEditorContents (const Txt& nameOfTextEditor) const
{
    if (auto* t = getTextEditor (nameOfTextEditor))
        return t->getText();

    return {};
}


//==============================================================================
z0 AlertWindow::addComboBox (const Txt& name,
                               const StringArray& items,
                               const Txt& onScreenLabel)
{
    auto* cb = new ComboBox (name);
    comboBoxes.add (cb);
    allComps.add (cb);

    cb->addItemList (items, 1);

    addAndMakeVisible (cb);
    cb->setSelectedItemIndex (0);

    comboBoxNames.add (onScreenLabel);
    updateLayout (false);
}

ComboBox* AlertWindow::getComboBoxComponent (const Txt& nameOfList) const
{
    for (auto* cb : comboBoxes)
        if (cb->getName() == nameOfList)
            return cb;

    return nullptr;
}

//==============================================================================
class AlertTextComp final : public TextEditor
{
public:
    AlertTextComp (AlertWindow& owner, const Txt& message, const Font& font)
    {
        if (owner.isColorSpecified (AlertWindow::textColorId))
            setColor (TextEditor::textColorId, owner.findColor (AlertWindow::textColorId));

        setColor (TextEditor::backgroundColorId, Colors::transparentBlack);
        setColor (TextEditor::outlineColorId, Colors::transparentBlack);
        setColor (TextEditor::shadowColorId, Colors::transparentBlack);

        setReadOnly (true);
        setMultiLine (true, true);
        setCaretVisible (false);
        setScrollbarsShown (true);
        lookAndFeelChanged();
        setWantsKeyboardFocus (false);
        setFont (font);
        setText (message, false);

        bestWidth = 2 * (i32) std::sqrt (font.getHeight() * GlyphArrangement::getStringWidth (font, message));
    }

    z0 updateLayout (i32k width)
    {
        AttributedString s;
        s.setJustification (Justification::topLeft);
        s.append (getText(), getFont());

        TextLayout text;
        text.createLayoutWithBalancedLineLengths (s, (f32) width - 8.0f);
        setSize (width, jmin (width, (i32) (text.getHeight() + getFont().getHeight())));
    }

    i32 bestWidth;

    DRX_DECLARE_NON_COPYABLE (AlertTextComp)
};

z0 AlertWindow::addTextBlock (const Txt& textBlock)
{
    auto* c = new AlertTextComp (*this, textBlock, getLookAndFeel().getAlertWindowMessageFont());
    textBlocks.add (c);
    allComps.add (c);
    addAndMakeVisible (c);

    updateLayout (false);
}

//==============================================================================
z0 AlertWindow::addProgressBarComponent (f64& progressValue, std::optional<ProgressBar::Style> style)
{
    auto* pb = new ProgressBar (progressValue, style);
    progressBars.add (pb);
    allComps.add (pb);
    addAndMakeVisible (pb);

    updateLayout (false);
}

//==============================================================================
z0 AlertWindow::addCustomComponent (Component* const component)
{
    customComps.add (component);
    allComps.add (component);
    addAndMakeVisible (component);

    updateLayout (false);
}

i32 AlertWindow::getNumCustomComponents() const                 { return customComps.size(); }
Component* AlertWindow::getCustomComponent (i32 index) const    { return customComps [index]; }

Component* AlertWindow::removeCustomComponent (i32k index)
{
    auto* c = getCustomComponent (index);

    if (c != nullptr)
    {
        customComps.removeFirstMatchingValue (c);
        allComps.removeFirstMatchingValue (c);
        removeChildComponent (c);

        updateLayout (false);
    }

    return c;
}

//==============================================================================
z0 AlertWindow::paint (Graphics& g)
{
    auto& lf = getLookAndFeel();
    lf.drawAlertBox (g, *this, textArea, textLayout);

    g.setColor (findColor (textColorId));
    g.setFont (lf.getAlertWindowFont());

    for (i32 i = textBoxes.size(); --i >= 0;)
    {
        auto* te = textBoxes.getUnchecked (i);

        g.drawFittedText (textboxNames[i],
                          te->getX(), te->getY() - 14,
                          te->getWidth(), 14,
                          Justification::centredLeft, 1);
    }

    for (i32 i = comboBoxNames.size(); --i >= 0;)
    {
        auto* cb = comboBoxes.getUnchecked (i);

        g.drawFittedText (comboBoxNames[i],
                          cb->getX(), cb->getY() - 14,
                          cb->getWidth(), 14,
                          Justification::centredLeft, 1);
    }

    for (auto* c : customComps)
        g.drawFittedText (c->getName(),
                          c->getX(), c->getY() - 14,
                          c->getWidth(), 14,
                          Justification::centredLeft, 1);
}

z0 AlertWindow::updateLayout (const b8 onlyIncreaseSize)
{
    i32k titleH = 24;
    i32k iconWidth = 80;

    auto& lf = getLookAndFeel();
    auto messageFont (lf.getAlertWindowMessageFont());

    auto wid = jmax (GlyphArrangement::getStringWidth (messageFont, text),
                     GlyphArrangement::getStringWidth (messageFont, getName()));

    auto sw = (i32) std::sqrt (messageFont.getHeight() * (f32) wid);
    auto w = jmin (300 + sw * 2, (i32) ((f32) getParentWidth() * 0.7f));
    i32k edgeGap = 10;
    i32k labelHeight = 18;
    i32 iconSpace = 0;

    AttributedString attributedText;
    attributedText.append (getName(), lf.getAlertWindowTitleFont());

    if (text.isNotEmpty())
        attributedText.append ("\n\n" + text, messageFont);

    attributedText.setColor (findColor (textColorId));

    if (alertIconType == NoIcon)
    {
        attributedText.setJustification (Justification::centredTop);
        textLayout.createLayoutWithBalancedLineLengths (attributedText, (f32) w);
    }
    else
    {
        attributedText.setJustification (Justification::topLeft);
        textLayout.createLayoutWithBalancedLineLengths (attributedText, (f32) w);
        iconSpace = iconWidth;
    }

    w = jmax (350, (i32) textLayout.getWidth() + iconSpace + edgeGap * 4);
    w = jmin (w, (i32) ((f32) getParentWidth() * 0.7f));

    auto textLayoutH = (i32) textLayout.getHeight();
    auto textBottom = 16 + titleH + textLayoutH;
    i32 h = textBottom;

    i32 buttonW = 40;

    for (auto* b : buttons)
        buttonW += 16 + b->getWidth();

    w = jmax (buttonW, w);

    h += (textBoxes.size() + comboBoxes.size() + progressBars.size()) * 50;

    if (auto* b = buttons[0])
        h += 20 + b->getHeight();

    for (auto* c : customComps)
    {
        w = jmax (w, (c->getWidth() * 100) / 80);
        h += 10 + c->getHeight();

        if (c->getName().isNotEmpty())
            h += labelHeight;
    }

    for (auto* tb : textBlocks)
        w = jmax (w, static_cast<const AlertTextComp*> (tb)->bestWidth);

    w = jmin (w, (i32) ((f32) getParentWidth() * 0.7f));

    for (auto* tb : textBlocks)
    {
        auto* ac = static_cast<AlertTextComp*> (tb);
        ac->updateLayout ((i32) ((f32) w * 0.8f));
        h += ac->getHeight() + 10;
    }

    h = jmin (getParentHeight() - 50, h);

    if (onlyIncreaseSize)
    {
        w = jmax (w, getWidth());
        h = jmax (h, getHeight());
    }

    if (! isVisible())
        centreAroundComponent (associatedComponent, w, h);
    else
        setBounds (getBounds().withSizeKeepingCentre (w, h));

    textArea.setBounds (edgeGap, edgeGap, w - (edgeGap * 2), h - edgeGap);
    accessibleMessageLabel.setBounds (textArea);

    i32k spacer = 16;
    i32 totalWidth = -spacer;

    for (auto* b : buttons)
        totalWidth += b->getWidth() + spacer;

    auto x = (w - totalWidth) / 2;
    auto y = (i32) ((f32) getHeight() * 0.95f);

    for (auto* c : buttons)
    {
        i32 ny = proportionOfHeight (0.95f) - c->getHeight();
        c->setTopLeftPosition (x, ny);
        if (ny < y)
            y = ny;

        x += c->getWidth() + spacer;

        c->toFront (false);
    }

    y = textBottom;

    for (auto* c : allComps)
    {
        h = 22;

        i32k comboIndex = comboBoxes.indexOf (dynamic_cast<ComboBox*> (c));
        if (comboIndex >= 0 && comboBoxNames [comboIndex].isNotEmpty())
            y += labelHeight;

        i32k tbIndex = textBoxes.indexOf (dynamic_cast<TextEditor*> (c));
        if (tbIndex >= 0 && textboxNames[tbIndex].isNotEmpty())
            y += labelHeight;

        if (customComps.contains (c))
        {
            if (c->getName().isNotEmpty())
                y += labelHeight;

            c->setTopLeftPosition (proportionOfWidth (0.1f), y);
            h = c->getHeight();
        }
        else if (textBlocks.contains (c))
        {
            c->setTopLeftPosition ((getWidth() - c->getWidth()) / 2, y);
            h = c->getHeight();
        }
        else
        {
            c->setBounds (proportionOfWidth (0.1f), y, proportionOfWidth (0.8f), h);
        }

        y += h + 10;
    }

    setWantsKeyboardFocus (getNumChildComponents() == 0);
}

b8 AlertWindow::containsAnyExtraComponents() const
{
    return allComps.size() > 0;
}

//==============================================================================
z0 AlertWindow::mouseDown (const MouseEvent& e)
{
    dragger.startDraggingComponent (this, e);
}

z0 AlertWindow::mouseDrag (const MouseEvent& e)
{
    dragger.dragComponent (this, e, &constrainer);
}

b8 AlertWindow::keyPressed (const KeyPress& key)
{
    for (auto* b : buttons)
    {
        if (b->isRegisteredForShortcut (key))
        {
            b->triggerClick();
            return true;
        }
    }

    if (key.isKeyCode (KeyPress::escapeKey) && escapeKeyCancels)
    {
        exitModalState (0);
        return true;
    }

    if (key.isKeyCode (KeyPress::returnKey) && buttons.size() == 1)
    {
        buttons.getUnchecked (0)->triggerClick();
        return true;
    }

    return false;
}

z0 AlertWindow::lookAndFeelChanged()
{
    i32k newFlags = getLookAndFeel().getAlertBoxWindowFlags();

    setUsingNativeTitleBar ((newFlags & ComponentPeer::windowHasTitleBar) != 0);
    setDropShadowEnabled (isOpaque() && (newFlags & ComponentPeer::windowHasDropShadow) != 0);
    updateLayout (false);
}

i32 AlertWindow::getDesktopWindowStyleFlags() const
{
    return getLookAndFeel().getAlertBoxWindowFlags();
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
z0 AlertWindow::showMessageBox (MessageBoxIconType iconType,
                                  const Txt& title,
                                  const Txt& message,
                                  const Txt& buttonText,
                                  Component* associatedComponent)
{
    show (MessageBoxOptions()
            .withIconType (iconType)
            .withTitle (title)
            .withMessage (message)
            .withButton (buttonText.isEmpty() ? TRANS ("OK") : buttonText)
            .withAssociatedComponent (associatedComponent));
}

i32 AlertWindow::show (const MessageBoxOptions& options)
{
    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
        return NativeMessageBox::show (options);

    return showAlertWindowUnmanaged (options, nullptr);
}

b8 AlertWindow::showNativeDialogBox (const Txt& title,
                                       const Txt& bodyText,
                                       b8 isOkCancel)
{
    if (isOkCancel)
        return NativeMessageBox::showOkCancelBox (AlertWindow::NoIcon, title, bodyText);

    NativeMessageBox::showMessageBox (AlertWindow::NoIcon, title, bodyText);
    return true;
}
#endif

z0 AlertWindow::showAsync (const MessageBoxOptions& options, ModalComponentManager::Callback* callback)
{
    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
        NativeMessageBox::showAsync (options, callback);
    else
        showAlertWindowUnmanaged (options, callback);
}

z0 AlertWindow::showAsync (const MessageBoxOptions& options, std::function<z0 (i32)> callback)
{
    showAsync (options, ModalCallbackFunction::create (callback));
}

z0 AlertWindow::showMessageBoxAsync (MessageBoxIconType iconType,
                                       const Txt& title,
                                       const Txt& message,
                                       const Txt& buttonText,
                                       Component* associatedComponent,
                                       ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsOk (iconType,
                                                     title,
                                                     message,
                                                     buttonText,
                                                     associatedComponent);
    showAsync (options, callback);
}

static i32 showMaybeAsync (const MessageBoxOptions& options,
                           ModalComponentManager::Callback* callbackIn)
{
    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
        return showNativeBoxUnmanaged (options, callbackIn, ResultCodeMappingMode::alertWindow);

    return showAlertWindowUnmanaged (options, callbackIn);
}

b8 AlertWindow::showOkCancelBox (MessageBoxIconType iconType,
                                   const Txt& title,
                                   const Txt& message,
                                   const Txt& button1Text,
                                   const Txt& button2Text,
                                   Component* associatedComponent,
                                   ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsOkCancel (iconType,
                                                           title,
                                                           message,
                                                           button1Text,
                                                           button2Text,
                                                           associatedComponent);
    return showMaybeAsync (options, callback) == 1;
}

i32 AlertWindow::showYesNoCancelBox (MessageBoxIconType iconType,
                                     const Txt& title,
                                     const Txt& message,
                                     const Txt& button1Text,
                                     const Txt& button2Text,
                                     const Txt& button3Text,
                                     Component* associatedComponent,
                                     ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsYesNoCancel (iconType,
                                                              title,
                                                              message,
                                                              button1Text,
                                                              button2Text,
                                                              button3Text,
                                                              associatedComponent);
    return showMaybeAsync (options, callback);
}

ScopedMessageBox AlertWindow::showScopedAsync (const MessageBoxOptions& options, std::function<z0 (i32)> callback)
{
    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
        return NativeMessageBox::showScopedAsync (options, std::move (callback));

    return detail::ConcreteScopedMessageBoxImpl::show (detail::AlertWindowHelpers::create (options), std::move (callback));
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> AlertWindow::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::dialogWindow);
}

} // namespace drx
