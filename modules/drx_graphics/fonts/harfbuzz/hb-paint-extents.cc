/*
 * Copyright © 2022 Behdad Esfahbod
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

#include "hb.hh"

#ifndef HB_NO_PAINT

#include "hb-paint-extents.hh"

#include "hb-draw.h"

#include "hb-machinery.hh"


/*
 * This file implements bounds-extraction as well as boundedness
 * computation of COLRv1 fonts as described in:
 *
 * https://learn.microsoft.com/en-us/typography/opentype/spec/colr#glyph-metrics-and-boundedness
 */

static z0
hb_paint_extents_push_transform (hb_paint_funcs_t *funcs HB_UNUSED,
				 uk paint_data,
				 f32 xx, f32 yx,
				 f32 xy, f32 yy,
				 f32 dx, f32 dy,
				 uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->push_transform (hb_transform_t {xx, yx, xy, yy, dx, dy});
}

static z0
hb_paint_extents_pop_transform (hb_paint_funcs_t *funcs HB_UNUSED,
			        uk paint_data,
				uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->pop_transform ();
}

static z0
hb_draw_extents_move_to (hb_draw_funcs_t *dfuncs HB_UNUSED,
			 uk data,
			 hb_draw_state_t *st,
			 f32 to_x, f32 to_y,
			 uk user_data HB_UNUSED)
{
  hb_extents_t *extents = (hb_extents_t *) data;

  extents->add_point (to_x, to_y);
}

static z0
hb_draw_extents_line_to (hb_draw_funcs_t *dfuncs HB_UNUSED,
			 uk data,
			 hb_draw_state_t *st,
			 f32 to_x, f32 to_y,
			 uk user_data HB_UNUSED)
{
  hb_extents_t *extents = (hb_extents_t *) data;

  extents->add_point (to_x, to_y);
}

static z0
hb_draw_extents_quadratic_to (hb_draw_funcs_t *dfuncs HB_UNUSED,
			      uk data,
			      hb_draw_state_t *st,
			      f32 control_x, f32 control_y,
			      f32 to_x, f32 to_y,
			      uk user_data HB_UNUSED)
{
  hb_extents_t *extents = (hb_extents_t *) data;

  extents->add_point (control_x, control_y);
  extents->add_point (to_x, to_y);
}

static z0
hb_draw_extents_cubic_to (hb_draw_funcs_t *dfuncs HB_UNUSED,
			  uk data,
			  hb_draw_state_t *st,
			  f32 control1_x, f32 control1_y,
			  f32 control2_x, f32 control2_y,
			  f32 to_x, f32 to_y,
			  uk user_data HB_UNUSED)
{
  hb_extents_t *extents = (hb_extents_t *) data;

  extents->add_point (control1_x, control1_y);
  extents->add_point (control2_x, control2_y);
  extents->add_point (to_x, to_y);
}

static inline z0 free_static_draw_extents_funcs ();

static struct hb_draw_extents_funcs_lazy_loader_t : hb_draw_funcs_lazy_loader_t<hb_draw_extents_funcs_lazy_loader_t>
{
  static hb_draw_funcs_t *create ()
  {
    hb_draw_funcs_t *funcs = hb_draw_funcs_create ();

    hb_draw_funcs_set_move_to_func (funcs, hb_draw_extents_move_to, nullptr, nullptr);
    hb_draw_funcs_set_line_to_func (funcs, hb_draw_extents_line_to, nullptr, nullptr);
    hb_draw_funcs_set_quadratic_to_func (funcs, hb_draw_extents_quadratic_to, nullptr, nullptr);
    hb_draw_funcs_set_cubic_to_func (funcs, hb_draw_extents_cubic_to, nullptr, nullptr);

    hb_draw_funcs_make_immutable (funcs);

    hb_atexit (free_static_draw_extents_funcs);

    return funcs;
  }
} static_draw_extents_funcs;

static inline
z0 free_static_draw_extents_funcs ()
{
  static_draw_extents_funcs.free_instance ();
}

static hb_draw_funcs_t *
hb_draw_extents_get_funcs ()
{
  return static_draw_extents_funcs.get_unconst ();
}

static z0
hb_paint_extents_push_clip_glyph (hb_paint_funcs_t *funcs HB_UNUSED,
				  uk paint_data,
				  hb_codepoint_t glyph,
				  hb_font_t *font,
				  uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  hb_extents_t extents;
  hb_draw_funcs_t *draw_extent_funcs = hb_draw_extents_get_funcs ();
  hb_font_draw_glyph (font, glyph, draw_extent_funcs, &extents);
  c->push_clip (extents);
}

static z0
hb_paint_extents_push_clip_rectangle (hb_paint_funcs_t *funcs HB_UNUSED,
				      uk paint_data,
				      f32 xmin, f32 ymin, f32 xmax, f32 ymax,
				      uk user_data)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  hb_extents_t extents = {xmin, ymin, xmax, ymax};
  c->push_clip (extents);
}

static z0
hb_paint_extents_pop_clip (hb_paint_funcs_t *funcs HB_UNUSED,
			   uk paint_data,
			   uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->pop_clip ();
}

static z0
hb_paint_extents_push_group (hb_paint_funcs_t *funcs HB_UNUSED,
			     uk paint_data,
			     uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->push_group ();
}

