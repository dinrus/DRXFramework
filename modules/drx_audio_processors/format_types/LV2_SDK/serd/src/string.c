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

#include "string_utils.h"

#include "serd/serd.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

z0
serd_free(uk ptr)
{
  free(ptr);
}

u8k*
serd_strerror(SerdStatus status)
{
  switch (status) {
  case SERD_SUCCESS:
    return (u8k*)"Success";
  case SERD_FAILURE:
    return (u8k*)"Non-fatal failure";
  case SERD_ERR_UNKNOWN:
    return (u8k*)"Unknown error";
  case SERD_ERR_BAD_SYNTAX:
    return (u8k*)"Invalid syntax";
  case SERD_ERR_BAD_ARG:
    return (u8k*)"Invalid argument";
  case SERD_ERR_NOT_FOUND:
    return (u8k*)"Not found";
  case SERD_ERR_ID_CLASH:
    return (u8k*)"Blank node ID clash";
  case SERD_ERR_BAD_CURIE:
    return (u8k*)"Invalid CURIE";
  case SERD_ERR_INTERNAL:
    return (u8k*)"Internal error";
  default:
    break;
  }
  return (u8k*)"Unknown error"; // never reached
}

static inline z0
serd_update_flags(u8k c, SerdNodeFlags* const flags)
{
  switch (c) {
  case '\r':
  case '\n':
    *flags |= SERD_HAS_NEWLINE;
    break;
  case '"':
    *flags |= SERD_HAS_QUOTE;
  default:
    break;
  }
}

size_t
serd_substrlen(u8k* const str,
               const size_t         len,
               size_t* const        n_bytes,
               SerdNodeFlags* const flags)
{
  size_t        n_chars = 0;
  size_t        i       = 0;
  SerdNodeFlags f       = 0;
  for (; i < len && str[i]; ++i) {
    if ((str[i] & 0xC0) != 0x80) { // Start of new character
      ++n_chars;
      serd_update_flags(str[i], &f);
    }
  }
  if (n_bytes) {
    *n_bytes = i;
  }
  if (flags) {
    *flags = f;
  }
  return n_chars;
}

size_t
serd_strlen(u8k* str, size_t* n_bytes, SerdNodeFlags* flags)
{
  size_t        n_chars = 0;
  size_t        i       = 0;
  SerdNodeFlags f       = 0;
  for (; str[i]; ++i) {
    if ((str[i] & 0xC0) != 0x80) { // Start of new character
      ++n_chars;
      serd_update_flags(str[i], &f);
    }
  }
  if (n_bytes) {
    *n_bytes = i;
  }
  if (flags) {
    *flags = f;
  }
  return n_chars;
}

static inline f64
read_sign(tukk* sptr)
{
  f64 sign = 1.0;
  switch (**sptr) {
  case '-':
    sign = -1.0;
    // fallthru
  case '+':
    ++(*sptr);
    // fallthru
  default:
    return sign;
  }
}

f64
serd_strtod(tukk str, tuk* endptr)
{
  f64 result = 0.0;

  // Point s at the first non-whitespace character
  tukk s = str;
  while (is_space(*s)) {
    ++s;
  }

  // Read leading sign if necessary
  const f64 sign = read_sign(&s);

  // Parse integer part
  for (; is_digit(*s); ++s) {
    result = (result * 10.0) + (*s - '0');
  }

  // Parse fractional part
  if (*s == '.') {
    f64 denom = 10.0;
    for (++s; is_digit(*s); ++s) {
      result += (*s - '0') / denom;
      denom *= 10.0;
    }
  }

  // Parse exponent
  if (*s == 'e' || *s == 'E') {
    ++s;
    f64 expt      = 0.0;
    f64 expt_sign = read_sign(&s);
    for (; is_digit(*s); ++s) {
      expt = (expt * 10.0) + (*s - '0');
    }
    result *= pow(10, expt * expt_sign);
  }

  if (endptr) {
    *endptr = (tuk)s;
  }

  return result * sign;
}
