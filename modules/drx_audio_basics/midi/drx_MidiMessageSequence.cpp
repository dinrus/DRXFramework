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

MidiMessageSequence::MidiEventHolder::MidiEventHolder (const MidiMessage& mm) : message (mm) {}
MidiMessageSequence::MidiEventHolder::MidiEventHolder (MidiMessage&& mm) : message (std::move (mm)) {}

//==============================================================================
MidiMessageSequence::MidiMessageSequence()
{
}

MidiMessageSequence::MidiMessageSequence (const MidiMessageSequence& other)
{
    list.addCopiesOf (other.list);

    for (i32 i = 0; i < list.size(); ++i)
    {
        auto noteOffIndex = other.getIndexOfMatchingKeyUp (i);

        if (noteOffIndex >= 0)
            list.getUnchecked (i)->noteOffObject = list.getUnchecked (noteOffIndex);
    }
}

MidiMessageSequence& MidiMessageSequence::operator= (const MidiMessageSequence& other)
{
    MidiMessageSequence otherCopy (other);
    swapWith (otherCopy);
    return *this;
}

MidiMessageSequence::MidiMessageSequence (MidiMessageSequence&& other) noexcept
    : list (std::move (other.list))
{
}

MidiMessageSequence& MidiMessageSequence::operator= (MidiMessageSequence&& other) noexcept
{
    list = std::move (other.list);
    return *this;
}

z0 MidiMessageSequence::swapWith (MidiMessageSequence& other) noexcept
{
    list.swapWith (other.list);
}

z0 MidiMessageSequence::clear()
{
    list.clear();
}

i32 MidiMessageSequence::getNumEvents() const noexcept
{
    return list.size();
}

MidiMessageSequence::MidiEventHolder* MidiMessageSequence::getEventPointer (i32 index) const noexcept
{
    return list[index];
}

MidiMessageSequence::MidiEventHolder** MidiMessageSequence::begin() noexcept               { return list.begin(); }
MidiMessageSequence::MidiEventHolder* const* MidiMessageSequence::begin() const noexcept   { return list.begin(); }
MidiMessageSequence::MidiEventHolder** MidiMessageSequence::end() noexcept                 { return list.end(); }
MidiMessageSequence::MidiEventHolder* const* MidiMessageSequence::end() const noexcept     { return list.end(); }

f64 MidiMessageSequence::getTimeOfMatchingKeyUp (i32 index) const noexcept
{
    if (auto* meh = list[index])
        if (auto* noteOff = meh->noteOffObject)
            return noteOff->message.getTimeStamp();

    return 0;
}

i32 MidiMessageSequence::getIndexOfMatchingKeyUp (i32 index) const noexcept
{
    if (auto* meh = list[index])
    {
        if (auto* noteOff = meh->noteOffObject)
        {
            for (i32 i = index; i < list.size(); ++i)
                if (list.getUnchecked (i) == noteOff)
                    return i;

            jassertfalse; // we've somehow got a pointer to a note-off object that isn't in the sequence
        }
    }

    return -1;
}

i32 MidiMessageSequence::getIndexOf (const MidiEventHolder* event) const noexcept
{
    return list.indexOf (event);
}

i32 MidiMessageSequence::getNextIndexAtTime (f64 timeStamp) const noexcept
{
    auto numEvents = list.size();
    i32 i;

    for (i = 0; i < numEvents; ++i)
        if (list.getUnchecked (i)->message.getTimeStamp() >= timeStamp)
            break;

    return i;
}

//==============================================================================
f64 MidiMessageSequence::getStartTime() const noexcept
{
    return getEventTime (0);
}

f64 MidiMessageSequence::getEndTime() const noexcept
{
    return getEventTime (list.size() - 1);
}

f64 MidiMessageSequence::getEventTime (i32k index) const noexcept
{
    if (auto* meh = list[index])
        return meh->message.getTimeStamp();

    return 0;
}

//==============================================================================
MidiMessageSequence::MidiEventHolder* MidiMessageSequence::addEvent (MidiEventHolder* newEvent, f64 timeAdjustment)
{
    newEvent->message.addToTimeStamp (timeAdjustment);
    auto time = newEvent->message.getTimeStamp();
    i32 i;

    for (i = list.size(); --i >= 0;)
        if (list.getUnchecked (i)->message.getTimeStamp() <= time)
            break;

    list.insert (i + 1, newEvent);
    return newEvent;
}

