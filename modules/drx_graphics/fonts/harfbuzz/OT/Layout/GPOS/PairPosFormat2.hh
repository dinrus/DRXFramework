#ifndef OT_LAYOUT_GPOS_PAIRPOSFORMAT2_HH
#define OT_LAYOUT_GPOS_PAIRPOSFORMAT2_HH

#include "ValueFormat.hh"

namespace OT {
namespace Layout {
namespace GPOS_impl {

template <typename Types>
struct PairPosFormat2_4 : ValueBase
{
  protected:
  HBUINT16      format;                 /* Format identifier--format = 2 */
  typename Types::template OffsetTo<Coverage>
                coverage;               /* Offset to Coverage table--from
                                         * beginning of subtable */
  ValueFormat   valueFormat1;           /* ValueRecord definition--for the
                                         * first glyph of the pair--may be zero
                                         * (0) */
  ValueFormat   valueFormat2;           /* ValueRecord definition--for the
                                         * second glyph of the pair--may be
                                         * zero (0) */
  typename Types::template OffsetTo<ClassDef>
                classDef1;              /* Offset to ClassDef table--from
                                         * beginning of PairPos subtable--for
                                         * the first glyph of the pair */
  typename Types::template OffsetTo<ClassDef>
                classDef2;              /* Offset to ClassDef table--from
                                         * beginning of PairPos subtable--for
                                         * the second glyph of the pair */
  HBUINT16      class1Count;            /* Number of classes in ClassDef1
                                         * table--includes Class0 */
  HBUINT16      class2Count;            /* Number of classes in ClassDef2
                                         * table--includes Class0 */
  ValueRecord   values;                 /* Matrix of value pairs:
                                         * class1-major, class2-minor,
                                         * Each entry has value1 and value2 */
  public:
  DEFINE_SIZE_ARRAY (10 + 3 * Types::size, values);

  b8 sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    if (!(c->check_struct (this)
       && coverage.sanitize (c, this)
       && classDef1.sanitize (c, this)
       && classDef2.sanitize (c, this))) return_trace (false);

    u32 len1 = valueFormat1.get_len ();
    u32 len2 = valueFormat2.get_len ();
    u32 stride = HBUINT16::static_size * (len1 + len2);
    u32 count = (u32) class1Count * (u32) class2Count;
    return_trace (c->check_range ((ukk ) values,
                                  count,
                                  stride) &&
		  (c->lazy_some_gpos ||
		   (valueFormat1.sanitize_values_stride_unsafe (c, this, &values[0], count, stride) &&
		    valueFormat2.sanitize_values_stride_unsafe (c, this, &values[len1], count, stride))));
  }

  b8 intersects (const hb_set_t *glyphs) const
  {
    return (this+coverage).intersects (glyphs) &&
           (this+classDef2).intersects (glyphs);
  }

  z0 closure_lookups (hb_closure_lookups_context_t *c) const {}
  z0 collect_variation_indices (hb_collect_variation_indices_context_t *c) const
  {
    if (!intersects (c->glyph_set)) return;
    if ((!valueFormat1.has_device ()) && (!valueFormat2.has_device ())) return;

    hb_set_t klass1_glyphs, klass2_glyphs;
    if (!(this+classDef1).collect_coverage (&klass1_glyphs)) return;
    if (!(this+classDef2).collect_coverage (&klass2_glyphs)) return;

    hb_set_t class1_set, class2_set;
    for (const u32 cp : + c->glyph_set->iter () | hb_filter (this + coverage))
    {
      if (!klass1_glyphs.has (cp)) class1_set.add (0);
      else
      {
        u32 klass1 = (this+classDef1).get (cp);
        class1_set.add (klass1);
      }
    }

    class2_set.add (0);
    for (const u32 cp : + c->glyph_set->iter () | hb_filter (klass2_glyphs))
    {
      u32 klass2 = (this+classDef2).get (cp);
      class2_set.add (klass2);
    }

    if (class1_set.is_empty ()
        || class2_set.is_empty ()
        || (class2_set.get_population() == 1 && class2_set.has(0)))
      return;

    u32 len1 = valueFormat1.get_len ();
    u32 len2 = valueFormat2.get_len ();
    const hb_array_t<const Value> values_array = values.as_array ((u32)class1Count * (u32) class2Count * (len1 + len2));
    for (const u32 class1_idx : class1_set.iter ())
    {
      for (const u32 class2_idx : class2_set.iter ())
      {
        u32 start_offset = (class1_idx * (u32) class2Count + class2_idx) * (len1 + len2);
        if (valueFormat1.has_device ())
          valueFormat1.collect_variation_indices (c, this, values_array.sub_array (start_offset, len1));

        if (valueFormat2.has_device ())
          valueFormat2.collect_variation_indices (c, this, values_array.sub_array (start_offset+len1, len2));
      }
    }
  }

