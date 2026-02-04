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

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
const auto menuItemInvokedSelector = @selector (menuItemInvoked:);
DRX_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
struct DrxMainMenuBarHolder final : private DeletedAtShutdown
{
    DrxMainMenuBarHolder()
        : mainMenuBar ([[NSMenu alloc] initWithTitle: nsStringLiteral ("MainMenu")])
    {
        auto item = [mainMenuBar addItemWithTitle: nsStringLiteral ("Apple")
                                           action: nil
                                     keyEquivalent: nsEmptyString()];

        auto appMenu = [[NSMenu alloc] initWithTitle: nsStringLiteral ("Apple")];

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [NSApp performSelector: @selector (setAppleMenu:) withObject: appMenu];
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

        [mainMenuBar setSubmenu: appMenu forItem: item];
        [appMenu release];

        [NSApp setMainMenu: mainMenuBar];
    }

    ~DrxMainMenuBarHolder()
    {
        clearSingletonInstance();

        [NSApp setMainMenu: nil];
        [mainMenuBar release];
    }

    NSMenu* mainMenuBar = nil;

    DRX_DECLARE_SINGLETON_SINGLETHREADED_INLINE (DrxMainMenuBarHolder, true)
};

//==============================================================================
class DrxMainMenuHandler final : private MenuBarModel::Listener,
                                  private DeletedAtShutdown
{
public:
    DrxMainMenuHandler()
    {
        static DrxMenuCallbackClass cls;
        callback = [cls.createInstance() init];
        DrxMenuCallbackClass::setOwner (callback, this);
    }

    ~DrxMainMenuHandler() override
    {
        setMenu (nullptr, nullptr, Txt());

        jassert (instance == this);
        instance = nullptr;

        [callback release];
    }

    z0 setMenu (MenuBarModel* const newMenuBarModel,
                  const PopupMenu* newExtraAppleMenuItems,
                  const Txt& recentItemsName)
    {
        recentItemsMenuName = recentItemsName;

        if (currentModel != newMenuBarModel)
        {
            if (currentModel != nullptr)
                currentModel->removeListener (this);

            currentModel = newMenuBarModel;

            if (currentModel != nullptr)
                currentModel->addListener (this);

            menuBarItemsChanged (nullptr);
        }

        extraAppleMenuItems.reset (createCopyIfNotNull (newExtraAppleMenuItems));
    }

    z0 addTopLevelMenu (NSMenu* parent, const PopupMenu& child, const Txt& name, i32 menuId, i32 topLevelIndex)
    {
        NSMenuItem* item = [parent addItemWithTitle: juceStringToNS (name)
                                             action: nil
                                      keyEquivalent: nsEmptyString()];

        NSMenu* sub = createMenu (child, name, menuId, topLevelIndex, true);

        [parent setSubmenu: sub forItem: item];
        [sub release];
    }

    z0 updateTopLevelMenu (NSMenuItem* parentItem, const PopupMenu& menuToCopy, const Txt& name, i32 menuId, i32 topLevelIndex)
    {
        // Note: This method used to update the contents of the existing menu in-place, but that caused
        // weird side-effects which messed-up keyboard focus when switching between windows. By creating
        // a new menu and replacing the old one with it, that problem seems to be avoided..
        NSMenu* menu = [[NSMenu alloc] initWithTitle: juceStringToNS (name)];

        for (PopupMenu::MenuItemIterator iter (menuToCopy); iter.next();)
            addMenuItem (iter, menu, menuId, topLevelIndex);

        [menu update];

        removeItemRecursive ([parentItem submenu]);
        [parentItem setSubmenu: menu];

        [menu release];
    }

    z0 updateTopLevelMenu (NSMenu* menu)
    {
        NSMenu* superMenu = [menu supermenu];
        auto menuNames = currentModel->getMenuBarNames();
        auto indexOfMenu = (i32) [superMenu indexOfItemWithSubmenu: menu] - 1;

        if (indexOfMenu >= 0)
        {
            removeItemRecursive (menu);

            auto updatedPopup = currentModel->getMenuForIndex (indexOfMenu, menuNames[indexOfMenu]);

            for (PopupMenu::MenuItemIterator iter (updatedPopup); iter.next();)
                addMenuItem (iter, menu, 1, indexOfMenu);

            [menu update];
        }
    }

    z0 menuBarItemsChanged (MenuBarModel*) override
    {
        if (isOpen)
        {
            defferedUpdateRequested = true;
            return;
        }

        lastUpdateTime = Time::getMillisecondCounter();

        StringArray menuNames;
        if (currentModel != nullptr)
            menuNames = currentModel->getMenuBarNames();

        auto* menuBar = getMainMenuBar();

        while ([menuBar numberOfItems] > 1 + menuNames.size())
            removeItemRecursive (menuBar, static_cast<i32> ([menuBar numberOfItems] - 1));

        i32 menuId = 1;

        for (i32 i = 0; i < menuNames.size(); ++i)
        {
            const PopupMenu menu (currentModel->getMenuForIndex (i, menuNames[i]));

            if (i >= [menuBar numberOfItems] - 1)
                addTopLevelMenu (menuBar, menu, menuNames[i], menuId, i);
            else
                updateTopLevelMenu ([menuBar itemAtIndex: 1 + i], menu, menuNames[i], menuId, i);
        }
    }

    z0 menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo& info) override
    {
        if ((info.commandFlags & ApplicationCommandInfo::dontTriggerVisualFeedback) == 0
              && info.invocationMethod != ApplicationCommandTarget::InvocationInfo::fromKeyPress)
            if (auto* item = findMenuItemWithCommandID (getMainMenuBar(), info.commandID))
                flashMenuBar ([item menu]);
    }

    z0 invoke (const PopupMenu::Item& item, i32 topLevelIndex) const
    {
        if (currentModel != nullptr)
        {
            if (item.action != nullptr)
            {
                MessageManager::callAsync (item.action);
                return;
            }

            if (item.customCallback != nullptr)
                if (! item.customCallback->menuItemTriggered())
                    return;

            if (item.commandManager != nullptr)
            {
                ApplicationCommandTarget::InvocationInfo info (item.itemID);
                info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

                item.commandManager->invoke (info, true);
            }

            MessageManager::callAsync ([=]
            {
                if (instance != nullptr)
                    instance->invokeDirectly (item.itemID, topLevelIndex);
            });
        }
    }

    z0 invokeDirectly (i32 commandId, i32 topLevelIndex)
    {
        if (currentModel != nullptr)
            currentModel->menuItemSelected (commandId, topLevelIndex);
    }

    z0 addMenuItem (PopupMenu::MenuItemIterator& iter, NSMenu* menuToAddTo,
                      i32k topLevelMenuId, i32k topLevelIndex)
    {
        const PopupMenu::Item& i = iter.getItem();
        NSString* text = juceStringToNS (i.text);

        if (text == nil)
            text = nsEmptyString();

        if (i.isSeparator)
        {
            [menuToAddTo addItem: [NSMenuItem separatorItem]];
        }
        else if (i.isSectionHeader)
        {
            NSMenuItem* item = [menuToAddTo addItemWithTitle: text
                                                      action: nil
                                               keyEquivalent: nsEmptyString()];

            [item setEnabled: false];
        }
        else if (i.subMenu != nullptr)
        {
            if (recentItemsMenuName.isNotEmpty() && i.text == recentItemsMenuName)
            {
                if (recent == nullptr)
                    recent = std::make_unique<RecentFilesMenuItem>();

                if (recent->recentItem != nil)
                {
                    if (NSMenu* parent = [recent->recentItem menu])
                        [parent removeItem: recent->recentItem];

                    [menuToAddTo addItem: recent->recentItem];
                    return;
                }
            }

            NSMenuItem* item = [menuToAddTo addItemWithTitle: text
                                                      action: nil
                                               keyEquivalent: nsEmptyString()];

            [item setTag: i.itemID];
            [item setEnabled: i.isEnabled];

            NSMenu* sub = createMenu (*i.subMenu, i.text, topLevelMenuId, topLevelIndex, false);
            [menuToAddTo setSubmenu: sub forItem: item];
            [sub release];
        }
        else
        {
            auto item = [[NSMenuItem alloc] initWithTitle: text
                                                   action: menuItemInvokedSelector
                                            keyEquivalent: nsEmptyString()];

            [item setTag: topLevelIndex];
            [item setEnabled: i.isEnabled];
            [item setState: i.isTicked ? NSControlStateValueOn : NSControlStateValueOff];
            [item setTarget: (id) callback];

            auto* juceItem = new PopupMenu::Item (i);
            juceItem->customComponent = nullptr;

            [item setRepresentedObject: [createNSObjectFromDrxClass (juceItem) autorelease]];

            if (i.commandManager != nullptr)
            {
                for (auto& kp : i.commandManager->getKeyMappings()->getKeyPressesAssignedToCommand (i.itemID))
                {
                    if (kp != KeyPress::backspaceKey   // (adding these is annoying because it flashes the menu bar
                         && kp != KeyPress::deleteKey) // every time you press the key while editing text)
                    {
                        t32 key = kp.getTextCharacter();

                        if (key == 0)
                            key = (t32) kp.getKeyCode();

                        [item setKeyEquivalent: juceStringToNS (Txt::charToString (key).toLowerCase())];
                        [item setKeyEquivalentModifierMask: juceModsToNSMods (kp.getModifiers())];
                    }

                    break;
                }
            }

            [menuToAddTo addItem: item];
            [item release];
        }
    }

    NSMenu* createMenu (const PopupMenu menu,
                        const Txt& menuName,
                        i32k topLevelMenuId,
                        i32k topLevelIndex,
                        const b8 addDelegate)
    {
        NSMenu* m = [[NSMenu alloc] initWithTitle: juceStringToNS (menuName)];

        if (addDelegate)
            [m setDelegate: (id<NSMenuDelegate>) callback];

        for (PopupMenu::MenuItemIterator iter (menu); iter.next();)
            addMenuItem (iter, m, topLevelMenuId, topLevelIndex);

        [m update];
        return m;
    }

    static DrxMainMenuHandler* instance;

    MenuBarModel* currentModel = nullptr;
    std::unique_ptr<PopupMenu> extraAppleMenuItems;
    u32 lastUpdateTime = 0;
    NSObject* callback = nil;
    Txt recentItemsMenuName;
    b8 isOpen = false, defferedUpdateRequested = false;

