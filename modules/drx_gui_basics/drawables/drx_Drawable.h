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

#ifndef DOXYGEN
namespace detail
{
class BoundsChangeListener final : private ComponentListener
{
public:
    BoundsChangeListener (Component& c, std::function<z0()> cb)
        : callback (std::move (cb)),
          componentListenerGuard { [comp = &c, this] { comp->removeComponentListener (this); } }
    {
        jassert (callback != nullptr);

        c.addComponentListener (this);
    }

private:
    z0 componentMovedOrResized (Component&, b8, b8) override
    {
        callback();
    }

    std::function<z0()> callback;
    ErasedScopeGuard componentListenerGuard;
};
} // namespace detail
#endif

//==============================================================================
/**
    The base class for objects which can draw themselves, e.g. polygons, images, etc.

    @see DrawableComposite, DrawableImage, DrawablePath, DrawableText

    @tags{GUI}
*/
class DRX_API  Drawable  : public Component
{
protected:
    //==============================================================================
    /** The base class can't be instantiated directly.

        @see DrawableComposite, DrawableImage, DrawablePath, DrawableText
    */
    Drawable();

public:
    /** Destructor. */
    ~Drawable() override;

    //==============================================================================
    /** Creates a deep copy of this Drawable object.

        Use this to create a new copy of this and any sub-objects in the tree.
    */
    virtual std::unique_ptr<Drawable> createCopy() const = 0;

    /** Creates a path that describes the outline of this drawable. */
    virtual Path getOutlineAsPath() const = 0;

    //==============================================================================
    /** Renders this Drawable object.

        Note that the preferred way to render a drawable in future is by using it
        as a component and adding it to a parent, so you might want to consider that
        before using this method.

        @see drawWithin
    */
    z0 draw (Graphics& g, f32 opacity,
               const AffineTransform& transform = AffineTransform()) const;

    /** Renders the Drawable at a given offset within the Graphics context.

        The coordinates passed-in are used to translate the object relative to its own
        origin before drawing it - this is basically a quick way of saying:

        @code
        draw (g, AffineTransform::translation (x, y)).
        @endcode

        Note that the preferred way to render a drawable in future is by using it
        as a component and adding it to a parent, so you might want to consider that
        before using this method.
    */
    z0 drawAt (Graphics& g, f32 x, f32 y, f32 opacity) const;

    /** Renders the Drawable within a rectangle, scaling it to fit neatly inside without
        changing its aspect-ratio.

        The object can placed arbitrarily within the rectangle based on a Justification type,
        and can either be made as big as possible, or just reduced to fit.

        Note that the preferred way to render a drawable in future is by using it
        as a component and adding it to a parent, so you might want to consider that
        before using this method.

        @param g                        the graphics context to render onto
        @param destArea                 the target rectangle to fit the drawable into
        @param placement                defines the alignment and rescaling to use to fit
                                        this object within the target rectangle.
        @param opacity                  the opacity to use, in the range 0 to 1.0
    */
    z0 drawWithin (Graphics& g,
                     Rectangle<f32> destArea,
                     RectanglePlacement placement,
                     f32 opacity) const;


    //==============================================================================
    /** Resets any transformations on this drawable, and positions its origin within
        its parent component.
    */
    z0 setOriginWithOriginalSize (Point<f32> originWithinParent);

    /** Sets a transform for this drawable that will position it within the specified
        area of its parent component.
    */
    z0 setTransformToFit (const Rectangle<f32>& areaInParent, RectanglePlacement placement);

    /** Returns the DrawableComposite that contains this object, if there is one. */
    DrawableComposite* getParent() const;

    /** Sets a the clipping region of this drawable using another drawable.
        The drawable passed in will be deleted when no longer needed.
    */
    z0 setClipPath (std::unique_ptr<Drawable> drawableClipPath);

    //==============================================================================
    /** Tries to turn some kind of image file into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static std::unique_ptr<Drawable> createFromImageData (ukk data, size_t numBytes);

    /** Tries to turn a stream containing some kind of image data into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static std::unique_ptr<Drawable> createFromImageDataStream (InputStream& dataSource);

    /** Tries to turn a file containing some kind of image data into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static std::unique_ptr<Drawable> createFromImageFile (const File& file);

    /** Attempts to parse an SVG (Scalable Vector Graphics) document, and to turn this
        into a Drawable tree.

        If something goes wrong while parsing, it may return nullptr.

        SVG is a pretty large and complex spec, and this doesn't aim to be a full
        implementation, but it can return the basic vector objects.
    */
    static std::unique_ptr<Drawable> createFromSVG (const XmlElement& svgDocument);

    /** Attempts to parse an SVG (Scalable Vector Graphics) document from a file,
        and to turn this into a Drawable tree.

        If something goes wrong while parsing, it may return nullptr.

        SVG is a pretty large and complex spec, and this doesn't aim to be a full
        implementation, but it can return the basic vector objects.

        Any references to references to external image files will be relative to
        the parent directory of the file passed.
    */
    static std::unique_ptr<Drawable> createFromSVGFile (const File& svgFile);

    /** Parses an SVG path string and returns it. */
    static Path parseSVGPath (const Txt& svgPath);

    //==============================================================================
    /** Returns the area that this drawable covers.
        The result is expressed in this drawable's own coordinate space, and does not take
        into account any transforms that may be applied to the component.
    */
    virtual Rectangle<f32> getDrawableBounds() const = 0;

    /** Recursively replaces a colour that might be used for filling or stroking.
        return true if any instances of this colour were found.
    */
    virtual b8 replaceColor (Color originalColor, Color replacementColor);

    /** Sets a transformation that applies to the same coordinate system in which the rest of the
        draw calls are made. You almost certainly want to call this function when working with
        Drawables as opposed to Component::setTransform().

        The reason for this is that the origin of a Drawable is not the same as the point returned
        by Component::getPosition() but has an additional offset internal to the Drawable class.

        Using setDrawableTransform() will take this internal offset into account when applying the
        transform to the Component base.

        You can only use Drawable::setDrawableTransform() or Component::setTransform() for a given
        object. Using both will lead to unpredictable behaviour.
    */
    z0 setDrawableTransform (const AffineTransform& transform);

protected:
    //==============================================================================
    friend class DrawableComposite;
    friend class DrawableShape;

    /** @internal */
    z0 transformContextToCorrectOrigin (Graphics&);
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    z0 setBoundsToEnclose (Rectangle<f32>);
    /** @internal */
    z0 applyDrawableClipPath (Graphics&);

    Point<i32> originRelativeToComponent;
    std::unique_ptr<Drawable> drawableClipPath;
    AffineTransform drawableTransform;

    z0 nonConstDraw (Graphics&, f32 opacity, const AffineTransform&);

    Drawable (const Drawable&);
    Drawable& operator= (const Drawable&);
    DRX_LEAK_DETECTOR (Drawable)


private:
    z0 updateTransform();

    detail::BoundsChangeListener boundsChangeListener { *this, [this] { updateTransform(); } };
};

} // namespace drx
