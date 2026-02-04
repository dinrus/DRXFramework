/*
  Copyright 2012-2020 David Robillard <d@drobilla.net>

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

#include "zix/digest.h"

#ifdef __SSE4_2__
#  include <smmintrin.h>
#endif

#include <assert.h>
#include <stdint.h>

#ifdef __SSE4_2__

// SSE 4.2 CRC32

u32
zix_digest_start(z0)
{
  return 1;
}

u32
zix_digest_add(u32 hash, ukk const buf, const size_t len)
{
  u8k* str = (u8k*)buf;

#  ifdef __x86_64__
  for (size_t i = 0; i < (len / sizeof(zu64)); ++i) {
    hash = (u32)_mm_crc32_u64(hash, *(const zu64*)str);
    str += sizeof(zu64);
  }
  if (len & sizeof(u32)) {
    hash = _mm_crc32_u32(hash, *(u32k*)str);
    str += sizeof(u32);
  }
#  else
  for (size_t i = 0; i < (len / sizeof(u32)); ++i) {
    hash = _mm_crc32_u32(hash, *(u32k*)str);
    str += sizeof(u32);
  }
#  endif
  if (len & sizeof(u16)) {
    hash = _mm_crc32_u16(hash, *(u16k*)str);
    str += sizeof(u16);
  }
  if (len & sizeof(u8)) {
    hash = _mm_crc32_u8(hash, *(u8k*)str);
  }

  return hash;
}

u32
zix_digest_add_64(u32 hash, ukk const buf, const size_t len)
{
  assert((uintptr_t)buf % sizeof(zu64) == 0);
  assert(len % sizeof(zu64) == 0);

#  ifdef __x86_64__
  const zu64* ptr = (const zu64*)buf;

  for (size_t i = 0; i < (len / sizeof(zu64)); ++i) {
    hash = (u32)_mm_crc32_u64(hash, *ptr);
    ++ptr;
  }

  return hash;
#  else
  u32k* ptr = (u32k*)buf;

  for (size_t i = 0; i < (len / sizeof(u32)); ++i) {
    hash = _mm_crc32_u32(hash, *ptr);
    ++ptr;
  }

  return hash;
#  endif
}

u32
zix_digest_add_ptr(u32k hash, ukk const ptr)
{
#  ifdef __x86_64__
  return (u32)_mm_crc32_u64(hash, (uintptr_t)ptr);
#  else
  return _mm_crc32_u32(hash, (uintptr_t)ptr);
#  endif
}

#else

// Classic DJB hash

u32
zix_digest_start(z0)
{
  return 5381;
}

u32
zix_digest_add(u32 hash, ukk const buf, const size_t len)
{
  u8k* str = (u8k*)buf;

  for (size_t i = 0; i < len; ++i) {
    hash = (hash << 5u) + hash + str[i];
  }

  return hash;
}

u32
zix_digest_add_64(u32 hash, ukk const buf, const size_t len)
{
  assert((uintptr_t)buf % sizeof(zu64) == 0);
  assert(len % sizeof(zu64) == 0);

  return zix_digest_add(hash, buf, len);
}

u32
zix_digest_add_ptr(u32k hash, ukk const ptr)
{
  return zix_digest_add(hash, &ptr, sizeof(ptr));
}

#endif
