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

#pragma once


//==============================================================================
class IntroScreen final : public Component
{
public:
    IntroScreen()
    {
        setOpaque (true);

        addAndMakeVisible (versionLabel);
        addAndMakeVisible (linkButton);
        addAndMakeVisible (logo);

        versionLabel.setText (Txt ("{version}  built on {date}")
                                  .replace ("{version}", SystemStats::getDRXVersion())
                                  .replace ("{date}",    Txt (__DATE__).replace ("  ", " ")),
                              dontSendNotification);

        linkButton.setColor (HyperlinkButton::textColorId, Colors::lightblue);

        setTitle ("Home");
        setFocusContainerType (FocusContainerType::focusContainer);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto area = getLocalBounds().reduced (10);

        auto bottomSlice = area.removeFromBottom (24);
        linkButton.setBounds (bottomSlice.removeFromRight (getWidth() / 4));
        versionLabel.setBounds (bottomSlice);

        logo.setBounds (area);
    }

private:
    Label versionLabel;
    HyperlinkButton linkButton { "www.drx.com", { "http://www.drx.com" } };

    //==============================================================================
    struct LogoDrawComponent final : public Component,
                                     private Timer
    {
        LogoDrawComponent()
        {
            setTitle ("DRX Logo");
            startTimerHz (30); // repaint at 30 fps
        }

        z0 paint (Graphics& g) override
        {
            Path wavePath;

            auto waveStep = 10.0f;
            auto waveY = (f32) getHeight() * 0.5f;
            i32 i = 0;

            for (auto x = waveStep * 0.5f; x < (f32) getWidth(); x += waveStep)
            {
                auto y1 = waveY + (f32) getHeight() * 0.05f * std::sin ((f32) i * 0.38f + elapsed);
                auto y2 = waveY + (f32) getHeight() * 0.10f * std::sin ((f32) i * 0.20f + elapsed * 2.0f);

                wavePath.addLineSegment ({ x, y1, x, y2 }, 2.0f);
                wavePath.addEllipse (x - waveStep * 0.3f, y1 - waveStep * 0.3f, waveStep * 0.6f, waveStep * 0.6f);
                wavePath.addEllipse (x - waveStep * 0.3f, y2 - waveStep * 0.3f, waveStep * 0.6f, waveStep * 0.6f);

                ++i;
            }

            g.setColor (Color::greyLevel (0.4f));
            g.fillPath (wavePath);

            g.setColor (Color (0xc4f39082));
            g.fillPath (logoPath, RectanglePlacement (RectanglePlacement::centred)
                                    .getTransformToFit (logoPath.getBounds(),
                                                        getLocalBounds().reduced (20, getHeight() / 4).toFloat()));
        }

        z0 timerCallback() override
        {
            repaint();
            elapsed += 0.02f;
        }

        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::image);
        }

        Path logoPath  { getDRXLogoPath() };
        f32 elapsed = 0.0f;
    };

    LogoDrawComponent logo;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IntroScreen)
};
