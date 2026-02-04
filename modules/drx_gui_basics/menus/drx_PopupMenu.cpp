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

// Some things to keep in mind when modifying this file:
// - Popup menus may be free-floating or parented. Make sure to test both!
// - Menus may open while the mouse button is down, in which case the following mouse-up may
//   trigger a hovered menu item if the mouse has moved since the menu was displayed.
// - Consider a i64 menu attached to a button. It's possible for a such a menu to open underneath
//   the mouse cursor. In this case, the menu item underneath the mouse should *not* be initially
//   selected or clickable. Instead, wait until the mouse cursor is moved, which we interpret as the
//   user signalling intent to trigger a menu item.
// - Menu items may be navigated with the cursor keys. The most recent input mechanism should
//   generally win, so pressing a cursor key should cause the mouse state to be ignored until
//   the mouse is next moved.
// - It's possible for menus to overlap, especially in the case of nested submenus. Of course,
//   clicking an overlapping menu should only trigger the topmost menu item.
// - Long menus must update properly when the mouse is completely stationary inside the scroll area
//   at the end of the menu. This means it's not sufficient to drive all menu updates from mouse
//   and keyboard input callbacks. Scrolling must be driven by some other periodic update mechanism
//   such as a timer.

namespace drx
{

namespace PopupMenuSettings
{
    i32k scrollZone = 24;
    i32k dismissCommandId = 0x6287345f;

    static b8 menuWasHiddenBecauseOfAppChange = false;
}

//==============================================================================
struct PopupMenu::HelperClasses
{

class MouseSourceState;
struct MenuWindow;

static b8 canBeTriggered (const PopupMenu::Item& item) noexcept
{
    return item.isEnabled
        && item.itemID != 0
        && ! item.isSectionHeader
        && (item.customComponent == nullptr || item.customComponent->isTriggeredAutomatically());
}

static b8 hasActiveSubMenu (const PopupMenu::Item& item) noexcept
{
    return item.isEnabled
        && item.subMenu != nullptr
        && item.subMenu->items.size() > 0;
}

//==============================================================================
struct HeaderItemComponent final : public PopupMenu::CustomComponent
{
    HeaderItemComponent (const Txt& name, const Options& opts)
        : CustomComponent (false), options (opts)
    {
        setName (name);
    }

    z0 paint (Graphics& g) override
    {
        getLookAndFeel().drawPopupMenuSectionHeaderWithOptions (g,
                                                                getLocalBounds(),
                                                                getName(),
                                                                options);
    }

    z0 getIdealSize (i32& idealWidth, i32& idealHeight) override
    {
        getLookAndFeel().getIdealPopupMenuSectionHeaderSizeWithOptions (getName(),
                                                                        -1,
                                                                        idealWidth,
                                                                        idealHeight,
                                                                        options);
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    const Options& options;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderItemComponent)
};

//==============================================================================
struct ItemComponent final : public Component
{
    ItemComponent (const PopupMenu::Item& i, const PopupMenu::Options& o, MenuWindow& parent)
        : item (i), parentWindow (parent), options (o), customComp (i.customComponent)
    {
        if (item.isSectionHeader)
            customComp = *new HeaderItemComponent (item.text, options);

        if (customComp != nullptr)
        {
            setItem (*customComp, &item);
            addAndMakeVisible (*customComp);
        }

        parent.addAndMakeVisible (this);

        updateShortcutKeyDescription();

        i32 itemW = 80;
        i32 itemH = 16;
        getIdealSize (itemW, itemH, options.getStandardItemHeight());
        setSize (itemW, jlimit (1, 600, itemH));

        addMouseListener (&parent, false);
    }

    ~ItemComponent() override
    {
        if (customComp != nullptr)
            setItem (*customComp, nullptr);

        removeChildComponent (customComp.get());
    }

    z0 getIdealSize (i32& idealWidth, i32& idealHeight, i32k standardItemHeight)
    {
        if (customComp != nullptr)
            customComp->getIdealSize (idealWidth, idealHeight);
        else
            getLookAndFeel().getIdealPopupMenuItemSizeWithOptions (getTextForMeasurement(),
                                                                   item.isSeparator,
                                                                   standardItemHeight,
                                                                   idealWidth, idealHeight,
                                                                   options);
    }

    z0 paint (Graphics& g) override
    {
        if (customComp == nullptr)
            getLookAndFeel().drawPopupMenuItemWithOptions (g, getLocalBounds(),
                                                           isHighlighted,
                                                           item,
                                                           options);
    }

    z0 resized() override
    {
        if (auto* child = getChildComponent (0))
        {
            const auto border = getLookAndFeel().getPopupMenuBorderSizeWithOptions (options);
            child->setBounds (getLocalBounds().reduced (border, 0));
        }
    }

    z0 setHighlighted (b8 shouldBeHighlighted)
    {
        shouldBeHighlighted = shouldBeHighlighted && item.isEnabled;

        if (isHighlighted != shouldBeHighlighted)
        {
            isHighlighted = shouldBeHighlighted;

            if (customComp != nullptr)
                customComp->setHighlighted (shouldBeHighlighted);

            if (isHighlighted)
                if (auto* handler = getAccessibilityHandler())
                    handler->grabFocus();

            repaint();
        }
    }

    static b8 isAccessibilityHandlerRequired (const PopupMenu::Item& item)
    {
        return item.isSectionHeader || hasActiveSubMenu (item) || canBeTriggered (item);
    }

    PopupMenu::Item item;

private:
    //==============================================================================
    class ItemAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        explicit ItemAccessibilityHandler (ItemComponent& itemComponentToWrap)
            : AccessibilityHandler (itemComponentToWrap,
                                    isAccessibilityHandlerRequired (itemComponentToWrap.item) ? AccessibilityRole::menuItem
                                                                                              : AccessibilityRole::ignored,
                                    getAccessibilityActions (*this, itemComponentToWrap)),
              itemComponent (itemComponentToWrap)
        {
        }

        Txt getTitle() const override
        {
            return itemComponent.item.text;
        }

        AccessibleState getCurrentState() const override
        {
            auto state = AccessibilityHandler::getCurrentState().withSelectable()
                                                                .withAccessibleOffscreen();

            if (hasActiveSubMenu (itemComponent.item))
            {
                state = itemComponent.parentWindow.isSubMenuVisible() ? state.withExpandable().withExpanded()
                                                                      : state.withExpandable().withCollapsed();
            }

            if (itemComponent.item.isTicked)
                state = state.withCheckable().withChecked();

            return state.isFocused() ? state.withSelected() : state;
        }

    private:
        static AccessibilityActions getAccessibilityActions (ItemAccessibilityHandler& handler,
                                                             ItemComponent& item)
        {
            auto onFocus = [&item]
            {
                item.parentWindow.disableMouseMovesOnMenuAndAncestors();
                item.parentWindow.ensureItemComponentIsVisible (item, -1);
                item.parentWindow.setCurrentlyHighlightedChild (&item);
            };

            auto onToggle = [&handler, &item, onFocus]
            {
                if (handler.getCurrentState().isSelected())
                    item.parentWindow.setCurrentlyHighlightedChild (nullptr);
                else
                    onFocus();
            };

            auto actions = AccessibilityActions().addAction (AccessibilityActionType::focus,  std::move (onFocus))
                                                 .addAction (AccessibilityActionType::toggle, std::move (onToggle));

            if (canBeTriggered (item.item))
            {
                actions.addAction (AccessibilityActionType::press, [&item]
                {
                    item.parentWindow.setCurrentlyHighlightedChild (&item);
                    item.parentWindow.triggerCurrentlyHighlightedItem();
                });
            }

            if (hasActiveSubMenu (item.item))
            {
                auto showSubMenu = [&item]
                {
                    item.parentWindow.showSubMenuFor (&item);

                    if (auto* subMenu = item.parentWindow.activeSubMenu.get())
                        subMenu->setCurrentlyHighlightedChild (subMenu->items.getFirst());
                };

                actions.addAction (AccessibilityActionType::press,    showSubMenu);
                actions.addAction (AccessibilityActionType::showMenu, showSubMenu);
            }

            return actions;
        }

        ItemComponent& itemComponent;
    };

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return item.isSeparator ? createIgnoredAccessibilityHandler (*this)
                                : std::make_unique<ItemAccessibilityHandler> (*this);
    }

    //==============================================================================
    MenuWindow& parentWindow;
    const PopupMenu::Options& options;
    // NB: we use a copy of the one from the item info in case we're using our own section comp
    ReferenceCountedObjectPtr<CustomComponent> customComp;
    b8 isHighlighted = false;

    z0 updateShortcutKeyDescription()
    {
        if (item.commandManager != nullptr
             && item.itemID != 0
             && item.shortcutKeyDescription.isEmpty())
        {
            Txt shortcutKey;

            for (auto& keypress : item.commandManager->getKeyMappings()
                                    ->getKeyPressesAssignedToCommand (item.itemID))
            {
                auto key = keypress.getTextDescriptionWithIcons();

                if (shortcutKey.isNotEmpty())
                    shortcutKey << ", ";

                if (key.length() == 1 && key[0] < 128)
                    shortcutKey << "shortcut: '" << key << '\'';
                else
                    shortcutKey << key;
            }

            item.shortcutKeyDescription = shortcutKey.trim();
        }
    }

