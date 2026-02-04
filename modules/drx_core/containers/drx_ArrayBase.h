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

/**
    A basic object container.

    This class isn't really for public use - it's used by the other
    array classes, but might come in handy for some purposes.

    It inherits from a critical section class to allow the arrays to use
    the "empty base class optimisation" pattern to reduce their footprint.

    @see Array, OwnedArray, ReferenceCountedArray

    @tags{Core}
*/
template <class ElementType, class TypeOfCriticalSectionToUse>
class ArrayBase  : public TypeOfCriticalSectionToUse
{
private:
    using ParameterType = typename TypeHelpers::ParameterType<ElementType>::type;

    static_assert (std::is_nothrow_constructible_v<TypeOfCriticalSectionToUse>,
                   "The critical section type used must not throw during construction");

    template <class OtherElementType, class OtherCriticalSection>
    using AllowConversion = std::enable_if_t<! std::is_same_v<std::tuple<ElementType, TypeOfCriticalSectionToUse>,
                                                              std::tuple<OtherElementType, OtherCriticalSection>>>;

public:
    //==============================================================================
    ArrayBase() noexcept = default;

    ~ArrayBase()
    {
        clear();
    }

    ArrayBase (ArrayBase&& other) noexcept
        : elements (std::move (other.elements)),
          numAllocated (other.numAllocated),
          numUsed (other.numUsed)
    {
        other.numAllocated = 0;
        other.numUsed = 0;
    }

    ArrayBase& operator= (ArrayBase&& other) noexcept
    {
        if (this != &other)
        {
            auto tmp (std::move (other));
            swapWith (tmp);
        }

        return *this;
    }

    /** Converting move constructor.
        Only enabled when the other array has a different type to this one.
        If you see a compile error here, it's probably because you're attempting a conversion that
        HeapBlock won't allow.
    */
    template <class OtherElementType,
              class OtherCriticalSection,
              typename = AllowConversion<OtherElementType, OtherCriticalSection>>
    ArrayBase (ArrayBase<OtherElementType, OtherCriticalSection>&& other) noexcept
        : elements (std::move (other.elements)),
          numAllocated (other.numAllocated),
          numUsed (other.numUsed)
    {
        other.numAllocated = 0;
        other.numUsed = 0;
    }

    /** Converting move assignment operator.
        Only enabled when the other array has a different type to this one.
        If you see a compile error here, it's probably because you're attempting a conversion that
        HeapBlock won't allow.
    */
    template <class OtherElementType,
              class OtherCriticalSection,
              typename = AllowConversion<OtherElementType, OtherCriticalSection>>
    ArrayBase& operator= (ArrayBase<OtherElementType, OtherCriticalSection>&& other) noexcept
    {
        // No need to worry about assignment to *this, because 'other' must be of a different type.
        elements = std::move (other.elements);
        numAllocated = other.numAllocated;
        numUsed = other.numUsed;

        other.numAllocated = 0;
        other.numUsed = 0;

        return *this;
    }

    //==============================================================================
    template <class OtherArrayType>
    b8 operator== (const OtherArrayType& other) const noexcept
    {
        if (size() != (i32) other.size())
            return false;

        auto* e = begin();

        for (auto& o : other)
            if (! exactlyEqual (*e++, o))
                return false;

        return true;
    }

    template <class OtherArrayType>
    b8 operator!= (const OtherArrayType& other) const noexcept
    {
        return ! operator== (other);
    }

    //==============================================================================
    inline ElementType& operator[] (i32k index) noexcept
    {
        jassert (elements != nullptr);
        jassert (isPositiveAndBelow (index, numUsed));
        return elements[index];
    }

    inline const ElementType& operator[] (i32k index) const noexcept
    {
        jassert (elements != nullptr);
        jassert (isPositiveAndBelow (index, numUsed));
        return elements[index];
    }

    inline ElementType getValueWithDefault (i32k index) const noexcept
    {
        return isPositiveAndBelow (index, numUsed) ? elements[index] : ElementType();
    }

    inline ElementType getFirst() const noexcept
    {
        return numUsed > 0 ? elements[0] : ElementType();
    }

