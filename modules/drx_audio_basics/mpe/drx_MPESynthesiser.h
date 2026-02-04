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
    Base class for an MPE-compatible musical device that can play sounds.

    This class extends MPESynthesiserBase by adding the concept of voices,
    each of which can play a sound triggered by a MPENote that can be modulated
    by MPE dimensions like pressure, pitchbend, and timbre, while the note is
    sounding.

    To create a synthesiser, you'll need to create a subclass of MPESynthesiserVoice
    which can play back one of these sounds at a time.

    Then you can use the addVoice() methods to give the synthesiser a set of voices
    it can use to play notes. If you only give it one voice it will be monophonic -
    the more voices it has, the more polyphony it'll have available.

    Then repeatedly call the renderNextBlock() method to produce the audio (inherited
    from MPESynthesiserBase). The voices will be started, stopped, and modulated
    automatically, based on the MPE/MIDI messages that the synthesiser receives.

    Before rendering, be sure to call the setCurrentPlaybackSampleRate() to tell it
    what the target playback rate is. This value is passed on to the voices so that
    they can pitch their output correctly.

    @see MPESynthesiserBase, MPESynthesiserVoice, MPENote, MPEInstrument

    @tags{Audio}
*/
class DRX_API  MPESynthesiser   : public MPESynthesiserBase
{
public:
    //==============================================================================
    /** Constructor.
        You'll need to add some voices before it'll make any sound.

        @see addVoice
    */
    MPESynthesiser();

    /** Constructor to pass to the synthesiser a custom MPEInstrument object
        to handle the MPE note state, MIDI channel assignment etc.
        (in case you need custom logic for this that goes beyond MIDI and MPE).

        @see MPESynthesiserBase, MPEInstrument
    */
    MPESynthesiser (MPEInstrument& instrumentToUse);

    /** Destructor. */
    ~MPESynthesiser() override;

    //==============================================================================
    /** Deletes all voices. */
    z0 clearVoices();

    /** Returns the number of voices that have been added. */
    i32 getNumVoices() const noexcept                               { return voices.size(); }

    /** Returns one of the voices that have been added. */
    MPESynthesiserVoice* getVoice (i32 index) const;

    /** Adds a new voice to the synth.

        All the voices should be the same class of object and are treated equally.

        The object passed in will be managed by the synthesiser, which will delete
        it later on when no longer needed. The caller should not retain a pointer to the
        voice.
    */
    z0 addVoice (MPESynthesiserVoice* newVoice);

    /** Deletes one of the voices. */
    z0 removeVoice (i32 index);

    /** Reduces the number of voices to newNumVoices.

        This will repeatedly call findVoiceToSteal() and remove that voice, until
        the total number of voices equals newNumVoices. If newNumVoices is greater than
        or equal to the current number of voices, this method does nothing.
    */
    z0 reduceNumVoices (i32 newNumVoices);

    /** Release all MPE notes and turn off all voices.

        If allowTailOff is true, the voices will be allowed to fade out the notes gracefully
        (if they can do). If this is false, the notes will all be cut off immediately.

        This method is meant to be called by the user, for example to implement
        a MIDI panic button in a synth.
    */
    virtual z0 turnOffAllVoices (b8 allowTailOff);

    //==============================================================================
    /** If set to true, then the synth will try to take over an existing voice if
        it runs out and needs to play another note.

        The value of this boolean is passed into findFreeVoice(), so the result will
        depend on the implementation of this method.
    */
    z0 setVoiceStealingEnabled (b8 shouldSteal) noexcept    { shouldStealVoices = shouldSteal; }

    /** Возвращает true, если note-stealing is enabled. */
    b8 isVoiceStealingEnabled() const noexcept                { return shouldStealVoices; }

    //==============================================================================
    /** Tells the synthesiser what the sample rate is for the audio it's being used to render.

        This overrides the implementation in MPESynthesiserBase, to additionally
        propagate the new value to the voices so that they can use it to render the correct
        pitches.
    */
    z0 setCurrentPlaybackSampleRate (f64 newRate) override;

    //==============================================================================
    /** Handle incoming MIDI events.

        This method will be called automatically according to the MIDI data passed
        into renderNextBlock(), but you can also call it yourself to manually
        inject MIDI events.

        This implementation forwards program change messages and non-MPE-related
        controller messages to handleProgramChange and handleController, respectively,
        and then simply calls through to MPESynthesiserBase::handleMidiEvent to deal
        with MPE-related MIDI messages used for MPE notes, zones etc.

        This method can be overridden further if you need to do custom MIDI
        handling on top of what is provided here.
    */
    z0 handleMidiEvent (const MidiMessage&) override;

    /** Callback for MIDI controller messages. The default implementation
        provided here does nothing; override this method if you need custom
        MIDI controller handling on top of MPE.

        This method will be called automatically according to the midi data passed into
        renderNextBlock().
    */
    virtual z0 handleController (i32 /*midiChannel*/,
                                   i32 /*controllerNumber*/,
                                   i32 /*controllerValue*/) {}

