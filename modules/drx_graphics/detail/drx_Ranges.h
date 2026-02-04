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

namespace drx::detail
{

inline std::optional<Range<z64>> getRangeIntersectionWith (Range<z64> r1, Range<z64> r2)
{
    auto intersection = r1.getIntersectionWith (r2);

    if (intersection.getLength() == 0)
        return std::nullopt;

    return Range<z64> { intersection };
}

/*  This is to get rid of the warning where advance isn't of type difference_type.
*/
template <typename Iterator, typename Value>
auto iteratorWithAdvance (Iterator&& it, Value advance)
{
    auto outIt = std::move (it);
    std::advance (outIt, static_cast<typename std::iterator_traits<Iterator>::difference_type> (advance));
    return outIt;
}

struct Ranges final
{
    struct Ops
    {
        Ops() = delete;

        struct New
        {
            explicit New (size_t x) : index { x } {}

            size_t index;
        };

        struct Split
        {
            Split (size_t x, Range<z64> leftRangeIn, Range<z64> rightRangeIn)
                : index { x },
                  leftRange { leftRangeIn },
                  rightRange { rightRangeIn }
            {}

            size_t index;
            Range<z64> leftRange;
            Range<z64> rightRange;
        };

        struct Erase
        {
            explicit Erase (Range<size_t> x) : range { x } {}

            Range<size_t> range;
        };

        struct Change
        {
            Change (size_t x, Range<z64> oldRangeIn, Range<z64> newRangeIn)
                : index { x },
                  oldRange { oldRangeIn },
                  newRange { newRangeIn }
            {}

            size_t index;
            Range<z64> oldRange;
            Range<z64> newRange;
        };
    };

    using Op = std::variant<Ops::New, Ops::Split, Ops::Erase, Ops::Change>;

    using Operations = std::vector<Op>;

    Ranges() = default;

    explicit Ranges (std::vector<Range<z64>> rangesIn)
        : ranges (std::move (rangesIn))
    {
       #if DRX_DEBUG
        jassert (std::is_sorted (ranges.cbegin(), ranges.cend(), [] (const auto& a, const auto& b)
        {
            return a.getEnd() < b.getStart();
        }));
       #endif
    }

    b8 operator== (const Ranges& other) const { return ranges == other.ranges; }
    b8 operator!= (const Ranges& other) const { return ranges != other.ranges; }

    auto& getRanges()       { return ranges; }
    auto& getRanges() const { return ranges; }

    //==============================================================================
    // Basic operations
    z0 split (z64 i, Operations& ops)
    {
        const auto elemIndex = getIndexForEnclosingRange (i);

        if (! elemIndex.has_value())
            return;

        auto& elem = ranges[*elemIndex];

        if (elem.getStart() == i)
            return;

        ops.push_back (Ops::Split { *elemIndex, elem.withEnd (i), elem.withStart (i) });

        const auto oldLength = elem.getLength();
        elem.setEnd (i);

        ranges.insert (iteratorWithAdvance (ranges.begin(), *elemIndex + 1),
                       { i, i + oldLength - elem.getLength() });
    }

    z0 erase (Range<z64> r, Operations& ops)
    {
        if (r.isEmpty())
            return;

        for (auto i : { r.getStart(), r.getEnd() })
            split (i, ops);

        const auto firstToDelete = std::lower_bound (ranges.begin(),
                                                     ranges.end(),
                                                     r.getStart(),
                                                     [] (auto& elem, auto& value)
                                                     { return elem.getStart() < value; });

        const auto beyondLastToDelete = std::lower_bound (firstToDelete,
                                                          ranges.end(),
                                                          r.getEnd(),
                                                          [] (auto& elem, auto& value)
                                                          { return elem.getStart() < value; });

        if (firstToDelete != ranges.end())
            ops.push_back (Ops::Erase { { getIndex (firstToDelete), getIndex (beyondLastToDelete) } });

        ranges.erase (firstToDelete, beyondLastToDelete);
    }

    z0 drop (Range<z64> r, Operations& ops)
    {
        erase (r, ops);
        shift (r.getEnd(), -r.getLength(), ops);
    }

