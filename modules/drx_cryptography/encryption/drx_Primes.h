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
/**
    Prime number creation class.

    This class contains static methods for generating and testing prime numbers.

    @see BigInteger

    @tags{Cryptography}
*/
class DRX_API  Primes
{
public:
    //==============================================================================
    /** Creates a random prime number with a given bit-length.

        The certainty parameter specifies how many iterations to use when testing
        for primality. A safe value might be anything over about 20-30.

        The randomSeeds parameter lets you optionally pass it a set of values with
        which to seed the random number generation, improving the security of the
        keys generated.
    */
    static BigInteger createProbablePrime (i32 bitLength,
                                           i32 certainty,
                                        i32k* randomSeeds = nullptr,
                                           i32 numRandomSeeds = 0);

    /** Tests a number to see if it's prime.

        This isn't a bulletproof test, it uses a Miller-Rabin test to determine
        whether the number is prime.

        The certainty parameter specifies how many iterations to use when testing - a
        safe value might be anything over about 20-30.
    */
    static b8 isProbablyPrime (const BigInteger& number, i32 certainty);


private:
    Primes();

    DRX_DECLARE_NON_COPYABLE (Primes)
};

} // namespace drx