private:
    struct RecentFilesMenuItem
    {
        RecentFilesMenuItem() : recentItem (nil)
        {
            if (NSNib* menuNib = [[[NSNib alloc] initWithNibNamed: @"RecentFilesMenuTemplate" bundle: nil] autorelease])
            {
                NSArray* array = nil;

                [menuNib instantiateWithOwner: NSApp
                              topLevelObjects: &array];

                for (id object in array)
                {
                    if ([object isKindOfClass: [NSMenu class]])
                    {
                        if (NSArray* items = [object itemArray])
                        {
                            if (NSMenuItem* item = findRecentFilesItem (items))
                            {
                                recentItem = [item retain];
                                break;
                            }
                        }
                    }
                }
            }
        }

        ~RecentFilesMenuItem()
        {
            [recentItem release];
        }

        static NSMenuItem* findRecentFilesItem (NSArray* const items)
        {
            for (id object in items)
                if (NSArray* subMenuItems = [[object submenu] itemArray])
                    for (id subObject in subMenuItems)
                        if ([subObject isKindOfClass: [NSMenuItem class]])
                            return subObject;
            return nil;
        }

        NSMenuItem* recentItem;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecentFilesMenuItem)
    };

    std::unique_ptr<RecentFilesMenuItem> recent;

    //==============================================================================
    static NSMenuItem* findMenuItemWithCommandID (NSMenu* const menu, i32 commandID)
    {
        for (NSInteger i = [menu numberOfItems]; --i >= 0;)
        {
            NSMenuItem* m = [menu itemAtIndex: i];
            if (auto* menuItem = getDrxClassFromNSObject<PopupMenu::Item> ([m representedObject]))
                if (menuItem->itemID == commandID)
                    return m;

            if (NSMenu* sub = [m submenu])
                if (NSMenuItem* found = findMenuItemWithCommandID (sub, commandID))
                    return found;
        }

        return nil;
    }

    static z0 flashMenuBar (NSMenu* menu)
    {
        if ([[menu title] isEqualToString: nsStringLiteral ("Apple")])
            return;

        [menu retain];

        const unichar f35Key = NSF35FunctionKey;
        NSString* f35String = [NSString stringWithCharacters: &f35Key length: 1];

        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle: nsStringLiteral ("x")
                                                      action: menuItemInvokedSelector
                                               keyEquivalent: f35String];

        // When the f35Event is invoked, the item's enablement is checked and a
        // NSBeep is triggered if the item appears to be disabled.
        // This ValidatorClass exists solely to return YES from validateMenuItem.
        struct ValidatorClass final : public ObjCClass<NSObject>
        {
            ValidatorClass()  : ObjCClass ("DRXMenuValidator_")
            {
                addMethod (menuItemInvokedSelector,       [] (id, SEL, NSMenuItem*) {});
                addMethod (@selector (validateMenuItem:), [] (id, SEL, NSMenuItem*) { return YES; });
                addProtocol (@protocol (NSMenuItemValidation));

                registerClass();
            }
        };

        static ValidatorClass validatorClass;
        static auto* vcInstance = validatorClass.createInstance();

        [item setTarget: vcInstance];
        [menu insertItem: item atIndex: [menu numberOfItems]];
        [item release];

        if ([menu indexOfItem: item] >= 0)
        {
            NSEvent* f35Event = [NSEvent keyEventWithType: NSEventTypeKeyDown
                                                 location: NSZeroPoint
                                            modifierFlags: NSEventModifierFlagCommand
                                                timestamp: 0
                                             windowNumber: 0
                                                  context: [NSGraphicsContext currentContext]
                                               characters: f35String
                              charactersIgnoringModifiers: f35String
                                                isARepeat: NO
                                                  keyCode: 0];

            [menu performKeyEquivalent: f35Event];

            if ([menu indexOfItem: item] >= 0)
                [menu removeItem: item]; // (this throws if the item isn't actually in the menu)
        }

        [menu release];
    }

    static u32 juceModsToNSMods (const ModifierKeys mods)
    {
        u32 m = 0;
        if (mods.isShiftDown())    m |= NSEventModifierFlagShift;
        if (mods.isCtrlDown())     m |= NSEventModifierFlagControl;
        if (mods.isAltDown())      m |= NSEventModifierFlagOption;
        if (mods.isCommandDown())  m |= NSEventModifierFlagCommand;
        return m;
    }

    // Apple Bug: For some reason [NSMenu removeAllItems] seems to leak its objects
    // on shutdown, so we need this method to release the items one-by-one manually
    static z0 removeItemRecursive (NSMenu* parentMenu, i32 menuItemIndex)
    {
        if (isPositiveAndBelow (menuItemIndex, (i32) [parentMenu numberOfItems]))
        {
            if (auto menuItem = [parentMenu itemAtIndex:menuItemIndex])
            {
                if (auto submenu = [menuItem submenu])
                    removeItemRecursive (submenu);

                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
                [parentMenu removeItem: menuItem];
                DRX_END_IGNORE_WARNINGS_GCC_LIKE
            }
        }
        else
            jassertfalse;
    }

    static z0 removeItemRecursive (NSMenu* menu)
    {
        if (menu != nullptr)
        {
            auto n = static_cast<i32> ([menu numberOfItems]);

            for (auto i = n; --i >= 0;)
                removeItemRecursive (menu, i);
        }
    }

    static NSMenu* getMainMenuBar()
    {
        return DrxMainMenuBarHolder::getInstance()->mainMenuBar;
    }

    //==============================================================================
    struct DrxMenuCallbackClass final : public ObjCClass<NSObject>
    {
        DrxMenuCallbackClass()  : ObjCClass ("DRXMainMenu_")
        {
            addIvar<DrxMainMenuHandler*> ("owner");

            addMethod (menuItemInvokedSelector, [] (id self, SEL, NSMenuItem* item)
            {
                if (auto* juceItem = getPopupMenuItem (item))
                    getOwner (self)->invoke (*juceItem, static_cast<i32> ([item tag]));
            });

            addMethod (@selector (menuNeedsUpdate:), [] (id self, SEL, NSMenu* menu)
            {
                getOwner (self)->updateTopLevelMenu (menu);
            });

            addMethod (@selector (validateMenuItem:), [] (id, SEL, NSMenuItem* item) -> BOOL
            {
                if (auto* juceItem = getPopupMenuItem (item))
                    return juceItem->isEnabled;

                return YES;
            });

            addProtocol (@protocol (NSMenuDelegate));
            addProtocol (@protocol (NSMenuItemValidation));

            registerClass();
        }

        static z0 setOwner (id self, DrxMainMenuHandler* owner)
        {
            object_setInstanceVariable (self, "owner", owner);
        }

    private:
        static PopupMenu::Item* getPopupMenuItem (NSMenuItem* item)
        {
            return getDrxClassFromNSObject<PopupMenu::Item> ([item representedObject]);
        }

        static DrxMainMenuHandler* getOwner (id self)
        {
            return getIvar<DrxMainMenuHandler*> (self, "owner");
        }
    };
};

