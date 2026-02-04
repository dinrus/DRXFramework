#ifndef OT_LAYOUT_GPOS_SINGLEPOSFORMAT1_HH
#define OT_LAYOUT_GPOS_SINGLEPOSFORMAT1_HH

#include "Common.hh"
#include "ValueFormat.hh"

namespace OT {
namespace Layout {
namespace GPOS_impl {

struct SinglePosFormat1 : ValueBase
{
  protected:
  HBUINT16      format;                 /* Format identifier--format = 1 */
  Offset16To<Coverage>
                coverage;               /* Offset to Coverage table--from
                                         * beginning of subtable */
  ValueFormat   valueFormat;            /* Defines the types of data in the
                                         * ValueRecord */
  ValueRecord   values;                 /* Defines positioning
                                         * value(s)--applied to all glyphs in
                                         * the Coverage table */
  public:
  DEFINE_SIZE_ARRAY (6, values);

  b8 sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) &&
                  coverage.sanitize (c, this) &&
		  hb_barrier () &&
                  /* The coverage  table may use a range to represent a set
                   * of glyphs, which means a small number of bytes can
                   * generate a large glyph set. Manually modify the
                   * sanitizer max ops to take this into account.
                   *
                   * Note: This check *must* be right after coverage sanitize. */
                  c->check_ops ((this + coverage).get_population () >> 1) &&
                  valueFormat.sanitize_value (c, this, values));

  }

  b8 intersects (const hb_set_t *glyphs) const
  { return (this+coverage).intersects (glyphs); }

  z0 closure_lookups (hb_closure_lookups_context_t *c) const {}
  z0 collect_variation_indices (hb_collect_variation_indices_context_t *c) const
  {
    if (!valueFormat.has_device ()) return;

    hb_set_t intersection;
    (this+coverage).intersect_set (*c->glyph_set, intersection);
    if (!intersection) return;

    valueFormat.collect_variation_indices (c, this, values.as_array (valueFormat.get_len ()));
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

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "positioning glyph at %u",
			  c->buffer->idx);
    }

    valueFormat.apply_value (c, this, values, buffer->cur_pos());

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

    /* This is ugly... */
    hb_buffer_t buffer;
    buffer.props.direction = direction;
    OT::hb_ot_apply_context_t c (1, font, &buffer, table_blob);

    valueFormat.apply_value (&c, this, values, pos);
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
    if (unlikely (!c->extend_min (this))) return;
    if (unlikely (!c->check_assign (valueFormat,
                                    newFormat,
                                    HB_SERIALIZE_ERROR_INT_OVERFLOW))) return;

    for (const hb_array_t<const Value>& _ : + it | hb_map (hb_second))
    {
      src->get_value_format ().copy_values (c, newFormat, src,  &_, layout_variation_idx_delta_map);
      // Only serialize the first entry in the iterator, the rest are assumed to
      // be the same.
      break;
    }

    auto glyphs =
    + it
    | hb_map_retains_sorting (hb_first)
    ;

    coverage.serialize_serialize (c, glyphs);
  }

  b8 subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->glyphset_gsub ();
    const hb_map_t &glyph_map = *c->plan->glyph_map;

    hb_set_t intersection;
    (this+coverage).intersect_set (glyphset, intersection);

    u32 new_format = valueFormat;

    if (c->plan->normalized_coords)
    {
      new_format = valueFormat.get_effective_format (values.arrayZ, false, false, this, &c->plan->layout_variation_idx_delta_map);
    }
    /* do not strip hints for VF */
    else if (c->plan->flags & HB_SUBSET_FLAGS_NO_HINTING)
    {
      hb_blob_t* blob = hb_face_reference_table (c->plan->source, HB_TAG ('f','v','a','r'));
      b8 has_fvar = (blob != hb_blob_get_empty ());
      hb_blob_destroy (blob);

      b8 strip = !has_fvar;
      /* special case: strip hints when a VF has no GDEF varstore after
       * subsetting*/
      if (has_fvar && !c->plan->has_gdef_varstore)
        strip = true;
      new_format = valueFormat.get_effective_format (values.arrayZ,
                                                     strip, /* strip hints */
                                                     true, /* strip empty */
                                                     this, nullptr);
    }

    auto it =
    + hb_iter (intersection)
    | hb_map_retains_sorting (glyph_map)
    | hb_zip (hb_repeat (values.as_array (valueFormat.get_len ())))
    ;

    b8 ret = b8 (it);
    SinglePos_serialize (c->serializer, this, it, &c->plan->layout_variation_idx_delta_map, new_format);
    return_trace (ret);
  }
};

}
}
}

#endif /* OT_LAYOUT_GPOS_SINGLEPOSFORMAT1_HH */
