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

namespace MidiHelpers
{
    inline u8 initialByte (i32k type, i32k channel) noexcept
    {
        return (u8) (type | jlimit (0, 15, channel - 1));
    }

    inline u8 validVelocity (i32k v) noexcept
    {
        return (u8) jlimit (0, 127, v);
    }
}

//==============================================================================
u8 MidiMessage::floatValueToMidiByte (const f32 v) noexcept
{
    jassert (v >= 0 && v <= 1.0f);  // if your value is > 1, maybe you're passing an
                                    // integer value to a f32 method by mistake?

    return MidiHelpers::validVelocity (roundToInt (v * 127.0f));
}

u16 MidiMessage::pitchbendToPitchwheelPos (const f32 pitchbend,
                                 const f32 pitchbendRange) noexcept
{
    // can't translate a pitchbend value that is outside of the given range!
    jassert (std::abs (pitchbend) <= pitchbendRange);

    return static_cast<u16> (pitchbend > 0.0f
                                  ? jmap (pitchbend, 0.0f, pitchbendRange, 8192.0f, 16383.0f)
                                  : jmap (pitchbend, -pitchbendRange, 0.0f, 0.0f, 8192.0f));
}

//==============================================================================
MidiMessage::VariableLengthValue MidiMessage::readVariableLengthValue (u8k* data, i32 maxBytesToUse) noexcept
{
    u32 v = 0;

    // The largest allowable variable-length value is 0x0f'ff'ff'ff which is
    // represented by the 4-byte stream 0xff 0xff 0xff 0x7f.
    // Longer bytestreams risk overflowing a 32-bit i32.
    const auto limit = jmin (maxBytesToUse, 4);

    for (i32 numBytesUsed = 0; numBytesUsed < limit; ++numBytesUsed)
    {
        const auto i = data[numBytesUsed];
        v = (v << 7) + (i & 0x7f);

        if (! (i & 0x80))
            return { (i32) v, numBytesUsed + 1 };
    }

    // If this is hit, the input was malformed. Either there were not enough
    // bytes of input to construct a full value, or no terminating byte was
    // found. This implementation only supports variable-length values of up
    // to four bytes.
    return {};
}

i32 MidiMessage::readVariableLengthVal (u8k* data, i32& numBytesUsed) noexcept
{
    numBytesUsed = 0;
    i32 v = 0, i;

    do
    {
        i = (i32) *data++;

        if (++numBytesUsed > 6)
            break;

        v = (v << 7) + (i & 0x7f);

    } while (i & 0x80);

    return v;
}

i32 MidiMessage::getMessageLengthFromFirstByte (u8k firstByte) noexcept
{
    // this method only works for valid starting bytes of a short midi message
    jassert (firstByte >= 0x80 && firstByte != 0xf0 && firstByte != 0xf7);

    static const t8 messageLengths[] =
    {
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        1, 2, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    return messageLengths[firstByte & 0x7f];
}

//==============================================================================
MidiMessage::MidiMessage() noexcept
   : size (2)
{
    packedData.asBytes[0] = 0xf0;
    packedData.asBytes[1] = 0xf7;
}

MidiMessage::MidiMessage (ukk const d, i32k dataSize, const f64 t)
   : timeStamp (t), size (dataSize)
{
    jassert (dataSize > 0);
    // this checks that the length matches the data..
    jassert (dataSize > 3 || *(u8*)d >= 0xf0 || getMessageLengthFromFirstByte (*(u8*)d) == size);

    memcpy (allocateSpace (dataSize), d, (size_t) dataSize);
}

MidiMessage::MidiMessage (i32k byte1, const f64 t) noexcept
   : timeStamp (t), size (1)
{
    packedData.asBytes[0] = (u8) byte1;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((u8) byte1) == 1);
}

MidiMessage::MidiMessage (i32k byte1, i32k byte2, const f64 t) noexcept
   : timeStamp (t), size (2)
{
    packedData.asBytes[0] = (u8) byte1;
    packedData.asBytes[1] = (u8) byte2;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((u8) byte1) == 2);
}

MidiMessage::MidiMessage (i32k byte1, i32k byte2, i32k byte3, const f64 t) noexcept
   : timeStamp (t), size (3)
{
    packedData.asBytes[0] = (u8) byte1;
    packedData.asBytes[1] = (u8) byte2;
    packedData.asBytes[2] = (u8) byte3;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((u8) byte1) == 3);
}

MidiMessage::MidiMessage (const MidiMessage& other)
   : timeStamp (other.timeStamp), size (other.size)
{
    if (isHeapAllocated())
        memcpy (allocateSpace (size), other.getData(), (size_t) size);
    else
        packedData.allocatedData = other.packedData.allocatedData;
}

MidiMessage::MidiMessage (const MidiMessage& other, const f64 newTimeStamp)
   : timeStamp (newTimeStamp), size (other.size)
{
    if (isHeapAllocated())
        memcpy (allocateSpace (size), other.getData(), (size_t) size);
    else
        packedData.allocatedData = other.packedData.allocatedData;
}

MidiMessage::MidiMessage (ukk srcData, i32 sz, i32& numBytesUsed, u8k lastStatusByte,
                          f64 t, b8 sysexHasEmbeddedLength)
    : timeStamp (t)
{
    auto src = static_cast<u8k*> (srcData);
    auto byte = (u32) *src;

    if (byte < 0x80)
    {
        byte = (u32) lastStatusByte;
        numBytesUsed = -1;
    }
    else
    {
        numBytesUsed = 0;
        --sz;
        ++src;
    }

    if (byte >= 0x80)
    {
        if (byte == 0xf0)
        {
            auto d = src;
            b8 haveReadAllLengthBytes = ! sysexHasEmbeddedLength;
            i32 numVariableLengthSysexBytes = 0;

            while (d < src + sz)
            {
                if (*d >= 0x80)
                {
                    if (*d == 0xf7)
                    {
                        ++d;  // include the trailing 0xf7 when we hit it
                        break;
                    }

                    if (haveReadAllLengthBytes) // if we see a 0x80 bit set after the initial data length
                        break;                  // bytes, assume it's the end of the sysex

                    ++numVariableLengthSysexBytes;
                }
                else if (! haveReadAllLengthBytes)
                {
                    haveReadAllLengthBytes = true;
                    ++numVariableLengthSysexBytes;
                }

                ++d;
            }

            src += numVariableLengthSysexBytes;
            size = 1 + (i32) (d - src);

            auto dest = allocateSpace (size);
            *dest = (u8) byte;
            memcpy (dest + 1, src, (size_t) (size - 1));

            numBytesUsed += (numVariableLengthSysexBytes + size);  // (these aren't counted in the size)
        }
        else if (byte == 0xff)
        {
            const auto bytesLeft = readVariableLengthValue (src + 1, sz - 1);
            size = jmin (sz + 1, bytesLeft.bytesUsed + 2 + bytesLeft.value);

            auto dest = allocateSpace (size);
            *dest = (u8) byte;
            memcpy (dest + 1, src, (size_t) size - 1);

            numBytesUsed += size;
        }
        else
        {
            size = getMessageLengthFromFirstByte ((u8) byte);
            packedData.asBytes[0] = (u8) byte;

            if (size > 1)
            {
                packedData.asBytes[1] = (sz > 0 ? src[0] : 0);

                if (size > 2)
                    packedData.asBytes[2] = (sz > 1 ? src[1] : 0);
            }

            numBytesUsed += jmin (size, sz + 1);
        }
    }
    else
    {
        packedData.allocatedData = nullptr;
        size = 0;
    }
}

