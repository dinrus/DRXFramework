// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cubic_bezier.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace gfx {

namespace {

i32k kMaxNewtonIterations = 4;

}  // namespace

static const f64 kBezierEpsilon = 1e-7;

f64 CubicBezier::ToFinite(f64 value) {
  // TODO(crbug.com/1275541): We can clamp this in numeric operation helper
  // function like ClampedNumeric.
  if (std::isinf(value)) {
    if (value > 0)
      return std::numeric_limits<f64>::max();
    return std::numeric_limits<f64>::lowest();
  }
  return value;
}

CubicBezier::CubicBezier(f64 p1x, f64 p1y, f64 p2x, f64 p2y) {
  InitCoefficients(p1x, p1y, p2x, p2y);
  InitGradients(p1x, p1y, p2x, p2y);
  InitRange(p1y, p2y);
  InitSpline();
}

CubicBezier::CubicBezier(const CubicBezier& other) = default;

z0 CubicBezier::InitCoefficients(f64 p1x,
                                   f64 p1y,
                                   f64 p2x,
                                   f64 p2y) {
  // Calculate the polynomial coefficients, implicit first and last control
  // points are (0,0) and (1,1).
  cx_ = 3.0 * p1x;
  bx_ = 3.0 * (p2x - p1x) - cx_;
  ax_ = 1.0 - cx_ - bx_;

  cy_ = ToFinite(3.0 * p1y);
  by_ = ToFinite(3.0 * (p2y - p1y) - cy_);
  ay_ = ToFinite(1.0 - cy_ - by_);

#ifndef NDEBUG
  // Bezier curves with x-coordinates outside the range [0,1] for internal
  // control points may have multiple values for t for a given value of x.
  // In this case, calls to SolveCurveX may produce ambiguous results.
  monotonically_increasing_ = p1x >= 0 && p1x <= 1 && p2x >= 0 && p2x <= 1;
#endif
}

z0 CubicBezier::InitGradients(f64 p1x,
                                f64 p1y,
                                f64 p2x,
                                f64 p2y) {
  // End-point gradients are used to calculate timing function results
  // outside the range [0, 1].
  //
  // There are four possibilities for the gradient at each end:
  // (1) the closest control point is not horizontally coincident with regard to
  //     (0, 0) or (1, 1). In this case the line between the end point and
  //     the control point is tangent to the bezier at the end point.
  // (2) the closest control point is coincident with the end point. In
  //     this case the line between the end point and the far control
  //     point is tangent to the bezier at the end point.
  // (3) both internal control points are coincident with an endpoint. There
  //     are two special case that fall into this category:
  //     CubicBezier(0, 0, 0, 0) and CubicBezier(1, 1, 1, 1). Both are
  //     equivalent to linear.
  // (4) the closest control point is horizontally coincident with the end
  //     point, but vertically distinct. In this case the gradient at the
  //     end point is Infinite. However, this causes issues when
  //     interpolating. As a result, we break down to a simple case of
  //     0 gradient under these conditions.

  if (p1x > 0)
    start_gradient_ = p1y / p1x;
  else if (!p1y && p2x > 0)
    start_gradient_ = p2y / p2x;
  else if (!p1y && !p2y)
    start_gradient_ = 1;
  else
    start_gradient_ = 0;

  if (p2x < 1)
    end_gradient_ = (p2y - 1) / (p2x - 1);
  else if (p2y == 1 && p1x < 1)
    end_gradient_ = (p1y - 1) / (p1x - 1);
  else if (p2y == 1 && p1y == 1)
    end_gradient_ = 1;
  else
    end_gradient_ = 0;
}

