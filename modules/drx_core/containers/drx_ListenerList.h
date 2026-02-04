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
    Holds a set of objects and can invoke a member function callback on each object in the set with
    a single call.

    It is safe to add listeners, remove listeners, clear the listeners, and even delete the
    ListenerList itself during any listener callback. If you don't need these extra guarantees
    consider using a LightweightListenerList instead.

    If a Listener is added during a callback, it is guaranteed not to be called in the same
    iteration.

    If a Listener is removed during a callback, it is guaranteed not to be called if it hasn't
    already been called.

    If the ListenerList is cleared or deleted during a callback, it is guaranteed that no more
    listeners will be called.

    It is NOT safe to make concurrent calls to the listeners without a mutex. If you need this
    functionality, either use a LightweightListenerList or a ThreadSafeListenerList.

    When calling listeners the iteration can be escaped early by using a "BailOutChecker".
    A BailOutChecker is a type that has a public member function with the following signature:
    @code b8 shouldBailOut() const @endcode
    This function will be called before making a call to each listener.
    For an example see the DummyBailOutChecker.

    @see LightweightListenerList, ThreadSafeListenerList

    @tags{Core}
*/
template <typename ListenerClass,
          typename ArrayType = Array<ListenerClass*>>
class ListenerList
{
public:
    //==============================================================================
    /** Creates an empty list. */
    ListenerList() = default;

    /** Destructor. */
    ~ListenerList() { clear(); }

    //==============================================================================
    /** Adds a listener to the list.
        A listener can only be added once, so if the listener is already in the list, this method
        has no effect.

        If a Listener is added during a callback, it is guaranteed not to be called in the same
        iteration.

        @see remove
    */
    z0 add (ListenerClass* listenerToAdd)
    {
        initialiseIfNeeded();

        if (listenerToAdd != nullptr)
            listeners->addIfNotAlreadyThere (listenerToAdd);
        else
            jassertfalse; // Listeners can't be null pointers!
    }

    /** Removes a listener from the list.
        If the listener wasn't in the list, this has no effect.

        If a Listener is removed during a callback, it is guaranteed not to be called if it hasn't
        already been called.
    */
    z0 remove (ListenerClass* listenerToRemove)
    {
        jassert (listenerToRemove != nullptr); // Listeners can't be null pointers!

        if (! initialised())
            return;

        const ScopedLockType lock (listeners->getLock());

        if (const auto index = listeners->removeFirstMatchingValue (listenerToRemove); index >= 0)
        {
            for (auto* it : *iterators)
            {
                if (index < it->end)
                    --it->end;

                if (index <= it->index)
                    --it->index;
            }
        }
    }

    /** Adds a listener that will be automatically removed again when the Guard is destroyed.

        Be very careful to ensure that the ErasedScopeGuard is destroyed or released before the
        ListenerList is destroyed, otherwise the ErasedScopeGuard may attempt to dereference a
        dangling pointer when it is destroyed, which will result in a crash.
    */
    [[nodiscard]] ErasedScopeGuard addScoped (ListenerClass& listenerToAdd)
    {
        add (&listenerToAdd);
        return ErasedScopeGuard { [this, &listenerToAdd] { remove (&listenerToAdd); } };
    }

    /** Returns the number of registered listeners. */
    [[nodiscard]] i32 size() const noexcept                                { return ! initialised() ? 0 : listeners->size(); }

    /** Возвращает true, если no listeners are registered, false otherwise. */
    [[nodiscard]] b8 isEmpty() const noexcept                            { return ! initialised() || listeners->isEmpty(); }

    /** Clears the list.

        If the ListenerList is cleared during a callback, it is guaranteed that no more
        listeners will be called.
    */
    z0 clear()
    {
        if (! initialised())
            return;

        const ScopedLockType lock { listeners->getLock() };

        listeners->clear();

        for (auto* it : *iterators)
            it->end = 0;
    }

    /** Возвращает true, если the specified listener has been added to the list. */
    [[nodiscard]] b8 contains (ListenerClass* listener) const noexcept
    {
        return initialised()
            && listeners->contains (listener);
    }

