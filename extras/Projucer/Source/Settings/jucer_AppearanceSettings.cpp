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

#include "../Application/jucer_Headers.h"
#include "../Application/jucer_Application.h"
#include "jucer_AppearanceSettings.h"

//==============================================================================
AppearanceSettings::AppearanceSettings (b8 updateAppWhenChanged)
    : settings ("COLOUR_SCHEME")
{
    CodeDocument doc;
    CPlusPlusCodeTokeniser tokeniser;
    CodeEditorComponent editor (doc, &tokeniser);

    CodeEditorComponent::ColorScheme cs (editor.getColorScheme());

    for (i32 i = cs.types.size(); --i >= 0;)
    {
        auto& t = cs.types.getReference (i);
        getColorValue (t.name) = t.colour.toString();
    }

    getCodeFontValue() = getDefaultCodeFont().toString();

    if (updateAppWhenChanged)
        settings.addListener (this);
}

File AppearanceSettings::getSchemesFolder()
{
    File f (getGlobalProperties().getFile().getSiblingFile ("Schemes"));
    f.createDirectory();
    return f;
}

z0 AppearanceSettings::writeDefaultSchemeFile (const Txt& xmlString, const Txt& name)
{
    auto file = getSchemesFolder().getChildFile (name).withFileExtension (getSchemeFileSuffix());

    AppearanceSettings settings (false);

    if (auto xml = parseXML (xmlString))
        settings.readFromXML (*xml);

    settings.writeToFile (file);
}

z0 AppearanceSettings::refreshPresetSchemeList()
{
    writeDefaultSchemeFile (BinaryData::colourscheme_dark_xml,  "Default (Dark)");
    writeDefaultSchemeFile (BinaryData::colourscheme_light_xml, "Default (Light)");

    auto newSchemes = getSchemesFolder().findChildFiles (File::findFiles, false, Txt ("*") + getSchemeFileSuffix());

    if (newSchemes != presetSchemeFiles)
    {
        presetSchemeFiles.swapWith (newSchemes);
        ProjucerApplication::getCommandManager().commandStatusChanged();
    }
}

StringArray AppearanceSettings::getPresetSchemes()
{
    StringArray s;
    for (i32 i = 0; i < presetSchemeFiles.size(); ++i)
        s.add (presetSchemeFiles.getReference (i).getFileNameWithoutExtension());

    return s;
}

z0 AppearanceSettings::selectPresetScheme (i32 index)
{
    readFromFile (presetSchemeFiles [index]);
}

b8 AppearanceSettings::readFromXML (const XmlElement& xml)
{
    if (xml.hasTagName (settings.getType().toString()))
    {
        const ValueTree newSettings (ValueTree::fromXml (xml));

        // we'll manually copy across the new properties to the existing tree so that
        // any open editors will be kept up to date..
        settings.copyPropertiesFrom (newSettings, nullptr);

        for (i32 i = settings.getNumChildren(); --i >= 0;)
        {
            ValueTree c (settings.getChild (i));

            const ValueTree newValue (newSettings.getChildWithProperty (Ids::name, c.getProperty (Ids::name)));

            if (newValue.isValid())
                c.copyPropertiesFrom (newValue, nullptr);
        }

        return true;
    }

    return false;
}

b8 AppearanceSettings::readFromFile (const File& file)
{
    if (auto xml = parseXML (file))
        return readFromXML (*xml);

    return false;
}

b8 AppearanceSettings::writeToFile (const File& file) const
{
    if (auto xml = settings.createXml())
        return xml->writeTo (file, {});

    return false;
}

Font AppearanceSettings::getDefaultCodeFont()
{
    return FontOptions (Font::getDefaultMonospacedFontName(), Font::getDefaultStyle(), 13.0f);
}

StringArray AppearanceSettings::getColorNames() const
{
    StringArray s;

    for (auto c : settings)
        if (c.hasType ("COLOUR"))
            s.add (c[Ids::name]);

    return s;
}

z0 AppearanceSettings::updateColorScheme()
{
    ProjucerApplication::getApp().mainWindowList.sendLookAndFeelChange();
}

z0 AppearanceSettings::applyToCodeEditor (CodeEditorComponent& editor) const
{
    CodeEditorComponent::ColorScheme cs (editor.getColorScheme());

    for (i32 i = cs.types.size(); --i >= 0;)
    {
        CodeEditorComponent::ColorScheme::TokenType& t = cs.types.getReference (i);
        getColor (t.name, t.colour);
    }

    editor.setColorScheme (cs);
    editor.setFont (getCodeFont());

    editor.setColor (ScrollBar::thumbColorId, editor.findColor (CodeEditorComponent::backgroundColorId)
                                                      .contrasting()
                                                      .withAlpha (0.13f));
}

Font AppearanceSettings::getCodeFont() const
{
    const Txt fontString (settings [Ids::font].toString());

    if (fontString.isEmpty())
        return getDefaultCodeFont();

    return Font::fromString (fontString);
}

Value AppearanceSettings::getCodeFontValue()
{
    return settings.getPropertyAsValue (Ids::font, nullptr);
}

Value AppearanceSettings::getColorValue (const Txt& colourName)
{
    ValueTree c (settings.getChildWithProperty (Ids::name, colourName));

    if (! c.isValid())
    {
        c = ValueTree ("COLOUR");
        c.setProperty (Ids::name, colourName, nullptr);
        settings.appendChild (c, nullptr);
    }

    return c.getPropertyAsValue (Ids::colour, nullptr);
}

b8 AppearanceSettings::getColor (const Txt& name, Color& result) const
{
    const ValueTree colour (settings.getChildWithProperty (Ids::name, name));

    if (colour.isValid())
    {
        result = Color::fromString (colour [Ids::colour].toString());
        return true;
    }

    return false;
}