    inline ElementType getLast() const noexcept
    {
        return numUsed > 0 ? elements[numUsed - 1] : ElementType();
    }

    //==============================================================================
    inline ElementType* begin() noexcept
    {
        return elements;
    }

    inline const ElementType* begin() const noexcept
    {
        return elements;
    }

    inline ElementType* end() noexcept
    {
        return elements + numUsed;
    }

    inline const ElementType* end() const noexcept
    {
        return elements + numUsed;
    }

    inline ElementType* data() noexcept
    {
        return elements;
    }

    inline const ElementType* data() const noexcept
    {
        return elements;
    }

    inline i32 size() const noexcept
    {
        return numUsed;
    }

    inline i32 capacity() const noexcept
    {
        return numAllocated;
    }

    //==============================================================================
    z0 setAllocatedSize (i32 numElements)
    {
        jassert (numElements >= numUsed);

        if (numAllocated != numElements)
        {
            if (numElements > 0)
                setAllocatedSizeInternal (numElements);
            else
                elements.free();
        }

        numAllocated = numElements;
    }

    z0 ensureAllocatedSize (i32 minNumElements)
    {
        if (minNumElements > numAllocated)
            setAllocatedSize ((minNumElements + minNumElements / 2 + 8) & ~7);

        jassert (numAllocated <= 0 || elements != nullptr);
    }

    z0 shrinkToNoMoreThan (i32 maxNumElements)
    {
        if (maxNumElements < numAllocated)
            setAllocatedSize (maxNumElements);
    }

    z0 clear()
    {
        for (i32 i = 0; i < numUsed; ++i)
            elements[i].~ElementType();

        numUsed = 0;
    }

    //==============================================================================
    z0 swapWith (ArrayBase& other) noexcept
    {
        elements.swapWith (other.elements);
        std::swap (numAllocated, other.numAllocated);
        std::swap (numUsed,      other.numUsed);
    }

    //==============================================================================
    z0 add (const ElementType& newElement)
    {
        addImpl (newElement);
    }

    z0 add (ElementType&& newElement)
    {
        addImpl (std::move (newElement));
    }

    template <typename... OtherElements>
    z0 add (const ElementType& firstNewElement, OtherElements&&... otherElements)
    {
        addImpl (firstNewElement, std::forward<OtherElements> (otherElements)...);
    }

    template <typename... OtherElements>
    z0 add (ElementType&& firstNewElement, OtherElements&&... otherElements)
    {
        addImpl (std::move (firstNewElement), std::forward<OtherElements> (otherElements)...);
    }

    //==============================================================================
    template <typename Type>
    z0 addArray (const Type* elementsToAdd, i32 numElementsToAdd)
    {
        ensureAllocatedSize (numUsed + numElementsToAdd);
        addArrayInternal (elementsToAdd, numElementsToAdd);
        numUsed += numElementsToAdd;
    }

    template <typename TypeToCreateFrom>
    z0 addArray (const std::initializer_list<TypeToCreateFrom>& items)
    {
        ensureAllocatedSize (numUsed + (i32) items.size());

        for (auto& item : items)
            new (elements + numUsed++) ElementType (item);
    }

    template <class OtherArrayType>
    z0 addArray (const OtherArrayType& arrayToAddFrom)
    {
        jassert ((ukk) this != (ukk) &arrayToAddFrom); // can't add from our own elements!
        ensureAllocatedSize (numUsed + (i32) arrayToAddFrom.size());

        for (auto& e : arrayToAddFrom)
            addAssumingCapacityIsReady (e);
    }

    template <class OtherArrayType>
    std::enable_if_t<! std::is_pointer_v<OtherArrayType>, i32>
    addArray (const OtherArrayType& arrayToAddFrom,
              i32 startIndex, i32 numElementsToAdd = -1)
    {
        jassert ((ukk) this != (ukk) &arrayToAddFrom); // can't add from our own elements!

        if (startIndex < 0)
        {
            jassertfalse;
            startIndex = 0;
        }

        if (numElementsToAdd < 0 || startIndex + numElementsToAdd > (i32) arrayToAddFrom.size())
            numElementsToAdd = (i32) arrayToAddFrom.size() - startIndex;

        addArray (arrayToAddFrom.data() + startIndex, numElementsToAdd);

        return numElementsToAdd;
    }

