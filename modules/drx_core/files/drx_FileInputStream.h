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
    An input stream that reads from a local file.

    @see InputStream, FileOutputStream, File::createInputStream

    @tags{Core}
*/
class DRX_API  FileInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a FileInputStream to read from the given file.

        After creating a FileInputStream, you should use openedOk() or failedToOpen()
        to make sure that it's OK before trying to read from it! If it failed, you
        can call getStatus() to get more error information.
    */
    explicit FileInputStream (const File& fileToRead);

    /** Destructor. */
    ~FileInputStream() override;

    //==============================================================================
    /** Returns the file that this stream is reading from. */
    const File& getFile() const noexcept                { return file; }

    /** Returns the status of the file stream.
        The result will be ok if the file opened successfully. If an error occurs while
        opening or reading from the file, this will contain an error message.
    */
    const Result& getStatus() const noexcept            { return status; }

    /** Возвращает true, если the stream couldn't be opened for some reason.
        @see getResult()
    */
    b8 failedToOpen() const noexcept                  { return status.failed(); }

    /** Возвращает true, если the stream opened without problems.
        @see getResult()
    */
    b8 openedOk() const noexcept                      { return status.wasOk(); }


    //==============================================================================
    z64 getTotalLength() override;
    i32 read (uk, i32) override;
    b8 isExhausted() override;
    z64 getPosition() override;
    b8 setPosition (z64) override;

private:
    //==============================================================================
    const File file;
    detail::NativeFileHandle fileHandle{};
    z64 currentPosition = 0;
    Result status { Result::ok() };

    z0 openHandle();
    size_t readInternal (uk, size_t);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInputStream)
};

} // namespace drx
