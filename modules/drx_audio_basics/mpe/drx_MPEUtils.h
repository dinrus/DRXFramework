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
    This class handles the assignment of new MIDI notes to member channels of an active
    MPE zone.

    To use it, create an instance passing in the MPE zone that it should operate on
    and then call use the findMidiChannelForNewNote() method for all note-on messages
    and the noteOff() method for all note-off messages.

    @tags{Audio}
*/
class MPEChannelAssigner
{
public:
    /** Constructor.

        This will assign channels within the range of the specified MPE zone.
    */
    MPEChannelAssigner (MPEZoneLayout::Zone zoneToUse);

    /** Legacy mode constructor.

        This will assign channels within the specified range.
    */
    MPEChannelAssigner (Range<i32> channelRange = Range<i32> (1, 17));

    /** This method will use a set of rules recommended in the MPE specification to
        determine which member channel the specified MIDI note should be assigned to
        and will return this channel number.

        The rules have the following precedence:
          - find a free channel on which the last note played was the same as the one specified
          - find the next free channel in round-robin assignment
          - find the channel number that is currently playing the closest note (but not the same)

        @param noteNumber  the MIDI note number to be assigned to a channel
        @returns           the zone's member channel that this note should be assigned to
    */
    i32 findMidiChannelForNewNote (i32 noteNumber) noexcept;

    /** If a note has been added using findMidiChannelForNewNote() this will return the channel
        to which it was assigned, otherwise it will return -1.
    */
    i32 findMidiChannelForExistingNote (i32 initialNoteOnNumber) noexcept;

    /** You must call this method for all note-offs that you receive so that this class
        can keep track of the currently playing notes internally.

        You can specify the channel number the note off happened on. If you don't, it will
        look through all channels to find the registered midi note matching the given note number.
    */
    z0 noteOff (i32 noteNumber, i32 midiChannel = -1);

    /** Call this to clear all currently playing notes. */
    z0 allNotesOff();

private:
    b8 isLegacy = false;
    std::unique_ptr<MPEZoneLayout::Zone> zone;
    i32 channelIncrement, numChannels, firstChannel, lastChannel, midiChannelLastAssigned;

    //==============================================================================
    struct MidiChannel
    {
        Array<i32> notes;
        i32 lastNotePlayed = -1;
        b8 isFree() const noexcept  { return notes.isEmpty(); }
    };
    std::array<MidiChannel, 17> midiChannels;

    //==============================================================================
    i32 findMidiChannelPlayingClosestNonequalNote (i32 noteNumber) noexcept;
};

//==============================================================================
/**
    This class handles the logic for remapping MIDI note messages from multiple MPE
    sources onto a specified MPE zone.

    @tags{Audio}
*/
class MPEChannelRemapper
{
public:
    /** Used to indicate that a particular source & channel combination is not currently using MPE. */
    static u32k notMPE = 0;

    /** Constructor */
    MPEChannelRemapper (MPEZoneLayout::Zone zoneToRemap);

    //==============================================================================
    /** Remaps the MIDI channel of the specified MIDI message (if necessary).

        Note that the MidiMessage object passed in will have it's channel changed if it
        needs to be remapped.

        @param message      the message to be remapped
        @param mpeSourceID  the ID of the MPE source of the message. This is up to the
                            user to define and keep constant
    */
    z0 remapMidiChannelIfNeeded (MidiMessage& message, u32 mpeSourceID) noexcept;

    //==============================================================================
    /** Resets all the source & channel combinations. */
    z0 reset() noexcept;

    /** Clears a specified channel of this MPE zone. */
    z0 clearChannel (i32 channel) noexcept;

    /** Clears all channels in use by a specified source. */
    z0 clearSource (u32 mpeSourceID);

private:
    MPEZoneLayout::Zone zone;

    i32 channelIncrement;
    i32 firstChannel, lastChannel;

    u32 sourceAndChannel[17];
    u32 lastUsed[17];
    u32 counter = 0;

    //==============================================================================
    b8 applyRemapIfExisting (i32 channel, u32 sourceAndChannelID, MidiMessage& m) noexcept;
    i32 getBestChanToReuse() const noexcept;

    z0 zeroArrays();

    //==============================================================================
    b8 messageIsNoteData (const MidiMessage& m)    { return (*m.getRawData() & 0xf0) != 0xf0; }
};

} // namespace drx
