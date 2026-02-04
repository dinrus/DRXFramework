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
    An output stream that writes into a local file.

    @see OutputStream, FileInputStream, File::createOutputStream

    @tags{Core}
*/
class DRX_API  FileOutputStream  : public OutputStream
{
public:
    //==============================================================================
    /** Creates a FileOutputStream.

        If the file doesn't exist, it will first be created. If the file can't be
        created or opened (for example, because the parent directory of the file
        does not exist), the failedToOpen() method will return true.

        If the file already exists when opened, the stream's write-position will
        be set to the end of the file. To overwrite an existing file, you can truncate
        it like this:

        @code
        FileOutputStream stream (file);

        if (stream.openedOk())
        {
            stream.setPosition (0);
            stream.truncate();
            ...
        }
        @endcode


        Destroying a FileOutputStream object does not force the operating system
        to write the buffered data to disk immediately. If this is required you
        should call flush() before triggering the destructor.

        @see TemporaryFile
    */
    FileOutputStream (const File& fileToWriteTo,
                      size_t bufferSizeToUse = 16384);

    /** Destructor. */
    ~FileOutputStream() override;

    //==============================================================================
    /** Returns the file that this stream is writing to.
    */
    const File& getFile() const                         { return file; }

    /** Returns the status of the file stream.
        The result will be ok if the file opened successfully. If an error occurs while
        opening or writing to the file, this will contain an error message.
    */
    const Result& getStatus() const noexcept            { return status; }

    /** Возвращает true, если the stream couldn't be opened for some reason.
        @see getStatus()
    */
    b8 failedToOpen() const noexcept                  { return status.failed(); }

    /** Возвращает true, если the stream opened without problems.
        @see getStatus()
    */
    b8 openedOk() const noexcept                      { return status.wasOk(); }

    /** Attempts to truncate the file to the current write position.
        To truncate a file to a specific size, first use setPosition() to seek to the
        appropriate location, and then call this method.
    */
    Result truncate();

    //==============================================================================
    z0 flush() override;
    z64 getPosition() override;
    b8 setPosition (z64) override;
    b8 write (ukk, size_t) override;
    b8 writeRepeatedByte (u8 byte, size_t numTimesToRepeat) override;


private:
    //==============================================================================
    File file;
    detail::NativeFileHandle fileHandle{};
    Result status { Result::ok() };
    z64 currentPosition = 0;
    size_t bufferSize, bytesInBuffer = 0;
    HeapBlock<t8> buffer;

    z0 openHandle();
    z0 closeHandle();
    z0 flushInternal();
    b8 flushBuffer();
    z64 setPositionInternal (z64);
    ssize_t writeInternal (ukk, size_t);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileOutputStream)
};

} // namespace drx