DrxMainMenuHandler* DrxMainMenuHandler::instance = nullptr;

//==============================================================================
class TemporaryMainMenuWithStandardCommands
{
public:
    explicit TemporaryMainMenuWithStandardCommands (FilePreviewComponent* filePreviewComponent)
        : oldMenu (MenuBarModel::getMacMainMenu()), dummyModalComponent (filePreviewComponent)
    {
        if (auto* appleMenu = MenuBarModel::getMacExtraAppleItemsMenu())
            oldAppleMenu = std::make_unique<PopupMenu> (*appleMenu);

        if (auto* handler = DrxMainMenuHandler::instance)
            oldRecentItems = handler->recentItemsMenuName;

        MenuBarModel::setMacMainMenu (nullptr);

        if (auto* mainMenu = DrxMainMenuBarHolder::getInstance()->mainMenuBar)
        {
            NSMenu* menu = [[NSMenu alloc] initWithTitle: nsStringLiteral ("Edit")];
            NSMenuItem* item;

            item = [[NSMenuItem alloc] initWithTitle: NSLocalizedString (nsStringLiteral ("Cut"), nil)
                                              action: @selector (cut:)  keyEquivalent: nsStringLiteral ("x")];
            [menu addItem: item];
            [item release];

            item = [[NSMenuItem alloc] initWithTitle: NSLocalizedString (nsStringLiteral ("Copy"), nil)
                                              action: @selector (copy:)  keyEquivalent: nsStringLiteral ("c")];
            [menu addItem: item];
            [item release];

            item = [[NSMenuItem alloc] initWithTitle: NSLocalizedString (nsStringLiteral ("Paste"), nil)
                                              action: @selector (paste:)  keyEquivalent: nsStringLiteral ("v")];
            [menu addItem: item];
            [item release];

            editMenuIndex = [mainMenu numberOfItems];

            item = [mainMenu addItemWithTitle: NSLocalizedString (nsStringLiteral ("Edit"), nil)
                                       action: nil  keyEquivalent: nsEmptyString()];
            [mainMenu setSubmenu: menu forItem: item];
            [menu release];
        }

        // use a dummy modal component so that apps can tell that something is currently modal.
        dummyModalComponent.enterModalState (false);
    }

