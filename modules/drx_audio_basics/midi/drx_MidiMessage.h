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
    Encapsulates a MIDI message.

    @see MidiMessageSequence, MidiOutput, MidiInput

    @tags{Audio}
*/
class DRX_API  MidiMessage
{
public:
    //==============================================================================
    /** Creates a 3-byte short midi message.

        @param byte1            message byte 1
        @param byte2            message byte 2
        @param byte3            message byte 3
        @param timeStamp        the time to give the midi message - this value doesn't
                                use any particular units, so will be application-specific
    */
    MidiMessage (i32 byte1, i32 byte2, i32 byte3, f64 timeStamp = 0) noexcept;

    /** Creates a 2-byte short midi message.

        @param byte1            message byte 1
        @param byte2            message byte 2
        @param timeStamp        the time to give the midi message - this value doesn't
                                use any particular units, so will be application-specific
    */
    MidiMessage (i32 byte1, i32 byte2, f64 timeStamp = 0) noexcept;

    /** Creates a 1-byte short midi message.

        @param byte1            message byte 1
        @param timeStamp        the time to give the midi message - this value doesn't
                                use any particular units, so will be application-specific
    */
    MidiMessage (i32 byte1, f64 timeStamp = 0) noexcept;

    /** Creates a midi message from a list of bytes. */
    template <typename... Data>
    MidiMessage (i32 byte1, i32 byte2, i32 byte3, Data... otherBytes)  : size (3 + sizeof... (otherBytes))
    {
        // this checks that the length matches the data..
        jassert (size > 3 || byte1 >= 0xf0 || getMessageLengthFromFirstByte ((u8) byte1) == size);

        u8k data[] = { (u8) byte1, (u8) byte2, (u8) byte3, static_cast<u8> (otherBytes)... };
        memcpy (allocateSpace (size), data, (size_t) size);
    }


    /** Creates a midi message from a block of data. */
    MidiMessage (ukk data, i32 numBytes, f64 timeStamp = 0);

    /** Reads the next midi message from some data.

        This will read as many bytes from a data stream as it needs to make a
        complete message, and will return the number of bytes it used. This lets
        you read a sequence of midi messages from a file or stream.

        @param data                     the data to read from
        @param maxBytesToUse            the maximum number of bytes it's allowed to read
        @param numBytesUsed             returns the number of bytes that were actually needed
        @param lastStatusByte           in a sequence of midi messages, the initial byte
                                        can be dropped from a message if it's the same as the
                                        first byte of the previous message, so this lets you
                                        supply the byte to use if the first byte of the message
                                        has in fact been dropped.
        @param timeStamp                the time to give the midi message - this value doesn't
                                        use any particular units, so will be application-specific
        @param sysexHasEmbeddedLength   when reading sysexes, this flag indicates whether
                                        to expect the data to begin with a variable-length
                                        field indicating its size
    */
    MidiMessage (ukk data, i32 maxBytesToUse,
                 i32& numBytesUsed, u8 lastStatusByte,
                 f64 timeStamp = 0,
                 b8 sysexHasEmbeddedLength = true);

    /** Creates an empty sysex message.

        Since the MidiMessage has to contain a valid message, this default constructor
        just initialises it with an empty sysex message.
    */
    MidiMessage() noexcept;

    /** Creates a copy of another midi message. */
    MidiMessage (const MidiMessage&);

    /** Creates a copy of another midi message, with a different timestamp. */
    MidiMessage (const MidiMessage&, f64 newTimeStamp);

    /** Destructor. */
    ~MidiMessage() noexcept;

    /** Copies this message from another one. */
    MidiMessage& operator= (const MidiMessage& other);

    /** Move constructor */
    MidiMessage (MidiMessage&&) noexcept;

    /** Move assignment operator */
    MidiMessage& operator= (MidiMessage&&) noexcept;

    //==============================================================================
    /** Returns a pointer to the raw midi data.
        @see getRawDataSize
    */
    u8k* getRawData() const noexcept            { return getData(); }

    /** Returns the number of bytes of data in the message.
        @see getRawData
    */
    i32 getRawDataSize() const noexcept                 { return size; }

    //==============================================================================
    /** Returns a human-readable description of the midi message as a string,
        for example "Note On C#3 Velocity 120 Channel 1".
    */
    Txt getDescription() const;

