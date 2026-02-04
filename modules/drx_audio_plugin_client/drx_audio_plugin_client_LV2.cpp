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

#if DrxPlugin_Build_LV2

#ifndef _SCL_SECURE_NO_WARNINGS
 #define _SCL_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
 #define _CRT_SECURE_NO_WARNINGS
#endif

#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_GUI_BASICS_INCLUDE_XHEADERS 1

#include <drx_audio_plugin_client/drx_audio_plugin_client.h>
#include <drx_audio_plugin_client/detail/drx_CheckSettingMacros.h>
#include <drx_audio_plugin_client/detail/drx_PluginUtilities.h>
#include <drx_audio_plugin_client/detail/drx_LinuxMessageThread.h>

#include <drx_audio_processors/utilities/drx_FlagCache.h>
#include <drx_audio_processors/format_types/drx_LegacyAudioParameter.cpp>

#include "DrxLV2Defines.h"
#include <drx_audio_processors/format_types/drx_LV2Common.h>

#include <fstream>

#define DRX_TURTLE_RECALL_URI "https://lv2-extensions.drx.com/turtle_recall"

#ifndef DrxPlugin_LV2URI
 #error "You need to define the DrxPlugin_LV2URI value! If you're using the Projucer/CMake, the definition will be written into DrxLV2Defines.h automatically."
#endif

namespace drx::lv2_client
{

constexpr b8 startsWithValidScheme (const std::string_view str)
{
    constexpr tukk prefixes[] { "http://", "https://", "urn:" };

    for (const std::string_view prefix : prefixes)
        if (prefix == str.substr (0, prefix.size()))
            return true;

    return false;
}

// If your LV2 plugin fails to build here, it may be because you haven't explicitly set an LV2 URI,
// or you've requested a malformed URI.
// If you're using the Projucer, update the value of the "LV2 URI" field in your project settings.
// If you're using CMake, specify a valid LV2URI argument to drx_add_plugin.
static_assert (startsWithValidScheme (DrxPlugin_LV2URI),
               "Your configured LV2 URI must include a leading scheme specifier.");

constexpr auto uriSeparator = ":";
const auto DrxPluginLV2UriUi      = Txt (DrxPlugin_LV2URI) + uriSeparator + "UI";
const auto DrxPluginLV2UriState   = Txt (DrxPlugin_LV2URI) + uriSeparator + "StateString";
const auto DrxPluginLV2UriProgram = Txt (DrxPlugin_LV2URI) + uriSeparator + "Program";

static const LV2_Feature* findMatchingFeature (const LV2_Feature* const* features, tukk uri)
{
    for (auto feature = features; *feature != nullptr; ++feature)
        if (std::strcmp ((*feature)->URI, uri) == 0)
            return *feature;

    return nullptr;
}

static b8 hasFeature (const LV2_Feature* const* features, tukk uri)
{
    return findMatchingFeature (features, uri) != nullptr;
}

template <typename Data>
Data findMatchingFeatureData (const LV2_Feature* const* features, tukk uri)
{
    if (const auto* feature = findMatchingFeature (features, uri))
        return static_cast<Data> (feature->data);

    return {};
}

static const LV2_Options_Option* findMatchingOption (const LV2_Options_Option* options, LV2_URID urid)
{
    for (auto option = options; option->value != nullptr; ++option)
        if (option->key == urid)
            return option;

    return nullptr;
}

class ParameterStorage final : private AudioProcessorListener
{
public:
    ParameterStorage (AudioProcessor& proc, LV2_URID_Map map)
        : processor (proc),
          mapFeature (map),
          legacyParameters (proc, false)
    {
        processor.addListener (this);
    }

    ~ParameterStorage() override
    {
        processor.removeListener (this);
    }

    /*  This is the string that will be used to uniquely identify the parameter.

        This string will be written into the plugin's manifest as an IRI, so it must be
        syntactically valid.

        We escape this string rather than writing the user-defined parameter ID directly to avoid
        writing a malformed manifest in the case that user IDs contain spaces or other reserved
        characters. This should allow users to keep the same param IDs for all plugin formats.
    */
    static Txt getIri (const AudioProcessorParameter& param)
    {
        const auto urlSanitised = URL::addEscapeChars (LegacyAudioParameter::getParamID (&param, false), true);
        const auto ttlSanitised = lv2_shared::sanitiseStringAsTtlName (urlSanitised);

        // If this is hit, the parameter ID could not be represented directly in the plugin ttl.
        // We'll replace offending characters with '_'.
        jassert (urlSanitised == ttlSanitised);

        return ttlSanitised;
    }

    z0 setValueFromHost (LV2_URID urid, f32 value) noexcept
    {
        const auto it = uridToIndexMap.find (urid);

        if (it == uridToIndexMap.end())
        {
            // No such parameter.
            jassertfalse;
            return;
        }

        if (auto* param = legacyParameters.getParamForIndex ((i32) it->second))
        {
            const auto scaledValue = [&]
            {
                if (auto* rangedParam = dynamic_cast<RangedAudioParameter*> (param))
                    return rangedParam->convertTo0to1 (value);

                return value;
            }();

            if (! approximatelyEqual (scaledValue, param->getValue()))
            {
                ScopedValueSetter<b8> scope (ignoreCallbacks, true);
                param->setValueNotifyingHost (scaledValue);
            }
        }
    }

    struct Options
    {
        b8 parameterValue, gestureBegin, gestureEnd;
    };

    static constexpr auto newClientValue = 1 << 0,
                          gestureBegan   = 1 << 1,
                          gestureEnded   = 1 << 2;

    template <typename Callback>
    z0 forEachChangedParameter (Callback&& callback)
    {
        stateCache.ifSet ([this, &callback] (size_t parameterIndex, f32, u32 bits)
        {
            const Options options { (bits & newClientValue) != 0,
                                    (bits & gestureBegan)   != 0,
                                    (bits & gestureEnded)   != 0 };

            callback (*legacyParameters.getParamForIndex ((i32) parameterIndex),
                      indexToUridMap[parameterIndex],
                      options);
        });
    }

private:
    z0 audioProcessorParameterChanged (AudioProcessor*, i32 parameterIndex, f32 value) override
    {
        if (! ignoreCallbacks)
            stateCache.setValueAndBits ((size_t) parameterIndex, value, newClientValue);
    }

    z0 audioProcessorParameterChangeGestureBegin (AudioProcessor*, i32 parameterIndex) override
    {
        if (! ignoreCallbacks)
            stateCache.setBits ((size_t) parameterIndex, gestureBegan);
    }

    z0 audioProcessorParameterChangeGestureEnd (AudioProcessor*, i32 parameterIndex) override
    {
        if (! ignoreCallbacks)
            stateCache.setBits ((size_t) parameterIndex, gestureEnded);
    }

    z0 audioProcessorChanged (AudioProcessor*, const ChangeDetails&) override {}

    AudioProcessor& processor;
    const LV2_URID_Map mapFeature;
    const LegacyAudioParametersWrapper legacyParameters;
    const std::vector<LV2_URID> indexToUridMap = [&]
    {
        std::vector<LV2_URID> result;

        for (auto* param : legacyParameters)
        {
            jassert ((size_t) param->getParameterIndex() == result.size());

            const auto uri  = DrxPlugin_LV2URI + Txt (uriSeparator) + getIri (*param);
            const auto urid = mapFeature.map (mapFeature.handle, uri.toRawUTF8());
            result.push_back (urid);
        }

        // If this is hit, some parameters have duplicate IDs.
        // This may be because the IDs resolve to the same string when removing characters that
        // are invalid in a TTL name.
        jassert (std::set<LV2_URID> (result.begin(), result.end()).size() == result.size());

        return result;
    }();
    const std::map<LV2_URID, size_t> uridToIndexMap = [&]
    {
        std::map<LV2_URID, size_t> result;

        for (const auto [index, urid] : enumerate (indexToUridMap))
            result.emplace (urid, index);

        return result;
    }();
    FlaggedFloatCache<3> stateCache { (size_t) legacyParameters.getNumParameters() };
    b8 ignoreCallbacks = false;

