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
    This class represents an instrument handling MPE.

    It has an MPE zone layout and maintains a state of currently
    active (playing) notes and the values of their dimensions of expression.

    You can trigger and modulate notes:
      - by passing MIDI messages with the method processNextMidiEvent;
      - by directly calling the methods noteOn, noteOff etc.

    The class implements the channel and note management logic specified in
    MPE. If you pass it a message, it will know what notes on what
    channels (if any) should be affected by that message.

    The class has a Listener class that can be used to react to note and
    state changes and trigger some functionality for your application.
    For example, you can use this class to write an MPE visualiser.

    If you want to write a real-time audio synth with MPE functionality,
    you should instead use the classes MPESynthesiserBase, which adds
    the ability to render audio and to manage voices.

    @see MPENote, MPEZoneLayout, MPESynthesiser

    @tags{Audio}
*/
class DRX_API  MPEInstrument
{
public:
    /** Constructor.

        This will construct an MPE instrument with inactive lower and upper zones.

        In order to process incoming MIDI messages call setZoneLayout, use the MPEZoneLayout
        constructor, define the layout via MIDI RPN messages, or set the instrument to legacy mode.
    */
    MPEInstrument() noexcept;

    /** Constructs an MPE instrument with the specified zone layout. */
    MPEInstrument (MPEZoneLayout layout);

    /** Destructor. */
    virtual ~MPEInstrument();

    //==============================================================================
    /** Returns the current zone layout of the instrument.
        This happens by value, to enforce thread-safety and class invariants.

        Note: If the instrument is in legacy mode, the return value of this
        method is unspecified.
    */
    MPEZoneLayout getZoneLayout() const noexcept;

    /** Re-sets the zone layout of the instrument to the one passed in.
        As a side effect, this will discard all currently playing notes,
        and call noteReleased for all of them.

        This will also disable legacy mode in case it was enabled previously.
    */
    z0 setZoneLayout (MPEZoneLayout newLayout);

    /** Возвращает true, если the given MIDI channel (1-16) is a note channel in any
        of the MPEInstrument's MPE zones; false otherwise.

        When in legacy mode, this will return true if the given channel is
        contained in the current legacy mode channel range; false otherwise.
    */
    b8 isMemberChannel (i32 midiChannel) const noexcept;

    /** Возвращает true, если the given MIDI channel (1-16) is a master channel (channel
        1 or 16).

        In legacy mode, this will always return false.
    */
    b8 isMasterChannel (i32 midiChannel) const noexcept;

    /** Возвращает true, если the given MIDI channel (1-16) is used by any of the
        MPEInstrument's MPE zones; false otherwise.

        When in legacy mode, this will return true if the given channel is
        contained in the current legacy mode channel range; false otherwise.
     */
    b8 isUsingChannel (i32 midiChannel) const noexcept;

    //==============================================================================
    /** The MPE note tracking mode. In case there is more than one note playing
        simultaneously on the same MIDI channel, this determines which of these
        notes will be modulated by an incoming MPE message on that channel
        (pressure, pitchbend, or timbre).

        The default is lastNotePlayedOnChannel.
    */
    enum TrackingMode
    {
        lastNotePlayedOnChannel, /**< The most recent note on the channel that is still played (key down and/or sustained). */
        lowestNoteOnChannel,     /**< The lowest note (by initialNote) on the channel with the note key still down. */
        highestNoteOnChannel,    /**< The highest note (by initialNote) on the channel with the note key still down. */
        allNotesOnChannel        /**< All notes on the channel (key down and/or sustained). */
    };

    /** Set the MPE tracking mode for the pressure dimension. */
    z0 setPressureTrackingMode (TrackingMode modeToUse);

    /** Set the MPE tracking mode for the pitchbend dimension. */
    z0 setPitchbendTrackingMode (TrackingMode modeToUse);