    //==============================================================================
    /** Returns the timestamp associated with this message.

        The exact meaning of this time and its units will vary, as messages are used in
        a variety of different contexts.

        If you're getting the message from a midi file, this could be a time in seconds, or
        a number of ticks - see MidiFile::convertTimestampTicksToSeconds().

        If the message is being used in a MidiBuffer, it might indicate the number of
        audio samples from the start of the buffer.

        If the message was created by a MidiInput, see MidiInputCallback::handleIncomingMidiMessage()
        for details of the way that it initialises this value.

        @see setTimeStamp, addToTimeStamp
    */
    f64 getTimeStamp() const noexcept                { return timeStamp; }

    /** Changes the message's associated timestamp.
        The units for the timestamp will be application-specific - see the notes for getTimeStamp().
        @see addToTimeStamp, getTimeStamp
    */
    z0 setTimeStamp (f64 newTimestamp) noexcept    { timeStamp = newTimestamp; }

    /** Adds a value to the message's timestamp.
        The units for the timestamp will be application-specific.
    */
    z0 addToTimeStamp (f64 delta) noexcept         { timeStamp += delta; }

    /** Return a copy of this message with a new timestamp.
        The units for the timestamp will be application-specific - see the notes for getTimeStamp().
    */
    MidiMessage withTimeStamp (f64 newTimestamp) const;

    //==============================================================================
    /** Returns the midi channel associated with the message.

        @returns    a value 1 to 16 if the message has a channel, or 0 if it hasn't (e.g.
                    if it's a sysex)
        @see isForChannel, setChannel
    */
    i32 getChannel() const noexcept;

    /** Возвращает true, если the message applies to the given midi channel.

        @param channelNumber    the channel number to look for, in the range 1 to 16
        @see getChannel, setChannel
    */
    b8 isForChannel (i32 channelNumber) const noexcept;

    /** Changes the message's midi channel.
        This won't do anything for non-channel messages like sysexes.
        @param newChannelNumber    the channel number to change it to, in the range 1 to 16
    */
    z0 setChannel (i32 newChannelNumber) noexcept;

    //==============================================================================
    /** Возвращает true, если this is a system-exclusive message.
    */
    b8 isSysEx() const noexcept;

    /** Returns a pointer to the sysex data inside the message.
        If this event isn't a sysex event, it'll return 0.
        @see getSysExDataSize
    */
    u8k* getSysExData() const noexcept;

    /** Returns the size of the sysex data.
        This value excludes the 0xf0 header byte and the 0xf7 at the end.
        @see getSysExData
    */
    i32 getSysExDataSize() const noexcept;

    /** Returns a span that bounds the sysex body bytes contained in this message. */
    Span<const std::byte> getSysExDataSpan() const noexcept
    {
        return { reinterpret_cast<const std::byte*> (getSysExData()),
                 (size_t) getSysExDataSize() };
    }

    //==============================================================================
    /** Возвращает true, если this message is a 'key-down' event.

        @param returnTrueForVelocity0   if true, then if this event is a note-on with
                        velocity 0, it will still be considered to be a note-on and the
                        method will return true. If returnTrueForVelocity0 is false, then
                        if this is a note-on event with velocity 0, it'll be regarded as
                        a note-off, and the method will return false

        @see isNoteOff, getNoteNumber, getVelocity, noteOn
    */
    b8 isNoteOn (b8 returnTrueForVelocity0 = false) const noexcept;

    /** Creates a key-down message (using a floating-point velocity).

        @param channel      the midi channel, in the range 1 to 16
        @param noteNumber   the key number, 0 to 127
        @param velocity     in the range 0 to 1.0
        @see isNoteOn
    */
    static MidiMessage noteOn (i32 channel, i32 noteNumber, f32 velocity) noexcept;

    /** Creates a key-down message (using an integer velocity).

        @param channel      the midi channel, in the range 1 to 16
        @param noteNumber   the key number, 0 to 127
        @param velocity     in the range 0 to 127
        @see isNoteOn
    */
    static MidiMessage noteOn (i32 channel, i32 noteNumber, u8 velocity) noexcept;

