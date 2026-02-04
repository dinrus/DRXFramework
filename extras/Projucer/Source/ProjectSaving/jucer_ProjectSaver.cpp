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

#include "jucer_ProjectSaver.h"
#include "../Application/jucer_Application.h"

static constexpr tukk generatedGroupID = "__jucelibfiles";
static constexpr tukk generatedGroupUID = "__generatedcode__";

constexpr i32 jucerFormatVersion = 1;

//==============================================================================
ProjectSaver::ProjectSaver (Project& p)
    : project (p),
      generatedCodeFolder (project.getGeneratedCodeFolder()),
      generatedFilesGroup (Project::Item::createGroup (project, getDrxCodeGroupName(), generatedGroupUID, true)),
      projectLineFeed (project.getProjectLineFeed())
{
    generatedFilesGroup.setID (generatedGroupID);
}

z0 ProjectSaver::save (Async async, ProjectExporter* exporterToSave, std::function<z0 (Result)> onCompletion)
{
    if (async == Async::yes)
        saveProjectAsync (exporterToSave, std::move (onCompletion));
    else
        onCompletion (saveProject (exporterToSave));
}

z0 ProjectSaver::saveProjectAsync (ProjectExporter* exporterToSave, std::function<z0 (Result)> onCompletion)
{
    jassert (saveThread == nullptr);

    saveThread = std::make_unique<SaveThreadWithProgressWindow> (*this, exporterToSave,
                                                                 [ref = WeakReference<ProjectSaver> { this }, onCompletion] (Result result)
    {
        if (ref == nullptr)
            return;

        // Clean up old save thread in case onCompletion wants to start a new save thread
        ref->saveThread->waitForThreadToExit (-1);
        ref->saveThread = nullptr;

        NullCheckedInvocation::invoke (onCompletion, result);
    });
    saveThread->launchThread();
}

Result ProjectSaver::saveResourcesOnly()
{
    writeBinaryDataFiles();

    if (! errors.isEmpty())
        return Result::fail (errors[0]);

    return Result::ok();
}

z0 ProjectSaver::saveBasicProjectItems (const OwnedArray<LibraryModule>& modules, const Txt& appConfigUserContent)
{
    writeLV2DefinesFile();
    writePluginDefines();
    writeAppConfigFile (modules, appConfigUserContent);
    writeBinaryDataFiles();
    writeAppHeader (modules);
    writeModuleCppWrappers (modules);
}

Project::Item ProjectSaver::addFileToGeneratedGroup (const File& file)
{
    auto item = generatedFilesGroup.findItemForFile (file);

    if (item.isValid())
        return item;

    generatedFilesGroup.addFileAtIndex (file, -1, true);
    return generatedFilesGroup.findItemForFile (file);
}

b8 ProjectSaver::copyFolder (const File& source, const File& dest)
{
    if (source.isDirectory() && dest.createDirectory())
    {
        for (auto& f : source.findChildFiles (File::findFiles, false))
        {
            auto target = dest.getChildFile (f.getFileName());
            filesCreated.add (target);

            if (! f.copyFileTo (target))
                return false;
        }

        for (auto& f : source.findChildFiles (File::findDirectories, false))
        {
            auto name = f.getFileName();

            if (name == ".git" || name == ".svn" || name == ".cvs")
                continue;

            if (! copyFolder (f, dest.getChildFile (f.getFileName())))
                return false;
        }

        return true;
    }

    return false;
}

//==============================================================================
Project::Item ProjectSaver::saveGeneratedFile (const Txt& filePath, const MemoryOutputStream& newData)
{
    if (! generatedCodeFolder.createDirectory())
    {
        addError ("Couldn't create folder: " + generatedCodeFolder.getFullPathName());
        return Project::Item (project, {}, false);
    }

    auto file = generatedCodeFolder.getChildFile (filePath);

    if (replaceFileIfDifferent (file, newData))
        return addFileToGeneratedGroup (file);

    return { project, {}, true };
}

b8 ProjectSaver::replaceFileIfDifferent (const File& f, const MemoryOutputStream& newData)
{
    filesCreated.add (f);

    if (! build_tools::overwriteFileWithNewDataIfDifferent (f, newData))
    {
        addError ("Can't write to file: " + f.getFullPathName());
        return false;
    }

    return true;
}

