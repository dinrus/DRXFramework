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
class MakefileProjectExporter final : public ProjectExporter
{
protected:
    //==============================================================================
    class MakeBuildConfiguration final : public BuildConfiguration
    {
    public:
        MakeBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e),
              architectureTypeValue     (config, Ids::linuxArchitecture,          getUndoManager(), Txt()),
              pluginBinaryCopyStepValue (config, Ids::enablePluginBinaryCopyStep, getUndoManager(), true),
              vstBinaryLocation         (config, Ids::vstBinaryLocation,          getUndoManager(), "$(HOME)/.vst"),
              vst3BinaryLocation        (config, Ids::vst3BinaryLocation,         getUndoManager(), "$(HOME)/.vst3"),
              lv2BinaryLocation         (config, Ids::lv2BinaryLocation,          getUndoManager(), "$(HOME)/.lv2"),
              unityPluginBinaryLocation (config, Ids::unityPluginBinaryLocation,  getUndoManager(), "$(HOME)/UnityPlugins")
        {
            linkTimeOptimisationValue.setDefault (false);
            optimisationLevelValue.setDefault (isDebug() ? gccO0 : gccO3);
        }

        z0 createConfigProperties (PropertyListBuilder& props) override
        {
            addRecommendedLinuxCompilerWarningsProperty (props);
            addGCCOptimisationProperty (props);

            props.add (new ChoicePropertyComponent (architectureTypeValue, "Architecture",
                                                    { "<None>",     "Native",        "32-bit (-m32)", "64-bit (-m64)", "ARM v6",       "ARM v7",       "ARM v8-a" },
                                                    { { Txt() }, "-march=native", "-m32",          "-m64",          "-march=armv6", "-march=armv7", "-march=armv8-a" }),
                       "Specifies the 32/64-bit architecture to use. If you don't see the required architecture in this list, you can also specify the desired "
                       "flag on the command-line when invoking make by passing \"TARGET_ARCH=-march=<arch to use>\"");

            auto isBuildingAnyPlugins = (project.shouldBuildVST() || project.shouldBuildVST3() || project.shouldBuildUnityPlugin() || project.shouldBuildLV2());

            if (isBuildingAnyPlugins)
            {
                props.add (new ChoicePropertyComponent (pluginBinaryCopyStepValue, "Enable Plugin Copy Step"),
                           "Enable this to copy plugin binaries to a specified folder after building.");

                if (project.shouldBuildVST3())
                    props.add (new TextPropertyComponentWithEnablement (vst3BinaryLocation, pluginBinaryCopyStepValue, "VST3 Binary Location",
                                                                        1024, false),
                               "The folder in which the compiled VST3 binary should be placed.");

                if (project.shouldBuildLV2())
                    props.add (new TextPropertyComponentWithEnablement (lv2BinaryLocation, pluginBinaryCopyStepValue, "LV2 Binary Location",
                                                                        1024, false),
                               "The folder in which the compiled LV2 binary should be placed.");

                if (project.shouldBuildUnityPlugin())
                    props.add (new TextPropertyComponentWithEnablement (unityPluginBinaryLocation, pluginBinaryCopyStepValue, "Unity Binary Location",
                                                                        1024, false),
                               "The folder in which the compiled Unity plugin binary and associated C# GUI script should be placed.");

                if (project.shouldBuildVST())
                    props.add (new TextPropertyComponentWithEnablement (vstBinaryLocation, pluginBinaryCopyStepValue, "VST (Legacy) Binary Location",
                                                                        1024, false),
                               "The folder in which the compiled legacy VST binary should be placed.");
            }
        }

        Txt getModuleLibraryArchName() const override
        {
            auto archFlag = getArchitectureTypeString();
            Txt prefix ("-march=");

            if (archFlag.startsWith (prefix))
                return archFlag.substring (prefix.length());

            if (archFlag == "-m64")
                return "x86_64";

            if (archFlag == "-m32")
                return "i386";

            return "${DRX_ARCH_LABEL}";
        }

        Txt getArchitectureTypeString() const           { return architectureTypeValue.get(); }

        b8 isPluginBinaryCopyStepEnabled() const         { return pluginBinaryCopyStepValue.get(); }
        Txt getVSTBinaryLocationString() const          { return vstBinaryLocation.get(); }
        Txt getVST3BinaryLocationString() const         { return vst3BinaryLocation.get(); }
        Txt getLV2BinaryLocationString() const          { return lv2BinaryLocation.get(); }
        Txt getUnityPluginBinaryLocationString() const  { return unityPluginBinaryLocation.get(); }

    private:
        //==============================================================================
        ValueTreePropertyWithDefault architectureTypeValue, pluginBinaryCopyStepValue,
                                     vstBinaryLocation, vst3BinaryLocation, lv2BinaryLocation, unityPluginBinaryLocation;
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return *new MakeBuildConfiguration (project, tree, *this);
    }