    /** Возвращает true, если this message is a 'key-up' event.

        If returnTrueForNoteOnVelocity0 is true, then his will also return true
        for a note-on event with a velocity of 0.

        @see isNoteOn, getNoteNumber, getVelocity, noteOff
    */
    b8 isNoteOff (b8 returnTrueForNoteOnVelocity0 = true) const noexcept;

    /** Creates a key-up message.

        @param channel      the midi channel, in the range 1 to 16
        @param noteNumber   the key number, 0 to 127
        @param velocity     in the range 0 to 1.0
        @see isNoteOff
    */
    static MidiMessage noteOff (i32 channel, i32 noteNumber, f32 velocity) noexcept;

    /** Creates a key-up message.

        @param channel      the midi channel, in the range 1 to 16
        @param noteNumber   the key number, 0 to 127
        @param velocity     in the range 0 to 127
        @see isNoteOff
    */
    static MidiMessage noteOff (i32 channel, i32 noteNumber, u8 velocity) noexcept;

    /** Creates a key-up message.

        @param channel      the midi channel, in the range 1 to 16
        @param noteNumber   the key number, 0 to 127
        @see isNoteOff
    */
    static MidiMessage noteOff (i32 channel, i32 noteNumber) noexcept;

    /** Возвращает true, если this message is a 'key-down' or 'key-up' event.

        @see isNoteOn, isNoteOff
    */
    b8 isNoteOnOrOff() const noexcept;

    /** Returns the midi note number for note-on and note-off messages.
        If the message isn't a note-on or off, the value returned is undefined.
        @see isNoteOff, getMidiNoteName, getMidiNoteInHertz, setNoteNumber
    */
    i32 getNoteNumber() const noexcept;

    /** Changes the midi note number of a note-on or note-off message.
        If the message isn't a note on or off, this will do nothing.
    */
    z0 setNoteNumber (i32 newNoteNumber) noexcept;

    //==============================================================================
    /** Returns the velocity of a note-on or note-off message.

        The value returned will be in the range 0 to 127.
        If the message isn't a note-on or off event, it will return 0.

        @see getFloatVelocity
    */
    u8 getVelocity() const noexcept;

    /** Returns the velocity of a note-on or note-off message.

        The value returned will be in the range 0 to 1.0
        If the message isn't a note-on or off event, it will return 0.

        @see getVelocity, setVelocity
    */
    f32 getFloatVelocity() const noexcept;

    /** Changes the velocity of a note-on or note-off message.

        If the message isn't a note on or off, this will do nothing.

        @param newVelocity  the new velocity, in the range 0 to 1.0
        @see getFloatVelocity, multiplyVelocity
    */
    z0 setVelocity (f32 newVelocity) noexcept;

    /** Multiplies the velocity of a note-on or note-off message by a given amount.

        If the message isn't a note on or off, this will do nothing.

        @param scaleFactor  the value by which to multiply the velocity
        @see setVelocity
    */
    z0 multiplyVelocity (f32 scaleFactor) noexcept;

    //==============================================================================
    /** Возвращает true, если this message is a 'sustain pedal down' controller message. */
    b8 isSustainPedalOn() const noexcept;
    /** Возвращает true, если this message is a 'sustain pedal up' controller message. */
    b8 isSustainPedalOff() const noexcept;

    /** Возвращает true, если this message is a 'sostenuto pedal down' controller message. */
    b8 isSostenutoPedalOn() const noexcept;
    /** Возвращает true, если this message is a 'sostenuto pedal up' controller message. */
    b8 isSostenutoPedalOff() const noexcept;

    /** Возвращает true, если this message is a 'soft pedal down' controller message. */
    b8 isSoftPedalOn() const noexcept;
    /** Возвращает true, если this message is a 'soft pedal up' controller message. */
    b8 isSoftPedalOff() const noexcept;

    //==============================================================================
    /** Возвращает true, если the message is a program (patch) change message.
        @see getProgramChangeNumber, getGMInstrumentName
    */
    b8 isProgramChange() const noexcept;

    /** Returns the new program number of a program change message.
        If the message isn't a program change, the value returned is undefined.
        @see isProgramChange, getGMInstrumentName
    */
    i32 getProgramChangeNumber() const noexcept;

    /** Creates a program-change message.

        @param channel          the midi channel, in the range 1 to 16
        @param programNumber    the midi program number, 0 to 127
        @see isProgramChange, getGMInstrumentName
    */
    static MidiMessage programChange (i32 channel, i32 programNumber) noexcept;

