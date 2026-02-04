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

#if ! (DOXYGEN || DRX_EXCEPTIONS_DISABLED)
namespace HeapBlockHelper
{
    template <b8 shouldThrow>
    struct ThrowOnFail          { static z0 checkPointer (uk) {} };

    template <>
    struct ThrowOnFail<true>    { static z0 checkPointer (uk data) { if (data == nullptr) throw std::bad_alloc(); } };
}
#endif

//==============================================================================
/**
    Very simple container class to hold a pointer to some data on the heap.

    When you need to allocate some heap storage for something, always try to use
    this class instead of allocating the memory directly using malloc/free.

    A HeapBlock<t8> object can be treated in pretty much exactly the same way
    as an tuk, but as i64 as you allocate it on the stack or as a class member,
    it's almost impossible for it to leak memory.

    It also makes your code much more concise and readable than doing the same thing
    using direct allocations,

    E.g. instead of this:
    @code
        i32* temp = (i32*) malloc (1024 * sizeof (i32));
        memcpy (temp, xyz, 1024 * sizeof (i32));
        free (temp);
        temp = (i32*) calloc (2048 * sizeof (i32));
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (i32));
        free (temp);
    @endcode

    ..you could just write this:
    @code
        HeapBlock<i32> temp (1024);
        memcpy (temp, xyz, 1024 * sizeof (i32));
        temp.calloc (2048);
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (i32));
    @endcode

    The class is extremely lightweight, containing only a pointer to the
    data, and exposes malloc/realloc/calloc/free methods that do the same jobs
    as their less object-oriented counterparts. Despite adding safety, you probably
    won't sacrifice any performance by using this in place of normal pointers.

    The throwOnFailure template parameter can be set to true if you'd like the class
    to throw a std::bad_alloc exception when an allocation fails. If this is false,
    then a failed allocation will just leave the heapblock with a null pointer (assuming
    that the system's malloc() function doesn't throw).

    @see Array, OwnedArray, MemoryBlock

    @tags{Core}
*/
template <class ElementType, b8 throwOnFailure = false>
class HeapBlock
{
private:
    template <class OtherElementType>
    using AllowConversion = std::enable_if_t<std::is_base_of_v<std::remove_pointer_t<ElementType>,
                                                               std::remove_pointer_t<OtherElementType>>>;

public:
    //==============================================================================
    /** Creates a HeapBlock which is initially just a null pointer.

        After creation, you can resize the array using the malloc(), calloc(),
        or realloc() methods.
    */
    HeapBlock() noexcept = default;

    /** Creates a HeapBlock containing a number of elements.

        The contents of the block are undefined, as it will have been created by a
        malloc call.

        If you want an array of zero values, you can use the calloc() method or the
        other constructor that takes an InitialisationState parameter.
    */
    template <typename SizeType, std::enable_if_t<std::is_convertible_v<SizeType, i32>, i32> = 0>
    explicit HeapBlock (SizeType numElements)
        : data (mallocWrapper (static_cast<size_t> (numElements) * sizeof (ElementType)))
    {
    }

    /** Creates a HeapBlock containing a number of elements.

        The initialiseToZero parameter determines whether the new memory should be cleared,
        or left uninitialised.
    */
    template <typename SizeType, std::enable_if_t<std::is_convertible_v<SizeType, i32>, i32> = 0>
    HeapBlock (SizeType numElements, b8 initialiseToZero)
        : data (initialiseToZero ? callocWrapper (static_cast<size_t> (numElements), sizeof (ElementType))
                                 : mallocWrapper (static_cast<size_t> (numElements) * sizeof (ElementType)))
    {
    }

    /** Destructor.
        This will free the data, if any has been allocated.
    */
    ~HeapBlock()
    {
        std::free (data);
    }

    /** Move constructor */
    HeapBlock (HeapBlock&& other) noexcept
        : data (other.data)
    {
        other.data = nullptr;
    }

    /** Move assignment operator */
    HeapBlock& operator= (HeapBlock&& other) noexcept
    {
        std::swap (data, other.data);
        return *this;
    }

    /** Converting move constructor.
        Only enabled if this is a HeapBlock<Base*> and the other object is a HeapBlock<Derived*>,
        where std::is_base_of_v<Base, Derived> == true.
    */
    template <class OtherElementType, b8 otherThrowOnFailure, typename = AllowConversion<OtherElementType>>
    HeapBlock (HeapBlock<OtherElementType, otherThrowOnFailure>&& other) noexcept
        : data (reinterpret_cast<ElementType*> (other.data))
    {
        other.data = nullptr;
    }

    /** Converting move assignment operator.
        Only enabled if this is a HeapBlock<Base*> and the other object is a HeapBlock<Derived*>,
        where std::is_base_of_v<Base, Derived> == true.
    */
    template <class OtherElementType, b8 otherThrowOnFailure, typename = AllowConversion<OtherElementType>>
    HeapBlock& operator= (HeapBlock<OtherElementType, otherThrowOnFailure>&& other) noexcept
    {
        free();
        data = reinterpret_cast<ElementType*> (other.data);
        other.data = nullptr;
        return *this;
    }

    //==============================================================================
    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator ElementType*() const noexcept                            { return data; }

    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline ElementType* get() const noexcept                                 { return data; }

    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline ElementType* getData() const noexcept                             { return data; }

