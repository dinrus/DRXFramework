/*
 * Copyright © 2010  Google, Inc.
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

#ifndef HB_OT_SHAPE_HH
#define HB_OT_SHAPE_HH

#include "hb.hh"

#include "hb-ot-map.hh"
#include "hb-aat-map.hh"


struct hb_ot_shape_plan_key_t
{
  u32 variations_index[2];

  z0 init (hb_face_t *face,
	     i32k *coords,
	     u32   num_coords)
  {
    for (u32 table_index = 0; table_index < 2; table_index++)
      hb_ot_layout_table_find_feature_variations (face,
						  table_tags[table_index],
						  coords,
						  num_coords,
						  &variations_index[table_index]);
  }

  b8 equal (const hb_ot_shape_plan_key_t *other)
  {
    return 0 == hb_memcmp (this, other, sizeof (*this));
  }
};


struct hb_shape_plan_key_t;

struct hb_ot_shape_plan_t
{
  ~hb_ot_shape_plan_t () { fini (); }

  hb_segment_properties_t props;
  const struct hb_ot_shaper_t *shaper;
  hb_ot_map_t map;
  ukk data;
#ifndef HB_NO_OT_SHAPE_FRACTIONS
  hb_mask_t frac_mask, numr_mask, dnom_mask;
#else
  static constexpr hb_mask_t frac_mask = 0;
  static constexpr hb_mask_t numr_mask = 0;
  static constexpr hb_mask_t dnom_mask = 0;
#endif
  hb_mask_t rtlm_mask;
#ifndef HB_NO_OT_KERN
  hb_mask_t kern_mask;
#else
  static constexpr hb_mask_t kern_mask = 0;
#endif
#ifndef HB_NO_AAT_SHAPE
  hb_mask_t trak_mask;
#else
  static constexpr hb_mask_t trak_mask = 0;
#endif

#ifndef HB_NO_OT_KERN
  b8 requested_kerning : 1;
#else
  static constexpr b8 requested_kerning = false;
#endif
#ifndef HB_NO_AAT_SHAPE
  b8 requested_tracking : 1;
#else
  static constexpr b8 requested_tracking = false;
#endif
#ifndef HB_NO_OT_SHAPE_FRACTIONS
  b8 has_frac : 1;
#else
  static constexpr b8 has_frac = false;
#endif
  b8 has_vert : 1;
  b8 has_gpos_mark : 1;
  b8 zero_marks : 1;
  b8 fallback_glyph_classes : 1;
  b8 fallback_mark_positioning : 1;
  b8 adjust_mark_positioning_when_zeroing : 1;

  b8 apply_gpos : 1;
#ifndef HB_NO_OT_KERN
  b8 apply_kern : 1;
#else
  static constexpr b8 apply_kern = false;
#endif
  b8 apply_fallback_kern : 1;
#ifndef HB_NO_AAT_SHAPE
  b8 apply_kerx : 1;
  b8 apply_morx : 1;
  b8 apply_trak : 1;
#else
  static constexpr b8 apply_kerx = false;
  static constexpr b8 apply_morx = false;
  static constexpr b8 apply_trak = false;
#endif

  z0 collect_lookups (hb_tag_t table_tag, hb_set_t *lookups) const
  {
    u32 table_index;
    switch (table_tag) {
      case HB_OT_TAG_GSUB: table_index = 0; break;
      case HB_OT_TAG_GPOS: table_index = 1; break;
      default: return;
    }
    map.collect_lookups (table_index, lookups);
  }

  HB_INTERNAL b8 init0 (hb_face_t                     *face,
			  const hb_shape_plan_key_t     *key);
  HB_INTERNAL z0 fini ();

  HB_INTERNAL z0 substitute (hb_font_t *font, hb_buffer_t *buffer) const;
  HB_INTERNAL z0 position (hb_font_t *font, hb_buffer_t *buffer) const;
};

struct hb_shape_plan_t;

struct hb_ot_shape_planner_t
{
  /* In the order that they are filled in. */
  hb_face_t *face;
  hb_segment_properties_t props;
  hb_ot_map_builder_t map;
#ifndef HB_NO_AAT_SHAPE
  b8 apply_morx : 1;
#else
  static constexpr b8 apply_morx = false;
#endif
  b8 script_zero_marks : 1;
  b8 script_fallback_mark_positioning : 1;
  const struct hb_ot_shaper_t *shaper;

  HB_INTERNAL hb_ot_shape_planner_t (hb_face_t                     *face,
				     const hb_segment_properties_t &props);

  HB_INTERNAL z0 compile (hb_ot_shape_plan_t           &plan,
			    const hb_ot_shape_plan_key_t &key);
};


#endif /* HB_OT_SHAPE_HH */
