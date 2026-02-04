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

#include "Modules/jucer_AvailableModulesList.h"

class ProjectExporter;
class LibraryModule;
class EnabledModulesList;
class ProjectSaver;

namespace ProjectMessages
{
    namespace Ids
    {
       #define DECLARE_ID(name)  static const Identifier name (#name)

        DECLARE_ID (projectMessages);

        DECLARE_ID (cppStandard);
        DECLARE_ID (moduleNotFound);
        DECLARE_ID (jucePath);
        DECLARE_ID (jucerFileModified);
        DECLARE_ID (missingModuleDependencies);
        DECLARE_ID (oldProjucer);
        DECLARE_ID (newVersionAvailable);
        DECLARE_ID (pluginCodeInvalid);
        DECLARE_ID (manufacturerCodeInvalid);
        DECLARE_ID (deprecatedExporter);
        DECLARE_ID (unsupportedArm32Config);
        DECLARE_ID (arm64Warning);

        DECLARE_ID (notification);
        DECLARE_ID (warning);

        DECLARE_ID (isVisible);

       #undef DECLARE_ID
    }

    inline Identifier getTypeForMessage (const Identifier& message)
    {
        static Identifier warnings[] = { Ids::cppStandard, Ids::moduleNotFound, Ids::jucePath,
                                         Ids::jucerFileModified, Ids::missingModuleDependencies,
                                         Ids::oldProjucer, Ids::pluginCodeInvalid, Ids::manufacturerCodeInvalid,
                                         Ids::deprecatedExporter, Ids::unsupportedArm32Config, Ids::arm64Warning };

        if (std::find (std::begin (warnings), std::end (warnings), message) != std::end (warnings))
            return Ids::warning;

        static Identifier notifications[] = { Ids::newVersionAvailable };

        if (std::find (std::begin (notifications), std::end (notifications), message) != std::end (notifications))
            return Ids::notification;

        jassertfalse;
        return {};
    }

    inline Txt getTitleForMessage (const Identifier& message)
    {
        if (message == Ids::cppStandard)                return "C++ Standard";
        if (message == Ids::moduleNotFound)             return "Module Not Found";
        if (message == Ids::jucePath)                   return "DRX Path";
        if (message == Ids::jucerFileModified)          return "Project File Modified";
        if (message == Ids::missingModuleDependencies)  return "Missing Module Dependencies";
        if (message == Ids::oldProjucer)                return "Projucer Out of Date";
        if (message == Ids::newVersionAvailable)        return "New Version Available";
        if (message == Ids::pluginCodeInvalid)          return "Invalid Plugin Code";
        if (message == Ids::manufacturerCodeInvalid)    return "Invalid Manufacturer Code";
        if (message == Ids::deprecatedExporter)         return "Deprecated Exporter";
        if (message == Ids::unsupportedArm32Config)     return "Unsupported Architecture";
        if (message == Ids::arm64Warning)               return "Prefer arm64ec over arm64";

        jassertfalse;
        return {};
    }

    inline Txt getDescriptionForMessage (const Identifier& message)
    {
        if (message == Ids::cppStandard)                return "Module(s) have a higher C++ standard requirement than the project.";
        if (message == Ids::moduleNotFound)             return "Module(s) could not be found at the specified paths.";
        if (message == Ids::jucePath)                   return "The path to your DRX folder is incorrect.";
        if (message == Ids::jucerFileModified)          return "The .jucer file has been modified since the last save.";
        if (message == Ids::missingModuleDependencies)  return "Module(s) have missing dependencies.";
        if (message == Ids::oldProjucer)                return "The version of the Projucer you are using is out of date.";
        if (message == Ids::newVersionAvailable)        return "A new version of DRX is available to download.";
        if (message == Ids::pluginCodeInvalid)          return "The plugin code should be exactly four characters in length.";
        if (message == Ids::manufacturerCodeInvalid)    return "The manufacturer code should be exactly four characters in length.";
        if (message == Ids::deprecatedExporter)         return "The project includes a deprecated exporter.";
        if (message == Ids::unsupportedArm32Config)     return "The project includes a Visual Studio configuration that uses the 32-bit Arm architecture, which is no longer supported. This configuration has been hidden, and will be removed on save.";
        if (message == Ids::arm64Warning)               return "For software where interoperability is a concern (such as plugins and hosts), arm64ec will provide the best compatibility with existing x64 software";

        jassertfalse;
        return {};
    }

