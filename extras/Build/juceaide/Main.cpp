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

#include <drx_build_tools/drx_build_tools.h>

#include <fstream>
#include <unordered_map>

namespace
{

constexpr auto headerTemplate = R"(/*
    IMPORTANT! This file is auto-generated.
    If you alter its contents, your changes may be overwritten!

    This is the header file that your files should include in order to get all the
    DRX library headers. You should avoid including the DRX headers directly in
    your own source files, because that wouldn't pick up the correct configuration
    options for your app.

*/

#pragma once

${DRX_INCLUDES}

#if DRX_TARGET_HAS_BINARY_DATA
 #include "BinaryData.h"
#endif

#if ! DONT_SET_USING_DRX_NAMESPACE
 // If your code uses a lot of DRX classes, then this will obviously save you
 // a lot of typing, but can be disabled by setting DONT_SET_USING_DRX_NAMESPACE.
 using namespace drx;
#endif

#if ! DRX_DONT_DECLARE_PROJECTINFO
namespace ProjectInfo
{
    tukk const  projectName    = "${DRX_EXECUTABLE_NAME}";
    tukk const  companyName    = "${DRX_COMPANY_NAME}";
    tukk const  versionString  = "${DRX_PROJECT_VERSION}";
    i32k          versionNumber  =  ${DRX_PROJECT_VERSION_HEX};
}
#endif
)";

i32 writeBinaryData (drx::ArgumentList&& args)
{
    args.checkMinNumArguments (4);
    const auto namespaceName = args.arguments.removeAndReturn (0);
    const auto headerName    = args.arguments.removeAndReturn (0);
    const auto outFolder     = args.arguments.removeAndReturn (0).resolveAsExistingFolder();
    const auto inputFileList = args.arguments.removeAndReturn (0).resolveAsExistingFile();

    drx::build_tools::ResourceFile resourceFile;

    resourceFile.setClassName (namespaceName.text);
    const auto lineEndings = args.removeOptionIfFound ("--windows") ? "\r\n" : "\n";

    const auto allLines = [&]
    {
        auto lines = drx::StringArray::fromLines (inputFileList.loadFileAsString());
        lines.removeEmptyStrings();
        return lines;
    }();

    for (const auto& arg : allLines)
        resourceFile.addFile (drx::File (arg));

    const auto result = resourceFile.write (0,
                                            lineEndings,
                                            outFolder.getChildFile (headerName.text),
                                            [&outFolder] (i32 index)
                                            {
                                                return outFolder.getChildFile ("./BinaryData" + drx::Txt { index + 1 } + ".cpp");
                                            });

    if (result.result.failed())
        drx::ConsoleApplication::fail (result.result.getErrorMessage(), 1);

    return 0;
}

struct IconParseResults
{
    drx::build_tools::Icons icons;
    drx::File output;
};

IconParseResults parseIconArguments (drx::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto output = args.arguments.removeAndReturn (0);

    const auto popDrawable = [&args]() -> std::unique_ptr<drx::Drawable>
    {
        if (args.size() == 0)
            return {};

        const auto firstArgText = args.arguments.removeAndReturn (0).text;
        return drx::Drawable::createFromImageFile (firstArgText);
    };

    auto smallIcon = popDrawable();
    auto bigIcon   = popDrawable();

    return { { std::move (smallIcon), std::move (bigIcon) }, output.text };
}

i32 writeMacIcon (drx::ArgumentList&& argumentList)
{
    const auto parsed = parseIconArguments (std::move (argumentList));
    drx::build_tools::writeMacIcon (parsed.icons, parsed.output);
    return 0;
}

i32 writeiOSAssets (drx::ArgumentList&& argumentList)
{
    const auto parsed = parseIconArguments (std::move (argumentList));
    drx::build_tools::createXcassetsFolderFromIcons (parsed.icons,
                                                      parsed.output.getParentDirectory(),
                                                      parsed.output.getFileName());
    return 0;
}

i32 writeWinIcon (drx::ArgumentList&& argumentList)
{
    const auto parsed = parseIconArguments (std::move (argumentList));
    drx::build_tools::writeWinIcon (parsed.icons, parsed.output);
    return 0;
}

