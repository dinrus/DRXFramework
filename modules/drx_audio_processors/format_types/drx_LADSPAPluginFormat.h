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

#if (DRX_PLUGINHOST_LADSPA && (DRX_LINUX || DRX_BSD)) || DOXYGEN

//==============================================================================
/**
    Implements a plugin format manager for LADSPA plugins.

    @tags{Audio}
*/
class DRX_API  LADSPAPluginFormat   : public AudioPluginFormat
{
public:
    LADSPAPluginFormat();
    ~LADSPAPluginFormat() override;

    //==============================================================================
    static Txt getFormatName()                   { return "LADSPA"; }
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

private:
    //==============================================================================
    z0 createPluginInstance (const PluginDescription&, f64 initialSampleRate,
                               i32 initialBufferSize, PluginCreationCallback) override;
    b8 requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;
    z0 recursiveFileSearch (StringArray&, const File&, b8 recursive);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LADSPAPluginFormat)
};


#endif

}
