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
    A graphics context, used for drawing a component or image.

    When a Component needs painting, a Graphics context is passed to its
    Component::paint() method, and this you then call methods within this
    object to actually draw the component's content.

    A Graphics can also be created from an image, to allow drawing directly onto
    that image.

    @see Component::paint

    @tags{Graphics}
*/
class DRX_API  Graphics  final
{
public:
    //==============================================================================
    /** Creates a Graphics object to draw directly onto the given image.

        The graphics object that is created will be set up to draw onto the image,
        with the context's clipping area being the entire size of the image, and its
        origin being the image's origin. To draw into a subsection of an image, use the
        reduceClipRegion() and setOrigin() methods.

        Obviously you shouldn't delete the image before this context is deleted.
    */
    explicit Graphics (const Image& imageToDrawOnto);

    //==============================================================================
    /** Changes the current drawing colour.

        This sets the colour that will now be used for drawing operations - it also
        sets the opacity to that of the colour passed-in.

        If a brush is being used when this method is called, the brush will be deselected,
        and any subsequent drawing will be done with a solid colour brush instead.

        @see setOpacity
    */
    z0 setColor (Color newColor);

    /** Changes the opacity to use with the current colour.

        If a solid colour is being used for drawing, this changes its opacity
        to this new value (i.e. it doesn't multiply the colour's opacity by this amount).

        If a gradient is being used, this will have no effect on it.

        A value of 0.0 is completely transparent, 1.0 is completely opaque.
    */
    z0 setOpacity (f32 newOpacity);

    /** Sets the context to use a gradient for its fill pattern. */
    z0 setGradientFill (const ColorGradient& gradient);

    /** Sets the context to use a gradient for its fill pattern. */
    z0 setGradientFill (ColorGradient&& gradient);

    /** Sets the context to use a tiled image pattern for filling.
        Make sure that you don't delete this image while it's still being used by
        this context!
    */
    z0 setTiledImageFill (const Image& imageToUse,
                            i32 anchorX, i32 anchorY,
                            f32 opacity);

    /** Changes the current fill settings.
        @see setColor, setGradientFill, setTiledImageFill
    */
    z0 setFillType (const FillType& newFill);

    //==============================================================================
    /** Changes the font to use for subsequent text-drawing functions.
        @see drawSingleLineText, drawMultiLineText, drawText, drawFittedText
    */
    z0 setFont (const Font& newFont);

    /** Changes the size of the currently-selected font.
        This is a convenient shortcut that changes the context's current font to a
        different size. The typeface won't be changed.
        @see Font
    */
    z0 setFont (f32 newFontHeight);

    /** Returns the currently selected font. */
    Font getCurrentFont() const;

    /** Draws a one-line text string.

        This will use the current colour (or brush) to fill the text. The font is the last
        one specified by setFont().

        @param text          the string to draw
        @param startX        the position to draw the left-hand edge of the text
        @param baselineY     the position of the text's baseline
        @param justification the horizontal flags indicate which end of the text string is
                             anchored at the specified point.
        @see drawMultiLineText, drawText, drawFittedText, GlyphArrangement::addLineOfText
    */
    z0 drawSingleLineText (const Txt& text,
                             i32 startX, i32 baselineY,
                             Justification justification = Justification::left) const;

    /** Draws text across multiple lines.

        This will break the text onto a new line where there's a new-line or
        carriage-return character, or at a word-boundary when the text becomes wider
        than the size specified by the maximumLineWidth parameter. New-lines
        will be vertically separated by the specified leading.

        @see setFont, drawSingleLineText, drawFittedText, GlyphArrangement::addJustifiedText
    */
    z0 drawMultiLineText (const Txt& text,
                            i32 startX, i32 baselineY,
                            i32 maximumLineWidth,
                            Justification justification = Justification::left,
                            f32 leading = 0.0f) const;

