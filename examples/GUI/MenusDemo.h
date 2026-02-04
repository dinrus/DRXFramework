/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             MenusDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases menu features.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MenusDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/**
    This struct contains a header component that will be used when the burger menu
    is enabled. It contains an icon that can be used to show the side panel containing
    the menu.
*/
struct BurgerMenuHeader final : public Component
{
    BurgerMenuHeader (SidePanel& sp)
        : sidePanel (sp)
    {
        static u8k burgerMenuPathData[]
            = { 110,109,0,0,128,64,0,0,32,65,108,0,0,224,65,0,0,32,65,98,254,212,232,65,0,0,32,65,0,0,240,65,252,
                169,17,65,0,0,240,65,0,0,0,65,98,0,0,240,65,8,172,220,64,254,212,232,65,0,0,192,64,0,0,224,65,0,0,
                192,64,108,0,0,128,64,0,0,192,64,98,16,88,57,64,0,0,192,64,0,0,0,64,8,172,220,64,0,0,0,64,0,0,0,65,
                98,0,0,0,64,252,169,17,65,16,88,57,64,0,0,32,65,0,0,128,64,0,0,32,65,99,109,0,0,224,65,0,0,96,65,108,
                0,0,128,64,0,0,96,65,98,16,88,57,64,0,0,96,65,0,0,0,64,4,86,110,65,0,0,0,64,0,0,128,65,98,0,0,0,64,
                254,212,136,65,16,88,57,64,0,0,144,65,0,0,128,64,0,0,144,65,108,0,0,224,65,0,0,144,65,98,254,212,232,
                65,0,0,144,65,0,0,240,65,254,212,136,65,0,0,240,65,0,0,128,65,98,0,0,240,65,4,86,110,65,254,212,232,
                65,0,0,96,65,0,0,224,65,0,0,96,65,99,109,0,0,224,65,0,0,176,65,108,0,0,128,64,0,0,176,65,98,16,88,57,
                64,0,0,176,65,0,0,0,64,2,43,183,65,0,0,0,64,0,0,192,65,98,0,0,0,64,254,212,200,65,16,88,57,64,0,0,208,
                65,0,0,128,64,0,0,208,65,108,0,0,224,65,0,0,208,65,98,254,212,232,65,0,0,208,65,0,0,240,65,254,212,
                200,65,0,0,240,65,0,0,192,65,98,0,0,240,65,2,43,183,65,254,212,232,65,0,0,176,65,0,0,224,65,0,0,176,
                65,99,101,0,0 };

        Path p;
        p.loadPathFromData (burgerMenuPathData, sizeof (burgerMenuPathData));
        burgerButton.setShape (p, true, true, false);

        burgerButton.onClick = [this] { showOrHide(); };
        addAndMakeVisible (burgerButton);
    }

    ~BurgerMenuHeader() override
    {
        sidePanel.showOrHide (false);
    }

private:
    z0 paint (Graphics& g) override
    {
        auto titleBarBackgroundColor = getLookAndFeel().findColor (ResizableWindow::backgroundColorId)
                                                        .darker();

        g.setColor (titleBarBackgroundColor);
        g.fillRect (getLocalBounds());
    }

    z0 resized() override
    {
        auto r = getLocalBounds();

        burgerButton.setBounds (r.removeFromRight (40).withSizeKeepingCentre (20, 20));

        titleLabel.setFont (FontOptions ((f32) getHeight() * 0.5f, Font::plain));
        titleLabel.setBounds (r);
    }

    z0 showOrHide()
    {
        sidePanel.showOrHide (! sidePanel.isPanelShowing());
    }

    SidePanel& sidePanel;

    Label titleLabel         { "titleLabel", "DRX Demo" };
    ShapeButton burgerButton { "burgerButton", Colors::lightgrey, Colors::lightgrey, Colors::white };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BurgerMenuHeader)
};