    //==============================================================================
    /** Возвращает true, если the message is a pitch-wheel move.
        @see getPitchWheelValue, pitchWheel
    */
    b8 isPitchWheel() const noexcept;

    /** Returns the pitch wheel position from a pitch-wheel move message.

        The value returned is a 14-bit number from 0 to 0x3fff, indicating the wheel position.
        If called for messages which aren't pitch wheel events, the number returned will be
        nonsense.

        @see isPitchWheel
    */
    i32 getPitchWheelValue() const noexcept;

    /** Creates a pitch-wheel move message.

        @param channel      the midi channel, in the range 1 to 16
        @param position     the wheel position, in the range 0 to 16383
        @see isPitchWheel
    */
    static MidiMessage pitchWheel (i32 channel, i32 position) noexcept;

    //==============================================================================
    /** Возвращает true, если the message is an aftertouch event.

        For aftertouch events, use the getNoteNumber() method to find out the key
        that it applies to, and getAfterTouchValue() to find out the amount. Use
        getChannel() to find out the channel.

        @see getAftertouchValue, getNoteNumber
    */
    b8 isAftertouch() const noexcept;

    /** Returns the amount of aftertouch from an aftertouch messages.

        The value returned is in the range 0 to 127, and will be nonsense for messages
        other than aftertouch messages.

        @see isAftertouch
    */
    i32 getAfterTouchValue() const noexcept;

    /** Creates an aftertouch message.

        @param channel              the midi channel, in the range 1 to 16
        @param noteNumber           the key number, 0 to 127
        @param aftertouchAmount     the amount of aftertouch, 0 to 127
        @see isAftertouch
    */
    static MidiMessage aftertouchChange (i32 channel,
                                         i32 noteNumber,
                                         i32 aftertouchAmount) noexcept;

    /** Возвращает true, если the message is a channel-pressure change event.

        This is like aftertouch, but common to the whole channel rather than a specific
        note. Use getChannelPressureValue() to find out the pressure, and getChannel()
        to find out the channel.

        @see channelPressureChange
    */
    b8 isChannelPressure() const noexcept;

    /** Returns the pressure from a channel pressure change message.

        @returns the pressure, in the range 0 to 127
        @see isChannelPressure, channelPressureChange
    */
    i32 getChannelPressureValue() const noexcept;

    /** Creates a channel-pressure change event.

        @param channel              the midi channel: 1 to 16
        @param pressure             the pressure, 0 to 127
        @see isChannelPressure
    */
    static MidiMessage channelPressureChange (i32 channel, i32 pressure) noexcept;

    //==============================================================================
    /** Возвращает true, если this is a midi controller message.

        @see getControllerNumber, getControllerValue, controllerEvent
    */
    b8 isController() const noexcept;

    /** Returns the controller number of a controller message.

        The name of the controller can be looked up using the getControllerName() method.
        Note that the value returned is invalid for messages that aren't controller changes.

        @see isController, getControllerName, getControllerValue
    */
    i32 getControllerNumber() const noexcept;

    /** Returns the controller value from a controller message.

        A value 0 to 127 is returned to indicate the new controller position.
        Note that the value returned is invalid for messages that aren't controller changes.

        @see isController, getControllerNumber
    */
    i32 getControllerValue() const noexcept;

    /** Возвращает true, если this message is a controller message and if it has the specified
        controller type.
    */
    b8 isControllerOfType (i32 controllerType) const noexcept;

    /** Creates a controller message.
        @param channel          the midi channel, in the range 1 to 16
        @param controllerType   the type of controller
        @param value            the controller value
        @see isController
    */
    static MidiMessage controllerEvent (i32 channel,
                                        i32 controllerType,
                                        i32 value) noexcept;

    /** Checks whether this message is an all-notes-off message.
        @see allNotesOff
    */
    b8 isAllNotesOff() const noexcept;

    /** Checks whether this message is an all-sound-off message.
        @see allSoundOff
    */
    b8 isAllSoundOff() const noexcept;

    /** Checks whether this message is a reset all controllers message.
        @see allControllerOff
    */
    b8 isResetAllControllers() const noexcept;