    /** Draws a line of text within a specified rectangle.

        The text will be positioned within the rectangle based on the justification
        flags passed-in. If the string is too i64 to fit inside the rectangle, it will
        either be truncated or will have ellipsis added to its end (if the useEllipsesIfTooBig
        flag is true).

        @see drawSingleLineText, drawFittedText, drawMultiLineText, GlyphArrangement::addJustifiedText
    */
    z0 drawText (const Txt& text,
                   i32 x, i32 y, i32 width, i32 height,
                   Justification justificationType,
                   b8 useEllipsesIfTooBig = true) const;

    /** Draws a line of text within a specified rectangle.

        The text will be positioned within the rectangle based on the justification
        flags passed-in. If the string is too i64 to fit inside the rectangle, it will
        either be truncated or will have ellipsis added to its end (if the useEllipsesIfTooBig
        flag is true).

        @see drawSingleLineText, drawFittedText, drawMultiLineText, GlyphArrangement::addJustifiedText
    */
    z0 drawText (const Txt& text,
                   Rectangle<i32> area,
                   Justification justificationType,
                   b8 useEllipsesIfTooBig = true) const;

    /** Draws a line of text within a specified rectangle.

        The text will be positioned within the rectangle based on the justification
        flags passed-in. If the string is too i64 to fit inside the rectangle, it will
        either be truncated or will have ellipsis added to its end (if the useEllipsesIfTooBig
        flag is true).

        @see drawSingleLineText, drawFittedText, drawMultiLineText, GlyphArrangement::addJustifiedText
    */
    z0 drawText (const Txt& text,
                   Rectangle<f32> area,
                   Justification justificationType,
                   b8 useEllipsesIfTooBig = true) const;

    /** Tries to draw a text string inside a given space.

        This does its best to make the given text readable within the specified rectangle,
        so it's useful for labelling things.

        If the text is too big, it'll be squashed horizontally or broken over multiple lines
        if the maximumLinesToUse value allows this. If the text just won't fit into the space,
        it'll cram as much as possible in there, and put some ellipsis at the end to show that
        it's been truncated.

        A Justification parameter lets you specify how the text is laid out within the rectangle,
        both horizontally and vertically.

        The minimumHorizontalScale parameter specifies how much the text can be squashed horizontally
        to try to squeeze it into the space. If you don't want any horizontal scaling to occur, you
        can set this value to 1.0f. Pass 0 if you want it to use a default value.

        @see GlyphArrangement::addFittedText
    */
    z0 drawFittedText (const Txt& text,
                         i32 x, i32 y, i32 width, i32 height,
                         Justification justificationFlags,
                         i32 maximumNumberOfLines,
                         f32 minimumHorizontalScale = 0.0f) const;

    /** Tries to draw a text string inside a given space.

        This does its best to make the given text readable within the specified rectangle,
        so it's useful for labelling things.

        If the text is too big, it'll be squashed horizontally or broken over multiple lines
        if the maximumLinesToUse value allows this. If the text just won't fit into the space,
        it'll cram as much as possible in there, and put some ellipsis at the end to show that
        it's been truncated.

        A Justification parameter lets you specify how the text is laid out within the rectangle,
        both horizontally and vertically.

        The minimumHorizontalScale parameter specifies how much the text can be squashed horizontally
        to try to squeeze it into the space. If you don't want any horizontal scaling to occur, you
        can set this value to 1.0f. Pass 0 if you want it to use a default value.

        @see GlyphArrangement::addFittedText
    */
    z0 drawFittedText (const Txt& text,
                         Rectangle<i32> area,
                         Justification justificationFlags,
                         i32 maximumNumberOfLines,
                         f32 minimumHorizontalScale = 0.0f) const;

    //==============================================================================
    /** Fills the context's entire clip region with the current colour or brush.

        (See also the fillAll (Color) method which is a quick way of filling
        it with a given colour).
    */
    z0 fillAll() const;

    /** Fills the context's entire clip region with a given colour.

        This leaves the context's current colour and brush unchanged, it just
        uses the specified colour temporarily.
    */
    z0 fillAll (Color colourToUse) const;