MidiMessageSequence::MidiEventHolder* MidiMessageSequence::addEvent (const MidiMessage& newMessage, f64 timeAdjustment)
{
    return addEvent (new MidiEventHolder (newMessage), timeAdjustment);
}

MidiMessageSequence::MidiEventHolder* MidiMessageSequence::addEvent (MidiMessage&& newMessage, f64 timeAdjustment)
{
    return addEvent (new MidiEventHolder (std::move (newMessage)), timeAdjustment);
}

z0 MidiMessageSequence::deleteEvent (i32 index, b8 deleteMatchingNoteUp)
{
    if (isPositiveAndBelow (index, list.size()))
    {
        if (deleteMatchingNoteUp)
            deleteEvent (getIndexOfMatchingKeyUp (index), false);

        list.remove (index);
    }
}

z0 MidiMessageSequence::addSequence (const MidiMessageSequence& other, f64 timeAdjustment)
{
    for (auto* m : other)
    {
        auto newOne = new MidiEventHolder (m->message);
        newOne->message.addToTimeStamp (timeAdjustment);
        list.add (newOne);
    }

    sort();
}

z0 MidiMessageSequence::addSequence (const MidiMessageSequence& other,
                                       f64 timeAdjustment,
                                       f64 firstAllowableTime,
                                       f64 endOfAllowableDestTimes)
{
    for (auto* m : other)
    {
        auto t = m->message.getTimeStamp() + timeAdjustment;

        if (t >= firstAllowableTime && t < endOfAllowableDestTimes)
        {
            auto newOne = new MidiEventHolder (m->message);
            newOne->message.setTimeStamp (t);
            list.add (newOne);
        }
    }

    sort();
}

z0 MidiMessageSequence::sort() noexcept
{
    std::stable_sort (list.begin(), list.end(),
                      [] (const MidiEventHolder* a, const MidiEventHolder* b) { return a->message.getTimeStamp() < b->message.getTimeStamp(); });
}

z0 MidiMessageSequence::updateMatchedPairs() noexcept
{
    for (i32 i = 0; i < list.size(); ++i)
    {
        auto* meh = list.getUnchecked (i);
        auto& m1 = meh->message;

        if (m1.isNoteOn())
        {
            meh->noteOffObject = nullptr;
            auto note = m1.getNoteNumber();
            auto chan = m1.getChannel();
            auto len = list.size();

            for (i32 j = i + 1; j < len; ++j)
            {
                auto* meh2 = list.getUnchecked (j);
                auto& m = meh2->message;

                if (m.getNoteNumber() == note && m.getChannel() == chan)
                {
                    if (m.isNoteOff())
                    {
                        meh->noteOffObject = meh2;
                        break;
                    }

                    if (m.isNoteOn())
                    {
                        auto newEvent = new MidiEventHolder (MidiMessage::noteOff (chan, note));
                        list.insert (j, newEvent);
                        newEvent->message.setTimeStamp (m.getTimeStamp());
                        meh->noteOffObject = newEvent;
                        break;
                    }
                }
            }
        }
    }
}

z0 MidiMessageSequence::addTimeToMessages (f64 delta) noexcept
{
    if (! approximatelyEqual (delta, 0.0))
        for (auto* m : list)
            m->message.addToTimeStamp (delta);
}

//==============================================================================
z0 MidiMessageSequence::extractMidiChannelMessages (i32k channelNumberToExtract,
                                                      MidiMessageSequence& destSequence,
                                                      const b8 alsoIncludeMetaEvents) const
{
    for (auto* meh : list)
        if (meh->message.isForChannel (channelNumberToExtract)
             || (alsoIncludeMetaEvents && meh->message.isMetaEvent()))
            destSequence.addEvent (meh->message);
}

z0 MidiMessageSequence::extractSysExMessages (MidiMessageSequence& destSequence) const
{
    for (auto* meh : list)
        if (meh->message.isSysEx())
            destSequence.addEvent (meh->message);
}

z0 MidiMessageSequence::deleteMidiChannelMessages (i32k channelNumberToRemove)
{
    for (i32 i = list.size(); --i >= 0;)
        if (list.getUnchecked (i)->message.isForChannel (channelNumberToRemove))
            list.remove (i);
}

z0 MidiMessageSequence::deleteSysExMessages()
{
    for (i32 i = list.size(); --i >= 0;)
        if (list.getUnchecked (i)->message.isSysEx())
            list.remove (i);
}

