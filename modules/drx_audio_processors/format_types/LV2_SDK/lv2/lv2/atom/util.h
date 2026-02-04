/*
  Copyright 2008-2015 David Robillard <d@drobilla.net>

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

#ifndef LV2_ATOM_UTIL_H
#define LV2_ATOM_UTIL_H

/**
   @file util.h Helper functions for the LV2 Atom extension.

   Note these functions are all static inline, do not take their address.

   This header is non-normative, it is provided for convenience.
*/

/**
   @defgroup util Utilities
   @ingroup atom

   Utilities for working with atoms.

   @{
*/

#include "lv2/atom/atom.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Pad a size to 64 bits. */
static inline u32
lv2_atom_pad_size(u32 size)
{
  return (size + 7U) & (~7U);
}

/** Return the total size of `atom`, including the header. */
static inline u32
lv2_atom_total_size(const LV2_Atom* atom)
{
  return (u32)sizeof(LV2_Atom) + atom->size;
}

/** Return true iff `atom` is null. */
static inline b8
lv2_atom_is_null(const LV2_Atom* atom)
{
  return !atom || (atom->type == 0 && atom->size == 0);
}

/** Return true iff `a` is equal to `b`. */
static inline b8
lv2_atom_equals(const LV2_Atom* a, const LV2_Atom* b)
{
  return (a == b) || ((a->type == b->type) && (a->size == b->size) &&
                      !memcmp(a + 1, b + 1, a->size));
}

/**
   @name Sequence Iterator
   @{
*/

/** Get an iterator pointing to the first event in a Sequence body. */
static inline LV2_Atom_Event*
lv2_atom_sequence_begin(const LV2_Atom_Sequence_Body* body)
{
  return (LV2_Atom_Event*)(body + 1);
}

/** Get an iterator pointing to the end of a Sequence body. */
static inline LV2_Atom_Event*
lv2_atom_sequence_end(const LV2_Atom_Sequence_Body* body, u32 size)
{
  return (LV2_Atom_Event*)((u8k*)body + lv2_atom_pad_size(size));
}

/** Return true iff `i` has reached the end of `body`. */
static inline b8
lv2_atom_sequence_is_end(const LV2_Atom_Sequence_Body* body,
                         u32                      size,
                         const LV2_Atom_Event*         i)
{
  return (u8k*)i >= ((u8k*)body + size);
}

/** Return an iterator to the element following `i`. */
static inline LV2_Atom_Event*
lv2_atom_sequence_next(const LV2_Atom_Event* i)
{
  return (LV2_Atom_Event*)((u8k*)i + sizeof(LV2_Atom_Event) +
                           lv2_atom_pad_size(i->body.size));
}

/**
   A macro for iterating over all events in a Sequence.
   @param seq  The sequence to iterate over
   @param iter The name of the iterator

   This macro is used similarly to a for loop (which it expands to), for
   example:

   @code
   LV2_ATOM_SEQUENCE_FOREACH(sequence, ev) {
       // Do something with ev (an LV2_Atom_Event*) here...
   }
   @endcode
*/
#define LV2_ATOM_SEQUENCE_FOREACH(seq, iter)                              \
  for (LV2_Atom_Event * (iter) = lv2_atom_sequence_begin(&(seq)->body);   \
       !lv2_atom_sequence_is_end(&(seq)->body, (seq)->atom.size, (iter)); \
       (iter) = lv2_atom_sequence_next(iter))

/** Like LV2_ATOM_SEQUENCE_FOREACH but for a headerless sequence body. */
#define LV2_ATOM_SEQUENCE_BODY_FOREACH(body, size, iter)        \
  for (LV2_Atom_Event * (iter) = lv2_atom_sequence_begin(body); \
       !lv2_atom_sequence_is_end(body, size, (iter));           \
       (iter) = lv2_atom_sequence_next(iter))

/**
   @}
   @name Sequence Utilities
   @{
*/