    /** Creates an all-notes-off message.
        @param channel              the midi channel, in the range 1 to 16
        @see isAllNotesOff
    */
    static MidiMessage allNotesOff (i32 channel) noexcept;

    /** Creates an all-sound-off message.
        @param channel              the midi channel, in the range 1 to 16
        @see isAllSoundOff
    */
    static MidiMessage allSoundOff (i32 channel) noexcept;

    /** Creates an all-controllers-off message.
        @param channel              the midi channel, in the range 1 to 16
    */
    static MidiMessage allControllersOff (i32 channel) noexcept;

    //==============================================================================
    /** Возвращает true, если this event is a meta-event.

        Meta-events are things like tempo changes, track names, etc.

        @see getMetaEventType, isTrackMetaEvent, isEndOfTrackMetaEvent,
             isTextMetaEvent, isTrackNameEvent, isTempoMetaEvent, isTimeSignatureMetaEvent,
             isKeySignatureMetaEvent, isMidiChannelMetaEvent
    */
    b8 isMetaEvent() const noexcept;

    /** Returns a meta-event's type number.

        If the message isn't a meta-event, this will return -1.

        @see isMetaEvent, isTrackMetaEvent, isEndOfTrackMetaEvent,
             isTextMetaEvent, isTrackNameEvent, isTempoMetaEvent, isTimeSignatureMetaEvent,
             isKeySignatureMetaEvent, isMidiChannelMetaEvent
    */
    i32 getMetaEventType() const noexcept;

    /** Returns a pointer to the data in a meta-event.
        @see isMetaEvent, getMetaEventLength
    */
    u8k* getMetaEventData() const noexcept;

    /** Returns the length of the data for a meta-event.
        @see isMetaEvent, getMetaEventData
    */
    i32 getMetaEventLength() const noexcept;

    //==============================================================================
    /** Возвращает true, если this is a 'track' meta-event. */
    b8 isTrackMetaEvent() const noexcept;

    /** Возвращает true, если this is an 'end-of-track' meta-event. */
    b8 isEndOfTrackMetaEvent() const noexcept;

    /** Creates an end-of-track meta-event.
        @see isEndOfTrackMetaEvent
    */
    static MidiMessage endOfTrack() noexcept;

    /** Возвращает true, если this is an 'track name' meta-event.
        You can use the getTextFromTextMetaEvent() method to get the track's name.
    */
    b8 isTrackNameEvent() const noexcept;

    /** Возвращает true, если this is a 'text' meta-event.
        @see getTextFromTextMetaEvent
    */
    b8 isTextMetaEvent() const noexcept;

    /** Returns the text from a text meta-event.
        @see isTextMetaEvent
    */
    Txt getTextFromTextMetaEvent() const;

    /** Creates a text meta-event. */
    static MidiMessage textMetaEvent (i32 type, StringRef text);

    //==============================================================================
    /** Возвращает true, если this is a 'tempo' meta-event.
        @see getTempoMetaEventTickLength, getTempoSecondsPerQuarterNote
    */
    b8 isTempoMetaEvent() const noexcept;

    /** Returns the tick length from a tempo meta-event.

        @param timeFormat   the 16-bit time format value from the midi file's header.
        @returns the tick length (in seconds).
        @see isTempoMetaEvent
    */
    f64 getTempoMetaEventTickLength (short timeFormat) const noexcept;

    /** Calculates the seconds-per-quarter-note from a tempo meta-event.
        @see isTempoMetaEvent, getTempoMetaEventTickLength
    */
    f64 getTempoSecondsPerQuarterNote() const noexcept;

    /** Creates a tempo meta-event.
        @see isTempoMetaEvent
    */
    static MidiMessage tempoMetaEvent (i32 microsecondsPerQuarterNote) noexcept;

    //==============================================================================
    /** Возвращает true, если this is a 'time-signature' meta-event.
        @see getTimeSignatureInfo
    */
    b8 isTimeSignatureMetaEvent() const noexcept;

    /** Returns the time-signature values from a time-signature meta-event.
        @see isTimeSignatureMetaEvent
    */
    z0 getTimeSignatureInfo (i32& numerator, i32& denominator) const noexcept;

    /** Creates a time-signature meta-event.
        @see isTimeSignatureMetaEvent
    */
    static MidiMessage timeSignatureMetaEvent (i32 numerator, i32 denominator);

