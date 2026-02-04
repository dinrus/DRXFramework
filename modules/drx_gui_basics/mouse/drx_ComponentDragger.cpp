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

ComponentDragger::ComponentDragger() {}
ComponentDragger::~ComponentDragger() {}

//==============================================================================
z0 ComponentDragger::startDraggingComponent (Component* const componentToDrag, const MouseEvent& e)
{
    jassert (componentToDrag != nullptr);
    jassert (e.mods.isAnyMouseButtonDown()); // The event has to be a drag event!

    if (componentToDrag != nullptr)
        mouseDownWithinTarget = e.getEventRelativeTo (componentToDrag).getMouseDownPosition();
}

z0 ComponentDragger::dragComponent (Component* const componentToDrag, const MouseEvent& e,
                                      ComponentBoundsConstrainer* const constrainer)
{
    jassert (componentToDrag != nullptr);
    jassert (e.mods.isAnyMouseButtonDown()); // The event has to be a drag event!

    if (componentToDrag != nullptr)
    {
        auto bounds = componentToDrag->getBounds();

        // If the component is a window, multiple mouse events can get queued while it's in the same position,
        // so their coordinates become wrong after the first one moves the window, so in that case, we'll use
        // the current mouse position instead of the one that the event contains...
        if (componentToDrag->isOnDesktop())
            bounds += componentToDrag->getLocalPoint (nullptr, e.source.getScreenPosition()).roundToInt() - mouseDownWithinTarget;
        else
            bounds += e.getEventRelativeTo (componentToDrag).getPosition() - mouseDownWithinTarget;

        if (constrainer != nullptr)
            constrainer->setBoundsForComponent (componentToDrag, bounds, false, false, false, false);
        else
            componentToDrag->setBounds (bounds);
    }
}

} // namespace drx
