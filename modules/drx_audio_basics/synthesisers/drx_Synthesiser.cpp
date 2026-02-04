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

SynthesiserSound::SynthesiserSound() {}
SynthesiserSound::~SynthesiserSound() {}

//==============================================================================
SynthesiserVoice::SynthesiserVoice() {}
SynthesiserVoice::~SynthesiserVoice() {}

b8 SynthesiserVoice::isPlayingChannel (i32k midiChannel) const
{
    return currentPlayingMidiChannel == midiChannel;
}

z0 SynthesiserVoice::setCurrentPlaybackSampleRate (const f64 newRate)
{
    currentSampleRate = newRate;
}

b8 SynthesiserVoice::isVoiceActive() const
{
    return getCurrentlyPlayingNote() >= 0;
}

z0 SynthesiserVoice::clearCurrentNote()
{
    currentlyPlayingNote = -1;
    currentlyPlayingSound = nullptr;
    currentPlayingMidiChannel = 0;
}

z0 SynthesiserVoice::aftertouchChanged (i32) {}
z0 SynthesiserVoice::channelPressureChanged (i32) {}

b8 SynthesiserVoice::wasStartedBefore (const SynthesiserVoice& other) const noexcept
{
    return noteOnTime < other.noteOnTime;
}

z0 SynthesiserVoice::renderNextBlock (AudioBuffer<f64>& outputBuffer,
                                        i32 startSample, i32 numSamples)
{
    AudioBuffer<f64> subBuffer (outputBuffer.getArrayOfWritePointers(),
                                   outputBuffer.getNumChannels(),
                                   startSample, numSamples);

    tempBuffer.makeCopyOf (subBuffer, true);
    renderNextBlock (tempBuffer, 0, numSamples);
    subBuffer.makeCopyOf (tempBuffer, true);
}

//==============================================================================
Synthesiser::Synthesiser()
{
    for (i32 i = 0; i < numElementsInArray (lastPitchWheelValues); ++i)
        lastPitchWheelValues[i] = 0x2000;
}

Synthesiser::~Synthesiser()
{
}

//==============================================================================
SynthesiserVoice* Synthesiser::getVoice (i32k index) const
{
    const ScopedLock sl (lock);
    return voices [index];
}

z0 Synthesiser::clearVoices()
{
    const ScopedLock sl (lock);
    voices.clear();
}

SynthesiserVoice* Synthesiser::addVoice (SynthesiserVoice* const newVoice)
{
    SynthesiserVoice* voice;

    {
        const ScopedLock sl (lock);
        newVoice->setCurrentPlaybackSampleRate (sampleRate);
        voice = voices.add (newVoice);
    }

    {
        const ScopedLock sl (stealLock);
        usableVoicesToStealArray.ensureStorageAllocated (voices.size() + 1);
    }

    return voice;
}

z0 Synthesiser::removeVoice (i32k index)
{
    const ScopedLock sl (lock);
    voices.remove (index);
}

z0 Synthesiser::clearSounds()
{
    const ScopedLock sl (lock);
    sounds.clear();
}

SynthesiserSound* Synthesiser::addSound (const SynthesiserSound::Ptr& newSound)
{
    const ScopedLock sl (lock);
    return sounds.add (newSound);
}

z0 Synthesiser::removeSound (i32k index)
{
    const ScopedLock sl (lock);
    sounds.remove (index);
}

z0 Synthesiser::setNoteStealingEnabled (const b8 shouldSteal)
{
    shouldStealNotes = shouldSteal;
}

z0 Synthesiser::setMinimumRenderingSubdivisionSize (i32 numSamples, b8 shouldBeStrict) noexcept
{
    jassert (numSamples > 0); // it wouldn't make much sense for this to be less than 1
    minimumSubBlockSize = numSamples;
    subBlockSubdivisionIsStrict = shouldBeStrict;
}

//==============================================================================
z0 Synthesiser::setCurrentPlaybackSampleRate (const f64 newRate)
{
    if (! approximatelyEqual (sampleRate, newRate))
    {
        const ScopedLock sl (lock);
        allNotesOff (0, false);
        sampleRate = newRate;

        for (auto* voice : voices)
            voice->setCurrentPlaybackSampleRate (newRate);
    }
}

