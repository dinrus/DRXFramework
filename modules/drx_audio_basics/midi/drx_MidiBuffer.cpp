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

namespace MidiBufferHelpers
{
    inline i32 getEventTime (ukk d) noexcept
    {
        return readUnaligned<i32> (d);
    }

    inline u16 getEventDataSize (ukk d) noexcept
    {
        return readUnaligned<u16> (static_cast<tukk> (d) + sizeof (i32));
    }

    inline u16 getEventTotalSize (ukk d) noexcept
    {
        return (u16) (getEventDataSize (d) + sizeof (i32) + sizeof (u16));
    }

    static i32 findActualEventLength (u8k* data, i32 maxBytes) noexcept
    {
        auto byte = (u32) *data;

        if (byte == 0xf0 || byte == 0xf7)
        {
            i32 i = 1;

            while (i < maxBytes)
                if (data[i++] == 0xf7)
                    break;

            return i;
        }

        if (byte == 0xff)
        {
            if (maxBytes == 1)
                return 1;

            const auto var = MidiMessage::readVariableLengthValue (data + 1, maxBytes - 1);
            return jmin (maxBytes, var.value + 2 + var.bytesUsed);
        }

        if (byte >= 0x80)
            return jmin (maxBytes, MidiMessage::getMessageLengthFromFirstByte ((u8) byte));

        return 0;
    }

    static u8* findEventAfter (u8* d, u8* endData, i32 samplePosition) noexcept
    {
        while (d < endData && getEventTime (d) <= samplePosition)
            d += getEventTotalSize (d);

        return d;
    }
}

//==============================================================================
MidiBufferIterator& MidiBufferIterator::operator++() noexcept
{
    data += sizeof (i32) + sizeof (u16) + size_t (MidiBufferHelpers::getEventDataSize (data));
    return *this;
}

MidiBufferIterator MidiBufferIterator::operator++ (i32) noexcept
{
    auto copy = *this;
    ++(*this);
    return copy;
}

MidiBufferIterator::reference MidiBufferIterator::operator*() const noexcept
{
    return { data + sizeof (i32) + sizeof (u16),
             MidiBufferHelpers::getEventDataSize (data),
             MidiBufferHelpers::getEventTime (data) };
}

//==============================================================================
MidiBuffer::MidiBuffer (const MidiMessage& message) noexcept
{
    addEvent (message, 0);
}

z0 MidiBuffer::swapWith (MidiBuffer& other) noexcept      { data.swapWith (other.data); }
z0 MidiBuffer::clear() noexcept                           { data.clearQuick(); }
z0 MidiBuffer::ensureSize (size_t minimumNumBytes)        { data.ensureStorageAllocated ((i32) minimumNumBytes); }
b8 MidiBuffer::isEmpty() const noexcept                   { return data.size() == 0; }

z0 MidiBuffer::clear (i32 startSample, i32 numSamples)
{
    auto start = MidiBufferHelpers::findEventAfter (data.begin(), data.end(), startSample - 1);
    auto end   = MidiBufferHelpers::findEventAfter (start,        data.end(), startSample + numSamples - 1);

    data.removeRange ((i32) (start - data.begin()), (i32) (end - start));
}

b8 MidiBuffer::addEvent (const MidiMessage& m, i32 sampleNumber)
{
    return addEvent (m.getRawData(), m.getRawDataSize(), sampleNumber);
}

b8 MidiBuffer::addEvent (ukk newData, i32 maxBytes, i32 sampleNumber)
{
    auto numBytes = MidiBufferHelpers::findActualEventLength (static_cast<u8k*> (newData), maxBytes);

    if (numBytes <= 0)
        return true;

    if (std::numeric_limits<u16>::max() < numBytes)
    {
        // This method only supports messages smaller than (1 << 16) bytes
        return false;
    }

    auto newItemSize = (size_t) numBytes + sizeof (i32) + sizeof (u16);
    auto offset = (i32) (MidiBufferHelpers::findEventAfter (data.begin(), data.end(), sampleNumber) - data.begin());

    data.insertMultiple (offset, 0, (i32) newItemSize);

    auto* d = data.begin() + offset;
    writeUnaligned<i32>  (d, sampleNumber);
    d += sizeof (i32);
    writeUnaligned<u16> (d, static_cast<u16> (numBytes));
    d += sizeof (u16);
    memcpy (d, newData, (size_t) numBytes);

    return true;
}

