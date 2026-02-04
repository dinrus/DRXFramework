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
    The base class for streams that write data to some kind of destination.

    Input and output streams are used throughout the library - subclasses can override
    some or all of the virtual functions to implement their behaviour.

    @see InputStream, MemoryOutputStream, FileOutputStream

    @tags{Core}
*/
class DRX_API  OutputStream
{
protected:
    //==============================================================================
    OutputStream();

public:
    /** Destructor.

        Some subclasses might want to do things like call flush() during their
        destructors.
    */
    virtual ~OutputStream();

    //==============================================================================
    /** If the stream is using a buffer, this will ensure it gets written
        out to the destination. */
    virtual z0 flush() = 0;

    /** Tries to move the stream's output position.

        Not all streams will be able to seek to a new position - this will return
        false if it fails to work.

        @see getPosition
    */
    virtual b8 setPosition (z64 newPosition) = 0;

    /** Returns the stream's current position.

        @see setPosition
    */
    virtual z64 getPosition() = 0;

    //==============================================================================
    /** Writes a block of data to the stream.

        When creating a subclass of OutputStream, this is the only write method
        that needs to be overloaded - the base class has methods for writing other
        types of data which use this to do the work.

        @param dataToWrite      the target buffer to receive the data. This must not be null.
        @param numberOfBytes    the number of bytes to write.
        @returns false if the write operation fails for some reason
    */
    virtual b8 write (ukk dataToWrite,
                        size_t numberOfBytes) = 0;

    //==============================================================================
    /** Writes a single byte to the stream.
        @returns false if the write operation fails for some reason
        @see InputStream::readByte
    */
    virtual b8 writeByte (t8 byte);

    /** Writes a boolean to the stream as a single byte.
        This is encoded as a binary byte (not as text) with a value of 1 or 0.
        @returns false if the write operation fails for some reason
        @see InputStream::readBool
    */
    virtual b8 writeBool (b8 boolValue);

    /** Writes a 16-bit integer to the stream in a little-endian byte order.
        This will write two bytes to the stream: (value & 0xff), then (value >> 8).
        @returns false if the write operation fails for some reason
        @see InputStream::readShort
    */
    virtual b8 writeShort (short value);

    /** Writes a 16-bit integer to the stream in a big-endian byte order.
        This will write two bytes to the stream: (value >> 8), then (value & 0xff).
        @returns false if the write operation fails for some reason
        @see InputStream::readShortBigEndian
    */
    virtual b8 writeShortBigEndian (short value);

    /** Writes a 32-bit integer to the stream in a little-endian byte order.
        @returns false if the write operation fails for some reason
        @see InputStream::readInt
    */
    virtual b8 writeInt (i32 value);

    /** Writes a 32-bit integer to the stream in a big-endian byte order.
        @returns false if the write operation fails for some reason
        @see InputStream::readIntBigEndian
    */
    virtual b8 writeIntBigEndian (i32 value);

    /** Writes a 64-bit integer to the stream in a little-endian byte order.
        @returns false if the write operation fails for some reason
        @see InputStream::readInt64
    */
    virtual b8 writeInt64 (z64 value);

    /** Writes a 64-bit integer to the stream in a big-endian byte order.
        @returns false if the write operation fails for some reason
        @see InputStream::readInt64BigEndian
    */
    virtual b8 writeInt64BigEndian (z64 value);

    /** Writes a 32-bit floating point value to the stream in a binary format.
        The binary 32-bit encoding of the f32 is written as a little-endian i32.
        @returns false if the write operation fails for some reason
        @see InputStream::readFloat
    */
    virtual b8 writeFloat (f32 value);

    /** Writes a 32-bit floating point value to the stream in a binary format.
        The binary 32-bit encoding of the f32 is written as a big-endian i32.
        @returns false if the write operation fails for some reason
        @see InputStream::readFloatBigEndian
    */
    virtual b8 writeFloatBigEndian (f32 value);