public:
    //==============================================================================
    class MakefileTarget final : public build_tools::ProjectType::Target
    {
    public:
        MakefileTarget (Type targetType, const MakefileProjectExporter& exporter)
            : Target (targetType), owner (exporter)
        {}

        StringArray getCompilerFlags() const
        {
            StringArray result;

            if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle || type == SharedCodeTarget)
            {
                result.add ("-fPIC");
                result.add ("-fvisibility=hidden");
            }

            return result;
        }

        StringArray getLinkerFlags() const
        {
            StringArray result;

            if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle)
            {
                result.add ("-shared");

                if (getTargetFileType() == pluginBundle)
                    result.add ("-Wl,--no-undefined");
            }

            return result;
        }

        StringPairArray getDefines (const BuildConfiguration& config) const
        {
            StringPairArray result;
            auto commonOptionKeys = owner.getAllPreprocessorDefs (config, unspecified).getAllKeys();
            auto targetSpecific = owner.getAllPreprocessorDefs (config, type);

            for (auto& key : targetSpecific.getAllKeys())
                if (! commonOptionKeys.contains (key))
                    result.set (key, targetSpecific[key]);

            return result;
        }

        StringArray getTargetSettings (const MakeBuildConfiguration& config) const
        {
            if (type == AggregateTarget) // the aggregate target should not specify any settings at all!
                return {};               // it just defines dependencies on the other targets.

            StringArray s;

            auto cppflagsVarName = "DRX_CPPFLAGS_" + getTargetVarName();

            s.add (cppflagsVarName + " := " + createGCCPreprocessorFlags (getDefines (config)));

            auto cflags = getCompilerFlags();

            if (! cflags.isEmpty())
                s.add ("DRX_CFLAGS_" + getTargetVarName() + " := " + cflags.joinIntoString (" "));

            auto ldflags = getLinkerFlags();

            if (! ldflags.isEmpty())
                s.add ("DRX_LDFLAGS_" + getTargetVarName() + " := " + ldflags.joinIntoString (" "));

            auto targetName = owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString (type == UnityPlugIn));

            if (owner.projectType.isStaticLibrary())
                targetName = getStaticLibbedFilename (targetName);
            else if (owner.projectType.isDynamicLibrary())
                targetName = getDynamicLibbedFilename (targetName);
            else
                targetName = targetName.upToLastOccurrenceOf (".", false, false) + getTargetFileSuffix();

            if (type == VST3PlugIn)
            {
                s.add ("DRX_VST3DIR := " + escapeQuotesAndSpaces (targetName).upToLastOccurrenceOf (".", false, false) + ".vst3");
                s.add ("VST3_PLATFORM_ARCH := $(shell $(CXX) make_helpers/arch_detection.cpp 2>&1 | tr '\\n' ' ' | sed \"s/.*DRX_ARCH \\([a-zA-Z0-9_-]*\\).*/\\1/\")");
                s.add ("DRX_VST3SUBDIR := Contents/$(VST3_PLATFORM_ARCH)-linux");

                targetName = "$(DRX_VST3DIR)/$(DRX_VST3SUBDIR)/" + targetName;
            }
            else if (type == UnityPlugIn)
            {
                s.add ("DRX_UNITYDIR := Unity");
                targetName = "$(DRX_UNITYDIR)/" + targetName;
            }
            else if (type == LV2PlugIn)
            {
                s.add ("DRX_LV2DIR := " + escapeQuotesAndSpaces (targetName) + ".lv2");
                targetName = "$(DRX_LV2DIR)/" + targetName + ".so";
            }
            else if (type == LV2Helper)
            {
                targetName = Project::getLV2FileWriterName();
            }
            else if (type == VST3Helper)
            {
                targetName = Project::getVST3FileWriterName();
            }

            s.add ("DRX_TARGET_" + getTargetVarName() + Txt (" := ") + escapeQuotesAndSpaces (targetName));

            if (type == LV2PlugIn)
                s.add ("DRX_LV2_FULL_PATH := $(DRX_OUTDIR)/$(DRX_TARGET_LV2_PLUGIN)");

            if (config.isPluginBinaryCopyStepEnabled()
                && (type == VST3PlugIn || type == VSTPlugIn || type == UnityPlugIn || type == LV2PlugIn))
            {
                Txt copyCmd ("DRX_COPYCMD_" + getTargetVarName() + Txt (" := $(DRX_OUTDIR)/"));

                if (type == VST3PlugIn)
                {
                    s.add ("DRX_VST3DESTDIR := " + config.getVST3BinaryLocationString());
                    s.add (copyCmd + "$(DRX_VST3DIR) $(DRX_VST3DESTDIR)");
                }
                else if (type == VSTPlugIn)
                {
                    s.add ("DRX_VSTDESTDIR := " + config.getVSTBinaryLocationString());
                    s.add (copyCmd + escapeQuotesAndSpaces (targetName) + " $(DRX_VSTDESTDIR)");
                }
                else if (type == UnityPlugIn)
                {
                    s.add ("DRX_UNITYDESTDIR := " + config.getUnityPluginBinaryLocationString());
                    s.add (copyCmd + "$(DRX_UNITYDIR)/. $(DRX_UNITYDESTDIR)");
                }
                else if (type == LV2PlugIn)
                {
                    s.add ("DRX_LV2DESTDIR := " + config.getLV2BinaryLocationString());
                    s.add (copyCmd + "$(DRX_LV2DIR) $(DRX_LV2DESTDIR)");
                }
            }

            return s;
        }

        Txt getTargetFileSuffix() const
        {
            if (type == VSTPlugIn || type == VST3PlugIn || type == UnityPlugIn || type == DynamicLibrary)
                return ".so";

            if (type == SharedCodeTarget || type == StaticLibrary)
                return ".a";

            return {};
        }

        Txt getTargetVarName() const
        {
            return Txt (getName()).toUpperCase().replaceCharacter (L' ', L'_');
        }

        z0 writeObjects (OutputStream& out, const std::vector<std::pair<build_tools::RelativePath, Txt>>& filesToCompile) const
        {
            out << "OBJECTS_" + getTargetVarName() + Txt (" := \\") << newLine;

            for (auto& f : filesToCompile)
                out << "  $(DRX_OBJDIR)/" << escapeQuotesAndSpaces (owner.getObjectFileFor (f.first))
                    << " \\" << newLine;

            out << newLine;
        }

        z0 addFiles (OutputStream& out, const std::vector<std::pair<build_tools::RelativePath, Txt>>& filesToCompile)
        {
            auto cppflagsVarName = "DRX_CPPFLAGS_" + getTargetVarName();
            auto cflagsVarName   = "DRX_CFLAGS_"   + getTargetVarName();

            for (auto& [path, flags] : filesToCompile)
            {
                const auto additionalTargetDependencies = [&p = path, this]
                {
                    if (   owner.linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper()
                        && p.getFileName().contains ("include_drx_gui_extra.cpp"))
                    {
                        return owner.linuxSubprocessHelperProperties
                            .getLinuxSubprocessHelperBinaryDataSource()
                            .toUnixStyle();
                    }

                    return Txt{};
                }();

                const auto prependedWithSpaceIfNotEmpty = [] (auto s)
                {
                    return s.isEmpty() ? s : " " + s;
                };

                out << "$(DRX_OBJDIR)/" << escapeQuotesAndSpaces (owner.getObjectFileFor (path)) << ": " << escapeQuotesAndSpaces (path.toUnixStyle())
                    << prependedWithSpaceIfNotEmpty (additionalTargetDependencies)                                                                      << newLine
                    << "\t-$(V_AT)mkdir -p $(@D)"                                                                                                       << newLine
                    << "\t@echo \"Compiling " << path.getFileName() << "\""                                                                             << newLine
                    << (path.hasFileExtension ("c;s;S") ? "\t$(V_AT)$(CC) $(DRX_CFLAGS) " : "\t$(V_AT)$(CXX) $(DRX_CXXFLAGS) ")
                    << "$(" << cppflagsVarName << ") $(" << cflagsVarName << ")"
                    << (flags.isNotEmpty() ? " $(" + owner.getCompilerFlagSchemeVariableName (flags) + ")" : "") << " -o \"$@\" -c \"$<\""              << newLine
                    << newLine;
            }
        }

        Txt getBuildProduct() const
        {
            return "$(DRX_OUTDIR)/$(DRX_TARGET_" + getTargetVarName() + ")";
        }

        Txt getPhonyName() const
        {
            if (type == LV2Helper)
                return "LV2_MANIFEST_HELPER";

            if (type == VST3Helper)
                return "VST3_MANIFEST_HELPER";

            return Txt (getName()).upToFirstOccurrenceOf (" ", false, false);
        }

        z0 writeTargetLine (OutputStream& out, const StringArray& packages)
        {
            jassert (type != AggregateTarget);

            out << getBuildProduct() << " : "
                << "$(OBJECTS_" << getTargetVarName() << ") $(DRX_OBJDIR)/execinfo.cmd $(RESOURCES)";

            if (type != SharedCodeTarget && owner.shouldBuildTargetType (SharedCodeTarget))
                out << " $(DRX_OUTDIR)/$(DRX_TARGET_SHARED_CODE)";

            if (type == LV2PlugIn)
                out << " $(DRX_OUTDIR)/$(DRX_TARGET_LV2_MANIFEST_HELPER)";
            else if (type == VST3PlugIn)
                out << " $(DRX_OUTDIR)/$(DRX_TARGET_VST3_MANIFEST_HELPER)";
            else if (type == VST3Helper)
                out << " $(DRX_OBJDIR)/cxxfs.cmd";

            out << newLine;

            if (! packages.isEmpty())
            {
                out << "\t@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { echo >&2 \"pkg-config not installed. Please, install it.\"; exit 1; }" << newLine
                    << "\t@$(PKG_CONFIG) --print-errors";

                for (auto& pkg : packages)
                    out << " " << pkg;

                out << newLine;
            }

            out << "\t@echo Linking \"" << owner.projectName << " - " << getName() << "\"" << newLine
                << "\t-$(V_AT)mkdir -p $(DRX_BINDIR)" << newLine
                << "\t-$(V_AT)mkdir -p $(DRX_LIBDIR)" << newLine
                << "\t-$(V_AT)mkdir -p $(DRX_OUTDIR)" << newLine;

            if (type == VST3PlugIn)
                out << "\t-$(V_AT)mkdir -p $(DRX_OUTDIR)/$(DRX_VST3DIR)/$(DRX_VST3SUBDIR)" << newLine;
            else if (type == UnityPlugIn)
                out << "\t-$(V_AT)mkdir -p $(DRX_OUTDIR)/$(DRX_UNITYDIR)" << newLine;
            else if (type == LV2PlugIn)
                out << "\t-$(V_AT)mkdir -p $(DRX_OUTDIR)/$(DRX_LV2DIR)" << newLine;

            if (owner.projectType.isStaticLibrary() || type == SharedCodeTarget)
            {
                out << "\t$(V_AT)$(AR) -rcs " << getBuildProduct()
                    << " $(OBJECTS_" << getTargetVarName() << ")" << newLine;
            }
            else
            {
                out << "\t$(V_AT)$(CXX) -o " << getBuildProduct()
                    << " $(OBJECTS_" << getTargetVarName() << ") ";

                if (owner.shouldBuildTargetType (SharedCodeTarget))
                    out << "$(DRX_OUTDIR)/$(DRX_TARGET_SHARED_CODE) ";

                out << "$(DRX_LDFLAGS) $(shell cat $(DRX_OBJDIR)/execinfo.cmd) ";

                if (type == VST3Helper)
                    out << "$(shell cat $(DRX_OBJDIR)/cxxfs.cmd) ";

                if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle
                        || type == GUIApp || type == StandalonePlugIn)
                    out << "$(DRX_LDFLAGS_" << getTargetVarName() << ") ";

                out << "$(RESOURCES) $(TARGET_ARCH)" << newLine;
            }

            if (type == VST3PlugIn)
            {
                out << "\t-$(V_AT)mkdir -p $(DRX_OUTDIR)/$(DRX_VST3DIR)/Contents/Resources" << newLine
                    << "\t-$(V_AT)rm -f $(DRX_OUTDIR)/$(DRX_VST3DIR)/Contents/moduleinfo.json" << newLine
                    << "\t$(V_AT) $(DRX_OUTDIR)/$(DRX_TARGET_VST3_MANIFEST_HELPER) "
                       "-create "
                       "-version " << owner.project.getVersionString().quoted() << " "
                       "-path $(DRX_OUTDIR)/$(DRX_VST3DIR) "
                       "-output $(DRX_OUTDIR)/$(DRX_VST3DIR)/Contents/Resources/moduleinfo.json" << newLine
                    << "\t-$(V_AT)[ ! \"$(DRX_VST3DESTDIR)\" ] || (mkdir -p $(DRX_VST3DESTDIR) && cp -R $(DRX_COPYCMD_VST3))" << newLine;
            }
            else if (type == VSTPlugIn)
            {
                out << "\t-$(V_AT)[ ! \"$(DRX_VSTDESTDIR)\" ]  || (mkdir -p $(DRX_VSTDESTDIR)  && cp -R $(DRX_COPYCMD_VST))"  << newLine;
            }
            else if (type == UnityPlugIn)
            {
                auto scriptName = owner.getProject().getUnityScriptName();

                build_tools::RelativePath scriptPath (owner.getProject().getGeneratedCodeFolder().getChildFile (scriptName),
                                                      owner.getTargetFolder(),
                                                      build_tools::RelativePath::projectFolder);

                out << "\t-$(V_AT)cp " + scriptPath.toUnixStyle() + " $(DRX_OUTDIR)/$(DRX_UNITYDIR)" << newLine
                    << "\t-$(V_AT)[ ! \"$(DRX_UNITYDESTDIR)\" ] || (mkdir -p $(DRX_UNITYDESTDIR) && cp -R $(DRX_COPYCMD_UNITY_PLUGIN))" << newLine;
            }
            else if (type == LV2PlugIn)
            {
                out << "\t$(V_AT) $(DRX_OUTDIR)/$(DRX_TARGET_LV2_MANIFEST_HELPER) $(DRX_LV2_FULL_PATH)" << newLine
                    << "\t-$(V_AT)[ ! \"$(DRX_LV2DESTDIR)\" ] || (mkdir -p $(DRX_LV2DESTDIR) && cp -R $(DRX_COPYCMD_LV2_PLUGIN))" << newLine;
            }

            out << newLine;
        }

        const MakefileProjectExporter& owner;
    };

    //==============================================================================
    static Txt getDisplayName()        { return "Linux Makefile"; }
    static Txt getValueTreeTypeName()  { return "LINUX_MAKE"; }
    static Txt getTargetFolderName()   { return "LinuxMakefile"; }

    Identifier getExporterIdentifier() const override { return getValueTreeTypeName(); }

    static MakefileProjectExporter* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        if (settingsToUse.hasType (getValueTreeTypeName()))
            return new MakefileProjectExporter (projectToUse, settingsToUse);

        return nullptr;
    }

    //==============================================================================
    MakefileProjectExporter (Project& p, const ValueTree& t)
        : ProjectExporter (p, t),
          extraPkgConfigValue (settings, Ids::linuxExtraPkgConfig, getUndoManager())
    {
        name = getDisplayName();
        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderName());
    }

    //==============================================================================
    b8 canLaunchProject() override                        { return false; }
    b8 launchProject() override                           { return false; }
    b8 usesMMFiles() const override                       { return false; }
    b8 canCopeWithDuplicateFiles() override               { return false; }
    b8 supportsUserDefinedConfigurations() const override { return true; }

    b8 isXcode() const override                           { return false; }
    b8 isVisualStudio() const override                    { return false; }
    b8 isMakefile() const override                        { return true; }
    b8 isAndroidStudio() const override                   { return false; }

    b8 isAndroid() const override                         { return false; }
    b8 isWindows() const override                         { return false; }
    b8 isLinux() const override                           { return true; }
    b8 isOSX() const override                             { return false; }
    b8 isiOS() const override                             { return false; }

    Txt getNewLineString() const override                { return "\n"; }

    b8 supportsTargetType (build_tools::ProjectType::Target::Type type) const override
    {
        using Target = build_tools::ProjectType::Target;

        switch (type)
        {
            case Target::GUIApp:
            case Target::ConsoleApp:
            case Target::StaticLibrary:
            case Target::SharedCodeTarget:
            case Target::AggregateTarget:
            case Target::VSTPlugIn:
            case Target::VST3PlugIn:
            case Target::VST3Helper:
            case Target::StandalonePlugIn:
            case Target::DynamicLibrary:
            case Target::UnityPlugIn:
            case Target::LV2PlugIn:
            case Target::LV2Helper:
                return true;
            case Target::AAXPlugIn:
            case Target::AudioUnitPlugIn:
            case Target::AudioUnitv3PlugIn:
            case Target::unspecified:
            default:
                break;
        }

        return false;
    }

    z0 createExporterProperties (PropertyListBuilder& properties) override
    {
        properties.add (new TextPropertyComponent (extraPkgConfigValue, "pkg-config libraries", 8192, false),
                   "Extra pkg-config libraries for you application. Each package should be space separated.");
    }

    z0 initialiseDependencyPathValues() override
    {
        vstLegacyPathValueWrapper.init ({ settings, Ids::vstLegacyFolder, nullptr },
                                          getAppSettings().getStoredPath (Ids::vstLegacyPath, TargetOS::linux), TargetOS::linux);

        araPathValueWrapper.init ({ settings, Ids::araFolder, nullptr },
                                  getAppSettings().getStoredPath (Ids::araPath, TargetOS::linux), TargetOS::linux);
    }

    //==============================================================================
    b8 anyTargetIsSharedLibrary() const
    {
        for (auto* target : targets)
        {
            auto fileType = target->getTargetFileType();

            if (fileType == build_tools::ProjectType::Target::sharedLibraryOrDLL
             || fileType == build_tools::ProjectType::Target::pluginBundle)
                return true;
        }

        return false;
    }

    //==============================================================================
    z0 create (const OwnedArray<LibraryModule>&) const override
    {
        build_tools::writeStreamToFile (getTargetFolder().getChildFile ("Makefile"), [&] (MemoryOutputStream& mo)
        {
            mo.setNewLineString (getNewLineString());
            writeMakefile (mo);
        });

        if (project.shouldBuildVST3())
        {
            auto helperDir = getTargetFolder().getChildFile ("make_helpers");
            helperDir.createDirectory();
            build_tools::overwriteFileIfDifferentOrThrow (helperDir.getChildFile ("arch_detection.cpp"),
                                                          BinaryData::drx_runtime_arch_detection_cpp);
        }

        linuxSubprocessHelperProperties.deployLinuxSubprocessHelperSourceFilesIfNecessary();
    }

    //==============================================================================
    z0 addPlatformSpecificSettingsForProjectType (const build_tools::ProjectType&) override
    {
        linuxSubprocessHelperProperties.addToExtraSearchPathsIfNecessary();

        callForAllSupportedTargets ([this] (build_tools::ProjectType::Target::Type targetType)
                                    {
                                        targets.insert (targetType == build_tools::ProjectType::Target::AggregateTarget ? 0 : -1,
                                                        new MakefileTarget (targetType, *this));
                                    });

        // If you hit this assert, you tried to generate a project for an exporter
        // that does not support any of your targets!
        jassert (targets.size() > 0);
    }