    /*  Shift all ranges starting at or beyond the specified from parameter, by the specified amount.
    */
    z0 shift (z64 from, z64 amount, Operations& ops)
    {
        if (amount == 0)
            return;

        const auto shiftStartingFrom = std::lower_bound (ranges.begin(),
                                                         ranges.end(),
                                                         from,
                                                         [] (auto& elem, auto& value)
                                                         { return elem.getStart() < value; });

        for (auto it = shiftStartingFrom; it < ranges.end(); ++it)
        {
            const auto oldRange = *it;
            *it += amount;
            ops.push_back (Ops::Change { getIndex (it), oldRange, *it });
        }
    }

    z0 set (Range<z64> newRange, Operations& ops)
    {
        if (newRange.isEmpty())
            return;

        erase (newRange, ops);

        const auto insertBefore = std::lower_bound (ranges.begin(),
                                                    ranges.end(),
                                                    newRange.getStart(),
                                                    [] (auto& elem, auto& value)
                                                    { return elem.getStart() < value; });

        ops.push_back (Ops::New { getIndex (insertBefore) });
        ranges.insert (insertBefore, newRange);
    }

    z0 insert (Range<z64> newRange, Operations& ops)
    {
        if (newRange.isEmpty())
            return;

        split (newRange.getStart(), ops);
        shift (newRange.getStart(), newRange.getLength(), ops);

        const auto insertBefore = std::lower_bound (ranges.begin(),
                                                    ranges.end(),
                                                    newRange.getStart(),
                                                    [] (auto& elem, auto& value)
                                                    { return elem.getStart() < value; });

        const auto insertBeforeIndex = getIndex (insertBefore);

        ranges.insert (insertBefore, newRange);
        ops.push_back (Ops::New { insertBeforeIndex });
    }

    //==============================================================================
    // Convenience functions
    z0 clear()
    {
        ranges.clear();
    }

    z0 eraseFrom (z64 i, Operations& ops)
    {
        if (ranges.empty())
            return;

        erase ({ i, ranges.back().getEnd() }, ops);
    }

    /*  Merges neighbouring ranges backwards if they form a contiguous range.
    */
    z0 mergeBack (size_t i, Operations& ops)
    {
        jassert (isPositiveAndBelow (i, ranges.size()));

        if (i == 0 || i >= ranges.size())
            return;

        const auto start = i - 1;
        const auto end = i;

        if (ranges[start].getEnd() != ranges[end].getStart())
            return;

        const auto oldRange = ranges[start];
        ranges[start].setEnd (ranges[end].getEnd());
        ops.push_back (Ops::Change { start, oldRange, ranges[start] });
        ops.push_back (Ops::Erase { { end, end + 1 } });

        ranges.erase (iteratorWithAdvance (ranges.begin(), end),
                      iteratorWithAdvance (ranges.begin(), end + 1));
    }

    /*  Returns the ranges that have an intersection with the provided range. */
    std::vector<Range<z64>> getIntersectionsWith (Range<z64> r) const
    {
        std::vector<Range<z64>> result;

        const auto firstOverlapping = [&]
        {
            auto it = std::lower_bound (ranges.cbegin(),
                                        ranges.cend(),
                                        r,
                                        [] (auto& elem, auto& value)
                                        { return elem.getEnd() <= value.getStart(); });

            return it;
        }();

        const auto lastOverlapping = [&]
        {
            auto it = std::lower_bound (firstOverlapping,
                                        ranges.cend(),
                                        r,
                                        [] (auto& elem, auto& value)
                                        { return elem.getEnd() < value.getEnd(); });

            return it != std::cend (ranges) ? it + 1 : it;
        }();

        for (auto it = firstOverlapping; it != lastOverlapping; ++it)
            if (auto intersection = getRangeIntersectionWith (Range<z64> { *it }, r))
                result.push_back (*intersection);

        return result;
    }

    //==============================================================================
    size_t size() const
    {
        return ranges.size();
    }

    b8 isEmpty() const
    {
        return ranges.empty();
    }

    Range<z64> get (size_t rangeIndex) const
    {
        return ranges[rangeIndex];
    }

    //==============================================================================
    auto begin() const
    {
        return ranges.cbegin();
    }

    auto cbegin() const
    {
        return ranges.cbegin();
    }

    auto end() const
    {
        return ranges.cend();
    }

