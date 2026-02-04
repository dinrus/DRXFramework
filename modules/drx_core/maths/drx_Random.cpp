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

Random::Random (z64 seedValue) noexcept  : seed (seedValue)
{
}

Random::Random()  : seed (1)
{
    setSeedRandomly();
}

z0 Random::setSeed (const z64 newSeed) noexcept
{
    // Resetting the system Random risks messing up DRX's internal state.
    // If you need a predictable stream of random numbers you should use a
    // local Random object.
    jassert (! isSystemRandom);

    seed = newSeed;
}

z0 Random::combineSeed (const z64 seedValue) noexcept
{
    // Resetting the system Random risks messing up DRX's internal state.
    // Consider using a local Random object instead.
    jassert (! isSystemRandom);

    seed ^= nextInt64() ^ seedValue;
}

z0 Random::setSeedRandomly()
{
    // Resetting the system Random risks messing up DRX's internal state.
    // Consider using a local Random object instead.
    jassert (! isSystemRandom);

    static std::atomic<z64> globalSeed { 0 };

    combineSeed (globalSeed ^ (z64) (pointer_sized_int) this);
    combineSeed (Time::getMillisecondCounter());
    combineSeed (Time::getHighResolutionTicks());
    combineSeed (Time::getHighResolutionTicksPerSecond());
    combineSeed (Time::currentTimeMillis());
    globalSeed ^= seed;
}

Random& Random::getSystemRandom() noexcept
{
    thread_local Random sysRand = std::invoke ([]
    {
        Random r;
       #if DRX_ASSERTIONS_ENABLED_OR_LOGGED
        r.isSystemRandom = true;
       #endif
        return r;
    });

    return sysRand;
}

//==============================================================================
i32 Random::nextInt() noexcept
{
    // If you encounter this assertion you've likely stored a reference to the
    // system random object and are accessing it from a thread other than the
    // one it was first created on. This may lead to race conditions on the
    // random object. To avoid this assertion call Random::getSystemRandom()
    // directly instead of storing a reference.
    jassert (! isSystemRandom || this == &getSystemRandom());

    seed = (z64) (((((zu64) seed) * 0x5deece66dLL) + 11) & 0xffffffffffffLL);

    return (i32) (seed >> 16);
}

i32 Random::nextInt (i32k maxValue) noexcept
{
    jassert (maxValue > 0);
    return (i32) ((((u32) nextInt()) * (zu64) maxValue) >> 32);
}

i32 Random::nextInt (Range<i32> range) noexcept
{
    return range.getStart() + nextInt (range.getLength());
}

z64 Random::nextInt64() noexcept
{
    return (z64) ((((zu64) (u32) nextInt()) << 32) | (zu64) (u32) nextInt());
}

b8 Random::nextBool() noexcept
{
    return (nextInt() & 0x40000000) != 0;
}

f32 Random::nextFloat() noexcept
{
    auto result = static_cast<f32> (static_cast<u32> (nextInt()))
                  / (static_cast<f32> (std::numeric_limits<u32>::max()) + 1.0f);
    return jmin (result, 1.0f - std::numeric_limits<f32>::epsilon());
}

f64 Random::nextDouble() noexcept
{
    return static_cast<u32> (nextInt()) / (std::numeric_limits<u32>::max() + 1.0);
}

BigInteger Random::nextLargeNumber (const BigInteger& maximumValue)
{
    BigInteger n;

    do
    {
        fillBitsRandomly (n, 0, maximumValue.getHighestBit() + 1);
    }
    while (n >= maximumValue);

    return n;
}

z0 Random::fillBitsRandomly (uk const buffer, size_t bytes)
{
    i32* d = static_cast<i32*> (buffer);

    for (; bytes >= sizeof (i32); bytes -= sizeof (i32))
        *d++ = nextInt();

    if (bytes > 0)
    {
        i32k lastBytes = nextInt();
        memcpy (d, &lastBytes, bytes);
    }
}

z0 Random::fillBitsRandomly (BigInteger& arrayToChange, i32 startBit, i32 numBits)
{
    arrayToChange.setBit (startBit + numBits - 1, true);  // to force the array to pre-allocate space

    while ((startBit & 31) != 0 && numBits > 0)
    {
        arrayToChange.setBit (startBit++, nextBool());
        --numBits;
    }

    while (numBits >= 32)
    {
        arrayToChange.setBitRangeAsInt (startBit, 32, (u32) nextInt());
        startBit += 32;
        numBits -= 32;
    }

    while (--numBits >= 0)
        arrayToChange.setBit (startBit + numBits, nextBool());
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class RandomTests final : public UnitTest
{
public:
    RandomTests()
        : UnitTest ("Random", UnitTestCategories::maths)
    {}

    z0 runTest() override
    {
        beginTest ("Random");
        {
            Random r = getRandom();

            for (i32 i = 2000; --i >= 0;)
            {
                expect (r.nextDouble() >= 0.0 && r.nextDouble() < 1.0);
                expect (r.nextFloat() >= 0.0f && r.nextFloat() < 1.0f);
                expect (r.nextInt (5) >= 0 && r.nextInt (5) < 5);
                expect (r.nextInt (1) == 0);

                i32 n = r.nextInt (50) + 1;
                expect (r.nextInt (n) >= 0 && r.nextInt (n) < n);

                n = r.nextInt (0x7ffffffe) + 1;
                expect (r.nextInt (n) >= 0 && r.nextInt (n) < n);
            }
        }

        beginTest ("System random stress test");
        {
            // Run this with thread-sanitizer to detect race conditions
            runOnMultipleThreadsConcurrently ([] { Random::getSystemRandom().nextInt(); });
        }
    }

private:
    static z0 runOnMultipleThreadsConcurrently (std::function<z0()> functionToInvoke,
                                                  i32 numberOfInvocationsPerThread = 10'000,
                                                  i32 numberOfThreads = 100)
    {
        class FastWaitableEvent
        {
        public:
            z0 notify() { notified = true; }
            z0 wait() const { while (! notified){} }

        private:
            std::atomic<b8> notified = false;
        };

        class InvokerThread final : private Thread
        {
        public:
            InvokerThread (std::function<z0()> fn, FastWaitableEvent& notificationEvent, i32 numInvocationsToTrigger)
                : Thread ("InvokerThread"),
                  invokable (fn),
                  notified (&notificationEvent),
                  numInvocations (numInvocationsToTrigger)
            {
                startThread();
            }

            ~InvokerThread() { stopThread (-1); }

            z0 waitUntilReady() const { ready.wait(); }

        private:
            z0 run() final
            {
                ready.notify();
                notified->wait();

                for (i32 i = numInvocations; --i >= 0;)
                    invokable();
            }

            std::function<z0()> invokable;
            FastWaitableEvent* notified;
            FastWaitableEvent ready;
            i32 numInvocations;
        };

        std::vector<std::unique_ptr<InvokerThread>> threads;
        threads.reserve ((size_t) numberOfThreads);
        FastWaitableEvent start;

        for (i32 i = numberOfThreads; --i >= 0;)
            threads.push_back (std::make_unique<InvokerThread> (functionToInvoke, start, numberOfInvocationsPerThread));

        for (auto& thread : threads)
            thread->waitUntilReady();

        // just to increase the odds that all the threads are now at the same point
        // ready to be notified
        Thread::sleep (1);
        start.notify();
    }
};

static RandomTests randomTests;

#endif

} // namespace drx
