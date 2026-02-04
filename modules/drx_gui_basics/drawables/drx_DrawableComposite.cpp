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

DrawableComposite::DrawableComposite()
    : bounds ({ 0.0f, 0.0f, 100.0f, 100.0f })
{
    setContentArea ({ 0.0f, 0.0f, 100.0f, 100.0f });
}

DrawableComposite::DrawableComposite (const DrawableComposite& other)
    : Drawable (other),
      bounds (other.bounds),
      contentArea (other.contentArea)
{
    for (auto* c : other.getChildren())
        if (auto* d = dynamic_cast<const Drawable*> (c))
            addAndMakeVisible (d->createCopy().release());
}

DrawableComposite::~DrawableComposite()
{
    deleteAllChildren();
}

std::unique_ptr<Drawable> DrawableComposite::createCopy() const
{
    return std::make_unique<DrawableComposite> (*this);
}

//==============================================================================
Rectangle<f32> DrawableComposite::getDrawableBounds() const
{
    Rectangle<f32> r;

    for (auto* c : getChildren())
        if (auto* d = dynamic_cast<const Drawable*> (c))
            r = r.getUnion (d->isTransformed() ? d->getDrawableBounds().transformedBy (d->getTransform())
                                               : d->getDrawableBounds());

    return r;
}

z0 DrawableComposite::setContentArea (Rectangle<f32> newArea)
{
    contentArea = newArea;
}

z0 DrawableComposite::setBoundingBox (Rectangle<f32> newBounds)
{
    setBoundingBox (Parallelogram<f32> (newBounds));
}

z0 DrawableComposite::setBoundingBox (Parallelogram<f32> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;

        auto t = AffineTransform::fromTargetPoints (contentArea.getTopLeft(),     bounds.topLeft,
                                                    contentArea.getTopRight(),    bounds.topRight,
                                                    contentArea.getBottomLeft(),  bounds.bottomLeft);

        if (t.isSingularity())
            t = {};

        setTransform (t);
    }
}

z0 DrawableComposite::resetBoundingBoxToContentArea()
{
    setBoundingBox (contentArea);
}

z0 DrawableComposite::resetContentAreaAndBoundingBoxToFitChildren()
{
    setContentArea (getDrawableBounds());
    resetBoundingBoxToContentArea();
}

z0 DrawableComposite::parentHierarchyChanged()
{
    if (auto* parent = getParent())
        originRelativeToComponent = parent->originRelativeToComponent - getPosition();
}

z0 DrawableComposite::childBoundsChanged (Component*)
{
    updateBoundsToFitChildren();
}

z0 DrawableComposite::childrenChanged()
{
    updateBoundsToFitChildren();
}

z0 DrawableComposite::updateBoundsToFitChildren()
{
    if (! updateBoundsReentrant)
    {
        const ScopedValueSetter<b8> setter (updateBoundsReentrant, true, false);

        Rectangle<i32> childArea;

        for (auto* c : getChildren())
            childArea = childArea.getUnion (c->getBoundsInParent());

        auto delta = childArea.getPosition();
        childArea += getPosition();

        if (childArea != getBounds())
        {
            if (! delta.isOrigin())
            {
                originRelativeToComponent -= delta;

                for (auto* c : getChildren())
                    c->setBounds (c->getBounds() - delta);
            }

            setBounds (childArea);
        }
    }
}

//==============================================================================
Path DrawableComposite::getOutlineAsPath() const
{
    Path p;

    for (auto* c : getChildren())
        if (auto* d = dynamic_cast<Drawable*> (c))
            p.addPath (d->getOutlineAsPath());

    p.applyTransform (getTransform());
    return p;
}

} // namespace drx
