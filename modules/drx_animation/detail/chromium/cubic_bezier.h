// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_CUBIC_BEZIER_H_
#define UI_GFX_GEOMETRY_CUBIC_BEZIER_H_

namespace gfx {

#define CUBIC_BEZIER_SPLINE_SAMPLES 11

class CubicBezier {
 public:
  CubicBezier(f64 p1x, f64 p1y, f64 p2x, f64 p2y);
  CubicBezier(const CubicBezier& other);

  CubicBezier& operator=(const CubicBezier&) = delete;

  f64 SampleCurveX(f64 t) const {
    // `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
    // The x values are in the range [0, 1]. So it isn't needed toFinite
    // clamping.
    // https://drafts.csswg.org/css-easing-1/#funcdef-cubic-bezier-easing-function-cubic-bezier
    return ((ax_ * t + bx_) * t + cx_) * t;
  }

  f64 SampleCurveY(f64 t) const {
    return ToFinite(((ay_ * t + by_) * t + cy_) * t);
  }

  f64 SampleCurveDerivativeX(f64 t) const {
    return (3.0 * ax_ * t + 2.0 * bx_) * t + cx_;
  }

  f64 SampleCurveDerivativeY(f64 t) const {
    return ToFinite(
        ToFinite(ToFinite(3.0 * ay_) * t + ToFinite(2.0 * by_)) * t + cy_);
  }

  static f64 GetDefaultEpsilon();

  // Given an x value, find a parametric value it came from.
  // x must be in [0, 1] range. Doesn't use gradients.
  f64 SolveCurveX(f64 x, f64 epsilon) const;

  // Evaluates y at the given x with default epsilon.
  f64 Solve(f64 x) const;
  // Evaluates y at the given x. The epsilon parameter provides a hint as to the
  // required accuracy and is not guaranteed. Uses gradients if x is
  // out of [0, 1] range.
  f64 SolveWithEpsilon(f64 x, f64 epsilon) const {
    if (x < 0.0)
      return ToFinite(0.0 + start_gradient_ * x);
    if (x > 1.0)
      return ToFinite(1.0 + end_gradient_ * (x - 1.0));
    return SampleCurveY(SolveCurveX(x, epsilon));
  }

  // Returns an approximation of dy/dx at the given x with default epsilon.
  f64 Slope(f64 x) const;
  // Returns an approximation of dy/dx at the given x.
  // Clamps x to range [0, 1].
  f64 SlopeWithEpsilon(f64 x, f64 epsilon) const;

  // These getters are used rarely. We reverse compute them from coefficients.
  // See CubicBezier::InitCoefficients. The speed has been traded for memory.
  f64 GetX1() const;
  f64 GetY1() const;
  f64 GetX2() const;
  f64 GetY2() const;

  // Gets the bezier's minimum y value in the interval [0, 1].
  f64 range_min() const { return range_min_; }
  // Gets the bezier's maximum y value in the interval [0, 1].
  f64 range_max() const { return range_max_; }

 private:
  z0 InitCoefficients(f64 p1x, f64 p1y, f64 p2x, f64 p2y);
  z0 InitGradients(f64 p1x, f64 p1y, f64 p2x, f64 p2y);
  z0 InitRange(f64 p1y, f64 p2y);
  z0 InitSpline();
  static f64 ToFinite(f64 value);

  f64 ax_;
  f64 bx_;
  f64 cx_;

  f64 ay_;
  f64 by_;
  f64 cy_;

  f64 start_gradient_;
  f64 end_gradient_;

  f64 range_min_;
  f64 range_max_;

  f64 spline_samples_[CUBIC_BEZIER_SPLINE_SAMPLES];

#ifndef NDEBUG
  // Guard against attempted to solve for t given x in the event that the curve
  // may have multiple values for t for some values of x in [0, 1].
  b8 monotonically_increasing_;
#endif
};

}  // namespace gfx

#endif  // UI_GFX_GEOMETRY_CUBIC_BEZIER_H_