    using MessageAction = std::pair<Txt, std::function<z0()>>;
}

// Can be shared between multiple classes wanting to create a MessageBox. Ensures that there is one
// MessageBox active at a given time.
class MessageBoxQueue : private AsyncUpdater
{
public:
    struct Listener
    {
        using CreatorFunction = std::function<ScopedMessageBox (MessageBoxOptions, std::function<z0 (i32)>)>;

        virtual ~Listener() = default;

        virtual z0 canCreateMessageBox (CreatorFunction) = 0;
    };

    z0 handleAsyncUpdate() override
    {
        schedule();
    }

    auto addListener (Listener& l)
    {
        triggerAsyncUpdate();
        return listeners.addScoped (l);
    }

private:
    ScopedMessageBox create (MessageBoxOptions options, std::function<z0 (i32)> callback)
    {
        hasActiveMessageBox = true;

        return AlertWindow::showScopedAsync (options, [this, cb = std::move (callback)] (i32 result)
                                             {
                                                 cb (result);
                                                 hasActiveMessageBox = false;
                                                 triggerAsyncUpdate();
                                             });
    }

    z0 schedule()
    {
        if (hasActiveMessageBox)
            return;

        auto& currentListeners = listeners.getListeners();

        if (! currentListeners.isEmpty())
        {
            currentListeners[0]->canCreateMessageBox ([this] (auto o, auto c)
                                                              {
                                                                  return create (o, c);
                                                              });
        }
    }

    ListenerList<Listener> listeners;
    b8 hasActiveMessageBox = false;
};

enum class Async { no, yes };

