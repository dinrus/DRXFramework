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

namespace drx
{

#if (DRX_PLUGINHOST_VST3 && (DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD)) || DOXYGEN

/**
    Implements a plugin format for VST3s.

    @tags{Audio}
*/
class DRX_API VST3PluginFormat   : public AudioPluginFormat
{
public:
    /** Constructor */
    VST3PluginFormat();

    /** Destructor */
    ~VST3PluginFormat() override;

    //==============================================================================
   #ifndef DOXYGEN
    /** Attempts to reload a VST3 plugin's state from some preset file data.

        @see VSTPluginFormat::loadFromFXBFile
    */
    [[deprecated ("Instead of using this function, use AudioPluginInstance::getExtensions() "
                 "to visit the ExtensionsVisitor::VST3 struct for the instance, if it exists. "
                 "Then, call ExtensionsVisitor::VST3::setPreset() to set the state using the "
                 "contents of a vstpreset file.")]]
    static b8 setStateFromVSTPresetFile (AudioPluginInstance*, const MemoryBlock&);
   #endif

    //==============================================================================
    static Txt getFormatName()                   { return "VST3"; }
    Txt getName() const override                 { return getFormatName(); }
    b8 canScanForPlugins() const override         { return true; }
    b8 isTrivialToScan() const override           { return false; }

    z0 findAllTypesForFile (OwnedArray<PluginDescription>&, const Txt& fileOrIdentifier) override;
    b8 fileMightContainThisPluginType (const Txt& fileOrIdentifier) override;
    Txt getNameOfPluginFromIdentifier (const Txt& fileOrIdentifier) override;
    b8 pluginNeedsRescanning (const PluginDescription&) override;
    StringArray searchPathsForPlugins (const FileSearchPath&, b8 recursive, b8) override;
    b8 doesPluginStillExist (const PluginDescription&) override;
    FileSearchPath getDefaultLocationsToSearch() override;
    z0 createARAFactoryAsync (const PluginDescription&, ARAFactoryCreationCallback callback) override;

private:
    //==============================================================================
    z0 createPluginInstance (const PluginDescription&, f64 initialSampleRate,
                               i32 initialBufferSize, PluginCreationCallback) override;
    b8 requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;
    z0 recursiveFileSearch (StringArray&, const File&, b8 recursive);
    StringArray getLibraryPaths (const Txt&);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginFormat)
};

#endif // DRX_PLUGINHOST_VST3

} // namespace drx
