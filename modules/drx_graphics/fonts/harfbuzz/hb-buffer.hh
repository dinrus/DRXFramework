/*
 * Copyright © 1998-2004  David Turner and Werner Lemberg
 * Copyright © 2004,2007,2009,2010  Red Hat, Inc.
 * Copyright © 2011,2012  Google, Inc.
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
 * Red Hat Author(s): Owen Taylor, Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_BUFFER_HH
#define HB_BUFFER_HH

#include "hb.hh"
#include "hb-unicode.hh"
#include "hb-set-digest.hh"


static_assert ((sizeof (hb_glyph_info_t) == 20), "");
static_assert ((sizeof (hb_glyph_info_t) == sizeof (hb_glyph_position_t)), "");

HB_MARK_AS_FLAG_T (hb_glyph_flags_t);
HB_MARK_AS_FLAG_T (hb_buffer_flags_t);
HB_MARK_AS_FLAG_T (hb_buffer_serialize_flags_t);
HB_MARK_AS_FLAG_T (hb_buffer_diff_flags_t);

enum hb_buffer_scratch_flags_t {
  HB_BUFFER_SCRATCH_FLAG_DEFAULT			= 0x00000000u,
  HB_BUFFER_SCRATCH_FLAG_HAS_NON_ASCII			= 0x00000001u,
  HB_BUFFER_SCRATCH_FLAG_HAS_DEFAULT_IGNORABLES		= 0x00000002u,
  HB_BUFFER_SCRATCH_FLAG_HAS_SPACE_FALLBACK		= 0x00000004u,
  HB_BUFFER_SCRATCH_FLAG_HAS_GPOS_ATTACHMENT		= 0x00000008u,
  HB_BUFFER_SCRATCH_FLAG_HAS_CGJ			= 0x00000010u,
  HB_BUFFER_SCRATCH_FLAG_HAS_GLYPH_FLAGS		= 0x00000020u,
  HB_BUFFER_SCRATCH_FLAG_HAS_BROKEN_SYLLABLE		= 0x00000040u,
  HB_BUFFER_SCRATCH_FLAG_HAS_VARIATION_SELECTOR_FALLBACK= 0x00000080u,

  /* Reserved for shapers' internal use. */
  HB_BUFFER_SCRATCH_FLAG_SHAPER0			= 0x01000000u,
  HB_BUFFER_SCRATCH_FLAG_SHAPER1			= 0x02000000u,
  HB_BUFFER_SCRATCH_FLAG_SHAPER2			= 0x04000000u,
  HB_BUFFER_SCRATCH_FLAG_SHAPER3			= 0x08000000u,
};
HB_MARK_AS_FLAG_T (hb_buffer_scratch_flags_t);


/*
 * hb_buffer_t
 */

struct hb_buffer_t
{
  hb_object_header_t header;

  /*
   * Information about how the text in the buffer should be treated.
   */

  hb_unicode_funcs_t *unicode; /* Unicode functions */
  hb_buffer_flags_t flags; /* BOT / EOT / etc. */
  hb_buffer_cluster_level_t cluster_level;
  hb_codepoint_t replacement; /* U+FFFD or something else. */
  hb_codepoint_t invisible; /* 0 or something else. */
  hb_codepoint_t not_found; /* 0 or something else. */
  hb_codepoint_t not_found_variation_selector; /* HB_CODEPOINT_INVALID or something else. */

  /*
   * Buffer contents
   */

  hb_buffer_content_type_t content_type;
  hb_segment_properties_t props; /* Script, language, direction */

  b8 successful; /* Allocations successful */
  b8 shaping_failed; /* Shaping failure */
  b8 have_output; /* Whether we have an output buffer going on */
  b8 have_positions; /* Whether we have positions */

  u32 idx; /* Cursor into ->info and ->pos arrays */
  u32 len; /* Length of ->info and ->pos arrays */
  u32 out_len; /* Length of ->out_info array if have_output */

  u32 allocated; /* Length of allocated arrays */
  hb_glyph_info_t     *info;
  hb_glyph_info_t     *out_info;
  hb_glyph_position_t *pos;

