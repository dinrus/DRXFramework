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
    Collects incoming realtime MIDI messages and turns them into blocks suitable for
    processing by a block-based audio callback.

    The class can also be used as either a MidiKeyboardState::Listener or a MidiInputCallback
    so it can easily use a midi input or keyboard component as its source.

    @see MidiMessage, MidiInput

    @tags{Audio}
*/
class DRX_API  MidiMessageCollector    : public MidiKeyboardState::Listener,
                                          public MidiInputCallback
{
public:
    //==============================================================================
    /** Creates a MidiMessageCollector. */
    MidiMessageCollector();

    /** Destructor. */
    ~MidiMessageCollector() override;

    //==============================================================================
    /** Clears any messages from the queue.

        You need to call this method before starting to use the collector, so that
        it knows the correct sample rate to use.
    */
    z0 reset (f64 sampleRate);

    /** Takes an incoming real-time message and adds it to the queue.

        The message's timestamp is taken, and it will be ready for retrieval as part
        of the block returned by the next call to removeNextBlockOfMessages().

        This method is fully thread-safe when overlapping calls are made with
        removeNextBlockOfMessages().
    */
    z0 addMessageToQueue (const MidiMessage& message);

    /** Removes all the pending messages from the queue as a buffer.

        This will also correct the messages' timestamps to make sure they're in
        the range 0 to numSamples - 1.

        This call should be made regularly by something like an audio processing
        callback, because the time that it happens is used in calculating the
        midi event positions.

        This method is fully thread-safe when overlapping calls are made with
        addMessageToQueue().

        Precondition: numSamples must be greater than 0.
    */
    z0 removeNextBlockOfMessages (MidiBuffer& destBuffer, i32 numSamples);

    /** Preallocates storage for collected messages.

        This can be called before audio processing begins to ensure that there
        is sufficient space for the expected MIDI messages, in order to avoid
        allocations within the audio callback.
    */
    z0 ensureStorageAllocated (size_t bytes);


    //==============================================================================
    /** @internal */
    z0 handleNoteOn (MidiKeyboardState*, i32 midiChannel, i32 midiNoteNumber, f32 velocity) override;
    /** @internal */
    z0 handleNoteOff (MidiKeyboardState*, i32 midiChannel, i32 midiNoteNumber, f32 velocity) override;
    /** @internal */
    z0 handleIncomingMidiMessage (MidiInput*, const MidiMessage&) override;

private:
    //==============================================================================
    f64 lastCallbackTime = 0;
    CriticalSection midiCallbackLock;
    MidiBuffer incomingMessages;
    f64 sampleRate = 44100.0;
   #if DRX_DEBUG
    b8 hasCalledReset = false;
   #endif

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiMessageCollector)
};

} // namespace drx
