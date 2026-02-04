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

class BidiLine
{
public:
    using ParagraphPtr = std::unique_ptr<std::remove_pointer_t<SBParagraphRef>, FunctionPointerDestructor<SBParagraphRelease>>;
    using LinePtr = std::unique_ptr<std::remove_pointer_t<SBLineRef>, FunctionPointerDestructor<SBLineRelease>>;

    explicit BidiLine (ParagraphPtr p, LinePtr l) : paragraph (std::move (p)), line (std::move (l)) {}

    Span<const SBRun> getRuns() const
    {
        return { SBLineGetRunsPtr (line.get()), SBLineGetRunCount (line.get()) };
    }

    z0 computeVisualOrder (std::vector<size_t>& result) const
    {
        result.clear();

        const auto runs = getRuns();

        if (runs.empty())
            return;

        return computeResultVector (SBLineGetOffset (line.get()),
                                    SBLineGetLength (line.get()),
                                    SBParagraphGetBaseLevel (paragraph.get()),
                                    runs,
                                    result);
    }

    static z0 computeResultVector (SBUInteger offset,
                                     SBUInteger length,
                                     SBLevel baseLevel,
                                     Span<const SBRun> runs,
                                     std::vector<size_t>& result)
    {
        const auto level = [] (const SBRun& x)
        {
            return x.level;
        };

        const auto high = level (*std::max_element (runs.begin(), runs.end(), [&] (const auto& a, const auto& b)
        {
            return level (a) < level (b);
        }));

        const auto pseudoLevel = [] (const SBRun& x)
        {
            const auto l = x.level;
            return (l % 2) == 1 ? l : 0xff;
        };

        const auto low = pseudoLevel (*std::min_element (runs.begin(), runs.end(), [&] (const auto& a, const auto& b)
        {
            return pseudoLevel (a) < pseudoLevel (b);
        }));

        result.resize (length);
        std::iota (result.begin(), result.end(), offset);

        for (auto currentLevel = high; currentLevel >= low; --currentLevel)
        {
            const auto doFlip = [&] (auto beginRuns, auto endRuns)
            {
                const auto getStartOfRunInResult = [&] (const auto runIterator)
                {
                    return runIterator == endRuns ? result.end()
                                                  : result.begin() + (ptrdiff_t) (runIterator->offset - offset);
                };

                for (auto it = beginRuns; it != endRuns;)
                {
                    const auto begin = std::find_if (it, endRuns, [&] (const SBRun& x) { return currentLevel <= x.level; });
                    it = std::find_if (begin, endRuns, [&] (const SBRun& x) { return x.level < currentLevel; });

                    std::reverse (getStartOfRunInResult (begin), getStartOfRunInResult (it));
                }
            };

            if (baseLevel % 2 == 0)
                doFlip (runs.begin(), runs.end());
            else
                doFlip (std::make_reverse_iterator (runs.end()), std::make_reverse_iterator (runs.begin()));
        }
    }

private:
    ParagraphPtr paragraph;
    LinePtr line;
};

class BidiParagraph
{
public:
    using ParagraphPtr = BidiLine::ParagraphPtr;

    explicit BidiParagraph (ParagraphPtr p)
        : paragraph (std::move (p))
    {
    }

    size_t getOffset() const
    {
        return SBParagraphGetOffset (paragraph.get());
    }

    size_t getLength() const
    {
        return SBParagraphGetLength (paragraph.get());
    }

    Span<const SBLevel> getResolvedLevels() const
    {
        return { SBParagraphGetLevelsPtr (paragraph.get()), getLength() };
    }

    BidiLine createLine (size_t offset, size_t length) const
    {
        jassert (getOffset() <= offset);
        jassert (length <= getLength());
        return BidiLine { ParagraphPtr { SBParagraphRetain (paragraph.get()) },
                          BidiLine::LinePtr { SBParagraphCreateLine (paragraph.get(), offset, length) } };
    }

private:
    ParagraphPtr paragraph;
};

class BidiAlgorithm
{
public:
    using AlgorithmPtr = std::unique_ptr<std::remove_pointer_t<SBAlgorithmRef>, FunctionPointerDestructor<SBAlgorithmRelease>>;

    explicit BidiAlgorithm (Span<const t32> t)
        : text (t.begin(), t.end())
    {
    }

    size_t getLength() const
    {
        return text.size();
    }