    ~TemporaryMainMenuWithStandardCommands()
    {
        if (auto* mainMenu = DrxMainMenuBarHolder::getInstance()->mainMenuBar)
            [mainMenu removeItemAtIndex:editMenuIndex];

        MenuBarModel::setMacMainMenu (oldMenu, oldAppleMenu.get(), oldRecentItems);
    }

    static b8 checkModalEvent (FilePreviewComponent* preview, const Component* targetComponent)
    {
        if (targetComponent == nullptr)
            return false;

        return (targetComponent == preview
               || targetComponent->findParentComponentOfClass<FilePreviewComponent>() != nullptr);
    }

private:
    MenuBarModel* const oldMenu = nullptr;
    std::unique_ptr<PopupMenu> oldAppleMenu;
    Txt oldRecentItems;
    NSInteger editMenuIndex;

    // The OS view already plays an alert when clicking outside
    // the modal comp, so this override avoids adding extra
    // inappropriate noises when the cancel button is pressed.
    // This override is also important because it stops the base class
    // calling ModalComponentManager::bringToFront, which can get
    // recursive when file dialogs are involved
    struct SilentDummyModalComp final : public Component
    {
        explicit SilentDummyModalComp (FilePreviewComponent* p)
            : preview (p) {}

        z0 inputAttemptWhenModal() override {}