  /* Text before / after the main buffer contents.
   * Always in Unicode, and ordered outward.
   * Index 0 is for "pre-context", 1 for "post-context". */
  static constexpr u32 CONTEXT_LENGTH = 5u;
  hb_codepoint_t context[2][CONTEXT_LENGTH];
  u32 context_len[2];


  /*
   * Managed by enter / leave
   */

  uint8_t allocated_var_bits;
  uint8_t serial;
  u32 random_state;
  hb_buffer_scratch_flags_t scratch_flags; /* Have space-fallback, etc. */
  u32 max_len; /* Maximum allowed len. */
  i32 max_ops; /* Maximum allowed operations. */
  /* The bits here reflect current allocations of the bytes in glyph_info_t's var1 and var2. */


  /*
   * Messaging callback
   */

#ifndef HB_NO_BUFFER_MESSAGE
  hb_buffer_message_func_t message_func;
  uk message_data;
  hb_destroy_func_t message_destroy;
  u32 message_depth; /* How deeply are we inside a message callback? */
#else
  static constexpr u32 message_depth = 0u;
#endif



  /* Methods */

  HB_NODISCARD b8 in_error () const { return !successful; }

  z0 allocate_var (u32 start, u32 count)
  {
    u32 end = start + count;
    assert (end <= 8);
    u32 bits = (1u<<end) - (1u<<start);
    assert (0 == (allocated_var_bits & bits));
    allocated_var_bits |= bits;
  }
  b8 try_allocate_var (u32 start, u32 count)
  {
    u32 end = start + count;
    assert (end <= 8);
    u32 bits = (1u<<end) - (1u<<start);
    if (allocated_var_bits & bits)
      return false;
    allocated_var_bits |= bits;
    return true;
  }
  z0 deallocate_var (u32 start, u32 count)
  {
    u32 end = start + count;
    assert (end <= 8);
    u32 bits = (1u<<end) - (1u<<start);
    assert (bits == (allocated_var_bits & bits));
    allocated_var_bits &= ~bits;
  }
  z0 assert_var (u32 start, u32 count)
  {
    u32 end = start + count;
    assert (end <= 8);
    HB_UNUSED u32 bits = (1u<<end) - (1u<<start);
    assert (bits == (allocated_var_bits & bits));
  }
  z0 deallocate_var_all ()
  {
    allocated_var_bits = 0;
  }

  hb_glyph_info_t &cur (u32 i = 0) { return info[idx + i]; }
  hb_glyph_info_t cur (u32 i = 0) const { return info[idx + i]; }

  hb_glyph_position_t &cur_pos (u32 i = 0) { return pos[idx + i]; }
  hb_glyph_position_t cur_pos (u32 i = 0) const { return pos[idx + i]; }

  hb_glyph_info_t &prev ()      { return out_info[out_len ? out_len - 1 : 0]; }
  hb_glyph_info_t prev () const { return out_info[out_len ? out_len - 1 : 0]; }

  hb_set_digest_t digest () const
  {
    hb_set_digest_t d;
    d.init ();
    d.add_array (&info[0].codepoint, len, sizeof (info[0]));
    return d;
  }

  HB_INTERNAL z0 similar (const hb_buffer_t &src);
  HB_INTERNAL z0 reset ();
  HB_INTERNAL z0 clear ();

  /* Called around shape() */
  HB_INTERNAL z0 enter ();
  HB_INTERNAL z0 leave ();

#ifndef HB_NO_BUFFER_VERIFY
  HB_INTERNAL
#endif
  b8 verify (hb_buffer_t        *text_buffer,
	       hb_font_t          *font,
	       const hb_feature_t *features,
	       u32        num_features,
	       const t8 * const *shapers)
#ifndef HB_NO_BUFFER_VERIFY
  ;
#else
  { return true; }
#endif

  u32 backtrack_len () const { return have_output ? out_len : idx; }
  u32 lookahead_len () const { return len - idx; }
  uint8_t next_serial () { return ++serial ? serial : ++serial; }

