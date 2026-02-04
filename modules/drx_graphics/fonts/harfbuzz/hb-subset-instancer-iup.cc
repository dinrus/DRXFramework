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
 */

#include "hb-subset-instancer-iup.hh"

/* This file is a straight port of the following:
 *
 * https://github.com/fonttools/fonttools/blob/main/Lib/fontTools/varLib/iup.py
 *
 * Where that file returns optimzied deltas vector, we return optimized
 * referenced point indices.
 */

constexpr static u32 MAX_LOOKBACK = 8;

static z0 _iup_contour_bound_forced_set (const hb_array_t<const contour_point_t> contour_points,
                                           const hb_array_t<i32k> x_deltas,
                                           const hb_array_t<i32k> y_deltas,
                                           hb_set_t& forced_set, /* OUT */
                                           f64 tolerance = 0.0)
{
  u32 len = contour_points.length;
  u32 next_i = 0;
  for (i32 i = len - 1; i >= 0; i--)
  {
    u32 last_i = (len + i -1) % len;
    for (u32 j = 0; j < 2; j++)
    {
      f64 cj, lcj, ncj;
      i32 dj, ldj, ndj;
      if (j == 0)
      {
        cj = static_cast<f64> (contour_points.arrayZ[i].x);
        dj = x_deltas.arrayZ[i];
        lcj = static_cast<f64> (contour_points.arrayZ[last_i].x);
        ldj = x_deltas.arrayZ[last_i];
        ncj = static_cast<f64> (contour_points.arrayZ[next_i].x);
        ndj = x_deltas.arrayZ[next_i];
      }
      else
      {
        cj = static_cast<f64> (contour_points.arrayZ[i].y);
        dj = y_deltas.arrayZ[i];
        lcj = static_cast<f64> (contour_points.arrayZ[last_i].y);
        ldj = y_deltas.arrayZ[last_i];
        ncj = static_cast<f64> (contour_points.arrayZ[next_i].y);
        ndj = y_deltas.arrayZ[next_i];
      }

      f64 c1, c2;
      i32 d1, d2;
      if (lcj <= ncj)
      {
        c1 = lcj;
        c2 = ncj;
        d1 = ldj;
        d2 = ndj;
      }
      else
      {
        c1 = ncj;
        c2 = lcj;
        d1 = ndj;
        d2 = ldj;
      }

      b8 force = false;
      if (c1 == c2)
      {
        if (abs (d1 - d2) > tolerance && abs (dj) > tolerance)
          force = true;
      }
      else if (c1 <= cj && cj <= c2)
      {
        if (!(hb_min (d1, d2) - tolerance <= dj &&
              dj <= hb_max (d1, d2) + tolerance))
          force = true;
      }
      else
      {
        if (d1 != d2)
        {
          if (cj < c1)
          {
            if (abs (dj) > tolerance &&
                abs (dj - d1) > tolerance &&
                ((dj - tolerance < d1) != (d1 < d2)))
                force = true;
          }
          else
          {
            if (abs (dj) > tolerance &&
                abs (dj - d2) > tolerance &&
                ((d2 < dj + tolerance) != (d1 < d2)))
              force = true;
          }
        }
      }

      if (force)
      {
        forced_set.add (i);
        break;
      }
    }
    next_i = i;
  }
}

template <typename T,
          hb_enable_if (hb_is_trivially_copyable (T))>
static b8 rotate_array (const hb_array_t<const T>& org_array,
                          i32 k,
                          hb_vector_t<T>& out)
{
  u32 n = org_array.length;
  if (!n) return true;
  if (unlikely (!out.resize (n, false)))
    return false;

  u32 item_size = hb_static_size (T);
  if (k < 0)
    k = n - (-k) % n;
  else
    k %= n;

  hb_memcpy ((uk ) out.arrayZ, (ukk ) (org_array.arrayZ + n - k), k * item_size);
  hb_memcpy ((uk ) (out.arrayZ + k), (ukk ) org_array.arrayZ, (n - k) * item_size);
  return true;
}

static b8 rotate_set (const hb_set_t& org_set,
                        i32 k,
                        u32 n,
                        hb_set_t& out)
{
  if (!n) return false;
  k %= n;
  if (k < 0)
    k = n + k;

  if (k == 0)
  {
    out.set (org_set);
  }
  else
  {
    for (auto v : org_set)
      out.add ((v + k) % n);
  }
  return !out.in_error ();
}

