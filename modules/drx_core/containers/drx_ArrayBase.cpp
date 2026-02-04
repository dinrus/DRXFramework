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

#if DRX_UNIT_TESTS

namespace ArrayBaseTestsHelpers
{
    class TriviallyCopyableType
    {
    public:
        TriviallyCopyableType() = default;

        TriviallyCopyableType (i32 v)
            : value (v)
        {}

        TriviallyCopyableType (f32 v)
            : value ((i32) v)
        {}

        b8 operator== (const TriviallyCopyableType& other) const
        {
            return getValue() == other.getValue();
        }

        i32 getValue() const   { return value; }

    private:
        i32 value { -1111 };
    };

    class NonTriviallyCopyableType
    {
    public:
        NonTriviallyCopyableType() = default;

        NonTriviallyCopyableType (i32 v)
            : value (v)
        {}

        NonTriviallyCopyableType (f32 v)
            : value ((i32) v)
        {}

        NonTriviallyCopyableType (const NonTriviallyCopyableType& other)
            : value (other.value)
        {}

        NonTriviallyCopyableType& operator= (const NonTriviallyCopyableType& other)
        {
            value = other.value;
            return *this;
        }

        b8 operator== (const NonTriviallyCopyableType& other) const
        {
            return getValue() == other.getValue();
        }

        i32 getValue() const   { return *ptr; }

    private:
        i32 value { -1111 };
        i32* ptr = &value;
    };
}

static b8 operator== (const ArrayBaseTestsHelpers::TriviallyCopyableType& tct,
                        const ArrayBaseTestsHelpers::NonTriviallyCopyableType& ntct)
{
    return tct.getValue() == ntct.getValue();
}

static b8 operator== (const ArrayBaseTestsHelpers::NonTriviallyCopyableType& ntct,
                        const ArrayBaseTestsHelpers::TriviallyCopyableType& tct)
{
    return tct == ntct;
}

class ArrayBaseTests final : public UnitTest
{
    using CopyableType    = ArrayBaseTestsHelpers::TriviallyCopyableType;
    using NoncopyableType = ArrayBaseTestsHelpers::NonTriviallyCopyableType;

   #if ! (defined (__GNUC__) && __GNUC__ < 5 && ! defined (__clang__))
    static_assert (std::is_trivially_copyable_v<CopyableType>,
                   "Test TriviallyCopyableType is not trivially copyable");
    static_assert (! std::is_trivially_copyable_v<NoncopyableType>,
                   "Test NonTriviallyCopyableType is trivially copyable");
   #endif

public:
    ArrayBaseTests()
        : UnitTest ("ArrayBase", UnitTestCategories::containers)
    {}

    z0 runTest() override
    {
        beginTest ("grow capacity");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            i32 originalCapacity = 4;
            referenceContainer.reserve ((size_t) originalCapacity);
            expectEquals ((i32) referenceContainer.capacity(), originalCapacity);
            copyableContainer.setAllocatedSize (originalCapacity);
            expectEquals (copyableContainer.capacity(), originalCapacity);
            noncopyableContainer.setAllocatedSize (originalCapacity);
            expectEquals (noncopyableContainer.capacity(), originalCapacity);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            addData (referenceContainer, copyableContainer, noncopyableContainer, 33);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            expect ((i32) referenceContainer.capacity() != originalCapacity);
            expect (copyableContainer.capacity()        != originalCapacity);
            expect (noncopyableContainer.capacity()     != originalCapacity);
        }

        beginTest ("shrink capacity");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            i32 numElements = 45;
            addData (referenceContainer, copyableContainer, noncopyableContainer, numElements);

