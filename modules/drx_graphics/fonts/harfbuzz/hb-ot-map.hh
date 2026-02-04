/*
 * Copyright © 2009,2010  Red Hat, Inc.
 * Copyright © 2010,2011,2012,2013  Google, Inc.
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
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_OT_MAP_HH
#define HB_OT_MAP_HH

#include "hb-buffer.hh"


#define HB_OT_MAP_MAX_BITS 8u
#define HB_OT_MAP_MAX_VALUE ((1u << HB_OT_MAP_MAX_BITS) - 1u)

struct hb_ot_shape_plan_t;

static const hb_tag_t table_tags[2] = {HB_OT_TAG_GSUB, HB_OT_TAG_GPOS};

struct hb_ot_map_t
{
  friend struct hb_ot_map_builder_t;

  public:

  struct feature_map_t {
    hb_tag_t tag; /* should be first for our bsearch to work */
    u32 index[2]; /* GSUB/GPOS */
    u32 stage[2]; /* GSUB/GPOS */
    u32 shift;
    hb_mask_t mask;
    hb_mask_t _1_mask; /* mask for value=1, for quick access */
    u32 needs_fallback : 1;
    u32 auto_zwnj : 1;
    u32 auto_zwj : 1;
    u32 random : 1;
    u32 per_syllable : 1;

    i32 cmp (const hb_tag_t tag_) const
    { return tag_ < tag ? -1 : tag_ > tag ? 1 : 0; }

    HB_INTERNAL static i32 cmp (ukk pa, ukk pb)
    {
      const feature_map_t *a = (const feature_map_t *) pa;
      const feature_map_t *b = (const feature_map_t *) pb;
      return a->tag < b->tag ? -1 : a->tag > b->tag ? 1 : 0;
    }
  };

  struct lookup_map_t {
    u16 index;
    u16 auto_zwnj : 1;
    u16 auto_zwj : 1;
    u16 random : 1;
    u16 per_syllable : 1;
    hb_mask_t mask;
    hb_tag_t feature_tag;

    HB_INTERNAL static i32 cmp (ukk pa, ukk pb)
    {
      const lookup_map_t *a = (const lookup_map_t *) pa;
      const lookup_map_t *b = (const lookup_map_t *) pb;
      return a->index < b->index ? -1 : a->index > b->index ? 1 : 0;
    }
  };

  /* Pause functions return true if new glyph indices might have been
   * added to the buffer.  This is used to update buffer digest. */
  typedef b8 (*pause_func_t) (const struct hb_ot_shape_plan_t *plan, hb_font_t *font, hb_buffer_t *buffer);

  struct stage_map_t {
    u32 last_lookup; /* Cumulative */
    pause_func_t pause_func;
  };

  z0 init ()
  {
    hb_memset (this, 0, sizeof (*this));

    features.init0 ();
    for (u32 table_index = 0; table_index < 2; table_index++)
    {
      lookups[table_index].init0 ();
      stages[table_index].init0 ();
    }
  }
  z0 fini ()
  {
    features.fini ();
    for (u32 table_index = 0; table_index < 2; table_index++)
    {
      lookups[table_index].fini ();
      stages[table_index].fini ();
    }
  }

  hb_mask_t get_global_mask () const { return global_mask; }

  hb_mask_t get_mask (hb_tag_t feature_tag, u32 *shift = nullptr) const
  {
    const feature_map_t *map = features.bsearch (feature_tag);
    if (shift) *shift = map ? map->shift : 0;
    return map ? map->mask : 0;
  }

  b8 needs_fallback (hb_tag_t feature_tag) const
  {
    const feature_map_t *map = features.bsearch (feature_tag);
    return map ? map->needs_fallback : false;
  }

  hb_mask_t get_1_mask (hb_tag_t feature_tag) const
  {
    const feature_map_t *map = features.bsearch (feature_tag);
    return map ? map->_1_mask : 0;
  }

  u32 get_feature_index (u32 table_index, hb_tag_t feature_tag) const
  {
    const feature_map_t *map = features.bsearch (feature_tag);
    return map ? map->index[table_index] : HB_OT_LAYOUT_NO_FEATURE_INDEX;
  }

  u32 get_feature_stage (u32 table_index, hb_tag_t feature_tag) const
  {
    const feature_map_t *map = features.bsearch (feature_tag);
    return map ? map->stage[table_index] : UINT_MAX;
  }

  hb_array_t<const hb_ot_map_t::lookup_map_t>
  get_stage_lookups (u32 table_index, u32 stage) const
  {
    if (unlikely (stage > stages[table_index].length))
      return hb_array<const hb_ot_map_t::lookup_map_t> (nullptr, 0);

    u32 start = stage ? stages[table_index][stage - 1].last_lookup : 0;
    u32 end   = stage < stages[table_index].length ? stages[table_index][stage].last_lookup : lookups[table_index].length;
    return lookups[table_index].as_array ().sub_array (start, end - start);
  }

  HB_INTERNAL z0 collect_lookups (u32 table_index, hb_set_t *lookups) const;
  template <typename Proxy>
  HB_INTERNAL z0 apply (const Proxy &proxy,
			  const struct hb_ot_shape_plan_t *plan, hb_font_t *font, hb_buffer_t *buffer) const;
  HB_INTERNAL z0 substitute (const struct hb_ot_shape_plan_t *plan, hb_font_t *font, hb_buffer_t *buffer) const;
  HB_INTERNAL z0 position (const struct hb_ot_shape_plan_t *plan, hb_font_t *font, hb_buffer_t *buffer) const;

  public:
  hb_tag_t chosen_script[2];
  b8 found_script[2];

  private:

  hb_mask_t global_mask = 0;

  hb_sorted_vector_t<feature_map_t> features;
  hb_vector_t<lookup_map_t> lookups[2]; /* GSUB/GPOS */
  hb_vector_t<stage_map_t> stages[2]; /* GSUB/GPOS */
};

