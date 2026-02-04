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

ArrowButton::ArrowButton (const Txt& name, f32 arrowDirectionInRadians, Color arrowColor)
   : Button (name), colour (arrowColor)
{
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (MathConstants<f32>::twoPi * arrowDirectionInRadians, 0.5f, 0.5f));
}

ArrowButton::~ArrowButton() {}

z0 ArrowButton::paintButton (Graphics& g, b8 /*shouldDrawButtonAsHighlighted*/, b8 shouldDrawButtonAsDown)
{
    Path p (path);

    const f32 offset = shouldDrawButtonAsDown ? 1.0f : 0.0f;
    p.applyTransform (path.getTransformToScaleToFit (offset, offset, (f32) getWidth() - 3.0f, (f32) getHeight() - 3.0f, false));

    DropShadow (Colors::black.withAlpha (0.3f), shouldDrawButtonAsDown ? 2 : 4, Point<i32>()).drawForPath (g, p);

    g.setColor (colour);
    g.fillPath (p);
}

} // namespace drx
