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
class ItemPreviewComponent final : public Component
{
public:
    ItemPreviewComponent (const File& f)  : file (f)
    {
        setOpaque (true);
        tryToLoadImage();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (backgroundColorId));

        if (drawable != nullptr)
        {
            auto contentBounds = drawable->getDrawableBounds();

            if (auto* dc = dynamic_cast<DrawableComposite*> (drawable.get()))
            {
                auto r = dc->getContentArea();

                if (! r.isEmpty())
                    contentBounds = r;
            }

            auto area = RectanglePlacement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize)
                            .appliedTo (contentBounds, Rectangle<f32> (4.0f, 22.0f, (f32) getWidth() - 8.0f, (f32) getHeight() - 26.0f));

            Path p;
            p.addRectangle (area);
            DropShadow (Colors::black.withAlpha (0.5f), 6, Point<i32> (0, 1)).drawForPath (g, p);

            g.fillCheckerBoard (area, 24.0f, 24.0f, Color (0xffffffff), Color (0xffeeeeee));

            drawable->draw (g, 1.0f, RectanglePlacement (RectanglePlacement::stretchToFit)
                                        .getTransformToFit (contentBounds, area.toFloat()));
        }

        g.setFont (FontOptions (14.0f, Font::bold));
        g.setColor (findColor (defaultTextColorId));
        g.drawMultiLineText (facts.joinIntoString ("\n"), 10, 15, getWidth() - 16);
    }

private:
    StringArray facts;
    File file;
    std::unique_ptr<Drawable> drawable;

    z0 tryToLoadImage()
    {
        facts.clear();
        facts.add (file.getFullPathName());
        drawable.reset();

        if (auto input = std::unique_ptr<FileInputStream> (file.createInputStream()))
        {
            auto totalSize = input->getTotalLength();
            Txt formatName;

            if (auto* format = ImageFileFormat::findImageFormatForStream (*input))
                formatName = " " + format->getFormatName();

            input.reset();

            auto image = ImageCache::getFromFile (file);

            if (image.isValid())
            {
                auto* d = new DrawableImage();
                d->setImage (image);
                drawable.reset (d);

                facts.add (Txt (image.getWidth()) + " x " + Txt (image.getHeight()) + formatName);
            }

            if (totalSize > 0)
                facts.add (File::descriptionOfSizeInBytes (totalSize));
        }

        if (drawable == nullptr)
            if (auto svg = parseXML (file))
                drawable = Drawable::createFromSVG (*svg);

        facts.removeEmptyStrings (true);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemPreviewComponent)
};
