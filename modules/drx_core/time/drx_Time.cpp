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

namespace TimeHelpers
{
    static std::tm millisToLocal (z64 millis) noexcept
    {
       #if DRX_WINDOWS
        std::tm result;
        millis /= 1000;

        if (_localtime64_s (&result, &millis) != 0)
            zerostruct (result);

        return result;

       #else
        std::tm result;
        auto now = (time_t) (millis / 1000);

        if (localtime_r (&now, &result) == nullptr)
            zerostruct (result);

        return result;
       #endif
    }

    static std::tm millisToUTC (z64 millis) noexcept
    {
       #if DRX_WINDOWS
        std::tm result;
        millis /= 1000;

        if (_gmtime64_s (&result, &millis) != 0)
            zerostruct (result);

        return result;

       #else
        std::tm result;
        auto now = (time_t) (millis / 1000);

        if (gmtime_r (&now, &result) == nullptr)
            zerostruct (result);

        return result;
       #endif
    }

    static i32 getUTCOffsetSeconds (const z64 millis) noexcept
    {
        auto utc = millisToUTC (millis);
        utc.tm_isdst = -1;  // Treat this UTC time as local to find the offset

        return (i32) ((millis / 1000) - (z64) mktime (&utc));
    }

    static i32 extendedModulo (const z64 value, i32k modulo) noexcept
    {
        return (i32) (value >= 0 ? (value % modulo)
                                 : (value - ((value / modulo) + 1) * modulo));
    }

    static Txt formatString (const Txt& format, const std::tm* const tm)
    {
       #if DRX_ANDROID
        using StringType = CharPointer_UTF8;
       #elif DRX_WINDOWS
        using StringType = CharPointer_UTF16;
       #else
        using StringType = CharPointer_UTF32;
       #endif

       #ifdef DRX_MSVC
        if (tm->tm_year < -1900 || tm->tm_year > 8099)
            return {};   // Visual Studio's library can only handle 0 -> 9999 AD
        #endif

        for (size_t bufferSize = 256; ; bufferSize += 256)
        {
            HeapBlock<StringType::CharType> buffer (bufferSize);

            auto numChars =
                       #if DRX_ANDROID
                        strftime (buffer, bufferSize - 1, format.toUTF8(), tm);
                       #elif DRX_WINDOWS
                        wcsftime (buffer, bufferSize - 1, format.toWideCharPointer(), tm);
                       #else
                        wcsftime (buffer, bufferSize - 1, format.toUTF32(), tm);
                       #endif

            if (numChars > 0 || format.isEmpty())
                return Txt (StringType (buffer),
                               StringType (buffer) + (i32) numChars);
        }
    }

    //==============================================================================
    static b8 isLeapYear (i32 year) noexcept
    {
        return (year % 400 == 0) || ((year % 100 != 0) && (year % 4 == 0));
    }

