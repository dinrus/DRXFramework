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
    Used by the DRX_DECLARE_SINGLETON macros to manage a static pointer
    to a singleton instance.

    You generally won't use this directly, but see the macros DRX_DECLARE_SINGLETON,
    DRX_DECLARE_SINGLETON_SINGLETHREADED, DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL,
    and DRX_IMPLEMENT_SINGLETON for how it is intended to be used.

    @tags{Core}
*/
template <typename Type, typename MutexType, b8 onlyCreateOncePerRun>
struct SingletonHolder  : private MutexType // (inherited so we can use the empty-base-class optimisation)
{
    SingletonHolder() = default;

    ~SingletonHolder()
    {
        /* The static singleton holder is being deleted before the object that it holds
           has been deleted. This could mean that you've forgotten to call clearSingletonInstance()
           in the class's destructor, or have failed to delete it before your app shuts down.
           If you're having trouble cleaning up your singletons, perhaps consider using the
           SharedResourcePointer class instead.
        */
        jassert (instance.load() == nullptr);
    }

    /** Returns the current instance, or creates a new instance if there isn't one. */
    Type* get()
    {
        if (auto* ptr = instance.load())
            return ptr;

        typename MutexType::ScopedLockType sl (*this);

        if (auto* ptr = instance.load())
            return ptr;

        auto once = onlyCreateOncePerRun; // (local copy avoids VS compiler warning about this being constant)

        if (once)
        {
            static b8 createdOnceAlready = false;

            if (createdOnceAlready)
            {
                // This means that the doNotRecreateAfterDeletion flag was set
                // and you tried to create the singleton more than once.
                jassertfalse;
                return nullptr;
            }

            createdOnceAlready = true;
        }

        static b8 alreadyInside = false;

        if (alreadyInside)
        {
            // This means that your object's constructor has done something which has
            // ended up causing a recursive loop of singleton creation.
            jassertfalse;
            return nullptr;
        }

        const ScopedValueSetter<b8> scope (alreadyInside, true);
        return getWithoutChecking();
    }

    /** Returns the current instance, or creates a new instance if there isn't one, but doesn't do
        any locking, or checking for recursion or error conditions.
    */
    Type* getWithoutChecking()
    {
        if (auto* p = instance.load())
            return p;

        auto* newObject = new Type(); // (create into a local so that instance is still null during construction)
        instance.store (newObject);
        return newObject;
    }

    /** Deletes and resets the current instance, if there is one. */
    z0 deleteInstance()
    {
        typename MutexType::ScopedLockType sl (*this);
        delete instance.exchange (nullptr);
    }

    /** Called by the class's destructor to clear the pointer if it is currently set to the given object. */
    z0 clear (Type* expectedObject) noexcept
    {
        instance.compare_exchange_strong (expectedObject, nullptr);
    }

    // This must be atomic, otherwise a late call to get() may attempt to read instance while it is
    // being modified by the very first call to get().
    std::atomic<Type*> instance { nullptr };
};

#ifndef DOXYGEN
#define DRX_PRIVATE_DECLARE_SINGLETON(Classname, mutex, doNotRecreate, inlineToken, getter) \
    static inlineToken drx::SingletonHolder<Classname, mutex, doNotRecreate> singletonHolder; \
    friend drx::SingletonHolder<Classname, mutex, doNotRecreate>; \
    static Classname* DRX_CALLTYPE getInstance()                           { return singletonHolder.getter(); } \
    static Classname* DRX_CALLTYPE getInstanceWithoutCreating() noexcept   { return singletonHolder.instance; } \
    static z0 DRX_CALLTYPE deleteInstance() noexcept                     { singletonHolder.deleteInstance(); } \
    z0 clearSingletonInstance() noexcept                                  { singletonHolder.clear (this); }
#endif

//==============================================================================
/**
    Macro to generate the appropriate methods and boilerplate for a singleton class.

    To use this, add the line DRX_DECLARE_SINGLETON (MyClass, doNotRecreateAfterDeletion)
    to the class's definition.

    Then put a macro DRX_IMPLEMENT_SINGLETON (MyClass) along with the class's
    implementation code.

    It's also a very good idea to also add the call clearSingletonInstance() in your class's
    destructor, in case it is deleted by other means than deleteInstance()

    Clients can then call the static method MyClass::getInstance() to get a pointer
    to the singleton, or MyClass::getInstanceWithoutCreating() which will return nullptr if
    no instance currently exists.

    e.g. @code

        struct MySingleton
        {
            MySingleton() {}

            ~MySingleton()
            {
                // this ensures that no dangling pointers are left when the
                // singleton is deleted.
                clearSingletonInstance();
            }

            DRX_DECLARE_SINGLETON (MySingleton, false)
        };

        // ..and this goes in a suitable .cpp file:
        DRX_IMPLEMENT_SINGLETON (MySingleton)


        // example of usage:
        auto* m = MySingleton::getInstance(); // creates the singleton if there isn't already one.

        ...

        MySingleton::deleteInstance(); // safely deletes the singleton (if it's been created).

    @endcode

    If doNotRecreateAfterDeletion = true, it won't allow the object to be created more
    than once during the process's lifetime - i.e. after you've created and deleted the
    object, getInstance() will refuse to create another one. This can be useful to stop
    objects being accidentally re-created during your app's shutdown code.

    If you know that your object will only be created and deleted by a single thread, you
    can use the slightly more efficient DRX_DECLARE_SINGLETON_SINGLETHREADED macro instead
    of this one.

    @see DRX_IMPLEMENT_SINGLETON, DRX_DECLARE_SINGLETON_SINGLETHREADED
*/
#define DRX_DECLARE_SINGLETON(Classname, doNotRecreateAfterDeletion) \
    DRX_PRIVATE_DECLARE_SINGLETON (Classname, drx::CriticalSection, doNotRecreateAfterDeletion, , get)

