/*
 * Copyright © 2007,2008,2009  Red Hat, Inc.
 * Copyright © 2010,2012  Google, Inc.
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
 * Google Author(s): Behdad Esfahbod, Garret Rieger
 */


#ifndef OT_LAYOUT_COMMON_COVERAGEFORMAT1_HH
#define OT_LAYOUT_COMMON_COVERAGEFORMAT1_HH

namespace OT {
namespace Layout {
namespace Common {

#define NOT_COVERED             ((u32) -1)

template <typename Types>
struct CoverageFormat1_3
{
  friend struct Coverage;

  protected:
  HBUINT16      coverageFormat; /* Format identifier--format = 1 */
  SortedArray16Of<typename Types::HBGlyphID>
                glyphArray;     /* Array of GlyphIDs--in numerical order */
  public:
  DEFINE_SIZE_ARRAY (4, glyphArray);

  private:
  b8 sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (glyphArray.sanitize (c));
  }

  u32 get_coverage (hb_codepoint_t glyph_id) const
  {
    u32 i;
    glyphArray.bfind (glyph_id, &i, HB_NOT_FOUND_STORE, NOT_COVERED);
    return i;
  }

  u32 get_population () const
  {
    return glyphArray.len;
  }

  template <typename Iterator,
      hb_requires (hb_is_sorted_source_of (Iterator, hb_codepoint_t))>
  b8 serialize (hb_serialize_context_t *c, Iterator glyphs)
  {
    TRACE_SERIALIZE (this);
    return_trace (glyphArray.serialize (c, glyphs));
  }

  b8 intersects (const hb_set_t *glyphs) const
  {
    if (glyphArray.len > glyphs->get_population () * hb_bit_storage ((u32) glyphArray.len) / 2)
    {
      for (auto g : *glyphs)
        if (get_coverage (g) != NOT_COVERED)
	  return true;
      return false;
    }

    for (const auto& g : glyphArray.as_array ())
      if (glyphs->has (g))
        return true;
    return false;
  }
  b8 intersects_coverage (const hb_set_t *glyphs, u32 index) const
  { return glyphs->has (glyphArray[index]); }

  template <typename IterableOut,
	    hb_requires (hb_is_sink_of (IterableOut, hb_codepoint_t))>
  z0 intersect_set (const hb_set_t &glyphs, IterableOut&& intersect_glyphs) const
  {
    u32 count = glyphArray.len;
    for (u32 i = 0; i < count; i++)
      if (glyphs.has (glyphArray[i]))
        intersect_glyphs << glyphArray[i];
  }

  template <typename set_t>
  b8 collect_coverage (set_t *glyphs) const
  { return glyphs->add_sorted_array (glyphArray.as_array ()); }

  public:
  /* Older compilers need this to be public. */
  struct iter_t
  {
    z0 init (const struct CoverageFormat1_3 &c_) { c = &c_; i = 0; }
    b8 __more__ () const { return i < c->glyphArray.len; }
    z0 __next__ () { i++; }
    hb_codepoint_t get_glyph () const { return c->glyphArray[i]; }
    b8 operator != (const iter_t& o) const
    { return i != o.i; }
    iter_t __end__ () const { iter_t it; it.init (*c); it.i = c->glyphArray.len; return it; }

    private:
    const struct CoverageFormat1_3 *c;
    u32 i;
  };
  private:
};

}
}
}

#endif  // #ifndef OT_LAYOUT_COMMON_COVERAGEFORMAT1_HH
