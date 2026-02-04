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
    Represents a type of audio driver, such as DirectSound, ASIO, CoreAudio, etc.

    To get a list of available audio driver types, use the AudioDeviceManager::createAudioDeviceTypes()
    method. Each of the objects returned can then be used to list the available
    devices of that type. E.g.
    @code
    OwnedArray<AudioIODeviceType> types;
    myAudioDeviceManager.createAudioDeviceTypes (types);

    for (i32 i = 0; i < types.size(); ++i)
    {
        Txt typeName (types[i]->getTypeName());  // This will be things like "DirectSound", "CoreAudio", etc.

        types[i]->scanForDevices();                 // This must be called before getting the list of devices

        StringArray deviceNames (types[i]->getDeviceNames());  // This will now return a list of available devices of this type

        for (i32 j = 0; j < deviceNames.size(); ++j)
        {
            AudioIODevice* device = types[i]->createDevice (deviceNames [j]);

            ...
        }
    }
    @endcode

    For an easier way of managing audio devices and their settings, have a look at the
    AudioDeviceManager class.

    @see AudioIODevice, AudioDeviceManager

    @tags{Audio}
*/
class DRX_API  AudioIODeviceType
{
public:
    //==============================================================================
    /** Returns the name of this type of driver that this object manages.

        This will be something like "DirectSound", "ASIO", "CoreAudio", "ALSA", etc.
    */
    const Txt& getTypeName() const noexcept                      { return typeName; }

    //==============================================================================
    /** Refreshes the object's cached list of known devices.

        This must be called at least once before calling getDeviceNames() or any of
        the other device creation methods.
    */
    virtual z0 scanForDevices() = 0;

    /** Returns the list of available devices of this type.

        The scanForDevices() method must have been called to create this list.

        @param wantInputNames    for devices which have separate inputs and outputs
                                 this determines which list of names is returned
    */
    virtual StringArray getDeviceNames (b8 wantInputNames = false) const = 0;

    /** Returns the name of the default device.

        This will be one of the names from the getDeviceNames() list.

        @param forInput     if true, this means that a default input device should be
                            returned; if false, it should return the default output
    */
    virtual i32 getDefaultDeviceIndex (b8 forInput) const = 0;

    /** Returns the index of a given device in the list of device names.
        If asInput is true, it shows the index in the inputs list, otherwise it
        looks for it in the outputs list.
    */
    virtual i32 getIndexOfDevice (AudioIODevice* device, b8 asInput) const = 0;

    /** Возвращает true, если two different devices can be used for the input and output.
    */
    virtual b8 hasSeparateInputsAndOutputs() const = 0;

    /** Creates one of the devices of this type.

        The deviceName must be one of the strings returned by getDeviceNames(), and
        scanForDevices() must have been called before this method is used.
    */
    virtual AudioIODevice* createDevice (const Txt& outputDeviceName,
                                         const Txt& inputDeviceName) = 0;

    //==============================================================================
    /**
        A class for receiving events when audio devices are inserted or removed.

        You can register an AudioIODeviceType::Listener with an AudioIODeviceType object
        using the AudioIODeviceType::addListener() method, and it will be called when
        devices of that type are added or removed.

        @see AudioIODeviceType::addListener, AudioIODeviceType::removeListener
    */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** Called when the list of available audio devices changes. */
        virtual z0 audioDeviceListChanged() = 0;
    };

    /** Adds a listener that will be called when this type of device is added or
        removed from the system.
    */
    z0 addListener (Listener* listener);

    /** Removes a listener that was previously added with addListener(). */
    z0 removeListener (Listener* listener);

    //==============================================================================
    /** Destructor. */
    virtual ~AudioIODeviceType();

    //==============================================================================
    /** Creates a CoreAudio device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_CoreAudio();
    /** Creates an iOS device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_iOSAudio();
    /** Creates a WASAPI device type in the specified mode if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_WASAPI (WASAPIDeviceMode deviceMode);
    /** Creates a DirectSound device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_DirectSound();
    /** Creates an ASIO device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_ASIO();
    /** Creates an ALSA device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_ALSA();
    /** Creates a JACK device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_JACK();
    /** Creates an Android device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_Android();
    /** Creates an Android OpenSLES device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_OpenSLES();
    /** Creates an Oboe device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_Oboe();
    /** Creates a Bela device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_Bela();

   #ifndef DOXYGEN
    [[deprecated ("You should call the method which takes a WASAPIDeviceMode instead.")]]
    static AudioIODeviceType* createAudioIODeviceType_WASAPI (b8 exclusiveMode);
   #endif

protected:
    explicit AudioIODeviceType (const Txt& typeName);

    /** Synchronously calls all the registered device list change listeners. */
    z0 callDeviceChangeListeners();

private:
    Txt typeName;
    ListenerList<Listener> listeners;

    DRX_DECLARE_NON_COPYABLE (AudioIODeviceType)
};

} // namespace drx
