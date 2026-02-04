/*
 * Copyright © 2018  Google, Inc.
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

#ifndef HB_ARRAY_HH
#define HB_ARRAY_HH

#include "hb.hh"
#include "hb-algs.hh"
#include "hb-iter.hh"
#include "hb-null.hh"


template <typename Type>
struct hb_sorted_array_t;

enum hb_not_found_t
{
  HB_NOT_FOUND_DONT_STORE,
  HB_NOT_FOUND_STORE,
  HB_NOT_FOUND_STORE_CLOSEST,
};


template <typename Type>
struct hb_array_t : hb_iter_with_fallback_t<hb_array_t<Type>, Type&>
{
  static constexpr b8 realloc_move = true;

  /*
   * Constructors.
   */
  hb_array_t () = default;
  hb_array_t (const hb_array_t&) = default;
  ~hb_array_t () = default;
  hb_array_t& operator= (const hb_array_t&) = default;
  hb_array_t& operator= (hb_array_t&&) = default;

  constexpr hb_array_t (Type *array_, u32 length_) : arrayZ (array_), length (length_) {}
  template <u32 length_>
  constexpr hb_array_t (Type (&array_)[length_]) : hb_array_t (array_, length_) {}

  template <typename U,
	    hb_enable_if (hb_is_cr_convertible(U, Type))>
  constexpr hb_array_t (const hb_array_t<U> &o) :
    hb_iter_with_fallback_t<hb_array_t, Type&> (),
    arrayZ (o.arrayZ), length (o.length), backwards_length (o.backwards_length) {}
  template <typename U,
	    hb_enable_if (hb_is_cr_convertible(U, Type))>
  hb_array_t& operator = (const hb_array_t<U> &o)
  { arrayZ = o.arrayZ; length = o.length; backwards_length = o.backwards_length; return *this; }

  /*
   * Iterator implementation.
   */
  typedef Type& __item_t__;
  static constexpr b8 is_random_access_iterator = true;
  static constexpr b8 has_fast_len = true;
  Type& __item__ () const
  {
    if (unlikely (!length)) return CrapOrNull (Type);
    return *arrayZ;
  }
  Type& __item_at__ (u32 i) const
  {
    if (unlikely (i >= length)) return CrapOrNull (Type);
    return arrayZ[i];
  }
  z0 __next__ ()
  {
    if (unlikely (!length))
      return;
    length--;
    backwards_length++;
    arrayZ++;
  }
  z0 __forward__ (u32 n)
  {
    if (unlikely (n > length))
      n = length;
    length -= n;
    backwards_length += n;
    arrayZ += n;
  }
  z0 __prev__ ()
  {
    if (unlikely (!backwards_length))
      return;
    length++;
    backwards_length--;
    arrayZ--;
  }
  z0 __rewind__ (u32 n)
  {
    if (unlikely (n > backwards_length))
      n = backwards_length;
    length += n;
    backwards_length -= n;
    arrayZ -= n;
  }
  u32 __len__ () const { return length; }
  /* Ouch. The operator== compares the contents of the array.  For range-based for loops,
   * it's best if we can just compare arrayZ, though comparing contents is still fast,
   * but also would require that Type has operator==.  As such, we optimize this operator
   * for range-based for loop and just compare arrayZ and length.
   *
   * The above comment is outdated now because we implemented separate begin/end to
   * objects that were using hb_array_t for range-based loop before. */
  b8 operator != (const hb_array_t& o) const
  { return this->arrayZ != o.arrayZ || this->length != o.length; }

  /* Faster range-based for loop without bounds-check. */
  Type *begin () const { return arrayZ; }
  Type *end () const { return arrayZ + length; }


  /* Extra operators.
   */
  Type * operator & () const { return arrayZ; }
  operator hb_array_t<const Type> () { return hb_array_t<const Type> (arrayZ, length); }
  template <typename T> operator T * () const { return arrayZ; }

  HB_INTERNAL b8 operator == (const hb_array_t &o) const;

  u32 hash () const
  {
    // FNV-1a hash function
    // https://github.com/harfbuzz/harfbuzz/pull/4228
    u32 current = /*cbf29ce4*/0x84222325;
    for (auto &v : *this)
    {
      current = current ^ hb_hash (v);
      current = current * 16777619;
    }
    return current;
  }

  /*
   * Compare, Sort, and Search.
   */

  /* Note: our compare is NOT lexicographic; it also does NOT call Type::cmp. */
  i32 cmp (const hb_array_t &a) const
  {
    if (length != a.length)
      return (i32) a.length - (i32) length;
    return hb_memcmp (a.arrayZ, arrayZ, get_size ());
  }
  HB_INTERNAL static i32 cmp (ukk pa, ukk pb)
  {
    hb_array_t *a = (hb_array_t *) pa;
    hb_array_t *b = (hb_array_t *) pb;
    return b->cmp (*a);
  }

  template <typename T>
  Type *lsearch (const T &x, Type *not_found = nullptr)
  {
    u32 i;
    return lfind (x, &i) ? &this->arrayZ[i] : not_found;
  }
  template <typename T>
  const Type *lsearch (const T &x, const Type *not_found = nullptr) const
  {
    u32 i;
    return lfind (x, &i) ? &this->arrayZ[i] : not_found;
  }
  template <typename T>
  b8 lfind (const T &x, u32 *pos = nullptr,
	      hb_not_found_t not_found = HB_NOT_FOUND_DONT_STORE,
	      u32 to_store = (u32) -1) const
  {
    for (u32 i = 0; i < length; ++i)
      if (hb_equal (x, this->arrayZ[i]))
      {
	if (pos)
	  *pos = i;
	return true;
      }

    if (pos)
    {
      switch (not_found)
      {
	case HB_NOT_FOUND_DONT_STORE:
	  break;

	case HB_NOT_FOUND_STORE:
	  *pos = to_store;
	  break;

	case HB_NOT_FOUND_STORE_CLOSEST:
	  *pos = length;
	  break;
      }
    }
    return false;
  }

  hb_sorted_array_t<Type> qsort (i32 (*cmp_)(ukk, ukk))
  {
    //static_assert (hb_enable_if (hb_is_trivially_copy_assignable(Type)), "");
    if (likely (length))
      hb_qsort (arrayZ, length, this->get_item_size (), cmp_);
    return hb_sorted_array_t<Type> (*this);
  }
  hb_sorted_array_t<Type> qsort ()
  {
    //static_assert (hb_enable_if (hb_is_trivially_copy_assignable(Type)), "");
    if (likely (length))
      hb_qsort (arrayZ, length, this->get_item_size (), Type::cmp);
    return hb_sorted_array_t<Type> (*this);
  }

  /*
   * Other methods.
   */

  u32 get_size () const { return length * this->get_item_size (); }

  /*
   * Reverse the order of items in this array in the range [start, end).
   */
  z0 reverse (u32 start = 0, u32 end = -1)
  {
    start = hb_min (start, length);
    end = hb_min (end, length);

    if (end < start + 2)
      return;

    for (u32 lhs = start, rhs = end - 1; lhs < rhs; lhs++, rhs--)
      hb_swap (arrayZ[rhs], arrayZ[lhs]);
  }

  hb_array_t sub_array (u32 start_offset = 0, u32 *seg_count = nullptr /* IN/OUT */) const
  {
    if (!start_offset && !seg_count)
      return *this;

    u32 count = length;
    if (unlikely (start_offset > count))
      count = 0;
    else
      count -= start_offset;
    if (seg_count)
      count = *seg_count = hb_min (count, *seg_count);
    return hb_array_t (arrayZ + start_offset, count);
  }
  hb_array_t sub_array (u32 start_offset, u32 seg_count) const
  { return sub_array (start_offset, &seg_count); }

  hb_array_t truncate (u32 length) const { return sub_array (0, length); }

  template <typename T,
	    u32 P = sizeof (Type),
	    hb_enable_if (P == 1)>
  const T *as () const
  { return length < hb_min_size (T) ? &Null (T) : reinterpret_cast<const T *> (arrayZ); }

  template <typename T,
	    u32 P = sizeof (Type),
	    hb_enable_if (P == 1)>
  b8 check_range (const T *p, u32 size = T::static_size) const
  {
    return arrayZ <= ((const t8 *) p)
	&& ((const t8 *) p) <= arrayZ + length
	&& (u32) (arrayZ + length - (const t8 *) p) >= size;
  }

  /* Only call if you allocated the underlying array using hb_malloc() or similar. */
  z0 fini ()
  { hb_free ((uk ) arrayZ); arrayZ = nullptr; length = 0; }

  template <typename hb_serialize_context_t,
	    typename U = Type,
	    hb_enable_if (!(sizeof (U) < sizeof (z64) && hb_is_trivially_copy_assignable(hb_decay<Type>)))>
  hb_array_t copy (hb_serialize_context_t *c) const
  {
    TRACE_SERIALIZE (this);
    auto* out = c->start_embed (arrayZ);
    if (unlikely (!c->extend_size (out, get_size (), false))) return_trace (hb_array_t ());
    for (u32 i = 0; i < length; i++)
      out[i] = arrayZ[i]; /* TODO: add version that calls c->copy() */
    return_trace (hb_array_t (out, length));
  }

  template <typename hb_serialize_context_t,
	    typename U = Type,
	    hb_enable_if (sizeof (U) < sizeof (z64) && hb_is_trivially_copy_assignable(hb_decay<Type>))>
  hb_array_t copy (hb_serialize_context_t *c) const
  {
    TRACE_SERIALIZE (this);
    auto* out = c->start_embed (arrayZ);
    if (unlikely (!c->extend_size (out, get_size (), false))) return_trace (hb_array_t ());
    hb_memcpy (out, arrayZ, get_size ());
    return_trace (hb_array_t (out, length));
  }

  template <typename hb_sanitize_context_t>
  b8 sanitize (hb_sanitize_context_t *c) const
  { return c->check_array (arrayZ, length); }

  /*
   * Members
   */

  public:
  Type *arrayZ = nullptr;
  u32 length = 0;
  u32 backwards_length = 0;
};
template <typename T> inline hb_array_t<T>
hb_array ()
{ return hb_array_t<T> (); }
template <typename T> inline hb_array_t<T>
hb_array (T *array, u32 length)
{ return hb_array_t<T> (array, length); }
template <typename T, u32 length_> inline hb_array_t<T>
hb_array (T (&array_)[length_])
{ return hb_array_t<T> (array_); }

