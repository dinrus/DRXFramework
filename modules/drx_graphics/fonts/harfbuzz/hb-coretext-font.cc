/*
 * Copyright © 2024  Google, Inc.
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
 * Author(s): Behdad Esfahbod
 */

#include "hb.hh"

#ifdef HAVE_CORETEXT

#include "hb-coretext.h"

#include "hb-draw.hh"
#include "hb-font.hh"
#include "hb-machinery.hh"

#if MAC_OS_X_VERSION_MIN_REQUIRED < 101100
#  define kCTFontOrientationDefault kCTFontDefaultOrientation
#endif

#define MAX_GLYPHS 64u

static z0
_hb_coretext_font_destroy (uk font_data)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  CFRelease (ct_font);
}

static hb_bool_t
hb_coretext_get_nominal_glyph (hb_font_t *font HB_UNUSED,
			       uk font_data,
			       hb_codepoint_t unicode,
			       hb_codepoint_t *glyph,
			       uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;
  UniChar ch = unicode;
  CGGlyph cg_glyph;
  if (CTFontGetGlyphsForCharacters (ct_font, &ch, &cg_glyph, 1))
  {
    *glyph = cg_glyph;
    return true;
  }
  return false;
}

static u32
hb_coretext_get_nominal_glyphs (hb_font_t *font HB_UNUSED,
				uk font_data,
				u32 count,
				const hb_codepoint_t *first_unicode,
				u32 unicode_stride,
				hb_codepoint_t *first_glyph,
				u32 glyph_stride,
				uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  UniChar ch[MAX_GLYPHS];
  CGGlyph cg_glyph[MAX_GLYPHS];
  for (u32 i = 0; i < count; i += MAX_GLYPHS)
  {
    u32 c = (u32) hb_min ((i32) MAX_GLYPHS, (i32) count - (i32) i);
    for (u32 j = 0; j < c; j++)
    {
      ch[j] = *first_unicode;
      first_unicode = &StructAtOffset<const hb_codepoint_t> (first_unicode, unicode_stride);
    }
    CTFontGetGlyphsForCharacters (ct_font, ch, cg_glyph, c);
    for (u32 j = 0; j < c; j++)
    {
      *first_glyph = cg_glyph[j];
      first_glyph = &StructAtOffset<hb_codepoint_t> (first_glyph, glyph_stride);
    }
  }

  return count;
}

static hb_bool_t
hb_coretext_get_variation_glyph (hb_font_t *font HB_UNUSED,
				 uk font_data,
				 hb_codepoint_t unicode,
				 hb_codepoint_t variation_selector,
				 hb_codepoint_t *glyph,
				 uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  UniChar ch[2] = { unicode, variation_selector };
  CGGlyph cg_glyph[2];

  CTFontGetGlyphsForCharacters (ct_font, ch, cg_glyph, 2);

  if (cg_glyph[1])
    return false;

  *glyph = cg_glyph[0];
  return true;
}

static z0
hb_coretext_get_glyph_h_advances (hb_font_t* font, uk font_data,
				  u32 count,
				  const hb_codepoint_t *first_glyph,
				  u32 glyph_stride,
				  hb_position_t *first_advance,
				  u32 advance_stride,
				  uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  CGFloat ct_font_size = CTFontGetSize (ct_font);
  CGFloat x_mult = (CGFloat) font->x_scale / ct_font_size;

  CGGlyph cg_glyph[MAX_GLYPHS];
  CGSize advances[MAX_GLYPHS];
  for (u32 i = 0; i < count; i += MAX_GLYPHS)
  {
    u32 c = (u32) hb_min ((i32) MAX_GLYPHS, (i32) count - (i32) i);
    for (u32 j = 0; j < c; j++)
    {
      cg_glyph[j] = *first_glyph;
      first_glyph = &StructAtOffset<const hb_codepoint_t> (first_glyph, glyph_stride);
    }
    CTFontGetAdvancesForGlyphs (ct_font, kCTFontOrientationHorizontal, cg_glyph, advances, c);
    for (u32 j = 0; j < c; j++)
    {
      *first_advance = round (advances[j].width * x_mult);
      first_advance = &StructAtOffset<hb_position_t> (first_advance, advance_stride);
    }
  }
}

#ifndef HB_NO_VERTICAL
static z0
hb_coretext_get_glyph_v_advances (hb_font_t* font, uk font_data,
				  u32 count,
				  const hb_codepoint_t *first_glyph,
				  u32 glyph_stride,
				  hb_position_t *first_advance,
				  u32 advance_stride,
				  uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  CGFloat ct_font_size = CTFontGetSize (ct_font);
  CGFloat y_mult = (CGFloat) -font->y_scale / ct_font_size;

  CGGlyph cg_glyph[MAX_GLYPHS];
  CGSize advances[MAX_GLYPHS];
  for (u32 i = 0; i < count; i += MAX_GLYPHS)
  {
    u32 c = (u32) hb_min ((i32) MAX_GLYPHS, (i32) count - (i32) i);
    for (u32 j = 0; j < c; j++)
    {
      cg_glyph[j] = *first_glyph;
      first_glyph = &StructAtOffset<const hb_codepoint_t> (first_glyph, glyph_stride);
    }
    CTFontGetAdvancesForGlyphs (ct_font, kCTFontOrientationVertical, cg_glyph, advances, c);
    for (u32 j = 0; j < c; j++)
    {
      *first_advance = round (advances[j].width * y_mult);
      first_advance = &StructAtOffset<hb_position_t> (first_advance, advance_stride);
    }
  }
}
#endif

