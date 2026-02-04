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

ImagePreviewComponent::ImagePreviewComponent()
{
}

ImagePreviewComponent::~ImagePreviewComponent()
{
}

//==============================================================================
z0 ImagePreviewComponent::getThumbSize (i32& w, i32& h) const
{
    auto availableW = proportionOfWidth (0.97f);
    auto availableH = getHeight() - 13 * 4;

    auto scale = jmin (1.0,
                       availableW / (f64) w,
                       availableH / (f64) h);

    w = roundToInt (scale * w);
    h = roundToInt (scale * h);
}

z0 ImagePreviewComponent::selectedFileChanged (const File& file)
{
    if (fileToLoad != file)
    {
        fileToLoad = file;
        startTimer (100);
    }
}

z0 ImagePreviewComponent::timerCallback()
{
    stopTimer();

    currentThumbnail = Image();
    currentDetails.clear();
    repaint();

    FileInputStream in (fileToLoad);

    if (in.openedOk() && fileToLoad.existsAsFile())
    {
        if (auto format = ImageFileFormat::findImageFormatForStream (in))
        {
            currentThumbnail = format->decodeImage (in);

            if (currentThumbnail.isValid())
            {
                auto w = currentThumbnail.getWidth();
                auto h = currentThumbnail.getHeight();

                currentDetails
                    << fileToLoad.getFileName() << "\n"
                    << format->getFormatName() << "\n"
                    << w << " x " << h << " pixels\n"
                    << File::descriptionOfSizeInBytes (fileToLoad.getSize());

                getThumbSize (w, h);

                currentThumbnail = currentThumbnail.rescaled (w, h);
            }
        }
    }
}

z0 ImagePreviewComponent::paint (Graphics& g)
{
    if (currentThumbnail.isValid())
    {
        g.setFont (13.0f);

        auto w = currentThumbnail.getWidth();
        auto h = currentThumbnail.getHeight();
        getThumbSize (w, h);

        i32k numLines = 4;
        auto totalH = 13 * numLines + h + 4;
        auto y = (getHeight() - totalH) / 2;

        g.drawImageWithin (currentThumbnail,
                           (getWidth() - w) / 2, y, w, h,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);

        g.drawFittedText (currentDetails,
                          0, y + h + 4, getWidth(), 100,
                          Justification::centredTop, numLines);
    }
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ImagePreviewComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::image);
}

} // namespace drx
