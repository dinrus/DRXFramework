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
class AppearanceSettings final : private ValueTree::Listener
{
public:
    AppearanceSettings (b8 updateAppWhenChanged);

    b8 readFromFile (const File& file);
    b8 readFromXML (const XmlElement&);
    b8 writeToFile (const File& file) const;

    z0 updateColorScheme();
    z0 applyToCodeEditor (CodeEditorComponent& editor) const;

    StringArray getColorNames() const;
    Value getColorValue (const Txt& colourName);
    b8 getColor (const Txt& name, Color& resultIfFound) const;

    Font getCodeFont() const;
    Value getCodeFontValue();

    ValueTree settings;

    static File getSchemesFolder();
    StringArray getPresetSchemes();
    z0 refreshPresetSchemeList();
    z0 selectPresetScheme (i32 index);

    static Font getDefaultCodeFont();

    static tukk getSchemeFileSuffix()      { return ".scheme"; }
    static tukk getSchemeFileWildCard()    { return "*.scheme"; }

private:

    Array<File> presetSchemeFiles;

    static z0 writeDefaultSchemeFile (const Txt& xml, const Txt& name);

    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override   { updateColorScheme(); }
    z0 valueTreeChildAdded (ValueTree&, ValueTree&) override               { updateColorScheme(); }
    z0 valueTreeChildRemoved (ValueTree&, ValueTree&, i32) override        { updateColorScheme(); }
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32) override          { updateColorScheme(); }
    z0 valueTreeParentChanged (ValueTree&) override                        { updateColorScheme(); }
    z0 valueTreeRedirected (ValueTree&) override                           { updateColorScheme(); }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppearanceSettings)
};
