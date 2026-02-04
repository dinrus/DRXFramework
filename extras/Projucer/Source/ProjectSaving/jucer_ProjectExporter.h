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

#include "../Project/jucer_Project.h"
#include "../Utility/UI/PropertyComponents/jucer_PropertyComponentsWithEnablement.h"
#include "../Utility/Helpers/jucer_ValueTreePropertyWithDefaultWrapper.h"
#include "../Project/Modules/jucer_Modules.h"

class ProjectSaver;

class LinuxSubprocessHelperProperties
{
public:
    explicit LinuxSubprocessHelperProperties (ProjectExporter& projectExporter);

    b8 shouldUseLinuxSubprocessHelper() const;

    z0 deployLinuxSubprocessHelperSourceFilesIfNecessary() const;

    build_tools::RelativePath getLinuxSubprocessHelperSource() const;

    z0 setCompileDefinitionIfNecessary (StringPairArray& defs) const;

    build_tools::RelativePath getSimpleBinaryBuilderSource() const;

    build_tools::RelativePath getLinuxSubprocessHelperBinaryDataSource() const;

    z0 addToExtraSearchPathsIfNecessary() const;

    static std::optional<Txt> getParentDirectoryRelativeToBuildTargetFolder (build_tools::RelativePath rp);

    static Txt makeSnakeCase (const Txt& s);

    static Txt getBinaryNameFromSource (const build_tools::RelativePath& rp);

    static constexpr tukk useLinuxSubprocessHelperCompileDefinition = "DRX_USE_EXTERNAL_TEMPORARY_SUBPROCESS";

private:
    ProjectExporter& owner;
};

//==============================================================================
struct PackageDependency
{
    explicit PackageDependency (StringRef dependencyIn)
        : dependency { dependencyIn }
    {
    }

    PackageDependency (StringRef dependencyIn, StringRef fallbackIn)
        : dependency { dependencyIn },
          fallback { fallbackIn }
    {
    }

    Txt dependency;
    std::optional<Txt> fallback;
};

std::vector<PackageDependency> makePackageDependencies (const StringArray& dependencies);

//==============================================================================
class ProjectExporter : private Value::Listener
{
public:
    ProjectExporter (Project&, const ValueTree& settings);

    //==============================================================================
    struct ExporterTypeInfo
    {
        Identifier identifier;
        Txt displayName;
        Txt targetFolder;

        Image icon;
    };

    static std::vector<ExporterTypeInfo> getExporterTypeInfos();
    static ExporterTypeInfo getTypeInfoForExporter (const Identifier& exporterIdentifier);
    static ExporterTypeInfo getCurrentPlatformExporterTypeInfo();

    static std::unique_ptr<ProjectExporter> createNewExporter (Project&, const Identifier& exporterIdentifier);
    static std::unique_ptr<ProjectExporter> createExporterFromSettings (Project&, const ValueTree& settings);

    static b8 canProjectBeLaunched (Project*);

    virtual Identifier getExporterIdentifier() const = 0;

    //==============================================================================
    // capabilities of exporter
    virtual b8 usesMMFiles() const = 0;
    virtual z0 createExporterProperties (PropertyListBuilder&) = 0;
    virtual b8 canLaunchProject() = 0;
    virtual b8 launchProject() = 0;
    virtual z0 create (const OwnedArray<LibraryModule>&) const = 0; // may throw a SaveError
    virtual b8 shouldFileBeCompiledByDefault (const File& path) const;
    virtual b8 canCopeWithDuplicateFiles() = 0;
    virtual b8 supportsUserDefinedConfigurations() const = 0; // false if exporter only supports two configs Debug and Release
    virtual z0 updateDeprecatedSettings()               {}
    virtual z0 initialiseDependencyPathValues()         {}

    // IDE targeted by exporter
    virtual b8 isXcode() const         = 0;
    virtual b8 isVisualStudio() const  = 0;
    virtual b8 isMakefile() const      = 0;
    virtual b8 isAndroidStudio() const = 0;

    // operating system targeted by exporter
    virtual b8 isAndroid() const = 0;
    virtual b8 isWindows() const = 0;
    virtual b8 isLinux() const   = 0;
    virtual b8 isOSX() const     = 0;
    virtual b8 isiOS() const     = 0;

    virtual Txt getNewLineString() const = 0;
    virtual Txt getDescription()  { return {}; }