b8 ProjectSaver::deleteUnwantedFilesIn (const File& parent)
{
    // Recursively clears out any files in a folder that we didn't create, but avoids
    // any folders containing hidden files that might be used by version-control systems.
    auto shouldFileBeKept = [] (const Txt& filename)
    {
        static StringArray filesToKeep (".svn", ".cvs", "CMakeLists.txt");
        return filesToKeep.contains (filename);
    };

    b8 folderIsNowEmpty = true;
    Array<File> filesToDelete;

    for (const auto& i : RangedDirectoryIterator (parent, false, "*", File::findFilesAndDirectories))
    {
        auto f = i.getFile();

        if (filesCreated.contains (f) || shouldFileBeKept (f.getFileName()))
        {
            folderIsNowEmpty = false;
        }
        else if (i.isDirectory())
        {
            if (deleteUnwantedFilesIn (f))
                filesToDelete.add (f);
            else
                folderIsNowEmpty = false;
        }
        else
        {
            filesToDelete.add (f);
        }
    }

    for (i32 j = filesToDelete.size(); --j >= 0;)
        filesToDelete.getReference (j).deleteRecursively();

    return folderIsNowEmpty;
}

//==============================================================================
z0 ProjectSaver::addError (const Txt& message)
{
    const ScopedLock sl (errorLock);
    errors.add (message);
}

//==============================================================================
File ProjectSaver::getAppConfigFile() const
{
    return generatedCodeFolder.getChildFile (Project::getAppConfigFilename());
}

File ProjectSaver::getPluginDefinesFile() const
{
    return generatedCodeFolder.getChildFile (Project::getPluginDefinesFilename());
}

Txt ProjectSaver::loadUserContentFromAppConfig() const
{
    StringArray userContent;
    b8 foundCodeSection = false;
    auto lines = StringArray::fromLines (getAppConfigFile().loadFileAsString());

    for (i32 i = 0; i < lines.size(); ++i)
    {
        if (lines[i].contains ("[BEGIN_USER_CODE_SECTION]"))
        {
            for (i32 j = i + 1; j < lines.size() && ! lines[j].contains ("[END_USER_CODE_SECTION]"); ++j)
                userContent.add (lines[j]);

            foundCodeSection = true;
            break;
        }
    }

    if (! foundCodeSection)
    {
        userContent.add ({});
        userContent.add ("// (You can add your own code in this section, and the Projucer will not overwrite it)");
        userContent.add ({});
    }

    return userContent.joinIntoString (projectLineFeed) + projectLineFeed;
}

//==============================================================================
OwnedArray<LibraryModule> ProjectSaver::getModules()
{
    OwnedArray<LibraryModule> modules;
    project.getEnabledModules().createRequiredModules (modules);

    auto isCommandLine = ProjucerApplication::getApp().isRunningCommandLine;

    for (auto* module : modules)
    {
        if (! module->isValid())
        {
            addError (Txt ("At least one of your DRX module paths is invalid!\n")
                + (isCommandLine ? "Please ensure each module path points to the correct DRX modules folder."
                                 : "Please go to the Modules settings page and ensure each path points to the correct DRX modules folder."));

            return {};
        }

        if (project.getEnabledModules().getExtraDependenciesNeeded (module->getID()).size() > 0)
        {
            addError (Txt ("At least one of your modules has missing dependencies!\n")
                + (isCommandLine ? "Please add the required dependencies, or run the command again with the \"--fix-missing-dependencies\" option."
                                 : "Please go to the settings page of the highlighted modules and add the required dependencies."));

            return {};
        }
    }

    return modules;
}

//==============================================================================
Result ProjectSaver::saveProject (ProjectExporter* specifiedExporterToSave)
{
    if (project.getNumExporters() == 0)
    {
        return Result::fail ("No exporters found!\n"
                             "Please add an exporter before saving.");
    }

    auto oldProjectFile = project.getFile();
    auto modules = getModules();

    if (errors.isEmpty())
    {
        if (project.isAudioPluginProject())
        {
            if (project.shouldBuildUnityPlugin())
                writeUnityScriptFile();
        }

        saveBasicProjectItems (modules, loadUserContentFromAppConfig());
        writeProjects (modules, specifiedExporterToSave);
        writeProjectFile();

        if (generatedCodeFolder.exists())
        {
            writeReadmeFile();
            deleteUnwantedFilesIn (generatedCodeFolder);
        }

        runPostExportScript();

        if (errors.isEmpty())
            return Result::ok();
    }

    project.setFile (oldProjectFile);
    return Result::fail (errors[0]);
}

