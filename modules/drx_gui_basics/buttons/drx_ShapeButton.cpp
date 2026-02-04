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

ShapeButton::ShapeButton (const Txt& t, Color n, Color o, Color d)
  : Button (t),
    normalColor   (n), overColor   (o), downColor   (d),
    normalColorOn (n), overColorOn (o), downColorOn (d),
    useOnColors (false),
    maintainShapeProportions (false),
    outlineWidth (0.0f)
{
}

ShapeButton::~ShapeButton() {}

z0 ShapeButton::setColors (Color newNormalColor, Color newOverColor, Color newDownColor)
{
    normalColor = newNormalColor;
    overColor   = newOverColor;
    downColor   = newDownColor;
}

z0 ShapeButton::setOnColors (Color newNormalColorOn, Color newOverColorOn, Color newDownColorOn)
{
    normalColorOn = newNormalColorOn;
    overColorOn   = newOverColorOn;
    downColorOn   = newDownColorOn;
}

z0 ShapeButton::shouldUseOnColors (b8 shouldUse)
{
    useOnColors = shouldUse;
}

z0 ShapeButton::setOutline (Color newOutlineColor, const f32 newOutlineWidth)
{
    outlineColor = newOutlineColor;
    outlineWidth = newOutlineWidth;
}

z0 ShapeButton::setBorderSize (BorderSize<i32> newBorder)
{
    border = newBorder;
}

z0 ShapeButton::setShape (const Path& newShape,
                            const b8 resizeNowToFitThisShape,
                            const b8 maintainShapeProportions_,
                            const b8 hasShadow)
{
    shape = newShape;
    maintainShapeProportions = maintainShapeProportions_;

    shadow.setShadowProperties (DropShadow (Colors::black.withAlpha (0.5f), 3, Point<i32>()));
    setComponentEffect (hasShadow ? &shadow : nullptr);

    if (resizeNowToFitThisShape)
    {
        auto newBounds = shape.getBounds();

        if (hasShadow)
            newBounds = newBounds.expanded (4.0f);

        shape.applyTransform (AffineTransform::translation (-newBounds.getX(),
                                                            -newBounds.getY()));

        setSize (1 + (i32) (newBounds.getWidth()  + outlineWidth) + border.getLeftAndRight(),
                 1 + (i32) (newBounds.getHeight() + outlineWidth) + border.getTopAndBottom());
    }

    repaint();
}

z0 ShapeButton::paintButton (Graphics& g, b8 shouldDrawButtonAsHighlighted, b8 shouldDrawButtonAsDown)
{
    if (! isEnabled())
    {
        shouldDrawButtonAsHighlighted = false;
        shouldDrawButtonAsDown = false;
    }

    auto r = border.subtractedFrom (getLocalBounds())
                   .toFloat()
                   .reduced (outlineWidth * 0.5f);

    if (getComponentEffect() != nullptr)
        r = r.reduced (2.0f);

    if (shouldDrawButtonAsDown)
    {
        const f32 sizeReductionWhenPressed = 0.04f;

        r = r.reduced (sizeReductionWhenPressed * r.getWidth(),
                       sizeReductionWhenPressed * r.getHeight());
    }

    auto trans = shape.getTransformToScaleToFit (r, maintainShapeProportions);

    if      (shouldDrawButtonAsDown)        g.setColor (getToggleState() && useOnColors ? downColorOn   : downColor);
    else if (shouldDrawButtonAsHighlighted) g.setColor (getToggleState() && useOnColors ? overColorOn   : overColor);
    else                                    g.setColor (getToggleState() && useOnColors ? normalColorOn : normalColor);

    g.fillPath (shape, trans);

    if (outlineWidth > 0.0f)
    {
        g.setColor (outlineColor);
        g.strokePath (shape, PathStrokeType (outlineWidth), trans);
    }
}

} // namespace drx