    auto cend() const
    {
        return ranges.cend();
    }

    auto* data() const
    {
        return ranges.data();
    }

    /* Returns an iterator for the Range element which includes the provided value. */
    auto find (z64 i) const
    {
        const auto it = std::lower_bound (cbegin(),
                                          cend(),
                                          i,
                                          [] (auto& elem, auto& value) { return elem.getEnd() <= value; });

        return it != cend() && it->getStart() <= i ? it : cend();
    }

    std::optional<size_t> getIndexForEnclosingRange (z64 positionInTextRange) const
    {
        const auto iter = find (positionInTextRange);
        return iter != ranges.end() ? std::make_optional (getIndex (iter)) : std::nullopt;
    }

    /* Возвращает true, если this object covers each element in the provided range. For empty ranges it
       returns true if the start value is covered.
    */
    b8 covers (Range<z64> range) const
    {
        for (auto curr = find (range.getStart()), prev = curr; curr != cend(); ++curr)
        {
            if (prev != curr && prev->getEnd() != curr->getStart())
                return false;

            if (range.getEnd() <= curr->getEnd())
                return true;

            prev = curr;
        }

        return false;
    }

private:
    size_t getIndex (std::vector<Range<z64>>::const_iterator it) const
    {
        return (size_t) std::distance (ranges.cbegin(), it);
    }

    std::vector<Range<z64>> ranges;
};

//==============================================================================
template <typename T, typename = z0>
constexpr auto hasEqualityOperator = false;

template <typename T>
constexpr auto hasEqualityOperator<T, std::void_t<decltype (std::declval<T>() == std::declval<T>())>> = true;

//==============================================================================
template <typename T>
struct RangedValuesIteratorItem
{
    Range<z64> range;
    T& value;
};

//==============================================================================
template <typename T>
class RangedValuesIterator
{
private:
    using InternalIterator = const Range<z64>*;

public:
    using value_type = RangedValuesIteratorItem<T>;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;

    struct PointerProxy
    {
        explicit PointerProxy (reference r) : ref { r } {}

        auto operator->() const { return &ref; }

        reference ref;
    };

    using pointer = PointerProxy;
    using iterator_category = std::random_access_iterator_tag;

    RangedValuesIterator (T* valuesIn, InternalIterator iteratorBaseIn, InternalIterator iteratorIn)
        : values { valuesIn },
          iteratorBase { iteratorBaseIn },
          iterator { iteratorIn }
    {}

    RangedValuesIterator& operator+= (difference_type distance)
    {
        iterator += distance;
        return *this;
    }

    friend RangedValuesIterator operator+ (RangedValuesIterator i, difference_type d) { return i += d; }
    friend RangedValuesIterator operator+ (difference_type d, RangedValuesIterator i) { return i += d; }

    RangedValuesIterator& operator-= (difference_type distance)
    {
        iterator -= distance;
        return *this;
    }

    friend RangedValuesIterator operator- (RangedValuesIterator i, difference_type d) { return i -= d; }

    reference operator[] (difference_type d) const
    {
        auto it = iterator[d];
        return makeReference (it);
    }

    friend difference_type operator- (RangedValuesIterator a, RangedValuesIterator b)   { return a.iterator - b.iterator; }

    friend b8 operator<  (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator <  b.iterator; }
    friend b8 operator<= (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator <= b.iterator; }
    friend b8 operator>  (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator >  b.iterator; }
    friend b8 operator>= (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator >= b.iterator; }
    friend b8 operator== (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator == b.iterator; }
    friend b8 operator!= (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator != b.iterator; }

    RangedValuesIterator& operator++()           { ++iterator; return *this; }
    RangedValuesIterator& operator--()           { --iterator; return *this; }
    RangedValuesIterator  operator++ (i32) const { RangedValuesIterator copy (*this); ++(*this); return copy; }
    RangedValuesIterator  operator-- (i32) const { RangedValuesIterator copy (*this); --(*this); return copy; }

    reference operator* () const { return makeReference (iterator); }
    pointer   operator->() const { return PointerProxy { makeReference (iterator) }; }

private:
    reference makeReference (const InternalIterator& it) const
    {
        const auto valueIt = values + (size_t) std::distance (iteratorBase, it);

        return { *it, *valueIt };
    }

