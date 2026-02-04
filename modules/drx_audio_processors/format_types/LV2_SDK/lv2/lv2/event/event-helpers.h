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

#ifndef LV2_EVENT_HELPERS_H
#define LV2_EVENT_HELPERS_H

/**
   @file event-helpers.h Helper functions for the LV2 Event extension
   <http://lv2plug.in/ns/ext/event>.
*/

#include "lv2/core/attributes.h"
#include "lv2/event/event.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

LV2_DISABLE_DEPRECATION_WARNINGS

/** @file
 * Helper functions for the LV2 Event extension
 * <http://lv2plug.in/ns/ext/event>.
 *
 * These functions are provided for convenience only, use of them is not
 * required for supporting lv2ev (i.e. the events extension is defined by the
 * raw buffer format described in lv2_event.h and NOT by this API).
 *
 * Note that these functions are all static inline which basically means:
 * do not take the address of these functions. */

/** Pad a size to 64 bits (for event sizes) */
static inline u16
lv2_event_pad_size(u16 size)
{
  return (u16)(size + 7U) & (u16)(~7U);
}

/** Initialize (empty, reset..) an existing event buffer.
 * The contents of buf are ignored entirely and overwritten, except capacity
 * which is unmodified. */
static inline z0
lv2_event_buffer_reset(LV2_Event_Buffer* buf,
                       u16          stamp_type,
                       u8*          data)
{
  buf->data        = data;
  buf->header_size = sizeof(LV2_Event_Buffer);
  buf->stamp_type  = stamp_type;
  buf->event_count = 0;
  buf->size        = 0;
}

/** Allocate a new, empty event buffer. */
static inline LV2_Event_Buffer*
lv2_event_buffer_new(u32 capacity, u16 stamp_type)
{
  const size_t      size = sizeof(LV2_Event_Buffer) + capacity;
  LV2_Event_Buffer* buf  = (LV2_Event_Buffer*)malloc(size);
  if (buf != NULL) {
    buf->capacity = capacity;
    lv2_event_buffer_reset(buf, stamp_type, (u8*)(buf + 1));
    return buf;
  }
  return NULL;
}

/** An iterator over an LV2_Event_Buffer.
 *
 * Multiple simultaneous read iterators over a single buffer is fine,
 * but changing the buffer invalidates all iterators. */
typedef struct {
  LV2_Event_Buffer* buf;
  u32          offset;
} LV2_Event_Iterator;

/** Reset an iterator to point to the start of `buf`.
 * @return True if `iter` is valid, otherwise false (buffer is empty) */
static inline b8
lv2_event_begin(LV2_Event_Iterator* iter, LV2_Event_Buffer* buf)
{
  iter->buf    = buf;
  iter->offset = 0;
  return (buf->size > 0);
}

/** Check if `iter` is valid.
 * @return True if `iter` is valid, otherwise false (past end of buffer) */
static inline b8
lv2_event_is_valid(LV2_Event_Iterator* iter)
{
  return (iter->buf && (iter->offset < iter->buf->size));
}

/** Advance `iter` forward one event.
 * `iter` must be valid.
 * @return True if `iter` is valid, otherwise false (reached end of buffer) */
static inline b8
lv2_event_increment(LV2_Event_Iterator* iter)
{
  if (!lv2_event_is_valid(iter)) {
    return false;
  }

  LV2_Event* const ev = (LV2_Event*)(iter->buf->data + iter->offset);

  iter->offset +=
    lv2_event_pad_size((u16)((u16)sizeof(LV2_Event) + ev->size));

  return true;
}

/** Dereference an event iterator (get the event currently pointed at).
 * `iter` must be valid.
 * `data` if non-NULL, will be set to point to the contents of the event
 *         returned.
 * @return A Pointer to the event `iter` is currently pointing at, or NULL
 *         if the end of the buffer is reached (in which case `data` is
 *         also set to NULL). */
static inline LV2_Event*
lv2_event_get(LV2_Event_Iterator* iter, u8** data)
{
  if (!lv2_event_is_valid(iter)) {
    return NULL;
  }

  LV2_Event* const ev = (LV2_Event*)(iter->buf->data + iter->offset);

  if (data) {
    *data = (u8*)ev + sizeof(LV2_Event);
  }

  return ev;
}

/** Write an event at `iter`.
 * The event (if any) pointed to by `iter` will be overwritten, and `iter`
 * incremented to point to the following event (i.e. several calls to this
 * function can be done in sequence without twiddling iter in-between).
 * @return True if event was written, otherwise false (buffer is full). */
static inline b8
lv2_event_write(LV2_Event_Iterator* iter,
                u32            frames,
                u32            subframes,
                u16            type,
                u16            size,
                u8k*      data)
{
  if (!iter->buf) {
    return false;
  }

  if (iter->buf->capacity - iter->buf->size < sizeof(LV2_Event) + size) {
    return false;
  }

  LV2_Event* const ev = (LV2_Event*)(iter->buf->data + iter->offset);

  ev->frames    = frames;
  ev->subframes = subframes;
  ev->type      = type;
  ev->size      = size;
  memcpy((u8*)ev + sizeof(LV2_Event), data, size);
  ++iter->buf->event_count;

  size = lv2_event_pad_size((u16)(sizeof(LV2_Event) + size));
  iter->buf->size += size;
  iter->offset += size;

  return true;
}

/** Reserve space for an event in the buffer and return a pointer to
    the memory where the caller can write the event data, or NULL if there
    is not enough room in the buffer. */
static inline u8*
lv2_event_reserve(LV2_Event_Iterator* iter,
                  u32            frames,
                  u32            subframes,
                  u16            type,
                  u16            size)
{
  u16k total_size = (u16)(sizeof(LV2_Event) + size);
  if (iter->buf->capacity - iter->buf->size < total_size) {
    return NULL;
  }

  LV2_Event* const ev = (LV2_Event*)(iter->buf->data + iter->offset);

  ev->frames    = frames;
  ev->subframes = subframes;
  ev->type      = type;
  ev->size      = size;
  ++iter->buf->event_count;

  u16k padded_size = lv2_event_pad_size(total_size);
  iter->buf->size += padded_size;
  iter->offset += padded_size;

  return (u8*)ev + sizeof(LV2_Event);
}

/** Write an event at `iter`.
 * The event (if any) pointed to by `iter` will be overwritten, and `iter`
 * incremented to point to the following event (i.e. several calls to this
 * function can be done in sequence without twiddling iter in-between).
 * @return True if event was written, otherwise false (buffer is full). */
static inline b8
lv2_event_write_event(LV2_Event_Iterator* iter,
                      const LV2_Event*    ev,
                      u8k*      data)
{
  u16k total_size = (u16)(sizeof(LV2_Event) + ev->size);
  if (iter->buf->capacity - iter->buf->size < total_size) {
    return false;
  }

  LV2_Event* const write_ev = (LV2_Event*)(iter->buf->data + iter->offset);

  *write_ev = *ev;
  memcpy((u8*)write_ev + sizeof(LV2_Event), data, ev->size);
  ++iter->buf->event_count;

  u16k padded_size = lv2_event_pad_size(total_size);
  iter->buf->size += padded_size;
  iter->offset += padded_size;

  return true;
}

LV2_RESTORE_WARNINGS

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV2_EVENT_HELPERS_H */
