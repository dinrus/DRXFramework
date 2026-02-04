/*
 * Copyright © 2012,2017  Google, Inc.
 * Copyright © 2021 Behdad Esfahbod
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
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_BIT_SET_INVERTIBLE_HH
#define HB_BIT_SET_INVERTIBLE_HH

#include "hb.hh"
#include "hb-bit-set.hh"


struct hb_bit_set_invertible_t
{
  hb_bit_set_t s;
  b8 inverted = false;

  hb_bit_set_invertible_t () = default;
  hb_bit_set_invertible_t (const hb_bit_set_invertible_t& o) = default;
  hb_bit_set_invertible_t (hb_bit_set_invertible_t&& other)  noexcept : hb_bit_set_invertible_t () { hb_swap (*this, other); }
  hb_bit_set_invertible_t& operator= (const hb_bit_set_invertible_t& o) = default;
  hb_bit_set_invertible_t& operator= (hb_bit_set_invertible_t&& other)  noexcept { hb_swap (*this, other); return *this; }
  friend z0 swap (hb_bit_set_invertible_t &a, hb_bit_set_invertible_t &b) noexcept
  {
    if (likely (!a.s.successful || !b.s.successful))
      return;
    hb_swap (a.inverted, b.inverted);
    hb_swap (a.s, b.s);
  }

  z0 init () { s.init (); inverted = false; }
  z0 fini () { s.fini (); }
  z0 err () { s.err (); }
  b8 in_error () const { return s.in_error (); }
  explicit operator b8 () const { return !is_empty (); }

  z0 alloc (u32 sz) { s.alloc (sz); }
  z0 reset ()
  {
    s.reset ();
    inverted = false;
  }
  z0 clear ()
  {
    s.clear ();
    if (likely (s.successful))
      inverted = false;
  }
  z0 invert ()
  {
    if (likely (s.successful))
      inverted = !inverted;
  }

  b8 is_inverted () const
  {
    return inverted;
  }

  b8 is_empty () const
  {
    hb_codepoint_t v = INVALID;
    next (&v);
    return v == INVALID;
  }
  u32 hash () const { return s.hash () ^ (u32) inverted; }

  hb_codepoint_t get_min () const
  {
    hb_codepoint_t v = INVALID;
    next (&v);
    return v;
  }
  hb_codepoint_t get_max () const
  {
    hb_codepoint_t v = INVALID;
    previous (&v);
    return v;
  }
  u32 get_population () const
  { return inverted ? INVALID - s.get_population () : s.get_population (); }


  z0 add (hb_codepoint_t g) { unlikely (inverted) ? s.del (g) : s.add (g); }
  b8 add_range (hb_codepoint_t a, hb_codepoint_t b)
  { return unlikely (inverted) ? ((z0) s.del_range (a, b), true) : s.add_range (a, b); }

  template <typename T>
  z0 add_array (const T *array, u32 count, u32 stride=sizeof(T))
  { inverted ? s.del_array (array, count, stride) : s.add_array (array, count, stride); }
  template <typename T>
  z0 add_array (const hb_array_t<const T>& arr) { add_array (&arr, arr.len ()); }

  /* Might return false if array looks unsorted.
   * Used for faster rejection of corrupt data. */
  template <typename T>
  b8 add_sorted_array (const T *array, u32 count, u32 stride=sizeof(T))
  { return inverted ? s.del_sorted_array (array, count, stride) : s.add_sorted_array (array, count, stride); }
  template <typename T>
  b8 add_sorted_array (const hb_sorted_array_t<const T>& arr) { return add_sorted_array (&arr, arr.len ()); }

  z0 del (hb_codepoint_t g) { unlikely (inverted) ? s.add (g) : s.del (g); }
  z0 del_range (hb_codepoint_t a, hb_codepoint_t b)
  { unlikely (inverted) ? (z0) s.add_range (a, b) : s.del_range (a, b); }

  b8 get (hb_codepoint_t g) const { return s.get (g) ^ inverted; }

  /* Has interface. */
  b8 operator [] (hb_codepoint_t k) const { return get (k); }
  b8 has (hb_codepoint_t k) const { return (*this)[k]; }
  /* Predicate. */
  b8 operator () (hb_codepoint_t k) const { return has (k); }

  /* Sink interface. */
  hb_bit_set_invertible_t& operator << (hb_codepoint_t v)
  { add (v); return *this; }
  hb_bit_set_invertible_t& operator << (const hb_codepoint_pair_t& range)
  { add_range (range.first, range.second); return *this; }

  b8 intersects (hb_codepoint_t first, hb_codepoint_t last) const
  {
    hb_codepoint_t c = first - 1;
    return next (&c) && c <= last;
  }

  z0 set (const hb_bit_set_invertible_t &other)
  {
    s.set (other.s);
    if (likely (s.successful))
      inverted = other.inverted;
  }

  b8 is_equal (const hb_bit_set_invertible_t &other) const
  {
    if (likely (inverted == other.inverted))
      return s.is_equal (other.s);
    else
    {
      /* TODO Add iter_ranges() and use here. */
      auto it1 = iter ();
      auto it2 = other.iter ();
      return hb_all (+ hb_zip (it1, it2)
		     | hb_map ([](hb_codepoint_pair_t _) { return _.first == _.second; }));
    }
  }

  b8 is_subset (const hb_bit_set_invertible_t &larger_set) const
  {
    if (unlikely (inverted != larger_set.inverted))
      return hb_all (hb_iter (s) | hb_map (larger_set.s));
    else
      return unlikely (inverted) ? larger_set.s.is_subset (s) : s.is_subset (larger_set.s);
  }

  protected:
  template <typename Op>
  z0 process (const Op& op, const hb_bit_set_invertible_t &other)
  { s.process (op, other.s); }
  public:
  z0 union_ (const hb_bit_set_invertible_t &other)
  {
    if (likely (inverted == other.inverted))
    {
      if (unlikely (inverted))
	process (hb_bitwise_and, other);
      else
	process (hb_bitwise_or, other); /* Main branch. */
    }
    else
    {
      if (unlikely (inverted))
	process (hb_bitwise_gt, other);
      else
	process (hb_bitwise_lt, other);
    }
    if (likely (s.successful))
      inverted = inverted || other.inverted;
  }
  z0 intersect (const hb_bit_set_invertible_t &other)
  {
    if (likely (inverted == other.inverted))
    {
      if (unlikely (inverted))
	process (hb_bitwise_or, other);
      else
	process (hb_bitwise_and, other); /* Main branch. */
    }
    else
    {
      if (unlikely (inverted))
	process (hb_bitwise_lt, other);
      else
	process (hb_bitwise_gt, other);
    }
    if (likely (s.successful))
      inverted = inverted && other.inverted;
  }
  z0 subtract (const hb_bit_set_invertible_t &other)
  {
    if (likely (inverted == other.inverted))
    {
      if (unlikely (inverted))
	process (hb_bitwise_lt, other);
      else
	process (hb_bitwise_gt, other); /* Main branch. */
    }
    else
    {
      if (unlikely (inverted))
	process (hb_bitwise_or, other);
      else
	process (hb_bitwise_and, other);
    }
    if (likely (s.successful))
      inverted = inverted && !other.inverted;
  }
  z0 symmetric_difference (const hb_bit_set_invertible_t &other)
  {
    process (hb_bitwise_xor, other);
    if (likely (s.successful))
      inverted = inverted ^ other.inverted;
  }

  b8 next (hb_codepoint_t *codepoint) const
  {
    if (likely (!inverted))
      return s.next (codepoint);

    auto old = *codepoint;
    if (unlikely (old + 1 == INVALID))
    {
      *codepoint = INVALID;
      return false;
    }

    auto v = old;
    s.next (&v);
    if (old + 1 < v)
    {
      *codepoint = old + 1;
      return true;
    }

    v = old;
    s.next_range (&old, &v);

    *codepoint = v + 1;
    return *codepoint != INVALID;
  }
  b8 previous (hb_codepoint_t *codepoint) const
  {
    if (likely (!inverted))
      return s.previous (codepoint);

    auto old = *codepoint;
    if (unlikely (old - 1 == INVALID))
    {
      *codepoint = INVALID;
      return false;
    }

    auto v = old;
    s.previous (&v);

    if (old - 1 > v || v == INVALID)
    {
      *codepoint = old - 1;
      return true;
    }

    v = old;
    s.previous_range (&v, &old);

    *codepoint = v - 1;
    return *codepoint != INVALID;
  }
  b8 next_range (hb_codepoint_t *first, hb_codepoint_t *last) const
  {
    if (likely (!inverted))
      return s.next_range (first, last);

    if (!next (last))
    {
      *last = *first = INVALID;
      return false;
    }

    *first = *last;
    s.next (last);
    --*last;
    return true;
  }
  b8 previous_range (hb_codepoint_t *first, hb_codepoint_t *last) const
  {
    if (likely (!inverted))
      return s.previous_range (first, last);

    if (!previous (first))
    {
      *last = *first = INVALID;
      return false;
    }

    *last = *first;
    s.previous (first);
    ++*first;
    return true;
  }

  u32 next_many (hb_codepoint_t  codepoint,
			  hb_codepoint_t *out,
			  u32    size) const
  {
    return inverted ? s.next_many_inverted (codepoint, out, size)
		    : s.next_many (codepoint, out, size);
  }

  static constexpr hb_codepoint_t INVALID = hb_bit_set_t::INVALID;

  /*
   * Iterator implementation.
   */
  struct iter_t : hb_iter_with_fallback_t<iter_t, hb_codepoint_t>
  {
    static constexpr b8 is_sorted_iterator = true;
    static constexpr b8 has_fast_len = true;
    iter_t (const hb_bit_set_invertible_t &s_ = Null (hb_bit_set_invertible_t),
	    b8 init = true) : s (&s_), v (INVALID), l(0)
    {
      if (init)
      {
	l = s->get_population () + 1;
	__next__ ();
      }
    }

    typedef hb_codepoint_t __item_t__;
    hb_codepoint_t __item__ () const { return v; }
    b8 __more__ () const { return v != INVALID; }
    z0 __next__ () { s->next (&v); if (likely (l)) l--; }
    z0 __prev__ () { s->previous (&v); l++; }
    u32 __len__ () const { return l; }
    iter_t end () const { return iter_t (*s, false); }
    b8 operator != (const iter_t& o) const
    { return v != o.v || s != o.s; }

    protected:
    const hb_bit_set_invertible_t *s;
    hb_codepoint_t v;
    u32 l;
  };
  iter_t iter () const { return iter_t (*this); }
  operator iter_t () const { return iter (); }
};


#endif /* HB_BIT_SET_INVERTIBLE_HH */