    //==============================================================================
    /** Возвращает true, если this is a 'key-signature' meta-event.
        @see getKeySignatureNumberOfSharpsOrFlats, isKeySignatureMajorKey
    */
    b8 isKeySignatureMetaEvent() const noexcept;

    /** Returns the key from a key-signature meta-event.
        This method must only be called if isKeySignatureMetaEvent() is true.
        A positive number here indicates the number of sharps in the key signature,
        and a negative number indicates a number of flats. So e.g. 3 = F# + C# + G#,
        -2 = Bb + Eb
        @see isKeySignatureMetaEvent, isKeySignatureMajorKey
    */
    i32 getKeySignatureNumberOfSharpsOrFlats() const noexcept;

    /** Возвращает true, если this key-signature event is major, or false if it's minor.
        This method must only be called if isKeySignatureMetaEvent() is true.
    */
    b8 isKeySignatureMajorKey() const noexcept;

    /** Creates a key-signature meta-event.
        @param numberOfSharpsOrFlats    if positive, this indicates the number of sharps
                                        in the key; if negative, the number of flats
        @param isMinorKey               if true, the key is minor; if false, it is major
        @see isKeySignatureMetaEvent
    */
    static MidiMessage keySignatureMetaEvent (i32 numberOfSharpsOrFlats, b8 isMinorKey);

    //==============================================================================
    /** Возвращает true, если this is a 'channel' meta-event.

        A channel meta-event specifies the midi channel that should be used
        for subsequent meta-events.

        @see getMidiChannelMetaEventChannel
    */
    b8 isMidiChannelMetaEvent() const noexcept;

    /** Returns the channel number from a channel meta-event.

        @returns the channel, in the range 1 to 16.
        @see isMidiChannelMetaEvent
    */
    i32 getMidiChannelMetaEventChannel() const noexcept;

    /** Creates a midi channel meta-event.

        @param channel              the midi channel, in the range 1 to 16
        @see isMidiChannelMetaEvent
    */
    static MidiMessage midiChannelMetaEvent (i32 channel) noexcept;

    //==============================================================================
    /** Возвращает true, если this is an active-sense message. */
    b8 isActiveSense() const noexcept;

    //==============================================================================
    /** Возвращает true, если this is a midi start event.
        @see midiStart
    */
    b8 isMidiStart() const noexcept;

    /** Creates a midi start event. */
    static MidiMessage midiStart() noexcept;

    /** Возвращает true, если this is a midi continue event.
        @see midiContinue
    */
    b8 isMidiContinue() const noexcept;

    /** Creates a midi continue event. */
    static MidiMessage midiContinue() noexcept;

    /** Возвращает true, если this is a midi stop event.
        @see midiStop
    */
    b8 isMidiStop() const noexcept;

    /** Creates a midi stop event. */
    static MidiMessage midiStop() noexcept;

    /** Возвращает true, если this is a midi clock event.
        @see midiClock, songPositionPointer
    */
    b8 isMidiClock() const noexcept;

    /** Creates a midi clock event. */
    static MidiMessage midiClock() noexcept;

    /** Возвращает true, если this is a song-position-pointer message.
        @see getSongPositionPointerMidiBeat, songPositionPointer
    */
    b8 isSongPositionPointer() const noexcept;

    /** Returns the midi beat-number of a song-position-pointer message.
        @see isSongPositionPointer, songPositionPointer
    */
    i32 getSongPositionPointerMidiBeat() const noexcept;

    /** Creates a song-position-pointer message.

        The position is a number of midi beats from the start of the song, where 1 midi
        beat is 6 midi clocks, and there are 24 midi clocks in a quarter-note. So there
        are 4 midi beats in a quarter-note.

        @see isSongPositionPointer, getSongPositionPointerMidiBeat
    */
    static MidiMessage songPositionPointer (i32 positionInMidiBeats) noexcept;

    //==============================================================================
    /** Возвращает true, если this is a quarter-frame midi timecode message.
        @see quarterFrame, getQuarterFrameSequenceNumber, getQuarterFrameValue
    */
    b8 isQuarterFrame() const noexcept;

    /** Returns the sequence number of a quarter-frame midi timecode message.
        This will be a value between 0 and 7.
        @see isQuarterFrame, getQuarterFrameValue, quarterFrame
    */
    i32 getQuarterFrameSequenceNumber() const noexcept;