    /** Set the MPE tracking mode for the timbre dimension. */
    z0 setTimbreTrackingMode (TrackingMode modeToUse);

    //==============================================================================
    /** Process a MIDI message and trigger the appropriate method calls
        (noteOn, noteOff etc.)

        You can override this method if you need some special MIDI message
        treatment on top of the standard MPE logic implemented here.
    */
    virtual z0 processNextMidiEvent (const MidiMessage& message);

    //==============================================================================
    /** Request a note-on on the given channel, with the given initial note
        number and velocity.

        If the message arrives on a valid note channel, this will create a
        new MPENote and call the noteAdded callback.
    */
    virtual z0 noteOn (i32 midiChannel, i32 midiNoteNumber, MPEValue midiNoteOnVelocity);

    /** Request a note-off.

        If there is a matching playing note, this will release the note
        (except if it is sustained by a sustain or sostenuto pedal) and call
        the noteReleased callback.
    */
    virtual z0 noteOff (i32 midiChannel, i32 midiNoteNumber, MPEValue midiNoteOffVelocity);

    /** Request a pitchbend on the given channel with the given value (in units
        of MIDI pitchwheel position).

        Internally, this will determine whether the pitchwheel move is a
        per-note pitchbend or a master pitchbend (depending on midiChannel),
        take the correct per-note or master pitchbend range of the affected MPE
        zone, and apply the resulting pitchbend to the affected note(s) (if any).
    */
    virtual z0 pitchbend (i32 midiChannel, MPEValue pitchbend);

    /** Request a pressure change on the given channel with the given value.

        This will modify the pressure dimension of the note currently held down
        on this channel (if any). If the channel is a zone master channel,
        the pressure change will be broadcast to all notes in this zone.
    */
    virtual z0 pressure (i32 midiChannel, MPEValue value);

    /** Request a third dimension (timbre) change on the given channel with the
        given value.

        This will modify the timbre dimension of the note currently held down
        on this channel (if any). If the channel is a zone master channel,
        the timbre change will be broadcast to all notes in this zone.
    */
    virtual z0 timbre (i32 midiChannel, MPEValue value);

    /** Request a poly-aftertouch change for a given note number.

        The change will be broadcast to all notes sharing the channel and note
        number of the change message.
     */
    virtual z0 polyAftertouch (i32 midiChannel, i32 midiNoteNumber, MPEValue value);

    /** Request a sustain pedal press or release.

        If midiChannel is a zone's master channel, this will act on all notes in
        that zone; otherwise, nothing will happen.
    */
    virtual z0 sustainPedal (i32 midiChannel, b8 isDown);

    /** Request a sostenuto pedal press or release.

        If midiChannel is a zone's master channel, this will act on all notes in
        that zone; otherwise, nothing will happen.
    */
    virtual z0 sostenutoPedal (i32 midiChannel, b8 isDown);

    /** Discard all currently playing notes.

        This will also call the noteReleased listener callback for all of them.
    */
    z0 releaseAllNotes();

    //==============================================================================
    /** Returns the number of MPE notes currently played by the instrument. */
    i32 getNumPlayingNotes() const noexcept;

    /** Returns the note at the given index.

        If there is no such note, returns an invalid MPENote. The notes are sorted
        such that the most recently added note is the last element.
    */
    MPENote getNote (i32 index) const noexcept;

    /** Returns the note currently playing on the given midiChannel with the
        specified initial MIDI note number, if there is such a note. Otherwise,
        this returns an invalid MPENote (check with note.isValid() before use!)
    */
    MPENote getNote (i32 midiChannel, i32 midiNoteNumber) const noexcept;

    /** Returns the note with a given ID. */
    MPENote getNoteWithID (u16 noteID) const noexcept;

    /** Returns the most recent note that is playing on the given midiChannel
        (this will be the note which has received the most recent note-on without
        a corresponding note-off), if there is such a note. Otherwise, this returns an
        invalid MPENote (check with note.isValid() before use!)
    */
    MPENote getMostRecentNote (i32 midiChannel) const noexcept;

