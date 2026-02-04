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
    Holds a set of primitive values, storing them as a set of ranges.

    This container acts like an array, but can efficiently hold large contiguous
    ranges of values. It's quite a specialised class, mostly useful for things
    like keeping the set of selected rows in a listbox.

    The type used as a template parameter must be an integer type, such as i32, short,
    z64, etc.

    @tags{Core}
*/
template <class Type>
class SparseSet
{
public:
    //==============================================================================
    SparseSet() = default;

    SparseSet (const SparseSet&) = default;
    SparseSet& operator= (const SparseSet&) = default;

    SparseSet (SparseSet&& other) noexcept : ranges (std::move (other.ranges)) {}
    SparseSet& operator= (SparseSet&& other) noexcept { ranges = std::move (other.ranges); return *this; }

    //==============================================================================
    /** Clears the set. */
    z0 clear()                                { ranges.clear(); }

    /** Checks whether the set is empty.
        This is much quicker than using (size() == 0).
    */
    b8 isEmpty() const noexcept               { return ranges.isEmpty(); }

    /** Returns the number of values in the set.

        Because of the way the data is stored, this method can take longer if there
        are a lot of items in the set. Use isEmpty() for a quick test of whether there
        are any items.
    */
    Type size() const noexcept
    {
        Type total = {};

        for (auto& r : ranges)
            total += r.getLength();

        return total;
    }

    /** Returns one of the values in the set.

        @param index    the index of the value to retrieve, in the range 0 to (size() - 1).
        @returns        the value at this index, or 0 if it's out-of-range
    */
    Type operator[] (Type index) const noexcept
    {
        Type total = {};

        for (auto& r : ranges)
        {
            auto end = total + r.getLength();

            if (index < end)
                return r.getStart() + (index - total);

            total = end;
        }

        return {};
    }

    /** Checks whether a particular value is in the set. */
    b8 contains (Type valueToLookFor) const noexcept
    {
        for (auto& r : ranges)
        {
            if (r.getStart() > valueToLookFor)
                break;

            if (r.getEnd() > valueToLookFor)
                return true;
        }

        return false;
    }

    //==============================================================================
    /** Returns the number of contiguous blocks of values.
        @see getRange
    */
    i32 getNumRanges() const noexcept                           { return ranges.size(); }

    /** Returns one of the contiguous ranges of values stored.
        @param rangeIndex   the index of the range to look up, between 0
                            and (getNumRanges() - 1)
        @see getTotalRange
    */
    Range<Type> getRange (i32 rangeIndex) const noexcept        { return ranges[rangeIndex]; }

    /** Returns the range between the lowest and highest values in the set.
        @see getRange
    */
    Range<Type> getTotalRange() const noexcept
    {
        if (ranges.isEmpty())
            return {};

        return { ranges.getFirst().getStart(),
                 ranges.getLast().getEnd() };
    }

    //==============================================================================
    /** Adds a range of contiguous values to the set.
        e.g. addRange (Range \<i32\> (10, 14)) will add (10, 11, 12, 13) to the set.
    */
    z0 addRange (Range<Type> range)
    {
        if (! range.isEmpty())
        {
            removeRange (range);
            ranges.add (range);
            std::sort (ranges.begin(), ranges.end(),
                       [] (Range<Type> a, Range<Type> b) { return a.getStart() < b.getStart(); });
            simplify();
        }
    }

    /** Removes a range of values from the set.
        e.g. removeRange (Range\<i32\> (10, 14)) will remove (10, 11, 12, 13) from the set.
    */
    z0 removeRange (Range<Type> rangeToRemove)
    {
        if (getTotalRange().intersects (rangeToRemove) && ! rangeToRemove.isEmpty())
        {
            for (i32 i = ranges.size(); --i >= 0;)
            {
                auto& r = ranges.getReference (i);

                if (r.getEnd() <= rangeToRemove.getStart())
                    break;

                if (r.getStart() >= rangeToRemove.getEnd())
                    continue;

                if (rangeToRemove.contains (r))
                {
                    ranges.remove (i);
                }
                else if (r.contains (rangeToRemove))
                {
                    auto r1 = r.withEnd (rangeToRemove.getStart());
                    auto r2 = r.withStart (rangeToRemove.getEnd());

                    // this should be covered in if (rangeToRemove.contains (r))
                    jassert (! r1.isEmpty() || ! r2.isEmpty());

                    r = r1;

                    if (r.isEmpty())
                        r = r2;

                    if (! r1.isEmpty() && ! r2.isEmpty())
                        ranges.insert (i + 1, r2);
                }
                else if (rangeToRemove.getEnd() > r.getEnd())
                {
                    r.setEnd (rangeToRemove.getStart());
                }
                else
                {
                    r.setStart (rangeToRemove.getEnd());
                }
            }
        }
    }

    /** Does an XOR of the values in a given range. */
    z0 invertRange (Range<Type> range)
    {
        SparseSet newItems;
        newItems.addRange (range);

        for (auto& r : ranges)
            newItems.removeRange (r);

        removeRange (range);

        for (auto& r : newItems.ranges)
            addRange (r);
    }

    /** Checks whether any part of a given range overlaps any part of this set. */
    b8 overlapsRange (Range<Type> range) const noexcept
    {
        if (! range.isEmpty())
            for (auto& r : ranges)
                if (r.intersects (range))
                    return true;

        return false;
    }

    /** Checks whether the whole of a given range is contained within this one. */
    b8 containsRange (Range<Type> range) const noexcept
    {
        if (! range.isEmpty())
            for (auto& r : ranges)
                if (r.contains (range))
                    return true;

        return false;
    }

    /** Returns the set as a list of ranges, which you may want to iterate over. */
    const Array<Range<Type>>& getRanges() const noexcept        { return ranges; }

    //==============================================================================
    b8 operator== (const SparseSet& other) const noexcept     { return ranges == other.ranges; }
    b8 operator!= (const SparseSet& other) const noexcept     { return ranges != other.ranges; }

private:
    //==============================================================================
    Array<Range<Type>> ranges;

    z0 simplify()
    {
        for (i32 i = ranges.size(); --i > 0;)
        {
            auto& r1 = ranges.getReference (i - 1);
            auto& r2 = ranges.getReference (i);

            if (r1.getEnd() == r2.getStart())
            {
                r1.setEnd (r2.getEnd());
                ranges.remove (i);
            }
        }
    }
};

} // namespace drx