            copyableContainer.shrinkToNoMoreThan (numElements);
            noncopyableContainer.setAllocatedSize (numElements + 1);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            referenceContainer.clear();
            copyableContainer.removeElements    (0, numElements);
            noncopyableContainer.removeElements (0, numElements);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            copyableContainer.setAllocatedSize    (0);
            noncopyableContainer.setAllocatedSize (0);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            addData (referenceContainer, copyableContainer, noncopyableContainer, numElements);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
        }

        beginTest ("equality");
        {
            std::vector<i32> referenceContainer = { 1, 2, 3 };
            ArrayBase<i32, DummyCriticalSection> testContainer1, testContainer2;

            for (auto i : referenceContainer)
            {
                testContainer1.add (i);
                testContainer2.add (i);
            }

            expect (testContainer1 == referenceContainer);
            expect (testContainer2 == testContainer1);

            testContainer1.ensureAllocatedSize (257);
            referenceContainer.shrink_to_fit();

            expect (testContainer1 == referenceContainer);
            expect (testContainer2 == testContainer1);

            testContainer1.removeElements (0, 1);

            expect (testContainer1 != referenceContainer);
            expect (testContainer2 != testContainer1);
        }

        beginTest ("accessors");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            addData (referenceContainer, copyableContainer, noncopyableContainer, 3);

            i32 testValue = -123;
            referenceContainer[0]   = testValue;
            copyableContainer[0]    = testValue;
            noncopyableContainer[0] = testValue;

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            expect (copyableContainer   .getFirst().getValue() == testValue);
            expect (noncopyableContainer.getFirst().getValue() == testValue);

            auto last = referenceContainer.back().getValue();

            expectEquals (copyableContainer   .getLast().getValue(), last);
            expectEquals (noncopyableContainer.getLast().getValue(), last);

            ArrayBase<CopyableType,    DummyCriticalSection> copyableEmpty;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableEmpty;

            auto defualtValue = CopyableType().getValue();
            expectEquals (defualtValue, NoncopyableType().getValue());

            expectEquals (copyableEmpty   .getFirst().getValue(), defualtValue);
            expectEquals (noncopyableEmpty.getFirst().getValue(), defualtValue);
            expectEquals (copyableEmpty   .getLast() .getValue(), defualtValue);
            expectEquals (noncopyableEmpty.getLast() .getValue(), defualtValue);

            ArrayBase<f32*, DummyCriticalSection> floatPointers;
            expect (floatPointers.getValueWithDefault (-3) == nullptr);
        }

        beginTest ("add moved");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            for (i32 i = 0; i < 5; ++i)
            {
                CopyableType ref    (-i);
                CopyableType ct     (-i);
                NoncopyableType nct (-i);
                referenceContainer.push_back (std::move (ref));
                copyableContainer.add (std::move (ct));
                noncopyableContainer.add (std::move (nct));
            }

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
        }

        beginTest ("add multiple");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            for (i32 i = 4; i < 7; ++i)
                referenceContainer.push_back ({ -i });

            copyableContainer.add    (CopyableType    (-4), CopyableType    (-5), CopyableType    (-6));
            noncopyableContainer.add (NoncopyableType (-4), NoncopyableType (-5), NoncopyableType (-6));

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
        }

        beginTest ("add array from a pointer");
        {
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            std::vector<CopyableType>    copyableData    = { 3, 4, 5 };
            std::vector<NoncopyableType> noncopyableData = { 3, 4, 5 };

            copyableContainer.addArray    (copyableData.data(),    (i32) copyableData.size());
            noncopyableContainer.addArray (noncopyableData.data(), (i32) noncopyableData.size());

            checkEqual (copyableContainer, noncopyableContainer, copyableData);
        }

        beginTest ("add array from a pointer of a different type");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            std::vector<f32> floatData = { 1.4f, 2.5f, 3.6f };

            for (auto f : floatData)
                referenceContainer.push_back ({ f });

            copyableContainer.addArray    (floatData.data(), (i32) floatData.size());
            noncopyableContainer.addArray (floatData.data(), (i32) floatData.size());

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
        }

        beginTest ("add array from initializer_list");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            std::initializer_list<CopyableType>    ilct  { { 3 }, { 4 }, { 5 } };
            std::initializer_list<NoncopyableType> ilnct { { 3 }, { 4 }, { 5 } };

            for (auto v : ilct)
                referenceContainer.push_back ({ v });

            copyableContainer.addArray    (ilct);
            noncopyableContainer.addArray (ilnct);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
        }

        beginTest ("add array from containers");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            addData (referenceContainer, copyableContainer, noncopyableContainer, 5);

            std::vector<CopyableType> referenceContainerCopy (referenceContainer);
            std::vector<NoncopyableType> noncopyableReferenceContainerCopy;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainerCopy;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainerCopy;

            for (auto& v : referenceContainerCopy)
                noncopyableReferenceContainerCopy.push_back ({ v.getValue() });

            for (size_t i = 0; i < referenceContainerCopy.size(); ++i)
            {
                auto value = referenceContainerCopy[i].getValue();
                copyableContainerCopy.add    ({ value });
                noncopyableContainerCopy.add ({ value });
            }

            // From self-types
            copyableContainer.addArray    (copyableContainerCopy);
            noncopyableContainer.addArray (noncopyableContainerCopy);

            for (auto v : referenceContainerCopy)
                referenceContainer.push_back (v);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            // From std containers
            copyableContainer.addArray    (referenceContainerCopy);
            noncopyableContainer.addArray (noncopyableReferenceContainerCopy);

            for (auto v : referenceContainerCopy)
                referenceContainer.push_back (v);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            // From std containers with offset
            i32 offset = 5;
            copyableContainer.addArray    (referenceContainerCopy,            offset);
            noncopyableContainer.addArray (noncopyableReferenceContainerCopy, offset);

            for (size_t i = 5; i < referenceContainerCopy.size(); ++i)
                referenceContainer.push_back (referenceContainerCopy[i]);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
        }

        beginTest ("insert");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            addData (referenceContainer, copyableContainer, noncopyableContainer, 8);

            referenceContainer.insert (referenceContainer.begin(), -4);
            copyableContainer.insert    (0, -4, 1);
            noncopyableContainer.insert (0, -4, 1);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            for (i32 i = 0; i < 3; ++i)
                referenceContainer.insert (referenceContainer.begin() + 1, -3);

            copyableContainer.insert    (1, -3, 3);
            noncopyableContainer.insert (1, -3, 3);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            for (i32 i = 0; i < 50; ++i)
                referenceContainer.insert (referenceContainer.end() - 1, -9);

            copyableContainer.insert    (copyableContainer.size()    - 2, -9, 50);
            noncopyableContainer.insert (noncopyableContainer.size() - 2, -9, 50);
        }

        beginTest ("insert array");
        {
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            std::vector<CopyableType>    copyableData    = { 3, 4, 5, 6, 7, 8 };
            std::vector<NoncopyableType> noncopyableData = { 3, 4, 5, 6, 7, 8 };

            std::vector<CopyableType> referenceContainer { copyableData };

            copyableContainer.insertArray    (0, copyableData.data(),    (i32) copyableData.size());
            noncopyableContainer.insertArray (0, noncopyableData.data(), (i32) noncopyableData.size());

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            i32 insertPos = copyableContainer.size() - 1;

            for (auto it = copyableData.end(); it != copyableData.begin(); --it)
                referenceContainer.insert (referenceContainer.begin() + insertPos, CopyableType (*(it - 1)));

            copyableContainer.insertArray    (insertPos, copyableData.data(),    (i32) copyableData.size());
            noncopyableContainer.insertArray (insertPos, noncopyableData.data(), (i32) noncopyableData.size());

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
        }

        beginTest ("remove");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            addData (referenceContainer, copyableContainer, noncopyableContainer, 17);

            for (i32 i = 0; i < 4; ++i)
            {
                referenceContainer.erase (referenceContainer.begin() + i);
                copyableContainer.removeElements (i, 1);
                noncopyableContainer.removeElements (i, 1);
            }

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            addData (referenceContainer, copyableContainer, noncopyableContainer, 17);
            i32 blockSize = 3;

            for (i32 i = 0; i < 4; ++i)
            {
                for (i32 j = 0; j < blockSize; ++j)
                    referenceContainer.erase (referenceContainer.begin() + i);

                copyableContainer.removeElements (i, blockSize);
                noncopyableContainer.removeElements (i, blockSize);
            }

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);

            auto numToRemove = copyableContainer.size() - 2;

            for (i32 i = 0; i < numToRemove; ++i)
                referenceContainer.erase (referenceContainer.begin() + 1);

            copyableContainer.removeElements    (1, numToRemove);
            noncopyableContainer.removeElements (1, numToRemove);

            checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
        }

        beginTest ("move");
        {
            std::vector<CopyableType> referenceContainer;
            ArrayBase<CopyableType,    DummyCriticalSection> copyableContainer;
            ArrayBase<NoncopyableType, DummyCriticalSection> noncopyableContainer;

            addData (referenceContainer, copyableContainer, noncopyableContainer, 6);

            std::vector<std::pair<i32, i32>> testValues;
            testValues.emplace_back (2, 4);
            testValues.emplace_back (0, 5);
            testValues.emplace_back (4, 1);
            testValues.emplace_back (5, 0);

            for (auto p : testValues)
            {
                if (p.second > p.first)
                    std::rotate (referenceContainer.begin() + p.first,
                                 referenceContainer.begin() + p.first + 1,
                                 referenceContainer.begin() + p.second + 1);
                else
                    std::rotate (referenceContainer.begin() + p.second,
                                 referenceContainer.begin() + p.first,
                                 referenceContainer.begin() + p.first + 1);

                copyableContainer.move    (p.first, p.second);
                noncopyableContainer.move (p.first, p.second);

                checkEqual (copyableContainer, noncopyableContainer, referenceContainer);
            }
        }

        beginTest ("After converting move construction, ownership is transferred");
        {
            Derived obj;
            ArrayBase<Derived*, DummyCriticalSection> derived;
            derived.setAllocatedSize (5);
            derived.add (&obj);

            ArrayBase<Base*, DummyCriticalSection> base { std::move (derived) };

            expectEquals (base.capacity(), 5);
            expectEquals (base.size(), 1);
            expect (base.getFirst() == &obj);
            expectEquals (derived.capacity(), 0);
            expectEquals (derived.size(), 0);
            expect (derived.data() == nullptr);
        }

        beginTest ("After converting move assignment, ownership is transferred");
        {
            Derived obj;
            ArrayBase<Derived*, DummyCriticalSection> derived;
            derived.setAllocatedSize (5);
            derived.add (&obj);
            ArrayBase<Base*, DummyCriticalSection> base;

            base = std::move (derived);

            expectEquals (base.capacity(), 5);
            expectEquals (base.size(), 1);
            expect (base.getFirst() == &obj);
            expectEquals (derived.capacity(), 0);
            expectEquals (derived.size(), 0);
            expect (derived.data() == nullptr);
        }
    }

