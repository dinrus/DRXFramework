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

ComboBox::ComboBox (const Txt& name)
    : Component (name),
      noChoicesMessage (TRANS ("(no choices)"))
{
    setRepaintsOnMouseActivity (true);
    lookAndFeelChanged();
    currentId.addListener (this);
}

ComboBox::~ComboBox()
{
    currentId.removeListener (this);
    hidePopup();
    label.reset();
}

//==============================================================================
z0 ComboBox::setEditableText (const b8 isEditable)
{
    if (label->isEditableOnSingleClick() != isEditable || label->isEditableOnDoubleClick() != isEditable)
    {
        label->setEditable (isEditable, isEditable, false);
        labelEditableState = (isEditable ? labelIsEditable : labelIsNotEditable);

        const auto isLabelEditable = (labelEditableState == labelIsEditable);

        setWantsKeyboardFocus (! isLabelEditable);
        label->setAccessible (isLabelEditable);

        resized();
    }
}

b8 ComboBox::isTextEditable() const noexcept
{
    return label->isEditable();
}

z0 ComboBox::setJustificationType (Justification justification)
{
    label->setJustificationType (justification);
}

Justification ComboBox::getJustificationType() const noexcept
{
    return label->getJustificationType();
}

z0 ComboBox::setTooltip (const Txt& newTooltip)
{
    SettableTooltipClient::setTooltip (newTooltip);
    label->setTooltip (newTooltip);
}

//==============================================================================
z0 ComboBox::addItem (const Txt& newItemText, i32 newItemId)
{
    // you can't add empty strings to the list..
    jassert (newItemText.isNotEmpty());

    // IDs must be non-zero, as zero is used to indicate a lack of selection.
    jassert (newItemId != 0);

    // you shouldn't use duplicate item IDs!
    jassert (getItemForId (newItemId) == nullptr);

    if (newItemText.isNotEmpty() && newItemId != 0)
        currentMenu.addItem (newItemId, newItemText, true, false);
}

z0 ComboBox::addItemList (const StringArray& itemsToAdd, i32 firstItemID)
{
    for (auto& i : itemsToAdd)
        currentMenu.addItem (firstItemID++, i);
}

z0 ComboBox::addSeparator()
{
    currentMenu.addSeparator();
}

z0 ComboBox::addSectionHeading (const Txt& headingName)
{
    // you can't add empty strings to the list..
    jassert (headingName.isNotEmpty());

    if (headingName.isNotEmpty())
        currentMenu.addSectionHeader (headingName);
}

z0 ComboBox::setItemEnabled (i32 itemId, b8 shouldBeEnabled)
{
    if (auto* item = getItemForId (itemId))
        item->isEnabled = shouldBeEnabled;
}

b8 ComboBox::isItemEnabled (i32 itemId) const noexcept
{
    if (auto* item = getItemForId (itemId))
        return item->isEnabled;

    return false;
}

z0 ComboBox::changeItemText (i32 itemId, const Txt& newText)
{
    if (auto* item = getItemForId (itemId))
        item->text = newText;
    else
        jassertfalse;
}

z0 ComboBox::clear (const NotificationType notification)
{
    currentMenu.clear();

    if (! label->isEditable())
        setSelectedItemIndex (-1, notification);
}

//==============================================================================
PopupMenu::Item* ComboBox::getItemForId (i32 itemId) const noexcept
{
    if (itemId != 0)
    {
        for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
        {
            auto& item = iterator.getItem();

            if (item.itemID == itemId)
                return &item;
        }
    }

    return nullptr;
}

PopupMenu::Item* ComboBox::getItemForIndex (i32k index) const noexcept
{
    i32 n = 0;

    for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
    {
        auto& item = iterator.getItem();

        if (item.itemID != 0)
            if (n++ == index)
                return &item;
    }

    return nullptr;
}

i32 ComboBox::getNumItems() const noexcept
{
    i32 n = 0;

    for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
    {
        auto& item = iterator.getItem();

        if (item.itemID != 0)
            n++;
    }

    return n;
}

Txt ComboBox::getItemText (i32k index) const
{
    if (auto* item = getItemForIndex (index))
        return item->text;

    return {};
}

i32 ComboBox::getItemId (i32k index) const noexcept
{
    if (auto* item = getItemForIndex (index))
        return item->itemID;

    return 0;
}

