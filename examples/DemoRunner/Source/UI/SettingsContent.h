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

#include "MainComponent.h"

//==============================================================================
class SettingsContent final : public Component
{
public:
    SettingsContent (MainComponent& topLevelComponent)
        : innerContent (topLevelComponent)
    {
        settingsViewport.setViewedComponent (&innerContent, false);
        addAndMakeVisible (settingsViewport);

        setFocusContainerType (FocusContainerType::focusContainer);
        setTitle ("DemoRunner Settings");

        setOpaque (true);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (ResizableWindow::backgroundColorId));
    }

    z0 resized() override
    {
        constexpr i32 minimumWidth  = 350;
        constexpr i32 minimumHeight = 550;

        auto r = getLocalBounds();
        const auto scrollBarWidth = getLookAndFeel().getDefaultScrollbarWidth();

        innerContent.setSize (jmax (r.getWidth() - scrollBarWidth, minimumWidth),
                              jmax (r.getHeight(), minimumHeight));

        settingsViewport.setBounds (r);
    }

private:
    static constexpr f32 titleLabelFontHeight = 18.0f;
    static constexpr i32 itemHeight = 30;
    static constexpr i32 itemSpacing = 7;

    class GraphicsSettingsGroup final : public Component,
                                        private ComponentMovementWatcher
    {
    public:
        GraphicsSettingsGroup (MainComponent& comp)
            : ComponentMovementWatcher (&comp),
              mainComponent (comp)
        {
            addAndMakeVisible (titleLabel);
            titleLabel.setFont (FontOptions { titleLabelFontHeight });

            addLookAndFeels();

            addAndMakeVisible (lookAndFeelSelector);

            for (i32 i = 0; i < lookAndFeelNames.size(); ++i)
                lookAndFeelSelector.addItem (lookAndFeelNames.getReference (i), i + 1);

            lookAndFeelSelector.setSelectedItemIndex (lookAndFeelNames.indexOf ("LookAndFeel_V4 (Dark)"));

            lookAndFeelSelector.onChange = [this]
            {
                auto* lf = lookAndFeels.getUnchecked (lookAndFeelSelector.getSelectedItemIndex());
                Desktop::getInstance().setDefaultLookAndFeel (lf);
            };

            addAndMakeVisible (lookAndFeelLabel);
            lookAndFeelLabel.setJustificationType (Justification::centredRight);
            lookAndFeelLabel.attachToComponent (&lookAndFeelSelector, true);

            addAndMakeVisible (rendererSelector);
            rendererSelector.onChange = [this] { mainComponent.setRenderingEngine (rendererSelector.getSelectedItemIndex()); };

            addAndMakeVisible (rendererLabel);
            rendererLabel.setJustificationType (Justification::centredRight);
            rendererLabel.attachToComponent (&rendererSelector, true);

            setFocusContainerType (FocusContainerType::focusContainer);
            setTitle ("Graphics Settings");
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds();

            titleLabel.setBounds (bounds.removeFromTop (itemHeight));
            bounds.removeFromTop (itemSpacing);

            const auto xPos  = roundToInt ((f32) bounds.getX() + ((f32) bounds.getWidth() * 0.35f));
            const auto width = roundToInt ((f32) bounds.getWidth() * 0.6f);

            lookAndFeelSelector.setBounds (bounds.removeFromTop (itemHeight).withWidth (width).withX (xPos));
            bounds.removeFromTop (itemSpacing);

            rendererSelector.setBounds (bounds.removeFromTop (itemHeight).withWidth (width).withX (xPos));
        }

    private:
        z0 componentMovedOrResized (b8, b8) override    {}
        using ComponentListener::componentMovedOrResized;

        z0 componentVisibilityChanged() override            {}
        using ComponentListener::componentVisibilityChanged;

        z0 componentPeerChanged() override
        {
            auto* newPeer = mainComponent.getPeer();

            if (peer != newPeer)
            {
                peer = newPeer;

                if (peer != nullptr)
                    refreshRenderingEngineSelector();
            }
        }

        z0 refreshRenderingEngineSelector()
        {
            rendererSelector.clear (NotificationType::dontSendNotification);

            rendererSelector.addItemList (mainComponent.getRenderingEngines(), 1);
            rendererSelector.setSelectedItemIndex (mainComponent.getCurrentRenderingEngine());
        }

        z0 addLookAndFeels()
        {
            lookAndFeelNames.addArray ({ "LookAndFeel_V1", "LookAndFeel_V2", "LookAndFeel_V3",
                                         "LookAndFeel_V4 (Dark)", "LookAndFeel_V4 (Midnight)",
                                         "LookAndFeel_V4 (Grey)", "LookAndFeel_V4 (Light)" });

            lookAndFeels.add (new LookAndFeel_V1());
            lookAndFeels.add (new LookAndFeel_V2());
            lookAndFeels.add (new LookAndFeel_V3());
            lookAndFeels.add (new LookAndFeel_V4 (LookAndFeel_V4::getDarkColorScheme()));
            lookAndFeels.add (new LookAndFeel_V4 (LookAndFeel_V4::getMidnightColorScheme()));
            lookAndFeels.add (new LookAndFeel_V4 (LookAndFeel_V4::getGreyColorScheme()));
            lookAndFeels.add (new LookAndFeel_V4 (LookAndFeel_V4::getLightColorScheme()));
        }

        MainComponent& mainComponent;
        ComponentPeer* peer = nullptr;

        Label titleLabel       { {}, "Graphics" },
              lookAndFeelLabel { {}, "LookAndFeel:" },
              rendererLabel    { {}, "Renderer:" };

        ComboBox lookAndFeelSelector, rendererSelector;
        StringArray lookAndFeelNames;
        OwnedArray<LookAndFeel> lookAndFeels;
    };

    class AudioSettingsGroup final : public Component
    {
    public:
        AudioSettingsGroup()
            : deviceSelectorComp (getSharedAudioDeviceManager(), 0, 256, 0, 256, true, true, true, false)
        {
            addAndMakeVisible (titleLabel);
            titleLabel.setFont (FontOptions { titleLabelFontHeight });

            addAndMakeVisible (deviceSelectorComp);
            deviceSelectorComp.setItemHeight (itemHeight);

            setFocusContainerType (FocusContainerType::focusContainer);
            setTitle ("Audio Settings");
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds();

            titleLabel.setBounds (bounds.removeFromTop (itemHeight));
            bounds.removeFromTop (itemSpacing);

            deviceSelectorComp.setBounds (bounds);
        }

    private:
        Label titleLabel { {}, "Audio" };
        AudioDeviceSelectorComponent deviceSelectorComp;
    };

    //==============================================================================
    class InnerContent final : public Component
    {
    public:
        InnerContent (MainComponent& mainComponent)
            : graphicsSettings (mainComponent)
        {
            addAndMakeVisible (graphicsSettings);
            addAndMakeVisible (audioSettings);

            setOpaque (true);
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (findColor (ResizableWindow::backgroundColorId).contrasting (0.2f));
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds();

            graphicsSettings.setBounds (bounds.removeFromTop (150));
            audioSettings.setBounds (bounds);
        }

        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            return createIgnoredAccessibilityHandler (*this);
        }

    private:
        GraphicsSettingsGroup graphicsSettings;
        AudioSettingsGroup audioSettings;
    };

    Viewport settingsViewport;
    InnerContent innerContent;
};