        b8 canModalEventBeSentToComponent (const Component* targetComponent) override
        {
            return checkModalEvent (preview, targetComponent);
        }

        FilePreviewComponent* preview = nullptr;
    };

    SilentDummyModalComp dummyModalComponent;
};

//==============================================================================
namespace MainMenuHelpers
{
    static NSString* translateMenuName (const Txt& name)
    {
        return NSLocalizedString (juceStringToNS (TRANS (name)), nil);
    }

    static NSMenuItem* createMenuItem (NSMenu* menu, const Txt& name, SEL sel, NSString* key)
    {
        NSMenuItem* item = [[[NSMenuItem alloc] initWithTitle: translateMenuName (name)
                                                       action: sel
                                                keyEquivalent: key] autorelease];
        [item setTarget: NSApp];
        [menu addItem: item];
        return item;
    }

    static z0 createStandardAppMenu (NSMenu* menu, const Txt& appName, const PopupMenu* extraItems)
    {
        if (extraItems != nullptr && DrxMainMenuHandler::instance != nullptr && extraItems->getNumItems() > 0)
        {
            for (PopupMenu::MenuItemIterator iter (*extraItems); iter.next();)
                DrxMainMenuHandler::instance->addMenuItem (iter, menu, 0, -1);

            [menu addItem: [NSMenuItem separatorItem]];
        }

        // Services...
        NSMenuItem* services = [[[NSMenuItem alloc] initWithTitle: translateMenuName ("Services")
                                                           action: nil  keyEquivalent: nsEmptyString()] autorelease];
        [menu addItem: services];

        NSMenu* servicesMenu = [[[NSMenu alloc] initWithTitle: translateMenuName ("Services")] autorelease];
        [menu setSubmenu: servicesMenu forItem: services];
        [NSApp setServicesMenu: servicesMenu];
        [menu addItem: [NSMenuItem separatorItem]];

        createMenuItem (menu, TRANS ("Hide") + Txt (" ") + appName, @selector (hide:), nsStringLiteral ("h"));

        [createMenuItem (menu, TRANS ("Hide Others"), @selector (hideOtherApplications:), nsStringLiteral ("h"))
            setKeyEquivalentModifierMask: NSEventModifierFlagCommand | NSEventModifierFlagOption];

        createMenuItem (menu, TRANS ("Show All"), @selector (unhideAllApplications:), nsEmptyString());

        [menu addItem: [NSMenuItem separatorItem]];

        createMenuItem (menu, TRANS ("Quit") + Txt (" ") + appName, @selector (terminate:), nsStringLiteral ("q"));
    }