std::unordered_map<drx::Txt, drx::Txt> parseProjectData (const drx::File& file)
{
    constexpr auto recordSeparator = "\x1e";
    const auto contents = file.loadFileAsString();
    const auto lines    = drx::StringArray::fromTokens (contents, recordSeparator, {});

    std::unordered_map<drx::Txt, drx::Txt> result;

    constexpr auto unitSeparator = "\x1f";

    for (const auto& line : lines)
    {
        if (line.isEmpty())
            continue;

        result.emplace (line.upToFirstOccurrenceOf (unitSeparator, false, false),
                        line.fromFirstOccurrenceOf (unitSeparator, false, false));
    }

    return result;
}

drx::Txt getStringValue (const std::unordered_map<drx::Txt, drx::Txt>& dict,
                             drx::StringRef key)
{
    const auto it = dict.find (key);
    return it != dict.cend() ? it->second : drx::Txt{};
}

b8 getBoolValue (const std::unordered_map<drx::Txt, drx::Txt>& dict, drx::StringRef key)
{
    const auto str = getStringValue (dict, key);
    return str.equalsIgnoreCase ("yes")
        || str.equalsIgnoreCase ("true")
        || str.equalsIgnoreCase ("1")
        || str.equalsIgnoreCase ("on");
}

struct UpdateField
{
    const std::unordered_map<drx::Txt, drx::Txt>& dict;

    z0 operator() (drx::StringRef key, drx::Txt& value) const
    {
        value = getStringValue (dict, key);
    }

    z0 operator() (drx::StringRef key, drx::File& value) const
    {
        value = getStringValue (dict, key);
    }

    z0 operator() (drx::StringRef key, b8& value) const
    {
        value = getBoolValue (dict, key);
    }

    z0 operator() (drx::StringRef key, drx::StringArray& value) const
    {
        value = drx::StringArray::fromTokens (getStringValue (dict, key), ";", {});
    }
};

z0 setIfEmpty (drx::Txt& field, drx::StringRef fallback)
{
    if (field.isEmpty())
        field = fallback;
}

