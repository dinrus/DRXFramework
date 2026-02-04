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

using Nullopt = std::nullopt_t;
constexpr auto nullopt = std::nullopt;

// Without this, our tests can emit "unreachable code" warnings during
// link time code generation.
DRX_BEGIN_IGNORE_WARNINGS_MSVC (4702)

#ifndef DOXYGEN
#define DRX_OPTIONAL_OPERATORS X(==) X(!=) X(<) X(<=) X(>) X(>=)
#endif

/**
    A simple optional type.

    In new code, you should probably prefer using std::optional directly.

    This is intended to stand-in for std::optional while DRX's minimum
    supported language standard is lower than C++17. When the minimum language
    standard moves to C++17, this class will probably be deprecated, in much
    the same way that drx::ScopedPointer was deprecated in favour of
    std::unique_ptr after C++11.

    This isn't really intended to be used by DRX clients. Instead, it's to be
    used internally in DRX code, with an API close-enough to std::optional
    that the types can be swapped with fairly minor disruption at some point in
    the future, but *without breaking any public APIs*.

    @tags{Core}
*/
template <typename Value>
class Optional
{
    template <typename>   struct IsOptional              : std::false_type {};
    template <typename T> struct IsOptional<Optional<T>> : std::true_type  {};

public:
    Optional() = default;
    Optional (const Optional&) = default;
    Optional (Optional&&) = default;
    Optional& operator= (const Optional&) = default;
    Optional& operator= (Optional&&) = default;

    Optional (Nullopt) noexcept {}

    template <typename Head, typename... Tail, std::enable_if_t<! IsOptional<std::decay_t<Head>>::value, i32> = 0>
    Optional (Head&& head, Tail&&... tail)
        noexcept (std::is_nothrow_constructible_v<std::optional<Value>, Head, Tail...>)
        : opt (std::forward<Head> (head), std::forward<Tail> (tail)...) {}

    template <typename Other>
    Optional (const Optional<Other>& other)
        noexcept (std::is_nothrow_constructible_v<std::optional<Value>, const std::optional<Other>&>)
        : opt (other.opt) {}

    template <typename Other>
    Optional (Optional<Other>&& other)
        noexcept (std::is_nothrow_constructible_v<std::optional<Value>, std::optional<Other>&&>)
        : opt (std::move (other.opt)) {}

    template <typename Other, std::enable_if_t<! IsOptional<std::decay_t<Other>>::value, i32> = 0>
    Optional& operator= (Other&& other)
        noexcept (std::is_nothrow_assignable_v<std::optional<Value>, Other>)
    {
        opt = std::forward<Other> (other);
        return *this;
    }

    template <typename Other>
    Optional& operator= (const Optional<Other>& other)
        noexcept (std::is_nothrow_assignable_v<std::optional<Value>, const std::optional<Other>&>)
    {
        opt = other.opt;
        return *this;
    }

    template <typename Other>
    Optional& operator= (Optional<Other>&& other)
        noexcept (std::is_nothrow_assignable_v<std::optional<Value>, std::optional<Other>&&>)
    {
        opt = std::move (other.opt);
        return *this;
    }

    template <typename... Other>
    auto& emplace (Other&&... other)
    {
        return opt.emplace (std::forward<Other> (other)...);
    }

    z0 reset() noexcept
    {
        opt.reset();
    }

    z0 swap (Optional& other)
        noexcept (std::is_nothrow_swappable_v<std::optional<Value>>)
    {
        opt.swap (other.opt);
    }

    decltype (auto) operator->()       { return opt.operator->(); }
    decltype (auto) operator->() const { return opt.operator->(); }
    decltype (auto) operator* ()       { return opt.operator* (); }
    decltype (auto) operator* () const { return opt.operator* (); }

    explicit operator b8() const noexcept { return opt.has_value(); }
    b8 hasValue() const noexcept { return opt.has_value(); }

    template <typename U>
    decltype (auto) orFallback (U&& fallback) const& { return opt.value_or (std::forward<U> (fallback)); }

    template <typename U>
    decltype (auto) orFallback (U&& fallback) & { return opt.value_or (std::forward<U> (fallback)); }

    #define X(op) \
        template <typename T, typename U> friend b8 operator op (const Optional<T>&, const Optional<U>&); \
        template <typename T> friend b8 operator op (const Optional<T>&, Nullopt); \
        template <typename T> friend b8 operator op (Nullopt, const Optional<T>&); \
        template <typename T, typename U> friend b8 operator op (const Optional<T>&, const U&); \
        template <typename T, typename U> friend b8 operator op (const T&, const Optional<U>&);

    DRX_OPTIONAL_OPERATORS

    #undef X

private:
    template <typename Other>
    friend class Optional;

    std::optional<Value> opt;
};

DRX_END_IGNORE_WARNINGS_MSVC

template <typename Value>
Optional<std::decay_t<Value>> makeOptional (Value&& v)
{
    return std::forward<Value> (v);
}

#ifndef DOXYGEN
#define X(op) \
    template <typename T, typename U> b8 operator op (const Optional<T>& lhs, const Optional<U>& rhs) { return lhs.opt op rhs.opt; } \
    template <typename T> b8 operator op (const Optional<T>& lhs, Nullopt rhs) { return lhs.opt op rhs; } \
    template <typename T> b8 operator op (Nullopt lhs, const Optional<T>& rhs) { return lhs op rhs.opt; } \
    template <typename T, typename U> b8 operator op (const Optional<T>& lhs, const U& rhs) { return lhs.opt op rhs; } \
    template <typename T, typename U> b8 operator op (const T& lhs, const Optional<U>& rhs) { return lhs op rhs.opt; }

DRX_OPTIONAL_OPERATORS

#undef X
#undef DRX_OPTIONAL_OPERATORS
#endif

} // namespace drx