    //==============================================================================
    /** Fills a rectangle with the current colour or brush.
        @see drawRect, fillRoundedRectangle
    */
    z0 fillRect (Rectangle<i32> rectangle) const;

    /** Fills a rectangle with the current colour or brush.
        @see drawRect, fillRoundedRectangle
    */
    z0 fillRect (Rectangle<f32> rectangle) const;

    /** Fills a rectangle with the current colour or brush.
        @see drawRect, fillRoundedRectangle
    */
    z0 fillRect (i32 x, i32 y, i32 width, i32 height) const;

    /** Fills a rectangle with the current colour or brush.
        @see drawRect, fillRoundedRectangle
    */
    z0 fillRect (f32 x, f32 y, f32 width, f32 height) const;

    /** Fills a set of rectangles using the current colour or brush.
        If you have a lot of rectangles to draw, it may be more efficient
        to create a RectangleList and use this method than to call fillRect()
        multiple times.
    */
    z0 fillRectList (const RectangleList<f32>& rectangles) const;

    /** Fills a set of rectangles using the current colour or brush.
        If you have a lot of rectangles to draw, it may be more efficient
        to create a RectangleList and use this method than to call fillRect()
        multiple times.
    */
    z0 fillRectList (const RectangleList<i32>& rectangles) const;

    /** Uses the current colour or brush to fill a rectangle with rounded corners.
        @see drawRoundedRectangle, Path::addRoundedRectangle
    */
    z0 fillRoundedRectangle (f32 x, f32 y, f32 width, f32 height,
                               f32 cornerSize) const;

    /** Uses the current colour or brush to fill a rectangle with rounded corners.
        @see drawRoundedRectangle, Path::addRoundedRectangle
    */
    z0 fillRoundedRectangle (Rectangle<f32> rectangle,
                               f32 cornerSize) const;

    /** Fills a rectangle with a checkerboard pattern, alternating between two colours. */
    z0 fillCheckerBoard (Rectangle<f32> area,
                           f32 checkWidth, f32 checkHeight,
                           Color colour1, Color colour2) const;

    /** Draws a rectangular outline, using the current colour or brush.
        The lines are drawn inside the given rectangle, and greater line thicknesses extend inwards.
        @see fillRect
    */
    z0 drawRect (i32 x, i32 y, i32 width, i32 height, i32 lineThickness = 1) const;

    /** Draws a rectangular outline, using the current colour or brush.
        The lines are drawn inside the given rectangle, and greater line thicknesses extend inwards.
        @see fillRect
    */
    z0 drawRect (f32 x, f32 y, f32 width, f32 height, f32 lineThickness = 1.0f) const;

    /** Draws a rectangular outline, using the current colour or brush.
        The lines are drawn inside the given rectangle, and greater line thicknesses extend inwards.
        @see fillRect
    */
    z0 drawRect (Rectangle<i32> rectangle, i32 lineThickness = 1) const;

    /** Draws a rectangular outline, using the current colour or brush.
        The lines are drawn inside the given rectangle, and greater line thicknesses extend inwards.
        @see fillRect
    */
    z0 drawRect (Rectangle<f32> rectangle, f32 lineThickness = 1.0f) const;

    /** Uses the current colour or brush to draw the outline of a rectangle with rounded corners.
        @see fillRoundedRectangle, Path::addRoundedRectangle
    */
    z0 drawRoundedRectangle (f32 x, f32 y, f32 width, f32 height,
                               f32 cornerSize, f32 lineThickness) const;

    /** Uses the current colour or brush to draw the outline of a rectangle with rounded corners.
        @see fillRoundedRectangle, Path::addRoundedRectangle
    */
    z0 drawRoundedRectangle (Rectangle<f32> rectangle,
                               f32 cornerSize, f32 lineThickness) const;

    //==============================================================================
    /** Fills an ellipse with the current colour or brush.
        The ellipse is drawn to fit inside the given rectangle.
        @see drawEllipse, Path::addEllipse
    */
    z0 fillEllipse (f32 x, f32 y, f32 width, f32 height) const;