static z0
hb_paint_extents_pop_group (hb_paint_funcs_t *funcs HB_UNUSED,
			    uk paint_data,
			    hb_paint_composite_mode_t mode,
			    uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->pop_group (mode);
}

static hb_bool_t
hb_paint_extents_paint_image (hb_paint_funcs_t *funcs HB_UNUSED,
			      uk paint_data,
			      hb_blob_t *blob HB_UNUSED,
			      u32 width HB_UNUSED,
			      u32 height HB_UNUSED,
			      hb_tag_t format HB_UNUSED,
			      f32 slant HB_UNUSED,
			      hb_glyph_extents_t *glyph_extents,
			      uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  hb_extents_t extents = {(f32) glyph_extents->x_bearing,
			  (f32) glyph_extents->y_bearing + glyph_extents->height,
			  (f32) glyph_extents->x_bearing + glyph_extents->width,
			  (f32) glyph_extents->y_bearing};
  c->push_clip (extents);
  c->paint ();
  c->pop_clip ();

  return true;
}

static z0
hb_paint_extents_paint_color (hb_paint_funcs_t *funcs HB_UNUSED,
			      uk paint_data,
			      hb_bool_t use_foreground HB_UNUSED,
			      hb_color_t color HB_UNUSED,
			      uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->paint ();
}

static z0
hb_paint_extents_paint_linear_gradient (hb_paint_funcs_t *funcs HB_UNUSED,
				        uk paint_data,
				        hb_color_line_t *color_line HB_UNUSED,
				        f32 x0 HB_UNUSED, f32 y0 HB_UNUSED,
				        f32 x1 HB_UNUSED, f32 y1 HB_UNUSED,
				        f32 x2 HB_UNUSED, f32 y2 HB_UNUSED,
				        uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->paint ();
}

static z0
hb_paint_extents_paint_radial_gradient (hb_paint_funcs_t *funcs HB_UNUSED,
				        uk paint_data,
				        hb_color_line_t *color_line HB_UNUSED,
				        f32 x0 HB_UNUSED, f32 y0 HB_UNUSED, f32 r0 HB_UNUSED,
				        f32 x1 HB_UNUSED, f32 y1 HB_UNUSED, f32 r1 HB_UNUSED,
				        uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->paint ();
}

static z0
hb_paint_extents_paint_sweep_gradient (hb_paint_funcs_t *funcs HB_UNUSED,
				       uk paint_data,
				       hb_color_line_t *color_line HB_UNUSED,
				       f32 cx HB_UNUSED, f32 cy HB_UNUSED,
				       f32 start_angle HB_UNUSED,
				       f32 end_angle HB_UNUSED,
				       uk user_data HB_UNUSED)
{
  hb_paint_extents_context_t *c = (hb_paint_extents_context_t *) paint_data;

  c->paint ();
}

static inline z0 free_static_paint_extents_funcs ();

static struct hb_paint_extents_funcs_lazy_loader_t : hb_paint_funcs_lazy_loader_t<hb_paint_extents_funcs_lazy_loader_t>
{
  static hb_paint_funcs_t *create ()
  {
    hb_paint_funcs_t *funcs = hb_paint_funcs_create ();

    hb_paint_funcs_set_push_transform_func (funcs, hb_paint_extents_push_transform, nullptr, nullptr);
    hb_paint_funcs_set_pop_transform_func (funcs, hb_paint_extents_pop_transform, nullptr, nullptr);
    hb_paint_funcs_set_push_clip_glyph_func (funcs, hb_paint_extents_push_clip_glyph, nullptr, nullptr);
    hb_paint_funcs_set_push_clip_rectangle_func (funcs, hb_paint_extents_push_clip_rectangle, nullptr, nullptr);
    hb_paint_funcs_set_pop_clip_func (funcs, hb_paint_extents_pop_clip, nullptr, nullptr);
    hb_paint_funcs_set_push_group_func (funcs, hb_paint_extents_push_group, nullptr, nullptr);
    hb_paint_funcs_set_pop_group_func (funcs, hb_paint_extents_pop_group, nullptr, nullptr);
    hb_paint_funcs_set_color_func (funcs, hb_paint_extents_paint_color, nullptr, nullptr);
    hb_paint_funcs_set_image_func (funcs, hb_paint_extents_paint_image, nullptr, nullptr);
    hb_paint_funcs_set_linear_gradient_func (funcs, hb_paint_extents_paint_linear_gradient, nullptr, nullptr);
    hb_paint_funcs_set_radial_gradient_func (funcs, hb_paint_extents_paint_radial_gradient, nullptr, nullptr);
    hb_paint_funcs_set_sweep_gradient_func (funcs, hb_paint_extents_paint_sweep_gradient, nullptr, nullptr);

    hb_paint_funcs_make_immutable (funcs);

    hb_atexit (free_static_paint_extents_funcs);

    return funcs;
  }
} static_paint_extents_funcs;

static inline
z0 free_static_paint_extents_funcs ()
{
  static_paint_extents_funcs.free_instance ();
}

hb_paint_funcs_t *
hb_paint_extents_get_funcs ()
{
  return static_paint_extents_funcs.get_unconst ();
}


#endif
