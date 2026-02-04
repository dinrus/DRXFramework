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

namespace
{
    u16 generateNoteID (i32 midiChannel, i32 midiNoteNumber) noexcept
    {
        jassert (midiChannel > 0 && midiChannel <= 16);
        jassert (midiNoteNumber >= 0 && midiNoteNumber < 128);

        return u16 ((midiChannel << 7) + midiNoteNumber);
    }
}

//==============================================================================
MPENote::MPENote (i32 midiChannel_,
                  i32 initialNote_,
                  MPEValue noteOnVelocity_,
                  MPEValue pitchbend_,
                  MPEValue pressure_,
                  MPEValue timbre_,
                  KeyState keyState_) noexcept
    : noteID (generateNoteID (midiChannel_, initialNote_)),
      midiChannel (u8 (midiChannel_)),
      initialNote (u8 (initialNote_)),
      noteOnVelocity (noteOnVelocity_),
      pitchbend (pitchbend_),
      pressure (pressure_),
      initialTimbre (timbre_),
      timbre (timbre_),
      keyState (keyState_)
{
    jassert (keyState != MPENote::off);
    jassert (isValid());
}

MPENote::MPENote() noexcept {}

//==============================================================================
b8 MPENote::isValid() const noexcept
{
    return midiChannel > 0 && midiChannel <= 16 && initialNote < 128;
}

//==============================================================================
f64 MPENote::getFrequencyInHertz (f64 frequencyOfA) const noexcept
{
    auto pitchInSemitones = f64 (initialNote) + totalPitchbendInSemitones;
    return frequencyOfA * std::pow (2.0, (pitchInSemitones - 69.0) / 12.0);
}

//==============================================================================
b8 MPENote::operator== (const MPENote& other) const noexcept
{
    jassert (isValid() && other.isValid());
    return noteID == other.noteID;
}

b8 MPENote::operator!= (const MPENote& other) const noexcept
{
    jassert (isValid() && other.isValid());
    return noteID != other.noteID;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class MPENoteTests final : public UnitTest
{
public:
    MPENoteTests()
        : UnitTest ("MPENote class", UnitTestCategories::midi)
    {}

    //==============================================================================
    z0 runTest() override
    {
        beginTest ("getFrequencyInHertz");
        {
            MPENote note;
            note.initialNote = 60;
            note.totalPitchbendInSemitones = -0.5;
            expectEqualsWithinOneCent (note.getFrequencyInHertz(), 254.178);
        }
    }

private:
    //==============================================================================
    z0 expectEqualsWithinOneCent (f64 frequencyInHertzActual,
                                    f64 frequencyInHertzExpected)
    {
        f64 ratio = frequencyInHertzActual / frequencyInHertzExpected;
        f64 oneCent = 1.0005946;
        expect (ratio < oneCent);
        expect (ratio > 1.0 / oneCent);
    }
};

static MPENoteTests MPENoteUnitTests;

#endif

} // namespace drx