    T* values{};
    InternalIterator iteratorBase, iterator;
};

struct MergeEqualItemsYes{};
struct MergeEqualItemsNo{};

/*  Data structure for storing values associated with non-overlapping ranges.

    Has set() and insert() operations with the optional merging of ranges that contain equal values.
    These operations emit a sequence of simpler operations, that are more easily executable on an
    external container to bring it in sync with the ranges and values stored in this class.

    These operations are communicating things such as
    - "the range associated with element 5 of your container have changed"
    - "a new element was inserted at index 5, that refers to new data"

    The second example operation also implies, that all elements beyond the new index 5 element
    still refer to the same data, since when the data associated with a given element changes, a
    change operation is emitted.
*/
template <typename T>
class RangedValues
{
    template <typename Self>
    static auto getItemWithEnclosingRangeImpl (Self& self, z64 i)
    {
        const auto j = self.ranges.getIndexForEnclosingRange (i);
        return j ? std::make_optional (self.getItem (*j)) : std::nullopt;
    }

    auto tie() const { return std::tie (ranges, values); }

public:
    static constexpr b8 canMergeEqualItems = hasEqualityOperator<T>;

    template <typename RangedValuesType>
    static auto makeIterator (RangedValuesType* rv, const Range<z64>* base, const Range<z64>* iterator)
    {
        return RangedValuesIterator<std::remove_pointer_t<decltype (rv->values.data())>> (rv->values.data(), base, iterator);
    }

    //==============================================================================
    b8 operator== (const RangedValues& other) const { return tie() == other.tie(); }
    b8 operator!= (const RangedValues& other) const { return tie() != other.tie(); }

    //==============================================================================
    auto begin()
    {
        return makeIterator (this, ranges.data(), ranges.data());
    }

    auto begin() const
    {
        return makeIterator (this, ranges.data(), ranges.data());
    }

    auto cbegin() const
    {
        return makeIterator (this, ranges.data(), ranges.data());
    }

    auto end()
    {
        return makeIterator (this, ranges.data(), ranges.data() + ranges.size());
    }

    auto end() const
    {
        return makeIterator (this, ranges.data(), ranges.data() + ranges.size());
    }

    auto cend() const
    {
        return makeIterator (this, ranges.data(), ranges.data() + ranges.size());
    }

    struct Item
    {
        Range<z64> range;
        T& value;
    };

    struct ConstItem
    {
        Range<z64> range;
        const T& value;
    };

    template <typename U>
    static auto createSubSpan (U& s, size_t offset)
    {
        Span span { s };

        if (span.empty())
            return span;

        const auto newSize = s.size() - std::min (s.size(), offset);

        if (newSize == 0)
            return decltype (span){};

        auto start = s.begin();
        std::advance (start, static_cast<typename U::difference_type> (offset));

        return Span { start, newSize };
    }

    //==============================================================================
    // Basic operations
    template <typename MergeEquals = MergeEqualItemsYes>
    z0 set (Range<z64> r, T v, Ranges::Operations& ops, MergeEquals = {})
    {
        static_assert (std::is_same_v<MergeEqualItemsNo, MergeEquals> || canMergeEqualItems,
                       "You can't use MergeEqualItems::yes if your type doesn't have operator==.");

        const auto opsStart = ops.size();
        ranges.set (r, ops);
        applyOperations (createSubSpan (ops, opsStart), std::move (v));

        if constexpr (std::is_same_v<MergeEquals, MergeEqualItemsYes>)
        {
            mergeEqualItems (r.getStart(), ops);
            mergeEqualItems (r.getEnd(), ops);
        }
    }

    template <typename MergeEquals = MergeEqualItemsYes>
    z0 insert (Range<z64> r, T v, Ranges::Operations& ops, MergeEquals = {})
    {
        static_assert (std::is_same_v<MergeEquals, MergeEqualItemsNo> || canMergeEqualItems,
                       "You can't use MergeEqualItems::yes if your type doesn't have operator==.");

        const auto opsStart = ops.size();
        ranges.insert (r, ops);
        applyOperations (createSubSpan (ops, opsStart), std::move (v));

        if constexpr (std::is_same_v<MergeEquals, MergeEqualItemsYes>)
        {
            mergeEqualItems (r.getStart(), ops);
            mergeEqualItems (r.getEnd(), ops);
        }
    }

