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

#ifndef HB_SET_HH
#define HB_SET_HH

#include "hb.hh"
#include "hb-bit-set-invertible.hh"


template <typename impl_t>
struct hb_sparseset_t
{
  static constexpr b8 realloc_move = true;

  hb_object_header_t header;
  impl_t s;

  hb_sparseset_t () { init (); }
  ~hb_sparseset_t () { fini (); }

  hb_sparseset_t (const hb_sparseset_t& other) : hb_sparseset_t () { set (other); }
  hb_sparseset_t (hb_sparseset_t&& other)  noexcept : hb_sparseset_t () { s = std::move (other.s); }
  hb_sparseset_t& operator = (const hb_sparseset_t& other) { set (other); return *this; }
  hb_sparseset_t& operator = (hb_sparseset_t&& other)  noexcept { s = std::move (other.s); return *this; }
  friend z0 swap (hb_sparseset_t& a, hb_sparseset_t& b)  noexcept { hb_swap (a.s, b.s); }

  hb_sparseset_t (std::initializer_list<hb_codepoint_t> lst) : hb_sparseset_t ()
  {
    for (auto&& item : lst)
      add (item);
  }
  template <typename Iterable,
           hb_requires (hb_is_iterable (Iterable))>
  hb_sparseset_t (const Iterable &o) : hb_sparseset_t ()
  {
    hb_copy (o, *this);
  }

  z0 init ()
  {
    hb_object_init (this);
    s.init ();
  }
  z0 fini ()
  {
    hb_object_fini (this);
    s.fini ();
  }

  explicit operator b8 () const { return !is_empty (); }

  z0 err () { s.err (); }
  b8 in_error () const { return s.in_error (); }

  z0 alloc (u32 sz) { s.alloc (sz); }
  z0 reset () { s.reset (); }
  z0 clear () { s.clear (); }
  z0 invert () { s.invert (); }
  b8 is_inverted () const { return s.is_inverted (); }
  b8 is_empty () const { return s.is_empty (); }
  u32 hash () const { return s.hash (); }

  z0 add (hb_codepoint_t g) { s.add (g); }
  b8 add_range (hb_codepoint_t first, hb_codepoint_t last) { return s.add_range (first, last); }

  template <typename T>
  z0 add_array (const T *array, u32 count, u32 stride=sizeof(T))
  { s.add_array (array, count, stride); }
  template <typename T>
  z0 add_array (const hb_array_t<const T>& arr) { add_array (&arr, arr.len ()); }

  /* Might return false if array looks unsorted.
   * Used for faster rejection of corrupt data. */
  template <typename T>
  b8 add_sorted_array (const T *array, u32 count, u32 stride=sizeof(T))
  { return s.add_sorted_array (array, count, stride); }
  template <typename T>
  b8 add_sorted_array (const hb_sorted_array_t<const T>& arr) { return add_sorted_array (&arr, arr.len ()); }

  z0 del (hb_codepoint_t g) { s.del (g); }
  z0 del_range (hb_codepoint_t a, hb_codepoint_t b) { s.del_range (a, b); }

  b8 get (hb_codepoint_t g) const { return s.get (g); }

  /* Has interface. */
  b8 operator [] (hb_codepoint_t k) const { return get (k); }
  b8 has (hb_codepoint_t k) const { return (*this)[k]; }

  /* Predicate. */
  b8 operator () (hb_codepoint_t k) const { return has (k); }

  /* Sink interface. */
  hb_sparseset_t& operator << (hb_codepoint_t v)
  { add (v); return *this; }
  hb_sparseset_t& operator << (const hb_codepoint_pair_t& range)
  { add_range (range.first, range.second); return *this; }

  b8 intersects (hb_codepoint_t first, hb_codepoint_t last) const
  { return s.intersects (first, last); }

  z0 set (const hb_sparseset_t &other) { s.set (other.s); }

  b8 is_equal (const hb_sparseset_t &other) const { return s.is_equal (other.s); }
  b8 operator == (const hb_set_t &other) const { return is_equal (other); }
  b8 operator != (const hb_set_t &other) const { return !is_equal (other); }

  b8 is_subset (const hb_sparseset_t &larger_set) const { return s.is_subset (larger_set.s); }

  z0 union_ (const hb_sparseset_t &other) { s.union_ (other.s); }
  z0 intersect (const hb_sparseset_t &other) { s.intersect (other.s); }
  z0 subtract (const hb_sparseset_t &other) { s.subtract (other.s); }
  z0 symmetric_difference (const hb_sparseset_t &other) { s.symmetric_difference (other.s); }

  b8 next (hb_codepoint_t *codepoint) const { return s.next (codepoint); }
  b8 previous (hb_codepoint_t *codepoint) const { return s.previous (codepoint); }
  b8 next_range (hb_codepoint_t *first, hb_codepoint_t *last) const
  { return s.next_range (first, last); }
  b8 previous_range (hb_codepoint_t *first, hb_codepoint_t *last) const
  { return s.previous_range (first, last); }
  u32 next_many (hb_codepoint_t codepoint, hb_codepoint_t *out, u32 size) const
  { return s.next_many (codepoint, out, size); }

  u32 get_population () const { return s.get_population (); }
  hb_codepoint_t get_min () const { return s.get_min (); }
  hb_codepoint_t get_max () const { return s.get_max (); }

  static constexpr hb_codepoint_t INVALID = impl_t::INVALID;

  /*
   * Iterator implementation.
   */
  using iter_t = typename impl_t::iter_t;
  iter_t iter () const { return iter_t (this->s); }
  operator iter_t () const { return iter (); }
};

struct hb_set_t : hb_sparseset_t<hb_bit_set_invertible_t>
{
  using sparseset = hb_sparseset_t<hb_bit_set_invertible_t>;

  ~hb_set_t () = default;
  hb_set_t () : sparseset () {};
  hb_set_t (const hb_set_t &o) : sparseset ((sparseset &) o) {};
  hb_set_t (hb_set_t&& o)  noexcept : sparseset (std::move ((sparseset &) o)) {}
  hb_set_t& operator = (const hb_set_t&) = default;
  hb_set_t& operator = (hb_set_t&&) = default;
  hb_set_t (std::initializer_list<hb_codepoint_t> lst) : sparseset (lst) {}
  template <typename Iterable,
	    hb_requires (hb_is_iterable (Iterable))>
  hb_set_t (const Iterable &o) : sparseset (o) {}

  hb_set_t& operator << (hb_codepoint_t v)
  { sparseset::operator<< (v); return *this; }
  hb_set_t& operator << (const hb_codepoint_pair_t& range)
  { sparseset::operator<< (range); return *this; }
};

static_assert (hb_set_t::INVALID == HB_SET_VALUE_INVALID, "");


#endif /* HB_SET_HH */