//==============================================================================
class MenusDemo final : public Component,
                        public ApplicationCommandTarget,
                        public MenuBarModel
{
public:
    //==============================================================================
    /** A list of the commands that this demo responds to. */
    enum CommandIDs
    {
        menuPositionInsideWindow = 1,
        menuPositionGlobalMenuBar,
        menuPositionBurgerMenu,
        outerColorRed,
        outerColorGreen,
        outerColorBlue,
        innerColorRed,
        innerColorGreen,
        innerColorBlue
    };

    //==============================================================================
    /** Represents the possible menu positions. */
    enum class MenuBarPosition
    {
        window,
        global,
        burger
    };

    //==============================================================================
    MenusDemo()
    {
        menuBar.reset (new MenuBarComponent (this));
        addAndMakeVisible (menuBar.get());
        setApplicationCommandManagerToWatch (&commandManager);
        commandManager.registerAllCommandsForTarget (this);

        // this ensures that commands invoked on the DemoRunner application are correctly
        // forwarded to this demo
        commandManager.setFirstCommandTarget (this);

        // this lets the command manager use keypresses that arrive in our window to send out commands
        addKeyListener (commandManager.getKeyMappings());

        addChildComponent (menuHeader);
        addAndMakeVisible (outerCommandTarget);
        addAndMakeVisible (sidePanel);

        setWantsKeyboardFocus (true);

        setSize (500, 500);
    }

    ~MenusDemo() override
    {
       #if DRX_MAC
        MenuBarModel::setMacMainMenu (nullptr);
       #endif

        commandManager.setFirstCommandTarget (nullptr);
    }

    z0 resized() override
    {
        auto b = getLocalBounds();

        if (menuBarPosition == MenuBarPosition::window)
        {
            menuBar->setBounds (b.removeFromTop (LookAndFeel::getDefaultLookAndFeel()
                                                             .getDefaultMenuBarHeight()));
        }
        else if (menuBarPosition == MenuBarPosition::burger)
        {
            menuHeader.setBounds (b.removeFromTop (40));
        }

        outerCommandTarget.setBounds (b);
    }

    //==============================================================================
    StringArray getMenuBarNames() override
    {
        return { "Menu Position", "Outer Color", "Inner Color" };
    }

    PopupMenu getMenuForIndex (i32 menuIndex, const Txt& /*menuName*/) override
    {
        PopupMenu menu;

        if (menuIndex == 0)
        {
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionInsideWindow);
           #if DRX_MAC
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionGlobalMenuBar);
           #endif
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionBurgerMenu);
        }
        else if (menuIndex == 1)
        {
            menu.addCommandItem (&commandManager, CommandIDs::outerColorRed);
            menu.addCommandItem (&commandManager, CommandIDs::outerColorGreen);
            menu.addCommandItem (&commandManager, CommandIDs::outerColorBlue);
        }
        else if (menuIndex == 2)
        {
            menu.addCommandItem (&commandManager, CommandIDs::innerColorRed);
            menu.addCommandItem (&commandManager, CommandIDs::innerColorGreen);
            menu.addCommandItem (&commandManager, CommandIDs::innerColorBlue);
        }

        return menu;
    }

    z0 menuItemSelected (i32 /*menuItemID*/, i32 /*topLevelMenuIndex*/) override {}

    //==============================================================================
    // The following methods implement the ApplicationCommandTarget interface, allowing
    // this window to publish a set of actions it can perform, and which can be mapped
    // onto menus, keypresses, etc.

    ApplicationCommandTarget* getNextCommandTarget() override
    {
        return &outerCommandTarget;
    }

    z0 getAllCommands (Array<CommandID>& c) override
    {
        Array<CommandID> commands { CommandIDs::menuPositionInsideWindow,
                                    CommandIDs::menuPositionGlobalMenuBar,
                                    CommandIDs::menuPositionBurgerMenu };
        c.addArray (commands);
    }

    z0 getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
    {
        switch (commandID)
        {
            case CommandIDs::menuPositionInsideWindow:
                result.setInfo ("Inside Window", "Places the menu bar inside the application window", "Menu", 0);
                result.setTicked (menuBarPosition == MenuBarPosition::window);
                result.addDefaultKeypress ('w', ModifierKeys::shiftModifier);
                break;
            case CommandIDs::menuPositionGlobalMenuBar:
                result.setInfo ("Global", "Uses a global menu bar", "Menu", 0);
                result.setTicked (menuBarPosition == MenuBarPosition::global);
                result.addDefaultKeypress ('g', ModifierKeys::shiftModifier);
                break;
            case CommandIDs::menuPositionBurgerMenu:
                result.setInfo ("Burger Menu", "Uses a burger menu", "Menu", 0);
                result.setTicked (menuBarPosition == MenuBarPosition::burger);
                result.addDefaultKeypress ('b', ModifierKeys::shiftModifier);
                break;
            default:
                break;
        }
    }

    b8 perform (const InvocationInfo& info) override
    {
        switch (info.commandID)
        {
            case CommandIDs::menuPositionInsideWindow:
                setMenuBarPosition (MenuBarPosition::window);
                break;
            case CommandIDs::menuPositionGlobalMenuBar:
                setMenuBarPosition (MenuBarPosition::global);
                break;
            case CommandIDs::menuPositionBurgerMenu:
                setMenuBarPosition (MenuBarPosition::burger);
                break;
            default:
                return false;
        }

        return true;
    }

    z0 setMenuBarPosition (MenuBarPosition newPosition)
    {
        if (menuBarPosition != newPosition)
        {
            menuBarPosition = newPosition;

            if (menuBarPosition != MenuBarPosition::burger)
                sidePanel.showOrHide (false);

           #if DRX_MAC
            MenuBarModel::setMacMainMenu (menuBarPosition == MenuBarPosition::global ? this : nullptr);
           #endif

            menuBar->setVisible   (menuBarPosition == MenuBarPosition::window);
            burgerMenu.setModel   (menuBarPosition == MenuBarPosition::burger ? this : nullptr);
            menuHeader.setVisible (menuBarPosition == MenuBarPosition::burger);

            sidePanel.setContent  (menuBarPosition == MenuBarPosition::burger ? &burgerMenu : nullptr, false);
            menuItemsChanged();

            resized();
        }
    }

