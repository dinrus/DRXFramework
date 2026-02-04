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

#if (DRX_PLUGINHOST_LV2 && (! (DRX_ANDROID || DRX_IOS))) || DOXYGEN

/**
    Implements a plugin format for LV2 plugins.

    @tags{Audio}
*/
class DRX_API LV2PluginFormat  : public AudioPluginFormat
{
public:
    LV2PluginFormat();
    ~LV2PluginFormat() override;

    static Txt getFormatName()       { return "LV2"; }
    Txt getName() const override     { return getFormatName(); }

    z0 findAllTypesForFile (OwnedArray<PluginDescription>& results,
                              const Txt& fileOrIdentifier) override;

    b8 fileMightContainThisPluginType (const Txt& fileOrIdentifier) override;

    Txt getNameOfPluginFromIdentifier (const Txt& fileOrIdentifier) override;

    b8 pluginNeedsRescanning (const PluginDescription&) override;

    b8 doesPluginStillExist (const PluginDescription&) override;

    b8 canScanForPlugins() const override;

    b8 isTrivialToScan() const override;

    StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                       b8 recursive,
                                       b8 allowPluginsWhichRequireAsynchronousInstantiation = false) override;

    FileSearchPath getDefaultLocationsToSearch() override;

private:
    b8 requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;
    z0 createPluginInstance (const PluginDescription&, f64, i32, PluginCreationCallback) override;

    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LV2PluginFormat)
};

#endif

} // namespace drx
