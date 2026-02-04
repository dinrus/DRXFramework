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

z64 drx_fileSetPosition (detail::NativeFileHandle handle, z64 pos);


//==============================================================================
FileInputStream::FileInputStream (const File& f)  : file (f)
{
    openHandle();
}

z64 FileInputStream::getTotalLength()
{
    // You should always check that a stream opened successfully before using it!
    jassert (openedOk());

    return file.getSize();
}

i32 FileInputStream::read (uk buffer, i32 bytesToRead)
{
    // You should always check that a stream opened successfully before using it!
    jassert (openedOk());

    // The buffer should never be null, and a negative size is probably a
    // sign that something is broken!
    jassert (buffer != nullptr && bytesToRead >= 0);

    auto num = readInternal (buffer, (size_t) bytesToRead);
    currentPosition += (z64) num;

    return (i32) num;
}

b8 FileInputStream::isExhausted()
{
    return currentPosition >= getTotalLength();
}

z64 FileInputStream::getPosition()
{
    return currentPosition;
}

b8 FileInputStream::setPosition (z64 pos)
{
    // You should always check that a stream opened successfully before using it!
    jassert (openedOk());

    if (pos != currentPosition)
        currentPosition = drx_fileSetPosition (fileHandle, pos);

    return currentPosition == pos;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct FileInputStreamTests final : public UnitTest
{
    FileInputStreamTests()
        : UnitTest ("FileInputStream", UnitTestCategories::streams)
    {}

    z0 runTest() override
    {
        beginTest ("Open stream non-existent file");
        {
            auto tempFile = File::createTempFile (".txt");
            expect (! tempFile.exists());

            FileInputStream stream (tempFile);
            expect (stream.failedToOpen());
        }

        beginTest ("Open stream existing file");
        {
            auto tempFile = File::createTempFile (".txt");
            tempFile.create();
            expect (tempFile.exists());

            FileInputStream stream (tempFile);
            expect (stream.openedOk());
        }

        const MemoryBlock data ("abcdefghijklmnopqrstuvwxyz", 26);
        File f (File::createTempFile (".txt"));
        f.appendData (data.getData(), data.getSize());
        FileInputStream stream (f);

        beginTest ("Read");
        {
            expectEquals (stream.getPosition(), (z64) 0);
            expectEquals (stream.getTotalLength(), (z64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
            expect (! stream.isExhausted());

            size_t numBytesRead = 0;
            MemoryBlock readBuffer (data.getSize());

            while (numBytesRead < data.getSize())
            {
                numBytesRead += (size_t) stream.read (&readBuffer[numBytesRead], 3);

                expectEquals (stream.getPosition(), (z64) numBytesRead);
                expectEquals (stream.getNumBytesRemaining(), (z64) (data.getSize() - numBytesRead));
                expect (stream.isExhausted() == (numBytesRead == data.getSize()));
            }

            expectEquals (stream.getPosition(), (z64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), (z64) 0);
            expect (stream.isExhausted());

            expect (readBuffer == data);
        }

        beginTest ("Skip");
        {
            stream.setPosition (0);
            expectEquals (stream.getPosition(), (z64) 0);
            expectEquals (stream.getTotalLength(), (z64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
            expect (! stream.isExhausted());

            size_t numBytesRead = 0;
            i32k numBytesToSkip = 5;

            while (numBytesRead < data.getSize())
            {
                stream.skipNextBytes (numBytesToSkip);
                numBytesRead += numBytesToSkip;
                numBytesRead = std::min (numBytesRead, data.getSize());

                expectEquals (stream.getPosition(), (z64) numBytesRead);
                expectEquals (stream.getNumBytesRemaining(), (z64) (data.getSize() - numBytesRead));
                expect (stream.isExhausted() == (numBytesRead == data.getSize()));
            }

            expectEquals (stream.getPosition(), (z64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), (z64) 0);
            expect (stream.isExhausted());

            f.deleteFile();
        }
    }
};

static FileInputStreamTests fileInputStreamTests;

#endif

} // namespace drx