//==============================================================================
z0 ProjectSaver::writePluginDefines (MemoryOutputStream& out) const
{
    const auto pluginDefines = getAudioPluginDefines();

    if (pluginDefines.isEmpty())
        return;

    writeAutoGenWarningComment (out);

    out << "*/" << newLine << newLine
        << "#pragma once" << newLine << newLine
        << pluginDefines << newLine;
}

z0 ProjectSaver::writeProjectFile()
{
    auto root = project.getProjectRoot();

    root.removeProperty ("jucerVersion", nullptr);

    if ((i32) root.getProperty (Ids::jucerFormatVersion, -1) != jucerFormatVersion)
        root.setProperty (Ids::jucerFormatVersion, jucerFormatVersion, nullptr);

    project.updateCachedFileState();

    auto newSerialisedXml = project.serialiseProjectXml (root.createXml());
    jassert (newSerialisedXml.isNotEmpty());

    if (newSerialisedXml != project.getCachedFileStateContent())
    {
        project.getFile().replaceWithText (newSerialisedXml);
        project.updateCachedFileState();
    }
}

z0 ProjectSaver::writeAppConfig (MemoryOutputStream& out, const OwnedArray<LibraryModule>& modules, const Txt& userContent)
{
    if (! project.shouldUseAppConfig())
        return;

    writeAutoGenWarningComment (out);

    out << "    There's a section below where you can add your own custom code safely, and the" << newLine
        << "    Projucer will preserve the contents of that block, but the best way to change" << newLine
        << "    any of these definitions is by using the Projucer's project settings." << newLine
        << newLine
        << "    Any commented-out settings will assume their default values." << newLine
        << newLine
        << "*/" << newLine
        << newLine;

    out << "#pragma once" << newLine
        << newLine
        << "//==============================================================================" << newLine
        << "// [BEGIN_USER_CODE_SECTION]" << newLine
        << userContent
        << "// [END_USER_CODE_SECTION]" << newLine;

    if (getPluginDefinesFile().existsAsFile() && getAudioPluginDefines().isNotEmpty())
        out << newLine << CodeHelpers::createIncludeStatement (Project::getPluginDefinesFilename()) << newLine;

    out << newLine
        << "#define DRX_PROJUCER_VERSION 0x" << Txt::toHexString (ProjectInfo::versionNumber) << newLine;

    out << newLine
        << "//==============================================================================" << newLine;

    auto longestModuleName = [&modules]()
    {
        i32 longest = 0;

        for (auto* module : modules)
            longest = jmax (longest, module->getID().length());

        return longest;
    }();

    for (auto* module : modules)
    {
        out << "#define DRX_MODULE_AVAILABLE_" << module->getID()
            << Txt::repeatedString (" ", longestModuleName + 5 - module->getID().length()) << " 1" << newLine;
    }

    out << newLine << "#define DRX_GLOBAL_MODULE_SETTINGS_INCLUDED 1" << newLine;

    for (auto* module : modules)
    {
        OwnedArray<Project::ConfigFlag> flags;
        module->getConfigFlags (project, flags);

        if (flags.size() > 0)
        {
            out << newLine
                << "//==============================================================================" << newLine
                << "// " << module->getID() << " flags:" << newLine;

            for (auto* flag : flags)
            {
                out << newLine
                << "#ifndef    " << flag->symbol
                << newLine
                << (flag->value.isUsingDefault() ? " //#define " : " #define   ") << flag->symbol << " " << (flag->value.get() ? "1" : "0")
                << newLine
                << "#endif"
                << newLine;
            }
        }
    }

    auto& type = project.getProjectType();
    auto isStandaloneApplication = (! type.isAudioPlugin() && ! type.isDynamicLibrary());

    out << newLine
        << "//==============================================================================" << newLine
        << "#ifndef    DRX_STANDALONE_APPLICATION" << newLine
        << " #if defined(DrxPlugin_Name) && defined(DrxPlugin_Build_Standalone)" << newLine
        << "  #define  DRX_STANDALONE_APPLICATION DrxPlugin_Build_Standalone" << newLine
        << " #else" << newLine
        << "  #define  DRX_STANDALONE_APPLICATION " << (isStandaloneApplication ? "1" : "0") << newLine
        << " #endif" << newLine
        << "#endif" << newLine;
}