MidiMessage& MidiMessage::operator= (const MidiMessage& other)
{
    if (this != &other)
    {
        if (other.isHeapAllocated())
        {
            auto* newStorage = static_cast<u8*> (isHeapAllocated()
              ? std::realloc (packedData.allocatedData, (size_t) other.size)
              : std::malloc ((size_t) other.size));

            if (newStorage == nullptr)
                throw std::bad_alloc{}; // The midi message has not been adjusted at this point

            packedData.allocatedData = newStorage;
            memcpy (packedData.allocatedData, other.packedData.allocatedData, (size_t) other.size);
        }
        else
        {
            if (isHeapAllocated())
                std::free (packedData.allocatedData);

            packedData.allocatedData = other.packedData.allocatedData;
        }

        timeStamp = other.timeStamp;
        size = other.size;
    }

    return *this;
}

MidiMessage::MidiMessage (MidiMessage&& other) noexcept
   : timeStamp (other.timeStamp), size (other.size)
{
    packedData.allocatedData = other.packedData.allocatedData;
    other.size = 0;
}

MidiMessage& MidiMessage::operator= (MidiMessage&& other) noexcept
{
    packedData.allocatedData = other.packedData.allocatedData;
    timeStamp = other.timeStamp;
    size = other.size;
    other.size = 0;
    return *this;
}

MidiMessage::~MidiMessage() noexcept
{
    if (isHeapAllocated())
        std::free (packedData.allocatedData);
}

u8* MidiMessage::allocateSpace (i32 bytes)
{
    if (bytes > (i32) sizeof (packedData))
    {
        auto d = static_cast<u8*> (std::malloc ((size_t) bytes));
        packedData.allocatedData = d;
        return d;
    }

    return packedData.asBytes;
}

Txt MidiMessage::getDescription() const
{
    if (isNoteOn())           return "Note on "  + MidiMessage::getMidiNoteName (getNoteNumber(), true, true, 3) + " Velocity " + Txt (getVelocity()) + " Channel " + Txt (getChannel());
    if (isNoteOff())          return "Note off " + MidiMessage::getMidiNoteName (getNoteNumber(), true, true, 3) + " Velocity " + Txt (getVelocity()) + " Channel " + Txt (getChannel());
    if (isProgramChange())    return "Program change " + Txt (getProgramChangeNumber()) + " Channel " + Txt (getChannel());
    if (isPitchWheel())       return "Pitch wheel " + Txt (getPitchWheelValue()) + " Channel " + Txt (getChannel());
    if (isAftertouch())       return "Aftertouch " + MidiMessage::getMidiNoteName (getNoteNumber(), true, true, 3) +  ": " + Txt (getAfterTouchValue()) + " Channel " + Txt (getChannel());
    if (isChannelPressure())  return "Channel pressure " + Txt (getChannelPressureValue()) + " Channel " + Txt (getChannel());
    if (isAllNotesOff())      return "All notes off Channel " + Txt (getChannel());
    if (isAllSoundOff())      return "All sound off Channel " + Txt (getChannel());
    if (isMetaEvent())        return "Meta event";

    if (isController())
    {
        Txt name (MidiMessage::getControllerName (getControllerNumber()));

        if (name.isEmpty())
            name = Txt (getControllerNumber());

        return "Controller " + name + ": " + Txt (getControllerValue()) + " Channel " + Txt (getChannel());
    }

    return Txt::toHexString (getRawData(), getRawDataSize());
}

MidiMessage MidiMessage::withTimeStamp (f64 newTimestamp) const
{
    return { *this, newTimestamp };
}

i32 MidiMessage::getChannel() const noexcept
{
    auto data = getRawData();

    if ((data[0] & 0xf0) != 0xf0)
        return (data[0] & 0xf) + 1;

    return 0;
}

b8 MidiMessage::isForChannel (i32k channel) const noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    auto data = getRawData();

    return ((data[0] & 0xf) == channel - 1)
             && ((data[0] & 0xf0) != 0xf0);
}

z0 MidiMessage::setChannel (i32k channel) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    auto data = getData();

    if ((data[0] & 0xf0) != (u8) 0xf0)
        data[0] = (u8) ((data[0] & (u8) 0xf0)
                            | (u8) (channel - 1));
}

b8 MidiMessage::isNoteOn (const b8 returnTrueForVelocity0) const noexcept
{
    auto data = getRawData();

    return ((data[0] & 0xf0) == 0x90)
             && (returnTrueForVelocity0 || data[2] != 0);
}

b8 MidiMessage::isNoteOff (const b8 returnTrueForNoteOnVelocity0) const noexcept
{
    auto data = getRawData();

    return ((data[0] & 0xf0) == 0x80)
            || (returnTrueForNoteOnVelocity0 && (data[2] == 0) && ((data[0] & 0xf0) == 0x90));
}

b8 MidiMessage::isNoteOnOrOff() const noexcept
{
    auto d = getRawData()[0] & 0xf0;
    return (d == 0x90) || (d == 0x80);
}

i32 MidiMessage::getNoteNumber() const noexcept
{
    return getRawData()[1];
}

z0 MidiMessage::setNoteNumber (i32k newNoteNumber) noexcept
{
    if (isNoteOnOrOff() || isAftertouch())
        getData()[1] = (u8) (newNoteNumber & 127);
}

u8 MidiMessage::getVelocity() const noexcept
{
    if (isNoteOnOrOff())
        return getRawData()[2];

    return 0;
}

f32 MidiMessage::getFloatVelocity() const noexcept
{
    return getVelocity() * (1.0f / 127.0f);
}

z0 MidiMessage::setVelocity (const f32 newVelocity) noexcept
{
    if (isNoteOnOrOff())
        getData()[2] = floatValueToMidiByte (newVelocity);
}