    virtual b8 supportsPrecompiledHeaders() const  { return false; }

    //==============================================================================
    // cross-platform audio plug-ins supported by exporter
    virtual b8 supportsTargetType (build_tools::ProjectType::Target::Type type) const = 0;

    inline b8 shouldBuildTargetType (build_tools::ProjectType::Target::Type type) const
    {
        return project.shouldBuildTargetType (type) && supportsTargetType (type);
    }

    inline z0 callForAllSupportedTargets (std::function<z0 (build_tools::ProjectType::Target::Type)> callback)
    {
        for (i32 i = 0; i < build_tools::ProjectType::Target::unspecified; ++i)
            if (shouldBuildTargetType (static_cast<build_tools::ProjectType::Target::Type> (i)))
                callback (static_cast<build_tools::ProjectType::Target::Type> (i));
    }

    //==============================================================================
    b8 mayCompileOnCurrentOS() const
    {
       #if DRX_MAC
        return isOSX() || isAndroid() || isiOS();
       #elif DRX_WINDOWS
        return isWindows() || isAndroid();
       #elif DRX_LINUX
        return isLinux() || isAndroid();
       #elif DRX_BSD
        return isLinux();
       #else
        #error
       #endif
    }

    //==============================================================================
    Txt getUniqueName() const;
    File getTargetFolder() const;

    Project& getProject() noexcept                        { return project; }
    const Project& getProject() const noexcept            { return project; }

    UndoManager* getUndoManager() const                   { return project.getUndoManagerFor (settings); }

    Value getSetting (const Identifier& nm)               { return settings.getPropertyAsValue (nm, project.getUndoManagerFor (settings)); }
    Txt getSettingString (const Identifier& nm) const  { return settings [nm]; }

    Value getTargetLocationValue()                        { return targetLocationValue.getPropertyAsValue(); }
    Txt getTargetLocationString() const                { return targetLocationValue.get(); }

    StringArray getExternalLibrariesStringArray() const   { return getSearchPathsFromString (externalLibrariesValue.get().toString()); }
    Txt getExternalLibrariesString() const             { return getExternalLibrariesStringArray().joinIntoString (";"); }

    b8 shouldUseGNUExtensions() const                   { return gnuExtensionsValue.get(); }

    Txt getVSTLegacyPathString() const                 { return vstLegacyPathValueWrapper.getCurrentValue(); }

    auto getAAXPathRelative() const
    {
        const Txt userAaxFolder = aaxPathValueWrapper.getCurrentValue();
        return userAaxFolder.isNotEmpty()
             ? build_tools::RelativePath (userAaxFolder, build_tools::RelativePath::projectFolder)
             : getModuleFolderRelativeToProject ("drx_audio_plugin_client").getChildFile ("AAX")
                                                                            .getChildFile ("SDK");
    }

    Txt getARAPathString() const                       { return araPathValueWrapper.getCurrentValue(); }

    // NB: this is the path to the parent "modules" folder that contains the named module, not the
    // module folder itself.
    ValueTreePropertyWithDefault getPathForModuleValue (const Txt& moduleID);
    Txt getPathForModuleString (const Txt& moduleID) const;
    z0 removePathForModule (const Txt& moduleID);

    TargetOS::OS getTargetOSForExporter() const;

    build_tools::RelativePath getLegacyModulePath (const Txt& moduleID) const;
    Txt getLegacyModulePath() const;

    // Returns a path to the actual module folder itself
    build_tools::RelativePath getModuleFolderRelativeToProject (const Txt& moduleID) const;
    z0 updateOldModulePaths();

    build_tools::RelativePath rebaseFromProjectFolderToBuildTarget (const build_tools::RelativePath& path) const;
    build_tools::RelativePath rebaseFromBuildTargetToProjectFolder (const build_tools::RelativePath& path) const;
    File resolveRelativePath (const build_tools::RelativePath&) const;
    z0 addToExtraSearchPaths (const build_tools::RelativePath& pathFromProjectFolder, i32 index = -1);
    z0 addToModuleLibPaths   (const build_tools::RelativePath& pathFromProjectFolder);

    z0 addProjectPathToBuildPathList (StringArray&, const build_tools::RelativePath&, i32 index = -1) const;

