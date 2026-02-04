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

MidiKeyboardState::MidiKeyboardState()
{
    zerostruct (noteStates);
}

//==============================================================================
z0 MidiKeyboardState::reset()
{
    const ScopedLock sl (lock);
    zerostruct (noteStates);
    eventsToAdd.clear();
}

b8 MidiKeyboardState::isNoteOn (i32k midiChannel, i32k n) const noexcept
{
    jassert (midiChannel > 0 && midiChannel <= 16);

    return isPositiveAndBelow (n, 128)
            && (noteStates[n] & (1 << (midiChannel - 1))) != 0;
}

b8 MidiKeyboardState::isNoteOnForChannels (i32k midiChannelMask, i32k n) const noexcept
{
    return isPositiveAndBelow (n, 128)
            && (noteStates[n] & midiChannelMask) != 0;
}

z0 MidiKeyboardState::noteOn (i32k midiChannel, i32k midiNoteNumber, const f32 velocity)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    jassert (isPositiveAndBelow (midiNoteNumber, 128));

    const ScopedLock sl (lock);

    if (isPositiveAndBelow (midiNoteNumber, 128))
    {
        i32k timeNow = (i32) Time::getMillisecondCounter();
        eventsToAdd.addEvent (MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity), timeNow);
        eventsToAdd.clear (0, timeNow - 500);

        noteOnInternal (midiChannel, midiNoteNumber, velocity);
    }
}

z0 MidiKeyboardState::noteOnInternal  (i32k midiChannel, i32k midiNoteNumber, const f32 velocity)
{
    if (isPositiveAndBelow (midiNoteNumber, 128))
    {
        noteStates[midiNoteNumber] = static_cast<u16> (noteStates[midiNoteNumber] | (1 << (midiChannel - 1)));
        listeners.call ([&] (Listener& l) { l.handleNoteOn (this, midiChannel, midiNoteNumber, velocity); });
    }
}

z0 MidiKeyboardState::noteOff (i32k midiChannel, i32k midiNoteNumber, const f32 velocity)
{
    const ScopedLock sl (lock);

    if (isNoteOn (midiChannel, midiNoteNumber))
    {
        i32k timeNow = (i32) Time::getMillisecondCounter();
        eventsToAdd.addEvent (MidiMessage::noteOff (midiChannel, midiNoteNumber), timeNow);
        eventsToAdd.clear (0, timeNow - 500);

        noteOffInternal (midiChannel, midiNoteNumber, velocity);
    }
}

z0 MidiKeyboardState::noteOffInternal  (i32k midiChannel, i32k midiNoteNumber, const f32 velocity)
{
    if (isNoteOn (midiChannel, midiNoteNumber))
    {
        noteStates[midiNoteNumber] = static_cast<u16> (noteStates[midiNoteNumber] & ~(1 << (midiChannel - 1)));
        listeners.call ([&] (Listener& l) { l.handleNoteOff (this, midiChannel, midiNoteNumber, velocity); });
    }
}

z0 MidiKeyboardState::allNotesOff (i32k midiChannel)
{
    const ScopedLock sl (lock);

    if (midiChannel <= 0)
    {
        for (i32 i = 1; i <= 16; ++i)
            allNotesOff (i);
    }
    else
    {
        for (i32 i = 0; i < 128; ++i)
            noteOff (midiChannel, i, 0.0f);
    }
}

z0 MidiKeyboardState::processNextMidiEvent (const MidiMessage& message)
{
    if (message.isNoteOn())
    {
        noteOnInternal (message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    }
    else if (message.isNoteOff())
    {
        noteOffInternal (message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    }
    else if (message.isAllNotesOff())
    {
        for (i32 i = 0; i < 128; ++i)
            noteOffInternal (message.getChannel(), i, 0.0f);
    }
}

z0 MidiKeyboardState::processNextMidiBuffer (MidiBuffer& buffer,
                                               i32k startSample,
                                               i32k numSamples,
                                               const b8 injectIndirectEvents)
{
    const ScopedLock sl (lock);

    for (const auto metadata : buffer)
        processNextMidiEvent (metadata.getMessage());

    if (injectIndirectEvents)
    {
        i32k firstEventToAdd = eventsToAdd.getFirstEventTime();
        const f64 scaleFactor = numSamples / (f64) (eventsToAdd.getLastEventTime() + 1 - firstEventToAdd);

        for (const auto metadata : eventsToAdd)
        {
            const auto pos = jlimit (0, numSamples - 1, roundToInt ((metadata.samplePosition - firstEventToAdd) * scaleFactor));
            buffer.addEvent (metadata.getMessage(), startSample + pos);
        }
    }

    eventsToAdd.clear();
}

//==============================================================================
z0 MidiKeyboardState::addListener (Listener* listener)
{
    const ScopedLock sl (lock);
    listeners.add (listener);
}

z0 MidiKeyboardState::removeListener (Listener* listener)
{
    const ScopedLock sl (lock);
    listeners.remove (listener);
}

} // namespace drx
