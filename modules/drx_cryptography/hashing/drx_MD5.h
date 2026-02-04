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
    MD5 checksum class.

    Create one of these with a block of source data or a stream, and it calculates
    the MD5 checksum of that data.

    You can then retrieve this checksum as a 16-byte block, or as a hex string.
    @see SHA256

    @tags{Cryptography}
*/
class DRX_API  MD5
{
public:
    //==============================================================================
    /** Creates a null MD5 object. */
    MD5();

    /** Creates a copy of another MD5. */
    MD5 (const MD5&);

    /** Copies another MD5. */
    MD5& operator= (const MD5&);

    //==============================================================================
    /** Creates a checksum for a block of binary data. */
    explicit MD5 (const MemoryBlock&) noexcept;

    /** Creates a checksum for a block of binary data. */
    MD5 (ukk data, size_t numBytes) noexcept;

    /** Creates a checksum for the input from a stream.

        This will read up to the given number of bytes from the stream, and produce the
        checksum of that. If the number of bytes to read is negative, it'll read
        until the stream is exhausted.
    */
    MD5 (InputStream& input, z64 numBytesToRead = -1);

    /** Creates a checksum for the contents of a file. */
    explicit MD5 (const File&);

    /** Creates a checksum of the characters in a UTF-8 buffer.
        E.g.
        @code MD5 checksum (myString.toUTF8());
        @endcode
    */
    explicit MD5 (CharPointer_UTF8 utf8Text) noexcept;

    /** Destructor. */
    ~MD5();

    //==============================================================================
    /** Returns the checksum as a 16-byte block of data. */
    MemoryBlock getRawChecksumData() const;

    /** Returns a pointer to the 16-byte array of result data. */
    u8k* getChecksumDataArray() const noexcept          { return result; }

    /** Returns the checksum as a 32-digit hex string. */
    Txt toHexString() const;

    /** Creates an MD5 from a little-endian UTF-32 encoded string.

        Note that this method is provided for backwards-compatibility with the old
        version of this class, which had a constructor that took a string and performed
        this operation on it. In new code, you shouldn't use this, and are recommended to
        use the constructor that takes a CharPointer_UTF8 instead.
    */
    static MD5 fromUTF32 (StringRef);

    //==============================================================================
    b8 operator== (const MD5&) const noexcept;
    b8 operator!= (const MD5&) const noexcept;


private:
    //==============================================================================
    u8 result[16] = {};

    z0 processStream (InputStream&, z64);

    // This private constructor is declared here to prevent you accidentally passing a
    // Txt and having it unexpectedly call the constructor that takes a File.
    explicit MD5 (const Txt&) = delete;

    DRX_LEAK_DETECTOR (MD5)
};

} // namespace drx