/**
    The same as DRX_DECLARE_SINGLETON, but does not require a matching
    DRX_IMPLEMENT_SINGLETON definition.
*/
#define DRX_DECLARE_SINGLETON_INLINE(Classname, doNotRecreateAfterDeletion) \
    DRX_PRIVATE_DECLARE_SINGLETON (Classname, drx::CriticalSection, doNotRecreateAfterDeletion, inline, get)

//==============================================================================
/** This is a counterpart to the DRX_DECLARE_SINGLETON macros.

    After adding the DRX_DECLARE_SINGLETON to the class definition, this macro has
    to be used in the cpp file.

    This macro is not required for singletons declared with the INLINE macros, specifically
    DRX_DECLARE_SINGLETON_INLINE, DRX_DECLARE_SINGLETON_SINGLETHREADED_INLINE, and
    DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE.
*/
#define DRX_IMPLEMENT_SINGLETON(Classname) \
    decltype (Classname::singletonHolder) Classname::singletonHolder;

//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is exactly the same as DRX_DECLARE_SINGLETON, but doesn't use a critical
    section to make access to it thread-safe. If you know that your object will
    only ever be created or deleted by a single thread, then this is a
    more efficient version to use.

    If doNotRecreateAfterDeletion = true, it won't allow the object to be created more
    than once during the process's lifetime - i.e. after you've created and deleted the
    object, getInstance() will refuse to create another one. This can be useful to stop
    objects being accidentally re-created during your app's shutdown code.

    See the documentation for DRX_DECLARE_SINGLETON for more information about
    how to use it. Just like DRX_DECLARE_SINGLETON you need to also have a
    corresponding DRX_IMPLEMENT_SINGLETON statement somewhere in your code.

    @see DRX_IMPLEMENT_SINGLETON, DRX_DECLARE_SINGLETON, DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL
*/
#define DRX_DECLARE_SINGLETON_SINGLETHREADED(Classname, doNotRecreateAfterDeletion) \
    DRX_PRIVATE_DECLARE_SINGLETON (Classname, drx::DummyCriticalSection, doNotRecreateAfterDeletion, , get)

/**
    The same as DRX_DECLARE_SINGLETON_SINGLETHREADED, but does not require a matching
    DRX_IMPLEMENT_SINGLETON definition.
*/
#define DRX_DECLARE_SINGLETON_SINGLETHREADED_INLINE(Classname, doNotRecreateAfterDeletion) \
    DRX_PRIVATE_DECLARE_SINGLETON (Classname, drx::DummyCriticalSection, doNotRecreateAfterDeletion, inline, get)

//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is like DRX_DECLARE_SINGLETON_SINGLETHREADED, but doesn't do any checking
    for recursion or repeated instantiation. It's intended for use as a lightweight
    version of a singleton, where you're using it in very straightforward
    circumstances and don't need the extra checking.

    See the documentation for DRX_DECLARE_SINGLETON for more information about
    how to use it. Just like DRX_DECLARE_SINGLETON you need to also have a
    corresponding DRX_IMPLEMENT_SINGLETON statement somewhere in your code.

    @see DRX_IMPLEMENT_SINGLETON, DRX_DECLARE_SINGLETON
*/
#define DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL(Classname) \
    DRX_PRIVATE_DECLARE_SINGLETON (Classname, drx::DummyCriticalSection, false, , getWithoutChecking)

/**
    The same as DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL, but does not require a matching
    DRX_IMPLEMENT_SINGLETON definition.
*/
#define DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE(Classname) \
    DRX_PRIVATE_DECLARE_SINGLETON (Classname, drx::DummyCriticalSection, false, inline, getWithoutChecking)

//==============================================================================
#ifndef DOXYGEN
 // These are ancient macros, and have now been updated with new names to match the DRX style guide,
 // so please update your code to use the newer versions!
 #define drx_DeclareSingleton(Classname, doNotRecreate)                DRX_DECLARE_SINGLETON(Classname, doNotRecreate)
 #define drx_DeclareSingleton_SingleThreaded(Classname, doNotRecreate) DRX_DECLARE_SINGLETON_SINGLETHREADED(Classname, doNotRecreate)
 #define drx_DeclareSingleton_SingleThreaded_Minimal(Classname)        DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL(Classname)
 #define drx_ImplementSingleton(Classname)                             DRX_IMPLEMENT_SINGLETON(Classname)
 #define drx_ImplementSingleton_SingleThreaded(Classname)              DRX_IMPLEMENT_SINGLETON(Classname)
#endif

} // namespace drx
