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
    This struct represents an MPE zone.

    It can either be a lower or an upper zone, where:
      - A lower zone encompasses master channel 1 and an arbitrary number of ascending
        MIDI channels, increasing from channel 2.
      - An upper zone encompasses master channel 16 and an arbitrary number of descending
        MIDI channels, decreasing from channel 15.

    It also defines a pitchbend range (in semitones) to be applied for per-note pitchbends and
    master pitchbends, respectively.

    @tags{Audio}
*/
struct MPEZone
{
    enum class Type { lower, upper };

    MPEZone() = default;

    MPEZone (Type type, i32 memberChannels = 0, i32 perNotePitchbend = 48, i32 masterPitchbend = 2)
        : zoneType (type),
          numMemberChannels (memberChannels),
          perNotePitchbendRange (perNotePitchbend),
          masterPitchbendRange (masterPitchbend)
    {}

    b8 isLowerZone() const noexcept             { return zoneType == Type::lower; }
    b8 isUpperZone() const noexcept             { return zoneType == Type::upper; }

    b8 isActive() const noexcept                { return numMemberChannels > 0; }

    i32 getMasterChannel() const noexcept         { return isLowerZone() ? lowerZoneMasterChannel : upperZoneMasterChannel; }
    i32 getFirstMemberChannel() const noexcept    { return isLowerZone() ? lowerZoneMasterChannel + 1 : upperZoneMasterChannel - 1; }
    i32 getLastMemberChannel() const noexcept     { return isLowerZone() ? (lowerZoneMasterChannel + numMemberChannels)
                                                                         : (upperZoneMasterChannel - numMemberChannels); }

    b8 isUsingChannelAsMemberChannel (i32 channel) const noexcept
    {
        return isLowerZone() ? (lowerZoneMasterChannel < channel && channel <= getLastMemberChannel())
                             : (channel < upperZoneMasterChannel && getLastMemberChannel() <= channel);
    }

    b8 isUsing (i32 channel) const noexcept
    {
        return isUsingChannelAsMemberChannel (channel) || channel == getMasterChannel();
    }

    static auto tie (const MPEZone& z)
    {
        return std::tie (z.zoneType,
                         z.numMemberChannels,
                         z.perNotePitchbendRange,
                         z.masterPitchbendRange);
    }

    b8 operator== (const MPEZone& other) const
    {
        return tie (*this) == tie (other);
    }

    b8 operator!= (const MPEZone& other) const
    {
        return tie (*this) != tie (other);
    }

    //==============================================================================
    static constexpr i32 lowerZoneMasterChannel = 1,
                         upperZoneMasterChannel = 16;

    Type zoneType = Type::lower;

    i32 numMemberChannels     = 0;
    i32 perNotePitchbendRange = 48;
    i32 masterPitchbendRange  = 2;
};

//==============================================================================
/**
    This class represents the current MPE zone layout of a device capable of handling MPE.

    An MPE device can have up to two zones: a lower zone with master channel 1 and
    allocated MIDI channels increasing from channel 2, and an upper zone with master
    channel 16 and allocated MIDI channels decreasing from channel 15. MPE mode is
    enabled on a device when one of these zones is active and disabled when both
    are inactive.

    Use the MPEMessages helper class to convert the zone layout represented
    by this object to MIDI message sequences that you can send to an Expressive
    MIDI device to set its zone layout, add zones etc.

    @see MPEInstrument

    @tags{Audio}
*/
class DRX_API  MPEZoneLayout
{
public:
    //==============================================================================
    /** Creates a layout with inactive upper and lower zones. */
    MPEZoneLayout() = default;

    /** Creates a layout with the given upper and lower zones. */
    MPEZoneLayout (MPEZone lower, MPEZone upper);

    /** Creates a layout with a single upper or lower zone, leaving the other zone uninitialised. */
    MPEZoneLayout (MPEZone singleZone);

    MPEZoneLayout (const MPEZoneLayout& other);
    MPEZoneLayout& operator= (const MPEZoneLayout& other);

    b8 operator== (const MPEZoneLayout& other) const { return lowerZone == other.lowerZone && upperZone == other.upperZone; }
    b8 operator!= (const MPEZoneLayout& other) const { return ! operator== (other); }

    //==============================================================================
    /** Returns a struct representing the lower MPE zone. */
    MPEZone getLowerZone() const noexcept    { return lowerZone; }

    /** Returns a struct representing the upper MPE zone. */
    MPEZone getUpperZone() const noexcept    { return upperZone; }

    /** Sets the lower zone of this layout. */
    z0 setLowerZone (i32 numMemberChannels = 0,
                       i32 perNotePitchbendRange = 48,
                       i32 masterPitchbendRange = 2) noexcept;

    /** Sets the upper zone of this layout. */
    z0 setUpperZone (i32 numMemberChannels = 0,
                       i32 perNotePitchbendRange = 48,
                       i32 masterPitchbendRange = 2) noexcept;

    /** Clears the lower and upper zones of this layout, making them both inactive
        and disabling MPE mode.
    */
    z0 clearAllZones();

    /** Возвращает true, если either of the zones are active. */
    b8 isActive() const  { return lowerZone.isActive() || upperZone.isActive(); }

    //==============================================================================
    /** Pass incoming MIDI messages to an object of this class if you want the
        zone layout to properly react to MPE RPN messages like an
        MPE device.

        MPEMessages::rpnNumber will add or remove zones; RPN 0 will
        set the per-note or master pitchbend ranges.

        Any other MIDI messages will be ignored by this class.

        @see MPEMessages
    */
    z0 processNextMidiEvent (const MidiMessage& message);

    /** Pass incoming MIDI buffers to an object of this class if you want the
        zone layout to properly react to MPE RPN messages like an
        MPE device.

        MPEMessages::rpnNumber will add or remove zones; RPN 0 will
        set the per-note or master pitchbend ranges.

        Any other MIDI messages will be ignored by this class.

        @see MPEMessages
     */
    z0 processNextMidiBuffer (const MidiBuffer& buffer);

    //==============================================================================
    /** Listener class. Derive from this class to allow your class to be
        notified about changes to the zone layout.
    */
    class Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Implement this callback to be notified about any changes to this
            MPEZoneLayout. Will be called whenever a zone is added, zones are
            removed, or any zone's master or note pitchbend ranges change.
        */
        virtual z0 zoneLayoutChanged (const MPEZoneLayout& layout) = 0;
    };

    //==============================================================================
    /** Adds a listener. */
    z0 addListener (Listener* const listenerToAdd) noexcept;

    /** Removes a listener. */
    z0 removeListener (Listener* const listenerToRemove) noexcept;

   #ifndef DOXYGEN
    using Zone = MPEZone;
   #endif

private:
    //==============================================================================
    MPEZone lowerZone { MPEZone::Type::lower, 0 };
    MPEZone upperZone { MPEZone::Type::upper, 0 };

    MidiRPNDetector rpnDetector;
    ListenerList<Listener> listeners;

    //==============================================================================
    z0 setZone (b8, i32, i32, i32) noexcept;

    z0 processRpnMessage (MidiRPNMessage);
    z0 processZoneLayoutRpnMessage (MidiRPNMessage);
    z0 processPitchbendRangeRpnMessage (MidiRPNMessage);

    z0 updateMasterPitchbend (MPEZone&, i32);
    z0 updatePerNotePitchbendRange (MPEZone&, i32);

    z0 sendLayoutChangeMessage();
    z0 checkAndLimitZoneParameters (i32, i32, i32&) noexcept;
};

} // namespace drx
