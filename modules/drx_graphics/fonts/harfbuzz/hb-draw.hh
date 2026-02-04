/*
 * Copyright © 2020  Ebrahim Byagowi
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

#ifndef HB_DRAW_HH
#define HB_DRAW_HH

#include "hb.hh"


/*
 * hb_draw_funcs_t
 */

#define HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS \
  HB_DRAW_FUNC_IMPLEMENT (move_to) \
  HB_DRAW_FUNC_IMPLEMENT (line_to) \
  HB_DRAW_FUNC_IMPLEMENT (quadratic_to) \
  HB_DRAW_FUNC_IMPLEMENT (cubic_to) \
  HB_DRAW_FUNC_IMPLEMENT (close_path) \
  /* ^--- Add new callbacks here */

struct hb_draw_funcs_t
{
  hb_object_header_t header;

  struct {
#define HB_DRAW_FUNC_IMPLEMENT(name) hb_draw_##name##_func_t name;
    HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_DRAW_FUNC_IMPLEMENT
  } func;

  struct {
#define HB_DRAW_FUNC_IMPLEMENT(name) uk name;
    HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_DRAW_FUNC_IMPLEMENT
  } *user_data;

  struct {
#define HB_DRAW_FUNC_IMPLEMENT(name) hb_destroy_func_t name;
    HB_DRAW_FUNCS_IMPLEMENT_CALLBACKS
#undef HB_DRAW_FUNC_IMPLEMENT
  } *destroy;

  z0 emit_move_to (uk draw_data, hb_draw_state_t &st,
		     f32 to_x, f32 to_y)
  { func.move_to (this, draw_data, &st,
		  to_x, to_y,
		  !user_data ? nullptr : user_data->move_to); }
  z0 emit_line_to (uk draw_data, hb_draw_state_t &st,
		     f32 to_x, f32 to_y)
  { func.line_to (this, draw_data, &st,
		  to_x, to_y,
		  !user_data ? nullptr : user_data->line_to); }
  z0 emit_quadratic_to (uk draw_data, hb_draw_state_t &st,
			  f32 control_x, f32 control_y,
			  f32 to_x, f32 to_y)
  { func.quadratic_to (this, draw_data, &st,
		       control_x, control_y,
		       to_x, to_y,
		       !user_data ? nullptr : user_data->quadratic_to); }
  z0 emit_cubic_to (uk draw_data, hb_draw_state_t &st,
		      f32 control1_x, f32 control1_y,
		      f32 control2_x, f32 control2_y,
		      f32 to_x, f32 to_y)
  { func.cubic_to (this, draw_data, &st,
		   control1_x, control1_y,
		   control2_x, control2_y,
		   to_x, to_y,
		   !user_data ? nullptr : user_data->cubic_to); }
  z0 emit_close_path (uk draw_data, hb_draw_state_t &st)
  { func.close_path (this, draw_data, &st,
		     !user_data ? nullptr : user_data->close_path); }


  z0
  HB_ALWAYS_INLINE
  move_to (uk draw_data, hb_draw_state_t &st,
	   f32 to_x, f32 to_y)
  {
    if (unlikely (st.path_open)) close_path (draw_data, st);
    st.current_x = to_x;
    st.current_y = to_y;
  }

  z0
  HB_ALWAYS_INLINE
  line_to (uk draw_data, hb_draw_state_t &st,
	   f32 to_x, f32 to_y)
  {
    if (unlikely (!st.path_open)) start_path (draw_data, st);
    emit_line_to (draw_data, st, to_x, to_y);
    st.current_x = to_x;
    st.current_y = to_y;
  }

  z0
  HB_ALWAYS_INLINE
  quadratic_to (uk draw_data, hb_draw_state_t &st,
		f32 control_x, f32 control_y,
		f32 to_x, f32 to_y)
  {
    if (unlikely (!st.path_open)) start_path (draw_data, st);
    emit_quadratic_to (draw_data, st, control_x, control_y, to_x, to_y);
    st.current_x = to_x;
    st.current_y = to_y;
  }

