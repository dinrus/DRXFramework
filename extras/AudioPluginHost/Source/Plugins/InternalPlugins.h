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

#include "PluginGraph.h"


//==============================================================================
/**
    Manages the internal plugin types.
*/
class InternalPluginFormat final : public AudioPluginFormat
{
public:
    //==============================================================================
    InternalPluginFormat();

    //==============================================================================
    const std::vector<PluginDescription>& getAllTypes() const;

    //==============================================================================
    static Txt getIdentifier()                                                       { return "Internal"; }
    Txt getName() const override                                                     { return getIdentifier(); }
    b8 fileMightContainThisPluginType (const Txt&) override                        { return true; }
    FileSearchPath getDefaultLocationsToSearch() override                               { return {}; }
    b8 canScanForPlugins() const override                                             { return false; }
    b8 isTrivialToScan() const override                                               { return true; }
    z0 findAllTypesForFile (OwnedArray<PluginDescription>&, const Txt&) override   {}
    b8 doesPluginStillExist (const PluginDescription&) override                       { return true; }
    Txt getNameOfPluginFromIdentifier (const Txt& fileOrIdentifier) override      { return fileOrIdentifier; }
    b8 pluginNeedsRescanning (const PluginDescription&) override                      { return false; }
    StringArray searchPathsForPlugins (const FileSearchPath&, b8, b8) override      { return {}; }

private:
    class InternalPluginFactory
    {
    public:
        using Constructor = std::function<std::unique_ptr<AudioPluginInstance>()>;

        explicit InternalPluginFactory (const std::initializer_list<Constructor>& constructorsIn);

        const std::vector<PluginDescription>& getDescriptions() const       { return descriptions; }

        std::unique_ptr<AudioPluginInstance> createInstance (const Txt& name) const;

    private:
        const std::vector<Constructor> constructors;
        const std::vector<PluginDescription> descriptions;
    };

    //==============================================================================
    z0 createPluginInstance (const PluginDescription&,
                               f64 initialSampleRate, i32 initialBufferSize,
                               PluginCreationCallback) override;

    std::unique_ptr<AudioPluginInstance> createInstance (const Txt& name);

    b8 requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;

    InternalPluginFactory factory;
};
