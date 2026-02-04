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

#include <map>
#include "jucer_AppearanceSettings.h"

//==============================================================================
class StoredSettings final : private ValueTree::Listener
{
public:
    StoredSettings();
    ~StoredSettings() override;

    PropertiesFile& getGlobalProperties();
    PropertiesFile& getProjectProperties (const Txt& projectUID);

    z0 flush();
    z0 reload();

    //==============================================================================
    RecentlyOpenedFilesList recentFiles;

    Array<File> getLastProjects();
    z0 setLastProjects (const Array<File>& files);

    //==============================================================================
    Array<Color> swatchColors;

    struct ColorSelectorWithSwatches final : public ColorSelector
    {
        ColorSelectorWithSwatches();
        ~ColorSelectorWithSwatches() override;

        i32 getNumSwatches() const override;
        Color getSwatchColor (i32 index) const override;
        z0 setSwatchColor (i32 index, const Color& newColor) override;
    };

    //==============================================================================
    z0 addProjectDefaultsListener (ValueTree::Listener&);
    z0 removeProjectDefaultsListener (ValueTree::Listener&);

    z0 addFallbackPathsListener (ValueTree::Listener&);
    z0 removeFallbackPathsListener (ValueTree::Listener&);

    ValueTreePropertyWithDefault getStoredPath (const Identifier& key, DependencyPathOS os);
    b8 isDRXPathIncorrect();

    //==============================================================================
    AppearanceSettings appearance;
    StringArray monospacedFontNames;
    File lastWizardFolder;

private:
    //==============================================================================
    z0 updateGlobalPreferences();
    z0 updateRecentFiles();
    z0 updateLastWizardFolder();
    z0 updateKeyMappings();

    z0 loadSwatchColors();
    z0 saveSwatchColors();

    z0 updateOldProjectSettingsFiles();
    z0 checkDRXPaths();

    //==============================================================================
    z0 changed (b8);

    z0 valueTreePropertyChanged (ValueTree& vt, const Identifier&) override  { changed (vt == projectDefaults); }
    z0 valueTreeChildAdded (ValueTree& vt, ValueTree&) override              { changed (vt == projectDefaults); }
    z0 valueTreeChildRemoved (ValueTree& vt, ValueTree&, i32) override       { changed (vt == projectDefaults); }
    z0 valueTreeChildOrderChanged (ValueTree& vt, i32, i32) override         { changed (vt == projectDefaults); }
    z0 valueTreeParentChanged (ValueTree& vt) override                       { changed (vt == projectDefaults); }

    //==============================================================================
    OwnedArray<PropertiesFile> propertyFiles;
    ValueTree projectDefaults;
    ValueTree fallbackPaths;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StoredSettings)
};

StoredSettings& getAppSettings();
PropertiesFile& getGlobalProperties();
