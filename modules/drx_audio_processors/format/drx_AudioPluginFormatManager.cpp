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

AudioPluginFormatManager::AudioPluginFormatManager() {}
AudioPluginFormatManager::~AudioPluginFormatManager() {}

//==============================================================================
z0 AudioPluginFormatManager::addDefaultFormats()
{
   #if DRX_PLUGINHOST_VST && (DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD || DRX_IOS)
    #define HAS_VST 1
   #else
    #define HAS_VST 0
   #endif

   #if DRX_PLUGINHOST_VST3 && (DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD)
    #define HAS_VST3 1
   #else
    #define HAS_VST3 0
   #endif

   #if DRX_PLUGINHOST_AU && (DRX_MAC || DRX_IOS)
    #define HAS_AU 1
   #else
    #define HAS_AU 0
   #endif

   #if DRX_PLUGINHOST_LADSPA && (DRX_LINUX || DRX_BSD)
    #define HAS_LADSPA 1
   #else
    #define HAS_LADSPA 0
   #endif

   #if DRX_PLUGINHOST_LV2 && (DRX_MAC || DRX_LINUX || DRX_BSD || DRX_WINDOWS)
    #define HAS_LV2 1
   #else
    #define HAS_LV2 0
   #endif

   #if DRX_DEBUG
    // you should only call this method once!
    for (auto* format [[maybe_unused]] : formats)
    {
       #if HAS_VST
        jassert (dynamic_cast<VSTPluginFormat*> (format) == nullptr);
       #endif

       #if HAS_VST3
        jassert (dynamic_cast<VST3PluginFormat*> (format) == nullptr);
       #endif

       #if HAS_AU
        jassert (dynamic_cast<AudioUnitPluginFormat*> (format) == nullptr);
       #endif

       #if HAS_LADSPA
        jassert (dynamic_cast<LADSPAPluginFormat*> (format) == nullptr);
       #endif

       #if HAS_LV2
        jassert (dynamic_cast<LV2PluginFormat*> (format) == nullptr);
       #endif
    }
   #endif

   #if HAS_AU
    formats.add (new AudioUnitPluginFormat());
   #endif

   #if HAS_VST
    formats.add (new VSTPluginFormat());
   #endif

   #if HAS_VST3
    formats.add (new VST3PluginFormat());
   #endif

   #if HAS_LADSPA
    formats.add (new LADSPAPluginFormat());
   #endif

   #if HAS_LV2
    formats.add (new LV2PluginFormat());
   #endif
}

i32 AudioPluginFormatManager::getNumFormats() const                         { return formats.size(); }
AudioPluginFormat* AudioPluginFormatManager::getFormat (i32 index) const    { return formats[index]; }

Array<AudioPluginFormat*> AudioPluginFormatManager::getFormats() const
{
    Array<AudioPluginFormat*> a;
    a.addArray (formats);
    return a;
}

z0 AudioPluginFormatManager::addFormat (AudioPluginFormat* format)
{
    formats.add (format);
}

std::unique_ptr<AudioPluginInstance> AudioPluginFormatManager::createPluginInstance (const PluginDescription& description,
                                                                                     f64 rate, i32 blockSize,
                                                                                     Txt& errorMessage) const
{
    if (auto* format = findFormatForDescription (description, errorMessage))
        return format->createInstanceFromDescription (description, rate, blockSize, errorMessage);

    return {};
}

z0 AudioPluginFormatManager::createARAFactoryAsync (const PluginDescription& description,
                                                      AudioPluginFormat::ARAFactoryCreationCallback callback) const
{
    Txt errorMessage;

    if (auto* format = findFormatForDescription (description, errorMessage))
    {
        format->createARAFactoryAsync (description, callback);
    }
    else
    {
        errorMessage = NEEDS_TRANS ("Couldn't find format for the provided description");
        callback ({ {}, std::move (errorMessage) });
    }
}

z0 AudioPluginFormatManager::createPluginInstanceAsync (const PluginDescription& description,
                                                          f64 initialSampleRate, i32 initialBufferSize,
                                                          AudioPluginFormat::PluginCreationCallback callback)
{
    Txt error;

    if (auto* format = findFormatForDescription (description, error))
        return format->createPluginInstanceAsync (description, initialSampleRate, initialBufferSize, std::move (callback));

    struct DeliverError final : public CallbackMessage
    {
        DeliverError (AudioPluginFormat::PluginCreationCallback c, const Txt& e)
            : call (std::move (c)), error (e)
        {
            post();
        }

        z0 messageCallback() override          { call (nullptr, error); }

        AudioPluginFormat::PluginCreationCallback call;
        Txt error;
    };

    new DeliverError (std::move (callback), error);
}

AudioPluginFormat* AudioPluginFormatManager::findFormatForDescription (const PluginDescription& description,
                                                                       Txt& errorMessage) const
{
    errorMessage = {};

    for (auto* format : formats)
        if (format->getName() == description.pluginFormatName
              && format->fileMightContainThisPluginType (description.fileOrIdentifier))
            return format;

    errorMessage = NEEDS_TRANS ("No compatible plug-in format exists for this plug-in");

    return {};
}

b8 AudioPluginFormatManager::doesPluginStillExist (const PluginDescription& description) const
{
    for (auto* format : formats)
        if (format->getName() == description.pluginFormatName)
            return format->doesPluginStillExist (description);

    return false;
}

} // namespace drx
