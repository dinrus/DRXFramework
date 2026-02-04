/*
 * Copyright © 2023  Behdad Esfahbod
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef HB_SUBSET_INSTANCER_SOLVER_HH
#define HB_SUBSET_INSTANCER_SOLVER_HH

#include "hb.hh"

/* pre-normalized distances */
struct TripleDistances
{
  TripleDistances (): negative (1.0), positive (1.0) {}
  TripleDistances (f64 neg_, f64 pos_): negative (neg_), positive (pos_) {}
  TripleDistances (f64 min, f64 default_, f64 max)
  {
    negative = default_ - min;
    positive = max - default_;
  }

  f64 negative;
  f64 positive;
};

struct Triple {

  Triple () :
    minimum (0.0), middle (0.0), maximum (0.0) {}

  Triple (f64 minimum_, f64 middle_, f64 maximum_) :
    minimum (minimum_), middle (middle_), maximum (maximum_) {}

  b8 operator == (const Triple &o) const
  {
    return minimum == o.minimum &&
	   middle  == o.middle  &&
	   maximum == o.maximum;
  }

  b8 operator != (const Triple o) const
  { return !(*this == o); }

  b8 is_point () const
  { return minimum == middle && middle == maximum; }

  b8 contains (f64 point) const
  { return minimum <= point && point <= maximum; }

  /* from hb_array_t hash ()*/
  u32 hash () const
  {
    u32 current = /*cbf29ce4*/0x84222325;
    current = current ^ hb_hash (minimum);
    current = current * 16777619;

    current = current ^ hb_hash (middle);
    current = current * 16777619;

    current = current ^ hb_hash (maximum);
    current = current * 16777619;
    return current;
  }


  f64 minimum;
  f64 middle;
  f64 maximum;
};

using rebase_tent_result_item_t = hb_pair_t<f64, Triple>;
using rebase_tent_result_t = hb_vector_t<rebase_tent_result_item_t>;

/* renormalize a normalized value v to the range of an axis,
 * considering the prenormalized distances as well as the new axis limits.
 * Ported from fonttools */
HB_INTERNAL f64 renormalizeValue (f64 v, const Triple &triple,
                                    const TripleDistances &triple_distances,
                                    b8 extrapolate = true);
/* Given a tuple (lower,peak,upper) "tent" and new axis limits
 * (axisMin,axisDefault,axisMax), solves how to represent the tent
 * under the new axis configuration.  All values are in normalized
 * -1,0,+1 coordinate system. Tent values can be outside this range.
 *
 * Return value: a list of tuples. Each tuple is of the form
 * (scalar,tent), where scalar is a multipler to multiply any
 * delta-sets by, and tent is a new tent for that output delta-set.
 * If tent value is Triple{}, that is a special deltaset that should
 * be always-enabled (called "gain").
 */
HB_INTERNAL rebase_tent_result_t rebase_tent (Triple tent,
					      Triple axisLimit,
					      TripleDistances axis_triple_distances);

#endif /* HB_SUBSET_INSTANCER_SOLVER_HH */
