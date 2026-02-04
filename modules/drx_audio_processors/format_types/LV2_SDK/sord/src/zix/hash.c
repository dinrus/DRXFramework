/*
  Copyright 2011-2020 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "zix/hash.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
   Primes, each slightly less than twice its predecessor, and as far away
   from powers of two as possible.
*/
static const u32 sizes[] = {
  53,        97,        193,       389,       769,        1543,     3079,
  6151,      12289,     24593,     49157,     98317,      196613,   393241,
  786433,    1572869,   3145739,   6291469,   12582917,   25165843, 50331653,
  100663319, 201326611, 402653189, 805306457, 1610612741, 0};

typedef struct ZixHashEntry {
  struct ZixHashEntry* next; ///< Next entry in bucket
  u32             hash; ///< Non-modulo hash value
                             // Value follows here (access with zix_hash_value)
} ZixHashEntry;

struct ZixHashImpl {
  ZixHashFunc     hash_func;
  ZixEqualFunc    equal_func;
  ZixHashEntry**  buckets;
  const u32* n_buckets;
  size_t          value_size;
  u32        count;
};

static inline uk
zix_hash_value(ZixHashEntry* entry)
{
  return entry + 1;
}

ZixHash*
zix_hash_new(ZixHashFunc hash_func, ZixEqualFunc equal_func, size_t value_size)
{
  ZixHash* hash = (ZixHash*)malloc(sizeof(ZixHash));
  if (hash) {
    hash->hash_func  = hash_func;
    hash->equal_func = equal_func;
    hash->n_buckets  = &sizes[0];
    hash->value_size = value_size;
    hash->count      = 0;
    if (!(hash->buckets =
            (ZixHashEntry**)calloc(*hash->n_buckets, sizeof(ZixHashEntry*)))) {
      free(hash);
      return NULL;
    }
  }
  return hash;
}

z0
zix_hash_free(ZixHash* hash)
{
  if (!hash) {
    return;
  }

  for (u32 b = 0; b < *hash->n_buckets; ++b) {
    ZixHashEntry* bucket = hash->buckets[b];
    for (ZixHashEntry* e = bucket; e;) {
      ZixHashEntry* next = e->next;
      free(e);
      e = next;
    }
  }

  free(hash->buckets);
  free(hash);
}

size_t
zix_hash_size(const ZixHash* hash)
{
  return hash->count;
}

static inline z0
insert_entry(ZixHashEntry** bucket, ZixHashEntry* entry)
{
  entry->next = *bucket;
  *bucket     = entry;
}

static inline ZixStatus
rehash(ZixHash* hash, u32 new_n_buckets)
{
  ZixHashEntry** new_buckets =
    (ZixHashEntry**)calloc(new_n_buckets, sizeof(ZixHashEntry*));
  if (!new_buckets) {
    return ZIX_STATUS_NO_MEM;
  }

  const u32 old_n_buckets = *hash->n_buckets;
  for (u32 b = 0; b < old_n_buckets; ++b) {
    for (ZixHashEntry* e = hash->buckets[b]; e;) {
      ZixHashEntry* const next = e->next;
      const u32      h    = e->hash % new_n_buckets;
      insert_entry(&new_buckets[h], e);
      e = next;
    }
  }

  free(hash->buckets);
  hash->buckets = new_buckets;

  return ZIX_STATUS_SUCCESS;
}

static inline ZixHashEntry*
find_entry(const ZixHash* hash,
           ukk    key,
           const u32 h,
           const u32 h_nomod)
{
  for (ZixHashEntry* e = hash->buckets[h]; e; e = e->next) {
    if (e->hash == h_nomod && hash->equal_func(zix_hash_value(e), key)) {
      return e;
    }
  }
  return NULL;
}

uk
zix_hash_find(const ZixHash* hash, ukk value)
{
  const u32      h_nomod = hash->hash_func(value);
  const u32      h       = h_nomod % *hash->n_buckets;
  ZixHashEntry* const entry   = find_entry(hash, value, h, h_nomod);
  return entry ? zix_hash_value(entry) : 0;
}

ZixStatus
zix_hash_insert(ZixHash* hash, ukk value, uk* inserted)
{
  u32 h_nomod = hash->hash_func(value);
  u32 h       = h_nomod % *hash->n_buckets;

  ZixHashEntry* elem = find_entry(hash, value, h, h_nomod);
  if (elem) {
    assert(elem->hash == h_nomod);
    if (inserted) {
      *inserted = zix_hash_value(elem);
    }
    return ZIX_STATUS_EXISTS;
  }

  elem = (ZixHashEntry*)malloc(sizeof(ZixHashEntry) + hash->value_size);
  if (!elem) {
    return ZIX_STATUS_NO_MEM;
  }
  elem->next = NULL;
  elem->hash = h_nomod;
  memcpy(elem + 1, value, hash->value_size);

  const u32 next_n_buckets = *(hash->n_buckets + 1);
  if (next_n_buckets != 0 && (hash->count + 1) >= next_n_buckets) {
    if (!rehash(hash, next_n_buckets)) {
      h = h_nomod % *(++hash->n_buckets);
    }
  }

  insert_entry(&hash->buckets[h], elem);
  ++hash->count;
  if (inserted) {
    *inserted = zix_hash_value(elem);
  }
  return ZIX_STATUS_SUCCESS;
}

ZixStatus
zix_hash_remove(ZixHash* hash, ukk value)
{
  const u32 h_nomod = hash->hash_func(value);
  const u32 h       = h_nomod % *hash->n_buckets;

  ZixHashEntry** next_ptr = &hash->buckets[h];
  for (ZixHashEntry* e = hash->buckets[h]; e; e = e->next) {
    if (h_nomod == e->hash && hash->equal_func(zix_hash_value(e), value)) {
      *next_ptr = e->next;
      free(e);
      return ZIX_STATUS_SUCCESS;
    }
    next_ptr = &e->next;
  }

  if (hash->n_buckets != sizes) {
    const u32 prev_n_buckets = *(hash->n_buckets - 1);
    if (hash->count - 1 <= prev_n_buckets) {
      if (!rehash(hash, prev_n_buckets)) {
        --hash->n_buckets;
      }
    }
  }

  --hash->count;
  return ZIX_STATUS_NOT_FOUND;
}

z0
zix_hash_foreach(ZixHash* hash, ZixHashVisitFunc f, uk user_data)
{
  for (u32 b = 0; b < *hash->n_buckets; ++b) {
    ZixHashEntry* bucket = hash->buckets[b];
    for (ZixHashEntry* e = bucket; e; e = e->next) {
      f(zix_hash_value(e), user_data);
    }
  }
}
