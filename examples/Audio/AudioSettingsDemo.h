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

 name:             AudioSettingsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Displays information about audio devices.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioSettingsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class AudioSettingsDemo final : public Component,
                                public ChangeListener
{
public:
    AudioSettingsDemo()
    {
        setOpaque (true);

       #ifndef DRX_DEMO_RUNNER
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (b8 granted)
                                     {
                                         i32 numInputChannels = granted ? 2 : 0;
                                         audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                     });
       #endif

        audioSetupComp.reset (new AudioDeviceSelectorComponent (audioDeviceManager,
                                                                0, 256, 0, 256, true, true, true, false));
        addAndMakeVisible (audioSetupComp.get());

        addAndMakeVisible (diagnosticsBox);
        diagnosticsBox.setMultiLine (true);
        diagnosticsBox.setReturnKeyStartsNewLine (true);
        diagnosticsBox.setReadOnly (true);
        diagnosticsBox.setScrollbarsShown (true);
        diagnosticsBox.setCaretVisible (false);
        diagnosticsBox.setPopupMenuEnabled (true);

        audioDeviceManager.addChangeListener (this);

        logMessage ("Audio device diagnostics:\n");
        dumpDeviceInfo();

        setSize (500, 600);
    }

    ~AudioSettingsDemo() override
    {
        audioDeviceManager.removeChangeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto r =  getLocalBounds().reduced (4);
        audioSetupComp->setBounds (r.removeFromTop (proportionOfHeight (0.65f)));
        diagnosticsBox.setBounds (r);
    }

    z0 dumpDeviceInfo()
    {
        logMessage ("--------------------------------------");
        logMessage ("Current audio device type: " + (audioDeviceManager.getCurrentDeviceTypeObject() != nullptr
                                                     ? audioDeviceManager.getCurrentDeviceTypeObject()->getTypeName()
                                                     : "<none>"));

        if (AudioIODevice* device = audioDeviceManager.getCurrentAudioDevice())
        {
            logMessage ("Current audio device: "   + device->getName().quoted());
            logMessage ("Sample rate: "    + Txt (device->getCurrentSampleRate()) + " Hz");
            logMessage ("Block size: "     + Txt (device->getCurrentBufferSizeSamples()) + " samples");
            logMessage ("Output Latency: " + Txt (device->getOutputLatencyInSamples())   + " samples");
            logMessage ("Input Latency: "  + Txt (device->getInputLatencyInSamples())    + " samples");
            logMessage ("Bit depth: "      + Txt (device->getCurrentBitDepth()));
            logMessage ("Input channel names: "    + device->getInputChannelNames().joinIntoString (", "));
            logMessage ("Active input channels: "  + getListOfActiveBits (device->getActiveInputChannels()));
            logMessage ("Output channel names: "   + device->getOutputChannelNames().joinIntoString (", "));
            logMessage ("Active output channels: " + getListOfActiveBits (device->getActiveOutputChannels()));
        }
        else
        {
            logMessage ("No audio device open");
        }
    }

    z0 logMessage (const Txt& m)
    {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret (m + newLine);
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef DRX_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager() };
   #endif

    std::unique_ptr<AudioDeviceSelectorComponent> audioSetupComp;
    TextEditor diagnosticsBox;

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        dumpDeviceInfo();
    }

    z0 lookAndFeelChanged() override
    {
        diagnosticsBox.applyFontToAllText (diagnosticsBox.getFont());
    }

    static Txt getListOfActiveBits (const BigInteger& b)
    {
        StringArray bits;

        for (i32 i = 0; i <= b.getHighestBit(); ++i)
            if (b[i])
                bits.add (Txt (i));

        return bits.joinIntoString (", ");
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettingsDemo)
};
