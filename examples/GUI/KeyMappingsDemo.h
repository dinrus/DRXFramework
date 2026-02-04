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

 name:             KeyMappingsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases key mapping features.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        KeyMappingsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

/** A list of the command IDs that this demo can perform. */
enum KeyPressCommandIDs
{
    buttonMoveUp = 1,
    buttonMoveRight,
    buttonMoveDown,
    buttonMoveLeft,
    nextButtonColor,
    previousButtonColor,
    nextBackgroundColor,
    previousBackgroundColor
};

//==============================================================================
/**
    This is a simple target for the key-presses which will live inside the demo component
    and contains a button that can be moved around with the arrow keys.
*/
class KeyPressTarget final : public Component,
                             public ApplicationCommandTarget
{
public:
    KeyPressTarget()
    {
        Array<Color> coloursToUse { Colors::darkblue, Colors::darkgrey, Colors::red,
                                     Colors::green, Colors::blue, Colors::hotpink };
        colours.addArray (coloursToUse);

        addAndMakeVisible (button);
    }

    //==============================================================================
    z0 resized() override
    {
        auto bounds = getLocalBounds();

        // keep the button on-screen
        if (buttonX < -150 || buttonX > bounds.getWidth()
            || buttonY < -30 || buttonY > bounds.getHeight())
        {
            buttonX = bounds.getCentreX() - 75;
            buttonY = bounds.getCentreY() - 15;
        }

        button.setBounds (buttonX, buttonY, 150, 30);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (colours.getUnchecked (backgroundColorIndex));
    }

    //==============================================================================
    /** No other command targets in this simple example so just return nullptr. */
    ApplicationCommandTarget* getNextCommandTarget() override   { return nullptr; }

    z0 getAllCommands (Array<CommandID>& commands) override
    {
        Array<CommandID> ids { KeyPressCommandIDs::buttonMoveUp, KeyPressCommandIDs::buttonMoveRight,
                               KeyPressCommandIDs::buttonMoveDown, KeyPressCommandIDs::buttonMoveLeft,
                               KeyPressCommandIDs::nextButtonColor, KeyPressCommandIDs::previousButtonColor,
                               KeyPressCommandIDs::nextBackgroundColor, KeyPressCommandIDs::previousBackgroundColor };

        commands.addArray (ids);
    }

    z0 getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
    {
        switch (commandID)
        {
            case KeyPressCommandIDs::buttonMoveUp:
                result.setInfo ("Move up", "Move the button up", "Button", 0);
                result.addDefaultKeypress (KeyPress::upKey, 0);
                break;
            case KeyPressCommandIDs::buttonMoveRight:
                result.setInfo ("Move right", "Move the button right", "Button", 0);
                result.addDefaultKeypress (KeyPress::rightKey, 0);
                break;
            case KeyPressCommandIDs::buttonMoveDown:
                result.setInfo ("Move down", "Move the button down", "Button", 0);
                result.addDefaultKeypress (KeyPress::downKey, 0);
                break;
            case KeyPressCommandIDs::buttonMoveLeft:
                result.setInfo ("Move left", "Move the button left", "Button", 0);
                result.addDefaultKeypress (KeyPress::leftKey, 0);
                break;
            case KeyPressCommandIDs::nextButtonColor:
                result.setInfo ("Next colour", "Change the colour of the button to the next in the list", "Button", 0);
                result.addDefaultKeypress (KeyPress::rightKey, ModifierKeys::shiftModifier);
                break;
            case KeyPressCommandIDs::previousButtonColor:
                result.setInfo ("Previous colour", "Change the colour of the button to the previous in the list", "Button", 0);
                result.addDefaultKeypress (KeyPress::leftKey, ModifierKeys::shiftModifier);
                break;
            case KeyPressCommandIDs::nextBackgroundColor:
                result.setInfo ("Next colour", "Change the colour of the background to the next in the list", "Other", 0);
                result.addDefaultKeypress (KeyPress::rightKey, ModifierKeys::commandModifier);
                break;
            case KeyPressCommandIDs::previousBackgroundColor:
                result.setInfo ("Previous colour", "Change the colour of the background to the previous in the list", "Other", 0);
                result.addDefaultKeypress (KeyPress::leftKey, ModifierKeys::commandModifier);
                break;
            default:
                break;
        }
    }

    b8 perform (const InvocationInfo& info) override
    {
        switch (info.commandID)
        {
            case KeyPressCommandIDs::buttonMoveUp:
                buttonY -= 5;
                resized();
                break;
            case KeyPressCommandIDs::buttonMoveRight:
                buttonX += 5;
                resized();
                break;
            case KeyPressCommandIDs::buttonMoveDown:
                buttonY += 5;
                resized();
                break;
            case KeyPressCommandIDs::buttonMoveLeft:
                buttonX -= 5;
                resized();
                break;
            case KeyPressCommandIDs::nextButtonColor:
                ++buttonColorIndex %= colours.size();
                button.setColor (TextButton::buttonColorId, colours.getUnchecked (buttonColorIndex));
                break;
            case KeyPressCommandIDs::previousButtonColor:
                --buttonColorIndex;
                if (buttonColorIndex < 0)
                    buttonColorIndex = colours.size() - 1;
                button.setColor (TextButton::buttonColorId, colours.getUnchecked (buttonColorIndex));
                break;
            case KeyPressCommandIDs::nextBackgroundColor:
                ++backgroundColorIndex %= colours.size();
                repaint();
                break;
            case KeyPressCommandIDs::previousBackgroundColor:
                --backgroundColorIndex;
                if (backgroundColorIndex < 0)
                    backgroundColorIndex = colours.size() - 1;
                repaint();
                break;
            default:
                return false;
        }

        return true;
    }

private:
    TextButton button;
    i32 buttonX = -200, buttonY = -200;

    Array<Color> colours;

    i32 buttonColorIndex     = 0;
    i32 backgroundColorIndex = 1;
};

//==============================================================================
class KeyMappingsDemo final : public Component
{
public:
    KeyMappingsDemo()
    {
        // register the commands that the target component can perform
        commandManager.registerAllCommandsForTarget (&keyTarget);

        setOpaque (true);
        addAndMakeVisible (keyMappingEditor);
        addAndMakeVisible (keyTarget);

        // add command manager key mappings as a KeyListener to the top-level component
        // so it is notified of key presses
        getTopLevelComponent()->addKeyListener (commandManager.getKeyMappings());

        setSize (500, 500);

        Timer::callAfterDelay (300, [this] { keyTarget.grabKeyboardFocus(); }); // ensure that key presses are sent to the KeyPressTarget object
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Color::greyLevel (0.93f)));
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();

        keyTarget       .setBounds (bounds.removeFromTop (bounds.getHeight() / 2).reduced (4));
        keyMappingEditor.setBounds (bounds.reduced (4));
    }

private:
   #if DRX_DEMO_RUNNER
    ApplicationCommandManager& commandManager = getGlobalCommandManager();
   #else
    ApplicationCommandManager commandManager;
   #endif

    KeyMappingEditorComponent keyMappingEditor  { *commandManager.getKeyMappings(), true};

    KeyPressTarget keyTarget;

    z0 lookAndFeelChanged() override
    {
        auto* lf = &LookAndFeel::getDefaultLookAndFeel();

        keyMappingEditor.setColors (lf->findColor (KeyMappingEditorComponent::backgroundColorId),
                                     lf->findColor (KeyMappingEditorComponent::textColorId));
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappingsDemo)
};
