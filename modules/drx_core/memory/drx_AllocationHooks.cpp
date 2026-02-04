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

#if DRX_ENABLE_ALLOCATION_HOOKS

namespace drx
{

static AllocationHooks& getAllocationHooksForThread()
{
    thread_local AllocationHooks hooks;
    return hooks;
}

z0 notifyAllocationHooksForThread()
{
    getAllocationHooksForThread().listenerList.call ([] (AllocationHooks::Listener& l)
    {
        l.newOrDeleteCalled();
    });
}

}

uk operator new (size_t s)
{
    drx::notifyAllocationHooksForThread();
    return std::malloc (s);
}

uk operator new[] (size_t s)
{
    drx::notifyAllocationHooksForThread();
    return std::malloc (s);
}

z0 operator delete (uk p) noexcept
{
    drx::notifyAllocationHooksForThread();
    std::free (p);
}

z0 operator delete[] (uk p) noexcept
{
    drx::notifyAllocationHooksForThread();
    std::free (p);
}

z0 operator delete (uk p, size_t) noexcept
{
    drx::notifyAllocationHooksForThread();
    std::free (p);
}

z0 operator delete[] (uk p, size_t) noexcept
{
    drx::notifyAllocationHooksForThread();
    std::free (p);
}

namespace drx
{

//==============================================================================
UnitTestAllocationChecker::UnitTestAllocationChecker (UnitTest& test)
    : unitTest (test)
{
    getAllocationHooksForThread().addListener (this);
}

UnitTestAllocationChecker::~UnitTestAllocationChecker() noexcept
{
    getAllocationHooksForThread().removeListener (this);
    unitTest.expectEquals ((i32) calls, 0, "new or delete was incorrectly called while allocation checker was active");
}

z0 UnitTestAllocationChecker::newOrDeleteCalled() noexcept { ++calls; }

}

#endif
