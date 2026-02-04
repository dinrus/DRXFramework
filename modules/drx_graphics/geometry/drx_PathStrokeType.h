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
    Describes a type of stroke used to render a solid outline along a path.

    A PathStrokeType object can be used directly to create the shape of an outline
    around a path, and is used by Graphics::strokePath to specify the type of
    stroke to draw.

    @see Path, Graphics::strokePath

    @tags{Graphics}
*/
class DRX_API  PathStrokeType
{
public:
    //==============================================================================
    /** The type of shape to use for the corners between two adjacent line segments. */
    enum JointStyle
    {
        mitered,    /**< Indicates that corners should be drawn with sharp joints.
                         Note that for angles that curve back on themselves, drawing a
                         mitre could require extending the point too far away from the
                         path, so a mitre limit is imposed and any corners that exceed it
                         are drawn as bevelled instead. */
        curved,     /**< Indicates that corners should be drawn as rounded-off. */
        beveled     /**< Indicates that corners should be drawn with a line flattening their
                         outside edge. */
    };

    /** The type shape to use for the ends of lines. */
    enum EndCapStyle
    {
        butt,       /**< Ends of lines are flat and don't extend beyond the end point. */
        square,     /**< Ends of lines are flat, but stick out beyond the end point for half
                         the thickness of the stroke. */
        rounded     /**< Ends of lines are rounded-off with a circular shape. */
    };

    //==============================================================================
    /** Creates a stroke type with a given line-width, and default joint/end styles. */
    explicit PathStrokeType (f32 strokeThickness) noexcept;

    /** Creates a stroke type.

        @param strokeThickness      the width of the line to use
        @param jointStyle           the type of joints to use for corners
        @param endStyle             the type of end-caps to use for the ends of open paths.
    */
    PathStrokeType (f32 strokeThickness,
                    JointStyle jointStyle,
                    EndCapStyle endStyle = butt) noexcept;

    /** Creates a copy of another stroke type. */
    PathStrokeType (const PathStrokeType&) noexcept;

    /** Copies another stroke onto this one. */
    PathStrokeType& operator= (const PathStrokeType&) noexcept;

    /** Destructor. */
    ~PathStrokeType() noexcept;

    //==============================================================================
    /** Applies this stroke type to a path and returns the resultant stroke as another Path.

        @param destPath         the resultant stroked outline shape will be copied into this path.
                                Note that it's ok for the source and destination Paths to be
                                the same object, so you can easily turn a path into a stroked version
                                of itself.
        @param sourcePath       the path to use as the source
        @param transform        an optional transform to apply to the points from the source path
                                as they are being used
        @param extraAccuracy    if this is greater than 1.0, it will subdivide the path to
                                a higher resolution, which improves the quality if you'll later want
                                to enlarge the stroked path. So for example, if you're planning on drawing
                                the stroke at 3x the size that you're creating it, you should set this to 3.

        @see createDashedStroke
    */
    z0 createStrokedPath (Path& destPath,
                            const Path& sourcePath,
                            const AffineTransform& transform = AffineTransform(),
                            f32 extraAccuracy = 1.0f) const;


    //==============================================================================
    /** Applies this stroke type to a path, creating a dashed line.

        This is similar to createStrokedPath, but uses the array passed in to
        break the stroke up into a series of dashes.

        @param destPath         the resultant stroked outline shape will be copied into this path.
                                Note that it's ok for the source and destination Paths to be
                                the same object, so you can easily turn a path into a stroked version
                                of itself.
        @param sourcePath       the path to use as the source
        @param dashLengths      An array of alternating on/off lengths. E.g. { 2, 3, 4, 5 } will create
                                a line of length 2, then skip a length of 3, then add a line of length 4,
                                skip 5, and keep repeating this pattern.
        @param numDashLengths   The number of lengths in the dashLengths array. This should really be
                                an even number, otherwise the pattern will get out of step as it
                                repeats.
        @param transform        an optional transform to apply to the points from the source path
                                as they are being used
        @param extraAccuracy    if this is greater than 1.0, it will subdivide the path to
                                a higher resolution, which improves the quality if you'll later want
                                to enlarge the stroked path. So for example, if you're planning on drawing
                                the stroke at 3x the size that you're creating it, you should set this to 3.
    */
    z0 createDashedStroke (Path& destPath,
                             const Path& sourcePath,
                             const f32* dashLengths,
                             i32 numDashLengths,
                             const AffineTransform& transform = AffineTransform(),
                             f32 extraAccuracy = 1.0f) const;

    //==============================================================================
    /** Applies this stroke type to a path and returns the resultant stroke as another Path.

        @param destPath             the resultant stroked outline shape will be copied into this path.
                                    Note that it's ok for the source and destination Paths to be
                                    the same object, so you can easily turn a path into a stroked version
                                    of itself.
        @param sourcePath           the path to use as the source
        @param arrowheadStartWidth  the width of the arrowhead at the start of the path
        @param arrowheadStartLength the length of the arrowhead at the start of the path
        @param arrowheadEndWidth    the width of the arrowhead at the end of the path
        @param arrowheadEndLength   the length of the arrowhead at the end of the path
        @param transform            an optional transform to apply to the points from the source path
                                    as they are being used
        @param extraAccuracy        if this is greater than 1.0, it will subdivide the path to
                                    a higher resolution, which improves the quality if you'll later want
                                    to enlarge the stroked path. So for example, if you're planning on drawing
                                    the stroke at 3x the size that you're creating it, you should set this to 3.
        @see createDashedStroke
    */
    z0 createStrokeWithArrowheads (Path& destPath,
                                     const Path& sourcePath,
                                     f32 arrowheadStartWidth, f32 arrowheadStartLength,
                                     f32 arrowheadEndWidth, f32 arrowheadEndLength,
                                     const AffineTransform& transform = AffineTransform(),
                                     f32 extraAccuracy = 1.0f) const;

    //==============================================================================
    /** Returns the stroke thickness. */
    f32 getStrokeThickness() const noexcept                   { return thickness; }

    /** Sets the stroke thickness. */
    z0 setStrokeThickness (f32 newThickness) noexcept       { thickness = newThickness; }

    /** Returns the joint style. */
    JointStyle getJointStyle() const noexcept                   { return jointStyle; }

    /** Sets the joint style. */
    z0 setJointStyle (JointStyle newStyle) noexcept           { jointStyle = newStyle; }

    /** Returns the end-cap style. */
    EndCapStyle getEndStyle() const noexcept                    { return endStyle; }

    /** Sets the end-cap style. */
    z0 setEndStyle (EndCapStyle newStyle) noexcept            { endStyle = newStyle; }

    //==============================================================================
    /** Compares the stroke thickness, joint and end styles of two stroke types. */
    b8 operator== (const PathStrokeType&) const noexcept;

    /** Compares the stroke thickness, joint and end styles of two stroke types. */
    b8 operator!= (const PathStrokeType&) const noexcept;

private:
    //==============================================================================
    f32 thickness;
    JointStyle jointStyle;
    EndCapStyle endStyle;

    DRX_LEAK_DETECTOR (PathStrokeType)
};

} // namespace drx