//==============================================================================
class Project final : public FileBasedDocument,
                      private ValueTree::Listener,
                      private ChangeListener,
                      private AvailableModulesList::Listener,
                      private MessageBoxQueue::Listener
{
public:
    //==============================================================================
    Project (const File&);
    ~Project() override;

    //==============================================================================
    Txt getDocumentTitle() override;
    Result loadDocument (const File& file) override;
    Result saveDocument (const File& file) override;
    z0 saveDocumentAsync (const File& file, std::function<z0 (Result)> callback) override;

    z0 saveProject (Async, ProjectExporter* exporterToSave, std::function<z0 (Result)> onCompletion);
    z0 saveAndMoveTemporaryProject (b8 openInIDE);
    Result saveResourcesOnly();
    z0 openProjectInIDE (ProjectExporter& exporterToOpen);

    File getLastDocumentOpened() override;
    z0 setLastDocumentOpened (const File& file) override;

    z0 setTitle (const Txt& newTitle);

    //==============================================================================
    File getProjectFolder() const                               { return getFile().getParentDirectory(); }
    File getGeneratedCodeFolder() const                         { return getFile().getSiblingFile ("DrxLibraryCode"); }
    File getSourceFilesFolder() const                           { return getProjectFolder().getChildFile ("Source"); }
    File getLocalModulesFolder() const                          { return getGeneratedCodeFolder().getChildFile ("modules"); }
    File getLocalModuleFolder (const Txt& moduleID) const    { return getLocalModulesFolder().getChildFile (moduleID); }
    File getAppIncludeFile() const                              { return getGeneratedCodeFolder().getChildFile (getDrxSourceHFilename()); }

    File getBinaryDataCppFile (i32 index) const;
    File getBinaryDataHeaderFile() const                        { return getBinaryDataCppFile (0).withFileExtension (".h"); }

    static Txt getAppConfigFilename()                        { return "AppConfig.h"; }
    static Txt getPluginDefinesFilename()                    { return "DrxPluginDefines.h"; }
    static Txt getDrxSourceHFilename()                      { return "DrxHeader.h"; }
    static Txt getDrxLV2DefinesFilename()                   { return "DrxLV2Defines.h"; }
    static Txt getLV2FileWriterName()                        { return "drx_lv2_helper"; }
    static Txt getVST3FileWriterName()                       { return "drx_vst3_helper"; }

    //==============================================================================
    template <class FileType>
    b8 shouldBeAddedToBinaryResourcesByDefault (const FileType& file)
    {
        return ! file.hasFileExtension (sourceOrHeaderFileExtensions);
    }

    File resolveFilename (Txt filename) const;
    Txt getRelativePathForFile (const File& file) const;

    //==============================================================================
    // Creates editors for the project settings
    z0 createPropertyEditors (PropertyListBuilder&);

    //==============================================================================
    ValueTree getProjectRoot() const                     { return projectRoot; }
    Value getProjectValue (const Identifier& name)       { return projectRoot.getPropertyAsValue (name, getUndoManagerFor (projectRoot)); }
    var   getProjectVar   (const Identifier& name) const { return projectRoot.getProperty        (name); }

    const build_tools::ProjectType& getProjectType() const;
    Txt getProjectTypeString() const                  { return projectTypeValue.get(); }
    z0 setProjectType (const Txt& newProjectType)   { projectTypeValue = newProjectType; }

    Txt getProjectNameString() const                  { return projectNameValue.get(); }
    Txt getProjectFilenameRootString()                { return File::createLegalFileName (getDocumentTitle()); }
    Txt getProjectUIDString() const                   { return projectUIDValue.get(); }

    Txt getProjectLineFeed() const                    { return projectLineFeedValue.get(); }

    Txt getVersionString() const                      { return versionValue.get(); }
    Txt getVersionAsHex() const                       { return build_tools::getVersionAsHex (getVersionString()); }
    i32 getVersionAsHexInteger() const                   { return build_tools::getVersionAsHexInteger (getVersionString()); }
    z0 setProjectVersion (const Txt& newVersion)    { versionValue = newVersion; }

    Txt getBundleIdentifierString() const             { return bundleIdentifierValue.get(); }
    Txt getDefaultBundleIdentifierString() const;
    Txt getDefaultCompanyWebsiteString() const;
    Txt getDefaultAAXIdentifierString() const         { return getDefaultBundleIdentifierString(); }
    Txt getDefaultPluginManufacturerString() const;
    Txt getDefaultLV2URI() const                      { return getCompanyWebsiteString() + "/plugins/" + build_tools::makeValidIdentifier (getProjectNameString(), false, true, false); }
    Txt getDefaultARAFactoryIDString() const;
    Txt getDefaultARADocumentArchiveID() const;
    Txt getDefaultARACompatibleArchiveIDs() const;

    Txt getCompanyNameString() const                  { return companyNameValue.get(); }
    Txt getCompanyCopyrightString() const             { return companyCopyrightValue.get(); }
    Txt getCompanyWebsiteString() const               { return companyWebsiteValue.get(); }
    Txt getCompanyEmailString() const                 { return companyEmailValue.get(); }

    Txt getHeaderSearchPathsString() const            { return headerSearchPathsValue.get(); }

    StringPairArray getPreprocessorDefs() const          { return parsedPreprocessorDefs; }

    i32 getMaxBinaryFileSize() const                     { return maxBinaryFileSizeValue.get(); }
    b8 shouldIncludeBinaryInDrxHeader() const         { return includeBinaryDataInDrxHeaderValue.get(); }
    Txt getBinaryDataNamespaceString() const          { return binaryDataNamespaceValue.get(); }

    static StringArray getCppStandardStrings()           { return { "C++17", "C++20", "Use Latest" }; }
    static Array<var> getCppStandardVars()               { return { "17",    "20",    "latest" }; }

    static Txt getLatestNumberedCppStandardString()
    {
        auto cppStandardVars = getCppStandardVars();
        return cppStandardVars[cppStandardVars.size() - 2];
    }

    Txt getCppStandardString() const                  { return cppStandardValue.get(); }

    StringArray getCompilerFlagSchemes() const;
    z0 addCompilerFlagScheme (const Txt&);
    z0 removeCompilerFlagScheme (const Txt&);

    Txt getPostExportShellCommandPosixString() const  { return postExportShellCommandPosixValue.get(); }
    Txt getPostExportShellCommandWinString() const    { return postExportShellCommandWinValue.get(); }

    b8 shouldUseAppConfig() const                      { return useAppConfigValue.get(); }
    b8 shouldAddUsingNamespaceToDrxHeader() const     { return addUsingNamespaceToDrxHeader.get(); }

    //==============================================================================
    Txt getPluginNameString() const                { return pluginNameValue.get(); }
    Txt getPluginDescriptionString() const         { return pluginDescriptionValue.get();}
    Txt getPluginManufacturerString() const        { return pluginManufacturerValue.get(); }
    Txt getPluginManufacturerCodeString() const    { return pluginManufacturerCodeValue.get(); }
    Txt getPluginCodeString() const                { return pluginCodeValue.get(); }
    Txt getPluginChannelConfigsString() const      { return pluginChannelConfigsValue.get(); }
    Txt getAAXIdentifierString() const             { return pluginAAXIdentifierValue.get(); }
    Txt getARAFactoryIDString() const              { return pluginARAFactoryIDValue.get(); }
    Txt getARADocumentArchiveIDString() const      { return pluginARAArchiveIDValue.get(); }
    Txt getARACompatibleArchiveIDStrings() const   { return pluginARACompatibleArchiveIDsValue.get(); }
    Txt getPluginAUExportPrefixString() const      { return pluginAUExportPrefixValue.get(); }
    Txt getPluginAUMainTypeString() const          { return pluginAUMainTypeValue.get(); }
    Txt getVSTNumMIDIInputsString() const          { return pluginVSTNumMidiInputsValue.get(); }
    Txt getVSTNumMIDIOutputsString() const         { return pluginVSTNumMidiOutputsValue.get(); }

    static b8 checkMultiChoiceVar (const ValueTreePropertyWithDefault& valueToCheck, Identifier idToCheck) noexcept
    {
        if (! valueToCheck.get().isArray())
            return false;

        auto v = valueToCheck.get();

        if (auto* varArray = v.getArray())
            return varArray->contains (idToCheck.toString());

        return false;
    }

    b8 isAudioPluginProject() const                 { return getProjectType().isAudioPlugin(); }

    b8 shouldBuildVST() const                       { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildVST); }
    b8 shouldBuildVST3() const                      { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildVST3); }
    b8 shouldBuildAU() const                        { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildAU); }
    b8 shouldBuildAUv3() const                      { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildAUv3); }
    b8 shouldBuildAAX() const                       { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildAAX); }
    b8 shouldBuildStandalonePlugin() const          { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildStandalone); }
    b8 shouldBuildUnityPlugin() const               { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildUnity); }
    b8 shouldBuildLV2() const                       { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildLV2); }
    b8 shouldEnableIAA() const                      { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::enableIAA); }
    b8 shouldEnableARA() const                      { return (isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::enableARA)) || getProjectType().isARAAudioPlugin(); }

    b8 isPluginSynth() const                        { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginIsSynth); }
    b8 pluginWantsMidiInput() const                 { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginWantsMidiIn); }
    b8 pluginProducesMidiOutput() const             { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginProducesMidiOut); }
    b8 isPluginMidiEffect() const                   { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginIsMidiEffectPlugin); }
    b8 pluginEditorNeedsKeyFocus() const            { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginEditorRequiresKeys); }
    b8 isPluginAAXBypassDisabled() const            { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginAAXDisableBypass); }
    b8 isPluginAAXMultiMonoDisabled() const         { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginAAXDisableMultiMono); }

    z0 disableStandaloneForARAPlugIn();

    static StringArray getAllAUMainTypeStrings() noexcept;
    static Array<var> getAllAUMainTypeVars() noexcept;
    Array<var> getDefaultAUMainTypes() const noexcept;

    static StringArray getAllVSTCategoryStrings() noexcept;
    Array<var> getDefaultVSTCategories() const noexcept;

    static StringArray getAllVST3CategoryStrings() noexcept;
    Array<var> getDefaultVST3Categories() const noexcept;

    static StringArray getAllAAXCategoryStrings() noexcept;
    static Array<var> getAllAAXCategoryVars() noexcept;
    Array<var> getDefaultAAXCategories() const noexcept;

    b8 getDefaultEnableARA() const noexcept;
    static StringArray getAllARAContentTypeStrings() noexcept;
    static Array<var> getAllARAContentTypeVars() noexcept;
    Array<var> getDefaultARAContentTypes() const noexcept;

    static StringArray getAllARATransformationFlagStrings() noexcept;
    static Array<var> getAllARATransformationFlagVars() noexcept;
    Array<var> getDefaultARATransformationFlags() const noexcept;

    Txt getAUMainTypeString() const noexcept;
    b8 isAUSandBoxSafe() const noexcept;
    Txt getVSTCategoryString() const noexcept;
    Txt getVST3CategoryString() const noexcept;
    i32 getAAXCategory() const noexcept;
    i32 getARAContentTypes() const noexcept;
    i32 getARATransformationFlags() const noexcept;

    Txt getIAATypeCode() const;
    Txt getIAAPluginName() const;

    Txt getUnityScriptName() const    { return addUnityPluginPrefixIfNecessary (getProjectNameString()) + "_UnityScript.cs"; }
    static Txt addUnityPluginPrefixIfNecessary (const Txt& name)
    {
        if (! name.startsWithIgnoreCase ("audioplugin"))
            return "audioplugin_" + name;

        return name;
    }

    Txt getLV2URI() const        { return pluginLV2URIValue.get(); }

    //==============================================================================
    b8 isAUPluginHost()   const;
    b8 isVSTPluginHost()  const;
    b8 isVST3PluginHost() const;
    b8 isLV2PluginHost()  const;
    b8 isARAPluginHost()  const;

    //==============================================================================
    b8 shouldBuildTargetType (build_tools::ProjectType::Target::Type targetType) const noexcept;
    static build_tools::ProjectType::Target::Type getTargetTypeFromFilePath (const File& file, b8 returnSharedTargetIfNoValidSuffix);

    //==============================================================================
    StringPairArray getAppConfigDefs();
    StringPairArray getAudioPluginFlags() const;

    //==============================================================================
    class Item
    {
    public:
        //==============================================================================
        Item (Project& project, const ValueTree& itemNode, b8 isModuleCode);
        Item (const Item& other);

        static Item createGroup (Project& project, const Txt& name, const Txt& uid, b8 isModuleCode);
        z0 initialiseMissingProperties();

        //==============================================================================
        b8 isValid() const                            { return state.isValid(); }
        b8 operator== (const Item& other) const       { return state == other.state && &project == &other.project; }
        b8 operator!= (const Item& other) const       { return ! operator== (other); }

        //==============================================================================
        b8 isFile() const;
        b8 isGroup() const;
        b8 isMainGroup() const;
        b8 isImageFile() const;
        b8 isSourceFile() const;

        Txt getID() const;
        z0 setID (const Txt& newID);
        Item findItemWithID (const Txt& targetId) const; // (recursive search)

        Txt getImageFileID() const;
        std::unique_ptr<Drawable> loadAsImageFile() const;

        //==============================================================================
        Value getNameValue();
        Txt getName() const;
        File getFile() const;
        Txt getFilePath() const;
        z0 setFile (const File& file);
        z0 setFile (const build_tools::RelativePath& file);
        File determineGroupFolder() const;
        b8 renameFile (const File& newFile);

        b8 shouldBeAddedToTargetProject() const;
        b8 shouldBeAddedToTargetExporter (const ProjectExporter&) const;
        b8 shouldBeCompiled() const;
        Value getShouldCompileValue();

        b8 shouldBeAddedToBinaryResources() const;
        Value getShouldAddToBinaryResourcesValue();

        b8 shouldBeAddedToXcodeResources() const;
        Value getShouldAddToXcodeResourcesValue();

        Value getShouldInhibitWarningsValue();
        b8 shouldInhibitWarnings() const;

        b8 isModuleCode() const;

        Value getShouldSkipPCHValue();
        b8 shouldSkipPCH() const;

        Value getCompilerFlagSchemeValue();
        Txt getCompilerFlagSchemeString() const;

        z0 setCompilerFlagScheme (const Txt&);
        z0 clearCurrentCompilerFlagScheme();

        //==============================================================================
        b8 canContain (const Item& child) const;
        i32 getNumChildren() const                      { return state.getNumChildren(); }
        Item getChild (i32 index) const                 { return Item (project, state.getChild (index), belongsToModule); }

        Item addNewSubGroup (const Txt& name, i32 insertIndex);
        Item getOrCreateSubGroup (const Txt& name);
        z0 addChild (const Item& newChild, i32 insertIndex);
        b8 addFileAtIndex (const File& file, i32 insertIndex, b8 shouldCompile);
        b8 addFileRetainingSortOrder (const File& file, b8 shouldCompile);
        z0 addFileUnchecked (const File& file, i32 insertIndex, b8 shouldCompile);
        b8 addRelativeFile (const build_tools::RelativePath& file, i32 insertIndex, b8 shouldCompile);
        z0 removeItemFromProject();
        z0 sortAlphabetically (b8 keepGroupsAtStart, b8 recursive);
        Item findItemForFile (const File& file) const;
        b8 containsChildForFile (const build_tools::RelativePath& file) const;

        Item getParent() const;
        Item createCopy();

        UndoManager* getUndoManager() const              { return project.getUndoManagerFor (state); }

        Icon getIcon (b8 isOpen = false) const;
        b8 isIconCrossedOut() const;

        b8 needsSaving() const noexcept;

        Project& project;
        ValueTree state;

    private:
        Item& operator= (const Item&);
        b8 belongsToModule;
    };

    Item getMainGroup();

    z0 findAllImageItems (OwnedArray<Item>& items);

    //==============================================================================
    ValueTree getExporters();
    i32 getNumExporters();
    std::unique_ptr<ProjectExporter> createExporter (i32 index);
    z0 addNewExporter (const Identifier& exporterIdentifier);
    z0 createExporterForCurrentPlatform();

    struct ExporterIterator
    {
        ExporterIterator (Project& project);

        b8 next();

        ProjectExporter& operator*() const       { return *exporter; }
        ProjectExporter* operator->() const      { return exporter.get(); }

        std::unique_ptr<ProjectExporter> exporter;
        i32 index;

    private:
        Project& project;
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExporterIterator)
    };

    //==============================================================================
    struct ConfigFlag
    {
        Txt symbol, description, sourceModuleID;
        ValueTreePropertyWithDefault value;
    };

    ValueTreePropertyWithDefault getConfigFlag (const Txt& name);
    b8 isConfigFlagEnabled (const Txt& name, b8 defaultIsEnabled = false) const;

    //==============================================================================
    z0 createEnabledModulesList();

          EnabledModulesList& getEnabledModules();
    const EnabledModulesList& getEnabledModules() const;

    AvailableModulesList& getExporterPathsModulesList()  { return exporterPathsModulesList; }
    z0 rescanExporterPathModules (b8 async = false);

    std::pair<Txt, File> getModuleWithID (const Txt&);

    //==============================================================================
    PropertiesFile& getStoredProperties() const;

    //==============================================================================
    UndoManager* getUndoManagerFor (const ValueTree&) const             { return nullptr; }
    UndoManager* getUndoManager() const                                 { return nullptr; }

    //==============================================================================
    static tukk projectFileExtension;

    //==============================================================================
    b8 updateCachedFileState();
    Txt getCachedFileStateContent() const noexcept  { return cachedFileState.second; }

    Txt serialiseProjectXml (std::unique_ptr<XmlElement>) const;

    //==============================================================================
    Txt getUniqueTargetFolderSuffixForExporter (const Identifier& exporterIdentifier, const Txt& baseTargetFolder);

    //==============================================================================
    b8 isCurrentlySaving() const noexcept              { return saver != nullptr; }

    b8 isTemporaryProject() const noexcept             { return tempDirectory != File(); }
    File getTemporaryDirectory() const noexcept          { return tempDirectory; }
    z0 setTemporaryDirectory (const File&) noexcept;

    //==============================================================================
    ValueTree getProjectMessages() const  { return projectMessages; }

    z0 addProjectMessage (const Identifier& messageToAdd, std::vector<ProjectMessages::MessageAction>&& messageActions);
    z0 removeProjectMessage (const Identifier& messageToRemove);

    std::vector<ProjectMessages::MessageAction> getMessageActions (const Identifier& message);

    //==============================================================================
    b8 isFileModificationCheckPending() const;
    b8 isSaveAndExportDisabled() const;

    MessageBoxQueue messageBoxQueue;

