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

Box2DRenderer::Box2DRenderer() noexcept   : graphics (nullptr)
{
    SetFlags (e_shapeBit);
}

z0 Box2DRenderer::render (Graphics& g, b2World& world,
                            f32 left, f32 top, f32 right, f32 bottom,
                            const Rectangle<f32>& target)
{
    graphics = &g;

    g.addTransform (AffineTransform::fromTargetPoints (left,  top,    target.getX(),     target.getY(),
                                                       right, top,    target.getRight(), target.getY(),
                                                       left,  bottom, target.getX(),     target.getBottom()));

    world.SetDebugDraw (this);
    world.DrawDebugData();
}

Color Box2DRenderer::getColor (const b2Color& c) const
{
    return Color::fromFloatRGBA (c.r, c.g, c.b, 1.0f);
}

f32 Box2DRenderer::getLineThickness() const
{
    return 0.1f;
}

static z0 createPath (Path& p, const b2Vec2* vertices, i32 vertexCount)
{
    p.startNewSubPath (vertices[0].x, vertices[0].y);

    for (i32 i = 1; i < vertexCount; ++i)
        p.lineTo (vertices[i].x, vertices[i].y);

    p.closeSubPath();
}

z0 Box2DRenderer::DrawPolygon (const b2Vec2* vertices, i32 vertexCount, const b2Color& color)
{
    graphics->setColor (getColor (color));

    Path p;
    createPath (p, vertices, vertexCount);
    graphics->strokePath (p, PathStrokeType (getLineThickness()));
}

z0 Box2DRenderer::DrawSolidPolygon (const b2Vec2* vertices, i32 vertexCount, const b2Color& color)
{
    graphics->setColor (getColor (color));

    Path p;
    createPath (p, vertices, vertexCount);
    graphics->fillPath (p);
}

z0 Box2DRenderer::DrawCircle (const b2Vec2& center, float32 radius, const b2Color& color)
{
    graphics->setColor (getColor (color));
    graphics->drawEllipse (center.x - radius, center.y - radius,
                           radius * 2.0f, radius * 2.0f,
                           getLineThickness());
}

z0 Box2DRenderer::DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& /*axis*/, const b2Color& colour)
{
    graphics->setColor (getColor (colour));
    graphics->fillEllipse (center.x - radius, center.y - radius,
                           radius * 2.0f, radius * 2.0f);
}

z0 Box2DRenderer::DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    graphics->setColor (getColor (color));
    graphics->drawLine (p1.x, p1.y, p2.x, p2.y, getLineThickness());
}

z0 Box2DRenderer::DrawTransform (const b2Transform&)
{
}

} // namespace drx