#ifndef HB_NO_VERTICAL
static hb_bool_t
hb_coretext_get_glyph_v_origin (hb_font_t *font,
				uk font_data,
				hb_codepoint_t glyph,
				hb_position_t *x,
				hb_position_t *y,
				uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  CGFloat ct_font_size = CTFontGetSize (ct_font);
  CGFloat x_mult = (CGFloat) -font->x_scale / ct_font_size;
  CGFloat y_mult = (CGFloat) -font->y_scale / ct_font_size;

  const CGGlyph glyphs = glyph;
  CGSize origin;
  CTFontGetVerticalTranslationsForGlyphs (ct_font, &glyphs, &origin, 1);

  *x = round (x_mult * origin.width);
  *y = round (y_mult * origin.height);

  return true;
}
#endif

static hb_bool_t
hb_coretext_get_glyph_extents (hb_font_t *font,
			       uk font_data,
			       hb_codepoint_t glyph,
			       hb_glyph_extents_t *extents,
			       uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  CGFloat ct_font_size = CTFontGetSize (ct_font);
  CGFloat x_mult = (CGFloat) font->x_scale / ct_font_size;
  CGFloat y_mult = (CGFloat) font->y_scale / ct_font_size;

  CGGlyph glyphs[1] = { glyph };
  CGRect bounds = ::CTFontGetBoundingRectsForGlyphs(ct_font,
						    kCTFontOrientationDefault, glyphs, NULL, 1);

  extents->x_bearing = round (bounds.origin.x * x_mult);
  extents->y_bearing = round (bounds.origin.y * y_mult);
  extents->width = round (bounds.size.width * x_mult);
  extents->height = round (bounds.size.height * y_mult);

  return true;
}

static hb_bool_t
hb_coretext_get_font_h_extents (hb_font_t *font,
				uk font_data,
				hb_font_extents_t *metrics,
				uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;
  CGFloat ct_font_size = CTFontGetSize (ct_font);
  CGFloat y_mult = (CGFloat) font->y_scale / ct_font_size;

  metrics->ascender = round (CTFontGetAscent (ct_font) * y_mult);
  metrics->descender = -round (CTFontGetDescent (ct_font) * y_mult);
  metrics->line_gap = round (CTFontGetLeading (ct_font) * y_mult);

  return true;
}

#ifndef HB_NO_DRAW

static z0
ct_apply_func (uk info, const CGPathElement *element)
{
  hb_draw_session_t *draws = (hb_draw_session_t *) info;

  switch (element->type)
  {
  case kCGPathElementMoveToPoint:
    draws->move_to (element->points[0].x, element->points[0].y);
    break;
  case kCGPathElementAddLineToPoint:
    draws->line_to (element->points[0].x, element->points[0].y);
    break;
  case kCGPathElementAddQuadCurveToPoint:
    draws->quadratic_to (element->points[0].x, element->points[0].y,
			 element->points[1].x, element->points[1].y);
    break;
  case kCGPathElementAddCurveToPoint:
    draws->cubic_to (element->points[0].x, element->points[0].y,
		     element->points[1].x, element->points[1].y,
		     element->points[2].x, element->points[2].y);
    break;
  case kCGPathElementCloseSubpath:
    draws->close_path ();
    break;
  }
}

static z0
hb_coretext_draw_glyph (hb_font_t *font,
			uk font_data HB_UNUSED,
			hb_codepoint_t glyph,
			hb_draw_funcs_t *draw_funcs, uk draw_data,
			uk user_data)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  CGFloat ct_font_size = CTFontGetSize (ct_font);
  CGFloat x_mult = (CGFloat) font->x_scale / ct_font_size;
  CGFloat y_mult = (CGFloat) font->y_scale / ct_font_size;

  CGAffineTransform transform = CGAffineTransformIdentity;
  transform = CGAffineTransformScale (transform, x_mult, y_mult);

  CGPathRef path = CTFontCreatePathForGlyph (ct_font, glyph, &transform);
  if (!path)
    return;

  hb_draw_session_t drawing = {draw_funcs, draw_data, font->slant};

  CGPathApply (path, &drawing, ct_apply_func);

  CFRelease (path);
}
#endif

static hb_bool_t
hb_coretext_get_glyph_name (hb_font_t *font,
			    uk font_data HB_UNUSED,
			    hb_codepoint_t glyph,
			    t8 *name, u32 size,
			    uk user_data HB_UNUSED)
{
  CGFontRef cg_font = (CGFontRef) (ukk ) font->face->data.coretext;

  CGGlyph cg_glyph = glyph;
  CFStringRef cf_name = CGFontCopyGlyphNameForGlyph (cg_font, cg_glyph);
  if (!cf_name)
    return false;

  CFIndex len = CFStringGetLength (cf_name);
  if (len > size - 1)
    len = size - 1;

  CFStringGetBytes (cf_name, CFRangeMake (0, len),
		    kCFStringEncodingUTF8, 0, false,
		    (UInt8 *) name, size, &len);

  name[len] = '\0';
  return true;
}

