/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

AffineTransform::AffineTransform (f32 m00, f32 m01, f32 m02,
                                  f32 m10, f32 m11, f32 m12) noexcept
 :  mat00 (m00), mat01 (m01), mat02 (m02),
    mat10 (m10), mat11 (m11), mat12 (m12)
{
}

b8 AffineTransform::operator== (const AffineTransform& other) const noexcept
{
    const auto tie = [] (const AffineTransform& a)
    {
        return std::tie (a.mat00, a.mat01, a.mat02, a.mat10, a.mat11, a.mat12);
    };

    return tie (*this) == tie (other);
}

b8 AffineTransform::operator!= (const AffineTransform& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
b8 AffineTransform::isIdentity() const noexcept
{
    return operator== (AffineTransform());
}

const AffineTransform AffineTransform::identity (1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

//==============================================================================
AffineTransform AffineTransform::followedBy (const AffineTransform& other) const noexcept
{
    return { other.mat00 * mat00 + other.mat01 * mat10,
             other.mat00 * mat01 + other.mat01 * mat11,
             other.mat00 * mat02 + other.mat01 * mat12 + other.mat02,
             other.mat10 * mat00 + other.mat11 * mat10,
             other.mat10 * mat01 + other.mat11 * mat11,
             other.mat10 * mat02 + other.mat11 * mat12 + other.mat12 };
}

AffineTransform AffineTransform::translated (f32 dx, f32 dy) const noexcept
{
    return { mat00, mat01, mat02 + dx,
             mat10, mat11, mat12 + dy };
}

AffineTransform AffineTransform::translation (f32 dx, f32 dy) noexcept
{
    return { 1.0f, 0.0f, dx,
             0.0f, 1.0f, dy };
}

AffineTransform AffineTransform::withAbsoluteTranslation (f32 tx, f32 ty) const noexcept
{
    return { mat00, mat01, tx,
             mat10, mat11, ty };
}

AffineTransform AffineTransform::rotated (f32 rad) const noexcept
{
    auto cosRad = std::cos (rad);
    auto sinRad = std::sin (rad);

    return { cosRad * mat00 - sinRad * mat10,
             cosRad * mat01 - sinRad * mat11,
             cosRad * mat02 - sinRad * mat12,
             sinRad * mat00 + cosRad * mat10,
             sinRad * mat01 + cosRad * mat11,
             sinRad * mat02 + cosRad * mat12 };
}

AffineTransform AffineTransform::rotation (f32 rad) noexcept
{
    auto cosRad = std::cos (rad);
    auto sinRad = std::sin (rad);

    return { cosRad, -sinRad, 0,
             sinRad,  cosRad, 0 };
}

AffineTransform AffineTransform::rotation (f32 rad, f32 pivotX, f32 pivotY) noexcept
{
    auto cosRad = std::cos (rad);
    auto sinRad = std::sin (rad);

    return { cosRad, -sinRad, -cosRad * pivotX +  sinRad * pivotY + pivotX,
             sinRad,  cosRad, -sinRad * pivotX + -cosRad * pivotY + pivotY };
}

AffineTransform AffineTransform::rotated (f32 angle, f32 pivotX, f32 pivotY) const noexcept
{
    return followedBy (rotation (angle, pivotX, pivotY));
}

AffineTransform AffineTransform::scaled (f32 factorX, f32 factorY) const noexcept
{
    return { factorX * mat00, factorX * mat01, factorX * mat02,
             factorY * mat10, factorY * mat11, factorY * mat12 };
}

AffineTransform AffineTransform::scaled (f32 factor) const noexcept
{
    return { factor * mat00, factor * mat01, factor * mat02,
             factor * mat10, factor * mat11, factor * mat12 };
}

AffineTransform AffineTransform::scale (f32 factorX, f32 factorY) noexcept
{
    return { factorX, 0, 0, 0, factorY, 0 };
}

AffineTransform AffineTransform::scale (f32 factor) noexcept
{
    return { factor, 0, 0, 0, factor, 0 };
}

AffineTransform AffineTransform::scaled (f32 factorX, f32 factorY,
                                         f32 pivotX, f32 pivotY) const noexcept
{
    return { factorX * mat00, factorX * mat01, factorX * mat02 + pivotX * (1.0f - factorX),
             factorY * mat10, factorY * mat11, factorY * mat12 + pivotY * (1.0f - factorY) };
}

AffineTransform AffineTransform::scale (f32 factorX, f32 factorY,
                                        f32 pivotX, f32 pivotY) noexcept
{
    return { factorX, 0, pivotX * (1.0f - factorX),
             0, factorY, pivotY * (1.0f - factorY) };
}

AffineTransform AffineTransform::shear (f32 shearX, f32 shearY) noexcept
{
    return { 1.0f,   shearX, 0,
             shearY, 1.0f,   0 };
}

AffineTransform AffineTransform::sheared (f32 shearX, f32 shearY) const noexcept
{
    return { mat00 + shearX * mat10,
             mat01 + shearX * mat11,
             mat02 + shearX * mat12,
             mat10 + shearY * mat00,
             mat11 + shearY * mat01,
             mat12 + shearY * mat02 };
}

AffineTransform AffineTransform::verticalFlip (f32 height) noexcept
{
    return { 1.0f,  0.0f, 0.0f,
             0.0f, -1.0f, height };
}

AffineTransform AffineTransform::inverted() const noexcept
{
    f64 determinant = getDeterminant();

    if (! approximatelyEqual (determinant, 0.0))
    {
        determinant = 1.0 / determinant;

        auto dst00 = (f32) ( mat11 * determinant);
        auto dst10 = (f32) (-mat10 * determinant);
        auto dst01 = (f32) (-mat01 * determinant);
        auto dst11 = (f32) ( mat00 * determinant);

        return { dst00, dst01, -mat02 * dst00 - mat12 * dst01,
                 dst10, dst11, -mat02 * dst10 - mat12 * dst11 };
    }

    // singularity..
    return *this;
}

b8 AffineTransform::isSingularity() const noexcept
{
    return exactlyEqual (mat00 * mat11 - mat10 * mat01, 0.0f);
}

AffineTransform AffineTransform::fromTargetPoints (f32 x00, f32 y00,
                                                   f32 x10, f32 y10,
                                                   f32 x01, f32 y01) noexcept
{
    return { x10 - x00, x01 - x00, x00,
             y10 - y00, y01 - y00, y00 };
}

AffineTransform AffineTransform::fromTargetPoints (f32 sx1, f32 sy1, f32 tx1, f32 ty1,
                                                   f32 sx2, f32 sy2, f32 tx2, f32 ty2,
                                                   f32 sx3, f32 sy3, f32 tx3, f32 ty3) noexcept
{
    return fromTargetPoints (sx1, sy1, sx2, sy2, sx3, sy3)
            .inverted()
            .followedBy (fromTargetPoints (tx1, ty1, tx2, ty2, tx3, ty3));
}

b8 AffineTransform::isOnlyTranslation() const noexcept
{
    return exactlyEqual (mat01, 0.0f)
        && exactlyEqual (mat10, 0.0f)
        && exactlyEqual (mat00, 1.0f)
        && exactlyEqual (mat11, 1.0f);
}

b8 AffineTransform::isOnlyTranslationOrScale() const noexcept
{
    return exactlyEqual (mat01, 0.0f) && exactlyEqual (mat10, 0.0f);
}

f32 AffineTransform::getDeterminant() const noexcept
{
    return (mat00 * mat11) - (mat01 * mat10);
}

f32 AffineTransform::getScaleFactor() const noexcept
{
    return (std::abs (mat00) + std::abs (mat11)) / 2.0f;
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class AffineTransformTests final : public UnitTest
{
public:
    AffineTransformTests()
        : UnitTest ("AffineTransform", UnitTestCategories::maths)
    {}

    z0 runTest() override
    {
        beginTest ("Determinant");
        {
            constexpr f32 scale1 = 1.5f, scale2 = 1.3f;

            auto transform = AffineTransform::scale (scale1)
                                             .followedBy (AffineTransform::rotation (degreesToRadians (72.0f)))
                                             .followedBy (AffineTransform::translation (100.0f, 20.0f))
                                             .followedBy (AffineTransform::scale (scale2));

            expect (approximatelyEqual (std::sqrt (std::abs (transform.getDeterminant())), scale1 * scale2));
        }

        beginTest ("fromTargetPoints");
        {
            const Point a (0.0f, 0.0f);
            const Point b (1.0f, 0.0f);
            const Point c (0.0f, 1.0f);
            const Point translation (1.0f, 1.0f);
            const auto transform = AffineTransform::fromTargetPoints (a, a + translation,
                                                                      b, b + translation,
                                                                      c, c + translation);
            expect (exactlyEqual (transform.mat00, 1.0f));
            expect (exactlyEqual (transform.mat01, 0.0f));
            expect (exactlyEqual (transform.mat02, translation.x));

            expect (exactlyEqual (transform.mat10, 0.0f));
            expect (exactlyEqual (transform.mat11, 1.0f));
            expect (exactlyEqual (transform.mat12, translation.y));
        }
    }
};

static AffineTransformTests timeTests;

#endif

} // namespace drx
