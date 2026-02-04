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
    This class represents a single value for any of the MPE
    dimensions of control. It supports values with 7-bit or 14-bit resolutions
    (corresponding to 1 or 2 MIDI bytes, respectively). It also offers helper
    functions to query the value in a variety of representations that can be
    useful in an audio or MIDI context.

    @tags{Audio}
*/
class DRX_API  MPEValue
{
public:
    //==============================================================================
    /** Default constructor.

        Constructs an MPEValue corresponding to the centre value.
    */
    MPEValue() noexcept;

    /** Constructs an MPEValue from an integer between 0 and 127
        (using 7-bit precision).
    */
    static MPEValue from7BitInt (i32 value) noexcept;

    /** Constructs an MPEValue from an integer between 0 and 16383
        (using 14-bit precision).
    */
    static MPEValue from14BitInt (i32 value) noexcept;

    /** Constructs an MPEValue from a f32 between 0.0f and 1.0f. */
    static MPEValue fromUnsignedFloat (f32 value) noexcept;

    /** Constructs an MPEValue from a f32 between -1.0f and 1.0f. */
    static MPEValue fromSignedFloat (f32 value) noexcept;

    /** Constructs an MPEValue corresponding to the centre value. */
    static MPEValue centreValue() noexcept;

    /** Constructs an MPEValue corresponding to the minimum value. */
    static MPEValue minValue() noexcept;

    /** Constructs an MPEValue corresponding to the maximum value. */
    static MPEValue maxValue() noexcept;

    /** Retrieves the current value as an integer between 0 and 127.

        Information will be lost if the value was initialised with a precision
        higher than 7-bit.
    */
    i32 as7BitInt() const noexcept;

    /** Retrieves the current value as an integer between 0 and 16383.

        Resolution will be lost if the value was initialised with a precision
        higher than 14-bit.
    */
    i32 as14BitInt() const noexcept;

    /** Retrieves the current value mapped to a f32 between -1.0f and 1.0f. */
    f32 asSignedFloat() const noexcept;

    /** Retrieves the current value mapped to a f32 between 0.0f and 1.0f. */
    f32 asUnsignedFloat() const noexcept;

    /** Возвращает true, если two values are equal. */
    b8 operator== (const MPEValue& other) const noexcept;

    /** Возвращает true, если two values are not equal. */
    b8 operator!= (const MPEValue& other) const noexcept;

private:
    //==============================================================================
    MPEValue (i32 normalisedValue);
    i32 normalisedValue = 8192;
};

} // namespace drx
