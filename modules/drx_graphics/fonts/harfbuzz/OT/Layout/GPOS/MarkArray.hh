#ifndef OT_LAYOUT_GPOS_MARKARRAY_HH
#define OT_LAYOUT_GPOS_MARKARRAY_HH

#include "AnchorMatrix.hh"
#include "MarkRecord.hh"

namespace OT {
namespace Layout {
namespace GPOS_impl {

struct MarkArray : Array16Of<MarkRecord>        /* Array of MarkRecords--in Coverage order */
{
  b8 sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (Array16Of<MarkRecord>::sanitize (c, this));
  }

  b8 apply (hb_ot_apply_context_t *c,
              u32 mark_index, u32 glyph_index,
              const AnchorMatrix &anchors, u32 class_count,
              u32 glyph_pos) const
  {
    TRACE_APPLY (this);
    hb_buffer_t *buffer = c->buffer;
    const MarkRecord &record = Array16Of<MarkRecord>::operator[](mark_index);
    u32 mark_class = record.klass;

    const Anchor& mark_anchor = this + record.markAnchor;
    b8 found;
    const Anchor& glyph_anchor = anchors.get_anchor (c, glyph_index, mark_class, class_count, &found);
    /* If this subtable doesn't have an anchor for this base and this class,
     * return false such that the subsequent subtables have a chance at it. */
    if (unlikely (!found)) return_trace (false);

    f32 mark_x, mark_y, base_x, base_y;

    buffer->unsafe_to_break (glyph_pos, buffer->idx + 1);
    mark_anchor.get_anchor (c, buffer->cur().codepoint, &mark_x, &mark_y);
    glyph_anchor.get_anchor (c, buffer->info[glyph_pos].codepoint, &base_x, &base_y);

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "attaching mark glyph at %u to glyph at %u",
			  c->buffer->idx, glyph_pos);
    }

    hb_glyph_position_t &o = buffer->cur_pos();
    o.x_offset = roundf (base_x - mark_x);
    o.y_offset = roundf (base_y - mark_y);
    o.attach_type() = ATTACH_TYPE_MARK;
    o.attach_chain() = (i32) glyph_pos - (i32) buffer->idx;
    buffer->scratch_flags |= HB_BUFFER_SCRATCH_FLAG_HAS_GPOS_ATTACHMENT;

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "attached mark glyph at %u to glyph at %u",
			  c->buffer->idx, glyph_pos);
    }

    buffer->idx++;
    return_trace (true);
  }

  template <typename Iterator,
      hb_requires (hb_is_iterator (Iterator))>
  b8 subset (hb_subset_context_t *c,
               Iterator             coverage,
               const hb_map_t      *klass_mapping) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->glyphset_gsub ();

    auto* out = c->serializer->start_embed (this);
    if (unlikely (!c->serializer->extend_min (out))) return_trace (false);

    auto mark_iter =
    + hb_zip (coverage, this->iter ())
    | hb_filter (glyphset, hb_first)
    | hb_map (hb_second)
    ;

    b8 ret = false;
    u32 new_length = 0;
    for (const auto& mark_record : mark_iter) {
      ret |= mark_record.subset (c, this, klass_mapping);
      new_length++;
    }

    if (unlikely (!c->serializer->check_assign (out->len, new_length,
                                                HB_SERIALIZE_ERROR_ARRAY_OVERFLOW)))
      return_trace (false);

    return_trace (ret);
  }
};

HB_INTERNAL inline
z0 Markclass_closure_and_remap_indexes (const Coverage  &mark_coverage,
                                          const MarkArray &mark_array,
                                          const hb_set_t  &glyphset,
                                          hb_map_t*        klass_mapping /* INOUT */)
{
  hb_set_t orig_classes;

  + hb_zip (mark_coverage, mark_array)
  | hb_filter (glyphset, hb_first)
  | hb_map (hb_second)
  | hb_map (&MarkRecord::get_class)
  | hb_sink (orig_classes)
  ;

  u32 idx = 0;
  for (auto klass : orig_classes.iter ())
  {
    if (klass_mapping->has (klass)) continue;
    klass_mapping->set (klass, idx);
    idx++;
  }
}

}
}
}

#endif /* OT_LAYOUT_GPOS_MARKARRAY_HH */