/* Given two reference coordinates (start and end of contour_points array),
 * output interpolated deltas for points in between */
static b8 _iup_segment (const hb_array_t<const contour_point_t> contour_points,
                          const hb_array_t<i32k> x_deltas,
                          const hb_array_t<i32k> y_deltas,
                          const contour_point_t& p1, const contour_point_t& p2,
                          i32 p1_dx, i32 p2_dx,
                          i32 p1_dy, i32 p2_dy,
                          hb_vector_t<f64>& interp_x_deltas, /* OUT */
                          hb_vector_t<f64>& interp_y_deltas /* OUT */)
{
  u32 n = contour_points.length;
  if (unlikely (!interp_x_deltas.resize (n, false) ||
                !interp_y_deltas.resize (n, false)))
    return false;

  for (u32 j = 0; j < 2; j++)
  {
    f64 x1, x2, d1, d2;
    f64 *out;
    if (j == 0)
    {
      x1 = static_cast<f64> (p1.x);
      x2 = static_cast<f64> (p2.x);
      d1 = p1_dx;
      d2 = p2_dx;
      out = interp_x_deltas.arrayZ;
    }
    else
    {
      x1 = static_cast<f64> (p1.y);
      x2 = static_cast<f64> (p2.y);
      d1 = p1_dy;
      d2 = p2_dy;
      out = interp_y_deltas.arrayZ;
    }

    if (x1 == x2)
    {
      if (d1 == d2)
      {
        for (u32 i = 0; i < n; i++)
          out[i] = d1;
      }
      else
      {
        for (u32 i = 0; i < n; i++)
          out[i] = 0.0;
      }
      continue;
    }

    if (x1 > x2)
    {
      hb_swap (x1, x2);
      hb_swap (d1, d2);
    }

    f64 scale = (d2 - d1) / (x2 - x1);
    for (u32 i = 0; i < n; i++)
    {
      f64 x = (j == 0 ? static_cast<f64> (contour_points.arrayZ[i].x) : static_cast<f64> (contour_points.arrayZ[i].y));
      f64 d;
      if (x <= x1)
        d = d1;
      else if (x >= x2)
        d = d2;
      else
        d = d1 + (x - x1) * scale;

      out[i] = d;
    }
  }
  return true;
}

static b8 _can_iup_in_between (const hb_array_t<const contour_point_t> contour_points,
                                 const hb_array_t<i32k> x_deltas,
                                 const hb_array_t<i32k> y_deltas,
                                 const contour_point_t& p1, const contour_point_t& p2,
                                 i32 p1_dx, i32 p2_dx,
                                 i32 p1_dy, i32 p2_dy,
                                 f64 tolerance)
{
  hb_vector_t<f64> interp_x_deltas, interp_y_deltas;
  if (!_iup_segment (contour_points, x_deltas, y_deltas,
                     p1, p2, p1_dx, p2_dx, p1_dy, p2_dy,
                     interp_x_deltas, interp_y_deltas))
    return false;

  u32 num = contour_points.length;

  for (u32 i = 0; i < num; i++)
  {
    f64 dx = static_cast<f64> (x_deltas.arrayZ[i]) - interp_x_deltas.arrayZ[i];
    f64 dy = static_cast<f64> (y_deltas.arrayZ[i]) - interp_y_deltas.arrayZ[i];
  
    if (sqrt (dx * dx + dy * dy) > tolerance)
      return false;
  }
  return true;
}