    // erase will always cause a discontinuity and thus, there is no opportunity to merge
    z0 erase (Range<z64> r, Ranges::Operations& ops)
    {
        const auto opsStart = ops.size();
        ranges.erase (r, ops);
        applyOperations (createSubSpan (ops, opsStart));
    }

    // drop moves subsequent ranges downward, and can end up in these ranges forming a contiguous
    // range with the ones on the left side of the drop. Hence, it makes sense to ask if we want
    // merging behaviour.
    template <typename MergeEquals = MergeEqualItemsYes>
    z0 drop (Range<z64> r, Ranges::Operations& ops, MergeEquals = {})
    {
        static_assert (std::is_same_v<MergeEquals, MergeEqualItemsNo> || canMergeEqualItems,
                       "You can't use MergeEqualItems::yes if your type doesn't have operator==.");

        const auto opsStart = ops.size();
        ranges.drop (r, ops);
        applyOperations (createSubSpan (ops, opsStart));

        if constexpr (std::is_same_v<MergeEquals, MergeEqualItemsYes>)
            mergeEqualItems (r.getStart(), ops);
    }

    //==============================================================================
    z0 clear()
    {
        ranges.clear();
        values.clear();
    }

    z0 shift (z64 from, z64 amount, Ranges::Operations& ops)
    {
        ranges.shift (from, amount, ops);
    }

    z0 eraseFrom (z64 i, Ranges::Operations& ops)
    {
        if (ranges.isEmpty())
            return;

        erase ({ i, ranges.get (ranges.size() - 1).getEnd() }, ops);
    }

    z0 eraseUpTo (z64 i, Ranges::Operations& ops)
    {
        if (ranges.isEmpty())
            return;

        erase ({ ranges.get (0).getStart(), i }, ops);
    }

    auto getItemWithEnclosingRange (z64 i)
    {
        return getItemWithEnclosingRangeImpl (*this, i);
    }

    auto getItemWithEnclosingRange (z64 i) const
    {
        return getItemWithEnclosingRangeImpl (*this, i);
    }

    // Finds the item whose range encloses the provided value
    template <typename Self>
    static auto findImpl (Self& self, z64 i)
    {
        return iteratorWithAdvance (self.begin(),
                                    std::distance (self.ranges.cbegin(), self.ranges.find (i)));
    }

    auto find (z64 i) { return findImpl (*this, i); }
    auto find (z64 i) const { return findImpl (*this, i); }

    Item getItem (size_t i)
    {
        jassert (i < values.size());

        return { ranges.get (i), values[i] };
    }

    ConstItem getItem (size_t i) const
    {
        jassert (i < values.size());

        return { ranges.get (i), values[i] };
    }

    Item front()
    {
        jassert (! ranges.isEmpty());
        return getItem (0);
    }

    ConstItem front() const
    {
        jassert (! ranges.isEmpty());
        return getItem (0);
    }

    Item back()
    {
        jassert (! ranges.isEmpty());
        return getItem (values.size() - 1);
    }

    ConstItem back() const
    {
        jassert (! ranges.isEmpty());
        return getItem (values.size() - 1);
    }

    /*  Returns the stored values together with the overlapping range, that overlap with the
        provided range.
    */
    RangedValues<T> getIntersectionsWith (Range<z64> r) const
    {
        const auto intersections = ranges.getIntersectionsWith (r);

        RangedValues<T> result;

        detail::Ranges::Operations ops;

        for (const auto& is : intersections)
        {
            auto valueIndex = ranges.getIndexForEnclosingRange (is.getStart());
            jassert (valueIndex.has_value());
            result.set (is, values[*valueIndex], ops, MergeEqualItemsNo{});
            ops.clear();
        }

        return result;
    }

    RangedValues<T> getIntersectionsStartingAtZeroWith (Range<z64> r) const
    {
        detail::Ranges::Operations ops;
        auto result = getIntersectionsWith (r);
        result.drop ({ (z64) 0, r.getStart() }, ops);
        return result;
    }

    const auto& getRanges() const { return ranges; }