    /** Fills an ellipse with the current colour or brush.
        The ellipse is drawn to fit inside the given rectangle.
        @see drawEllipse, Path::addEllipse
    */
    z0 fillEllipse (Rectangle<f32> area) const;

    /** Draws an elliptical stroke using the current colour or brush.
        @see fillEllipse, Path::addEllipse
    */
    z0 drawEllipse (f32 x, f32 y, f32 width, f32 height,
                      f32 lineThickness) const;

    /** Draws an elliptical stroke using the current colour or brush.
        @see fillEllipse, Path::addEllipse
    */
    z0 drawEllipse (Rectangle<f32> area, f32 lineThickness) const;

    //==============================================================================
    /** Draws a line between two points.
        The line is 1 pixel wide and drawn with the current colour or brush.
        TIP: If you're trying to draw horizontal or vertical lines, don't use this -
        it's better to use fillRect() instead unless you really need an angled line.
    */
    z0 drawLine (f32 startX, f32 startY, f32 endX, f32 endY) const;

    /** Draws a line between two points with a given thickness.
        TIP: If you're trying to draw horizontal or vertical lines, don't use this -
        it's better to use fillRect() instead unless you really need an angled line.
        @see Path::addLineSegment
    */
    z0 drawLine (f32 startX, f32 startY, f32 endX, f32 endY, f32 lineThickness) const;

    /** Draws a line between two points.
        The line is 1 pixel wide and drawn with the current colour or brush.
        TIP: If you're trying to draw horizontal or vertical lines, don't use this -
        it's better to use fillRect() instead unless you really need an angled line.
    */
    z0 drawLine (Line<f32> line) const;

    /** Draws a line between two points with a given thickness.
        @see Path::addLineSegment
        TIP: If you're trying to draw horizontal or vertical lines, don't use this -
        it's better to use fillRect() instead unless you really need an angled line.
    */
    z0 drawLine (Line<f32> line, f32 lineThickness) const;

    /** Draws a dashed line using a custom set of dash-lengths.

        @param line             the line to draw
        @param dashLengths      a series of lengths to specify the on/off lengths - e.g.
                                { 4, 5, 6, 7 } will draw a line of 4 pixels, skip 5 pixels,
                                draw 6 pixels, skip 7 pixels, and then repeat.
        @param numDashLengths   the number of elements in the array (this must be an even number).
        @param lineThickness    the thickness of the line to draw
        @param dashIndexToStartFrom     the index in the dash-length array to use for the first segment
        @see PathStrokeType::createDashedStroke
    */
    z0 drawDashedLine (Line<f32> line,
                         const f32* dashLengths, i32 numDashLengths,
                         f32 lineThickness = 1.0f,
                         i32 dashIndexToStartFrom = 0) const;

    /** Draws a vertical line of pixels at a given x position.

        The x position is an integer, but the top and bottom of the line can be sub-pixel
        positions, and these will be anti-aliased if necessary.

        The bottom parameter must be greater than or equal to the top parameter.
    */
    z0 drawVerticalLine (i32 x, f32 top, f32 bottom) const;

    /** Draws a horizontal line of pixels at a given y position.

        The y position is an integer, but the left and right ends of the line can be sub-pixel
        positions, and these will be anti-aliased if necessary.

        The right parameter must be greater than or equal to the left parameter.
    */
    z0 drawHorizontalLine (i32 y, f32 left, f32 right) const;

    //==============================================================================
    /** Fills a path using the currently selected colour or brush. */
    z0 fillPath (const Path& path) const;

    /** Fills a path using the currently selected colour or brush, and adds a transform. */
    z0 fillPath (const Path& path, const AffineTransform& transform) const;

    /** Draws a path's outline using the currently selected colour or brush. */
    z0 strokePath (const Path& path,
                     const PathStrokeType& strokeType,
                     const AffineTransform& transform = {}) const;

