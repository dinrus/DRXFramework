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
/** Fills a block of memory with zeros. */
inline z0 zeromem (uk memory, size_t numBytes) noexcept        { memset (memory, 0, numBytes); }

/** Overwrites a structure or object with zeros. */
template <typename Type>
inline z0 zerostruct (Type& structure) noexcept                   { memset ((uk) &structure, 0, sizeof (structure)); }

/** Delete an object pointer, and sets the pointer to null.

    Remember that it's not good c++ practice to use delete directly - always try to use a std::unique_ptr
    or other automatic lifetime-management system rather than resorting to deleting raw pointers!
*/
template <typename Type>
inline z0 deleteAndZero (Type& pointer)                           { delete pointer; pointer = nullptr; }

/** A handy function to round up a pointer to the nearest multiple of a given number of bytes.
    alignmentBytes must be a power of two. */
template <typename Type, typename IntegerType>
inline Type* snapPointerToAlignment (Type* basePointer, IntegerType alignmentBytes) noexcept
{
    return (Type*) ((((size_t) basePointer) + (alignmentBytes - 1)) & ~(alignmentBytes - 1));
}

/** A handy function which returns the difference between any two pointers, in bytes.
    The address of the second pointer is subtracted from the first, and the difference in bytes is returned.
*/
template <typename Type1, typename Type2>
inline i32 getAddressDifference (Type1* pointer1, Type2* pointer2) noexcept  { return (i32) (((tukk) pointer1) - (tukk) pointer2); }

/** If a pointer is non-null, this returns a new copy of the object that it points to, or safely returns
    nullptr if the pointer is null.
*/
template <class Type>
inline Type* createCopyIfNotNull (const Type* objectToCopy) { return objectToCopy != nullptr ? new Type (*objectToCopy) : nullptr; }

//==============================================================================
/** A handy function to read un-aligned memory without a performance penalty or bus-error. */
template <typename Type>
inline Type readUnaligned (ukk srcPtr) noexcept
{
    Type value;
    memcpy (&value, srcPtr, sizeof (Type));
    return value;
}

/** A handy function to write un-aligned memory without a performance penalty or bus-error. */
template <typename Type>
inline z0 writeUnaligned (uk dstPtr, Type value) noexcept
{
    memcpy (dstPtr, &value, sizeof (Type));
}

//==============================================================================
/** Casts a pointer to another type via `uk`, which suppresses the cast-align
    warning which sometimes arises when casting pointers to types with different
    alignment.
    You should only use this when you know for a fact that the input pointer points
    to a region that has suitable alignment for `Type`, e.g. regions returned from
    malloc/calloc that should be suitable for any non-over-aligned type.
*/
template <typename Type>
inline Type unalignedPointerCast (uk ptr) noexcept
{
    static_assert (std::is_pointer_v<Type>);
    return reinterpret_cast<Type> (ptr);
}

/** Casts a pointer to another type via `uk`, which suppresses the cast-align
    warning which sometimes arises when casting pointers to types with different
    alignment.
    You should only use this when you know for a fact that the input pointer points
    to a region that has suitable alignment for `Type`, e.g. regions returned from
    malloc/calloc that should be suitable for any non-over-aligned type.
*/
template <typename Type>
inline Type unalignedPointerCast (ukk ptr) noexcept
{
    static_assert (std::is_pointer_v<Type>);
    return reinterpret_cast<Type> (ptr);
}

/** A handy function which adds a number of bytes to any type of pointer and returns the result.
    This can be useful to avoid casting pointers to a tuk and back when you want to move them by
    a specific number of bytes,
*/
template <typename Type, typename IntegerType>
inline Type* addBytesToPointer (Type* basePointer, IntegerType bytes) noexcept
{
    return unalignedPointerCast<Type*> (reinterpret_cast<tuk> (basePointer) + bytes);
}

/** A handy function which adds a number of bytes to any type of pointer and returns the result.
    This can be useful to avoid casting pointers to a tuk and back when you want to move them by
    a specific number of bytes,
*/
template <typename Type, typename IntegerType>
inline const Type* addBytesToPointer (const Type* basePointer, IntegerType bytes) noexcept
{
    return unalignedPointerCast<const Type*> (reinterpret_cast<tukk> (basePointer) + bytes);
}

//==============================================================================
#if DRX_MAC || DRX_IOS || DOXYGEN

 /** A handy C++ wrapper that creates and deletes an NSAutoreleasePool object using RAII.
     You should use the DRX_AUTORELEASEPOOL macro to create a local auto-release pool on the stack.

     @tags{Core}
 */
 class DRX_API  ScopedAutoReleasePool
 {
 public:
     ScopedAutoReleasePool();
     ~ScopedAutoReleasePool();

 private:
     uk pool;

     DRX_DECLARE_NON_COPYABLE (ScopedAutoReleasePool)
 };

 /** A macro that can be used to easily declare a local ScopedAutoReleasePool
     object for RAII-based obj-C autoreleasing.
     Because this may use the \@autoreleasepool syntax, you must follow the macro with
     a set of braces to mark the scope of the pool.
 */
#if (DRX_COMPILER_SUPPORTS_ARC && defined (__OBJC__)) || DOXYGEN
 #define DRX_AUTORELEASEPOOL  @autoreleasepool
#else
 #define DRX_AUTORELEASEPOOL  const drx::ScopedAutoReleasePool DRX_JOIN_MACRO (autoReleasePool_, __LINE__);
#endif

#else
 #define DRX_AUTORELEASEPOOL
#endif

//==============================================================================
/* In a Windows DLL build, we'll expose some malloc/free functions that live inside the DLL, and use these for
   allocating all the objects - that way all drx objects in the DLL and in the host will live in the same heap,
   avoiding problems when an object is created in one module and passed across to another where it is deleted.
   By piggy-backing on the DRX_LEAK_DETECTOR macro, these allocators can be injected into most drx classes.
*/
#if DRX_MSVC && (defined (DRX_DLL) || defined (DRX_DLL_BUILD)) && ! (DRX_DISABLE_DLL_ALLOCATORS || DOXYGEN)
 extern DRX_API uk juceDLL_malloc (size_t);
 extern DRX_API z0  juceDLL_free (uk);

 #define DRX_LEAK_DETECTOR(OwnerClass)  public:\
    static uk operator new (size_t sz)           { return drx::juceDLL_malloc (sz); } \
    static uk operator new (size_t, uk p)     { return p; } \
    static z0 operator delete (uk p)           { drx::juceDLL_free (p); } \
    static z0 operator delete (uk, uk)      {}
#endif

//==============================================================================
/** (Deprecated) This was a Windows-specific way of checking for object leaks - now please
    use the DRX_LEAK_DETECTOR instead.
*/
#ifndef drx_UseDebuggingNewOperator
 #define drx_UseDebuggingNewOperator
#endif

/** Converts an owning raw pointer into a unique_ptr, deriving the
    type of the unique_ptr automatically.

    This should only be used with pointers to single objects.
    Do NOT pass a pointer to an array to this function, as the
    destructor of the unique_ptr will incorrectly call `delete`
    instead of `delete[]` on the pointer.
*/
template <typename T>
std::unique_ptr<T> rawToUniquePtr (T* ptr)
{
    return std::unique_ptr<T> (ptr);
}

} // namespace drx