drx::build_tools::PlistOptions parsePlistOptions (const drx::File& file,
                                                   drx::build_tools::ProjectType::Target::Type type)
{
    if (type == drx::build_tools::ProjectType::Target::ConsoleApp)
        drx::ConsoleApplication::fail ("Deduced project type does not require a plist", 1);

    const auto dict = parseProjectData (file);

    UpdateField updateField { dict };

    drx::build_tools::PlistOptions result;

    updateField ("EXECUTABLE_NAME",                      result.executableName);
    updateField ("PLIST_TO_MERGE",                       result.plistToMerge);
    updateField ("IS_IOS",                               result.iOS);
    updateField ("MICROPHONE_PERMISSION_ENABLED",        result.microphonePermissionEnabled);
    updateField ("MICROPHONE_PERMISSION_TEXT",           result.microphonePermissionText);
    updateField ("CAMERA_PERMISSION_ENABLED",            result.cameraPermissionEnabled);
    updateField ("CAMERA_PERMISSION_TEXT",               result.cameraPermissionText);
    updateField ("BLUETOOTH_PERMISSION_ENABLED",         result.bluetoothPermissionEnabled);
    updateField ("BLUETOOTH_PERMISSION_TEXT",            result.bluetoothPermissionText);
    updateField ("SEND_APPLE_EVENTS_PERMISSION_ENABLED", result.sendAppleEventsPermissionEnabled);
    updateField ("SEND_APPLE_EVENTS_PERMISSION_TEXT",    result.sendAppleEventsPermissionText);
    updateField ("SHOULD_ADD_STORYBOARD",                result.shouldAddStoryboardToProject);
    updateField ("LAUNCH_STORYBOARD_FILE",               result.storyboardName);
    updateField ("PROJECT_NAME",                         result.projectName);
    updateField ("VERSION",                              result.marketingVersion);
    updateField ("BUILD_VERSION",                        result.currentProjectVersion);
    updateField ("COMPANY_COPYRIGHT",                    result.companyCopyright);
    updateField ("DOCUMENT_EXTENSIONS",                  result.documentExtensions);
    updateField ("FILE_SHARING_ENABLED",                 result.fileSharingEnabled);
    updateField ("DOCUMENT_BROWSER_ENABLED",             result.documentBrowserEnabled);
    updateField ("STATUS_BAR_HIDDEN",                    result.statusBarHidden);
    updateField ("REQUIRES_FULL_SCREEN",                 result.requiresFullScreen);
    updateField ("BACKGROUND_AUDIO_ENABLED",             result.backgroundAudioEnabled);
    updateField ("BACKGROUND_BLE_ENABLED",               result.backgroundBleEnabled);
    updateField ("PUSH_NOTIFICATIONS_ENABLED",           result.pushNotificationsEnabled);
    updateField ("PLUGIN_MANUFACTURER_CODE",             result.pluginManufacturerCode);
    updateField ("PLUGIN_CODE",                          result.pluginCode);
    updateField ("IPHONE_SCREEN_ORIENTATIONS",           result.iPhoneScreenOrientations);
    updateField ("IPAD_SCREEN_ORIENTATIONS",             result.iPadScreenOrientations);
    updateField ("PLUGIN_NAME",                          result.pluginName);
    updateField ("PLUGIN_MANUFACTURER",                  result.pluginManufacturer);
    updateField ("PLUGIN_DESCRIPTION",                   result.pluginDescription);
    updateField ("PLUGIN_AU_EXPORT_PREFIX",              result.pluginAUExportPrefix);
    updateField ("PLUGIN_AU_MAIN_TYPE",                  result.auMainType);
    updateField ("IS_AU_SANDBOX_SAFE",                   result.isAuSandboxSafe);
    updateField ("IS_PLUGIN_SYNTH",                      result.isPluginSynth);
    updateField ("IS_PLUGIN_ARA_EFFECT",                 result.isPluginARAEffect);
    updateField ("SUPPRESS_AU_PLIST_RESOURCE_USAGE",     result.suppressResourceUsage);
    updateField ("BUNDLE_ID",                            result.bundleIdentifier);
    updateField ("ICON_FILE",                            result.iconFile);

    result.type = type;

    if (result.storyboardName.isNotEmpty())
        result.storyboardName = result.storyboardName.fromLastOccurrenceOf ("/", false, false)
                                                     .upToLastOccurrenceOf (".storyboard", false, false);

    setIfEmpty (result.microphonePermissionText,
                "This app requires audio input. If you do not have an audio interface connected it will use the built-in microphone.");
    setIfEmpty (result.cameraPermissionText,
                "This app requires access to the camera to function correctly.");
    setIfEmpty (result.bluetoothPermissionText,
                "This app requires access to Bluetooth to function correctly.");
    setIfEmpty (result.sendAppleEventsPermissionText,
                "This app requires the ability to send Apple events to function correctly.");

    result.documentExtensions = result.documentExtensions.replace (";", ",");

    // AUv3 needs a slightly different bundle ID
    if (type == drx::build_tools::ProjectType::Target::Type::AudioUnitv3PlugIn)
    {
        const auto bundleIdSegments = drx::StringArray::fromTokens (result.bundleIdentifier, ".", {});
        jassert (! bundleIdSegments.isEmpty());

        const auto last = bundleIdSegments.isEmpty() ? ""
                                                     : bundleIdSegments[bundleIdSegments.size() - 1];

        result.bundleIdentifier += "." + last + "AUv3";
    }

    return result;
}

i32 writePlist (drx::ArgumentList&& args)
{
    args.checkMinNumArguments (3);
    const auto kind   = args.arguments.removeAndReturn (0);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);
    parsePlistOptions (input.resolveAsExistingFile(),
                       drx::build_tools::ProjectType::Target::typeFromName (kind.text))
        .write (output.resolveAsFile());
    return 0;
}

