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

class AllocationHooks
{
public:
    struct Listener
    {
        virtual ~Listener() noexcept = default;
        virtual z0 newOrDeleteCalled() noexcept = 0;
    };

    z0 addListener    (Listener* l)          { listenerList.add (l); }
    z0 removeListener (Listener* l) noexcept { listenerList.remove (l); }

private:
    friend z0 notifyAllocationHooksForThread();
    LightweightListenerList<Listener> listenerList;
};

//==============================================================================
/** Scoped checker which will cause a unit test failure if any new/delete calls
    are made during the lifetime of the UnitTestAllocationChecker.
*/
class UnitTestAllocationChecker  : private AllocationHooks::Listener
{
public:
    /** Create a checker which will log a failure to the passed test if
        any calls to new/delete are made.

        Remember to call `UnitTest::beginTest` before constructing this checker!
    */
    explicit UnitTestAllocationChecker (UnitTest& test);

    /** Will add a failure to the test if the number of new/delete calls during
        this object's lifetime was greater than zero.
    */
    ~UnitTestAllocationChecker() noexcept override;

private:
    z0 newOrDeleteCalled() noexcept override;

    UnitTest& unitTest;
    size_t calls = 0;
};

}

#endif
