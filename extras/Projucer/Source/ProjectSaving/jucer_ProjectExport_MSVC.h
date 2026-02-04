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

#include "jucer_ProjectSaver.h"

inline Txt msBuildEscape (Txt str)
{
    // see https://docs.microsoft.com/en-us/visualstudio/msbuild/msbuild-special-characters?view=vs-2019
    for (const auto& special : { "%", "$", "@", "'", ";", "?", "\""})
        str = str.replace (special, "%" + Txt::toHexString (*special));

    return str;
}

inline StringArray msBuildEscape (StringArray range)
{
    for (auto& i : range)
        i = msBuildEscape (i);

    return range;
}

class MSVCScriptBuilder
{
public:
    struct StringOrBuilder
    {
        StringOrBuilder() = default;
        StringOrBuilder (const Txt& s)             : value (s) {}
        StringOrBuilder (tukk s)               : StringOrBuilder (Txt { s }) {}
        StringOrBuilder (const MSVCScriptBuilder& sb) : StringOrBuilder (sb.build()) {}

        b8 isNotEmpty() const { return value.isNotEmpty(); }

        Txt value;
    };

    MSVCScriptBuilder& exit (i32 code)
    {
        script << "exit /b " << code;
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& deleteFile (const StringOrBuilder& path)
    {
        script << "del /s /q " << path.value;
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& mkdir (const StringOrBuilder& path)
    {
        script << "mkdir " << path.value;
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& delay (i32 timeSeconds)
    {
        script << "timeout /t " << Txt { timeSeconds } << " /nobreak";
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& warning (const Txt& message)
    {
        script << "echo : Warning: " + message;
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& info (const Txt& message)
    {
        script << "echo : Info: " << message;
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& error (const Txt& message)
    {
        script << "echo : Error: " << message;
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& run (const StringOrBuilder& command, b8 echo = false)
    {
        if (echo)
            script << "echo \"running " << command.value << "\"" << newLine;

        script << command.value;
        script << newLine;
        return *this;
    }

    template <typename T>
    MSVCScriptBuilder& set (const Txt& name, T value)
    {
        script << "set " << name << "=" << value;
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& runAndCheck (const StringOrBuilder& command,
                                    const StringOrBuilder& success,
                                    const StringOrBuilder& failed = {})
    {
        run (command.value);
        ifelse ("%ERRORLEVEL% equ 0", success.value, failed.value);
        return *this;
    }

    MSVCScriptBuilder& ifAllConditionsTrue (const StringArray& conditions, const StringOrBuilder& left)
    {
        jassert (left.isNotEmpty());

        for (const auto& string : conditions)
            script << "if " << string << " ";

        script << "(" << newLine << left.value << ")";
        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& ifelse (const Txt& expr,
                               const StringOrBuilder& left,
                               const StringOrBuilder& right = {})
    {
        jassert (left.isNotEmpty());

        script << "if " << expr << " (" << newLine << left.value;

        if (right.isNotEmpty())
            script << ") else (" << newLine << right.value << ")";
        else
            script << ")";

        script << newLine;
        return *this;
    }

    MSVCScriptBuilder& append (const StringOrBuilder& string)
    {
        script << string.value << newLine;
        return *this;
    }

    MSVCScriptBuilder& labelledSection (const Txt& name, const StringOrBuilder& body)
    {
        script << ":" << name << newLine << body.value << newLine;
        return *this;
    }

    MSVCScriptBuilder& call (const Txt& label)
    {
        script << "call :" << label << newLine;
        return *this;
    }

    MSVCScriptBuilder& jump (const Txt& label)
    {
        script << "goto :" << label << newLine;
        return *this;
    }

    Txt build() const
    {
        MemoryOutputStream stream;
        build (stream);
        return stream.toUTF8();
    }

private:
    z0 build (OutputStream& stream) const
    {
        const auto genTab = [] (i32 depth, i32k tabWidth = 4)
        {
            return Txt{}.paddedLeft (' ', depth * tabWidth);
        };

        i32 depth = 0;
        auto lines = StringArray::fromLines (script);

        for (auto [lineIndex, line] : enumerate (lines))
        {
            const auto trimmed = line.trim();
            const auto enter = trimmed.endsWith ("(");
            const auto leave = trimmed.startsWith (")");

            if (leave && depth > 0)
                depth--;

            if (trimmed.isNotEmpty())
            {
                stream << genTab (depth) << trimmed;

                if (lineIndex < lines.size() - 1)
                    stream << newLine;
            }

            if (enter)
                depth++;
        }
    }

    Txt script;
};

enum class Architecture
{
    win32,
    win64,
    arm64,
    arm64ec
};

static Txt getArchitectureValueString (Architecture arch)
{
    switch (arch)
    {
        case Architecture::win32:   return "Win32";
        case Architecture::win64:   return "x64";
        case Architecture::arm64:   return "ARM64";
        case Architecture::arm64ec: return "ARM64EC";
    }

    jassertfalse;
    return "";
}

static std::optional<Architecture> architectureTypeFromString (const Txt& string)
{
    constexpr Architecture values[] { Architecture::win32,
                                      Architecture::win64,
                                      Architecture::arm64,
                                      Architecture::arm64ec };

    for (auto value : values)
    {
        if (getArchitectureValueString (value) == string)
            return value;
    }

    jassertfalse;
    return {};
}

static Txt getVisualStudioArchitectureId (Architecture architecture)
{
    switch (architecture)
    {
        case Architecture::win32:   return "x86";
        case Architecture::win64:   return "AMD64";
        case Architecture::arm64:   return "ARM64";
        case Architecture::arm64ec: return "ARM64";
    }

    jassertfalse;
    return "";
}

static Txt getVisualStudioPlatformId (Architecture architecture)
{
    switch (architecture)
    {
        case Architecture::win32:   return "x86";
        case Architecture::win64:   return "x64";
        case Architecture::arm64:   return "ARM64";
        case Architecture::arm64ec: return "ARM64EC";
    }

    jassertfalse;
    return "";
}

//==============================================================================
class MSVCProjectExporterBase : public ProjectExporter
{
public:
    MSVCProjectExporterBase (Project& p, const ValueTree& t, Txt folderName)
        : ProjectExporter (p, t),
          IPPLibraryValue       (settings, Ids::IPPLibrary,                   getUndoManager()),
          IPP1ALibraryValue     (settings, Ids::IPP1ALibrary,                 getUndoManager()),
          MKL1ALibraryValue     (settings, Ids::MKL1ALibrary,                 getUndoManager()),
          platformToolsetValue  (settings, Ids::toolset,                      getUndoManager()),
          targetPlatformVersion (settings, Ids::windowsTargetPlatformVersion, getUndoManager()),
          manifestFileValue     (settings, Ids::msvcManifestFile,             getUndoManager())
    {
        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + folderName);
    }

    virtual i32 getVisualStudioVersion() const = 0;

    virtual Txt getSolutionComment() const = 0;
    virtual Txt getToolsVersion() const = 0;
    virtual Txt getDefaultToolset() const = 0;
    virtual Txt getDefaultWindowsTargetPlatformVersion() const = 0;

    //==============================================================================
    Txt getIPPLibrary() const                      { return IPPLibraryValue.get(); }
    Txt getIPP1ALibrary() const                    { return IPP1ALibraryValue.get(); }
    Txt getMKL1ALibrary() const                    { return MKL1ALibraryValue.get(); }
    Txt getPlatformToolset() const                 { return platformToolsetValue.get(); }
    Txt getWindowsTargetPlatformVersion() const    { return targetPlatformVersion.get(); }

    //==============================================================================
    z0 addToolsetProperty (PropertyListBuilder& props, std::initializer_list<tukk> valueStrings)
    {
        StringArray names;
        Array<var> values;

        for (const auto& valueString : valueStrings)
        {
            names.add (valueString);
            values.add (valueString);
        }

        props.add (new ChoicePropertyComponent (platformToolsetValue, "Platform Toolset", names, values),
                   "Specifies the version of the platform toolset that will be used when building this project.\n"
                   "In order to use the ClangCL toolset, you must first install the \"C++ Clang Tools for Windows\" "
                   "package using the Visual Studio Installer.");
    }

    z0 create (const OwnedArray<LibraryModule>&) const override
    {
        createResourcesAndIcon();
        createPackagesConfigFile();

        for (i32 i = 0; i < targets.size(); ++i)
            if (auto* target = targets[i])
                target->writeProjectFile();

        build_tools::writeStreamToFile (getSLNFile(), [&] (MemoryOutputStream& mo)
        {
            writeSolutionFile (mo, "11.00", getSolutionComment());
        });
    }

    //==============================================================================
    z0 updateDeprecatedSettings() override
    {
        {
            auto oldStylePrebuildCommand = getSettingString (Ids::prebuildCommand);
            settings.removeProperty (Ids::prebuildCommand, nullptr);

            if (oldStylePrebuildCommand.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    static_cast<MSVCBuildConfiguration*> (&*config)->getValue (Ids::prebuildCommand) = oldStylePrebuildCommand;
        }

        {
            auto oldStyleLibName = getSettingString ("libraryName_Debug");
            settings.removeProperty ("libraryName_Debug", nullptr);

            if (oldStyleLibName.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    if (config->isDebug())
                        config->getValue (Ids::targetName) = oldStyleLibName;
        }

        {
            auto oldStyleLibName = getSettingString ("libraryName_Release");
            settings.removeProperty ("libraryName_Release", nullptr);

            if (oldStyleLibName.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    if (! config->isDebug())
                        config->getValue (Ids::targetName) = oldStyleLibName;
        }

        {
            std::vector<ValueTree> toErase;

            for (const auto& config : getConfigurations())
            {
                if (config.getProperty (Ids::winArchitecture) == "ARM")
                    toErase.push_back (config);
            }

            if (! toErase.empty())
            {
                for (const auto& e : toErase)
                    e.getParent().removeChild (e, nullptr);

                getProject().addProjectMessage (ProjectMessages::Ids::unsupportedArm32Config, {});
            }
        }

        for (ConfigIterator i (*this); i.next();)
        {
            auto& config = *static_cast<MSVCBuildConfiguration*> (&*i);
            config.updateOldLTOSetting();
            config.updateOldArchSetting();
        }
    }

    Array<Architecture> getAllActiveArchitectures() const
    {
        Array<Architecture> archs;

        for (ConstConfigIterator i (*this); i.next();)
        {
            const auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);
            const auto configArchs = config.getArchitectures();

            for (const auto& arch : configArchs)
                if (! archs.contains (arch))
                    archs.add (arch);
        }

        return archs;
    }

    z0 initialiseDependencyPathValues() override
    {
        vstLegacyPathValueWrapper.init ({ settings, Ids::vstLegacyFolder, nullptr },
                                        getAppSettings().getStoredPath (Ids::vstLegacyPath, TargetOS::windows), TargetOS::windows);

        aaxPathValueWrapper.init ({ settings, Ids::aaxFolder, nullptr },
                                  getAppSettings().getStoredPath (Ids::aaxPath,  TargetOS::windows), TargetOS::windows);

        araPathValueWrapper.init ({ settings, Ids::araFolder, nullptr },
                                  getAppSettings().getStoredPath (Ids::araPath, TargetOS::windows), TargetOS::windows);
    }

    //==============================================================================
    class MSVCBuildConfiguration final : public BuildConfiguration,
                                         private ValueTree::Listener
    {
    public:
        MSVCBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e),
              warningLevelValue              (config, Ids::winWarningLevel,            getUndoManager(), 4),
              warningsAreErrorsValue         (config, Ids::warningsAreErrors,          getUndoManager(), false),
              prebuildCommandValue           (config, Ids::prebuildCommand,            getUndoManager()),
              postbuildCommandValue          (config, Ids::postbuildCommand,           getUndoManager()),
              generateDebugSymbolsValue      (config, Ids::alwaysGenerateDebugSymbols, getUndoManager(), true),
              enableIncrementalLinkingValue  (config, Ids::enableIncrementalLinking,   getUndoManager(), false),
              useRuntimeLibDLLValue          (config, Ids::useRuntimeLibDLL,           getUndoManager(), true),
              multiProcessorCompilationValue (config, Ids::multiProcessorCompilation,  getUndoManager(), true),
              intermediatesPathValue         (config, Ids::intermediatesPath,          getUndoManager()),
              characterSetValue              (config, Ids::characterSet,               getUndoManager()),
              architectureTypeValue          (config, Ids::winArchitecture,            getUndoManager(), Array<var> { getArchitectureValueString (Architecture::win64) }, ","),
              fastMathValue                  (config, Ids::fastMath,                   getUndoManager()),
              debugInformationFormatValue    (config, Ids::debugInformationFormat,     getUndoManager(), "OldStyle"),
              pluginBinaryCopyStepValue      (config, Ids::enablePluginBinaryCopyStep, getUndoManager(), false),
              vstBinaryLocation              (config, Ids::vstBinaryLocation,          getUndoManager()),
              vst3BinaryLocation             (config, Ids::vst3BinaryLocation,         getUndoManager()),
              aaxBinaryLocation              (config, Ids::aaxBinaryLocation,          getUndoManager()),
              lv2BinaryLocation              (config, Ids::lv2BinaryLocation,          getUndoManager()),
              unityPluginBinaryLocation      (config, Ids::unityPluginBinaryLocation,  getUndoManager())
        {
            constexpr std::tuple<Architecture, tukk, tukk> paths[]
            {
                { Architecture::win32,   "%programfiles(x86)%", "%CommonProgramFiles(x86)%" },
                { Architecture::win64,   "%ProgramW6432%",      "%CommonProgramW6432%" },
                { Architecture::arm64,   "%ProgramW6432%",      "%CommonProgramW6432%" },
                { Architecture::arm64ec, "%ProgramW6432%",      "%CommonProgramW6432%" }
            };

            for (const auto& [arch, programFolderPath, commonFolderPath] : paths)
            {
                setBinaryPathDefault (Ids::vstBinaryLocation,  arch, programFolderPath + Txt ("\\Steinberg\\Vstplugins"));
                setBinaryPathDefault (Ids::vst3BinaryLocation, arch, commonFolderPath  + Txt ("\\VST3"));
                setBinaryPathDefault (Ids::aaxBinaryLocation,  arch, commonFolderPath  + Txt ("\\Avid\\Audio\\Plug-Ins"));
                setBinaryPathDefault (Ids::lv2BinaryLocation,  arch, "%APPDATA%\\LV2");
            }

            optimisationLevelValue.setDefault (isDebug() ? optimisationOff : optimiseFull);

            config.addListener (this);
        }

        ~MSVCBuildConfiguration() override
        {
            config.removeListener (this);
        }

        z0 valueTreePropertyChanged (ValueTree&, const Identifier& property) override
        {
            if (property != Ids::winArchitecture)
                return;

            project.removeProjectMessage (ProjectMessages::Ids::arm64Warning);

            const auto selectedArchs = architectureTypeValue.get();

            if (! selectedArchs.getArray()->contains (getArchitectureValueString (Architecture::arm64)))
                return;

            if (selectedArchs.getArray()->contains (getArchitectureValueString (Architecture::arm64ec)))
                return;

            project.addProjectMessage (ProjectMessages::Ids::arm64Warning, {});
        }

        Txt getBinaryPath (const Identifier& id, Architecture arch) const
        {
            if (auto* location = getLocationForArchitecture (id, arch))
                return location->get().toString();

            return "";
        }

        z0 setBinaryPathDefault (const Identifier& id, Architecture arch, const Txt& path)
        {
            if (auto* location = getLocationForArchitecture (id, arch))
                location->setDefault (path);
        }

        //==============================================================================
        i32 getWarningLevel() const                       { return warningLevelValue.get(); }
        b8 areWarningsTreatedAsErrors() const           { return warningsAreErrorsValue.get(); }

        Array<Architecture> getArchitectures() const
        {
            auto value = architectureTypeValue.get();
            auto* array = value.getArray();

            if (array == nullptr)
                return {};

            Array<Architecture> result;
            result.resize (array->size());

            std::transform (array->begin(), array->end(), result.begin(), [] (const var& archVar)
            {
                return *architectureTypeFromString (archVar.toString());
            });

            return result;
        }

        Txt getPrebuildCommandString() const           { return prebuildCommandValue.get(); }
        Txt getPostbuildCommandString() const          { return postbuildCommandValue.get(); }
        Txt getIntermediatesPathString() const         { return intermediatesPathValue.get(); }
        Txt getCharacterSetString() const              { return characterSetValue.get(); }
        Txt getDebugInformationFormatString() const    { return debugInformationFormatValue.get(); }

        b8 shouldGenerateDebugSymbols() const           { return generateDebugSymbolsValue.get(); }
        b8 shouldLinkIncremental() const                { return enableIncrementalLinkingValue.get(); }
        b8 isUsingRuntimeLibDLL() const                 { return useRuntimeLibDLLValue.get(); }
        b8 shouldUseMultiProcessorCompilation() const   { return multiProcessorCompilationValue.get(); }
        b8 isFastMathEnabled() const                    { return fastMathValue.get(); }
        b8 isPluginBinaryCopyStepEnabled() const        { return pluginBinaryCopyStepValue.get(); }

        static b8 shouldBuildTarget (build_tools::ProjectType::Target::Type targetType, Architecture arch)
        {
            return targetType != build_tools::ProjectType::Target::AAXPlugIn
                || (arch != Architecture::arm64 && arch != Architecture::arm64ec);
        }

        //==============================================================================
        Txt createMSVCConfigName (Architecture arch) const
        {
            return getName() + "|" + getArchitectureValueString (arch);
        }

        Txt getOutputFilename (const Txt& suffix,
                                  b8 forceSuffix,
                                  build_tools::ProjectType::Target::Type type) const
        {
            using Target = build_tools::ProjectType::Target::Type;

            if (type == Target::LV2Helper)
                return Project::getLV2FileWriterName() + suffix;

            if (type == Target::VST3Helper)
                return Project::getVST3FileWriterName() + suffix;

            const auto forceUnityPrefix = type == Target::UnityPlugIn;
            auto target = File::createLegalFileName (getTargetBinaryNameString (forceUnityPrefix).trim());

            if (forceSuffix || ! target.containsChar ('.'))
                return target.upToLastOccurrenceOf (".", false, false) + suffix;

            return target;
        }

        z0 createConfigProperties (PropertyListBuilder& props) override
        {
            if (project.isAudioPluginProject())
                addVisualStudioPluginInstallPathProperties (props);

            const auto architectureList = exporter.getExporterIdentifier() == Identifier { "VS2022" }
                                        ? std::vector<Architecture> { Architecture::win32, Architecture::win64, Architecture::arm64, Architecture::arm64ec }
                                        : std::vector<Architecture> { Architecture::win32, Architecture::win64, Architecture::arm64 };

            Array<Txt> architectureListAsStrings;
            Array<var> architectureListAsVars;

            for (const auto& arch : architectureList)
            {
                architectureListAsStrings.add (getArchitectureValueString (arch));
                architectureListAsVars.add (getArchitectureValueString (arch));
            }

            props.add (new MultiChoicePropertyComponent (architectureTypeValue, "Architecture", architectureListAsStrings, architectureListAsVars),
                       "Which Windows architecture to use.");

            props.add (new ChoicePropertyComponentWithEnablement (debugInformationFormatValue,
                                                                  isDebug() ? isDebugValue : generateDebugSymbolsValue,
                                                                  "Debug Information Format",
                                                                  { "None", "C7 Compatible (/Z7)", "Program Database (/Zi)", "Program Database for Edit And Continue (/ZI)" },
                                                                  { "None", "OldStyle",            "ProgramDatabase",        "EditAndContinue" }),
                       "The type of debugging information created for your program for this configuration."
                       " This will always be used in a debug configuration and will be used in a release configuration"
                       " with forced generation of debug symbols.");

            props.add (new ChoicePropertyComponent (fastMathValue, "Relax IEEE Compliance"),
                       "Enable this to use FAST_MATH non-IEEE mode. (Warning: this can have unexpected results!)");

            props.add (new ChoicePropertyComponent (optimisationLevelValue, "Optimisation",
                                                    { "Disabled (/Od)", "Minimise size (/O1)", "Maximise speed (/O2)", "Full optimisation (/Ox)" },
                                                    { optimisationOff,  optimiseMinSize,       optimiseMaxSpeed,       optimiseFull }),
                       "The optimisation level for this configuration");

            props.add (new TextPropertyComponent (intermediatesPathValue, "Intermediates Path", 2048, false),
                       "An optional path to a folder to use for the intermediate build files. Note that Visual Studio allows "
                       "you to use macros in this path, e.g. \"$(TEMP)\\MyAppBuildFiles\\$(Configuration)\", which is a handy way to "
                       "send them to the user's temp folder.");

            props.add (new ChoicePropertyComponent (warningLevelValue, "Warning Level",
                                                    { "Low", "Medium", "High" },
                                                    { 2,     3,        4 }),
                       "The compilation warning level to use.");

            props.add (new ChoicePropertyComponent (warningsAreErrorsValue, "Treat Warnings as Errors"),
                       "Enable this to treat compilation warnings as errors.");

            props.add (new ChoicePropertyComponent (useRuntimeLibDLLValue, "Runtime Library",
                                                    { "Use static runtime", "Use DLL runtime" },
                                                    { false,                true }),
                       "If the static runtime is selected then your app/plug-in will not be dependent upon users having Microsoft's redistributable "
                       "C++ runtime installed. However, if you are linking libraries from different sources you must select the same type of runtime "
                       "used by the libraries.");

            props.add (new ChoicePropertyComponent (multiProcessorCompilationValue, "Multi-Processor Compilation",
                                                    { "Enabled", "Disabled" },
                                                    { true,      false }),
                       "Allows the compiler to use of all the available processors, which can reduce compilation time. "
                       "This is enabled by default and should only be disabled if you know what you are doing.");

            props.add (new ChoicePropertyComponent (enableIncrementalLinkingValue, "Incremental Linking"),
                       "Enable to avoid linking from scratch for every new build. "
                       "Disable to ensure that your final release build does not contain padding or thunks.");

            if (! isDebug())
            {
                props.add (new ChoicePropertyComponent (generateDebugSymbolsValue, "Force Generation of Debug Symbols"),
                           "Enable this to force generation of debug symbols in a release configuration.");
            }

            props.add (new TextPropertyComponent (prebuildCommandValue,  "Pre-build Command",  2048, true),
                       "Some command that will be run before a build starts.");

            props.add (new TextPropertyComponent (postbuildCommandValue, "Post-build Command", 2048, true),
                       "Some command that will be run after a build starts.");

            props.add (new ChoicePropertyComponent (characterSetValue, "Character Set",
                                                    { "MultiByte", "Unicode" },
                                                    { "MultiByte", "Unicode" }),
                       "Specifies the character set used when building.");
        }

        Txt getModuleLibraryArchName() const override
        {
            Txt result ("$(Platform)\\");
            result += isUsingRuntimeLibDLL() ? "MD" : "MT";

            if (isDebug())
                result += "d";

            return result;
        }

        z0 updateOldLTOSetting()
        {
            if (! isDebug() && config.getPropertyAsValue ("wholeProgramOptimisation", nullptr) != Value())
                linkTimeOptimisationValue = (static_cast<i32> (config ["wholeProgramOptimisation"]) == 0);
        }

        z0 updateOldArchSetting()
        {
            if (architectureTypeValue.get().isArray())
                return;

            const auto archString = architectureTypeValue.get().toString();
            const auto archType = architectureTypeFromString (archString);

            if (! archType)
                return;

            const auto pluginBinaryPathLocationIds =
            {
                Ids::vstBinaryLocation,
                Ids::vst3BinaryLocation,
                Ids::lv2BinaryLocation,
                Ids::aaxBinaryLocation,
                Ids::unityPluginBinaryLocation
            };

            for (const auto& location : pluginBinaryPathLocationIds)
            {
                if (auto* prop = config.getPropertyPointer (location))
                {
                    setBinaryPathDefault (location, *archType, prop->toString());
                }
            }

            architectureTypeValue = Array<var> { archString };
        }

    private:
        ValueTreePropertyWithDefault warningLevelValue, warningsAreErrorsValue, prebuildCommandValue, postbuildCommandValue, generateDebugSymbolsValue,
                                     enableIncrementalLinkingValue, useRuntimeLibDLLValue, multiProcessorCompilationValue,
                                     intermediatesPathValue, characterSetValue, architectureTypeValue, fastMathValue, debugInformationFormatValue,
                                     pluginBinaryCopyStepValue;

        struct LocationProperties
        {
            LocationProperties (ValueTree& tree, const Identifier& propertyID, UndoManager* um)
                : win32   (tree, propertyID + "_Win32",   um),
                  x64     (tree, propertyID + "_x64",     um),
                  arm64   (tree, propertyID + "_arm64",   um),
                  arm64ec (tree, propertyID + "_arm64ec", um)
            {
            }

            const ValueTreePropertyWithDefault* get (Architecture arch) const
            {
                return get (*this, arch);
            }

            ValueTreePropertyWithDefault* get (Architecture arch)
            {
                return get (*this, arch);
            }

            ValueTreePropertyWithDefault win32, x64, arm64, arm64ec;

        private:
            template <typename This>
            static auto get (This& t, Architecture arch) -> decltype (t.get (arch))
            {
                switch (arch)
                {
                    case Architecture::win32:   return &t.win32;
                    case Architecture::win64:   return &t.x64;
                    case Architecture::arm64:   return &t.arm64;
                    case Architecture::arm64ec: return &t.arm64ec;
                }

                jassertfalse;
                return nullptr;
            }
        };

        template <typename This>
        static auto getLocationForArchitecture (This& t, const Identifier& id, Architecture arch)
        {
            const auto properties =
            {
                std::pair { Ids::vstBinaryLocation,  &t.vstBinaryLocation },
                std::pair { Ids::vst3BinaryLocation, &t.vst3BinaryLocation },
                std::pair { Ids::aaxBinaryLocation,  &t.aaxBinaryLocation },
                std::pair { Ids::lv2BinaryLocation,  &t.lv2BinaryLocation }
            };

            const auto iter = std::find_if (properties.begin(),
                                            properties.end(),
                                            [id] (auto pair) { return id == pair.first; });

            return iter != properties.end() ? iter->second->get (arch) : nullptr;
        }

        ValueTreePropertyWithDefault* getLocationForArchitecture (const Identifier& id, Architecture arch)
        {
            return getLocationForArchitecture (*this, id, arch);
        }

        const ValueTreePropertyWithDefault* getLocationForArchitecture (const Identifier& id, Architecture arch) const
        {
            return getLocationForArchitecture (*this, id, arch);
        }

        LocationProperties vstBinaryLocation, vst3BinaryLocation, aaxBinaryLocation, lv2BinaryLocation, unityPluginBinaryLocation;

        //==============================================================================
        z0 addVisualStudioPluginInstallPathProperties (PropertyListBuilder& props)
        {
            auto isBuildingAnyPlugins = (project.shouldBuildVST() || project.shouldBuildVST3()
                                          || project.shouldBuildAAX() || project.shouldBuildUnityPlugin());

            if (isBuildingAnyPlugins)
                props.add (new ChoicePropertyComponent (pluginBinaryCopyStepValue, "Enable Plugin Copy Step"),
                           "Enable this to copy plugin binaries to a specified folder after building.");

            const auto addLocationProperties = [&] (auto& locationProps, const Txt& format)
            {
                for (const auto& [member, arch] : { std::tuple (&LocationProperties::win32,   Architecture::win32),
                                                    std::tuple (&LocationProperties::x64,     Architecture::win64),
                                                    std::tuple (&LocationProperties::arm64,   Architecture::arm64),
                                                    std::tuple (&LocationProperties::arm64ec, Architecture::arm64ec) })
                {
                    const auto archAndFormat = getArchitectureValueString (arch) + " " + format;
                    props.add (new TextPropertyComponentWithEnablement (locationProps.*member, pluginBinaryCopyStepValue, archAndFormat + " Binary Location", 1024, false),
                               "The folder in which the compiled " + archAndFormat + " binary should be placed.");
                }
            };

            if (project.shouldBuildVST3())
                addLocationProperties (vst3BinaryLocation, "VST3");

            if (project.shouldBuildAAX())
                addLocationProperties (aaxBinaryLocation, "AAX");

            if (project.shouldBuildLV2())
                addLocationProperties (lv2BinaryLocation, "LV2");

            if (project.shouldBuildUnityPlugin())
                addLocationProperties (unityPluginBinaryLocation, "Unity");

            if (project.shouldBuildVST())
                addLocationProperties (vstBinaryLocation, "VST (Legacy)");
        }
    };

    //==============================================================================
    class MSVCTarget final : public build_tools::ProjectType::Target
    {
    public:
        MSVCTarget (build_tools::ProjectType::Target::Type targetType, const MSVCProjectExporterBase& exporter)
            : build_tools::ProjectType::Target (targetType), owner (exporter)
        {
            projectGuid = createGUID (owner.getProject().getProjectUIDString() + getName());
        }

        virtual ~MSVCTarget() = default;

        Txt getProjectVersionString() const     { return "10.00"; }
        Txt getProjectFileSuffix() const        { return ".vcxproj"; }
        Txt getFiltersFileSuffix() const        { return ".vcxproj.filters"; }
        Txt getTopLevelXmlEntity() const        { return "Project"; }

        //==============================================================================
        z0 fillInProjectXml (XmlElement& projectXml) const
        {
            projectXml.setAttribute ("DefaultTargets", "Build");
            projectXml.setAttribute ("ToolsVersion", getOwner().getToolsVersion());
            projectXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

            const auto allArchitectures = owner.getAllActiveArchitectures();

            {
                auto* configsGroup = projectXml.createNewChildElement ("ItemGroup");
                configsGroup->setAttribute ("Label", "ProjectConfigurations");

                for (ConstConfigIterator i (owner); i.next();)
                {
                    auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);
                    for (const auto& arch : allArchitectures)
                    {
                        auto* e = configsGroup->createNewChildElement ("ProjectConfiguration");
                        e->setAttribute ("Include", config.createMSVCConfigName (arch));
                        e->createNewChildElement ("Configuration")->addTextElement (config.getName());
                        e->createNewChildElement ("Platform")->addTextElement (getArchitectureValueString (arch));
                    }
                }
            }

            {
                auto* globals = projectXml.createNewChildElement ("PropertyGroup");
                globals->setAttribute ("Label", "Globals");
                globals->createNewChildElement ("ProjectGuid")->addTextElement (getProjectGuid());
            }

            {
                auto* imports = projectXml.createNewChildElement ("Import");
                imports->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");
            }

            for (ConstConfigIterator i (owner); i.next();)
            {
                auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);

                for (const auto& arch : allArchitectures)
                {
                    auto* e = projectXml.createNewChildElement ("PropertyGroup");
                    setConditionAttribute (*e, config, arch);
                    e->setAttribute ("Label", "Configuration");
                    e->createNewChildElement ("ConfigurationType")->addTextElement (getProjectType());
                    e->createNewChildElement ("UseOfMfc")->addTextElement ("false");
                    e->createNewChildElement ("WholeProgramOptimization")->addTextElement (config.isLinkTimeOptimisationEnabled() ? "true"
                                                                                                                                  : "false");

                    auto charSet = config.getCharacterSetString();

                    if (charSet.isNotEmpty())
                        e->createNewChildElement ("CharacterSet")->addTextElement (charSet);

                    if (config.shouldLinkIncremental())
                        e->createNewChildElement ("LinkIncremental")->addTextElement ("true");

                    e->createNewChildElement ("PlatformToolset")->addTextElement (owner.getPlatformToolset());

                    addWindowsTargetPlatformToConfig (*e);

                    struct IntelLibraryInfo
                    {
                        Txt libraryKind;
                        Txt configString;
                    };

                    for (const auto& info : { IntelLibraryInfo { owner.getIPPLibrary(),   "UseIntelIPP" },
                                              IntelLibraryInfo { owner.getIPP1ALibrary(), "UseIntelIPP1A" },
                                              IntelLibraryInfo { owner.getMKL1ALibrary(), "UseInteloneMKL" } })
                    {
                        if (info.libraryKind.isNotEmpty())
                            e->createNewChildElement (info.configString)->addTextElement (info.libraryKind);
                    }
                }
            }

            {
                auto* e = projectXml.createNewChildElement ("Import");
                e->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props");
            }

            {
                auto* e = projectXml.createNewChildElement ("ImportGroup");
                e->setAttribute ("Label", "ExtensionSettings");
            }

            {
                auto* e = projectXml.createNewChildElement ("ImportGroup");
                e->setAttribute ("Label", "PropertySheets");
                auto* p = e->createNewChildElement ("Import");
                p->setAttribute ("Project", "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props");
                p->setAttribute ("Condition", "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')");
                p->setAttribute ("Label", "LocalAppDataPlatform");
            }

            {
                auto* props = projectXml.createNewChildElement ("PropertyGroup");
                props->createNewChildElement ("_ProjectFileVersion")->addTextElement ("10.0.30319.1");
                props->createNewChildElement ("TargetExt")->addTextElement (getTargetSuffix());

                for (ConstConfigIterator i (owner); i.next();)
                {
                    auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);

                    for (const auto& arch : allArchitectures)
                    {
                        if (getConfigTargetPath (config).isNotEmpty())
                        {
                            auto* outdir = props->createNewChildElement ("OutDir");
                            setConditionAttribute (*outdir, config, arch);
                            outdir->addTextElement (build_tools::windowsStylePath (getConfigTargetPath (config)) + "\\");
                        }

                        {
                            auto* intdir = props->createNewChildElement ("IntDir");
                            setConditionAttribute (*intdir, config, arch);

                            auto intermediatesPath = getIntermediatesPath (config);
                            if (! intermediatesPath.endsWithChar (L'\\'))
                                intermediatesPath += L'\\';

                            intdir->addTextElement (build_tools::windowsStylePath (intermediatesPath));
                        }

                        {
                            auto* targetName = props->createNewChildElement ("TargetName");
                            setConditionAttribute (*targetName, config, arch);
                            targetName->addTextElement (msBuildEscape (config.getOutputFilename ("", false, type)));
                        }

                        {
                            auto* manifest = props->createNewChildElement ("GenerateManifest");
                            setConditionAttribute (*manifest, config, arch);
                            manifest->addTextElement ("true");
                        }

                        if (type != SharedCodeTarget)
                        {
                            auto librarySearchPaths = getLibrarySearchPaths (config);

                            if (! librarySearchPaths.isEmpty())
                            {
                                auto* libPath = props->createNewChildElement ("LibraryPath");
                                setConditionAttribute (*libPath, config, arch);
                                libPath->addTextElement ("$(LibraryPath);" + librarySearchPaths.joinIntoString (";"));
                            }
                        }

                        const auto enabled = config.getArchitectures().contains (arch) ? "true" : "false";

                        for (const auto optionName : { "PreBuildEventUseInBuild", "PostBuildEventUseInBuild" })
                        {
                            auto* tag = props->createNewChildElement (optionName);
                            setConditionAttribute (*tag, config, arch);
                            tag->addTextElement (enabled);
                        }
                    }
                }
            }

            for (ConstConfigIterator i (owner); i.next();)
            {
                auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);

                for (const auto& arch : allArchitectures)
                {
                    enum class EscapeQuotes { no, yes };

                    // VS doesn't correctly escape f64 quotes in preprocessor definitions, so we have
                    // to add our own layer of escapes
                    const auto addIncludePathsAndPreprocessorDefinitions = [this, &config] (XmlElement& xml, EscapeQuotes escapeQuotes)
                    {
                        auto includePaths = getOwner().getHeaderSearchPaths (config);
                        includePaths.add ("%(AdditionalIncludeDirectories)");
                        xml.createNewChildElement ("AdditionalIncludeDirectories")->addTextElement (includePaths.joinIntoString (";"));

                        const auto preprocessorDefs = getPreprocessorDefs (config, ";") + ";%(PreprocessorDefinitions)";
                        const auto preprocessorDefsEscaped = escapeQuotes == EscapeQuotes::yes ? preprocessorDefs.replace ("\"", "\\\"")
                                                                                               : preprocessorDefs;
                        xml.createNewChildElement ("PreprocessorDefinitions")->addTextElement (preprocessorDefsEscaped);
                    };

                    b8 isDebug = config.isDebug();

                    auto* group = projectXml.createNewChildElement ("ItemDefinitionGroup");
                    setConditionAttribute (*group, config, arch);

                    {
                        auto* midl = group->createNewChildElement ("Midl");
                        midl->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                                         : "NDEBUG;%(PreprocessorDefinitions)");
                        midl->createNewChildElement ("MkTypLibCompatible")->addTextElement ("true");
                        midl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                        midl->createNewChildElement ("TargetEnvironment")->addTextElement ("Win32");
                        midl->createNewChildElement ("HeaderFileName");
                    }

                    b8 isUsingEditAndContinue = false;
                    const auto pdbFilename = getOwner().getIntDirFile (config, config.getOutputFilename (".pdb", true, type));

                    {
                        auto* cl = group->createNewChildElement ("ClCompile");

                        cl->createNewChildElement ("Optimization")->addTextElement (getOptimisationLevelString (config.getOptimisationLevelInt()));

                        const auto debugInfoFormat = isDebug || config.shouldGenerateDebugSymbols()
                                                   ? config.getDebugInformationFormatString()
                                                   : "OldStyle";

                        cl->createNewChildElement ("DebugInformationFormat")->addTextElement (debugInfoFormat);

                        addIncludePathsAndPreprocessorDefinitions (*cl, EscapeQuotes::no);

                        cl->createNewChildElement ("RuntimeLibrary")->addTextElement (config.isUsingRuntimeLibDLL() ? (isDebug ? "MultiThreadedDebugDLL" : "MultiThreadedDLL")
                                                                                                                    : (isDebug ? "MultiThreadedDebug"    : "MultiThreaded"));
                        cl->createNewChildElement ("RuntimeTypeInfo")->addTextElement ("true");
                        cl->createNewChildElement ("PrecompiledHeader")->addTextElement ("NotUsing");
                        cl->createNewChildElement ("AssemblerListingLocation")->addTextElement ("$(IntDir)\\");
                        cl->createNewChildElement ("ObjectFileName")->addTextElement ("$(IntDir)\\");
                        cl->createNewChildElement ("ProgramDataBaseFileName")->addTextElement (pdbFilename);
                        cl->createNewChildElement ("WarningLevel")->addTextElement ("Level" + Txt (config.getWarningLevel()));
                        cl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                        cl->createNewChildElement ("MultiProcessorCompilation")->addTextElement (config.shouldUseMultiProcessorCompilation() ? "true" : "false");

                        if (config.isFastMathEnabled())
                            cl->createNewChildElement ("FloatingPointModel")->addTextElement ("Fast");

                        auto extraFlags = getOwner().replacePreprocessorTokens (config, config.getAllCompilerFlagsString()).trim();

                        if (extraFlags.isNotEmpty())
                            cl->createNewChildElement ("AdditionalOptions")->addTextElement (extraFlags + " %(AdditionalOptions)");

                        if (config.areWarningsTreatedAsErrors())
                            cl->createNewChildElement ("TreatWarningAsError")->addTextElement ("true");

                        auto cppStandard = owner.project.getCppStandardString();
                        cl->createNewChildElement ("LanguageStandard")->addTextElement ("stdcpp" + cppStandard);
                    }

                    {
                        auto* res = group->createNewChildElement ("ResourceCompile");
                        addIncludePathsAndPreprocessorDefinitions (*res, EscapeQuotes::yes);
                    }

                    auto externalLibraries = getExternalLibraries (config, getOwner().getExternalLibrariesStringArray());
                    auto additionalDependencies = type != SharedCodeTarget && type != LV2Helper && type != VST3Helper && ! externalLibraries.isEmpty()
                                                  ? externalLibraries.joinIntoString (";") + ";%(AdditionalDependencies)"
                                                  : Txt();

                    auto librarySearchPaths = config.getLibrarySearchPaths();
                    auto additionalLibraryDirs = type != SharedCodeTarget && type != LV2Helper && type != VST3Helper && librarySearchPaths.size() > 0
                                                 ? getOwner().replacePreprocessorTokens (config, librarySearchPaths.joinIntoString (";")) + ";%(AdditionalLibraryDirectories)"
                                                 : Txt();

                    {
                        auto* link = group->createNewChildElement ("Link");
                        link->createNewChildElement ("OutputFile")->addTextElement (getOutputFilePath (config));
                        link->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                        link->createNewChildElement ("IgnoreSpecificDefaultLibraries")->addTextElement (isDebug ? "libcmt.lib; msvcrt.lib;;%(IgnoreSpecificDefaultLibraries)"
                                                                                                                : "%(IgnoreSpecificDefaultLibraries)");
                        link->createNewChildElement ("GenerateDebugInformation")->addTextElement ((isDebug || config.shouldGenerateDebugSymbols()) ? "true" : "false");
                        link->createNewChildElement ("ProgramDatabaseFile")->addTextElement (pdbFilename);
                        link->createNewChildElement ("SubSystem")->addTextElement (type == ConsoleApp || type == LV2Helper || type == VST3Helper ? "Console" : "Windows");

                        if (arch == Architecture::win32)
                            link->createNewChildElement ("TargetMachine")->addTextElement ("MachineX86");

                        if (isUsingEditAndContinue)
                            link->createNewChildElement ("ImageHasSafeExceptionHandlers")->addTextElement ("false");

                        if (! isDebug)
                        {
                            link->createNewChildElement ("OptimizeReferences")->addTextElement ("true");
                            link->createNewChildElement ("EnableCOMDATFolding")->addTextElement ("true");
                        }

                        if (additionalLibraryDirs.isNotEmpty())
                            link->createNewChildElement ("AdditionalLibraryDirectories")->addTextElement (additionalLibraryDirs);

                        link->createNewChildElement ("LargeAddressAware")->addTextElement ("true");

                        if (config.isLinkTimeOptimisationEnabled())
                            link->createNewChildElement ("LinkTimeCodeGeneration")->addTextElement ("UseLinkTimeCodeGeneration");

                        if (additionalDependencies.isNotEmpty())
                            link->createNewChildElement ("AdditionalDependencies")->addTextElement (additionalDependencies);

                        auto extraLinkerOptions = config.getAllLinkerFlagsString();
                        if (extraLinkerOptions.isNotEmpty())
                            link->createNewChildElement ("AdditionalOptions")->addTextElement (getOwner().replacePreprocessorTokens (config, extraLinkerOptions).trim()
                                                                                               + " %(AdditionalOptions)");

                        auto delayLoadedDLLs = getOwner().msvcDelayLoadedDLLs;
                        if (delayLoadedDLLs.isNotEmpty())
                            link->createNewChildElement ("DelayLoadDLLs")->addTextElement (delayLoadedDLLs);

                        auto moduleDefinitionsFile = getModuleDefinitions (config);
                        if (moduleDefinitionsFile.isNotEmpty())
                            link->createNewChildElement ("ModuleDefinitionFile")
                                ->addTextElement (moduleDefinitionsFile);
                    }

                    {
                        auto* bsc = group->createNewChildElement ("Bscmake");
                        bsc->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                        bsc->createNewChildElement ("OutputFile")->addTextElement (getOwner().getIntDirFile (config, config.getOutputFilename (".bsc", true, type)));
                    }

                    if (type != SharedCodeTarget && type != LV2Helper && type != VST3Helper)
                    {
                        auto* lib = group->createNewChildElement ("Lib");

                        if (additionalDependencies.isNotEmpty())
                            lib->createNewChildElement ("AdditionalDependencies")->addTextElement (additionalDependencies);

                        if (additionalLibraryDirs.isNotEmpty())
                            lib->createNewChildElement ("AdditionalLibraryDirectories")->addTextElement (additionalLibraryDirs);
                    }

                    if (auto manifestFile = getOwner().getManifestPath(); manifestFile.getRoot() != build_tools::RelativePath::unknown || type == VST3Helper)
                    {
                        auto* bsc = group->createNewChildElement ("Manifest");
                        auto* additional = bsc->createNewChildElement ("AdditionalManifestFiles");

                        if (manifestFile.getRoot() != build_tools::RelativePath::unknown)
                        {
                            additional->addTextElement (manifestFile.rebased (getOwner().getProject().getFile().getParentDirectory(),
                                                                              getOwner().getTargetFolder(),
                                                                              build_tools::RelativePath::buildTargetFolder).toWindowsStyle());
                        }
                    }

                    if (getTargetFileType() == staticLibrary && arch == Architecture::win32)
                    {
                        auto* lib = group->createNewChildElement ("Lib");
                        lib->createNewChildElement ("TargetMachine")->addTextElement ("MachineX86");
                    }

                    auto preBuild = getPreBuildSteps (config, arch);
                    if (preBuild.isNotEmpty())
                        group->createNewChildElement ("PreBuildEvent")
                             ->createNewChildElement ("Command")
                             ->addTextElement (preBuild);

                    auto postBuild = getPostBuildSteps (config, arch);
                    if (postBuild.isNotEmpty())
                        group->createNewChildElement ("PostBuildEvent")
                             ->createNewChildElement ("Command")
                             ->addTextElement (postBuild);
                }
            }

            std::unique_ptr<XmlElement> otherFilesGroup (new XmlElement ("ItemGroup"));

            {
                auto* cppFiles    = projectXml.createNewChildElement ("ItemGroup");
                auto* headerFiles = projectXml.createNewChildElement ("ItemGroup");

                writePrecompiledHeaderFiles (*cppFiles);

                for (i32 i = 0; i < getOwner().getAllGroups().size(); ++i)
                {
                    auto& group = getOwner().getAllGroups().getReference (i);

                    if (group.getNumChildren() > 0)
                        addFilesToCompile (group, *cppFiles, *headerFiles, *otherFilesGroup);
                }

                if (type == LV2Helper)
                {
                    const auto location = owner.rebaseFromProjectFolderToBuildTarget (owner.getLV2HelperProgramSource())
                                               .toWindowsStyle();
                    cppFiles->createNewChildElement ("ClCompile")->setAttribute ("Include", location);
                }
                else if (type == VST3Helper)
                {
                    const auto location = owner.rebaseFromProjectFolderToBuildTarget (owner.getVST3HelperProgramSource())
                                               .toWindowsStyle();
                    cppFiles->createNewChildElement ("ClCompile")->setAttribute ("Include", location);
                }
            }

            if (getOwner().iconFile.existsAsFile())
            {
                auto* e = otherFilesGroup->createNewChildElement ("None");
                e->setAttribute ("Include", prependDot (getOwner().iconFile.getFileName()));
            }

            if (getOwner().packagesConfigFile.existsAsFile())
            {
                auto* e = otherFilesGroup->createNewChildElement ("None");
                e->setAttribute ("Include", getOwner().packagesConfigFile.getFileName());
            }

            if (otherFilesGroup->getFirstChildElement() != nullptr)
                projectXml.addChildElement (otherFilesGroup.release());

            if (type != SharedCodeTarget && getOwner().hasResourceFile())
            {
                auto* rcGroup = projectXml.createNewChildElement ("ItemGroup");
                auto* e = rcGroup->createNewChildElement ("ResourceCompile");
                e->setAttribute ("Include", prependDot (getOwner().rcFile.getFileName()));
            }

            {
                auto* e = projectXml.createNewChildElement ("Import");
                e->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets");
            }

            {
                if (owner.shouldAddWebView2Package())
                {
                    auto* importGroup = projectXml.createNewChildElement ("ImportGroup");
                    importGroup->setAttribute ("Label", "ExtensionTargets");

                    auto packageTargetsPath = "packages\\" + getWebView2PackageName() + "." + getWebView2PackageVersion()
                                            + "\\build\\native\\" + getWebView2PackageName() + ".targets";

                    auto* e = importGroup->createNewChildElement ("Import");
                    e->setAttribute ("Project", packageTargetsPath);
                    e->setAttribute ("Condition", "Exists('" + packageTargetsPath + "')");
                }

                if (owner.shouldLinkWebView2Statically())
                {
                    auto* propertyGroup = projectXml.createNewChildElement ("PropertyGroup");
                    auto* loaderPref = propertyGroup->createNewChildElement ("WebView2LoaderPreference");
                    loaderPref->addTextElement ("Static");
                }
            }
        }

        Txt getProjectType() const
        {
            auto targetFileType = getTargetFileType();

            if (targetFileType == executable)     return "Application";
            if (targetFileType == staticLibrary)  return "StaticLibrary";

            return "DynamicLibrary";
        }

        //==============================================================================
        static z0 setSourceFilePCHSettings (XmlElement& element,
                                              const File& pchFile,
                                              const Txt& option,
                                              const BuildConfiguration& config,
                                              Architecture arch)
        {
            auto setConfigConditionAttribute = [&config, arch] (XmlElement* elementToSet) -> XmlElement*
            {
                setConditionAttribute (*elementToSet, config, arch);
                return elementToSet;
            };

            setConfigConditionAttribute (element.createNewChildElement ("PrecompiledHeader"))->addTextElement (option);
            setConfigConditionAttribute (element.createNewChildElement ("PrecompiledHeaderFile"))->addTextElement (pchFile.getFileName());
            setConfigConditionAttribute (element.createNewChildElement ("PrecompiledHeaderOutputFile"))->addTextElement ("$(Platform)\\$(Configuration)\\DrxPrecompiledHeader.pch");
            setConfigConditionAttribute (element.createNewChildElement ("ForcedIncludeFiles"))->addTextElement (pchFile.getFileName());
        }

        z0 writePrecompiledHeaderFiles (XmlElement& cpps) const
        {
            for (ConstConfigIterator i (owner); i.next();)
            {
                if (! i->shouldUsePrecompiledHeaderFile())
                    continue;

                auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);

                for (const auto& arch : config.getArchitectures())
                {
                    auto pchFileContent = config.getPrecompiledHeaderFileContent();

                    if (pchFileContent.isEmpty())
                        continue;

                    auto pchFile = owner.getTargetFolder().getChildFile (config.getPrecompiledHeaderFilename()).withFileExtension (".h");

                    build_tools::writeStreamToFile (pchFile, [&] (MemoryOutputStream& mo)
                    {
                        mo << pchFileContent;
                    });

                    auto pchSourceFile = pchFile.withFileExtension (".cpp");

                    build_tools::writeStreamToFile (pchSourceFile, [this] (MemoryOutputStream& mo)
                    {
                        mo.setNewLineString (owner.getNewLineString());

                        writeAutoGenWarningComment (mo);

                        mo << "    This is an empty source file generated by DRX required for Visual Studio PCH." << newLine
                            << newLine
                            << "*/" << newLine
                            << newLine;
                    });

                    auto* pchSourceElement = cpps.createNewChildElement ("ClCompile");
                    pchSourceElement->setAttribute ("Include", prependDot (pchSourceFile.getFileName()));
                    setSourceFilePCHSettings (*pchSourceElement, pchFile, "Create", config, arch);
                }
            }
        }

        z0 addFilesToCompile (const Project::Item& projectItem, XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles) const
        {
            auto targetType = (getOwner().getProject().isAudioPluginProject() ? type : SharedCodeTarget);

            if (projectItem.isGroup())
            {
                for (i32 i = 0; i < projectItem.getNumChildren(); ++i)
                    addFilesToCompile (projectItem.getChild (i), cpps, headers, otherFiles);
            }
            else if (projectItem.shouldBeAddedToTargetProject() && projectItem.shouldBeAddedToTargetExporter (getOwner())
                     && getOwner().getProject().getTargetTypeFromFilePath (projectItem.getFile(), true) == targetType)
            {
                build_tools::RelativePath path (projectItem.getFile(), getOwner().getTargetFolder(), build_tools::RelativePath::buildTargetFolder);

                jassert (path.getRoot() == build_tools::RelativePath::buildTargetFolder);

                if (path.hasFileExtension (cOrCppFileExtensions) || path.hasFileExtension (asmFileExtensions))
                {
                    auto* e = cpps.createNewChildElement ("ClCompile");
                    e->setAttribute ("Include", path.toWindowsStyle());

                    if (projectItem.shouldBeCompiled())
                    {
                        auto extraCompilerFlags = getOwner().getCompilerFlagsForProjectItem (projectItem);

                        if (shouldAddBigobjFlag (path))
                        {
                            const Txt bigobjFlag ("/bigobj");

                            if (! extraCompilerFlags.contains (bigobjFlag))
                            {
                                extraCompilerFlags << " " << bigobjFlag;
                                extraCompilerFlags.trim();
                            }
                        }

                        if (extraCompilerFlags.isNotEmpty())
                            e->createNewChildElement ("AdditionalOptions")->addTextElement (extraCompilerFlags + " %(AdditionalOptions)");

                        if (! projectItem.shouldSkipPCH())
                        {
                            for (ConstConfigIterator i (owner); i.next();)
                            {
                                auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);

                                if (config.shouldUsePrecompiledHeaderFile())
                                {
                                    for (const auto& arch : config.getArchitectures())
                                    {
                                        auto pchFile = owner.getTargetFolder().getChildFile (i->getPrecompiledHeaderFilename()).withFileExtension (".h");

                                        if (pchFile.existsAsFile())
                                            setSourceFilePCHSettings (*e, pchFile, "Use", *i, arch);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        e->createNewChildElement ("ExcludedFromBuild")->addTextElement ("true");
                    }
                }
                else if (path.hasFileExtension (headerFileExtensions))
                {
                    headers.createNewChildElement ("ClInclude")->setAttribute ("Include", path.toWindowsStyle());
                }
                else if (! path.hasFileExtension (objCFileExtensions))
                {
                    otherFiles.createNewChildElement ("None")->setAttribute ("Include", path.toWindowsStyle());
                }
            }
        }

        static z0 setConditionAttribute (XmlElement& xml, const BuildConfiguration& config, Architecture arch)
        {
            auto& msvcConfig = *static_cast<const MSVCBuildConfiguration*> (&config);
            xml.setAttribute ("Condition", "'$(Configuration)|$(Platform)'=='" + msvcConfig.createMSVCConfigName (arch) + "'");
        }

        //==============================================================================
        z0 addFilterGroup (XmlElement& groups, const Txt& path) const
        {
            auto* e = groups.createNewChildElement ("Filter");
            e->setAttribute ("Include", path);
            e->createNewChildElement ("UniqueIdentifier")->addTextElement (createGUID (path + "_guidpathsaltxhsdf"));
        }

        z0 addFileToFilter (const build_tools::RelativePath& file, const Txt& groupPath,
                              XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles) const
        {
            XmlElement* e = nullptr;

            if (file.hasFileExtension (headerFileExtensions))
                e = headers.createNewChildElement ("ClInclude");
            else if (file.hasFileExtension (sourceFileExtensions))
                e = cpps.createNewChildElement ("ClCompile");
            else
                e = otherFiles.createNewChildElement ("None");

            jassert (file.getRoot() == build_tools::RelativePath::buildTargetFolder);
            e->setAttribute ("Include", file.toWindowsStyle());
            e->createNewChildElement ("Filter")->addTextElement (groupPath);
        }

        b8 addFilesToFilter (const Project::Item& projectItem, const Txt& path,
                               XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles, XmlElement& groups) const
        {
            auto targetType = (getOwner().getProject().isAudioPluginProject() ? type : SharedCodeTarget);

            if (projectItem.isGroup())
            {
                b8 filesWereAdded = false;

                for (i32 i = 0; i < projectItem.getNumChildren(); ++i)
                    if (addFilesToFilter (projectItem.getChild (i),
                                          (path.isEmpty() ? Txt() : (path + "\\")) + projectItem.getChild (i).getName(),
                                          cpps, headers, otherFiles, groups))
                        filesWereAdded = true;

                if (filesWereAdded)
                    addFilterGroup (groups, path);

                return filesWereAdded;
            }
            else if (projectItem.shouldBeAddedToTargetProject()
                     && projectItem.shouldBeAddedToTargetExporter (getOwner())
                     && getOwner().getProject().getTargetTypeFromFilePath (projectItem.getFile(), true) == targetType)
            {
                build_tools::RelativePath relativePath (projectItem.getFile(),
                                                        getOwner().getTargetFolder(),
                                                        build_tools::RelativePath::buildTargetFolder);

                jassert (relativePath.getRoot() == build_tools::RelativePath::buildTargetFolder);

                addFileToFilter (relativePath, path.upToLastOccurrenceOf ("\\", false, false), cpps, headers, otherFiles);
                return true;
            }

            return false;
        }

        z0 fillInFiltersXml (XmlElement& filterXml) const
        {
            filterXml.setAttribute ("ToolsVersion", getOwner().getToolsVersion());
            filterXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

            auto* groupsXml  = filterXml.createNewChildElement ("ItemGroup");
            auto* cpps       = filterXml.createNewChildElement ("ItemGroup");
            auto* headers    = filterXml.createNewChildElement ("ItemGroup");
            std::unique_ptr<XmlElement> otherFilesGroup (new XmlElement ("ItemGroup"));

            for (i32 i = 0; i < getOwner().getAllGroups().size(); ++i)
            {
                auto& group = getOwner().getAllGroups().getReference (i);

                if (group.getNumChildren() > 0)
                    addFilesToFilter (group, group.getName(), *cpps, *headers, *otherFilesGroup, *groupsXml);
            }

            if (getOwner().iconFile.existsAsFile())
            {
                auto* e = otherFilesGroup->createNewChildElement ("None");
                e->setAttribute ("Include", prependDot (getOwner().iconFile.getFileName()));
                e->createNewChildElement ("Filter")->addTextElement (ProjectSaver::getDrxCodeGroupName());
            }

            if (getOwner().packagesConfigFile.existsAsFile())
            {
                auto* e = otherFilesGroup->createNewChildElement ("None");
                e->setAttribute ("Include", getOwner().packagesConfigFile.getFileName());
            }

            if (otherFilesGroup->getFirstChildElement() != nullptr)
                filterXml.addChildElement (otherFilesGroup.release());

            if (type != SharedCodeTarget && getOwner().hasResourceFile())
            {
                auto* rcGroup = filterXml.createNewChildElement ("ItemGroup");
                auto* e = rcGroup->createNewChildElement ("ResourceCompile");
                e->setAttribute ("Include", prependDot (getOwner().rcFile.getFileName()));
                e->createNewChildElement ("Filter")->addTextElement (ProjectSaver::getDrxCodeGroupName());
            }
        }

        const MSVCProjectExporterBase& getOwner() const { return owner; }
        const Txt& getProjectGuid() const            { return projectGuid; }

        //==============================================================================
        z0 writeProjectFile()
        {
            {
                XmlElement projectXml (getTopLevelXmlEntity());
                fillInProjectXml (projectXml);
                writeXmlOrThrow (projectXml, getVCProjFile(), "UTF-8", 10);
            }

            {
                XmlElement filtersXml (getTopLevelXmlEntity());
                fillInFiltersXml (filtersXml);
                writeXmlOrThrow (filtersXml, getVCProjFiltersFile(), "UTF-8", 100);
            }
        }

        Txt getSolutionTargetPath (const BuildConfiguration& config) const
        {
            auto binaryPath = config.getTargetBinaryRelativePathString().trim();
            if (binaryPath.isEmpty())
                return "$(SolutionDir)$(Platform)\\$(Configuration)";

            build_tools::RelativePath binaryRelPath (binaryPath, build_tools::RelativePath::projectFolder);

            if (binaryRelPath.isAbsolute())
                return binaryRelPath.toWindowsStyle();

            return prependDot (binaryRelPath.rebased (getOwner().projectFolder,
                                                      getOwner().getTargetFolder(),
                                                      build_tools::RelativePath::buildTargetFolder)
                               .toWindowsStyle());
        }

        Txt getConfigTargetPath (const MSVCBuildConfiguration& config) const
        {
            const auto result = getSolutionTargetPath (config) + "\\" + getName();

            if (type == LV2PlugIn)
                return result + "\\" + config.getTargetBinaryNameString() + ".lv2";

            return result;
        }

        /*  Like getConfigTargetPath, but expands $(ProjectName) so that build products can be used
            in other projects where $(ProjectName) will expand to a different value.
        */
        Txt getExpandedConfigTargetPath (const MSVCBuildConfiguration& config) const
        {
            return getConfigTargetPath (config).replace ("$(ProjectName)", getOwner().getProjectFileBaseName (getName()));
        }

        Txt getIntermediatesPath (const MSVCBuildConfiguration& config) const
        {
            auto intDir = (config.getIntermediatesPathString().isNotEmpty() ? config.getIntermediatesPathString()
                                                                            : "$(Platform)\\$(Configuration)");

            if (! intDir.endsWithChar (L'\\'))
                intDir += L'\\';

            return intDir + getName();
        }

        static tukk getOptimisationLevelString (i32 level)
        {
            switch (level)
            {
                case optimiseMinSize:   return "MinSpace";
                case optimiseMaxSpeed:  return "MaxSpeed";
                case optimiseFull:      return "Full";
                default:                return "Disabled";
            }
        }

        Txt getTargetSuffix() const
        {
            auto fileType = getTargetFileType();

            if (fileType == executable)          return ".exe";
            if (fileType == staticLibrary)       return ".lib";
            if (fileType == sharedLibraryOrDLL)  return ".dll";

            if (fileType == pluginBundle)
            {
                if (type == AAXPlugIn)   return ".aaxdll";

                return ".dll";
            }

            return {};
        }

        Txt getPreprocessorDefs (const BuildConfiguration& config, const Txt& joinString) const
        {
            auto defines = getOwner().msvcExtraPreprocessorDefs;
            defines.set ("WIN32", "");
            defines.set ("_WINDOWS", "");

            if (config.isDebug())
            {
                defines.set ("DEBUG", "");
                defines.set ("_DEBUG", "");
            }
            else
            {
                defines.set ("NDEBUG", "");
            }

            defines = mergePreprocessorDefs (defines, getOwner().getAllPreprocessorDefs (config, type));

            if (getTargetFileType() == staticLibrary || getTargetFileType() == sharedLibraryOrDLL)
                defines.set("_LIB", "");

            StringArray result;

            for (i32 i = 0; i < defines.size(); ++i)
            {
                auto def = defines.getAllKeys()[i];
                auto value = defines.getAllValues()[i];
                if (value.isNotEmpty())
                    def << "=" << value;

                result.add (def);
            }

            return result.joinIntoString (joinString);
        }

        //==============================================================================
        build_tools::RelativePath getAAXIconFile() const
        {
            const auto aaxSdk = owner.getAAXPathRelative();
            build_tools::RelativePath projectIcon ("icon.ico", build_tools::RelativePath::buildTargetFolder);

            if (getOwner().getTargetFolder().getChildFile ("icon.ico").existsAsFile())
                return projectIcon.rebased (getOwner().getTargetFolder(),
                                            getOwner().getProject().getProjectFolder(),
                                            build_tools::RelativePath::projectFolder);

            return aaxSdk.getChildFile ("Utilities").getChildFile ("PlugIn.ico");
        }

        Txt getExtraPostBuildSteps (const MSVCBuildConfiguration& config, Architecture arch) const
        {
            using Builder = MSVCScriptBuilder;

            const auto copyBuildOutputIntoBundle = [&] (const StringArray& segments)
            {
                return "copy /Y "
                     + getOutputFilePath (config).quoted()
                     + " "
                     + getOwner().getOutDirFile (config, segments.joinIntoString ("\\")).quoted();
            };

            const auto copyBundleToInstallDirectory = [&] (const StringArray& segments, const Txt& directory)
            {
                const auto copyStep = "\r\nxcopy /E /H /K /R /Y /I "
                                    + getOwner().getOutDirFile (config, segments[0]).quoted()
                                    + " "
                                    + (directory + "\\" + segments[0] + "\\").quoted();

                return config.isPluginBinaryCopyStepEnabled() ? copyStep : "";
            };

            if (type == AAXPlugIn)
            {
                const auto aaxSdk = owner.getAAXPathRelative();
                const auto aaxLibsFolder = aaxSdk.getChildFile ("Libs");
                const auto bundleScript  = aaxSdk.getChildFile ("Utilities").getChildFile ("CreatePackage.bat");
                const auto iconFilePath  = getAAXIconFile();

                const auto segments = getAaxBundleStructure (config, arch);

                const auto pkgScript = copyBuildOutputIntoBundle (segments);

                const auto archDir = StringArray (segments.strings.data(), segments.size() - 1).joinIntoString ("\\");
                const auto rebasedArchDir = getOwner().getOutDirFile (config, archDir);
                const auto fixScript = "\r\ncall "
                                     + createRebasedPath (bundleScript)
                                     + " "
                                     + rebasedArchDir.quoted()
                                     + Txt (" ")
                                     + createRebasedPath (iconFilePath);

                const auto copyScript = copyBundleToInstallDirectory (segments, config.getBinaryPath (Ids::aaxBinaryLocation, arch));

                return pkgScript + fixScript + copyScript;
            }

            if (type == UnityPlugIn)
            {
                build_tools::RelativePath scriptPath (config.project.getGeneratedCodeFolder().getChildFile (config.project.getUnityScriptName()),
                                                      getOwner().getTargetFolder(),
                                                      build_tools::RelativePath::projectFolder);

                auto pkgScript = Txt ("copy /Y ") + scriptPath.toWindowsStyle().quoted() + " \"$(OutDir)\"";

                if (config.isPluginBinaryCopyStepEnabled())
                {
                    auto copyLocation = config.getBinaryPath (Ids::unityPluginBinaryLocation, arch);

                    pkgScript += "\r\ncopy /Y \"$(OutDir)$(TargetFileName)\" " +  Txt (copyLocation + "\\$(TargetFileName)").quoted();
                    pkgScript += "\r\ncopy /Y " + Txt ("$(OutDir)" + config.project.getUnityScriptName()).quoted() + " " + Txt (copyLocation + "\\" + config.project.getUnityScriptName()).quoted();
                }

                return pkgScript;
            }

            if (type == LV2PlugIn)
            {
                const auto* writerTarget = [&]() -> MSVCTarget*
                {
                    for (auto* target : owner.targets)
                        if (target->type == LV2Helper)
                            return target;

                    return nullptr;
                }();

                Builder builder;
                const auto writer = (writerTarget->getExpandedConfigTargetPath (config)
                                  + "\\"
                                  + writerTarget->getBinaryNameWithSuffix (config)).quoted()
                                  + " \"$(OutDir)$(TargetFileName)\"\r\n";

                const auto copyStep = "xcopy /E /H /I /K /R /Y \"$(OutDir)\" \""
                                    + config.getBinaryPath (Ids::lv2BinaryLocation, arch)
                                    + '\\'
                                    + config.getTargetBinaryNameString()
                                    + ".lv2\"\r\n";

                builder.runAndCheck (writer,
                                     config.isPluginBinaryCopyStepEnabled() ? copyStep : Builder{}.info ("Sucessfully generated LV2 manifest").build(),
                                     Builder{}.error ("Failed to generate LV2 manifest.")
                                              .exit (-1));

                return builder.build();
            }

            if (type == VST3PlugIn)
            {
                const auto segments = getVst3BundleStructure (config, arch);

                const auto manifestScript = [&]() -> Txt
                {
                    const auto* writerTarget = [&]() -> MSVCTarget*
                    {
                        for (auto* target : owner.targets)
                            if (target->type == VST3Helper)
                                return target;

                        return nullptr;
                    }();

                    if (writerTarget == nullptr)
                        return "";

                    const auto helperExecutablePath = writerTarget->getExpandedConfigTargetPath (config)
                                                    + "\\"
                                                    + writerTarget->getBinaryNameWithSuffix (config);

                    {
                        // moduleinfotool doesn't handle Windows-style path separators properly when computing the bundle name
                        const auto normalisedBundlePath = getOwner().getOutDirFile (config, segments[0]).replace ("\\", "/");
                        const auto contentsDir = normalisedBundlePath + "\\Contents";
                        const auto resourceDir = contentsDir + "\\Resources";
                        const auto manifestPath = (resourceDir + "\\moduleinfo.json");
                        const auto resourceDirPath = resourceDir + "\\";
                        const auto pluginName = getOwner().project.getPluginNameString();

                        const auto manifestInvocationString = StringArray
                        {
                            helperExecutablePath.quoted(),
                            "-create",
                            "-version", getOwner().project.getVersionString().quoted(),
                            "-path",    normalisedBundlePath.quoted(),
                            "-output",  manifestPath.quoted()
                        }.joinIntoString (" ");

                        const auto crossCompilationPairs =
                        {
                            // This catches ARM64 and EC for x64 manifest generation
                            std::pair { Architecture::arm64, Architecture::win64 },
                            std::pair { arch, arch }
                        };

                        Builder builder;

                        builder.set ("manifest_generated", 0);

                        for (auto [hostArch, targetArch] : crossCompilationPairs)
                        {
                            const StringArray expr
                            {
                                "\"$(PROCESSOR_ARCHITECTURE)\" == " + getVisualStudioArchitectureId (hostArch).quoted(),
                                "\"$(Platform)\""            " == " + getVisualStudioPlatformId (targetArch).quoted()
                            };

                            builder.ifAllConditionsTrue (expr, Builder{}.call ("_generate_manifest")
                                                                        .set ("manifest_generated", 1));
                        }

                        const auto archMismatchErrorString = StringArray
                        {
                            "VST3 manifest generation is disabled for",
                            pluginName,
                            "because a",
                            getVisualStudioArchitectureId (arch),
                            "manifest helper cannot run on a host system",
                            "processor detected to be $(PROCESSOR_ARCHITECTURE)."
                        }.joinIntoString (" ");

                        const auto architectureMatched = Builder{}
                            .ifelse ("exist " + manifestPath.quoted(),
                                     Builder{}.deleteFile (manifestPath.quoted()))
                            .ifelse ("not exist "  + resourceDirPath.quoted(),
                                     Builder{}.mkdir (resourceDirPath.quoted()))
                            .runAndCheck (manifestInvocationString,
                                          Builder{}.info ("Successfully generated a manifest for " + pluginName)
                                                   .jump ("_continue"),
                                          Builder{}.info ("The manifest helper failed")
                                                   .jump ("_continue"));

                        builder.ifelse ("%manifest_generated% equ 0",
                                        Builder{}.jump ("_arch_mismatch"));

                        builder.jump ("_continue");
                        builder.labelledSection ("_generate_manifest", architectureMatched);
                        builder.labelledSection ("_arch_mismatch",     Builder{}.info (archMismatchErrorString));
                        builder.labelledSection ("_continue", "");

                        return builder.build();
                    }
                }();

                const auto pkgScript = copyBuildOutputIntoBundle (segments);
                const auto copyScript = copyBundleToInstallDirectory (segments, config.getBinaryPath (Ids::vst3BinaryLocation, arch));

                return MSVCScriptBuilder{}
                    .append (pkgScript)
                    .append (manifestScript)
                    .append (copyScript)
                    .build();
            }

            if (type == VSTPlugIn && config.isPluginBinaryCopyStepEnabled())
            {
                const Txt copyCommand = "copy /Y \"$(OutDir)$(TargetFileName)\" \""
                                         + config.getBinaryPath (Ids::vstBinaryLocation, arch)
                                         + "\\$(TargetFileName)\"";

                return MSVCScriptBuilder{}
                    .mkdir (config.getBinaryPath (Ids::vstBinaryLocation, arch).quoted())
                    .append (copyCommand)
                    .build();
            }

            return {};
        }

        Txt getExtraPreBuildSteps (const MSVCBuildConfiguration& config, Architecture arch) const
        {
            const auto createBundleStructure = [&] (const StringArray& segments)
            {
                auto directory = getOwner().getOutDirFile (config, "");
                MSVCScriptBuilder script;

                std::for_each (segments.begin(), std::prev (segments.end()), [&] (const auto& s)
                {
                    directory += (directory.isEmpty() ? "" : "\\") + s;

                    script.ifelse ("not exist " + (directory + "\\").quoted(),
                                   MSVCScriptBuilder{}.deleteFile (directory.quoted())
                                                      .mkdir (directory.quoted()).build());
                });

                return script.build();
            };

            MSVCScriptBuilder builder;

            if (arch == Architecture::win64)
            {
                const auto x86ToolchainErrorMessage =
                    "echo : Warning: Toolchain configuration issue!"
                    " You are using a 32-bit toolchain to compile a 64-bit target on a 64-bit system."
                    " This may cause problems with the build system."
                    " To resolve this, use the x64 version of MSBuild. You can invoke it directly at:"
                    " \"<VisualStudioPathHere>/MSBuild/Current/Bin/amd64/MSBuild.exe\""
                    " Or, use the \"x64 Native Tools Command Prompt\" script.";

                builder.ifAllConditionsTrue (
                {
                    "\"$(PROCESSOR_ARCHITECTURE)\" == " + getVisualStudioArchitectureId (Architecture::win32).quoted(),

                    // This only exists if the process is x86 but the host is x64.
                    "defined PROCESSOR_ARCHITEW6432"
                }, MSVCScriptBuilder{}.append (x86ToolchainErrorMessage));
            }

            if (type == LV2PlugIn)
            {
                const auto crossCompilationPairs =
                {
                    // This catches ARM64 and EC for x64 manifest generation
                    std::pair { Architecture::arm64, Architecture::win64 },
                    std::pair { arch, arch }
                };

                for (auto [hostArch, targetArch] : crossCompilationPairs)
                {
                    const StringArray expr
                    {
                        "\"$(PROCESSOR_ARCHITECTURE)\" == " + getVisualStudioArchitectureId (hostArch).quoted(),
                        "\"$(Platform)\""            " == " + getVisualStudioPlatformId (targetArch).quoted()
                    };

                    builder.ifAllConditionsTrue (expr, MSVCScriptBuilder{}.jump ("_continue"));
                }

                builder.error (StringArray
                               {
                                   "\"$(Platform)\"",
                                   "LV2 cross-compilation is not available on",
                                   "\"$(PROCESSOR_ARCHITECTURE)\" hosts."
                               }.joinIntoString (" "));
                builder.exit (-1);
                builder.labelledSection ("_continue", "");

                return builder.build();
            }

            if (type == AAXPlugIn)
                return createBundleStructure (getAaxBundleStructure (config, arch));

            if (type == VST3PlugIn)
                return builder.build() + "\r\n" + createBundleStructure (getVst3BundleStructure (config, arch));

            return {};
        }

        Txt getPostBuildSteps (const MSVCBuildConfiguration& config, Architecture arch) const
        {
            const auto post = config.getPostbuildCommandString();
            const auto extra = getExtraPostBuildSteps (config, arch);

            if (post.isNotEmpty() || extra.isNotEmpty())
            {
                return MSVCScriptBuilder{}
                    .append (post.replace ("\n", "\r\n"))
                    .append (extra)
                    .build();
            }

            return "";
        }

        Txt getPreBuildSteps (const MSVCBuildConfiguration& config, Architecture arch) const
        {
            const auto pre = config.getPrebuildCommandString();
            const auto extra = getExtraPreBuildSteps (config, arch);

            if (pre.isNotEmpty() || extra.isNotEmpty())
            {
                return MSVCScriptBuilder{}
                    .append (pre.replace ("\n", "\r\n"))
                    .append (extra)
                    .build();
            }

            return "";
        }

        Txt getBinaryNameWithSuffix (const MSVCBuildConfiguration& config) const
        {
            return config.getOutputFilename (getTargetSuffix(), true, type);
        }

        Txt getOutputFilePath (const MSVCBuildConfiguration& config) const
        {
            return getOwner().getOutDirFile (config, getBinaryNameWithSuffix (config));
        }

        StringArray getLibrarySearchPaths (const MSVCBuildConfiguration& config) const
        {
            auto librarySearchPaths = config.getLibrarySearchPaths();

            if (type != SharedCodeTarget && type != LV2Helper && type != VST3Helper)
                if (auto* shared = getOwner().getSharedCodeTarget())
                    librarySearchPaths.add (shared->getExpandedConfigTargetPath (config));

            return librarySearchPaths;
        }

        /*  Libraries specified in the Projucer don't get escaped automatically.
            To include a special character in the name of a library,
            you must use the appropriate escape code instead.
            Module and shared code library names are not preprocessed.
            Special characters in the names of these libraries will be toEscape
            as appropriate.
        */
        StringArray getExternalLibraries (const MSVCBuildConfiguration& config, const StringArray& otherLibs) const
        {
            auto result = otherLibs;

            for (auto& i : result)
                i = getOwner().replacePreprocessorTokens (config, i).trim();

            result.addArray (msBuildEscape (getOwner().getModuleLibs()));

            if (type != SharedCodeTarget && type != LV2Helper && type != VST3Helper)
                if (auto* shared = getOwner().getSharedCodeTarget())
                    result.add (msBuildEscape (shared->getBinaryNameWithSuffix (config)));

            return result;
        }

        Txt getModuleDefinitions (const MSVCBuildConfiguration& config) const
        {
            auto moduleDefinitions = config.config [Ids::msvcModuleDefinitionFile].toString();

            if (moduleDefinitions.isNotEmpty())
                return moduleDefinitions;

            return {};
        }

        File getVCProjFile() const                                   { return getOwner().getProjectFile (getProjectFileSuffix(), getName()); }
        File getVCProjFiltersFile() const                            { return getOwner().getProjectFile (getFiltersFileSuffix(), getName()); }

        Txt createRebasedPath (const build_tools::RelativePath& path) const { return getOwner().createRebasedPath (path); }

        z0 addWindowsTargetPlatformToConfig (XmlElement& e) const
        {
            auto target = owner.getWindowsTargetPlatformVersion();

            if (target == "Latest")
            {
                auto* child = e.createNewChildElement ("WindowsTargetPlatformVersion");

                child->setAttribute ("Condition", "'$(WindowsTargetPlatformVersion)' == ''");
                child->addTextElement ("$([Microsoft.Build.Utilities.ToolLocationHelper]::GetLatestSDKTargetPlatformVersion('Windows', '10.0'))");
            }
            else
            {
                e.createNewChildElement ("WindowsTargetPlatformVersion")->addTextElement (target);
            }
        }

    protected:
        StringArray getAaxBundleStructure (const MSVCBuildConfiguration& config, Architecture arch) const
        {
            const auto dllName = config.getOutputFilename (".aaxplugin", false, type);
            return { dllName, "Contents", getArchitectureValueString (arch), dllName };
        }

        StringArray getVst3BundleStructure (const MSVCBuildConfiguration& config, Architecture arch) const
        {
            // These suffixes are defined in the VST3 SDK docs
            const auto suffix = std::invoke ([&]() -> Txt
            {
                switch (arch)
                {
                    case Architecture::win32:    return "x86";
                    case Architecture::win64:    return "x86_64";
                    case Architecture::arm64:    return "arm64";
                    case Architecture::arm64ec:  return "arm64ec";
                }

                jassertfalse;
                return {};
            });

            const auto dllName = config.getOutputFilename (".vst3", false, type);
            return { dllName, "Contents", suffix + "-win", dllName };
        }

        const MSVCProjectExporterBase& owner;
        Txt projectGuid;
    };

    //==============================================================================
    b8 usesMMFiles() const override                        { return false; }
    b8 canCopeWithDuplicateFiles() override                { return false; }
    b8 supportsUserDefinedConfigurations() const override  { return true; }

    b8 isXcode() const override                            { return false; }
    b8 isVisualStudio() const override                     { return true; }
    b8 isMakefile() const override                         { return false; }
    b8 isAndroidStudio() const override                    { return false; }

    b8 isAndroid() const override                          { return false; }
    b8 isWindows() const override                          { return true; }
    b8 isLinux() const override                            { return false; }
    b8 isOSX() const override                              { return false; }
    b8 isiOS() const override                              { return false; }

    b8 supportsPrecompiledHeaders() const override         { return true; }

    Txt getNewLineString() const override                 { return "\r\n"; }

    b8 supportsTargetType (build_tools::ProjectType::Target::Type type) const override
    {
        using Target = build_tools::ProjectType::Target;

        switch (type)
        {
        case Target::StandalonePlugIn:
        case Target::GUIApp:
        case Target::ConsoleApp:
        case Target::StaticLibrary:
        case Target::SharedCodeTarget:
        case Target::AggregateTarget:
        case Target::VSTPlugIn:
        case Target::VST3PlugIn:
        case Target::VST3Helper:
        case Target::AAXPlugIn:
        case Target::UnityPlugIn:
        case Target::LV2PlugIn:
        case Target::LV2Helper:
        case Target::DynamicLibrary:
            return true;
        case Target::AudioUnitPlugIn:
        case Target::AudioUnitv3PlugIn:
        case Target::unspecified:
        default:
            break;
        }

        return false;
    }

    //==============================================================================
    build_tools::RelativePath getManifestPath() const
    {
        auto path = manifestFileValue.get().toString();

        return path.isEmpty() ? build_tools::RelativePath()
                              : build_tools::RelativePath (path, build_tools::RelativePath::projectFolder);
    }

    //==============================================================================
    b8 launchProject() override
    {
       #if DRX_WINDOWS
        // Don't launch if already open
        const auto foundDBFiles = getSLNFile().getSiblingFile (".vs").findChildFiles (File::findFiles, true, "*.opendb", File::FollowSymlinks::no);

        if (! foundDBFiles.isEmpty())
            return false;

        return getSLNFile().startAsProcess();
       #else
        return false;
       #endif
    }

    b8 canLaunchProject() override
    {
       #if DRX_WINDOWS
        return true;
       #else
        return false;
       #endif
    }

    z0 createExporterProperties (PropertyListBuilder& props) override
    {
        props.add (new TextPropertyComponent (manifestFileValue, "Manifest file", 8192, false),
            "Path to a manifest input file which should be linked into your binary (path is relative to jucer file).");

        props.add (new ChoicePropertyComponent (IPPLibraryValue, "(deprecated) Use IPP Library",
                                                { "No",  "Yes (Default Linking)",  "Multi-Threaded Static Library", "Single-Threaded Static Library", "Multi-Threaded DLL", "Single-Threaded DLL" },
                                                { var(), "true",                   "Parallel_Static",               "Sequential",                     "Parallel_Dynamic",   "Sequential_Dynamic" }),
                   "This option is deprecated, use the \"Use IPP Library (oneAPI)\" option instead. "
                   "Enable this to use Intel's Integrated Performance Primitives library, if you have an older version that was not supplied in the oneAPI toolkit.");

        props.add (new ChoicePropertyComponent (IPP1ALibraryValue, "Use IPP Library (oneAPI)",
                                                { "No",  "Yes (Default Linking)",  "Static Library",     "Dynamic Library" },
                                                { var(), "true",                   "Static_Library",     "Dynamic_Library" }),
                   "Enable this to use Intel's Integrated Performance Primitives library, supplied as part of the oneAPI toolkit.");

        props.add (new ChoicePropertyComponent (MKL1ALibraryValue, "Use MKL Library (oneAPI)",
                                                { "No",  "Parallel", "Sequential", "Cluster" },
                                                { var(), "Parallel", "Sequential", "Cluster" }),
                   "Enable this to use Intel's MKL library, supplied as part of the oneAPI toolkit.");

        {
            auto isWindows10SDK = getVisualStudioVersion() > 14;

            props.add (new TextPropertyComponent (targetPlatformVersion, "Windows Target Platform", 20, false),
                       Txt ("Specifies the version of the Windows SDK that will be used when building this project. ")
                       + (isWindows10SDK ? "Leave this field empty to use the latest Windows 10 SDK installed on the build machine."
                                         : "The default value for this exporter is " + getDefaultWindowsTargetPlatformVersion()));
        }
    }

    enum OptimisationLevel
    {
        optimisationOff = 1,
        optimiseMinSize = 2,
        optimiseFull = 3,
        optimiseMaxSpeed = 4
    };

    //==============================================================================
    z0 addPlatformSpecificSettingsForProjectType (const build_tools::ProjectType& type) override
    {
        msvcExtraPreprocessorDefs.set ("_CRT_SECURE_NO_WARNINGS", "");

        if (type.isCommandLineApp())
            msvcExtraPreprocessorDefs.set ("_CONSOLE", "");

        callForAllSupportedTargets ([this] (build_tools::ProjectType::Target::Type targetType)
                                    {
                                        if (targetType != build_tools::ProjectType::Target::AggregateTarget)
                                            targets.add (new MSVCTarget (targetType, *this));
                                    });

        // If you hit this assert, you tried to generate a project for an exporter
        // that does not support any of your targets!
        jassert (targets.size() > 0);
    }

    const MSVCTarget* getSharedCodeTarget() const
    {
        for (auto target : targets)
            if (target->type == build_tools::ProjectType::Target::SharedCodeTarget)
                return target;

        return nullptr;
    }

    b8 hasTarget (build_tools::ProjectType::Target::Type type) const
    {
        for (auto target : targets)
            if (target->type == type)
                return true;

        return false;
    }

    static z0 createRCFile (const Project& p, const File& iconFile, const File& rcFile)
    {
        build_tools::ResourceRcOptions resourceRc;
        resourceRc.version = p.getVersionString();
        resourceRc.companyName = p.getCompanyNameString();
        resourceRc.companyCopyright = p.getCompanyCopyrightString();
        resourceRc.projectName = p.getProjectNameString();
        resourceRc.icon = iconFile;

        resourceRc.write (rcFile);
    }

private:
    //==============================================================================
    Txt createRebasedPath (const build_tools::RelativePath& path) const
    {
        auto rebasedPath = rebaseFromProjectFolderToBuildTarget (path).toWindowsStyle();

        return getVisualStudioVersion() < 10  // (VS10 automatically adds escape characters to the quotes for this definition)
                                          ? CppTokeniserFunctions::addEscapeChars (rebasedPath.quoted())
                                          : CppTokeniserFunctions::addEscapeChars (rebasedPath).quoted();
    }

protected:
    //==============================================================================
    mutable File rcFile, iconFile, packagesConfigFile;
    OwnedArray<MSVCTarget> targets;

    ValueTreePropertyWithDefault IPPLibraryValue,
                                 IPP1ALibraryValue,
                                 MKL1ALibraryValue,
                                 platformToolsetValue,
                                 targetPlatformVersion,
                                 manifestFileValue;

    Txt getProjectFileBaseName (const Txt& target) const
    {
        const auto filename = project.getProjectFilenameRootString();

        return filename + (target.isNotEmpty()
                           ? (Txt ("_") + target.removeCharacters (" "))
                           : "");
    }

    File getProjectFile (const Txt& extension, const Txt& target) const
    {
        const auto filename = getProjectFileBaseName (target);

        return getTargetFolder().getChildFile (filename).withFileExtension (extension);
    }

    File getSLNFile() const     { return getProjectFile (".sln", Txt()); }

    static Txt prependIfNotAbsolute (const Txt& file, tukk prefix)
    {
        if (File::isAbsolutePath (file) || file.startsWithChar ('$'))
            prefix = "";

        return prefix + build_tools::windowsStylePath (file);
    }

    Txt getIntDirFile (const BuildConfiguration& config, const Txt& file) const  { return prependIfNotAbsolute (replacePreprocessorTokens (config, file), "$(IntDir)\\"); }
    Txt getOutDirFile (const BuildConfiguration& config, const Txt& file) const  { return prependIfNotAbsolute (replacePreprocessorTokens (config, file), "$(OutDir)\\"); }

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return *new MSVCBuildConfiguration (project, v, *this);
    }

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        auto searchPaths = extraSearchPaths;
        searchPaths.addArray (config.getHeaderSearchPaths());
        return getCleanedStringArray (searchPaths);
    }

    Txt getTargetGuid (MSVCTarget::Type type) const
    {
        for (auto* target : targets)
            if (target != nullptr && target->type == type)
                return target->getProjectGuid();

        return {};
    }

    //==============================================================================
    z0 writeProjectDependencies (OutputStream& out) const
    {
        const auto sharedCodeGuid = getTargetGuid (MSVCTarget::SharedCodeTarget);
        const auto lv2HelperGuid  = getTargetGuid (MSVCTarget::LV2Helper);
        const auto vst3HelperGuid = getTargetGuid (MSVCTarget::VST3Helper);

        for (i32 addingOtherTargets = 0; addingOtherTargets < (sharedCodeGuid.isNotEmpty() ? 2 : 1); ++addingOtherTargets)
        {
            for (i32 i = 0; i < targets.size(); ++i)
            {
                if (auto* target = targets[i])
                {
                    if (sharedCodeGuid.isEmpty() || (addingOtherTargets != 0) == (target->type != MSVCTarget::StandalonePlugIn))
                    {
                        out << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << projectName << " - "
                            << target->getName() << "\", \""
                            << target->getVCProjFile().getFileName() << "\", \"" << target->getProjectGuid() << '"' << newLine;

                        if (sharedCodeGuid.isNotEmpty()
                            && target->type != MSVCTarget::SharedCodeTarget
                            && target->type != MSVCTarget::LV2Helper
                            && target->type != MSVCTarget::VST3Helper)
                        {
                            out << "\tProjectSection(ProjectDependencies) = postProject" << newLine
                                << "\t\t" << sharedCodeGuid << " = " << sharedCodeGuid << newLine;

                            if (target->type == MSVCTarget::LV2PlugIn && lv2HelperGuid.isNotEmpty())
                                out << "\t\t" << lv2HelperGuid << " = " << lv2HelperGuid << newLine;

                            if (target->type == MSVCTarget::VST3PlugIn && vst3HelperGuid.isNotEmpty())
                                out << "\t\t" << vst3HelperGuid << " = " << vst3HelperGuid << newLine;

                            out << "\tEndProjectSection" << newLine;
                        }

                        out << "EndProject" << newLine;
                    }
                }
            }
        }
    }

    z0 writeSolutionFile (OutputStream& out, const Txt& versionString, Txt commentString) const
    {
        u8k bomBytes[] { CharPointer_UTF8::byteOrderMark1,
                                         CharPointer_UTF8::byteOrderMark2,
                                         CharPointer_UTF8::byteOrderMark3 };

        for (const auto& byte : bomBytes)
            out.writeByte ((t8) byte);

        if (commentString.isNotEmpty())
            commentString += newLine;

        out << newLine
            << "Microsoft Visual Studio Solution File, Format Version " << versionString << newLine
            << commentString << newLine;

        writeProjectDependencies (out);

        out << "Global" << newLine
            << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << newLine;

        for (ConstConfigIterator i (*this); i.next();)
        {
            auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);

            for (const auto& arch : config.getArchitectures())
            {
                auto configName = config.createMSVCConfigName (arch);
                out << "\t\t" << configName << " = " << configName << newLine;
            }
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << newLine;

        const auto allArchitectures = getAllActiveArchitectures();

        for (auto& target : targets)
        {
            for (ConstConfigIterator i (*this); i.next();)
            {
                auto& config = *static_cast<const MSVCBuildConfiguration*> (&*i);

                // Add a configuration for all projects but only mark the desired to be built.
                // We have to do this as VS will automatically add the entry anyway.
                for (const auto& arch : allArchitectures)
                {
                    auto configName = config.createMSVCConfigName (arch);

                    out << "\t\t" << target->getProjectGuid() << "." << configName << "." << "ActiveCfg" << " = " << configName << newLine;

                    const auto shouldBuild = config.shouldBuildTarget (target->type, arch) && config.getArchitectures().contains (arch);

                    if (shouldBuild)
                        out << "\t\t" << target->getProjectGuid() << "." << configName << "." << "Build.0" << " = " << configName << newLine;
                }
            }
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(SolutionProperties) = preSolution" << newLine
            << "\t\tHideSolutionNode = FALSE" << newLine
            << "\tEndGlobalSection" << newLine;

        out << "EndGlobal" << newLine;
    }

    //==============================================================================
    b8 hasResourceFile() const
    {
        return ! projectType.isStaticLibrary();
    }

    z0 createResourcesAndIcon() const
    {
        if (hasResourceFile())
        {
            iconFile = getTargetFolder().getChildFile ("icon.ico");
            build_tools::writeWinIcon (getIcons(), iconFile);
            rcFile = getTargetFolder().getChildFile ("resources.rc");
            createRCFile (project, iconFile, rcFile);
        }
    }

    b8 shouldAddWebView2Package() const
    {
        return project.getEnabledModules().isModuleEnabled ("drx_gui_extra")
              && (   project.isConfigFlagEnabled ("DRX_USE_WIN_WEBVIEW2", false)
                  || project.isConfigFlagEnabled ("DRX_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING", false));
    }

    b8 shouldLinkWebView2Statically() const
    {
        return project.getEnabledModules().isModuleEnabled ("drx_gui_extra")
               && project.isConfigFlagEnabled ("DRX_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING", false);
    }

    static Txt getWebView2PackageName()     { return "Microsoft.Web.WebView2"; }
    static Txt getWebView2PackageVersion()  { return "1.0.1901.177"; }

    z0 createPackagesConfigFile() const
    {
        if (shouldAddWebView2Package())
        {
            packagesConfigFile = getTargetFolder().getChildFile ("packages.config");

            build_tools::writeStreamToFile (packagesConfigFile, [] (MemoryOutputStream& mo)
            {
                mo.setNewLineString ("\r\n");

                mo << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"                   << newLine
                   << "<packages>"                                                   << newLine
                   << "\t" << "<package id=" << getWebView2PackageName().quoted()
                           << " version="    << getWebView2PackageVersion().quoted()
                           << " />"                                                  << newLine
                   << "</packages>"                                                  << newLine;
            });
        }
    }

    static Txt prependDot (const Txt& filename)
    {
        return build_tools::isAbsolutePath (filename) ? filename
                                                      : (".\\" + filename);
    }

    static b8 shouldAddBigobjFlag (const build_tools::RelativePath& path)
    {
        const auto name = path.getFileNameWithoutExtension();

        return name.equalsIgnoreCase ("include_drx_gui_basics")
            || name.equalsIgnoreCase ("include_drx_audio_processors")
            || name.equalsIgnoreCase ("include_drx_core")
            || name.equalsIgnoreCase ("include_drx_graphics");
    }

    StringArray getModuleLibs() const
    {
        StringArray result;

        for (auto& lib : windowsLibs)
            result.add (lib + ".lib");

        return result;
    }

    DRX_DECLARE_NON_COPYABLE (MSVCProjectExporterBase)
};

//==============================================================================
class MSVCProjectExporterVC2019 final : public MSVCProjectExporterBase
{
public:
    MSVCProjectExporterVC2019 (Project& p, const ValueTree& t)
        : MSVCProjectExporterBase (p, t, getTargetFolderName())
    {
        name = getDisplayName();

        targetPlatformVersion.setDefault (defaultTargetPlatform);
        platformToolsetValue.setDefault (defaultToolset);
    }

    static Txt getDisplayName()        { return "Visual Studio 2019"; }
    static Txt getValueTreeTypeName()  { return "VS2019"; }
    static Txt getTargetFolderName()   { return "VisualStudio2019"; }

    Identifier getExporterIdentifier() const override { return getValueTreeTypeName(); }

    i32 getVisualStudioVersion() const override                      { return 16; }
    Txt getSolutionComment() const override                       { return "# Visual Studio Version 16"; }
    Txt getToolsVersion() const override                          { return "16.0"; }
    Txt getDefaultToolset() const override                        { return defaultToolset; }
    Txt getDefaultWindowsTargetPlatformVersion() const override   { return defaultTargetPlatform; }

    static MSVCProjectExporterVC2019* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        if (settingsToUse.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2019 (projectToUse, settingsToUse);

        return nullptr;
    }

    z0 createExporterProperties (PropertyListBuilder& props) override
    {
        addToolsetProperty (props, { "v140", "v140_xp", "v141", "v141_xp", "v142", "ClangCL" });
        MSVCProjectExporterBase::createExporterProperties (props);
    }

private:
    const Txt defaultToolset { "v142" }, defaultTargetPlatform { "10.0" };

    DRX_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2019)
};

//==============================================================================
class MSVCProjectExporterVC2022 final : public MSVCProjectExporterBase
{
public:
    MSVCProjectExporterVC2022 (Project& p, const ValueTree& t)
        : MSVCProjectExporterBase (p, t, getTargetFolderName())
    {
        name = getDisplayName();

        targetPlatformVersion.setDefault (defaultTargetPlatform);
        platformToolsetValue.setDefault (defaultToolset);
    }

    static Txt getDisplayName()        { return "Visual Studio 2022"; }
    static Txt getValueTreeTypeName()  { return "VS2022"; }
    static Txt getTargetFolderName()   { return "VisualStudio2022"; }

    Identifier getExporterIdentifier() const override { return getValueTreeTypeName(); }

    i32 getVisualStudioVersion() const override                      { return 17; }
    Txt getSolutionComment() const override                       { return "# Visual Studio Version 17"; }
    Txt getToolsVersion() const override                          { return "17.0"; }
    Txt getDefaultToolset() const override                        { return defaultToolset; }
    Txt getDefaultWindowsTargetPlatformVersion() const override   { return defaultTargetPlatform; }

    static MSVCProjectExporterVC2022* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        if (settingsToUse.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2022 (projectToUse, settingsToUse);

        return nullptr;
    }

    z0 createExporterProperties (PropertyListBuilder& props) override
    {
        addToolsetProperty (props, { "v140", "v140_xp", "v141", "v141_xp", "v142", "v143", "ClangCL" });
        MSVCProjectExporterBase::createExporterProperties (props);
    }

private:
    const Txt defaultToolset { "v143" }, defaultTargetPlatform { "10.0" };

    DRX_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2022)
};