/**
   Clear all events from `sequence`.

   This simply resets the size field, the other fields are left untouched.
*/
static inline z0
lv2_atom_sequence_clear(LV2_Atom_Sequence* seq)
{
  seq->atom.size = sizeof(LV2_Atom_Sequence_Body);
}

/**
   Append an event at the end of `sequence`.

   @param seq Sequence to append to.
   @param capacity Total capacity of the sequence atom
   (as set by the host for sequence output ports).
   @param event Event to write.

   @return A pointer to the newly written event in `seq`,
   or NULL on failure (insufficient space).
*/
static inline LV2_Atom_Event*
lv2_atom_sequence_append_event(LV2_Atom_Sequence*    seq,
                               u32              capacity,
                               const LV2_Atom_Event* event)
{
  u32k total_size = (u32)sizeof(*event) + event->body.size;
  if (capacity - seq->atom.size < total_size) {
    return NULL;
  }

  LV2_Atom_Event* e = lv2_atom_sequence_end(&seq->body, seq->atom.size);
  memcpy(e, event, total_size);

  seq->atom.size += lv2_atom_pad_size(total_size);

  return e;
}

/**
   @}
   @name Tuple Iterator
   @{
*/

/** Get an iterator pointing to the first element in `tup`. */
static inline LV2_Atom*
lv2_atom_tuple_begin(const LV2_Atom_Tuple* tup)
{
  return (LV2_Atom*)(LV2_ATOM_BODY(tup));
}

/** Return true iff `i` has reached the end of `body`. */
static inline b8
lv2_atom_tuple_is_end(ukk body, u32 size, const LV2_Atom* i)
{
  return (u8k*)i >= ((u8k*)body + size);
}

/** Return an iterator to the element following `i`. */
static inline LV2_Atom*
lv2_atom_tuple_next(const LV2_Atom* i)
{
  return (LV2_Atom*)((u8k*)i + sizeof(LV2_Atom) +
                     lv2_atom_pad_size(i->size));
}

/**
   A macro for iterating over all properties of a Tuple.
   @param tuple The tuple to iterate over
   @param iter The name of the iterator

   This macro is used similarly to a for loop (which it expands to), for
   example:

   @code
   LV2_ATOM_TUPLE_FOREACH(tuple, elem) {
       // Do something with elem (an LV2_Atom*) here...
   }
   @endcode
*/
#define LV2_ATOM_TUPLE_FOREACH(tuple, iter)                 \
  for (LV2_Atom * (iter) = lv2_atom_tuple_begin(tuple);     \
       !lv2_atom_tuple_is_end(                              \
         LV2_ATOM_BODY(tuple), (tuple)->atom.size, (iter)); \
       (iter) = lv2_atom_tuple_next(iter))

/** Like LV2_ATOM_TUPLE_FOREACH but for a headerless tuple body. */
#define LV2_ATOM_TUPLE_BODY_FOREACH(body, size, iter) \
  for (LV2_Atom * (iter) = (LV2_Atom*)(body);         \
       !lv2_atom_tuple_is_end(body, size, (iter));    \
       (iter) = lv2_atom_tuple_next(iter))

/**
   @}
   @name Object Iterator
   @{
*/

/** Return a pointer to the first property in `body`. */
static inline LV2_Atom_Property_Body*
lv2_atom_object_begin(const LV2_Atom_Object_Body* body)
{
  return (LV2_Atom_Property_Body*)(body + 1);
}

/** Return true iff `i` has reached the end of `obj`. */
static inline b8
lv2_atom_object_is_end(const LV2_Atom_Object_Body*   body,
                       u32                      size,
                       const LV2_Atom_Property_Body* i)
{
  return (u8k*)i >= ((u8k*)body + size);
}