    DRX_LEAK_DETECTOR (ParameterStorage)
};

enum class PortKind { seqInput, seqOutput, latencyOutput, freeWheelingInput, enabledInput };

struct PortIndices
{
    PortIndices (i32 numInputsIn, i32 numOutputsIn)
        : numInputs (numInputsIn), numOutputs (numOutputsIn) {}

    i32 getPortIndexForAudioInput (i32 audioIndex) const noexcept
    {
        return audioIndex;
    }

    i32 getPortIndexForAudioOutput (i32 audioIndex) const noexcept
    {
        return audioIndex + numInputs;
    }

    i32 getPortIndexFor (PortKind p) const noexcept { return getMaxAudioPortIndex() + (i32) p; }

    // Audio ports are numbered from 0 to numInputs + numOutputs
    i32 getMaxAudioPortIndex() const noexcept { return numInputs + numOutputs; }

    i32 numInputs, numOutputs;
};

//==============================================================================
class PlayHead final : public AudioPlayHead
{
public:
    PlayHead (LV2_URID_Map mapFeatureIn, f64 sampleRateIn)
        : parser (mapFeatureIn), sampleRate (sampleRateIn)
    {
    }

    z0 invalidate() { info = nullopt; }

    z0 readNewInfo (const LV2_Atom_Event* event)
    {
        if (event->body.type != mLV2_ATOM__Object && event->body.type != mLV2_ATOM__Blank)
            return;

        const auto* object = reinterpret_cast<const LV2_Atom_Object*> (&event->body);

        if (object->body.otype != mLV2_TIME__Position)
            return;

        const LV2_Atom* atomFrame          = nullptr;
        const LV2_Atom* atomSpeed          = nullptr;
        const LV2_Atom* atomBar            = nullptr;
        const LV2_Atom* atomBeat           = nullptr;
        const LV2_Atom* atomBeatUnit       = nullptr;
        const LV2_Atom* atomBeatsPerBar    = nullptr;
        const LV2_Atom* atomBeatsPerMinute = nullptr;

        LV2_Atom_Object_Query query[] { { mLV2_TIME__frame,             &atomFrame },
                                        { mLV2_TIME__speed,             &atomSpeed },
                                        { mLV2_TIME__bar,               &atomBar },
                                        { mLV2_TIME__beat,              &atomBeat },
                                        { mLV2_TIME__beatUnit,          &atomBeatUnit },
                                        { mLV2_TIME__beatsPerBar,       &atomBeatsPerBar },
                                        { mLV2_TIME__beatsPerMinute,    &atomBeatsPerMinute },
                                        LV2_ATOM_OBJECT_QUERY_END };

        lv2_atom_object_query (object, query);

        info.emplace();

        // Carla always seems to give us an integral 'beat' even though I'd expect
        // it to be a floating-point value

        const auto numerator   = parser.parseNumericAtom<f32>   (atomBeatsPerBar);
        const auto denominator = parser.parseNumericAtom<i32> (atomBeatUnit);

        if (numerator.hasValue() && denominator.hasValue())
            info->setTimeSignature (TimeSignature { (i32) *numerator, (i32) *denominator });

        info->setBpm (parser.parseNumericAtom<f32> (atomBeatsPerMinute));
        info->setPpqPosition (parser.parseNumericAtom<f64> (atomBeat));
        info->setIsPlaying (! approximatelyEqual (parser.parseNumericAtom<f32> (atomSpeed).orFallback (0.0f), 0.0f));
        info->setBarCount (parser.parseNumericAtom<z64> (atomBar));

        if (const auto parsed = parser.parseNumericAtom<z64> (atomFrame))
        {
            info->setTimeInSamples (*parsed);
            info->setTimeInSeconds ((f64) *parsed / sampleRate);
        }
    }

    Optional<PositionInfo> getPosition() const override
    {
        return info;
    }

private:
    lv2_shared::NumericAtomParser parser;
    Optional<PositionInfo> info;
    f64 sampleRate;

   #define X(str) const LV2_URID m##str = parser.map (str);
    X (LV2_ATOM__Blank)
    X (LV2_ATOM__Object)
    X (LV2_TIME__Position)
    X (LV2_TIME__beat)
    X (LV2_TIME__beatUnit)
    X (LV2_TIME__beatsPerBar)
    X (LV2_TIME__beatsPerMinute)
    X (LV2_TIME__frame)
    X (LV2_TIME__speed)
    X (LV2_TIME__bar)
   #undef X

    DRX_LEAK_DETECTOR (PlayHead)
};

//==============================================================================
class Ports
{
public:
    Ports (LV2_URID_Map map, i32 numInputsIn, i32 numOutputsIn)
        : forge (map),
          indices (numInputsIn, numOutputsIn),
          mLV2_ATOM__Sequence (map.map (map.handle, LV2_ATOM__Sequence))
    {
        audioBuffers.resize (static_cast<size_t> (numInputsIn + numOutputsIn), nullptr);
    }

    z0 connect (i32 port, uk data)
    {
        // The following is not UB _if_ data really points to an object with the expected type.

        if (port == indices.getPortIndexFor (PortKind::seqInput))
        {
            inputData = static_cast<const LV2_Atom_Sequence*> (data);
        }
        else if (port == indices.getPortIndexFor (PortKind::seqOutput))
        {
            outputData = static_cast<LV2_Atom_Sequence*> (data);
        }
        else if (port == indices.getPortIndexFor (PortKind::latencyOutput))
        {
            latency = static_cast<f32*> (data);
        }
        else if (port == indices.getPortIndexFor (PortKind::freeWheelingInput))
        {
            freeWheeling = static_cast<f32*> (data);
        }
        else if (port == indices.getPortIndexFor (PortKind::enabledInput))
        {
            enabled = static_cast<f32*> (data);
        }
        else if (isPositiveAndBelow (port, indices.getMaxAudioPortIndex()))
        {
            audioBuffers[(size_t) port] = static_cast<f32*> (data);
        }
        else
        {
            // This port was not declared!
            jassertfalse;
        }
    }

    template <typename Callback>
    z0 forEachInputEvent (Callback&& callback)
    {
        if (inputData != nullptr && inputData->atom.type == mLV2_ATOM__Sequence)
            for (const auto* event : lv2_shared::SequenceIterator { lv2_shared::SequenceWithSize { inputData } })
                callback (event);
    }

    z0 prepareToWrite()
    {
        // Note: Carla seems to have a bug (verified with the eg-fifths plugin) where
        // the output buffer size is incorrect on alternate calls.
        forge.setBuffer (reinterpret_cast<tuk> (outputData), outputData->atom.size);
    }

    z0 writeLatency (i32 value)
    {
        if (latency != nullptr)
            *latency = (f32) value;
    }

    const f32* getBufferForAudioInput (i32 index) const noexcept
    {
        return audioBuffers[(size_t) indices.getPortIndexForAudioInput (index)];
    }

    f32* getBufferForAudioOutput (i32 index) const noexcept
    {
        return audioBuffers[(size_t) indices.getPortIndexForAudioOutput (index)];
    }

    b8 isFreeWheeling() const noexcept
    {
        if (freeWheeling != nullptr)
            return *freeWheeling > 0.5f;

        return false;
    }

    b8 isEnabled() const noexcept
    {
        if (enabled != nullptr)
            return *enabled > 0.5f;

        return true;
    }

    lv2_shared::AtomForge forge;
    PortIndices indices;

private:
    static constexpr auto numParamPorts = 3;
    const LV2_Atom_Sequence* inputData = nullptr;
    LV2_Atom_Sequence* outputData = nullptr;
    f32* latency = nullptr;
    f32* freeWheeling = nullptr;
    f32* enabled = nullptr;
    std::vector<f32*> audioBuffers;
    const LV2_URID mLV2_ATOM__Sequence;

    DRX_LEAK_DETECTOR (Ports)
};

class LV2PluginInstance final : private AudioProcessorListener
{
public:
    LV2PluginInstance (f64 sampleRate,
                       z64 maxBlockSize,
                       tukk,
                       LV2_URID_Map mapFeatureIn)
        : mapFeature (mapFeatureIn),
          playHead (mapFeature, sampleRate)
    {
        processor->addListener (this);
        processor->setPlayHead (&playHead);
        prepare (sampleRate, (i32) maxBlockSize);
    }