    /** Returns the raw array of listeners.

        Any attempt to mutate the array may result in undefined behaviour.

        If the array uses a mutex/CriticalSection, reading from the array without first obtaining
        the lock may potentially result in undefined behaviour.

        @see add, remove, clear, contains
    */
    [[nodiscard]] const ArrayType& getListeners() const noexcept
    {
        const_cast<ListenerList*> (this)->initialiseIfNeeded();
        return *listeners;
    }

    //==============================================================================
    /** Calls an invokable object for each listener in the list. */
    template <typename Callback>
    z0 call (Callback&& callback)
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the listener specified
        by listenerToExclude.
    */
    template <typename Callback>
    z0 callExcluding (ListenerClass* listenerToExclude, Callback&& callback)
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));

    }

    /** Calls an invokable object for each listener in the list, additionally checking the bail-out
        checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    z0 callChecked (const BailOutCheckerType& bailOutChecker, Callback&& callback)
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the listener specified
        by listenerToExclude, additionally checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    z0 callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               Callback&& callback)
    {
       #if DRX_ASSERTIONS_ENABLED_OR_LOGGED
        // Keep a reference to the mutex to protect against the case where this list gets deleted
        // during a callback.
        auto localMutexPtr = callCheckedExcludingMutex;
        const ScopedTryLock callCheckedExcludingLock (*localMutexPtr);

        // If you hit this assertion it means you're trying to call the listeners from multiple
        // threads concurrently. If you need to do this either use a LightweightListenerList, for a
        // lock free option, or a ThreadSafeListenerList if you also need the extra guarantees
        // provided by ListenerList. See the class descriptions for more details.
        jassert (callCheckedExcludingLock.isLocked());
       #endif

        if (! initialised())
            return;

        const auto localListeners = listeners;
        const ScopedLockType lock { localListeners->getLock() };

        Iterator it{};
        it.end = localListeners->size();

        iterators->push_back (&it);

        const ScopeGuard scope { [i = iterators, &it]
        {
            i->erase (std::remove (i->begin(), i->end(), &it), i->end());
        } };

        for (; it.index < it.end; ++it.index)
        {
            if (bailOutChecker.shouldBailOut())
                return;

            auto* listener = localListeners->getUnchecked (it.index);

            if (listener == listenerToExclude)
                continue;

            callback (*listener);
        }
    }

    //==============================================================================
    /** Calls a specific listener method for each listener in the list. */
    template <typename... MethodArgs, typename... Args>
    z0 call (z0 (ListenerClass::*callbackFunction) (MethodArgs...), Args&&... args)
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, except for the listener
        specified by listenerToExclude.
    */
    template <typename... MethodArgs, typename... Args>
    z0 callExcluding (ListenerClass* listenerToExclude,
                        z0 (ListenerClass::*callbackFunction) (MethodArgs...),
                        Args&&... args)
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list,
        additionally checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    z0 callChecked (const BailOutCheckerType& bailOutChecker,
                      z0 (ListenerClass::*callbackFunction) (MethodArgs...),
                      Args&&... args)
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, except
        for the listener specified by listenerToExclude, additionally checking
        the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    z0 callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               z0 (ListenerClass::*callbackFunction) (MethodArgs...),
                               Args&&... args)
    {
        callCheckedExcluding (listenerToExclude, bailOutChecker, [&] (ListenerClass& l)
        {
            (l.*callbackFunction) (args...);
        });
    }

    //==============================================================================
    /** A dummy bail-out checker that always returns false.
        See the class description for info about writing a bail-out checker.
    */
    struct DummyBailOutChecker
    {
        constexpr b8 shouldBailOut() const noexcept { return false; }
    };

    //==============================================================================
    using ThisType      = ListenerList<ListenerClass, ArrayType>;
    using ListenerType  = ListenerClass;

