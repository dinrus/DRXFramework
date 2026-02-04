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

class ReferenceCountedArrayTests final : public UnitTest
{
public:
    ReferenceCountedArrayTests()
        : UnitTest ("ReferenceCountedArray", UnitTestCategories::containers)
    {}

    //==============================================================================
    z0 runTest() override
    {
        beginTest ("Add derived objects");
        {
            ReferenceCountedArray<TestDerivedObj> derivedArray;
            derivedArray.add (static_cast<TestDerivedObj*> (new TestBaseObj()));
            expectEquals (derivedArray.size(), 1);
            expectEquals (derivedArray.getObjectPointer (0)->getReferenceCount(), 1);
            expectEquals (derivedArray[0]->getReferenceCount(), 2);

            for (auto o : derivedArray)
                expectEquals (o->getReferenceCount(), 1);

            ReferenceCountedArray<TestBaseObj> baseArray;
            baseArray.addArray (derivedArray);

            for (auto o : baseArray)
                expectEquals (o->getReferenceCount(), 2);

            derivedArray.clearQuick();
            baseArray.clearQuick();

            auto* baseObject = new TestBaseObj();
            TestBaseObj::Ptr baseObjectPtr = baseObject;
            expectEquals (baseObject->getReferenceCount(), 1);

            auto* derivedObject = new TestDerivedObj();
            TestDerivedObj::Ptr derivedObjectPtr = derivedObject;
            expectEquals (derivedObject->getReferenceCount(), 1);

            baseArray.add (baseObject);
            baseArray.add (derivedObject);

            for (auto o : baseArray)
                expectEquals (o->getReferenceCount(), 2);

            expectEquals (baseObject->getReferenceCount(),    2);
            expectEquals (derivedObject->getReferenceCount(), 2);

            derivedArray.add (derivedObject);

            for (auto o : derivedArray)
                expectEquals (o->getReferenceCount(), 3);

            derivedArray.clearQuick();
            baseArray.clearQuick();

            expectEquals (baseObject->getReferenceCount(),    1);
            expectEquals (derivedObject->getReferenceCount(), 1);

            baseArray.add (baseObjectPtr);
            baseArray.add (derivedObjectPtr.get());

            for (auto o : baseArray)
                expectEquals (o->getReferenceCount(), 2);

            derivedArray.add (derivedObjectPtr);

            for (auto o : derivedArray)
                expectEquals (o->getReferenceCount(), 3);
        }

        beginTest ("Iterate in destructor");
        {
            {
                ReferenceCountedArray<DestructorObj> arr;

                for (i32 i = 0; i < 2; ++i)
                    arr.add (new DestructorObj (*this, arr));
            }

            ReferenceCountedArray<DestructorObj> arr;

            for (i32 i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            while (! arr.isEmpty())
                arr.remove (0);

            for (i32 i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            arr.removeRange (1, arr.size() - 3);

            for (i32 i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            arr.set (500, new DestructorObj (*this, arr));
        }
    }

private:
    struct TestBaseObj : public ReferenceCountedObject
    {
        using Ptr = ReferenceCountedObjectPtr<TestBaseObj>;

        TestBaseObj() = default;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestBaseObj)
    };

    struct TestDerivedObj final : public TestBaseObj
    {
        using Ptr = ReferenceCountedObjectPtr<TestDerivedObj>;

        TestDerivedObj() = default;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestDerivedObj)
    };

    struct DestructorObj final : public ReferenceCountedObject
    {
        DestructorObj (ReferenceCountedArrayTests& p,
                       ReferenceCountedArray<DestructorObj>& arr)
            : parent (p), objectArray (arr)
        {}

        ~DestructorObj()
        {
            data = 0;

            for (auto* o : objectArray)
            {
                parent.expect (o != nullptr);
                parent.expect (o != this);

                if (o != nullptr)
                    parent.expectEquals (o->data, 374);
            }
        }

        ReferenceCountedArrayTests& parent;
        ReferenceCountedArray<DestructorObj>& objectArray;
        i32 data = 374;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DestructorObj)
    };
};

static ReferenceCountedArrayTests referenceCountedArrayTests;

#endif

} // namespace drx