    z0 connect (u32 port, uk data)
    {
        ports.connect ((i32) port, data);
    }

    z0 activate() {}

    template<typename UnaryFunction>
    static z0 iterateAudioBuffer (AudioBuffer<f32>& ab, UnaryFunction fn)
    {
        f32* const* sampleData = ab.getArrayOfWritePointers();

        for (i32 c = ab.getNumChannels(); --c >= 0;)
            for (i32 s = ab.getNumSamples(); --s >= 0;)
                fn (sampleData[c][s]);
    }

    static i32 countNaNs (AudioBuffer<f32>& ab) noexcept
    {
        i32 count = 0;
        iterateAudioBuffer (ab, [&count] (f32 s)
                                {
                                    if (std::isnan (s))
                                        ++count;
                                });

        return count;
    }

    z0 run (u32 numSteps)
    {
        // If this is hit, the host is trying to process more samples than it told us to prepare
        jassert (static_cast<i32> (numSteps) <= processor->getBlockSize());

        midi.clear();
        playHead.invalidate();
        audio.setSize (audio.getNumChannels(), static_cast<i32> (numSteps), true, false, true);

        ports.forEachInputEvent ([&] (const LV2_Atom_Event* event)
        {
            struct Callback
            {
                explicit Callback (LV2PluginInstance& s) : self (s) {}

                z0 setParameter (LV2_URID property, f32 value) const noexcept
                {
                    self.parameters.setValueFromHost (property, value);
                }

                // The host probably shouldn't send us 'touched' messages.
                z0 gesture (LV2_URID, b8) const noexcept {}

                LV2PluginInstance& self;
            };

            patchSetHelper.processPatchSet (event, Callback { *this });

            playHead.readNewInfo (event);

            if (event->body.type == mLV2_MIDI__MidiEvent)
                midi.addEvent (event + 1, static_cast<i32> (event->body.size), static_cast<i32> (event->time.frames));
        });

        processor->setNonRealtime (ports.isFreeWheeling());

        for (auto i = 0, end = processor->getTotalNumInputChannels(); i < end; ++i)
            audio.copyFrom (i, 0, ports.getBufferForAudioInput (i), audio.getNumSamples());

        jassert (countNaNs (audio) == 0);

        {
            const ScopedLock lock { processor->getCallbackLock() };

            if (processor->isSuspended())
            {
                for (auto i = 0, end = processor->getTotalNumOutputChannels(); i < end; ++i)
                {
                    const auto ptr = ports.getBufferForAudioOutput (i);
                    std::fill (ptr, ptr + numSteps, 0.0f);
                }
            }
            else
            {
                const auto isEnabled = ports.isEnabled();

                if (auto* param = processor->getBypassParameter())
                {
                    param->setValueNotifyingHost (isEnabled ? 0.0f : 1.0f);
                    processor->processBlock (audio, midi);
                }
                else if (isEnabled)
                {
                    processor->processBlock (audio, midi);
                }
                else
                {
                    processor->processBlockBypassed (audio, midi);
                }
            }
        }

        for (auto i = 0, end = processor->getTotalNumOutputChannels(); i < end; ++i)
        {
            const auto src = audio.getReadPointer (i);
            const auto dst = ports.getBufferForAudioOutput (i);

            if (dst != nullptr)
                std::copy (src, src + numSteps, dst);
        }

        ports.prepareToWrite();
        auto* forge = ports.forge.get();
        lv2_shared::SequenceFrame sequence { forge, (u32) 0 };

        parameters.forEachChangedParameter ([&] (const AudioProcessorParameter& param,
                                                 LV2_URID paramUrid,
                                                 const ParameterStorage::Options& options)
        {
            const auto sendTouched = [&] (b8 state)
            {
                // TODO Implement begin/end change gesture support once it's supported by LV2
                ignoreUnused (state);
            };

            if (options.gestureBegin)
                sendTouched (true);

            if (options.parameterValue)
            {
                lv2_atom_forge_frame_time (forge, 0);

                lv2_shared::ObjectFrame object { forge, (u32) 0, patchSetHelper.mLV2_PATCH__Set };

                lv2_atom_forge_key (forge, patchSetHelper.mLV2_PATCH__property);
                lv2_atom_forge_urid (forge, paramUrid);

                lv2_atom_forge_key (forge, patchSetHelper.mLV2_PATCH__value);
                lv2_atom_forge_float (forge, [&]
                {
                    if (auto* rangedParam = dynamic_cast<const RangedAudioParameter*> (&param))
                        return rangedParam->convertFrom0to1 (rangedParam->getValue());

                    return param.getValue();
                }());
            }

            if (options.gestureEnd)
                sendTouched (false);
        });

        if (shouldSendStateChange.exchange (false))
        {
            lv2_atom_forge_frame_time (forge, 0);
            lv2_shared::ObjectFrame { forge, (u32) 0, mLV2_STATE__StateChanged };
        }

        for (const auto meta : midi)
        {
            const auto bytes = static_cast<u32> (meta.numBytes);
            lv2_atom_forge_frame_time (forge, meta.samplePosition);
            lv2_atom_forge_atom (forge, bytes, mLV2_MIDI__MidiEvent);
            lv2_atom_forge_write (forge, meta.data, bytes);
        }

        ports.writeLatency (processor->getLatencySamples());
    }

    z0 deactivate() {}