    /** Returns the value from a quarter-frame message.
        This will be the lower nybble of the message's data-byte, a value between 0 and 15
    */
    i32 getQuarterFrameValue() const noexcept;

    /** Creates a quarter-frame MTC message.

        @param sequenceNumber   a value 0 to 7 for the upper nybble of the message's data byte
        @param value            a value 0 to 15 for the lower nybble of the message's data byte
    */
    static MidiMessage quarterFrame (i32 sequenceNumber, i32 value) noexcept;

    /** SMPTE timecode types.
        Used by the getFullFrameParameters() and fullFrame() methods.
    */
    enum SmpteTimecodeType
    {
        fps24       = 0,
        fps25       = 1,
        fps30drop   = 2,
        fps30       = 3
    };

    /** Возвращает true, если this is a full-frame midi timecode message. */
    b8 isFullFrame() const noexcept;

    /** Extracts the timecode information from a full-frame midi timecode message.

        You should only call this on messages where you've used isFullFrame() to
        check that they're the right kind.
    */
    z0 getFullFrameParameters (i32& hours,
                                 i32& minutes,
                                 i32& seconds,
                                 i32& frames,
                                 SmpteTimecodeType& timecodeType) const noexcept;

    /** Creates a full-frame MTC message. */
    static MidiMessage fullFrame (i32 hours,
                                  i32 minutes,
                                  i32 seconds,
                                  i32 frames,
                                  SmpteTimecodeType timecodeType);

    //==============================================================================
    /** Types of MMC command.

        @see isMidiMachineControlMessage, getMidiMachineControlCommand, midiMachineControlCommand
    */
    enum MidiMachineControlCommand
    {
        mmc_stop            = 1,
        mmc_play            = 2,
        mmc_deferredplay    = 3,
        mmc_fastforward     = 4,
        mmc_rewind          = 5,
        mmc_recordStart     = 6,
        mmc_recordStop      = 7,
        mmc_pause           = 9
    };

    /** Checks whether this is an MMC message.
        If it is, you can use the getMidiMachineControlCommand() to find out its type.
    */
    b8 isMidiMachineControlMessage() const noexcept;

    /** For an MMC message, this returns its type.

        Make sure it's actually an MMC message with isMidiMachineControlMessage() before
        calling this method.
    */
    MidiMachineControlCommand getMidiMachineControlCommand() const noexcept;

    /** Creates an MMC message. */
    static MidiMessage midiMachineControlCommand (MidiMachineControlCommand command);

    /** Checks whether this is an MMC "goto" message.
        If it is, the parameters passed-in are set to the time that the message contains.
        @see midiMachineControlGoto
    */
    b8 isMidiMachineControlGoto (i32& hours,
                                   i32& minutes,
                                   i32& seconds,
                                   i32& frames) const noexcept;

    /** Creates an MMC "goto" message.
        This messages tells the device to go to a specific frame.
        @see isMidiMachineControlGoto
    */
    static MidiMessage midiMachineControlGoto (i32 hours,
                                               i32 minutes,
                                               i32 seconds,
                                               i32 frames);

    //==============================================================================
    /** Creates a master-volume change message.
        @param volume   the volume, 0 to 1.0
    */
    static MidiMessage masterVolume (f32 volume);

    //==============================================================================
    /** Creates a system-exclusive message.
        The data passed in is wrapped with header and tail bytes of 0xf0 and 0xf7.
    */
    static MidiMessage createSysExMessage (ukk sysexData,
                                           i32 dataSize);

    /** Creates a system-exclusive message.
        The data passed in is wrapped with header and tail bytes of 0xf0 and 0xf7.
    */
    static MidiMessage createSysExMessage (Span<const std::byte> data);

    //==============================================================================
   #ifndef DOXYGEN
    /** Reads a midi variable-length integer.

        The `data` argument indicates the data to read the number from,
        and `numBytesUsed` is used as an out-parameter to indicate the number
        of bytes that were read.
    */
    [[deprecated ("This signature has been deprecated in favour of the safer readVariableLengthValue.")]]
    static i32 readVariableLengthVal (u8k* data, i32& numBytesUsed) noexcept;
   #endif