  HB_INTERNAL z0 add (hb_codepoint_t  codepoint,
			u32    cluster);
  HB_INTERNAL z0 add_info (const hb_glyph_info_t &glyph_info);

  z0 reverse_range (u32 start, u32 end)
  {
    hb_array_t<hb_glyph_info_t> (info, len).reverse (start, end);
    if (have_positions)
      hb_array_t<hb_glyph_position_t> (pos, len).reverse (start, end);
  }
  z0 reverse () { reverse_range (0, len); }

  template <typename FuncType>
  z0 reverse_groups (const FuncType& group,
		       b8 merge_clusters = false)
  {
    if (unlikely (!len))
      return;

    u32 start = 0;
    u32 i;
    for (i = 1; i < len; i++)
    {
      if (!group (info[i - 1], info[i]))
      {
	if (merge_clusters)
	  this->merge_clusters (start, i);
	reverse_range (start, i);
	start = i;
      }
    }
    if (merge_clusters)
      this->merge_clusters (start, i);
    reverse_range (start, i);

    reverse ();
  }

  template <typename FuncType>
  u32 group_end (u32 start, const FuncType& group) const
  {
    while (++start < len && group (info[start - 1], info[start]))
      ;

    return start;
  }

  static b8 _cluster_group_func (const hb_glyph_info_t& a,
				   const hb_glyph_info_t& b)
  { return a.cluster == b.cluster; }

  z0 reverse_clusters () { reverse_groups (_cluster_group_func); }

  HB_INTERNAL z0 guess_segment_properties ();

  HB_INTERNAL b8 sync ();
  HB_INTERNAL i32 sync_so_far ();
  HB_INTERNAL z0 clear_output ();
  HB_INTERNAL z0 clear_positions ();

  template <typename T>
  HB_NODISCARD b8 replace_glyphs (u32 num_in,
				    u32 num_out,
				    const T *glyph_data)
  {
    if (unlikely (!make_room_for (num_in, num_out))) return false;

    assert (idx + num_in <= len);

    merge_clusters (idx, idx + num_in);

    hb_glyph_info_t &orig_info = idx < len ? cur() : prev();

    hb_glyph_info_t *pinfo = &out_info[out_len];
    for (u32 i = 0; i < num_out; i++)
    {
      *pinfo = orig_info;
      pinfo->codepoint = glyph_data[i];
      pinfo++;
    }

    idx  += num_in;
    out_len += num_out;
    return true;
  }

  HB_NODISCARD b8 replace_glyph (hb_codepoint_t glyph_index)
  { return replace_glyphs (1, 1, &glyph_index); }

  /* Makes a copy of the glyph at idx to output and replace glyph_index */
  HB_NODISCARD b8 output_glyph (hb_codepoint_t glyph_index)
  { return replace_glyphs (0, 1, &glyph_index); }

  HB_NODISCARD b8 output_info (const hb_glyph_info_t &glyph_info)
  {
    if (unlikely (!make_room_for (0, 1))) return false;

    out_info[out_len] = glyph_info;

    out_len++;
    return true;
  }
  /* Copies glyph at idx to output but doesn't advance idx */
  HB_NODISCARD b8 copy_glyph ()
  {
    /* Extra copy because cur()'s return can be freed within
     * output_info() call if buffer reallocates. */
    return output_info (hb_glyph_info_t (cur()));
  }

