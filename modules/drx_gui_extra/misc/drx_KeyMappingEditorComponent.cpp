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

class KeyMappingEditorComponent::ChangeKeyButton final : public Button
{
public:
    ChangeKeyButton (KeyMappingEditorComponent& kec, CommandID command,
                     const Txt& keyName, i32 keyIndex)
        : Button (keyName),
          owner (kec),
          commandID (command),
          keyNum (keyIndex)
    {
        setWantsKeyboardFocus (false);
        setTriggeredOnMouseDown (keyNum >= 0);

        setTooltip (keyIndex < 0 ? TRANS ("Adds a new key-mapping")
                                 : TRANS ("Click to change this key-mapping"));
    }

    z0 paintButton (Graphics& g, b8 /*isOver*/, b8 /*isDown*/) override
    {
        getLookAndFeel().drawKeymapChangeButton (g, getWidth(), getHeight(), *this,
                                                 keyNum >= 0 ? getName() : Txt());
    }

    z0 clicked() override
    {
        if (keyNum >= 0)
        {
            Component::SafePointer<ChangeKeyButton> button (this);
            PopupMenu m;

            m.addItem (TRANS ("Change this key-mapping"),
                       [button]
                       {
                           if (button != nullptr)
                               button.getComponent()->assignNewKey();
                       });

            m.addSeparator();

            m.addItem (TRANS ("Remove this key-mapping"),
                       [button]
                       {
                           if (button != nullptr)
                               button->owner.getMappings().removeKeyPress (button->commandID,
                                                                           button->keyNum);
                       });

            m.showMenuAsync (PopupMenu::Options().withTargetComponent (this));
        }
        else
        {
            assignNewKey();  // + button pressed..
        }
    }

    using Button::clicked;

    z0 fitToContent (i32k h) noexcept
    {
        if (keyNum < 0)
        {
            setSize (h, h);
        }
        else
        {
            const auto idealWidth = GlyphArrangement::getStringWidthInt (withDefaultMetrics (FontOptions { (f32) h * 0.6f }), getName());
            setSize (jlimit (h * 4, h * 8, 6 + idealWidth), h);
        }
    }

    //==============================================================================
    class KeyEntryWindow final : public AlertWindow
    {
    public:
        KeyEntryWindow (KeyMappingEditorComponent& kec)
            : AlertWindow (TRANS ("New key-mapping"),
                           TRANS ("Please press a key combination now..."),
                           MessageBoxIconType::NoIcon),
              owner (kec)
        {
            addButton (TRANS ("OK"), 1);
            addButton (TRANS ("Cancel"), 0);

            // (avoid return + escape keys getting processed by the buttons..)
            for (auto* child : getChildren())
                child->setWantsKeyboardFocus (false);

            setWantsKeyboardFocus (true);
            grabKeyboardFocus();
        }

        b8 keyPressed (const KeyPress& key) override
        {
            lastPress = key;
            Txt message (TRANS ("Key") + ": " + owner.getDescriptionForKeyPress (key));

            auto previousCommand = owner.getMappings().findCommandForKeyPress (key);

            if (previousCommand != 0)
                message << "\n\n("
                        << TRANS ("Currently assigned to \"CMDN\"")
                            .replace ("CMDN", TRANS (owner.getCommandManager().getNameOfCommand (previousCommand)))
                        << ')';

            setMessage (message);
            return true;
        }

        b8 keyStateChanged (b8) override
        {
            return true;
        }

        KeyPress lastPress;

    private:
        KeyMappingEditorComponent& owner;

        DRX_DECLARE_NON_COPYABLE (KeyEntryWindow)
    };

    z0 setNewKey (const KeyPress& newKey, b8 dontAskUser)
    {
        if (newKey.isValid())
        {
            auto previousCommand = owner.getMappings().findCommandForKeyPress (newKey);

            if (previousCommand == 0 || dontAskUser)
            {
                owner.getMappings().removeKeyPress (newKey);

                if (keyNum >= 0)
                    owner.getMappings().removeKeyPress (commandID, keyNum);

                owner.getMappings().addKeyPress (commandID, newKey, keyNum);
            }
            else
            {
                auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                                       TRANS ("Change key-mapping"),
                                                                       TRANS ("This key is already assigned to the command \"CMDN\"")
                                                                           .replace ("CMDN", owner.getCommandManager().getNameOfCommand (previousCommand))
                                                                         + "\n\n"
                                                                         + TRANS ("Do you want to re-assign it to this new command instead?"),
                                                                       TRANS ("Re-assign"),
                                                                       TRANS ("Cancel"),
                                                                       this);
                messageBox = AlertWindow::showScopedAsync (options, [this, newKey] (i32 result)
                {
                    if (result != 0)
                        setNewKey (newKey, true);
                });
            }
        }
    }

    static z0 keyChosen (i32 result, ChangeKeyButton* button)
    {
        if (button != nullptr && button->currentKeyEntryWindow != nullptr)
        {
            if (result != 0)
            {
                button->currentKeyEntryWindow->setVisible (false);
                button->setNewKey (button->currentKeyEntryWindow->lastPress, false);
            }

            button->currentKeyEntryWindow.reset();
        }
    }

    z0 assignNewKey()
    {
        currentKeyEntryWindow.reset (new KeyEntryWindow (owner));
        currentKeyEntryWindow->enterModalState (true, ModalCallbackFunction::forComponent (keyChosen, this));
    }