private:
    //==============================================================================
    using ScopedLockType = typename ArrayType::ScopedLockType;

    //==============================================================================
    using SharedListeners = std::shared_ptr<ArrayType>;
    SharedListeners listeners;

    struct Iterator
    {
        i32 index{};
        i32 end{};
    };

    using SafeIterators = std::vector<Iterator*>;
    using SharedIterators = std::shared_ptr<SafeIterators>;
    SharedIterators iterators;

    enum class State
    {
        uninitialised,
        initialising,
        initialised
    };

    std::atomic<State> state { State::uninitialised };

    inline b8 initialised() const noexcept { return state == State::initialised; }

    inline z0 initialiseIfNeeded() noexcept
    {
        if (initialised())
            return;

        auto expected = State::uninitialised;

        if (state.compare_exchange_strong (expected, State::initialising))
        {
            static_assert (std::is_nothrow_constructible_v<ArrayType>,
                           "Any ListenerList ArrayType must have a noexcept default constructor");

            static_assert (std::is_nothrow_constructible_v<SafeIterators>,
                           "Please notify the DRX team if you encounter this assertion");

            listeners = std::make_shared<ArrayType>();
            iterators = std::make_shared<SafeIterators>();
            state = State::initialised;
            return;
        }

        while (! initialised())
            std::this_thread::yield();
    }

   #if DRX_ASSERTIONS_ENABLED_OR_LOGGED
    // using a shared_ptr helps keep the size of this class down to prevent excessive stack sizes
    // due to objects that contain a ListenerList being created on the stack
    std::shared_ptr<CriticalSection> callCheckedExcludingMutex = std::make_shared<CriticalSection>();
   #endif

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE (ListenerList)
};

//==============================================================================
/**
    A thread safe version of the ListenerList class.

    @see ListenerList, LightweightListenerList

    @tags{Core}
*/
template <typename ListenerClass>
using ThreadSafeListenerList = ListenerList<ListenerClass, Array<ListenerClass*, CriticalSection>>;

//==============================================================================
/**
    A lightweight version of the ListenerList that doesn't provide any guarantees when mutating the
    list from a callback, but allows callbacks to be triggered concurrently without a mutex.

    @see ListenerList, ThreadSafeListenerList

    @tags{Core}
*/
template <typename ListenerClass>
class LightweightListenerList
{
public:
    //==============================================================================
    /** Creates an empty list. */
    LightweightListenerList() = default;

    /** Destructor. */
    ~LightweightListenerList()
    {
        // If you hit this jassert it means you're trying to delete the list while iterating through
        // the listeners! If you need to handle this situation gracefully use a ListenerList or
        // ThreadSafeListenerList.
        jassert (numCallsInProgress == 0);
    }

    //==============================================================================
    /** Adds a listener to the list.
        A listener can only be added once, so if the listener is already in the list, this method
        has no effect.

        If you need to add a Listener during a callback, use the ListenerList type.

        @see remove
    */
    z0 add (ListenerClass* listenerToAdd)
    {
        // If you hit this jassert it means you're trying to add a listener while iterating through
        // the listeners! If you need to handle this situation gracefully use a ListenerList or
        // ThreadSafeListenerList.
        jassert (numCallsInProgress == 0);

        if (listenerToAdd != nullptr)
            listeners.addIfNotAlreadyThere (listenerToAdd);
        else
            jassertfalse; // Listeners can't be null pointers!
    }

    /** Removes a listener from the list.
        If the listener wasn't in the list, this has no effect.

        If you need to remove a Listener during a callback, use the ListenerList type.
    */
    z0 remove (ListenerClass* listenerToRemove)
    {
        // If you hit this jassert it means you're trying to remove a listener while iterating
        // through the listeners! If you need to handle this situation gracefully use a ListenerList
        // or ThreadSafeListenerList.
        jassert (numCallsInProgress == 0);

        jassert (listenerToRemove != nullptr); // Listeners can't be null pointers!

        listeners.removeFirstMatchingValue (listenerToRemove);
    }

