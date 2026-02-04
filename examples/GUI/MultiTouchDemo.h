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

 name:             MultiTouchDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases multi-touch features.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MultiTouchDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class MultiTouchDemo final : public Component
{
public:
    MultiTouchDemo()
    {
        setOpaque (true);

        setSize (500, 500);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Color::greyLevel (0.4f)));

        g.setColor (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::defaultText,
                                             Colors::lightgrey));
        g.setFont (14.0f);
        g.drawFittedText ("Drag here with as many fingers as you have!",
                          getLocalBounds().reduced (30), Justification::centred, 4);

        for (auto* trail : trails)
            drawTrail (*trail, g);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        auto* t = getTrail (e.source);

        if (t == nullptr)
        {
            t = new Trail (e.source);
            t->path.startNewSubPath (e.position);
            trails.add (t);
        }

        t->pushPoint (e.position, e.mods, e.pressure);
        repaint();
    }

    z0 mouseUp (const MouseEvent& e) override
    {
        trails.removeObject (getTrail (e.source));
        repaint();
    }

    struct Trail
    {
        Trail (const MouseInputSource& ms)
            : source (ms)
        {}

        z0 pushPoint (Point<f32> newPoint, ModifierKeys newMods, f32 pressure)
        {
            currentPosition = newPoint;
            modifierKeys = newMods;

            if (lastPoint.getDistanceFrom (newPoint) > 5.0f)
            {
                if (lastPoint != Point<f32>())
                {
                    Path newSegment;
                    newSegment.startNewSubPath (lastPoint);
                    newSegment.lineTo (newPoint);

                    auto diameter = 20.0f * (pressure > 0 && pressure < 1.0f ? pressure : 1.0f);

                    PathStrokeType (diameter, PathStrokeType::curved, PathStrokeType::rounded).createStrokedPath (newSegment, newSegment);
                    path.addPath (newSegment);
                }

                lastPoint = newPoint;
            }
        }

        MouseInputSource source;
        Path path;
        Color colour  { getRandomBrightColor().withAlpha (0.6f) };
        Point<f32> lastPoint, currentPosition;
        ModifierKeys modifierKeys;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trail)
    };

    OwnedArray<Trail> trails;

    z0 drawTrail (Trail& trail, Graphics& g)
    {
        g.setColor (trail.colour);
        g.fillPath (trail.path);

        auto radius = 40.0f;

        g.setColor (Colors::black);
        g.drawEllipse (trail.currentPosition.x - radius,
                       trail.currentPosition.y - radius,
                       radius * 2.0f, radius * 2.0f, 2.0f);

        g.setFont (14.0f);

        Txt desc ("Mouse #");
        desc << trail.source.getIndex();

        auto pressure = trail.source.getCurrentPressure();

        if (pressure > 0.0f && pressure < 1.0f)
            desc << "  (pressure: " << (i32) (pressure * 100.0f) << "%)";

        if (trail.modifierKeys.isCommandDown()) desc << " (CMD)";
        if (trail.modifierKeys.isShiftDown())   desc << " (SHIFT)";
        if (trail.modifierKeys.isCtrlDown())    desc << " (CTRL)";
        if (trail.modifierKeys.isAltDown())     desc << " (ALT)";

        g.drawText (desc,
                    Rectangle<i32> ((i32) trail.currentPosition.x - 200,
                                    (i32) trail.currentPosition.y - 60,
                                    400, 20),
                    Justification::centredTop, false);
    }

    Trail* getTrail (const MouseInputSource& source)
    {
        for (auto* trail : trails)
        {
            if (trail->source == source)
                return trail;
        }

        return nullptr;
    }
};