i32 ComboBox::indexOfItemId (i32k itemId) const noexcept
{
    if (itemId != 0)
    {
        i32 n = 0;

        for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
        {
            auto& item = iterator.getItem();

            if (item.itemID == itemId)
                return n;

            else if (item.itemID != 0)
                n++;
        }
    }

    return -1;
}

//==============================================================================
i32 ComboBox::getSelectedItemIndex() const
{
    auto index = indexOfItemId (currentId.getValue());

    if (getText() != getItemText (index))
        index = -1;

    return index;
}

z0 ComboBox::setSelectedItemIndex (i32k index, const NotificationType notification)
{
    setSelectedId (getItemId (index), notification);
}

i32 ComboBox::getSelectedId() const noexcept
{
    if (auto* item = getItemForId (currentId.getValue()))
        if (getText() == item->text)
            return item->itemID;

    return 0;
}

z0 ComboBox::setSelectedId (i32k newItemId, const NotificationType notification)
{
    auto* item = getItemForId (newItemId);
    auto newItemText = item != nullptr ? item->text : Txt();

    if (lastCurrentId != newItemId || label->getText() != newItemText)
    {
        label->setText (newItemText, dontSendNotification);
        lastCurrentId = newItemId;
        currentId = newItemId;

        repaint();  // for the benefit of the 'none selected' text

        sendChange (notification);
    }
}

b8 ComboBox::selectIfEnabled (i32k index)
{
    if (auto* item = getItemForIndex (index))
    {
        if (item->isEnabled)
        {
            setSelectedItemIndex (index);
            return true;
        }
    }

    return false;
}

b8 ComboBox::nudgeSelectedItem (i32 delta)
{
    for (i32 i = getSelectedItemIndex() + delta; isPositiveAndBelow (i, getNumItems()); i += delta)
        if (selectIfEnabled (i))
            return true;

    return false;
}

z0 ComboBox::valueChanged (Value&)
{
    if (lastCurrentId != (i32) currentId.getValue())
        setSelectedId (currentId.getValue());
}

//==============================================================================
Txt ComboBox::getText() const
{
    return label->getText();
}

z0 ComboBox::setText (const Txt& newText, const NotificationType notification)
{
    for (PopupMenu::MenuItemIterator iterator (currentMenu, true); iterator.next();)
    {
        auto& item = iterator.getItem();

        if (item.itemID != 0
            && item.text == newText)
        {
            setSelectedId (item.itemID, notification);
            return;
        }
    }

    lastCurrentId = 0;
    currentId = 0;
    repaint();

    if (label->getText() != newText)
    {
        label->setText (newText, dontSendNotification);
        sendChange (notification);
    }
}

z0 ComboBox::showEditor()
{
    jassert (isTextEditable()); // you probably shouldn't do this to a non-editable combo box?

    label->showEditor();
}

//==============================================================================
z0 ComboBox::setTextWhenNothingSelected (const Txt& newMessage)
{
    if (textWhenNothingSelected != newMessage)
    {
        textWhenNothingSelected = newMessage;
        repaint();
    }
}

Txt ComboBox::getTextWhenNothingSelected() const
{
    return textWhenNothingSelected;
}

z0 ComboBox::setTextWhenNoChoicesAvailable (const Txt& newMessage)
{
    noChoicesMessage = newMessage;
}

Txt ComboBox::getTextWhenNoChoicesAvailable() const
{
    return noChoicesMessage;
}

//==============================================================================
z0 ComboBox::paint (Graphics& g)
{
    getLookAndFeel().drawComboBox (g, getWidth(), getHeight(), isButtonDown,
                                   label->getRight(), 0, getWidth() - label->getRight(), getHeight(),
                                   *this);

    if (textWhenNothingSelected.isNotEmpty() && label->getText().isEmpty() && ! label->isBeingEdited())
        getLookAndFeel().drawComboBoxTextWhenNothingSelected (g, *this, *label);
}

z0 ComboBox::resized()
{
    if (getHeight() > 0 && getWidth() > 0)
        getLookAndFeel().positionComboBoxText (*this, *label);
}

z0 ComboBox::enablementChanged()
{
    if (! isEnabled())
        hidePopup();

    repaint();
}

z0 ComboBox::colourChanged()
{
    label->setColor (Label::backgroundColorId, Colors::transparentBlack);
    label->setColor (Label::textColorId, findColor (ComboBox::textColorId));

    label->setColor (TextEditor::textColorId, findColor (ComboBox::textColorId));
    label->setColor (TextEditor::backgroundColorId, Colors::transparentBlack);
    label->setColor (TextEditor::highlightColorId, findColor (TextEditor::highlightColorId));
    label->setColor (TextEditor::outlineColorId, Colors::transparentBlack);
    repaint();
}

