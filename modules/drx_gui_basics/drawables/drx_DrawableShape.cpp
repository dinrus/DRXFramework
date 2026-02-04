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

DrawableShape::DrawableShape()
    : strokeType (0.0f),
      mainFill (Colors::black),
      strokeFill (Colors::black)
{
}

DrawableShape::DrawableShape (const DrawableShape& other)
    : Drawable (other),
      strokeType (other.strokeType),
      dashLengths (other.dashLengths),
      mainFill (other.mainFill),
      strokeFill (other.strokeFill)
{
}

DrawableShape::~DrawableShape()
{
}

//==============================================================================
z0 DrawableShape::setFill (const FillType& newFill)
{
    if (mainFill != newFill)
    {
        mainFill = newFill;
        repaint();
    }
}

z0 DrawableShape::setStrokeFill (const FillType& newFill)
{
    if (strokeFill != newFill)
    {
        strokeFill = newFill;
        repaint();
    }
}

z0 DrawableShape::setStrokeType (const PathStrokeType& newStrokeType)
{
    if (strokeType != newStrokeType)
    {
        strokeType = newStrokeType;
        strokeChanged();
    }
}

z0 DrawableShape::setDashLengths (const Array<f32>& newDashLengths)
{
    if (dashLengths != newDashLengths)
    {
        dashLengths = newDashLengths;
        strokeChanged();
    }
}

z0 DrawableShape::setStrokeThickness (const f32 newThickness)
{
    setStrokeType (PathStrokeType (newThickness, strokeType.getJointStyle(), strokeType.getEndStyle()));
}

b8 DrawableShape::isStrokeVisible() const noexcept
{
    return strokeType.getStrokeThickness() > 0.0f && ! strokeFill.isInvisible();
}

//==============================================================================
z0 DrawableShape::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);
    applyDrawableClipPath (g);

    g.setFillType (mainFill);
    g.fillPath (path);

    if (isStrokeVisible())
    {
        g.setFillType (strokeFill);
        g.fillPath (strokePath);
    }
}

z0 DrawableShape::pathChanged()
{
    strokeChanged();
}

z0 DrawableShape::strokeChanged()
{
    strokePath.clear();
    const f32 extraAccuracy = 4.0f;

    if (dashLengths.isEmpty())
        strokeType.createStrokedPath (strokePath, path, AffineTransform(), extraAccuracy);
    else
        strokeType.createDashedStroke (strokePath, path, dashLengths.getRawDataPointer(),
                                       dashLengths.size(), AffineTransform(), extraAccuracy);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

Rectangle<f32> DrawableShape::getDrawableBounds() const
{
    if (isStrokeVisible())
        return strokePath.getBounds();

    return path.getBounds();
}

b8 DrawableShape::hitTest (i32 x, i32 y)
{
    b8 allowsClicksOnThisComponent, allowsClicksOnChildComponents;
    getInterceptsMouseClicks (allowsClicksOnThisComponent, allowsClicksOnChildComponents);

    if (! allowsClicksOnThisComponent)
        return false;

    auto globalX = (f32) (x - originRelativeToComponent.x);
    auto globalY = (f32) (y - originRelativeToComponent.y);

    return path.contains (globalX, globalY)
            || (isStrokeVisible() && strokePath.contains (globalX, globalY));
}

//==============================================================================
static b8 replaceColorInFill (FillType& fill, Color original, Color replacement)
{
    if (fill.colour == original && fill.isColor())
    {
        fill = FillType (replacement);
        return true;
    }

    return false;
}

b8 DrawableShape::replaceColor (Color original, Color replacement)
{
    b8 changed1 = replaceColorInFill (mainFill,   original, replacement);
    b8 changed2 = replaceColorInFill (strokeFill, original, replacement);
    return changed1 || changed2;
}

Path DrawableShape::getOutlineAsPath() const
{
    auto outline = isStrokeVisible() ? strokePath : path;
    outline.applyTransform (getTransform());
    return outline;
}

} // namespace drx