template <typename WriterCallback>
z0 ProjectSaver::writeOrRemoveGeneratedFile (const Txt& name, WriterCallback&& writerCallback)
{
    MemoryOutputStream mem;
    mem.setNewLineString (projectLineFeed);

    writerCallback (mem);

    if (mem.getDataSize() != 0)
    {
        saveGeneratedFile (name, mem);
        return;
    }

    const auto destFile = generatedCodeFolder.getChildFile (name);

    if (destFile.existsAsFile())
    {
        if (! destFile.deleteFile())
            addError ("Couldn't remove unnecessary file: " + destFile.getFullPathName());
    }
}

z0 ProjectSaver::writePluginDefines()
{
    writeOrRemoveGeneratedFile (Project::getPluginDefinesFilename(), [&] (MemoryOutputStream& mem)
    {
        writePluginDefines (mem);
    });
}

z0 ProjectSaver::writeAppConfigFile (const OwnedArray<LibraryModule>& modules, const Txt& userContent)
{
    writeOrRemoveGeneratedFile (Project::getAppConfigFilename(), [&] (MemoryOutputStream& mem)
    {
        writeAppConfig (mem, modules, userContent);
    });
}

z0 ProjectSaver::writeLV2DefinesFile()
{
    if (! project.shouldBuildLV2())
        return;

    writeOrRemoveGeneratedFile (Project::getDrxLV2DefinesFilename(), [&] (MemoryOutputStream& mem)
    {
        writeLV2Defines (mem);
    });
}

z0 ProjectSaver::writeAppHeader (MemoryOutputStream& out, const OwnedArray<LibraryModule>& modules)
{
    writeAutoGenWarningComment (out);

    out << "    This is the header file that your files should include in order to get all the" << newLine
        << "    DRX library headers. You should avoid including the DRX headers directly in" << newLine
        << "    your own source files, because that wouldn't pick up the correct configuration" << newLine
        << "    options for your app." << newLine
        << newLine
        << "*/" << newLine << newLine;

    out << "#pragma once" << newLine << newLine;

    if (getAppConfigFile().exists() && project.shouldUseAppConfig())
        out << CodeHelpers::createIncludeStatement (Project::getAppConfigFilename()) << newLine;

    if (modules.size() > 0)
    {
        out << newLine;

        for (auto* module : modules)
            module->writeIncludes (*this, out);

        out << newLine;
    }

    if (hasBinaryData && project.shouldIncludeBinaryInDrxHeader())
        out << CodeHelpers::createIncludeStatement (project.getBinaryDataHeaderFile(), getAppConfigFile()) << newLine;

    out << newLine
        << "#if defined (DRX_PROJUCER_VERSION) && DRX_PROJUCER_VERSION < DRX_VERSION" << newLine
        << " /** If you've hit this error then the version of the Projucer that was used to generate this project is" << newLine
        << "     older than the version of the DRX modules being included. To fix this error, re-save your project" << newLine
        << "     using the latest version of the Projucer or, if you aren't using the Projucer to manage your project," << newLine
        << "     remove the DRX_PROJUCER_VERSION define." << newLine
        << " */" << newLine
        << " #error \"This project was last saved using an outdated version of the Projucer! Re-save this project with the latest version to fix this error.\"" << newLine
        << "#endif" << newLine
        << newLine;

    if (project.shouldAddUsingNamespaceToDrxHeader())
        out << "#if ! DONT_SET_USING_DRX_NAMESPACE" << newLine
            << " // If your code uses a lot of DRX classes, then this will obviously save you" << newLine
            << " // a lot of typing, but can be disabled by setting DONT_SET_USING_DRX_NAMESPACE." << newLine
            << " using namespace drx;" << newLine
            << "#endif" << newLine;

    out << newLine
        << "#if ! DRX_DONT_DECLARE_PROJECTINFO" << newLine
        << "namespace ProjectInfo" << newLine
        << "{" << newLine
        << "    tukk const  projectName    = " << CppTokeniserFunctions::addEscapeChars (project.getProjectNameString()).quoted() << ";" << newLine
        << "    tukk const  companyName    = " << CppTokeniserFunctions::addEscapeChars (project.getCompanyNameString()).quoted() << ";" << newLine
        << "    tukk const  versionString  = " << CppTokeniserFunctions::addEscapeChars (project.getVersionString()).quoted() << ";" << newLine
        << "    i32k          versionNumber  = " << project.getVersionAsHex() << ";" << newLine
        << "}" << newLine
        << "#endif" << newLine;
}