private:
    ValueTreePropertyWithDefault extraPkgConfigValue;

    //==============================================================================
    StringPairArray getDefines (const BuildConfiguration& config) const
    {
        StringPairArray result;

        result.set ("LINUX", "1");

        if (config.isDebug())
        {
            result.set ("DEBUG", "1");
            result.set ("_DEBUG", "1");
        }
        else
        {
            result.set ("NDEBUG", "1");
        }

        result = mergePreprocessorDefs (result, getAllPreprocessorDefs (config, build_tools::ProjectType::Target::unspecified));

        return result;
    }

    std::vector<PackageDependency> getExtraPkgConfigPackages() const
    {
        auto packages = StringArray::fromTokens (extraPkgConfigValue.get().toString(), " ", "\"'");
        packages.removeEmptyStrings();

        return makePackageDependencies (packages);
    }

    std::vector<PackageDependency> getCompilePackages() const
    {
        auto packages = getLinuxPackages (PackageDependencyType::compile);
        const auto extra = getExtraPkgConfigPackages();
        packages.insert (packages.end(), extra.begin(), extra.end());

        return packages;
    }

    std::vector<PackageDependency> getLinkPackages() const
    {
        auto packages = getLinuxPackages (PackageDependencyType::link);
        const auto extra = getExtraPkgConfigPackages();
        packages.insert (packages.end(), extra.begin(), extra.end());

        return packages;
    }

    static StringArray getPackagesCommand (const std::vector<PackageDependency>& dependencies)
    {
        StringArray packages;

        for (const auto& d : dependencies)
        {
            if (d.fallback.has_value())
            {
                packages.add (Txt { "$(shell ($(PKG_CONFIG) --exists %VALUE% && echo %VALUE%) || echo %OR_ELSE%)" }
                                  .replace ("%VALUE%", d.dependency)
                                  .replace ("%OR_ELSE%", *d.fallback));
            }
            else
            {
                packages.add (d.dependency);
            }
        }

        return packages;
    }

    Txt getPreprocessorPkgConfigFlags() const
    {
        auto compilePackages = getPackagesCommand (getCompilePackages());

        if (compilePackages.size() > 0)
            return "$(shell $(PKG_CONFIG) --cflags " + compilePackages.joinIntoString (" ") + ")";

        return {};
    }

    Txt getLinkerPkgConfigFlags() const
    {
        auto linkPackages = getPackagesCommand (getLinkPackages());

        if (linkPackages.size() > 0)
            return "$(shell $(PKG_CONFIG) --libs " + linkPackages.joinIntoString (" ") + ")";

        return {};
    }

    StringArray getCPreprocessorFlags (const BuildConfiguration&) const
    {
        StringArray result;

        if (linuxLibs.contains ("pthread"))
            result.add ("-pthread");

        return result;
    }

    StringArray getCFlags (const BuildConfiguration& config) const
    {
        StringArray result;

        if (anyTargetIsSharedLibrary())
            result.add ("-fPIC");

        if (config.isDebug())
        {
            result.add ("-g");
            result.add ("-ggdb");
        }

        result.add ("-O" + config.getGCCOptimisationFlag());

        if (config.isLinkTimeOptimisationEnabled())
            result.add ("-flto");

        for (auto& recommended : config.getRecommendedCompilerWarningFlags().common)
            result.add (recommended);

        auto extra = replacePreprocessorTokens (config, config.getAllCompilerFlagsString()).trim();

        if (extra.isNotEmpty())
            result.add (extra);

        return result;
    }

    StringArray getCXXFlags (const BuildConfiguration& config) const
    {
        StringArray result;

        for (auto& recommended : config.getRecommendedCompilerWarningFlags().cpp)
            result.add (recommended);

        auto cppStandard = project.getCppStandardString();

        if (cppStandard == "latest")
            cppStandard = project.getLatestNumberedCppStandardString();

        result.add ("-std=" + Txt (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard);

        return result;
    }

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        searchPaths = getCleanedStringArray (searchPaths);

        StringArray result;

        for (auto& path : searchPaths)
            result.add (build_tools::unixStylePath (replacePreprocessorTokens (config, path)));

        return result;
    }

    StringArray getLibraryNames (const BuildConfiguration& config) const
    {
        StringArray result (linuxLibs);

        auto libraries = StringArray::fromTokens (getExternalLibrariesString(), ";", "\"'");
        libraries.removeEmptyStrings();

        for (auto& lib : libraries)
            result.add (replacePreprocessorTokens (config, lib).trim());

        return result;
    }

    StringArray getLibrarySearchPaths (const BuildConfiguration& config) const
    {
        auto result = getSearchPathsFromString (config.getLibrarySearchPathString());

        for (auto path : moduleLibSearchPaths)
            result.add (path + "/" + config.getModuleLibraryArchName());

        return result;
    }

    StringArray getLinkerFlags (const BuildConfiguration& config) const
    {
        auto result = makefileExtraLinkerFlags;

        result.add ("-fvisibility=hidden");

        if (config.isLinkTimeOptimisationEnabled())
            result.add ("-flto");

        const auto extraFlags = config.getAllLinkerFlagsString().trim();

        if (extraFlags.isNotEmpty())
            result.add (replacePreprocessorTokens (config, extraFlags));

        return result;
    }

    //==============================================================================
    z0 writeDefineFlags (OutputStream& out, const MakeBuildConfiguration& config) const
    {
        out << createGCCPreprocessorFlags (mergePreprocessorDefs (getDefines (config), getAllPreprocessorDefs (config, build_tools::ProjectType::Target::unspecified)));
    }

    z0 writePkgConfigFlags (OutputStream& out) const
    {
        auto flags = getPreprocessorPkgConfigFlags();

        if (flags.isNotEmpty())
            out << " " << flags;
    }

    z0 writeCPreprocessorFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        auto flags = getCPreprocessorFlags (config);

        if (! flags.isEmpty())
            out << " " << flags.joinIntoString (" ");
    }

    z0 writeHeaderPathFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        for (auto& path : getHeaderSearchPaths (config))
            out << " -I" << escapeQuotesAndSpaces (path).replace ("~", "$(HOME)");
    }

    z0 writeCppFlags (OutputStream& out, const MakeBuildConfiguration& config) const
    {
        out << "  DRX_CPPFLAGS := $(DEPFLAGS)";
        writeDefineFlags (out, config);
        writePkgConfigFlags (out);
        writeCPreprocessorFlags (out, config);
        writeHeaderPathFlags (out, config);
        out << " $(CPPFLAGS)" << newLine;
    }

    z0 writeLinkerFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        out << "  DRX_LDFLAGS += $(TARGET_ARCH) -L$(DRX_BINDIR) -L$(DRX_LIBDIR)";

        for (auto path : getLibrarySearchPaths (config))
            out << " -L" << escapeQuotesAndSpaces (path).replace ("~", "$(HOME)");

        auto pkgConfigFlags = getLinkerPkgConfigFlags();

        if (pkgConfigFlags.isNotEmpty())
            out << " " << getLinkerPkgConfigFlags();

        auto linkerFlags = getLinkerFlags (config).joinIntoString (" ");

        if (linkerFlags.isNotEmpty())
            out << " " << linkerFlags;

        for (auto& libName : getLibraryNames (config))
            out << " -l" << libName;

        out << " $(LDFLAGS)" << newLine;
    }

    z0 writeLinesForAggregateTarget (OutputStream& out) const
    {
        const auto isPartOfAggregate = [&] (const MakefileTarget* x)
        {
            return x != nullptr
                   && x->type != build_tools::ProjectType::Target::AggregateTarget
                   && x->type != build_tools::ProjectType::Target::SharedCodeTarget;
        };

        std::vector<MakefileTarget*> dependencies;
        std::copy_if (targets.begin(), targets.end(), std::back_inserter (dependencies), isPartOfAggregate);

        out << "all :";

        for (const auto& d : dependencies)
            out << ' ' << d->getPhonyName();

        out << newLine << newLine;

        for (const auto& d : dependencies)
            out << d->getPhonyName() << " : " << d->getBuildProduct() << newLine;

        out << newLine << newLine;
    }

    z0 writeLinesForTarget (OutputStream& out, const StringArray& packages, MakefileTarget& target) const
    {
        if (target.type == build_tools::ProjectType::Target::AggregateTarget)
        {
            writeLinesForAggregateTarget (out);
        }
        else
        {
            if (! getProject().isAudioPluginProject())
                out << "all : " << target.getBuildProduct() << newLine << newLine;

            target.writeTargetLine (out, packages);
        }
    }

    z0 writeTargetLines (OutputStream& out, const StringArray& packages) const
    {
        for (const auto& target : targets)
            if (target != nullptr)
                writeLinesForTarget (out, packages, *target);
    }

    z0 writeConfig (OutputStream& out, const MakeBuildConfiguration& config) const
    {
        Txt buildDirName ("build");
        auto intermediatesDirName = buildDirName + "/intermediate/" + config.getName();
        auto outputDir = buildDirName;

        if (config.getTargetBinaryRelativePathString().isNotEmpty())
        {
            build_tools::RelativePath binaryPath (config.getTargetBinaryRelativePathString(), build_tools::RelativePath::projectFolder);
            outputDir = binaryPath.rebased (projectFolder, getTargetFolder(), build_tools::RelativePath::buildTargetFolder).toUnixStyle();
        }

        out << "ifeq ($(CONFIG)," << escapeQuotesAndSpaces (config.getName()) << ")" << newLine
            << "  DRX_BINDIR := " << escapeQuotesAndSpaces (buildDirName)           << newLine
            << "  DRX_LIBDIR := " << escapeQuotesAndSpaces (buildDirName)           << newLine
            << "  DRX_OBJDIR := " << escapeQuotesAndSpaces (intermediatesDirName)   << newLine
            << "  DRX_OUTDIR := " << escapeQuotesAndSpaces (outputDir)              << newLine
            << newLine
            << "  ifeq ($(TARGET_ARCH),)"                                   << newLine
            << "    TARGET_ARCH := " << getArchFlags (config)               << newLine
            << "  endif"                                                    << newLine
            << newLine;

        writeCppFlags (out, config);

        for (auto target : targets)
        {
            auto lines = target->getTargetSettings (config);

            if (lines.size() > 0)
                out << "  " << lines.joinIntoString ("\n  ") << newLine;

            out << newLine;
        }

        out << "  DRX_CFLAGS += $(DRX_CPPFLAGS) $(TARGET_ARCH)";

        auto cflags = getCFlags (config).joinIntoString (" ");

        if (cflags.isNotEmpty())
            out << " " << cflags;

        out << " $(CFLAGS)" << newLine;

        out << "  DRX_CXXFLAGS += $(DRX_CFLAGS)";

        auto cxxflags = getCXXFlags (config).joinIntoString (" ");

        if (cxxflags.isNotEmpty())
            out << " " << cxxflags;

        out << " $(CXXFLAGS)" << newLine;

        writeLinkerFlags (out, config);

        out << newLine;

        const auto preBuildDirectory = [&]() -> Txt
        {
            if (linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
            {
                using LSHP = LinuxSubprocessHelperProperties;
                const auto dataSource = linuxSubprocessHelperProperties.getLinuxSubprocessHelperBinaryDataSource();

                if (auto preBuildDir = LSHP::getParentDirectoryRelativeToBuildTargetFolder (dataSource))
                    return " " + *preBuildDir;
            }

            return "";
        }();

        const auto targetsToClean = [&]
        {
            StringArray result;

            for (const auto& target : targets)
                if (target->type != build_tools::ProjectType::Target::AggregateTarget)
                    result.add (target->getBuildProduct());

            return result;
        }();

        out << "  CLEANCMD = rm -rf " << targetsToClean.joinIntoString (" ") << " $(DRX_OBJDIR)" << preBuildDirectory << newLine
            << "endif" << newLine
            << newLine;
    }

    z0 writeIncludeLines (OutputStream& out) const
    {
        auto n = targets.size();

        for (i32 i = 0; i < n; ++i)
        {
            if (auto* target = targets.getUnchecked (i))
            {
                if (target->type == build_tools::ProjectType::Target::AggregateTarget)
                    continue;

                out << "-include $(OBJECTS_" << target->getTargetVarName()
                    << ":%.o=%.d)" << newLine;
            }
        }
    }

    static Txt getCompilerFlagSchemeVariableName (const Txt& schemeName)   { return "DRX_COMPILERFLAGSCHEME_" + schemeName; }

    std::vector<std::pair<File, Txt>> findAllFilesToCompile (const Project::Item& projectItem) const
    {
        std::vector<std::pair<File, Txt>> results;

        if (projectItem.isGroup())
        {
            for (i32 i = 0; i < projectItem.getNumChildren(); ++i)
            {
                auto inner = findAllFilesToCompile (projectItem.getChild (i));
                results.insert (results.end(),
                                std::make_move_iterator (inner.cbegin()),
                                std::make_move_iterator (inner.cend()));
            }
        }
        else
        {
            if (projectItem.shouldBeCompiled())
            {
                auto f = projectItem.getFile();

                if (shouldFileBeCompiledByDefault (f))
                {
                    auto scheme = projectItem.getCompilerFlagSchemeString();
                    auto flags = getCompilerFlagsForProjectItem (projectItem);

                    if (scheme.isNotEmpty() && flags.isNotEmpty())
                        results.emplace_back (f, scheme);
                    else
                        results.emplace_back (f, Txt{});
                }
            }
        }

        return results;
    }

    z0 writeCompilerFlagSchemes (OutputStream& out, const std::vector<std::pair<File, Txt>>& filesToCompile) const
    {
        std::set<Txt> schemesToWrite;

        for (const auto& pair : filesToCompile)
            if (pair.second.isNotEmpty())
                schemesToWrite.insert (pair.second);

        if (schemesToWrite.empty())
            return;

        for (const auto& s : schemesToWrite)
            if (const auto flags = getCompilerFlagsForFileCompilerFlagScheme (s); flags.isNotEmpty())
                out << getCompilerFlagSchemeVariableName (s) << " := " << flags << newLine;

        out << newLine;
    }

    /*  These targets are responsible for building the drx_linux_subprocess_helper, the
        drx_simple_binary_builder, and then using the binary builder to create embeddable .h and .cpp
        files from the linux subprocess helper.
    */
    z0 writeSubprocessHelperTargets (OutputStream& out) const
    {
        using LSHP = LinuxSubprocessHelperProperties;

        const auto ensureDirs = [] (auto& outStream, std::vector<Txt> dirs)
        {
            for (const auto& dir : dirs)
                outStream << "\t-$(V_AT)mkdir -p " << dir << newLine;
        };

        const auto makeTarget = [&ensureDirs] (auto& outStream, Txt input, Txt output)
        {
            const auto isObjectTarget = output.endsWith (".o");
            const auto isSourceInput  = input.endsWith (".cpp");

            const auto targetOutput = isObjectTarget ? "$(DRX_OBJDIR)/" + output : output;

            outStream << (isObjectTarget ? "$(DRX_OBJDIR)/" : "") << output << ": " << input << newLine;

            const auto createBuildTargetRelative = [] (auto path)
            {
                return build_tools::RelativePath { path, build_tools::RelativePath::buildTargetFolder };
            };

            if (isObjectTarget)
                ensureDirs (outStream, { "$(DRX_OBJDIR)" });
            else if (auto outputParentFolder = LSHP::getParentDirectoryRelativeToBuildTargetFolder (createBuildTargetRelative (output)))
                ensureDirs (outStream, { *outputParentFolder });

            outStream << (isObjectTarget ? "\t@echo \"Compiling " : "\t@echo \"Linking ")
                      << (isObjectTarget ? input : output) << "\"" << newLine
                      << "\t$(V_AT)$(CXX) $(DRX_CXXFLAGS) -o " << targetOutput.quoted()
                      << " " << (isSourceInput ? "-c \"$<\"" : input.quoted());

            if (! isObjectTarget)
                outStream << " $(DRX_LDFLAGS)";

            outStream << " $(TARGET_ARCH)" << newLine << newLine;

            return targetOutput;
        };

        const auto subprocessHelperSource = linuxSubprocessHelperProperties.getLinuxSubprocessHelperSource();

        const auto subprocessHelperObj = makeTarget (out,
                                                     subprocessHelperSource.toUnixStyle(),
                                                     getObjectFileFor (subprocessHelperSource));

        const auto subprocessHelperPath = makeTarget (out,
                                                      subprocessHelperObj,
                                                      "$(DRX_BINDIR)/" + LSHP::getBinaryNameFromSource (subprocessHelperSource));

        const auto binaryBuilderSource = linuxSubprocessHelperProperties.getSimpleBinaryBuilderSource();

        const auto binaryBuilderObj = makeTarget (out,
                                                 binaryBuilderSource.toUnixStyle(),
                                                 getObjectFileFor (binaryBuilderSource));

        const auto binaryBuilderPath = makeTarget (out,
                                                   binaryBuilderObj,
                                                   "$(DRX_BINDIR)/" + LSHP::getBinaryNameFromSource (binaryBuilderSource));

        const auto binaryDataSource = linuxSubprocessHelperProperties.getLinuxSubprocessHelperBinaryDataSource();
        jassert (binaryDataSource.getRoot() == build_tools::RelativePath::buildTargetFolder);

        out << binaryDataSource.toUnixStyle() << ": " << subprocessHelperPath
                                              << " " << binaryBuilderPath
                                              << newLine;

        const auto binarySourceDir = [&]() -> Txt
        {
            if (const auto p = LSHP::getParentDirectoryRelativeToBuildTargetFolder (binaryDataSource))
                return *p;

            return ".";
        }();

        out << "\t$(V_AT)" << binaryBuilderPath.quoted() << " " << subprocessHelperPath.quoted()
            << " " << binarySourceDir.quoted() << " " << binaryDataSource.getFileNameWithoutExtension().quoted()
            << " LinuxSubprocessHelperBinaryData" << newLine;

        out << newLine;
    }

    z0 writeMakefile (OutputStream& out) const
    {
        out << "# Automatically generated makefile, created by the Projucer"                                     << newLine
            << "# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
            << newLine;

        out << "# build with \"V=1\" for verbose builds" << newLine
            << "ifeq ($(V), 1)"                          << newLine
            << "V_AT ="                                  << newLine
            << "else"                                    << newLine
            << "V_AT = @"                                << newLine
            << "endif"                                   << newLine
            << newLine;

        out << "# (this disables dependency generation if multiple architectures are set)" << newLine
            << "DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)"                       << newLine
            << newLine;

        out << "ifndef PKG_CONFIG"       << newLine
            << "  PKG_CONFIG=pkg-config" << newLine
            << "endif"                   << newLine
            << newLine;

        out << "ifndef STRIP"  << newLine
            << "  STRIP=strip" << newLine
            << "endif"         << newLine
            << newLine;

        out << "ifndef AR" << newLine
            << "  AR=ar"   << newLine
            << "endif"     << newLine
            << newLine;

        out << "ifndef CONFIG"                                                       << newLine
            << "  CONFIG=" << escapeQuotesAndSpaces (getConfiguration(0)->getName()) << newLine
            << "endif"                                                               << newLine
            << newLine;

        out << "DRX_ARCH_LABEL := $(shell uname -m)" << newLine
            << newLine;

        for (ConstConfigIterator config (*this); config.next();)
            writeConfig (out, dynamic_cast<const MakeBuildConfiguration&> (*config));

        std::vector<std::pair<File, Txt>> filesToCompile;

        for (i32 i = 0; i < getAllGroups().size(); ++i)
        {
            auto group = findAllFilesToCompile (getAllGroups().getReference (i));
            filesToCompile.insert (filesToCompile.end(),
                                   std::make_move_iterator (group.cbegin()),
                                   std::make_move_iterator (group.cend()));
        }

        writeCompilerFlagSchemes (out, filesToCompile);

        const auto getFilesForTarget = [this] (const std::vector<std::pair<File, Txt>>& files,
                                               MakefileTarget* target,
                                               const Project& p)
        {
            std::vector<std::pair<build_tools::RelativePath, Txt>> targetFiles;

            auto targetType = (p.isAudioPluginProject() ? target->type : MakefileTarget::SharedCodeTarget);

            for (auto& [path, flags] : files)
            {
                if (p.getTargetTypeFromFilePath (path, true) == targetType)
                {
                    targetFiles.emplace_back (build_tools::RelativePath { path,
                                                                          getTargetFolder(),
                                                                          build_tools::RelativePath::buildTargetFolder },
                                              flags);
                }
            }

            if ((      targetType == MakefileTarget::SharedCodeTarget
                    || targetType == MakefileTarget::StaticLibrary
                    || targetType == MakefileTarget::DynamicLibrary)
                && linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
            {
                targetFiles.emplace_back (linuxSubprocessHelperProperties.getLinuxSubprocessHelperBinaryDataSource(), "");
            }

            if (targetType == MakefileTarget::LV2Helper)
            {
                targetFiles.emplace_back (getLV2HelperProgramSource().rebased (projectFolder,
                                                                               getTargetFolder(),
                                                                               build_tools::RelativePath::buildTargetFolder),
                                          Txt{});
            }
            else if (targetType == MakefileTarget::VST3Helper)
            {
                targetFiles.emplace_back (getVST3HelperProgramSource().rebased (projectFolder,
                                                                                getTargetFolder(),
                                                                                build_tools::RelativePath::buildTargetFolder),
                                          Txt{});
            }

            return targetFiles;
        };

        for (auto target : targets)
            target->writeObjects (out, getFilesForTarget (filesToCompile, target, project));

        out << getPhonyTargetLine() << newLine << newLine;

        writeTargetLines (out, getPackagesCommand (getLinkPackages()));

        for (auto target : targets)
            target->addFiles (out, getFilesForTarget (filesToCompile, target, project));

        // libexecinfo is a separate library on BSD
        out << "$(DRX_OBJDIR)/execinfo.cmd:" << newLine
            << "\t-$(V_AT)mkdir -p $(@D)" << newLine
            << "\t-@if [ -z \"$(V_AT)\" ]; then echo \"Checking if we need to link libexecinfo\"; fi" << newLine
            << "\t$(V_AT)printf \"i32 main() { return 0; }\" | $(CXX) -x c++ -o $(@D)/execinfo.x -lexecinfo - >/dev/null 2>&1 && printf -- \"-lexecinfo\" > \"$@\" || touch \"$@\"" << newLine
            << newLine;

        // stdc++fs is only needed for some compilers
        out << "$(DRX_OBJDIR)/cxxfs.cmd:" << newLine
            << "\t-$(V_AT)mkdir -p $(@D)" << newLine
            << "\t-@if [ -z \"$(V_AT)\" ]; then echo \"Checking if we need to link stdc++fs\"; fi" << newLine
            << "\t$(V_AT)printf \"i32 main() { return 0; }\" | $(CXX) -x c++ -o $(@D)/cxxfs.x -lstdc++fs - >/dev/null 2>&1 && printf -- \"-lstdc++fs\" > \"$@\" || touch \"$@\"" << newLine
            << newLine;

        if (linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
            writeSubprocessHelperTargets (out);

        out << "clean:"                           << newLine
            << "\t@echo Cleaning " << projectName << newLine
            << "\t$(V_AT)$(CLEANCMD)"             << newLine
            << newLine;

        out << "strip:"                                                            << newLine
            << "\t@echo Stripping " << projectName                                 << newLine;

        for (const auto& target : targets)
        {
            if (target->type != build_tools::ProjectType::Target::AggregateTarget
                && target->type != build_tools::ProjectType::Target::SharedCodeTarget)
            {
                out << "\t-$(V_AT)$(STRIP) --strip-unneeded " << target->getBuildProduct() << newLine;
            }
        }

        out << newLine;

        writeIncludeLines (out);
    }

    Txt getArchFlags (const BuildConfiguration& config) const
    {
        if (auto* makeConfig = dynamic_cast<const MakeBuildConfiguration*> (&config))
            return makeConfig->getArchitectureTypeString();

        return "-march=native";
    }

    Txt getObjectFileFor (const build_tools::RelativePath& file) const
    {
        return file.getFileNameWithoutExtension()
                + "_" + Txt::toHexString (file.toUnixStyle().hashCode()) + ".o";
    }

    Txt getPhonyTargetLine() const
    {
        MemoryOutputStream phonyTargetLine;
        phonyTargetLine.setNewLineString (getNewLineString());

        phonyTargetLine << ".PHONY: clean all strip";

        if (! getProject().isAudioPluginProject())
            return phonyTargetLine.toString();

        for (auto target : targets)
        {
            if (target->type != build_tools::ProjectType::Target::SharedCodeTarget
                && target->type != build_tools::ProjectType::Target::AggregateTarget)
            {
                phonyTargetLine << " " << target->getPhonyName();
            }
        }

        return phonyTargetLine.toString();
    }

    OwnedArray<MakefileTarget> targets;

    DRX_DECLARE_NON_COPYABLE (MakefileProjectExporter)
};
