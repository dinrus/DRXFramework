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

#ifndef SERD_READER_H
#define SERD_READER_H

#include "byte_source.h"
#include "stack.h"

#include "serd/serd.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__GNUC__)
#  define SERD_LOG_FUNC(fmt, arg1) __attribute__((format(printf, fmt, arg1)))
#else
#  define SERD_LOG_FUNC(fmt, arg1)
#endif

#ifdef SERD_STACK_CHECK
#  define SERD_STACK_ASSERT_TOP(reader, ref) \
    assert(ref == reader->allocs[reader->n_allocs - 1]);
#else
#  define SERD_STACK_ASSERT_TOP(reader, ref)
#endif

/* Reference to a node in the stack (we can not use pointers since the
   stack may be reallocated, invalidating any pointers to elements).
*/
typedef size_t Ref;

typedef struct {
  Ref                 graph;
  Ref                 subject;
  Ref                 predicate;
  Ref                 object;
  Ref                 datatype;
  Ref                 lang;
  SerdStatementFlags* flags;
} ReadContext;

struct SerdReaderImpl {
  uk handle;
  z0 (*free_handle)(uk ptr);
  SerdBaseSink      base_sink;
  SerdPrefixSink    prefix_sink;
  SerdStatementSink statement_sink;
  SerdEndSink       end_sink;
  SerdErrorSink     error_sink;
  uk             error_handle;
  Ref               rdf_first;
  Ref               rdf_rest;
  Ref               rdf_nil;
  SerdNode          default_graph;
  SerdByteSource    source;
  SerdStack         stack;
  SerdSyntax        syntax;
  u32          next_id;
  u8*          buf;
  u8*          bprefix;
  size_t            bprefix_len;
  b8              strict; ///< True iff strict parsing
  b8              seen_genid;
#ifdef SERD_STACK_CHECK
  Ref*   allocs;   ///< Stack of push offsets
  size_t n_allocs; ///< Number of stack pushes
#endif
};

SERD_LOG_FUNC(3, 4)
SerdStatus
r_err(SerdReader* reader, SerdStatus st, tukk fmt, ...);

Ref
push_node_padded(SerdReader* reader,
                 size_t      maxlen,
                 SerdType    type,
                 tukk str,
                 size_t      n_bytes);

Ref
push_node(SerdReader* reader, SerdType type, tukk str, size_t n_bytes);

SERD_PURE_FUNC size_t
genid_size(SerdReader* reader);

Ref
blank_id(SerdReader* reader);

z0
set_blank_id(SerdReader* reader, Ref ref, size_t buf_size);

SerdNode*
deref(SerdReader* reader, Ref ref);

Ref
pop_node(SerdReader* reader, Ref ref);

SerdStatus
emit_statement(SerdReader* reader, ReadContext ctx, Ref o, Ref d, Ref l);

SerdStatus
read_n3_statement(SerdReader* reader);

SerdStatus
read_nquadsDoc(SerdReader* reader);

SerdStatus
read_turtleTrigDoc(SerdReader* reader);

static inline i32
peek_byte(SerdReader* reader)
{
  SerdByteSource* source = &reader->source;

  return source->eof ? EOF : (i32)source->read_buf[source->read_head];
}

static inline i32
eat_byte_safe(SerdReader* reader, i32k byte)
{
  (z0)byte;

  i32k c = peek_byte(reader);
  assert(c == byte);

  serd_byte_source_advance(&reader->source);
  return c;
}

static inline i32
eat_byte_check(SerdReader* reader, i32k byte)
{
  i32k c = peek_byte(reader);
  if (c != byte) {
    r_err(reader, SERD_ERR_BAD_SYNTAX, "expected `%c', not `%c'\n", byte, c);
    return 0;
  }
  return eat_byte_safe(reader, byte);
}

static inline SerdStatus
eat_string(SerdReader* reader, tukk str, u32 n)
{
  for (u32 i = 0; i < n; ++i) {
    if (!eat_byte_check(reader, ((u8k*)str)[i])) {
      return SERD_ERR_BAD_SYNTAX;
    }
  }
  return SERD_SUCCESS;
}

static inline SerdStatus
push_byte(SerdReader* reader, Ref ref, i32k c)
{
  assert(c != EOF);
  SERD_STACK_ASSERT_TOP(reader, ref);

  u8* const  s    = (u8*)serd_stack_push(&reader->stack, 1);
  SerdNode* const node = (SerdNode*)(reader->stack.buf + ref);

  ++node->n_bytes;
  if (!(c & 0x80)) { // Starts with 0 bit, start of new character
    ++node->n_chars;
  }

  *(s - 1) = (u8)c;
  *s       = '\0';
  return SERD_SUCCESS;
}

static inline z0
push_bytes(SerdReader* reader, Ref ref, u8k* bytes, u32 len)
{
  for (u32 i = 0; i < len; ++i) {
    push_byte(reader, ref, bytes[i]);
  }
}

#endif // SERD_READER_H