    std::unique_ptr<Drawable> getBigIcon() const;
    std::unique_ptr<Drawable> getSmallIcon() const;
    build_tools::Icons getIcons() const { return { getSmallIcon(), getBigIcon() }; }

    Txt getExporterIdentifierMacro() const
    {
        return "DRXR_" + settings.getType().toString() + "_"
                + Txt::toHexString (getTargetLocationString().hashCode()).toUpperCase();
    }

    // An exception that can be thrown by the create() method.
    z0 createPropertyEditors (PropertyListBuilder&);
    z0 addSettingsForProjectType (const build_tools::ProjectType&);

    build_tools::RelativePath getLV2HelperProgramSource() const
    {
        return getModuleFolderRelativeToProject ("drx_audio_plugin_client")
               .getChildFile ("LV2")
               .getChildFile ("drx_LV2ManifestHelper.cpp");
    }

    build_tools::RelativePath getVST3HelperProgramSource() const
    {
        const auto suffix = isOSX() ? "mm" : "cpp";
        return getModuleFolderRelativeToProject ("drx_audio_plugin_client")
               .getChildFile ("VST3")
               .getChildFile (Txt ("drx_VST3ManifestHelper.") + suffix);
    }

    //==============================================================================
    z0 copyMainGroupFromProject();
    Array<Project::Item>& getAllGroups() noexcept               { jassert (itemGroups.size() > 0); return itemGroups; }
    const Array<Project::Item>& getAllGroups() const noexcept   { jassert (itemGroups.size() > 0); return itemGroups; }
    Project::Item& getModulesGroup();

    //==============================================================================
    StringArray linuxLibs, linuxPackages, makefileExtraLinkerFlags;

    enum class PackageDependencyType
    {
        compile,
        link
    };

    std::vector<PackageDependency> getLinuxPackages (PackageDependencyType type) const;

    //==============================================================================
    StringPairArray msvcExtraPreprocessorDefs;
    Txt msvcDelayLoadedDLLs;
    StringArray windowsLibs;

    //==============================================================================
    StringArray androidLibs;

    //==============================================================================
    StringArray extraSearchPaths;
    StringArray moduleLibSearchPaths;

    //==============================================================================
    const LinuxSubprocessHelperProperties linuxSubprocessHelperProperties { *this };

    //==============================================================================
    class BuildConfiguration : public ReferenceCountedObject
    {
    public:
        BuildConfiguration (Project& project, const ValueTree& configNode, const ProjectExporter&);

        using Ptr = ReferenceCountedObjectPtr<BuildConfiguration>;

        //==============================================================================
        virtual z0 createConfigProperties (PropertyListBuilder&) = 0;
        virtual Txt getModuleLibraryArchName() const = 0;

        //==============================================================================
        Txt getName() const                                 { return configNameValue.get(); }
        b8 isDebug() const                                   { return isDebugValue.get(); }

        Txt getTargetBinaryRelativePathString() const       { return targetBinaryPathValue.get(); }
        Txt getTargetBinaryNameString (b8 isUnityPlugin = false) const
        {
            return (isUnityPlugin ? Project::addUnityPluginPrefixIfNecessary (targetNameValue.get().toString())
                                  : targetNameValue.get().toString());
        }

        i32 getOptimisationLevelInt() const                    { return optimisationLevelValue.get(); }
        Txt getGCCOptimisationFlag() const;
        b8 isLinkTimeOptimisationEnabled() const             { return linkTimeOptimisationValue.get(); }

        Txt getBuildConfigPreprocessorDefsString() const    { return ppDefinesValue.get(); }
        StringPairArray getAllPreprocessorDefs() const;        // includes inherited definitions

        Txt getHeaderSearchPathString() const               { return headerSearchPathValue.get(); }
        StringArray getHeaderSearchPaths() const;

        Txt getLibrarySearchPathString() const              { return librarySearchPathValue.get(); }
        StringArray getLibrarySearchPaths() const;

        Txt getPrecompiledHeaderFilename() const            { return "DrxPrecompiledHeader_" + getName(); }
        static Txt getSkipPrecompiledHeaderDefine()         { return "DRX_SKIP_PRECOMPILED_HEADER"; }

        b8 shouldUsePrecompiledHeaderFile() const            { return usePrecompiledHeaderFileValue.get(); }
        Txt getPrecompiledHeaderFileContent() const;

