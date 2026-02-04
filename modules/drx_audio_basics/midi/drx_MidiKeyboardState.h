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
    Represents a piano keyboard, keeping track of which keys are currently pressed.

    This object can parse a stream of midi events, using them to update its idea
    of which keys are pressed for each individual midi channel.

    When keys go up or down, it can broadcast these events to listener objects.

    It also allows key up/down events to be triggered with its noteOn() and noteOff()
    methods, and midi messages for these events will be merged into the
    midi stream that gets processed by processNextMidiBuffer().

    @tags{Audio}
*/
class DRX_API  MidiKeyboardState
{
public:
    //==============================================================================
    MidiKeyboardState();

    //==============================================================================
    /** Resets the state of the object.

        All internal data for all the channels is reset, but no events are sent as a
        result.

        If you want to release any keys that are currently down, and to send out note-up
        midi messages for this, use the allNotesOff() method instead.
    */
    z0 reset();

    /** Возвращает true, если the given midi key is currently held down for the given midi channel.

        The channel number must be between 1 and 16. If you want to see if any notes are
        on for a range of channels, use the isNoteOnForChannels() method.
    */
    b8 isNoteOn (i32 midiChannel, i32 midiNoteNumber) const noexcept;

    /** Возвращает true, если the given midi key is currently held down on any of a set of midi channels.

        The channel mask has a bit set for each midi channel you want to test for - bit
        0 = midi channel 1, bit 1 = midi channel 2, etc.

        If a note is on for at least one of the specified channels, this returns true.
    */
    b8 isNoteOnForChannels (i32 midiChannelMask, i32 midiNoteNumber) const noexcept;

    /** Turns a specified note on.

        This will cause a suitable midi note-on event to be injected into the midi buffer during the
        next call to processNextMidiBuffer().

        It will also trigger a synchronous callback to the listeners to tell them that the key has
        gone down.
    */
    z0 noteOn (i32 midiChannel, i32 midiNoteNumber, f32 velocity);

    /** Turns a specified note off.

        This will cause a suitable midi note-off event to be injected into the midi buffer during the
        next call to processNextMidiBuffer().

        It will also trigger a synchronous callback to the listeners to tell them that the key has
        gone up.

        But if the note isn't actually down for the given channel, this method will in fact do nothing.
    */
    z0 noteOff (i32 midiChannel, i32 midiNoteNumber, f32 velocity);

    /** This will turn off any currently-down notes for the given midi channel.

        If you pass 0 for the midi channel, it will in fact turn off all notes on all channels.

        Calling this method will make calls to noteOff(), so can trigger synchronous callbacks
        and events being added to the midi stream.
    */
    z0 allNotesOff (i32 midiChannel);

    //==============================================================================
    /** Looks at a key-up/down event and uses it to update the state of this object.

        To process a buffer full of midi messages, use the processNextMidiBuffer() method
        instead.
    */
    z0 processNextMidiEvent (const MidiMessage& message);

    /** Scans a midi stream for up/down events and adds its own events to it.

        This will look for any up/down events and use them to update the internal state,
        synchronously making suitable callbacks to the listeners.

        If injectIndirectEvents is true, then midi events to produce the recent noteOn()
        and noteOff() calls will be added into the buffer.

        Only the section of the buffer whose timestamps are between startSample and
        (startSample + numSamples) will be affected, and any events added will be placed
        between these times.

        If you're going to use this method, you'll need to keep calling it regularly for
        it to work satisfactorily.

        To process a single midi event at a time, use the processNextMidiEvent() method
        instead.
    */
    z0 processNextMidiBuffer (MidiBuffer& buffer,
                                i32 startSample,
                                i32 numSamples,
                                b8 injectIndirectEvents);

    //==============================================================================
    /** Receives events from a MidiKeyboardState object. */
    class DRX_API Listener
    {
    public:
        //==============================================================================
        virtual ~Listener() = default;

        //==============================================================================
        /** Called when one of the MidiKeyboardState's keys is pressed.

            This will be called synchronously when the state is either processing a
            buffer in its MidiKeyboardState::processNextMidiBuffer() method, or
            when a note is being played with its MidiKeyboardState::noteOn() method.

            Note that this callback could happen from an audio callback thread, so be
            careful not to block, and avoid any UI activity in the callback.
        */
        virtual z0 handleNoteOn (MidiKeyboardState* source,
                                   i32 midiChannel, i32 midiNoteNumber, f32 velocity) = 0;

        /** Called when one of the MidiKeyboardState's keys is released.

            This will be called synchronously when the state is either processing a
            buffer in its MidiKeyboardState::processNextMidiBuffer() method, or
            when a note is being played with its MidiKeyboardState::noteOff() method.

            Note that this callback could happen from an audio callback thread, so be
            careful not to block, and avoid any UI activity in the callback.
        */
        virtual z0 handleNoteOff (MidiKeyboardState* source,
                                    i32 midiChannel, i32 midiNoteNumber, f32 velocity) = 0;
    };

    /** Registers a listener for callbacks when keys go up or down.
        @see removeListener
    */
    z0 addListener (Listener* listener);

    /** Deregisters a listener.
        @see addListener
    */
    z0 removeListener (Listener* listener);

private:
    //==============================================================================
    CriticalSection lock;
    std::atomic<u16> noteStates[128];
    MidiBuffer eventsToAdd;
    ListenerList<Listener> listeners;

    z0 noteOnInternal  (i32 midiChannel, i32 midiNoteNumber, f32 velocity);
    z0 noteOffInternal (i32 midiChannel, i32 midiNoteNumber, f32 velocity);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiKeyboardState)
};

using MidiKeyboardStateListener = MidiKeyboardState::Listener;

} // namespace drx
