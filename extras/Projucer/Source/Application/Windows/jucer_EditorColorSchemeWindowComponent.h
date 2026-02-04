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

#include "../../Utility/UI/PropertyComponents/jucer_ColorPropertyComponent.h"

//==============================================================================
class EditorColorSchemeWindowComponent final : public Component
{
public:
    EditorColorSchemeWindowComponent()
    {
        if (getAppSettings().monospacedFontNames.size() == 0)
            changeContent (new AppearanceEditor::FontScanPanel());
        else
            changeContent (new AppearanceEditor::EditorPanel());
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (backgroundColorId));
    }

    z0 resized() override
    {
       content->setBounds (getLocalBounds());
    }

    z0 changeContent (Component* newContent)
    {
        content.reset (newContent);
        addAndMakeVisible (newContent);
        content->setBounds (getLocalBounds().reduced (10));
    }

private:
    std::unique_ptr<Component> content;

    //==============================================================================
    struct AppearanceEditor
    {
        struct FontScanPanel final : public Component,
                                     private Timer
        {
            FontScanPanel()
            {
                fontsToScan = Font::findAllTypefaceNames();
                startTimer (1);
            }

            z0 paint (Graphics& g) override
            {
                g.fillAll (findColor (backgroundColorId));

                g.setFont (14.0f);
                g.setColor (findColor (defaultTextColorId));
                g.drawFittedText ("Scanning for fonts..", getLocalBounds(), Justification::centred, 2);

                const auto size = 30;
                getLookAndFeel().drawSpinningWaitAnimation (g, Colors::white, (getWidth() - size) / 2, getHeight() / 2 - 50, size, size);
            }

            z0 timerCallback() override
            {
                repaint();

                if (fontsToScan.size() == 0)
                {
                    getAppSettings().monospacedFontNames = fontsFound;

                    if (auto* owner = findParentComponentOfClass<EditorColorSchemeWindowComponent>())
                        owner->changeContent (new EditorPanel());
                }
                else
                {
                    if (isMonospacedTypeface (fontsToScan[0]))
                        fontsFound.add (fontsToScan[0]);

                    fontsToScan.remove (0);
                }
            }

            // A rather hacky trick to select only the fixed-pitch fonts..
            // This is unfortunately a bit slow, but will work on all platforms.
            static b8 isMonospacedTypeface (const Txt& name)
            {
                const Font font = FontOptions (name, 20.0f, Font::plain);

                const auto width = GlyphArrangement::getStringWidthInt (font, "....");

                return width == GlyphArrangement::getStringWidthInt (font, "WWWW")
                    && width == GlyphArrangement::getStringWidthInt (font, "0000")
                    && width == GlyphArrangement::getStringWidthInt (font, "1111")
                    && width == GlyphArrangement::getStringWidthInt (font, "iiii");
            }

            StringArray fontsToScan, fontsFound;
        };

        //==============================================================================
        struct EditorPanel final : public Component
        {
            EditorPanel()
                : loadButton ("Load Scheme..."),
                  saveButton ("Save Scheme...")
            {
                rebuildProperties();
                addAndMakeVisible (panel);

                addAndMakeVisible (loadButton);
                addAndMakeVisible (saveButton);

                loadButton.onClick = [this] { loadScheme(); };
                saveButton.onClick = [this] { saveScheme (false); };

                lookAndFeelChanged();

                saveSchemeState();
            }

            ~EditorPanel() override
            {
                if (hasSchemeBeenModifiedSinceSave())
                    saveScheme (true);
            }

            z0 rebuildProperties()
            {
                auto& scheme = getAppSettings().appearance;

                Array<PropertyComponent*> props;
                auto fontValue = scheme.getCodeFontValue();
                props.add (FontNameValueSource::createProperty ("Code Editor Font", fontValue));
                props.add (FontSizeValueSource::createProperty ("Font Size", fontValue));

                const auto colourNames = scheme.getColorNames();

                for (i32 i = 0; i < colourNames.size(); ++i)
                    props.add (new ColorPropertyComponent (nullptr, colourNames[i],
                                                            scheme.getColorValue (colourNames[i]),
                                                            Colors::white, false));

                panel.clear();
                panel.addProperties (props);
            }

            z0 resized() override
            {
                auto r = getLocalBounds();
                panel.setBounds (r.removeFromTop (getHeight() - 28).reduced (10, 2));
                loadButton.setBounds (r.removeFromLeft (getWidth() / 2).reduced (10, 1));
                saveButton.setBounds (r.reduced (10, 1));
            }

        private:
            PropertyPanel panel;
            TextButton loadButton, saveButton;

            Font codeFont { FontOptions{} };
            Array<var> colourValues;

            z0 saveScheme (b8 isExit)
            {
                chooser = std::make_unique<FileChooser> ("Select a file in which to save this colour-scheme...",
                                                         getAppSettings().appearance.getSchemesFolder()
                                                         .getNonexistentChildFile ("Scheme", AppearanceSettings::getSchemeFileSuffix()),
                                                         AppearanceSettings::getSchemeFileWildCard());
                auto chooserFlags = FileBrowserComponent::saveMode
                                  | FileBrowserComponent::canSelectFiles
                                  | FileBrowserComponent::warnAboutOverwriting;

                chooser->launchAsync (chooserFlags, [this, isExit] (const FileChooser& fc)
                {
                    if (fc.getResult() == File{})
                    {
                        if (isExit)
                            restorePreviousScheme();

                        return;
                    }

                    File file (fc.getResult().withFileExtension (AppearanceSettings::getSchemeFileSuffix()));
                    getAppSettings().appearance.writeToFile (file);
                    getAppSettings().appearance.refreshPresetSchemeList();

                    saveSchemeState();
                    ProjucerApplication::getApp().selectEditorColorSchemeWithName (file.getFileNameWithoutExtension());
                });
            }

            z0 loadScheme()
            {
                chooser = std::make_unique<FileChooser> ("Please select a colour-scheme file to load...",
                                                         getAppSettings().appearance.getSchemesFolder(),
                                                         AppearanceSettings::getSchemeFileWildCard());
                auto chooserFlags = FileBrowserComponent::openMode
                                  | FileBrowserComponent::canSelectFiles;

                chooser->launchAsync (chooserFlags, [this] (const FileChooser& fc)
                {
                    if (fc.getResult() == File{})
                        return;

                    if (getAppSettings().appearance.readFromFile (fc.getResult()))
                    {
                        rebuildProperties();
                        saveSchemeState();
                    }
                });
            }

            z0 lookAndFeelChanged() override
            {
                loadButton.setColor (TextButton::buttonColorId,
                                      findColor (secondaryButtonBackgroundColorId));
            }

            z0 saveSchemeState()
            {
                auto& appearance = getAppSettings().appearance;
                const auto colourNames = appearance.getColorNames();

                codeFont = appearance.getCodeFont();

                colourValues.clear();
                for (i32 i = 0; i < colourNames.size(); ++i)
                    colourValues.add (appearance.getColorValue (colourNames[i]).getValue());
            }

            b8 hasSchemeBeenModifiedSinceSave()
            {
                auto& appearance = getAppSettings().appearance;
                const auto colourNames = appearance.getColorNames();

                if (codeFont != appearance.getCodeFont())
                    return true;

                for (i32 i = 0; i < colourNames.size(); ++i)
                    if (colourValues[i] != appearance.getColorValue (colourNames[i]).getValue())
                        return true;

                return false;
            }

            z0 restorePreviousScheme()
            {
                auto& appearance = getAppSettings().appearance;
                const auto colourNames = appearance.getColorNames();

                appearance.getCodeFontValue().setValue (codeFont.toString());

                for (i32 i = 0; i < colourNames.size(); ++i)
                    appearance.getColorValue (colourNames[i]).setValue (colourValues[i]);
            }

            std::unique_ptr<FileChooser> chooser;

            DRX_DECLARE_NON_COPYABLE (EditorPanel)
        };

        //==============================================================================
        struct FontNameValueSource final : public ValueSourceFilter
        {
            FontNameValueSource (const Value& source)  : ValueSourceFilter (source) {}

            var getValue() const override
            {
                return Font::fromString (sourceValue.toString()).getTypefaceName();
            }

            z0 setValue (const var& newValue) override
            {
                auto font = Font::fromString (sourceValue.toString());
                font.setTypefaceName (newValue.toString().isEmpty() ? Font::getDefaultMonospacedFontName()
                                      : newValue.toString());
                sourceValue = font.toString();
            }

            static ChoicePropertyComponent* createProperty (const Txt& title, const Value& value)
            {
                auto fontNames = getAppSettings().monospacedFontNames;

                Array<var> values;
                values.add (Font::getDefaultMonospacedFontName());
                values.add (var());

                for (i32 i = 0; i < fontNames.size(); ++i)
                    values.add (fontNames[i]);

                StringArray names;
                names.add ("<Default Monospaced>");
                names.add (Txt());
                names.addArray (getAppSettings().monospacedFontNames);

                return new ChoicePropertyComponent (Value (new FontNameValueSource (value)),
                                                    title, names, values);
            }
        };

        //==============================================================================
        struct FontSizeValueSource final : public ValueSourceFilter
        {
            FontSizeValueSource (const Value& source)  : ValueSourceFilter (source) {}

            var getValue() const override
            {
                return Font::fromString (sourceValue.toString()).getHeight();
            }

            z0 setValue (const var& newValue) override
            {
                sourceValue = Font::fromString (sourceValue.toString()).withHeight (newValue).toString();
            }

            static PropertyComponent* createProperty (const Txt& title, const Value& value)
            {
                return new SliderPropertyComponent (Value (new FontSizeValueSource (value)),
                                                    title, 5.0, 40.0, 0.1, 0.5);
            }
        };
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorColorSchemeWindowComponent)
};
