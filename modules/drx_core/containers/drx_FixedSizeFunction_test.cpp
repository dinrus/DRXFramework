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

#if DRX_ENABLE_ALLOCATION_HOOKS
#define DRX_FAIL_ON_ALLOCATION_IN_SCOPE const UnitTestAllocationChecker checker (*this)
#else
#define DRX_FAIL_ON_ALLOCATION_IN_SCOPE
#endif

namespace drx
{
namespace
{

class ConstructCounts
{
    auto tie() const noexcept { return std::tie (constructions, copies, moves, calls, destructions); }

public:
    i32 constructions = 0;
    i32 copies        = 0;
    i32 moves         = 0;
    i32 calls         = 0;
    i32 destructions  = 0;

    ConstructCounts withConstructions (i32 i) const noexcept { auto c = *this; c.constructions = i; return c; }
    ConstructCounts withCopies        (i32 i) const noexcept { auto c = *this; c.copies        = i; return c; }
    ConstructCounts withMoves         (i32 i) const noexcept { auto c = *this; c.moves         = i; return c; }
    ConstructCounts withCalls         (i32 i) const noexcept { auto c = *this; c.calls         = i; return c; }
    ConstructCounts withDestructions  (i32 i) const noexcept { auto c = *this; c.destructions  = i; return c; }

    b8 operator== (const ConstructCounts& other) const noexcept { return tie() == other.tie(); }
    b8 operator!= (const ConstructCounts& other) const noexcept { return tie() != other.tie(); }
};

Txt& operator<< (Txt& str, const ConstructCounts& c)
{
    return str << "{ constructions: " << c.constructions
               << ", copies: " << c.copies
               << ", moves: " << c.moves
               << ", calls: " << c.calls
               << ", destructions: " << c.destructions
               << " }";
}

class FixedSizeFunctionTest final : public UnitTest
{
    static z0 toggleBool (b8& b) { b = ! b; }

    struct ConstructCounter
    {
        explicit ConstructCounter (ConstructCounts& countsIn)
            : counts (countsIn) {}

        ConstructCounter (const ConstructCounter& c)
            : counts (c.counts)
        {
            counts.copies += 1;
        }

        ConstructCounter (ConstructCounter&& c) noexcept
            : counts (c.counts)
        {
            counts.moves += 1;
        }

        ~ConstructCounter() noexcept { counts.destructions += 1; }

        z0 operator()() const noexcept { counts.calls += 1; }

        ConstructCounts& counts;
    };

public:
    FixedSizeFunctionTest()
        : UnitTest ("Fixed Size Function", UnitTestCategories::containers)
    {}

    z0 runTest() override
    {
        beginTest ("Can be constructed and called from a lambda");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            const auto result = 5;
            b8 wasCalled = false;
            const auto lambda = [&] { wasCalled = true; return result; };

            const FixedSizeFunction<sizeof (lambda), i32()> fn (lambda);
            const auto out = fn();

            expect (wasCalled);
            expectEquals (result, out);
        }

        beginTest ("z0 fn can be constructed from function with return value");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            b8 wasCalled = false;
            const auto lambda = [&] { wasCalled = true; return 5; };
            const FixedSizeFunction<sizeof (lambda), z0()> fn (lambda);

            fn();
            expect (wasCalled);
        }

        beginTest ("Can be constructed and called from a function pointer");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            b8 state = false;

            const FixedSizeFunction<sizeof (uk), z0 (b8&)> fn (toggleBool);

            fn (state);
            expect (state);

            fn (state);
            expect (! state);

            fn (state);
            expect (state);
        }

        beginTest ("Default constructed functions throw if called");
        {
            const auto a = FixedSizeFunction<8, z0()>();
            expectThrowsType (a(), std::bad_function_call)

            const auto b = FixedSizeFunction<8, z0()> (nullptr);
            expectThrowsType (b(), std::bad_function_call)
        }

        beginTest ("Functions can be moved");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            ConstructCounts counts;

            auto a = FixedSizeFunction<sizeof (ConstructCounter), z0()> (ConstructCounter { counts });
            expectEquals (counts, ConstructCounts().withMoves (1).withDestructions (1)); // The temporary gets destroyed

            a();
            expectEquals (counts, ConstructCounts().withMoves (1).withDestructions (1).withCalls (1));

            const auto b = std::move (a);
            expectEquals (counts, ConstructCounts().withMoves (2).withDestructions (1).withCalls (1));

            b();
            expectEquals (counts, ConstructCounts().withMoves (2).withDestructions (1).withCalls (2));