z0 MidiMessage::multiplyVelocity (const f32 scaleFactor) noexcept
{
    if (isNoteOnOrOff())
    {
        auto data = getData();
        data[2] = MidiHelpers::validVelocity (roundToInt (scaleFactor * data[2]));
    }
}

b8 MidiMessage::isAftertouch() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xa0;
}

i32 MidiMessage::getAfterTouchValue() const noexcept
{
    jassert (isAftertouch());
    return getRawData()[2];
}

MidiMessage MidiMessage::aftertouchChange (i32k channel,
                                           i32k noteNum,
                                           i32k aftertouchValue) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (isPositiveAndBelow (noteNum, 128));
    jassert (isPositiveAndBelow (aftertouchValue, 128));

    return MidiMessage (MidiHelpers::initialByte (0xa0, channel),
                        noteNum & 0x7f,
                        aftertouchValue & 0x7f);
}

b8 MidiMessage::isChannelPressure() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xd0;
}

i32 MidiMessage::getChannelPressureValue() const noexcept
{
    jassert (isChannelPressure());
    return getRawData()[1];
}

MidiMessage MidiMessage::channelPressureChange (i32k channel, i32k pressure) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (isPositiveAndBelow (pressure, 128));

    return MidiMessage (MidiHelpers::initialByte (0xd0, channel), pressure & 0x7f);
}

b8 MidiMessage::isSustainPedalOn() const noexcept     { return isControllerOfType (0x40) && getRawData()[2] >= 64; }
b8 MidiMessage::isSustainPedalOff() const noexcept    { return isControllerOfType (0x40) && getRawData()[2] <  64; }

b8 MidiMessage::isSostenutoPedalOn() const noexcept   { return isControllerOfType (0x42) && getRawData()[2] >= 64; }
b8 MidiMessage::isSostenutoPedalOff() const noexcept  { return isControllerOfType (0x42) && getRawData()[2] <  64; }

b8 MidiMessage::isSoftPedalOn() const noexcept        { return isControllerOfType (0x43) && getRawData()[2] >= 64; }
b8 MidiMessage::isSoftPedalOff() const noexcept       { return isControllerOfType (0x43) && getRawData()[2] <  64; }


b8 MidiMessage::isProgramChange() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xc0;
}

i32 MidiMessage::getProgramChangeNumber() const noexcept
{
    jassert (isProgramChange());
    return getRawData()[1];
}

MidiMessage MidiMessage::programChange (i32k channel, i32k programNumber) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    return MidiMessage (MidiHelpers::initialByte (0xc0, channel), programNumber & 0x7f);
}

b8 MidiMessage::isPitchWheel() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xe0;
}

i32 MidiMessage::getPitchWheelValue() const noexcept
{
    jassert (isPitchWheel());
    auto data = getRawData();
    return data[1] | (data[2] << 7);
}

MidiMessage MidiMessage::pitchWheel (i32k channel, i32k position) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (isPositiveAndBelow (position, 0x4000));

    return MidiMessage (MidiHelpers::initialByte (0xe0, channel),
                        position & 127, (position >> 7) & 127);
}

b8 MidiMessage::isController() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xb0;
}

b8 MidiMessage::isControllerOfType (i32k controllerType) const noexcept
{
    auto data = getRawData();
    return (data[0] & 0xf0) == 0xb0 && data[1] == controllerType;
}

i32 MidiMessage::getControllerNumber() const noexcept
{
    jassert (isController());
    return getRawData()[1];
}

i32 MidiMessage::getControllerValue() const noexcept
{
    jassert (isController());
    return getRawData()[2];
}

MidiMessage MidiMessage::controllerEvent (i32k channel, i32k controllerType, i32k value) noexcept
{
    // the channel must be between 1 and 16 inclusive
    jassert (channel > 0 && channel <= 16);

    return MidiMessage (MidiHelpers::initialByte (0xb0, channel),
                        controllerType & 127, value & 127);
}

MidiMessage MidiMessage::noteOn (i32k channel, i32k noteNumber, u8k velocity) noexcept
{
    jassert (channel > 0 && channel <= 16);
    jassert (isPositiveAndBelow (noteNumber, 128));

    return MidiMessage (MidiHelpers::initialByte (0x90, channel),
                        noteNumber & 127, MidiHelpers::validVelocity (velocity));
}

MidiMessage MidiMessage::noteOn (i32k channel, i32k noteNumber, const f32 velocity) noexcept
{
    return noteOn (channel, noteNumber, floatValueToMidiByte (velocity));
}

MidiMessage MidiMessage::noteOff (i32k channel, i32k noteNumber, u8 velocity) noexcept
{
    jassert (channel > 0 && channel <= 16);
    jassert (isPositiveAndBelow (noteNumber, 128));

    return MidiMessage (MidiHelpers::initialByte (0x80, channel),
                        noteNumber & 127, MidiHelpers::validVelocity (velocity));
}

MidiMessage MidiMessage::noteOff (i32k channel, i32k noteNumber, f32 velocity) noexcept
{
    return noteOff (channel, noteNumber, floatValueToMidiByte (velocity));
}

MidiMessage MidiMessage::noteOff (i32k channel, i32k noteNumber) noexcept
{
    jassert (channel > 0 && channel <= 16);
    jassert (isPositiveAndBelow (noteNumber, 128));

    return MidiMessage (MidiHelpers::initialByte (0x80, channel), noteNumber & 127, 0);
}

MidiMessage MidiMessage::allNotesOff (i32k channel) noexcept
{
    return controllerEvent (channel, 123, 0);
}

b8 MidiMessage::isAllNotesOff() const noexcept
{
    auto data = getRawData();
    return (data[0] & 0xf0) == 0xb0 && data[1] == 123;
}

MidiMessage MidiMessage::allSoundOff (i32k channel) noexcept
{
    return controllerEvent (channel, 120, 0);
}

b8 MidiMessage::isAllSoundOff() const noexcept
{
    auto data = getRawData();
    return data[1] == 120 && (data[0] & 0xf0) == 0xb0;
}

b8 MidiMessage::isResetAllControllers() const noexcept
{
    auto data = getRawData();
    return (data[0] & 0xf0) == 0xb0 && data[1] == 121;
}

MidiMessage MidiMessage::allControllersOff (i32k channel) noexcept
{
    return controllerEvent (channel, 121, 0);
}

MidiMessage MidiMessage::masterVolume (const f32 volume)
{
    auto vol = jlimit (0, 0x3fff, roundToInt (volume * 0x4000));

    return { 0xf0, 0x7f, 0x7f, 0x04, 0x01, vol & 0x7f, vol >> 7, 0xf7 };
}

//==============================================================================
b8 MidiMessage::isSysEx() const noexcept
{
    return *getRawData() == 0xf0;
}

