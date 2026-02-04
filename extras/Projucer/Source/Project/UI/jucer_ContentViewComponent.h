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
class ContentViewComponent final : public Component
{
public:
    ContentViewComponent()
    {
        setTitle ("Content");
        setFocusContainerType (Component::FocusContainerType::focusContainer);

        addAndMakeVisible (logoComponent);

        addAndMakeVisible (fileNameLabel);
        fileNameLabel.setJustificationType (Justification::centred);
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();

        fileNameLabel.setBounds (bounds.removeFromTop (15));

        if (content != nullptr)
            content->setBounds (bounds);
        else
            logoComponent.setBounds (bounds);
    }

    Component* getCurrentComponent() noexcept
    {
        return content.get();
    }

    z0 setContent (std::unique_ptr<Component> newContent,
                     const Txt& labelText)
    {
        content = std::move (newContent);
        addAndMakeVisible (content.get());

        fileNameLabel.setVisible (labelText.isNotEmpty());
        fileNameLabel.setText (labelText, dontSendNotification);

        resized();
    }

private:
    class LogoComponent final : public Component
    {
    public:
        z0 paint (Graphics& g) override
        {
            g.setColor (findColor (defaultTextColorId));

            auto bounds = getLocalBounds();
            bounds.reduce (bounds.getWidth() / 6, bounds.getHeight() / 6);

            g.setFont (15.0f);
            g.drawFittedText (versionInfo, bounds.removeFromBottom (50), Justification::centredBottom, 3);

            if (logo != nullptr)
                logo->drawWithin (g, bounds.withTrimmedBottom (bounds.getHeight() / 4).toFloat(),
                                  RectanglePlacement (RectanglePlacement::centred), 1.0f);
        }

    private:
        std::unique_ptr<Drawable> logo = []() -> std::unique_ptr<Drawable>
        {
            if (auto svg = parseXML (BinaryData::background_logo_svg))
                return Drawable::createFromSVG (*svg);

            jassertfalse;
            return {};
        }();

        Txt versionInfo = SystemStats::getDRXVersion()
                             + newLine
                             + ProjucerApplication::getApp().getVersionDescription();
    };

    std::unique_ptr<Component> content;
    LogoComponent logoComponent;
    Label fileNameLabel;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentViewComponent)
};