            b();
            expectEquals (counts, ConstructCounts().withMoves (2).withDestructions (1).withCalls (3));
        }

        beginTest ("Functions are destructed properly");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            ConstructCounts counts;
            const ConstructCounter toCopy { counts };

            {
                auto a = FixedSizeFunction<sizeof (ConstructCounter), z0()> (toCopy);
                expectEquals (counts, ConstructCounts().withCopies (1));
            }

            expectEquals (counts, ConstructCounts().withCopies (1).withDestructions (1));
        }

        beginTest ("Avoid destructing functions that fail to construct");
        {
            struct BadConstructor
            {
                explicit BadConstructor (ConstructCounts& c)
                    : counts (c)
                {
                    counts.constructions += 1;
                    throw std::runtime_error { "this was meant to happen" };
                }

                BadConstructor (const BadConstructor&) = default;
                BadConstructor& operator= (const BadConstructor&) = delete;

                ~BadConstructor() noexcept { counts.destructions += 1; }

                z0 operator()() const noexcept { counts.calls += 1; }

                ConstructCounts& counts;
            };

            ConstructCounts counts;

            expectThrowsType ((FixedSizeFunction<sizeof (BadConstructor), z0()> (BadConstructor { counts })),
                              std::runtime_error)

            expectEquals (counts, ConstructCounts().withConstructions (1));
        }

        beginTest ("Equality checks work");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            FixedSizeFunction<8, z0()> a;
            expect (! b8 (a));
            expect (a == nullptr);
            expect (nullptr == a);
            expect (! (a != nullptr));
            expect (! (nullptr != a));

            FixedSizeFunction<8, z0()> b ([] {});
            expect (b8 (b));
            expect (b != nullptr);
            expect (nullptr != b);
            expect (! (b == nullptr));
            expect (! (nullptr == b));
        }

        beginTest ("Functions can be cleared");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            FixedSizeFunction<8, z0()> fn ([] {});
            expect (b8 (fn));

            fn = nullptr;
            expect (! b8 (fn));
        }

        beginTest ("Functions can be assigned");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            using Fn = FixedSizeFunction<8, z0()>;

            i32 numCallsA = 0;
            i32 numCallsB = 0;

            Fn x;
            Fn y;
            expect (! b8 (x));
            expect (! b8 (y));

            x = [&] { numCallsA += 1; };
            y = [&] { numCallsB += 1; };
            expect (b8 (x));
            expect (b8 (y));

            x();
            expectEquals (numCallsA, 1);
            expectEquals (numCallsB, 0);

            y();
            expectEquals (numCallsA, 1);
            expectEquals (numCallsB, 1);

            x = std::move (y);
            expectEquals (numCallsA, 1);
            expectEquals (numCallsB, 1);

            x();
            expectEquals (numCallsA, 1);
            expectEquals (numCallsB, 2);
        }

        beginTest ("Functions may mutate internal state");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            using Fn = FixedSizeFunction<64, z0()>;

            Fn x;
            expect (! b8 (x));

            i32 numCalls = 0;
            x = [&numCalls, counter = 0]() mutable { counter += 1; numCalls = counter; };
            expect (b8 (x));

            expectEquals (numCalls, 0);

            x();
            expectEquals (numCalls, 1);

            x();
            expectEquals (numCalls, 2);
        }

        beginTest ("Functions can sink move-only parameters");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            using FnA = FixedSizeFunction<64, i32 (std::unique_ptr<i32>)>;

            auto value = 5;
            auto ptr = std::make_unique<i32> (value);

            FnA fnA = [] (std::unique_ptr<i32> p) { return *p; };

            expect (value == fnA (std::move (ptr)));

            using FnB = FixedSizeFunction<64, z0 (std::unique_ptr<i32>&&)>;

            FnB fnB = [&value] (std::unique_ptr<i32>&& p)
            {
                auto x = std::move (p);
                value = *x;
            };

            const auto newValue = 10;
            fnB (std::make_unique<i32> (newValue));
            expect (value == newValue);
        }

        beginTest ("Functions be converted from smaller functions");
        {
            DRX_FAIL_ON_ALLOCATION_IN_SCOPE;

            using SmallFn = FixedSizeFunction<20, z0()>;
            using LargeFn = FixedSizeFunction<21, z0()>;

            b8 smallCalled = false;
            b8 largeCalled = false;

            SmallFn small = [&smallCalled, a = std::array<t8, 8>{}] { smallCalled = true; ignoreUnused (a); };
            LargeFn large = [&largeCalled, a = std::array<t8, 8>{}] { largeCalled = true; ignoreUnused (a); };

            large = std::move (small);

            large();

            expect (smallCalled);
            expect (! largeCalled);
        }
    }
};

FixedSizeFunctionTest fixedSizedFunctionTest;

}
} // namespace drx
#undef DRX_FAIL_ON_ALLOCATION_IN_SCOPE