    //==============================================================================
    z0 insert (i32 indexToInsertAt, ParameterType newElement, i32 numberOfTimesToInsertIt)
    {
        checkSourceIsNotAMember (newElement);
        auto* space = createInsertSpace (indexToInsertAt, numberOfTimesToInsertIt);

        for (i32 i = 0; i < numberOfTimesToInsertIt; ++i)
            new (space++) ElementType (newElement);

        numUsed += numberOfTimesToInsertIt;
    }

    z0 insertArray (i32 indexToInsertAt, const ElementType* newElements, i32 numberOfElements)
    {
        auto* space = createInsertSpace (indexToInsertAt, numberOfElements);

        for (i32 i = 0; i < numberOfElements; ++i)
            new (space++) ElementType (*(newElements++));

        numUsed += numberOfElements;
    }

    //==============================================================================
    z0 removeElements (i32 indexToRemoveAt, i32 numElementsToRemove)
    {
        jassert (indexToRemoveAt >= 0);
        jassert (numElementsToRemove >= 0);
        jassert (indexToRemoveAt + numElementsToRemove <= numUsed);

        if (numElementsToRemove > 0)
        {
            removeElementsInternal (indexToRemoveAt, numElementsToRemove);
            numUsed -= numElementsToRemove;
        }
    }

    //==============================================================================
    z0 swap (i32 index1, i32 index2)
    {
        if (isPositiveAndBelow (index1, numUsed)
         && isPositiveAndBelow (index2, numUsed))
        {
            std::swap (elements[index1],
                       elements[index2]);
        }
    }

    //==============================================================================
    z0 move (i32 currentIndex, i32 newIndex) noexcept
    {
        if (isPositiveAndBelow (currentIndex, numUsed))
        {
            if (! isPositiveAndBelow (newIndex, numUsed))
                newIndex = numUsed - 1;

            moveInternal (currentIndex, newIndex);
        }
    }

private:
    //==============================================================================
   #if defined (__GNUC__) && __GNUC__ < 5 && ! defined (__clang__)
    static constexpr auto isTriviallyCopyable = std::is_scalar_v<ElementType>;
   #else
    static constexpr auto isTriviallyCopyable = std::is_trivially_copyable_v<ElementType>;
   #endif

    //==============================================================================
    template <typename Type>
    z0 addArrayInternal (const Type* otherElements, i32 numElements)
    {
        if constexpr (isTriviallyCopyable && std::is_same_v<Type, ElementType>)
        {
            if (numElements > 0)
                memcpy (elements + numUsed, otherElements, (size_t) numElements * sizeof (ElementType));
        }
        else
        {
            auto* start = elements + numUsed;

            while (--numElements >= 0)
                new (start++) ElementType (*(otherElements++));
        }
    }

    //==============================================================================
    z0 setAllocatedSizeInternal (i32 numElements)
    {
        if constexpr (isTriviallyCopyable)
        {
            elements.realloc ((size_t) numElements);
        }
        else
        {
            HeapBlock<ElementType> newElements (numElements);

            for (i32 i = 0; i < numUsed; ++i)
            {
                new (newElements + i) ElementType (std::move (elements[i]));
                elements[i].~ElementType();
            }

            elements = std::move (newElements);
        }
    }

    //==============================================================================
    ElementType* createInsertSpace (i32 indexToInsertAt, i32 numElements)
    {
        ensureAllocatedSize (numUsed + numElements);

        if (! isPositiveAndBelow (indexToInsertAt, numUsed))
            return elements + numUsed;

        createInsertSpaceInternal (indexToInsertAt, numElements);

        return elements + indexToInsertAt;
    }

    z0 createInsertSpaceInternal (i32 indexToInsertAt, i32 numElements)
    {
        if constexpr (isTriviallyCopyable)
        {
            auto* start = elements + indexToInsertAt;
            auto numElementsToShift = numUsed - indexToInsertAt;
            memmove (start + numElements, start, (size_t) numElementsToShift * sizeof (ElementType));
        }
        else
        {
            auto* end = elements + numUsed;
            auto* newEnd = end + numElements;
            auto numElementsToShift = numUsed - indexToInsertAt;

            for (i32 i = 0; i < numElementsToShift; ++i)
            {
                new (--newEnd) ElementType (std::move (*(--end)));
                end->~ElementType();
            }
        }
    }

