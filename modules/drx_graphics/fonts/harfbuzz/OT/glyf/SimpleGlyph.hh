#ifndef OT_GLYF_SIMPLEGLYPH_HH
#define OT_GLYF_SIMPLEGLYPH_HH


#include "../../hb-open-type.hh"


namespace OT {
namespace glyf_impl {


struct SimpleGlyph
{
  enum simple_glyph_flag_t
  {
    FLAG_ON_CURVE       = 0x01,
    FLAG_X_SHORT        = 0x02,
    FLAG_Y_SHORT        = 0x04,
    FLAG_REPEAT         = 0x08,
    FLAG_X_SAME         = 0x10,
    FLAG_Y_SAME         = 0x20,
    FLAG_OVERLAP_SIMPLE = 0x40,
    FLAG_CUBIC          = 0x80
  };

  const GlyphHeader &header;
  hb_bytes_t bytes;
  SimpleGlyph (const GlyphHeader &header_, hb_bytes_t bytes_) :
    header (header_), bytes (bytes_) {}

  u32 instruction_len_offset () const
  { return GlyphHeader::static_size + 2 * header.numberOfContours; }

  u32 length (u32 instruction_len) const
  { return instruction_len_offset () + 2 + instruction_len; }

  b8 has_instructions_length () const
  {
    return instruction_len_offset () + 2 <= bytes.length;
  }

  u32 instructions_length () const
  {
    u32 instruction_length_offset = instruction_len_offset ();
    if (unlikely (instruction_length_offset + 2 > bytes.length)) return 0;

    const HBUINT16 &instructionLength = StructAtOffset<HBUINT16> (&bytes, instruction_length_offset);
    /* Out of bounds of the current glyph */
    if (unlikely (length (instructionLength) > bytes.length)) return 0;
    return instructionLength;
  }

  const hb_bytes_t trim_padding () const
  {
    /* based on FontTools _g_l_y_f.py::trim */
    const uint8_t *glyph = (uint8_t*) bytes.arrayZ;
    const uint8_t *glyph_end = glyph + bytes.length;
    /* simple glyph w/contours, possibly trimmable */
    glyph += instruction_len_offset ();

    if (unlikely (glyph + 2 >= glyph_end)) return hb_bytes_t ();
    u32 num_coordinates = StructAtOffset<HBUINT16> (glyph - 2, 0) + 1;
    u32 num_instructions = StructAtOffset<HBUINT16> (glyph, 0);

    glyph += 2 + num_instructions;

    u32 coord_bytes = 0;
    u32 coords_with_flags = 0;
    while (glyph < glyph_end)
    {
      uint8_t flag = *glyph;
      glyph++;

      u32 repeat = 1;
      if (flag & FLAG_REPEAT)
      {
	if (unlikely (glyph >= glyph_end)) return hb_bytes_t ();
	repeat = *glyph + 1;
	glyph++;
      }

      u32 xBytes, yBytes;
      xBytes = yBytes = 0;
      if (flag & FLAG_X_SHORT) xBytes = 1;
      else if ((flag & FLAG_X_SAME) == 0) xBytes = 2;

      if (flag & FLAG_Y_SHORT) yBytes = 1;
      else if ((flag & FLAG_Y_SAME) == 0) yBytes = 2;

      coord_bytes += (xBytes + yBytes) * repeat;
      coords_with_flags += repeat;
      if (coords_with_flags >= num_coordinates) break;
    }

    if (unlikely (coords_with_flags != num_coordinates)) return hb_bytes_t ();
    return bytes.sub_array (0, bytes.length + coord_bytes - (glyph_end - glyph));
  }

  /* zero instruction length */
  z0 drop_hints ()
  {
    if (!has_instructions_length ()) return;
    GlyphHeader &glyph_header = const_cast<GlyphHeader &> (header);
    (HBUINT16 &) StructAtOffset<HBUINT16> (&glyph_header, instruction_len_offset ()) = 0;
  }

  z0 drop_hints_bytes (hb_bytes_t &dest_start, hb_bytes_t &dest_end) const
  {
    u32 instructions_len = instructions_length ();
    u32 glyph_length = length (instructions_len);
    dest_start = bytes.sub_array (0, glyph_length - instructions_len);
    dest_end = bytes.sub_array (glyph_length, bytes.length - glyph_length);
  }

