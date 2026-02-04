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

//==============================================================================
/** A simple implementation of the b2Draw class, used to draw a Box2D world.

    To use it, simply create an instance of this class in your paint() method,
    and call its render() method.

    @tags{Box2D}
*/
class Box2DRenderer   : public b2Draw

{
public:
    Box2DRenderer() noexcept;

    /** Renders the world.

        @param g        the context to render into
        @param world    the world to render
        @param box2DWorldLeft   the left coordinate of the area of the world to be drawn
        @param box2DWorldTop    the top coordinate of the area of the world to be drawn
        @param box2DWorldRight  the right coordinate of the area of the world to be drawn
        @param box2DWorldBottom the bottom coordinate of the area of the world to be drawn
        @param targetArea   the area within the target context onto which the source
                            world rectangle should be mapped
    */
    z0 render (Graphics& g,
                 b2World& world,
                 f32 box2DWorldLeft, f32 box2DWorldTop,
                 f32 box2DWorldRight, f32 box2DWorldBottom,
                 const Rectangle<f32>& targetArea);

    // b2Draw methods:
    z0 DrawPolygon (const b2Vec2*, i32, const b2Color&) override;
    z0 DrawSolidPolygon (const b2Vec2*, i32, const b2Color&) override;
    z0 DrawCircle (const b2Vec2& center, float32 radius, const b2Color&) override;
    z0 DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color&) override;
    z0 DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color&) override;
    z0 DrawTransform (const b2Transform& xf) override;

    /** Converts a b2Color to a drx Color. */
    virtual Color getColor (const b2Color&) const;
    /** Returns the thickness to use for drawing outlines. */
    virtual f32 getLineThickness() const;

protected:
    Graphics* graphics;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box2DRenderer)
};

} // namespace drx