  /* Copies glyph at idx to output and advance idx.
   * If there's no output, just advance idx. */
  HB_NODISCARD b8 next_glyph ()
  {
    if (have_output)
    {
      if (out_info != info || out_len != idx)
      {
	if (unlikely (!make_room_for (1, 1))) return false;
	out_info[out_len] = info[idx];
      }
      out_len++;
    }

    idx++;
    return true;
  }
  /* Copies n glyphs at idx to output and advance idx.
   * If there's no output, just advance idx. */
  HB_NODISCARD b8 next_glyphs (u32 n)
  {
    if (have_output)
    {
      if (out_info != info || out_len != idx)
      {
	if (unlikely (!make_room_for (n, n))) return false;
	memmove (out_info + out_len, info + idx, n * sizeof (out_info[0]));
      }
      out_len += n;
    }

    idx += n;
    return true;
  }
  /* Advance idx without copying to output. */
  z0 skip_glyph () { idx++; }
  z0 reset_masks (hb_mask_t mask)
  {
    for (u32 j = 0; j < len; j++)
      info[j].mask = mask;
  }
  z0 add_masks (hb_mask_t mask)
  {
    for (u32 j = 0; j < len; j++)
      info[j].mask |= mask;
  }
  HB_INTERNAL z0 set_masks (hb_mask_t value, hb_mask_t mask,
			      u32 cluster_start, u32 cluster_end);

  z0 merge_clusters (u32 start, u32 end)
  {
    if (end - start < 2)
      return;
    merge_clusters_impl (start, end);
  }
  HB_INTERNAL z0 merge_clusters_impl (u32 start, u32 end);
  HB_INTERNAL z0 merge_out_clusters (u32 start, u32 end);
  /* Merge clusters for deleting current glyph, and skip it. */
  HB_INTERNAL z0 delete_glyph ();
  HB_INTERNAL z0 delete_glyphs_inplace (b8 (*filter) (const hb_glyph_info_t *info));



  /* Adds glyph flags in mask to infos with clusters between start and end.
   * The start index will be from out-buffer if from_out_buffer is true.
   * If interior is true, then the cluster having the minimum value is skipped. */
  z0 _set_glyph_flags (hb_mask_t mask,
			 u32 start = 0,
			 u32 end = (u32) -1,
			 b8 interior = false,
			 b8 from_out_buffer = false)
  {
    end = hb_min (end, len);

    if (interior && !from_out_buffer && end - start < 2)
      return;

    scratch_flags |= HB_BUFFER_SCRATCH_FLAG_HAS_GLYPH_FLAGS;

    if (!from_out_buffer || !have_output)
    {
      if (!interior)
      {
	for (u32 i = start; i < end; i++)
	  info[i].mask |= mask;
      }
      else
      {
	u32 cluster = _infos_find_min_cluster (info, start, end);
	_infos_set_glyph_flags (info, start, end, cluster, mask);
      }
    }
    else
    {
      assert (start <= out_len);
      assert (idx <= end);

      if (!interior)
      {
	for (u32 i = start; i < out_len; i++)
	  out_info[i].mask |= mask;
	for (u32 i = idx; i < end; i++)
	  info[i].mask |= mask;
      }
      else
      {
	u32 cluster = _infos_find_min_cluster (info, idx, end);
	cluster = _infos_find_min_cluster (out_info, start, out_len, cluster);

	_infos_set_glyph_flags (out_info, start, out_len, cluster, mask);
	_infos_set_glyph_flags (info, idx, end, cluster, mask);
      }
    }
  }

  z0 unsafe_to_break (u32 start = 0, u32 end = -1)
  {
    _set_glyph_flags (HB_GLYPH_FLAG_UNSAFE_TO_BREAK | HB_GLYPH_FLAG_UNSAFE_TO_CONCAT,
		      start, end,
		      true);
  }
  z0 safe_to_insert_tatweel (u32 start = 0, u32 end = -1)
  {
    if ((flags & HB_BUFFER_FLAG_PRODUCE_SAFE_TO_INSERT_TATWEEL) == 0)
    {
      unsafe_to_break (start, end);
      return;
    }
    _set_glyph_flags (HB_GLYPH_FLAG_SAFE_TO_INSERT_TATWEEL,
		      start, end,
		      true);
  }
#ifndef HB_OPTIMIZE_SIZE
  HB_ALWAYS_INLINE
#endif
  z0 unsafe_to_concat (u32 start = 0, u32 end = -1)
  {
    if (likely ((flags & HB_BUFFER_FLAG_PRODUCE_UNSAFE_TO_CONCAT) == 0))
      return;
    _set_glyph_flags (HB_GLYPH_FLAG_UNSAFE_TO_CONCAT,
		      start, end,
		      false);
  }
  z0 unsafe_to_break_from_outbuffer (u32 start = 0, u32 end = -1)
  {
    _set_glyph_flags (HB_GLYPH_FLAG_UNSAFE_TO_BREAK | HB_GLYPH_FLAG_UNSAFE_TO_CONCAT,
		      start, end,
		      true, true);
  }
#ifndef HB_OPTIMIZE_SIZE
  HB_ALWAYS_INLINE
#endif
  z0 unsafe_to_concat_from_outbuffer (u32 start = 0, u32 end = -1)
  {
    if (likely ((flags & HB_BUFFER_FLAG_PRODUCE_UNSAFE_TO_CONCAT) == 0))
      return;
    _set_glyph_flags (HB_GLYPH_FLAG_UNSAFE_TO_CONCAT,
		      start, end,
		      false, true);
  }