    /** Adds a listener that will be automatically removed when the Guard is destroyed.

        Be very careful to ensure that the ErasedScopeGuard is destroyed or released before the
        ListenerList is destroyed, otherwise the ErasedScopeGuard may attempt to dereference a
        dangling pointer when it is destroyed, which will result in a crash.
    */
    [[nodiscard]] ErasedScopeGuard addScoped (ListenerClass& listenerToAdd)
    {
        add (&listenerToAdd);
        return ErasedScopeGuard { [this, &listenerToAdd] { remove (&listenerToAdd); } };
    }

    /** Returns the number of registered listeners. */
    [[nodiscard]] i32 size() const noexcept { return listeners.size(); }

    /** Возвращает true, если no listeners are registered, false otherwise. */
    [[nodiscard]] b8 isEmpty() const noexcept { return listeners.isEmpty(); }

    /** Clears the list.

        If you need to clear the list during a callback, use the ListenerList type.
    */
    z0 clear()
    {
        // If you hit this jassert it means you're trying to clear the listener list while iterating
        // through the listeners! If you need to handle this situation gracefully use a ListenerList
        // or ThreadSafeListenerList.
        jassert (numCallsInProgress == 0);

        listeners.clear();
    }

    /** Возвращает true, если the specified listener has been added to the list. */
    [[nodiscard]] b8 contains (ListenerClass* listener) const noexcept
    {
        return listeners.contains (listener);
    }

    //==============================================================================
    /** Calls an invokable object for each listener in the list. */
    template <typename Callback>
    z0 call (Callback&& callback) const
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the listener specified
        by listenerToExclude.
    */
    template <typename Callback>
    z0 callExcluding (ListenerClass* listenerToExclude, Callback&& callback) const
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));

    }

    /** Calls an invokable object for each listener in the list, additionally checking the bail-out
        checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    z0 callChecked (const BailOutCheckerType& bailOutChecker, Callback&& callback) const
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the listener specified
        by listenerToExclude, additionally checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    z0 callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               Callback&& callback) const
    {
       #if DRX_ASSERTIONS_ENABLED_OR_LOGGED
        ++numCallsInProgress;
        const ScopeGuard decrementPerformingCallbackCount { [&] { --numCallsInProgress; }};
       #endif

        for (auto* listener : listeners)
        {
            if (bailOutChecker.shouldBailOut())
                return;

            if (listener == listenerToExclude)
                continue;

            callback (*listener);
        }
    }

    //==============================================================================
    /** Calls a specific listener method for each listener in the list. */
    template <typename... MethodArgs, typename... Args>
    z0 call (z0 (ListenerClass::*callbackFunction) (MethodArgs...), Args&&... args) const
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, except for the listener
        specified by listenerToExclude.
    */
    template <typename... MethodArgs, typename... Args>
    z0 callExcluding (ListenerClass* listenerToExclude,
                        z0 (ListenerClass::*callbackFunction) (MethodArgs...),
                        Args&&... args) const
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, additionally checking the
        bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    z0 callChecked (const BailOutCheckerType& bailOutChecker,
                      z0 (ListenerClass::*callbackFunction) (MethodArgs...),
                      Args&&... args) const
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, except for the listener
        specified by listenerToExclude, additionally checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    z0 callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               z0 (ListenerClass::*callbackFunction) (MethodArgs...),
                               Args&&... args) const
    {
        callCheckedExcluding (listenerToExclude, bailOutChecker, [&] (ListenerClass& l)
        {
            (l.*callbackFunction) (args...);
        });
    }

    //==============================================================================
    /** A dummy bail-out checker that always returns false.
        See the class description for info about writing a bail-out checker.
    */
    using DummyBailOutChecker = typename ListenerList<ListenerClass>::DummyBailOutChecker;

    //==============================================================================
    using ThisType      = LightweightListenerList<ListenerClass>;
    using ListenerType  = ListenerClass;

private:
   #if DRX_ASSERTIONS_ENABLED_OR_LOGGED
    mutable std::atomic<i32> numCallsInProgress { 0 };
   #endif

    Array<ListenerClass*> listeners;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE (LightweightListenerList)
};

} // namespace drx
