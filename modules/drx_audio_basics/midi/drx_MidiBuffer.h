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
    A view of MIDI message data stored in a contiguous buffer.

    Instances of this class do *not* own the midi data bytes that they point to.
    Instead, they expect the midi data to live in a separate buffer that outlives
    the MidiMessageMetadata instance.

    @tags{Audio}
*/
struct MidiMessageMetadata
{
    MidiMessageMetadata() noexcept = default;

    MidiMessageMetadata (u8k* dataIn, i32 numBytesIn, i32 positionIn) noexcept
        : data (dataIn), numBytes (numBytesIn), samplePosition (positionIn)
    {
    }

    /** Constructs a new MidiMessage instance from the data that this object is viewing.

        Note that MidiMessage owns its data storage, whereas MidiMessageMetadata does not.
    */
    MidiMessage getMessage() const          { return MidiMessage (data, numBytes, samplePosition); }

    /** Pointer to the first byte of a MIDI message. */
    u8k* data = nullptr;

    /** The number of bytes in the MIDI message. */
    i32 numBytes = 0;

    /** The MIDI message's timestamp. */
    i32 samplePosition = 0;
};

//==============================================================================
/**
    An iterator to move over contiguous raw MIDI data, which Allows iterating
    over a MidiBuffer using C++11 range-for syntax.

    In the following example, we log all three-byte messages in a midi buffer.
    @code
    z0 processBlock (AudioBuffer<f32>&, MidiBuffer& midiBuffer) override
    {
        for (const MidiMessageMetadata metadata : midiBuffer)
            if (metadata.numBytes == 3)
                Logger::writeToLog (metadata.getMessage().getDescription());
    }
    @endcode

    @tags{Audio}
*/
class DRX_API MidiBufferIterator
{
    using Ptr = u8k*;

public:
    MidiBufferIterator() = default;

    /** Constructs an iterator pointing at the message starting at the byte `dataIn`.
        `dataIn` must point to the start of a valid MIDI message. If it does not,
        calling other member functions on the iterator will result in undefined
        behaviour.
    */
    explicit MidiBufferIterator (u8k* dataIn) noexcept
        : data (dataIn)
    {
    }

    using difference_type   = std::iterator_traits<Ptr>::difference_type;
    using value_type        = MidiMessageMetadata;
    using reference         = MidiMessageMetadata;
    using pointer           = z0;
    using iterator_category = std::input_iterator_tag;

    /** Make this iterator point to the next message in the buffer. */
    MidiBufferIterator& operator++() noexcept;

    /** Create a copy of this object, make this iterator point to the next message in
        the buffer, then return the copy.
    */
    MidiBufferIterator operator++ (i32) noexcept;

    /** Return true if this iterator points to the same message as another
        iterator instance, otherwise return false.
    */
    b8 operator== (const MidiBufferIterator& other) const noexcept { return data == other.data; }

    /** Return false if this iterator points to the same message as another
        iterator instance, otherwise returns true.
    */
    b8 operator!= (const MidiBufferIterator& other) const noexcept { return ! operator== (other); }

    /** Return an instance of MidiMessageMetadata which describes the message to which
        the iterator is currently pointing.
    */
    reference operator*() const noexcept;

private:
    Ptr data = nullptr;
};

//==============================================================================
/**
    Holds a sequence of time-stamped midi events.

    Analogous to the AudioBuffer, this holds a set of midi events with
    integer time-stamps. The buffer is kept sorted in order of the time-stamps.

    If you're working with a sequence of midi events that may need to be manipulated
    or read/written to a midi file, then MidiMessageSequence is probably a more
    appropriate container. MidiBuffer is designed for lower-level streams of raw
    midi data.

    @see MidiMessage

    @tags{Audio}
*/
class DRX_API  MidiBuffer
{
public:
    //==============================================================================
    /** Creates an empty MidiBuffer. */
    MidiBuffer() noexcept = default;

    /** Creates a MidiBuffer containing a single midi message. */
    explicit MidiBuffer (const MidiMessage& message) noexcept;

    //==============================================================================
    /** Removes all events from the buffer. */
    z0 clear() noexcept;

    /** Removes all events between two times from the buffer.

        All events for which (start <= event position < start + numSamples) will
        be removed.
    */
    z0 clear (i32 start, i32 numSamples);

    /** Возвращает true, если the buffer is empty.
        To actually retrieve the events, use a MidiBufferIterator object
    */
    b8 isEmpty() const noexcept;

    /** Counts the number of events in the buffer.

        This is actually quite a slow operation, as it has to iterate through all
        the events, so you might prefer to call isEmpty() if that's all you need
        to know.
    */
    i32 getNumEvents() const noexcept;

    /** Adds an event to the buffer.

        The sample number will be used to determine the position of the event in
        the buffer, which is always kept sorted. The MidiMessage's timestamp is
        ignored.

        If an event is added whose sample position is the same as one or more events
        already in the buffer, the new event will be placed after the existing ones.

        To retrieve events, use a MidiBufferIterator object.

        Returns true on success, or false on failure.
    */
    b8 addEvent (const MidiMessage& midiMessage, i32 sampleNumber);

