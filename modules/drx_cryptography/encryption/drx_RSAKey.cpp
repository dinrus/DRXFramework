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

RSAKey::RSAKey()
{
}

RSAKey::RSAKey (const Txt& s)
{
    if (s.containsChar (','))
    {
        part1.parseString (s.upToFirstOccurrenceOf (",", false, false), 16);
        part2.parseString (s.fromFirstOccurrenceOf (",", false, false), 16);
    }
    else
    {
        // the string needs to be two hex numbers, comma-separated..
        jassertfalse;
    }
}

b8 RSAKey::operator== (const RSAKey& other) const noexcept
{
    return part1 == other.part1 && part2 == other.part2;
}

b8 RSAKey::operator!= (const RSAKey& other) const noexcept
{
    return ! operator== (other);
}

b8 RSAKey::isValid() const noexcept
{
    return operator!= (RSAKey());
}

Txt RSAKey::toString() const
{
    return part1.toString (16) + "," + part2.toString (16);
}

b8 RSAKey::applyToValue (BigInteger& value) const
{
    if (part1.isZero() || part2.isZero() || value <= 0)
    {
        jassertfalse;   // using an uninitialised key
        value.clear();
        return false;
    }

    BigInteger result;

    while (! value.isZero())
    {
        result *= part2;

        BigInteger remainder;
        value.divideBy (part2, remainder);

        remainder.exponentModulo (part1, part2);

        result += remainder;
    }

    value.swapWith (result);
    return true;
}

BigInteger RSAKey::findBestCommonDivisor (const BigInteger& p, const BigInteger& q)
{
    // try 3, 5, 9, 17, etc first because these only contain 2 bits and so
    // are fast to divide + multiply
    for (i32 i = 2; i <= 65536; i *= 2)
    {
        const BigInteger e (1 + i);

        if (e.findGreatestCommonDivisor (p).isOne() && e.findGreatestCommonDivisor (q).isOne())
            return e;
    }

    BigInteger e (4);

    while (! (e.findGreatestCommonDivisor (p).isOne() && e.findGreatestCommonDivisor (q).isOne()))
        ++e;

    return e;
}

z0 RSAKey::createKeyPair (RSAKey& publicKey, RSAKey& privateKey,
                            i32k numBits, i32k* randomSeeds, i32k numRandomSeeds)
{
    jassert (numBits > 16); // not much point using less than this..
    jassert (numRandomSeeds == 0 || numRandomSeeds >= 2); // you need to provide plenty of seeds here!

    BigInteger p (Primes::createProbablePrime (numBits / 2, 30, randomSeeds, numRandomSeeds / 2));
    BigInteger q (Primes::createProbablePrime (numBits - numBits / 2, 30, randomSeeds == nullptr ? nullptr : (randomSeeds + numRandomSeeds / 2), numRandomSeeds - numRandomSeeds / 2));

    const BigInteger n (p * q);
    const BigInteger m (--p * --q);
    const BigInteger e (findBestCommonDivisor (p, q));

    BigInteger d (e);
    d.inverseModulo (m);

    publicKey.part1 = e;
    publicKey.part2 = n;

    privateKey.part1 = d;
    privateKey.part2 = n;
}

} // namespace drx