    /** Draws a line with an arrowhead at its end.

        @param line             the line to draw
        @param lineThickness    the thickness of the line
        @param arrowheadWidth   the width of the arrow head (perpendicular to the line)
        @param arrowheadLength  the length of the arrow head (along the length of the line)
    */
    z0 drawArrow (Line<f32> line,
                    f32 lineThickness,
                    f32 arrowheadWidth,
                    f32 arrowheadLength) const;


    //==============================================================================
    /** Types of rendering quality that can be specified when drawing images.

        @see Graphics::setImageResamplingQuality
    */
    enum ResamplingQuality
    {
        lowResamplingQuality     = 0,    /**< Just uses a nearest-neighbour algorithm for resampling. */
        mediumResamplingQuality  = 1,    /**< Uses bilinear interpolation for upsampling and area-averaging for downsampling. */
        highResamplingQuality    = 2,    /**< Uses bicubic interpolation for upsampling and area-averaging for downsampling. */
    };

    /** Changes the quality that will be used when resampling images.
        By default a Graphics object will be set to mediumRenderingQuality.
        @see Graphics::drawImage, Graphics::drawImageTransformed, Graphics::drawImageWithin
    */
    z0 setImageResamplingQuality (ResamplingQuality newQuality);

    /** Draws an image.

        This will draw the whole of an image, positioning its top-left corner at the
        given coordinates, and keeping its size the same. This is the simplest image
        drawing method - the others give more control over the scaling and clipping
        of the images.

        Images are composited using the context's current opacity, so if you
        don't want it to be drawn semi-transparently, be sure to call setOpacity (1.0f)
        (or setColor() with an opaque colour) before drawing images.
    */
    z0 drawImageAt (const Image& imageToDraw, i32 topLeftX, i32 topLeftY,
                      b8 fillAlphaChannelWithCurrentBrush = false) const;

    /** Draws part of an image, rescaling it to fit in a given target region.

        The specified area of the source image is rescaled and drawn to fill the
        specified destination rectangle.

        Images are composited using the context's current opacity, so if you
        don't want it to be drawn semi-transparently, be sure to call setOpacity (1.0f)
        (or setColor() with an opaque colour) before drawing images.

        @param imageToDraw      the image to overlay
        @param destX            the left of the destination rectangle
        @param destY            the top of the destination rectangle
        @param destWidth        the width of the destination rectangle
        @param destHeight       the height of the destination rectangle
        @param sourceX          the left of the rectangle to copy from the source image
        @param sourceY          the top of the rectangle to copy from the source image
        @param sourceWidth      the width of the rectangle to copy from the source image
        @param sourceHeight     the height of the rectangle to copy from the source image
        @param fillAlphaChannelWithCurrentBrush     if true, then instead of drawing the source image's pixels,
                                                    the source image's alpha channel is used as a mask with
                                                    which to fill the destination using the current colour
                                                    or brush. (If the source is has no alpha channel, then
                                                    it will just fill the target with a solid rectangle)
        @see setImageResamplingQuality, drawImageAt, drawImageWithin, fillAlphaMap
    */
    z0 drawImage (const Image& imageToDraw,
                    i32 destX, i32 destY, i32 destWidth, i32 destHeight,
                    i32 sourceX, i32 sourceY, i32 sourceWidth, i32 sourceHeight,
                    b8 fillAlphaChannelWithCurrentBrush = false) const;

    /** Draws an image, having applied an affine transform to it.

        This lets you throw the image around in some wacky ways, rotate it, shear,
        scale it, etc.

        Images are composited using the context's current opacity, so if you
        don't want it to be drawn semi-transparently, be sure to call setOpacity (1.0f)
        (or setColor() with an opaque colour) before drawing images.

        If fillAlphaChannelWithCurrentBrush is set to true, then the image's RGB channels
        are ignored and it is filled with the current brush, masked by its alpha channel.

        If you want to render only a subsection of an image, use Image::getClippedImage() to
        create the section that you need.

        @see setImageResamplingQuality, drawImage
    */
    z0 drawImageTransformed (const Image& imageToDraw,
                               const AffineTransform& transform,
                               b8 fillAlphaChannelWithCurrentBrush = false) const;