    Txt getTextForMeasurement() const
    {
        return item.shortcutKeyDescription.isNotEmpty() ? item.text + "   " + item.shortcutKeyDescription
                                                        : item.text;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};

//==============================================================================
struct MenuWindow final : public Component
{
    MenuWindow (const PopupMenu& menu,
                MenuWindow* parentWindow,
                Options opts,
                b8 alignToRectangle,
                ApplicationCommandManager** manager,
                f32 parentScaleFactor = 1.0f)
        : Component ("menu"),
          parent (parentWindow),
          options (opts.withParentComponent (findNonNullLookAndFeel (menu, parentWindow).getParentComponentForMenuOptions (opts))),
          managerOfChosenCommand (manager),
          componentAttachedTo (options.getTargetComponent()),
          windowCreationTime (Time::getMillisecondCounter()),
          lastFocusedTime (windowCreationTime),
          timeEnteredCurrentChildComp (windowCreationTime),
          scaleFactor (parentWindow != nullptr ? parentScaleFactor : 1.0f)
    {
        setWantsKeyboardFocus (false);
        setMouseClickGrabsKeyboardFocus (false);
        setAlwaysOnTop (true);
        setFocusContainerType (FocusContainerType::focusContainer);

        setLookAndFeel (findLookAndFeel (menu, parentWindow));

        auto& lf = getLookAndFeel();

        if (auto* pc = options.getParentComponent())
        {
            pc->addChildComponent (this);
        }
        else
        {
            const auto shouldDisableAccessibility = [this]
            {
                const auto* compToCheck = parent != nullptr ? parent
                                                            : options.getTargetComponent();

                return compToCheck != nullptr && ! compToCheck->isAccessible();
            }();

            if (shouldDisableAccessibility)
                setAccessible (false);

            addToDesktop (ComponentPeer::windowIsTemporary
                          | ComponentPeer::windowIgnoresKeyPresses
                          | lf.getMenuWindowFlags());
        }

        // Using a global mouse listener means that we get notifications about all mouse events.
        // Without this, drags that are started on a button that displays a menu won't reach the
        // menu, because they *only* target the component that initiated the drag interaction.
        Desktop::getInstance().addGlobalMouseListener (this);

        if (options.getParentComponent() == nullptr && parentWindow == nullptr && lf.shouldPopupMenuScaleWithTargetComponent (options))
            if (auto* targetComponent = options.getTargetComponent())
                scaleFactor = Component::getApproximateScaleFactorForComponent (targetComponent);

        setOpaque (lf.findColor (PopupMenu::backgroundColorId).isOpaque()
                     || ! Desktop::canUseSemiTransparentWindows());

        const auto initialSelectedId = options.getInitiallySelectedItemId();

        for (i32 i = 0; i < menu.items.size(); ++i)
        {
            auto& item = menu.items.getReference (i);

            if (i + 1 < menu.items.size() || ! item.isSeparator)
            {
                auto* child = items.add (new ItemComponent (item, options, *this));
                child->setExplicitFocusOrder (1 + i);

                if (initialSelectedId != 0 && item.itemID == initialSelectedId)
                    setCurrentlyHighlightedChild (child);
            }
        }

        auto targetArea = options.getTargetScreenArea() / scaleFactor;

        calculateWindowPos (targetArea, alignToRectangle);
        setTopLeftPosition (windowPos.getPosition());

        if (auto visibleID = options.getItemThatMustBeVisible())
        {
            for (auto* item : items)
            {
                if (item->item.itemID == visibleID)
                {
                    const auto targetPosition = [&]
                    {
                        if (auto* pc = options.getParentComponent())
                            return pc->getLocalPoint (nullptr, targetArea.getTopLeft());

                        return targetArea.getTopLeft();
                    }();

                    auto y = targetPosition.getY() - windowPos.getY();
                    ensureItemComponentIsVisible (*item, isPositiveAndBelow (y, windowPos.getHeight()) ? y : -1);

                    break;
                }
            }
        }

        resizeToBestWindowPos();

        getActiveWindows().add (this);
        lf.preparePopupMenuWindow (*this);

        getMouseState (Desktop::getInstance().getMainMouseSource()); // forces creation of a mouse source watcher for the main mouse
    }

    ~MenuWindow() override
    {
        getActiveWindows().removeFirstMatchingValue (this);
        Desktop::getInstance().removeGlobalMouseListener (this);
        activeSubMenu.reset();
        items.clear();
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        if (isOpaque())
            g.fillAll (Colors::white);

        auto& theme = getLookAndFeel();
        theme.drawPopupMenuBackgroundWithOptions (g, getWidth(), getHeight(), options);

        if (columnWidths.isEmpty())
            return;

        const auto separatorWidth = theme.getPopupMenuColumnSeparatorWidthWithOptions (options);
        const auto border = theme.getPopupMenuBorderSizeWithOptions (options);

        auto currentX = 0;

        std::for_each (columnWidths.begin(), std::prev (columnWidths.end()), [&] (i32 width)
        {
            const Rectangle<i32> separator (currentX + width,
                                            border,
                                            separatorWidth,
                                            getHeight() - border * 2);
            theme.drawPopupMenuColumnSeparatorWithOptions (g, separator, options);
            currentX += width + separatorWidth;
        });
    }

    z0 paintOverChildren (Graphics& g) override
    {
        auto& lf = getLookAndFeel();

        if (options.getParentComponent())
            lf.drawResizableFrame (g, getWidth(), getHeight(),
                                   BorderSize<i32> (getLookAndFeel().getPopupMenuBorderSizeWithOptions (options)));

        if (canScroll())
        {
            if (isTopScrollZoneActive())
            {
                lf.drawPopupMenuUpDownArrowWithOptions (g,
                                                        getWidth(),
                                                        PopupMenuSettings::scrollZone,
                                                        true,
                                                        options);
            }

            if (isBottomScrollZoneActive())
            {
                g.setOrigin (0, getHeight() - PopupMenuSettings::scrollZone);
                lf.drawPopupMenuUpDownArrowWithOptions (g,
                                                        getWidth(),
                                                        PopupMenuSettings::scrollZone,
                                                        false,
                                                        options);
            }
        }
    }

    //==============================================================================
    // hide this and all sub-comps
    z0 hide (const PopupMenu::Item* item, b8 makeInvisible)
    {
        if (isVisible())
        {
            WeakReference<Component> deletionChecker (this);

            activeSubMenu.reset();
            currentChild = nullptr;

            if (item != nullptr
                 && item->commandManager != nullptr
                 && item->itemID != 0)
            {
                *managerOfChosenCommand = item->commandManager;
            }

            auto resultID = options.hasWatchedComponentBeenDeleted() ? 0 : getResultItemID (item);

            exitModalState (resultID);

            if (deletionChecker != nullptr)
            {
                exitingModalState = true;

                if (makeInvisible)
                    setVisible (false);
            }

            if (resultID != 0
                 && item != nullptr
                 && item->action != nullptr)
                MessageManager::callAsync (item->action);
        }
    }

    static i32 getResultItemID (const PopupMenu::Item* item)
    {
        if (item == nullptr)
            return 0;

        if (auto* cc = item->customCallback.get())
            if (! cc->menuItemTriggered())
                return 0;

        return item->itemID;
    }

    z0 dismissMenu (const PopupMenu::Item* item)
    {
        if (parent != nullptr)
        {
            parent->dismissMenu (item);
        }
        else
        {
            if (item != nullptr)
            {
                // need a copy of this on the stack as the one passed in will get deleted during this call
                auto mi (*item);
                hide (&mi, false);
            }
            else
            {
                hide (nullptr, true);
            }
        }
    }

    f32 getDesktopScaleFactor() const override    { return scaleFactor * Desktop::getInstance().getGlobalScaleFactor(); }

    z0 visibilityChanged() override
    {
        if (! isShowing())
            return;

        auto* accessibleFocus = [this]
        {
          if (currentChild != nullptr)
              if (auto* childHandler = currentChild->getAccessibilityHandler())
                  return childHandler;

            return getAccessibilityHandler();
        }();

        if (accessibleFocus != nullptr)
            accessibleFocus->grabFocus();
    }

    //==============================================================================
    b8 keyPressed (const KeyPress& key) override
    {
        if (key.isKeyCode (KeyPress::downKey))
        {
            selectNextItem (MenuSelectionDirection::forwards);
        }
        else if (key.isKeyCode (KeyPress::upKey))
        {
            selectNextItem (MenuSelectionDirection::backwards);
        }
        else if (key.isKeyCode (KeyPress::leftKey))
        {
            if (parent != nullptr)
            {
                Component::SafePointer<MenuWindow> parentWindow (parent);
                ItemComponent* currentChildOfParent = parentWindow->currentChild;

                hide (nullptr, true);

                if (parentWindow != nullptr)
                    parentWindow->setCurrentlyHighlightedChild (currentChildOfParent);

                disableMouseMovesOnMenuAndAncestors();
            }
            else if (componentAttachedTo != nullptr)
            {
                componentAttachedTo->keyPressed (key);
            }
        }
        else if (key.isKeyCode (KeyPress::rightKey))
        {
            disableMouseMovesOnMenuAndAncestors();

            if (showSubMenuFor (currentChild))
            {
                if (isSubMenuVisible())
                    activeSubMenu->selectNextItem (MenuSelectionDirection::current);
            }
            else if (componentAttachedTo != nullptr)
            {
                componentAttachedTo->keyPressed (key);
            }
        }
        else if (key.isKeyCode (KeyPress::returnKey) || key.isKeyCode (KeyPress::spaceKey))
        {
            triggerCurrentlyHighlightedItem();
        }
        else if (key.isKeyCode (KeyPress::escapeKey))
        {
            dismissMenu (nullptr);
        }
        else
        {
            return false;
        }

        return true;
    }

