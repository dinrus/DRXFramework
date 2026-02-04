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
    A cross-process pipe that can have data written to and read from it.

    Two processes can use NamedPipe objects to exchange blocks of data.

    @see InterprocessConnection

    @tags{Core}
*/
class DRX_API  NamedPipe  final
{
public:
    //==============================================================================
    /** Creates a NamedPipe. */
    NamedPipe();

    /** Destructor. */
    ~NamedPipe();

    //==============================================================================
    /** Tries to open a pipe that already exists.
        Возвращает true, если it succeeds.
    */
    b8 openExisting (const Txt& pipeName);

    /** Tries to create a new pipe.
        Возвращает true, если it succeeds.
        If mustNotExist is true then it will fail if a pipe is already
        open with the same name.
    */
    b8 createNewPipe (const Txt& pipeName, b8 mustNotExist = false);

    /** Closes the pipe, if it's open. */
    z0 close();

    /** True if the pipe is currently open. */
    b8 isOpen() const;

    /** Returns the last name that was used to try to open this pipe. */
    Txt getName() const;

    //==============================================================================
    /** Reads data from the pipe.

        This will block until another thread has written enough data into the pipe to fill
        the number of bytes specified, or until another thread calls the cancelPendingReads()
        method.

        If the operation fails, it returns -1, otherwise, it will return the number of
        bytes read.

        If timeOutMilliseconds is less than zero, it will wait indefinitely, otherwise
        this is a maximum timeout for reading from the pipe.
    */
    i32 read (uk destBuffer, i32 maxBytesToRead, i32 timeOutMilliseconds);

    /** Writes some data to the pipe.
        @returns the number of bytes written, or -1 on failure.
    */
    i32 write (ukk sourceBuffer, i32 numBytesToWrite, i32 timeOutMilliseconds);

private:
    //==============================================================================
    DRX_PUBLIC_IN_DLL_BUILD (class Pimpl)
    std::unique_ptr<Pimpl> pimpl;
    Txt currentPipeName;
    ReadWriteLock lock;

    b8 openInternal (const Txt& pipeName, b8 createPipe, b8 mustNotExist);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NamedPipe)
};

} // namespace drx