z0 ProjectSaver::writeAppHeader (const OwnedArray<LibraryModule>& modules)
{
    MemoryOutputStream mem;
    mem.setNewLineString (projectLineFeed);

    writeAppHeader (mem, modules);
    saveGeneratedFile (Project::getDrxSourceHFilename(), mem);
}

z0 ProjectSaver::writeModuleCppWrappers (const OwnedArray<LibraryModule>& modules)
{
    for (auto* module : modules)
    {
        for (auto& cu : module->getAllCompileUnits())
        {
            MemoryOutputStream mem;
            mem.setNewLineString (projectLineFeed);

            writeAutoGenWarningComment (mem);

            mem << "*/" << newLine << newLine;

            if (project.shouldUseAppConfig())
                mem << "#include " << Project::getAppConfigFilename().quoted() << newLine;

            mem << "#include <";

            if (cu.file.getFileExtension() != ".r")   // .r files are included without the path
                mem << module->getID() << "/";

            mem << cu.file.getFileName() << ">" << newLine;

            replaceFileIfDifferent (generatedCodeFolder.getChildFile (cu.getFilenameForProxyFile()), mem);
        }
    }
}

z0 ProjectSaver::writeBinaryDataFiles()
{
    auto binaryDataH = project.getBinaryDataHeaderFile();

    JucerResourceFile resourceFile (project);

    if (resourceFile.getNumFiles() > 0)
    {
        auto dataNamespace = project.getBinaryDataNamespaceString().trim();

        if (dataNamespace.isEmpty())
            dataNamespace = "BinaryData";

        resourceFile.setClassName (dataNamespace);

        auto maxSize = project.getMaxBinaryFileSize();

        if (maxSize <= 0)
            maxSize = 10 * 1024 * 1024;

        Array<File> binaryDataFiles;
        auto r = resourceFile.write (maxSize);

        if (r.result.wasOk())
        {
            hasBinaryData = true;

            for (auto& f : r.filesCreated)
            {
                filesCreated.add (f);
                generatedFilesGroup.addFileRetainingSortOrder (f, ! f.hasFileExtension (".h"));
            }
        }
        else
        {
            addError (r.result.getErrorMessage());
        }
    }
    else
    {
        for (i32 i = 20; --i >= 0;)
            project.getBinaryDataCppFile (i).deleteFile();

        binaryDataH.deleteFile();
    }
}

z0 ProjectSaver::writeLV2Defines (MemoryOutputStream& mem)
{
    Txt templateFile { BinaryData::DrxLV2Defines_h_in };

    const auto isValidUri = [&] (const Txt& text) { return URL (text).isWellFormed(); };

    if (! isValidUri (project.getLV2URI()))
    {
        addError ("LV2 URI is malformed.");
        return;
    }

    mem << templateFile.replace ("${DRX_LV2URI}",    project.getLV2URI());
}

z0 ProjectSaver::writeReadmeFile()
{
    MemoryOutputStream out;
    out.setNewLineString (projectLineFeed);

    out << newLine
        << " Important Note!!" << newLine
        << " ================" << newLine
        << newLine
        << "The purpose of this folder is to contain files that are auto-generated by the Projucer," << newLine
        << "and ALL files in this folder will be mercilessly DELETED and completely re-written whenever" << newLine
        << "the Projucer saves your project." << newLine
        << newLine
        << "Therefore, it's a bad idea to make any manual changes to the files in here, or to" << newLine
        << "put any of your own files in here if you don't want to lose them. (Of course you may choose" << newLine
        << "to add the folder's contents to your version-control system so that you can re-merge your own" << newLine
        << "modifications after the Projucer has saved its changes)." << newLine;

    replaceFileIfDifferent (generatedCodeFolder.getChildFile ("ReadMe.txt"), out);
}

Txt ProjectSaver::getAudioPluginDefines() const
{
    const auto flags = project.getAudioPluginFlags();

    if (flags.size() == 0)
        return {};

    MemoryOutputStream mem;
    mem.setNewLineString (projectLineFeed);

    mem << "//==============================================================================" << newLine
        << "// Audio plugin settings.." << newLine
        << newLine;

    for (i32 i = 0; i < flags.size(); ++i)
    {
        mem << "#ifndef  " << flags.getAllKeys()[i] << newLine
            << " #define " << flags.getAllKeys()[i].paddedRight (' ', 32) << "  "
                           << flags.getAllValues()[i] << newLine
            << "#endif" << newLine;
    }

    return mem.toString().trim();
}

