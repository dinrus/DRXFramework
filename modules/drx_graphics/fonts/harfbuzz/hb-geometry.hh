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
#ifndef HB_GEOMETRY_HH
#define HB_GEOMETRY_HH

#include "hb.hh"


struct hb_extents_t
{
  hb_extents_t () {}
  hb_extents_t (f32 xmin, f32 ymin, f32 xmax, f32 ymax) :
    xmin (xmin), ymin (ymin), xmax (xmax), ymax (ymax) {}

  b8 is_empty () const { return xmin >= xmax || ymin >= ymax; }
  b8 is_void () const { return xmin > xmax; }

  z0 union_ (const hb_extents_t &o)
  {
    xmin = hb_min (xmin, o.xmin);
    ymin = hb_min (ymin, o.ymin);
    xmax = hb_max (xmax, o.xmax);
    ymax = hb_max (ymax, o.ymax);
  }

  z0 intersect (const hb_extents_t &o)
  {
    xmin = hb_max (xmin, o.xmin);
    ymin = hb_max (ymin, o.ymin);
    xmax = hb_min (xmax, o.xmax);
    ymax = hb_min (ymax, o.ymax);
  }

  z0
  add_point (f32 x, f32 y)
  {
    if (unlikely (is_void ()))
    {
      xmin = xmax = x;
      ymin = ymax = y;
    }
    else
    {
      xmin = hb_min (xmin, x);
      ymin = hb_min (ymin, y);
      xmax = hb_max (xmax, x);
      ymax = hb_max (ymax, y);
    }
  }

  f32 xmin = 0.f;
  f32 ymin = 0.f;
  f32 xmax = -1.f;
  f32 ymax = -1.f;
};

struct hb_transform_t
{
  hb_transform_t () {}
  hb_transform_t (f32 xx, f32 yx,
		  f32 xy, f32 yy,
		  f32 x0, f32 y0) :
    xx (xx), yx (yx), xy (xy), yy (yy), x0 (x0), y0 (y0) {}

  z0 multiply (const hb_transform_t &o)
  {
    /* Copied from cairo, with "o" being "a" there and "this" being "b" there. */
    hb_transform_t r;

    r.xx = o.xx * xx + o.yx * xy;
    r.yx = o.xx * yx + o.yx * yy;

    r.xy = o.xy * xx + o.yy * xy;
    r.yy = o.xy * yx + o.yy * yy;

    r.x0 = o.x0 * xx + o.y0 * xy + x0;
    r.y0 = o.x0 * yx + o.y0 * yy + y0;

    *this = r;
  }

  z0 transform_distance (f32 &dx, f32 &dy) const
  {
    f32 new_x = xx * dx + xy * dy;
    f32 new_y = yx * dx + yy * dy;
    dx = new_x;
    dy = new_y;
  }

  z0 transform_point (f32 &x, f32 &y) const
  {
    transform_distance (x, y);
    x += x0;
    y += y0;
  }

  z0 transform_extents (hb_extents_t &extents) const
  {
    f32 quad_x[4], quad_y[4];

    quad_x[0] = extents.xmin;
    quad_y[0] = extents.ymin;
    quad_x[1] = extents.xmin;
    quad_y[1] = extents.ymax;
    quad_x[2] = extents.xmax;
    quad_y[2] = extents.ymin;
    quad_x[3] = extents.xmax;
    quad_y[3] = extents.ymax;

    extents = hb_extents_t {};
    for (u32 i = 0; i < 4; i++)
    {
      transform_point (quad_x[i], quad_y[i]);
      extents.add_point (quad_x[i], quad_y[i]);
    }
  }

  z0 transform (const hb_transform_t &o) { multiply (o); }

  z0 translate (f32 x, f32 y)
  {
    if (x == 0.f && y == 0.f)
      return;

    x0 += xx * x + xy * y;
    y0 += yx * x + yy * y;
  }

  z0 scale (f32 scaleX, f32 scaleY)
  {
    if (scaleX == 1.f && scaleY == 1.f)
      return;

    xx *= scaleX;
    yx *= scaleX;
    xy *= scaleY;
    yy *= scaleY;
  }

  z0 rotate (f32 rotation)
  {
    if (rotation == 0.f)
      return;

    // https://github.com/fonttools/fonttools/blob/f66ee05f71c8b57b5f519ee975e95edcd1466e14/Lib/fontTools/misc/transform.py#L240
    rotation = rotation * HB_PI;
    f32 c;
    f32 s;
#ifdef HAVE_SINCOSF
    sincosf (rotation, &s, &c);
#else
    c = cosf (rotation);
    s = sinf (rotation);
#endif
    auto other = hb_transform_t{c, s, -s, c, 0.f, 0.f};
    transform (other);
  }

  z0 skew (f32 skewX, f32 skewY)
  {
    if (skewX == 0.f && skewY == 0.f)
      return;

    // https://github.com/fonttools/fonttools/blob/f66ee05f71c8b57b5f519ee975e95edcd1466e14/Lib/fontTools/misc/transform.py#L255
    skewX = skewX * HB_PI;
    skewY = skewY * HB_PI;
    auto other = hb_transform_t{1.f,
				skewY ? tanf (skewY) : 0.f,
				skewX ? tanf (skewX) : 0.f,
				1.f,
				0.f, 0.f};
    transform (other);
  }

  f32 xx = 1.f;
  f32 yx = 0.f;
  f32 xy = 0.f;
  f32 yy = 1.f;
  f32 x0 = 0.f;
  f32 y0 = 0.f;
};

struct hb_bounds_t
{
  enum status_t {
    UNBOUNDED,
    BOUNDED,
    EMPTY,
  };

  hb_bounds_t (status_t status) : status (status) {}
  hb_bounds_t (const hb_extents_t &extents) :
    status (extents.is_empty () ? EMPTY : BOUNDED), extents (extents) {}

  z0 union_ (const hb_bounds_t &o)
  {
    if (o.status == UNBOUNDED)
      status = UNBOUNDED;
    else if (o.status == BOUNDED)
    {
      if (status == EMPTY)
	*this = o;
      else if (status == BOUNDED)
        extents.union_ (o.extents);
    }
  }

  z0 intersect (const hb_bounds_t &o)
  {
    if (o.status == EMPTY)
      status = EMPTY;
    else if (o.status == BOUNDED)
    {
      if (status == UNBOUNDED)
	*this = o;
      else if (status == BOUNDED)
      {
        extents.intersect (o.extents);
	if (extents.is_empty ())
	  status = EMPTY;
      }
    }
  }

  status_t status;
  hb_extents_t extents;
};

struct hb_transform_decomposed_t
{
  f32 translateX = 0;
  f32 translateY = 0;
  f32 rotation = 0;  // in degrees, counter-clockwise
  f32 scaleX = 1;
  f32 scaleY = 1;
  f32 skewX = 0;  // in degrees, counter-clockwise
  f32 skewY = 0;  // in degrees, counter-clockwise
  f32 tCenterX = 0;
  f32 tCenterY = 0;

  operator b8 () const
  {
    return translateX || translateY ||
	   rotation ||
	   scaleX != 1 || scaleY != 1 ||
	   skewX || skewY ||
	   tCenterX || tCenterY;
  }

  hb_transform_t to_transform () const
  {
    hb_transform_t t;
    t.translate (translateX + tCenterX, translateY + tCenterY);
    t.rotate (rotation);
    t.scale (scaleX, scaleY);
    t.skew (-skewX, skewY);
    t.translate (-tCenterX, -tCenterY);
    return t;
  }
};


#endif /* HB_GEOMETRY_HH */