MidiMessage MidiMessage::createSysExMessage (ukk sysexData, i32k dataSize)
{
    HeapBlock<u8> m (dataSize + 2);

    m[0] = 0xf0;
    memcpy (m + 1, sysexData, (size_t) dataSize);
    m[dataSize + 1] = 0xf7;

    return MidiMessage (m, dataSize + 2);
}

MidiMessage MidiMessage::createSysExMessage (Span<const std::byte> data)
{
    return createSysExMessage (data.data(), (i32) data.size());
}

u8k* MidiMessage::getSysExData() const noexcept
{
    return isSysEx() ? getRawData() + 1 : nullptr;
}

i32 MidiMessage::getSysExDataSize() const noexcept
{
    return isSysEx() ? size - 2 : 0;
}

//==============================================================================
b8 MidiMessage::isMetaEvent() const noexcept      { return *getRawData() == 0xff; }
b8 MidiMessage::isActiveSense() const noexcept    { return *getRawData() == 0xfe; }

i32 MidiMessage::getMetaEventType() const noexcept
{
    auto data = getRawData();
    return (size < 2 || *data != 0xff) ? -1 : data[1];
}

i32 MidiMessage::getMetaEventLength() const noexcept
{
    auto data = getRawData();

    if (*data == 0xff)
    {
        const auto var = readVariableLengthValue (data + 2, size - 2);
        return jmax (0, jmin (size - 2 - var.bytesUsed, var.value));
    }

    return 0;
}

u8k* MidiMessage::getMetaEventData() const noexcept
{
    jassert (isMetaEvent());

    auto d = getRawData() + 2;
    const auto var = readVariableLengthValue (d, size - 2);
    return d + var.bytesUsed;
}

b8 MidiMessage::isTrackMetaEvent() const noexcept         { return getMetaEventType() == 0; }
b8 MidiMessage::isEndOfTrackMetaEvent() const noexcept    { return getMetaEventType() == 47; }

b8 MidiMessage::isTextMetaEvent() const noexcept
{
    auto t = getMetaEventType();
    return t > 0 && t < 16;
}

Txt MidiMessage::getTextFromTextMetaEvent() const
{
    auto textData = reinterpret_cast<tukk> (getMetaEventData());

    return Txt (CharPointer_UTF8 (textData),
                   CharPointer_UTF8 (textData + getMetaEventLength()));
}

MidiMessage MidiMessage::textMetaEvent (i32 type, StringRef text)
{
    jassert (type > 0 && type < 16);

    MidiMessage result;

    const size_t textSize = text.text.sizeInBytes() - 1;

    u8 header[8];
    size_t n = sizeof (header);

    header[--n] = (u8) (textSize & 0x7f);

    for (size_t i = textSize; (i >>= 7) != 0;)
        header[--n] = (u8) ((i & 0x7f) | 0x80);

    header[--n] = (u8) type;
    header[--n] = 0xff;

    const size_t headerLen = sizeof (header) - n;
    i32k totalSize = (i32) (headerLen + textSize);

    auto dest = result.allocateSpace (totalSize);
    result.size = totalSize;

    memcpy (dest, header + n, headerLen);
    memcpy (dest + headerLen, text.text.getAddress(), textSize);

    return result;
}

b8 MidiMessage::isTrackNameEvent() const noexcept         { auto data = getRawData(); return (data[1] == 3)    && (*data == 0xff); }
b8 MidiMessage::isTempoMetaEvent() const noexcept         { auto data = getRawData(); return (data[1] == 81)   && (*data == 0xff); }
b8 MidiMessage::isMidiChannelMetaEvent() const noexcept   { auto data = getRawData(); return (data[1] == 0x20) && (*data == 0xff) && (data[2] == 1); }

i32 MidiMessage::getMidiChannelMetaEventChannel() const noexcept
{
    jassert (isMidiChannelMetaEvent());
    return getRawData()[3] + 1;
}

f64 MidiMessage::getTempoSecondsPerQuarterNote() const noexcept
{
    if (! isTempoMetaEvent())
        return 0.0;

    auto d = getMetaEventData();

    return (((u32) d[0] << 16)
             | ((u32) d[1] << 8)
             | d[2])
            / 1000000.0;
}

f64 MidiMessage::getTempoMetaEventTickLength (const short timeFormat) const noexcept
{
    if (timeFormat > 0)
    {
        if (! isTempoMetaEvent())
            return 0.5 / timeFormat;

        return getTempoSecondsPerQuarterNote() / timeFormat;
    }

    i32k frameCode = (-timeFormat) >> 8;
    f64 framesPerSecond;

    switch (frameCode)
    {
        case 24: framesPerSecond = 24.0;   break;
        case 25: framesPerSecond = 25.0;   break;
        case 29: framesPerSecond = 30.0 * 1000.0 / 1001.0;  break;
        case 30: framesPerSecond = 30.0;   break;
        default: framesPerSecond = 30.0;   break;
    }

    return (1.0 / framesPerSecond) / (timeFormat & 0xff);
}

MidiMessage MidiMessage::tempoMetaEvent (i32 microsecondsPerQuarterNote) noexcept
{
    return { 0xff, 81, 3,
             (u8) (microsecondsPerQuarterNote >> 16),
             (u8) (microsecondsPerQuarterNote >> 8),
             (u8) microsecondsPerQuarterNote };
}

b8 MidiMessage::isTimeSignatureMetaEvent() const noexcept
{
    auto data = getRawData();
    return (data[1] == 0x58) && (*data == (u8) 0xff);
}

z0 MidiMessage::getTimeSignatureInfo (i32& numerator, i32& denominator) const noexcept
{
    if (isTimeSignatureMetaEvent())
    {
        auto d = getMetaEventData();
        numerator = d[0];
        denominator = 1 << d[1];
    }
    else
    {
        numerator = 4;
        denominator = 4;
    }
}

MidiMessage MidiMessage::timeSignatureMetaEvent (i32k numerator, i32k denominator)
{
    i32 n = 1;
    i32 powerOfTwo = 0;

    while (n < denominator)
    {
        n <<= 1;
        ++powerOfTwo;
    }

    return { 0xff, 0x58, 0x04, numerator, powerOfTwo, 1, 96 };
}

MidiMessage MidiMessage::midiChannelMetaEvent (i32k channel) noexcept
{
    return { 0xff, 0x20, 0x01, jlimit (0, 0xff, channel - 1) };
}

b8 MidiMessage::isKeySignatureMetaEvent() const noexcept
{
    return getMetaEventType() == 0x59;
}

i32 MidiMessage::getKeySignatureNumberOfSharpsOrFlats() const noexcept
{
    return (i32) (i8) getMetaEventData()[0];
}

