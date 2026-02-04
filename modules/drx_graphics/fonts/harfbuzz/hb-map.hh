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

#ifndef HB_MAP_HH
#define HB_MAP_HH

#include "hb.hh"

#include "hb-set.hh"


/*
 * hb_hashmap_t
 */

extern HB_INTERNAL const hb_codepoint_t minus_1;

template <typename K, typename V,
	  b8 minus_one = false>
struct hb_hashmap_t
{
  static constexpr b8 realloc_move = true;

  hb_hashmap_t ()  { init (); }
  ~hb_hashmap_t () { fini (); }

  hb_hashmap_t (const hb_hashmap_t& o) : hb_hashmap_t ()
  {
    if (unlikely (!o.mask)) return;

    if (item_t::is_trivial)
    {
      items = (item_t *) hb_malloc (sizeof (item_t) * (o.mask + 1));
      if (unlikely (!items))
      {
	successful = false;
	return;
      }
      population = o.population;
      occupancy = o.occupancy;
      mask = o.mask;
      prime = o.prime;
      max_chain_length = o.max_chain_length;
      memcpy (items, o.items, sizeof (item_t) * (mask + 1));
      return;
    }

    alloc (o.population); hb_copy (o, *this);
  }
  hb_hashmap_t (hb_hashmap_t&& o)  noexcept : hb_hashmap_t () { hb_swap (*this, o); }
  hb_hashmap_t& operator= (const hb_hashmap_t& o)  { reset (); alloc (o.population); hb_copy (o, *this); return *this; }
  hb_hashmap_t& operator= (hb_hashmap_t&& o)   noexcept { hb_swap (*this, o); return *this; }

  hb_hashmap_t (std::initializer_list<hb_pair_t<K, V>> lst) : hb_hashmap_t ()
  {
    for (auto&& item : lst)
      set (item.first, item.second);
  }
  template <typename Iterable,
	    hb_requires (hb_is_iterable (Iterable))>
  hb_hashmap_t (const Iterable &o) : hb_hashmap_t ()
  {
    auto iter = hb_iter (o);
    if (iter.is_random_access_iterator || iter.has_fast_len)
      alloc (hb_len (iter));
    hb_copy (iter, *this);
  }

  struct item_t
  {
    K key;
    u32 is_real_ : 1;
    u32 is_used_ : 1;
    u32 hash : 30;
    V value;

    item_t () : key (),
		is_real_ (false), is_used_ (false),
		hash (0),
		value () {}

    // Needed for https://github.com/harfbuzz/harfbuzz/issues/4138
    K& get_key () { return key; }
    V& get_value () { return value; }

    b8 is_used () const { return is_used_; }
    z0 set_used (b8 is_used) { is_used_ = is_used; }
    z0 set_real (b8 is_real) { is_real_ = is_real; }
    b8 is_real () const { return is_real_; }

    template <b8 v = minus_one,
	      hb_enable_if (v == false)>
    static inline const V& default_value () { return Null(V); };
    template <b8 v = minus_one,
	      hb_enable_if (v == true)>
    static inline const V& default_value ()
    {
      static_assert (hb_is_same (V, hb_codepoint_t), "");
      return minus_1;
    };

    b8 operator == (const K &o) const { return hb_deref (key) == hb_deref (o); }
    b8 operator == (const item_t &o) const { return *this == o.key; }
    hb_pair_t<K, V> get_pair() const { return hb_pair_t<K, V> (key, value); }
    hb_pair_t<const K &, V &> get_pair_ref() { return hb_pair_t<const K &, V &> (key, value); }

    u32 total_hash () const
    { return (hash * 31u) + hb_hash (value); }

    static constexpr b8 is_trivial = hb_is_trivially_constructible(K) &&
				       hb_is_trivially_destructible(K) &&
				       hb_is_trivially_constructible(V) &&
				       hb_is_trivially_destructible(V);
  };

