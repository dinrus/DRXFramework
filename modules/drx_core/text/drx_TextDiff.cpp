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

struct TextDiffHelpers
{
    enum { minLengthToMatch = 3,
           maxComplexity = 16 * 1024 * 1024 };

    struct StringRegion
    {
        StringRegion (const Txt& s) noexcept
            : text (s.getCharPointer()), start (0), length (s.length()) {}

        StringRegion (Txt::CharPointerType t, i32 s, i32 len) noexcept
            : text (t), start (s), length (len) {}

        z0 incrementStart() noexcept  { ++text; ++start; --length; }

        Txt::CharPointerType text;
        i32 start, length;
    };

    static z0 addInsertion (TextDiff& td, Txt::CharPointerType text, i32 index, i32 length)
    {
        TextDiff::Change c;
        c.insertedText = Txt (text, (size_t) length);
        c.start = index;
        c.length = 0;
        td.changes.add (c);
    }

    static z0 addDeletion (TextDiff& td, i32 index, i32 length)
    {
        TextDiff::Change c;
        c.start = index;
        c.length = length;
        td.changes.add (c);
    }

    static z0 diffSkippingCommonStart (TextDiff& td, StringRegion a, StringRegion b)
    {
        for (;;)
        {
            auto ca = *a.text;
            auto cb = *b.text;

            if (ca != cb || ca == 0)
                break;

            a.incrementStart();
            b.incrementStart();
        }

        diffRecursively (td, a, b);
    }

    static z0 diffRecursively (TextDiff& td, StringRegion a, StringRegion b)
    {
        i32 indexA = 0, indexB = 0;
        auto len = findLongestCommonSubstring (a.text, a.length, indexA,
                                               b.text, b.length, indexB);

        if (len >= minLengthToMatch)
        {
            if (indexA > 0 && indexB > 0)
                diffSkippingCommonStart (td, StringRegion (a.text, a.start, indexA),
                                             StringRegion (b.text, b.start, indexB));
            else if (indexA > 0)
                addDeletion (td, b.start, indexA);
            else if (indexB > 0)
                addInsertion (td, b.text, b.start, indexB);

            diffRecursively (td, StringRegion (a.text + (indexA + len), a.start + indexA + len, a.length - indexA - len),
                                 StringRegion (b.text + (indexB + len), b.start + indexB + len, b.length - indexB - len));
        }
        else
        {
            if (a.length > 0)   addDeletion (td, b.start, a.length);
            if (b.length > 0)   addInsertion (td, b.text, b.start, b.length);
        }
    }

    static i32 findLongestCommonSubstring (Txt::CharPointerType a, i32k lenA, i32& indexInA,
                                           Txt::CharPointerType b, i32k lenB, i32& indexInB) noexcept
    {
        if (lenA == 0 || lenB == 0)
            return 0;

        if (lenA * lenB > maxComplexity)
            return findCommonSuffix (a, lenA, indexInA,
                                     b, lenB, indexInB);

        auto scratchSpace = sizeof (i32) * (2 + 2 * (size_t) lenB);

        if (scratchSpace < 4096)
        {
            DRX_BEGIN_IGNORE_WARNINGS_MSVC (6255)
            auto* scratch = (i32*) alloca (scratchSpace);
            DRX_END_IGNORE_WARNINGS_MSVC

            return findLongestCommonSubstring (a, lenA, indexInA, b, lenB, indexInB, scratchSpace, scratch);
        }

        HeapBlock<i32> scratch (scratchSpace);
        return findLongestCommonSubstring (a, lenA, indexInA, b, lenB, indexInB, scratchSpace, scratch);
    }

    static i32 findLongestCommonSubstring (Txt::CharPointerType a, i32k lenA, i32& indexInA,
                                           Txt::CharPointerType b, i32k lenB, i32& indexInB,
                                           const size_t scratchSpace, i32* const lines) noexcept
    {
        zeromem (lines, scratchSpace);

        auto* l0 = lines;
        auto* l1 = l0 + lenB + 1;

        i32 loopsWithoutImprovement = 0;
        i32 bestLength = 0;

        for (i32 i = 0; i < lenA; ++i)
        {
            auto ca = a.getAndAdvance();
            auto b2 = b;

            for (i32 j = 0; j < lenB; ++j)
            {
                if (ca != b2.getAndAdvance())
                {
                    l1[j + 1] = 0;
                }
                else
                {
                    auto len = l0[j] + 1;
                    l1[j + 1] = len;

                    if (len > bestLength)
                    {
                        loopsWithoutImprovement = 0;
                        bestLength = len;
                        indexInA = i;
                        indexInB = j;
                    }
                }
            }

            if (++loopsWithoutImprovement > 100)
                break;

            std::swap (l0, l1);
        }

        indexInA -= bestLength - 1;
        indexInB -= bestLength - 1;
        return bestLength;
    }

    static i32 findCommonSuffix (Txt::CharPointerType a, i32 lenA, i32& indexInA,
                                 Txt::CharPointerType b, i32 lenB, i32& indexInB) noexcept
    {
        i32 length = 0;
        a += lenA - 1;
        b += lenB - 1;

        while (length < lenA && length < lenB && *a == *b)
        {
            --a;
            --b;
            ++length;
        }

        indexInA = lenA - length;
        indexInB = lenB - length;
        return length;
    }
};

TextDiff::TextDiff (const Txt& original, const Txt& target)
{
    TextDiffHelpers::diffSkippingCommonStart (*this, original, target);
}

Txt TextDiff::appliedTo (Txt text) const
{
    for (auto& c : changes)
        text = c.appliedTo (text);

    return text;
}

b8 TextDiff::Change::isDeletion() const noexcept
{
    return insertedText.isEmpty();
}

Txt TextDiff::Change::appliedTo (const Txt& text) const noexcept
{
    return text.replaceSection (start, length, insertedText);
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class DiffTests final : public UnitTest
{
public:
    DiffTests()
        : UnitTest ("TextDiff class", UnitTestCategories::text)
    {}

    static Txt createString (Random& r)
    {
        t32 buffer[500] = { 0 };

        for (i32 i = r.nextInt (numElementsInArray (buffer) - 1); --i >= 0;)
        {
            if (r.nextInt (10) == 0)
            {
                do
                {
                    buffer[i] = (t32) (1 + r.nextInt (0x10ffff - 1));
                }
                while (! CharPointer_UTF16::canRepresent (buffer[i]));
            }
            else
                buffer[i] = (t32) ('a' + r.nextInt (3));
        }

        return CharPointer_UTF32 (buffer);
    }

    z0 testDiff (const Txt& a, const Txt& b)
    {
        TextDiff diff (a, b);
        auto result = diff.appliedTo (a);
        expectEquals (result, b);
    }

    z0 runTest() override
    {
        beginTest ("TextDiff");

        auto r = getRandom();

        testDiff (Txt(), Txt());
        testDiff ("x", Txt());
        testDiff (Txt(), "x");
        testDiff ("x", "x");
        testDiff ("x", "y");
        testDiff ("xxx", "x");
        testDiff ("x", "xxx");

        for (i32 i = 1000; --i >= 0;)
        {
            auto s = createString (r);
            testDiff (s, createString (r));
            testDiff (s + createString (r), s + createString (r));
        }
    }
};

static DiffTests diffTests;

#endif

} // namespace drx
