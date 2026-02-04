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
    An AudioSource that takes a PositionableAudioSource and allows it to be
    played, stopped, started, etc.

    This can also be told use a buffer and background thread to read ahead, and
    if can correct for different sample-rates.

    You may want to use one of these along with an AudioSourcePlayer and AudioIODevice
    to control playback of an audio file.

    @see AudioSource, AudioSourcePlayer

    @tags{Audio}
*/
class DRX_API  AudioTransportSource  : public PositionableAudioSource,
                                        public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an AudioTransportSource.
        After creating one of these, use the setSource() method to select an input source.
    */
    AudioTransportSource();

    /** Destructor. */
    ~AudioTransportSource() override;

    //==============================================================================
    /** Sets the reader that is being used as the input source.

        This will stop playback, reset the position to 0 and change to the new reader.

        The source passed in will not be deleted by this object, so must be managed by
        the caller.

        @param newSource                        the new input source to use. This may be a nullptr
        @param readAheadBufferSize              a size of buffer to use for reading ahead. If this
                                                is zero, no reading ahead will be done; if it's
                                                greater than zero, a BufferingAudioSource will be used
                                                to do the reading-ahead. If you set a non-zero value here,
                                                you'll also need to set the readAheadThread parameter.
        @param readAheadThread                  if you set readAheadBufferSize to a non-zero value, then
                                                you'll also need to supply this TimeSliceThread object for
                                                the background reader to use. The thread object must not be
                                                deleted while the AudioTransport source is still using it.
        @param sourceSampleRateToCorrectFor     if this is non-zero, it specifies the sample
                                                rate of the source, and playback will be sample-rate
                                                adjusted to maintain playback at the correct pitch. If
                                                this is 0, no sample-rate adjustment will be performed
        @param maxNumChannels                   the maximum number of channels that may need to be played
    */
    z0 setSource (PositionableAudioSource* newSource,
                    i32 readAheadBufferSize = 0,
                    TimeSliceThread* readAheadThread = nullptr,
                    f64 sourceSampleRateToCorrectFor = 0.0,
                    i32 maxNumChannels = 2);

    //==============================================================================
    /** Changes the current playback position in the source stream.

        The next time the getNextAudioBlock() method is called, this
        is the time from which it'll read data.

        @param newPosition    the new playback position in seconds

        @see getCurrentPosition
    */
    z0 setPosition (f64 newPosition);

    /** Returns the position that the next data block will be read from.
        This is a time in seconds.
    */
    f64 getCurrentPosition() const;

    /** Returns the stream's length in seconds. */
    f64 getLengthInSeconds() const;

    /** Возвращает true, если the player has stopped because its input stream ran out of data. */
    b8 hasStreamFinished() const noexcept;

    //==============================================================================
    /** Starts playing (if a source has been selected).

        If it starts playing, this will send a message to any ChangeListeners
        that are registered with this object.
    */
    z0 start();

    /** Stops playing.

        If it's actually playing, this will send a message to any ChangeListeners
        that are registered with this object.
    */
    z0 stop();

    /** Возвращает true, если it's currently playing. */
    b8 isPlaying() const noexcept     { return playing; }

    //==============================================================================
    /** Changes the gain to apply to the output.
        @param newGain  a factor by which to multiply the outgoing samples,
                        so 1.0 = 0dB, 0.5 = -6dB, 2.0 = 6dB, etc.
    */
    z0 setGain (f32 newGain) noexcept;

    /** Returns the current gain setting.
        @see setGain
    */
    f32 getGain() const noexcept      { return gain; }

    //==============================================================================
    /** Implementation of the AudioSource method. */
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate) override;

    /** Implementation of the AudioSource method. */
    z0 releaseResources() override;

    /** Implementation of the AudioSource method. */
    z0 getNextAudioBlock (const AudioSourceChannelInfo&) override;

    //==============================================================================
    /** Implements the PositionableAudioSource method. */
    z0 setNextReadPosition (z64 newPosition) override;

    /** Implements the PositionableAudioSource method. */
    z64 getNextReadPosition() const override;

    /** Implements the PositionableAudioSource method. */
    z64 getTotalLength() const override;

    /** Implements the PositionableAudioSource method. */
    b8 isLooping() const override;

private:
    //==============================================================================
    PositionableAudioSource* source = nullptr;
    ResamplingAudioSource* resamplerSource = nullptr;
    BufferingAudioSource* bufferingSource = nullptr;
    PositionableAudioSource* positionableSource = nullptr;
    AudioSource* masterSource = nullptr;

    CriticalSection callbackLock;
    f32 gain = 1.0f, lastGain = 1.0f;
    std::atomic<b8> playing { false }, stopped { true };
    f64 sampleRate = 44100.0, sourceSampleRate = 0;
    i32 blockSize = 128, readAheadBufferSize = 0;
    b8 isPrepared = false;

    z0 releaseMasterResources();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTransportSource)
};

} // namespace drx
