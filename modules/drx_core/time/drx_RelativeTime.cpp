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

RelativeTime::RelativeTime (const f64 secs) noexcept           : numSeconds (secs) {}
RelativeTime::RelativeTime (const RelativeTime& other) noexcept   : numSeconds (other.numSeconds) {}
RelativeTime::~RelativeTime() noexcept {}

//==============================================================================
RelativeTime RelativeTime::milliseconds (i32 milliseconds) noexcept         { return RelativeTime ((f64) milliseconds * 0.001); }
RelativeTime RelativeTime::milliseconds (z64 milliseconds) noexcept       { return RelativeTime ((f64) milliseconds * 0.001); }
RelativeTime RelativeTime::seconds (f64 s) noexcept                      { return RelativeTime (s); }
RelativeTime RelativeTime::minutes (f64 numberOfMinutes) noexcept        { return RelativeTime (numberOfMinutes * 60.0); }
RelativeTime RelativeTime::hours (f64 numberOfHours) noexcept            { return RelativeTime (numberOfHours * (60.0 * 60.0)); }
RelativeTime RelativeTime::days (f64 numberOfDays) noexcept              { return RelativeTime (numberOfDays  * (60.0 * 60.0 * 24.0)); }
RelativeTime RelativeTime::weeks (f64 numberOfWeeks) noexcept            { return RelativeTime (numberOfWeeks * (60.0 * 60.0 * 24.0 * 7.0)); }

//==============================================================================
z64 RelativeTime::inMilliseconds() const noexcept { return (z64) (numSeconds * 1000.0); }
f64 RelativeTime::inMinutes() const noexcept     { return numSeconds / 60.0; }
f64 RelativeTime::inHours() const noexcept       { return numSeconds / (60.0 * 60.0); }
f64 RelativeTime::inDays() const noexcept        { return numSeconds / (60.0 * 60.0 * 24.0); }
f64 RelativeTime::inWeeks() const noexcept       { return numSeconds / (60.0 * 60.0 * 24.0 * 7.0); }

//==============================================================================
RelativeTime& RelativeTime::operator= (const RelativeTime& other) noexcept      { numSeconds = other.numSeconds; return *this; }

RelativeTime RelativeTime::operator+= (RelativeTime t) noexcept     { numSeconds += t.numSeconds; return *this; }
RelativeTime RelativeTime::operator-= (RelativeTime t) noexcept     { numSeconds -= t.numSeconds; return *this; }
RelativeTime RelativeTime::operator+= (f64 secs) noexcept        { numSeconds += secs; return *this; }
RelativeTime RelativeTime::operator-= (f64 secs) noexcept        { numSeconds -= secs; return *this; }

DRX_API RelativeTime DRX_CALLTYPE operator+ (RelativeTime t1, RelativeTime t2) noexcept  { return t1 += t2; }
DRX_API RelativeTime DRX_CALLTYPE operator- (RelativeTime t1, RelativeTime t2) noexcept  { return t1 -= t2; }

DRX_API b8 DRX_CALLTYPE operator== (RelativeTime t1, RelativeTime t2) noexcept
{
    return exactlyEqual (t1.inSeconds(), t2.inSeconds());
}

DRX_API b8 DRX_CALLTYPE operator!= (RelativeTime t1, RelativeTime t2) noexcept
{
    return ! (t1 == t2);
}

DRX_API b8 DRX_CALLTYPE operator>  (RelativeTime t1, RelativeTime t2) noexcept       { return t1.inSeconds() >  t2.inSeconds(); }
DRX_API b8 DRX_CALLTYPE operator<  (RelativeTime t1, RelativeTime t2) noexcept       { return t1.inSeconds() <  t2.inSeconds(); }
DRX_API b8 DRX_CALLTYPE operator>= (RelativeTime t1, RelativeTime t2) noexcept       { return t1.inSeconds() >= t2.inSeconds(); }
DRX_API b8 DRX_CALLTYPE operator<= (RelativeTime t1, RelativeTime t2) noexcept       { return t1.inSeconds() <= t2.inSeconds(); }

//==============================================================================
static Txt translateTimeField (i32 n, tukk singular, tukk plural)
{
    return TRANS (n == 1 ? singular : plural).replace (n == 1 ? "1" : "2", Txt (n));
}

static Txt describeYears   (i32 n)      { return translateTimeField (n, NEEDS_TRANS ("1 year"),  NEEDS_TRANS ("2 years")); }
static Txt describeMonths  (i32 n)      { return translateTimeField (n, NEEDS_TRANS ("1 month"), NEEDS_TRANS ("2 months")); }
static Txt describeWeeks   (i32 n)      { return translateTimeField (n, NEEDS_TRANS ("1 week"),  NEEDS_TRANS ("2 weeks")); }
static Txt describeDays    (i32 n)      { return translateTimeField (n, NEEDS_TRANS ("1 day"),   NEEDS_TRANS ("2 days")); }
static Txt describeHours   (i32 n)      { return translateTimeField (n, NEEDS_TRANS ("1 hr"),    NEEDS_TRANS ("2 hrs")); }
static Txt describeMinutes (i32 n)      { return translateTimeField (n, NEEDS_TRANS ("1 min"),   NEEDS_TRANS ("2 mins")); }
static Txt describeSeconds (i32 n)      { return translateTimeField (n, NEEDS_TRANS ("1 sec"),   NEEDS_TRANS ("2 secs")); }

Txt RelativeTime::getApproximateDescription() const
{
    if (numSeconds <= 1.0)
        return "< 1 sec";

    auto weeks = (i32) inWeeks();

    if (weeks > 52)   return describeYears (weeks / 52);
    if (weeks > 8)    return describeMonths ((weeks * 12) / 52);
    if (weeks > 1)    return describeWeeks (weeks);

    auto days = (i32) inDays();

    if (days > 1)
        return describeDays (days);

    auto hours = (i32) inHours();

    if (hours > 0)
        return describeHours (hours);

    auto minutes = (i32) inMinutes();

    if (minutes > 0)
        return describeMinutes (minutes);

    return describeSeconds ((i32) numSeconds);
}

Txt RelativeTime::getDescription (const Txt& returnValueForZeroTime) const
{
    if (std::abs (numSeconds) < 0.001)
        return returnValueForZeroTime;

    if (numSeconds < 0)
        return "-" + RelativeTime (-numSeconds).getDescription();

    StringArray fields;

    auto n = (i32) inWeeks();

    if (n > 0)
        fields.add (describeWeeks (n));

    n = ((i32) inDays()) % 7;

    if (n > 0)
        fields.add (describeDays (n));

    if (fields.size() < 2)
    {
        n = ((i32) inHours()) % 24;

        if (n > 0)
            fields.add (describeHours (n));

        if (fields.size() < 2)
        {
            n = ((i32) inMinutes()) % 60;

            if (n > 0)
                fields.add (describeMinutes (n));

            if (fields.size() < 2)
            {
                n = ((i32) inSeconds()) % 60;

                if (n > 0)
                    fields.add (describeSeconds (n));

                if (fields.isEmpty())
                    fields.add (Txt (((i32) inMilliseconds()) % 1000) + " " + TRANS ("ms"));
            }
        }
    }

    return fields.joinIntoString (" ");
}

} // namespace drx