    LV2_State_Status store (LV2_State_Store_Function storeFn,
                            LV2_State_Handle handle,
                            u32,
                            const LV2_Feature* const*)
    {
        MemoryBlock block;
        processor->getStateInformation (block);
        const auto text = block.toBase64Encoding();
        storeFn (handle,
                 mDrxPluginLV2UriState,
                 text.toRawUTF8(),
                 text.getNumBytesAsUTF8() + 1,
                 mLV2_ATOM__String,
                 LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

        return LV2_STATE_SUCCESS;
    }

    LV2_State_Status retrieve (LV2_State_Retrieve_Function retrieveFn,
                               LV2_State_Handle handle,
                               u32,
                               const LV2_Feature* const*)
    {
        size_t size = 0;
        u32 type = 0;
        u32 dataFlags = 0;

        // Try retrieving a port index (if this is a 'program' preset).
        const auto* programData = retrieveFn (handle, mDrxPluginLV2UriProgram, &size, &type, &dataFlags);

        if (programData != nullptr && type == mLV2_ATOM__Int && size == sizeof (i32))
        {
            const auto programIndex = readUnaligned<i32> (programData);
            processor->setCurrentProgram (programIndex);
            return LV2_STATE_SUCCESS;
        }

        // This doesn't seem to be a 'program' preset, try setting the full state from a string instead.
        const auto* data = retrieveFn (handle, mDrxPluginLV2UriState, &size, &type, &dataFlags);

        if (data == nullptr)
            return LV2_STATE_ERR_NO_PROPERTY;

        if (type != mLV2_ATOM__String)
            return LV2_STATE_ERR_BAD_TYPE;

        Txt text (static_cast<tukk> (data), (size_t) size);
        MemoryBlock block;
        block.fromBase64Encoding (text);
        processor->setStateInformation (block.getData(), (i32) block.getSize());

        return LV2_STATE_SUCCESS;
    }

    std::unique_ptr<AudioProcessorEditor> createEditor()
    {
        return std::unique_ptr<AudioProcessorEditor> (processor->createEditorIfNeeded());
    }

    z0 editorBeingDeleted (AudioProcessorEditor* editor)
    {
        processor->editorBeingDeleted (editor);
    }

    static std::unique_ptr<AudioProcessor> createProcessorInstance()
    {
        auto result = createPluginFilterOfType (AudioProcessor::wrapperType_LV2);

       #if defined (DrxPlugin_PreferredChannelConfigurations)
        constexpr short channelConfigurations[][2] { DrxPlugin_PreferredChannelConfigurations };

        static_assert (numElementsInArray (channelConfigurations) > 0,
                       "DrxPlugin_PreferredChannelConfigurations must contain at least one entry");
        static_assert (channelConfigurations[0][0] > 0 || channelConfigurations[0][1] > 0,
                       "DrxPlugin_PreferredChannelConfigurations must have at least one input or output channel");
        result->setPlayConfigDetails (channelConfigurations[0][0], channelConfigurations[0][1], 44100.0, 1024);

        const auto desiredChannels = std::make_tuple (channelConfigurations[0][0], channelConfigurations[0][1]);
        const auto actualChannels  = std::make_tuple (result->getTotalNumInputChannels(), result->getTotalNumOutputChannels());

        if (desiredChannels != actualChannels)
            Logger::outputDebugString ("Failed to apply requested channel configuration!");
       #else
        result->enableAllBuses();
       #endif

        return result;
    }

private:
    z0 audioProcessorParameterChanged (AudioProcessor*, i32, f32) override {}

    z0 audioProcessorChanged (AudioProcessor*, const ChangeDetails& details) override
    {
        // Only check for non-parameter state here because:
        // - Latency is automatically written every block.
        // - There's no way for an LV2 plugin to report an internal program change.
        // - Parameter info is hard-coded in the plugin's turtle description.
        if (details.nonParameterStateChanged)
            shouldSendStateChange = true;
    }

    z0 prepare (f64 sampleRate, i32 maxBlockSize)
    {
        jassert (processor != nullptr);
        processor->setRateAndBufferSizeDetails (sampleRate, maxBlockSize);
        processor->prepareToPlay (sampleRate, maxBlockSize);

        const auto numChannels = jmax (processor->getTotalNumInputChannels(),
                                       processor->getTotalNumOutputChannels());

        midi.ensureSize (8192);
        audio.setSize (numChannels, maxBlockSize);
        audio.clear();
    }

    LV2_URID map (StringRef uri) const { return mapFeature.map (mapFeature.handle, uri); }

    ScopedDrxInitialiser_GUI scopedDrxInitialiser;

   #if DRX_LINUX || DRX_BSD
    SharedResourcePointer<detail::MessageThread> messageThread;
   #endif

    std::unique_ptr<AudioProcessor> processor = createProcessorInstance();
    const LV2_URID_Map mapFeature;
    ParameterStorage parameters { *processor, mapFeature };
    Ports ports { mapFeature,
                  processor->getTotalNumInputChannels(),
                  processor->getTotalNumOutputChannels() };
    lv2_shared::PatchSetHelper patchSetHelper { mapFeature, DrxPlugin_LV2URI };
    PlayHead playHead;
    MidiBuffer midi;
    AudioBuffer<f32> audio;
    std::atomic<b8> shouldSendStateChange { false };

   #define X(str) const LV2_URID m##str = map (str);
    X (DrxPluginLV2UriProgram)
    X (DrxPluginLV2UriState)
    X (LV2_ATOM__Int)
    X (LV2_ATOM__String)
    X (LV2_BUF_SIZE__maxBlockLength)
    X (LV2_BUF_SIZE__sequenceSize)
    X (LV2_MIDI__MidiEvent)
    X (LV2_PATCH__Set)
    X (LV2_STATE__StateChanged)
   #undef X

    DRX_LEAK_DETECTOR (LV2PluginInstance)
};

//==============================================================================
struct RecallFeature
{
    i32 (*doRecall) (tukk) = [] (tukk libraryPath) -> i32
    {
        const ScopedDrxInitialiser_GUI scope;
        const auto processor = LV2PluginInstance::createProcessorInstance();

        const Txt pathString { CharPointer_UTF8 { libraryPath } };

        const auto absolutePath = File::isAbsolutePath (pathString) ? File (pathString)
                                                                    : File::getCurrentWorkingDirectory().getChildFile (pathString);

        const auto writers = { writeManifestTtl, writeDspTtl, writeUiTtl };

        const auto wroteSuccessfully = [&processor, &absolutePath] (auto* fn)
        {
            const auto result = fn (*processor, absolutePath);

            if (! result.wasOk())
                std::cerr << result.getErrorMessage() << '\n';

            return result.wasOk();
        };

        return std::all_of (writers.begin(), writers.end(), wroteSuccessfully) ? 0 : 1;
    };

private:
    static Txt getPresetUri (i32 index)
    {
        return DrxPlugin_LV2URI + Txt (uriSeparator) + "preset" + Txt (index + 1);
    }

    static FileOutputStream openStream (const File& libraryPath, StringRef name)
    {
        return FileOutputStream { libraryPath.getSiblingFile (name + ".ttl") };
    }

    static Result prepareStream (FileOutputStream& stream)
    {
        if (const auto result = stream.getStatus(); ! result)
            return result;

        stream.setPosition (0);
        stream.truncate();
        return Result::ok();
    }

    static Result writeManifestTtl (AudioProcessor& proc, const File& libraryPath)
    {
        auto os = openStream (libraryPath, "manifest");

        if (const auto result = prepareStream (os); ! result)
            return result;

        os << "@prefix lv2:   <http://lv2plug.in/ns/lv2core#> .\n"
              "@prefix rdfs:  <http://www.w3.org/2000/01/rdf-schema#> .\n"
              "@prefix pset:  <http://lv2plug.in/ns/ext/presets#> .\n"
              "@prefix state: <http://lv2plug.in/ns/ext/state#> .\n"
              "@prefix ui:    <http://lv2plug.in/ns/extensions/ui#> .\n"
              "@prefix xsd:   <http://www.w3.org/2001/XMLSchema#> .\n"
              "\n"
              "<" DrxPlugin_LV2URI ">\n"
              "\ta lv2:Plugin ;\n"
              "\tlv2:binary <" << URL::addEscapeChars (libraryPath.getFileName(), false) << "> ;\n"
              "\trdfs:seeAlso <dsp.ttl> .\n";

        if (proc.hasEditor())
        {
           #if DRX_MAC
            #define DRX_LV2_UI_KIND "CocoaUI"
           #elif DRX_WINDOWS
            #define DRX_LV2_UI_KIND "WindowsUI"
           #elif DRX_LINUX || DRX_BSD
            #define DRX_LV2_UI_KIND "X11UI"
           #else
            #error "LV2 UI is unsupported on this platform"
           #endif

            os << "\n"
                  "<" << DrxPluginLV2UriUi << ">\n"
                  "\ta ui:" DRX_LV2_UI_KIND " ;\n"
                  "\tlv2:binary <" << URL::addEscapeChars (libraryPath.getFileName(), false) << "> ;\n"
                  "\trdfs:seeAlso <ui.ttl> .\n"
                  "\n";
        }

        for (auto i = 0, end = proc.getNumPrograms(); i < end; ++i)
        {
            os << "<" << getPresetUri (i) << ">\n"
                  "\ta pset:Preset ;\n"
                  "\tlv2:appliesTo <" DrxPlugin_LV2URI "> ;\n"
                  "\trdfs:label \"" << proc.getProgramName (i) << "\" ;\n"
                  "\tstate:state [ <" << DrxPluginLV2UriProgram << "> \"" << i << "\"^^xsd:i32 ; ] .\n"
                  "\n";
        }

        return Result::ok();
    }

    static std::vector<const AudioProcessorParameterGroup*> findAllSubgroupsDepthFirst (const AudioProcessorParameterGroup& group,
                                                                                        std::vector<const AudioProcessorParameterGroup*> foundSoFar = {})
    {
        foundSoFar.push_back (&group);

        for (auto* node : group)
        {
            if (auto* subgroup = node->getGroup())
                foundSoFar = findAllSubgroupsDepthFirst (*subgroup, std::move (foundSoFar));
        }

        return foundSoFar;
    }

    using GroupSymbolMap = std::map<const AudioProcessorParameterGroup*, Txt>;

    static Txt getFlattenedGroupSymbol (const AudioProcessorParameterGroup& group, Txt symbol = "")
    {
        if (auto* parent = group.getParent())
            return getFlattenedGroupSymbol (*parent, group.getID() + (symbol.isEmpty() ? "" : group.getSeparator() + symbol));

        return symbol;
    }

