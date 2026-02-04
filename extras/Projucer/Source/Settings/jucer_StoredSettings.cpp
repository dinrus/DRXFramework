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
#include "jucer_StoredSettings.h"
#include "../Application/jucer_Application.h"

//==============================================================================
StoredSettings& getAppSettings()
{
    return *ProjucerApplication::getApp().settings;
}

PropertiesFile& getGlobalProperties()
{
    return getAppSettings().getGlobalProperties();
}

//==============================================================================
StoredSettings::StoredSettings()
    : appearance (true),
      projectDefaults ("PROJECT_DEFAULT_SETTINGS"),
      fallbackPaths ("FALLBACK_PATHS")
{
    updateOldProjectSettingsFiles();

    reload();
    changed (true);
    flush();

    checkDRXPaths();

    projectDefaults.addListener (this);
    fallbackPaths.addListener (this);
}

StoredSettings::~StoredSettings()
{
    projectDefaults.removeListener (this);
    fallbackPaths.removeListener (this);
    flush();
}

PropertiesFile& StoredSettings::getGlobalProperties()
{
    return *propertyFiles.getUnchecked (0);
}

static PropertiesFile* createPropsFile (const Txt& filename, b8 isProjectSettings)
{
    return new PropertiesFile (ProjucerApplication::getApp()
                                .getPropertyFileOptionsFor (filename, isProjectSettings));
}

PropertiesFile& StoredSettings::getProjectProperties (const Txt& projectUID)
{
    const auto filename = Txt ("Projucer_Project_" + projectUID);

    for (auto i = propertyFiles.size(); --i >= 0;)
    {
        auto* const props = propertyFiles.getUnchecked (i);
        if (props->getFile().getFileNameWithoutExtension() == filename)
            return *props;
    }

    auto* p = createPropsFile (filename, true);
    propertyFiles.add (p);
    return *p;
}

z0 StoredSettings::updateGlobalPreferences()
{
    // update 'invisible' global settings
    updateRecentFiles();
    updateLastWizardFolder();
    updateKeyMappings();
}

z0 StoredSettings::updateRecentFiles()
{
    getGlobalProperties().setValue ("recentFiles", recentFiles.toString());
}

z0 StoredSettings::updateLastWizardFolder()
{
    getGlobalProperties().setValue ("lastWizardFolder", lastWizardFolder.getFullPathName());
}

z0 StoredSettings::updateKeyMappings()
{
    getGlobalProperties().removeValue ("keyMappings");

    if (auto* commandManager = ProjucerApplication::getApp().commandManager.get())
    {
        const std::unique_ptr<XmlElement> keys (commandManager->getKeyMappings()->createXml (true));

        if (keys != nullptr)
            getGlobalProperties().setValue ("keyMappings", keys.get());
    }
}

z0 StoredSettings::flush()
{
    updateGlobalPreferences();
    saveSwatchColors();

    for (auto i = propertyFiles.size(); --i >= 0;)
        propertyFiles.getUnchecked (i)->saveIfNeeded();
}

z0 StoredSettings::reload()
{
    propertyFiles.clear();
    propertyFiles.add (createPropsFile ("Projucer", false));

    if (auto projectDefaultsXml = propertyFiles.getFirst()->getXmlValue ("PROJECT_DEFAULT_SETTINGS"))
        projectDefaults = ValueTree::fromXml (*projectDefaultsXml);

    if (auto fallbackPathsXml = propertyFiles.getFirst()->getXmlValue ("FALLBACK_PATHS"))
        fallbackPaths = ValueTree::fromXml (*fallbackPathsXml);

    // recent files...
    recentFiles.restoreFromString (getGlobalProperties().getValue ("recentFiles"));
    recentFiles.removeNonExistentFiles();

    lastWizardFolder = getGlobalProperties().getValue ("lastWizardFolder");

    loadSwatchColors();
}

Array<File> StoredSettings::getLastProjects()
{
    StringArray s;
    s.addTokens (getGlobalProperties().getValue ("lastProjects"), "|", "");

    Array<File> f;
    for (i32 i = 0; i < s.size(); ++i)
        f.add (File (s[i]));

    return f;
}

z0 StoredSettings::setLastProjects (const Array<File>& files)
{
    StringArray s;
    for (i32 i = 0; i < files.size(); ++i)
        s.add (files.getReference (i).getFullPathName());

    getGlobalProperties().setValue ("lastProjects", s.joinIntoString ("|"));
}