  z0 collect_glyphs (hb_collect_glyphs_context_t *c) const
  {
    if (unlikely (!(this+coverage).collect_coverage (c->input))) return;
    if (unlikely (!(this+classDef2).collect_coverage (c->input))) return;
  }

  const Coverage &get_coverage () const { return this+coverage; }

  b8 apply (hb_ot_apply_context_t *c) const
  {
    TRACE_APPLY (this);
    hb_buffer_t *buffer = c->buffer;
    u32 index = (this+coverage).get_coverage  (buffer->cur().codepoint);
    if (likely (index == NOT_COVERED)) return_trace (false);

    hb_ot_apply_context_t::skipping_iterator_t &skippy_iter = c->iter_input;
    skippy_iter.reset_fast (buffer->idx);
    u32 unsafe_to;
    if (unlikely (!skippy_iter.next (&unsafe_to)))
    {
      buffer->unsafe_to_concat (buffer->idx, unsafe_to);
      return_trace (false);
    }

    u32 klass1 = (this+classDef1).get_class (buffer->cur().codepoint);
    u32 klass2 = (this+classDef2).get_class (buffer->info[skippy_iter.idx].codepoint);
    if (unlikely (klass1 >= class1Count || klass2 >= class2Count))
    {
      buffer->unsafe_to_concat (buffer->idx, skippy_iter.idx + 1);
      return_trace (false);
    }

    u32 len1 = valueFormat1.get_len ();
    u32 len2 = valueFormat2.get_len ();
    u32 record_len = len1 + len2;

    const Value *v = &values[record_len * (klass1 * class2Count + klass2)];

    b8 applied_first = false, applied_second = false;


    /* Isolate simple kerning and apply it half to each side.
     * Results in better cursor positioning / underline drawing.
     *
     * Disabled, because causes issues... :-(
     * https://github.com/harfbuzz/harfbuzz/issues/3408
     * https://github.com/harfbuzz/harfbuzz/pull/3235#issuecomment-1029814978
     */
#ifndef HB_SPLIT_KERN
    if (false)
#endif
    {
      if (!len2)
      {
        const hb_direction_t dir = buffer->props.direction;
        const b8 horizontal = HB_DIRECTION_IS_HORIZONTAL (dir);
        const b8 backward = HB_DIRECTION_IS_BACKWARD (dir);
        u32 mask = horizontal ? ValueFormat::xAdvance : ValueFormat::yAdvance;
        if (backward)
          mask |= mask >> 2; /* Add eg. xPlacement in RTL. */
        /* Add Devices. */
        mask |= mask << 4;

        if (valueFormat1 & ~mask)
          goto bail;

        /* Is simple kern. Apply value on an empty position slot,
         * then split it between sides. */

        hb_glyph_position_t pos{};
        if (valueFormat1.apply_value (c, this, v, pos))
        {
          hb_position_t *src  = &pos.x_advance;
          hb_position_t *dst1 = &buffer->cur_pos().x_advance;
          hb_position_t *dst2 = &buffer->pos[skippy_iter.idx].x_advance;
          u32 i = horizontal ? 0 : 1;

          hb_position_t kern  = src[i];
          hb_position_t kern1 = kern >> 1;
          hb_position_t kern2 = kern - kern1;

          if (!backward)
          {
            dst1[i] += kern1;
            dst2[i] += kern2;
            dst2[i + 2] += kern2;
          }
          else
          {
            dst1[i] += kern1;
            dst1[i + 2] += src[i + 2] - kern2;
            dst2[i] += kern2;
          }

          applied_first = applied_second = kern != 0;
          goto success;
        }
        goto boring;
      }
    }
    bail:

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "try kerning glyphs at %u,%u",
			  c->buffer->idx, skippy_iter.idx);
    }

    applied_first = len1 && valueFormat1.apply_value (c, this, v, buffer->cur_pos());
    applied_second = len2 && valueFormat2.apply_value (c, this, v + len1, buffer->pos[skippy_iter.idx]);

    if (applied_first || applied_second)
      if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
      {
	c->buffer->message (c->font,
			    "kerned glyphs at %u,%u",
			    c->buffer->idx, skippy_iter.idx);
      }

