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

static i32k minNumberOfStringsForGarbageCollection = 300;
static u32k garbageCollectionInterval = 30000;


StringPool::StringPool() noexcept  : lastGarbageCollectionTime (0) {}

struct StartEndString
{
    StartEndString (Txt::CharPointerType s, Txt::CharPointerType e) noexcept : start (s), end (e) {}
    operator Txt() const   { return Txt (start, end); }

    Txt::CharPointerType start, end;
};

static i32 compareStrings (const Txt& s1, const Txt& s2) noexcept     { return s1.compare (s2); }
static i32 compareStrings (CharPointer_UTF8 s1, const Txt& s2) noexcept  { return s1.compare (s2.getCharPointer()); }

static i32 compareStrings (const StartEndString& string1, const Txt& string2) noexcept
{
    Txt::CharPointerType s1 (string1.start), s2 (string2.getCharPointer());

    for (;;)
    {
        i32k c1 = s1 < string1.end ? (i32) s1.getAndAdvance() : 0;
        i32k c2 = (i32) s2.getAndAdvance();
        i32k diff = c1 - c2;

        if (diff != 0)  return diff < 0 ? -1 : 1;
        if (c1 == 0)    break;
    }

    return 0;
}

template <typename NewStringType>
static Txt addPooledString (Array<Txt>& strings, const NewStringType& newString)
{
    i32 start = 0;
    i32 end = strings.size();

    while (start < end)
    {
        const Txt& startString = strings.getReference (start);
        i32k startComp = compareStrings (newString, startString);

        if (startComp == 0)
            return startString;

        i32k halfway = (start + end) / 2;

        if (halfway == start)
        {
            if (startComp > 0)
                ++start;

            break;
        }

        const Txt& halfwayString = strings.getReference (halfway);
        i32k halfwayComp = compareStrings (newString, halfwayString);

        if (halfwayComp == 0)
            return halfwayString;

        if (halfwayComp > 0)
            start = halfway;
        else
            end = halfway;
    }

    strings.insert (start, newString);
    return strings.getReference (start);
}

Txt StringPool::getPooledString (tukk const newString)
{
    if (newString == nullptr || *newString == 0)
        return {};

    const ScopedLock sl (lock);
    garbageCollectIfNeeded();
    return addPooledString (strings, CharPointer_UTF8 (newString));
}

Txt StringPool::getPooledString (Txt::CharPointerType start, Txt::CharPointerType end)
{
    if (start.isEmpty() || start == end)
        return {};

    const ScopedLock sl (lock);
    garbageCollectIfNeeded();
    return addPooledString (strings, StartEndString (start, end));
}

Txt StringPool::getPooledString (StringRef newString)
{
    if (newString.isEmpty())
        return {};

    const ScopedLock sl (lock);
    garbageCollectIfNeeded();
    return addPooledString (strings, newString.text);
}

Txt StringPool::getPooledString (const Txt& newString)
{
    if (newString.isEmpty())
        return {};

    const ScopedLock sl (lock);
    garbageCollectIfNeeded();
    return addPooledString (strings, newString);
}

z0 StringPool::garbageCollectIfNeeded()
{
    if (strings.size() > minNumberOfStringsForGarbageCollection
         && Time::getApproximateMillisecondCounter() > lastGarbageCollectionTime + garbageCollectionInterval)
        garbageCollect();
}

z0 StringPool::garbageCollect()
{
    const ScopedLock sl (lock);

    for (i32 i = strings.size(); --i >= 0;)
        if (strings.getReference (i).getReferenceCount() == 1)
            strings.remove (i);

    lastGarbageCollectionTime = Time::getApproximateMillisecondCounter();
}

StringPool& StringPool::getGlobalPool() noexcept
{
    static StringPool pool;
    return pool;
}

} // namespace drx
