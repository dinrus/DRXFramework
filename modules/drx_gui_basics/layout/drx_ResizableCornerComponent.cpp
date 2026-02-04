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

ResizableCornerComponent::ResizableCornerComponent (Component* componentToResize,
                                                    ComponentBoundsConstrainer* boundsConstrainer)
   : component (componentToResize),
     constrainer (boundsConstrainer)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (MouseCursor::BottomRightCornerResizeCursor);
}

ResizableCornerComponent::~ResizableCornerComponent() = default;

//==============================================================================
z0 ResizableCornerComponent::paint (Graphics& g)
{
    getLookAndFeel().drawCornerResizer (g, getWidth(), getHeight(),
                                        isMouseOverOrDragging(),
                                        isMouseButtonDown());
}

z0 ResizableCornerComponent::mouseDown (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer is supposed to be controlling!
        return;
    }

    originalBounds = component->getBounds();

    using Zone = ResizableBorderComponent::Zone;
    const Zone zone { Zone::bottom | Zone::right };

    if (auto* peer = component->getPeer())
        if (&peer->getComponent() == component)
            peer->startHostManagedResize (peer->globalToLocal (localPointToGlobal (e.getPosition())), zone);

    if (constrainer != nullptr)
        constrainer->resizeStart();
}

z0 ResizableCornerComponent::mouseDrag (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer is supposed to be controlling!
        return;
    }

    auto r = originalBounds.withSize (originalBounds.getWidth() + e.getDistanceFromDragStartX(),
                                      originalBounds.getHeight() + e.getDistanceFromDragStartY());

    if (constrainer != nullptr)
        constrainer->setBoundsForComponent (component, r, false, false, true, true);
    else if (auto pos = component->getPositioner())
        pos->applyNewBounds (r);
    else
        component->setBounds (r);
}

z0 ResizableCornerComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != nullptr)
        constrainer->resizeEnd();
}

b8 ResizableCornerComponent::hitTest (i32 x, i32 y)
{
    if (getWidth() <= 0)
        return false;

    auto yAtX = getHeight() - (getHeight() * x / getWidth());
    return y >= yAtX - getHeight() / 4;
}

} // namespace drx