    z0 inputAttemptWhenModal() override
    {
        WeakReference<Component> deletionChecker (this);

        for (auto* ms : mouseSourceStates)
        {
            ms->handleMouseEventWithPosition (ms->source.getScreenPosition().roundToInt());

            if (deletionChecker == nullptr)
                return;
        }

        if (! isOverAnyMenu())
        {
            if (componentAttachedTo != nullptr)
            {
                // we want to dismiss the menu, but if we do it synchronously, then
                // the mouse-click will be allowed to pass through. That's good, except
                // when the user clicks on the button that originally popped the menu up,
                // as they'll expect the menu to go away, and in fact it'll just
                // come back. So only dismiss synchronously if they're not on the original
                // comp that we're attached to.
                auto mousePos = componentAttachedTo->getMouseXYRelative();

                if (componentAttachedTo->reallyContains (mousePos, true))
                {
                    postCommandMessage (PopupMenuSettings::dismissCommandId); // dismiss asynchronously
                    return;
                }
            }

            dismissMenu (nullptr);
        }
    }

    z0 handleCommandMessage (i32 commandId) override
    {
        Component::handleCommandMessage (commandId);

        if (commandId == PopupMenuSettings::dismissCommandId)
            dismissMenu (nullptr);
    }

    //==============================================================================
    z0 mouseDown  (const MouseEvent& e) override    { handleMouseEvent (e); }

    z0 mouseUp (const MouseEvent& e) override
    {
        SafePointer self { this };

        handleMouseEvent (e);

        // Check whether this menu was deleted as a result of the mouse being released.
        if (self == nullptr)
            return;

        // If the mouse was down when the menu was created, releasing the mouse should
        // not trigger the item under the mouse, because we might still be handling the click
        // that caused the menu to show in the first place. Once the mouse has been released once,
        // then the user must have clicked the mouse again, so they are attempting to trigger or
        // dismiss the menu.
        mouseUpCanTrigger |= true;
    }

    // Any move/drag after the menu is created will allow the mouse to trigger a highlighted item
    z0 mouseDrag  (const MouseEvent& e) override    { mouseUpCanTrigger |= true; handleMouseEvent (e); }
    z0 mouseMove  (const MouseEvent& e) override    { mouseUpCanTrigger |= true; handleMouseEvent (e); }

    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        alterChildYPos (roundToInt (-10.0f * wheel.deltaY * PopupMenuSettings::scrollZone));
    }

    b8 windowIsStillValid()
    {
        if (! isVisible())
            return false;

        if (componentAttachedTo != options.getTargetComponent())
        {
            dismissMenu (nullptr);
            return false;
        }

        if (auto* currentlyModalWindow = dynamic_cast<MenuWindow*> (Component::getCurrentlyModalComponent()))
            if (! treeContains (currentlyModalWindow))
                return false;

        if (exitingModalState)
            return false;

        return true;
    }

    static Array<MenuWindow*>& getActiveWindows()
    {
        static Array<MenuWindow*> activeMenuWindows;
        return activeMenuWindows;
    }

    MouseSourceState& getMouseState (MouseInputSource source)
    {
        MouseSourceState* mouseState = nullptr;

        for (auto* ms : mouseSourceStates)
        {
            if (ms->source == source)
                mouseState = ms;
            else if (ms->source.getType() != source.getType())
                ms->stopTimer();
        }

        if (mouseState == nullptr)
        {
            mouseState = new MouseSourceState (*this, source);
            mouseSourceStates.add (mouseState);
        }

        return *mouseState;
    }

    //==============================================================================
    b8 isOverAnyMenu() const
    {
        return parent != nullptr ? parent->isOverAnyMenu()
                                 : isOverChildren();
    }

    b8 isOverChildren() const
    {
        return isVisible()
                && (isAnyMouseOver() || (activeSubMenu != nullptr && activeSubMenu->isOverChildren()));
    }

    b8 isAnyMouseOver() const
    {
        for (auto* ms : mouseSourceStates)
            if (ms->isOver())
                return true;

        return false;
    }

    b8 treeContains (const MenuWindow* const window) const noexcept
    {
        auto* mw = this;

        while (mw->parent != nullptr)
            mw = mw->parent;

        while (mw != nullptr)
        {
            if (mw == window)
                return true;

            mw = mw->activeSubMenu.get();
        }

        return false;
    }

    b8 doesAnyDrxCompHaveFocus()
    {
        if (! detail::WindowingHelpers::isForegroundOrEmbeddedProcess (componentAttachedTo))
            return false;

        if (Component::getCurrentlyFocusedComponent() != nullptr)
            return true;

        for (i32 i = ComponentPeer::getNumPeers(); --i >= 0;)
        {
            if (ComponentPeer::getPeer (i)->isFocused())
            {
                hasAnyDrxCompHadFocus = true;
                return true;
            }
        }

        return ! hasAnyDrxCompHadFocus;
    }

    //==============================================================================
    Rectangle<i32> getParentArea (Point<i32> targetPoint, Component* relativeTo = nullptr)
    {
        if (relativeTo != nullptr)
            targetPoint = relativeTo->localPointToGlobal (targetPoint);

        auto* display = Desktop::getInstance().getDisplays().getDisplayForPoint (targetPoint * scaleFactor);
        auto parentArea = display->userArea.getIntersection (display->safeAreaInsets.subtractedFrom (display->totalArea));

        if (auto* pc = options.getParentComponent())
        {
            return pc->getLocalArea (nullptr,
                                     pc->getScreenBounds()
                                           .reduced (getLookAndFeel().getPopupMenuBorderSizeWithOptions (options))
                                           .getIntersection (parentArea));
        }

        return parentArea;
    }

    z0 calculateWindowPos (Rectangle<i32> target, const b8 alignToRectangle)
    {
        auto parentArea = getParentArea (target.getCentre()) / scaleFactor;

        if (auto* pc = options.getParentComponent())
            target = pc->getLocalArea (nullptr, target).getIntersection (parentArea);

        auto maxMenuHeight = parentArea.getHeight() - 24;

        i32 x, y, widthToUse, heightToUse;
        layoutMenuItems (parentArea.getWidth() - 24, maxMenuHeight, widthToUse, heightToUse);

        if (alignToRectangle)
        {
            x = target.getX();

            auto spaceUnder = parentArea.getBottom() - target.getBottom();
            auto spaceOver = target.getY() - parentArea.getY();
            auto bufferHeight = 30;

            if (options.getPreferredPopupDirection() == Options::PopupDirection::upwards)
                y = (heightToUse < spaceOver - bufferHeight  || spaceOver >= spaceUnder) ? target.getY() - heightToUse
                                                                                         : target.getBottom();
            else
                y = (heightToUse < spaceUnder - bufferHeight || spaceUnder >= spaceOver) ? target.getBottom()
                                                                                         : target.getY() - heightToUse;
        }
        else
        {
            b8 tendTowardsRight = target.getCentreX() < parentArea.getCentreX();

            if (parent != nullptr)
            {
                if (parent->parent != nullptr)
                {
                    const b8 parentGoingRight = (parent->getX() + parent->getWidth() / 2
                                                    > parent->parent->getX() + parent->parent->getWidth() / 2);

                    if (parentGoingRight && target.getRight() + widthToUse < parentArea.getRight() - 4)
                        tendTowardsRight = true;
                    else if ((! parentGoingRight) && target.getX() > widthToUse + 4)
                        tendTowardsRight = false;
                }
                else if (target.getRight() + widthToUse < parentArea.getRight() - 32)
                {
                    tendTowardsRight = true;
                }
            }

            auto biggestSpace = jmax (parentArea.getRight() - target.getRight(),
                                      target.getX() - parentArea.getX()) - 32;

            if (biggestSpace < widthToUse)
            {
                layoutMenuItems (biggestSpace + target.getWidth() / 3, maxMenuHeight, widthToUse, heightToUse);

                if (numColumns > 1)
                    layoutMenuItems (biggestSpace - 4, maxMenuHeight, widthToUse, heightToUse);

                tendTowardsRight = (parentArea.getRight() - target.getRight()) >= (target.getX() - parentArea.getX());
            }

            x = tendTowardsRight ? jmin (parentArea.getRight() - widthToUse - 4, target.getRight())
                                 : jmax (parentArea.getX() + 4, target.getX() - widthToUse);

            if (getLookAndFeel().getPopupMenuBorderSizeWithOptions (options) == 0) // workaround for dismissing the window on mouse up when border size is 0
                x += tendTowardsRight ? 1 : -1;

            const auto border = getLookAndFeel().getPopupMenuBorderSizeWithOptions (options);
            y = target.getCentreY() > parentArea.getCentreY() ? jmax (parentArea.getY(), target.getBottom() - heightToUse) + border
                                                              : target.getY() - border;
        }

        x = jmax (parentArea.getX() + 1, jmin (parentArea.getRight()  - (widthToUse  + 6), x));
        y = jmax (parentArea.getY() + 1, jmin (parentArea.getBottom() - (heightToUse + 6), y));

        windowPos.setBounds (x, y, widthToUse, heightToUse);

        // sets this flag if it's big enough to obscure any of its parent menus
        hideOnExit = parent != nullptr
                      && parent->windowPos.intersects (windowPos.expanded (-4, -4));
    }