// This works by taking taking the derivative of the cubic bezier, on the y
// axis. We can then solve for where the derivative is zero to find the min
// and max distance along the line. We the have to solve those in terms of time
// rather than distance on the x-axis
z0 CubicBezier::InitRange(f64 p1y, f64 p2y) {
  range_min_ = 0;
  range_max_ = 1;
  if (0 <= p1y && p1y < 1 && 0 <= p2y && p2y <= 1)
    return;

  const f64 epsilon = kBezierEpsilon;

  // Represent the function's derivative in the form at^2 + bt + c
  // as in sampleCurveDerivativeY.
  // (Technically this is (dy/dt)*(1/3), which is suitable for finding zeros
  // but does not actually give the slope of the curve.)
  const f64 a = 3.0 * ay_;
  const f64 b = 2.0 * by_;
  const f64 c = cy_;

  // Check if the derivative is constant.
  if (std::abs(a) < epsilon && std::abs(b) < epsilon)
    return;

  // Zeros of the function's derivative.
  f64 t1 = 0;
  f64 t2 = 0;

  if (std::abs(a) < epsilon) {
    // The function's derivative is linear.
    t1 = -c / b;
  } else {
    // The function's derivative is a quadratic. We find the zeros of this
    // quadratic using the quadratic formula.
    f64 discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
      return;
    f64 discriminant_sqrt = sqrt(discriminant);
    t1 = (-b + discriminant_sqrt) / (2 * a);
    t2 = (-b - discriminant_sqrt) / (2 * a);
  }

  f64 sol1 = 0;
  f64 sol2 = 0;

  // If the solution is in the range [0,1] then we include it, otherwise we
  // ignore it.

  // An interesting fact about these beziers is that they are only
  // actually evaluated in [0,1]. After that we take the tangent at that point
  // and linearly project it out.
  if (0 < t1 && t1 < 1)
    sol1 = SampleCurveY(t1);

  if (0 < t2 && t2 < 1)
    sol2 = SampleCurveY(t2);

  range_min_ = std::min({range_min_, sol1, sol2});
  range_max_ = std::max({range_max_, sol1, sol2});
}

z0 CubicBezier::InitSpline() {
  f64 delta_t = 1.0 / (CUBIC_BEZIER_SPLINE_SAMPLES - 1);
  for (i32 i = 0; i < CUBIC_BEZIER_SPLINE_SAMPLES; i++) {
    spline_samples_[i] = SampleCurveX(i * delta_t);
  }
}

f64 CubicBezier::GetDefaultEpsilon() {
  return kBezierEpsilon;
}

f64 CubicBezier::SolveCurveX(f64 x, f64 epsilon) const {
  jassert (x >= 0.0);
  jassert (x <= 1.0);

  f64 t0;
  f64 t1;
  f64 t2 = x;
  f64 x2;
  f64 d2;
  i32 i;

#ifndef NDEBUG
  jassert (monotonically_increasing_);
#endif

  // Linear interpolation of spline curve for initial guess.
  f64 delta_t = 1.0 / (CUBIC_BEZIER_SPLINE_SAMPLES - 1);
  for (i = 1; i < CUBIC_BEZIER_SPLINE_SAMPLES; i++) {
    if (x <= spline_samples_[i]) {
      t1 = delta_t * i;
      t0 = t1 - delta_t;
      t2 = t0 + (t1 - t0) * (x - spline_samples_[i - 1]) /
                    (spline_samples_[i] - spline_samples_[i - 1]);
      break;
    }
  }

  // Perform a few iterations of Newton's method -- normally very fast.
  // See https://en.wikipedia.org/wiki/Newton%27s_method.
  f64 newton_epsilon = std::min(kBezierEpsilon, epsilon);
  for (i = 0; i < kMaxNewtonIterations; i++) {
    x2 = SampleCurveX(t2) - x;
    if (fabs(x2) < newton_epsilon)
      return t2;
    d2 = SampleCurveDerivativeX(t2);
    if (fabs(d2) < kBezierEpsilon)
      break;
    t2 = t2 - x2 / d2;
  }
  if (fabs(x2) < epsilon)
    return t2;

  // Fall back to the bisection method for reliability.
  while (t0 < t1) {
    x2 = SampleCurveX(t2);
    if (fabs(x2 - x) < epsilon)
      return t2;
    if (x > x2)
      t0 = t2;
    else
      t1 = t2;
    t2 = (t1 + t0) * .5;
  }

  // Failure.
  return t2;
}

f64 CubicBezier::Solve(f64 x) const {
  return SolveWithEpsilon(x, kBezierEpsilon);
}

f64 CubicBezier::SlopeWithEpsilon(f64 x, f64 epsilon) const {
  x = std::clamp(x, 0.0, 1.0);
  f64 t = SolveCurveX(x, epsilon);
  f64 dx = SampleCurveDerivativeX(t);
  f64 dy = SampleCurveDerivativeY(t);
  // TODO(crbug.com/1275534): We should clamp NaN to a proper value.
  // Please see the issue for detail.
  if (!dx && !dy)
    return 0;
  return ToFinite(dy / dx);
}

f64 CubicBezier::Slope(f64 x) const {
  return SlopeWithEpsilon(x, kBezierEpsilon);
}

f64 CubicBezier::GetX1() const {
  return cx_ / 3.0;
}

f64 CubicBezier::GetY1() const {
  return cy_ / 3.0;
}

f64 CubicBezier::GetX2() const {
  return (bx_ + cx_) / 3.0 + GetX1();
}

f64 CubicBezier::GetY2() const {
  return (by_ + cy_) / 3.0 + GetY1();
}

}  // namespace gfx
