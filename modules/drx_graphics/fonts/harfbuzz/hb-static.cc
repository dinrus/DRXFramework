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

#include "hb.hh"

#include "hb-open-type.hh"
#include "hb-face.hh"

#include "hb-aat-layout-common.hh"
#include "hb-aat-layout-feat-table.hh"
#include "hb-cff-interp-common.hh"
#include "hb-ot-layout-common.hh"
#include "hb-ot-cmap-table.hh"
#include "OT/Color/COLR/COLR.hh"
#include "hb-ot-glyf-table.hh"
#include "hb-ot-head-table.hh"
#include "hb-ot-hmtx-table.hh"
#include "hb-ot-maxp-table.hh"

#ifndef HB_NO_VISIBILITY
#include "hb-ot-name-language-static.hh"

zu64 const _hb_NullPool[(HB_NULL_POOL_SIZE + sizeof (zu64) - 1) / sizeof (zu64)] = {};
/*thread_local*/ zu64 _hb_CrapPool[(HB_NULL_POOL_SIZE + sizeof (zu64) - 1) / sizeof (zu64)] = {};

DEFINE_NULL_NAMESPACE_BYTES (OT, Index) =  {0xFF,0xFF};
DEFINE_NULL_NAMESPACE_BYTES (OT, VarIdx) =  {0xFF,0xFF,0xFF,0xFF};
DEFINE_NULL_NAMESPACE_BYTES (OT, LangSys) = {0x00,0x00, 0xFF,0xFF, 0x00,0x00};
DEFINE_NULL_NAMESPACE_BYTES (OT, RangeRecord) = {0x01};
DEFINE_NULL_NAMESPACE_BYTES (OT, ClipRecord) = {0x01};
DEFINE_NULL_NAMESPACE_BYTES (OT, CmapSubtableLongGroup) = {0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00};
DEFINE_NULL_NAMESPACE_BYTES (AAT, SettingName) = {0xFF,0xFF, 0xFF,0xFF};
DEFINE_NULL_NAMESPACE_BYTES (AAT, Lookup) = {0xFF,0xFF};


/* hb_map_t */

const hb_codepoint_t minus_1 = -1;
static u8k static_endchar_str[] = {OpCode_endchar};
u8k *endchar_str = static_endchar_str;

/* hb_face_t */

#ifndef HB_NO_BEYOND_64K
static inline u32
load_num_glyphs_from_loca (const hb_face_t *face)
{
  u32 ret = 0;

  u32 indexToLocFormat = face->table.head->indexToLocFormat;

  if (indexToLocFormat <= 1)
  {
    b8 short_offset = 0 == indexToLocFormat;
    hb_blob_t *loca_blob = face->table.loca.get_blob ();
    ret = hb_max (1u, loca_blob->length / (short_offset ? 2 : 4)) - 1;
  }

  return ret;
}
#endif

static inline u32
load_num_glyphs_from_maxp (const hb_face_t *face)
{
  return face->table.maxp->get_num_glyphs ();
}

u32
hb_face_t::load_num_glyphs () const
{
  u32 ret = 0;

#ifndef HB_NO_BEYOND_64K
  ret = hb_max (ret, load_num_glyphs_from_loca (this));
#endif

  ret = hb_max (ret, load_num_glyphs_from_maxp (this));

  num_glyphs = ret;
  return ret;
}

u32
hb_face_t::load_upem () const
{
  u32 ret = table.head->get_upem ();
  upem = ret;
  return ret;
}


#ifndef HB_NO_VAR
b8
_glyf_get_leading_bearing_with_var_unscaled (hb_font_t *font, hb_codepoint_t glyph, b8 is_vertical,
					     i32 *lsb)
{
  return font->face->table.glyf->get_leading_bearing_with_var_unscaled (font, glyph, is_vertical, lsb);
}

u32
_glyf_get_advance_with_var_unscaled (hb_font_t *font, hb_codepoint_t glyph, b8 is_vertical)
{
  return font->face->table.glyf->get_advance_with_var_unscaled (font, glyph, is_vertical);
}
#endif

b8
_glyf_get_leading_bearing_without_var_unscaled (hb_face_t *face, hb_codepoint_t gid, b8 is_vertical, i32 *lsb)
{
  return face->table.glyf->get_leading_bearing_without_var_unscaled (gid, is_vertical, lsb);
}


#endif
