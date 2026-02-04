/*
 * Copyright © 2022 Matthias Clasen
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
 */

#ifndef HB_PAINT_HH
#define HB_PAINT_HH

#include "hb.hh"
#include "hb-face.hh"
#include "hb-font.hh"

#define HB_PAINT_FUNCS_IMPLEMENT_CALLBACKS \
  HB_PAINT_FUNC_IMPLEMENT (push_transform) \
  HB_PAINT_FUNC_IMPLEMENT (pop_transform) \
  HB_PAINT_FUNC_IMPLEMENT (color_glyph) \
  HB_PAINT_FUNC_IMPLEMENT (push_clip_glyph) \
  HB_PAINT_FUNC_IMPLEMENT (push_clip_rectangle) \
  HB_PAINT_FUNC_IMPLEMENT (pop_clip) \
  HB_PAINT_FUNC_IMPLEMENT (color) \
  HB_PAINT_FUNC_IMPLEMENT (image) \
  HB_PAINT_FUNC_IMPLEMENT (linear_gradient) \
  HB_PAINT_FUNC_IMPLEMENT (radial_gradient) \
  HB_PAINT_FUNC_IMPLEMENT (sweep_gradient) \
  HB_PAINT_FUNC_IMPLEMENT (push_group) \
  HB_PAINT_FUNC_IMPLEMENT (pop_group) \
  HB_PAINT_FUNC_IMPLEMENT (custom_palette_color) \
  /* ^--- Add new callbacks here */

struct hb_paint_funcs_t
{
  hb_object_header_t header;

  struct {
#define HB_PAINT_FUNC_IMPLEMENT(name) hb_paint_##name##_func_t name;
    HB_PAINT_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_PAINT_FUNC_IMPLEMENT
  } func;

  struct {
#define HB_PAINT_FUNC_IMPLEMENT(name) uk name;
    HB_PAINT_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_PAINT_FUNC_IMPLEMENT
  } *user_data;

  struct {
#define HB_PAINT_FUNC_IMPLEMENT(name) hb_destroy_func_t name;
    HB_PAINT_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_PAINT_FUNC_IMPLEMENT
  } *destroy;

  z0 push_transform (uk paint_data,
                       f32 xx, f32 yx,
                       f32 xy, f32 yy,
                       f32 dx, f32 dy)
  { func.push_transform (this, paint_data,
                         xx, yx, xy, yy, dx, dy,
                         !user_data ? nullptr : user_data->push_transform); }
  z0 pop_transform (uk paint_data)
  { func.pop_transform (this, paint_data,
                        !user_data ? nullptr : user_data->pop_transform); }
  b8 color_glyph (uk paint_data,
                    hb_codepoint_t glyph,
                    hb_font_t *font)
  { return func.color_glyph (this, paint_data,
                             glyph,
                             font,
                             !user_data ? nullptr : user_data->push_clip_glyph); }
  z0 push_clip_glyph (uk paint_data,
                        hb_codepoint_t glyph,
                        hb_font_t *font)
  { func.push_clip_glyph (this, paint_data,
                          glyph,
                          font,
                          !user_data ? nullptr : user_data->push_clip_glyph); }
  z0 push_clip_rectangle (uk paint_data,
                           f32 xmin, f32 ymin, f32 xmax, f32 ymax)
  { func.push_clip_rectangle (this, paint_data,
                              xmin, ymin, xmax, ymax,
                              !user_data ? nullptr : user_data->push_clip_rectangle); }
  z0 pop_clip (uk paint_data)
  { func.pop_clip (this, paint_data,
                   !user_data ? nullptr : user_data->pop_clip); }
  z0 color (uk paint_data,
              hb_bool_t is_foreground,
              hb_color_t color)
  { func.color (this, paint_data,
                is_foreground, color,
                !user_data ? nullptr : user_data->color); }
  b8 image (uk paint_data,
              hb_blob_t *image,
              u32 width, u32 height,
              hb_tag_t format,
              f32 slant,
              hb_glyph_extents_t *extents)
  { return func.image (this, paint_data,
                       image, width, height, format, slant, extents,
                       !user_data ? nullptr : user_data->image); }
  z0 linear_gradient (uk paint_data,
                        hb_color_line_t *color_line,
                        f32 x0, f32 y0,
                        f32 x1, f32 y1,
                        f32 x2, f32 y2)
  { func.linear_gradient (this, paint_data,
                          color_line, x0, y0, x1, y1, x2, y2,
                          !user_data ? nullptr : user_data->linear_gradient); }
  z0 radial_gradient (uk paint_data,
                        hb_color_line_t *color_line,
                        f32 x0, f32 y0, f32 r0,
                        f32 x1, f32 y1, f32 r1)
  { func.radial_gradient (this, paint_data,
                          color_line, x0, y0, r0, x1, y1, r1,
                          !user_data ? nullptr : user_data->radial_gradient); }
  z0 sweep_gradient (uk paint_data,
                       hb_color_line_t *color_line,
                       f32 x0, f32 y0,
                       f32 start_angle,
                       f32 end_angle)
  { func.sweep_gradient (this, paint_data,
                         color_line, x0, y0, start_angle, end_angle,
                         !user_data ? nullptr : user_data->sweep_gradient); }
  z0 push_group (uk paint_data)
  { func.push_group (this, paint_data,
                     !user_data ? nullptr : user_data->push_group); }
  z0 pop_group (uk paint_data,
                  hb_paint_composite_mode_t mode)
  { func.pop_group (this, paint_data,
                    mode,
                    !user_data ? nullptr : user_data->pop_group); }
  b8 custom_palette_color (uk paint_data,
                             u32 color_index,
                             hb_color_t *color)
  { return func.custom_palette_color (this, paint_data,
                                      color_index,
                                      color,
                                      !user_data ? nullptr : user_data->custom_palette_color); }


