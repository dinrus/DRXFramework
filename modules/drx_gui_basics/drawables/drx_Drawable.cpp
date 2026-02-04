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

Drawable::Drawable()
{
    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);
    setAccessible (false);
}

Drawable::Drawable (const Drawable& other)
    : Component (other.getName())
{
    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);
    setAccessible (false);

    setComponentID (other.getComponentID());
    setTransform (other.getTransform());

    if (auto* clipPath = other.drawableClipPath.get())
        setClipPath (clipPath->createCopy());
}

Drawable::~Drawable()
{
}

z0 Drawable::applyDrawableClipPath (Graphics& g)
{
    if (drawableClipPath != nullptr)
    {
        auto clipPath = drawableClipPath->getOutlineAsPath();

        if (! clipPath.isEmpty())
            g.getInternalContext().clipToPath (clipPath, {});
    }
}

//==============================================================================
z0 Drawable::draw (Graphics& g, f32 opacity, const AffineTransform& transform) const
{
    const_cast<Drawable*> (this)->nonConstDraw (g, opacity, transform);
}

z0 Drawable::nonConstDraw (Graphics& g, f32 opacity, const AffineTransform& transform)
{
    Graphics::ScopedSaveState ss (g);

    g.addTransform (AffineTransform::translation ((f32) -(originRelativeToComponent.x),
                                                  (f32) -(originRelativeToComponent.y))
                        .followedBy (getTransform())
                        .followedBy (transform));

    applyDrawableClipPath (g);

    if (! g.isClipEmpty())
    {
        if (opacity < 1.0f)
        {
            g.beginTransparencyLayer (opacity);
            paintEntireComponent (g, true);
            g.endTransparencyLayer();
        }
        else
        {
            paintEntireComponent (g, true);
        }
    }
}

z0 Drawable::drawAt (Graphics& g, f32 x, f32 y, f32 opacity) const
{
    draw (g, opacity, AffineTransform::translation (x, y));
}

z0 Drawable::drawWithin (Graphics& g, Rectangle<f32> destArea,
                           RectanglePlacement placement, f32 opacity) const
{
    draw (g, opacity, placement.getTransformToFit (getDrawableBounds(), destArea));
}

//==============================================================================
DrawableComposite* Drawable::getParent() const
{
    return dynamic_cast<DrawableComposite*> (getParentComponent());
}

z0 Drawable::setClipPath (std::unique_ptr<Drawable> clipPath)
{
    if (drawableClipPath != clipPath)
    {
        drawableClipPath = std::move (clipPath);
        repaint();
    }
}

z0 Drawable::transformContextToCorrectOrigin (Graphics& g)
{
    g.setOrigin (originRelativeToComponent);
}

z0 Drawable::parentHierarchyChanged()
{
    setBoundsToEnclose (getDrawableBounds());
}

z0 Drawable::setBoundsToEnclose (Rectangle<f32> area)
{
    Point<i32> parentOrigin;

    if (auto* parent = getParent())
        parentOrigin = parent->originRelativeToComponent;

    const auto smallestIntegerContainer = area.getSmallestIntegerContainer();
    auto newBounds = smallestIntegerContainer + parentOrigin;
    originRelativeToComponent = -smallestIntegerContainer.getPosition();
    setBounds (newBounds);
}

//==============================================================================
b8 Drawable::replaceColor (Color original, Color replacement)
{
    b8 changed = false;

    for (auto* c : getChildren())
        if (auto* d = dynamic_cast<Drawable*> (c))
            changed = d->replaceColor (original, replacement) || changed;

    return changed;
}

z0 Drawable::setDrawableTransform (const AffineTransform& transform)
{
    drawableTransform = transform;
    updateTransform();
}

z0 Drawable::updateTransform()
{
    if (drawableTransform.isIdentity())
        return;

    const auto transformationOrigin = originRelativeToComponent + getPosition();
    setTransform (AffineTransform::translation (transformationOrigin * (-1))
                      .followedBy (drawableTransform)
                      .followedBy (AffineTransform::translation (transformationOrigin)));
}

//==============================================================================
z0 Drawable::setOriginWithOriginalSize (Point<f32> originWithinParent)
{
    setTransform (AffineTransform::translation (originWithinParent.x, originWithinParent.y));
}

z0 Drawable::setTransformToFit (const Rectangle<f32>& area, RectanglePlacement placement)
{
    if (! area.isEmpty())
        setTransform (placement.getTransformToFit (getDrawableBounds(), area));
}

//==============================================================================
std::unique_ptr<Drawable> Drawable::createFromImageData (ukk data, const size_t numBytes)
{
    auto image = ImageFileFormat::loadFrom (data, numBytes);

    if (image.isValid())
        return std::make_unique<DrawableImage> (image);

    if (auto svg = parseXMLIfTagMatches (Txt::createStringFromData (data, (i32) numBytes), "svg"))
        return Drawable::createFromSVG (*svg);

    return {};
}

std::unique_ptr<Drawable> Drawable::createFromImageDataStream (InputStream& dataSource)
{
    MemoryOutputStream mo;
    mo << dataSource;

    return createFromImageData (mo.getData(), mo.getDataSize());
}

std::unique_ptr<Drawable> Drawable::createFromImageFile (const File& file)
{
    FileInputStream fin (file);

    if (fin.openedOk())
        return createFromImageDataStream (fin);

    return {};
}

} // namespace drx