    static Txt getSymbolForGroup (const AudioProcessorParameterGroup& group)
    {
        const Txt allowedCharacters ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
        const auto base = getFlattenedGroupSymbol (group);

        if (base.isEmpty())
            return {};

        Txt copy;

        for (const auto character : base)
            copy << Txt::charToString (allowedCharacters.containsChar (character) ? character : (t32) '_');

        return "paramgroup_" +  copy;
    }

    static GroupSymbolMap getGroupsAndSymbols (const std::vector<const AudioProcessorParameterGroup*>& groups)
    {
        std::set<Txt> symbols;
        GroupSymbolMap result;

        for (const auto& group : groups)
        {
            const auto symbol = [&]
            {
                const auto idealSymbol = getSymbolForGroup (*group);

                if (symbols.find (idealSymbol) == symbols.cend())
                    return idealSymbol;

                for (auto i = 2; i < std::numeric_limits<i32>::max(); ++i)
                {
                    const auto toTest = idealSymbol + "_" + Txt (i);

                    if (symbols.find (toTest) == symbols.cend())
                        return toTest;
                }

                jassertfalse;
                return Txt();
            }();

            symbols.insert (symbol);
            result.emplace (group, symbol);
        }

        return result;
    }

    template <typename Fn>
    static z0 visitAllParameters (const GroupSymbolMap& groups, Fn&& fn)
    {
        for (const auto& group : groups)
            for (const auto* node : *group.first)
                if (auto* param = node->getParameter())
                    fn (group.second, *param);
    }

