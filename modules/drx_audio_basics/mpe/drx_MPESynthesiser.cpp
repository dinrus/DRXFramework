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

MPESynthesiser::MPESynthesiser()
{
}

MPESynthesiser::MPESynthesiser (MPEInstrument& mpeInstrument)
    : MPESynthesiserBase (mpeInstrument)
{
}

MPESynthesiser::~MPESynthesiser()
{
}

//==============================================================================
z0 MPESynthesiser::startVoice (MPESynthesiserVoice* voice, MPENote noteToStart)
{
    jassert (voice != nullptr);

    voice->currentlyPlayingNote = noteToStart;
    voice->noteOnTime = lastNoteOnCounter++;
    voice->noteStarted();
}

z0 MPESynthesiser::stopVoice (MPESynthesiserVoice* voice, MPENote noteToStop, b8 allowTailOff)
{
    jassert (voice != nullptr);

    voice->currentlyPlayingNote = noteToStop;
    voice->noteStopped (allowTailOff);
}

//==============================================================================
z0 MPESynthesiser::noteAdded (MPENote newNote)
{
    const ScopedLock sl (voicesLock);

    if (auto* voice = findFreeVoice (newNote, shouldStealVoices))
        startVoice (voice, newNote);
}

z0 MPESynthesiser::notePressureChanged (MPENote changedNote)
{
    const ScopedLock sl (voicesLock);

    for (auto* voice : voices)
    {
        if (voice->isCurrentlyPlayingNote (changedNote))
        {
            voice->currentlyPlayingNote = changedNote;
            voice->notePressureChanged();
        }
    }
}

z0 MPESynthesiser::notePitchbendChanged (MPENote changedNote)
{
    const ScopedLock sl (voicesLock);

    for (auto* voice : voices)
    {
        if (voice->isCurrentlyPlayingNote (changedNote))
        {
            voice->currentlyPlayingNote = changedNote;
            voice->notePitchbendChanged();
        }
    }
}

z0 MPESynthesiser::noteTimbreChanged (MPENote changedNote)
{
    const ScopedLock sl (voicesLock);

    for (auto* voice : voices)
    {
        if (voice->isCurrentlyPlayingNote (changedNote))
        {
            voice->currentlyPlayingNote = changedNote;
            voice->noteTimbreChanged();
        }
    }
}

z0 MPESynthesiser::noteKeyStateChanged (MPENote changedNote)
{
    const ScopedLock sl (voicesLock);

    for (auto* voice : voices)
    {
        if (voice->isCurrentlyPlayingNote (changedNote))
        {
            voice->currentlyPlayingNote = changedNote;
            voice->noteKeyStateChanged();
        }
    }
}

z0 MPESynthesiser::noteReleased (MPENote finishedNote)
{
    const ScopedLock sl (voicesLock);

    for (auto i = voices.size(); --i >= 0;)
    {
        auto* voice = voices.getUnchecked (i);

        if (voice->isCurrentlyPlayingNote (finishedNote))
            stopVoice (voice, finishedNote, true);
    }
}

z0 MPESynthesiser::setCurrentPlaybackSampleRate (const f64 newRate)
{
    MPESynthesiserBase::setCurrentPlaybackSampleRate (newRate);

    const ScopedLock sl (voicesLock);

    turnOffAllVoices (false);

    for (auto i = voices.size(); --i >= 0;)
        voices.getUnchecked (i)->setCurrentSampleRate (newRate);
}

z0 MPESynthesiser::handleMidiEvent (const MidiMessage& m)
{
    if (m.isController())
        handleController (m.getChannel(), m.getControllerNumber(), m.getControllerValue());
    else if (m.isProgramChange())
        handleProgramChange (m.getChannel(), m.getProgramChangeNumber());

    MPESynthesiserBase::handleMidiEvent (m);
}

MPESynthesiserVoice* MPESynthesiser::findFreeVoice (MPENote noteToFindVoiceFor, b8 stealIfNoneAvailable) const
{
    const ScopedLock sl (voicesLock);

    for (auto* voice : voices)
    {
        if (! voice->isActive())
            return voice;
    }

    if (stealIfNoneAvailable)
        return findVoiceToSteal (noteToFindVoiceFor);

    return nullptr;
}