    /** Returns the most recent note that is not the note passed in. If there is no
        such note, this returns an invalid MPENote (check with note.isValid() before use!).

        This helper method might be useful for some custom voice handling algorithms.
    */
    MPENote getMostRecentNoteOtherThan (MPENote otherThanThisNote) const noexcept;

    //==============================================================================
    /** Derive from this class to be informed about any changes in the MPE notes played
        by this instrument, and any changes to its zone layout.

        Note: This listener type receives its callbacks immediately, and not
        via the message thread (so you might be for example in the MIDI thread).
        Therefore you should never do heavy work such as graphics rendering etc.
        inside those callbacks.
    */
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Implement this callback to be informed whenever a new expressive MIDI
            note is triggered.
        */
        virtual z0 noteAdded (MPENote newNote);

        /** Implement this callback to be informed whenever a currently playing
            MPE note's pressure value changes.
        */
        virtual z0 notePressureChanged (MPENote changedNote);

        /** Implement this callback to be informed whenever a currently playing
            MPE note's pitchbend value changes.

            Note: This can happen if the note itself is bent, if there is a
            master channel pitchbend event, or if both occur simultaneously.
            Call MPENote::getFrequencyInHertz to get the effective note frequency.
        */
        virtual z0 notePitchbendChanged (MPENote changedNote);

        /** Implement this callback to be informed whenever a currently playing
            MPE note's timbre value changes.
        */
        virtual z0 noteTimbreChanged (MPENote changedNote);

        /** Implement this callback to be informed whether a currently playing
            MPE note's key state (whether the key is down and/or the note is
            sustained) has changed.

            Note: If the key state changes to MPENote::off, noteReleased is
            called instead.
        */
        virtual z0 noteKeyStateChanged (MPENote changedNote);

        /** Implement this callback to be informed whenever an MPE note
            is released (either by a note-off message, or by a sustain/sostenuto
            pedal release for a note that already received a note-off),
            and should therefore stop playing.
        */
        virtual z0 noteReleased (MPENote finishedNote);

        /** Implement this callback to be informed whenever the MPE zone layout
            or legacy mode settings of this instrument have been changed.
        */
        virtual z0 zoneLayoutChanged();
    };

    //==============================================================================
    /** Adds a listener. */
    z0 addListener (Listener* listenerToAdd);

    /** Removes a listener. */
    z0 removeListener (Listener* listenerToRemove);

    //==============================================================================
    /** Puts the instrument into legacy mode. If legacy mode is already enabled this method
        does nothing.

        As a side effect, this will discard all currently playing notes,
        and call noteReleased for all of them.

        This special zone layout mode is for backwards compatibility with
        non-MPE MIDI devices. In this mode, the instrument will ignore the
        current MPE zone layout. It will instead take a range of MIDI channels
        (default: all channels 1-16) and treat them as note channels, with no
        master channel. MIDI channels outside of this range will be ignored.

        @param pitchbendRange   The note pitchbend range in semitones to use when in legacy mode.
                                Must be between 0 and 96, otherwise behaviour is undefined.
                                The default pitchbend range in legacy mode is +/- 2 semitones.

        @param channelRange     The range of MIDI channels to use for notes when in legacy mode.
                                The default is to use all MIDI channels (1-16).

        To get out of legacy mode, set a new MPE zone layout using setZoneLayout.
    */
    z0 enableLegacyMode (i32 pitchbendRange = 2,
                           Range<i32> channelRange = Range<i32> (1, 17));

    /** Возвращает true, если the instrument is in legacy mode, false otherwise. */
    b8 isLegacyModeEnabled() const noexcept;

    /** Returns the range of MIDI channels (1-16) to be used for notes when in legacy mode. */
    Range<i32> getLegacyModeChannelRange() const noexcept;

    /** Re-sets the range of MIDI channels (1-16) to be used for notes when in legacy mode. */
    z0 setLegacyModeChannelRange (Range<i32> channelRange);

    /** Returns the pitchbend range in semitones (0-96) to be used for notes when in legacy mode. */
    i32 getLegacyModePitchbendRange() const noexcept;

    /** Re-sets the pitchbend range in semitones (0-96) to be used for notes when in legacy mode. */
    z0 setLegacyModePitchbendRange (i32 pitchbendRange);