b8 MidiMessage::isKeySignatureMajorKey() const noexcept
{
    return getMetaEventData()[1] == 0;
}

MidiMessage MidiMessage::keySignatureMetaEvent (i32 numberOfSharpsOrFlats, b8 isMinorKey)
{
    jassert (numberOfSharpsOrFlats >= -7 && numberOfSharpsOrFlats <= 7);

    return { 0xff, 0x59, 0x02, numberOfSharpsOrFlats, isMinorKey ? 1 : 0 };
}

MidiMessage MidiMessage::endOfTrack() noexcept
{
    return { 0xff, 0x2f, 0x00 };
}

//==============================================================================
b8 MidiMessage::isSongPositionPointer() const noexcept         { return *getRawData() == 0xf2; }
i32 MidiMessage::getSongPositionPointerMidiBeat() const noexcept { auto data = getRawData(); return data[1] | (data[2] << 7); }

MidiMessage MidiMessage::songPositionPointer (i32k positionInMidiBeats) noexcept
{
    return { 0xf2,
             positionInMidiBeats & 127,
             (positionInMidiBeats >> 7) & 127 };
}

b8 MidiMessage::isMidiStart() const noexcept                  { return *getRawData() == 0xfa; }
MidiMessage MidiMessage::midiStart() noexcept                   { return MidiMessage (0xfa); }

b8 MidiMessage::isMidiContinue() const noexcept               { return *getRawData() == 0xfb; }
MidiMessage MidiMessage::midiContinue() noexcept                { return MidiMessage (0xfb); }

b8 MidiMessage::isMidiStop() const noexcept                   { return *getRawData() == 0xfc; }
MidiMessage MidiMessage::midiStop() noexcept                    { return MidiMessage (0xfc); }

b8 MidiMessage::isMidiClock() const noexcept                  { return *getRawData() == 0xf8; }
MidiMessage MidiMessage::midiClock() noexcept                   { return MidiMessage (0xf8); }

b8 MidiMessage::isQuarterFrame() const noexcept               { return *getRawData() == 0xf1; }
i32 MidiMessage::getQuarterFrameSequenceNumber() const noexcept { return ((i32) getRawData()[1]) >> 4; }
i32 MidiMessage::getQuarterFrameValue() const noexcept          { return ((i32) getRawData()[1]) & 0x0f; }

MidiMessage MidiMessage::quarterFrame (i32k sequenceNumber, i32k value) noexcept
{
    return MidiMessage (0xf1, (sequenceNumber << 4) | value);
}

b8 MidiMessage::isFullFrame() const noexcept
{
    auto data = getRawData();

    return data[0] == 0xf0
            && data[1] == 0x7f
            && size >= 10
            && data[3] == 0x01
            && data[4] == 0x01;
}

z0 MidiMessage::getFullFrameParameters (i32& hours, i32& minutes, i32& seconds, i32& frames,
                                          MidiMessage::SmpteTimecodeType& timecodeType) const noexcept
{
    jassert (isFullFrame());

    auto data = getRawData();
    timecodeType = (SmpteTimecodeType) (data[5] >> 5);
    hours   = data[5] & 0x1f;
    minutes = data[6];
    seconds = data[7];
    frames  = data[8];
}

MidiMessage MidiMessage::fullFrame (i32 hours, i32 minutes, i32 seconds, i32 frames,
                                    MidiMessage::SmpteTimecodeType timecodeType)
{
    return { 0xf0, 0x7f, 0x7f, 0x01, 0x01,
             (hours & 0x01f) | (timecodeType << 5),
             minutes, seconds, frames,
             0xf7 };
}

b8 MidiMessage::isMidiMachineControlMessage() const noexcept
{
    auto data = getRawData();

    return data[0] == 0xf0
        && data[1] == 0x7f
        && data[3] == 0x06
        && size > 5;
}

MidiMessage::MidiMachineControlCommand MidiMessage::getMidiMachineControlCommand() const noexcept
{
    jassert (isMidiMachineControlMessage());

    return (MidiMachineControlCommand) getRawData()[4];
}

MidiMessage MidiMessage::midiMachineControlCommand (MidiMessage::MidiMachineControlCommand command)
{
    return { 0xf0, 0x7f, 0, 6, command, 0xf7 };
}

//==============================================================================
b8 MidiMessage::isMidiMachineControlGoto (i32& hours, i32& minutes, i32& seconds, i32& frames) const noexcept
{
    auto data = getRawData();

    if (size >= 12
         && data[0] == 0xf0
         && data[1] == 0x7f
         && data[3] == 0x06
         && data[4] == 0x44
         && data[5] == 0x06
         && data[6] == 0x01)
    {
        hours = data[7] % 24;   // (that some machines send out hours > 24)
        minutes = data[8];
        seconds = data[9];
        frames  = data[10];

        return true;
    }

    return false;
}

MidiMessage MidiMessage::midiMachineControlGoto (i32 hours, i32 minutes, i32 seconds, i32 frames)
{
    return { 0xf0, 0x7f, 0, 6, 0x44, 6, 1, hours, minutes, seconds, frames, 0xf7 };
}

//==============================================================================
Txt MidiMessage::getMidiNoteName (i32 note, b8 useSharps, b8 includeOctaveNumber, i32 octaveNumForMiddleC)
{
    static tukk const sharpNoteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    static tukk const flatNoteNames[]  = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

    if (isPositiveAndBelow (note, 128))
    {
        Txt s (useSharps ? sharpNoteNames[note % 12]
                            : flatNoteNames [note % 12]);

        if (includeOctaveNumber)
            s << (note / 12 + (octaveNumForMiddleC - 5));

        return s;
    }

    return {};
}

f64 MidiMessage::getMidiNoteInHertz (i32k noteNumber, const f64 frequencyOfA) noexcept
{
    return frequencyOfA * std::pow (2.0, (noteNumber - 69) / 12.0);
}

b8 MidiMessage::isMidiNoteBlack (i32 noteNumber) noexcept
{
    return ((1 << (noteNumber % 12)) & 0x054a) != 0;
}