z0 MidiBuffer::addEvents (const MidiBuffer& otherBuffer,
                            i32 startSample, i32 numSamples, i32 sampleDeltaToAdd)
{
    for (auto i = otherBuffer.findNextSamplePosition (startSample); i != otherBuffer.cend(); ++i)
    {
        const auto metadata = *i;

        if (metadata.samplePosition >= startSample + numSamples && numSamples >= 0)
            break;

        addEvent (metadata.data, metadata.numBytes, metadata.samplePosition + sampleDeltaToAdd);
    }
}

i32 MidiBuffer::getNumEvents() const noexcept
{
    i32 n = 0;
    auto end = data.end();

    for (auto d = data.begin(); d < end; ++n)
        d += MidiBufferHelpers::getEventTotalSize (d);

    return n;
}

i32 MidiBuffer::getFirstEventTime() const noexcept
{
    return data.size() > 0 ? MidiBufferHelpers::getEventTime (data.begin()) : 0;
}

i32 MidiBuffer::getLastEventTime() const noexcept
{
    if (data.size() == 0)
        return 0;

    auto endData = data.end();

    for (auto d = data.begin();;)
    {
        auto nextOne = d + MidiBufferHelpers::getEventTotalSize (d);

        if (nextOne >= endData)
            return MidiBufferHelpers::getEventTime (d);

        d = nextOne;
    }
}

MidiBufferIterator MidiBuffer::findNextSamplePosition (i32 samplePosition) const noexcept
{
    return std::find_if (cbegin(), cend(), [&] (const MidiMessageMetadata& metadata) noexcept
    {
        return metadata.samplePosition >= samplePosition;
    });
}

//==============================================================================
DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

MidiBuffer::Iterator::Iterator (const MidiBuffer& b) noexcept
    : buffer (b), iterator (b.data.begin())
{
}

z0 MidiBuffer::Iterator::setNextSamplePosition (i32 samplePosition) noexcept
{
    iterator = buffer.findNextSamplePosition (samplePosition);
}

b8 MidiBuffer::Iterator::getNextEvent (u8k*& midiData, i32& numBytes, i32& samplePosition) noexcept
{
    if (iterator == buffer.cend())
        return false;

    const auto metadata = *iterator++;
    midiData = metadata.data;
    numBytes = metadata.numBytes;
    samplePosition = metadata.samplePosition;
    return true;
}

b8 MidiBuffer::Iterator::getNextEvent (MidiMessage& result, i32& samplePosition) noexcept
{
    if (iterator == buffer.cend())
        return false;

    const auto metadata = *iterator++;
    result = metadata.getMessage();
    samplePosition = metadata.samplePosition;
    return true;
}

DRX_END_IGNORE_DEPRECATION_WARNINGS

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct MidiBufferTest final : public UnitTest
{
    MidiBufferTest()
        : UnitTest ("MidiBuffer", UnitTestCategories::midi)
    {}

    z0 runTest() override
    {
        beginTest ("Clear messages");
        {
            const auto message = MidiMessage::noteOn (1, 64, 0.5f);

            const auto testBuffer = [&]
            {
                MidiBuffer buffer;
                buffer.addEvent (message, 0);
                buffer.addEvent (message, 10);
                buffer.addEvent (message, 20);
                buffer.addEvent (message, 30);
                return buffer;
            }();

            {
                auto buffer = testBuffer;
                buffer.clear (10, 0);
                expectEquals (buffer.getNumEvents(), 4);
            }

            {
                auto buffer = testBuffer;
                buffer.clear (10, 1);
                expectEquals (buffer.getNumEvents(), 3);
            }

            {
                auto buffer = testBuffer;
                buffer.clear (10, 10);
                expectEquals (buffer.getNumEvents(), 3);
            }

            {
                auto buffer = testBuffer;
                buffer.clear (10, 20);
                expectEquals (buffer.getNumEvents(), 2);
            }

            {
                auto buffer = testBuffer;
                buffer.clear (10, 30);
                expectEquals (buffer.getNumEvents(), 1);
            }

            {
                auto buffer = testBuffer;
                buffer.clear (10, 300);
                expectEquals (buffer.getNumEvents(), 1);
            }
        }
    }
};

static MidiBufferTest midiBufferTest;

#endif

} // namespace drx
