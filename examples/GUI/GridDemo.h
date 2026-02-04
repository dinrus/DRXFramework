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

 name:             GridDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Responsive layouts using Grid.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        GridDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct GridDemo final : public Component
{
    GridDemo()
    {
        addGridItemPanel (Colors::aquamarine, "0");
        addGridItemPanel (Colors::red,        "1");
        addGridItemPanel (Colors::blue,       "2");
        addGridItemPanel (Colors::green,      "3");
        addGridItemPanel (Colors::orange,     "4");
        addGridItemPanel (Colors::white,      "5");
        addGridItemPanel (Colors::aquamarine, "6");
        addGridItemPanel (Colors::red,        "7");
        addGridItemPanel (Colors::blue,       "8");
        addGridItemPanel (Colors::green,      "9");
        addGridItemPanel (Colors::orange,     "10");
        addGridItemPanel (Colors::white,      "11");

        setSize (750, 750);
    }

    z0 addGridItemPanel (Color colour, tukk text)
    {
        addAndMakeVisible (items.add (new GridItemPanel (colour, text)));
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::black);
    }

    z0 resized() override
    {
        Grid grid;

        grid.rowGap    = 20_px;
        grid.columnGap = 20_px;

        using Track = Grid::TrackInfo;

        grid.templateRows = { Track (1_fr), Track (1_fr), Track (1_fr) };

        grid.templateColumns = { Track (1_fr),
                                 Track (1_fr),
                                 Track (1_fr) };


        grid.autoColumns = Track (1_fr);
        grid.autoRows    = Track (1_fr);

        grid.autoFlow = Grid::AutoFlow::column;

        grid.items.addArray ({ GridItem (items[0]).withArea (2, 2, 4, 4),
                               GridItem (items[1]),
                               GridItem (items[2]).withArea ({}, 3),
                               GridItem (items[3]),
                               GridItem (items[4]).withArea (GridItem::Span (2), {}),
                               GridItem (items[5]),
                               GridItem (items[6]),
                               GridItem (items[7]),
                               GridItem (items[8]),
                               GridItem (items[9]),
                               GridItem (items[10]),
                               GridItem (items[11])
                            });

        grid.performLayout (getLocalBounds());
    }

    //==============================================================================
    struct GridItemPanel  : public Component
    {
        GridItemPanel (Color colourToUse, const Txt& textToUse)
            : colour (colourToUse),
              text (textToUse)
        {}

        z0 paint (Graphics& g) override
        {
            g.fillAll (colour.withAlpha (0.5f));

            g.setColor (Colors::black);
            g.drawText (text, getLocalBounds().withSizeKeepingCentre (100, 100), Justification::centred, false);
        }

        Color colour;
        Txt text;
    };

    OwnedArray<GridItemPanel> items;
};
