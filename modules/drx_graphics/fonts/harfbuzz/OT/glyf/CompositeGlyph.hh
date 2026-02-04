#ifndef OT_GLYF_COMPOSITEGLYPH_HH
#define OT_GLYF_COMPOSITEGLYPH_HH


#include "../../hb-open-type.hh"
#include "composite-iter.hh"


namespace OT {
namespace glyf_impl {


struct CompositeGlyphRecord
{
  protected:
  enum composite_glyph_flag_t
  {
    ARG_1_AND_2_ARE_WORDS	= 0x0001,
    ARGS_ARE_XY_VALUES		= 0x0002,
    ROUND_XY_TO_GRID		= 0x0004,
    WE_HAVE_A_SCALE		= 0x0008,
    MORE_COMPONENTS		= 0x0020,
    WE_HAVE_AN_X_AND_Y_SCALE	= 0x0040,
    WE_HAVE_A_TWO_BY_TWO	= 0x0080,
    WE_HAVE_INSTRUCTIONS	= 0x0100,
    USE_MY_METRICS		= 0x0200,
    OVERLAP_COMPOUND		= 0x0400,
    SCALED_COMPONENT_OFFSET	= 0x0800,
    UNSCALED_COMPONENT_OFFSET	= 0x1000,
#ifndef HB_NO_BEYOND_64K
    GID_IS_24BIT		= 0x2000
#endif
  };

  public:
  u32 get_size () const
  {
    u32 size = min_size;
    /* glyphIndex is 24bit instead of 16bit */
#ifndef HB_NO_BEYOND_64K
    if (flags & GID_IS_24BIT) size += HBGlyphID24::static_size - HBGlyphID16::static_size;
#endif
    /* arg1 and 2 are int16 */
    if (flags & ARG_1_AND_2_ARE_WORDS) size += 4;
    /* arg1 and 2 are int8 */
    else size += 2;

    /* One x 16 bit (scale) */
    if (flags & WE_HAVE_A_SCALE) size += 2;
    /* Two x 16 bit (xscale, yscale) */
    else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) size += 4;
    /* Four x 16 bit (xscale, scale01, scale10, yscale) */
    else if (flags & WE_HAVE_A_TWO_BY_TWO) size += 8;

    return size;
  }

  z0 drop_instructions_flag ()  { flags = (u16) flags & ~WE_HAVE_INSTRUCTIONS; }
  z0 set_overlaps_flag ()
  {
    flags = (u16) flags | OVERLAP_COMPOUND;
  }

  b8 has_instructions ()  const { return   flags & WE_HAVE_INSTRUCTIONS; }

  b8 has_more ()          const { return   flags & MORE_COMPONENTS; }
  b8 is_use_my_metrics () const { return   flags & USE_MY_METRICS; }
  b8 is_anchored ()       const { return !(flags & ARGS_ARE_XY_VALUES); }
  z0 get_anchor_points (u32 &point1, u32 &point2) const
  {
    const auto *p = &StructAfter<const HBUINT8> (flags);
#ifndef HB_NO_BEYOND_64K
    if (flags & GID_IS_24BIT)
      p += HBGlyphID24::static_size;
    else
#endif
      p += HBGlyphID16::static_size;
    if (flags & ARG_1_AND_2_ARE_WORDS)
    {
      point1 = ((const HBUINT16 *) p)[0];
      point2 = ((const HBUINT16 *) p)[1];
    }
    else
    {
      point1 = p[0];
      point2 = p[1];
    }
  }

  static z0 transform (const f32 (&matrix)[4],
			 hb_array_t<contour_point_t> points)
  {
    if (matrix[0] != 1.f || matrix[1] != 0.f ||
	matrix[2] != 0.f || matrix[3] != 1.f)
      for (auto &point : points)
        point.transform (matrix);
  }

  static z0 translate (const contour_point_t &trans,
			 hb_array_t<contour_point_t> points)
  {
    if (HB_OPTIMIZE_SIZE_VAL)
    {
      if (trans.x != 0.f || trans.y != 0.f)
        for (auto &point : points)
	  point.translate (trans);
    }
    else
    {
      if (trans.x != 0.f && trans.y != 0.f)
        for (auto &point : points)
	  point.translate (trans);
      else
      {
	if (trans.x != 0.f)
	  for (auto &point : points)
	    point.x += trans.x;
	else if (trans.y != 0.f)
	  for (auto &point : points)
	    point.y += trans.y;
      }
    }
  }

  z0 transform_points (hb_array_t<contour_point_t> points,
			 const f32 (&matrix)[4],
			 const contour_point_t &trans) const
  {
    if (scaled_offsets ())
    {
      translate (trans, points);
      transform (matrix, points);
    }
    else
    {
      transform (matrix, points);
      translate (trans, points);
    }
  }