drx::build_tools::EntitlementOptions parseEntitlementsOptions (const drx::File& file,
                                                                drx::build_tools::ProjectType::Target::Type type)
{
    const auto dict = parseProjectData (file);

    UpdateField updateField { dict };

    drx::build_tools::EntitlementOptions result;

    updateField ("IS_IOS",                          result.isiOS);
    updateField ("IS_PLUGIN",                       result.isAudioPluginProject);
    updateField ("IS_AU_PLUGIN_HOST",               result.isAUPluginHost);
    updateField ("ICLOUD_PERMISSIONS_ENABLED",      result.isiCloudPermissionsEnabled);
    updateField ("PUSH_NOTIFICATIONS_ENABLED",      result.isPushNotificationsEnabled);
    updateField ("APP_GROUPS_ENABLED",              result.isAppGroupsEnabled);
    updateField ("APP_GROUP_IDS",                   result.appGroupIdString);
    updateField ("HARDENED_RUNTIME_ENABLED",        result.isHardenedRuntimeEnabled);
    updateField ("HARDENED_RUNTIME_OPTIONS",        result.hardenedRuntimeOptions);
    updateField ("APP_SANDBOX_ENABLED",             result.isAppSandboxEnabled);
    updateField ("APP_SANDBOX_INHERIT",             result.isAppSandboxInhertianceEnabled);
    updateField ("APP_SANDBOX_OPTIONS",             result.appSandboxOptions);
    updateField ("NETWORK_MULTICAST_ENABLED",       result.isNetworkingMulticastEnabled);

    struct SandboxTemporaryAccessKey
    {
        drx::Txt cMakeVar, key;
    };

    SandboxTemporaryAccessKey sandboxTemporaryAccessKeys[]
    {
        { "APP_SANDBOX_FILE_ACCESS_HOME_RO", "home-relative-path.read-only" },
        { "APP_SANDBOX_FILE_ACCESS_HOME_RW", "home-relative-path.read-write" },
        { "APP_SANDBOX_FILE_ACCESS_ABS_RO",  "absolute-path.read-only" },
        { "APP_SANDBOX_FILE_ACCESS_ABS_RW",  "absolute-path.read-write" }
    };

    for (const auto& entry : sandboxTemporaryAccessKeys)
    {
        drx::StringArray values;
        updateField (entry.cMakeVar, values);

        if (! values.isEmpty())
            result.appSandboxTemporaryPaths.push_back ({ "com.apple.security.temporary-exception.files." + entry.key,
                                                         std::move (values) });
    }

    {
        drx::StringArray values;
        updateField ("APP_SANDBOX_EXCEPTION_IOKIT", values);

        if (! values.isEmpty())
            result.appSandboxExceptionIOKit = values;
    }

    result.type = type;

    return result;
}

i32 writeEntitlements (drx::ArgumentList&& args)
{
    args.checkMinNumArguments (3);
    const auto kind   = args.arguments.removeAndReturn (0);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);

    const auto options = parseEntitlementsOptions (input.resolveAsExistingFile(),
                                                   drx::build_tools::ProjectType::Target::typeFromName (kind.text));
    drx::build_tools::overwriteFileIfDifferentOrThrow (output.resolveAsFile(), options.getEntitlementsFileContent());
    return 0;
}

i32 createAndWrite (const drx::File& file, drx::StringRef text)
{
    if (file.create())
        return file.replaceWithText (text) ? 0 : 1;

    return 1;
}

i32 writePkgInfo (drx::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto kind   = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);

    const auto projectType = drx::build_tools::ProjectType::Target::typeFromName (kind.text);
    return createAndWrite (output.resolveAsFile(),
                           drx::build_tools::getXcodePackageType (projectType)
                           + drx::build_tools::getXcodeBundleSignature (projectType));
}

drx::build_tools::ResourceRcOptions parseRcFileOptions (const drx::File& file)
{
    const auto dict = parseProjectData (file);
    UpdateField updateField { dict };

    drx::build_tools::ResourceRcOptions result;

    updateField ("VERSION",           result.version);
    updateField ("COMPANY_NAME",      result.companyName);
    updateField ("COMPANY_COPYRIGHT", result.companyCopyright);
    updateField ("PROJECT_NAME",      result.projectName);
    updateField ("ICON_FILE",         result.icon);

    return result;
}

i32 writeRcFile (drx::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);
    parseRcFileOptions (input.resolveAsExistingFile()).write (output.resolveAsFile());
    return 0;
}

drx::Txt createDefineStatements (drx::StringRef definitions)
{
    const auto split = drx::StringArray::fromTokens (definitions, ";", "\"");

    drx::Txt defineStatements;

    for (const auto& def : split)
    {
        if (! def.startsWith ("DrxPlugin_"))
            continue;

        const auto defineName  = def.upToFirstOccurrenceOf ("=", false, false);
        const auto defineValue = def.fromFirstOccurrenceOf ("=", false, false);
        defineStatements += "#define " + defineName + " " + defineValue + '\n';
    }

    return defineStatements;
}