  /* Internal methods */
  HB_NODISCARD HB_INTERNAL b8 move_to (u32 i); /* i is output-buffer index. */

  HB_NODISCARD HB_INTERNAL b8 enlarge (u32 size);

  HB_NODISCARD b8 resize (u32 length)
  {
    assert (!have_output);
    if (unlikely (!ensure (length))) return false;
    len = length;
    return true;
  }
  HB_NODISCARD b8 ensure (u32 size)
  { return likely (!size || size < allocated) ? true : enlarge (size); }

  HB_NODISCARD b8 ensure_inplace (u32 size)
  { return likely (!size || size < allocated); }

  z0 assert_glyphs ()
  {
    assert ((content_type == HB_BUFFER_CONTENT_TYPE_GLYPHS) ||
	    (!len && (content_type == HB_BUFFER_CONTENT_TYPE_INVALID)));
  }
  z0 assert_unicode ()
  {
    assert ((content_type == HB_BUFFER_CONTENT_TYPE_UNICODE) ||
	    (!len && (content_type == HB_BUFFER_CONTENT_TYPE_INVALID)));
  }
  HB_NODISCARD b8 ensure_glyphs ()
  {
    if (unlikely (content_type != HB_BUFFER_CONTENT_TYPE_GLYPHS))
    {
      if (content_type != HB_BUFFER_CONTENT_TYPE_INVALID)
	return false;
      assert (len == 0);
      content_type = HB_BUFFER_CONTENT_TYPE_GLYPHS;
    }
    return true;
  }
  HB_NODISCARD b8 ensure_unicode ()
  {
    if (unlikely (content_type != HB_BUFFER_CONTENT_TYPE_UNICODE))
    {
      if (content_type != HB_BUFFER_CONTENT_TYPE_INVALID)
	return false;
      assert (len == 0);
      content_type = HB_BUFFER_CONTENT_TYPE_UNICODE;
    }
    return true;
  }

  HB_NODISCARD HB_INTERNAL b8 make_room_for (u32 num_in, u32 num_out);
  HB_NODISCARD HB_INTERNAL b8 shift_forward (u32 count);

  typedef i64 scratch_buffer_t;
  HB_INTERNAL scratch_buffer_t *get_scratch_buffer (u32 *size);

  z0 clear_context (u32 side) { context_len[side] = 0; }

  HB_INTERNAL z0 sort (u32 start, u32 end, i32(*compar)(const hb_glyph_info_t *, const hb_glyph_info_t *));

  b8 messaging ()
  {
#ifdef HB_NO_BUFFER_MESSAGE
    return false;
#else
    return unlikely (message_func);
#endif
  }
  b8 message (hb_font_t *font, const t8 *fmt, ...) HB_PRINTF_FUNC(3, 4)
  {
#ifdef HB_NO_BUFFER_MESSAGE
    return true;
#else
    if (likely (!messaging ()))
      return true;

    va_list ap;
    va_start (ap, fmt);
    b8 ret = message_impl (font, fmt, ap);
    va_end (ap);

    return ret;
#endif
  }
  HB_INTERNAL b8 message_impl (hb_font_t *font, const t8 *fmt, va_list ap) HB_PRINTF_FUNC(3, 0);