    z0 layoutMenuItems (i32k maxMenuW, i32k maxMenuH, i32& width, i32& height)
    {
        // Ensure we don't try to add an empty column after the final item
        if (auto* last = items.getLast())
            last->item.shouldBreakAfter = false;

        const auto isBreak = [] (const ItemComponent* item) { return item->item.shouldBreakAfter; };
        const auto numBreaks = static_cast<i32> (std::count_if (items.begin(), items.end(), isBreak));
        numColumns = numBreaks + 1;

        if (numBreaks == 0)
            insertColumnBreaks (maxMenuW, maxMenuH);

        workOutManualSize (maxMenuW);
        height = jmin (contentHeight, maxMenuH);

        needsToScroll = contentHeight > height;

        width = updateYPositions();
    }

    z0 insertColumnBreaks (i32k maxMenuW, i32k maxMenuH)
    {
        numColumns = options.getMinimumNumColumns();
        contentHeight = 0;

        auto maximumNumColumns = options.getMaximumNumColumns() > 0 ? options.getMaximumNumColumns() : 7;

        for (;;)
        {
            auto totalW = workOutBestSize (maxMenuW);

            if (totalW > maxMenuW)
            {
                numColumns = jmax (1, numColumns - 1);
                workOutBestSize (maxMenuW); // to update col widths
                break;
            }

            if (totalW > maxMenuW / 2
                || contentHeight < maxMenuH
                || numColumns >= maximumNumColumns)
                break;

            ++numColumns;
        }

        const auto itemsPerColumn = (items.size() + numColumns - 1) / numColumns;

        for (auto i = 0;; i += itemsPerColumn)
        {
            const auto breakIndex = i + itemsPerColumn - 1;

            if (breakIndex >= items.size())
                break;

            items[breakIndex]->item.shouldBreakAfter = true;
        }

        if (! items.isEmpty())
            (*std::prev (items.end()))->item.shouldBreakAfter = false;
    }

    i32 correctColumnWidths (i32k maxMenuW)
    {
        auto totalW = std::accumulate (columnWidths.begin(), columnWidths.end(), 0);
        const auto minWidth = jmin (maxMenuW, options.getMinimumWidth());

        if (totalW < minWidth)
        {
            totalW = minWidth;

            for (auto& column : columnWidths)
                column = totalW / numColumns;
        }

        return totalW;
    }

    z0 workOutManualSize (i32k maxMenuW)
    {
        contentHeight = 0;
        columnWidths.clear();

        for (auto it = items.begin(), end = items.end(); it != end;)
        {
            const auto isBreak = [] (const ItemComponent* item) { return item->item.shouldBreakAfter; };
            const auto nextBreak = std::find_if (it, end, isBreak);
            const auto columnEnd = nextBreak == end ? end : std::next (nextBreak);

            const auto getMaxWidth = [] (i32 acc, const ItemComponent* item) { return jmax (acc, item->getWidth()); };
            const auto colW = std::accumulate (it, columnEnd, options.getStandardItemHeight(), getMaxWidth);
            const auto adjustedColW = jmin (maxMenuW / jmax (1, numColumns - 2),
                                            colW + getLookAndFeel().getPopupMenuBorderSizeWithOptions (options) * 2);

            const auto sumHeight = [] (i32 acc, const ItemComponent* item) { return acc + item->getHeight(); };
            const auto colH = std::accumulate (it, columnEnd, 0, sumHeight);

            contentHeight = jmax (contentHeight, colH);
            columnWidths.add (adjustedColW);
            it = columnEnd;
        }

        contentHeight += getLookAndFeel().getPopupMenuBorderSizeWithOptions (options) * 2;

        correctColumnWidths (maxMenuW);
    }

    i32 workOutBestSize (i32k maxMenuW)
    {
        contentHeight = 0;
        i32 childNum = 0;

        for (i32 col = 0; col < numColumns; ++col)
        {
            i32 colW = options.getStandardItemHeight(), colH = 0;

            auto numChildren = jmin (items.size() - childNum,
                                     (items.size() + numColumns - 1) / numColumns);

            for (i32 i = numChildren; --i >= 0;)
            {
                colW = jmax (colW, items.getUnchecked (childNum + i)->getWidth());
                colH += items.getUnchecked (childNum + i)->getHeight();
            }

            colW = jmin (maxMenuW / jmax (1, numColumns - 2),
                         colW + getLookAndFeel().getPopupMenuBorderSizeWithOptions (options) * 2);

            columnWidths.set (col, colW);
            contentHeight = jmax (contentHeight, colH);

            childNum += numChildren;
        }

        return correctColumnWidths (maxMenuW);
    }

    z0 ensureItemComponentIsVisible (const ItemComponent& itemComp, i32 wantedY)
    {
        if (windowPos.getHeight() > PopupMenuSettings::scrollZone * 4)
        {
            auto currentY = itemComp.getY();

            if (wantedY > 0 || currentY < 0 || itemComp.getBottom() > windowPos.getHeight())
            {
                if (wantedY < 0)
                    wantedY = jlimit (PopupMenuSettings::scrollZone,
                                      jmax (PopupMenuSettings::scrollZone,
                                            windowPos.getHeight() - (PopupMenuSettings::scrollZone + itemComp.getHeight())),
                                      currentY);

                auto parentArea = getParentArea (windowPos.getPosition(), options.getParentComponent()) / scaleFactor;
                auto deltaY = wantedY - currentY;

                windowPos.setSize (jmin (windowPos.getWidth(), parentArea.getWidth()),
                                   jmin (windowPos.getHeight(), parentArea.getHeight()));

                auto newY = jlimit (parentArea.getY(),
                                    parentArea.getBottom() - windowPos.getHeight(),
                                    windowPos.getY() + deltaY);

                deltaY -= newY - windowPos.getY();

                childYOffset -= deltaY;
                windowPos.setPosition (windowPos.getX(), newY);

                updateYPositions();
            }
        }
    }

    z0 resizeToBestWindowPos()
    {
        auto r = windowPos;

        if (childYOffset < 0)
        {
            r = r.withTop (r.getY() - childYOffset);
        }
        else if (childYOffset > 0)
        {
            auto spaceAtBottom = r.getHeight() - (contentHeight - childYOffset);

            if (spaceAtBottom > 0)
                r.setSize (r.getWidth(), r.getHeight() - spaceAtBottom);
        }

        setBounds (r);
        updateYPositions();
    }

    z0 alterChildYPos (i32 delta)
    {
        if (canScroll())
        {
            childYOffset += delta;

            childYOffset = [&]
            {
                if (delta < 0)
                    return jmax (childYOffset, 0);

                if (delta > 0)
                {
                    const auto limit = contentHeight
                                        - windowPos.getHeight()
                                        + getLookAndFeel().getPopupMenuBorderSizeWithOptions (options);
                    return jmin (childYOffset, limit);
                }

                return childYOffset;
            }();

            updateYPositions();
        }
        else
        {
            childYOffset = 0;
        }

        resizeToBestWindowPos();
        repaint();
    }

    i32 updateYPositions()
    {
        const auto separatorWidth = getLookAndFeel().getPopupMenuColumnSeparatorWidthWithOptions (options);
        const auto initialY = getLookAndFeel().getPopupMenuBorderSizeWithOptions (options)
                              - (childYOffset + (getY() - windowPos.getY()));

        auto col = 0;
        auto x = 0;
        auto y = initialY;

        for (const auto& item : items)
        {
            jassert (col < columnWidths.size());
            const auto columnWidth = columnWidths[col];
            item->setBounds (x, y, columnWidth, item->getHeight());
            y += item->getHeight();

            if (item->item.shouldBreakAfter)
            {
                col += 1;
                x += columnWidth + separatorWidth;
                y = initialY;
            }
        }

        return std::accumulate (columnWidths.begin(), columnWidths.end(), 0)
               + (separatorWidth * (columnWidths.size() - 1));
    }

    z0 setCurrentlyHighlightedChild (ItemComponent* child)
    {
        if (currentChild != nullptr)
            currentChild->setHighlighted (false);

        currentChild = child;

        if (currentChild != nullptr)
        {
            currentChild->setHighlighted (true);
            timeEnteredCurrentChildComp = Time::getApproximateMillisecondCounter();
        }

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
    }

    b8 isSubMenuVisible() const noexcept          { return activeSubMenu != nullptr && activeSubMenu->isVisible(); }

    b8 showSubMenuFor (ItemComponent* childComp)
    {
        activeSubMenu.reset();

        if (childComp == nullptr || ! hasActiveSubMenu (childComp->item))
            return false;

        activeSubMenu.reset (new HelperClasses::MenuWindow (*(childComp->item.subMenu), this,
                                                            options.forSubmenu()
                                                                   .withTargetScreenArea (childComp->getScreenBounds())
                                                                   .withMinimumWidth (0),
                                                            false, managerOfChosenCommand, scaleFactor));

        activeSubMenu->setVisible (true); // (must be called before enterModalState on Windows to avoid DropShadower confusion)
        activeSubMenu->enterModalState (false);
        activeSubMenu->toFront (false);
        return true;
    }

