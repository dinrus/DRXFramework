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

//==============================================================================
struct CustomMenuBarItemHolder final : public Component
{
    CustomMenuBarItemHolder (const ReferenceCountedObjectPtr<PopupMenu::CustomComponent>& customComponent)
    {
        setInterceptsMouseClicks (false, true);
        update (customComponent);
    }

    z0 update (const ReferenceCountedObjectPtr<PopupMenu::CustomComponent>& newComponent)
    {
        jassert (newComponent != nullptr);

        if (newComponent != custom)
        {
            if (custom != nullptr)
                removeChildComponent (custom.get());

            custom = newComponent;
            addAndMakeVisible (*custom);
            resized();
        }
    }

    z0 resized() override
    {
        custom->setBounds (getLocalBounds());
    }

    ReferenceCountedObjectPtr<PopupMenu::CustomComponent> custom;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomMenuBarItemHolder)
};

//==============================================================================
BurgerMenuComponent::BurgerMenuComponent (MenuBarModel* modelToUse)
{
    lookAndFeelChanged();
    listBox.addMouseListener (this, true);

    setModel (modelToUse);
    addAndMakeVisible (listBox);
}

BurgerMenuComponent::~BurgerMenuComponent()
{
    if (model != nullptr)
        model->removeListener (this);
}

z0 BurgerMenuComponent::setModel (MenuBarModel* newModel)
{
    if (newModel != model)
    {
        if (model != nullptr)
            model->removeListener (this);

        model = newModel;

        if (model != nullptr)
            model->addListener (this);

        refresh();
        listBox.updateContent();
    }
}

MenuBarModel* BurgerMenuComponent::getModel() const noexcept
{
    return model;
}

z0 BurgerMenuComponent::refresh()
{
    lastRowClicked = inputSourceIndexOfLastClick = -1;

    rows.clear();

    if (model != nullptr)
    {
        auto menuBarNames = model->getMenuBarNames();

        for (auto menuIdx = 0; menuIdx < menuBarNames.size(); ++menuIdx)
        {
            PopupMenu::Item menuItem;
            menuItem.text = menuBarNames[menuIdx];

            Txt ignore;
            auto menu = model->getMenuForIndex (menuIdx, ignore);

            rows.add (Row { true, menuIdx, menuItem });
            addMenuBarItemsForMenu (menu, menuIdx);
        }
    }
}

z0 BurgerMenuComponent::addMenuBarItemsForMenu (PopupMenu& menu, i32 menuIdx)
{
    for (PopupMenu::MenuItemIterator it (menu); it.next();)
    {
        auto& item = it.getItem();

        if (item.isSeparator)
            continue;

        if (hasSubMenu (item))
            addMenuBarItemsForMenu (*item.subMenu, menuIdx);
        else
            rows.add (Row {false, menuIdx, it.getItem()});
    }
}

i32 BurgerMenuComponent::getNumRows()
{
    return rows.size();
}

z0 BurgerMenuComponent::paint (Graphics& g)
{
    getLookAndFeel().drawPopupMenuBackground (g, getWidth(), getHeight());
}

z0 BurgerMenuComponent::paintListBoxItem (i32 rowIndex, Graphics& g, i32 w, i32 h, b8 highlight)
{
    auto& lf = getLookAndFeel();
    Rectangle<i32> r (w, h);

    auto row = (rowIndex < rows.size() ? rows.getReference (rowIndex)
                                       : Row { true, 0, {} });

    g.fillAll (findColor (PopupMenu::backgroundColorId));

    if (row.isMenuHeader)
    {
        lf.drawPopupMenuSectionHeader (g, r.reduced (20, 0), row.item.text);
        g.setColor (Colors::grey);
        g.fillRect (r.withHeight (1));
    }
    else
    {
        auto& item = row.item;
        auto* colour = item.colour != Color() ? &item.colour : nullptr;

        if (item.customComponent == nullptr)
            lf.drawPopupMenuItem (g, r.reduced (20, 0),
                                  item.isSeparator,
                                  item.isEnabled,
                                  highlight,
                                  item.isTicked,
                                  hasSubMenu (item),
                                  item.text,
                                  item.shortcutKeyDescription,
                                  item.image.get(),
                                  colour);
    }
}

b8 BurgerMenuComponent::hasSubMenu (const PopupMenu::Item& item)
{
    return item.subMenu != nullptr && (item.itemID == 0 || item.subMenu->getNumItems() > 0);
}

z0 BurgerMenuComponent::listBoxItemClicked (i32 rowIndex, const MouseEvent& e)
{
    auto row = rowIndex < rows.size() ? rows.getReference (rowIndex)
                                      : Row { true, 0, {} };

    if (! row.isMenuHeader)
    {
        lastRowClicked = rowIndex;
        inputSourceIndexOfLastClick = e.source.getIndex();
    }
}

Component* BurgerMenuComponent::refreshComponentForRow (i32 rowIndex, b8 isRowSelected, Component* existing)
{
    auto row = rowIndex < rows.size() ? rows.getReference (rowIndex)
                                      : Row { true, 0, {} };

    auto hasCustomComponent = (row.item.customComponent != nullptr);

    if (existing == nullptr && hasCustomComponent)
        return new CustomMenuBarItemHolder (row.item.customComponent);

    if (existing != nullptr)
    {
        auto* componentToUpdate = dynamic_cast<CustomMenuBarItemHolder*> (existing);
        jassert (componentToUpdate != nullptr);

        if (hasCustomComponent && componentToUpdate != nullptr)
        {
            row.item.customComponent->setHighlighted (isRowSelected);
            componentToUpdate->update (row.item.customComponent);
        }
        else
        {
            delete existing;
            existing = nullptr;
        }
    }

    return existing;
}

z0 BurgerMenuComponent::resized()
{
    listBox.setBounds (getLocalBounds());
}

z0 BurgerMenuComponent::menuBarItemsChanged (MenuBarModel* menuBarModel)
{
    setModel (menuBarModel);
}

z0 BurgerMenuComponent::menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo&)
{
}

z0 BurgerMenuComponent::mouseUp (const MouseEvent& event)
{
    auto rowIndex = listBox.getSelectedRow();

    if (rowIndex == lastRowClicked && rowIndex < rows.size()
         && event.source.getIndex() == inputSourceIndexOfLastClick)
    {
        auto& row = rows.getReference (rowIndex);

        if (! row.isMenuHeader)
        {
            listBox.selectRow (-1);

            lastRowClicked = -1;
            inputSourceIndexOfLastClick = -1;

            topLevelIndexClicked = row.topLevelMenuIndex;
            auto& item = row.item;

            if (auto* managerOfChosenCommand = item.commandManager)
            {
                ApplicationCommandTarget::InvocationInfo info (item.itemID);
                info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

                managerOfChosenCommand->invoke (info, true);
            }

            postCommandMessage (item.itemID);
        }
    }
}

z0 BurgerMenuComponent::handleCommandMessage (i32 commandID)
{
    if (model != nullptr)
    {
        model->menuItemSelected (commandID, topLevelIndexClicked);
        topLevelIndexClicked = -1;

        refresh();
        listBox.updateContent();
    }
}

z0 BurgerMenuComponent::lookAndFeelChanged()
{
    listBox.setRowHeight (roundToInt (getLookAndFeel().getPopupMenuFont().getHeight() * 2.0f));
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> BurgerMenuComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::menuBar);
}

} // namespace drx
