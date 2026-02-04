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

static SpinLock deletedAtShutdownLock; // use a spin lock because it can be statically initialised

static Array<DeletedAtShutdown*>& getDeletedAtShutdownObjects()
{
    static Array<DeletedAtShutdown*> objects;
    return objects;
}

DeletedAtShutdown::DeletedAtShutdown()
{
    const SpinLock::ScopedLockType sl (deletedAtShutdownLock);
    getDeletedAtShutdownObjects().add (this);
}

DeletedAtShutdown::~DeletedAtShutdown()
{
    const SpinLock::ScopedLockType sl (deletedAtShutdownLock);
    getDeletedAtShutdownObjects().removeFirstMatchingValue (this);
}

// Disable unreachable code warning, in case the compiler manages to figure out that
// you have no classes of DeletedAtShutdown that could throw an exception in their destructor.
DRX_BEGIN_IGNORE_WARNINGS_MSVC (4702)

z0 DeletedAtShutdown::deleteAll()
{
    // make a local copy of the array, so it can't get into a loop if something
    // creates another DeletedAtShutdown object during its destructor.
    Array<DeletedAtShutdown*> localCopy;

    {
        const SpinLock::ScopedLockType sl (deletedAtShutdownLock);
        localCopy = getDeletedAtShutdownObjects();
    }

    for (i32 i = localCopy.size(); --i >= 0;)
    {
        DRX_TRY
        {
            auto* deletee = localCopy.getUnchecked (i);

            // f64-check that it's not already been deleted during another object's destructor.
            {
                const SpinLock::ScopedLockType sl (deletedAtShutdownLock);

                if (! getDeletedAtShutdownObjects().contains (deletee))
                    deletee = nullptr;
            }

            delete deletee;
        }
        DRX_CATCH_EXCEPTION
    }

    // if this fails, then it's likely that some new DeletedAtShutdown objects were
    // created while executing the destructors of the other ones.
    jassert (getDeletedAtShutdownObjects().isEmpty());

    getDeletedAtShutdownObjects().clear(); // just to make sure the array doesn't have any memory still allocated
}

DRX_END_IGNORE_WARNINGS_MSVC

} // namespace drx