    z0 triggerCurrentlyHighlightedItem()
    {
        if (currentChild != nullptr && canBeTriggered (currentChild->item))
            dismissMenu (&currentChild->item);
    }

    enum class MenuSelectionDirection
    {
        forwards,
        backwards,
        current
    };

    z0 selectNextItem (MenuSelectionDirection direction)
    {
        disableMouseMovesOnMenuAndAncestors();

        auto start = [&]
        {
            auto index = items.indexOf (currentChild);

            if (index >= 0)
                return index;

            return direction == MenuSelectionDirection::backwards ? items.size() - 1
                                                                  : 0;
        }();

        auto preIncrement = (direction != MenuSelectionDirection::current && currentChild != nullptr);

        for (i32 i = items.size(); --i >= 0;)
        {
            if (preIncrement)
                start += (direction == MenuSelectionDirection::backwards ? -1 : 1);

            if (auto* mic = items.getUnchecked ((start + items.size()) % items.size()))
            {
                if (canBeTriggered (mic->item) || hasActiveSubMenu (mic->item))
                {
                    setCurrentlyHighlightedChild (mic);
                    return;
                }
            }

            if (! preIncrement)
                preIncrement = true;
        }
    }

    z0 disableMouseMovesOnMenuAndAncestors()
    {
        disableMouseMoves = true;

        if (parent != nullptr)
            parent->disableMouseMovesOnMenuAndAncestors();
    }

    b8 canScroll() const noexcept                 { return childYOffset != 0 || needsToScroll; }
    b8 isTopScrollZoneActive() const noexcept     { return canScroll() && childYOffset > 0; }
    b8 isBottomScrollZoneActive() const noexcept  { return canScroll() && childYOffset < contentHeight - windowPos.getHeight(); }

    //==============================================================================
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this,
                                                       AccessibilityRole::popupMenu,
                                                       AccessibilityActions().addAction (AccessibilityActionType::focus, [this]
                                                       {
                                                           if (currentChild != nullptr)
                                                           {
                                                               if (auto* handler = currentChild->getAccessibilityHandler())
                                                                   handler->grabFocus();
                                                           }
                                                           else
                                                           {
                                                               selectNextItem (MenuSelectionDirection::forwards);
                                                           }
                                                       }));
    }

    LookAndFeel* findLookAndFeel (const PopupMenu& menu, MenuWindow* parentWindow) const
    {
        return parentWindow != nullptr ? &(parentWindow->getLookAndFeel())
                                       : menu.lookAndFeel.get();
    }

    LookAndFeel& findNonNullLookAndFeel (const PopupMenu& menu, MenuWindow* parentWindow) const
    {
        if (auto* result = findLookAndFeel (menu, parentWindow))
            return *result;

        return getLookAndFeel();
    }

    b8 mouseHasBeenOver() const
    {
        return mouseWasOver;
    }

    b8 allowMouseUpToTriggerItem() const
    {
        return mouseUpCanTrigger;
    }

    //==============================================================================
    MenuWindow* parent;
    const Options options;
    OwnedArray<ItemComponent> items;
    ApplicationCommandManager** managerOfChosenCommand;
    WeakReference<Component> componentAttachedTo;
    Rectangle<i32> windowPos;
    b8 needsToScroll = false;
    b8 hideOnExit = false, disableMouseMoves = false, hasAnyDrxCompHadFocus = false;
    i32 numColumns = 0, contentHeight = 0, childYOffset = 0;
    Component::SafePointer<ItemComponent> currentChild;
    std::unique_ptr<MenuWindow> activeSubMenu;
    Array<i32> columnWidths;
    u32 windowCreationTime, lastFocusedTime, timeEnteredCurrentChildComp;
    OwnedArray<MouseSourceState> mouseSourceStates;
    f32 scaleFactor;
    b8 exitingModalState = false;

private:
    z0 handleMouseEvent (const MouseEvent& e)
    {
        mouseWasOver |= reallyContains (getLocalPoint (nullptr, e.getScreenPosition()), true);
        getMouseState (e.source).handleMouseEventWithPosition (e.getScreenPosition());
    }

    b8 mouseWasOver = false;
    b8 mouseUpCanTrigger = ! ModifierKeys::currentModifiers.isAnyMouseButtonDown();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuWindow)
};

//==============================================================================
class MouseSourceState final : private Timer
{
public:
    MouseSourceState (MenuWindow& w, MouseInputSource s)
        : window (w), source (s), lastScrollTime (Time::getMillisecondCounter())
    {
        startTimerHz (20);
    }

    ~MouseSourceState() override
    {
        stopTimer();
    }

    z0 handleMouseEventWithPosition (const Point<i32>& e)
    {
        if (! window.windowIsStillValid())
            return;

        startTimerHz (20);
        handleMousePosition (e);
    }

    b8 isOver() const
    {
        return window.reallyContains (window.getLocalPoint (nullptr, source.getScreenPosition()).roundToInt(), true);
    }

    using Timer::stopTimer;

    MenuWindow& window;
    MouseInputSource source;

private:
    Point<i32> lastMousePos;
    f64 scrollAcceleration = 0;
    u32 lastScrollTime;
    b8 isDown = false;

    // Although most mouse movements can be handled inside mouse event callbacks, scrolling of menus
    // may happen while the mouse is not moving, so periodic timer callbacks are required in this
    // scenario.
    z0 timerCallback() override
    {
       #if DRX_WINDOWS
        // touch and pen devices on Windows send an offscreen mouse move after mouse up events
        // but we don't want to forward these on as they will dismiss the menu
        if ((source.isTouch() || source.isPen()) && ! isValidMousePosition())
            return;
       #endif

        handleMouseEventWithPosition (source.getScreenPosition().roundToInt());
    }

    z0 handleMousePosition (Point<i32> globalMousePos)
    {
        auto localMousePos = window.getLocalPoint (nullptr, globalMousePos);
        auto timeNow = Time::getMillisecondCounter();

        if (timeNow > window.timeEnteredCurrentChildComp + 100
             && window.reallyContains (localMousePos, true)
             && window.currentChild != nullptr
             && ! (window.disableMouseMoves || window.isSubMenuVisible()))
        {
            window.showSubMenuFor (window.currentChild);
        }

        highlightItemUnderMouse (globalMousePos, localMousePos);

        const b8 overScrollArea = scrollIfNecessary (localMousePos, timeNow);
        const b8 isOverAny = window.isOverAnyMenu();

        if (window.hideOnExit && window.mouseHasBeenOver() && ! isOverAny)
            window.hide (nullptr, true);
        else
            checkButtonState (localMousePos, timeNow, isDown, overScrollArea, isOverAny);
    }

    z0 checkButtonState (Point<i32> localMousePos, u32k timeNow,
                           const b8 wasDown, const b8 overScrollArea, const b8 isOverAny)
    {
        isDown = window.mouseHasBeenOver()
                    && (ModifierKeys::currentModifiers.isAnyMouseButtonDown()
                         || ComponentPeer::getCurrentModifiersRealtime().isAnyMouseButtonDown());

        const auto reallyContained = window.reallyContains (localMousePos, true);

        if (! window.doesAnyDrxCompHaveFocus() && ! reallyContained)
        {
            if (timeNow > window.lastFocusedTime + 10)
            {
                PopupMenuSettings::menuWasHiddenBecauseOfAppChange = true;
                window.dismissMenu (nullptr);
                // Note: This object may have been deleted by the previous call.
            }
        }
        else if (wasDown && timeNow > window.windowCreationTime + 250 && ! isDown && ! overScrollArea)
        {
            if (reallyContained && window.allowMouseUpToTriggerItem())
                window.triggerCurrentlyHighlightedItem();
            else if ((window.mouseHasBeenOver() || ! window.allowMouseUpToTriggerItem()) && ! isOverAny)
                window.dismissMenu (nullptr);

            // Note: This object may have been deleted by the previous call.
        }
        else
        {
            window.lastFocusedTime = timeNow;
        }
    }

    z0 highlightItemUnderMouse (Point<i32> globalMousePos, Point<i32> localMousePos)
    {
        const auto mouseHasMoved = 2 < lastMousePos.getDistanceFrom (globalMousePos);

        if (! mouseHasMoved)
            return;

        const auto isMouseOver = window.reallyContains (localMousePos, true);

        if (isMouseOver)
            window.disableMouseMoves = false;

        if (window.disableMouseMoves || (window.activeSubMenu != nullptr && window.activeSubMenu->isOverChildren()))
            return;

        const b8 isMovingTowardsMenu = isMouseOver && globalMousePos != lastMousePos
                                            && isMovingTowardsSubmenu (globalMousePos);

        lastMousePos = globalMousePos;

        if (! isMovingTowardsMenu)
        {
            auto* c = window.getComponentAt (localMousePos);

            if (c == &window)
                c = nullptr;

            auto* itemUnderMouse = dynamic_cast<ItemComponent*> (c);

            if (itemUnderMouse == nullptr && c != nullptr)
                itemUnderMouse = c->findParentComponentOfClass<ItemComponent>();

            if (itemUnderMouse != window.currentChild
                  && (isMouseOver || (window.activeSubMenu == nullptr) || ! window.activeSubMenu->isVisible()))
            {
                if (isMouseOver && (c != nullptr) && (window.activeSubMenu != nullptr))
                    window.activeSubMenu->hide (nullptr, true);

                if (! isMouseOver)
                {
                    if (! window.mouseHasBeenOver())
                        return;

                    itemUnderMouse = nullptr;
                }

                window.setCurrentlyHighlightedChild (itemUnderMouse);
            }
        }
    }