template <typename Type>
struct hb_sorted_array_t :
	hb_array_t<Type>,
	hb_iter_t<hb_sorted_array_t<Type>, Type&>
{
  typedef hb_iter_t<hb_sorted_array_t, Type&> iter_base_t;
  HB_ITER_USING (iter_base_t);
  static constexpr b8 is_random_access_iterator = true;
  static constexpr b8 is_sorted_iterator = true;
  static constexpr b8 has_fast_len = true;

  hb_sorted_array_t () = default;
  hb_sorted_array_t (const hb_sorted_array_t&) = default;
  ~hb_sorted_array_t () = default;
  hb_sorted_array_t& operator= (const hb_sorted_array_t&) = default;
  hb_sorted_array_t& operator= (hb_sorted_array_t&&) = default;

  constexpr hb_sorted_array_t (Type *array_, u32 length_) : hb_array_t<Type> (array_, length_) {}
  template <u32 length_>
  constexpr hb_sorted_array_t (Type (&array_)[length_]) : hb_array_t<Type> (array_) {}

  template <typename U,
	    hb_enable_if (hb_is_cr_convertible(U, Type))>
  constexpr hb_sorted_array_t (const hb_array_t<U> &o) :
    hb_array_t<Type> (o),
    hb_iter_t<hb_sorted_array_t, Type&> () {}
  template <typename U,
	    hb_enable_if (hb_is_cr_convertible(U, Type))>
  hb_sorted_array_t& operator = (const hb_array_t<U> &o)
  { hb_array_t<Type> (*this) = o; return *this; }

  /* Iterator implementation. */

  /* See comment in hb_array_of::operator != */
  b8 operator != (const hb_sorted_array_t& o) const
  { return this->arrayZ != o.arrayZ || this->length != o.length; }

  /* Faster range-based for loop without bounds-check. */
  Type *begin () const { return this->arrayZ; }
  Type *end () const { return this->arrayZ + this->length; }


  hb_sorted_array_t sub_array (u32 start_offset, u32 *seg_count /* IN/OUT */) const
  { return hb_sorted_array_t (((const hb_array_t<Type> *) (this))->sub_array (start_offset, seg_count)); }
  hb_sorted_array_t sub_array (u32 start_offset, u32 seg_count) const
  { return sub_array (start_offset, &seg_count); }

  hb_sorted_array_t truncate (u32 length) const { return sub_array (0, length); }

  template <typename T>
  Type *bsearch (const T &x, Type *not_found = nullptr)
  {
    u32 i;
    return bfind (x, &i) ? &this->arrayZ[i] : not_found;
  }
  template <typename T>
  const Type *bsearch (const T &x, const Type *not_found = nullptr) const
  {
    u32 i;
    return bfind (x, &i) ? &this->arrayZ[i] : not_found;
  }
  template <typename T>
  b8 bfind (const T &x, u32 *i = nullptr,
	      hb_not_found_t not_found = HB_NOT_FOUND_DONT_STORE,
	      u32 to_store = (u32) -1) const
  {
    u32 pos;

    if (bsearch_impl (x, &pos))
    {
      if (i)
	*i = pos;
      return true;
    }

    if (i)
    {
      switch (not_found)
      {
	case HB_NOT_FOUND_DONT_STORE:
	  break;

	case HB_NOT_FOUND_STORE:
	  *i = to_store;
	  break;

	case HB_NOT_FOUND_STORE_CLOSEST:
	  *i = pos;
	  break;
      }
    }
    return false;
  }
  template <typename T, typename ...Ts>
  b8 bsearch_impl (const T &x, u32 *pos, Ts... ds) const
  {
    return hb_bsearch_impl (pos,
			    x,
			    this->arrayZ,
			    this->length,
			    sizeof (Type),
			    _hb_cmp_method<T, Type, Ts...>,
			    std::forward<Ts> (ds)...);
  }
};
template <typename T> inline hb_sorted_array_t<T>
hb_sorted_array (T *array, u32 length)
{ return hb_sorted_array_t<T> (array, length); }
template <typename T, u32 length_> inline hb_sorted_array_t<T>
hb_sorted_array (T (&array_)[length_])
{ return hb_sorted_array_t<T> (array_); }