  b8 get_points (contour_point_vector_t &points) const
  {
    f32 matrix[4];
    contour_point_t trans;
    get_transformation (matrix, trans);
    if (unlikely (!points.alloc (points.length + 4))) return false; // For phantom points
    points.push (trans);
    return true;
  }

  u32 compile_with_point (const contour_point_t &point,
                               t8 *out) const
  {
    const HBINT8 *p = &StructAfter<const HBINT8> (flags);
#ifndef HB_NO_BEYOND_64K
    if (flags & GID_IS_24BIT)
      p += HBGlyphID24::static_size;
    else
#endif
      p += HBGlyphID16::static_size;

    u32 len = get_size ();
    u32 len_before_val = (const t8 *)p - (const t8 *)this;
    if (flags & ARG_1_AND_2_ARE_WORDS)
    {
      // no overflow, copy value
      hb_memcpy (out, this, len);

      HBINT16 *o = reinterpret_cast<HBINT16 *> (out + len_before_val);
      o[0] = roundf (point.x);
      o[1] = roundf (point.y);
    }
    else
    {
      i32 new_x = roundf (point.x);
      i32 new_y = roundf (point.y);
      if (new_x <= 127 && new_x >= -128 &&
          new_y <= 127 && new_y >= -128)
      {
        hb_memcpy (out, this, len);
        HBINT8 *o = reinterpret_cast<HBINT8 *> (out + len_before_val);
        o[0] = new_x;
        o[1] = new_y;
      }
      else
      {
        // new point value has an int8 overflow
        hb_memcpy (out, this, len_before_val);
        
        //update flags
        CompositeGlyphRecord *o = reinterpret_cast<CompositeGlyphRecord *> (out);
        o->flags = flags | ARG_1_AND_2_ARE_WORDS;
        out += len_before_val;

        HBINT16 new_value;
        new_value = new_x;
        hb_memcpy (out, &new_value, HBINT16::static_size);
        out += HBINT16::static_size;

        new_value = new_y;
        hb_memcpy (out, &new_value, HBINT16::static_size);
        out += HBINT16::static_size;

        hb_memcpy (out, p+2, len - len_before_val - 2);
        len += 2;
      }
    }
    return len;
  }

  protected:
  b8 scaled_offsets () const
  { return (flags & (SCALED_COMPONENT_OFFSET | UNSCALED_COMPONENT_OFFSET)) == SCALED_COMPONENT_OFFSET; }

  public:
  b8 get_transformation (f32 (&matrix)[4], contour_point_t &trans) const
  {
    matrix[0] = matrix[3] = 1.f;
    matrix[1] = matrix[2] = 0.f;

    const auto *p = &StructAfter<const HBINT8> (flags);
#ifndef HB_NO_BEYOND_64K
    if (flags & GID_IS_24BIT)
      p += HBGlyphID24::static_size;
    else
#endif
      p += HBGlyphID16::static_size;
    i32 tx, ty;
    if (flags & ARG_1_AND_2_ARE_WORDS)
    {
      tx = *(const HBINT16 *) p;
      p += HBINT16::static_size;
      ty = *(const HBINT16 *) p;
      p += HBINT16::static_size;
    }
    else
    {
      tx = *p++;
      ty = *p++;
    }
    if (is_anchored ()) tx = ty = 0;

    /* set is_end_point flag to true, used by IUP delta optimization */
    trans.init ((f32) tx, (f32) ty, true);

    {
      const F2DOT14 *points = (const F2DOT14 *) p;
      if (flags & WE_HAVE_A_SCALE)
      {
	matrix[0] = matrix[3] = points[0].to_float ();
	return true;
      }
      else if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
      {
	matrix[0] = points[0].to_float ();
	matrix[3] = points[1].to_float ();
	return true;
      }
      else if (flags & WE_HAVE_A_TWO_BY_TWO)
      {
	matrix[0] = points[0].to_float ();
	matrix[1] = points[1].to_float ();
	matrix[2] = points[2].to_float ();
	matrix[3] = points[3].to_float ();
	return true;
      }
    }
    return tx || ty;
  }

