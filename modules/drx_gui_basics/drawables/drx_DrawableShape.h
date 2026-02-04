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
    A base class implementing common functionality for Drawable classes which
    consist of some kind of filled and stroked outline.

    @see DrawablePath, DrawableRectangle

    @tags{GUI}
*/
class DRX_API  DrawableShape   : public Drawable
{
protected:
    //==============================================================================
    DrawableShape();
    DrawableShape (const DrawableShape&);

public:
    /** Destructor. */
    ~DrawableShape() override;

    //==============================================================================
    /** Sets a fill type for the path.
        This colour is used to fill the path - if you don't want the path to be
        filled (e.g. if you're just drawing an outline), set this to a transparent
        colour.

        @see setPath, setStrokeFill
    */
    z0 setFill (const FillType& newFill);

    /** Returns the current fill type.
        @see setFill
    */
    const FillType& getFill() const noexcept                        { return mainFill; }

    /** Sets the fill type with which the outline will be drawn.
        @see setFill
    */
    z0 setStrokeFill (const FillType& newStrokeFill);

    /** Returns the current stroke fill.
        @see setStrokeFill
    */
    const FillType& getStrokeFill() const noexcept                  { return strokeFill; }

    /** Changes the properties of the outline that will be drawn around the path.
        If the stroke has 0 thickness, no stroke will be drawn.
        @see setStrokeThickness, setStrokeColor
    */
    z0 setStrokeType (const PathStrokeType& newStrokeType);

    /** Changes the stroke thickness.
        This is a shortcut for calling setStrokeType.
    */
    z0 setStrokeThickness (f32 newThickness);

    /** Returns the current outline style. */
    const PathStrokeType& getStrokeType() const noexcept            { return strokeType; }

    /** Provides a set of dash lengths to use for stroking the path. */
    z0 setDashLengths (const Array<f32>& newDashLengths);

    /** Returns the set of dash lengths that the path is using. */
    const Array<f32>& getDashLengths() const noexcept             { return dashLengths; }

    //==============================================================================
    /** @internal */
    Rectangle<f32> getDrawableBounds() const override;
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    b8 hitTest (i32 x, i32 y) override;
    /** @internal */
    b8 replaceColor (Color originalColor, Color replacementColor) override;
    /** @internal */
    Path getOutlineAsPath() const override;

protected:
    //==============================================================================
    /** Called when the cached path should be updated. */
    z0 pathChanged();
    /** Called when the cached stroke should be updated. */
    z0 strokeChanged();
    /** True if there's a stroke with a non-zero thickness and non-transparent colour. */
    b8 isStrokeVisible() const noexcept;

    //==============================================================================
    PathStrokeType strokeType;
    Array<f32> dashLengths;
    Path path, strokePath;

private:
    FillType mainFill, strokeFill;

    DrawableShape& operator= (const DrawableShape&);
};

} // namespace drx