    //==============================================================================
    z0 removeElementsInternal (i32 indexToRemoveAt, i32 numElementsToRemove)
    {
        if constexpr (isTriviallyCopyable)
        {
            auto* start = elements + indexToRemoveAt;
            auto numElementsToShift = numUsed - (indexToRemoveAt + numElementsToRemove);
            memmove (start, start + numElementsToRemove, (size_t) numElementsToShift * sizeof (ElementType));
        }
        else
        {
            auto numElementsToShift = numUsed - (indexToRemoveAt + numElementsToRemove);
            auto* destination = elements + indexToRemoveAt;
            auto* source = destination + numElementsToRemove;

            for (i32 i = 0; i < numElementsToShift; ++i)
                moveAssignElement (destination++, std::move (*(source++)));

            for (i32 i = 0; i < numElementsToRemove; ++i)
                (destination++)->~ElementType();
        }
    }

    //==============================================================================
    z0 moveInternal (i32 currentIndex, i32 newIndex) noexcept
    {
        if constexpr (isTriviallyCopyable)
        {
            t8 tempCopy[sizeof (ElementType)];
            memcpy (tempCopy, elements + currentIndex, sizeof (ElementType));

            if (newIndex > currentIndex)
            {
                memmove (elements + currentIndex,
                         elements + currentIndex + 1,
                         (size_t) (newIndex - currentIndex) * sizeof (ElementType));
            }
            else
            {
                memmove (elements + newIndex + 1,
                         elements + newIndex,
                         (size_t) (currentIndex - newIndex) * sizeof (ElementType));
            }

            memcpy (elements + newIndex, tempCopy, sizeof (ElementType));
        }
        else
        {
            auto* e = elements + currentIndex;
            ElementType tempCopy (std::move (*e));
            auto delta = newIndex - currentIndex;

            if (delta > 0)
            {
                for (i32 i = 0; i < delta; ++i)
                {
                    moveAssignElement (e, std::move (*(e + 1)));
                    ++e;
                }
            }
            else
            {
                for (i32 i = 0; i < -delta; ++i)
                {
                    moveAssignElement (e, std::move (*(e - 1)));
                    --e;
                }
            }

            moveAssignElement (e, std::move (tempCopy));
        }
    }

    //==============================================================================
    template <typename... Elements>
    z0 addImpl (Elements&&... toAdd)
    {
        (checkSourceIsNotAMember (toAdd), ...);
        ensureAllocatedSize (numUsed + (i32) sizeof... (toAdd));
        addAssumingCapacityIsReady (std::forward<Elements> (toAdd)...);
    }

    template <typename... Elements>
    z0 addAssumingCapacityIsReady (Elements&&... toAdd)
    {
        (new (elements + numUsed++) ElementType (std::forward<Elements> (toAdd)), ...);
    }

    //==============================================================================
    z0 moveAssignElement (ElementType* destination, ElementType&& source)
    {
        if constexpr (std::is_move_assignable_v<ElementType>)
        {
            *destination = std::move (source);
        }
        else
        {
            destination->~ElementType();
            new (destination) ElementType (std::move (source));
        }
    }

    z0 checkSourceIsNotAMember ([[maybe_unused]] const ElementType& element)
    {
        // when you pass a reference to an existing element into a method like add() which
        // may need to reallocate the array to make more space, the incoming reference may
        // be deleted indirectly during the reallocation operation! To work around this,
        // make a local copy of the item you're trying to add (and maybe use std::move to
        // move it into the add() method to avoid any extra overhead)
        jassert (std::addressof (element) < begin() || end() <= std::addressof (element));
    }

    //==============================================================================
    HeapBlock<ElementType> elements;
    i32 numAllocated = 0, numUsed = 0;

    template <class OtherElementType, class OtherCriticalSection>
    friend class ArrayBase;

    DRX_DECLARE_NON_COPYABLE (ArrayBase)
};

} // namespace drx