static hb_bool_t
hb_coretext_get_glyph_from_name (hb_font_t *font HB_UNUSED,
				 uk font_data,
				 const t8 *name, i32 len,
				 hb_codepoint_t *glyph,
				 uk user_data HB_UNUSED)
{
  CTFontRef ct_font = (CTFontRef) font_data;

  if (len == -1)
    len = strlen (name);

  CFStringRef cf_name = CFStringCreateWithBytes (kCFAllocatorDefault,
						 (const UInt8 *) name, len,
						 kCFStringEncodingUTF8, false);
  CGGlyph cg_glyph = CTFontGetGlyphWithName (ct_font, cf_name);
  *glyph = cg_glyph;

  CFRelease (cf_name);

  // TODO Return true for .notdef; hb-ft does that.

  return cg_glyph != 0;
}


static inline z0 free_static_coretext_funcs ();

static struct hb_coretext_font_funcs_lazy_loader_t : hb_font_funcs_lazy_loader_t<hb_coretext_font_funcs_lazy_loader_t>
{
  static hb_font_funcs_t *create ()
  {
    hb_font_funcs_t *funcs = hb_font_funcs_create ();

    hb_font_funcs_set_nominal_glyph_func (funcs, hb_coretext_get_nominal_glyph, nullptr, nullptr);
    hb_font_funcs_set_nominal_glyphs_func (funcs, hb_coretext_get_nominal_glyphs, nullptr, nullptr);
    hb_font_funcs_set_variation_glyph_func (funcs, hb_coretext_get_variation_glyph, nullptr, nullptr);

    hb_font_funcs_set_font_h_extents_func (funcs, hb_coretext_get_font_h_extents, nullptr, nullptr);
    hb_font_funcs_set_glyph_h_advances_func (funcs, hb_coretext_get_glyph_h_advances, nullptr, nullptr);
    //hb_font_funcs_set_glyph_h_origin_func (funcs, hb_coretext_get_glyph_h_origin, nullptr, nullptr);

#ifndef HB_NO_VERTICAL
    //hb_font_funcs_set_font_v_extents_func (funcs, hb_coretext_get_font_v_extents, nullptr, nullptr);
    hb_font_funcs_set_glyph_v_advances_func (funcs, hb_coretext_get_glyph_v_advances, nullptr, nullptr);
    hb_font_funcs_set_glyph_v_origin_func (funcs, hb_coretext_get_glyph_v_origin, nullptr, nullptr);
#endif

#ifndef HB_NO_DRAW
    hb_font_funcs_set_draw_glyph_func (funcs, hb_coretext_draw_glyph, nullptr, nullptr);
#endif

    hb_font_funcs_set_glyph_extents_func (funcs, hb_coretext_get_glyph_extents, nullptr, nullptr);

#ifndef HB_NO_OT_FONT_GLYPH_NAMES
    hb_font_funcs_set_glyph_name_func (funcs, hb_coretext_get_glyph_name, nullptr, nullptr);
    hb_font_funcs_set_glyph_from_name_func (funcs, hb_coretext_get_glyph_from_name, nullptr, nullptr);
#endif

    hb_font_funcs_make_immutable (funcs);

    hb_atexit (free_static_coretext_funcs);

    return funcs;
  }
} static_coretext_funcs;

static inline
z0 free_static_coretext_funcs ()
{
  static_coretext_funcs.free_instance ();
}

static hb_font_funcs_t *
_hb_coretext_get_font_funcs ()
{
  return static_coretext_funcs.get_unconst ();
}


/**
 * hb_coretext_font_set_funcs:
 * @font: #hb_font_t to work upon
 *
 * Configures the font-functions structure of the specified
 * #hb_font_t font object to use CoreText font functions.
 *
 * In particular, you can use this function to configure an
 * existing #hb_face_t face object for use with CoreText font
 * functions even if that #hb_face_t face object was initially
 * created with hb_face_create(), and therefore was not
 * initially configured to use CoreText font functions.
 *
 * An #hb_font_t object created with hb_coretext_font_create()
 * is preconfigured for CoreText font functions and does not
 * require this function to be used.
 *
 * <note>Note: Internally, this function creates a CTFont.
* </note>
 *
 * Since: 10.1.0
 **/
z0
hb_coretext_font_set_funcs (hb_font_t *font)
{
  CTFontRef ct_font = hb_coretext_font_get_ct_font (font);
  if (unlikely (!ct_font))
    return;

  hb_font_set_funcs (font,
		     _hb_coretext_get_font_funcs (),
		     (uk ) CFRetain (ct_font),
		     _hb_coretext_font_destroy);
}

#undef MAX_GLYPHS

#endif
