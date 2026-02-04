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
class IconButton final : public Button
{
public:
    IconButton (Txt buttonName, Image imageToDisplay)
        : Button (buttonName),
          iconImage (imageToDisplay)
    {
        setTooltip (buttonName);
    }

    IconButton (Txt buttonName, Path pathToDisplay)
        : Button (buttonName),
          iconPath (pathToDisplay),
          iconImage (createImageFromPath (iconPath))
    {
        setTooltip (buttonName);
    }

    z0 setImage (Image newImage)
    {
        iconImage = newImage;
        repaint();
    }

    z0 setPath (Path newPath)
    {
        iconImage = createImageFromPath (newPath);
        repaint();
    }

    z0 setBackgroundColor (Color backgroundColorToUse)
    {
        backgroundColor = backgroundColorToUse;
        usingNonDefaultBackgroundColor = true;
    }

    z0 setIconInset (i32 newIconInset)
    {
        iconInset = newIconInset;
        repaint();
    }

    z0 paintButton (Graphics& g, b8 isMouseOverButton, b8 isButtonDown) override
    {
        f32 alpha = 1.0f;

        if (! isEnabled())
        {
            isMouseOverButton = false;
            isButtonDown = false;

            alpha = 0.2f;
        }

        auto fill = isButtonDown ? backgroundColor.darker (0.5f)
                                 : isMouseOverButton ? backgroundColor.darker (0.2f)
                                                     : backgroundColor;

        auto bounds = getLocalBounds();

        if (isButtonDown)
            bounds.reduce (2, 2);

        Path ellipse;
        ellipse.addEllipse (bounds.toFloat());
        g.reduceClipRegion (ellipse);

        g.setColor (fill.withAlpha (alpha));
        g.fillAll();

        g.setOpacity (alpha);
        g.drawImage (iconImage, bounds.reduced (iconInset).toFloat(), RectanglePlacement::fillDestination, false);
    }

private:
    z0 lookAndFeelChanged() override
    {
        if (! usingNonDefaultBackgroundColor)
            backgroundColor = findColor (defaultButtonBackgroundColorId);

        if (iconPath != Path())
            iconImage = createImageFromPath (iconPath);

        repaint();
    }

    Image createImageFromPath (Path path)
    {
        Image image (Image::ARGB, 250, 250, true);
        Graphics g (image);

        g.setColor (findColor (defaultIconColorId));

        g.fillPath (path, RectanglePlacement (RectanglePlacement::centred)
                            .getTransformToFit (path.getBounds(), image.getBounds().toFloat()));

        return image;
    }

    Path iconPath;
    Image iconImage;
    Color backgroundColor { findColor (defaultButtonBackgroundColorId) };
    b8 usingNonDefaultBackgroundColor = false;
    i32 iconInset = 2;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IconButton)
};