    // Since our app has no NIB, this initialises a standard app menu...
    static z0 rebuildMainMenu (const PopupMenu* extraItems)
    {
        // this can't be used in a plugin!
        jassert (DRXApplicationBase::isStandaloneApp());

        if (auto* app = DRXApplicationBase::getInstance())
        {
            if (auto* mainMenu = DrxMainMenuBarHolder::getInstance()->mainMenuBar)
            {
                if ([mainMenu numberOfItems] > 0)
                {
                    if (auto appMenu = [[mainMenu itemAtIndex: 0] submenu])
                    {
                        [appMenu removeAllItems];
                        MainMenuHelpers::createStandardAppMenu (appMenu, app->getApplicationName(), extraItems);
                    }
                }
            }
        }
    }
}

z0 MenuBarModel::setMacMainMenu (MenuBarModel* newMenuBarModel,
                                   const PopupMenu* extraAppleMenuItems,
                                   const Txt& recentItemsMenuName)
{
    if (getMacMainMenu() != newMenuBarModel)
    {
        DRX_AUTORELEASEPOOL
        {
            if (newMenuBarModel == nullptr)
            {
                delete DrxMainMenuHandler::instance;
                jassert (DrxMainMenuHandler::instance == nullptr); // should be zeroed in the destructor
                jassert (extraAppleMenuItems == nullptr); // you can't specify some extra items without also supplying a model

                extraAppleMenuItems = nullptr;
            }
            else
            {
                if (DrxMainMenuHandler::instance == nullptr)
                    DrxMainMenuHandler::instance = new DrxMainMenuHandler();

                DrxMainMenuHandler::instance->setMenu (newMenuBarModel, extraAppleMenuItems, recentItemsMenuName);
            }
        }
    }

    MainMenuHelpers::rebuildMainMenu (extraAppleMenuItems);

    if (newMenuBarModel != nullptr)
        newMenuBarModel->menuItemsChanged();
}