template <typename T>
inline b8 hb_array_t<T>::operator == (const hb_array_t<T> &o) const
{
  if (o.length != this->length) return false;
  for (u32 i = 0; i < this->length; i++) {
    if (this->arrayZ[i] != o.arrayZ[i]) return false;
  }
  return true;
}
template <>
inline b8 hb_array_t<const t8>::operator == (const hb_array_t<const t8> &o) const
{
  if (o.length != this->length) return false;
  return 0 == hb_memcmp (arrayZ, o.arrayZ, length);
}
template <>
inline b8 hb_array_t<u8k>::operator == (const hb_array_t<u8k> &o) const
{
  if (o.length != this->length) return false;
  return 0 == hb_memcmp (arrayZ, o.arrayZ, length);
}


/* Specialize hash() for byte arrays. */

#ifndef HB_OPTIMIZE_SIZE_MORE
template <>
inline u32 hb_array_t<const t8>::hash () const
{
  // https://github.com/harfbuzz/harfbuzz/pull/4228
  return fasthash32(arrayZ, length, 0xf437ffe6 /* magic? */);
}

template <>
inline u32 hb_array_t<u8k>::hash () const
{
  // https://github.com/harfbuzz/harfbuzz/pull/4228
  return fasthash32(arrayZ, length, 0xf437ffe6 /* magic? */);
}
#endif


typedef hb_array_t<const t8> hb_bytes_t;
typedef hb_array_t<u8k> hb_ubytes_t;



#endif /* HB_ARRAY_HH */
