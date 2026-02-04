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

#ifndef HB_BIT_PAGE_HH
#define HB_BIT_PAGE_HH

#include "hb.hh"


/* Compiler-assisted vectorization. */

/* Type behaving similar to vectorized vars defined using __attribute__((vector_size(...))),
 * basically a fixed-size bitset. We can't use the compiler type because hb_vector_t cannot
 * guarantee alignment requirements. */
template <typename elt_t, u32 byte_size>
struct hb_vector_size_t
{
  elt_t& operator [] (u32 i) { return v[i]; }
  const elt_t& operator [] (u32 i) const { return v[i]; }

  z0 init0 ()
  {
    for (u32 i = 0; i < ARRAY_LENGTH (v); i++)
      v[i] = 0;
  }
  z0 init1 ()
  {
    for (u32 i = 0; i < ARRAY_LENGTH (v); i++)
      v[i] = (elt_t) -1;
  }

  template <typename Op>
  hb_vector_size_t process (const Op& op) const
  {
    hb_vector_size_t r;
    for (u32 i = 0; i < ARRAY_LENGTH (v); i++)
      r.v[i] = op (v[i]);
    return r;
  }
  template <typename Op>
  hb_vector_size_t process (const Op& op, const hb_vector_size_t &o) const
  {
    hb_vector_size_t r;
    for (u32 i = 0; i < ARRAY_LENGTH (v); i++)
      r.v[i] = op (v[i], o.v[i]);
    return r;
  }
  hb_vector_size_t operator | (const hb_vector_size_t &o) const
  { return process (hb_bitwise_or, o); }
  hb_vector_size_t operator & (const hb_vector_size_t &o) const
  { return process (hb_bitwise_and, o); }
  hb_vector_size_t operator ^ (const hb_vector_size_t &o) const
  { return process (hb_bitwise_xor, o); }
  hb_vector_size_t operator ~ () const
  { return process (hb_bitwise_neg); }

  hb_array_t<const elt_t> iter () const
  { return hb_array (v); }

  private:
  static_assert (0 == byte_size % sizeof (elt_t), "");
  elt_t v[byte_size / sizeof (elt_t)];
};


struct hb_bit_page_t
{
  z0 init0 () { v.init0 (); population = 0; }
  z0 init1 () { v.init1 (); population = PAGE_BITS; }

  z0 dirty () { population = UINT_MAX; }

  static inline constexpr u32 len ()
  { return ARRAY_LENGTH_CONST (v); }

  operator b8 () const { return !is_empty (); }
  b8 is_empty () const
  {
    if (has_population ()) return !population;
    return
    + hb_iter (v)
    | hb_none
    ;
  }
  u32 hash () const
  {
    return hb_bytes_t ((const t8 *) &v, sizeof (v)).hash ();
  }

  z0 add (hb_codepoint_t g) { elt (g) |= mask (g); dirty (); }
  z0 del (hb_codepoint_t g) { elt (g) &= ~mask (g); dirty (); }
  z0 set (hb_codepoint_t g, b8 value) { if (value) add (g); else del (g); }
  b8 get (hb_codepoint_t g) const { return elt (g) & mask (g); }

  z0 add_range (hb_codepoint_t a, hb_codepoint_t b)
  {
    elt_t *la = &elt (a);
    elt_t *lb = &elt (b);
    if (la == lb)
      *la |= (mask (b) << 1) - mask(a);
    else
    {
      *la |= ~(mask (a) - 1llu);
      la++;

      hb_memset (la, 0xff, (t8 *) lb - (t8 *) la);

      *lb |= ((mask (b) << 1) - 1llu);
    }
    dirty ();
  }
  z0 del_range (hb_codepoint_t a, hb_codepoint_t b)
  {
    elt_t *la = &elt (a);
    elt_t *lb = &elt (b);
    if (la == lb)
      *la &= ~((mask (b) << 1llu) - mask(a));
    else
    {
      *la &= mask (a) - 1;
      la++;

      hb_memset (la, 0, (t8 *) lb - (t8 *) la);

      *lb &= ~((mask (b) << 1) - 1llu);
    }
    dirty ();
  }
  z0 set_range (hb_codepoint_t a, hb_codepoint_t b, b8 v)
  { if (v) add_range (a, b); else del_range (a, b); }


  // Writes out page values to the array p. Returns the number of values
  // written. At most size codepoints will be written.
  u32 write (u32        base,
		      u32    start_value,
		      hb_codepoint_t *p,
		      u32    size) const
  {
    u32 start_v = start_value / ELT_BITS;
    u32 start_bit = start_value & ELT_MASK;
    u32 count = 0;
    for (u32 i = start_v; i < len () && count < size; i++)
    {
      elt_t bits = v[i];
      u32 v_base = base | (i * ELT_BITS);
      for (u32 j = start_bit; j < ELT_BITS && count < size; j++)
      {
	if ((elt_t(1) << j) & bits) {
	  *p++ = v_base | j;
	  count++;
	}
      }
      start_bit = 0;
    }
    return count;
  }