    b8 isMovingTowardsSubmenu (Point<i32> newGlobalPos) const
    {
        if (window.activeSubMenu == nullptr)
            return false;

        // try to intelligently guess whether the user is moving the mouse towards a currently-open
        // submenu. To do this, look at whether the mouse stays inside a triangular region that
        // extends from the last mouse pos to the submenu's rectangle..

        auto itemScreenBounds = window.activeSubMenu->getScreenBounds();
        auto subX = (f32) itemScreenBounds.getX();

        auto oldGlobalPos = lastMousePos;

        if (itemScreenBounds.getX() > window.getX())
        {
            oldGlobalPos -= Point<i32> (2, 0);  // to enlarge the triangle a bit, in case the mouse only moves a couple of pixels
        }
        else
        {
            oldGlobalPos += Point<i32> (2, 0);
            subX += (f32) itemScreenBounds.getWidth();
        }

        Path areaTowardsSubMenu;
        areaTowardsSubMenu.addTriangle ((f32) oldGlobalPos.x, (f32) oldGlobalPos.y,
                                        subX, (f32) itemScreenBounds.getY(),
                                        subX, (f32) itemScreenBounds.getBottom());

        return areaTowardsSubMenu.contains (newGlobalPos.toFloat());
    }

    b8 scrollIfNecessary (Point<i32> localMousePos, u32k timeNow)
    {
        if (window.canScroll()
             && isPositiveAndBelow (localMousePos.x, window.getWidth())
             && (isPositiveAndBelow (localMousePos.y, window.getHeight()) || source.isDragging()))
        {
            if (window.isTopScrollZoneActive() && localMousePos.y < PopupMenuSettings::scrollZone)
                return scroll (timeNow, -1);

            if (window.isBottomScrollZoneActive() && localMousePos.y > window.getHeight() - PopupMenuSettings::scrollZone)
                return scroll (timeNow, 1);
        }

        scrollAcceleration = 1.0;
        return false;
    }

    b8 scroll (u32k timeNow, i32k direction)
    {
        if (timeNow > lastScrollTime + 20)
        {
            scrollAcceleration = jmin (4.0, scrollAcceleration * 1.04);
            i32 amount = 0;

            for (i32 i = 0; i < window.items.size() && amount == 0; ++i)
                amount = ((i32) scrollAcceleration) * window.items.getUnchecked (i)->getHeight();

            window.alterChildYPos (amount * direction);
            lastScrollTime = timeNow;
        }

        return true;
    }

   #if DRX_WINDOWS
    b8 isValidMousePosition()
    {
        auto screenPos = source.getScreenPosition();
        auto localPos = (window.activeSubMenu == nullptr) ? window.getLocalPoint (nullptr, screenPos)
                                                          : window.activeSubMenu->getLocalPoint (nullptr, screenPos);

        if (localPos.x < 0 && localPos.y < 0)
            return false;

        return true;
    }
   #endif

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseSourceState)
};

//==============================================================================
struct NormalComponentWrapper final : public PopupMenu::CustomComponent
{
    NormalComponentWrapper (Component& comp, i32 w, i32 h, b8 triggerMenuItemAutomaticallyWhenClicked)
        : PopupMenu::CustomComponent (triggerMenuItemAutomaticallyWhenClicked),
          width (w), height (h)
    {
        addAndMakeVisible (comp);
    }

    z0 getIdealSize (i32& idealWidth, i32& idealHeight) override
    {
        idealWidth = width;
        idealHeight = height;
    }

    z0 resized() override
    {
        if (auto* child = getChildComponent (0))
            child->setBounds (getLocalBounds());
    }

    i32k width, height;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NormalComponentWrapper)
};

};

//==============================================================================
PopupMenu::PopupMenu (const PopupMenu& other)
    : items (other.items),
      lookAndFeel (other.lookAndFeel)
{
}

PopupMenu& PopupMenu::operator= (const PopupMenu& other)
{
    if (this != &other)
    {
        items = other.items;
        lookAndFeel = other.lookAndFeel;
    }

    return *this;
}

PopupMenu::PopupMenu (PopupMenu&& other) noexcept
    : items (std::move (other.items)),
      lookAndFeel (std::move (other.lookAndFeel))
{
}

PopupMenu& PopupMenu::operator= (PopupMenu&& other) noexcept
{
    items = std::move (other.items);
    lookAndFeel = other.lookAndFeel;
    return *this;
}

PopupMenu::~PopupMenu() = default;

z0 PopupMenu::clear()
{
    items.clear();
}

//==============================================================================
PopupMenu::Item::Item() = default;
PopupMenu::Item::Item (Txt t) : text (std::move (t)), itemID (-1) {}

PopupMenu::Item::Item (Item&&) = default;
PopupMenu::Item& PopupMenu::Item::operator= (Item&&) = default;

PopupMenu::Item::Item (const Item& other)
  : text (other.text),
    itemID (other.itemID),
    action (other.action),
    subMenu (createCopyIfNotNull (other.subMenu.get())),
    image (other.image != nullptr ? other.image->createCopy() : nullptr),
    customComponent (other.customComponent),
    customCallback (other.customCallback),
    commandManager (other.commandManager),
    shortcutKeyDescription (other.shortcutKeyDescription),
    colour (other.colour),
    isEnabled (other.isEnabled),
    isTicked (other.isTicked),
    isSeparator (other.isSeparator),
    isSectionHeader (other.isSectionHeader),
    shouldBreakAfter (other.shouldBreakAfter)
{}

