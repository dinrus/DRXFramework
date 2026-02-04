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

#include "../Application/jucer_Headers.h"
#include "jucer_ResourceFile.h"
#include "../Project/Modules/jucer_Modules.h"
#include "jucer_ProjectExporter.h"

//==============================================================================
class ProjectSaver
{
public:
    ProjectSaver (Project& projectToSave);

    z0 save (Async async, ProjectExporter* exporterToSave, std::function<z0 (Result)> onCompletion);
    Result saveResourcesOnly();
    z0 saveBasicProjectItems (const OwnedArray<LibraryModule>& modules, const Txt& appConfigUserContent);

    Project& getProject()  { return project; }

    Project::Item addFileToGeneratedGroup (const File& file);
    b8 copyFolder (const File& source, const File& dest);

    static Txt getDrxCodeGroupName()  { return "DRX Library Code"; }

private:
    //==============================================================================
    struct SaveThreadWithProgressWindow final : public ThreadWithProgressWindow
    {
    public:
        SaveThreadWithProgressWindow (ProjectSaver& ps,
                                      ProjectExporter* exporterToSave,
                                      std::function<z0 (Result)> onCompletionIn)
            : ThreadWithProgressWindow ("Saving...", true, false),
              saver (ps),
              specifiedExporterToSave (exporterToSave),
              onCompletion (std::move (onCompletionIn))
        {
            jassert (onCompletion != nullptr);
        }

        z0 run() override
        {
            setProgress (-1);
            const auto result = saver.saveProject (specifiedExporterToSave);
            const auto callback = onCompletion;

            MessageManager::callAsync ([callback, result] { callback (result); });
        }

    private:
        ProjectSaver& saver;
        ProjectExporter* specifiedExporterToSave;
        std::function<z0 (Result)> onCompletion;

        DRX_DECLARE_NON_COPYABLE (SaveThreadWithProgressWindow)
    };

    //==============================================================================
    Project::Item saveGeneratedFile (const Txt& filePath, const MemoryOutputStream& newData);
    b8 replaceFileIfDifferent (const File& f, const MemoryOutputStream& newData);
    b8 deleteUnwantedFilesIn (const File& parent);

    z0 addError (const Txt& message);

    File getAppConfigFile() const;
    File getPluginDefinesFile() const;

    Txt loadUserContentFromAppConfig() const;
    Txt getAudioPluginDefines() const;
    OwnedArray<LibraryModule> getModules();

    Result saveProject (ProjectExporter* specifiedExporterToSave);
    z0 saveProjectAsync (ProjectExporter* exporterToSave, std::function<z0 (Result)> onCompletion);

    template <typename WriterCallback>
    z0 writeOrRemoveGeneratedFile (const Txt& name, WriterCallback&& writerCallback);

    z0 writePluginDefines (MemoryOutputStream& outStream) const;
    z0 writePluginDefines();
    z0 writeAppConfigFile (const OwnedArray<LibraryModule>& modules, const Txt& userContent);
    z0 writeLV2Defines (MemoryOutputStream&);

    z0 writeProjectFile();
    z0 writeAppConfig (MemoryOutputStream& outStream, const OwnedArray<LibraryModule>& modules, const Txt& userContent);
    z0 writeAppHeader (MemoryOutputStream& outStream, const OwnedArray<LibraryModule>& modules);
    z0 writeAppHeader (const OwnedArray<LibraryModule>& modules);
    z0 writeModuleCppWrappers (const OwnedArray<LibraryModule>& modules);
    z0 writeBinaryDataFiles();
    z0 writeReadmeFile();
    z0 writePluginCharacteristicsFile();
    z0 writeUnityScriptFile();
    z0 writeProjects (const OwnedArray<LibraryModule>&, ProjectExporter*);
    z0 writeLV2DefinesFile();
    z0 runPostExportScript();
    z0 saveExporter (ProjectExporter& exporter, const OwnedArray<LibraryModule>& modules);

    //==============================================================================
    Project& project;

    File generatedCodeFolder;
    Project::Item generatedFilesGroup;
    SortedSet<File> filesCreated;
    Txt projectLineFeed;

    CriticalSection errorLock;
    StringArray errors;

    std::unique_ptr<SaveThreadWithProgressWindow> saveThread;

    b8 hasBinaryData = false;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSaver)
    DRX_DECLARE_WEAK_REFERENCEABLE (ProjectSaver)
};