tukk MidiMessage::getGMInstrumentName (i32k n)
{
    static tukk names[] =
    {
        NEEDS_TRANS ("Acoustic Grand Piano"),    NEEDS_TRANS ("Bright Acoustic Piano"),   NEEDS_TRANS ("Electric Grand Piano"),    NEEDS_TRANS ("Honky-tonk Piano"),
        NEEDS_TRANS ("Electric Piano 1"),        NEEDS_TRANS ("Electric Piano 2"),        NEEDS_TRANS ("Harpsichord"),             NEEDS_TRANS ("Clavinet"),
        NEEDS_TRANS ("Celesta"),                 NEEDS_TRANS ("Glockenspiel"),            NEEDS_TRANS ("Music Box"),               NEEDS_TRANS ("Vibraphone"),
        NEEDS_TRANS ("Marimba"),                 NEEDS_TRANS ("Xylophone"),               NEEDS_TRANS ("Tubular Bells"),           NEEDS_TRANS ("Dulcimer"),
        NEEDS_TRANS ("Drawbar Organ"),           NEEDS_TRANS ("Percussive Organ"),        NEEDS_TRANS ("Rock Organ"),              NEEDS_TRANS ("Church Organ"),
        NEEDS_TRANS ("Reed Organ"),              NEEDS_TRANS ("Accordion"),               NEEDS_TRANS ("Harmonica"),               NEEDS_TRANS ("Tango Accordion"),
        NEEDS_TRANS ("Acoustic Guitar (nylon)"), NEEDS_TRANS ("Acoustic Guitar (steel)"), NEEDS_TRANS ("Electric Guitar (jazz)"),  NEEDS_TRANS ("Electric Guitar (clean)"),
        NEEDS_TRANS ("Electric Guitar (mute)"),  NEEDS_TRANS ("Overdriven Guitar"),       NEEDS_TRANS ("Distortion Guitar"),       NEEDS_TRANS ("Guitar Harmonics"),
        NEEDS_TRANS ("Acoustic Bass"),           NEEDS_TRANS ("Electric Bass (finger)"),  NEEDS_TRANS ("Electric Bass (pick)"),    NEEDS_TRANS ("Fretless Bass"),
        NEEDS_TRANS ("Slap Bass 1"),             NEEDS_TRANS ("Slap Bass 2"),             NEEDS_TRANS ("Synth Bass 1"),            NEEDS_TRANS ("Synth Bass 2"),
        NEEDS_TRANS ("Violin"),                  NEEDS_TRANS ("Viola"),                   NEEDS_TRANS ("Cello"),                   NEEDS_TRANS ("Contrabass"),
        NEEDS_TRANS ("Tremolo Strings"),         NEEDS_TRANS ("Pizzicato Strings"),       NEEDS_TRANS ("Orchestral Harp"),         NEEDS_TRANS ("Timpani"),
        NEEDS_TRANS ("Txt Ensemble 1"),       NEEDS_TRANS ("Txt Ensemble 2"),       NEEDS_TRANS ("SynthStrings 1"),          NEEDS_TRANS ("SynthStrings 2"),
        NEEDS_TRANS ("Choir Aahs"),              NEEDS_TRANS ("Voice Oohs"),              NEEDS_TRANS ("Synth Voice"),             NEEDS_TRANS ("Orchestra Hit"),
        NEEDS_TRANS ("Trumpet"),                 NEEDS_TRANS ("Trombone"),                NEEDS_TRANS ("Tuba"),                    NEEDS_TRANS ("Muted Trumpet"),
        NEEDS_TRANS ("French Horn"),             NEEDS_TRANS ("Brass Section"),           NEEDS_TRANS ("SynthBrass 1"),            NEEDS_TRANS ("SynthBrass 2"),
        NEEDS_TRANS ("Soprano Sax"),             NEEDS_TRANS ("Alto Sax"),                NEEDS_TRANS ("Tenor Sax"),               NEEDS_TRANS ("Baritone Sax"),
        NEEDS_TRANS ("Oboe"),                    NEEDS_TRANS ("English Horn"),            NEEDS_TRANS ("Bassoon"),                 NEEDS_TRANS ("Clarinet"),
        NEEDS_TRANS ("Piccolo"),                 NEEDS_TRANS ("Flute"),                   NEEDS_TRANS ("Recorder"),                NEEDS_TRANS ("Pan Flute"),
        NEEDS_TRANS ("Blown Bottle"),            NEEDS_TRANS ("Shakuhachi"),              NEEDS_TRANS ("Whistle"),                 NEEDS_TRANS ("Ocarina"),
        NEEDS_TRANS ("Lead 1 (square)"),         NEEDS_TRANS ("Lead 2 (sawtooth)"),       NEEDS_TRANS ("Lead 3 (calliope)"),       NEEDS_TRANS ("Lead 4 (chiff)"),
        NEEDS_TRANS ("Lead 5 (charang)"),        NEEDS_TRANS ("Lead 6 (voice)"),          NEEDS_TRANS ("Lead 7 (fifths)"),         NEEDS_TRANS ("Lead 8 (bass+lead)"),
        NEEDS_TRANS ("Pad 1 (new age)"),         NEEDS_TRANS ("Pad 2 (warm)"),            NEEDS_TRANS ("Pad 3 (polysynth)"),       NEEDS_TRANS ("Pad 4 (choir)"),
        NEEDS_TRANS ("Pad 5 (bowed)"),           NEEDS_TRANS ("Pad 6 (metallic)"),        NEEDS_TRANS ("Pad 7 (halo)"),            NEEDS_TRANS ("Pad 8 (sweep)"),
        NEEDS_TRANS ("FX 1 (rain)"),             NEEDS_TRANS ("FX 2 (soundtrack)"),       NEEDS_TRANS ("FX 3 (crystal)"),          NEEDS_TRANS ("FX 4 (atmosphere)"),
        NEEDS_TRANS ("FX 5 (brightness)"),       NEEDS_TRANS ("FX 6 (goblins)"),          NEEDS_TRANS ("FX 7 (echoes)"),           NEEDS_TRANS ("FX 8 (sci-fi)"),
        NEEDS_TRANS ("Sitar"),                   NEEDS_TRANS ("Banjo"),                   NEEDS_TRANS ("Shamisen"),                NEEDS_TRANS ("Koto"),
        NEEDS_TRANS ("Kalimba"),                 NEEDS_TRANS ("Bag pipe"),                NEEDS_TRANS ("Fiddle"),                  NEEDS_TRANS ("Shanai"),
        NEEDS_TRANS ("Tinkle Bell"),             NEEDS_TRANS ("Agogo"),                   NEEDS_TRANS ("Steel Drums"),             NEEDS_TRANS ("Woodblock"),
        NEEDS_TRANS ("Taiko Drum"),              NEEDS_TRANS ("Melodic Tom"),             NEEDS_TRANS ("Synth Drum"),              NEEDS_TRANS ("Reverse Cymbal"),
        NEEDS_TRANS ("Guitar Fret Noise"),       NEEDS_TRANS ("Breath Noise"),            NEEDS_TRANS ("Seashore"),                NEEDS_TRANS ("Bird Tweet"),
        NEEDS_TRANS ("Telephone Ring"),          NEEDS_TRANS ("Helicopter"),              NEEDS_TRANS ("Applause"),                NEEDS_TRANS ("Gunshot")
    };

    return isPositiveAndBelow (n, numElementsInArray (names)) ? names[n] : nullptr;
}

