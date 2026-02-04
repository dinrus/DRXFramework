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

namespace drx:: build_tools
{

    Txt EntitlementOptions::getEntitlementsFileContent() const
    {
        Txt content =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n";

        const auto entitlements = getEntitlements();

        for (auto& key : entitlements.getAllKeys())
            content += "\t<key>" + key + "</key>\n\t" + entitlements[key] + "\n";

        return content + "</dict>\n</plist>\n";
    }

    StringPairArray EntitlementOptions::getEntitlements() const
    {
        StringPairArray entitlements;

        if (isiOS)
        {
            // The Inter-App Audio entitlement is currently deprecated, but it
            // also "provides access to Audio Unit extensions". Without the
            // entitlement iOS apps are unable to access AUv3 plug-ins.
            if ((isAudioPluginProject && shouldEnableIAA) || isAUPluginHost)
                entitlements.set ("inter-app-audio", "<true/>");

            if (isiCloudPermissionsEnabled)
            {
                entitlements.set ("com.apple.developer.icloud-container-identifiers",
                                  "<array>\n"
                                  "        <string>iCloud.$(CFBundleIdentifier)</string>\n"
                                  "    </array>");

                entitlements.set ("com.apple.developer.icloud-services",
                                  "<array>\n"
                                  "        <string>CloudDocuments</string>\n"
                                  "    </array>");

                entitlements.set ("com.apple.developer.ubiquity-container-identifiers",
                                  "<array>\n"
                                  "        <string>iCloud.$(CFBundleIdentifier)</string>\n"
                                  "    </array>");
            }
        }

        if (isPushNotificationsEnabled)
            entitlements.set (isiOS ? "aps-environment"
                                    : "com.apple.developer.aps-environment",
                              "<string>development</string>");

        if (isAppGroupsEnabled)
        {
            auto appGroups = StringArray::fromTokens (appGroupIdString, ";", {});
            Txt groups = "<array>";

            for (auto group : appGroups)
                groups += "\n\t\t<string>" + group.trim() + "</string>";

            groups += "\n\t</array>";

            entitlements.set ("com.apple.security.application-groups", groups);
        }

        if (isHardenedRuntimeEnabled)
            for (auto& option : hardenedRuntimeOptions)
                entitlements.set (option, "<true/>");

        if (isAppSandboxEnabled || (! isiOS && isAudioPluginProject && type == ProjectType::Target::AudioUnitv3PlugIn))
        {
            entitlements.set ("com.apple.security.app-sandbox", "<true/>");

            if (isAppSandboxInhertianceEnabled)
            {
                // no other sandbox options can be specified if sandbox inheritance is enabled!
                jassert (appSandboxOptions.isEmpty());
                jassert (appSandboxTemporaryPaths.empty());

                entitlements.set ("com.apple.security.inherit", "<true/>");
            }

            if (isAppSandboxEnabled)
            {
                for (auto& option : appSandboxOptions)
                    entitlements.set (option, "<true/>");

                for (auto& option : appSandboxTemporaryPaths)
                {
                    Txt paths = "<array>";

                    for (const auto& path : option.values)
                        paths += "\n\t\t<string>" + path + "</string>";

                    paths += "\n\t</array>";
                    entitlements.set (option.key, paths);
                }

                if (! appSandboxExceptionIOKit.isEmpty())
                {
                    Txt ioKitClasses = "<array>";

                    for (const auto& c : appSandboxExceptionIOKit)
                        ioKitClasses += "\n\t\t<string>" + c + "</string>";

                    ioKitClasses += "\n\t</array>";
                    entitlements.set ("com.apple.security.temporary-exception.iokit-user-client-class", ioKitClasses);
                }
            }
        }

        if (isNetworkingMulticastEnabled)
            entitlements.set ("com.apple.developer.networking.multicast", "<true/>");

        return entitlements;
    }

} // namespace drx::build_tools
