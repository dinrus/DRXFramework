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

 name:             AnimationAppDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Simple animation application.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AnimationAppDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class AnimationAppDemo final : public AnimatedAppComponent
{
public:
    //==============================================================================
    AnimationAppDemo()
    {
        setSize (800, 600);
        setSynchroniseToVBlank (true);
    }

    z0 update() override
    {
        // This function is called at the frequency specified by the setFramesPerSecond() call
        // in the constructor. You can use it to update counters, animate values, etc.
    }

    z0 paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));

        g.setColor (getLookAndFeel().findColor (Slider::thumbColorId));
        auto fishLength = 15;

        Path spinePath;

        for (auto i = 0; i < fishLength; ++i)
        {
            auto radius = 100 + 10 * std::sin ((f32) getFrameCounter() * 0.1f + (f32) i * 0.5f);

            Point<f32> p ((f32) getWidth()  / 2.0f + 1.5f * radius * std::sin ((f32) getFrameCounter() * 0.02f + (f32) i * 0.12f),
                            (f32) getHeight() / 2.0f + 1.0f * radius * std::cos ((f32) getFrameCounter() * 0.04f + (f32) i * 0.12f));

            // draw the circles along the fish
            g.fillEllipse (p.x - (f32) i, p.y - (f32) i, 2.0f + 2.0f * (f32) i, 2.0f + 2.0f * (f32) i);

            if (i == 0)
                spinePath.startNewSubPath (p);  // if this is the first point, start a new path..
            else
                spinePath.lineTo (p);           // ...otherwise add the next point
        }

        // draw an outline around the path that we have created
        g.strokePath (spinePath, PathStrokeType (4.0f));
    }

    z0 resized() override
    {
        // This is called when this component is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationAppDemo)
};