tukk MidiMessage::getGMInstrumentBankName (i32k n)
{
    static tukk names[] =
    {
        NEEDS_TRANS ("Piano"),           NEEDS_TRANS ("Chromatic Percussion"),    NEEDS_TRANS ("Organ"),       NEEDS_TRANS ("Guitar"),
        NEEDS_TRANS ("Bass"),            NEEDS_TRANS ("Strings"),                 NEEDS_TRANS ("Ensemble"),    NEEDS_TRANS ("Brass"),
        NEEDS_TRANS ("Reed"),            NEEDS_TRANS ("Pipe"),                    NEEDS_TRANS ("Synth Lead"),  NEEDS_TRANS ("Synth Pad"),
        NEEDS_TRANS ("Synth Effects"),   NEEDS_TRANS ("Ethnic"),                  NEEDS_TRANS ("Percussive"),  NEEDS_TRANS ("Sound Effects")
    };

    return isPositiveAndBelow (n, numElementsInArray (names)) ? names[n] : nullptr;
}

tukk MidiMessage::getRhythmInstrumentName (i32k n)
{
    static tukk names[] =
    {
        NEEDS_TRANS ("Acoustic Bass Drum"),  NEEDS_TRANS ("Bass Drum 1"),     NEEDS_TRANS ("Side Stick"),      NEEDS_TRANS ("Acoustic Snare"),
        NEEDS_TRANS ("Hand Clap"),           NEEDS_TRANS ("Electric Snare"),  NEEDS_TRANS ("Low Floor Tom"),   NEEDS_TRANS ("Closed Hi-Hat"),
        NEEDS_TRANS ("High Floor Tom"),      NEEDS_TRANS ("Pedal Hi-Hat"),    NEEDS_TRANS ("Low Tom"),         NEEDS_TRANS ("Open Hi-Hat"),
        NEEDS_TRANS ("Low-Mid Tom"),         NEEDS_TRANS ("Hi-Mid Tom"),      NEEDS_TRANS ("Crash Cymbal 1"),  NEEDS_TRANS ("High Tom"),
        NEEDS_TRANS ("Ride Cymbal 1"),       NEEDS_TRANS ("Chinese Cymbal"),  NEEDS_TRANS ("Ride Bell"),       NEEDS_TRANS ("Tambourine"),
        NEEDS_TRANS ("Splash Cymbal"),       NEEDS_TRANS ("Cowbell"),         NEEDS_TRANS ("Crash Cymbal 2"),  NEEDS_TRANS ("Vibraslap"),
        NEEDS_TRANS ("Ride Cymbal 2"),       NEEDS_TRANS ("Hi Bongo"),        NEEDS_TRANS ("Low Bongo"),       NEEDS_TRANS ("Mute Hi Conga"),
        NEEDS_TRANS ("Open Hi Conga"),       NEEDS_TRANS ("Low Conga"),       NEEDS_TRANS ("High Timbale"),    NEEDS_TRANS ("Low Timbale"),
        NEEDS_TRANS ("High Agogo"),          NEEDS_TRANS ("Low Agogo"),       NEEDS_TRANS ("Cabasa"),          NEEDS_TRANS ("Maracas"),
        NEEDS_TRANS ("Short Whistle"),       NEEDS_TRANS ("Long Whistle"),    NEEDS_TRANS ("Short Guiro"),     NEEDS_TRANS ("Long Guiro"),
        NEEDS_TRANS ("Claves"),              NEEDS_TRANS ("Hi Wood Block"),   NEEDS_TRANS ("Low Wood Block"),  NEEDS_TRANS ("Mute Cuica"),
        NEEDS_TRANS ("Open Cuica"),          NEEDS_TRANS ("Mute Triangle"),   NEEDS_TRANS ("Open Triangle")
    };

    return (n >= 35 && n <= 81) ? names[n - 35] : nullptr;
}