  hb_codepoint_t get_gid () const
  {
#ifndef HB_NO_BEYOND_64K
    if (flags & GID_IS_24BIT)
      return StructAfter<const HBGlyphID24> (flags);
    else
#endif
      return StructAfter<const HBGlyphID16> (flags);
  }
  z0 set_gid (hb_codepoint_t gid)
  {
#ifndef HB_NO_BEYOND_64K
    if (flags & GID_IS_24BIT)
      StructAfter<HBGlyphID24> (flags) = gid;
    else
#endif
      /* TODO assert? */
      StructAfter<HBGlyphID16> (flags) = gid;
  }

#ifndef HB_NO_BEYOND_64K
  z0 lower_gid_24_to_16 ()
  {
    hb_codepoint_t gid = get_gid ();
    if (!(flags & GID_IS_24BIT) || gid > 0xFFFFu)
      return;

    /* Lower the flag and move the rest of the struct down. */

    u32 size = get_size ();
    t8 *end = (t8 *) this + size;
    t8 *p = &StructAfter<t8> (flags);
    p += HBGlyphID24::static_size;

    flags = flags & ~GID_IS_24BIT;
    set_gid (gid);

    memmove (p - HBGlyphID24::static_size + HBGlyphID16::static_size, p, end - p);
  }
#endif

  protected:
  HBUINT16	flags;
  HBUINT24	pad;
  public:
  DEFINE_SIZE_MIN (4);
};

using composite_iter_t = composite_iter_tmpl<CompositeGlyphRecord>;

struct CompositeGlyph
{
  const GlyphHeader &header;
  hb_bytes_t bytes;
  CompositeGlyph (const GlyphHeader &header_, hb_bytes_t bytes_) :
    header (header_), bytes (bytes_) {}

  composite_iter_t iter () const
  { return composite_iter_t (bytes, &StructAfter<CompositeGlyphRecord, GlyphHeader> (header)); }

  u32 instructions_length (hb_bytes_t bytes) const
  {
    u32 start = bytes.length;
    u32 end = bytes.length;
    const CompositeGlyphRecord *last = nullptr;
    for (auto &item : iter ())
      last = &item;
    if (unlikely (!last)) return 0;

    if (last->has_instructions ())
      start = (t8 *) last - &bytes + last->get_size ();
    if (unlikely (start > end)) return 0;
    return end - start;
  }

  /* Trimming for composites not implemented.
   * If removing hints it falls out of that. */
  const hb_bytes_t trim_padding () const { return bytes; }

  z0 drop_hints ()
  {
    for (const auto &_ : iter ())
      const_cast<CompositeGlyphRecord &> (_).drop_instructions_flag ();
  }

  /* Chop instructions off the end */
  z0 drop_hints_bytes (hb_bytes_t &dest_start) const
  { dest_start = bytes.sub_array (0, bytes.length - instructions_length (bytes)); }

  z0 set_overlaps_flag ()
  {
    CompositeGlyphRecord& glyph_chain = const_cast<CompositeGlyphRecord &> (
	StructAfter<CompositeGlyphRecord, GlyphHeader> (header));
    if (!bytes.check_range(&glyph_chain, CompositeGlyphRecord::min_size))
      return;
    glyph_chain.set_overlaps_flag ();
  }

  b8 compile_bytes_with_deltas (const hb_bytes_t &source_bytes,
                                  const contour_point_vector_t &points_with_deltas,
                                  hb_bytes_t &dest_bytes /* OUT */)
  {
    if (source_bytes.length <= GlyphHeader::static_size ||
        header.numberOfContours != -1)
    {
      dest_bytes = hb_bytes_t ();
      return true;
    }

    u32 source_len = source_bytes.length - GlyphHeader::static_size;

    /* try to allocate more memories than source glyph bytes
     * in case that there might be an overflow for int8 value
     * and we would need to use int16 instead */
    t8 *o = (t8 *) hb_calloc (source_len * 2, sizeof (t8));
    if (unlikely (!o)) return false;

    const CompositeGlyphRecord *c = reinterpret_cast<const CompositeGlyphRecord *> (source_bytes.arrayZ + GlyphHeader::static_size);
    auto it = composite_iter_t (hb_bytes_t ((const t8 *)c, source_len), c);

    t8 *p = o;
    u32 i = 0, source_comp_len = 0;
    for (const auto &component : it)
    {
      /* last 4 points in points_with_deltas are phantom points and should not be included */
      if (i >= points_with_deltas.length - 4) {
        hb_free (o);
        return false;
      }

      u32 comp_len = component.get_size ();
      if (component.is_anchored ())
      {
        hb_memcpy (p, &component, comp_len);
        p += comp_len;
      }
      else
      {
        u32 new_len = component.compile_with_point (points_with_deltas[i], p);
        p += new_len;
      }
      i++;
      source_comp_len += comp_len;
    }

    //copy instructions if any
    if (source_len > source_comp_len)
    {
      u32 instr_len = source_len - source_comp_len;
      hb_memcpy (p, (const t8 *)c + source_comp_len, instr_len);
      p += instr_len;
    }

    u32 len = p - o;
    dest_bytes = hb_bytes_t (o, len);
    return true;
  }
};


} /* namespace glyf_impl */
} /* namespace OT */


#endif /* OT_GLYF_COMPOSITEGLYPH_HH */
