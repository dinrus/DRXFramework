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

#if (DRX_PLUGINHOST_VST || DOXYGEN)

namespace drx
{

//==============================================================================
/**
    Implements a plugin format manager for VSTs.

    @tags{Audio}
*/
class DRX_API  VSTPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    VSTPluginFormat();
    ~VSTPluginFormat() override;

    //==============================================================================
    /** Attempts to retrieve the VSTXML data from a plugin.
        Will return nullptr if the plugin isn't a VST, or if it doesn't have any VSTXML.
    */
    static const XmlElement* getVSTXML (AudioPluginInstance* plugin);

    /** Attempts to reload a VST plugin's state from some FXB or FXP data. */
    static b8 loadFromFXBFile (AudioPluginInstance* plugin, ukk data, size_t dataSize);

    /** Attempts to save a VST's state to some FXP or FXB data. */
    static b8 saveToFXBFile (AudioPluginInstance* plugin, MemoryBlock& result, b8 asFXB);

    /** Attempts to get a VST's state as a chunk of memory. */
    static b8 getChunkData (AudioPluginInstance* plugin, MemoryBlock& result, b8 isPreset);

    /** Attempts to set a VST's state from a chunk of memory. */
    static b8 setChunkData (AudioPluginInstance* plugin, ukk data, i32 size, b8 isPreset);

    /** Given a suitable function pointer to a VSTPluginMain function, this will attempt to
        instantiate and return a plugin for it.
    */
    static AudioPluginInstance* createCustomVSTFromMainCall (uk entryPointFunction,
                                                             f64 initialSampleRate,
                                                             i32 initialBufferSize);

    //==============================================================================
    /** Base class for some extra functions that can be attached to a VST plugin instance. */
    class ExtraFunctions
    {
    public:
        virtual ~ExtraFunctions() {}

        /** This should return 10000 * the BPM at this position in the current edit. */
        virtual z64 getTempoAt (z64 samplePos) = 0;

        /** This should return the host's automation state.
            @returns 0 = not supported, 1 = off, 2 = read, 3 = write, 4 = read/write
        */
        virtual i32 getAutomationState() = 0;
    };

    /** Provides an ExtraFunctions callback object for a plugin to use.
        The plugin will take ownership of the object and will delete it automatically.
    */
    static z0 setExtraFunctions (AudioPluginInstance* plugin, ExtraFunctions* functions);

    //==============================================================================
    /** This simply calls directly to the VST's AEffect::dispatcher() function. */
    static pointer_sized_int DRX_CALLTYPE dispatcher (AudioPluginInstance*, i32, i32, pointer_sized_int, uk, f32);

    /** Given a VstEffectInterface* (aka vst::AEffect*), this will return the drx AudioPluginInstance
        that is being used to wrap it
    */
    static AudioPluginInstance* getPluginInstanceFromVstEffectInterface (uk aEffect);

    //==============================================================================
    static Txt getFormatName()                   { return "VST"; }
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

    /** Can be overridden to receive a callback when each member of a shell plugin is about to be
        tested during a call to findAllTypesForFile().
        Only the name and uid members of the PluginDescription are guaranteed to be valid when
        this is called.
    */
    virtual z0 aboutToScanVSTShellPlugin (const PluginDescription&);

private:
    //==============================================================================
    z0 createPluginInstance (const PluginDescription&, f64 initialSampleRate,
                               i32 initialBufferSize, PluginCreationCallback) override;
    b8 requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;
    z0 recursiveFileSearch (StringArray&, const File&, b8 recursive);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTPluginFormat)
};

} // namespace drx

#endif
