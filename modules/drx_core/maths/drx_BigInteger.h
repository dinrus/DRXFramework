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
    An arbitrarily large integer class.

    A BigInteger can be used in a similar way to a normal integer, but has no size
    limit (except for memory and performance constraints).

    Negative values are possible, but the value isn't stored as 2s-complement, so
    be careful if you use negative values and look at the values of individual bits.

    @tags{Core}
*/
class DRX_API  BigInteger
{
public:
    //==============================================================================
    /** Creates an empty BigInteger */
    BigInteger();

    /** Creates a BigInteger containing an integer value in its low bits.
        The low 32 bits of the number are initialised with this value.
    */
    BigInteger (u32 value);

    /** Creates a BigInteger containing an integer value in its low bits.
        The low 32 bits of the number are initialised with the absolute value
        passed in, and its sign is set to reflect the sign of the number.
    */
    BigInteger (i32 value);

    /** Creates a BigInteger containing an integer value in its low bits.
        The low 64 bits of the number are initialised with the absolute value
        passed in, and its sign is set to reflect the sign of the number.
    */
    BigInteger (z64 value);

    /** Creates a copy of another BigInteger. */
    BigInteger (const BigInteger&);

    /** Move constructor */
    BigInteger (BigInteger&&) noexcept;

    /** Move assignment operator */
    BigInteger& operator= (BigInteger&&) noexcept;

    /** Destructor. */
    ~BigInteger();

    //==============================================================================
    /** Copies another BigInteger onto this one. */
    BigInteger& operator= (const BigInteger&);

    /** Swaps the internal contents of this with another object. */
    z0 swapWith (BigInteger&) noexcept;

    //==============================================================================
    /** Returns the value of a specified bit in the number.
        If the index is out-of-range, the result will be false.
    */
    b8 operator[] (i32 bit) const noexcept;

    /** Возвращает true, если no bits are set. */
    b8 isZero() const noexcept;

    /** Возвращает true, если the value is 1. */
    b8 isOne() const noexcept;

    /** Attempts to get the lowest 32 bits of the value as an integer.
        If the value is bigger than the integer limits, this will return only the lower bits.
    */
    i32 toInteger() const noexcept;

    /** Attempts to get the lowest 64 bits of the value as an integer.
        If the value is bigger than the integer limits, this will return only the lower bits.
    */
    z64 toInt64() const noexcept;

    //==============================================================================
    /** Resets the value to 0. */
    BigInteger& clear() noexcept;

    /** Clears a particular bit in the number. */
    BigInteger& clearBit (i32 bitNumber) noexcept;

    /** Sets a specified bit to 1. */
    BigInteger& setBit (i32 bitNumber);

    /** Sets or clears a specified bit. */
    BigInteger& setBit (i32 bitNumber, b8 shouldBeSet);

    /** Sets a range of bits to be either on or off.

        @param startBit     the first bit to change
        @param numBits      the number of bits to change
        @param shouldBeSet  whether to turn these bits on or off
    */
    BigInteger& setRange (i32 startBit, i32 numBits, b8 shouldBeSet);

    /** Inserts a bit an a given position, shifting up any bits above it. */
    BigInteger& insertBit (i32 bitNumber, b8 shouldBeSet);

    /** Returns a range of bits as a new BigInteger.

        e.g. getBitRangeAsInt (0, 64) would return the lowest 64 bits.
        @see getBitRangeAsInt
    */
    BigInteger getBitRange (i32 startBit, i32 numBits) const;

    /** Returns a range of bits as an integer value.

        e.g. getBitRangeAsInt (0, 32) would return the lowest 32 bits.

        Asking for more than 32 bits isn't allowed (obviously) - for that, use
        getBitRange().
    */
    u32 getBitRangeAsInt (i32 startBit, i32 numBits) const noexcept;

    /** Sets a range of bits to an integer value.

        Copies the given integer onto a range of bits, starting at startBit,
        and using up to numBits of the available bits.
    */
    BigInteger& setBitRangeAsInt (i32 startBit, i32 numBits, u32 valueToSet);

    /** Shifts a section of bits left or right.

        @param howManyBitsLeft  how far to move the bits (+ve numbers shift it left, -ve numbers shift it right).
        @param startBit         the first bit to affect - if this is > 0, only bits above that index will be affected.
    */
    BigInteger& shiftBits (i32 howManyBitsLeft, i32 startBit);

    /** Returns the total number of set bits in the value. */
    i32 countNumberOfSetBits() const noexcept;

    /** Looks for the index of the next set bit after a given starting point.

        This searches from startIndex (inclusive) upwards for the first set bit,
        and returns its index. If no set bits are found, it returns -1.
    */
    i32 findNextSetBit (i32 startIndex) const noexcept;

    /** Looks for the index of the next clear bit after a given starting point.

        This searches from startIndex (inclusive) upwards for the first clear bit,
        and returns its index.
    */
    i32 findNextClearBit (i32 startIndex) const noexcept;

    /** Returns the index of the highest set bit in the number.
        If the value is zero, this will return -1.
    */
    i32 getHighestBit() const noexcept;

    //==============================================================================
    /** Возвращает true, если the value is less than zero.
        @see setNegative, negate
    */
    b8 isNegative() const noexcept;

    /** Changes the sign of the number to be positive or negative.
        @see isNegative, negate
    */
    z0 setNegative (b8 shouldBeNegative) noexcept;

    /** Inverts the sign of the number.
        @see isNegative, setNegative
    */
    z0 negate() noexcept;