    static Result writeDspTtl (AudioProcessor& proc, const File& libraryPath)
    {
        auto os = openStream (libraryPath, "dsp");

        if (const auto result = prepareStream (os); ! result)
            return result;

        os << "@prefix atom:  <http://lv2plug.in/ns/ext/atom#> .\n"
              "@prefix bufs:  <http://lv2plug.in/ns/ext/buf-size#> .\n"
              "@prefix doap:  <http://usefulinc.com/ns/doap#> .\n"
              "@prefix foaf:  <http://xmlns.com/foaf/0.1/> .\n"
              "@prefix lv2:   <http://lv2plug.in/ns/lv2core#> .\n"
              "@prefix midi:  <http://lv2plug.in/ns/ext/midi#> .\n"
              "@prefix opts:  <http://lv2plug.in/ns/ext/options#> .\n"
              "@prefix param: <http://lv2plug.in/ns/ext/parameters#> .\n"
              "@prefix patch: <http://lv2plug.in/ns/ext/patch#> .\n"
              "@prefix pg:    <http://lv2plug.in/ns/ext/port-groups#> .\n"
              "@prefix plug:  <" DrxPlugin_LV2URI << uriSeparator << "> .\n"
              "@prefix pprop: <http://lv2plug.in/ns/ext/port-props#> .\n"
              "@prefix rdfs:  <http://www.w3.org/2000/01/rdf-schema#> .\n"
              "@prefix rdf:   <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n"
              "@prefix rsz:   <http://lv2plug.in/ns/ext/resize-port#> .\n"
              "@prefix state: <http://lv2plug.in/ns/ext/state#> .\n"
              "@prefix time:  <http://lv2plug.in/ns/ext/time#> .\n"
              "@prefix ui:    <http://lv2plug.in/ns/extensions/ui#> .\n"
              "@prefix units: <http://lv2plug.in/ns/extensions/units#> .\n"
              "@prefix urid:  <http://lv2plug.in/ns/ext/urid#> .\n"
              "@prefix xsd:   <http://www.w3.org/2001/XMLSchema#> .\n"
              "\n";

        LegacyAudioParametersWrapper legacyParameters (proc, false);

        const auto allGroups = findAllSubgroupsDepthFirst (legacyParameters.getGroup());
        const auto groupsAndSymbols = getGroupsAndSymbols (allGroups);

        const auto parameterVisitor = [&] (const Txt& symbol,
                                           const AudioProcessorParameter& param)
        {
            os << "plug:" << ParameterStorage::getIri (param) << "\n"
                  "\ta lv2:Parameter ;\n"
                  "\trdfs:label \"" << param.getName (1024) << "\" ;\n";

            if (symbol.isNotEmpty())
                os << "\tpg:group plug:" << symbol << " ;\n";

            os << "\trdfs:range atom:Float ;\n";

            const auto [defaultValue, min, max] = [&]
            {
                if (auto* rangedParam = dynamic_cast<const RangedAudioParameter*> (&param))
                {
                    return std::tuple (rangedParam->convertFrom0to1 (rangedParam->getDefaultValue()),
                                       rangedParam->getNormalisableRange().start,
                                       rangedParam->getNormalisableRange().end);
                }

                return std::tuple (param.getDefaultValue(), 0.0f, 1.0f);
            }();

            os << "\tlv2:default " << defaultValue << " ;\n"
                  "\tlv2:minimum " << min << " ;\n"
                  "\tlv2:maximum " << max;

            // Avoid writing out loads of scale points for parameters with lots of steps
            constexpr auto stepLimit = 1000;
            const auto numSteps = param.getNumSteps();

            if (param.isDiscrete() && 2 <= numSteps && numSteps < stepLimit)
            {
                os << "\t ;\n"
                      "\tlv2:portProperty lv2:enumeration " << (param.isBoolean() ? ", lv2:toggled " : "") << ";\n"
                      "\tlv2:scalePoint ";

                const auto strings = param.getAllValueStrings();

                for (const auto [counter, string] : enumerate (strings))
                {
                    const auto value = jmap ((f32) counter, 0.0f, (f32) numSteps - 1.0f, min, max);

                    os << (counter != 0 ? ", " : "") << "[\n"
                          "\t\trdfs:label \"" << string << "\" ;\n"
                          "\t\trdf:value " << value << " ;\n"
                          "\t]";
                }
            }

            os << " .\n\n";
        };

        visitAllParameters (groupsAndSymbols, parameterVisitor);

        for (const auto& groupInfo : groupsAndSymbols)
        {
            if (groupInfo.second.isEmpty())
                continue;

            os << "plug:" << groupInfo.second << "\n"
                  "\ta pg:Group ;\n";

            if (auto* parent = groupInfo.first->getParent())
            {
                if (parent->getParent() != nullptr)
                {
                    const auto it = groupsAndSymbols.find (parent);

                    if (it != groupsAndSymbols.cend())
                        os << "\tpg:subGroupOf plug:" << it->second << " ;\n";
                }
            }

            os << "\tlv2:symbol \"" << groupInfo.second << "\" ;\n"
                  "\tlv2:name \"" << groupInfo.first->getName() << "\" .\n\n";
        }

        const auto getBaseBusName = [] (b8 isInput) { return isInput ? "input_group_" : "output_group_"; };

        for (const auto isInput : { true, false })
        {
            const auto baseBusName = getBaseBusName (isInput);
            const auto groupKind = isInput ? "InputGroup" : "OutputGroup";
            const auto busCount = proc.getBusCount (isInput);

            for (auto i = 0; i < busCount; ++i)
            {
                if (const auto* bus = proc.getBus (isInput, i))
                {
                    os << "plug:" << baseBusName << (i + 1) << "\n"
                          "\ta pg:" << groupKind << " ;\n"
                          "\tlv2:name \"" << bus->getName() << "\" ;\n"
                          "\tlv2:symbol \"" << baseBusName  << (i + 1) << "\" .\n\n";
                }
            }
        }

        os << "<" DrxPlugin_LV2URI ">\n";

        if (proc.hasEditor())
            os << "\tui:ui <" << DrxPluginLV2UriUi << "> ;\n";

        const auto versionParts = StringArray::fromTokens (DrxPlugin_VersionString, ".", "");

        const auto getVersionOrZero = [&] (i32 indexFromBack)
        {
            const auto str = versionParts[versionParts.size() - indexFromBack];
            return str.isEmpty() ? 0 : str.getIntValue();
        };

        const auto minorVersion = getVersionOrZero (2);
        const auto microVersion = getVersionOrZero (1);

        os << "\ta "
             #if DrxPlugin_IsSynth
              "lv2:InstrumentPlugin"
             #else
              "lv2:Plugin"
             #endif
              " ;\n"
              "\tdoap:name \"" DrxPlugin_Name "\" ;\n"
              "\tdoap:description \"" DrxPlugin_Desc "\" ;\n"
              "\tlv2:minorVersion " << minorVersion << " ;\n"
              "\tlv2:microVersion " << microVersion << " ;\n"
              "\tdoap:maintainer [\n"
              "\t\ta foaf:Person ;\n"
              "\t\tfoaf:name \"" DrxPlugin_Manufacturer "\" ;\n"
              "\t\tfoaf:homepage <" DrxPlugin_ManufacturerWebsite "> ;\n"
              "\t\tfoaf:mbox <" DrxPlugin_ManufacturerEmail "> ;\n"
              "\t] ;\n"
              "\tdoap:release [\n"
              "\t\ta doap:Version ;\n"
              "\t\tdoap:revision \"" DrxPlugin_VersionString "\" ;\n"
              "\t] ;\n"
              "\tlv2:optionalFeature\n"
              "\t\tlv2:hardRTCapable ;\n"
              "\tlv2:extensionData\n"
              "\t\tstate:interface ;\n"
              "\tlv2:requiredFeature\n"
              "\t\turid:map ,\n"
              "\t\topts:options ,\n"
              "\t\tbufs:boundedBlockLength ;\n";

        for (const auto isInput : { true, false })
        {
            const auto kind = isInput ? "mainInput" : "mainOutput";

            if (proc.getBusCount (isInput) > 0)
                os << "\tpg:" << kind << " plug:" << getBaseBusName (isInput) << "1 ;\n";
        }

        if (legacyParameters.size() != 0)
        {
            for (const auto header : { "writable", "readable" })
            {
                os << "\tpatch:" << header;

                b8 isFirst = true;

                for (const auto* param : legacyParameters)
                {
                    os << (isFirst ? "" : " ,") << "\n\t\tplug:" << ParameterStorage::getIri (*param);
                    isFirst = false;
                }

                os << " ;\n";
            }
        }

        os << "\tlv2:port [\n";

        const PortIndices indices (proc.getTotalNumInputChannels(),
                                   proc.getTotalNumOutputChannels());

        const auto designationMap = [&]
        {
            std::map<AudioChannelSet::ChannelType, Txt> result;

            for (const auto& pair : lv2_shared::channelDesignationMap)
                result.emplace (pair.second, pair.first);

            return result;
        }();

        // TODO add support for specific audio group kinds
        for (const auto isInput : { true, false })
        {
            const auto baseBusName = getBaseBusName (isInput);
            const auto portKind = isInput ? "InputPort" : "OutputPort";
            const auto portName = isInput ? "Audio In " : "Audio Out ";
            const auto portSymbol = isInput ? "audio_in_" : "audio_out_";
            const auto busCount = proc.getBusCount (isInput);

            auto channelCounter = 0;

            for (auto busIndex = 0; busIndex < busCount; ++busIndex)
            {
                if (const auto* bus = proc.getBus (isInput, busIndex))
                {
                    const auto channelCount = bus->getNumberOfChannels();
                    const auto optionalBus = ! bus->isEnabledByDefault();

                    for (auto channelIndex = 0; channelIndex < channelCount; ++channelIndex, ++channelCounter)
                    {
                        const auto portIndex = isInput ? indices.getPortIndexForAudioInput (channelCounter)
                                                       : indices.getPortIndexForAudioOutput (channelCounter);

                        os << "\t\ta lv2:" << portKind << " , lv2:AudioPort ;\n"
                              "\t\tlv2:index " << portIndex << " ;\n"
                              "\t\tlv2:symbol \"" << portSymbol << (channelCounter + 1) << "\" ;\n"
                              "\t\tlv2:name \"" << portName << (channelCounter + 1) << "\" ;\n"
                              "\t\tpg:group plug:" << baseBusName << (busIndex + 1) << " ;\n";

                        if (optionalBus)
                            os << "\t\tlv2:portProperty lv2:connectionOptional ;\n";

                        const auto designation = bus->getCurrentLayout().getTypeOfChannel (channelIndex);
                        const auto it = designationMap.find (designation);

                        if (it != designationMap.end())
                            os << "\t\tlv2:designation <" << it->second << "> ;\n";

                        os << "\t] , [\n";
                    }
                }
            }
        }

        // In the event that the plugin decides to send all of its parameters in one go,
        // we should ensure that the output buffer is large enough to accommodate, with some
        // extra room for the sequence header, MIDI messages etc..
        const auto patchSetSizeBytes = 72;
        const auto additionalSize = 8192;
        const auto atomPortMinSize = proc.getParameters().size() * patchSetSizeBytes + additionalSize;

        os << "\t\ta lv2:InputPort , atom:AtomPort ;\n"
              "\t\trsz:minimumSize " << atomPortMinSize << " ;\n"
              "\t\tatom:bufferType atom:Sequence ;\n"
              "\t\tatom:supports\n";

       #if ! DrxPlugin_IsSynth
        if (proc.acceptsMidi() || proc.isMidiEffect())
       #endif
            os << "\t\t\tmidi:MidiEvent ,\n";

        os << "\t\t\tpatch:Message ,\n"
              "\t\t\ttime:Position ;\n"
              "\t\tlv2:designation lv2:control ;\n"
              "\t\tlv2:index " << indices.getPortIndexFor (PortKind::seqInput) << " ;\n"
              "\t\tlv2:symbol \"in\" ;\n"
              "\t\tlv2:name \"In\" ;\n"
              "\t] , [\n"
              "\t\ta lv2:OutputPort , atom:AtomPort ;\n"
              "\t\trsz:minimumSize " << atomPortMinSize << " ;\n"
              "\t\tatom:bufferType atom:Sequence ;\n"
              "\t\tatom:supports\n";

        if (proc.producesMidi() || proc.isMidiEffect())
            os << "\t\t\tmidi:MidiEvent ,\n";

        os << "\t\t\tpatch:Message ;\n"
              "\t\tlv2:designation lv2:control ;\n"
              "\t\tlv2:index " << indices.getPortIndexFor (PortKind::seqOutput) << " ;\n"
              "\t\tlv2:symbol \"out\" ;\n"
              "\t\tlv2:name \"Out\" ;\n"
              "\t] , [\n"
              "\t\ta lv2:OutputPort , lv2:ControlPort ;\n"
              "\t\tlv2:designation lv2:latency ;\n"
              "\t\tlv2:symbol \"latency\" ;\n"
              "\t\tlv2:name \"Latency\" ;\n"
              "\t\tlv2:index " << indices.getPortIndexFor (PortKind::latencyOutput) << " ;\n"
              "\t\tlv2:portProperty lv2:reportsLatency , lv2:integer , lv2:connectionOptional , pprop:notOnGUI ;\n"
              "\t\tunits:unit units:frame ;\n"
              "\t] , [\n"
              "\t\ta lv2:InputPort , lv2:ControlPort ;\n"
              "\t\tlv2:designation lv2:freeWheeling ;\n"
              "\t\tlv2:symbol \"freeWheeling\" ;\n"
              "\t\tlv2:name \"Free Wheeling\" ;\n"
              "\t\tlv2:default 0.0 ;\n"
              "\t\tlv2:minimum 0.0 ;\n"
              "\t\tlv2:maximum 1.0 ;\n"
              "\t\tlv2:index " << indices.getPortIndexFor (PortKind::freeWheelingInput) << " ;\n"
              "\t\tlv2:portProperty lv2:toggled , lv2:connectionOptional , pprop:notOnGUI ;\n"
              "\t] , [\n"
              "\t\ta lv2:InputPort , lv2:ControlPort ;\n"
              "\t\tlv2:designation lv2:enabled ;\n"
              "\t\tlv2:symbol \"enabled\" ;\n"
              "\t\tlv2:name \"Enabled\" ;\n"
              "\t\tlv2:default 1.0 ;\n"
              "\t\tlv2:minimum 0.0 ;\n"
              "\t\tlv2:maximum 1.0 ;\n"
              "\t\tlv2:index " << indices.getPortIndexFor (PortKind::enabledInput) << " ;\n"
              "\t\tlv2:portProperty lv2:toggled , lv2:connectionOptional , pprop:notOnGUI ;\n"
              "\t] ;\n"
              "\topts:supportedOption\n"
              "\t\tbufs:maxBlockLength .\n";

        return Result::ok();
    }

