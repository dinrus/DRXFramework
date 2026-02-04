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
/*
    A collection of methods and types for breaking down text into a unicode representation.
*/
class Unicode
{
    using Key = Txt;

public:
    Unicode() = delete;

    //==============================================================================
    /*  A unicode Codepoint, from this you can infer various Unicode properties such
        as direction, logical string index and breaking type, etc.
    */
    struct Codepoint
    {
        u32 codepoint;

        TextBreakType breaking;  // Breaking characteristics of this codepoint

        TextScript script;       // Script class for this codepoint
    };

    template <typename Value>
    static auto prefix (Span<Value> v, size_t num)
    {
        return Span { v.data(), std::min (v.size(), num) };
    }

    template <typename Value>
    static auto removePrefix (Span<Value> v, size_t num)
    {
        const auto increment = std::min (v.size(), num);
        return Span { v.data() + increment, v.size() - increment };
    }

    //==============================================================================
    /*  Performs unicode analysis on a piece of text and returns an array of Codepoints
        in logical order.
    */
    static Array<Codepoint> performAnalysis (const Txt& string)
    {
        if (string.isEmpty())
            return {};

        thread_local LruCache<Key, Array<Unicode::Codepoint>> cache;

        return cache.get (string, analysisCallback);
    }

    //==============================================================================
    template <typename Traits>
    struct Iterator
    {
        using ValueType = typename Traits::ValueType;

        Iterator() = default;
        explicit Iterator (Span<ValueType> s) : data (s) {}

        std::optional<Span<ValueType>> next()
        {
            if (data.empty())
                return {};

            const auto breakpoint = std::find_if (data.begin(), data.end(), [&] (const auto& i)
            {
                return ! Traits::compare (i, data.front());
            });
            const auto lengthToBreak = (size_t) std::distance (data.begin(), breakpoint) + (Traits::includeBreakingIndex() ? 1 : 0);
            const ScopeGuard scope { [&] { data = removePrefix (data, lengthToBreak); } };
            return prefix (data, lengthToBreak);
        }

    private:
        Span<ValueType> data;
    };

    struct LineTraits
    {
        using ValueType = const Codepoint;

        static b8 compare (const Codepoint& t1, const Codepoint&)
        {
            return t1.breaking != TextBreakType::hard;
        }

        static b8 includeBreakingIndex() { return true; }
    };

    using LineBreakIterator = Iterator<LineTraits>;

    struct WordTraits
    {
        using ValueType = const Codepoint;

        static b8 compare (const Codepoint& t1, const Codepoint&)
        {
            return t1.breaking != TextBreakType::soft;
        }

        static b8 includeBreakingIndex() { return false; }
    };

    using WordBreakIterator = Iterator<WordTraits>;

    struct ScriptTraits
    {
        using ValueType = const Codepoint;

        static b8 compare (const Codepoint& t1, const Codepoint& t2)
        {
            return t1.script == t2.script;
        }

        static b8 includeBreakingIndex() { return false; }
    };

    using ScriptRunIterator = Iterator<ScriptTraits>;

private:
    static Array<Unicode::Codepoint> analysisCallback (const Key& key)
    {
        auto analysisBuffer = [&key]
        {
            std::vector<UnicodeAnalysisPoint> points;

            const auto data   = key.toUTF32();
            const auto length = data.length();

            points.reserve (length);

            std::transform (data.getAddress(), data.getAddress() + length, std::back_inserter (points), [] (u32 cp)
            {
                UnicodeAnalysisPoint p { cp, UnicodeDataTable::getDataForCodepoint (cp) };

                // Define this to enable TR9 debugging. All upper case
                // characters will be interpreted as right-to-left.
               #if defined (DRX_TR9_UPPERCASE_IS_RTL)
                if (65 <= cp && cp <= 90)
                    p.data.bidi = BidiType::al;
               #endif

                return p;
            });

            return points;
        }();

        Array<Unicode::Codepoint> result;
        result.resize ((i32) analysisBuffer.size());

        for (size_t i = 0; i < analysisBuffer.size(); i++)
            result.getReference ((i32) i).codepoint = analysisBuffer[i].character;

        TR24::analyseScripts (analysisBuffer, [&result] (i32 index, TextScript script)
        {
            result.getReference (index).script = script;
        });

        TR14::analyseLineBreaks (analysisBuffer, [&result] (i32 index, TextBreakType type)
        {
            result.getReference (index).breaking = type;
        });

        return result;
    }
};

namespace detail
{

std::vector<i32> UnicodeHelpers::getLineBreaks (const Txt& data)
{
    std::vector<i32> lineBreaks;
    const auto analysis = Unicode::performAnalysis (data);

    for (const auto [index, codepoint] : enumerate (analysis, i32{}))
    {
        if (codepoint.breaking == TextBreakType::hard)
            lineBreaks.push_back (index);
    }

    return lineBreaks;
}

} // namespace detail

} // namespace drx