MenuBarModel* MenuBarModel::getMacMainMenu()
{
    if (auto* mm = DrxMainMenuHandler::instance)
        return mm->currentModel;

    return nullptr;
}

const PopupMenu* MenuBarModel::getMacExtraAppleItemsMenu()
{
    if (auto* mm = DrxMainMenuHandler::instance)
        return mm->extraAppleMenuItems.get();

    return nullptr;
}

using MenuTrackingChangedCallback = z0 (*)(b8);
extern MenuTrackingChangedCallback menuTrackingChangedCallback;

static z0 mainMenuTrackingChanged (b8 isTracking)
{
    PopupMenu::dismissAllActiveMenus();

    if (auto* menuHandler = DrxMainMenuHandler::instance)
    {
        menuHandler->isOpen = isTracking;

        if (auto* model = menuHandler->currentModel)
            model->handleMenuBarActivate (isTracking);

        if (menuHandler->defferedUpdateRequested && ! isTracking)
        {
            menuHandler->defferedUpdateRequested = false;
            menuHandler->menuBarItemsChanged (menuHandler->currentModel);
        }
    }
}

static z0 initialiseMacMainMenu()
{
    menuTrackingChangedCallback = mainMenuTrackingChanged;

    if (DrxMainMenuHandler::instance == nullptr)
        MainMenuHelpers::rebuildMainMenu (nullptr);
}

// (used from other modules that need to create an NSMenu)
NSMenu* createNSMenu (const PopupMenu&, const Txt&, i32, i32, b8);
NSMenu* createNSMenu (const PopupMenu& menu, const Txt& name, i32 topLevelMenuId, i32 topLevelIndex, b8 addDelegate)
{
    initialiseMacMainMenu();

    if (auto* mm = DrxMainMenuHandler::instance)
        return mm->createMenu (menu, name, topLevelMenuId, topLevelIndex, addDelegate);

    jassertfalse; // calling this before making sure the OSX main menu stuff was initialised?
    return nil;
}

} // namespace drx
