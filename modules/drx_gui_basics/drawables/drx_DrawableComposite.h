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
/**
    A drawable object which acts as a container for a set of other Drawables.

    Note that although this is a Component, it takes ownership of its child components
    and will delete them, so that you can use it as a self-contained graphic object.
    The intention is that you should not add your own components to it, only add other
    Drawable objects.

    @see Drawable

    @tags{GUI}
*/
class DRX_API  DrawableComposite  : public Drawable
{
public:
    //==============================================================================
    /** Creates a composite Drawable. */
    DrawableComposite();

    /** Creates a copy of a DrawableComposite. */
    DrawableComposite (const DrawableComposite&);

    /** Destructor. */
    ~DrawableComposite() override;

    //==============================================================================
    /** Sets the parallelogram that defines the target position of the content rectangle when the drawable is rendered.
        @see setContentArea
    */
    z0 setBoundingBox (Parallelogram<f32> newBoundingBox);

    /** Sets the rectangle that defines the target position of the content rectangle when the drawable is rendered.
        @see setContentArea
    */
    z0 setBoundingBox (Rectangle<f32> newBoundingBox);

    /** Returns the parallelogram that defines the target position of the content rectangle when the drawable is rendered.
        @see setBoundingBox
    */
    Parallelogram<f32> getBoundingBox() const noexcept            { return bounds; }

    /** Changes the bounding box transform to match the content area, so that any sub-items will
        be drawn at their untransformed positions.
    */
    z0 resetBoundingBoxToContentArea();

    /** Returns the main content rectangle.
        @see contentLeftMarkerName, contentRightMarkerName, contentTopMarkerName, contentBottomMarkerName
    */
    Rectangle<f32> getContentArea() const noexcept                { return contentArea; }

    /** Changes the main content area.
        @see setBoundingBox, contentLeftMarkerName, contentRightMarkerName, contentTopMarkerName, contentBottomMarkerName
    */
    z0 setContentArea (Rectangle<f32> newArea);

    /** Resets the content area and the bounding transform to fit around the area occupied
        by the child components.
    */
    z0 resetContentAreaAndBoundingBoxToFitChildren();

    //==============================================================================
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;
    /** @internal */
    Rectangle<f32> getDrawableBounds() const override;
    /** @internal */
    z0 childBoundsChanged (Component*) override;
    /** @internal */
    z0 childrenChanged() override;
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    Path getOutlineAsPath() const override;

private:
    //==============================================================================
    Parallelogram<f32> bounds;
    Rectangle<f32> contentArea;
    b8 updateBoundsReentrant = false;

    z0 updateBoundsToFitChildren();

    DrawableComposite& operator= (const DrawableComposite&);
    DRX_LEAK_DETECTOR (DrawableComposite)
};

} // namespace drx