  z0
  HB_ALWAYS_INLINE
  cubic_to (uk draw_data, hb_draw_state_t &st,
	    f32 control1_x, f32 control1_y,
	    f32 control2_x, f32 control2_y,
	    f32 to_x, f32 to_y)
  {
    if (unlikely (!st.path_open)) start_path (draw_data, st);
    emit_cubic_to (draw_data, st, control1_x, control1_y, control2_x, control2_y, to_x, to_y);
    st.current_x = to_x;
    st.current_y = to_y;
  }

  z0
  HB_ALWAYS_INLINE
  close_path (uk draw_data, hb_draw_state_t &st)
  {
    if (likely (st.path_open))
    {
      if ((st.path_start_x != st.current_x) || (st.path_start_y != st.current_y))
	emit_line_to (draw_data, st, st.path_start_x, st.path_start_y);
      emit_close_path (draw_data, st);
    }
    st.path_open = false;
    st.path_start_x = st.current_x = st.path_start_y = st.current_y = 0;
  }

  protected:

  z0 start_path (uk draw_data, hb_draw_state_t &st)
  {
    assert (!st.path_open);
    emit_move_to (draw_data, st, st.current_x, st.current_y);
    st.path_open = true;
    st.path_start_x = st.current_x;
    st.path_start_y = st.current_y;
  }
};
DECLARE_NULL_INSTANCE (hb_draw_funcs_t);

struct hb_draw_session_t
{
  hb_draw_session_t (hb_draw_funcs_t *funcs_, uk draw_data_, f32 slant_ = 0.f)
    : slant {slant_}, not_slanted {slant == 0.f},
      funcs {funcs_}, draw_data {draw_data_}, st HB_DRAW_STATE_DEFAULT
  {}

  ~hb_draw_session_t () { close_path (); }

  HB_ALWAYS_INLINE
  z0 move_to (f32 to_x, f32 to_y)
  {
    if (likely (not_slanted))
      funcs->move_to (draw_data, st,
		      to_x, to_y);
    else
      funcs->move_to (draw_data, st,
		      to_x + to_y * slant, to_y);
  }
  HB_ALWAYS_INLINE
  z0 line_to (f32 to_x, f32 to_y)
  {
    if (likely (not_slanted))
      funcs->line_to (draw_data, st,
		      to_x, to_y);
    else
      funcs->line_to (draw_data, st,
		      to_x + to_y * slant, to_y);
  }
  z0
  HB_ALWAYS_INLINE
  quadratic_to (f32 control_x, f32 control_y,
		f32 to_x, f32 to_y)
  {
    if (likely (not_slanted))
      funcs->quadratic_to (draw_data, st,
			   control_x, control_y,
			   to_x, to_y);
    else
      funcs->quadratic_to (draw_data, st,
			   control_x + control_y * slant, control_y,
			   to_x + to_y * slant, to_y);
  }
  z0
  HB_ALWAYS_INLINE
  cubic_to (f32 control1_x, f32 control1_y,
	    f32 control2_x, f32 control2_y,
	    f32 to_x, f32 to_y)
  {
    if (likely (not_slanted))
      funcs->cubic_to (draw_data, st,
		       control1_x, control1_y,
		       control2_x, control2_y,
		       to_x, to_y);
    else
      funcs->cubic_to (draw_data, st,
		       control1_x + control1_y * slant, control1_y,
		       control2_x + control2_y * slant, control2_y,
		       to_x + to_y * slant, to_y);
  }
  HB_ALWAYS_INLINE
  z0 close_path ()
  {
    funcs->close_path (draw_data, st);
  }

  public:
  f32 slant;
  b8 not_slanted;
  hb_draw_funcs_t *funcs;
  uk draw_data;
  hb_draw_state_t st;
};

#endif /* HB_DRAW_HH */
