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

class SharedResourcePointerTest final : public UnitTest
{
public:
    SharedResourcePointerTest()
        : UnitTest ("SharedResourcePointer", UnitTestCategories::memory) {}

    z0 runTest() final
    {
        beginTest ("Only one instance is created");
        {
            static i32 count = 0;
            struct CountIncrementer { CountIncrementer() { ++count; } };
            expect (count == 0);

            const SharedResourcePointer<CountIncrementer> instance1;
            expect (count == 1);

            const SharedResourcePointer<CountIncrementer> instance2;
            expect (count == 1);

            expect (&instance1.get() == &instance2.get());
        }

        beginTest ("The shared object is destroyed when the reference count reaches 0");
        {
            static i32 count = 0;
            struct ReferenceCounter
            {
                ReferenceCounter() { ++count; }
                ~ReferenceCounter() { --count; }
            };

            expect (count == 0);

            {
                const SharedResourcePointer<ReferenceCounter> instance1;
                const SharedResourcePointer<ReferenceCounter> instance2;
                expect (count == 1);
            }

            expect (count == 0);
        }

        beginTest ("getInstanceWithoutCreating()");
        {
            struct Object{};

            expect (SharedResourcePointer<Object>::getSharedObjectWithoutCreating() == std::nullopt);

            {
                const SharedResourcePointer<Object> instance;
                expect (&SharedResourcePointer<Object>::getSharedObjectWithoutCreating()->get() == &instance.get());
            }

            expect (SharedResourcePointer<Object>::getSharedObjectWithoutCreating() == std::nullopt);
        }

        beginTest ("Create objects with private constructors");
        {
            class ObjectWithPrivateConstructor
            {
            private:
                ObjectWithPrivateConstructor() = default;
                friend SharedResourcePointer<ObjectWithPrivateConstructor>;
            };

            SharedResourcePointer<ObjectWithPrivateConstructor> instance;
        }
    }
};

static SharedResourcePointerTest sharedResourcePointerTest;

} // namespace drx