    /** Callback for MIDI program change messages. The default implementation
        provided here does nothing; override this method if you need to handle
        those messages.

        This method will be called automatically according to the midi data passed into
        renderNextBlock().
    */
    virtual z0 handleProgramChange (i32 /*midiChannel*/,
                                      i32 /*programNumber*/) {}

protected:
    //==============================================================================
    /** Attempts to start playing a new note.

        The default method here will find a free voice that is appropriate for
        playing the given MPENote, and use that voice to start playing the sound.
        If isNoteStealingEnabled returns true (set this by calling setNoteStealingEnabled),
        the synthesiser will use the voice stealing algorithm to find a free voice for
        the note (if no voices are free otherwise).

        This method will be called automatically according to the midi data passed into
        renderNextBlock(). Do not call it yourself, otherwise the internal MPE note state
        will become inconsistent.
    */
    z0 noteAdded (MPENote newNote) override;

    /** Stops playing a note.

        This will be called whenever an  MPE note is released (either by a note-off message,
        or by a sustain/sostenuto pedal release for a note that already received a note-off),
        and should therefore stop playing.

        This will find any voice that is currently playing finishedNote,
        turn its currently playing note off, and call its noteStopped callback.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(). Do not call it yourself, otherwise the internal MPE note state
        will become inconsistent.
    */
    z0 noteReleased (MPENote finishedNote) override;

    /** Will find any voice that is currently playing changedNote, update its
        currently playing note, and call its notePressureChanged method.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(). Do not call it yourself.
    */
    z0 notePressureChanged (MPENote changedNote) override;

    /** Will find any voice that is currently playing changedNote, update its
        currently playing note, and call its notePitchbendChanged method.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(). Do not call it yourself.
    */
    z0 notePitchbendChanged (MPENote changedNote) override;

    /** Will find any voice that is currently playing changedNote, update its
        currently playing note, and call its noteTimbreChanged method.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(). Do not call it yourself.
    */
    z0 noteTimbreChanged (MPENote changedNote) override;

    /** Will find any voice that is currently playing changedNote, update its
        currently playing note, and call its noteKeyStateChanged method.

        This method will be called automatically according to the midi data passed into
        renderNextBlock(). Do not call it yourself.
     */
    z0 noteKeyStateChanged (MPENote changedNote) override;

    //==============================================================================
    /** This will simply call renderNextBlock for each currently active
        voice and fill the buffer with the sum.
        Override this method if you need to do more work to render your audio.
    */
    z0 renderNextSubBlock (AudioBuffer<f32>& outputAudio,
                             i32 startSample,
                             i32 numSamples) override;

    /** This will simply call renderNextBlock for each currently active
        voice and fill the buffer with the sum. (f64-precision version)
        Override this method if you need to do more work to render your audio.
    */
    z0 renderNextSubBlock (AudioBuffer<f64>& outputAudio,
                             i32 startSample,
                             i32 numSamples) override;

    //==============================================================================
    /** Searches through the voices to find one that's not currently playing, and
        which can play the given MPE note.

        If all voices are active and stealIfNoneAvailable is false, this returns
        a nullptr. If all voices are active and stealIfNoneAvailable is true,
        this will call findVoiceToSteal() to find a voice.

        If you need to find a free voice for something else than playing a note
        (e.g. for deleting it), you can pass an invalid (default-constructed) MPENote.
    */
    virtual MPESynthesiserVoice* findFreeVoice (MPENote noteToFindVoiceFor,
                                                b8 stealIfNoneAvailable) const;

    /** Chooses a voice that is most suitable for being re-used to play a new
        note, or for being deleted by reduceNumVoices.

        The default method will attempt to find the oldest voice that isn't the
        bottom or top note being played. If that's not suitable for your synth,
        you can override this method and do something more cunning instead.

        If you pass a valid MPENote for the optional argument, then the note number
        of that note will be taken into account for finding the ideal voice to steal.
        If you pass an invalid (default-constructed) MPENote instead, this part of
        the algorithm will be ignored.
    */
    virtual MPESynthesiserVoice* findVoiceToSteal (MPENote noteToStealVoiceFor = MPENote()) const;

    /** Starts a specified voice and tells it to play a particular MPENote.
        You should never need to call this, it's called internally by
        MPESynthesiserBase::instrument via the noteStarted callback,
        but is protected in case it's useful for some custom subclasses.
    */
    z0 startVoice (MPESynthesiserVoice* voice, MPENote noteToStart);

    /** Stops a given voice and tells it to stop playing a particular MPENote
        (which should be the same note it is actually playing).
        You should never need to call this, it's called internally by
        MPESynthesiserBase::instrument via the noteReleased callback,
        but is protected in case it's useful for some custom subclasses.
    */
    z0 stopVoice (MPESynthesiserVoice* voice, MPENote noteToStop, b8 allowTailOff);

    //==============================================================================
    OwnedArray<MPESynthesiserVoice> voices;
    CriticalSection voicesLock;

private:
    //==============================================================================
    std::atomic<b8> shouldStealVoices { false };
    u32 lastNoteOnCounter = 0;
    mutable CriticalSection stealLock;
    mutable Array<MPESynthesiserVoice*> usableVoicesToStealArray;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPESynthesiser)
};

} // namespace drx
