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
    A component that resizes its parent component when dragged.

    This component forms a frame around the edge of a component, allowing it to
    be dragged by the edges or corners to resize it - like the way windows are
    resized in MSWindows or Linux.

    To use it, just add it to your component, making it fill the entire parent component
    (there's a mouse hit-test that only traps mouse-events which land around the
    edge of the component, so it's even ok to put it on top of any other components
    you're using). Make sure you rescale the resizer component to fill the parent
    each time the parent's size changes.

    @see ResizableCornerComponent

    @tags{GUI}
*/
class DRX_API  ResizableBorderComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a resizer.

        Pass in the target component which you want to be resized when this one is
        dragged.

        The target component will usually be a parent of the resizer component, but this
        isn't mandatory.

        Remember that when the target component is resized, it'll need to move and
        resize this component to keep it in place, as this won't happen automatically.

        If the constrainer parameter is not a nullptr, then this object will be used to
        enforce limits on the size and position that the component can be stretched to.
        Make sure that the constrainer isn't deleted while still in use by this object.

        @see ComponentBoundsConstrainer
    */
    ResizableBorderComponent (Component* componentToResize,
                              ComponentBoundsConstrainer* constrainer);

    /** Destructor. */
    ~ResizableBorderComponent() override;


    //==============================================================================
    /** Specifies how many pixels wide the draggable edges of this component are.

        @see getBorderThickness
    */
    z0 setBorderThickness (BorderSize<i32> newBorderSize);

    /** Returns the number of pixels wide that the draggable edges of this component are.

        @see setBorderThickness
    */
    BorderSize<i32> getBorderThickness() const;


    //==============================================================================
    /** Represents the different sections of a resizable border, which allow it to
        resized in different ways.
    */
    class Zone
    {
    public:
        //==============================================================================
        enum Zones
        {
            centre  = 0,
            left    = 1,
            top     = 2,
            right   = 4,
            bottom  = 8
        };

        //==============================================================================
        /** Creates a Zone from a combination of the flags in zoneFlags. */
        explicit Zone (i32 zoneFlags) noexcept;

        Zone() noexcept;
        Zone (const Zone&) noexcept;
        Zone& operator= (const Zone&) noexcept;

        b8 operator== (const Zone&) const noexcept;
        b8 operator!= (const Zone&) const noexcept;

        //==============================================================================
        /** Given a point within a rectangle with a resizable border, this returns the
            zone that the point lies within.
        */
        static Zone fromPositionOnBorder (Rectangle<i32> totalSize,
                                          BorderSize<i32> border,
                                          Point<i32> position);

        /** Returns an appropriate mouse-cursor for this resize zone. */
        MouseCursor getMouseCursor() const noexcept;

        /** Возвращает true, если dragging this zone will move the entire object without resizing it. */
        b8 isDraggingWholeObject() const noexcept     { return zone == centre; }
        /** Возвращает true, если dragging this zone will move the object's left edge. */
        b8 isDraggingLeftEdge() const noexcept        { return (zone & left) != 0; }
        /** Возвращает true, если dragging this zone will move the object's right edge. */
        b8 isDraggingRightEdge() const noexcept       { return (zone & right) != 0; }
        /** Возвращает true, если dragging this zone will move the object's top edge. */
        b8 isDraggingTopEdge() const noexcept         { return (zone & top) != 0; }
        /** Возвращает true, если dragging this zone will move the object's bottom edge. */
        b8 isDraggingBottomEdge() const noexcept      { return (zone & bottom) != 0; }

        /** Resizes this rectangle by the given amount, moving just the edges that this zone
            applies to.
        */
        template <typename ValueType>
        Rectangle<ValueType> resizeRectangleBy (Rectangle<ValueType> original,
                                                const Point<ValueType>& distance) const noexcept
        {
            if (isDraggingWholeObject())
                return original + distance;

            if (isDraggingLeftEdge())   original.setLeft (jmin (original.getRight(), original.getX() + distance.x));
            if (isDraggingRightEdge())  original.setWidth (jmax (ValueType(), original.getWidth() + distance.x));
            if (isDraggingTopEdge())    original.setTop (jmin (original.getBottom(), original.getY() + distance.y));
            if (isDraggingBottomEdge()) original.setHeight (jmax (ValueType(), original.getHeight() + distance.y));

            return original;
        }

        /** Returns the raw flags for this zone. */
        i32 getZoneFlags() const noexcept               { return zone; }

    private:
        //==============================================================================
        i32 zone = centre;
    };

    /** Returns the zone in which the mouse was last seen. */
    Zone getCurrentZone() const noexcept                 { return mouseZone; }

protected:
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 mouseEnter (const MouseEvent&) override;
    /** @internal */
    z0 mouseMove (const MouseEvent&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    b8 hitTest (i32 x, i32 y) override;

private:
    WeakReference<Component> component;
    ComponentBoundsConstrainer* constrainer;
    BorderSize<i32> borderSize;
    Rectangle<i32> originalBounds;
    Zone mouseZone;

    z0 updateMouseZone (const MouseEvent&);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizableBorderComponent)
};

} // namespace drx
