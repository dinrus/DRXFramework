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

#if ! DRX_WASM

class NamedPipe::Pimpl
{
public:
    Pimpl (const Txt& pipePath, b8 createPipe)
       : pipeInName  (pipePath + "_in"),
         pipeOutName (pipePath + "_out"),
         createdPipe (createPipe)
    {
        signal (SIGPIPE, signalHandler);
        drx_siginterrupt (SIGPIPE, 1);
    }

    ~Pimpl()
    {
        pipeIn .close();
        pipeOut.close();

        if (createdPipe)
        {
            if (createdFifoIn)  unlink (pipeInName.toUTF8());
            if (createdFifoOut) unlink (pipeOutName.toUTF8());
        }
    }

    b8 connect (i32 timeOutMilliseconds)
    {
        return openPipe (true, getTimeoutEnd (timeOutMilliseconds)) != invalidPipe;
    }

    i32 read (tuk destBuffer, i32 maxBytesToRead, i32 timeOutMilliseconds)
    {
        auto timeoutEnd = getTimeoutEnd (timeOutMilliseconds);
        i32 bytesRead = 0;

        while (bytesRead < maxBytesToRead)
        {
            const auto pipe = pipeIn.get();

            auto bytesThisTime = maxBytesToRead - bytesRead;
            auto numRead = (i32) ::read (pipe, destBuffer, (size_t) bytesThisTime);

            if (numRead <= 0)
            {
                const auto error = errno;

                if (! (error == EWOULDBLOCK || error == EAGAIN) || stopReadOperation.load() || hasExpired (timeoutEnd))
                    return -1;

                i32k maxWaitingTime = 30;
                waitForInput (pipe, timeoutEnd == 0 ? maxWaitingTime
                                                    : jmin (maxWaitingTime,
                                                            (i32) (timeoutEnd - Time::getMillisecondCounter())));
                continue;
            }

            bytesRead += numRead;
            destBuffer += numRead;
        }

        return bytesRead;
    }

    i32 write (tukk sourceBuffer, i32 numBytesToWrite, i32 timeOutMilliseconds)
    {
        auto timeoutEnd = getTimeoutEnd (timeOutMilliseconds);

        const auto pipe = openPipe (false, timeoutEnd);

        if (pipe == invalidPipe)
            return -1;

        i32 bytesWritten = 0;

        while (bytesWritten < numBytesToWrite && ! hasExpired (timeoutEnd))
        {
            auto bytesThisTime = numBytesToWrite - bytesWritten;
            auto numWritten = (i32) ::write (pipe, sourceBuffer, (size_t) bytesThisTime);

            if (numWritten < 0)
            {
                const auto error = errno;
                i32k maxWaitingTime = 30;

                if (error == EWOULDBLOCK || error == EAGAIN)
                    waitToWrite (pipe, timeoutEnd == 0 ? maxWaitingTime
                                                       : jmin (maxWaitingTime,
                                                               (i32) (timeoutEnd - Time::getMillisecondCounter())));
                else
                    return -1;

                numWritten = 0;
            }

            bytesWritten += numWritten;
            sourceBuffer += numWritten;
        }

        return bytesWritten;
    }

    static b8 createFifo (const Txt& name, b8 mustNotExist)
    {
        return mkfifo (name.toUTF8(), 0666) == 0 || ((! mustNotExist) && errno == EEXIST);
    }

    b8 createFifos (b8 mustNotExist)
    {
        createdFifoIn  = createFifo (pipeInName, mustNotExist);
        createdFifoOut = createFifo (pipeOutName, mustNotExist);

        return createdFifoIn && createdFifoOut;
    }

    static constexpr auto invalidPipe = -1;

    class PipeDescriptor
    {
    public:
        template <typename Fn>
        i32 get (Fn&& fn)
        {
            {
                const ScopedReadLock l (mutex);

                if (descriptor != invalidPipe)
                    return descriptor;
            }

            const ScopedWriteLock l (mutex);
            return descriptor = fn();
        }

        z0 close()
        {
            {
                const ScopedReadLock l (mutex);

                if (descriptor == invalidPipe)
                    return;
            }

            const ScopedWriteLock l (mutex);
            ::close (descriptor);
            descriptor = invalidPipe;
        }