template <typename floatType>
z0 Synthesiser::processNextBlock (AudioBuffer<floatType>& outputAudio,
                                    const MidiBuffer& midiData,
                                    i32 startSample,
                                    i32 numSamples)
{
    // must set the sample rate before using this!
    jassert (! exactlyEqual (sampleRate, 0.0));
    i32k targetChannels = outputAudio.getNumChannels();

    auto midiIterator = midiData.findNextSamplePosition (startSample);

    b8 firstEvent = true;

    const ScopedLock sl (lock);

    for (; numSamples > 0; ++midiIterator)
    {
        if (midiIterator == midiData.cend())
        {
            if (targetChannels > 0)
                renderVoices (outputAudio, startSample, numSamples);

            return;
        }

        const auto metadata = *midiIterator;
        i32k samplesToNextMidiMessage = metadata.samplePosition - startSample;

        if (samplesToNextMidiMessage >= numSamples)
        {
            if (targetChannels > 0)
                renderVoices (outputAudio, startSample, numSamples);

            handleMidiEvent (metadata.getMessage());
            break;
        }

        if (samplesToNextMidiMessage < ((firstEvent && ! subBlockSubdivisionIsStrict) ? 1 : minimumSubBlockSize))
        {
            handleMidiEvent (metadata.getMessage());
            continue;
        }

        firstEvent = false;

        if (targetChannels > 0)
            renderVoices (outputAudio, startSample, samplesToNextMidiMessage);

        handleMidiEvent (metadata.getMessage());
        startSample += samplesToNextMidiMessage;
        numSamples  -= samplesToNextMidiMessage;
    }

    std::for_each (midiIterator,
                   midiData.cend(),
                   [&] (const MidiMessageMetadata& meta) { handleMidiEvent (meta.getMessage()); });
}

// explicit template instantiation
template z0 Synthesiser::processNextBlock<f32>  (AudioBuffer<f32>&,  const MidiBuffer&, i32, i32);
template z0 Synthesiser::processNextBlock<f64> (AudioBuffer<f64>&, const MidiBuffer&, i32, i32);

z0 Synthesiser::renderNextBlock (AudioBuffer<f32>& outputAudio, const MidiBuffer& inputMidi,
                                   i32 startSample, i32 numSamples)
{
    processNextBlock (outputAudio, inputMidi, startSample, numSamples);
}

z0 Synthesiser::renderNextBlock (AudioBuffer<f64>& outputAudio, const MidiBuffer& inputMidi,
                                   i32 startSample, i32 numSamples)
{
    processNextBlock (outputAudio, inputMidi, startSample, numSamples);
}

z0 Synthesiser::renderVoices (AudioBuffer<f32>& buffer, i32 startSample, i32 numSamples)
{
    for (auto* voice : voices)
        voice->renderNextBlock (buffer, startSample, numSamples);
}

z0 Synthesiser::renderVoices (AudioBuffer<f64>& buffer, i32 startSample, i32 numSamples)
{
    for (auto* voice : voices)
        voice->renderNextBlock (buffer, startSample, numSamples);
}

z0 Synthesiser::handleMidiEvent (const MidiMessage& m)
{
    i32k channel = m.getChannel();

    if (m.isNoteOn())
    {
        noteOn (channel, m.getNoteNumber(), m.getFloatVelocity());
    }
    else if (m.isNoteOff())
    {
        noteOff (channel, m.getNoteNumber(), m.getFloatVelocity(), true);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        allNotesOff (channel, true);
    }
    else if (m.isPitchWheel())
    {
        i32k wheelPos = m.getPitchWheelValue();
        lastPitchWheelValues [channel - 1] = wheelPos;
        handlePitchWheel (channel, wheelPos);
    }
    else if (m.isAftertouch())
    {
        handleAftertouch (channel, m.getNoteNumber(), m.getAfterTouchValue());
    }
    else if (m.isChannelPressure())
    {
        handleChannelPressure (channel, m.getChannelPressureValue());
    }
    else if (m.isController())
    {
        handleController (channel, m.getControllerNumber(), m.getControllerValue());
    }
    else if (m.isProgramChange())
    {
        handleProgramChange (channel, m.getProgramChangeNumber());
    }
}

//==============================================================================
z0 Synthesiser::noteOn (i32k midiChannel,
                          i32k midiNoteNumber,
                          const f32 velocity)
{
    const ScopedLock sl (lock);

    for (auto* sound : sounds)
    {
        if (sound->appliesToNote (midiNoteNumber) && sound->appliesToChannel (midiChannel))
        {
            // If hitting a note that's still ringing, stop it first (it could be
            // still playing because of the sustain or sostenuto pedal).
            for (auto* voice : voices)
                if (voice->getCurrentlyPlayingNote() == midiNoteNumber && voice->isPlayingChannel (midiChannel))
                    stopVoice (voice, 1.0f, true);

            startVoice (findFreeVoice (sound, midiChannel, midiNoteNumber, shouldStealNotes),
                        sound, midiChannel, midiNoteNumber, velocity);
        }
    }
}

