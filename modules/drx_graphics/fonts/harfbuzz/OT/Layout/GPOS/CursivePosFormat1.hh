#ifndef OT_LAYOUT_GPOS_CURSIVEPOSFORMAT1_HH
#define OT_LAYOUT_GPOS_CURSIVEPOSFORMAT1_HH

#include "Anchor.hh"

namespace OT {
namespace Layout {
namespace GPOS_impl {

struct EntryExitRecord
{
  friend struct CursivePosFormat1;

  b8 sanitize (hb_sanitize_context_t *c, const struct CursivePosFormat1 *base) const
  {
    TRACE_SANITIZE (this);
    return_trace (entryAnchor.sanitize (c, base) && exitAnchor.sanitize (c, base));
  }

  z0 collect_variation_indices (hb_collect_variation_indices_context_t *c,
                                  const struct CursivePosFormat1 *src_base) const
  {
    (src_base+entryAnchor).collect_variation_indices (c);
    (src_base+exitAnchor).collect_variation_indices (c);
  }

  b8 subset (hb_subset_context_t *c,
	       const struct CursivePosFormat1 *src_base) const
  {
    TRACE_SERIALIZE (this);
    auto *out = c->serializer->embed (this);
    if (unlikely (!out)) return_trace (false);

    b8 ret = false;
    ret |= out->entryAnchor.serialize_subset (c, entryAnchor, src_base);
    ret |= out->exitAnchor.serialize_subset (c, exitAnchor, src_base);
    return_trace (ret);
  }

  protected:
  Offset16To<Anchor, struct CursivePosFormat1>
                entryAnchor;            /* Offset to EntryAnchor table--from
                                         * beginning of CursivePos
                                         * subtable--may be NULL */
  Offset16To<Anchor, struct CursivePosFormat1>
                exitAnchor;             /* Offset to ExitAnchor table--from
                                         * beginning of CursivePos
                                         * subtable--may be NULL */
  public:
  DEFINE_SIZE_STATIC (4);
};

static z0
reverse_cursive_minor_offset (hb_glyph_position_t *pos, u32 i, hb_direction_t direction, u32 new_parent) {
  i32 chain = pos[i].attach_chain(), type = pos[i].attach_type();
  if (likely (!chain || 0 == (type & ATTACH_TYPE_CURSIVE)))
    return;

  pos[i].attach_chain() = 0;

  u32 j = (i32) i + chain;

  /* Stop if we see new parent in the chain. */
  if (j == new_parent)
    return;

  reverse_cursive_minor_offset (pos, j, direction, new_parent);

  if (HB_DIRECTION_IS_HORIZONTAL (direction))
    pos[j].y_offset = -pos[i].y_offset;
  else
    pos[j].x_offset = -pos[i].x_offset;

  pos[j].attach_chain() = -chain;
  pos[j].attach_type() = type;
}


struct CursivePosFormat1
{
  protected:
  HBUINT16      format;                 /* Format identifier--format = 1 */
  Offset16To<Coverage>
                coverage;               /* Offset to Coverage table--from
                                         * beginning of subtable */
  Array16Of<EntryExitRecord>
                entryExitRecord;        /* Array of EntryExit records--in
                                         * Coverage Index order */
  public:
  DEFINE_SIZE_ARRAY (6, entryExitRecord);

  b8 sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    if (unlikely (!coverage.sanitize (c, this)))
      return_trace (false);

    if (c->lazy_some_gpos)
      return_trace (entryExitRecord.sanitize_shallow (c));
    else
      return_trace (entryExitRecord.sanitize (c, this));
  }

  b8 intersects (const hb_set_t *glyphs) const
  { return (this+coverage).intersects (glyphs); }

  z0 closure_lookups (hb_closure_lookups_context_t *c) const {}

  z0 collect_variation_indices (hb_collect_variation_indices_context_t *c) const
  {
    + hb_zip (this+coverage, entryExitRecord)
    | hb_filter (c->glyph_set, hb_first)
    | hb_map (hb_second)
    | hb_apply ([&] (const EntryExitRecord& record) { record.collect_variation_indices (c, this); })
    ;
  }

  z0 collect_glyphs (hb_collect_glyphs_context_t *c) const
  { if (unlikely (!(this+coverage).collect_coverage (c->input))) return; }

  const Coverage &get_coverage () const { return this+coverage; }

  b8 apply (hb_ot_apply_context_t *c) const
  {
    TRACE_APPLY (this);
    hb_buffer_t *buffer = c->buffer;

    const EntryExitRecord &this_record = entryExitRecord[(this+coverage).get_coverage  (buffer->cur().codepoint)];
    if (!this_record.entryAnchor ||
	unlikely (!this_record.entryAnchor.sanitize (&c->sanitizer, this))) return_trace (false);
    hb_barrier ();

    hb_ot_apply_context_t::skipping_iterator_t &skippy_iter = c->iter_input;
    skippy_iter.reset_fast (buffer->idx);
    u32 unsafe_from;
    if (unlikely (!skippy_iter.prev (&unsafe_from)))
    {
      buffer->unsafe_to_concat_from_outbuffer (unsafe_from, buffer->idx + 1);
      return_trace (false);
    }

    const EntryExitRecord &prev_record = entryExitRecord[(this+coverage).get_coverage  (buffer->info[skippy_iter.idx].codepoint)];
    if (!prev_record.exitAnchor ||
	unlikely (!prev_record.exitAnchor.sanitize (&c->sanitizer, this)))
    {
      buffer->unsafe_to_concat_from_outbuffer (skippy_iter.idx, buffer->idx + 1);
      return_trace (false);
    }
    hb_barrier ();

    u32 i = skippy_iter.idx;
    u32 j = buffer->idx;

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "cursive attaching glyph at %u to glyph at %u",
			  i, j);
    }

