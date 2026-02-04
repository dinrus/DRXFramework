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
    A component containing controls to let the user change the audio settings of
    an AudioDeviceManager object.

    Very easy to use - just create one of these and show it to the user.

    @see AudioDeviceManager

    @tags{Audio}
*/
class DRX_API  AudioDeviceSelectorComponent  : public Component,
                                                private ChangeListener
{
public:
    //==============================================================================
    /** Creates the component.

        If your app needs only output channels, you might ask for a maximum of 0 input
        channels, and the component won't display any options for choosing the input
        channels. And likewise if you're doing an input-only app.

        @param deviceManager            the device manager that this component should control
        @param minAudioInputChannels    the minimum number of audio input channels that the application needs
        @param maxAudioInputChannels    the maximum number of audio input channels that the application needs
        @param minAudioOutputChannels   the minimum number of audio output channels that the application needs
        @param maxAudioOutputChannels   the maximum number of audio output channels that the application needs
        @param showMidiInputOptions     if true, the component will allow the user to select which midi inputs are enabled
        @param showMidiOutputSelector   if true, the component will let the user choose a default midi output device
        @param showChannelsAsStereoPairs    if true, channels will be treated as pairs; if false, channels will be
                                        treated as a set of separate mono channels.
        @param hideAdvancedOptionsWithButton    if true, only the minimum amount of UI components
                                        are shown, with an "advanced" button that shows the rest of them
    */
    AudioDeviceSelectorComponent (AudioDeviceManager& deviceManager,
                                  i32 minAudioInputChannels,
                                  i32 maxAudioInputChannels,
                                  i32 minAudioOutputChannels,
                                  i32 maxAudioOutputChannels,
                                  b8 showMidiInputOptions,
                                  b8 showMidiOutputSelector,
                                  b8 showChannelsAsStereoPairs,
                                  b8 hideAdvancedOptionsWithButton);

    /** Destructor */
    ~AudioDeviceSelectorComponent() override;

    /** The device manager that this component is controlling */
    AudioDeviceManager& deviceManager;

    /** Sets the standard height used for items in the panel. */
    z0 setItemHeight (i32 itemHeight);

    /** Returns the standard height used for items in the panel. */
    i32 getItemHeight() const noexcept      { return itemHeight; }

    /** Returns the ListBox that's being used to show the midi inputs, or nullptr if there isn't one. */
    ListBox* getMidiInputSelectorListBox() const noexcept;

    //==============================================================================
    /** @internal */
    z0 resized() override;

    /** @internal */
    z0 childBoundsChanged (Component* child) override;

private:
    //==============================================================================
    z0 handleBluetoothButton();
    z0 updateDeviceType();
    z0 changeListenerCallback (ChangeBroadcaster*) override;
    z0 updateAllControls();

    std::unique_ptr<ComboBox> deviceTypeDropDown;
    std::unique_ptr<Label> deviceTypeDropDownLabel;
    std::unique_ptr<Component> audioDeviceSettingsComp;
    Txt audioDeviceSettingsCompType;
    i32 itemHeight = 0;
    i32k minOutputChannels, maxOutputChannels, minInputChannels, maxInputChannels;
    const b8 showChannelsAsStereoPairs;
    const b8 hideAdvancedOptionsWithButton;

    class MidiInputSelectorComponentListBox;
    class MidiOutputSelector;

    Array<MidiDeviceInfo> currentMidiOutputs;
    std::unique_ptr<MidiInputSelectorComponentListBox> midiInputsList;
    std::unique_ptr<MidiOutputSelector> midiOutputSelector;
    std::unique_ptr<Label> midiInputsLabel, midiOutputLabel;
    std::unique_ptr<TextButton> bluetoothButton;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDeviceSelectorComponent)
};

} // namespace drx