    /** Holds information about a variable-length value which was parsed
        from a stream of bytes.

        A valid value requires that `bytesUsed` is greater than 0.
    */
    struct VariableLengthValue
    {
        VariableLengthValue() = default;

        VariableLengthValue (i32 valueIn, i32 bytesUsedIn)
            : value (valueIn), bytesUsed (bytesUsedIn) {}

        b8 isValid() const noexcept  { return bytesUsed > 0; }

        i32 value = 0;
        i32 bytesUsed = 0;
    };

    /** Reads a midi variable-length integer, with protection against buffer overflow.

        @param data             the data to read the number from
        @param maxBytesToUse    the number of bytes in the region following `data`
        @returns                a struct containing the parsed value, and the number
                                of bytes that were read. If parsing fails, both the
                                `value` and `bytesUsed` fields will be set to 0 and
                                `isValid()` will return false
    */
    static VariableLengthValue readVariableLengthValue (u8k* data,
                                                        i32 maxBytesToUse) noexcept;

    /** Based on the first byte of a short midi message, this uses a lookup table
        to return the message length (either 1, 2, or 3 bytes).

        The value passed in must be 0x80 or higher.
    */
    static i32 getMessageLengthFromFirstByte (u8 firstByte) noexcept;

    //==============================================================================
    /** Returns the name of a midi note number.

        E.g "C", "D#", etc.

        @param noteNumber           the midi note number, 0 to 127
        @param useSharps            if true, sharpened notes are used, e.g. "C#", otherwise
                                    they'll be flattened, e.g. "Db"
        @param includeOctaveNumber  if true, the octave number will be appended to the string,
                                    e.g. "C#4"
        @param octaveNumForMiddleC  if an octave number is being appended, this indicates the
                                    number that will be used for middle C's octave

        @see getMidiNoteInHertz
    */
    static Txt getMidiNoteName (i32 noteNumber,
                                   b8 useSharps,
                                   b8 includeOctaveNumber,
                                   i32 octaveNumForMiddleC);

    /** Returns the frequency of a midi note number.

        The frequencyOfA parameter is an optional frequency for 'A', normally 440-444Hz for concert pitch.
        @see getMidiNoteName
    */
    static f64 getMidiNoteInHertz (i32 noteNumber, f64 frequencyOfA = 440.0) noexcept;

    /** Возвращает true, если the given midi note number is a black key. */
    static b8 isMidiNoteBlack (i32 noteNumber) noexcept;

    /** Returns the standard name of a GM instrument, or nullptr if unknown for this index.

        @param midiInstrumentNumber     the program number 0 to 127
        @see getProgramChangeNumber
    */
    static tukk getGMInstrumentName (i32 midiInstrumentNumber);

    /** Returns the name of a bank of GM instruments, or nullptr if unknown for this bank number.
        @param midiBankNumber   the bank, 0 to 15
    */
    static tukk getGMInstrumentBankName (i32 midiBankNumber);

    /** Returns the standard name of a channel 10 percussion sound, or nullptr if unknown for this note number.
        @param midiNoteNumber   the key number, 35 to 81
    */
    static tukk getRhythmInstrumentName (i32 midiNoteNumber);

    /** Returns the name of a controller type number, or nullptr if unknown for this controller number.
        @see getControllerNumber
    */
    static tukk getControllerName (i32 controllerNumber);

    /** Converts a floating-point value between 0 and 1 to a MIDI 7-bit value between 0 and 127. */
    static u8 floatValueToMidiByte (f32 valueBetween0and1) noexcept;

    /** Converts a pitchbend value in semitones to a MIDI 14-bit pitchwheel position value. */
    static u16 pitchbendToPitchwheelPos (f32 pitchbendInSemitones,
                                            f32 pitchbendRangeInSemitones) noexcept;

private:
    //==============================================================================
   #ifndef DOXYGEN
    union PackedData
    {
        u8* allocatedData;
        u8 asBytes[sizeof (u8*)];
    };

    PackedData packedData;
    f64 timeStamp = 0;
    i32 size;
   #endif

    inline b8 isHeapAllocated() const noexcept  { return size > (i32) sizeof (packedData); }
    inline u8* getData() const noexcept        { return isHeapAllocated() ? packedData.allocatedData : (u8*) packedData.asBytes; }
    u8* allocateSpace (i32);
};

} // namespace drx