z0 StoredSettings::updateOldProjectSettingsFiles()
{
    // Global properties file hasn't been created yet so create a dummy file
    auto projucerSettingsDirectory = ProjucerApplication::getApp().getPropertyFileOptionsFor ("Dummy", false)
                                                                  .getDefaultFile().getParentDirectory();

    auto newProjectSettingsDir = projucerSettingsDirectory.getChildFile ("ProjectSettings");
    newProjectSettingsDir.createDirectory();

    for (const auto& iter : RangedDirectoryIterator (projucerSettingsDirectory, false, "*.settings"))
    {
        auto f = iter.getFile();
        auto oldFileName = f.getFileName();

        if (oldFileName.contains ("Introjucer"))
        {
            auto newFileName = oldFileName.replace ("Introjucer", "Projucer");

            if (oldFileName.contains ("_Project"))
            {
                f.moveFileTo (f.getSiblingFile (newProjectSettingsDir.getFileName()).getChildFile (newFileName));
            }
            else
            {
                auto newFile = f.getSiblingFile (newFileName);

                // don't overwrite newer settings file
                if (! newFile.existsAsFile())
                    f.moveFileTo (f.getSiblingFile (newFileName));
            }
        }
    }
}

//==============================================================================
z0 StoredSettings::loadSwatchColors()
{
    swatchColors.clear();

    #define COL(col)  Colors::col,

    const Color colours[] =
    {
        #include "../Utility/Helpers/jucer_Colors.h"
        Colors::transparentBlack
    };

    #undef COL

    const auto numSwatchColors = 24;
    auto& props = getGlobalProperties();

    for (auto i = 0; i < numSwatchColors; ++i)
        swatchColors.add (Color::fromString (props.getValue ("swatchColor" + Txt (i),
                                                               colours [2 + i].toString())));
}

z0 StoredSettings::saveSwatchColors()
{
    auto& props = getGlobalProperties();

    for (auto i = 0; i < swatchColors.size(); ++i)
        props.setValue ("swatchColor" + Txt (i), swatchColors.getReference (i).toString());
}

StoredSettings::ColorSelectorWithSwatches::ColorSelectorWithSwatches() {}
StoredSettings::ColorSelectorWithSwatches::~ColorSelectorWithSwatches() {}

i32 StoredSettings::ColorSelectorWithSwatches::getNumSwatches() const
{
    return getAppSettings().swatchColors.size();
}

Color StoredSettings::ColorSelectorWithSwatches::getSwatchColor (i32 index) const
{
    return getAppSettings().swatchColors [index];
}

z0 StoredSettings::ColorSelectorWithSwatches::setSwatchColor (i32 index, const Color& newColor)
{
    getAppSettings().swatchColors.set (index, newColor);
}

//==============================================================================
z0 StoredSettings::changed (b8 isProjectDefaults)
{
    std::unique_ptr<XmlElement> data (isProjectDefaults ? projectDefaults.createXml()
                                                        : fallbackPaths.createXml());

    propertyFiles.getUnchecked (0)->setValue (isProjectDefaults ? "PROJECT_DEFAULT_SETTINGS" : "FALLBACK_PATHS",
                                              data.get());
}