  /* Internal specializations. */

  z0 push_root_transform (uk paint_data,
                            const hb_font_t *font)
  {
    f32 upem = font->face->get_upem ();
    i32 xscale = font->x_scale, yscale = font->y_scale;
    f32 slant = font->slant_xy;

    push_transform (paint_data,
		    xscale/upem, 0, slant * yscale/upem, yscale/upem, 0, 0);
  }

  z0 push_inverse_root_transform (uk paint_data,
                                    hb_font_t *font)
  {
    f32 upem = font->face->get_upem ();
    i32 xscale = font->x_scale ? font->x_scale : upem;
    i32 yscale = font->y_scale ? font->y_scale : upem;
    f32 slant = font->slant_xy;

    push_transform (paint_data,
		    upem/xscale, 0, -slant * upem/xscale, upem/yscale, 0, 0);
  }

  HB_NODISCARD
  b8 push_translate (uk paint_data,
                       f32 dx, f32 dy)
  {
    if (!dx && !dy)
      return false;

    push_transform (paint_data,
		    1.f, 0.f, 0.f, 1.f, dx, dy);
    return true;
  }

  HB_NODISCARD
  b8 push_scale (uk paint_data,
                   f32 sx, f32 sy)
  {
    if (sx == 1.f && sy == 1.f)
      return false;

    push_transform (paint_data,
		    sx, 0.f, 0.f, sy, 0.f, 0.f);
    return true;
  }

  HB_NODISCARD
  b8 push_rotate (uk paint_data,
                    f32 a)
  {
    if (!a)
      return false;

    f32 cc = cosf (a * HB_PI);
    f32 ss = sinf (a * HB_PI);
    push_transform (paint_data, cc, ss, -ss, cc, 0.f, 0.f);
    return true;
  }

  HB_NODISCARD
  b8 push_skew (uk paint_data,
                  f32 sx, f32 sy)
  {
    if (!sx && !sy)
      return false;

    f32 x = tanf (-sx * HB_PI);
    f32 y = tanf (+sy * HB_PI);
    push_transform (paint_data, 1.f, y, x, 1.f, 0.f, 0.f);
    return true;
  }
};
DECLARE_NULL_INSTANCE (hb_paint_funcs_t);


#endif /* HB_PAINT_HH */
