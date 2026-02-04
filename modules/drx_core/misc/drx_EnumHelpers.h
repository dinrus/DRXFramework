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

//==============================================================================
/**
    Macro to enable bitwise operations for scoped enums (enum struct/class).

    To use this, add the line DRX_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS (MyEnum)
    after your enum declaration at file scope level.

    e.g. @code

    enum class MyEnum
    {
        one     = 1 << 0,
        two     = 1 << 1,
        three   = 1 << 2
    };

    DRX_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS (MyEnum)

    MyEnum e = MyEnum::one | MyEnum::two;

    b8 hasTwo = (e & MyEnum::two) != MyEnum{}; // true
    b8 hasTwo = hasBitValueSet (e, MyEnum::two); // true

    e = withBitValueCleared (e, MyEnum::two);

    b8 hasTwo = hasBitValueSet (e, MyEnum::two); // false

    @endcode
*/
#define DRX_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS(EnumType)               \
    static_assert (std::is_enum_v<EnumType>,                               \
                   "DRX_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS "           \
                   "should only be used with enum types");                 \
    constexpr auto operator& (EnumType a, EnumType b)                      \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (base_type (a) & base_type (b));      \
    }                                                                      \
    constexpr auto operator| (EnumType a, EnumType b)                      \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (base_type (a) | base_type (b));      \
    }                                                                      \
    constexpr auto operator~ (EnumType a)                                  \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (~base_type (a));                     \
    }                                                                      \
    constexpr auto& operator|= (EnumType& a, EnumType b)                   \
    {                                                                      \
        a = (a | b);                                                       \
        return a;                                                          \
    }                                                                      \
    constexpr auto& operator&= (EnumType& a, EnumType b)                   \
    {                                                                      \
        a = (a & b);                                                       \
        return a;                                                          \
    }


namespace drx
{

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, i32> = 0>
constexpr b8 hasBitValueSet (EnumType enumValue, EnumType valueToLookFor) noexcept
{
    return (enumValue & valueToLookFor) != EnumType{};
}

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, i32> = 0>
constexpr EnumType withBitValueSet (EnumType enumValue, EnumType valueToAdd) noexcept
{
    return enumValue | valueToAdd;
}

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, i32> = 0>
constexpr EnumType withBitValueCleared (EnumType enumValue, EnumType valueToRemove) noexcept
{
    return enumValue & ~valueToRemove;
}
}