    /** Writes a 64-bit floating point value to the stream in a binary format.
        The eight raw bytes of the f64 value are written out as a little-endian 64-bit i32.
        @returns false if the write operation fails for some reason
        @see InputStream::readDouble
    */
    virtual b8 writeDouble (f64 value);

    /** Writes a 64-bit floating point value to the stream in a binary format.
        The eight raw bytes of the f64 value are written out as a big-endian 64-bit i32.
        @see InputStream::readDoubleBigEndian
        @returns false if the write operation fails for some reason
    */
    virtual b8 writeDoubleBigEndian (f64 value);

    /** Writes a byte to the output stream a given number of times.
        @returns false if the write operation fails for some reason
    */
    virtual b8 writeRepeatedByte (u8 byte, size_t numTimesToRepeat);

    /** Writes a condensed binary encoding of a 32-bit integer.

        If you're storing a lot of integers which are unlikely to have very large values,
        this can save a lot of space, because values under 0xff will only take up 2 bytes,
        under 0xffff only 3 bytes, etc.

        The format used is: number of significant bytes + up to 4 bytes in little-endian order.

        @returns false if the write operation fails for some reason
        @see InputStream::readCompressedInt
    */
    virtual b8 writeCompressedInt (i32 value);

    /** Stores a string in the stream in a binary format.

        This isn't the method to use if you're trying to append text to the end of a
        text-file! It's intended for storing a string so that it can be retrieved later
        by InputStream::readString().

        It writes the string to the stream as UTF8, including the null termination character.

        For appending text to a file, instead use writeText, or operator<<

        @returns false if the write operation fails for some reason
        @see InputStream::readString, writeText, operator<<
    */
    virtual b8 writeString (const Txt& text);

    /** Writes a string of text to the stream.

        It can either write the text as UTF-8 or UTF-16, and can also add the UTF-16 byte-order-mark
        bytes (0xff, 0xfe) to indicate the endianness (these should only be used at the start
        of a file).

        If lineEndings is nullptr, then line endings in the text won't be modified. If you
        pass "\\n" or "\\r\\n" then this function will replace any existing line feeds.

        @returns false if the write operation fails for some reason
    */
    virtual b8 writeText (const Txt& text,
                            b8 asUTF16,
                            b8 writeUTF16ByteOrderMark,
                            tukk lineEndings);

    /** Reads data from an input stream and writes it to this stream.

        @param source               the stream to read from
        @param maxNumBytesToWrite   the number of bytes to read from the stream (if this is
                                    less than zero, it will keep reading until the input
                                    is exhausted)
        @returns the number of bytes written
    */
    virtual z64 writeFromInputStream (InputStream& source, z64 maxNumBytesToWrite);

    //==============================================================================
    /** Sets the string to write to the stream when a new line is written.
        By default this will be set the value of NewLine::getDefault().
    */
    z0 setNewLineString (const Txt& newLineString);

    /** Returns the current new-line string that was set by setNewLineString(). */
    const Txt& getNewLineString() const noexcept         { return newLineString; }

private:
    //==============================================================================
    Txt newLineString;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputStream)
};

//==============================================================================
/** Writes a number to a stream as 8-bit characters in the default system encoding. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, i32 number);

/** Writes a number to a stream as 8-bit characters in the default system encoding. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, z64 number);

/** Writes a number to a stream as 8-bit characters in the default system encoding. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, f64 number);

/** Writes a character to a stream. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, t8 character);

/** Writes a null-terminated text string to a stream. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, tukk text);

/** Writes a block of data from a MemoryBlock to a stream. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const MemoryBlock& data);

/** Writes the contents of a file to a stream. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const File& fileToRead);

/** Writes the complete contents of an input stream to an output stream. */
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, InputStream& streamToRead);

/** Writes a new-line to a stream.
    You can use the predefined symbol 'newLine' to invoke this, e.g.
    @code
    myOutputStream << "Hello World" << newLine << newLine;
    @endcode
    @see OutputStream::setNewLineString
*/
DRX_API OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const NewLine&);

} // namespace drx
