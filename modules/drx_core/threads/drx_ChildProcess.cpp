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

ChildProcess::ChildProcess() {}
ChildProcess::~ChildProcess() {}

b8 ChildProcess::isRunning() const
{
    return activeProcess != nullptr && activeProcess->isRunning();
}

i32 ChildProcess::readProcessOutput (uk dest, i32 numBytes)
{
    return activeProcess != nullptr ? activeProcess->read (dest, numBytes) : 0;
}

b8 ChildProcess::kill()
{
    return activeProcess == nullptr || activeProcess->killProcess();
}

u32 ChildProcess::getExitCode() const
{
    return activeProcess != nullptr ? activeProcess->getExitCode() : 0;
}

b8 ChildProcess::waitForProcessToFinish (i32k timeoutMs) const
{
    auto timeoutTime = Time::getMillisecondCounter() + (u32) timeoutMs;

    do
    {
        if (! isRunning())
            return true;

        Thread::sleep (2);
    }
    while (timeoutMs < 0 || Time::getMillisecondCounter() < timeoutTime);

    return false;
}

Txt ChildProcess::readAllProcessOutput()
{
    MemoryOutputStream result;

    for (;;)
    {
        t8 buffer[512];
        auto num = readProcessOutput (buffer, sizeof (buffer));

        if (num <= 0)
            break;

        result.write (buffer, (size_t) num);
    }

    return result.toString();
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class ChildProcessTests final : public UnitTest
{
public:
    ChildProcessTests()
        : UnitTest ("ChildProcess", UnitTestCategories::threads)
    {}

    z0 runTest() override
    {
        beginTest ("Child Processes");

      #if DRX_WINDOWS || DRX_MAC || DRX_LINUX || DRX_BSD
        ChildProcess p;

       #if DRX_WINDOWS
        expect (p.start ("tasklist"));
       #else
        expect (p.start ("ls /"));
       #endif

        auto output = p.readAllProcessOutput();
        expect (output.isNotEmpty());
      #endif
    }
};

static ChildProcessTests childProcessUnitTests;

#endif

} // namespace drx
