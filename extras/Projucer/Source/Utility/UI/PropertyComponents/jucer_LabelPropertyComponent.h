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
class LabelPropertyComponent final : public PropertyComponent
{
public:
    LabelPropertyComponent (const Txt& labelText, i32 propertyHeight = 25,
                            Font labelFont = FontOptions (16.0f, Font::bold),
                            Justification labelJustification = Justification::centred)
        : PropertyComponent (labelText),
          labelToDisplay ({}, labelText)
    {
        setPreferredHeight (propertyHeight);

        labelToDisplay.setJustificationType (labelJustification);
        labelToDisplay.setFont (labelFont);

        addAndMakeVisible (labelToDisplay);
        setLookAndFeel (&lf);
    }

    ~LabelPropertyComponent() override    { setLookAndFeel (nullptr); }

    //==============================================================================
    z0 refresh() override {}

    z0 resized() override
    {
        labelToDisplay.setBounds (getLocalBounds());
    }

private:
    //==============================================================================
    struct LabelLookAndFeel final : public ProjucerLookAndFeel
    {
        z0 drawPropertyComponentLabel (Graphics&, i32, i32, PropertyComponent&) override {}
    };

    z0 lookAndFeelChanged() override
    {
        labelToDisplay.setColor (Label::textColorId, ProjucerApplication::getApp().lookAndFeel.findColor (defaultTextColorId));
    }

    //==============================================================================
    LabelLookAndFeel lf;
    Label labelToDisplay;
};