  hb_object_header_t header;
  b8 successful; /* Allocations successful */
  u16 max_chain_length;
  u32 population; /* Not including tombstones. */
  u32 occupancy; /* Including tombstones. */
  u32 mask;
  u32 prime;
  item_t *items;

  friend z0 swap (hb_hashmap_t& a, hb_hashmap_t& b) noexcept
  {
    if (unlikely (!a.successful || !b.successful))
      return;
    hb_swap (a.max_chain_length, b.max_chain_length);
    hb_swap (a.population, b.population);
    hb_swap (a.occupancy, b.occupancy);
    hb_swap (a.mask, b.mask);
    hb_swap (a.prime, b.prime);
    hb_swap (a.items, b.items);
  }
  z0 init ()
  {
    hb_object_init (this);

    successful = true;
    max_chain_length = 0;
    population = occupancy = 0;
    mask = 0;
    prime = 0;
    items = nullptr;
  }
  z0 fini ()
  {
    hb_object_fini (this);

    if (likely (items))
    {
      u32 size = mask + 1;
      if (!item_t::is_trivial)
	for (u32 i = 0; i < size; i++)
	  items[i].~item_t ();
      hb_free (items);
      items = nullptr;
    }
    population = occupancy = 0;
  }

  z0 reset ()
  {
    successful = true;
    clear ();
  }

  b8 in_error () const { return !successful; }

  b8 alloc (u32 new_population = 0)
  {
    if (unlikely (!successful)) return false;

    if (new_population != 0 && (new_population + new_population / 2) < mask) return true;

    u32 power = hb_bit_storage (hb_max ((u32) population, new_population) * 2 + 8);
    u32 new_size = 1u << power;
    item_t *new_items = (item_t *) hb_malloc ((size_t) new_size * sizeof (item_t));
    if (unlikely (!new_items))
    {
      successful = false;
      return false;
    }
    if (!item_t::is_trivial)
      for (auto &_ : hb_iter (new_items, new_size))
	new (&_) item_t ();
    else
      hb_memset (new_items, 0, (size_t) new_size * sizeof (item_t));

    u32 old_size = size ();
    item_t *old_items = items;

    /* Switch to new, empty, array. */
    population = occupancy = 0;
    mask = new_size - 1;
    prime = prime_for (power);
    max_chain_length = power * 2;
    items = new_items;

    /* Insert back old items. */
    for (u32 i = 0; i < old_size; i++)
    {
      if (old_items[i].is_real ())
      {
	set_with_hash (std::move (old_items[i].key),
		       old_items[i].hash,
		       std::move (old_items[i].value));
      }
    }
    if (!item_t::is_trivial)
      for (u32 i = 0; i < old_size; i++)
	old_items[i].~item_t ();

    hb_free (old_items);

    return true;
  }

  template <typename KK, typename VV>
  b8 set_with_hash (KK&& key, u32 hash, VV&& value, b8 overwrite = true)
  {
    if (unlikely (!successful)) return false;
    if (unlikely ((occupancy + occupancy / 2) >= mask && !alloc ())) return false;

    hash &= 0x3FFFFFFF; // We only store lower 30bit of hash
    u32 tombstone = (u32) -1;
    u32 i = hash % prime;
    u32 length = 0;
    u32 step = 0;
    while (items[i].is_used ())
    {
      if ((std::is_integral<K>::value || items[i].hash == hash) &&
	  items[i] == key)
      {
        if (!overwrite)
	  return false;
        else
	  break;
      }
      if (!items[i].is_real () && tombstone == (u32) -1)
        tombstone = i;
      i = (i + ++step) & mask;
      length++;
    }

    item_t &item = items[tombstone == (u32) -1 ? i : tombstone];

    if (item.is_used ())
    {
      occupancy--;
      population -= item.is_real ();
    }

    item.key = std::forward<KK> (key);
    item.value = std::forward<VV> (value);
    item.hash = hash;
    item.set_used (true);
    item.set_real (true);

    occupancy++;
    population++;

    if (unlikely (length > max_chain_length) && occupancy * 8 > mask)
      alloc (mask - 8); // This ensures we jump to next larger size

    return true;
  }