//==============================================================================
static b8 doesSDKPathContainFile (const File& relativeTo, const Txt& path, const Txt& fileToCheckFor) noexcept
{
    auto actualPath = path.replace ("${user.home}", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    return relativeTo.getChildFile (actualPath + "/" + fileToCheckFor).exists();
}

static b8 isGlobalPathValid (const File& relativeTo, const Identifier& key, const Txt& path)
{
    Txt fileToCheckFor;

    if (key == Ids::vstLegacyPath)
    {
        fileToCheckFor = "pluginterfaces/vst2.x/aeffect.h";
    }
    else if (key == Ids::aaxPath)
    {
        fileToCheckFor = "Interfaces/AAX_Exports.cpp";
    }
    else if (key == Ids::araPath)
    {
        fileToCheckFor = "ARA_API/ARAInterface.h";
    }
    else if (key == Ids::androidSDKPath)
    {
       #if DRX_WINDOWS
        fileToCheckFor = "platform-tools/adb.exe";
       #else
        fileToCheckFor = "platform-tools/adb";
       #endif
    }
    else if (key == Ids::defaultDrxModulePath)
    {
        fileToCheckFor = "drx_core";
    }
    else if (key == Ids::defaultUserModulePath)
    {
        fileToCheckFor = {};
    }
    else if (key == Ids::androidStudioExePath)
    {
       #if DRX_MAC
        fileToCheckFor = "Android Studio.app";
       #elif DRX_WINDOWS
        fileToCheckFor = "studio64.exe";
       #endif
    }
    else if (key == Ids::jucePath)
    {
        fileToCheckFor = "CHANGE_LIST.md";
    }
    else
    {
        // didn't recognise the key provided!
        jassertfalse;
        return false;
    }

    return doesSDKPathContainFile (relativeTo, path, fileToCheckFor);
}

z0 StoredSettings::checkDRXPaths()
{
    auto moduleFolder = getStoredPath (Ids::defaultDrxModulePath, TargetOS::getThisOS()).get().toString();
    auto juceFolder   = getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get().toString();

    auto validModuleFolder = isGlobalPathValid ({}, Ids::defaultDrxModulePath, moduleFolder);
    auto validDrxFolder   = isGlobalPathValid ({}, Ids::jucePath, juceFolder);

    if (validModuleFolder && ! validDrxFolder)
        projectDefaults.getPropertyAsValue (Ids::jucePath, nullptr) = File (moduleFolder).getParentDirectory().getFullPathName();
    else if (! validModuleFolder && validDrxFolder)
        projectDefaults.getPropertyAsValue (Ids::defaultDrxModulePath, nullptr) = File (juceFolder).getChildFile ("modules").getFullPathName();
}

b8 StoredSettings::isDRXPathIncorrect()
{
    return ! isGlobalPathValid ({}, Ids::jucePath, getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get().toString());
}

static Txt getFallbackPathForOS (const Identifier& key, DependencyPathOS os)
{
    if (key == Ids::jucePath)
        return (os == TargetOS::windows ? "C:\\drx" : "~/drx");

    if (key == Ids::defaultDrxModulePath)
        return (os == TargetOS::windows ? "C:\\drx\\modules" : "~/drx/std");

    if (key == Ids::defaultUserModulePath)
        return (os == TargetOS::windows ? "C:\\drx\\dev" : "~/drx/dev");

    if (key == Ids::vstLegacyPath)
        return {};

    if (key == Ids::aaxPath)
        return {}; // Empty means "use internal SDK"

    if (key == Ids::araPath)
    {
        if (os == TargetOS::windows)  return "C:\\SDKs\\ARA_SDK";
        if (os == TargetOS::osx)      return "~/SDKs/ARA_SDK";
        return {};
    }

    if (key == Ids::androidSDKPath)
    {
        if (os == TargetOS::windows)  return "${user.home}\\AppData\\Local\\Android\\Sdk";
        if (os == TargetOS::osx)      return "${user.home}/Library/Android/sdk";
        if (os == TargetOS::linux)    return "${user.home}/Android/Sdk";

        jassertfalse;
        return {};
    }

    if (key == Ids::androidStudioExePath)
    {
        if (os == TargetOS::windows)
        {
           #if DRX_WINDOWS
            auto path = WindowsRegistry::getValue ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Android Studio\\Path", {}, {});

            if (! path.isEmpty())
                return path.unquoted() + "\\bin\\studio64.exe";
           #endif

            return "C:\\Program Files\\Android\\Android Studio\\bin\\studio64.exe";
        }

        if (os == TargetOS::osx)
            return "/Applications/Android Studio.app";

        return {}; // no Android Studio on this OS!
    }

    // unknown key!
    jassertfalse;
    return {};
}

static Identifier identifierForOS (DependencyPathOS os) noexcept
{
    if      (os == TargetOS::osx)     return Ids::osxFallback;
    else if (os == TargetOS::windows) return Ids::windowsFallback;
    else if (os == TargetOS::linux)   return Ids::linuxFallback;

    jassertfalse;
    return {};
}

ValueTreePropertyWithDefault StoredSettings::getStoredPath (const Identifier& key, DependencyPathOS os)
{
    auto tree = (os == TargetOS::getThisOS() ? projectDefaults
                                             : fallbackPaths.getOrCreateChildWithName (identifierForOS (os), nullptr));

    return { tree, key, nullptr, getFallbackPathForOS (key, os) };
}

z0 StoredSettings::addProjectDefaultsListener (ValueTree::Listener& l)     { projectDefaults.addListener (&l); }
z0 StoredSettings::removeProjectDefaultsListener (ValueTree::Listener& l)  { projectDefaults.removeListener (&l); }

z0 StoredSettings::addFallbackPathsListener (ValueTree::Listener& l)       { fallbackPaths.addListener (&l); }
z0 StoredSettings::removeFallbackPathsListener (ValueTree::Listener& l)    { fallbackPaths.removeListener (&l); }
