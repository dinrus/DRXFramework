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
    A button that contains a filled shape.

    @see Button, ImageButton, TextButton, ArrowButton

    @tags{GUI}
*/
class DRX_API  ShapeButton  : public Button
{
public:
    //==============================================================================
    /** Creates a ShapeButton.

        @param name             a name to give the component - see Component::setName()
        @param normalColor     the colour to fill the shape with when the mouse isn't over
        @param overColor       the colour to use when the mouse is over the shape
        @param downColor       the colour to use when the button is in the pressed-down state
    */
    ShapeButton (const Txt& name,
                 Color normalColor,
                 Color overColor,
                 Color downColor);

    /** Destructor. */
    ~ShapeButton() override;

    //==============================================================================
    /** Sets the shape to use.

        @param newShape                 the shape to use
        @param resizeNowToFitThisShape  if true, the button will be resized to fit the shape's bounds
        @param maintainShapeProportions if true, the shape's proportions will be kept fixed when
                                        the button is resized
        @param hasDropShadow            if true, the button will be given a drop-shadow effect
    */
    z0 setShape (const Path& newShape,
                   b8 resizeNowToFitThisShape,
                   b8 maintainShapeProportions,
                   b8 hasDropShadow);

    /** Set the colours to use for drawing the shape.

        @param normalColor     the colour to fill the shape with when the mouse isn't over
        @param overColor       the colour to use when the mouse is over the shape
        @param downColor       the colour to use when the button is in the pressed-down state
    */
    z0 setColors (Color normalColor,
                     Color overColor,
                     Color downColor);

    /** Sets the colours to use for drawing the shape when the button's toggle state is 'on'. To enable this behaviour, use the
        shouldUseOnColors() method.

        @param normalColorOn   the colour to fill the shape with when the mouse isn't over and the button's toggle state is 'on'
        @param overColorOn     the colour to use when the mouse is over the shape and the button's toggle state is 'on'
        @param downColorOn     the colour to use when the button is in the pressed-down state and the button's toggle state is 'on'
     */
    z0 setOnColors (Color normalColorOn,
                       Color overColorOn,
                       Color downColorOn);

    /** Set whether the button should use the 'on' set of colours when its toggle state is 'on'.
        By default these will be the same as the normal colours but the setOnColors method can be
        used to provide a different set of colours.
    */
    z0 shouldUseOnColors (b8 shouldUse);

    /** Sets up an outline to draw around the shape.

        @param outlineColor        the colour to use
        @param outlineStrokeWidth   the thickness of line to draw
    */
    z0 setOutline (Color outlineColor, f32 outlineStrokeWidth);

    /** This lets you specify a border to be left around the edge of the button when
        drawing the shape.
    */
    z0 setBorderSize (BorderSize<i32> border);

    /** @internal */
    z0 paintButton (Graphics&, b8, b8) override;

private:
    //==============================================================================
    Color normalColor,   overColor,   downColor,
           normalColorOn, overColorOn, downColorOn, outlineColor;
    b8 useOnColors;
    DropShadowEffect shadow;
    Path shape;
    BorderSize<i32> border;
    b8 maintainShapeProportions;
    f32 outlineWidth;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShapeButton)
};

} // namespace drx