  template <typename VV>
  b8 set (const K &key, VV&& value, b8 overwrite = true) { return set_with_hash (key, hb_hash (key), std::forward<VV> (value), overwrite); }
  template <typename VV>
  b8 set (K &&key, VV&& value, b8 overwrite = true)
  {
    u32 hash = hb_hash (key);
    return set_with_hash (std::move (key), hash, std::forward<VV> (value), overwrite);
  }
  b8 add (const K &key)
  {
    u32 hash = hb_hash (key);
    return set_with_hash (key, hash, item_t::default_value ());
  }

  const V& get_with_hash (const K &key, u32 hash) const
  {
    if (!items) return item_t::default_value ();
    auto *item = fetch_item (key, hash);
    if (item)
      return item->value;
    return item_t::default_value ();
  }
  const V& get (const K &key) const
  {
    if (!items) return item_t::default_value ();
    return get_with_hash (key, hb_hash (key));
  }

  z0 del (const K &key)
  {
    if (!items) return;
    auto *item = fetch_item (key, hb_hash (key));
    if (item)
    {
      item->set_real (false);
      population--;
    }
  }

  /* Has interface. */
  const V& operator [] (K k) const { return get (k); }
  template <typename VV=V>
  b8 has (const K &key, VV **vp = nullptr) const
  {
    if (!items) return false;
    auto *item = fetch_item (key, hb_hash (key));
    if (item)
    {
      if (vp) *vp = std::addressof (item->value);
      return true;
    }
    return false;
  }
  item_t *fetch_item (const K &key, u32 hash) const
  {
    hash &= 0x3FFFFFFF; // We only store lower 30bit of hash
    u32 i = hash % prime;
    u32 step = 0;
    while (items[i].is_used ())
    {
      if ((std::is_integral<K>::value || items[i].hash == hash) &&
	  items[i] == key)
      {
	if (items[i].is_real ())
	  return &items[i];
	else
	  return nullptr;
      }
      i = (i + ++step) & mask;
    }
    return nullptr;
  }
  /* Projection. */
  const V& operator () (K k) const { return get (k); }

  u32 size () const { return mask ? mask + 1 : 0; }

  z0 clear ()
  {
    if (unlikely (!successful)) return;

    for (auto &_ : hb_iter (items, size ()))
    {
      /* Reconstruct items. */
      _.~item_t ();
      new (&_) item_t ();
    }

    population = occupancy = 0;
  }

  b8 is_empty () const { return population == 0; }
  explicit operator b8 () const { return !is_empty (); }

  u32 hash () const
  {
    return
    + iter_items ()
    | hb_reduce ([] (u32 h, const item_t &_) { return h ^ _.total_hash (); }, (u32) 0u)
    ;
  }

  b8 is_equal (const hb_hashmap_t &other) const
  {
    if (population != other.population) return false;

    for (auto pair : iter ())
      if (other.get (pair.first) != pair.second)
        return false;

    return true;
  }
  b8 operator == (const hb_hashmap_t &other) const { return is_equal (other); }
  b8 operator != (const hb_hashmap_t &other) const { return !is_equal (other); }

  u32 get_population () const { return population; }

  z0 update (const hb_hashmap_t &other)
  {
    if (unlikely (!successful)) return;

    hb_copy (other, *this);
  }

  /*
   * Iterator
   */