  z0 set_overlaps_flag ()
  {
    if (unlikely (!header.numberOfContours)) return;

    u32 flags_offset = length (instructions_length ());
    if (unlikely (flags_offset + 1 > bytes.length)) return;

    HBUINT8 &first_flag = (HBUINT8 &) StructAtOffset<HBUINT16> (&bytes, flags_offset);
    first_flag = (uint8_t) first_flag | FLAG_OVERLAP_SIMPLE;
  }

  static b8 read_flags (const HBUINT8 *&p /* IN/OUT */,
			  hb_array_t<contour_point_t> points_ /* IN/OUT */,
			  const HBUINT8 *end)
  {
    u32 count = points_.length;
    for (u32 i = 0; i < count;)
    {
      if (unlikely (p + 1 > end)) return false;
      uint8_t flag = *p++;
      points_.arrayZ[i++].flag = flag;
      if (flag & FLAG_REPEAT)
      {
	if (unlikely (p + 1 > end)) return false;
	u32 repeat_count = *p++;
	u32 stop = hb_min (i + repeat_count, count);
	for (; i < stop; i++)
	  points_.arrayZ[i].flag = flag;
      }
    }
    return true;
  }

  static b8 read_points (const HBUINT8 *&p /* IN/OUT */,
			   hb_array_t<contour_point_t> points_ /* IN/OUT */,
			   const HBUINT8 *end,
			   f32 contour_point_t::*m,
			   const simple_glyph_flag_t short_flag,
			   const simple_glyph_flag_t same_flag)
  {
    i32 v = 0;

    for (auto &point : points_)
    {
      u32 flag = point.flag;
      if (flag & short_flag)
      {
	if (unlikely (p + 1 > end)) return false;
	if (flag & same_flag)
	  v += *p++;
	else
	  v -= *p++;
      }
      else
      {
	if (!(flag & same_flag))
	{
	  if (unlikely (p + HBINT16::static_size > end)) return false;
	  v += *(const HBINT16 *) p;
	  p += HBINT16::static_size;
	}
      }
      point.*m = v;
    }
    return true;
  }

  b8 get_contour_points (contour_point_vector_t &points /* OUT */,
			   b8 phantom_only = false) const
  {
    const HBUINT16 *endPtsOfContours = &StructAfter<HBUINT16> (header);
    i32 num_contours = header.numberOfContours;
    assert (num_contours > 0);
    /* One extra item at the end, for the instruction-count below. */
    if (unlikely (!bytes.check_range (&endPtsOfContours[num_contours]))) return false;
    u32 num_points = endPtsOfContours[num_contours - 1] + 1;

    u32 old_length = points.length;
    points.alloc (points.length + num_points + 4, true); // Allocate for phantom points, to avoid a possible copy
    if (unlikely (!points.resize (points.length + num_points, false))) return false;
    auto points_ = points.as_array ().sub_array (old_length);
    if (!phantom_only)
      hb_memset (points_.arrayZ, 0, sizeof (contour_point_t) * num_points);
    if (phantom_only) return true;

    for (i32 i = 0; i < num_contours; i++)
      points_[endPtsOfContours[i]].is_end_point = true;

    /* Skip instructions */
    const HBUINT8 *p = &StructAtOffset<HBUINT8> (&endPtsOfContours[num_contours + 1],
						 endPtsOfContours[num_contours]);

    if (unlikely ((const t8 *) p < bytes.arrayZ)) return false; /* Unlikely overflow */
    const HBUINT8 *end = (const HBUINT8 *) (bytes.arrayZ + bytes.length);
    if (unlikely (p >= end)) return false;

    /* Read x & y coordinates */
    return read_flags (p, points_, end)
        && read_points (p, points_, end, &contour_point_t::x,
			FLAG_X_SHORT, FLAG_X_SAME)
	&& read_points (p, points_, end, &contour_point_t::y,
			FLAG_Y_SHORT, FLAG_Y_SAME);
  }

