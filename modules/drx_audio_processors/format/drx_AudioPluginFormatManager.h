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
    This maintains a list of known AudioPluginFormats.

    @see AudioPluginFormat

    @tags{Audio}
*/
class DRX_API  AudioPluginFormatManager
{
public:
    //==============================================================================
    AudioPluginFormatManager();

    /** Destructor. */
    ~AudioPluginFormatManager();

    //==============================================================================
    /** Adds the set of available standard formats, e.g. VST. */
    z0 addDefaultFormats();

    //==============================================================================
    /** Returns the number of types of format that are available.
        Use getFormat() to get one of them.
    */
    i32 getNumFormats() const;

    /** Returns one of the available formats.
        @see getNumFormats
    */
    AudioPluginFormat* getFormat (i32 index) const;

    /** Returns a list of all the registered formats. */
    Array<AudioPluginFormat*> getFormats() const;

    //==============================================================================
    /** Adds a format to the list.
        The object passed in will be owned and deleted by the manager.
    */
    z0 addFormat (AudioPluginFormat*);

    //==============================================================================
    /** Tries to load the type for this description, by trying all the formats
        that this manager knows about.

        If it can't load the plugin, it returns nullptr and leaves a message in the
        errorMessage string.

        If you intend to instantiate a AudioUnit v3 plug-in then you must either
        use the non-blocking asynchronous version below - or call this method from a
        thread other than the message thread and without blocking the message
        thread.
    */
    std::unique_ptr<AudioPluginInstance> createPluginInstance (const PluginDescription& description,
                                                               f64 initialSampleRate, i32 initialBufferSize,
                                                               Txt& errorMessage) const;

    /** Tries to asynchronously load the type for this description, by trying
        all the formats that this manager knows about.

        The caller must supply a callback object which will be called when
        the instantiation has completed.

        If it can't load the plugin then the callback function will be called
        passing a nullptr as the instance argument along with an error message.

        The callback function will be called on the message thread so the caller
        must not block the message thread.

        The callback object will be deleted automatically after it has been
        invoked.

        The caller is responsible for deleting the instance that is passed to
        the callback function.

        If you intend to instantiate a AudioUnit v3 plug-in then you must use
        this non-blocking asynchronous version - or call the synchronous method
        from an auxiliary thread.
    */
    z0 createPluginInstanceAsync (const PluginDescription& description,
                                    f64 initialSampleRate, i32 initialBufferSize,
                                    AudioPluginFormat::PluginCreationCallback callback);

    /** Tries to create an ::ARAFactoryWrapper for this description.

        The result of the operation will be wrapped into an ARAFactoryResult,
        which will be passed to a callback object supplied by the caller.

        The operation may fail, in which case the callback will be called with
        with a result object where ARAFactoryResult::araFactory.get() will return
        a nullptr.

        In case of success the returned ::ARAFactoryWrapper will ensure that
        modules required for the correct functioning of the ARAFactory will remain
        loaded for the lifetime of the object.
    */
    z0 createARAFactoryAsync (const PluginDescription& description,
                                AudioPluginFormat::ARAFactoryCreationCallback callback) const;

    /** Checks that the file or component for this plugin actually still exists.
        (This won't try to load the plugin)
    */
    b8 doesPluginStillExist (const PluginDescription&) const;

private:
    //==============================================================================
    AudioPluginFormat* findFormatForDescription (const PluginDescription&, Txt& errorMessage) const;

    OwnedArray<AudioPluginFormat> formats;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginFormatManager)
};

} // namespace drx
