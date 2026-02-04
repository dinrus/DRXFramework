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
#include "uri_utils.h"

#include "serd/serd.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u8k*
serd_uri_to_path(u8k* uri)
{
  u8k* path = uri;
  if (!is_windows_path(uri) && serd_uri_string_has_scheme(uri)) {
    if (strncmp((tukk)uri, "file:", 5)) {
      fprintf(stderr, "Non-file URI `%s'\n", uri);
      return NULL;
    }

    if (!strncmp((tukk)uri, "file://localhost/", 17)) {
      path = uri + 16;
    } else if (!strncmp((tukk)uri, "file://", 7)) {
      path = uri + 7;
    } else {
      fprintf(stderr, "Invalid file URI `%s'\n", uri);
      return NULL;
    }

    if (is_windows_path(path + 1)) {
      ++path; // Special case for terrible Windows file URIs
    }
  }
  return path;
}

u8*
serd_file_uri_parse(u8k* uri, u8** hostname)
{
  u8k* path = uri;
  if (hostname) {
    *hostname = NULL;
  }
  if (!strncmp((tukk)uri, "file://", 7)) {
    u8k* auth = uri + 7;
    if (*auth == '/') { // No hostname
      path = auth;
    } else { // Has hostname
      if (!(path = (u8k*)strchr((tukk)auth, '/'))) {
        return NULL;
      }

      if (hostname) {
        *hostname = (u8*)calloc((size_t)(path - auth + 1), 1);
        memcpy(*hostname, auth, (size_t)(path - auth));
      }
    }
  }

  if (is_windows_path(path + 1)) {
    ++path;
  }

  SerdChunk chunk = {NULL, 0};
  for (u8k* s = path; *s; ++s) {
    if (*s == '%') {
      if (*(s + 1) == '%') {
        serd_chunk_sink("%", 1, &chunk);
        ++s;
      } else if (is_hexdig(*(s + 1)) && is_hexdig(*(s + 2))) {
        u8k code[3] = {*(s + 1), *(s + 2), 0};
        u8k c       = (u8)strtoul((tukk)code, NULL, 16);
        serd_chunk_sink(&c, 1, &chunk);
        s += 2;
      } else {
        s += 2; // Junk escape, ignore
      }
    } else {
      serd_chunk_sink(s, 1, &chunk);
    }
  }

  return serd_chunk_sink_finish(&chunk);
}

b8
serd_uri_string_has_scheme(u8k* utf8)
{
  // RFC3986: scheme ::= ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
  if (!utf8 || !is_alpha(utf8[0])) {
    return false; // Invalid scheme initial character, URI is relative
  }

  for (u8 c = 0; (c = *++utf8) != '\0';) {
    if (!is_uri_scheme_char(c)) {
      return false;
    }

    if (c == ':') {
      return true; // End of scheme
    }
  }

  return false;
}

SerdStatus
serd_uri_parse(u8k* utf8, SerdURI* out)
{
  *out = SERD_URI_NULL;

  u8k* ptr = utf8;

  /* See http://tools.ietf.org/html/rfc3986#section-3
     URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
  */

  /* S3.1: scheme ::= ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) */
  if (is_alpha(*ptr)) {
    for (u8 c = *++ptr; true; c = *++ptr) {
      switch (c) {
      case '\0':
      case '/':
      case '?':
      case '#':
        ptr = utf8;
        goto path; // Relative URI (starts with path by definition)
      case ':':
        out->scheme.buf = utf8;
        out->scheme.len = (size_t)((ptr++) - utf8);
        goto maybe_authority; // URI with scheme
      case '+':
      case '-':
      case '.':
        continue;
      default:
        if (is_alpha(c) || is_digit(c)) {
          continue;
        }
      }
    }
  }

  /* S3.2: The authority component is preceded by a f64 slash ("//")
     and is terminated by the next slash ("/"), question mark ("?"),
     or number sign ("#") character, or by the end of the URI.
  */
maybe_authority:
  if (*ptr == '/' && *(ptr + 1) == '/') {
    ptr += 2;
    out->authority.buf = ptr;
    for (u8 c = 0; (c = *ptr) != '\0'; ++ptr) {
      switch (c) {
      case '/':
        goto path;
      case '?':
        goto query;
      case '#':
        goto fragment;
      default:
        ++out->authority.len;
      }
    }
  }

  /* RFC3986 S3.3: The path is terminated by the first question mark ("?")
     or number sign ("#") character, or by the end of the URI.
  */
path:
  switch (*ptr) {
  case '?':
    goto query;
  case '#':
    goto fragment;
  case '\0':
    goto end;
  default:
    break;
  }
  out->path.buf = ptr;
  out->path.len = 0;
  for (u8 c = 0; (c = *ptr) != '\0'; ++ptr) {
    switch (c) {
    case '?':
      goto query;
    case '#':
      goto fragment;
    default:
      ++out->path.len;
    }
  }

  /* RFC3986 S3.4: The query component is indicated by the first question
     mark ("?") character and terminated by a number sign ("#") character
     or by the end of the URI.
  */
query:
  if (*ptr == '?') {
    out->query.buf = ++ptr;
    for (u8 c = 0; (c = *ptr) != '\0'; ++ptr) {
      if (c == '#') {
        goto fragment;
      }
      ++out->query.len;
    }
  }

  /* RFC3986 S3.5: A fragment identifier component is indicated by the
     presence of a number sign ("#") character and terminated by the end
     of the URI.
  */
fragment:
  if (*ptr == '#') {
    out->fragment.buf = ptr;
    while (*ptr++ != '\0') {
      ++out->fragment.len;
    }
  }

end:
  return SERD_SUCCESS;
}

