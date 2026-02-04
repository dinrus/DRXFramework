//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/thread/include/flock.h
// Created by  : Steinberg, 1995
// Description : locks
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------------
/** @file base/thread/include/flock.h
    Locks. */
/** @defgroup baseLocks Locks */
//----------------------------------------------------------------------------------
#pragma once

#include "base/source/fobject.h"
#include "pluginterfaces/base/ftypes.h"

#if SMTG_PTHREADS
#include <pthread.h>

#elif SMTG_OS_WINDOWS
struct CRITSECT							// CRITICAL_SECTION
{
	uk DebugInfo;					// PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
	Steinberg::i32 LockCount;			// LONG LockCount;
	Steinberg::i32 RecursionCount;	// LONG RecursionCount;
	uk OwningThread;					// HANDLE OwningThread
	uk LockSemaphore;				// HANDLE LockSemaphore
	Steinberg::i32 SpinCount;			// ULONG_PTR SpinCount
};
#endif

namespace Steinberg {
namespace Base {
namespace Thread {

//------------------------------------------------------------------------
/** Lock interface declaration.
@ingroup baseLocks	*/
//------------------------------------------------------------------------
struct ILock
{
//------------------------------------------------------------------------
	virtual ~ILock () {}

	/** Enables lock. */
	virtual z0 lock () = 0;

	/** Disables lock. */
	virtual z0 unlock () = 0;

	/** Tries to disable lock. */
	virtual b8 trylock () = 0;
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/**	FLock declaration.
@ingroup baseLocks	*/
//------------------------------------------------------------------------
class FLock : public ILock
{
public:
//------------------------------------------------------------------------

	/** Lock constructor.
	 *  @param name lock name
	 */
	FLock (const char8* name = "FLock");

	/** Lock destructor. */
	~FLock () SMTG_OVERRIDE;

	//-- ILock -----------------------------------------------------------
	z0 lock () SMTG_OVERRIDE;
	z0 unlock () SMTG_OVERRIDE;
	b8 trylock () SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:
#if SMTG_PTHREADS
	pthread_mutex_t mutex; ///< Mutex object

#elif SMTG_OS_WINDOWS
	CRITSECT section; ///< Critical section object
#endif
};

//------------------------------------------------------------------------
/**	FLockObj declaration. Reference counted lock
@ingroup baseLocks	*/
//------------------------------------------------------------------------
class FLockObject : public FObject, public FLock
{
public:
	OBJ_METHODS (FLockObject, FObject)
};

//------------------------------------------------------------------------
/**	FGuard - automatic object for locks.
@ingroup baseLocks	*/
//------------------------------------------------------------------------
class FGuard
{
public:
//------------------------------------------------------------------------

	/** FGuard constructor.
	 *  @param _lock guard this lock
	 */
	FGuard (ILock& _lock) : lock (_lock) { lock.lock (); }

	/** FGuard destructor. */
	~FGuard () { lock.unlock (); }

//------------------------------------------------------------------------
private:
	ILock& lock; ///< guarded lock
};

//------------------------------------------------------------------------
/**	Conditional Guard - Locks only if valid lock is passed.
@ingroup baseLocks	*/
//------------------------------------------------------------------------
class FConditionalGuard
{
public:
//------------------------------------------------------------------------

	/** FConditionGuard constructor.
	 *  @param _lock guard this lock
	 */
	FConditionalGuard (FLock* _lock) : lock (_lock)
	{
		if (lock)
			lock->lock ();
	}

	/** FConditionGuard destructor. */
	~FConditionalGuard ()
	{
		if (lock)
			lock->unlock ();
	}

//------------------------------------------------------------------------
private:
	FLock* lock; ///< guarded lock
};

//------------------------------------------------------------------------
} // namespace Thread
} // namespace Base
} // namespace Steinberg