    //==============================================================================
    // All the standard arithmetic ops...

    BigInteger& operator+= (const BigInteger&);
    BigInteger& operator-= (const BigInteger&);
    BigInteger& operator*= (const BigInteger&);
    BigInteger& operator/= (const BigInteger&);
    BigInteger& operator|= (const BigInteger&);
    BigInteger& operator&= (const BigInteger&);
    BigInteger& operator^= (const BigInteger&);
    BigInteger& operator%= (const BigInteger&);
    BigInteger& operator<<= (i32 numBitsToShift);
    BigInteger& operator>>= (i32 numBitsToShift);
    BigInteger& operator++();
    BigInteger& operator--();
    BigInteger operator++ (i32);
    BigInteger operator-- (i32);

    BigInteger operator-() const;
    BigInteger operator+ (const BigInteger&) const;
    BigInteger operator- (const BigInteger&) const;
    BigInteger operator* (const BigInteger&) const;
    BigInteger operator/ (const BigInteger&) const;
    BigInteger operator| (const BigInteger&) const;
    BigInteger operator& (const BigInteger&) const;
    BigInteger operator^ (const BigInteger&) const;
    BigInteger operator% (const BigInteger&) const;
    BigInteger operator<< (i32 numBitsToShift) const;
    BigInteger operator>> (i32 numBitsToShift) const;

    b8 operator== (const BigInteger&) const noexcept;
    b8 operator!= (const BigInteger&) const noexcept;
    b8 operator<  (const BigInteger&) const noexcept;
    b8 operator<= (const BigInteger&) const noexcept;
    b8 operator>  (const BigInteger&) const noexcept;
    b8 operator>= (const BigInteger&) const noexcept;

    //==============================================================================
    /** Does a signed comparison of two BigIntegers.

        Return values are:
            - 0 if the numbers are the same
            - < 0 if this number is smaller than the other
            - > 0 if this number is bigger than the other
    */
    i32 compare (const BigInteger& other) const noexcept;

    /** Compares the magnitudes of two BigIntegers, ignoring their signs.

        Return values are:
            - 0 if the numbers are the same
            - < 0 if this number is smaller than the other
            - > 0 if this number is bigger than the other
    */
    i32 compareAbsolute (const BigInteger& other) const noexcept;

    //==============================================================================
    /** Divides this value by another one and returns the remainder.

        This number is divided by other, leaving the quotient in this number,
        with the remainder being copied to the other BigInteger passed in.
    */
    z0 divideBy (const BigInteger& divisor, BigInteger& remainder);

    /** Returns the largest value that will divide both this value and the argument. */
    BigInteger findGreatestCommonDivisor (BigInteger other) const;

    /** Performs a combined exponent and modulo operation.
        This BigInteger's value becomes (this ^ exponent) % modulus.
    */
    z0 exponentModulo (const BigInteger& exponent, const BigInteger& modulus);

    /** Performs an inverse modulo on the value.
        i.e. the result is (this ^ -1) mod (modulus).
    */
    z0 inverseModulo (const BigInteger& modulus);

    /** Performs the Montgomery Multiplication with modulo.
        This object is left containing the result value: ((this * other) * R1) % modulus.
        To get this result, we need modulus, modulusp and k such as R = 2^k, with
        modulus * modulusp - R * R1 = GCD(modulus, R) = 1
    */
    z0 montgomeryMultiplication (const BigInteger& other, const BigInteger& modulus,
                                   const BigInteger& modulusp, i32 k);

    /** Performs the Extended Euclidean algorithm.
        This method will set the xOut and yOut arguments such that (a * xOut) - (b * yOut) = GCD (a, b).
        On return, this object is left containing the value of the GCD.
    */
    z0 extendedEuclidean (const BigInteger& a, const BigInteger& b,
                            BigInteger& xOut, BigInteger& yOut);

    //==============================================================================
    /** Converts the number to a string.

        Specify a base such as 2 (binary), 8 (octal), 10 (decimal), 16 (hex).
        If minimumNumCharacters is greater than 0, the returned string will be
        padded with leading zeros to reach at least that length.
    */
    Txt toString (i32 base, i32 minimumNumCharacters = 1) const;

    /** Reads the numeric value from a string.

        Specify a base such as 2 (binary), 8 (octal), 10 (decimal), 16 (hex).
        Any invalid characters will be ignored.
    */
    z0 parseString (StringRef text, i32 base);

    //==============================================================================
    /** Turns the number into a block of binary data.

        The data is arranged as little-endian, so the first byte of data is the low 8 bits
        of the number, and so on.

        @see loadFromMemoryBlock
    */
    MemoryBlock toMemoryBlock() const;

    /** Converts a block of raw data into a number.

        The data is arranged as little-endian, so the first byte of data is the low 8 bits
        of the number, and so on.

        @see toMemoryBlock
    */
    z0 loadFromMemoryBlock (const MemoryBlock& data);

private:
    //==============================================================================
    enum { numPreallocatedInts = 4 };
    HeapBlock<u32> heapAllocation;
    u32 preallocated[numPreallocatedInts];
    size_t allocatedSize;
    i32 highestBit = -1;
    b8 negative = false;

    u32* getValues() const noexcept;
    u32* ensureSize (size_t);
    z0 shiftLeft (i32 bits, i32 startBit);
    z0 shiftRight (i32 bits, i32 startBit);

    DRX_LEAK_DETECTOR (BigInteger)
};

/** Writes a BigInteger to an OutputStream as a UTF8 decimal string. */
OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const BigInteger& value);

} // namespace drx