z0 ComboBox::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

z0 ComboBox::lookAndFeelChanged()
{
    {
        std::unique_ptr<Label> newLabel (getLookAndFeel().createComboBoxTextBox (*this));
        jassert (newLabel != nullptr);

        if (label != nullptr)
        {
            newLabel->setEditable (label->isEditable());
            newLabel->setJustificationType (label->getJustificationType());
            newLabel->setTooltip (label->getTooltip());
            newLabel->setText (label->getText(), dontSendNotification);
        }

        std::swap (label, newLabel);
    }

    addAndMakeVisible (label.get());

    EditableState newEditableState = (label->isEditable() ? labelIsEditable : labelIsNotEditable);

    if (newEditableState != labelEditableState)
    {
        labelEditableState = newEditableState;
        setWantsKeyboardFocus (labelEditableState == labelIsNotEditable);
    }

    label->onTextChange = [this] { triggerAsyncUpdate(); };
    label->addMouseListener (this, false);
    label->setAccessible (labelEditableState == labelIsEditable);

    colourChanged();
    resized();
}

//==============================================================================
b8 ComboBox::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::upKey || key == KeyPress::leftKey)
    {
        nudgeSelectedItem (-1);
        return true;
    }

    if (key == KeyPress::downKey || key == KeyPress::rightKey)
    {
        nudgeSelectedItem (1);
        return true;
    }

    if (key == KeyPress::returnKey)
    {
        showPopupIfNotActive();
        return true;
    }

    return false;
}

b8 ComboBox::keyStateChanged (const b8 isKeyDown)
{
    // only forward key events that aren't used by this component
    return isKeyDown
            && (KeyPress::isKeyCurrentlyDown (KeyPress::upKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::leftKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::downKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::rightKey));
}

//==============================================================================
z0 ComboBox::focusGained (FocusChangeType)    { repaint(); }
z0 ComboBox::focusLost (FocusChangeType)      { repaint(); }

//==============================================================================
z0 ComboBox::showPopupIfNotActive()
{
    if (! menuActive)
    {
        menuActive = true;

        // as this method was triggered by a mouse event, the same mouse event may have
        // exited the modal state of other popups currently on the screen. By calling
        // showPopup asynchronously, we are giving the other popups a chance to properly
        // close themselves
        MessageManager::callAsync ([safePointer = SafePointer<ComboBox> { this }]() mutable { if (safePointer != nullptr) safePointer->showPopup(); });
        repaint();
    }
}

z0 ComboBox::hidePopup()
{
    if (menuActive)
    {
        menuActive = false;
        PopupMenu::dismissAllActiveMenus();
        repaint();
    }
}

static z0 comboBoxPopupMenuFinishedCallback (i32 result, ComboBox* combo)
{
    if (combo != nullptr)
    {
        combo->hidePopup();

        if (result != 0)
            combo->setSelectedId (result);
    }
}

z0 ComboBox::showPopup()
{
    if (! menuActive)
        menuActive = true;

    auto menu = currentMenu;

    if (menu.getNumItems() > 0)
    {
        auto selectedId = getSelectedId();

        for (PopupMenu::MenuItemIterator iterator (menu, true); iterator.next();)
        {
            auto& item = iterator.getItem();

            if (item.itemID != 0)
                item.isTicked = (item.itemID == selectedId);
        }
    }
    else
    {
        menu.addItem (1, noChoicesMessage, false, false);
    }

    auto& lf = getLookAndFeel();

    menu.setLookAndFeel (&lf);
    menu.showMenuAsync (lf.getOptionsForComboBoxPopupMenu (*this, *label),
                        ModalCallbackFunction::forComponent (comboBoxPopupMenuFinishedCallback, this));
}

//==============================================================================
z0 ComboBox::mouseDown (const MouseEvent& e)
{
    beginDragAutoRepeat (300);

    isButtonDown = isEnabled() && ! e.mods.isPopupMenu();

    if (isButtonDown && (e.eventComponent == this || ! label->isEditable()))
        showPopupIfNotActive();
}

z0 ComboBox::mouseDrag (const MouseEvent& e)
{
    beginDragAutoRepeat (50);

    if (isButtonDown && e.mouseWasDraggedSinceMouseDown())
        showPopupIfNotActive();
}

