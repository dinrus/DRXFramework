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

ResizableBorderComponent::Zone::Zone() noexcept {}
ResizableBorderComponent::Zone::Zone (i32 zoneFlags) noexcept  : zone (zoneFlags) {}
ResizableBorderComponent::Zone::Zone (const ResizableBorderComponent::Zone& other) noexcept  : zone (other.zone) {}

ResizableBorderComponent::Zone& ResizableBorderComponent::Zone::operator= (const ResizableBorderComponent::Zone& other) noexcept
{
    zone = other.zone;
    return *this;
}

b8 ResizableBorderComponent::Zone::operator== (const ResizableBorderComponent::Zone& other) const noexcept      { return zone == other.zone; }
b8 ResizableBorderComponent::Zone::operator!= (const ResizableBorderComponent::Zone& other) const noexcept      { return zone != other.zone; }

ResizableBorderComponent::Zone ResizableBorderComponent::Zone::fromPositionOnBorder (Rectangle<i32> totalSize,
                                                                                     BorderSize<i32> border,
                                                                                     Point<i32> position)
{
    i32 z = 0;

    if (totalSize.contains (position)
         && ! border.subtractedFrom (totalSize).contains (position))
    {
        auto minW = jmax (totalSize.getWidth() / 10, jmin (10, totalSize.getWidth() / 3));

        if (position.x < jmax (border.getLeft(), minW) && border.getLeft() > 0)
            z |= left;
        else if (position.x >= totalSize.getWidth() - jmax (border.getRight(), minW) && border.getRight() > 0)
            z |= right;

        auto minH = jmax (totalSize.getHeight() / 10, jmin (10, totalSize.getHeight() / 3));

        if (position.y < jmax (border.getTop(), minH) && border.getTop() > 0)
            z |= top;
        else if (position.y >= totalSize.getHeight() - jmax (border.getBottom(), minH) && border.getBottom() > 0)
            z |= bottom;
    }

    return Zone (z);
}

MouseCursor ResizableBorderComponent::Zone::getMouseCursor() const noexcept
{
    auto mc = MouseCursor::NormalCursor;

    switch (zone)
    {
        case (left | top):      mc = MouseCursor::TopLeftCornerResizeCursor; break;
        case top:               mc = MouseCursor::TopEdgeResizeCursor; break;
        case (right | top):     mc = MouseCursor::TopRightCornerResizeCursor; break;
        case left:              mc = MouseCursor::LeftEdgeResizeCursor; break;
        case right:             mc = MouseCursor::RightEdgeResizeCursor; break;
        case (left | bottom):   mc = MouseCursor::BottomLeftCornerResizeCursor; break;
        case bottom:            mc = MouseCursor::BottomEdgeResizeCursor; break;
        case (right | bottom):  mc = MouseCursor::BottomRightCornerResizeCursor; break;
        default:                break;
    }

    return mc;
}

//==============================================================================
ResizableBorderComponent::ResizableBorderComponent (Component* componentToResize,
                                                    ComponentBoundsConstrainer* boundsConstrainer)
   : component (componentToResize),
     constrainer (boundsConstrainer),
     borderSize (5)
{
}

ResizableBorderComponent::~ResizableBorderComponent() = default;

//==============================================================================
z0 ResizableBorderComponent::paint (Graphics& g)
{
    getLookAndFeel().drawResizableFrame (g, getWidth(), getHeight(), borderSize);
}

z0 ResizableBorderComponent::mouseEnter (const MouseEvent& e)
{
    updateMouseZone (e);
}

z0 ResizableBorderComponent::mouseMove (const MouseEvent& e)
{
    updateMouseZone (e);
}

z0 ResizableBorderComponent::mouseDown (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    updateMouseZone (e);

    originalBounds = component->getBounds();

    if (auto* peer = component->getPeer())
        if (&peer->getComponent() == component)
            peer->startHostManagedResize (peer->globalToLocal (localPointToGlobal (e.getPosition())), mouseZone);

    if (constrainer != nullptr)
        constrainer->resizeStart();
}

z0 ResizableBorderComponent::mouseDrag (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    auto newBounds = mouseZone.resizeRectangleBy (originalBounds, e.getOffsetFromDragStart());

    if (constrainer != nullptr)
    {
        constrainer->setBoundsForComponent (component, newBounds,
                                            mouseZone.isDraggingTopEdge(),
                                            mouseZone.isDraggingLeftEdge(),
                                            mouseZone.isDraggingBottomEdge(),
                                            mouseZone.isDraggingRightEdge());
    }
    else
    {
        if (auto* p = component->getPositioner())
            p->applyNewBounds (newBounds);
        else
            component->setBounds (newBounds);
    }
}

z0 ResizableBorderComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != nullptr)
        constrainer->resizeEnd();
}

b8 ResizableBorderComponent::hitTest (i32 x, i32 y)
{
    return ! borderSize.subtractedFrom (getLocalBounds()).contains (x, y);
}

z0 ResizableBorderComponent::setBorderThickness (BorderSize<i32> newBorderSize)
{
    if (borderSize != newBorderSize)
    {
        borderSize = newBorderSize;
        repaint();
    }
}

BorderSize<i32> ResizableBorderComponent::getBorderThickness() const
{
    return borderSize;
}

z0 ResizableBorderComponent::updateMouseZone (const MouseEvent& e)
{
    auto newZone = Zone::fromPositionOnBorder (getLocalBounds(), borderSize, e.getPosition());

    if (mouseZone != newZone)
    {
        mouseZone = newZone;
        setMouseCursor (newZone.getMouseCursor());
    }
}

} // namespace drx