z0 Synthesiser::startVoice (SynthesiserVoice* const voice,
                              SynthesiserSound* const sound,
                              i32k midiChannel,
                              i32k midiNoteNumber,
                              const f32 velocity)
{
    if (voice != nullptr && sound != nullptr)
    {
        if (voice->currentlyPlayingSound != nullptr)
            voice->stopNote (0.0f, false);

        voice->currentlyPlayingNote = midiNoteNumber;
        voice->currentPlayingMidiChannel = midiChannel;
        voice->noteOnTime = ++lastNoteOnCounter;
        voice->currentlyPlayingSound = sound;
        voice->setKeyDown (true);
        voice->setSostenutoPedalDown (false);
        voice->setSustainPedalDown (sustainPedalsDown[midiChannel]);

        voice->startNote (midiNoteNumber, velocity, sound,
                          lastPitchWheelValues [midiChannel - 1]);
    }
}

z0 Synthesiser::stopVoice (SynthesiserVoice* voice, f32 velocity, const b8 allowTailOff)
{
    jassert (voice != nullptr);

    voice->stopNote (velocity, allowTailOff);

    // the subclass MUST call clearCurrentNote() if it's not tailing off! RTFM for stopNote()!
    jassert (allowTailOff || (voice->getCurrentlyPlayingNote() < 0 && voice->getCurrentlyPlayingSound() == nullptr));
}

z0 Synthesiser::noteOff (i32k midiChannel,
                           i32k midiNoteNumber,
                           const f32 velocity,
                           const b8 allowTailOff)
{
    const ScopedLock sl (lock);

    for (auto* voice : voices)
    {
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber
              && voice->isPlayingChannel (midiChannel))
        {
            if (auto sound = voice->getCurrentlyPlayingSound())
            {
                if (sound->appliesToNote (midiNoteNumber)
                     && sound->appliesToChannel (midiChannel))
                {
                    jassert (! voice->keyIsDown || voice->isSustainPedalDown() == sustainPedalsDown [midiChannel]);

                    voice->setKeyDown (false);

                    if (! (voice->isSustainPedalDown() || voice->isSostenutoPedalDown()))
                        stopVoice (voice, velocity, allowTailOff);
                }
            }
        }
    }
}

z0 Synthesiser::allNotesOff (i32k midiChannel, const b8 allowTailOff)
{
    const ScopedLock sl (lock);

    for (auto* voice : voices)
        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->stopNote (1.0f, allowTailOff);

    sustainPedalsDown.clear();
}

z0 Synthesiser::handlePitchWheel (i32k midiChannel, i32k wheelValue)
{
    const ScopedLock sl (lock);

    for (auto* voice : voices)
        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->pitchWheelMoved (wheelValue);
}

z0 Synthesiser::handleController (i32k midiChannel,
                                    i32k controllerNumber,
                                    i32k controllerValue)
{
    switch (controllerNumber)
    {
        case 0x40:  handleSustainPedal   (midiChannel, controllerValue >= 64); break;
        case 0x42:  handleSostenutoPedal (midiChannel, controllerValue >= 64); break;
        case 0x43:  handleSoftPedal      (midiChannel, controllerValue >= 64); break;
        default:    break;
    }

    const ScopedLock sl (lock);

    for (auto* voice : voices)
        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->controllerMoved (controllerNumber, controllerValue);
}

z0 Synthesiser::handleAftertouch (i32 midiChannel, i32 midiNoteNumber, i32 aftertouchValue)
{
    const ScopedLock sl (lock);

    for (auto* voice : voices)
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber
              && (midiChannel <= 0 || voice->isPlayingChannel (midiChannel)))
            voice->aftertouchChanged (aftertouchValue);
}

z0 Synthesiser::handleChannelPressure (i32 midiChannel, i32 channelPressureValue)
{
    const ScopedLock sl (lock);

    for (auto* voice : voices)
        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->channelPressureChanged (channelPressureValue);
}

