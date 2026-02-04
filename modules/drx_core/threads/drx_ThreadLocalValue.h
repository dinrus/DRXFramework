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
    Provides cross-platform support for thread-local objects.

    This class holds an internal list of objects of the templated type, keeping
    an instance for each thread that requests one. The first time a thread attempts
    to access its value, an object is created and added to the list for that thread.

    Typically, you'll probably want to create a static instance of a ThreadLocalValue
    object, or hold one within a singleton.

    The templated class for your value must be a primitive type, or a simple POD struct.

    When a thread no longer needs to use its value, it can call releaseCurrentThreadStorage()
    to allow the storage to be re-used by another thread. If a thread exits without calling
    this method, the object storage will be left allocated until the ThreadLocalValue object
    is deleted.

    @tags{Core}
*/
template <typename Type>
class ThreadLocalValue
{
public:
    /** */
    ThreadLocalValue() = default;

    /** Destructor.
        When this object is deleted, all the value objects for all threads will be deleted.
    */
    ~ThreadLocalValue()
    {
        for (auto* o = first.get(); o != nullptr;)
        {
            auto* next = o->next;
            delete o;
            o = next;
        }
    }

    /** Returns a reference to this thread's instance of the value.
        Note that the first time a thread tries to access the value, an instance of the
        value object will be created - so if your value's class has a non-trivial
        constructor, be aware that this method could invoke it.
    */
    Type& operator*() const noexcept                        { return get(); }

    /** Returns a pointer to this thread's instance of the value.
        Note that the first time a thread tries to access the value, an instance of the
        value object will be created - so if your value's class has a non-trivial
        constructor, be aware that this method could invoke it.
    */
    operator Type*() const noexcept                         { return &get(); }

    /** Accesses a method or field of the value object.
        Note that the first time a thread tries to access the value, an instance of the
        value object will be created - so if your value's class has a non-trivial
        constructor, be aware that this method could invoke it.
    */
    Type* operator->() const noexcept                       { return &get(); }

    /** Assigns a new value to the thread-local object. */
    ThreadLocalValue& operator= (const Type& newValue)      { get() = newValue; return *this; }

    /** Returns a reference to this thread's instance of the value.
        Note that the first time a thread tries to access the value, an instance of the
        value object will be created - so if your value's class has a non-trivial
        constructor, be aware that this method could invoke it.
    */
    Type& get() const noexcept
    {
        auto threadId = Thread::getCurrentThreadId();
        ObjectHolder* o = nullptr;

        for (o = first.get(); o != nullptr; o = o->next)
            if (o->threadId.get() == threadId)
                return o->object;

        for (o = first.get(); o != nullptr; o = o->next)
            if (o->threadId.compareAndSetBool (threadId, nullptr))
                break;

        if (o != nullptr)
            o->object = Type();
        else
            for (o = new ObjectHolder (threadId, first.get());
                 ! first.compareAndSetBool (o, o->next);
                 o->next = first.get());

        return o->object;
    }

    /** Called by a thread before it terminates, to allow this class to release
        any storage associated with the thread.
    */
    z0 releaseCurrentThreadStorage()
    {
        auto threadId = Thread::getCurrentThreadId();

        for (auto* o = first.get(); o != nullptr; o = o->next)
            if (o->threadId.compareAndSetBool (nullptr, threadId))
                return;
    }

private:
    //==============================================================================
    struct ObjectHolder
    {
        ObjectHolder (Thread::ThreadID idToUse, ObjectHolder* n) : threadId (idToUse), next (n), object() {}

        Atomic<Thread::ThreadID> threadId;
        ObjectHolder* next;
        Type object;

        DRX_DECLARE_NON_COPYABLE (ObjectHolder)
    };

    mutable Atomic<ObjectHolder*> first;

    DRX_DECLARE_NON_COPYABLE (ThreadLocalValue)
};

} // namespace drx