  static z0 encode_coord (i32 value,
                            u32 &flag,
                            const simple_glyph_flag_t short_flag,
                            const simple_glyph_flag_t same_flag,
                            hb_vector_t<uint8_t> &coords /* OUT */)
  {
    if (value == 0)
    {
      flag |= same_flag;
    }
    else if (value >= -255 && value <= 255)
    {
      flag |= short_flag;
      if (value > 0) flag |= same_flag;
      else value = -value;

      coords.arrayZ[coords.length++] = (uint8_t) value;
    }
    else
    {
      int16_t val = value;
      coords.arrayZ[coords.length++] = val >> 8;
      coords.arrayZ[coords.length++] = val & 0xff;
    }
  }

  static z0 encode_flag (u32 flag,
                           u32 &repeat,
                           u32 lastflag,
                           hb_vector_t<uint8_t> &flags /* OUT */)
  {
    if (flag == lastflag && repeat != 255)
    {
      repeat++;
      if (repeat == 1)
      {
        /* We know there's room. */
        flags.arrayZ[flags.length++] = flag;
      }
      else
      {
        u32 len = flags.length;
        flags.arrayZ[len-2] = flag | FLAG_REPEAT;
        flags.arrayZ[len-1] = repeat;
      }
    }
    else
    {
      repeat = 0;
      flags.arrayZ[flags.length++] = flag;
    }
  }

  b8 compile_bytes_with_deltas (const contour_point_vector_t &all_points,
                                  b8 no_hinting,
                                  hb_bytes_t &dest_bytes /* OUT */)
  {
    if (header.numberOfContours == 0 || all_points.length <= 4)
    {
      dest_bytes = hb_bytes_t ();
      return true;
    }
    u32 num_points = all_points.length - 4;

    hb_vector_t<uint8_t> flags, x_coords, y_coords;
    if (unlikely (!flags.alloc (num_points, true))) return false;
    if (unlikely (!x_coords.alloc (2*num_points, true))) return false;
    if (unlikely (!y_coords.alloc (2*num_points, true))) return false;

    u32 lastflag = 255, repeat = 0;
    i32 prev_x = 0, prev_y = 0;

    for (u32 i = 0; i < num_points; i++)
    {
      u32 flag = all_points.arrayZ[i].flag;
      flag &= FLAG_ON_CURVE | FLAG_OVERLAP_SIMPLE | FLAG_CUBIC;

      i32 cur_x = roundf (all_points.arrayZ[i].x);
      i32 cur_y = roundf (all_points.arrayZ[i].y);
      encode_coord (cur_x - prev_x, flag, FLAG_X_SHORT, FLAG_X_SAME, x_coords);
      encode_coord (cur_y - prev_y, flag, FLAG_Y_SHORT, FLAG_Y_SAME, y_coords);
      encode_flag (flag, repeat, lastflag, flags);

      prev_x = cur_x;
      prev_y = cur_y;
      lastflag = flag;
    }

    u32 len_before_instrs = 2 * header.numberOfContours + 2;
    u32 len_instrs = instructions_length ();
    u32 total_len = len_before_instrs + flags.length + x_coords.length + y_coords.length;

    if (!no_hinting)
      total_len += len_instrs;

    t8 *p = (t8 *) hb_malloc (total_len);
    if (unlikely (!p)) return false;

    const t8 *src = bytes.arrayZ + GlyphHeader::static_size;
    t8 *cur = p;
    hb_memcpy (p, src, len_before_instrs);

    cur += len_before_instrs;
    src += len_before_instrs;

    if (!no_hinting)
    {
      hb_memcpy (cur, src, len_instrs);
      cur += len_instrs;
    }

    hb_memcpy (cur, flags.arrayZ, flags.length);
    cur += flags.length;

    hb_memcpy (cur, x_coords.arrayZ, x_coords.length);
    cur += x_coords.length;

    hb_memcpy (cur, y_coords.arrayZ, y_coords.length);

    dest_bytes = hb_bytes_t (p, total_len);
    return true;
  }
};


} /* namespace glyf_impl */
} /* namespace OT */


#endif /* OT_GLYF_SIMPLEGLYPH_HH */