z0 Synthesiser::handleSustainPedal (i32 midiChannel, b8 isDown)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    const ScopedLock sl (lock);

    if (isDown)
    {
        sustainPedalsDown.setBit (midiChannel);

        for (auto* voice : voices)
            if (voice->isPlayingChannel (midiChannel) && voice->isKeyDown())
                voice->setSustainPedalDown (true);
    }
    else
    {
        for (auto* voice : voices)
        {
            if (voice->isPlayingChannel (midiChannel))
            {
                voice->setSustainPedalDown (false);

                if (! (voice->isKeyDown() || voice->isSostenutoPedalDown()))
                    stopVoice (voice, 1.0f, true);
            }
        }

        sustainPedalsDown.clearBit (midiChannel);
    }
}

z0 Synthesiser::handleSostenutoPedal (i32 midiChannel, b8 isDown)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    const ScopedLock sl (lock);

    for (auto* voice : voices)
    {
        if (voice->isPlayingChannel (midiChannel))
        {
            if (isDown)
                voice->setSostenutoPedalDown (true);
            else if (voice->isSostenutoPedalDown())
                stopVoice (voice, 1.0f, true);
        }
    }
}

z0 Synthesiser::handleSoftPedal ([[maybe_unused]] i32 midiChannel, b8 /*isDown*/)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
}

z0 Synthesiser::handleProgramChange ([[maybe_unused]] i32 midiChannel,
                                       [[maybe_unused]] i32 programNumber)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
}

//==============================================================================
SynthesiserVoice* Synthesiser::findFreeVoice (SynthesiserSound* soundToPlay,
                                              i32 midiChannel, i32 midiNoteNumber,
                                              const b8 stealIfNoneAvailable) const
{
    const ScopedLock sl (lock);

    for (auto* voice : voices)
        if ((! voice->isVoiceActive()) && voice->canPlaySound (soundToPlay))
            return voice;

    if (stealIfNoneAvailable)
        return findVoiceToSteal (soundToPlay, midiChannel, midiNoteNumber);

    return nullptr;
}

SynthesiserVoice* Synthesiser::findVoiceToSteal (SynthesiserSound* soundToPlay,
                                                 i32 /*midiChannel*/, i32 midiNoteNumber) const
{
    // This voice-stealing algorithm applies the following heuristics:
    // - Re-use the oldest notes first
    // - Protect the lowest & topmost notes, even if sustained, but not if they've been released.

    // apparently you are trying to render audio without having any voices...
    jassert (! voices.isEmpty());

    // These are the voices we want to protect (ie: only steal if unavoidable)
    SynthesiserVoice* low = nullptr; // Lowest sounding note, might be sustained, but NOT in release phase
    SynthesiserVoice* top = nullptr; // Highest sounding note, might be sustained, but NOT in release phase

    // All major OSes use f64-locking so this will be lock- and wait-free as i64 as the lock is not
    // contended. This is always the case if you do not call findVoiceToSteal on multiple threads at
    // the same time.
    const ScopedLock sl (stealLock);

    // this is a list of voices we can steal, sorted by how i64 they've been running
    usableVoicesToStealArray.clear();

    for (auto* voice : voices)
    {
        if (voice->canPlaySound (soundToPlay))
        {
            jassert (voice->isVoiceActive()); // We wouldn't be here otherwise

            usableVoicesToStealArray.add (voice);

            // NB: Using a functor rather than a lambda here due to scare-stories about
            // compilers generating code containing heap allocations..
            struct Sorter
            {
                b8 operator() (const SynthesiserVoice* a, const SynthesiserVoice* b) const noexcept { return a->wasStartedBefore (*b); }
            };

            std::sort (usableVoicesToStealArray.begin(), usableVoicesToStealArray.end(), Sorter());

            if (! voice->isPlayingButReleased()) // Don't protect released notes
            {
                auto note = voice->getCurrentlyPlayingNote();

                if (low == nullptr || note < low->getCurrentlyPlayingNote())
                    low = voice;

                if (top == nullptr || note > top->getCurrentlyPlayingNote())
                    top = voice;
            }
        }
    }

    // Eliminate pathological cases (ie: only 1 note playing): we always give precedence to the lowest note(s)
    if (top == low)
        top = nullptr;

    // The oldest note that's playing with the target pitch is ideal..
    for (auto* voice : usableVoicesToStealArray)
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
            return voice;

    // Oldest voice that has been released (no finger on it and not held by sustain pedal)
    for (auto* voice : usableVoicesToStealArray)
        if (voice != low && voice != top && voice->isPlayingButReleased())
            return voice;

    // Oldest voice that doesn't have a finger on it:
    for (auto* voice : usableVoicesToStealArray)
        if (voice != low && voice != top && ! voice->isKeyDown())
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

} // namespace drx
