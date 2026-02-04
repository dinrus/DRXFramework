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
class AboutWindowComponent final : public Component
{
public:
    AboutWindowComponent()
    {
        addAndMakeVisible (titleLabel);
        titleLabel.setJustificationType (Justification::centred);
        titleLabel.setFont (FontOptions (35.0f, Font::FontStyleFlags::bold));

        auto buildDate = Time::getCompilationDate();
        addAndMakeVisible (versionLabel);
        versionLabel.setText ("DRX v" + ProjucerApplication::getApp().getApplicationVersion()
                              + "\nBuild date: " + Txt (buildDate.getDayOfMonth())
                                                 + " " + Time::getMonthName (buildDate.getMonth(), true)
                                                 + " " + Txt (buildDate.getYear()),
                              dontSendNotification);

        versionLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (copyrightLabel);
        copyrightLabel.setJustificationType (Justification::centred);

        addAndMakeVisible (aboutButton);
        aboutButton.setTooltip ( {} );
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromBottom (20);

        auto leftSlice   = bounds.removeFromLeft (150);
        auto centreSlice = bounds.withTrimmedRight (150);

        juceLogoBounds = leftSlice.removeFromTop (150).toFloat();
        juceLogoBounds.setWidth (juceLogoBounds.getWidth() + 100);
        juceLogoBounds.setHeight (juceLogoBounds.getHeight() + 100);

        auto titleHeight = 40;

        centreSlice.removeFromTop ((centreSlice.getHeight() / 2) - (titleHeight / 2));

        titleLabel.setBounds (centreSlice.removeFromTop (titleHeight));

        centreSlice.removeFromTop (10);
        versionLabel.setBounds (centreSlice.removeFromTop (40));

        centreSlice.removeFromTop (10);
        aboutButton.setBounds (centreSlice.removeFromTop (20));

        copyrightLabel.setBounds (getLocalBounds().removeFromBottom (50));
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (backgroundColorId));

        if (juceLogo != nullptr)
            juceLogo->drawWithin (g, juceLogoBounds.translated (-75, -75), RectanglePlacement::centred, 1.0);
    }

private:
    Label titleLabel { "title", "PROJUCER" },
          versionLabel { "version" },
          copyrightLabel { "copyright", Txt (CharPointer_UTF8 ("\xc2\xa9")) + Txt (" 2020 DinrusPro") };

    HyperlinkButton aboutButton { "About Us", URL ("https://drx.com") };

    Rectangle<f32> juceLogoBounds;

    std::unique_ptr<Drawable> juceLogo { Drawable::createFromImageData (BinaryData::drx_icon_png,
                                                                        BinaryData::drx_icon_pngSize) };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutWindowComponent)
};
