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

#ifndef DOXYGEN

namespace drx
{

//==============================================================================
/**
    This class is deprecated. You should use std::unique_ptr instead.
*/
template <class ObjectType>
class [[deprecated]] ScopedPointer
{
public:
    //==============================================================================
    DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

    inline ScopedPointer() {}

    inline ScopedPointer (decltype (nullptr)) noexcept {}

    inline ScopedPointer (ObjectType* objectToTakePossessionOf) noexcept
        : object (objectToTakePossessionOf)
    {
    }

    ScopedPointer (ScopedPointer& objectToTransferFrom) noexcept
        : object (objectToTransferFrom.release())
    {
    }

    inline ~ScopedPointer()         { reset(); }

    ScopedPointer& operator= (ScopedPointer& objectToTransferFrom)
    {
        if (this != objectToTransferFrom.getAddress())
        {
            // Two ScopedPointers should never be able to refer to the same object - if
            // this happens, you must have done something dodgy!
            jassert (object == nullptr || object != objectToTransferFrom.object);
            reset (objectToTransferFrom.release());
        }

        return *this;
    }

    ScopedPointer& operator= (ObjectType* newObjectToTakePossessionOf)
    {
        reset (newObjectToTakePossessionOf);
        return *this;
    }

    ScopedPointer (ScopedPointer&& other) noexcept  : object (other.object)
    {
        other.object = nullptr;
    }

    ScopedPointer& operator= (ScopedPointer&& other) noexcept
    {
        reset (other.release());
        return *this;
    }

    //==============================================================================
    inline operator ObjectType*() const noexcept                                    { return object; }
    inline ObjectType* get() const noexcept                                         { return object; }
    inline ObjectType& operator*() const noexcept                                   { return *object; }
    inline ObjectType* operator->() const noexcept                                  { return object; }

    z0 reset()
    {
        auto* oldObject = object;
        object = {};
        ContainerDeletePolicy<ObjectType>::destroy (oldObject);
    }

    z0 reset (ObjectType* newObject)
    {
        if (object != newObject)
        {
            auto* oldObject = object;
            object = newObject;
            ContainerDeletePolicy<ObjectType>::destroy (oldObject);
        }
        else
        {
            // You're trying to reset this ScopedPointer to itself! This will work here as ScopedPointer does an equality check
            // but be aware that std::unique_ptr won't do this and you could end up with some nasty, subtle bugs!
            jassert (newObject == nullptr);
        }
    }

    z0 reset (ScopedPointer& newObject)
    {
        reset (newObject.release());
    }

    ObjectType* release() noexcept  { auto* o = object; object = {}; return o; }

    //==============================================================================
    z0 swapWith (ScopedPointer<ObjectType>& other) noexcept
    {
        // Two ScopedPointers should never be able to refer to the same object - if
        // this happens, you must have done something dodgy!
        jassert (object != other.object || this == other.getAddress() || object == nullptr);

        std::swap (object, other.object);
    }

    inline ObjectType* createCopy() const { return createCopyIfNotNull (object); }

private:
    //==============================================================================
    ObjectType* object = nullptr;

    const ScopedPointer* getAddress() const noexcept  { return this; } // Used internally to avoid the & operator

   #if ! DRX_MSVC  // (MSVC can't deal with multiple copy constructors)
    ScopedPointer (const ScopedPointer&) = delete;
    ScopedPointer& operator= (const ScopedPointer&) = delete;
   #endif

    DRX_END_IGNORE_DEPRECATION_WARNINGS
};

//==============================================================================
DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

template <typename ObjectType1, typename ObjectType2>
b8 operator== (ObjectType1* pointer1, const ScopedPointer<ObjectType2>& pointer2) noexcept
{
    return pointer1 == pointer2.get();
}

template <typename ObjectType1, typename ObjectType2>
b8 operator!= (ObjectType1* pointer1, const ScopedPointer<ObjectType2>& pointer2) noexcept
{
    return pointer1 != pointer2.get();
}

template <typename ObjectType1, typename ObjectType2>
b8 operator== (const ScopedPointer<ObjectType1>& pointer1, ObjectType2* pointer2) noexcept
{
    return pointer1.get() == pointer2;
}

template <typename ObjectType1, typename ObjectType2>
b8 operator!= (const ScopedPointer<ObjectType1>& pointer1, ObjectType2* pointer2) noexcept
{
    return pointer1.get() != pointer2;
}

template <typename ObjectType1, typename ObjectType2>
b8 operator== (const ScopedPointer<ObjectType1>& pointer1, const ScopedPointer<ObjectType2>& pointer2) noexcept
{
    return pointer1.get() == pointer2.get();
}

template <typename ObjectType1, typename ObjectType2>
b8 operator!= (const ScopedPointer<ObjectType1>& pointer1, const ScopedPointer<ObjectType2>& pointer2) noexcept
{
    return pointer1.get() != pointer2.get();
}

template <class ObjectType>
b8 operator== (decltype (nullptr), const ScopedPointer<ObjectType>& pointer) noexcept
{
    return pointer.get() == nullptr;
}

template <class ObjectType>
b8 operator!= (decltype (nullptr), const ScopedPointer<ObjectType>& pointer) noexcept
{
    return pointer.get() != nullptr;
}

template <class ObjectType>
b8 operator== (const ScopedPointer<ObjectType>& pointer, decltype (nullptr)) noexcept
{
    return pointer.get() == nullptr;
}

template <class ObjectType>
b8 operator!= (const ScopedPointer<ObjectType>& pointer, decltype (nullptr)) noexcept
{
    return pointer.get() != nullptr;
}

//==============================================================================
// NB: This is just here to prevent any silly attempts to call deleteAndZero() on a ScopedPointer.
template <typename Type>
z0 deleteAndZero (ScopedPointer<Type>&)  { static_assert (sizeof (Type) == 12345,
                                                            "Attempt to call deleteAndZero() on a ScopedPointer"); }

DRX_END_IGNORE_DEPRECATION_WARNINGS

} // namespace drx

#endif