static b8 _iup_contour_optimize_dp (const contour_point_vector_t& contour_points,
                                      const hb_vector_t<i32>& x_deltas,
                                      const hb_vector_t<i32>& y_deltas,
                                      const hb_set_t& forced_set,
                                      f64 tolerance,
                                      u32 lookback,
                                      hb_vector_t<u32>& costs, /* OUT */
                                      hb_vector_t<i32>& chain /* OUT */)
{
  u32 n = contour_points.length;
  if (unlikely (!costs.resize (n, false) ||
                !chain.resize (n, false)))
    return false;

  lookback = hb_min (lookback, MAX_LOOKBACK);

  for (u32 i = 0; i < n; i++)
  {
    u32 best_cost = (i == 0 ? 1 : costs.arrayZ[i-1] + 1);
    
    costs.arrayZ[i] = best_cost;
    chain.arrayZ[i] = (i == 0 ? -1 : i - 1);

    if (i > 0 && forced_set.has (i - 1))
      continue;

    i32 lookback_index = hb_max ((i32) i - (i32) lookback + 1, -1);
    for (i32 j = i - 2; j >= lookback_index; j--)
    {
      u32 cost = j == -1 ? 1 : costs.arrayZ[j] + 1;
      /* num points between i and j */
      u32 num_points = i - j - 1;
      u32 p1 = (j == -1 ? n - 1 : j);
      if (cost < best_cost &&
          _can_iup_in_between (contour_points.as_array ().sub_array (j + 1, num_points),
                               x_deltas.as_array ().sub_array (j + 1, num_points),
                               y_deltas.as_array ().sub_array (j + 1, num_points),
                               contour_points.arrayZ[p1], contour_points.arrayZ[i],
                               x_deltas.arrayZ[p1], x_deltas.arrayZ[i],
                               y_deltas.arrayZ[p1], y_deltas.arrayZ[i],
                               tolerance))
      {
        best_cost = cost;
        costs.arrayZ[i] = best_cost;
        chain.arrayZ[i] = j;
      }

      if (j > 0 && forced_set.has (j))
        break;
    }
  }
  return true;
}