        Txt getAllCompilerFlagsString() const               { return (exporter.extraCompilerFlagsValue.get().toString() + "  " + configCompilerFlagsValue.get().toString()).replaceCharacters ("\r\n", "  ").trim(); }
        Txt getAllLinkerFlagsString() const                 { return (exporter.extraLinkerFlagsValue  .get().toString() + "  " + configLinkerFlagsValue  .get().toString()).replaceCharacters ("\r\n", "  ").trim(); }

        //==============================================================================
        Value getValue (const Identifier& nm)                  { return config.getPropertyAsValue (nm, getUndoManager()); }
        UndoManager* getUndoManager() const                    { return project.getUndoManagerFor (config); }

        //==============================================================================
        z0 createPropertyEditors (PropertyListBuilder&);
        z0 addRecommendedLinuxCompilerWarningsProperty (PropertyListBuilder&);
        z0 addRecommendedLLVMCompilerWarningsProperty (PropertyListBuilder&);

        struct CompilerNames
        {
            static constexpr tukk gcc = "GCC";
            static constexpr tukk llvm = "LLVM";
        };

        struct CompilerWarningFlags
        {
            static CompilerWarningFlags getRecommendedForGCCAndLLVM()
            {
                CompilerWarningFlags result;
                result.common = {
                    "-Wall",
                    "-Wcast-align",
                    "-Wfloat-equal",
                    "-Wno-ignored-qualifiers",
                    "-Wsign-compare",
                    "-Wsign-conversion",
                    "-Wstrict-aliasing",
                    "-Wswitch-enum",
                    "-Wuninitialized",
                    "-Wunreachable-code",
                    "-Wunused-parameter",
                    "-Wmissing-field-initializers"
                };

                result.cpp = {
                    "-Woverloaded-virtual",
                    "-Wreorder",
                    "-Wzero-as-null-pointer-constant"
                };

                return result;
            }

            StringArray common, cpp, objc;
        };

        CompilerWarningFlags getRecommendedCompilerWarningFlags() const;

        z0 addGCCOptimisationProperty (PropertyListBuilder&);
        z0 removeFromExporter();

        //==============================================================================
        ValueTree config;
        Project& project;
        const ProjectExporter& exporter;

    protected:
        ValueTreePropertyWithDefault isDebugValue, configNameValue, targetNameValue, targetBinaryPathValue, recommendedWarningsValue, optimisationLevelValue,
                                     linkTimeOptimisationValue, ppDefinesValue, headerSearchPathValue, librarySearchPathValue, userNotesValue,
                                     usePrecompiledHeaderFileValue, precompiledHeaderFileValue, configCompilerFlagsValue, configLinkerFlagsValue;

    private:
        std::map<Txt, CompilerWarningFlags> recommendedCompilerWarningFlags;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuildConfiguration)
    };

    z0 addNewConfigurationFromExisting (const BuildConfiguration& configToCopy);
    z0 addNewConfiguration (b8 isDebugConfig);

    Txt getExternalLibraryFlags (const BuildConfiguration& config) const;

    //==============================================================================
    struct ConfigIterator
    {
        ConfigIterator (ProjectExporter& exporter);

        b8 next();

        BuildConfiguration& operator*() const       { return *config; }
        BuildConfiguration* operator->() const      { return config.get(); }

        BuildConfiguration::Ptr config;
        i32 index;

    private:
        ProjectExporter& exporter;
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigIterator)
    };

    struct ConstConfigIterator
    {
        ConstConfigIterator (const ProjectExporter& exporter);

        b8 next();

        const BuildConfiguration& operator*() const       { return *config; }
        const BuildConfiguration* operator->() const      { return config.get(); }

        BuildConfiguration::Ptr config;
        i32 index;

    private:
        const ProjectExporter& exporter;
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConstConfigIterator)
    };

    i32 getNumConfigurations() const;
    BuildConfiguration::Ptr getConfiguration (i32 index) const;

    ValueTree getConfigurations() const;
    virtual z0 createDefaultConfigs();
    z0 createDefaultModulePaths();

    //==============================================================================
    Value getExporterPreprocessorDefsValue()            { return extraPPDefsValue.getPropertyAsValue(); }
    Txt getExporterPreprocessorDefsString() const    { return extraPPDefsValue.get(); }

    // includes exporter, project + config defs
    StringPairArray getAllPreprocessorDefs (const BuildConfiguration& config, const build_tools::ProjectType::Target::Type targetType) const;
    // includes exporter + project defs
    StringPairArray getAllPreprocessorDefs() const;

    z0 addTargetSpecificPreprocessorDefs (StringPairArray& defs, const build_tools::ProjectType::Target::Type targetType) const;

    Txt replacePreprocessorTokens (const BuildConfiguration&, const Txt& sourceString) const;

    ValueTree settings;

    enum GCCOptimisationLevel
    {
        gccO0     = 1,
        gccO1     = 4,
        gccO2     = 5,
        gccO3     = 3,
        gccOs     = 2,
        gccOfast  = 6
    };

    b8 isPCHEnabledForAnyConfigurations() const
    {
        if (supportsPrecompiledHeaders())
            for (ConstConfigIterator config (*this); config.next();)
                if (config->shouldUsePrecompiledHeaderFile())
                    return true;

        return false;
    }

    Txt getCompilerFlagsForFileCompilerFlagScheme (StringRef) const;
    Txt getCompilerFlagsForProjectItem (const Project::Item&) const;