    static Result writeUiTtl (AudioProcessor& proc, const File& libraryPath)
    {
        if (! proc.hasEditor())
            return Result::ok();

        auto os = openStream (libraryPath, "ui");

        if (const auto result = prepareStream (os); ! result)
            return result;

        const auto editorInstance = rawToUniquePtr (proc.createEditor());
        const auto resizeFeatureString = editorInstance->isResizable() ? "ui:resize" : "ui:noUserResize";

        os << "@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .\n"
              "@prefix opts: <http://lv2plug.in/ns/ext/options#> .\n"
              "@prefix param: <http://lv2plug.in/ns/ext/parameters#> .\n"
              "@prefix ui:   <http://lv2plug.in/ns/extensions/ui#> .\n"
              "@prefix urid: <http://lv2plug.in/ns/ext/urid#> .\n"
              "\n"
              "<" << DrxPluginLV2UriUi << ">\n"
              "\tlv2:extensionData\n"
             #if DRX_LINUX || DRX_BSD
              "\t\tui:idleInterface ,\n"
             #endif
              "\t\topts:interface ,\n"
              "\t\tui:noUserResize ,\n" // resize and noUserResize are always present in the extension data array
              "\t\tui:resize ;\n"
              "\n"
              "\tlv2:requiredFeature\n"
             #if DRX_LINUX || DRX_BSD
              "\t\tui:idleInterface ,\n"
             #endif
              "\t\turid:map ,\n"
              "\t\tui:parent ,\n"
              "\t\t<http://lv2plug.in/ns/ext/instance-access> ;\n"
              "\n"
              "\tlv2:optionalFeature\n"
              "\t\t" << resizeFeatureString << " ,\n"
              "\t\topts:interface ,\n"
              "\t\topts:options ;\n\n"
              "\topts:supportedOption\n"
              "\t\tui:scaleFactor ,\n"
              "\t\tparam:sampleRate .\n";

        return Result::ok();
    }