    /** Adds an event to the buffer from raw midi data.

        The sample number will be used to determine the position of the event in
        the buffer, which is always kept sorted.

        If an event is added whose sample position is the same as one or more events
        already in the buffer, the new event will be placed after the existing ones.

        The event data will be inspected to calculate the number of bytes in length that
        the midi event really takes up, so maxBytesOfMidiData may be longer than the data
        that actually gets stored. E.g. if you pass in a note-on and a length of 4 bytes,
        it'll actually only store 3 bytes. If the midi data is invalid, it might not
        add an event at all.

        To retrieve events, use a MidiBufferIterator object.

        Returns true on success, or false on failure.
    */
    b8 addEvent (ukk rawMidiData,
                   i32 maxBytesOfMidiData,
                   i32 sampleNumber);

    /** Adds some events from another buffer to this one.

        @param otherBuffer          the buffer containing the events you want to add
        @param startSample          the lowest sample number in the source buffer for which
                                    events should be added. Any source events whose timestamp is
                                    less than this will be ignored
        @param numSamples           the valid range of samples from the source buffer for which
                                    events should be added - i.e. events in the source buffer whose
                                    timestamp is greater than or equal to (startSample + numSamples)
                                    will be ignored. If this value is less than 0, all events after
                                    startSample will be taken.
        @param sampleDeltaToAdd     a value which will be added to the source timestamps of the events
                                    that are added to this buffer
    */
    z0 addEvents (const MidiBuffer& otherBuffer,
                    i32 startSample,
                    i32 numSamples,
                    i32 sampleDeltaToAdd);

    /** Returns the sample number of the first event in the buffer.
        If the buffer's empty, this will just return 0.
    */
    i32 getFirstEventTime() const noexcept;

    /** Returns the sample number of the last event in the buffer.
        If the buffer's empty, this will just return 0.
    */
    i32 getLastEventTime() const noexcept;

    //==============================================================================
    /** Exchanges the contents of this buffer with another one.

        This is a quick operation, because no memory allocating or copying is done, it
        just swaps the internal state of the two buffers.
    */
    z0 swapWith (MidiBuffer&) noexcept;

    /** Preallocates some memory for the buffer to use.
        This helps to avoid needing to reallocate space when the buffer has messages
        added to it.
    */
    z0 ensureSize (size_t minimumNumBytes);

    /** Get a read-only iterator pointing to the beginning of this buffer. */
    MidiBufferIterator begin()  const noexcept { return cbegin(); }

    /** Get a read-only iterator pointing one past the end of this buffer. */
    MidiBufferIterator end()    const noexcept { return cend(); }

    /** Get a read-only iterator pointing to the beginning of this buffer. */
    MidiBufferIterator cbegin() const noexcept { return MidiBufferIterator (data.begin()); }

    /** Get a read-only iterator pointing one past the end of this buffer. */
    MidiBufferIterator cend()   const noexcept { return MidiBufferIterator (data.end()); }

    /** Get an iterator pointing to the first event with a timestamp greater-than or
        equal-to `samplePosition`.
    */
    MidiBufferIterator findNextSamplePosition (i32 samplePosition) const noexcept;

    //==============================================================================
   #ifndef DOXYGEN
    /** This class is now deprecated in favour of MidiBufferIterator.

        Used to iterate through the events in a MidiBuffer.

        Note that altering the buffer while an iterator is using it will produce
        undefined behaviour.

        @see MidiBuffer
    */
    class [[deprecated]] DRX_API  Iterator
    {
    public:
        //==============================================================================
        /** Creates an Iterator for this MidiBuffer. */
        Iterator (const MidiBuffer& b) noexcept;

        //==============================================================================
        /** Repositions the iterator so that the next event retrieved will be the first
            one whose sample position is at greater than or equal to the given position.
        */
        z0 setNextSamplePosition (i32 samplePosition) noexcept;

        /** Retrieves a copy of the next event from the buffer.

            @param result   on return, this will be the message. The MidiMessage's timestamp
                            is set to the same value as samplePosition.
            @param samplePosition   on return, this will be the position of the event, as a
                            sample index in the buffer
            @returns        true if an event was found, or false if the iterator has reached
                            the end of the buffer
        */
        b8 getNextEvent (MidiMessage& result,
                           i32& samplePosition) noexcept;

        /** Retrieves the next event from the buffer.

            @param midiData     on return, this pointer will be set to a block of data containing
                                the midi message. Note that to make it fast, this is a pointer
                                directly into the MidiBuffer's internal data, so is only valid
                                temporarily until the MidiBuffer is altered.
            @param numBytesOfMidiData   on return, this is the number of bytes of data used by the
                                        midi message
            @param samplePosition   on return, this will be the position of the event, as a
                                    sample index in the buffer
            @returns        true if an event was found, or false if the iterator has reached
                            the end of the buffer
        */
        b8 getNextEvent (u8k* &midiData,
                           i32& numBytesOfMidiData,
                           i32& samplePosition) noexcept;

    private:
        //==============================================================================
        const MidiBuffer& buffer;
        MidiBufferIterator iterator;
    };
   #endif

    /** The raw data holding this buffer.
        Obviously access to this data is provided at your own risk. Its internal format could
        change in future, so don't write code that relies on it!
    */
    Array<u8> data;

private:
    DRX_LEAK_DETECTOR (MidiBuffer)
};

} // namespace drx