    if (HB_BUFFER_MESSAGE_MORE && c->buffer->messaging ())
    {
      c->buffer->message (c->font,
			  "tried kerning glyphs at %u,%u",
			  c->buffer->idx, skippy_iter.idx);
    }

    success:
    if (applied_first || applied_second)
      buffer->unsafe_to_break (buffer->idx, skippy_iter.idx + 1);
    else
    boring:
      buffer->unsafe_to_concat (buffer->idx, skippy_iter.idx + 1);

    if (len2)
    {
      skippy_iter.idx++;
      // https://github.com/harfbuzz/harfbuzz/issues/3824
      // https://github.com/harfbuzz/harfbuzz/issues/3888#issuecomment-1326781116
      buffer->unsafe_to_break (buffer->idx, skippy_iter.idx + 1);
    }

    buffer->idx = skippy_iter.idx;

    return_trace (true);
  }

  b8 subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    auto *out = c->serializer->start_embed (*this);
    if (unlikely (!c->serializer->extend_min (out))) return_trace (false);
    out->format = format;

    hb_map_t klass1_map;
    out->classDef1.serialize_subset (c, classDef1, this, &klass1_map, true, true, &(this + coverage));
    out->class1Count = klass1_map.get_population ();

    hb_map_t klass2_map;
    out->classDef2.serialize_subset (c, classDef2, this, &klass2_map, true, false);
    out->class2Count = klass2_map.get_population ();

    u32 len1 = valueFormat1.get_len ();
    u32 len2 = valueFormat2.get_len ();

    hb_pair_t<u32, u32> newFormats = hb_pair (valueFormat1, valueFormat2);

    if (c->plan->normalized_coords)
    {
      /* in case of full instancing, all var device flags will be dropped so no
       * need to strip hints here */
      newFormats = compute_effective_value_formats (klass1_map, klass2_map, false, false, &c->plan->layout_variation_idx_delta_map);
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
      newFormats = compute_effective_value_formats (klass1_map, klass2_map, strip, true);
    }

    out->valueFormat1 = newFormats.first;
    out->valueFormat2 = newFormats.second;

    u32 total_len = len1 + len2;
    hb_vector_t<u32> class2_idxs (+ hb_range ((u32) class2Count) | hb_filter (klass2_map));
    for (u32 class1_idx : + hb_range ((u32) class1Count) | hb_filter (klass1_map))
    {
      for (u32 class2_idx : class2_idxs)
      {
        u32 idx = (class1_idx * (u32) class2Count + class2_idx) * total_len;
        valueFormat1.copy_values (c->serializer, out->valueFormat1, this, &values[idx], &c->plan->layout_variation_idx_delta_map);
        valueFormat2.copy_values (c->serializer, out->valueFormat2, this, &values[idx + len1], &c->plan->layout_variation_idx_delta_map);
      }
    }

    b8 ret = out->coverage.serialize_subset(c, coverage, this);
    return_trace (out->class1Count && out->class2Count && ret);
  }


  hb_pair_t<u32, u32> compute_effective_value_formats (const hb_map_t& klass1_map,
                                                                 const hb_map_t& klass2_map,
                                                                 b8 strip_hints, b8 strip_empty,
                                                                 const hb_hashmap_t<u32, hb_pair_t<u32, i32>> *varidx_delta_map = nullptr) const
  {
    u32 len1 = valueFormat1.get_len ();
    u32 len2 = valueFormat2.get_len ();
    u32 record_size = len1 + len2;

    u32 format1 = 0;
    u32 format2 = 0;

    for (u32 class1_idx : + hb_range ((u32) class1Count) | hb_filter (klass1_map))
    {
      for (u32 class2_idx : + hb_range ((u32) class2Count) | hb_filter (klass2_map))
      {
        u32 idx = (class1_idx * (u32) class2Count + class2_idx) * record_size;
        format1 = format1 | valueFormat1.get_effective_format (&values[idx], strip_hints, strip_empty, this, varidx_delta_map);
        format2 = format2 | valueFormat2.get_effective_format (&values[idx + len1], strip_hints, strip_empty, this, varidx_delta_map);
      }

      if (format1 == valueFormat1 && format2 == valueFormat2)
        break;
    }

    return hb_pair (format1, format2);
  }
};

}
}
}

#endif  // OT_LAYOUT_GPOS_PAIRPOSFORMAT2_HH