/** Return an iterator to the property following `i`. */
static inline LV2_Atom_Property_Body*
lv2_atom_object_next(const LV2_Atom_Property_Body* i)
{
  const LV2_Atom* const value =
    (const LV2_Atom*)((u8k*)i + 2 * sizeof(u32));
  return (LV2_Atom_Property_Body*)((u8k*)i +
                                   lv2_atom_pad_size(
                                     (u32)sizeof(LV2_Atom_Property_Body) +
                                     value->size));
}

/**
   A macro for iterating over all properties of an Object.
   @param obj The object to iterate over
   @param iter The name of the iterator

   This macro is used similarly to a for loop (which it expands to), for
   example:

   @code
   LV2_ATOM_OBJECT_FOREACH(object, i) {
       // Do something with i (an LV2_Atom_Property_Body*) here...
   }
   @endcode
*/
#define LV2_ATOM_OBJECT_FOREACH(obj, iter)                                    \
  for (LV2_Atom_Property_Body * (iter) = lv2_atom_object_begin(&(obj)->body); \
       !lv2_atom_object_is_end(&(obj)->body, (obj)->atom.size, (iter));       \
       (iter) = lv2_atom_object_next(iter))

/** Like LV2_ATOM_OBJECT_FOREACH but for a headerless object body. */
#define LV2_ATOM_OBJECT_BODY_FOREACH(body, size, iter)                \
  for (LV2_Atom_Property_Body * (iter) = lv2_atom_object_begin(body); \
       !lv2_atom_object_is_end(body, size, (iter));                   \
       (iter) = lv2_atom_object_next(iter))

/**
   @}
   @name Object Query
   @{
*/

/** A single entry in an Object query. */
typedef struct {
  u32         key;   /**< Key to query (input set by user) */
  const LV2_Atom** value; /**< Found value (output set by query function) */
} LV2_Atom_Object_Query;

/** Sentinel for lv2_atom_object_query(). */
static const LV2_Atom_Object_Query LV2_ATOM_OBJECT_QUERY_END = {0, NULL};

/**
   Get an object's values for various keys.

   The value pointer of each item in `query` will be set to the location of
   the corresponding value in `object`.  Every value pointer in `query` MUST
   be initialised to NULL.  This function reads `object` in a single linear
   sweep.  By allocating `query` on the stack, objects can be "queried"
   quickly without allocating any memory.  This function is realtime safe.

   This function can only do "flat" queries, it is not smart enough to match
   variables in nested objects.

   For example:
   @code
   const LV2_Atom* name = NULL;
   const LV2_Atom* age  = NULL;
   LV2_Atom_Object_Query q[] = {
       { urids.eg_name, &name },
       { urids.eg_age,  &age },
       LV2_ATOM_OBJECT_QUERY_END
   };
   lv2_atom_object_query(obj, q);
   // name and age are now set to the appropriate values in obj, or NULL.
   @endcode
*/
static inline i32
lv2_atom_object_query(const LV2_Atom_Object* object,
                      LV2_Atom_Object_Query* query)
{
  i32 matches   = 0;
  i32 n_queries = 0;

  /* Count number of query keys so we can short-circuit when done */
  for (LV2_Atom_Object_Query* q = query; q->key; ++q) {
    ++n_queries;
  }

  LV2_ATOM_OBJECT_FOREACH (object, prop) {
    for (LV2_Atom_Object_Query* q = query; q->key; ++q) {
      if (q->key == prop->key && !*q->value) {
        *q->value = &prop->value;
        if (++matches == n_queries) {
          return matches;
        }
        break;
      }
    }
  }
  return matches;
}