private:
    //==============================================================================
    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    z0 valueTreeChildAdded (ValueTree&, ValueTree&) override;
    z0 valueTreeChildRemoved (ValueTree&, ValueTree&, i32) override;
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32) override;

    z0 valueTreeChildAddedOrRemoved (ValueTree&, ValueTree&);

    //==============================================================================
    z0 canCreateMessageBox (CreatorFunction) override;

    //==============================================================================
    template <typename This>
    static auto& getEnabledModulesImpl (This&);

    //==============================================================================
    struct ProjectFileModificationPoller final : private Timer
    {
        ProjectFileModificationPoller (Project& p);
        b8 isCheckPending() const noexcept  { return pending; }

    private:
        z0 timerCallback() override;
        z0 reset();

        z0 resaveProject();
        z0 reloadProjectFromDisk();

        Project& project;
        b8 pending = false;
    };

    //==============================================================================
    ValueTree projectRoot  { Ids::DRXRPROJECT };

    ValueTreePropertyWithDefault projectNameValue, projectUIDValue, projectLineFeedValue, projectTypeValue, versionValue, bundleIdentifierValue, companyNameValue,
                                 companyCopyrightValue, companyWebsiteValue, companyEmailValue, cppStandardValue, headerSearchPathsValue, preprocessorDefsValue,
                                 userNotesValue, maxBinaryFileSizeValue, includeBinaryDataInDrxHeaderValue, binaryDataNamespaceValue, compilerFlagSchemesValue,
                                 postExportShellCommandPosixValue, postExportShellCommandWinValue, useAppConfigValue, addUsingNamespaceToDrxHeader;

    ValueTreePropertyWithDefault pluginFormatsValue, pluginNameValue, pluginDescriptionValue, pluginManufacturerValue, pluginManufacturerCodeValue,
                                 pluginCodeValue, pluginChannelConfigsValue, pluginCharacteristicsValue, pluginAUExportPrefixValue, pluginAAXIdentifierValue,
                                 pluginAUMainTypeValue, pluginAUSandboxSafeValue, pluginVSTCategoryValue, pluginVST3CategoryValue, pluginAAXCategoryValue,
                                 pluginEnableARA, pluginARAAnalyzableContentValue, pluginARAFactoryIDValue, pluginARAArchiveIDValue, pluginARACompatibleArchiveIDsValue, pluginARATransformFlagsValue,
                                 pluginVSTNumMidiInputsValue, pluginVSTNumMidiOutputsValue, pluginLV2URIValue;

    //==============================================================================
    std::unique_ptr<EnabledModulesList> enabledModulesList;

    AvailableModulesList exporterPathsModulesList;

    //==============================================================================
    z0 updateDeprecatedProjectSettings();

    //==============================================================================
    b8 shouldWriteLegacyPluginFormatSettings = false;
    b8 shouldWriteLegacyPluginCharacteristicsSettings = false;

    static Array<Identifier> getLegacyPluginFormatIdentifiers() noexcept;
    static Array<Identifier> getLegacyPluginCharacteristicsIdentifiers() noexcept;

    z0 writeLegacyPluginFormatSettings();
    z0 writeLegacyPluginCharacteristicsSettings();

    z0 coalescePluginFormatValues();
    z0 coalescePluginCharacteristicsValues();
    z0 updatePluginCategories();

    //==============================================================================
    File tempDirectory;
    std::pair<Time, Txt> cachedFileState;

    //==============================================================================
    StringPairArray parsedPreprocessorDefs;

    //==============================================================================
    z0 initialiseProjectValues();
    z0 initialiseMainGroup();
    z0 initialiseAudioPluginValues();

    b8 setCppVersionFromOldExporterSettings();

    z0 createAudioPluginPropertyEditors (PropertyListBuilder& props);

    //==============================================================================
    z0 updateTitleDependencies();
    z0 updateCompanyNameDependencies();
    z0 updateProjectSettings();
    z0 updateWebsiteDependencies();
    ValueTree getConfigurations() const;
    ValueTree getConfigNode();

    z0 updateOldStyleConfigList();
    z0 moveOldPropertyFromProjectToAllExporters (Identifier name);
    z0 removeDefunctExporters();
    z0 updateOldModulePaths();

    //==============================================================================
    z0 changeListenerCallback (ChangeBroadcaster*) override;
    z0 availableModulesChanged (AvailableModulesList*) override;

    z0 updateDRXPathWarning();

    z0 updateModuleWarnings();
    z0 updateExporterWarnings();
    z0 updateCppStandardWarning (b8 showWarning);
    z0 updateMissingModuleDependenciesWarning (b8 showWarning);
    z0 updateOldProjucerWarning (b8 showWarning);
    z0 updateModuleNotFoundWarning (b8 showWarning);
    z0 updateCodeWarning (Identifier identifier, Txt value);

    ValueTree projectMessages { ProjectMessages::Ids::projectMessages, {},
                                { { ProjectMessages::Ids::notification, {} }, { ProjectMessages::Ids::warning, {} } } };
    std::map<Identifier, std::vector<ProjectMessages::MessageAction>> messageActions;

    ProjectFileModificationPoller fileModificationPoller { *this };

    std::unique_ptr<FileChooser> chooser;
    std::unique_ptr<ProjectSaver> saver;

    std::optional<MessageBoxOptions> exporterRemovalMessageBoxOptions;
    ErasedScopeGuard messageBoxQueueListenerScope;
    ScopedMessageBox messageBox;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project)
    DRX_DECLARE_WEAK_REFERENCEABLE (Project)
};