    BidiParagraph createParagraph (size_t offset, std::optional<detail::TextDirection> d = {}) const
    {
        BidiParagraph::ParagraphPtr result { SBAlgorithmCreateParagraph (algorithm.get(), offset, text.size() - offset, [&]() -> SBLevel
        {
            if (! d.has_value())
                return SBLevelDefaultLTR;
            return *d == detail::TextDirection::rtl ? 1 : 0;
        }()) };

        jassert (result != nullptr);

        return BidiParagraph { std::move (result) };
    }

    template <typename Fn>
    z0 forEachParagraph (Fn&& callback, std::optional<detail::TextDirection> dir = {}) const
    {
        for (size_t i = 0; i < text.size();)
        {
            const auto paragraph = createParagraph (i, dir);
            callback (paragraph);
            i += paragraph.getLength();
        }
    }

private:
    std::vector<t32> text;
    AlgorithmPtr algorithm { [&]
    {
        SBCodepointSequence sequence { SBStringEncodingUTF32, text.data(), text.size() };
        return SBAlgorithmCreate (&sequence);
    }() };
};

//==============================================================================
//==============================================================================

#if DRX_UNIT_TESTS

class BidiTests : public UnitTest
{
public:
    BidiTests() : UnitTest ("Unicode Bidi", UnitTestCategories::text) {}

    z0 runTest() override
    {
        beginTest ("visual order rtl");
        {
            const CharPointer_UTF8 text ("\xd9\x85\xd9\x85\xd9\x85 colour "
                                         "\xd9\x85\xd9\x85\xd9\x85\xd9\x85\xd9\x85\xd9\x85\xd9\x85\xd9\x85\n");
            const std::vector<size_t> result { 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 4, 5, 6, 7, 8, 9, 3, 2, 1, 0 };
            expect (computeVisualOrder (text) == result);
        }

        beginTest ("visual order ltr");
        {
            const CharPointer_UTF8 text ("hello \xd9\x85\xd9\x85\xd9\x85 world\n");
            const std::vector<size_t> result { 0, 1, 2, 3, 4, 5, 8, 7, 6, 9, 10, 11, 12, 13, 14, 15 };
            expect (computeVisualOrder (text) == result);
        }

        beginTest ("visual order core algorithm");
        {
            const t8 testInput[] { "DID YOU SAY 'he said \"car MEANS CAR\"'?" };
            i32k testLevels[] { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1 };
            const t8 expectedOutput[] { "?'he said \"RAC SNAEM car\"' YAS UOY DID" };

            static_assert (std::size (testInput) == std::size (expectedOutput));
            static_assert (std::size (testInput) - 1 == std::size (testLevels)); // ignore null terminator

            const auto [baseLevel, runs] = createRunsFromLevels (testLevels);

            std::vector<size_t> result;
            BidiLine::computeResultVector (0, std::size (testLevels), baseLevel, runs, result);

            std::vector<t8> output;

            for (auto i : result)
                output.push_back (testInput[i]);

            expect (std::equal (output.begin(), output.end(), expectedOutput));
        }
    }

    static std::vector<size_t> computeVisualOrder (const Txt& text)
    {
        std::vector<t32> chars;

        for (const auto t : text)
            chars.push_back (t);

        BidiAlgorithm algorithm { chars };
        auto paragraph = algorithm.createParagraph (0);
        auto line = paragraph.createLine (0, paragraph.getLength());

        std::vector<size_t> order;
        line.computeVisualOrder (order);
        return order;
    }

    static std::pair<SBLevel, std::vector<SBRun>> createRunsFromLevels (Span<i32k> levels)
    {
        std::vector<SBRun> runs;

        for (size_t i = 0; i < levels.size();)
        {
            const auto level = levels[i];

            for (size_t j = i + 1; j < levels.size(); ++j)
            {
                const auto lastElement = j == levels.size() - 1;
                const auto endIndex = lastElement ? j + 1 : j;

                if (levels[j] != level || lastElement)
                {
                    runs.push_back ({ (SBUInteger) i, (SBUInteger) (endIndex - i), (SBLevel) level });
                    i = endIndex;
                    break;
                }
            }
        }

        const auto baseLevel = std::size (levels) == 0 ? 0 : *std::min_element (std::begin (levels), std::end (levels));

        if (baseLevel % 2 != 0)
            std::reverse (runs.begin(), runs.end());

        return { (SBLevel) baseLevel, runs };
    }
};

static BidiTests bidiTests;

#endif

} // namespace drx