i32 writeAuPluginDefines (drx::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);

    const auto dict      = parseProjectData (input.resolveAsExistingFile());
    const auto getString = [&] (drx::StringRef key) { return getStringValue (dict, key); };

    const auto defines = "#pragma once\n" + createDefineStatements (getString ("MODULE_DEFINITIONS"));
    return createAndWrite (output.resolveAsFile(), defines);
}

drx::Txt createIncludeStatements (drx::StringRef definitions)
{
    const auto split = drx::StringArray::fromTokens (definitions, ";", "\"");

    drx::Txt includeStatements;

    for (const auto& def : split)
    {
        constexpr auto moduleToken = "DRX_MODULE_AVAILABLE_";

        if (def.startsWith (moduleToken))
        {
            const auto moduleName = def.fromFirstOccurrenceOf (moduleToken, false, false)
                                       .upToFirstOccurrenceOf ("=", false, false);
            includeStatements += "#include <" + moduleName + "/" + moduleName + ".h>\n";
        }
    }

    return includeStatements;
}

i32 writeHeader (drx::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);

    const auto dict = parseProjectData (input.resolveAsExistingFile());

    const auto getString = [&] (drx::StringRef key) { return getStringValue (dict, key); };

    const auto includes      = createIncludeStatements (getString ("MODULE_DEFINITIONS"));
    const auto projectName   = getString ("PROJECT_NAME");
    const auto name          = projectName.isEmpty() ? getString ("EXECUTABLE_NAME") : projectName;
    const auto versionString = getString ("VERSION");

    const auto headerText = drx::Txt (headerTemplate)
                                .replace ("${DRX_INCLUDES}", includes)
                                .replace ("${DRX_EXECUTABLE_NAME}", name)
                                .replace ("${DRX_COMPANY_NAME}", getString ("COMPANY_NAME"))
                                .replace ("${DRX_PROJECT_VERSION}", versionString)
                                .replace ("${DRX_PROJECT_VERSION_HEX}", drx::build_tools::getVersionAsHex (versionString));

    return createAndWrite (output.resolveAsFile(), headerText);
}

i32 printDRXVersion (drx::ArgumentList&&)
{
    std::cout << drx::SystemStats::getDRXVersion() << std::endl;
    return 0;
}

} // namespace

i32 main (i32 argc, tuk* argv)
{
    drx::ScopedDrxInitialiser_GUI libraryInitialiser;

    return drx::ConsoleApplication::invokeCatchingFailures ([argc, argv]
    {
        if (argc < 1)
            drx::ConsoleApplication::fail ("No arguments passed", 1);

        const auto getString = [&] (tukk text)
        {
            return drx::Txt (drx::CharPointer_UTF8 (text));
        };

        std::vector<drx::Txt> arguments;
        std::transform (argv, argv + argc, std::back_inserter (arguments), getString);

        drx::ArgumentList argumentList { arguments.front(),
                                          drx::StringArray (arguments.data() + 1, (i32) arguments.size() - 1) };

        using Fn = i32 (*) (drx::ArgumentList&&);

        const std::unordered_map<drx::Txt, Fn> commands
        {
            { "auplugindefines", writeAuPluginDefines },
            { "binarydata",      writeBinaryData },
            { "entitlements",    writeEntitlements },
            { "header",          writeHeader },
            { "iosassets",       writeiOSAssets },
            { "macicon",         writeMacIcon },
            { "pkginfo",         writePkgInfo },
            { "plist",           writePlist },
            { "rcfile",          writeRcFile },
            { "version",         printDRXVersion },
            { "winicon",         writeWinIcon }
        };

        argumentList.checkMinNumArguments (1);
        const auto mode = argumentList.arguments.removeAndReturn (0);
        const auto it   = commands.find (mode.text);

        if (it == commands.cend())
            drx::ConsoleApplication::fail ("No matching mode", 1);

        try
        {
            return it->second (std::move (argumentList));
        }
        catch (const drx::build_tools::SaveError& error)
        {
            drx::ConsoleApplication::fail (error.message);
        }
        catch (const std::exception& ex)
        {
            drx::ConsoleApplication::fail (ex.what());
        }
        catch (...)
        {
            drx::ConsoleApplication::fail ("Unhandled exception");
        }

        return 1;
    });

    return 0;
}
