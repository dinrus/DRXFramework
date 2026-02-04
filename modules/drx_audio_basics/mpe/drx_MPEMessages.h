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
    This helper class contains the necessary helper functions to generate
    MIDI messages that are exclusive to MPE, such as defining the upper and lower
    MPE zones and setting per-note and master pitchbend ranges.
    You can then send them to your MPE device using MidiOutput::sendBlockOfMessagesNow.

    All other MPE messages like per-note pitchbend, pressure, and third
    dimension, are ordinary MIDI messages that should be created using the MidiMessage
    class instead. You just need to take care to send them to the appropriate
    per-note MIDI channel.

    Note: If you are working with an MPEZoneLayout object inside your app,
    you should not use the message sequences provided here. Instead, you should
    change the zone layout programmatically with the member functions provided in the
    MPEZoneLayout class itself. You should also make sure that the Expressive
    MIDI zone layout of your C++ code and of the MPE device are kept in sync.

    @see MidiMessage, MPEZoneLayout

    @tags{Audio}
*/
class DRX_API  MPEMessages
{
public:
    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will set the lower MPE zone.
    */
    static MidiBuffer setLowerZone (i32 numMemberChannels = 0,
                                    i32 perNotePitchbendRange = 48,
                                    i32 masterPitchbendRange = 2);

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will set the upper MPE zone.
    */
    static MidiBuffer setUpperZone (i32 numMemberChannels = 0,
                                    i32 perNotePitchbendRange = 48,
                                    i32 masterPitchbendRange = 2);

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will set the per-note pitchbend range of the lower MPE zone.
    */
    static MidiBuffer setLowerZonePerNotePitchbendRange (i32 perNotePitchbendRange = 48);

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will set the per-note pitchbend range of the upper MPE zone.
    */
    static MidiBuffer setUpperZonePerNotePitchbendRange (i32 perNotePitchbendRange = 48);

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will set the master pitchbend range of the lower MPE zone.
    */
    static MidiBuffer setLowerZoneMasterPitchbendRange (i32 masterPitchbendRange = 2);

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will set the master pitchbend range of the upper MPE zone.
    */
    static MidiBuffer setUpperZoneMasterPitchbendRange (i32 masterPitchbendRange = 2);

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will clear the lower zone.
    */
    static MidiBuffer clearLowerZone();

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will clear the upper zone.
    */
    static MidiBuffer clearUpperZone();

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will clear the lower and upper zones.
    */
    static MidiBuffer clearAllZones();

    /** Returns the sequence of MIDI messages that, if sent to an Expressive
        MIDI device, will reset the whole MPE zone layout of the
        device to the layout passed in. This will first clear the current lower and upper
        zones, then then set the zones contained in the passed-in zone layout, and set their
        per-note and master pitchbend ranges to their current values.
    */
    static MidiBuffer setZoneLayout (MPEZoneLayout layout);

    /** The RPN number used for MPE zone layout messages.

        Pitchbend range messages (both per-note and master) are instead sent
        on RPN 0 as in standard MIDI 1.0.
    */
    static i32k zoneLayoutMessagesRpnNumber = 6;
};

} // namespace drx
