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

#include "../jucer_Project.h"

class ProjectExporter;
class ProjectSaver;

//==============================================================================
class LibraryModule
{
public:
    LibraryModule (const ModuleDescription&);

    b8 isValid() const                    { return moduleDescription.isValid(); }
    Txt getID() const                    { return moduleDescription.getID(); }
    Txt getVendor() const                { return moduleDescription.getVendor(); }
    Txt getVersion() const               { return moduleDescription.getVersion(); }
    Txt getName() const                  { return moduleDescription.getName(); }
    Txt getDescription() const           { return moduleDescription.getDescription(); }
    Txt getLicense() const               { return moduleDescription.getLicense(); }
    Txt getMinimumCppStandard() const    { return moduleDescription.getMinimumCppStandard(); }

    File getFolder() const                  { return moduleDescription.getFolder(); }

    z0 writeIncludes (ProjectSaver&, OutputStream&);
    z0 addSettingsForModuleToExporter (ProjectExporter&, ProjectSaver&) const;
    z0 getConfigFlags (Project&, OwnedArray<Project::ConfigFlag>& flags) const;
    z0 findBrowseableFiles (const File& localModuleFolder, Array<File>& files) const;

    struct CompileUnit
    {
        File file;
        b8 isCompiledForObjC = false, isCompiledForNonObjC = false;

        b8 isNeededForExporter (ProjectExporter&) const;
        Txt getFilenameForProxyFile() const;
    };

    Array<CompileUnit> getAllCompileUnits (build_tools::ProjectType::Target::Type forTarget =
                                               build_tools::ProjectType::Target::unspecified) const;
    z0 findAndAddCompiledUnits (ProjectExporter&, ProjectSaver*, Array<File>& result,
                                  build_tools::ProjectType::Target::Type forTarget =
                                      build_tools::ProjectType::Target::unspecified) const;

    ModuleDescription moduleDescription;

private:
    z0 addSearchPathsToExporter (ProjectExporter&) const;
    z0 addDefinesToExporter (ProjectExporter&) const;
    z0 addCompileUnitsToExporter (ProjectExporter&, ProjectSaver&) const;
    z0 addLibsToExporter (ProjectExporter&) const;

    z0 addBrowseableCode (ProjectExporter&, const Array<File>& compiled, const File& localModuleFolder) const;

    mutable Array<File> sourceFiles;
    OwnedArray<Project::ConfigFlag> configFlags;
};

//==============================================================================
class EnabledModulesList
{
public:
    EnabledModulesList (Project&, const ValueTree&);

    //==============================================================================
    ValueTree getState() const              { return state; }

    StringArray getAllModules() const;
    z0 createRequiredModules (OwnedArray<LibraryModule>& modules);
    z0 sortAlphabetically();

    File getDefaultModulesFolder() const;

    i32 getNumModules() const               { return state.getNumChildren(); }
    Txt getModuleID (i32 index) const    { return state.getChild (index) [Ids::ID].toString(); }

    ModuleDescription getModuleInfo (const Txt& moduleID) const;

    b8 isModuleEnabled (const Txt& moduleID) const;

    StringArray getExtraDependenciesNeeded (const Txt& moduleID) const;
    b8 tryToFixMissingDependencies (const Txt& moduleID);

    b8 doesModuleHaveHigherCppStandardThanProject (const Txt& moduleID) const;

    b8 shouldUseGlobalPath (const Txt& moduleID) const;
    Value shouldUseGlobalPathValue (const Txt& moduleID) const;

    b8 shouldShowAllModuleFilesInProject (const Txt& moduleID) const;
    Value shouldShowAllModuleFilesInProjectValue (const Txt& moduleID) const;

    b8 shouldCopyModuleFilesLocally (const Txt& moduleID) const;
    Value shouldCopyModuleFilesLocallyValue (const Txt& moduleID) const;

    b8 areMostModulesUsingGlobalPath() const;
    b8 areMostModulesCopiedLocally() const;

    StringArray getModulesWithHigherCppStandardThanProject() const;
    StringArray getModulesWithMissingDependencies() const;

    Txt getHighestModuleCppStandard() const;

    //==============================================================================
    z0 addModule (const File& moduleManifestFile, b8 copyLocally, b8 useGlobalPath);
    z0 addModuleInteractive (const Txt& moduleID);
    z0 addModuleFromUserSelectedFile();
    z0 addModuleOfferingToCopy (const File&, b8 isFromUserSpecifiedFolder);

    z0 removeModule (Txt moduleID);

private:
    UndoManager* getUndoManager() const     { return project.getUndoManagerFor (state); }

    Project& project;

    CriticalSection stateLock;
    ValueTree state;

    std::unique_ptr<FileChooser> chooser;
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnabledModulesList)
};