//==============================================================================
class OptionalPitchWheel
{
    Optional<i32> value;

public:
    z0 emit (i32 channel, Array<MidiMessage>& out) const
    {
        if (value.hasValue())
            out.add (MidiMessage::pitchWheel (channel, *value));
    }

    z0 set (i32 v)
    {
        value = v;
    }
};

class OptionalControllerValues
{
    Optional<t8> values[128];

public:
    z0 emit (i32 channel, Array<MidiMessage>& out) const
    {
        for (auto it = std::begin (values); it != std::end (values); ++it)
            if (it->hasValue())
                out.add (MidiMessage::controllerEvent (channel, (i32) std::distance (std::begin (values), it), **it));
    }

    z0 set (i32 controller, i32 value)
    {
        values[controller] = (t8) value;
    }
};

class OptionalProgramChange
{
    Optional<t8> value, bankLSB, bankMSB;

public:
    z0 emit (i32 channel, f64 time, Array<MidiMessage>& out) const
    {
        if (! value.hasValue())
            return;

        if (bankLSB.hasValue() && bankMSB.hasValue())
        {
            out.add (MidiMessage::controllerEvent (channel, 0x00, *bankMSB).withTimeStamp (time));
            out.add (MidiMessage::controllerEvent (channel, 0x20, *bankLSB).withTimeStamp (time));
        }

        out.add (MidiMessage::programChange (channel, *value).withTimeStamp (time));
    }

    // Возвращает true, если this is a bank number change, and false otherwise.
    b8 trySetBank (i32 controller, i32 v)
    {
        switch (controller)
        {
            case 0x00: bankMSB = (t8) v; return true;
            case 0x20: bankLSB = (t8) v; return true;
        }

        return false;
    }

    z0 setProgram (i32 v) { value = (t8) v; }
};

class ParameterNumberState
{
    enum class Kind { rpn, nrpn };

    Optional<t8> newestRpnLsb, newestRpnMsb, newestNrpnLsb, newestNrpnMsb, lastSentLsb, lastSentMsb;
    Kind lastSentKind = Kind::rpn, newestKind = Kind::rpn;

public:
    // If the effective parameter number has changed since the last time this function was called,
    // this will emit the current parameter in full (MSB and LSB).
    // This should be called before each data message (entry, increment, decrement: 0x06, 0x26, 0x60, 0x61)
    // to ensure that the data message operates on the correct parameter number.
    z0 sendIfNecessary (i32 channel, f64 time, Array<MidiMessage>& out)
    {
        const auto newestMsb = newestKind == Kind::rpn ? newestRpnMsb : newestNrpnMsb;
        const auto newestLsb = newestKind == Kind::rpn ? newestRpnLsb : newestNrpnLsb;

        auto lastSent = std::tie (lastSentKind, lastSentMsb, lastSentLsb);
        const auto newest = std::tie (newestKind, newestMsb, newestLsb);

        if (lastSent == newest || ! newestMsb.hasValue() || ! newestLsb.hasValue())
            return;

        out.add (MidiMessage::controllerEvent (channel, newestKind == Kind::rpn ? 0x65 : 0x63, *newestMsb).withTimeStamp (time));
        out.add (MidiMessage::controllerEvent (channel, newestKind == Kind::rpn ? 0x64 : 0x62, *newestLsb).withTimeStamp (time));

        lastSent = newest;
    }

    // Возвращает true, если this is a parameter number change, and false otherwise.
    b8 trySetProgramNumber (i32 controller, i32 value)
    {
        switch (controller)
        {
            case 0x65: newestRpnMsb  = (t8) value; newestKind = Kind::rpn;  return true;
            case 0x64: newestRpnLsb  = (t8) value; newestKind = Kind::rpn;  return true;
            case 0x63: newestNrpnMsb = (t8) value; newestKind = Kind::nrpn; return true;
            case 0x62: newestNrpnLsb = (t8) value; newestKind = Kind::nrpn; return true;
        }

        return false;
    }
};

