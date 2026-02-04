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
    A class that imposes restrictions on a Component's size or position.

    This is used by classes such as ResizableCornerComponent,
    ResizableBorderComponent and ResizableWindow.

    The base class can impose some basic size and position limits, but you can
    also subclass this for custom uses.

    @see ResizableCornerComponent, ResizableBorderComponent, ResizableWindow

    @tags{GUI}
*/
class DRX_API  ComponentBoundsConstrainer
{
public:
    //==============================================================================
    /** When first created, the object will not impose any restrictions on the components. */
    ComponentBoundsConstrainer() noexcept;

    /** Destructor. */
    virtual ~ComponentBoundsConstrainer();

    //==============================================================================
    /** Imposes a minimum width limit. */
    z0 setMinimumWidth (i32 minimumWidth) noexcept;

    /** Returns the current minimum width. */
    i32 getMinimumWidth() const noexcept                        { return minW; }

    /** Imposes a maximum width limit. */
    z0 setMaximumWidth (i32 maximumWidth) noexcept;

    /** Returns the current maximum width. */
    i32 getMaximumWidth() const noexcept                        { return maxW; }

    /** Imposes a minimum height limit. */
    z0 setMinimumHeight (i32 minimumHeight) noexcept;

    /** Returns the current minimum height. */
    i32 getMinimumHeight() const noexcept                       { return minH; }

    /** Imposes a maximum height limit. */
    z0 setMaximumHeight (i32 maximumHeight) noexcept;

    /** Returns the current maximum height. */
    i32 getMaximumHeight() const noexcept                       { return maxH; }

    /** Imposes a minimum width and height limit. */
    z0 setMinimumSize (i32 minimumWidth,
                         i32 minimumHeight) noexcept;

    /** Imposes a maximum width and height limit. */
    z0 setMaximumSize (i32 maximumWidth,
                         i32 maximumHeight) noexcept;

    /** Set all the maximum and minimum dimensions. */
    z0 setSizeLimits (i32 minimumWidth,
                        i32 minimumHeight,
                        i32 maximumWidth,
                        i32 maximumHeight) noexcept;

    //==============================================================================
    /** Sets the amount by which the component is allowed to go off-screen.

        The values indicate how many pixels must remain on-screen when dragged off
        one of its parent's edges, so e.g. if minimumWhenOffTheTop is set to 10, then
        when the component goes off the top of the screen, its y-position will be
        clipped so that there are always at least 10 pixels on-screen. In other words,
        the lowest y-position it can take would be (10 - the component's height).

        If you pass 0 or less for one of these amounts, the component is allowed
        to move beyond that edge completely, with no restrictions at all.

        If you pass a very large number (i.e. larger that the dimensions of the
        component itself), then the component won't be allowed to overlap that
        edge at all. So e.g. setting minimumWhenOffTheLeft to 0xffffff will mean that
        the component will bump into the left side of the screen and go no further.
    */
    z0 setMinimumOnscreenAmounts (i32 minimumWhenOffTheTop,
                                    i32 minimumWhenOffTheLeft,
                                    i32 minimumWhenOffTheBottom,
                                    i32 minimumWhenOffTheRight) noexcept;


    /** Returns the minimum distance the bounds can be off-screen. @see setMinimumOnscreenAmounts */
    i32 getMinimumWhenOffTheTop() const noexcept        { return minOffTop; }
    /** Returns the minimum distance the bounds can be off-screen. @see setMinimumOnscreenAmounts */
    i32 getMinimumWhenOffTheLeft() const noexcept       { return minOffLeft; }
    /** Returns the minimum distance the bounds can be off-screen. @see setMinimumOnscreenAmounts */
    i32 getMinimumWhenOffTheBottom() const noexcept     { return minOffBottom; }
    /** Returns the minimum distance the bounds can be off-screen. @see setMinimumOnscreenAmounts */
    i32 getMinimumWhenOffTheRight() const noexcept      { return minOffRight; }

    //==============================================================================
    /** Specifies a width-to-height ratio that the resizer should always maintain.

        If the value is 0, no aspect ratio is enforced. If it's non-zero, the width
        will always be maintained as this multiple of the height.

        @see setResizeLimits
    */
    z0 setFixedAspectRatio (f64 widthOverHeight) noexcept;

    /** Returns the aspect ratio that was set with setFixedAspectRatio().

        If no aspect ratio is being enforced, this will return 0.
    */
    f64 getFixedAspectRatio() const noexcept;


    //==============================================================================
    /** This callback changes the given coordinates to impose whatever the current
        constraints are set to be.

        @param bounds               the target position that should be examined and adjusted
        @param previousBounds       the component's current size
        @param limits               the region in which the component can be positioned
        @param isStretchingTop      whether the top edge of the component is being resized
        @param isStretchingLeft     whether the left edge of the component is being resized
        @param isStretchingBottom   whether the bottom edge of the component is being resized
        @param isStretchingRight    whether the right edge of the component is being resized
    */
    virtual z0 checkBounds (Rectangle<i32>& bounds,
                              const Rectangle<i32>& previousBounds,
                              const Rectangle<i32>& limits,
                              b8 isStretchingTop,
                              b8 isStretchingLeft,
                              b8 isStretchingBottom,
                              b8 isStretchingRight);

    /** This callback happens when the resizer is about to start dragging. */
    virtual z0 resizeStart();

    /** This callback happens when the resizer has finished dragging. */
    virtual z0 resizeEnd();

    /** Checks the given bounds, and then sets the component to the corrected size. */
    z0 setBoundsForComponent (Component* component,
                                Rectangle<i32> bounds,
                                b8 isStretchingTop,
                                b8 isStretchingLeft,
                                b8 isStretchingBottom,
                                b8 isStretchingRight);

    /** Performs a check on the current size of a component, and moves or resizes
        it if it fails the constraints.
    */
    z0 checkComponentBounds (Component* component);

    /** Called by setBoundsForComponent() to apply a new constrained size to a
        component.

        By default this just calls setBounds(), but is virtual in case it's needed for
        extremely cunning purposes.
    */
    virtual z0 applyBoundsToComponent (Component&, Rectangle<i32> bounds);

private:
    //==============================================================================
    i32 minW = 0, maxW = 0x3fffffff, minH = 0, maxH = 0x3fffffff;
    i32 minOffTop = 0, minOffLeft = 0, minOffBottom = 0, minOffRight = 0;
    f64 aspectRatio = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentBoundsConstrainer)
};

} // namespace drx