    /** Draws an image to fit within a designated rectangle.

        @param imageToDraw              the source image to draw
        @param targetArea               the target rectangle to fit it into
        @param placementWithinTarget    this specifies how the image should be positioned
                                        within the target rectangle - see the RectanglePlacement
                                        class for more details about this.
        @param fillAlphaChannelWithCurrentBrush     if true, then instead of drawing the image, just its
                                                    alpha channel will be used as a mask with which to
                                                    draw with the current brush or colour. This is
                                                    similar to fillAlphaMap(), and see also drawImage()
        @see drawImage, drawImageTransformed, drawImageAt, RectanglePlacement
    */
    z0 drawImage (const Image& imageToDraw, Rectangle<f32> targetArea,
                    RectanglePlacement placementWithinTarget = RectanglePlacement::stretchToFit,
                    b8 fillAlphaChannelWithCurrentBrush = false) const;

    /** Draws an image to fit within a designated rectangle.

        If the image is too big or too small for the space, it will be rescaled
        to fit as nicely as it can do without affecting its aspect ratio. It will
        then be placed within the target rectangle according to the justification flags
        specified.

        @param imageToDraw              the source image to draw
        @param destX                    top-left of the target rectangle to fit it into
        @param destY                    top-left of the target rectangle to fit it into
        @param destWidth                size of the target rectangle to fit the image into
        @param destHeight               size of the target rectangle to fit the image into
        @param placementWithinTarget    this specifies how the image should be positioned
                                        within the target rectangle - see the RectanglePlacement
                                        class for more details about this.
        @param fillAlphaChannelWithCurrentBrush     if true, then instead of drawing the image, just its
                                                    alpha channel will be used as a mask with which to
                                                    draw with the current brush or colour. This is
                                                    similar to fillAlphaMap(), and see also drawImage()
        @see setImageResamplingQuality, drawImage, drawImageTransformed, drawImageAt, RectanglePlacement
    */
    z0 drawImageWithin (const Image& imageToDraw,
                          i32 destX, i32 destY, i32 destWidth, i32 destHeight,
                          RectanglePlacement placementWithinTarget,
                          b8 fillAlphaChannelWithCurrentBrush = false) const;

    //==============================================================================
    /** Returns the position of the bounding box for the current clipping region.
        @see clipRegionIntersects
    */
    Rectangle<i32> getClipBounds() const;

    /** Checks whether a rectangle overlaps the context's clipping region.

        If this returns false, no part of the given area can be drawn onto, so this
        method can be used to optimise a component's paint() method, by letting it
        avoid drawing complex objects that aren't within the region being repainted.
    */
    b8 clipRegionIntersects (Rectangle<i32> area) const;

    /** Intersects the current clipping region with another region.

        @returns true if the resulting clipping region is non-zero in size
        @see setOrigin, clipRegionIntersects
    */
    b8 reduceClipRegion (i32 x, i32 y, i32 width, i32 height);

    /** Intersects the current clipping region with another region.

        @returns true if the resulting clipping region is non-zero in size
        @see setOrigin, clipRegionIntersects
    */
    b8 reduceClipRegion (Rectangle<i32> area);

    /** Intersects the current clipping region with a rectangle list region.

        @returns true if the resulting clipping region is non-zero in size
        @see setOrigin, clipRegionIntersects
    */
    b8 reduceClipRegion (const RectangleList<i32>& clipRegion);

    /** Intersects the current clipping region with a path.

        @returns true if the resulting clipping region is non-zero in size
        @see reduceClipRegion
    */
    b8 reduceClipRegion (const Path& path, const AffineTransform& transform = AffineTransform());