    buffer->unsafe_to_break (i, j + 1);
    f32 entry_x, entry_y, exit_x, exit_y;
    (this+prev_record.exitAnchor).get_anchor (c, buffer->info[i].codepoint, &exit_x, &exit_y);
    (this+this_record.entryAnchor).get_anchor (c, buffer->info[j].codepoint, &entry_x, &entry_y);

    hb_glyph_position_t *pos = buffer->pos;

    hb_position_t d;
    /* Main-direction adjustment */
    switch (c->direction) {
      case HB_DIRECTION_LTR:
        pos[i].x_advance  = roundf (exit_x) + pos[i].x_offset;

        d = roundf (entry_x) + pos[j].x_offset;
        pos[j].x_advance -= d;
        pos[j].x_offset  -= d;
        break;
      case HB_DIRECTION_RTL:
        d = roundf (exit_x) + pos[i].x_offset;
        pos[i].x_advance -= d;
        pos[i].x_offset  -= d;

        pos[j].x_advance  = roundf (entry_x) + pos[j].x_offset;
        break;
      case HB_DIRECTION_TTB:
        pos[i].y_advance  = roundf (exit_y) + pos[i].y_offset;

        d = roundf (entry_y) + pos[j].y_offset;
        pos[j].y_advance -= d;
        pos[j].y_offset  -= d;
        break;
      case HB_DIRECTION_BTT:
        d = roundf (exit_y) + pos[i].y_offset;
        pos[i].y_advance -= d;
        pos[i].y_offset  -= d;

        pos[j].y_advance  = roundf (entry_y);
        break;
      case HB_DIRECTION_INVALID:
      default:
        break;
    }

    /* Cross-direction adjustment */

    /* We attach child to parent (think graph theory and rooted trees whereas
     * the root stays on baseline and each node aligns itself against its
     * parent.
     *
     * Optimize things for the case of RightToLeft, as that's most common in
     * Arabic. */
    u32 child  = i;
    u32 parent = j;
    hb_position_t x_offset = roundf (entry_x - exit_x);
    hb_position_t y_offset = roundf (entry_y - exit_y);
    if  (!(c->lookup_props & LookupFlag::RightToLeft))
    {
      u32 k = child;
      child = parent;
      parent = k;
      x_offset = -x_offset;
      y_offset = -y_offset;
    }

    /* If child was already connected to someone else, walk through its old
     * chain and reverse the link direction, such that the whole tree of its
     * previous connection now attaches to new parent.  Watch out for case
     * where new parent is on the path from old chain...
     */
    reverse_cursive_minor_offset (pos, child, c->direction, parent);

    pos[child].attach_type() = ATTACH_TYPE_CURSIVE;
    pos[child].attach_chain() = (i32) parent - (i32) child;
    buffer->scratch_flags |= HB_BUFFER_SCRATCH_FLAG_HAS_GPOS_ATTACHMENT;
    if (likely (HB_DIRECTION_IS_HORIZONTAL (c->direction)))
      pos[child].y_offset = y_offset;
    else
      pos[child].x_offset = x_offset;

    /* If parent was attached to child, separate them.
     * https://github.com/harfbuzz/harfbuzz/issues/2469
     */
    if (unlikely (pos[parent].attach_chain() == -pos[child].attach_chain()))
    {
      pos[parent].attach_chain() = 0;
      if (likely (HB_DIRECTION_IS_HORIZONTAL (c->direction)))
	pos[parent].y_offset = 0;
      else
	pos[parent].x_offset = 0;
    }

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "cursive attached glyph at %u to glyph at %u",
			  i, j);
    }

    buffer->idx++;
    return_trace (true);
  }

  template <typename Iterator,
            hb_requires (hb_is_iterator (Iterator))>
  z0 serialize (hb_subset_context_t *c,
                  Iterator it,
                  const struct CursivePosFormat1 *src_base)
  {
    if (unlikely (!c->serializer->extend_min ((*this)))) return;
    this->format = 1;
    this->entryExitRecord.len = it.len ();

    for (const EntryExitRecord& entry_record : + it
                                               | hb_map (hb_second))
      entry_record.subset (c, src_base);

    auto glyphs =
    + it
    | hb_map_retains_sorting (hb_first)
    ;

    coverage.serialize_serialize (c->serializer, glyphs);
  }

  b8 subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    const hb_set_t &glyphset = *c->plan->glyphset_gsub ();
    const hb_map_t &glyph_map = *c->plan->glyph_map;

    auto *out = c->serializer->start_embed (*this);

    auto it =
    + hb_zip (this+coverage, entryExitRecord)
    | hb_filter (glyphset, hb_first)
    | hb_map_retains_sorting ([&] (hb_pair_t<hb_codepoint_t, const EntryExitRecord&> p) -> hb_pair_t<hb_codepoint_t, const EntryExitRecord&>
                              { return hb_pair (glyph_map[p.first], p.second);})
    ;

    b8 ret = b8 (it);
    out->serialize (c, it, this);
    return_trace (ret);
  }
};


}
}
}

#endif /* OT_LAYOUT_GPOS_CURSIVEPOSFORMAT1_HH */
