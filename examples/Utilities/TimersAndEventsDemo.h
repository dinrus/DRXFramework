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

 name:             TimersAndEventsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Application using timers and events.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        TimersAndEventsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** Simple message that holds a Color. */
struct ColorMessage final : public Message
{
    ColorMessage (Color col)  : colour (col) {}

    /** Returns the colour of a ColorMessage of white if the message is not a ColorMessage. */
    static Color getColor (const Message& message)
    {
        if (auto* cm = dynamic_cast<const ColorMessage*> (&message))
            return cm->colour;

        return Colors::white;
    }

    Color colour;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorMessage)
};

//==============================================================================
/** Simple component that can be triggered to flash.
    The flash will then fade using a Timer to repaint itself and will send a change
    message once it is finished.
 */
class FlashingComponent final : public Component,
                                public MessageListener,
                                public ChangeBroadcaster,
                                private Timer
{
public:
    FlashingComponent() {}

    z0 startFlashing()
    {
        flashAlpha = 1.0f;
        startTimerHz (25);
    }

    /** Stops this component flashing without sending a change message. */
    z0 stopFlashing()
    {
        flashAlpha = 0.0f;
        stopTimer();
        repaint();
    }

    /** Sets the colour of the component. */
    z0 setFlashColor (const Color newColor)
    {
        colour = newColor;
        repaint();
    }

    /** Draws our component. */
    z0 paint (Graphics& g) override
    {
        g.setColor (colour.overlaidWith (Colors::white.withAlpha (flashAlpha)));
        g.fillEllipse (getLocalBounds().toFloat());
    }

    /** Custom mouse handler to trigger a flash. */
    z0 mouseDown (const MouseEvent&) override
    {
        startFlashing();
    }

    /** Message listener callback used to change our colour */
    z0 handleMessage (const Message& message) override
    {
        setFlashColor (ColorMessage::getColor (message));
    }

private:
    f32 flashAlpha = 0.0f;
    Color colour { Colors::red };

    z0 timerCallback() override
    {
        // Reduce the alpha level of the flash slightly so it fades out
        flashAlpha -= 0.075f;

        if (flashAlpha < 0.05f)
        {
            stopFlashing();
            sendChangeMessage();
            // Once we've finished flashing send a change message to trigger the next component to flash
        }

        repaint();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlashingComponent)
};

//==============================================================================
class TimersAndEventsDemo final : public Component,
                                  private ChangeListener
{
public:
    TimersAndEventsDemo()
    {
        setOpaque (true);

        // Create and add our FlashingComponents with some random colours and sizes
        for (i32 i = 0; i < numFlashingComponents; ++i)
        {
            auto* newFlasher = new FlashingComponent();
            flashingComponents.add (newFlasher);

            newFlasher->setFlashColor (getRandomBrightColor());
            newFlasher->addChangeListener (this);

            auto diameter = 25 + random.nextInt (75);
            newFlasher->setSize (diameter, diameter);

            addAndMakeVisible (newFlasher);
        }

        addAndMakeVisible (stopButton);
        stopButton.onClick = [this] { stopButtonClicked(); };

        addAndMakeVisible (randomColorButton);
        randomColorButton.onClick = [this] { randomColorButtonClicked(); };

        // lay out our components in a pseudo random grid
        Rectangle<i32> area (0, 100, 150, 150);

        for (auto* comp : flashingComponents)
        {
            auto buttonArea = area.withSize (comp->getWidth(), comp->getHeight());
            buttonArea.translate (random.nextInt (area.getWidth()  - comp->getWidth()),
                                  random.nextInt (area.getHeight() - comp->getHeight()));
            comp->setBounds (buttonArea);

            area.translate (area.getWidth(), 0);

            // if we go off the right start a new row
            if (area.getRight() > (800 - area.getWidth()))
            {
                area.translate (0, area.getWidth());
                area.setX (0);
            }
        }

        setSize (600, 600);
    }

    ~TimersAndEventsDemo() override
    {
        for (auto* fc : flashingComponents)
            fc->removeChangeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Colors::darkgrey));
    }

    z0 paintOverChildren (Graphics& g) override
    {
        auto explanationArea = getLocalBounds().removeFromTop (100);

        AttributedString s;
        s.append ("Click on a circle to make it flash. When it has finished flashing it will send a message which causes the next circle to flash");
        s.append (newLine);
        s.append ("Click the \"Set Random Color\" button to change the colour of one of the circles.");
        s.append (newLine);
        s.setFont (FontOptions { 16.0f });
        s.setColor (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::defaultText, Colors::lightgrey));
        s.draw (g, explanationArea.reduced (10).toFloat());
    }

    z0 resized() override
    {
        auto area = getLocalBounds().removeFromBottom (40);
        randomColorButton.setBounds (area.removeFromLeft (166) .reduced (8));
        stopButton        .setBounds (area.removeFromRight (166).reduced (8));
    }

private:
    enum { numFlashingComponents = 9 };

    OwnedArray<FlashingComponent> flashingComponents;
    TextButton randomColorButton  { "Set Random Color" },
               stopButton          { "Stop" };
    Random random;

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        for (i32 i = 0; i < flashingComponents.size(); ++i)
            if (source == flashingComponents.getUnchecked (i))
                flashingComponents.getUnchecked ((i + 1) % flashingComponents.size())->startFlashing();
    }

    z0 randomColorButtonClicked()
    {
        // Here we post a new ColorMessage with a random colour to a random flashing component.
        // This will send a message to the component asynchronously and trigger its handleMessage callback
        flashingComponents.getUnchecked (random.nextInt (flashingComponents.size()))->postMessage (new ColorMessage (getRandomBrightColor()));
    }

    z0 stopButtonClicked()
    {
        for (auto* fc : flashingComponents)
            fc->stopFlashing();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimersAndEventsDemo)
};
