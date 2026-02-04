/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             ComponentTransformsDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Applies transformations to components.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        ComponentTransformsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "WidgetsDemo.h"

//==============================================================================
class ComponentTransformsDemo final : public Component
{
public:
    ComponentTransformsDemo()
    {
        content.reset (new WidgetsDemo (true));
        addAndMakeVisible (content.get());
        content->setSize (750, 500);

        for (i32 i = 0; i < 3; ++i)
        {
            auto* d = new CornerDragger();
            addAndMakeVisible (draggers.add (d));
        }

        draggers.getUnchecked (0)->relativePos = { 0.10f, 0.15f };
        draggers.getUnchecked (1)->relativePos = { 0.95f, 0.05f };
        draggers.getUnchecked (2)->relativePos = { 0.05f, 0.85f };

        setSize (800, 600);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));

        g.setColor (Colors::white);
        g.setFont (15.0f);
        g.drawFittedText ("Drag the corner-points around to show how complex components can have affine-transforms applied...",
                          getLocalBounds().removeFromBottom (40).reduced (10, 0), Justification::centred, 3);
    }

    z0 resized() override
    {
        for (auto* d : draggers)
            d->setCentrePosition (proportionOfWidth  (d->relativePos.x),
                                  proportionOfHeight (d->relativePos.y));
    }

    z0 childBoundsChanged (Component* child) override
    {
        if (dynamic_cast<CornerDragger*> (child) != nullptr)
            updateTransform();
    }

private:
    std::unique_ptr<Component> content;

    struct CornerDragger final : public Component
    {
        CornerDragger()
        {
            setSize (30, 30);
            setRepaintsOnMouseActivity (true);
        }

        z0 paint (Graphics& g) override
        {
            g.setColor (Colors::white.withAlpha (isMouseOverOrDragging() ? 0.9f : 0.5f));
            g.fillEllipse (getLocalBounds().reduced (3).toFloat());

            g.setColor (Colors::darkgreen);
            g.drawEllipse (getLocalBounds().reduced (3).toFloat(), 2.0f);
        }

        z0 resized() override
        {
            constrainer.setMinimumOnscreenAmounts (getHeight(), getWidth(), getHeight(), getWidth());
        }

        z0 moved() override
        {
            if (isMouseButtonDown())
                relativePos = getBounds().getCentre().toFloat() / Point<i32> (getParentWidth(), getParentHeight()).toFloat();
        }

        z0 mouseDown (const MouseEvent& e) override   { dragger.startDraggingComponent (this, e); }
        z0 mouseDrag (const MouseEvent& e) override   { dragger.dragComponent (this, e, &constrainer); }

        Point<f32> relativePos;

    private:
        ComponentBoundsConstrainer constrainer;
        ComponentDragger dragger;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CornerDragger)
    };

    OwnedArray<CornerDragger> draggers;

    Point<f32> getDraggerPos (i32 index) const
    {
        return draggers.getUnchecked (index)->getBounds().getCentre().toFloat();
    }

    z0 updateTransform()
    {
        auto p0 = getDraggerPos (0);
        auto p1 = getDraggerPos (1);
        auto p2 = getDraggerPos (2);

        if (p0 != p1 && p1 != p2 && p0 != p2)
            content->setTransform (AffineTransform::fromTargetPoints (0, 0, p0.x, p0.y,
                                                                      (f32) content->getWidth(), 0,  p1.x, p1.y,
                                                                      0, (f32) content->getHeight(), p2.x, p2.y));
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentTransformsDemo)
};