static b8 _iup_contour_optimize (const hb_array_t<const contour_point_t> contour_points,
                                   const hb_array_t<i32k> x_deltas,
                                   const hb_array_t<i32k> y_deltas,
                                   hb_array_t<b8> opt_indices, /* OUT */
                                   f64 tolerance = 0.0)
{
  u32 n = contour_points.length;
  if (opt_indices.length != n ||
      x_deltas.length != n ||
      y_deltas.length != n)
    return false;

  b8 all_within_tolerance = true;
  for (u32 i = 0; i < n; i++)
  {
    i32 dx = x_deltas.arrayZ[i];
    i32 dy = y_deltas.arrayZ[i];
    if (sqrt ((f64) dx * dx + (f64) dy * dy) > tolerance)
    {
      all_within_tolerance = false;
      break;
    }
  }

  /* If all are within tolerance distance, do nothing, opt_indices is
   * initilized to false */
  if (all_within_tolerance)
    return true;

  /* If there's exactly one point, return it */
  if (n == 1)
  {
    opt_indices.arrayZ[0] = true;
    return true;
  }

  /* If all deltas are exactly the same, return just one (the first one) */
  b8 all_deltas_are_equal = true;
  for (u32 i = 1; i < n; i++)
    if (x_deltas.arrayZ[i] != x_deltas.arrayZ[0] ||
        y_deltas.arrayZ[i] != y_deltas.arrayZ[0])
    {
      all_deltas_are_equal = false;
      break;
    }

  if (all_deltas_are_equal)
  {
    opt_indices.arrayZ[0] = true;
    return true;
  }

  /* else, solve the general problem using Dynamic Programming */
  hb_set_t forced_set;
  _iup_contour_bound_forced_set (contour_points, x_deltas, y_deltas, forced_set, tolerance);

  if (!forced_set.is_empty ())
  {
    i32 k = n - 1 - forced_set.get_max ();
    if (k < 0)
      return false;

    hb_vector_t<i32> rot_x_deltas, rot_y_deltas;
    contour_point_vector_t rot_points;
    hb_set_t rot_forced_set;
    if (!rotate_array (contour_points, k, rot_points) ||
        !rotate_array (x_deltas, k, rot_x_deltas) ||
        !rotate_array (y_deltas, k, rot_y_deltas) ||
        !rotate_set (forced_set, k, n, rot_forced_set))
      return false;

    hb_vector_t<u32> costs;
    hb_vector_t<i32> chain;

    if (!_iup_contour_optimize_dp (rot_points, rot_x_deltas, rot_y_deltas,
                                   rot_forced_set, tolerance, n,
                                   costs, chain))
      return false;

    hb_set_t solution;
    i32 index = n - 1;
    while (index != -1)
    {
      solution.add (index);
      index = chain.arrayZ[index];
    }

    if (solution.is_empty () ||
        forced_set.get_population () > solution.get_population ())
      return false;

    for (u32 i : solution)
      opt_indices.arrayZ[i] = true;

    hb_vector_t<b8> rot_indices;
    const hb_array_t<const b8> opt_indices_array (opt_indices.arrayZ, opt_indices.length);
    rotate_array (opt_indices_array, -k, rot_indices);

    for (u32 i = 0; i < n; i++)
      opt_indices.arrayZ[i] = rot_indices.arrayZ[i];
  }
  else
  {
    hb_vector_t<i32> repeat_x_deltas, repeat_y_deltas;
    contour_point_vector_t repeat_points;

    if (unlikely (!repeat_x_deltas.resize (n * 2, false) ||
                  !repeat_y_deltas.resize (n * 2, false) ||
                  !repeat_points.resize (n * 2, false)))
      return false;

    u32 contour_point_size = hb_static_size (contour_point_t);
    for (u32 i = 0; i < n; i++)
    {
      hb_memcpy ((uk ) repeat_x_deltas.arrayZ, (ukk ) x_deltas.arrayZ, n * sizeof (repeat_x_deltas[0]));
      hb_memcpy ((uk ) (repeat_x_deltas.arrayZ + n), (ukk ) x_deltas.arrayZ, n * sizeof (repeat_x_deltas[0]));

      hb_memcpy ((uk ) repeat_y_deltas.arrayZ, (ukk ) y_deltas.arrayZ, n * sizeof (repeat_x_deltas[0]));
      hb_memcpy ((uk ) (repeat_y_deltas.arrayZ + n), (ukk ) y_deltas.arrayZ, n * sizeof (repeat_x_deltas[0]));

      hb_memcpy ((uk ) repeat_points.arrayZ, (ukk ) contour_points.arrayZ, n * contour_point_size);
      hb_memcpy ((uk ) (repeat_points.arrayZ + n), (ukk ) contour_points.arrayZ, n * contour_point_size);
    }

    hb_vector_t<u32> costs;
    hb_vector_t<i32> chain;
    if (!_iup_contour_optimize_dp (repeat_points, repeat_x_deltas, repeat_y_deltas,
                                   forced_set, tolerance, n,
                                   costs, chain))
      return false;

    u32 best_cost = n + 1;
    i32 len = costs.length;
    hb_set_t best_sol;
    for (i32 start = n - 1; start < len; start++)
    {
      hb_set_t solution;
      i32 i = start;
      i32 lookback = start - (i32) n;
      while (i > lookback)
      {
        solution.add (i % n);
        i = chain.arrayZ[i];
      }
      if (i == lookback)
      {
        u32 cost_i = i < 0 ? 0 : costs.arrayZ[i];
        u32 cost = costs.arrayZ[start] - cost_i;
        if (cost <= best_cost)
        {
          best_sol.set (solution);
          best_cost = cost;
        }
      }
    }

    for (u32 i = 0; i < n; i++)
      if (best_sol.has (i))
        opt_indices.arrayZ[i] = true;
  }
  return true;
}

b8 iup_delta_optimize (const contour_point_vector_t& contour_points,
                         const hb_vector_t<i32>& x_deltas,
                         const hb_vector_t<i32>& y_deltas,
                         hb_vector_t<b8>& opt_indices, /* OUT */
                         f64 tolerance)
{
  if (!opt_indices.resize (contour_points.length))
      return false;

  hb_vector_t<u32> end_points;
  u32 count = contour_points.length;
  if (unlikely (!end_points.alloc (count)))
    return false;

  for (u32 i = 0; i < count - 4; i++)
    if (contour_points.arrayZ[i].is_end_point)
      end_points.push (i);

  /* phantom points */
  for (u32 i = count - 4; i < count; i++)
    end_points.push (i);

  if (end_points.in_error ()) return false;

  u32 start = 0;
  for (u32 end : end_points)
  {
    u32 len = end - start + 1;
    if (!_iup_contour_optimize (contour_points.as_array ().sub_array (start, len),
                                x_deltas.as_array ().sub_array (start, len),
                                y_deltas.as_array ().sub_array (start, len),
                                opt_indices.as_array ().sub_array (start, len),
                                tolerance))
      return false;
    start = end + 1;
  }
  return true;
}