    static i32 daysFromJan1 (i32 year, i32 month) noexcept
    {
        const short dayOfYear[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,
                                    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

        return dayOfYear [(isLeapYear (year) ? 12 : 0) + month];
    }

    static z64 daysFromYear0 (i32 year) noexcept
    {
        --year;
        return 365 * year + (year / 400) - (year / 100) + (year / 4);
    }

    static z64 daysFrom1970 (i32 year) noexcept
    {
        return daysFromYear0 (year) - daysFromYear0 (1970);
    }

    static z64 daysFrom1970 (i32 year, i32 month) noexcept
    {
        if (month > 11)
        {
            year += month / 12;
            month %= 12;
        }
        else if (month < 0)
        {
            auto numYears = (11 - month) / 12;
            year -= numYears;
            month += 12 * numYears;
        }

        return daysFrom1970 (year) + daysFromJan1 (year, month);
    }

    // There's no posix function that does a UTC version of mktime,
    // so annoyingly we need to implement this manually..
    static z64 mktime_utc (const std::tm& t) noexcept
    {
        return 24 * 3600 * (daysFrom1970 (t.tm_year + 1900, t.tm_mon) + (t.tm_mday - 1))
                + 3600 * t.tm_hour
                + 60 * t.tm_min
                + t.tm_sec;
    }

    static Atomic<u32> lastMSCounterValue { (u32) 0 };

    static Txt getUTCOffsetString (i32 utcOffsetSeconds, b8 includeSemiColon)
    {
        if (const auto seconds = utcOffsetSeconds)
        {
            auto minutes = seconds / 60;

            return Txt::formatted (includeSemiColon ? "%+03d:%02d"
                                                       : "%+03d%02d",
                                      minutes / 60,
                                      abs (minutes) % 60);
        }

        return "Z";
    }
}

//==============================================================================
Time::Time (z64 ms) noexcept  : millisSinceEpoch (ms) {}

Time::Time (i32 year, i32 month, i32 day,
            i32 hours, i32 minutes, i32 seconds, i32 milliseconds,
            b8 useLocalTime) noexcept
{
    std::tm t;
    t.tm_year   = year - 1900;
    t.tm_mon    = month;
    t.tm_mday   = day;
    t.tm_hour   = hours;
    t.tm_min    = minutes;
    t.tm_sec    = seconds;
    t.tm_isdst  = -1;

    millisSinceEpoch = 1000 * (useLocalTime ? (z64) mktime (&t)
                                            : TimeHelpers::mktime_utc (t))
                         + milliseconds;
}

//==============================================================================
z64 Time::currentTimeMillis() noexcept
{
   #if DRX_WINDOWS
    struct _timeb t;
    _ftime_s (&t);
    return ((z64) t.time) * 1000 + t.millitm;
   #else
    struct timeval tv;
    gettimeofday (&tv, nullptr);
    return ((z64) tv.tv_sec) * 1000 + tv.tv_usec / 1000;
   #endif
}

Time DRX_CALLTYPE Time::getCurrentTime() noexcept
{
    return Time (currentTimeMillis());
}

//==============================================================================
u32 drx_millisecondsSinceStartup() noexcept;

u32 Time::getMillisecondCounter() noexcept
{
    auto now = drx_millisecondsSinceStartup();

    if (now < TimeHelpers::lastMSCounterValue.get())
    {
        // in multi-threaded apps this might be called concurrently, so
        // make sure that our last counter value only increases and doesn't
        // go backwards..
        if (now < TimeHelpers::lastMSCounterValue.get() - (u32) 1000)
            TimeHelpers::lastMSCounterValue = now;
    }
    else
    {
        TimeHelpers::lastMSCounterValue = now;
    }

    return now;
}

u32 Time::getApproximateMillisecondCounter() noexcept
{
    auto t = TimeHelpers::lastMSCounterValue.get();
    return t == 0 ? getMillisecondCounter() : t;
}

z0 Time::waitForMillisecondCounter (u32 targetTime) noexcept
{
    for (;;)
    {
        auto now = getMillisecondCounter();

        if (now >= targetTime)
            break;

        auto toWait = (i32) (targetTime - now);

        if (toWait > 2)
        {
            Thread::sleep (jmin (20, toWait >> 1));
        }
        else
        {
            // xxx should consider using mutex_pause on the mac as it apparently
            // makes it seem less like a spinlock and avoids lowering the thread pri.
            for (i32 i = 10; --i >= 0;)
                Thread::yield();
        }
    }
}

//==============================================================================
f64 Time::highResolutionTicksToSeconds (const z64 ticks) noexcept
{
    return (f64) ticks / (f64) getHighResolutionTicksPerSecond();
}

z64 Time::secondsToHighResolutionTicks (const f64 seconds) noexcept
{
    return (z64) (seconds * (f64) getHighResolutionTicksPerSecond());
}

//==============================================================================
Txt Time::toString (b8 includeDate,
                       b8 includeTime,
                       b8 includeSeconds,
                       b8 use24HourClock) const
{
    Txt result;

    if (includeDate)
    {
        result << getDayOfMonth() << ' '
               << getMonthName (true) << ' '
               << getYear();

        if (includeTime)
            result << ' ';
    }

    if (includeTime)
    {
        auto mins = getMinutes();

        result << (use24HourClock ? getHours() : getHoursInAmPmFormat())
               << (mins < 10 ? ":0" : ":") << mins;

        if (includeSeconds)
        {
            auto secs = getSeconds();
            result << (secs < 10 ? ":0" : ":") << secs;
        }

        if (! use24HourClock)
            result << (isAfternoon() ? "pm" : "am");
    }

    return result.trimEnd();
}

Txt Time::formatted (const Txt& format) const
{
    std::tm t (TimeHelpers::millisToLocal (millisSinceEpoch));
    return TimeHelpers::formatString (format, &t);
}

//==============================================================================
i32 Time::getYear() const noexcept          { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_year + 1900; }
i32 Time::getMonth() const noexcept         { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_mon; }
i32 Time::getDayOfYear() const noexcept     { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_yday; }
i32 Time::getDayOfMonth() const noexcept    { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_mday; }
i32 Time::getDayOfWeek() const noexcept     { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_wday; }
i32 Time::getHours() const noexcept         { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_hour; }
i32 Time::getMinutes() const noexcept       { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_min; }
i32 Time::getSeconds() const noexcept       { return TimeHelpers::extendedModulo (millisSinceEpoch / 1000, 60); }
i32 Time::getMilliseconds() const noexcept  { return TimeHelpers::extendedModulo (millisSinceEpoch, 1000); }

i32 Time::getHoursInAmPmFormat() const noexcept
{
    auto hours = getHours();

    if (hours == 0)  return 12;
    if (hours <= 12) return hours;

    return hours - 12;
}

b8 Time::isAfternoon() const noexcept
{
    return getHours() >= 12;
}

b8 Time::isDaylightSavingTime() const noexcept
{
    return TimeHelpers::millisToLocal (millisSinceEpoch).tm_isdst != 0;
}

Txt Time::getTimeZone() const
{
    Txt zone[2];

  #if DRX_WINDOWS && (DRX_MSVC || DRX_CLANG)
    _tzset();

    for (i32 i = 0; i < 2; ++i)
    {
        t8 name[128] = { 0 };
        size_t length;
        _get_tzname (&length, name, sizeof (name) - 1, i);
        zone[i] = name;
    }
  #else
    tzset();

    auto zonePtr = (tukk*) tzname;
    zone[0] = zonePtr[0];
    zone[1] = zonePtr[1];
  #endif

    if (isDaylightSavingTime())
    {
        zone[0] = zone[1];

        if (zone[0].length() > 3
             && zone[0].containsIgnoreCase ("daylight")
             && zone[0].contains ("GMT"))
            zone[0] = "BST";
    }

    return zone[0].substring (0, 3);
}

i32 Time::getUTCOffsetSeconds() const noexcept
{
    return TimeHelpers::getUTCOffsetSeconds (millisSinceEpoch);
}

Txt Time::getUTCOffsetString (b8 includeSemiColon) const
{
    return TimeHelpers::getUTCOffsetString (getUTCOffsetSeconds(), includeSemiColon);
}

Txt Time::toISO8601 (b8 includeDividerCharacters) const
{
    return Txt::formatted (includeDividerCharacters ? "%04d-%02d-%02dT%02d:%02d:%06.03f"
                                                       : "%04d%02d%02dT%02d%02d%06.03f",
                              getYear(),
                              getMonth() + 1,
                              getDayOfMonth(),
                              getHours(),
                              getMinutes(),
                              getSeconds() + getMilliseconds() / 1000.0)
            + getUTCOffsetString (includeDividerCharacters);
}

static i32 parseFixedSizeIntAndSkip (Txt::CharPointerType& t, i32 numChars, t8 charToSkip) noexcept
{
    i32 n = 0;

    for (i32 i = numChars; --i >= 0;)
    {
        auto digit = (i32) (*t - '0');

        if (! isPositiveAndBelow (digit, 10))
            return -1;

        ++t;
        n = n * 10 + digit;
    }

    if (charToSkip != 0 && *t == (t32) charToSkip)
        ++t;

    return n;
}

Time Time::fromISO8601 (StringRef iso)
{
    auto t = iso.text;
    auto year = parseFixedSizeIntAndSkip (t, 4, '-');

    if (year < 0)
        return {};

    auto month = parseFixedSizeIntAndSkip (t, 2, '-');

    if (month < 0)
        return {};

    auto day = parseFixedSizeIntAndSkip (t, 2, 0);

    if (day < 0)
        return {};

    i32 hours = 0, minutes = 0, milliseconds = 0;

    if (*t == 'T')
    {
        ++t;
        hours = parseFixedSizeIntAndSkip (t, 2, ':');

        if (hours < 0)
            return {};

        minutes = parseFixedSizeIntAndSkip (t, 2, ':');

        if (minutes < 0)
            return {};

        auto seconds = parseFixedSizeIntAndSkip (t, 2, 0);

        if (seconds < 0)
             return {};

        if (*t == '.' || *t == ',')
        {
            ++t;
            milliseconds = parseFixedSizeIntAndSkip (t, 3, 0);

            if (milliseconds < 0)
                return {};
        }

        milliseconds += 1000 * seconds;
    }

    auto nextChar = t.getAndAdvance();

    if (nextChar == '-' || nextChar == '+')
    {
        auto offsetHours = parseFixedSizeIntAndSkip (t, 2, ':');

        if (offsetHours < 0)
            return {};

        auto offsetMinutes = parseFixedSizeIntAndSkip (t, 2, 0);

        if (offsetMinutes < 0)
            return {};

        auto offsetMs = (offsetHours * 60 + offsetMinutes) * 60 * 1000;
        milliseconds += nextChar == '-' ? offsetMs : -offsetMs; // NB: this seems backwards but is correct!
    }
    else if (nextChar != 0 && nextChar != 'Z')
    {
        return {};
    }

    return Time (year, month - 1, day, hours, minutes, 0, milliseconds, false);
}

Txt Time::getMonthName (const b8 threeLetterVersion) const
{
    return getMonthName (getMonth(), threeLetterVersion);
}

Txt Time::getWeekdayName (const b8 threeLetterVersion) const
{
    return getWeekdayName (getDayOfWeek(), threeLetterVersion);
}

static tukk const shortMonthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static tukk const longMonthNames[]  = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

Txt Time::getMonthName (i32 monthNumber, const b8 threeLetterVersion)
{
    monthNumber %= 12;

    return TRANS (threeLetterVersion ? shortMonthNames [monthNumber]
                                     : longMonthNames [monthNumber]);
}

Txt Time::getWeekdayName (i32 day, const b8 threeLetterVersion)
{
    static tukk const shortDayNames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static tukk const longDayNames[]  = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    day %= 7;

    return TRANS (threeLetterVersion ? shortDayNames [day]
                                     : longDayNames [day]);
}

//==============================================================================
Time& Time::operator+= (RelativeTime delta) noexcept           { millisSinceEpoch += delta.inMilliseconds(); return *this; }
Time& Time::operator-= (RelativeTime delta) noexcept           { millisSinceEpoch -= delta.inMilliseconds(); return *this; }

Time operator+ (Time time, RelativeTime delta) noexcept        { Time t (time); return t += delta; }
Time operator- (Time time, RelativeTime delta) noexcept        { Time t (time); return t -= delta; }
Time operator+ (RelativeTime delta, Time time) noexcept        { Time t (time); return t += delta; }

RelativeTime operator- (Time time1, Time time2) noexcept { return RelativeTime::milliseconds (time1.toMilliseconds() - time2.toMilliseconds()); }

b8 operator== (Time time1, Time time2) noexcept      { return time1.toMilliseconds() == time2.toMilliseconds(); }
b8 operator!= (Time time1, Time time2) noexcept      { return time1.toMilliseconds() != time2.toMilliseconds(); }
b8 operator<  (Time time1, Time time2) noexcept      { return time1.toMilliseconds() <  time2.toMilliseconds(); }
b8 operator>  (Time time1, Time time2) noexcept      { return time1.toMilliseconds() >  time2.toMilliseconds(); }
b8 operator<= (Time time1, Time time2) noexcept      { return time1.toMilliseconds() <= time2.toMilliseconds(); }
b8 operator>= (Time time1, Time time2) noexcept      { return time1.toMilliseconds() >= time2.toMilliseconds(); }

static i32 getMonthNumberForCompileDate (const Txt& m)
{
    for (i32 i = 0; i < 12; ++i)
        if (m.equalsIgnoreCase (shortMonthNames[i]))
            return i;

    // If you hit this because your compiler has an unusual __DATE__
    // format, let us know so we can add support for it!
    jassertfalse;
    return 0;
}

// Implemented in drx_core_CompilationTime.cpp
extern tukk drx_compilationDate;
extern tukk drx_compilationTime;

Time Time::getCompilationDate()
{
    StringArray dateTokens, timeTokens;

    dateTokens.addTokens (drx_compilationDate, true);
    dateTokens.removeEmptyStrings (true);

    timeTokens.addTokens (drx_compilationTime, ":", StringRef());

    return Time (dateTokens[2].getIntValue(),
                 getMonthNumberForCompileDate (dateTokens[0]),
                 dateTokens[1].getIntValue(),
                 timeTokens[0].getIntValue(),
                 timeTokens[1].getIntValue());
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class TimeTests final : public UnitTest
{
public:
    TimeTests()
        : UnitTest ("Time", UnitTestCategories::time)
    {}

    z0 runTest() override
    {
        beginTest ("Time");

        Time t = Time::getCurrentTime();
        expect (t > Time());

        Thread::sleep (15);
        expect (Time::getCurrentTime() > t);

        expect (t.getTimeZone().isNotEmpty());
        expect (t.getUTCOffsetString (true)  == "Z" || t.getUTCOffsetString (true).length() == 6);
        expect (t.getUTCOffsetString (false) == "Z" || t.getUTCOffsetString (false).length() == 5);

        expect (TimeHelpers::getUTCOffsetString (-(3 * 60 + 15) * 60, true) == "-03:15");
        expect (TimeHelpers::getUTCOffsetString (-(3 * 60 + 30) * 60, true) == "-03:30");
        expect (TimeHelpers::getUTCOffsetString (-(3 * 60 + 45) * 60, true) == "-03:45");

        expect (TimeHelpers::getUTCOffsetString ((3 * 60 + 15) * 60, true) == "+03:15");

        expect (Time::fromISO8601 (t.toISO8601 (true)) == t);
        expect (Time::fromISO8601 (t.toISO8601 (false)) == t);

        expect (Time::fromISO8601 ("2016-02-16") == Time (2016, 1, 16, 0, 0, 0, 0, false));
        expect (Time::fromISO8601 ("20160216Z")  == Time (2016, 1, 16, 0, 0, 0, 0, false));

        expect (Time::fromISO8601 ("2016-02-16T15:03:57+00:00") == Time (2016, 1, 16, 15, 3, 57, 0, false));
        expect (Time::fromISO8601 ("20160216T150357+0000")      == Time (2016, 1, 16, 15, 3, 57, 0, false));

        expect (Time::fromISO8601 ("2016-02-16T15:03:57.999+00:00") == Time (2016, 1, 16, 15, 3, 57, 999, false));
        expect (Time::fromISO8601 ("20160216T150357.999+0000")      == Time (2016, 1, 16, 15, 3, 57, 999, false));
        expect (Time::fromISO8601 ("2016-02-16T15:03:57.999Z")      == Time (2016, 1, 16, 15, 3, 57, 999, false));
        expect (Time::fromISO8601 ("2016-02-16T15:03:57,999Z")      == Time (2016, 1, 16, 15, 3, 57, 999, false));
        expect (Time::fromISO8601 ("20160216T150357.999Z")          == Time (2016, 1, 16, 15, 3, 57, 999, false));
        expect (Time::fromISO8601 ("20160216T150357,999Z")          == Time (2016, 1, 16, 15, 3, 57, 999, false));

        expect (Time::fromISO8601 ("2016-02-16T15:03:57.999-02:30") == Time (2016, 1, 16, 17, 33, 57, 999, false));
        expect (Time::fromISO8601 ("2016-02-16T15:03:57,999-02:30") == Time (2016, 1, 16, 17, 33, 57, 999, false));
        expect (Time::fromISO8601 ("20160216T150357.999-0230")      == Time (2016, 1, 16, 17, 33, 57, 999, false));
        expect (Time::fromISO8601 ("20160216T150357,999-0230")      == Time (2016, 1, 16, 17, 33, 57, 999, false));

        expect (Time (1970,  0,  1,  0,  0,  0, 0, false) == Time (0));
        expect (Time (2106,  1,  7,  6, 28, 15, 0, false) == Time (4294967295000));
        expect (Time (2007, 10,  7,  1,  7, 20, 0, false) == Time (1194397640000));
        expect (Time (2038,  0, 19,  3, 14,  7, 0, false) == Time (2147483647000));
        expect (Time (2016,  2,  7, 11, 20,  8, 0, false) == Time (1457349608000));
        expect (Time (1969, 11, 31, 23, 59, 59, 0, false) == Time (-1000));
        expect (Time (1901, 11, 13, 20, 45, 53, 0, false) == Time (-2147483647000));

        expect (Time (1982, 1, 1, 12, 0, 0, 0, true) + RelativeTime::days (365) == Time (1983, 1, 1, 12, 0, 0, 0, true));
        expect (Time (1970, 1, 1, 12, 0, 0, 0, true) + RelativeTime::days (365) == Time (1971, 1, 1, 12, 0, 0, 0, true));

       #if ! DRX_32BIT
        expect (Time (2038, 1, 1, 12, 0, 0, 0, true) + RelativeTime::days (365) == Time (2039, 1, 1, 12, 0, 0, 0, true));
       #endif

        expect (Time (1982, 1, 1, 12, 0, 0, 0, false) + RelativeTime::days (365) == Time (1983, 1, 1, 12, 0, 0, 0, false));
        expect (Time (1970, 1, 1, 12, 0, 0, 0, false) + RelativeTime::days (365) == Time (1971, 1, 1, 12, 0, 0, 0, false));
        expect (Time (2038, 1, 1, 12, 0, 0, 0, false) + RelativeTime::days (365) == Time (2039, 1, 1, 12, 0, 0, 0, false));
    }
};

static TimeTests timeTests;

#endif

} // namespace drx