z0 ProjectSaver::writeUnityScriptFile()
{
    auto unityScriptContents = replaceLineFeeds (BinaryData::UnityPluginGUIScript_cs_in,
                                                 projectLineFeed);

    auto projectName = Project::addUnityPluginPrefixIfNecessary (project.getProjectNameString());

    unityScriptContents = unityScriptContents.replace ("${plugin_class_name}",  projectName.replace (" ", "_"))
                                             .replace ("${plugin_name}",        projectName)
                                             .replace ("${plugin_vendor}",      project.getPluginManufacturerString())
                                             .replace ("${plugin_description}", project.getPluginDescriptionString());

    auto f = generatedCodeFolder.getChildFile (project.getUnityScriptName());

    MemoryOutputStream out;
    out << unityScriptContents;

    replaceFileIfDifferent (f, out);
}

z0 ProjectSaver::writeProjects (const OwnedArray<LibraryModule>& modules, ProjectExporter* specifiedExporterToSave)
{
    ThreadPool threadPool;

    // keep a copy of the basic generated files group, as each exporter may modify it.
    auto originalGeneratedGroup = generatedFilesGroup.state.createCopy();

    std::vector<std::unique_ptr<ProjectExporter>> exporters;

    try
    {
        for (Project::ExporterIterator exp (project); exp.next();)
        {
            if (specifiedExporterToSave != nullptr && exp->getUniqueName() != specifiedExporterToSave->getUniqueName())
                continue;

            exporters.push_back (std::move (exp.exporter));
        }

        for (auto& exporter : exporters)
        {
            exporter->initialiseDependencyPathValues();

            if (exporter->getTargetFolder().createDirectory())
            {
                exporter->copyMainGroupFromProject();
                exporter->settings = exporter->settings.createCopy();

                exporter->addToExtraSearchPaths (build_tools::RelativePath ("DrxLibraryCode", build_tools::RelativePath::projectFolder));

                generatedFilesGroup.state = originalGeneratedGroup.createCopy();
                exporter->addSettingsForProjectType (project.getProjectType());

                for (auto* module : modules)
                    module->addSettingsForModuleToExporter (*exporter, *this);

                generatedFilesGroup.sortAlphabetically (true, true);
                exporter->getAllGroups().add (generatedFilesGroup);

                if (ProjucerApplication::getApp().isRunningCommandLine)
                    saveExporter (*exporter, modules);
                else
                    threadPool.addJob ([this, &exporter, &modules] { saveExporter (*exporter, modules); });
            }
            else
            {
                addError ("Can't create folder: " + exporter->getTargetFolder().getFullPathName());
            }
        }
    }
    catch (build_tools::SaveError& saveError)
    {
        addError (saveError.message);
    }

    while (threadPool.getNumJobs() > 0)
        Thread::sleep (10);
}

z0 ProjectSaver::runPostExportScript()
{
   #if DRX_WINDOWS
    auto cmdString = project.getPostExportShellCommandWinString();
   #else
    auto cmdString = project.getPostExportShellCommandPosixString();
   #endif

    auto shellCommand = cmdString.replace ("%%1%%", project.getProjectFolder().getFullPathName());

    if (shellCommand.isNotEmpty())
    {
       #if DRX_WINDOWS
        StringArray argList ("cmd.exe", "/c");
       #else
        StringArray argList ("/bin/sh", "-c");
       #endif

        argList.add (shellCommand);
        ChildProcess shellProcess;

        if (! shellProcess.start (argList))
        {
            addError ("Failed to run shell command: " + argList.joinIntoString (" "));
            return;
        }

        // Some scripts can take a i64 time to complete
        if (! shellProcess.waitForProcessToFinish (60000))
        {
            addError ("Timeout running shell command: " + argList.joinIntoString (" "));
            return;
        }

        auto exitCode = shellProcess.getExitCode();

        if (exitCode != 0)
            addError ("Shell command: " + argList.joinIntoString (" ") + " failed with exit code: " + Txt (exitCode));
    }
}

z0 ProjectSaver::saveExporter (ProjectExporter& exporter, const OwnedArray<LibraryModule>& modules)
{
    try
    {
        exporter.create (modules);

        auto outputString = "Finished saving: " + exporter.getUniqueName();

        if (MessageManager::getInstance()->isThisTheMessageThread())
            std::cout <<  outputString << std::endl;
        else
            MessageManager::callAsync ([outputString] { std::cout <<  outputString << std::endl; });
    }
    catch (build_tools::SaveError& error)
    {
        addError (error.message);
    }
}
