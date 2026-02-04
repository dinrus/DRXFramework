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
    Wrapper class to continuously stream audio from an audio source to an
    AudioIODevice.

    This object acts as an AudioIODeviceCallback, so can be attached to an
    output device, and will stream audio from an AudioSource.

    @tags{Audio}
*/
class DRX_API  AudioSourcePlayer  : public AudioIODeviceCallback
{
public:
    //==============================================================================
    /** Creates an empty AudioSourcePlayer. */
    AudioSourcePlayer();

    /** Destructor.

        Make sure this object isn't still being used by an AudioIODevice before
        deleting it!
    */
    ~AudioSourcePlayer() override;

    //==============================================================================
    /** Changes the current audio source to play from.

        If the source passed in is already being used, this method will do nothing.
        If the source is not null, its prepareToPlay() method will be called
        before it starts being used for playback.

        If there's another source currently playing, its releaseResources() method
        will be called after it has been swapped for the new one.

        @param newSource                the new source to use - this will NOT be deleted
                                        by this object when no longer needed, so it's the
                                        caller's responsibility to manage it.
    */
    z0 setSource (AudioSource* newSource);

    /** Returns the source that's playing.
        May return nullptr if there's no source.
    */
    AudioSource* getCurrentSource() const noexcept      { return source; }

    /** Sets a gain to apply to the audio data.
        @see getGain
    */
    z0 setGain (f32 newGain) noexcept;

    /** Returns the current gain.
        @see setGain
    */
    f32 getGain() const noexcept                      { return gain; }

    //==============================================================================
    /** Implementation of the AudioIODeviceCallbackWithContext method. */
    z0 audioDeviceIOCallbackWithContext (const f32* const* inputChannelData,
                                           i32 totalNumInputChannels,
                                           f32* const* outputChannelData,
                                           i32 totalNumOutputChannels,
                                           i32 numSamples,
                                           const AudioIODeviceCallbackContext& context) override;

    /** Implementation of the AudioIODeviceCallback method. */
    z0 audioDeviceAboutToStart (AudioIODevice* device) override;

    /** Implementation of the AudioIODeviceCallback method. */
    z0 audioDeviceStopped() override;

    /** An alternative method for initialising the source without an AudioIODevice. */
    z0 prepareToPlay (f64 sampleRate, i32 blockSize);

private:
    //==============================================================================
    CriticalSection readLock;
    AudioSource* source = nullptr;
    f64 sampleRate = 0;
    i32 bufferSize = 0;
    f32* channels[128];
    f32* outputChans[128];
    const f32* inputChans[128];
    AudioBuffer<f32> tempBuffer;
    f32 lastGain = 1.0f;
    std::atomic<f32> gain { 1.0f };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSourcePlayer)
};

} // namespace drx
