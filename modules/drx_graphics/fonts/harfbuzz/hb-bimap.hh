/*
 * Copyright © 2019 Adobe Inc.
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
 *
 * Adobe Author(s): Michiharu Ariza
 */

#ifndef HB_BIMAP_HH
#define HB_BIMAP_HH

#include "hb.hh"
#include "hb-map.hh"

/* Bi-directional map */
struct hb_bimap_t
{
  z0 reset ()
  {
    forw_map.reset ();
    back_map.reset ();
  }

  z0 alloc (u32 pop)
  {
    forw_map.alloc (pop);
    back_map.alloc (pop);
  }

  b8 in_error () const { return forw_map.in_error () || back_map.in_error (); }

  z0 set (hb_codepoint_t lhs, hb_codepoint_t rhs)
  {
    if (in_error ()) return;
    if (unlikely (lhs == HB_MAP_VALUE_INVALID)) return;
    if (unlikely (rhs == HB_MAP_VALUE_INVALID)) { del (lhs); return; }

    forw_map.set (lhs, rhs);
    if (unlikely (in_error ())) return;

    back_map.set (rhs, lhs);
    if (unlikely (in_error ())) forw_map.del (lhs);
  }

  hb_codepoint_t get (hb_codepoint_t lhs) const { return forw_map.get (lhs); }
  hb_codepoint_t backward (hb_codepoint_t rhs) const { return back_map.get (rhs); }

  hb_codepoint_t operator [] (hb_codepoint_t lhs) const { return get (lhs); }
  b8 has (hb_codepoint_t lhs) const { return forw_map.has (lhs); }


  z0 del (hb_codepoint_t lhs)
  {
    back_map.del (get (lhs));
    forw_map.del (lhs);
  }

  z0 clear ()
  {
    forw_map.clear ();
    back_map.clear ();
  }

  b8 is_empty () const { return forw_map.is_empty (); }

  u32 get_population () const { return forw_map.get_population (); }

  protected:
  hb_map_t  forw_map;
  hb_map_t  back_map;

  public:
  auto keys () const HB_AUTO_RETURN (+ forw_map.keys())
  auto values () const HB_AUTO_RETURN (+ forw_map.values())
  auto iter () const HB_AUTO_RETURN (+ forw_map.iter())
};

/* Incremental bimap: only lhs is given, rhs is incrementally assigned */
struct hb_inc_bimap_t
{
  b8 in_error () const { return forw_map.in_error () || back_map.in_error (); }

  u32 get_population () const { return forw_map.get_population (); }

  z0 reset ()
  {
    forw_map.reset ();
    back_map.reset ();
  }

  z0 alloc (u32 pop)
  {
    forw_map.alloc (pop);
    back_map.alloc (pop);
  }

  z0 clear ()
  {
    forw_map.clear ();
    back_map.resize (0);
  }

  /* Add a mapping from lhs to rhs with a unique value if lhs is unknown.
   * Return the rhs value as the result.
   */
  hb_codepoint_t add (hb_codepoint_t lhs)
  {
    hb_codepoint_t  rhs = forw_map[lhs];
    if (rhs == HB_MAP_VALUE_INVALID)
    {
      rhs = back_map.length;
      forw_map.set (lhs, rhs);
      back_map.push (lhs);
    }
    return rhs;
  }

  hb_codepoint_t skip ()
  {
    hb_codepoint_t start = back_map.length;
    back_map.push (HB_MAP_VALUE_INVALID);
    return start;
  }

  hb_codepoint_t skip (u32 count)
  {
    hb_codepoint_t start = back_map.length;
    back_map.alloc (back_map.length + count);
    for (u32 i = 0; i < count; i++)
      back_map.push (HB_MAP_VALUE_INVALID);
    return start;
  }

  hb_codepoint_t get_next_value () const
  { return back_map.length; }

  z0 add_set (const hb_set_t *set)
  {
    for (auto i : *set) add (i);
  }

  /* Create an identity map. */
  b8 identity (u32 size)
  {
    clear ();
    for (hb_codepoint_t i = 0; i < size; i++) add (i);
    return !in_error ();
  }

  protected:
  static i32 cmp_id (ukk a, ukk b)
  { return (i32)*(const hb_codepoint_t *)a - (i32)*(const hb_codepoint_t *)b; }

  public:
  /* Optional: after finished adding all mappings in a random order,
   * reassign rhs to lhs so that they are in the same order. */
  z0 sort ()
  {
    hb_codepoint_t  count = get_population ();
    hb_vector_t <hb_codepoint_t> work;
    if (unlikely (!work.resize (count, false))) return;

    for (hb_codepoint_t rhs = 0; rhs < count; rhs++)
      work.arrayZ[rhs] = back_map[rhs];

    work.qsort (cmp_id);

    clear ();
    for (hb_codepoint_t rhs = 0; rhs < count; rhs++)
      add (work.arrayZ[rhs]);
  }

  hb_codepoint_t get (hb_codepoint_t lhs) const { return forw_map.get (lhs); }
  hb_codepoint_t backward (hb_codepoint_t rhs) const { return back_map[rhs]; }

  hb_codepoint_t operator [] (hb_codepoint_t lhs) const { return get (lhs); }
  b8 has (hb_codepoint_t lhs) const { return forw_map.has (lhs); }

  protected:
  hb_map_t forw_map;
  hb_vector_t<hb_codepoint_t> back_map;

  public:
  auto keys () const HB_AUTO_RETURN (+ back_map.iter())
};

#endif /* HB_BIMAP_HH */