    DRX_LEAK_DETECTOR (RecallFeature)
};

//==============================================================================
LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor (u32 index)
{
    if (index != 0)
        return nullptr;

    static const LV2_Descriptor descriptor
    {
        DrxPlugin_LV2URI, // TODO some constexpr check that this is a valid URI in terms of RFC 3986
        [] (const LV2_Descriptor*,
            f64 sampleRate,
            tukk pathToBundle,
            const LV2_Feature* const* features) -> LV2_Handle
        {
            const auto* mapFeature = findMatchingFeatureData<const LV2_URID_Map*> (features, LV2_URID__map);

            if (mapFeature == nullptr)
            {
                // The host doesn't provide the 'urid map' feature
                jassertfalse;
                return nullptr;
            }

            const auto boundedBlockLength = hasFeature (features, LV2_BUF_SIZE__boundedBlockLength);

            if (! boundedBlockLength)
            {
                // The host doesn't provide the 'bounded block length' feature
                jassertfalse;
                return nullptr;
            }

            const auto* options = findMatchingFeatureData<const LV2_Options_Option*> (features, LV2_OPTIONS__options);

            if (options == nullptr)
            {
                // The host doesn't provide the 'options' feature
                jassertfalse;
                return nullptr;
            }

            const lv2_shared::NumericAtomParser parser { *mapFeature };
            const auto blockLengthUrid = mapFeature->map (mapFeature->handle, LV2_BUF_SIZE__maxBlockLength);
            const auto blockSize = parser.parseNumericOption<z64> (findMatchingOption (options, blockLengthUrid));

            if (! blockSize.hasValue())
            {
                // The host doesn't specify a maximum block size
                jassertfalse;
                return nullptr;
            }

            return new LV2PluginInstance { sampleRate, *blockSize, pathToBundle, *mapFeature };
        },
        [] (LV2_Handle instance, u32 port, uk data)
        {
            static_cast<LV2PluginInstance*> (instance)->connect (port, data);
        },
        [] (LV2_Handle instance) { static_cast<LV2PluginInstance*> (instance)->activate(); },
        [] (LV2_Handle instance, u32 sampleCount)
        {
            static_cast<LV2PluginInstance*> (instance)->run (sampleCount);
        },
        [] (LV2_Handle instance) { static_cast<LV2PluginInstance*> (instance)->deactivate(); },
        [] (LV2_Handle instance)
        {
            DRX_AUTORELEASEPOOL
            {
                delete static_cast<LV2PluginInstance*> (instance);
            }
        },
        [] (tukk uri) -> ukk
        {
            const auto uriMatches = [&] (const LV2_Feature& f) { return std::strcmp (f.URI, uri) == 0; };

            static RecallFeature recallFeature;

            static LV2_State_Interface stateInterface
            {
                [] (LV2_Handle instance,
                    LV2_State_Store_Function store,
                    LV2_State_Handle handle,
                    u32 flags,
                    const LV2_Feature* const* features) -> LV2_State_Status
                {
                    return static_cast<LV2PluginInstance*> (instance)->store (store, handle, flags, features);
                },
                [] (LV2_Handle instance,
                    LV2_State_Retrieve_Function retrieve,
                    LV2_State_Handle handle,
                    u32 flags,
                    const LV2_Feature* const* features) -> LV2_State_Status
                {
                    return static_cast<LV2PluginInstance*> (instance)->retrieve (retrieve, handle, flags, features);
                }
            };

            static const LV2_Feature features[] { { DRX_TURTLE_RECALL_URI, &recallFeature },
                                                  { LV2_STATE__interface,   &stateInterface } };

            const auto it = std::find_if (std::begin (features), std::end (features), uriMatches);
            return it != std::end (features) ? it->data : nullptr;
        }
    };

    return &descriptor;
}

static Optional<f32> findScaleFactor (const LV2_URID_Map* symap, const LV2_Options_Option* options)
{
    if (options == nullptr || symap == nullptr)
        return {};

    const lv2_shared::NumericAtomParser parser { *symap };
    const auto scaleFactorUrid = symap->map (symap->handle, LV2_UI__scaleFactor);
    const auto* scaleFactorOption = findMatchingOption (options, scaleFactorUrid);
    return parser.parseNumericOption<f32> (scaleFactorOption);
}

class LV2UIInstance final : private Component,
                            private ComponentListener
{
public:
    LV2UIInstance (tukk,
                   tukk,
                   LV2UI_Write_Function writeFunctionIn,
                   LV2UI_Controller controllerIn,
                   LV2UI_Widget* widget,
                   LV2PluginInstance* pluginIn,
                   LV2UI_Widget parentIn,
                   const LV2_URID_Map* symapIn,
                   const LV2UI_Resize* resizeFeatureIn,
                   Optional<f32> scaleFactorIn)
        : writeFunction (writeFunctionIn),
          controller (controllerIn),
          plugin (pluginIn),
          parent (parentIn),
          symap (symapIn),
          resizeFeature (resizeFeatureIn),
          scaleFactor (scaleFactorIn),
          editor (plugin->createEditor())
    {
        jassert (plugin != nullptr);
        jassert (parent != nullptr);
        jassert (editor != nullptr);

        if (editor == nullptr)
            return;

        const auto bounds = getSizeToContainChild();
        setSize (bounds.getWidth(), bounds.getHeight());

        addAndMakeVisible (*editor);

        setBroughtToFrontOnMouseClick (true);
        setOpaque (true);
        setVisible (false);
        removeFromDesktop();
        addToDesktop (detail::PluginUtilities::getDesktopFlags (editor.get()), parent);
        editor->addComponentListener (this);

        *widget = getWindowHandle();

        setVisible (true);

        editor->setScaleFactor (getScaleFactor());
        requestResize();
    }

    ~LV2UIInstance() override
    {
        plugin->editorBeingDeleted (editor.get());
    }

    // This is called by the host when a parameter changes.
    // We don't care, our UI will listen to the processor directly.
    z0 portEvent (u32, u32, u32, ukk) {}

    // Called when the host requests a resize
    i32 resize (i32 width, i32 height)
    {
        const ScopedValueSetter<b8> scope (hostRequestedResize, true);
        setSize (width, height);
        return 0;
    }

    // Called by the host to give us an opportunity to process UI events
    z0 idleCallback()
    {
       #if DRX_LINUX || DRX_BSD
        messageThread->processPendingEvents();
       #endif
    }

    z0 resized() override
    {
        const ScopedValueSetter<b8> scope (hostRequestedResize, true);

        if (editor != nullptr)
        {
            const auto localArea = editor->getLocalArea (this, getLocalBounds());
            editor->setBoundsConstrained ({ localArea.getWidth(), localArea.getHeight() });
        }
    }

    z0 paint (Graphics& g) override { g.fillAll (Colors::black); }

    u32 getOptions (LV2_Options_Option* options)
    {
        const auto scaleFactorUrid = symap->map (symap->handle, LV2_UI__scaleFactor);
        const auto floatUrid = symap->map (symap->handle, LV2_ATOM__Float);;

        for (auto* opt = options; opt->key != 0; ++opt)
        {
            if (opt->context != LV2_OPTIONS_INSTANCE || opt->subject != 0 || opt->key != scaleFactorUrid)
                continue;

            if (scaleFactor.hasValue())
            {
                opt->type = floatUrid;
                opt->size = sizeof (f32);
                opt->value = &(*scaleFactor);
            }
        }

        return LV2_OPTIONS_SUCCESS;
    }

    u32 setOptions (const LV2_Options_Option* options)
    {
        const auto scaleFactorUrid = symap->map (symap->handle, LV2_UI__scaleFactor);
        const auto floatUrid = symap->map (symap->handle, LV2_ATOM__Float);;

        for (auto* opt = options; opt->key != 0; ++opt)
        {
            if (opt->context != LV2_OPTIONS_INSTANCE
                || opt->subject != 0
                || opt->key != scaleFactorUrid
                || opt->type != floatUrid
                || opt->size != sizeof (f32))
            {
                continue;
            }

            scaleFactor = *static_cast<const f32*> (opt->value);
            updateScale();
        }

        return LV2_OPTIONS_SUCCESS;
    }

private:
    z0 updateScale()
    {
        editor->setScaleFactor (getScaleFactor());
        requestResize();
    }

    Rectangle<i32> getSizeToContainChild() const
    {
        if (editor != nullptr)
            return getLocalArea (editor.get(), editor->getLocalBounds());

        return {};
    }

    f32 getScaleFactor() const noexcept
    {
        return scaleFactor.hasValue() ? *scaleFactor : 1.0f;
    }

    z0 componentMovedOrResized (Component&, b8, b8 wasResized) override
    {
        if (! hostRequestedResize && wasResized)
            requestResize();
    }

    z0 write (u32 portIndex, u32 bufferSize, u32 portProtocol, ukk data)
    {
        writeFunction (controller, portIndex, bufferSize, portProtocol, data);
    }

    z0 requestResize()
    {
        if (editor == nullptr)
            return;

        const auto bounds = getSizeToContainChild();

        if (resizeFeature == nullptr)
            return;

        if (auto* fn = resizeFeature->ui_resize)
            fn (resizeFeature->handle, bounds.getWidth(), bounds.getHeight());

        setSize (bounds.getWidth(), bounds.getHeight());
        repaint();
    }

   #if DRX_LINUX || DRX_BSD
    SharedResourcePointer<detail::HostDrivenEventLoop> messageThread;
   #endif

    LV2UI_Write_Function writeFunction;
    LV2UI_Controller controller;
    LV2PluginInstance* plugin;
    LV2UI_Widget parent;
    const LV2_URID_Map* symap = nullptr;
    const LV2UI_Resize* resizeFeature = nullptr;
    Optional<f32> scaleFactor;
    std::unique_ptr<AudioProcessorEditor> editor;
    b8 hostRequestedResize = false;

    DRX_LEAK_DETECTOR (LV2UIInstance)
};

LV2_SYMBOL_EXPORT const LV2UI_Descriptor* lv2ui_descriptor (u32 index)
{
    if (index != 0)
        return nullptr;

    static const LV2UI_Descriptor descriptor
    {
        DrxPluginLV2UriUi.toRawUTF8(), // TODO some constexpr check that this is a valid URI in terms of RFC 3986
        [] (const LV2UI_Descriptor*,
            tukk pluginUri,
            tukk bundlePath,
            LV2UI_Write_Function writeFunction,
            LV2UI_Controller controller,
            LV2UI_Widget* widget,
            const LV2_Feature* const* features) -> LV2UI_Handle
        {
           #if DRX_LINUX || DRX_BSD
            SharedResourcePointer<detail::HostDrivenEventLoop> messageThread;
           #endif

            auto* plugin = findMatchingFeatureData<LV2PluginInstance*> (features, LV2_INSTANCE_ACCESS_URI);

            if (plugin == nullptr)
            {
                // No instance access
                jassertfalse;
                return nullptr;
            }

            auto* parent = findMatchingFeatureData<LV2UI_Widget> (features, LV2_UI__parent);

            if (parent == nullptr)
            {
                // No parent access
                jassertfalse;
                return nullptr;
            }

            auto* resizeFeature = findMatchingFeatureData<const LV2UI_Resize*> (features, LV2_UI__resize);

            const auto* symap = findMatchingFeatureData<const LV2_URID_Map*>       (features, LV2_URID__map);
            const auto scaleFactor = findScaleFactor (symap, findMatchingFeatureData<const LV2_Options_Option*> (features, LV2_OPTIONS__options));

            return new LV2UIInstance { pluginUri,
                                       bundlePath,
                                       writeFunction,
                                       controller,
                                       widget,
                                       plugin,
                                       parent,
                                       symap,
                                       resizeFeature,
                                       scaleFactor };
        },
        [] (LV2UI_Handle ui)
        {
           #if DRX_LINUX || DRX_BSD
            SharedResourcePointer<detail::HostDrivenEventLoop> messageThread;
           #endif

            DRX_AUTORELEASEPOOL
            {
                delete static_cast<LV2UIInstance*> (ui);
            }
        },
        [] (LV2UI_Handle ui, u32 portIndex, u32 bufferSize, u32 format, ukk buffer)
        {
            DRX_ASSERT_MESSAGE_THREAD
            static_cast<LV2UIInstance*> (ui)->portEvent (portIndex, bufferSize, format, buffer);
        },
        [] (tukk uri) -> ukk
        {
            const auto uriMatches = [&] (const LV2_Feature& f) { return std::strcmp (f.URI, uri) == 0; };

            static LV2UI_Resize resize { nullptr, [] (LV2UI_Feature_Handle handle, i32 width, i32 height) -> i32
            {
                DRX_ASSERT_MESSAGE_THREAD
                return static_cast<LV2UIInstance*> (handle)->resize (width, height);
            } };

            static LV2UI_Idle_Interface idle { [] (LV2UI_Handle handle)
            {
                static_cast<LV2UIInstance*> (handle)->idleCallback();
                return 0;
            } };

            static LV2_Options_Interface options
            {
                [] (LV2_Handle handle, LV2_Options_Option* optionsIn)
                {
                    return static_cast<LV2UIInstance*> (handle)->getOptions (optionsIn);
                },
                [] (LV2_Handle handle, const LV2_Options_Option* optionsIn)
                {
                    return static_cast<LV2UIInstance*> (handle)->setOptions (optionsIn);
                }
            };

            // We'll always define noUserResize and idle in the extension data array, but we'll
            // only declare them in the ui.ttl if the UI is actually non-resizable, or requires
            // idle callbacks.
            // Well-behaved hosts should check the ttl before trying to search the
            // extension-data array.
            static const LV2_Feature features[] { { LV2_UI__resize, &resize },
                                                  { LV2_UI__noUserResize, nullptr },
                                                  { LV2_UI__idleInterface, &idle },
                                                  { LV2_OPTIONS__interface, &options } };

            const auto it = std::find_if (std::begin (features), std::end (features), uriMatches);
            return it != std::end (features) ? it->data : nullptr;
        }
    };

    return &descriptor;
}

} // namespace drx::lv2_client

#endif
