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

class AudioIODevice;

/**
    Additional information that may be passed to the AudioIODeviceCallback.

    @tags{Audio}
*/
struct AudioIODeviceCallbackContext
{
    /** If the host provides this information, this field will be set to point to
        an integer holding the current value; otherwise, this will be nullptr.
    */
    const zu64* hostTimeNs = nullptr;
};

//==============================================================================
/**
    One of these is passed to an AudioIODevice object to stream the audio data
    in and out.

    The AudioIODevice will repeatedly call this class's audioDeviceIOCallbackWithContext()
    method on its own high-priority audio thread, when it needs to send or receive
    the next block of data.

    @see AudioIODevice, AudioDeviceManager

    @tags{Audio}
*/
class DRX_API  AudioIODeviceCallback
{
public:
    /** Destructor. */
    virtual ~AudioIODeviceCallback()  = default;

    /** Processes a block of incoming and outgoing audio data.

        The subclass's implementation should use the incoming audio for whatever
        purposes it needs to, and must fill all the output channels with the next
        block of output data before returning.

        The channel data is arranged with the same array indices as the channel name
        array returned by AudioIODevice::getOutputChannelNames(), but those channels
        that aren't specified in AudioIODevice::open() will have a null pointer for their
        associated channel, so remember to check for this.

        @param inputChannelData     a set of arrays containing the audio data for each
                                    incoming channel - this data is valid until the function
                                    returns. There will be one channel of data for each input
                                    channel that was enabled when the audio device was opened
                                    (see AudioIODevice::open())
        @param numInputChannels     the number of pointers to channel data in the
                                    inputChannelData array.
        @param outputChannelData    a set of arrays which need to be filled with the data
                                    that should be sent to each outgoing channel of the device.
                                    There will be one channel of data for each output channel
                                    that was enabled when the audio device was opened (see
                                    AudioIODevice::open())
                                    The initial contents of the array is undefined, so the
                                    callback function must fill all the channels with zeros if
                                    its output is silence. Failing to do this could cause quite
                                    an unpleasant noise!
        @param numOutputChannels    the number of pointers to channel data in the
                                    outputChannelData array.
        @param numSamples           the number of samples in each channel of the input and
                                    output arrays. The number of samples will depend on the
                                    audio device's buffer size and will usually remain constant,
                                    although this isn't guaranteed. For example, on Android,
                                    on devices which support it, Android will chop up your audio
                                    processing into several smaller callbacks to ensure higher audio
                                    performance. So make sure your code can cope with reasonable
                                    changes in the buffer size from one callback to the next.
        @param context              Additional information that may be passed to the
                                    AudioIODeviceCallback.
    */
    virtual z0 audioDeviceIOCallbackWithContext (const f32* const* inputChannelData,
                                                   i32 numInputChannels,
                                                   f32* const* outputChannelData,
                                                   i32 numOutputChannels,
                                                   i32 numSamples,
                                                   const AudioIODeviceCallbackContext& context);

    /** Called to indicate that the device is about to start calling back.

        This will be called just before the audio callbacks begin, either when this
        callback has just been added to an audio device, or after the device has been
        restarted because of a sample-rate or block-size change.

        You can use this opportunity to find out the sample rate and block size
        that the device is going to use by calling the AudioIODevice::getCurrentSampleRate()
        and AudioIODevice::getCurrentBufferSizeSamples() on the supplied pointer.

        @param device       the audio IO device that will be used to drive the callback.
                            Note that if you're going to store this this pointer, it is
                            only valid until the next time that audioDeviceStopped is called.
    */
    virtual z0 audioDeviceAboutToStart (AudioIODevice* device) = 0;

    /** Called to indicate that the device has stopped. */
    virtual z0 audioDeviceStopped() = 0;

    /** This can be overridden to be told if the device generates an error while operating.
        Be aware that this could be called by any thread! And not all devices perform
        this callback.
    */
    virtual z0 audioDeviceError (const Txt& errorMessage);
};

//==============================================================================
/**
    Base class for an audio device with synchronised input and output channels.

    Subclasses of this are used to implement different protocols such as DirectSound,
    ASIO, CoreAudio, etc.

    To create one of these, you'll need to use the AudioIODeviceType class - see the
    documentation for that class for more info.

    For an easier way of managing audio devices and their settings, have a look at the
    AudioDeviceManager class.

    @see AudioIODeviceType, AudioDeviceManager

    @tags{Audio}
*/
class DRX_API  AudioIODevice
{
public:
    /** Destructor. */
    virtual ~AudioIODevice();

    //==============================================================================
    /** Returns the device's name, (as set in the constructor). */
    const Txt& getName() const noexcept                          { return name; }

    /** Returns the type of the device.

        E.g. "CoreAudio", "ASIO", etc. - this comes from the AudioIODeviceType that created it.
    */
    const Txt& getTypeName() const noexcept                      { return typeName; }

    //==============================================================================
    /** Returns the names of all the available output channels on this device.
        To find out which of these are currently in use, call getActiveOutputChannels().
    */
    virtual StringArray getOutputChannelNames() = 0;

    /** Returns the names of all the available input channels on this device.
        To find out which of these are currently in use, call getActiveInputChannels().
    */
    virtual StringArray getInputChannelNames() = 0;