/**
   Remove leading dot components from `path`.
   See http://tools.ietf.org/html/rfc3986#section-5.2.3
   @param up Set to the number of up-references (e.g. "../") trimmed
   @return A pointer to the new start of `path`
*/
static u8k*
remove_dot_segments(u8k* path, size_t len, size_t* up)
{
  u8k*       begin = path;
  u8k* const end   = path + len;

  *up = 0;
  while (begin < end) {
    switch (begin[0]) {
    case '.':
      switch (begin[1]) {
      case '/':
        begin += 2; // Chop leading "./"
        break;
      case '.':
        switch (begin[2]) {
        case '\0':
          ++*up;
          begin += 2; // Chop input ".."
          break;
        case '/':
          ++*up;
          begin += 3; // Chop leading "../"
          break;
        default:
          return begin;
        }
        break;
      case '\0':
        return ++begin; // Chop input "."
      default:
        return begin;
      }
      break;

    case '/':
      switch (begin[1]) {
      case '.':
        switch (begin[2]) {
        case '/':
          begin += 2; // Leading "/./" => "/"
          break;
        case '.':
          switch (begin[3]) {
          case '/':
            ++*up;
            begin += 3; // Leading "/../" => "/"
          }
          break;
        default:
          return begin;
        }
      }
      return begin;

    default:
      return begin; // Finished chopping dot components
    }
  }

  return begin;
}

/// Merge `base` and `path` in-place
static z0
merge(SerdChunk* base, SerdChunk* path)
{
  size_t         up    = 0;
  u8k* begin = remove_dot_segments(path->buf, path->len, &up);
  u8k* end   = path->buf + path->len;

  if (base->len) {
    // Find the up'th last slash
    u8k* base_last = (base->buf + base->len - 1);
    ++up;
    do {
      if (*base_last == '/') {
        --up;
      }
    } while (up > 0 && (--base_last > base->buf));

    // Set path prefix
    base->len = (size_t)(base_last - base->buf + 1);
  }

  // Set path suffix
  path->buf = begin;
  path->len = (size_t)(end - begin);
}

/// See http://tools.ietf.org/html/rfc3986#section-5.2.2
z0
serd_uri_resolve(const SerdURI* r, const SerdURI* base, SerdURI* t)
{
  if (!base->scheme.len) {
    *t = *r; // Don't resolve against non-absolute URIs
    return;
  }

  t->path_base.buf = NULL;
  t->path_base.len = 0;
  if (r->scheme.len) {
    *t = *r;
  } else {
    if (r->authority.len) {
      t->authority = r->authority;
      t->path      = r->path;
      t->query     = r->query;
    } else {
      t->path = r->path;
      if (!r->path.len) {
        t->path_base = base->path;
        if (r->query.len) {
          t->query = r->query;
        } else {
          t->query = base->query;
        }
      } else {
        if (r->path.buf[0] != '/') {
          t->path_base = base->path;
        }
        merge(&t->path_base, &t->path);
        t->query = r->query;
      }
      t->authority = base->authority;
    }
    t->scheme   = base->scheme;
    t->fragment = r->fragment;
  }
}

