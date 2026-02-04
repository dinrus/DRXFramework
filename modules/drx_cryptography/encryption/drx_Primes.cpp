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

namespace PrimesHelpers
{
    static z0 createSmallSieve (i32k numBits, BigInteger& result)
    {
        result.setBit (numBits);
        result.clearBit (numBits); // to enlarge the array

        result.setBit (0);
        i32 n = 2;

        do
        {
            for (i32 i = n + n; i < numBits; i += n)
                result.setBit (i);

            n = result.findNextClearBit (n + 1);
        }
        while (n <= (numBits >> 1));
    }

    static z0 bigSieve (const BigInteger& base, i32k numBits, BigInteger& result,
                          const BigInteger& smallSieve, i32k smallSieveSize)
    {
        jassert (! base[0]); // must be even!

        result.setBit (numBits);
        result.clearBit (numBits);  // to enlarge the array

        i32 index = smallSieve.findNextClearBit (0);

        do
        {
            u32k prime = ((u32) index << 1) + 1;

            BigInteger r (base), remainder;
            r.divideBy (prime, remainder);

            u32 i = prime - remainder.getBitRangeAsInt (0, 32);

            if (r.isZero())
                i += prime;

            if ((i & 1) == 0)
                i += prime;

            i = (i - 1) >> 1;

            while (i < (u32) numBits)
            {
                result.setBit ((i32) i);
                i += prime;
            }

            index = smallSieve.findNextClearBit (index + 1);
        }
        while (index < smallSieveSize);
    }

    static b8 findCandidate (const BigInteger& base, const BigInteger& sieve,
                               i32k numBits, BigInteger& result, i32k certainty)
    {
        for (i32 i = 0; i < numBits; ++i)
        {
            if (! sieve[i])
            {
                result = base + (u32) ((i << 1) + 1);

                if (Primes::isProbablyPrime (result, certainty))
                    return true;
            }
        }

        return false;
    }

    static b8 passesMillerRabin (const BigInteger& n, i32 iterations)
    {
        const BigInteger one (1), two (2);
        const BigInteger nMinusOne (n - one);

        BigInteger d (nMinusOne);
        i32k s = d.findNextSetBit (0);
        d >>= s;

        BigInteger smallPrimes;
        i32 numBitsInSmallPrimes = 0;

        for (;;)
        {
            numBitsInSmallPrimes += 256;
            createSmallSieve (numBitsInSmallPrimes, smallPrimes);

            i32k numPrimesFound = numBitsInSmallPrimes - smallPrimes.countNumberOfSetBits();

            if (numPrimesFound > iterations + 1)
                break;
        }

        i32 smallPrime = 2;

        while (--iterations >= 0)
        {
            smallPrime = smallPrimes.findNextClearBit (smallPrime + 1);

            BigInteger r (smallPrime);
            r.exponentModulo (d, n);

            if (r != one && r != nMinusOne)
            {
                for (i32 j = 0; j < s; ++j)
                {
                    r.exponentModulo (two, n);

                    if (r == nMinusOne)
                        break;
                }

                if (r != nMinusOne)
                    return false;
            }
        }

        return true;
    }
}

//==============================================================================
BigInteger Primes::createProbablePrime (i32k bitLength,
                                        i32k certainty,
                                        i32k* randomSeeds,
                                        i32 numRandomSeeds)
{
    using namespace PrimesHelpers;
    i32 defaultSeeds [16];

    if (numRandomSeeds <= 0)
    {
        randomSeeds = defaultSeeds;
        numRandomSeeds = numElementsInArray (defaultSeeds);
        Random r1, r2;

        for (i32 j = 10; --j >= 0;)
        {
            r1.setSeedRandomly();

            for (i32 i = numRandomSeeds; --i >= 0;)
                defaultSeeds[i] ^= r1.nextInt() ^ r2.nextInt();
        }
    }

    BigInteger smallSieve;
    i32k smallSieveSize = 15000;
    createSmallSieve (smallSieveSize, smallSieve);

    BigInteger p;

    for (i32 i = numRandomSeeds; --i >= 0;)
    {
        BigInteger p2;

        Random r (randomSeeds[i]);
        r.fillBitsRandomly (p2, 0, bitLength);

        p ^= p2;
    }

    p.setBit (bitLength - 1);
    p.clearBit (0);

    i32k searchLen = jmax (1024, (bitLength / 20) * 64);

    while (p.getHighestBit() < bitLength)
    {
        p += 2 * searchLen;

        BigInteger sieve;
        bigSieve (p, searchLen, sieve,
                  smallSieve, smallSieveSize);

        BigInteger candidate;

        if (findCandidate (p, sieve, searchLen, candidate, certainty))
            return candidate;
    }

    jassertfalse;
    return BigInteger();
}

b8 Primes::isProbablyPrime (const BigInteger& number, i32k certainty)
{
    using namespace PrimesHelpers;

    if (! number[0])
        return false;

    if (number.getHighestBit() <= 10)
    {
        u32k num = number.getBitRangeAsInt (0, 10);

        for (u32 i = num / 2; --i > 1;)
            if (num % i == 0)
                return false;

        return true;
    }
    else
    {
        if (number.findGreatestCommonDivisor (2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23) != 1)
            return false;

        return passesMillerRabin (number, certainty);
    }
}

} // namespace drx