protected:
    //==============================================================================
    CriticalSection lock;

private:
    //==============================================================================
    Array<MPENote> notes;
    MPEZoneLayout zoneLayout;
    ListenerList<Listener> listeners;

    u8 lastPressureLowerBitReceivedOnChannel[16];
    u8 lastTimbreLowerBitReceivedOnChannel[16];
    b8 isMemberChannelSustained[16];

    struct LegacyMode
    {
        b8 isEnabled = false;
        Range<i32> channelRange;
        i32 pitchbendRange = 2;
    };

    struct MPEDimension
    {
        TrackingMode trackingMode = lastNotePlayedOnChannel;
        MPEValue lastValueReceivedOnChannel[16];
        MPEValue MPENote::* value;
        MPEValue& getValue (MPENote& note) noexcept   { return note.*(value); }
    };

    LegacyMode legacyMode;
    MPEDimension pitchbendDimension, pressureDimension, timbreDimension;

    z0 resetLastReceivedValues();

    z0 updateDimension (i32 midiChannel, MPEDimension&, MPEValue);
    z0 updateDimensionMaster (b8, MPEDimension&, MPEValue);
    z0 updateDimensionForNote (MPENote&, MPEDimension&, MPEValue);
    z0 callListenersDimensionChanged (const MPENote&, const MPEDimension&);
    MPEValue getInitialValueForNewNote (i32 midiChannel, MPEDimension&) const;

    z0 processMidiNoteOnMessage (const MidiMessage&);
    z0 processMidiNoteOffMessage (const MidiMessage&);
    z0 processMidiPitchWheelMessage (const MidiMessage&);
    z0 processMidiChannelPressureMessage (const MidiMessage&);
    z0 processMidiControllerMessage (const MidiMessage&);
    z0 processMidiResetAllControllersMessage (const MidiMessage&);
    z0 processMidiAfterTouchMessage (const MidiMessage&);
    z0 handlePressureMSB (i32 midiChannel, i32 value) noexcept;
    z0 handlePressureLSB (i32 midiChannel, i32 value) noexcept;
    z0 handleTimbreMSB (i32 midiChannel, i32 value) noexcept;
    z0 handleTimbreLSB (i32 midiChannel, i32 value) noexcept;
    z0 handleSustainOrSostenuto (i32 midiChannel, b8 isDown, b8 isSostenuto);

    const MPENote* getNotePtr (i32 midiChannel, i32 midiNoteNumber) const noexcept;
    MPENote* getNotePtr (i32 midiChannel, i32 midiNoteNumber) noexcept;
    const MPENote* getNotePtr (i32 midiChannel, TrackingMode) const noexcept;
    MPENote* getNotePtr (i32 midiChannel, TrackingMode) noexcept;
    const MPENote* getLastNotePlayedPtr (i32 midiChannel) const noexcept;
    MPENote* getLastNotePlayedPtr (i32 midiChannel) noexcept;
    const MPENote* getHighestNotePtr (i32 midiChannel) const noexcept;
    MPENote* getHighestNotePtr (i32 midiChannel) noexcept;
    const MPENote* getLowestNotePtr (i32 midiChannel) const noexcept;
    MPENote* getLowestNotePtr (i32 midiChannel) noexcept;
    z0 updateNoteTotalPitchbend (MPENote&);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPEInstrument)
};

} // namespace drx