PopupMenu::Item& PopupMenu::Item::operator= (const Item& other)
{
    text = other.text;
    itemID = other.itemID;
    action = other.action;
    subMenu.reset (createCopyIfNotNull (other.subMenu.get()));
    image = other.image != nullptr ? other.image->createCopy() : std::unique_ptr<Drawable>();
    customComponent = other.customComponent;
    customCallback = other.customCallback;
    commandManager = other.commandManager;
    shortcutKeyDescription = other.shortcutKeyDescription;
    colour = other.colour;
    isEnabled = other.isEnabled;
    isTicked = other.isTicked;
    isSeparator = other.isSeparator;
    isSectionHeader = other.isSectionHeader;
    shouldBreakAfter = other.shouldBreakAfter;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setTicked (b8 shouldBeTicked) & noexcept
{
    isTicked = shouldBeTicked;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setEnabled (b8 shouldBeEnabled) & noexcept
{
    isEnabled = shouldBeEnabled;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setAction (std::function<z0()> newAction) & noexcept
{
    action = std::move (newAction);
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setID (i32 newID) & noexcept
{
    itemID = newID;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setColor (Color newColor) & noexcept
{
    colour = newColor;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setCustomComponent (ReferenceCountedObjectPtr<CustomComponent> comp) & noexcept
{
    customComponent = comp;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setImage (std::unique_ptr<Drawable> newImage) & noexcept
{
    image = std::move (newImage);
    return *this;
}

PopupMenu::Item&& PopupMenu::Item::setTicked (b8 shouldBeTicked) && noexcept
{
    isTicked = shouldBeTicked;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setEnabled (b8 shouldBeEnabled) && noexcept
{
    isEnabled = shouldBeEnabled;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setAction (std::function<z0()> newAction) && noexcept
{
    action = std::move (newAction);
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setID (i32 newID) && noexcept
{
    itemID = newID;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setColor (Color newColor) && noexcept
{
    colour = newColor;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setCustomComponent (ReferenceCountedObjectPtr<CustomComponent> comp) && noexcept
{
    customComponent = comp;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setImage (std::unique_ptr<Drawable> newImage) && noexcept
{
    image = std::move (newImage);
    return std::move (*this);
}

z0 PopupMenu::addItem (Item newItem)
{
    // An ID of 0 is used as a return value to indicate that the user
    // didn't pick anything, so you shouldn't use it as the ID for an item.
    jassert (newItem.itemID != 0
              || newItem.isSeparator || newItem.isSectionHeader
              || newItem.subMenu != nullptr);

    items.add (std::move (newItem));
}

z0 PopupMenu::addItem (Txt itemText, std::function<z0()> action)
{
    addItem (std::move (itemText), true, false, std::move (action));
}

z0 PopupMenu::addItem (Txt itemText, b8 isActive, b8 isTicked, std::function<z0()> action)
{
    Item i (std::move (itemText));
    i.action = std::move (action);
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    addItem (std::move (i));
}

z0 PopupMenu::addItem (i32 itemResultID, Txt itemText, b8 isActive, b8 isTicked)
{
    Item i (std::move (itemText));
    i.itemID = itemResultID;
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    addItem (std::move (i));
}

static std::unique_ptr<Drawable> createDrawableFromImage (const Image& im)
{
    if (im.isValid())
    {
        auto d = new DrawableImage();
        d->setImage (im);
        return std::unique_ptr<Drawable> (d);
    }

    return {};
}

z0 PopupMenu::addItem (i32 itemResultID, Txt itemText, b8 isActive, b8 isTicked, const Image& iconToUse)
{
    addItem (itemResultID, std::move (itemText), isActive, isTicked, createDrawableFromImage (iconToUse));
}

z0 PopupMenu::addItem (i32 itemResultID, Txt itemText, b8 isActive,
                         b8 isTicked, std::unique_ptr<Drawable> iconToUse)
{
    Item i (std::move (itemText));
    i.itemID = itemResultID;
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    i.image = std::move (iconToUse);
    addItem (std::move (i));
}

z0 PopupMenu::addCommandItem (ApplicationCommandManager* commandManager,
                                const CommandID commandID,
                                Txt displayName,
                                std::unique_ptr<Drawable> iconToUse)
{
    jassert (commandManager != nullptr && commandID != 0);

    if (auto* registeredInfo = commandManager->getCommandForID (commandID))
    {
        ApplicationCommandInfo info (*registeredInfo);
        auto* target = commandManager->getTargetForCommand (commandID, info);

        Item i;
        i.text = displayName.isNotEmpty() ? std::move (displayName) : info.shortName;
        i.itemID = (i32) commandID;
        i.commandManager = commandManager;
        i.isEnabled = target != nullptr && (info.flags & ApplicationCommandInfo::isDisabled) == 0;
        i.isTicked = (info.flags & ApplicationCommandInfo::isTicked) != 0;
        i.image = std::move (iconToUse);
        addItem (std::move (i));
    }
}

z0 PopupMenu::addColoredItem (i32 itemResultID, Txt itemText, Color itemTextColor,
                                 b8 isActive, b8 isTicked, std::unique_ptr<Drawable> iconToUse)
{
    Item i (std::move (itemText));
    i.itemID = itemResultID;
    i.colour = itemTextColor;
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    i.image = std::move (iconToUse);
    addItem (std::move (i));
}

z0 PopupMenu::addColoredItem (i32 itemResultID, Txt itemText, Color itemTextColor,
                                 b8 isActive, b8 isTicked, const Image& iconToUse)
{
    Item i (std::move (itemText));
    i.itemID = itemResultID;
    i.colour = itemTextColor;
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    i.image = createDrawableFromImage (iconToUse);
    addItem (std::move (i));
}

z0 PopupMenu::addCustomItem (i32 itemResultID,
                               std::unique_ptr<CustomComponent> cc,
                               std::unique_ptr<const PopupMenu> subMenu,
                               const Txt& itemTitle)
{
    Item i;
    i.text = itemTitle;
    i.itemID = itemResultID;
    i.customComponent = cc.release();
    i.subMenu.reset (createCopyIfNotNull (subMenu.get()));

    // If this assertion is hit, this item will be visible to screen readers but with
    // no name, which may be confusing to users.
    // It's probably a good idea to add a title for this menu item that describes
    // the meaning of the item, or the contents of the submenu, as appropriate.
    // If you don't want this menu item to be press-able directly, pass "false" to the
    // constructor of the CustomComponent.
    jassert (! (HelperClasses::ItemComponent::isAccessibilityHandlerRequired (i) && itemTitle.isEmpty()));

    addItem (std::move (i));
}

z0 PopupMenu::addCustomItem (i32 itemResultID,
                               Component& customComponent,
                               i32 idealWidth, i32 idealHeight,
                               b8 triggerMenuItemAutomaticallyWhenClicked,
                               std::unique_ptr<const PopupMenu> subMenu,
                               const Txt& itemTitle)
{
    auto comp = std::make_unique<HelperClasses::NormalComponentWrapper> (customComponent, idealWidth, idealHeight,
                                                                         triggerMenuItemAutomaticallyWhenClicked);
    addCustomItem (itemResultID, std::move (comp), std::move (subMenu), itemTitle);
}

z0 PopupMenu::addSubMenu (Txt subMenuName, PopupMenu subMenu, b8 isActive)
{
    addSubMenu (std::move (subMenuName), std::move (subMenu), isActive, nullptr, false, 0);
}

z0 PopupMenu::addSubMenu (Txt subMenuName, PopupMenu subMenu, b8 isActive,
                            const Image& iconToUse, b8 isTicked, i32 itemResultID)
{
    addSubMenu (std::move (subMenuName), std::move (subMenu), isActive,
                createDrawableFromImage (iconToUse), isTicked, itemResultID);
}

z0 PopupMenu::addSubMenu (Txt subMenuName, PopupMenu subMenu, b8 isActive,
                            std::unique_ptr<Drawable> iconToUse, b8 isTicked, i32 itemResultID)
{
    Item i (std::move (subMenuName));
    i.itemID = itemResultID;
    i.isEnabled = isActive && (itemResultID != 0 || subMenu.getNumItems() > 0);
    i.subMenu.reset (new PopupMenu (std::move (subMenu)));
    i.isTicked = isTicked;
    i.image = std::move (iconToUse);
    addItem (std::move (i));
}

z0 PopupMenu::addSeparator()
{
    if (items.size() > 0 && ! items.getLast().isSeparator)
    {
        Item i;
        i.isSeparator = true;
        addItem (std::move (i));
    }
}

z0 PopupMenu::addSectionHeader (Txt title)
{
    Item i (std::move (title));
    i.itemID = 0;
    i.isSectionHeader = true;
    addItem (std::move (i));
}

z0 PopupMenu::addColumnBreak()
{
    if (! items.isEmpty())
        std::prev (items.end())->shouldBreakAfter = true;
}

//==============================================================================
PopupMenu::Options::Options()
{
    targetArea.setPosition (Desktop::getMousePosition());
}

template <typename Member, typename Item>
static PopupMenu::Options with (PopupMenu::Options options, Member&& member, Item&& item)
{
    options.*member = std::forward<Item> (item);
    return options;
}

PopupMenu::Options PopupMenu::Options::withTargetComponent (Component* comp) const
{
    auto o = with (with (*this, &Options::targetComponent, comp), &Options::topLevelTarget, comp);

    if (comp != nullptr)
        o.targetArea = comp->getScreenBounds();

    return o;
}

PopupMenu::Options PopupMenu::Options::withTargetComponent (Component& comp) const
{
    return withTargetComponent (&comp);
}

PopupMenu::Options PopupMenu::Options::withTargetScreenArea (Rectangle<i32> area) const
{
    return with (*this, &Options::targetArea, area);
}

PopupMenu::Options PopupMenu::Options::withMousePosition() const
{
    return withTargetScreenArea (Rectangle<i32>{}.withPosition (Desktop::getMousePosition()));
}

PopupMenu::Options PopupMenu::Options::withDeletionCheck (Component& comp) const
{
    return with (with (*this, &Options::isWatchingForDeletion, true),
                 &Options::componentToWatchForDeletion,
                 &comp);
}

PopupMenu::Options PopupMenu::Options::withMinimumWidth (i32 w) const
{
    return with (*this, &Options::minWidth, w);
}

PopupMenu::Options PopupMenu::Options::withMinimumNumColumns (i32 cols) const
{
    return with (*this, &Options::minColumns, cols);
}

PopupMenu::Options PopupMenu::Options::withMaximumNumColumns (i32 cols) const
{
    return with (*this, &Options::maxColumns, cols);
}

PopupMenu::Options PopupMenu::Options::withStandardItemHeight (i32 height) const
{
    return with (*this, &Options::standardHeight, height);
}

PopupMenu::Options PopupMenu::Options::withItemThatMustBeVisible (i32 idOfItemToBeVisible) const
{
    return with (*this, &Options::visibleItemID, idOfItemToBeVisible);
}

PopupMenu::Options PopupMenu::Options::withParentComponent (Component* parent) const
{
    return with (*this, &Options::parentComponent, parent);
}

PopupMenu::Options PopupMenu::Options::withPreferredPopupDirection (PopupDirection direction) const
{
    return with (*this, &Options::preferredPopupDirection, direction);
}

PopupMenu::Options PopupMenu::Options::withInitiallySelectedItem (i32 idOfItemToBeSelected) const
{
    return with (*this, &Options::initiallySelectedItemId, idOfItemToBeSelected);
}

PopupMenu::Options PopupMenu::Options::forSubmenu() const
{
    return with (*this, &Options::targetComponent, nullptr);
}

Component* PopupMenu::createWindow (const Options& options,
                                    ApplicationCommandManager** managerOfChosenCommand) const
{
   #if DRX_WINDOWS
    const auto scope = [&]() -> std::unique_ptr<ScopedThreadDPIAwarenessSetter>
    {
        if (auto* target = options.getTargetComponent())
            if (auto* handle = target->getWindowHandle())
                return std::make_unique<ScopedThreadDPIAwarenessSetter> (handle);

        return nullptr;
    }();
   #endif

    return items.isEmpty() ? nullptr
                           : new HelperClasses::MenuWindow (*this, nullptr, options,
                                                            ! options.getTargetScreenArea().isEmpty(),
                                                            managerOfChosenCommand);
}

//==============================================================================
// This invokes any command manager commands and deletes the menu window when it is dismissed
struct PopupMenuCompletionCallback final : public ModalComponentManager::Callback
{
    PopupMenuCompletionCallback() = default;

    z0 modalStateFinished (i32 result) override
    {
        if (managerOfChosenCommand != nullptr && result != 0)
        {
            ApplicationCommandTarget::InvocationInfo info (result);
            info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

            managerOfChosenCommand->invoke (info, true);
        }

        // (this would be the place to fade out the component, if that's what's required)
        component.reset();

        if (PopupMenuSettings::menuWasHiddenBecauseOfAppChange)
            return;

        if (auto* focusComponent = Component::getCurrentlyFocusedComponent())
        {
            const auto focusedIsNotMinimised = [focusComponent]
            {
                if (auto* peer = focusComponent->getPeer())
                    return ! peer->isMinimised();

                return false;
            }();

            if (focusedIsNotMinimised)
            {
                if (auto* topLevel = focusComponent->getTopLevelComponent())
                    topLevel->toFront (true);

                if (focusComponent->isShowing() && ! focusComponent->hasKeyboardFocus (true))
                    focusComponent->grabKeyboardFocus();
            }
        }
    }

    ApplicationCommandManager* managerOfChosenCommand = nullptr;
    std::unique_ptr<Component> component;

    DRX_DECLARE_NON_COPYABLE (PopupMenuCompletionCallback)
};

i32 PopupMenu::showWithOptionalCallback (const Options& options,
                                         ModalComponentManager::Callback* userCallback,
                                         [[maybe_unused]] b8 canBeModal)
{
    std::unique_ptr<ModalComponentManager::Callback> userCallbackDeleter (userCallback);
    std::unique_ptr<PopupMenuCompletionCallback> callback (new PopupMenuCompletionCallback());

    if (auto* window = createWindow (options, &(callback->managerOfChosenCommand)))
    {
        callback->component.reset (window);

        PopupMenuSettings::menuWasHiddenBecauseOfAppChange = false;

        window->setVisible (true); // (must be called before enterModalState on Windows to avoid DropShadower confusion)
        window->enterModalState (false, userCallbackDeleter.release());
        ModalComponentManager::getInstance()->attachCallback (window, callback.release());

        window->toFront (false);  // need to do this after making it modal, or it could
                                  // be stuck behind other comps that are already modal..

       #if DRX_MODAL_LOOPS_PERMITTED
        if (userCallback == nullptr && canBeModal)
            return window->runModalLoop();
       #else
        jassert (! (userCallback == nullptr && canBeModal));
       #endif
    }

    return 0;
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
i32 PopupMenu::showMenu (const Options& options)
{
    return showWithOptionalCallback (options, nullptr, true);
}
#endif

z0 PopupMenu::showMenuAsync (const Options& options)
{
    showWithOptionalCallback (options, nullptr, false);
}

z0 PopupMenu::showMenuAsync (const Options& options, ModalComponentManager::Callback* userCallback)
{
   #if ! DRX_MODAL_LOOPS_PERMITTED
    jassert (userCallback != nullptr);
   #endif

    showWithOptionalCallback (options, userCallback, false);
}

z0 PopupMenu::showMenuAsync (const Options& options, std::function<z0 (i32)> userCallback)
{
    showWithOptionalCallback (options, ModalCallbackFunction::create (userCallback), false);
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
i32 PopupMenu::show (i32 itemIDThatMustBeVisible, i32 minimumWidth,
                     i32 maximumNumColumns, i32 standardItemHeight,
                     ModalComponentManager::Callback* callback)
{
    return showWithOptionalCallback (Options().withItemThatMustBeVisible (itemIDThatMustBeVisible)
                                              .withMinimumWidth (minimumWidth)
                                              .withMaximumNumColumns (maximumNumColumns)
                                              .withStandardItemHeight (standardItemHeight),
                                     callback, true);
}

i32 PopupMenu::showAt (Rectangle<i32> screenAreaToAttachTo,
                       i32 itemIDThatMustBeVisible, i32 minimumWidth,
                       i32 maximumNumColumns, i32 standardItemHeight,
                       ModalComponentManager::Callback* callback)
{
    return showWithOptionalCallback (Options().withTargetScreenArea (screenAreaToAttachTo)
                                              .withItemThatMustBeVisible (itemIDThatMustBeVisible)
                                              .withMinimumWidth (minimumWidth)
                                              .withMaximumNumColumns (maximumNumColumns)
                                              .withStandardItemHeight (standardItemHeight),
                                     callback, true);
}

i32 PopupMenu::showAt (Component* componentToAttachTo,
                       i32 itemIDThatMustBeVisible, i32 minimumWidth,
                       i32 maximumNumColumns, i32 standardItemHeight,
                       ModalComponentManager::Callback* callback)
{
    auto options = Options().withItemThatMustBeVisible (itemIDThatMustBeVisible)
                            .withMinimumWidth (minimumWidth)
                            .withMaximumNumColumns (maximumNumColumns)
                            .withStandardItemHeight (standardItemHeight);

    if (componentToAttachTo != nullptr)
        options = options.withTargetComponent (componentToAttachTo);

    return showWithOptionalCallback (options, callback, true);
}
#endif

b8 DRX_CALLTYPE PopupMenu::dismissAllActiveMenus()
{
    auto& windows = HelperClasses::MenuWindow::getActiveWindows();
    auto numWindows = windows.size();

    for (i32 i = numWindows; --i >= 0;)
    {
        if (auto* pmw = windows[i])
        {
            pmw->setLookAndFeel (nullptr);
            pmw->dismissMenu (nullptr);
        }
    }

    return numWindows > 0;
}

//==============================================================================
i32 PopupMenu::getNumItems() const noexcept
{
    i32 num = 0;

    for (auto& mi : items)
        if (! mi.isSeparator)
            ++num;

    return num;
}

b8 PopupMenu::containsCommandItem (i32k commandID) const
{
    for (auto& mi : items)
        if ((mi.itemID == commandID && mi.commandManager != nullptr)
              || (mi.subMenu != nullptr && mi.subMenu->containsCommandItem (commandID)))
            return true;

    return false;
}

b8 PopupMenu::containsAnyActiveItems() const noexcept
{
    for (auto& mi : items)
    {
        if (mi.subMenu != nullptr)
        {
            if (mi.subMenu->containsAnyActiveItems())
                return true;
        }
        else if (mi.isEnabled)
        {
            return true;
        }
    }

    return false;
}

z0 PopupMenu::setLookAndFeel (LookAndFeel* const newLookAndFeel)
{
    lookAndFeel = newLookAndFeel;
}

z0 PopupMenu::setItem (CustomComponent& c, const Item* itemToUse)
{
    c.item = itemToUse;
    c.repaint();
}

//==============================================================================
PopupMenu::CustomComponent::CustomComponent() : CustomComponent (true) {}

PopupMenu::CustomComponent::CustomComponent (b8 autoTrigger)
    : triggeredAutomatically (autoTrigger)
{
}

z0 PopupMenu::CustomComponent::setHighlighted (b8 shouldBeHighlighted)
{
    isHighlighted = shouldBeHighlighted;
    repaint();
}

z0 PopupMenu::CustomComponent::triggerMenuItem()
{
    if (auto* mic = findParentComponentOfClass<HelperClasses::ItemComponent>())
    {
        if (auto* pmw = mic->findParentComponentOfClass<HelperClasses::MenuWindow>())
        {
            pmw->dismissMenu (&mic->item);
        }
        else
        {
            // something must have gone wrong with the component hierarchy if this happens..
            jassertfalse;
        }
    }
    else
    {
        // why isn't this component inside a menu? Not much point triggering the item if
        // there's no menu.
        jassertfalse;
    }
}

//==============================================================================
PopupMenu::CustomCallback::CustomCallback() {}
PopupMenu::CustomCallback::~CustomCallback() {}

//==============================================================================
PopupMenu::MenuItemIterator::MenuItemIterator (const PopupMenu& m, b8 recurse) : searchRecursively (recurse)
{
    index.add (0);
    menus.add (&m);
}

PopupMenu::MenuItemIterator::~MenuItemIterator() = default;

b8 PopupMenu::MenuItemIterator::next()
{
    if (index.size() == 0 || menus.getLast()->items.size() == 0)
        return false;

    currentItem = const_cast<PopupMenu::Item*> (&(menus.getLast()->items.getReference (index.getLast())));

    if (searchRecursively && currentItem->subMenu != nullptr)
    {
        index.add (0);
        menus.add (currentItem->subMenu.get());
    }
    else
    {
        index.setUnchecked (index.size() - 1, index.getLast() + 1);
    }

    while (index.size() > 0 && index.getLast() >= (i32) menus.getLast()->items.size())
    {
        index.removeLast();
        menus.removeLast();

        if (index.size() > 0)
            index.setUnchecked (index.size() - 1, index.getLast() + 1);
    }

    return true;
}

PopupMenu::Item& PopupMenu::MenuItemIterator::getItem() const
{
    jassert (currentItem != nullptr);
    return *(currentItem);
}

z0 PopupMenu::LookAndFeelMethods::drawPopupMenuBackground (Graphics&, i32, i32) {}

z0 PopupMenu::LookAndFeelMethods::drawPopupMenuItem (Graphics&, const Rectangle<i32>&,
                                                       b8, b8, b8,
                                                       b8, b8,
                                                       const Txt&,
                                                       const Txt&,
                                                       const Drawable*,
                                                       const Color*) {}

z0 PopupMenu::LookAndFeelMethods::drawPopupMenuSectionHeader (Graphics&, const Rectangle<i32>&,
                                                                const Txt&) {}

z0 PopupMenu::LookAndFeelMethods::drawPopupMenuUpDownArrow (Graphics&, i32, i32, b8) {}

z0 PopupMenu::LookAndFeelMethods::getIdealPopupMenuItemSize (const Txt&, b8, i32, i32&, i32&) {}

i32 PopupMenu::LookAndFeelMethods::getPopupMenuBorderSize() { return 0; }

} // namespace drx