    /** Intersects the current clipping region with an image's alpha-channel.

        The current clipping path is intersected with the area covered by this image's
        alpha-channel, after the image has been transformed by the specified matrix.

        @param image    the image whose alpha-channel should be used. If the image doesn't
                        have an alpha-channel, it is treated as entirely opaque.
        @param transform    a matrix to apply to the image
        @returns true if the resulting clipping region is non-zero in size
        @see reduceClipRegion
    */
    b8 reduceClipRegion (const Image& image, const AffineTransform& transform);

    /** Excludes a rectangle to stop it being drawn into. */
    z0 excludeClipRegion (Rectangle<i32> rectangleToExclude);

    /** Возвращает true, если no drawing can be done because the clip region is zero. */
    b8 isClipEmpty() const;

    //==============================================================================
    /** Saves the current graphics state on an internal stack.
        To restore the state, use restoreState().
        @see ScopedSaveState
    */
    z0 saveState();

    /** Restores a graphics state that was previously saved with saveState().
        @see ScopedSaveState
    */
    z0 restoreState();

    /** Uses RAII to save and restore the state of a graphics context.
        On construction, this calls Graphics::saveState(), and on destruction it calls
        Graphics::restoreState() on the Graphics object that you supply.
    */
    class ScopedSaveState
    {
    public:
        ScopedSaveState (Graphics&);
        ~ScopedSaveState();

    private:
        Graphics& context;
        DRX_DECLARE_NON_COPYABLE (ScopedSaveState)
    };

    //==============================================================================
    /** Begins rendering to an off-screen bitmap which will later be flattened onto the current
        context with the given opacity.

        The context uses an internal stack of temporary image layers to do this. When you've
        finished drawing to the layer, call endTransparencyLayer() to complete the operation and
        composite the finished layer. Every call to beginTransparencyLayer() MUST be matched
        by a corresponding call to endTransparencyLayer()!

        This call also saves the current state, and endTransparencyLayer() restores it.
    */
    z0 beginTransparencyLayer (f32 layerOpacity);

    /** Completes a drawing operation to a temporary semi-transparent buffer.
        See beginTransparencyLayer() for more details.
    */
    z0 endTransparencyLayer();

    /** Moves the position of the context's origin.

        This changes the position that the context considers to be (0, 0) to
        the specified position.

        So if you call setOrigin with (100, 100), then the position that was previously
        referred to as (100, 100) will subsequently be considered to be (0, 0).

        @see reduceClipRegion, addTransform
    */
    z0 setOrigin (Point<i32> newOrigin);

    /** Moves the position of the context's origin.

        This changes the position that the context considers to be (0, 0) to
        the specified position.

        So if you call setOrigin (100, 100), then the position that was previously
        referred to as (100, 100) will subsequently be considered to be (0, 0).

        @see reduceClipRegion, addTransform
    */
    z0 setOrigin (i32 newOriginX, i32 newOriginY);

    /** Adds a transformation which will be performed on all the graphics operations that
        the context subsequently performs.

        After calling this, all the coordinates that are passed into the context will be
        transformed by this matrix.

        @see setOrigin
    */
    z0 addTransform (const AffineTransform& transform);

    /** Resets the current colour, brush, and font to default settings. */
    z0 resetToDefaultState();

    /** Возвращает true, если this context is drawing to a vector-based device, such as a printer. */
    b8 isVectorDevice() const;

    //==============================================================================
    /** Create a graphics that draws with a given low-level renderer.
        This method is intended for use only by people who know what they're doing.
        Note that the LowLevelGraphicsContext will NOT be deleted by this object.
    */
    Graphics (LowLevelGraphicsContext&) noexcept;

    /** @internal */
    LowLevelGraphicsContext& getInternalContext() const noexcept    { return context; }

private:
    //==============================================================================
    std::unique_ptr<LowLevelGraphicsContext> contextHolder;
    LowLevelGraphicsContext& context;

    b8 saveStatePending = false;
    z0 saveStateIfPending();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graphics)
};

} // namespace drx