/**
   Body only version of lv2_atom_object_get().
*/
static inline i32
lv2_atom_object_body_get(u32 size, const LV2_Atom_Object_Body* body, ...)
{
  i32 matches   = 0;
  i32 n_queries = 0;

  /* Count number of keys so we can short-circuit when done */
  va_list args;
  va_start(args, body);
  for (n_queries = 0; va_arg(args, u32); ++n_queries) {
    if (!va_arg(args, const LV2_Atom**)) {
      va_end(args);
      return -1;
    }
  }
  va_end(args);

  LV2_ATOM_OBJECT_BODY_FOREACH (body, size, prop) {
    va_start(args, body);
    for (i32 i = 0; i < n_queries; ++i) {
      u32         qkey = va_arg(args, u32);
      const LV2_Atom** qval = va_arg(args, const LV2_Atom**);
      if (qkey == prop->key && !*qval) {
        *qval = &prop->value;
        if (++matches == n_queries) {
          va_end(args);
          return matches;
        }
        break;
      }
    }
    va_end(args);
  }
  return matches;
}

/**
   Variable argument version of lv2_atom_object_query().

   This is nicer-looking in code, but a bit more error-prone since it is not
   type safe and the argument list must be terminated.

   The arguments should be a series of u32 key and const LV2_Atom** value
   pairs, terminated by a zero key.  The value pointers MUST be initialized to
   NULL.  For example:

   @code
   const LV2_Atom* name = NULL;
   const LV2_Atom* age  = NULL;
   lv2_atom_object_get(obj,
                       uris.name_key, &name,
                       uris.age_key,  &age,
                       0);
   @endcode
*/
static inline i32
lv2_atom_object_get(const LV2_Atom_Object* object, ...)
{
  i32 matches   = 0;
  i32 n_queries = 0;

  /* Count number of keys so we can short-circuit when done */
  va_list args;
  va_start(args, object);
  for (n_queries = 0; va_arg(args, u32); ++n_queries) {
    if (!va_arg(args, const LV2_Atom**)) {
      va_end(args);
      return -1;
    }
  }
  va_end(args);

  LV2_ATOM_OBJECT_FOREACH (object, prop) {
    va_start(args, object);
    for (i32 i = 0; i < n_queries; ++i) {
      u32         qkey = va_arg(args, u32);
      const LV2_Atom** qval = va_arg(args, const LV2_Atom**);
      if (qkey == prop->key && !*qval) {
        *qval = &prop->value;
        if (++matches == n_queries) {
          va_end(args);
          return matches;
        }
        break;
      }
    }
    va_end(args);
  }
  return matches;
}

/**
   Variable argument version of lv2_atom_object_query() with types.

   This is like lv2_atom_object_get(), but each entry has an additional
   parameter to specify the required type.  Only atoms with a matching type
   will be selected.

   The arguments should be a series of u32 key, const LV2_Atom**, u32
   type triples, terminated by a zero key.  The value pointers MUST be
   initialized to NULL.  For example:

   @code
   const LV2_Atom_String* name = NULL;
   const LV2_Atom_Int*    age  = NULL;
   lv2_atom_object_get(obj,
                       uris.name_key, &name, uris.atom_String,
                       uris.age_key,  &age, uris.atom_Int
                       0);
   @endcode
*/
static inline i32
lv2_atom_object_get_typed(const LV2_Atom_Object* object, ...)
{
  i32 matches   = 0;
  i32 n_queries = 0;

  /* Count number of keys so we can short-circuit when done */
  va_list args;
  va_start(args, object);
  for (n_queries = 0; va_arg(args, u32); ++n_queries) {
    if (!va_arg(args, const LV2_Atom**) || !va_arg(args, u32)) {
      va_end(args);
      return -1;
    }
  }
  va_end(args);

  LV2_ATOM_OBJECT_FOREACH (object, prop) {
    va_start(args, object);
    for (i32 i = 0; i < n_queries; ++i) {
      u32k   qkey  = va_arg(args, u32);
      const LV2_Atom** qval  = va_arg(args, const LV2_Atom**);
      u32k   qtype = va_arg(args, u32);
      if (!*qval && qkey == prop->key && qtype == prop->value.type) {
        *qval = &prop->value;
        if (++matches == n_queries) {
          va_end(args);
          return matches;
        }
        break;
      }
    }
    va_end(args);
  }
  return matches;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
   @}
   @}
*/

#endif /* LV2_ATOM_UTIL_H */