enum hb_ot_map_feature_flags_t
{
  F_NONE		= 0x0000u,
  F_GLOBAL		= 0x0001u, /* Feature applies to all characters; results in no mask allocated for it. */
  F_HAS_FALLBACK	= 0x0002u, /* Has fallback implementation, so include mask bit even if feature not found. */
  F_MANUAL_ZWNJ		= 0x0004u, /* Don't skip over ZWNJ when matching **context**. */
  F_MANUAL_ZWJ		= 0x0008u, /* Don't skip over ZWJ when matching **input**. */
  F_MANUAL_JOINERS	= F_MANUAL_ZWNJ | F_MANUAL_ZWJ,
  F_GLOBAL_MANUAL_JOINERS= F_GLOBAL | F_MANUAL_JOINERS,
  F_GLOBAL_HAS_FALLBACK = F_GLOBAL | F_HAS_FALLBACK,
  F_GLOBAL_SEARCH	= 0x0010u, /* If feature not found in LangSys, look for it in global feature list and pick one. */
  F_RANDOM		= 0x0020u, /* Randomly select a glyph from an AlternateSubstFormat1 subtable. */
  F_PER_SYLLABLE	= 0x0040u  /* Contain lookup application to within syllable. */
};
HB_MARK_AS_FLAG_T (hb_ot_map_feature_flags_t);


struct hb_ot_map_feature_t
{
  hb_tag_t tag;
  hb_ot_map_feature_flags_t flags;
};

struct hb_ot_shape_plan_key_t;

struct hb_ot_map_builder_t
{
  public:

  HB_INTERNAL hb_ot_map_builder_t (hb_face_t *face_,
				   const hb_segment_properties_t &props_);

  HB_INTERNAL ~hb_ot_map_builder_t ();

  HB_INTERNAL z0 add_feature (hb_tag_t tag,
				hb_ot_map_feature_flags_t flags=F_NONE,
				u32 value=1);

  HB_INTERNAL b8 has_feature (hb_tag_t tag);

  z0 add_feature (const hb_ot_map_feature_t &feat)
  { add_feature (feat.tag, feat.flags); }

  z0 enable_feature (hb_tag_t tag,
		       hb_ot_map_feature_flags_t flags=F_NONE,
		       u32 value=1)
  { add_feature (tag, F_GLOBAL | flags, value); }

  z0 disable_feature (hb_tag_t tag)
  { add_feature (tag, F_GLOBAL, 0); }

  z0 add_gsub_pause (hb_ot_map_t::pause_func_t pause_func)
  { add_pause (0, pause_func); }
  z0 add_gpos_pause (hb_ot_map_t::pause_func_t pause_func)
  { add_pause (1, pause_func); }

  HB_INTERNAL z0 compile (hb_ot_map_t                  &m,
			    const hb_ot_shape_plan_key_t &key);

  private:

  HB_INTERNAL z0 add_lookups (hb_ot_map_t  &m,
				u32  table_index,
				u32  feature_index,
				u32  variations_index,
				hb_mask_t     mask,
				b8          auto_zwnj = true,
				b8          auto_zwj = true,
				b8          random = false,
				b8          per_syllable = false,
				hb_tag_t      feature_tag = HB_TAG(' ',' ',' ',' '));

  struct feature_info_t {
    hb_tag_t tag;
    u32 seq; /* sequence#, used for stable sorting only */
    u32 max_value;
    hb_ot_map_feature_flags_t flags;
    u32 default_value; /* for non-global features, what should the unset glyphs take */
    u32 stage[2]; /* GSUB/GPOS */

    HB_INTERNAL static i32 cmp (ukk pa, ukk pb)
    {
      const feature_info_t *a = (const feature_info_t *) pa;
      const feature_info_t *b = (const feature_info_t *) pb;
      return (a->tag != b->tag) ?  (a->tag < b->tag ? -1 : 1) :
	     (a->seq < b->seq ? -1 : a->seq > b->seq ? 1 : 0);
    }
  };

  struct stage_info_t {
    u32 index;
    hb_ot_map_t::pause_func_t pause_func;
  };

  HB_INTERNAL z0 add_pause (u32 table_index, hb_ot_map_t::pause_func_t pause_func);

  public:

  hb_face_t *face;
  hb_segment_properties_t props;
  b8 is_simple;

  hb_tag_t chosen_script[2];
  b8 found_script[2];
  u32 script_index[2], language_index[2];

  private:

  u32 current_stage[2]; /* GSUB/GPOS */
  hb_vector_t<feature_info_t> feature_infos;
  hb_vector_t<stage_info_t> stages[2]; /* GSUB/GPOS */
};



#endif /* HB_OT_MAP_HH */