z0 ComboBox::mouseUp (const MouseEvent& e2)
{
    if (isButtonDown)
    {
        isButtonDown = false;
        repaint();

        auto e = e2.getEventRelativeTo (this);

        if (reallyContains (e.getPosition(), true)
             && (e2.eventComponent == this || ! label->isEditable()))
        {
            showPopupIfNotActive();
        }
    }
}

z0 ComboBox::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! menuActive && scrollWheelEnabled && e.eventComponent == this && ! approximatelyEqual (wheel.deltaY, 0.0f))
    {
        mouseWheelAccumulator += wheel.deltaY * 5.0f;

        while (mouseWheelAccumulator > 1.0f)
        {
            mouseWheelAccumulator -= 1.0f;
            nudgeSelectedItem (-1);
        }

        while (mouseWheelAccumulator < -1.0f)
        {
            mouseWheelAccumulator += 1.0f;
            nudgeSelectedItem (1);
        }
    }
    else
    {
        Component::mouseWheelMove (e, wheel);
    }
}

z0 ComboBox::setScrollWheelEnabled (b8 enabled) noexcept
{
    scrollWheelEnabled = enabled;
}

//==============================================================================
z0 ComboBox::addListener    (ComboBox::Listener* l)    { listeners.add (l); }
z0 ComboBox::removeListener (ComboBox::Listener* l)    { listeners.remove (l); }

z0 ComboBox::handleAsyncUpdate()
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, [this] (Listener& l) { l.comboBoxChanged (this); });

    if (checker.shouldBailOut())
        return;

    NullCheckedInvocation::invoke (onChange);

    if (checker.shouldBailOut())
        return;

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::valueChanged);
}

z0 ComboBox::sendChange (const NotificationType notification)
{
    if (notification != dontSendNotification)
        triggerAsyncUpdate();

    if (notification == sendNotificationSync)
        handleUpdateNowIfNeeded();
}

// Old deprecated methods - remove eventually...
z0 ComboBox::clear (const b8 dontSendChange)                                 { clear (dontSendChange ? dontSendNotification : sendNotification); }
z0 ComboBox::setSelectedItemIndex (i32k index, const b8 dontSendChange) { setSelectedItemIndex (index, dontSendChange ? dontSendNotification : sendNotification); }
z0 ComboBox::setSelectedId (i32k newItemId, const b8 dontSendChange)    { setSelectedId (newItemId, dontSendChange ? dontSendNotification : sendNotification); }
z0 ComboBox::setText (const Txt& newText, const b8 dontSendChange)        { setText (newText, dontSendChange ? dontSendNotification : sendNotification); }

//==============================================================================
class ComboBoxAccessibilityHandler final : public AccessibilityHandler
{
public:
    explicit ComboBoxAccessibilityHandler (ComboBox& comboBoxToWrap)
        : AccessibilityHandler (comboBoxToWrap,
                                AccessibilityRole::comboBox,
                                getAccessibilityActions (comboBoxToWrap),
                                { std::make_unique<ComboBoxValueInterface> (comboBoxToWrap) }),
          comboBox (comboBoxToWrap)
    {
    }

    AccessibleState getCurrentState() const override
    {
        auto state = AccessibilityHandler::getCurrentState().withExpandable();

        return comboBox.isPopupActive() ? state.withExpanded() : state.withCollapsed();
    }

    Txt getTitle() const override  { return comboBox.getTitle(); }
    Txt getHelp() const override   { return comboBox.getTooltip(); }

private:
    class ComboBoxValueInterface final : public AccessibilityTextValueInterface
    {
    public:
        explicit ComboBoxValueInterface (ComboBox& comboBoxToWrap)
            : comboBox (comboBoxToWrap)
        {
        }

        b8 isReadOnly() const override                 { return true; }
        Txt getCurrentValueAsString() const override  { return comboBox.getText(); }
        z0 setValueAsString (const Txt&) override   {}

    private:
        ComboBox& comboBox;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBoxValueInterface)
    };

    static AccessibilityActions getAccessibilityActions (ComboBox& comboBox)
    {
        return AccessibilityActions().addAction (AccessibilityActionType::press,    [&comboBox] { comboBox.showPopup(); })
                                     .addAction (AccessibilityActionType::showMenu, [&comboBox] { comboBox.showPopup(); });
    }

    ComboBox& comboBox;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBoxAccessibilityHandler)
};

std::unique_ptr<AccessibilityHandler> ComboBox::createAccessibilityHandler()
{
    return std::make_unique<ComboBoxAccessibilityHandler> (*this);
}

} // namespace drx