private:
    KeyMappingEditorComponent& owner;
    const CommandID commandID;
    i32k keyNum;
    std::unique_ptr<KeyEntryWindow> currentKeyEntryWindow;
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChangeKeyButton)
};

//==============================================================================
class KeyMappingEditorComponent::ItemComponent final : public Component
{
public:
    ItemComponent (KeyMappingEditorComponent& kec, CommandID command)
        : owner (kec), commandID (command)
    {
        setInterceptsMouseClicks (false, true);

        const b8 isReadOnly = owner.isCommandReadOnly (commandID);

        auto keyPresses = owner.getMappings().getKeyPressesAssignedToCommand (commandID);

        for (i32 i = 0; i < jmin ((i32) maxNumAssignments, keyPresses.size()); ++i)
            addKeyPressButton (owner.getDescriptionForKeyPress (keyPresses.getReference (i)), i, isReadOnly);

        addKeyPressButton ("Change Key Mapping", -1, isReadOnly);
    }

    z0 addKeyPressButton (const Txt& desc, i32k index, const b8 isReadOnly)
    {
        auto* b = new ChangeKeyButton (owner, commandID, desc, index);
        keyChangeButtons.add (b);

        b->setEnabled (! isReadOnly);
        b->setVisible (keyChangeButtons.size() <= (i32) maxNumAssignments);
        addChildComponent (b);
    }

    z0 paint (Graphics& g) override
    {
        g.setFont ((f32) getHeight() * 0.7f);
        g.setColor (owner.findColor (KeyMappingEditorComponent::textColorId));

        g.drawFittedText (TRANS (owner.getCommandManager().getNameOfCommand (commandID)),
                          4, 0, jmax (40, getChildComponent (0)->getX() - 5), getHeight(),
                          Justification::centredLeft, true);
    }

    z0 resized() override
    {
        i32 x = getWidth() - 4;

        for (i32 i = keyChangeButtons.size(); --i >= 0;)
        {
            auto* b = keyChangeButtons.getUnchecked (i);

            b->fitToContent (getHeight() - 2);
            b->setTopRightPosition (x, 1);
            x = b->getX() - 5;
        }
    }

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    KeyMappingEditorComponent& owner;
    OwnedArray<ChangeKeyButton> keyChangeButtons;
    const CommandID commandID;

    enum { maxNumAssignments = 3 };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};

//==============================================================================
class KeyMappingEditorComponent::MappingItem final : public TreeViewItem
{
public:
    MappingItem (KeyMappingEditorComponent& kec, CommandID command)
        : owner (kec), commandID (command)
    {}

    Txt getUniqueName() const override                      { return Txt ((i32) commandID) + "_id"; }
    b8 mightContainSubItems() override                       { return false; }
    i32 getItemHeight() const override                         { return 20; }
    std::unique_ptr<Component> createItemComponent() override  { return std::make_unique<ItemComponent> (owner, commandID); }
    Txt getAccessibilityName() override                     { return TRANS (owner.getCommandManager().getNameOfCommand (commandID)); }

private:
    KeyMappingEditorComponent& owner;
    const CommandID commandID;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MappingItem)
};


//==============================================================================
class KeyMappingEditorComponent::CategoryItem final : public TreeViewItem
{
public:
    CategoryItem (KeyMappingEditorComponent& kec, const Txt& name)
        : owner (kec), categoryName (name)
    {}

    Txt getUniqueName() const override       { return categoryName + "_cat"; }
    b8 mightContainSubItems() override        { return true; }
    i32 getItemHeight() const override          { return 22; }
    Txt getAccessibilityName() override      { return categoryName; }

    z0 paintItem (Graphics& g, i32 width, i32 height) override
    {
        g.setFont (owner.withDefaultMetrics (FontOptions ((f32) height * 0.7f, Font::bold)));
        g.setColor (owner.findColor (KeyMappingEditorComponent::textColorId));

        g.drawText (TRANS (categoryName), 2, 0, width - 2, height, Justification::centredLeft, true);
    }

