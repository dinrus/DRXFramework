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

const OSCTimeTag OSCTimeTag::immediately;

static const zu64 millisecondsBetweenOscAndDrxEpochs = 2208988800000ULL;
static const zu64 rawTimeTagRepresentingImmediately = 0x0000000000000001ULL;

//==============================================================================
OSCTimeTag::OSCTimeTag() noexcept  : rawTimeTag (rawTimeTagRepresentingImmediately)
{
}

OSCTimeTag::OSCTimeTag (zu64 t) noexcept  : rawTimeTag (t)
{
}

OSCTimeTag::OSCTimeTag (Time time) noexcept
{
    const zu64 milliseconds = (zu64) time.toMilliseconds() + millisecondsBetweenOscAndDrxEpochs;

    zu64 seconds = milliseconds / 1000;
    u32 fractionalPart = u32 (4294967.296 * (milliseconds % 1000));

    rawTimeTag = (seconds << 32) + fractionalPart;
}

//==============================================================================
Time OSCTimeTag::toTime() const noexcept
{
    const zu64 seconds = rawTimeTag >> 32;
    u32k fractionalPart = (rawTimeTag & 0x00000000FFFFFFFFULL);

    const auto fractionalPartInMillis = (f64) fractionalPart / 4294967.296;

    // now using signed integer, because this is allowed to become negative:
    const auto juceTimeInMillis = (z64) (seconds * 1000)
                                + (z64) roundToInt (fractionalPartInMillis)
                                - (z64) millisecondsBetweenOscAndDrxEpochs;

    return Time (juceTimeInMillis);
}

//==============================================================================
b8 OSCTimeTag::isImmediately() const noexcept
{
    return rawTimeTag == rawTimeTagRepresentingImmediately;
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class OSCTimeTagTests final : public UnitTest
{
public:
    OSCTimeTagTests()
        : UnitTest ("OSCTimeTag class", UnitTestCategories::osc)
    {}

    z0 runTest() override
    {
        beginTest ("Basics");

        {
            OSCTimeTag tag;
            expect (tag.isImmediately());
        }
        {
            OSCTimeTag tag (3535653);
            expect (! tag.isImmediately());

            OSCTimeTag otherTag;
            otherTag = tag;
            expect (! otherTag.isImmediately());

            OSCTimeTag copyTag (tag);
            expect (! copyTag.isImmediately());
        }

        beginTest ("Conversion to/from DRX Time");

        {
            Time time;
            OSCTimeTag tag (time);
            expect (! tag.isImmediately());
        }
        {
            OSCTimeTag tag;
            Time time = tag.toTime();
            expect (time < Time::getCurrentTime());
        }
        {
            Time currentTime (Time::currentTimeMillis());

            // Make sure we use a f64 that has a representation that will always
            // truncate to what we expect when multipled by 1000 and turned into
            // integer milliseconds.
            f64 deltaInSeconds = 1.562;

            RelativeTime delta (deltaInSeconds);
            Time laterTime = currentTime + delta;

            OSCTimeTag currentTimeTag (currentTime);
            OSCTimeTag laterTimeTag (laterTime);

            zu64 currentTimeTagRaw = currentTimeTag.getRawTimeTag();
            zu64 laterTimeTagRaw = laterTimeTag.getRawTimeTag();

            // in the raw time tag, the most significant 32 bits are seconds,
            // so let's verify that the difference is right:
            zu64 diff = laterTimeTagRaw - currentTimeTagRaw;
            f64 acceptableErrorInSeconds = 0.000001; // definitely not audible anymore.

            expect ((f32) diff / f32 (1ULL << 32) < deltaInSeconds + acceptableErrorInSeconds );
            expect ((f32) diff / f32 (1ULL << 32) > deltaInSeconds - acceptableErrorInSeconds );

            // round trip:

            Time currentTime2 = currentTimeTag.toTime();
            Time laterTime2 = laterTimeTag.toTime();
            RelativeTime delta2 = laterTime2 - currentTime2;

            expect (currentTime2 == currentTime);
            expect (laterTime2 == laterTime);
            expect (delta2 == delta);
        }
    }
};

static OSCTimeTagTests OSCTimeTagUnitTests;

#endif

} // namespace drx