private:
   #if DRX_DEMO_RUNNER
    ApplicationCommandManager& commandManager = getGlobalCommandManager();
   #else
    ApplicationCommandManager commandManager;
   #endif

    std::unique_ptr<MenuBarComponent> menuBar;
    MenuBarPosition menuBarPosition = MenuBarPosition::window;

    SidePanel sidePanel { "Menu", 300, false };

    BurgerMenuComponent burgerMenu;
    BurgerMenuHeader menuHeader { sidePanel };

    //==============================================================================
    /**
        Command messages that aren't handled in the main component will be passed
        to this class to respond to.
    */
    class OuterCommandTarget final : public Component,
                                     public ApplicationCommandTarget
    {
    public:
        OuterCommandTarget (ApplicationCommandManager& m)
            : commandManager (m),
              innerCommandTarget (commandManager)
        {
            commandManager.registerAllCommandsForTarget (this);

            addAndMakeVisible (innerCommandTarget);
        }

        z0 resized() override
        {
            innerCommandTarget.setBounds (getLocalBounds().reduced (50));
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (currentColor);
        }

        //==============================================================================
        ApplicationCommandTarget* getNextCommandTarget() override
        {
            return &innerCommandTarget;
        }

        z0 getAllCommands (Array<CommandID>& c) override
        {
            Array<CommandID> commands { CommandIDs::outerColorRed,
                                        CommandIDs::outerColorGreen,
                                        CommandIDs::outerColorBlue };

            c.addArray (commands);
        }

        z0 getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
        {
            switch (commandID)
            {
                case CommandIDs::outerColorRed:
                    result.setInfo ("Red", "Sets the outer colour to red", "Outer", 0);
                    result.setTicked (currentColor == Colors::red);
                    result.addDefaultKeypress ('r', ModifierKeys::commandModifier);
                    break;
                case CommandIDs::outerColorGreen:
                    result.setInfo ("Green", "Sets the outer colour to green", "Outer", 0);
                    result.setTicked (currentColor == Colors::green);
                    result.addDefaultKeypress ('g', ModifierKeys::commandModifier);
                    break;
                case CommandIDs::outerColorBlue:
                    result.setInfo ("Blue", "Sets the outer colour to blue", "Outer", 0);
                    result.setTicked (currentColor == Colors::blue);
                    result.addDefaultKeypress ('b', ModifierKeys::commandModifier);
                    break;
                default:
                    break;
            }
        }

        b8 perform (const InvocationInfo& info) override
        {
            switch (info.commandID)
            {
                case CommandIDs::outerColorRed:
                    currentColor = Colors::red;
                    break;
                case CommandIDs::outerColorGreen:
                    currentColor = Colors::green;
                    break;
                case CommandIDs::outerColorBlue:
                    currentColor = Colors::blue;
                    break;
                default:
                    return false;
            }

            repaint();
            return true;
        }

    private:
        //==============================================================================
        /**
            Command messages that aren't handled in the OuterCommandTarget will be passed
            to this class to respond to.
        */
        struct InnerCommandTarget final : public Component,
                                          public ApplicationCommandTarget
        {
            InnerCommandTarget (ApplicationCommandManager& m)
                : commandManager (m)
            {
                commandManager.registerAllCommandsForTarget (this);
            }

            z0 paint (Graphics& g) override
            {
                g.fillAll (currentColor);
            }

            //==============================================================================
            ApplicationCommandTarget* getNextCommandTarget() override
            {
                // this will return the next parent component that is an ApplicationCommandTarget
                return findFirstTargetParentComponent();
            }

            z0 getAllCommands (Array<CommandID>& c) override
            {
                Array<CommandID> commands { CommandIDs::innerColorRed,
                                            CommandIDs::innerColorGreen,
                                            CommandIDs::innerColorBlue };

                c.addArray (commands);
            }

            z0 getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
            {
                switch (commandID)
                {
                    case CommandIDs::innerColorRed:
                        result.setInfo ("Red", "Sets the inner colour to red", "Inner", 0);
                        result.setTicked (currentColor == Colors::red);
                        result.addDefaultKeypress ('r', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
                        break;
                    case CommandIDs::innerColorGreen:
                        result.setInfo ("Green", "Sets the inner colour to green", "Inner", 0);
                        result.setTicked (currentColor == Colors::green);
                        result.addDefaultKeypress ('g', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
                        break;
                    case CommandIDs::innerColorBlue:
                        result.setInfo ("Blue", "Sets the inner colour to blue", "Inner", 0);
                        result.setTicked (currentColor == Colors::blue);
                        result.addDefaultKeypress ('b', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
                        break;
                    default:
                        break;
                }
            }

            b8 perform (const InvocationInfo& info) override
            {
                switch (info.commandID)
                {
                    case CommandIDs::innerColorRed:
                        currentColor = Colors::red;
                        break;
                    case CommandIDs::innerColorGreen:
                        currentColor = Colors::green;
                        break;
                    case CommandIDs::innerColorBlue:
                        currentColor = Colors::blue;
                        break;
                    default:
                        return false;
                }

                repaint();
                return true;
            }

            ApplicationCommandManager& commandManager;

            Color currentColor { Colors::blue };
        };

        ApplicationCommandManager& commandManager;
        InnerCommandTarget innerCommandTarget;

        Color currentColor { Colors::red };
    };

    OuterCommandTarget outerCommandTarget { commandManager };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenusDemo)
};