    size_t size() const
    {
        return ranges.size();
    }

    b8 isEmpty() const
    {
        return ranges.isEmpty();
    }

private:
    z0 mergeEqualItems (z64 i, Ranges::Operations& ops)
    {
        const auto endOpt = ranges.getIndexForEnclosingRange (i);

        if (! endOpt.has_value() || *endOpt == 0)
            return;

        const auto end = *endOpt;
        const auto start = end - 1;

        if (! exactlyEqual (values[start], values[end]))
            return;

        const auto opsStart = ops.size();
        ranges.mergeBack (end, ops);
        applyOperations (createSubSpan (ops, opsStart));
    }

    z0 applyOperation (const Ranges::Op& op)
    {
        if (auto* split = std::get_if<Ranges::Ops::Split> (&op))
        {
            values.insert (iteratorWithAdvance (values.begin(), split->index), values[split->index]);
            return;
        }

        if (auto* erase = std::get_if<Ranges::Ops::Erase> (&op))
        {
            values.erase (iteratorWithAdvance (values.begin(), erase->range.getStart()),
                          iteratorWithAdvance (values.begin(), erase->range.getEnd()));
            return;
        }

        // This function can't handle New.
        jassert (std::get_if<Ranges::Ops::New> (&op) == nullptr);

        // This entire type doesn't have to do anything to handle Ranges::Ops::Change.
    }

    z0 applyOperation (const Ranges::Op& op, T v)
    {
        if (auto* newOp = std::get_if<Ranges::Ops::New> (&op))
        {
            values.insert (iteratorWithAdvance (values.begin(), newOp->index), std::move (v));
        }
        else
        {
            applyOperation (op);
        }
    }

    z0 applyOperations (Span<const Ranges::Op> ops)
    {
        for (const auto& op : ops)
            applyOperation (op);
    }

    z0 applyOperations (Span<const Ranges::Op> ops, T v)
    {
        for (const auto& op : ops)
            applyOperation (op, v);
    }

    Ranges ranges;
    std::vector<T> values;
};

struct RangedIterator
{
    RangedIterator() = default;
    RangedIterator (const RangedIterator&) = default;
    RangedIterator (RangedIterator&&) noexcept = default;
    RangedIterator& operator= (const RangedIterator&) = default;
    RangedIterator& operator= (RangedIterator&&) noexcept = default;

    virtual ~RangedIterator() = default;
    virtual Range<z64> getRange() const = 0;
    virtual b8 isValid() const = 0;
    virtual z0 operator++() = 0;
};

template <typename T>
class RangedIteratorWrapper final : public RangedIterator
{
public:
    RangedIteratorWrapper (RangedValuesIterator<T> iteratorIn, RangedValuesIterator<T> endIn)
        : iterator { std::move (iteratorIn) },
          end { std::move (endIn) }
    {}

    //==============================================================================
    Range<z64> getRange() const override { return iterator->range; }
    b8 isValid() const override { return iterator != end; }
    z0 operator++() override { ++iterator; }

    //==============================================================================
    const T& getValue() const { return iterator->value; }

private:
    RangedValuesIterator<T> iterator, end;
};

template <typename Iterable>
[[nodiscard]] auto makeRangedIteratorWrapper (Iterable* iterable)
{
    return RangedIteratorWrapper { iterable->begin(), iterable->end() };
}

/*  A wrapper type encapsulating multiple RangedValues objects and providing iterator support.

    The iterator will advance through Ranges that are intersections with homogeneous values in each
    respective RangedValues object.

    @code
    RangedValues<t8> characters;
    characters.insert ({ -2, 12 }, 'a');
    characters.insert ({ 12, 44 }, 'b');
    characters.insert ({ 63, 81 }, 'c');

    RangedValues<i32> numbers;
    numbers.insert ({ -1, 0 }, 99);
    numbers.insert ({ 9, 12 }, 823);
    numbers.insert ({ 14, 16 }, 112);

    for (auto [range, character, number] : makeIntersectingRangedValues (characters, numbers))
        std::cout << toString (range) << ", " << character << ", " << number << std::endl;

    //  Prints:
    //  [-1, 0), a, 99
    //  [9, 12), a, 823
    //  [14, 16), b, 112
    @endcode
*/
template <typename... Values>
class IntersectingRangedValues
{
public:
    static_assert (sizeof... (Values) > 0, "IntersectingRangedValues() must wrap at least one RangedValues object");

