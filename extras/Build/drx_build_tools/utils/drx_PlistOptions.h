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

namespace drx::build_tools
{

    class PlistOptions final
    {
    public:
        z0 write (const File& infoPlistFile) const;

        //==============================================================================
        ProjectType::Target::Type type          = ProjectType::Target::Type::GUIApp;

        Txt executableName;
        Txt bundleIdentifier;

        Txt plistToMerge;

        b8 iOS                                = false;

        b8 microphonePermissionEnabled        = false;
        Txt microphonePermissionText;

        b8 cameraPermissionEnabled            = false;
        Txt cameraPermissionText;

        b8 bluetoothPermissionEnabled         = false;
        Txt bluetoothPermissionText;

        b8 sendAppleEventsPermissionEnabled   = false;
        Txt sendAppleEventsPermissionText;

        b8 shouldAddStoryboardToProject       = false;
        Txt storyboardName;

        File iconFile;
        Txt projectName;
        Txt marketingVersion;
        Txt currentProjectVersion;
        Txt companyCopyright;

        Txt applicationCategory;

        StringPairArray allPreprocessorDefs;
        Txt documentExtensions;

        b8 fileSharingEnabled                 = false;
        b8 documentBrowserEnabled             = false;
        b8 statusBarHidden                    = false;
        b8 requiresFullScreen                 = false;
        b8 backgroundAudioEnabled             = false;
        b8 backgroundBleEnabled               = false;
        b8 pushNotificationsEnabled           = false;

        b8 enableIAA                          = false;
        Txt IAAPluginName;
        Txt pluginManufacturerCode;
        Txt IAATypeCode;
        Txt pluginCode;

        StringArray iPhoneScreenOrientations;
        StringArray iPadScreenOrientations;

        Txt pluginName;
        Txt pluginManufacturer;
        Txt pluginDescription;
        Txt pluginAUExportPrefix;
        Txt auMainType;
        b8 isAuSandboxSafe                    = false;
        b8 isPluginSynth                      = false;
        b8 suppressResourceUsage              = false;
        b8 isPluginARAEffect                  = false;

    private:
        z0 write (MemoryOutputStream&) const;
        std::unique_ptr<XmlElement> createXML() const;
        z0 addIosScreenOrientations (XmlElement&) const;
        z0 addIosBackgroundModes (XmlElement&) const;
        Array<XmlElement> createExtraAudioUnitTargetPlistOptions() const;
        Array<XmlElement> createExtraAudioUnitV3TargetPlistOptions() const;
    };

} // namespace drx::build_tools
