/*
 * Copyright © 2019  Ebrahim Byagowi
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
 */

#include "hb.hh"
#include "hb-number.hh"
#include "hb-number-parser.hh"

template<typename T, typename Func>
static b8
_parse_number (const t8 **pp, const t8 *end, T *pv,
	       b8 whole_buffer, Func f)
{
  t8 buf[32];
  u32 len = hb_min (ARRAY_LENGTH (buf) - 1, (u32) (end - *pp));
  strncpy (buf, *pp, len);
  buf[len] = '\0';

  t8 *p = buf;
  t8 *pend = p;

  errno = 0;
  *pv = f (p, &pend);
  if (unlikely (errno || p == pend ||
		/* Check if consumed whole buffer if is requested */
		(whole_buffer && pend - p != end - *pp)))
    return false;

  *pp += pend - p;
  return true;
}

b8
hb_parse_int (const t8 **pp, const t8 *end, i32 *pv, b8 whole_buffer)
{
  return _parse_number<i32> (pp, end, pv, whole_buffer,
			     [] (const t8 *p, t8 **end)
			     { return strtol (p, end, 10); });
}

b8
hb_parse_uint (const t8 **pp, const t8 *end, u32 *pv,
	       b8 whole_buffer, i32 base)
{
  return _parse_number<u32> (pp, end, pv, whole_buffer,
				  [base] (const t8 *p, t8 **end)
				  { return strtoul (p, end, base); });
}

b8
hb_parse_double (const t8 **pp, const t8 *end, f64 *pv, b8 whole_buffer)
{
  const t8 *pend = end;
  *pv = strtod_rl (*pp, &pend);
  if (unlikely (*pp == pend)) return false;
  *pp = pend;
  return !whole_buffer || end == pend;
}