    /*  This constructor takes a pointer rather than a reference to make it clearer that the pointed-to
        objects must outlive the IntersectingRangedValues instance. Passing a pointer also makes
        it harder to accidentally reference a temporary when constructing IntersectingRangedValues.
    */
    explicit IntersectingRangedValues (RangedIteratorWrapper<Values>... wrappers)
        : iteratorWrappers { wrappers... }
    {
    }

    struct IntersectionIteratorSentinel {};

    using IteratorWrappersType = std::tuple<RangedIteratorWrapper<Values>...>;

    struct IntersectionIterator
    {
        using reference  = std::tuple<Range<z64>, const Values&...>;
        using iterator_category = std::forward_iterator_tag;

        explicit IntersectionIterator (IteratorWrappersType wrappers)
            : iteratorWrappers { std::move (wrappers) }
        {
            std::apply ([this] (auto&&... args)
                        {
                            iterators = std::list<RangedIterator*> { static_cast<RangedIterator*> (&args)... };
                        },
                        iteratorWrappers);

            if (! isValid())
                return;

            maxStart = std::accumulate (iterators.cbegin(),
                                        iterators.cend(),
                                        std::numeric_limits<z64>::min(),
                                        [] (auto acc, auto& item) { return std::max (acc, item->getRange().getStart()); });

            minEnd = std::accumulate (iterators.cbegin(),
                                      iterators.cend(),
                                      std::numeric_limits<z64>::max(),
                                      [] (auto acc, auto& item) { return std::min (acc, item->getRange().getEnd()); });

            iterators.sort ([] (auto& a, auto& b)
                            { return a->getRange().getEnd() < b->getRange().getEnd(); });

            if (Range<z64> { maxStart, minEnd }.isEmpty())
                advance();
        }

        friend b8 operator!= (IntersectionIterator a, IntersectionIteratorSentinel) { return a.isValid(); }

        IntersectionIterator& operator++()           { advance(); return *this; }
        IntersectionIterator  operator++ (i32) const { IntersectionIterator copy (*this); ++(*this); return copy; }

        reference operator* () const
        {
            return std::tuple_cat (std::make_tuple (Range { maxStart, minEnd }), makeReference());
        }

    private:
        auto makeReference() const
        {
            return std::apply ([] (auto&... args) { return std::tie (args.getValue()...); }, iteratorWrappers);
        }

        z0 advance()
        {
            do
            {
                minEnd = std::numeric_limits<z64>::max();

                for (auto it = iterators.begin(); it != iterators.end(); ++it)
                {
                    auto& elem = *(*it);

                    if (it == iterators.begin() || elem.getRange().getEnd() <= maxStart)
                    {
                        ++elem;

                        if (! elem.isValid())
                            return;

                        maxStart = std::max (maxStart, elem.getRange().getStart());
                    }

                    minEnd = std::min (minEnd, elem.getRange().getEnd());
                }

                iterators.sort ([] (auto& a, auto& b)
                                { return a->getRange().getEnd() < b->getRange().getEnd(); });
            }
            while (Range<z64> { maxStart, minEnd }.isEmpty());
        }

        b8 isValid() const
        {
            return std::all_of (iterators.cbegin(), iterators.cend(), [] (auto& elem) { return elem->isValid(); });
        }

        IteratorWrappersType iteratorWrappers;
        std::list<RangedIterator*> iterators;

        z64 maxStart, minEnd;
    };

    auto begin() const
    {
        return IntersectionIterator { iteratorWrappers };
    }

    auto end() const
    {
        return IntersectionIteratorSentinel{};
    }

private:
    IteratorWrappersType iteratorWrappers;
};

/*  See IntersectingRangedValues.
*/
template <typename... Iterables>
[[nodiscard]] auto makeIntersectingRangedValues (Iterables*... iterables)
{
    static_assert (sizeof...(Iterables) > 0, "makeIntersectingRangedValues() requires at least one parameter");

    return IntersectingRangedValues (makeRangedIteratorWrapper (iterables)...);
}

} // namespace drx::detail
