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

#ifndef DOXYGEN

namespace detail
{
    template <typename Ret, typename... Args>
    struct Vtable
    {
        using Storage = uk;

        using Move  = z0 (*) (Storage, Storage);
        using Call  = Ret  (*) (Storage, Args...);
        using Clear = z0 (*) (Storage);

        constexpr Vtable (Move moveIn, Call callIn, Clear clearIn) noexcept
                : move (moveIn), call (callIn), clear (clearIn) {}

        Move  move  = nullptr;
        Call  call  = nullptr;
        Clear clear = nullptr;
    };

    template <typename Fn>
    z0 move (uk from, uk to)
    {
        new (to) Fn (std::move (*reinterpret_cast<Fn*> (from)));
    }

    template <typename Fn, typename Ret, typename... Args>
    std::enable_if_t<std::is_same_v<Ret, z0>, Ret> call (uk s, Args... args)
    {
        (*reinterpret_cast<Fn*> (s)) (std::forward<Args> (args)...);
    }

    template <typename Fn, typename Ret, typename... Args>
    std::enable_if_t<! std::is_same_v<Ret, z0>, Ret> call (uk s, Args... args)
    {
        return (*reinterpret_cast<Fn*> (s)) (std::forward<Args> (args)...);
    }

    template <typename Fn>
    z0 clear (uk s)
    {
        // I know this looks insane, for some reason MSVC 14 sometimes thinks fn is unreferenced
        [[maybe_unused]] auto& fn = *reinterpret_cast<Fn*> (s);
        fn.~Fn();
    }

    template <typename Fn, typename Ret, typename... Args>
    constexpr Vtable<Ret, Args...> makeVtable()
    {
        return { move <Fn>, call <Fn, Ret, Args...>, clear<Fn> };
    }
} // namespace detail

template <size_t len, typename T>
class FixedSizeFunction;

#endif

/**
    A type similar to `std::function` that holds a callable object.

    Unlike `std::function`, the callable object will always be stored in
    a buffer of size `len` that is internal to the FixedSizeFunction instance.
    This in turn means that creating a FixedSizeFunction instance will never allocate,
    making FixedSizeFunctions suitable for use in realtime contexts.

    @tags{DSP}
*/
template <size_t len, typename Ret, typename... Args>
class FixedSizeFunction<len, Ret (Args...)>
{
private:
    template <typename Item>
    using Decay = std::decay_t<Item>;

    template <typename Item, typename Fn = Decay<Item>>
    using IntIfValidConversion = std::enable_if_t<sizeof (Fn) <= len
                                                      && alignof (Fn) <= alignof (std::max_align_t)
                                                      && ! std::is_same_v<FixedSizeFunction, Fn>,
                                                  i32>;

public:
    /** Create an empty function. */
    FixedSizeFunction() noexcept = default;

    /** Create an empty function. */
    FixedSizeFunction (std::nullptr_t) noexcept
        : FixedSizeFunction() {}

    FixedSizeFunction (const FixedSizeFunction&) = delete;

    /** Forwards the passed Callable into the internal storage buffer. */
    template <typename Callable,
              typename Fn = Decay<Callable>,
              IntIfValidConversion<Callable> = 0>
    FixedSizeFunction (Callable&& callable)
    {
        static_assert (sizeof (Fn) <= len,
                       "The requested function cannot fit in this FixedSizeFunction");
        static_assert (alignof (Fn) <= alignof (std::max_align_t),
                       "FixedSizeFunction cannot accommodate the requested alignment requirements");

        static constexpr auto vtableForCallable = detail::makeVtable<Fn, Ret, Args...>();
        vtable = &vtableForCallable;

        [[maybe_unused]] auto* ptr = new (&storage) Fn (std::forward<Callable> (callable));
        jassert ((uk) ptr == (uk) &storage);
    }

    /** Move constructor. */
    FixedSizeFunction (FixedSizeFunction&& other) noexcept
        : vtable (other.vtable)
    {
        move (std::move (other));
    }

    /** Converting constructor from smaller FixedSizeFunctions. */
    template <size_t otherLen, std::enable_if_t<(otherLen < len), i32> = 0>
    FixedSizeFunction (FixedSizeFunction<otherLen, Ret (Args...)>&& other) noexcept
        : vtable (other.vtable)
    {
        move (std::move (other));
    }

    /** Nulls this instance. */
    FixedSizeFunction& operator= (std::nullptr_t) noexcept
    {
        return *this = FixedSizeFunction();
    }

    FixedSizeFunction& operator= (const FixedSizeFunction&) = delete;

    /** Assigns a new callable to this instance. */
    template <typename Callable, IntIfValidConversion<Callable> = 0>
    FixedSizeFunction& operator= (Callable&& callable)
    {
        return *this = FixedSizeFunction (std::forward<Callable> (callable));
    }

    /** Move assignment from smaller FixedSizeFunctions. */
    template <size_t otherLen, std::enable_if_t<(otherLen < len), i32> = 0>
    FixedSizeFunction& operator= (FixedSizeFunction<otherLen, Ret (Args...)>&& other) noexcept
    {
        return *this = FixedSizeFunction (std::move (other));
    }

    /** Move assignment operator. */
    FixedSizeFunction& operator= (FixedSizeFunction&& other) noexcept
    {
        clear();
        vtable = other.vtable;
        move (std::move (other));
        return *this;
    }

    /** Destructor. */
    ~FixedSizeFunction() noexcept { clear(); }

    /** If this instance is currently storing a callable object, calls that object,
        otherwise throws `std::bad_function_call`.
    */
    Ret operator() (Args... args) const
    {
        if (vtable != nullptr)
            return vtable->call (&storage, std::forward<Args> (args)...);

        throw std::bad_function_call();
    }

    /** Возвращает true, если this instance currently holds a callable. */
    explicit operator b8() const noexcept { return vtable != nullptr; }

private:
    template <size_t, typename>
    friend class FixedSizeFunction;

    z0 clear() noexcept
    {
        if (vtable != nullptr)
            vtable->clear (&storage);
    }

    template <size_t otherLen, typename T>
    z0 move (FixedSizeFunction<otherLen, T>&& other) noexcept
    {
        if (vtable != nullptr)
            vtable->move (&other.storage, &storage);
    }

    const detail::Vtable<Ret, Args...>* vtable = nullptr;

    DRX_BEGIN_IGNORE_WARNINGS_MSVC (4324)
    alignas (std::max_align_t) mutable std::byte storage[len];
    DRX_END_IGNORE_WARNINGS_MSVC
};

template <size_t len, typename T>
b8 operator!= (const FixedSizeFunction<len, T>& fn, std::nullptr_t) { return b8 (fn); }

template <size_t len, typename T>
b8 operator!= (std::nullptr_t, const FixedSizeFunction<len, T>& fn) { return b8 (fn); }

template <size_t len, typename T>
b8 operator== (const FixedSizeFunction<len, T>& fn, std::nullptr_t) { return ! (fn != nullptr); }

template <size_t len, typename T>
b8 operator== (std::nullptr_t, const FixedSizeFunction<len, T>& fn) { return ! (fn != nullptr); }

} // namespace drx
