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
    BlowFish encryption class.


    @tags{Cryptography}
*/
class DRX_API  BlowFish
{
public:
    //==============================================================================
    /** Creates an object that can encode/decode based on the specified key.

        The key data can be up to 72 bytes i64.
    */
    BlowFish (ukk keyData, i32 keyBytes);

    /** Creates a copy of another blowfish object. */
    BlowFish (const BlowFish&);

    /** Copies another blowfish object. */
    BlowFish& operator= (const BlowFish&) noexcept;

    /** Destructor. */
    ~BlowFish() noexcept;

    //==============================================================================
    /** Encrypts a pair of 32-bit integers. */
    z0 encrypt (u32& data1, u32& data2) const noexcept;

    /** Decrypts a pair of 32-bit integers. */
    z0 decrypt (u32& data1, u32& data2) const noexcept;

    //==============================================================================
    /** Encrypts a memory block */
    z0 encrypt (MemoryBlock& data) const;

    /** Decrypts a memory block */
    z0 decrypt (MemoryBlock& data) const;

    //==============================================================================
    /** Encrypts data in-place

        @param buffer       The message that should be encrypted. See bufferSize on size
                            requirements!
        @param sizeOfMsg    The size of the message that should be encrypted in bytes
        @param bufferSize   The size of the buffer in bytes. To accommodate the encrypted
                            data, the buffer must be larger than the message: the size of
                            the buffer needs to be equal or greater than the size of the
                            message in bytes rounded to the next integer which is divisible
                            by eight. If the message size in bytes is already divisible by eight
                            then you need to add eight bytes to the buffer size. If in doubt
                            simply use bufferSize = sizeOfMsg + 8.

        @returns            The size of the decrypted data in bytes or -1 if the decryption failed.
     */
    i32 encrypt (uk buffer, size_t sizeOfMsg, size_t bufferSize) const noexcept;

    /** Decrypts data in-place

        @param buffer  The encrypted data that should be decrypted
        @param bytes   The size of the encrypted data in bytes

        @returns       The size of the decrypted data in bytes or -1 if the decryption failed.
    */
    i32 decrypt (uk buffer, size_t bytes) const noexcept;

private:
    //==============================================================================
    static i32 pad   (uk, size_t, size_t) noexcept;
    static i32 unpad (ukk, size_t) noexcept;

    b8 apply (uk, size_t, z0 (BlowFish::*op) (u32&, u32&) const) const;

    //==============================================================================
    u32 p[18];
    HeapBlock<u32> s[4];

    u32 F (u32) const noexcept;

    DRX_LEAK_DETECTOR (BlowFish)
};

} // namespace drx