  // Writes out the values NOT in this page to the array p. Returns the
  // number of values written. At most size codepoints will be written.
  // Returns the number of codepoints written. next_value holds the next value
  // that should be written (if not present in this page). This is used to fill
  // any missing value gaps between this page and the previous page, if any.
  // next_value is updated to one more than the last value present in this page.
  u32 write_inverted (u32        base,
			       u32    start_value,
			       hb_codepoint_t *p,
			       u32    size,
			       hb_codepoint_t *next_value) const
  {
    u32 start_v = start_value / ELT_BITS;
    u32 start_bit = start_value & ELT_MASK;
    u32 count = 0;
    for (u32 i = start_v; i < len () && count < size; i++)
    {
      elt_t bits = v[i];
      u32 v_offset = i * ELT_BITS;
      for (u32 j = start_bit; j < ELT_BITS && count < size; j++)
      {
	if ((elt_t(1) << j) & bits)
	{
	  hb_codepoint_t value = base | v_offset | j;
	  // Emit all the missing values from next_value up to value - 1.
	  for (hb_codepoint_t k = *next_value; k < value && count < size; k++)
	  {
	    *p++ = k;
	    count++;
	  }
	  // Skip over this value;
	  *next_value = value + 1;
	}
      }
      start_bit = 0;
    }
    return count;
  }

  b8 operator == (const hb_bit_page_t &other) const { return is_equal (other); }
  b8 is_equal (const hb_bit_page_t &other) const
  {
    for (u32 i = 0; i < len (); i++)
      if (v[i] != other.v[i])
	return false;
    return true;
  }
  b8 operator <= (const hb_bit_page_t &larger_page) const { return is_subset (larger_page); }
  b8 is_subset (const hb_bit_page_t &larger_page) const
  {
    if (has_population () && larger_page.has_population () &&
	population > larger_page.population)
      return false;

    for (u32 i = 0; i < len (); i++)
      if (~larger_page.v[i] & v[i])
	return false;
    return true;
  }

  b8 has_population () const { return population != UINT_MAX; }
  u32 get_population () const
  {
    if (has_population ()) return population;
    population =
    + hb_iter (v)
    | hb_reduce ([] (u32 pop, const elt_t &_) { return pop + hb_popcount (_); }, 0u)
    ;
    return population;
  }

  b8 next (hb_codepoint_t *codepoint) const
  {
    u32 m = (*codepoint + 1) & MASK;
    if (!m)
    {
      *codepoint = INVALID;
      return false;
    }
    u32 i = m / ELT_BITS;
    u32 j = m & ELT_MASK;

    const elt_t vv = v[i] & ~((elt_t (1) << j) - 1);
    for (const elt_t *p = &vv; i < len (); p = &v[++i])
      if (*p)
      {
	*codepoint = i * ELT_BITS + elt_get_min (*p);
	return true;
      }

    *codepoint = INVALID;
    return false;
  }
  b8 previous (hb_codepoint_t *codepoint) const
  {
    u32 m = (*codepoint - 1) & MASK;
    if (m == MASK)
    {
      *codepoint = INVALID;
      return false;
    }
    u32 i = m / ELT_BITS;
    u32 j = m & ELT_MASK;

    /* Fancy mask to avoid shifting by elt_t bitsize, which is undefined. */
    const elt_t mask = j < 8 * sizeof (elt_t) - 1 ?
		       ((elt_t (1) << (j + 1)) - 1) :
		       (elt_t) -1;
    const elt_t vv = v[i] & mask;
    const elt_t *p = &vv;
    while (true)
    {
      if (*p)
      {
	*codepoint = i * ELT_BITS + elt_get_max (*p);
	return true;
      }
      if ((i32) i <= 0) break;
      p = &v[--i];
    }

    *codepoint = INVALID;
    return false;
  }
  hb_codepoint_t get_min () const
  {
    for (u32 i = 0; i < len (); i++)
      if (v[i])
	return i * ELT_BITS + elt_get_min (v[i]);
    return INVALID;
  }
  hb_codepoint_t get_max () const
  {
    for (i32 i = len () - 1; i >= 0; i--)
      if (v[i])
	return i * ELT_BITS + elt_get_max (v[i]);
    return 0;
  }

  static constexpr hb_codepoint_t INVALID = HB_SET_VALUE_INVALID;

  typedef zu64 elt_t;
  static constexpr u32 PAGE_BITS_LOG_2 = 9; // 512 bits
  static constexpr u32 PAGE_BITS = 1 << PAGE_BITS_LOG_2;
  static_assert (1 << PAGE_BITS_LOG_2 == PAGE_BITS, "");
  static_assert ((PAGE_BITS & ((PAGE_BITS) - 1)) == 0, "");
  static constexpr u32 PAGE_BITMASK = PAGE_BITS - 1;

  static u32 elt_get_min (const elt_t &elt) { return hb_ctz (elt); }
  static u32 elt_get_max (const elt_t &elt) { return hb_bit_storage (elt) - 1; }

  typedef hb_vector_size_t<elt_t, PAGE_BITS / 8> vector_t;

  static constexpr u32 ELT_BITS = sizeof (elt_t) * 8;
  static constexpr u32 ELT_MASK = ELT_BITS - 1;

  static constexpr u32 BITS = sizeof (vector_t) * 8;
  static constexpr u32 MASK = BITS - 1;
  static_assert ((u32) PAGE_BITS == (u32) BITS, "");

  elt_t &elt (hb_codepoint_t g) { return v[(g & MASK) / ELT_BITS]; }
  const elt_t& elt (hb_codepoint_t g) const { return v[(g & MASK) / ELT_BITS]; }
  static constexpr elt_t mask (hb_codepoint_t g) { return elt_t (1) << (g & ELT_MASK); }

  mutable u32 population;
  vector_t v;
};


#endif /* HB_BIT_PAGE_HH */
