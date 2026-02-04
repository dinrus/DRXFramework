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
    An AudioSource which takes another source as input, and buffers it using a thread.

    Create this as a wrapper around another thread, and it will read-ahead with
    a background thread to smooth out playback. You can either create one of these
    directly, or use it indirectly using an AudioTransportSource.

    @see PositionableAudioSource, AudioTransportSource

    @tags{Audio}
*/
class DRX_API  BufferingAudioSource  : public PositionableAudioSource,
                                        private TimeSliceClient
{
public:
    //==============================================================================
    /** Creates a BufferingAudioSource.

        @param source                       the input source to read from
        @param backgroundThread             a background thread that will be used for the
                                            background read-ahead. This object must not be deleted
                                            until after any BufferingAudioSources that are using it
                                            have been deleted!
        @param deleteSourceWhenDeleted      if true, then the input source object will
                                            be deleted when this object is deleted
        @param numberOfSamplesToBuffer      the size of buffer to use for reading ahead
        @param numberOfChannels             the number of channels that will be played
        @param prefillBufferOnPrepareToPlay if true, then calling prepareToPlay on this object will
                                            block until the buffer has been filled
    */
    BufferingAudioSource (PositionableAudioSource* source,
                          TimeSliceThread& backgroundThread,
                          b8 deleteSourceWhenDeleted,
                          i32 numberOfSamplesToBuffer,
                          i32 numberOfChannels = 2,
                          b8 prefillBufferOnPrepareToPlay = true);

    /** Destructor.

        The input source may be deleted depending on whether the deleteSourceWhenDeleted
        flag was set in the constructor.
    */
    ~BufferingAudioSource() override;

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
    z64 getTotalLength() const override       { return source->getTotalLength(); }

    /** Implements the PositionableAudioSource method. */
    b8 isLooping() const override             { return source->isLooping(); }

    /** A useful function to block until the next the buffer info can be filled.

        This is useful for offline rendering.
    */
    b8 waitForNextAudioBlockReady (const AudioSourceChannelInfo& info, u32 timeout);

private:
    //==============================================================================
    Range<i32> getValidBufferRange (i32 numSamples) const;
    b8 readNextBufferChunk();
    z0 readBufferSection (z64 start, i32 length, i32 bufferOffset);
    i32 useTimeSlice() override;

    //==============================================================================
    OptionalScopedPointer<PositionableAudioSource> source;
    TimeSliceThread& backgroundThread;
    i32 numberOfSamplesToBuffer, numberOfChannels;
    AudioBuffer<f32> buffer;
    CriticalSection callbackLock, bufferRangeLock;
    WaitableEvent bufferReadyEvent;
    z64 bufferValidStart = 0, bufferValidEnd = 0;
    std::atomic<z64> nextPlayPos { 0 };
    f64 sampleRate = 0;
    b8 wasSourceLooping = false, isPrepared = false;
    const b8 prefillBuffer;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferingAudioSource)
};

} // namespace drx