private:
    struct Base
    {
        virtual ~Base() = default;
    };

    struct Derived final : public Base
    {
    };

    static z0 addData (std::vector<CopyableType>& referenceContainer,
                         ArrayBase<CopyableType,    DummyCriticalSection>& copyableContainer,
                         ArrayBase<NoncopyableType, DummyCriticalSection>& NoncopyableContainer,
                         i32 numValues)
    {
        for (i32 i = 0; i < numValues; ++i)
        {
            referenceContainer.push_back ({ i });
            copyableContainer.add ({ i });
            NoncopyableContainer.add ({ i });
        }
    }

    template <typename A, typename B>
    z0 checkEqual (const ArrayBase<A, DummyCriticalSection>& a,
                     const ArrayBase<B, DummyCriticalSection>& b)
    {
        expectEquals ((i32) a.size(), (i32) b.size());

        for (i32 i = 0; i < (i32) a.size(); ++i)
            expect (a[i] == b[i]);
    }

    template <typename A, typename B>
    z0 checkEqual (ArrayBase<A, DummyCriticalSection>& a,
                     std::vector<B>& b)
    {
        expectEquals ((i32) a.size(), (i32) b.size());

        for (i32 i = 0; i < (i32) a.size(); ++i)
            expect (a[i] == b[(size_t) i]);
    }

    template <typename A, typename B, typename C>
    z0 checkEqual (ArrayBase<A, DummyCriticalSection>& a,
                     ArrayBase<B, DummyCriticalSection>& b,
                     std::vector<C>& c)
    {
        checkEqual (a, b);
        checkEqual (a, c);
        checkEqual (b, c);
    }
};

static ArrayBaseTests arrayBaseTests;

#endif

} // namespace drx