    //==============================================================================
    /** For devices that support a default layout, returns the channels that are enabled in the
        default layout.

        Returns nullopt if the device doesn't supply a default layout.
    */
    virtual std::optional<BigInteger> getDefaultOutputChannels() const { return {}; }

    /** For devices that support a default layout, returns the channels that are enabled in the
        default layout.

        Returns nullopt if the device doesn't supply a default layout.
    */
    virtual std::optional<BigInteger> getDefaultInputChannels()  const { return {}; }

    //==============================================================================
    /** Returns the set of sample-rates this device supports.
        @see getCurrentSampleRate
    */
    virtual Array<f64> getAvailableSampleRates() = 0;

    /** Returns the set of buffer sizes that are available.
        @see getCurrentBufferSizeSamples, getDefaultBufferSize
    */
    virtual Array<i32> getAvailableBufferSizes() = 0;

    /** Returns the default buffer-size to use.
        @returns a number of samples
        @see getAvailableBufferSizes
    */
    virtual i32 getDefaultBufferSize() = 0;

    //==============================================================================
    /** Tries to open the device ready to play.

        @param inputChannels        a BigInteger in which a set bit indicates that the corresponding
                                    input channel should be enabled
        @param outputChannels       a BigInteger in which a set bit indicates that the corresponding
                                    output channel should be enabled
        @param sampleRate           the sample rate to try to use - to find out which rates are
                                    available, see getAvailableSampleRates()
        @param bufferSizeSamples    the size of i/o buffer to use - to find out the available buffer
                                    sizes, see getAvailableBufferSizes()
        @returns    an error description if there's a problem, or an empty string if it succeeds in
                    opening the device
        @see close
    */
    virtual Txt open (const BigInteger& inputChannels,
                         const BigInteger& outputChannels,
                         f64 sampleRate,
                         i32 bufferSizeSamples) = 0;

    /** Closes and releases the device if it's open. */
    virtual z0 close() = 0;

    /** Возвращает true, если the device is still open.

        A device might spontaneously close itself if something goes wrong, so this checks if
        it's still open.
    */
    virtual b8 isOpen() = 0;

    /** Starts the device actually playing.

        This must be called after the device has been opened.

        @param callback     the callback to use for streaming the data.
        @see AudioIODeviceCallback, open
    */
    virtual z0 start (AudioIODeviceCallback* callback) = 0;

    /** Stops the device playing.

        Once a device has been started, this will stop it. Any pending calls to the
        callback class will be flushed before this method returns.
    */
    virtual z0 stop() = 0;

    /** Возвращает true, если the device is still calling back.

        The device might mysteriously stop, so this checks whether it's
        still playing.
    */
    virtual b8 isPlaying() = 0;

    /** Returns the last error that happened if anything went wrong. */
    virtual Txt getLastError() = 0;

    //==============================================================================
    /** Returns the buffer size that the device is currently using.

        If the device isn't actually open, this value doesn't really mean much.
    */
    virtual i32 getCurrentBufferSizeSamples() = 0;

    /** Returns the sample rate that the device is currently using.

        If the device isn't actually open, this value doesn't really mean much.
    */
    virtual f64 getCurrentSampleRate() = 0;

    /** Returns the device's current physical bit-depth.

        If the device isn't actually open, this value doesn't really mean much.
    */
    virtual i32 getCurrentBitDepth() = 0;

    /** Returns a mask showing which of the available output channels are currently
        enabled.
        @see getOutputChannelNames
    */
    virtual BigInteger getActiveOutputChannels() const = 0;

    /** Returns a mask showing which of the available input channels are currently
        enabled.
        @see getInputChannelNames
    */
    virtual BigInteger getActiveInputChannels() const = 0;

    /** Returns the device's output latency.

        This is the delay in samples between a callback getting a block of data, and
        that data actually getting played.
    */
    virtual i32 getOutputLatencyInSamples() = 0;

    /** Returns the device's input latency.

        This is the delay in samples between some audio actually arriving at the soundcard,
        and the callback getting passed this block of data.
    */
    virtual i32 getInputLatencyInSamples() = 0;

    /** Returns the workgroup for this device. */
    virtual AudioWorkgroup getWorkgroup() const { return {}; }

    //==============================================================================
    /** True if this device can show a pop-up control panel for editing its settings.

        This is generally just true of ASIO devices. If true, you can call showControlPanel()
        to display it.
    */
    virtual b8 hasControlPanel() const;

    /** Shows a device-specific control panel if there is one.

        This should only be called for devices which return true from hasControlPanel().
    */
    virtual b8 showControlPanel();

    /** On devices which support it, this allows automatic gain control or other
        mic processing to be disabled.
        If the device doesn't support this operation, it'll return false.
    */
    virtual b8 setAudioPreprocessingEnabled (b8 shouldBeEnabled);

    //==============================================================================
    /** Returns the number of under- or over runs reported by the OS since
        playback/recording has started.

        This number may be different than determining the Xrun count manually (by
        measuring the time spent in the audio callback) as the OS may be doing
        some buffering internally - especially on mobile devices.

        Returns -1 if playback/recording has not started yet or if getting the underrun
        count не поддерживается for this device (Android SDK 23 and lower).
    */
    virtual i32 getXRunCount() const noexcept;

    //==============================================================================
protected:
    /** Creates a device, setting its name and type member variables. */
    AudioIODevice (const Txt& deviceName,
                   const Txt& typeName);

    /** @internal */
    Txt name, typeName;
};

} // namespace drx