tukk MidiMessage::getControllerName (i32k n)
{
    static tukk names[] =
    {
        NEEDS_TRANS ("Bank Select"), NEEDS_TRANS ("Modulation Wheel (coarse)"), NEEDS_TRANS ("Breath controller (coarse)"),
        nullptr,
        NEEDS_TRANS ("Foot Pedal (coarse)"), NEEDS_TRANS ("Portamento Time (coarse)"), NEEDS_TRANS ("Data Entry (coarse)"),
        NEEDS_TRANS ("Volume (coarse)"), NEEDS_TRANS ("Balance (coarse)"),
        nullptr,
        NEEDS_TRANS ("Pan position (coarse)"), NEEDS_TRANS ("Expression (coarse)"), NEEDS_TRANS ("Effect Control 1 (coarse)"),
        NEEDS_TRANS ("Effect Control 2 (coarse)"),
        nullptr, nullptr,
        NEEDS_TRANS ("General Purpose Slider 1"), NEEDS_TRANS ("General Purpose Slider 2"),
        NEEDS_TRANS ("General Purpose Slider 3"), NEEDS_TRANS ("General Purpose Slider 4"),
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        NEEDS_TRANS ("Bank Select (fine)"), NEEDS_TRANS ("Modulation Wheel (fine)"), NEEDS_TRANS ("Breath controller (fine)"),
        nullptr,
        NEEDS_TRANS ("Foot Pedal (fine)"), NEEDS_TRANS ("Portamento Time (fine)"), NEEDS_TRANS ("Data Entry (fine)"), NEEDS_TRANS ("Volume (fine)"),
        NEEDS_TRANS ("Balance (fine)"), nullptr, NEEDS_TRANS ("Pan position (fine)"), NEEDS_TRANS ("Expression (fine)"),
        NEEDS_TRANS ("Effect Control 1 (fine)"), NEEDS_TRANS ("Effect Control 2 (fine)"),
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        NEEDS_TRANS ("Hold Pedal (on/off)"), NEEDS_TRANS ("Portamento (on/off)"), NEEDS_TRANS ("Sustenuto Pedal (on/off)"), NEEDS_TRANS ("Soft Pedal (on/off)"),
        NEEDS_TRANS ("Legato Pedal (on/off)"), NEEDS_TRANS ("Hold 2 Pedal (on/off)"), NEEDS_TRANS ("Sound Variation"), NEEDS_TRANS ("Sound Timbre"),
        NEEDS_TRANS ("Sound Release Time"), NEEDS_TRANS ("Sound Attack Time"), NEEDS_TRANS ("Sound Brightness"), NEEDS_TRANS ("Sound Control 6"),
        NEEDS_TRANS ("Sound Control 7"), NEEDS_TRANS ("Sound Control 8"), NEEDS_TRANS ("Sound Control 9"), NEEDS_TRANS ("Sound Control 10"),
        NEEDS_TRANS ("General Purpose Button 1 (on/off)"), NEEDS_TRANS ("General Purpose Button 2 (on/off)"),
        NEEDS_TRANS ("General Purpose Button 3 (on/off)"), NEEDS_TRANS ("General Purpose Button 4 (on/off)"),
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        NEEDS_TRANS ("Reverb Level"), NEEDS_TRANS ("Tremolo Level"), NEEDS_TRANS ("Chorus Level"), NEEDS_TRANS ("Celeste Level"),
        NEEDS_TRANS ("Phaser Level"), NEEDS_TRANS ("Data Button increment"), NEEDS_TRANS ("Data Button decrement"), NEEDS_TRANS ("Non-registered Parameter (fine)"),
        NEEDS_TRANS ("Non-registered Parameter (coarse)"), NEEDS_TRANS ("Registered Parameter (fine)"), NEEDS_TRANS ("Registered Parameter (coarse)"),
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        NEEDS_TRANS ("All Sound Off"), NEEDS_TRANS ("All Controllers Off"), NEEDS_TRANS ("Local Keyboard (on/off)"), NEEDS_TRANS ("All Notes Off"),
        NEEDS_TRANS ("Omni Mode Off"), NEEDS_TRANS ("Omni Mode On"), NEEDS_TRANS ("Mono Operation"), NEEDS_TRANS ("Poly Operation")
    };

    return isPositiveAndBelow (n, numElementsInArray (names)) ? names[n] : nullptr;
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct MidiMessageTest final : public UnitTest
{
    MidiMessageTest()
        : UnitTest ("MidiMessage", UnitTestCategories::midi)
    {}

    z0 runTest() override
    {
        using std::begin;
        using std::end;

        beginTest ("ReadVariableLengthValue should return valid, backward-compatible results");
        {
            const std::vector<u8> inputs[]
            {
                { 0x00 },
                { 0x40 },
                { 0x7f },
                { 0x81, 0x00 },
                { 0xc0, 0x00 },
                { 0xff, 0x7f },
                { 0x81, 0x80, 0x00 },
                { 0xc0, 0x80, 0x00 },
                { 0xff, 0xff, 0x7f },
                { 0x81, 0x80, 0x80, 0x00 },
                { 0xc0, 0x80, 0x80, 0x00 },
                { 0xff, 0xff, 0xff, 0x7f }
            };

            i32k outputs[]
            {
                0x00,
                0x40,
                0x7f,
                0x80,
                0x2000,
                0x3fff,
                0x4000,
                0x100000,
                0x1fffff,
                0x200000,
                0x8000000,
                0xfffffff,
            };

            expectEquals (std::distance (begin (inputs), end (inputs)),
                          std::distance (begin (outputs), end (outputs)));

            size_t index = 0;

            DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

            for (const auto& input : inputs)
            {
                auto copy = input;

                while (copy.size() < 16)
                    copy.push_back (0);

                const auto result = MidiMessage::readVariableLengthValue (copy.data(),
                                                                          (i32) copy.size());

                expect (result.isValid());
                expectEquals (result.value, outputs[index]);
                expectEquals (result.bytesUsed, (i32) inputs[index].size());

                i32 legacyNumUsed = 0;
                const auto legacyResult = MidiMessage::readVariableLengthVal (copy.data(),
                                                                              legacyNumUsed);

                expectEquals (result.value, legacyResult);
                expectEquals (result.bytesUsed, legacyNumUsed);

                ++index;
            }

            DRX_END_IGNORE_DEPRECATION_WARNINGS
        }

        beginTest ("ReadVariableLengthVal should return 0 if input is truncated");
        {
            for (size_t i = 0; i != 16; ++i)
            {
                std::vector<u8> input;
                input.resize (i, 0xFF);

                const auto result = MidiMessage::readVariableLengthValue (input.data(),
                                                                          (i32) input.size());

                expect (! result.isValid());
                expectEquals (result.value, 0);
                expectEquals (result.bytesUsed, 0);
            }
        }

        const std::vector<u8> metaEvents[]
        {
            // Format is 0xff, followed by a 'kind' byte, followed by a variable-length
            // 'data-length' value, followed by that many data bytes
            { 0xff, 0x00, 0x02, 0x00, 0x00 },                   // Sequence number
            { 0xff, 0x01, 0x00 },                               // Text event
            { 0xff, 0x02, 0x00 },                               // Copyright notice
            { 0xff, 0x03, 0x00 },                               // Track name
            { 0xff, 0x04, 0x00 },                               // Instrument name
            { 0xff, 0x05, 0x00 },                               // Lyric
            { 0xff, 0x06, 0x00 },                               // Marker
            { 0xff, 0x07, 0x00 },                               // Cue point
            { 0xff, 0x20, 0x01, 0x00 },                         // Channel prefix
            { 0xff, 0x2f, 0x00 },                               // End of track
            { 0xff, 0x51, 0x03, 0x01, 0x02, 0x03 },             // Set tempo
            { 0xff, 0x54, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05 }, // SMPTE offset
            { 0xff, 0x58, 0x04, 0x01, 0x02, 0x03, 0x04 },       // Time signature
            { 0xff, 0x59, 0x02, 0x01, 0x02 },                   // Key signature
            { 0xff, 0x7f, 0x00 },                               // Sequencer-specific
        };

        beginTest ("MidiMessage data constructor works for well-formed meta-events");
        {
            const auto status = (u8) 0x90;

            for (const auto& input : metaEvents)
            {
                i32 bytesUsed = 0;
                const MidiMessage msg (input.data(), (i32) input.size(), bytesUsed, status);

                expect (msg.isMetaEvent());
                expectEquals (msg.getMetaEventLength(), (i32) input.size() - 3);
                expectEquals (msg.getMetaEventType(), (i32) input[1]);
            }
        }

        beginTest ("MidiMessage data constructor works for malformed meta-events");
        {
            const auto status = (u8) 0x90;

            const auto runTest = [&] (const std::vector<u8>& input)
            {
                i32 bytesUsed = 0;
                const MidiMessage msg (input.data(), (i32) input.size(), bytesUsed, status);

                expect (msg.isMetaEvent());
                expectEquals (msg.getMetaEventLength(), jmax (0, (i32) input.size() - 3));
                expectEquals (msg.getMetaEventType(), input.size() >= 2 ? input[1] : -1);
            };

            runTest ({ 0xff });

            for (const auto& input : metaEvents)
            {
                auto copy = input;
                copy[2] = 0x40; // Set the size of the message to more bytes than are present

                runTest (copy);
            }
        }
    }
};

static MidiMessageTest midiMessageTests;

#endif

} // namespace drx
