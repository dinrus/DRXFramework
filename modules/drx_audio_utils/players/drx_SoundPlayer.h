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
    A simple sound player that you can add to the AudioDeviceManager to play
    simple sounds.

    @see AudioProcessor, AudioProcessorGraph

    @tags{Audio}
*/
class DRX_API  SoundPlayer   : public AudioIODeviceCallback
{
public:
    //==============================================================================
    SoundPlayer();

    /** Destructor. */
    ~SoundPlayer() override;

    //==============================================================================
    /** Plays a sound from a file. */
    z0 play (const File& file);

    /** Convenient method to play sound from a DRX resource. */
    z0 play (ukk resourceData, size_t resourceSize);

    /** Plays the sound from an audio format reader.

        If deleteWhenFinished is true then the format reader will be
        automatically deleted once the sound has finished playing.
     */
    z0 play (AudioFormatReader* buffer, b8 deleteWhenFinished = false);

    /** Plays the sound from a positionable audio source.

        This will output the sound coming from a positionable audio source.
        This gives you slightly more control over the sound playback compared
        to  the other playSound methods. For example, if you would like to
        stop the sound prematurely you can call this method with a
        AudioTransportSource and then call audioSource->stop. Note that,
        you must call audioSource->start to start the playback, if your
        audioSource is a AudioTransportSource.

        The audio device manager will not hold any references to this audio
        source once the audio source has stopped playing for any reason,
        for example when the sound has finished playing or when you have
        called audioSource->stop. Therefore, calling audioSource->start() on
        a finished audioSource will not restart the sound again. If this is
        desired simply call playSound with the same audioSource again.

        @param audioSource   the audio source to play
        @param deleteWhenFinished If this is true then the audio source will
                                  be deleted once the device manager has finished
                                  playing.
        @param sampleRateOfSource The sample rate of the source. If this is zero, DRX
                                  will assume that the sample rate is the same as the
                                  audio output device.
     */
    z0 play (PositionableAudioSource* audioSource, b8 deleteWhenFinished = false,
               f64 sampleRateOfSource = 0.0);

    /** Plays the sound from an audio sample buffer.

        This will output the sound contained in an audio sample buffer. If
        deleteWhenFinished is true then the audio sample buffer will be
        automatically deleted once the sound has finished playing.

        If playOnAllOutputChannels is true, then if there are more output channels
        than buffer channels, then the ones that are available will be re-used on
        multiple outputs so that something is sent to all output channels. If it
        is false, then the buffer will just be played on the first output channels.
     */
    z0 play (AudioBuffer<f32>* buffer,
               b8 deleteWhenFinished = false,
               b8 playOnAllOutputChannels = false);

    /** Plays a beep through the current audio device.

        This is here to allow the audio setup UI panels to easily include a "test"
        button so that the user can check where the audio is coming from.
     */
    z0 playTestSound();

    //==============================================================================
    /** @internal */
    z0 audioDeviceIOCallbackWithContext (const f32* const*, i32, f32* const*, i32, i32, const AudioIODeviceCallbackContext&) override;
    /** @internal */
    z0 audioDeviceAboutToStart (AudioIODevice*) override;
    /** @internal */
    z0 audioDeviceStopped() override;
    /** @internal */
    z0 audioDeviceError (const Txt& errorMessage) override;

private:
    //==============================================================================
    AudioFormatManager formatManager;
    AudioSourcePlayer player;
    MixerAudioSource mixer;
    OwnedArray<AudioSource> sources;

    //==============================================================================
    f64 sampleRate;
    i32 bufferSize;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundPlayer)
};

} // namespace drx
