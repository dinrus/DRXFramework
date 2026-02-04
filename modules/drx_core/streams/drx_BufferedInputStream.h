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
/** Wraps another input stream, and reads from it using an intermediate buffer

    If you're using an input stream such as a file input stream, and making lots of
    small read accesses to it, it's probably sensible to wrap it in one of these,
    so that the source stream gets accessed in larger chunk sizes, meaning less
    work for the underlying stream.

    @tags{Core}
*/
class DRX_API  BufferedInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a BufferedInputStream from an input source.

        @param sourceStream                 the source stream to read from
        @param bufferSize                   the size of reservoir to use to buffer the source
        @param deleteSourceWhenDestroyed    whether the sourceStream that is passed in should be
                                            deleted by this object when it is itself deleted.
    */
    BufferedInputStream (InputStream* sourceStream,
                         i32 bufferSize,
                         b8 deleteSourceWhenDestroyed);

    /** Creates a BufferedInputStream from an input source.

        @param sourceStream     the source stream to read from - the source stream  must not
                                be deleted until this object has been destroyed.
        @param bufferSize       the size of reservoir to use to buffer the source
    */
    BufferedInputStream (InputStream& sourceStream, i32 bufferSize);

    /** Destructor.

        This may also delete the source stream, if that option was chosen when the
        buffered stream was created.
    */
    ~BufferedInputStream() override;


    //==============================================================================
    /** Returns the next byte that would be read by a call to readByte() */
    t8 peekByte();

    z64 getTotalLength() override;
    z64 getPosition() override;
    b8 setPosition (z64 newPosition) override;
    i32 read (uk destBuffer, i32 maxBytesToRead) override;
    Txt readString() override;
    b8 isExhausted() override;


private:
    //==============================================================================
    OptionalScopedPointer<InputStream> source;
    Range<z64> bufferedRange;
    z64 position, bufferLength, lastReadPos = 0, bufferOverlap = 128;
    HeapBlock<t8> buffer;
    b8 ensureBuffered();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferedInputStream)
};

} // namespace drx
