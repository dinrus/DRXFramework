#ifndef OT_LAYOUT_GPOS_SINGLEPOSFORMAT2_HH
#define OT_LAYOUT_GPOS_SINGLEPOSFORMAT2_HH

#include "Common.hh"

namespace OT {
namespace Layout {
namespace GPOS_impl {

struct SinglePosFormat2 : ValueBase
{
  protected:
  HBUINT16      format;                 /* Format identifier--format = 2 */
  Offset16To<Coverage>
                coverage;               /* Offset to Coverage table--from
                                         * beginning of subtable */
  ValueFormat   valueFormat;            /* Defines the types of data in the
                                         * ValueRecord */
  HBUINT16      valueCount;             /* Number of ValueRecords */
  ValueRecord   values;                 /* Array of ValueRecords--positioning
                                         * values applied to glyphs */
  public:
  DEFINE_SIZE_ARRAY (8, values);

  b8 sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) &&
                  coverage.sanitize (c, this) &&
                  valueFormat.sanitize_values (c, this, values, valueCount));
  }

  b8 intersects (const hb_set_t *glyphs) const
  { return (this+coverage).intersects (glyphs); }

  z0 closure_lookups (hb_closure_lookups_context_t *c) const {}
  z0 collect_variation_indices (hb_collect_variation_indices_context_t *c) const
  {
    if (!valueFormat.has_device ()) return;

    auto it =
    + hb_zip (this+coverage, hb_range ((u32) valueCount))
    | hb_filter (c->glyph_set, hb_first)
    ;

    if (!it) return;

    u32 sub_length = valueFormat.get_len ();
    const hb_array_t<const Value> values_array = values.as_array (valueCount * sub_length);

    for (u32 i : + it
                      | hb_map (hb_second))
      valueFormat.collect_variation_indices (c, this, values_array.sub_array (i * sub_length, sub_length));

  }

  z0 collect_glyphs (hb_collect_glyphs_context_t *c) const
  { if (unlikely (!(this+coverage).collect_coverage (c->input))) return; }

  const Coverage &get_coverage () const { return this+coverage; }

  ValueFormat get_value_format () const { return valueFormat; }

  b8 apply (hb_ot_apply_context_t *c) const
  {
    TRACE_APPLY (this);
    hb_buffer_t *buffer = c->buffer;
    u32 index = (this+coverage).get_coverage  (buffer->cur().codepoint);
    if (likely (index == NOT_COVERED)) return_trace (false);

    if (unlikely (index >= valueCount)) return_trace (false);

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "positioning glyph at %u",
			  c->buffer->idx);
    }

    valueFormat.apply_value (c, this,
                             &values[index * valueFormat.get_len ()],
                             buffer->cur_pos());

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "positioned glyph at %u",
			  c->buffer->idx);
    }

    buffer->idx++;
    return_trace (true);
  }

  b8
  position_single (hb_font_t           *font,
		   hb_blob_t           *table_blob,
		   hb_direction_t       direction,
		   hb_codepoint_t       gid,
		   hb_glyph_position_t &pos) const
  {
    u32 index = (this+coverage).get_coverage  (gid);
    if (likely (index == NOT_COVERED)) return false;
    if (unlikely (index >= valueCount)) return false;

    /* This is ugly... */
    hb_buffer_t buffer;
    buffer.props.direction = direction;
    OT::hb_ot_apply_context_t c (1, font, &buffer, table_blob);

    valueFormat.apply_value (&c, this,
                             &values[index * valueFormat.get_len ()],
                             pos);
    return true;
  }


  template<typename Iterator,
      typename SrcLookup,
      hb_requires (hb_is_iterator (Iterator))>
  z0 serialize (hb_serialize_context_t *c,
                  const SrcLookup *src,
                  Iterator it,
                  ValueFormat newFormat,
                  const hb_hashmap_t<u32, hb_pair_t<u32, i32>> *layout_variation_idx_delta_map)
  {
    auto out = c->extend_min (this);
    if (unlikely (!out)) return;
    if (unlikely (!c->check_assign (valueFormat, newFormat, HB_SERIALIZE_ERROR_INT_OVERFLOW))) return;
    if (unlikely (!c->check_assign (valueCount, it.len (), HB_SERIALIZE_ERROR_ARRAY_OVERFLOW))) return;

    + it
    | hb_map (hb_second)
    | hb_apply ([&] (hb_array_t<const Value> _)
    { src->get_value_format ().copy_values (c, newFormat, src, &_, layout_variation_idx_delta_map); })
    ;

    auto glyphs =
    + it
    | hb_map_retains_sorting (hb_first)
    ;

    coverage.serialize_serialize (c, glyphs);
  }

  template<typename Iterator,
      hb_requires (hb_is_iterator (Iterator))>
  u32 compute_effective_format (const hb_face_t *face,
                                     Iterator it,
                                     b8 is_instancing, b8 strip_hints,
                                     b8 has_gdef_varstore,
                                     const hb_hashmap_t<u32, hb_pair_t<u32, i32>> *varidx_delta_map) const
  {
    hb_blob_t* blob = hb_face_reference_table (face, HB_TAG ('f','v','a','r'));
    b8 has_fvar = (blob != hb_blob_get_empty ());
    hb_blob_destroy (blob);

    u32 new_format = 0;
    if (is_instancing)
    {
      new_format = new_format | valueFormat.get_effective_format (+ it | hb_map (hb_second), false, false, this, varidx_delta_map);
    }
    /* do not strip hints for VF */
    else if (strip_hints)
    {
      b8 strip = !has_fvar;
      if (has_fvar && !has_gdef_varstore)
        strip = true;
      new_format = new_format | valueFormat.get_effective_format (+ it | hb_map (hb_second), strip, true, this, nullptr);
    }
    else
      new_format = valueFormat;

    return new_format;
  }

  b8 subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->glyphset_gsub ();
    const hb_map_t &glyph_map = *c->plan->glyph_map;

    u32 sub_length = valueFormat.get_len ();
    auto values_array = values.as_array (valueCount * sub_length);

    auto it =
    + hb_zip (this+coverage, hb_range ((u32) valueCount))
    | hb_filter (glyphset, hb_first)
    | hb_map_retains_sorting ([&] (const hb_pair_t<hb_codepoint_t, u32>& _)
                              {
                                return hb_pair (glyph_map[_.first],
                                                values_array.sub_array (_.second * sub_length,
                                                                        sub_length));
                              })
    ;

    u32 new_format = compute_effective_format (c->plan->source, it,
                                                    b8 (c->plan->normalized_coords),
                                                    b8 (c->plan->flags & HB_SUBSET_FLAGS_NO_HINTING),
                                                    c->plan->has_gdef_varstore,
                                                    &c->plan->layout_variation_idx_delta_map);
    b8 ret = b8 (it);
    SinglePos_serialize (c->serializer, this, it, &c->plan->layout_variation_idx_delta_map, new_format);
    return_trace (ret);
  }
};


}
}
}

#endif /* OT_LAYOUT_GPOS_SINGLEPOSFORMAT2_HH */