    z0 itemOpennessChanged (b8 isNowOpen) override
    {
        if (isNowOpen)
        {
            if (getNumSubItems() == 0)
                for (auto command : owner.getCommandManager().getCommandsInCategory (categoryName))
                    if (owner.shouldCommandBeIncluded (command))
                        addSubItem (new MappingItem (owner, command));
        }
        else
        {
            clearSubItems();
        }
    }

private:
    KeyMappingEditorComponent& owner;
    Txt categoryName;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CategoryItem)
};

//==============================================================================
class KeyMappingEditorComponent::TopLevelItem final : public TreeViewItem,
                                                      private ChangeListener
{
public:
    TopLevelItem (KeyMappingEditorComponent& kec)   : owner (kec)
    {
        setLinesDrawnForSubItems (false);
        owner.getMappings().addChangeListener (this);
    }

    ~TopLevelItem() override
    {
        owner.getMappings().removeChangeListener (this);
    }

    b8 mightContainSubItems() override             { return true; }
    Txt getUniqueName() const override            { return "keys"; }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        const OpennessRestorer opennessRestorer (*this);
        clearSubItems();

        for (auto category : owner.getCommandManager().getCommandCategories())
        {
            i32 count = 0;

            for (auto command : owner.getCommandManager().getCommandsInCategory (category))
                if (owner.shouldCommandBeIncluded (command))
                    ++count;

            if (count > 0)
                addSubItem (new CategoryItem (owner, category));
        }
    }

private:
    KeyMappingEditorComponent& owner;
};

//==============================================================================
KeyMappingEditorComponent::KeyMappingEditorComponent (KeyPressMappingSet& mappingManager,
                                                      const b8 showResetToDefaultButton)
    : mappings (mappingManager),
      resetButton (TRANS ("reset to defaults"))
{
    treeItem.reset (new TopLevelItem (*this));

    if (showResetToDefaultButton)
    {
        addAndMakeVisible (resetButton);

        resetButton.onClick = [this]
        {
            auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::QuestionIcon,
                                                                   TRANS ("Reset to defaults"),
                                                                   TRANS ("Are you sure you want to reset all the key-mappings to their default state?"),
                                                                   TRANS ("Reset"),
                                                                   {},
                                                                   this);
            messageBox = AlertWindow::showScopedAsync (options, [this] (i32 result)
            {
                if (result != 0)
                    getMappings().resetToDefaultMappings();
            });
        };
    }

    addAndMakeVisible (tree);
    tree.setTitle ("Key Mappings");
    tree.setColor (TreeView::backgroundColorId, findColor (backgroundColorId));
    tree.setRootItemVisible (false);
    tree.setDefaultOpenness (true);
    tree.setRootItem (treeItem.get());
    tree.setIndentSize (12);
}

KeyMappingEditorComponent::~KeyMappingEditorComponent()
{
    tree.setRootItem (nullptr);
}

//==============================================================================
z0 KeyMappingEditorComponent::setColors (Color mainBackground,
                                            Color textColor)
{
    setColor (backgroundColorId, mainBackground);
    setColor (textColorId, textColor);
    tree.setColor (TreeView::backgroundColorId, mainBackground);
}

z0 KeyMappingEditorComponent::parentHierarchyChanged()
{
    treeItem->changeListenerCallback (nullptr);
}

z0 KeyMappingEditorComponent::resized()
{
    i32 h = getHeight();

    if (resetButton.isVisible())
    {
        i32k buttonHeight = 20;
        h -= buttonHeight + 8;
        i32 x = getWidth() - 8;

        resetButton.changeWidthToFitText (buttonHeight);
        resetButton.setTopRightPosition (x, h + 6);
    }

    tree.setBounds (0, 0, getWidth(), h);
}

//==============================================================================
b8 KeyMappingEditorComponent::shouldCommandBeIncluded (const CommandID commandID)
{
    auto* ci = mappings.getCommandManager().getCommandForID (commandID);

    return ci != nullptr && (ci->flags & ApplicationCommandInfo::hiddenFromKeyEditor) == 0;
}

b8 KeyMappingEditorComponent::isCommandReadOnly (const CommandID commandID)
{
    auto* ci = mappings.getCommandManager().getCommandForID (commandID);

    return ci != nullptr && (ci->flags & ApplicationCommandInfo::readOnlyInKeyEditor) != 0;
}

Txt KeyMappingEditorComponent::getDescriptionForKeyPress (const KeyPress& key)
{
    return key.getTextDescription();
}

} // namespace drx