z0 MidiMessageSequence::createControllerUpdatesForTime (i32 channel, f64 time, Array<MidiMessage>& dest)
{
    OptionalProgramChange programChange;
    OptionalControllerValues controllers;
    OptionalPitchWheel pitchWheel;
    ParameterNumberState parameterNumberState;

    for (const auto& item : list)
    {
        const auto& mm = item->message;

        if (! (mm.isForChannel (channel) && mm.getTimeStamp() <= time))
            continue;

        if (mm.isController())
        {
            const auto num = mm.getControllerNumber();

            if (parameterNumberState.trySetProgramNumber (num, mm.getControllerValue()))
                continue;

            if (programChange.trySetBank (num, mm.getControllerValue()))
                continue;

            constexpr i32 passthroughs[] { 0x06, 0x26, 0x60, 0x61 };

            if (std::find (std::begin (passthroughs), std::end (passthroughs), num) != std::end (passthroughs))
            {
                parameterNumberState.sendIfNecessary (channel, mm.getTimeStamp(), dest);
                dest.add (mm);
            }
            else
            {
                controllers.set (num, mm.getControllerValue());
            }
        }
        else if (mm.isProgramChange())
        {
            programChange.setProgram (mm.getProgramChangeNumber());
        }
        else if (mm.isPitchWheel())
        {
            pitchWheel.set (mm.getPitchWheelValue());
        }
    }

    pitchWheel.emit (channel, dest);
    controllers.emit (channel, dest);

    // Also emits bank change messages if necessary.
    programChange.emit (channel, time, dest);

    // Set the parameter number to its final state.
    parameterNumberState.sendIfNecessary (channel, time, dest);
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct MidiMessageSequenceTest final : public UnitTest
{
    MidiMessageSequenceTest()
        : UnitTest ("MidiMessageSequence", UnitTestCategories::midi)
    {}

    z0 runTest() override
    {
        MidiMessageSequence s;

        s.addEvent (MidiMessage::noteOn  (1, 60, 0.5f).withTimeStamp (0.0));
        s.addEvent (MidiMessage::noteOff (1, 60, 0.5f).withTimeStamp (4.0));
        s.addEvent (MidiMessage::noteOn  (1, 30, 0.5f).withTimeStamp (2.0));
        s.addEvent (MidiMessage::noteOff (1, 30, 0.5f).withTimeStamp (8.0));

        beginTest ("Start & end time");
        expectEquals (s.getStartTime(), 0.0);
        expectEquals (s.getEndTime(), 8.0);
        expectEquals (s.getEventTime (1), 2.0);

        beginTest ("Matching note off & ons");
        s.updateMatchedPairs();
        expectEquals (s.getTimeOfMatchingKeyUp (0), 4.0);
        expectEquals (s.getTimeOfMatchingKeyUp (1), 8.0);
        expectEquals (s.getIndexOfMatchingKeyUp (0), 2);
        expectEquals (s.getIndexOfMatchingKeyUp (1), 3);

        beginTest ("Time & indices");
        expectEquals (s.getNextIndexAtTime (0.5), 1);
        expectEquals (s.getNextIndexAtTime (2.5), 2);
        expectEquals (s.getNextIndexAtTime (9.0), 4);

        beginTest ("Deleting events");
        s.deleteEvent (0, true);
        expectEquals (s.getNumEvents(), 2);

        beginTest ("Merging sequences");
        MidiMessageSequence s2;
        s2.addEvent (MidiMessage::noteOn  (2, 25, 0.5f).withTimeStamp (0.0));
        s2.addEvent (MidiMessage::noteOn  (2, 40, 0.5f).withTimeStamp (1.0));
        s2.addEvent (MidiMessage::noteOff (2, 40, 0.5f).withTimeStamp (5.0));
        s2.addEvent (MidiMessage::noteOn  (2, 80, 0.5f).withTimeStamp (3.0));
        s2.addEvent (MidiMessage::noteOff (2, 80, 0.5f).withTimeStamp (7.0));
        s2.addEvent (MidiMessage::noteOff (2, 25, 0.5f).withTimeStamp (9.0));

        s.addSequence (s2, 0.0, 0.0, 8.0); // Intentionally cut off the last note off
        s.updateMatchedPairs();

        expectEquals (s.getNumEvents(), 7);
        expectEquals (s.getIndexOfMatchingKeyUp (0), -1); // Truncated note, should be no note off
        expectEquals (s.getTimeOfMatchingKeyUp (1), 5.0);

        struct ControlValue { i32 control, value; };

        struct DataEntry
        {
            i32 controllerBase, channel, parameter, value;
            f64 time;

            std::array<ControlValue, 4> getControlValues() const
            {
                return { { { controllerBase + 1, (parameter >> 7) & 0x7f },
                           { controllerBase + 0, (parameter >> 0) & 0x7f },
                           { 0x06,               (value     >> 7) & 0x7f },
                           { 0x26,               (value     >> 0) & 0x7f } } };
            }

            z0 addToSequence (MidiMessageSequence& s) const
            {
                for (const auto& pair : getControlValues())
                    s.addEvent (MidiMessage::controllerEvent (channel, pair.control, pair.value), time);
            }

            b8 matches (const MidiMessage* begin, const MidiMessage* end) const
            {
                const auto isEqual = [this] (const ControlValue& cv, const MidiMessage& msg)
                {
                    return exactlyEqual (msg.getTimeStamp(), time)
                        && msg.isController()
                        && msg.getChannel() == channel
                        && msg.getControllerNumber() == cv.control
                        && msg.getControllerValue()  == cv.value;
                };

                const auto pairs = getControlValues();
                return std::equal (pairs.begin(), pairs.end(), begin, end, isEqual);
            }
        };

        const auto addNrpn = [&] (MidiMessageSequence& seq, i32 channel, i32 parameter, i32 value, f64 time = 0.0)
        {
            DataEntry { 0x62, channel, parameter, value, time }.addToSequence (seq);
        };

        const auto addRpn = [&] (MidiMessageSequence& seq, i32 channel, i32 parameter, i32 value, f64 time = 0.0)
        {
            DataEntry { 0x64, channel, parameter, value, time }.addToSequence (seq);
        };

        const auto checkNrpn = [&] (const MidiMessage* begin, const MidiMessage* end, i32 channel, i32 parameter, i32 value, f64 time = 0.0)
        {
            expect (DataEntry { 0x62, channel, parameter, value, time }.matches (begin, end));
        };

        const auto checkRpn = [&] (const MidiMessage* begin, const MidiMessage* end, i32 channel, i32 parameter, i32 value, f64 time = 0.0)
        {
            expect (DataEntry { 0x64, channel, parameter, value, time }.matches (begin, end));
        };

        beginTest ("createControllerUpdatesForTime should emit (N)RPN components in the correct order");
        {
            const auto channel = 1;
            const auto number = 200;
            const auto value = 300;

            MidiMessageSequence sequence;
            addNrpn (sequence, channel, number, value);

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, 1.0, m);

            checkNrpn (m.begin(), m.end(), channel, number, value);
        }

        beginTest ("createControllerUpdatesForTime ignores (N)RPNs after the final requested time");
        {
            const auto channel = 2;
            const auto number = 123;
            const auto value = 456;

            MidiMessageSequence sequence;
            addRpn (sequence, channel, number, value, 0.5);
            addRpn (sequence, channel, 111,    222,   1.5);
            addRpn (sequence, channel, 333,    444,   2.5);

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, 1.0, m);

            checkRpn (m.begin(), std::next (m.begin(), 4), channel, number, value, 0.5);
        }

        beginTest ("createControllerUpdatesForTime should emit separate (N)RPN messages when appropriate");
        {
            const auto channel = 2;
            const auto numberA = 1111;
            const auto valueA = 9999;

            const auto numberB = 8888;
            const auto valueB = 2222;

            const auto numberC = 7777;
            const auto valueC = 3333;

            const auto numberD = 6666;
            const auto valueD = 4444;

            const auto time = 0.5;

            MidiMessageSequence sequence;
            addRpn  (sequence, channel, numberA, valueA, time);
            addRpn  (sequence, channel, numberB, valueB, time);
            addNrpn (sequence, channel, numberC, valueC, time);
            addNrpn (sequence, channel, numberD, valueD, time);

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, time * 2, m);

            checkRpn  (std::next (m.begin(), 0),  std::next (m.begin(), 4),  channel, numberA, valueA, time);
            checkRpn  (std::next (m.begin(), 4),  std::next (m.begin(), 8),  channel, numberB, valueB, time);
            checkNrpn (std::next (m.begin(), 8),  std::next (m.begin(), 12), channel, numberC, valueC, time);
            checkNrpn (std::next (m.begin(), 12), std::next (m.begin(), 16), channel, numberD, valueD, time);
        }

        beginTest ("createControllerUpdatesForTime correctly emits (N)RPN messages on multiple channels");
        {
            struct Info { i32 channel, number, value; };

            const Info infos[] { { 2, 1111, 9999 },
                                 { 8, 8888, 2222 },
                                 { 5, 7777, 3333 },
                                 { 1, 6666, 4444 } };

            const auto time = 0.5;

            MidiMessageSequence sequence;

            for (const auto& info : infos)
                addRpn (sequence, info.channel, info.number, info.value, time);

            for (const auto& info : infos)
            {
                Array<MidiMessage> m;
                sequence.createControllerUpdatesForTime (info.channel, time * 2, m);
                checkRpn  (std::next (m.begin(), 0),  std::next (m.begin(), 4),  info.channel, info.number, info.value, time);
            }
        }

        const auto messagesAreEqual = [] (const MidiMessage& a, const MidiMessage& b)
        {
            return std::equal (a.getRawData(), a.getRawData() + a.getRawDataSize(),
                               b.getRawData(), b.getRawData() + b.getRawDataSize());
        };

        beginTest ("createControllerUpdatesForTime sends bank select messages when the next program is in a new bank");
        {
            MidiMessageSequence sequence;

            const auto time = 0.0;
            const auto channel = 1;

            sequence.addEvent (MidiMessage::programChange (channel, 5), time);

            sequence.addEvent (MidiMessage::controllerEvent (channel, 0x00, 128), time);
            sequence.addEvent (MidiMessage::controllerEvent (channel, 0x20, 64), time);
            sequence.addEvent (MidiMessage::programChange (channel, 63), time);

            const Array<MidiMessage> finalEvents { MidiMessage::controllerEvent (channel, 0x00, 50),
                                                   MidiMessage::controllerEvent (channel, 0x20, 40),
                                                   MidiMessage::programChange (channel, 30) };

            for (const auto& e : finalEvents)
                sequence.addEvent (e);

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, 1.0, m);

            expect (std::equal (m.begin(), m.end(), finalEvents.begin(), finalEvents.end(), messagesAreEqual));
        }

        beginTest ("createControllerUpdatesForTime preserves all Data Increment and Data Decrement messages");
        {
            MidiMessageSequence sequence;

            const auto time = 0.0;
            const auto channel = 1;

            const Array<MidiMessage> messages { MidiMessage::controllerEvent (channel, 0x60, 0),
                                                MidiMessage::controllerEvent (channel, 0x06, 100),
                                                MidiMessage::controllerEvent (channel, 0x26, 50),
                                                MidiMessage::controllerEvent (channel, 0x60, 10),
                                                MidiMessage::controllerEvent (channel, 0x61, 10),
                                                MidiMessage::controllerEvent (channel, 0x06, 20),
                                                MidiMessage::controllerEvent (channel, 0x26, 30),
                                                MidiMessage::controllerEvent (channel, 0x61, 10),
                                                MidiMessage::controllerEvent (channel, 0x61, 20) };

            for (const auto& m : messages)
                sequence.addEvent (m, time);

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, 1.0, m);

            expect (std::equal (m.begin(), m.end(), messages.begin(), messages.end(), messagesAreEqual));
        }

        beginTest ("createControllerUpdatesForTime does not emit redundant parameter number changes");
        {
            MidiMessageSequence sequence;

            const auto time = 0.0;
            const auto channel = 1;

            const Array<MidiMessage> messages { MidiMessage::controllerEvent (channel, 0x65, 0),
                                                MidiMessage::controllerEvent (channel, 0x64, 100),
                                                MidiMessage::controllerEvent (channel, 0x63, 50),
                                                MidiMessage::controllerEvent (channel, 0x62, 10),
                                                MidiMessage::controllerEvent (channel, 0x06, 10) };

            for (const auto& m : messages)
                sequence.addEvent (m, time);

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, 1.0, m);

            const Array<MidiMessage> expected { MidiMessage::controllerEvent (channel, 0x63, 50),
                                                MidiMessage::controllerEvent (channel, 0x62, 10),
                                                MidiMessage::controllerEvent (channel, 0x06, 10) };

            expect (std::equal (m.begin(), m.end(), expected.begin(), expected.end(), messagesAreEqual));
        }

        beginTest ("createControllerUpdatesForTime sets parameter number correctly at end of sequence");
        {
            MidiMessageSequence sequence;

            const auto time = 0.0;
            const auto channel = 1;

            const Array<MidiMessage> messages { MidiMessage::controllerEvent (channel, 0x65, 0),
                                                MidiMessage::controllerEvent (channel, 0x64, 100),
                                                MidiMessage::controllerEvent (channel, 0x63, 50),
                                                MidiMessage::controllerEvent (channel, 0x62, 10),
                                                MidiMessage::controllerEvent (channel, 0x06, 10),
                                                MidiMessage::controllerEvent (channel, 0x64, 5) };

            for (const auto& m : messages)
                sequence.addEvent (m, time);

            const auto finalTime = 1.0;

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, finalTime, m);

            const Array<MidiMessage> expected { MidiMessage::controllerEvent (channel, 0x63, 50),
                                                MidiMessage::controllerEvent (channel, 0x62, 10),
                                                MidiMessage::controllerEvent (channel, 0x06, 10),
                                                // Note: we should send both the MSB and LSB!
                                                MidiMessage::controllerEvent (channel, 0x65, 0).withTimeStamp (finalTime),
                                                MidiMessage::controllerEvent (channel, 0x64, 5).withTimeStamp (finalTime) };

            expect (std::equal (m.begin(), m.end(), expected.begin(), expected.end(), messagesAreEqual));
        }

        beginTest ("createControllerUpdatesForTime does not emit duplicate parameter number change messages");
        {
            MidiMessageSequence sequence;

            const auto time = 0.0;
            const auto channel = 1;

            const Array<MidiMessage> messages { MidiMessage::controllerEvent (channel, 0x65, 1),
                                                MidiMessage::controllerEvent (channel, 0x64, 2),
                                                MidiMessage::controllerEvent (channel, 0x63, 3),
                                                MidiMessage::controllerEvent (channel, 0x62, 4),
                                                MidiMessage::controllerEvent (channel, 0x06, 10),
                                                MidiMessage::controllerEvent (channel, 0x63, 30),
                                                MidiMessage::controllerEvent (channel, 0x62, 40),
                                                MidiMessage::controllerEvent (channel, 0x63, 3),
                                                MidiMessage::controllerEvent (channel, 0x62, 4),
                                                MidiMessage::controllerEvent (channel, 0x60, 5),
                                                MidiMessage::controllerEvent (channel, 0x65, 10) };

            for (const auto& m : messages)
                sequence.addEvent (m, time);

            const auto finalTime = 1.0;

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, finalTime, m);

            const Array<MidiMessage> expected { MidiMessage::controllerEvent (channel, 0x63, 3),
                                                MidiMessage::controllerEvent (channel, 0x62, 4),
                                                MidiMessage::controllerEvent (channel, 0x06, 10),
                                                // Parameter number is set to (30, 40) then back to (3, 4),
                                                // so there is no need to resend it
                                                MidiMessage::controllerEvent (channel, 0x60, 5),
                                                // Set parameter number to final value
                                                MidiMessage::controllerEvent (channel, 0x65, 10).withTimeStamp (finalTime),
                                                MidiMessage::controllerEvent (channel, 0x64, 2) .withTimeStamp (finalTime) };

            expect (std::equal (m.begin(), m.end(), expected.begin(), expected.end(), messagesAreEqual));
        }

        beginTest ("createControllerUpdatesForTime emits bank change messages immediately before program change");
        {
            MidiMessageSequence sequence;

            const auto time = 0.0;
            const auto channel = 1;

            const Array<MidiMessage> messages { MidiMessage::controllerEvent (channel, 0x00, 1),
                                                MidiMessage::controllerEvent (channel, 0x20, 2),
                                                MidiMessage::controllerEvent (channel, 0x65, 0),
                                                MidiMessage::controllerEvent (channel, 0x64, 0),
                                                MidiMessage::programChange (channel, 5) };

            for (const auto& m : messages)
                sequence.addEvent (m, time);

            const auto finalTime = 1.0;

            Array<MidiMessage> m;
            sequence.createControllerUpdatesForTime (channel, finalTime, m);

            const Array<MidiMessage> expected { MidiMessage::controllerEvent (channel, 0x00, 1),
                                                MidiMessage::controllerEvent (channel, 0x20, 2),
                                                MidiMessage::programChange (channel, 5),
                                                MidiMessage::controllerEvent (channel, 0x65, 0).withTimeStamp (finalTime),
                                                MidiMessage::controllerEvent (channel, 0x64, 0).withTimeStamp (finalTime) };


            expect (std::equal (m.begin(), m.end(), expected.begin(), expected.end(), messagesAreEqual));
        }
    }
};

static MidiMessageSequenceTest midiMessageSequenceTests;

#endif

} // namespace drx