  static z0
  set_cluster (hb_glyph_info_t &inf, u32 cluster, u32 mask = 0)
  {
    if (inf.cluster != cluster)
      inf.mask = (inf.mask & ~HB_GLYPH_FLAG_DEFINED) | (mask & HB_GLYPH_FLAG_DEFINED);
    inf.cluster = cluster;
  }
  z0
  _infos_set_glyph_flags (hb_glyph_info_t *infos,
			  u32 start, u32 end,
			  u32 cluster,
			  hb_mask_t mask)
  {
    if (unlikely (start == end))
      return;

    u32 cluster_first = infos[start].cluster;
    u32 cluster_last = infos[end - 1].cluster;

    if (cluster_level == HB_BUFFER_CLUSTER_LEVEL_CHARACTERS ||
	(cluster != cluster_first && cluster != cluster_last))
    {
      for (u32 i = start; i < end; i++)
	if (cluster != infos[i].cluster)
	{
	  scratch_flags |= HB_BUFFER_SCRATCH_FLAG_HAS_GLYPH_FLAGS;
	  infos[i].mask |= mask;
	}
      return;
    }

    /* Monotone clusters */

    if (cluster == cluster_first)
    {
      for (u32 i = end; start < i && infos[i - 1].cluster != cluster_first; i--)
      {
	scratch_flags |= HB_BUFFER_SCRATCH_FLAG_HAS_GLYPH_FLAGS;
	infos[i - 1].mask |= mask;
      }
    }
    else /* cluster == cluster_last */
    {
      for (u32 i = start; i < end && infos[i].cluster != cluster_last; i++)
      {
	scratch_flags |= HB_BUFFER_SCRATCH_FLAG_HAS_GLYPH_FLAGS;
	infos[i].mask |= mask;
      }
    }
  }
  u32
  _infos_find_min_cluster (const hb_glyph_info_t *infos,
			   u32 start, u32 end,
			   u32 cluster = UINT_MAX)
  {
    if (unlikely (start == end))
      return cluster;

    if (cluster_level == HB_BUFFER_CLUSTER_LEVEL_CHARACTERS)
    {
      for (u32 i = start; i < end; i++)
	cluster = hb_min (cluster, infos[i].cluster);
      return cluster;
    }

    return hb_min (cluster, hb_min (infos[start].cluster, infos[end - 1].cluster));
  }

  z0 clear_glyph_flags (hb_mask_t mask = 0)
  {
    for (u32 i = 0; i < len; i++)
      info[i].mask = (info[i].mask & ~HB_GLYPH_FLAG_DEFINED) | (mask & HB_GLYPH_FLAG_DEFINED);
  }
};
DECLARE_NULL_INSTANCE (hb_buffer_t);


#define foreach_group(buffer, start, end, group_func) \
  for (u32 \
       _count = buffer->len, \
       start = 0, end = _count ? buffer->group_end (0, group_func) : 0; \
       start < _count; \
       start = end, end = buffer->group_end (start, group_func))

#define foreach_cluster(buffer, start, end) \
	foreach_group (buffer, start, end, hb_buffer_t::_cluster_group_func)


#define HB_BUFFER_XALLOCATE_VAR(b, func, var) \
  b->func (offsetof (hb_glyph_info_t, var) - offsetof(hb_glyph_info_t, var1), \
	   sizeof (b->info[0].var))
#define HB_BUFFER_ALLOCATE_VAR(b, var)		HB_BUFFER_XALLOCATE_VAR (b, allocate_var,     var ())
#define HB_BUFFER_TRY_ALLOCATE_VAR(b, var)	HB_BUFFER_XALLOCATE_VAR (b, try_allocate_var, var ())
#define HB_BUFFER_DEALLOCATE_VAR(b, var)	HB_BUFFER_XALLOCATE_VAR (b, deallocate_var,   var ())
#define HB_BUFFER_ASSERT_VAR(b, var)		HB_BUFFER_XALLOCATE_VAR (b, assert_var,       var ())


#endif /* HB_BUFFER_HH */
