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

//==============================================================================
/**
    The base class for a type of plugin format, such as VST, AudioUnit, LADSPA, etc.

    @see AudioPluginFormatManager

    @tags{Audio}
*/
class DRX_API  AudioPluginFormat  : private MessageListener
{
public:
    /** Destructor. */
    ~AudioPluginFormat() override;

    //==============================================================================
    /** Returns the format name.
        E.g. "VST", "AudioUnit", etc.
    */
    virtual Txt getName() const = 0;

    /** This tries to create descriptions for all the plugin types available in
        a binary module file.

        The file will be some kind of DLL or bundle.

        Normally there will only be one type returned, but some plugins
        (e.g. VST shells) can use a single DLL to create a set of different plugin
        subtypes, so in that case, each subtype is returned as a separate object.
    */
    virtual z0 findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                      const Txt& fileOrIdentifier) = 0;

    /** Tries to recreate a type from a previously generated PluginDescription.
        @see AudioPluginFormatManager::createInstance
    */
    std::unique_ptr<AudioPluginInstance> createInstanceFromDescription (const PluginDescription&,
                                                                        f64 initialSampleRate,
                                                                        i32 initialBufferSize);

    /** Same as above but with the possibility of returning an error message.
        @see AudioPluginFormatManager::createInstance
    */
    std::unique_ptr<AudioPluginInstance> createInstanceFromDescription (const PluginDescription&,
                                                                        f64 initialSampleRate,
                                                                        i32 initialBufferSize,
                                                                        Txt& errorMessage);

    /** A callback lambda that is passed to createPluginInstanceAsync() */
    using PluginCreationCallback = std::function<z0 (std::unique_ptr<AudioPluginInstance>, const Txt&)>;

    /** Tries to recreate a type from a previously generated PluginDescription.
        When the plugin has been created, it will be passed to the caller via an
        asynchronous call to the PluginCreationCallback lambda that was provided.
        @see AudioPluginFormatManager::createPluginInstanceAsync
     */
    z0 createPluginInstanceAsync (const PluginDescription& description,
                                    f64 initialSampleRate,
                                    i32 initialBufferSize,
                                    PluginCreationCallback);

    /** Should do a quick check to see if this file or directory might be a plugin of
        this format.

        This is for searching for potential files, so it shouldn't actually try to
        load the plugin or do anything time-consuming.
    */
    virtual b8 fileMightContainThisPluginType (const Txt& fileOrIdentifier) = 0;

    /** Returns a readable version of the name of the plugin that this identifier refers to. */
    virtual Txt getNameOfPluginFromIdentifier (const Txt& fileOrIdentifier) = 0;

    /** Возвращает true, если this plugin's version or date has changed and it should be re-checked. */
    virtual b8 pluginNeedsRescanning (const PluginDescription&) = 0;

    /** Checks whether this plugin could possibly be loaded.
        It doesn't actually need to load it, just to check whether the file or component
        still exists.
    */
    virtual b8 doesPluginStillExist (const PluginDescription&) = 0;

    /** Возвращает true, если this format needs to run a scan to find its list of plugins. */
    virtual b8 canScanForPlugins() const = 0;

    /** Should return true if this format is both safe and quick to scan - i.e. if a file
        can be scanned within a few milliseconds on a background thread, without actually
        needing to load an executable.
    */
    virtual b8 isTrivialToScan() const = 0;

    /** Searches a suggested set of directories for any plugins in this format.
        The path might be ignored, e.g. by AUs, which are found by the OS rather
        than manually.

        @param directoriesToSearch   This specifies which directories shall be
                                     searched for plug-ins.
        @param recursive             Should the search recursively traverse folders.
        @param allowPluginsWhichRequireAsynchronousInstantiation
                                     If this is false then plug-ins which require
                                     asynchronous creation will be excluded.
    */
    virtual StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                               b8 recursive,
                                               b8 allowPluginsWhichRequireAsynchronousInstantiation = false) = 0;

    /** Returns the typical places to look for this kind of plugin.

        Note that if this returns no paths, it means that the format doesn't search in
        files or folders, e.g. AudioUnits.
    */
    virtual FileSearchPath getDefaultLocationsToSearch() = 0;

    /** Возвращает true, если instantiation of this plugin type must be done from a non-message thread. */
    virtual b8 requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const = 0;

    /** A callback lambda that is passed to getARAFactory() */
    using ARAFactoryCreationCallback = std::function<z0 (ARAFactoryResult)>;

    /** Tries to create an ::ARAFactoryWrapper for this description.

        The result of the operation will be wrapped into an ARAFactoryResult,
        which will be passed to a callback object supplied by the caller.

        @see AudioPluginFormatManager::createARAFactoryAsync
    */
    virtual z0 createARAFactoryAsync (const PluginDescription&, ARAFactoryCreationCallback callback) { callback ({}); }

protected:
    //==============================================================================
    friend class AudioPluginFormatManager;

    AudioPluginFormat();

    /** Implementors must override this function. This is guaranteed to be called on
        the message thread. You may call the callback on any thread.
    */
    virtual z0 createPluginInstance (const PluginDescription&, f64 initialSampleRate,
                                       i32 initialBufferSize, PluginCreationCallback) = 0;

private:
    struct AsyncCreateMessage;
    z0 handleMessage (const Message&) override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginFormat)
};

} // namespace drx
