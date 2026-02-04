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

#pragma once


//==============================================================================
struct Icon
{
    Icon() = default;

    Icon (const Path& pathToUse, Color pathColor)
        : path (pathToUse),
          colour (pathColor)
    {
    }

    z0 draw (Graphics& g, const drx::Rectangle<f32>& area, b8 isCrossedOut) const
    {
        if (! path.isEmpty())
        {
            g.setColor (colour);

            const RectanglePlacement placement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize);
            g.fillPath (path, placement.getTransformToFit (path.getBounds(), area));

            if (isCrossedOut)
            {
                g.setColor (Colors::red.withAlpha (0.8f));
                g.drawLine ((f32) area.getX(), area.getY() + area.getHeight() * 0.2f,
                            (f32) area.getRight(), area.getY() + area.getHeight() * 0.8f, 3.0f);
            }
        }
    }

    Icon withContrastingColorTo (Color background) const
    {
        return Icon (path, background.contrasting (colour, 0.6f));
    }

    Icon withColor (Color newColor)
    {
        return Icon (path, newColor);
    }

    Path path;
    Color colour;
};

//==============================================================================
class Icons
{
public:
    Icons();

    Path imageDoc, config, graph, info, warning, user, closedFolder, exporter, fileExplorer, file,
         modules, openFolder, settings, singleModule, plus, android, linux, xcode, visualStudio;

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Icons)
};

const Icons& getIcons();