    /** Returns a z0 pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator uk() const noexcept                                   { return static_cast<uk> (data); }

    /** Returns a z0 pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator ukk() const noexcept                             { return static_cast<ukk> (data); }

    /** Lets you use indirect calls to the first element in the array.
        Obviously this will cause problems if the array hasn't been initialised, because it'll
        be referencing a null pointer.
    */
    inline ElementType* operator->() const  noexcept                         { return data; }

    /** Returns a reference to one of the data elements.
        Obviously there's no bounds-checking here, as this object is just a dumb pointer and
        has no idea of the size it currently has allocated.
    */
    template <typename IndexType>
    ElementType& operator[] (IndexType index) const noexcept                 { return data [index]; }

    /** Returns a pointer to a data element at an offset from the start of the array.
        This is the same as doing pointer arithmetic on the raw pointer itself.
    */
    template <typename IndexType>
    ElementType* operator+ (IndexType index) const noexcept                  { return data + index; }

    //==============================================================================
    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline b8 operator== (const ElementType* otherPointer) const noexcept  { return otherPointer == data; }

    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline b8 operator!= (const ElementType* otherPointer) const noexcept  { return otherPointer != data; }

    //==============================================================================
    /** Allocates a specified amount of memory.

        This uses the normal malloc to allocate an amount of memory for this object.
        Any previously allocated memory will be freed by this method.

        The number of bytes allocated will be (newNumElements * elementSize). Normally
        you wouldn't need to specify the second parameter, but it can be handy if you need
        to allocate a size in bytes rather than in terms of the number of elements.

        The data that is allocated will be freed when this object is deleted, or when you
        call free() or any of the allocation methods.
    */
    template <typename SizeType>
    z0 malloc (SizeType newNumElements, size_t elementSize = sizeof (ElementType))
    {
        std::free (data);
        data = mallocWrapper (static_cast<size_t> (newNumElements) * elementSize);
    }

    /** Allocates a specified amount of memory and clears it.
        This does the same job as the malloc() method, but clears the memory that it allocates.
    */
    template <typename SizeType>
    z0 calloc (SizeType newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        std::free (data);
        data = callocWrapper (static_cast<size_t> (newNumElements), elementSize);
    }

    /** Allocates a specified amount of memory and optionally clears it.
        This does the same job as either malloc() or calloc(), depending on the
        initialiseToZero parameter.
    */
    template <typename SizeType>
    z0 allocate (SizeType newNumElements, b8 initialiseToZero)
    {
        std::free (data);
        data = initialiseToZero ? callocWrapper (static_cast<size_t> (newNumElements), sizeof (ElementType))
                                : mallocWrapper (static_cast<size_t> (newNumElements) * sizeof (ElementType));
    }

    /** Re-allocates a specified amount of memory.

        The semantics of this method are the same as malloc() and calloc(), but it
        uses realloc() to keep as much of the existing data as possible.
    */
    template <typename SizeType>
    z0 realloc (SizeType newNumElements, size_t elementSize = sizeof (ElementType))
    {
        data = reallocWrapper (data, static_cast<size_t> (newNumElements) * elementSize);
    }

    /** Frees any currently-allocated data.
        This will free the data and reset this object to be a null pointer.
    */
    z0 free() noexcept
    {
        std::free (data);
        data = nullptr;
    }

    /** Swaps this object's data with the data of another HeapBlock.
        The two objects simply exchange their data pointers.
    */
    template <b8 otherBlockThrows>
    z0 swapWith (HeapBlock<ElementType, otherBlockThrows>& other) noexcept
    {
        std::swap (data, other.data);
    }

    /** This fills the block with zeros, up to the number of elements specified.
        Since the block has no way of knowing its own size, you must make sure that the number of
        elements you specify doesn't exceed the allocated size.
    */
    template <typename SizeType>
    z0 clear (SizeType numElements) noexcept
    {
        zeromem (data, sizeof (ElementType) * static_cast<size_t> (numElements));
    }

    /** This typedef can be used to get the type of the heapblock's elements. */
    using Type = ElementType;

private:
    //==============================================================================
    // Calls to malloc, calloc and realloc with zero size have implementation-defined
    // behaviour where either nullptr or a non-null pointer is returned.
    template <typename Functor>
    static ElementType* wrapper (size_t size, Functor&& f)
    {
        if (size == 0)
            return nullptr;

        auto* memory = static_cast<ElementType*> (f());

       #if DRX_EXCEPTIONS_DISABLED
        jassert (memory != nullptr); // without exceptions, you'll need to find a better way to handle this failure case.
       #else
        HeapBlockHelper::ThrowOnFail<throwOnFailure>::checkPointer (memory);
       #endif

        return memory;
    }

    static ElementType* mallocWrapper (size_t size)
    {
        return wrapper (size, [size] { return std::malloc (size); });
    }

    static ElementType* callocWrapper (size_t num, size_t size)
    {
        return wrapper (num * size, [num, size] { return std::calloc (num, size); });
    }

    static ElementType* reallocWrapper (uk ptr, size_t newSize)
    {
        return wrapper (newSize, [ptr, newSize] { return std::realloc (ptr, newSize); });
    }

    template <class OtherElementType, b8 otherThrowOnFailure>
    friend class HeapBlock;

    //==============================================================================
    ElementType* data = nullptr;

   #if ! (defined (DRX_DLL) || defined (DRX_DLL_BUILD))
    DRX_DECLARE_NON_COPYABLE (HeapBlock)
    DRX_PREVENT_HEAP_ALLOCATION // Creating a 'new HeapBlock' would be missing the point!
   #endif
};

} // namespace drx