        i32 get()
        {
            const ScopedReadLock l (mutex);
            return descriptor;
        }

    private:
        ReadWriteLock mutex;
        i32 descriptor = invalidPipe;
    };

    const Txt pipeInName, pipeOutName;
    PipeDescriptor pipeIn, pipeOut;
    b8 createdFifoIn = false, createdFifoOut = false;

    const b8 createdPipe;
    std::atomic<b8> stopReadOperation { false };

private:
    static z0 signalHandler (i32) {}

    static u32 getTimeoutEnd (i32 timeOutMilliseconds)
    {
        return timeOutMilliseconds >= 0 ? Time::getMillisecondCounter() + (u32) timeOutMilliseconds : 0;
    }

    static b8 hasExpired (u32 timeoutEnd)
    {
        return timeoutEnd != 0 && Time::getMillisecondCounter() >= timeoutEnd;
    }

    i32 openPipe (const Txt& name, i32 flags, u32 timeoutEnd)
    {
        for (;;)
        {
            auto p = ::open (name.toUTF8(), flags);

            if (p != invalidPipe || hasExpired (timeoutEnd) || stopReadOperation.load())
                return p;

            Thread::sleep (2);
        }
    }

    i32 openPipe (b8 isInput, u32 timeoutEnd)
    {
        auto& pipe = isInput ? pipeIn : pipeOut;
        const auto flags = (isInput ? O_RDWR : O_WRONLY) | O_NONBLOCK;

        const Txt& pipeName = isInput ? (createdPipe ? pipeInName : pipeOutName)
                                         : (createdPipe ? pipeOutName : pipeInName);

        return pipe.get ([this, &pipeName, &flags, &timeoutEnd]
        {
            return openPipe (pipeName, flags, timeoutEnd);
        });
    }

    static z0 waitForInput (i32 handle, i32 timeoutMsecs) noexcept
    {
        pollfd pfd { handle, POLLIN, 0 };
        poll (&pfd, 1, timeoutMsecs);
    }

    static z0 waitToWrite (i32 handle, i32 timeoutMsecs) noexcept
    {
        pollfd pfd { handle, POLLOUT, 0 };
        poll (&pfd, 1, timeoutMsecs);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

z0 NamedPipe::close()
{
    {
        const ScopedReadLock sl (lock);

        if (pimpl != nullptr)
        {
            pimpl->stopReadOperation = true;

            const t8 buffer[] { 0 };
            [[maybe_unused]] const auto done = ::write (pimpl->pipeIn.get(), buffer, numElementsInArray (buffer));
        }
    }

    {
        const ScopedWriteLock sl (lock);
        pimpl.reset();
    }
}

b8 NamedPipe::openInternal (const Txt& pipeName, b8 createPipe, b8 mustNotExist)
{
   #if DRX_IOS
    pimpl.reset (new Pimpl (File::getSpecialLocation (File::tempDirectory)
                             .getChildFile (File::createLegalFileName (pipeName)).getFullPathName(), createPipe));
   #else
    auto file = pipeName;

    if (! File::isAbsolutePath (file))
        file = "/tmp/" + File::createLegalFileName (file);

    pimpl.reset (new Pimpl (file, createPipe));
   #endif

    if (createPipe && ! pimpl->createFifos (mustNotExist))
    {
        pimpl.reset();
        return false;
    }

    if (! pimpl->connect (200))
    {
        pimpl.reset();
        return false;
    }

    return true;
}

i32 NamedPipe::read (uk destBuffer, i32 maxBytesToRead, i32 timeOutMilliseconds)
{
    ScopedReadLock sl (lock);
    return pimpl != nullptr ? pimpl->read (static_cast<tuk> (destBuffer), maxBytesToRead, timeOutMilliseconds) : -1;
}

i32 NamedPipe::write (ukk sourceBuffer, i32 numBytesToWrite, i32 timeOutMilliseconds)
{
    ScopedReadLock sl (lock);
    return pimpl != nullptr ? pimpl->write (static_cast<tukk> (sourceBuffer), numBytesToWrite, timeOutMilliseconds) : -1;
}

#endif

} // namespace drx