MPESynthesiserVoice* MPESynthesiser::findVoiceToSteal (MPENote noteToStealVoiceFor) const
{
    // This voice-stealing algorithm applies the following heuristics:
    // - Re-use the oldest notes first
    // - Protect the lowest & topmost notes, even if sustained, but not if they've been released.


    // apparently you are trying to render audio without having any voices...
    jassert (voices.size() > 0);

    // These are the voices we want to protect (ie: only steal if unavoidable)
    MPESynthesiserVoice* low = nullptr; // Lowest sounding note, might be sustained, but NOT in release phase
    MPESynthesiserVoice* top = nullptr; // Highest sounding note, might be sustained, but NOT in release phase

    // All major OSes use f64-locking so this will be lock- and wait-free as i64 as stealLock is not
    // contended. This is always the case if you do not call findVoiceToSteal on multiple threads at
    // the same time.
    const ScopedLock sl (stealLock);

    // this is a list of voices we can steal, sorted by how i64 they've been running
    usableVoicesToStealArray.clear();

    for (auto* voice : voices)
    {
        jassert (voice->isActive()); // We wouldn't be here otherwise

        usableVoicesToStealArray.add (voice);

        // NB: Using a functor rather than a lambda here due to scare-stories about
        // compilers generating code containing heap allocations..
        struct Sorter
        {
            b8 operator() (const MPESynthesiserVoice* a, const MPESynthesiserVoice* b) const noexcept { return a->noteOnTime < b->noteOnTime; }
        };

        std::sort (usableVoicesToStealArray.begin(), usableVoicesToStealArray.end(), Sorter());

        if (! voice->isPlayingButReleased()) // Don't protect released notes
        {
            auto noteNumber = voice->getCurrentlyPlayingNote().initialNote;

            if (low == nullptr || noteNumber < low->getCurrentlyPlayingNote().initialNote)
                low = voice;

            if (top == nullptr || noteNumber > top->getCurrentlyPlayingNote().initialNote)
                top = voice;
        }
    }

    // Eliminate pathological cases (ie: only 1 note playing): we always give precedence to the lowest note(s)
    if (top == low)
        top = nullptr;

    // If we want to re-use the voice to trigger a new note,
    // then The oldest note that's playing the same note number is ideal.
    if (noteToStealVoiceFor.isValid())
        for (auto* voice : usableVoicesToStealArray)
            if (voice->getCurrentlyPlayingNote().initialNote == noteToStealVoiceFor.initialNote)
                return voice;

    // Oldest voice that has been released (no finger on it and not held by sustain pedal)
    for (auto* voice : usableVoicesToStealArray)
        if (voice != low && voice != top && voice->isPlayingButReleased())
            return voice;

    // Oldest voice that doesn't have a finger on it:
    for (auto* voice : usableVoicesToStealArray)
        if (voice != low && voice != top
             && voice->getCurrentlyPlayingNote().keyState != MPENote::keyDown
             && voice->getCurrentlyPlayingNote().keyState != MPENote::keyDownAndSustained)
            return voice;

    // Oldest voice that isn't protected
    for (auto* voice : usableVoicesToStealArray)
        if (voice != low && voice != top)
            return voice;

    // We've only got "protected" voices now: lowest note takes priority
    jassert (low != nullptr);

    // Duophonic synth: give priority to the bass note:
    if (top != nullptr)
        return top;

    return low;
}

//==============================================================================
z0 MPESynthesiser::addVoice (MPESynthesiserVoice* const newVoice)
{
    {
        const ScopedLock sl (voicesLock);
        newVoice->setCurrentSampleRate (getSampleRate());
        voices.add (newVoice);
    }

    {
        const ScopedLock sl (stealLock);
        usableVoicesToStealArray.ensureStorageAllocated (voices.size() + 1);
    }
}

z0 MPESynthesiser::clearVoices()
{
    const ScopedLock sl (voicesLock);
    voices.clear();
}

MPESynthesiserVoice* MPESynthesiser::getVoice (i32k index) const
{
    const ScopedLock sl (voicesLock);
    return voices [index];
}

z0 MPESynthesiser::removeVoice (i32k index)
{
    const ScopedLock sl (voicesLock);
    voices.remove (index);
}

z0 MPESynthesiser::reduceNumVoices (i32k newNumVoices)
{
    // we can't possibly get to a negative number of voices...
    jassert (newNumVoices >= 0);

    const ScopedLock sl (voicesLock);

    while (voices.size() > newNumVoices)
    {
        if (auto* voice = findFreeVoice ({}, true))
            voices.removeObject (voice);
        else
            voices.remove (0); // if there's no voice to steal, kill the oldest voice
    }
}

z0 MPESynthesiser::turnOffAllVoices (b8 allowTailOff)
{
    {
        const ScopedLock sl (voicesLock);

        // first turn off all voices (it's more efficient to do this immediately
        // rather than to go through the MPEInstrument for this).
        for (auto* voice : voices)
        {
            voice->currentlyPlayingNote.noteOffVelocity = MPEValue::from7BitInt (64); // some reasonable number
            voice->currentlyPlayingNote.keyState = MPENote::off;

            voice->noteStopped (allowTailOff);
        }
    }

    // finally make sure the MPE Instrument also doesn't have any notes anymore.
    instrument.releaseAllNotes();
}

//==============================================================================
z0 MPESynthesiser::renderNextSubBlock (AudioBuffer<f32>& buffer, i32 startSample, i32 numSamples)
{
    const ScopedLock sl (voicesLock);

    for (auto* voice : voices)
    {
        if (voice->isActive())
            voice->renderNextBlock (buffer, startSample, numSamples);
    }
}

z0 MPESynthesiser::renderNextSubBlock (AudioBuffer<f64>& buffer, i32 startSample, i32 numSamples)
{
    const ScopedLock sl (voicesLock);

    for (auto* voice : voices)
    {
        if (voice->isActive())
            voice->renderNextBlock (buffer, startSample, numSamples);
    }
}

} // namespace drx