  auto iter_items () const HB_AUTO_RETURN
  (
    + hb_iter (items, this->size ())
    | hb_filter (&item_t::is_real)
  )
  auto iter_ref () const HB_AUTO_RETURN
  (
    + this->iter_items ()
    | hb_map (&item_t::get_pair_ref)
  )
  auto iter () const HB_AUTO_RETURN
  (
    + this->iter_items ()
    | hb_map (&item_t::get_pair)
  )
  auto keys_ref () const HB_AUTO_RETURN
  (
    + this->iter_items ()
    | hb_map (&item_t::get_key)
  )
  auto keys () const HB_AUTO_RETURN
  (
    + this->keys_ref ()
    | hb_map (hb_ridentity)
  )
  auto values_ref () const HB_AUTO_RETURN
  (
    + this->iter_items ()
    | hb_map (&item_t::get_value)
  )
  auto values () const HB_AUTO_RETURN
  (
    + this->values_ref ()
    | hb_map (hb_ridentity)
  )

  /* C iterator. */
  b8 next (i32 *idx,
	     K *key,
	     V *value) const
  {
    u32 i = (u32) (*idx + 1);

    u32 count = size ();
    while (i < count && !items[i].is_real ())
      i++;

    if (i >= count)
    {
      *idx = -1;
      return false;
    }

    *key = items[i].key;
    *value = items[i].value;

    *idx = (signed) i;
    return true;
  }

  /* Sink interface. */
  hb_hashmap_t& operator << (const hb_pair_t<K, V>& v)
  { set (v.first, v.second); return *this; }
  hb_hashmap_t& operator << (const hb_pair_t<K, V&&>& v)
  { set (v.first, std::move (v.second)); return *this; }
  hb_hashmap_t& operator << (const hb_pair_t<K&&, V>& v)
  { set (std::move (v.first), v.second); return *this; }
  hb_hashmap_t& operator << (const hb_pair_t<K&&, V&&>& v)
  { set (std::move (v.first), std::move (v.second)); return *this; }

  static u32 prime_for (u32 shift)
  {
    /* Following comment and table copied from glib. */
    /* Each table size has an associated prime modulo (the first prime
     * lower than the table size) used to find the initial bucket. Probing
     * then works modulo 2^n. The prime modulo is necessary to get a
     * good distribution with poor hash functions.
     */
    /* Not declaring static to make all kinds of compilers happy... */
    /*static*/ u32k prime_mod [32] =
    {
      1,          /* For 1 << 0 */
      2,
      3,
      7,
      13,
      31,
      61,
      127,
      251,
      509,
      1021,
      2039,
      4093,
      8191,
      16381,
      32749,
      65521,      /* For 1 << 16 */
      131071,
      262139,
      524287,
      1048573,
      2097143,
      4194301,
      8388593,
      16777213,
      33554393,
      67108859,
      134217689,
      268435399,
      536870909,
      1073741789,
      2147483647  /* For 1 << 31 */
    };

    if (unlikely (shift >= ARRAY_LENGTH (prime_mod)))
      return prime_mod[ARRAY_LENGTH (prime_mod) - 1];

    return prime_mod[shift];
  }
};

/*
 * hb_map_t
 */

struct hb_map_t : hb_hashmap_t<hb_codepoint_t,
			       hb_codepoint_t,
			       true>
{
  using hashmap = hb_hashmap_t<hb_codepoint_t,
			       hb_codepoint_t,
			       true>;

  ~hb_map_t () = default;
  hb_map_t () : hashmap () {}
  hb_map_t (const hb_map_t &o) : hashmap ((hashmap &) o) {}
  hb_map_t (hb_map_t &&o)  noexcept : hashmap (std::move ((hashmap &) o)) {}
  hb_map_t& operator= (const hb_map_t&) = default;
  hb_map_t& operator= (hb_map_t&&) = default;
  hb_map_t (std::initializer_list<hb_codepoint_pair_t> lst) : hashmap (lst) {}
  template <typename Iterable,
	    hb_requires (hb_is_iterable (Iterable))>
  hb_map_t (const Iterable &o) : hashmap (o) {}
};


#endif /* HB_MAP_HH */
