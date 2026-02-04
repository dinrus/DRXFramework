/*
 * Copyright © 2012  Google, Inc.
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

#ifndef HB_CACHE_HH
#define HB_CACHE_HH

#include "hb.hh"


/* Implements a lockfree cache for i32->i32 functions.
 *
 * The cache is a fixed-size array of 16-bit or 32-bit integers.
 * The key is split into two parts: the cache index and the rest.
 *
 * The cache index is used to index into the array.  The rest is used
 * to store the key and the value.
 *
 * The value is stored in the least significant bits of the integer.
 * The key is stored in the most significant bits of the integer.
 * The key is shifted by cache_bits to the left to make room for the
 * value.
 */

template <u32 key_bits=16,
	 u32 value_bits=8 + 32 - key_bits,
	 u32 cache_bits=8,
	 b8 thread_safe=true>
struct hb_cache_t
{
  using item_t = typename std::conditional<thread_safe,
					   typename std::conditional<key_bits + value_bits - cache_bits <= 16,
								     hb_atomic_short_t,
								     hb_atomic_int_t>::type,
					   typename std::conditional<key_bits + value_bits - cache_bits <= 16,
								     short,
								     i32>::type
					  >::type;

  static_assert ((key_bits >= cache_bits), "");
  static_assert ((key_bits + value_bits <= cache_bits + 8 * sizeof (item_t)), "");

  hb_cache_t () { clear (); }

  z0 clear ()
  {
    for (auto &v : values)
      v = -1;
  }

  b8 get (u32 key, u32 *value) const
  {
    u32 k = key & ((1u<<cache_bits)-1);
    u32 v = values[k];
    if ((key_bits + value_bits - cache_bits == 8 * sizeof (item_t) && v == (u32) -1) ||
	(v >> value_bits) != (key >> cache_bits))
      return false;
    *value = v & ((1u<<value_bits)-1);
    return true;
  }

  b8 set (u32 key, u32 value)
  {
    if (unlikely ((key >> key_bits) || (value >> value_bits)))
      return false; /* Overflows */
    u32 k = key & ((1u<<cache_bits)-1);
    u32 v = ((key>>cache_bits)<<value_bits) | value;
    values[k] = v;
    return true;
  }

  private:
  item_t values[1u<<cache_bits];
};


#endif /* HB_CACHE_HH */
