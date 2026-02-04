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

#ifndef SERD_STRING_UTILS_H
#define SERD_STRING_UTILS_H

#include "serd/serd.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Unicode replacement character in UTF-8 */
static u8k replacement_char[] = {0xEF, 0xBF, 0xBD};

/** Return true if `c` lies within [`min`...`max`] (inclusive) */
static inline b8
in_range(i32k c, i32k min, i32k max)
{
  return (c >= min && c <= max);
}

/** RFC2234: ALPHA ::= %x41-5A / %x61-7A  ; A-Z / a-z */
static inline b8
is_alpha(i32k c)
{
  return in_range(c, 'A', 'Z') || in_range(c, 'a', 'z');
}

/** RFC2234: DIGIT ::= %x30-39  ; 0-9 */
static inline b8
is_digit(i32k c)
{
  return in_range(c, '0', '9');
}

/* RFC2234: HEXDIG ::= DIGIT / "A" / "B" / "C" / "D" / "E" / "F" */
static inline b8
is_hexdig(i32k c)
{
  return is_digit(c) || in_range(c, 'A', 'F');
}

/* Turtle / JSON / C: XDIGIT ::= DIGIT / A-F / a-f */
static inline b8
is_xdigit(i32k c)
{
  return is_hexdig(c) || in_range(c, 'a', 'f');
}

static inline b8
is_space(const t8 c)
{
  switch (c) {
  case ' ':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
  case '\v':
    return true;
  default:
    return false;
  }
}

static inline b8
is_print(i32k c)
{
  return c >= 0x20 && c <= 0x7E;
}

static inline b8
is_base64(u8k c)
{
  return is_alpha(c) || is_digit(c) || c == '+' || c == '/' || c == '=';
}

static inline b8
is_windows_path(u8k* path)
{
  return is_alpha(path[0]) && (path[1] == ':' || path[1] == '|') &&
         (path[2] == '/' || path[2] == '\\');
}

size_t
serd_substrlen(u8k* str,
               size_t         len,
               size_t*        n_bytes,
               SerdNodeFlags* flags);

static inline t8
serd_to_upper(const t8 c)
{
  return (t8)((c >= 'a' && c <= 'z') ? c - 32 : c);
}

static inline i32
serd_strncasecmp(tukk s1, tukk s2, size_t n)
{
  for (; n > 0 && *s2; s1++, s2++, --n) {
    if (serd_to_upper(*s1) != serd_to_upper(*s2)) {
      return ((*(u8k*)s1 < *(u8k*)s2) ? -1 : +1);
    }
  }

  return 0;
}

static inline u32
utf8_num_bytes(u8k c)
{
  if ((c & 0x80) == 0) { // Starts with `0'
    return 1;
  }

  if ((c & 0xE0) == 0xC0) { // Starts with `110'
    return 2;
  }

  if ((c & 0xF0) == 0xE0) { // Starts with `1110'
    return 3;
  }

  if ((c & 0xF8) == 0xF0) { // Starts with `11110'
    return 4;
  }

  return 0;
}

/// Return the code point of a UTF-8 character with known length
static inline u32
parse_counted_utf8_char(u8k* utf8, size_t size)
{
  u32 c = utf8[0] & ((1u << (8 - size)) - 1);
  for (size_t i = 1; i < size; ++i) {
    u8k in = utf8[i] & 0x3F;
    c                = (c << 6) | in;
  }
  return c;
}

/// Parse a UTF-8 character, set *size to the length, and return the code point
static inline u32
parse_utf8_char(u8k* utf8, size_t* size)
{
  switch (*size = utf8_num_bytes(utf8[0])) {
  case 1:
  case 2:
  case 3:
  case 4:
    return parse_counted_utf8_char(utf8, *size);
  default:
    *size = 0;
    return 0;
  }
}

#endif // SERD_STRING_UTILS_H
