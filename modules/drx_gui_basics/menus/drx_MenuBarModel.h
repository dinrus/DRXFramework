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
/**
    A class for controlling MenuBar components.

    This class is used to tell a MenuBar what menus to show, and to respond
    to a menu being selected.

    @see MenuBarModel::Listener, MenuBarComponent, PopupMenu

    @tags{GUI}
*/
class DRX_API  MenuBarModel      : private AsyncUpdater,
                                    private ApplicationCommandManagerListener
{
public:
    //==============================================================================
    MenuBarModel() noexcept;

    /** Destructor. */
    ~MenuBarModel() override;

    //==============================================================================
    /** Call this when some of your menu items have changed.

        This method will cause a callback to any MenuBarListener objects that
        are registered with this model.

        If this model is displaying items from an ApplicationCommandManager, you
        can use the setApplicationCommandManagerToWatch() method to cause
        change messages to be sent automatically when the ApplicationCommandManager
        is changed.

        @see addListener, removeListener, MenuBarListener
    */
    z0 menuItemsChanged();

    /** Tells the menu bar to listen to the specified command manager, and to update
        itself when the commands change.

        This will also allow it to flash a menu name when a command from that menu
        is invoked using a keystroke.
    */
    z0 setApplicationCommandManagerToWatch (ApplicationCommandManager* manager);

    //==============================================================================
    /** A class to receive callbacks when a MenuBarModel changes.

        @see MenuBarModel::addListener, MenuBarModel::removeListener, MenuBarModel::menuItemsChanged
    */
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        //==============================================================================
        /** This callback is made when items are changed in the menu bar model. */
        virtual z0 menuBarItemsChanged (MenuBarModel* menuBarModel) = 0;

        /** This callback is made when an application command is invoked that
            is represented by one of the items in the menu bar model.
        */
        virtual z0 menuCommandInvoked (MenuBarModel* menuBarModel,
                                         const ApplicationCommandTarget::InvocationInfo& info) = 0;

        /** Called when the menu bar is first activated or when the user finished interacting
            with the menu bar. */
        virtual z0 menuBarActivated (MenuBarModel* menuBarModel, b8 isActive);
    };

    /** Registers a listener for callbacks when the menu items in this model change.

        The listener object will get callbacks when this object's menuItemsChanged()
        method is called.

        @see removeListener
    */
    z0 addListener (Listener* listenerToAdd);

    /** Removes a listener.
        @see addListener
    */
    z0 removeListener (Listener* listenerToRemove);

    //==============================================================================
    /** This method must return a list of the names of the menus. */
    virtual StringArray getMenuBarNames() = 0;

    /** This should return the popup menu to display for a given top-level menu.

        @param topLevelMenuIndex    the index of the top-level menu to show
        @param menuName             the name of the top-level menu item to show
    */
    virtual PopupMenu getMenuForIndex (i32 topLevelMenuIndex,
                                       const Txt& menuName) = 0;

    /** This is called when a menu item has been clicked on.

        @param menuItemID           the item ID of the PopupMenu item that was selected
        @param topLevelMenuIndex    the index of the top-level menu from which the item was
                                    chosen (just in case you've used duplicate ID numbers
                                    on more than one of the popup menus)
    */
    virtual z0 menuItemSelected (i32 menuItemID,
                                   i32 topLevelMenuIndex) = 0;

    /** This is called when the user starts/stops navigating the menu bar.

        @param isActive              true when the user starts navigating the menu bar
    */
    virtual z0 menuBarActivated (b8 isActive);

    //==============================================================================
   #if DRX_MAC || DOXYGEN
    /** OSX ONLY - Sets the model that is currently being shown as the main
        menu bar at the top of the screen on the Mac.

        You can pass nullptr to stop the current model being displayed. Be careful
        not to delete a model while it is being used.

        An optional extra menu can be specified, containing items to add to the top of
        the apple menu. (Confusingly, the 'apple' menu isn't the one with a picture of
        an apple, it's the one next to it, with your application's name at the top
        and the services menu etc on it). When one of these items is selected, the
        menu bar model will be used to invoke it, and in the menuItemSelected() callback
        the topLevelMenuIndex parameter will be -1. If you pass in an extraAppleMenuItems
        object then newMenuBarModel must be non-null.

        If the recentItemsMenuName parameter is non-empty, then any sub-menus with this
        name will be replaced by OSX's special recent-files menu.
    */
    static z0 setMacMainMenu (MenuBarModel* newMenuBarModel,
                                const PopupMenu* extraAppleMenuItems = nullptr,
                                const Txt& recentItemsMenuName = Txt());

    /** OSX ONLY - Returns the menu model that is currently being shown as
        the main menu bar.
    */
    static MenuBarModel* getMacMainMenu();

    /** OSX ONLY - Returns the menu that was last passed as the extraAppleMenuItems
        argument to setMacMainMenu(), or nullptr if none was specified.
    */
    static const PopupMenu* getMacExtraAppleItemsMenu();
   #endif

    //==============================================================================
    /** @internal */
    z0 applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo&) override;
    /** @internal */
    z0 applicationCommandListChanged() override;
    /** @internal */
    z0 handleAsyncUpdate() override;
    /** @internal */
    z0 handleMenuBarActivate (b8 isActive);
private:
    ApplicationCommandManager* manager;
    ListenerList<Listener> listeners;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuBarModel)
};


} // namespace drx
