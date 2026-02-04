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

namespace drx::detail
{

struct StandardCachedComponentImage : public CachedComponentImage
{
    explicit StandardCachedComponentImage (Component& c) noexcept
        : owner (c)
    {
    }

    z0 paint (Graphics& g) override
    {
        scale = g.getInternalContext().getPhysicalPixelScaleFactor();
        auto compBounds = owner.getLocalBounds();
        auto imageBounds = compBounds * scale;

        if (image.isNull() || image.getBounds() != imageBounds)
        {
            image = Image (owner.isOpaque() ? Image::RGB
                                            : Image::ARGB,
                           jmax (1, imageBounds.getWidth()),
                           jmax (1, imageBounds.getHeight()),
                           ! owner.isOpaque());
            image.setBackupEnabled (false);
            validArea.clear();
        }

        // If the cached image is outdated but cannot be backed-up, this indicates that the graphics
        // device holding the most recent copy of the cached image has gone away. Therefore, we've
        // effectively lost the contents of the cache, and we must repaint the entire component.
        if (auto ptr = image.getPixelData())
            if (auto* extensions = ptr->getBackupExtensions())
                if (extensions->needsBackup() && ! extensions->canBackup())
                    validArea.clear();

        if (! validArea.containsRectangle (compBounds))
        {
            Graphics imG (image);
            auto& lg = imG.getInternalContext();

            lg.addTransform (AffineTransform::scale (scale));

            for (auto& i : validArea)
                lg.excludeClipRectangle (i);

            if (! owner.isOpaque())
            {
                lg.setFill (Colors::transparentBlack);
                lg.fillRect (compBounds, true);
                lg.setFill (Colors::black);
            }

            owner.paintEntireComponent (imG, true);
        }

        validArea = compBounds;

        g.setColor (Colors::black.withAlpha (owner.getAlpha()));
        g.drawImageTransformed (image, AffineTransform::scale ((f32) compBounds.getWidth()  / (f32) imageBounds.getWidth(),
                                                               (f32) compBounds.getHeight() / (f32) imageBounds.getHeight()), false);
    }

    b8 invalidateAll() override                            { validArea.clear(); return true; }
    b8 invalidate (const Rectangle<i32>& area) override    { validArea.subtract (area); return true; }
    z0 releaseResources() override                         { image = Image(); }

private:
    Image image;
    RectangleList<i32> validArea;
    Component& owner;
    f32 scale = 1.0f;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandardCachedComponentImage)
};

} // namespace drx::detail