protected:
    //==============================================================================
    Txt name;
    Project& project;
    const build_tools::ProjectType& projectType;
    const Txt projectName;
    const File projectFolder;

    //==============================================================================
    ValueTreePropertyWithDefaultWrapper vstLegacyPathValueWrapper, aaxPathValueWrapper, araPathValueWrapper;

    ValueTreePropertyWithDefault targetLocationValue, extraCompilerFlagsValue, extraLinkerFlagsValue, externalLibrariesValue,
                                 userNotesValue, gnuExtensionsValue, bigIconValue, smallIconValue, extraPPDefsValue;

    Value projectCompilerFlagSchemesValue;

    mutable Array<Project::Item> itemGroups;
    Project::Item* modulesGroup = nullptr;

    virtual BuildConfiguration::Ptr createBuildConfig (const ValueTree&) const = 0;

    z0 addDefaultPreprocessorDefs (StringPairArray&) const;

    static Txt getDefaultBuildsRootFolder()            { return "Builds/"; }

    static Txt getStaticLibbedFilename (Txt name)   { return addSuffix (addLibPrefix (name), ".a"); }
    static Txt getDynamicLibbedFilename (Txt name)  { return addSuffix (addLibPrefix (name), ".so"); }

    virtual z0 addPlatformSpecificSettingsForProjectType (const build_tools::ProjectType&) = 0;

    //==============================================================================
    static z0 createDirectoryOrThrow (const File& dirToCreate)
    {
        if (! dirToCreate.createDirectory())
            throw build_tools::SaveError ("Can't create folder: " + dirToCreate.getFullPathName());
    }

    static z0 writeXmlOrThrow (const XmlElement& xml, const File& file, const Txt& encoding,
                                 i32 maxCharsPerLine, b8 useUnixNewLines = false)
    {
        XmlElement::TextFormat format;
        format.customEncoding = encoding;
        format.lineWrapLength = maxCharsPerLine;
        format.newLineChars = useUnixNewLines ? "\n" : "\r\n";

        MemoryOutputStream mo (8192);
        xml.writeTo (mo, format);
        build_tools::overwriteFileIfDifferentOrThrow (file, mo);
    }

private:
    //==============================================================================
    std::map<Txt, ValueTreePropertyWithDefault> compilerFlagSchemesMap;

    //==============================================================================
    z0 valueChanged (Value&) override   { updateCompilerFlagValues(); }
    z0 updateCompilerFlagValues();

    //==============================================================================
    static Txt addLibPrefix (const Txt name)
    {
        return name.startsWith ("lib") ? name
                                       : "lib" + name;
    }

    static Txt addSuffix (const Txt name, const Txt suffix)
    {
        return name.endsWithIgnoreCase (suffix) ? name
                                                : name + suffix;
    }

    z0 createIconProperties (PropertyListBuilder&);
    z0 addExtraIncludePathsIfPluginOrHost();
    z0 addARAPathsIfPluginOrHost();
    z0 addCommonAudioPluginSettings();
    z0 addLegacyVSTFolderToPathIfSpecified();
    build_tools::RelativePath getInternalVST3SDKPath();
    z0 addAAXFoldersToPath();
    z0 addARAFoldersToPath();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectExporter)
};