/** Write the path of `uri` starting at index `i` */
static size_t
write_path_tail(SerdSink sink, uk stream, const SerdURI* uri, size_t i)
{
  size_t len = 0;
  if (i < uri->path_base.len) {
    len += sink(uri->path_base.buf + i, uri->path_base.len - i, stream);
  }

  if (uri->path.buf) {
    if (i < uri->path_base.len) {
      len += sink(uri->path.buf, uri->path.len, stream);
    } else {
      const size_t j = (i - uri->path_base.len);
      len += sink(uri->path.buf + j, uri->path.len - j, stream);
    }
  }

  return len;
}

/** Write the path of `uri` relative to the path of `base`. */
static size_t
write_rel_path(SerdSink       sink,
               uk          stream,
               const SerdURI* uri,
               const SerdURI* base)
{
  const size_t path_len = uri_path_len(uri);
  const size_t base_len = uri_path_len(base);
  const size_t min_len  = (path_len < base_len) ? path_len : base_len;

  // Find the last separator common to both paths
  size_t last_shared_sep = 0;
  size_t i               = 0;
  for (; i < min_len && uri_path_at(uri, i) == uri_path_at(base, i); ++i) {
    if (uri_path_at(uri, i) == '/') {
      last_shared_sep = i;
    }
  }

  if (i == path_len && i == base_len) { // Paths are identical
    return 0;
  }

  // Find the number of up references ("..") required
  size_t up = 0;
  for (size_t s = last_shared_sep + 1; s < base_len; ++s) {
    if (uri_path_at(base, s) == '/') {
      ++up;
    }
  }

  // Write up references
  size_t len = 0;
  for (size_t u = 0; u < up; ++u) {
    len += sink("../", 3, stream);
  }

  if (last_shared_sep == 0 && up == 0) {
    len += sink("/", 1, stream);
  }

  // Write suffix
  return len + write_path_tail(sink, stream, uri, last_shared_sep + 1);
}

static u8
serd_uri_path_starts_without_slash(const SerdURI* uri)
{
  return ((uri->path_base.len || uri->path.len) &&
          ((!uri->path_base.len || uri->path_base.buf[0] != '/') &&
           (!uri->path.len || uri->path.buf[0] != '/')));
}

/// See http://tools.ietf.org/html/rfc3986#section-5.3
size_t
serd_uri_serialise_relative(const SerdURI* uri,
                            const SerdURI* base,
                            const SerdURI* root,
                            SerdSink       sink,
                            uk          stream)
{
  size_t     len = 0;
  const b8 relative =
    root ? uri_is_under(uri, root) : uri_is_related(uri, base);

  if (relative) {
    len = write_rel_path(sink, stream, uri, base);
  }

  if (!relative || (!len && base->query.buf)) {
    if (uri->scheme.buf) {
      len += sink(uri->scheme.buf, uri->scheme.len, stream);
      len += sink(":", 1, stream);
    }
    if (uri->authority.buf) {
      len += sink("//", 2, stream);
      len += sink(uri->authority.buf, uri->authority.len, stream);
      if (uri->authority.len > 0 &&
          uri->authority.buf[uri->authority.len - 1] != '/' &&
          serd_uri_path_starts_without_slash(uri)) {
        // Special case: ensure path begins with a slash
        // https://tools.ietf.org/html/rfc3986#section-3.2
        len += sink("/", 1, stream);
      }
    }
    len += write_path_tail(sink, stream, uri, 0);
  }

  if (uri->query.buf) {
    len += sink("?", 1, stream);
    len += sink(uri->query.buf, uri->query.len, stream);
  }

  if (uri->fragment.buf) {
    // Note uri->fragment.buf includes the leading `#'
    len += sink(uri->fragment.buf, uri->fragment.len, stream);
  }

  return len;
}

/// See http://tools.ietf.org/html/rfc3986#section-5.3
size_t
serd_uri_serialise(const SerdURI* uri, SerdSink sink, uk stream)
{
  return serd_uri_serialise_relative(uri, NULL, NULL, sink, stream);
}
