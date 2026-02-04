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

#include <drx_core/system/drx_CompilerWarnings.h>
#include <drx_core/system/drx_TargetPlatform.h>
#include <drx_audio_plugin_client/detail/drx_CheckSettingMacros.h>

#if DrxPlugin_Build_VST

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4996 4100)

#include <drx_audio_plugin_client/detail/drx_IncludeSystemHeaders.h>
#include <drx_core/drx_core.h>

#if DrxPlugin_VersionCode < 0x010000   // Major < 0

 #if (DrxPlugin_VersionCode & 0x00FF00) > (9 * 0x100) // check if Minor number exceeds 9
  DRX_COMPILER_WARNING ("When version has 'major' = 0, VST2 has trouble displaying 'minor' exceeding 9")
 #endif

 #if (DrxPlugin_VersionCode & 0xFF) > 9   // check if Bugfix number exceeds 9
  DRX_COMPILER_WARNING ("When version has 'major' = 0, VST2 has trouble displaying 'bugfix' exceeding 9")
 #endif

#elif DrxPlugin_VersionCode >= 0x650000   // Major >= 101

 #if (DrxPlugin_VersionCode & 0x00FF00) > (99 * 0x100) // check if Minor number exceeds 99
  DRX_COMPILER_WARNING ("When version has 'major' > 100, VST2 has trouble displaying 'minor' exceeding 99")
 #endif

 #if (DrxPlugin_VersionCode & 0xFF) > 99  // check if Bugfix number exceeds 99
  DRX_COMPILER_WARNING ("When version has 'major' > 100, VST2 has trouble displaying 'bugfix' exceeding 99")
 #endif

#endif

#ifdef PRAGMA_ALIGN_SUPPORTED
 #undef PRAGMA_ALIGN_SUPPORTED
 #define PRAGMA_ALIGN_SUPPORTED 1
#endif

#if ! DRX_MSVC && ! defined (__cdecl)
 #define __cdecl
#endif

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wconversion",
                                     "-Wshadow",
                                     "-Wdeprecated-register",
                                     "-Wdeprecated-declarations",
                                     "-Wunused-parameter",
                                     "-Wdeprecated-writable-strings",
                                     "-Wnon-virtual-dtor",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wlanguage-extension-token")
DRX_BEGIN_IGNORE_WARNINGS_MSVC (4458)

#define VST_FORCE_DEPRECATED 0

namespace Vst2
{
// If the following files cannot be found then you are probably trying to build
// a VST2 plug-in or a VST2-compatible VST3 plug-in. To do this you must have a
// VST2 SDK in your header search paths or use the "VST (Legacy) SDK Folder"
// field in the Projucer. The VST2 SDK can be obtained from the
// vstsdk3610_11_06_2018_build_37 (or older) VST3 SDK or DRX version 5.3.2. You
// also need a VST2 license from Steinberg to distribute VST2 plug-ins.
#include "pluginterfaces/vst2.x/aeffect.h"
#include "pluginterfaces/vst2.x/aeffectx.h"
}

DRX_END_IGNORE_WARNINGS_MSVC
DRX_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
#if DRX_MSVC
 #pragma pack (push, 8)
#endif

#define DRX_VSTINTERFACE_H_INCLUDED 1
#define DRX_GUI_BASICS_INCLUDE_XHEADERS 1

#include <drx_audio_plugin_client/detail/drx_PluginUtilities.h>

using namespace drx;

#include <drx_gui_basics/native/drx_WindowsHooks_windows.h>
#include <drx_audio_plugin_client/detail/drx_LinuxMessageThread.h>
#include <drx_audio_plugin_client/detail/drx_VSTWindowUtilities.h>

#include <drx_audio_processors/format_types/drx_LegacyAudioParameter.cpp>
#include <drx_audio_processors/format_types/drx_VSTCommon.h>

#ifdef DRX_MSVC
 #pragma pack (pop)
#endif

#undef MemoryBlock

class DrxVSTWrapper;
static b8 recursionCheck = false;

namespace drx
{
 #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
  DRX_API f64 getScaleFactorForWindow (HWND);
 #endif
}

//==============================================================================
#if DRX_WINDOWS

namespace
{
    // Returns the actual container window, unlike GetParent, which can also return a separate owner window.
    static HWND getWindowParent (HWND w) noexcept    { return GetAncestor (w, GA_PARENT); }

    static HWND findMDIParentOf (HWND w)
    {
        i32k frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

        while (w != nullptr)
        {
            auto parent = getWindowParent (w);

            if (parent == nullptr)
                break;

            TCHAR windowType[32] = { 0 };
            GetClassName (parent, windowType, 31);

            if (Txt (windowType).equalsIgnoreCase ("MDIClient"))
                return parent;

            RECT windowPos, parentPos;
            GetWindowRect (w, &windowPos);
            GetWindowRect (parent, &parentPos);

            auto dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
            auto dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

            if (dw > 100 || dh > 100)
                break;

            w = parent;

            if (dw == 2 * frameThickness)
                break;
        }

        return w;
    }

    static i32 numActivePlugins = 0;
    static b8 messageThreadIsDefinitelyCorrect = false;
}

#endif

//==============================================================================
// Ableton Live host specific commands
struct AbletonLiveHostSpecific
{
    enum
    {
        KCantBeSuspended = (1 << 2)
    };

    u32 magic;        // 'AbLi'
    i32 cmd;             // 5 = realtime properties
    size_t commandSize;  // sizeof (i32)
    i32 flags;           // KCantBeSuspended = (1 << 2)
};

//==============================================================================
/**
    This is an AudioEffectX object that holds and wraps our AudioProcessor...
*/
class DrxVSTWrapper final : public AudioProcessorListener,
                             public AudioPlayHead,
                             private AudioProcessorParameter::Listener
{
private:
    //==============================================================================
    template <typename FloatType>
    struct VstTempBuffers
    {
        VstTempBuffers() {}
        ~VstTempBuffers() { release(); }

        z0 release() noexcept
        {
            for (auto* c : tempChannels)
                delete[] c;

            tempChannels.clear();
        }

        HeapBlock<FloatType*> channels;
        Array<FloatType*> tempChannels;  // see note in processReplacing()
        drx::AudioBuffer<FloatType> processTempBuffer;
    };

    /** Use the same names as the VST SDK. */
    struct VstOpCodeArguments
    {
        i32 index;
        pointer_sized_int value;
        uk ptr;
        f32 opt;
    };

public:
    //==============================================================================
    DrxVSTWrapper (Vst2::audioMasterCallback cb, std::unique_ptr<AudioProcessor> af)
       : hostCallback (cb),
         processor (std::move (af))
    {
        inParameterChangedCallback = false;

        // VST-2 does not support disabling buses: so always enable all of them
        processor->enableAllBuses();

        findMaxTotalChannels (maxNumInChannels, maxNumOutChannels);

        // You must at least have some channels
        jassert (processor->isMidiEffect() || (maxNumInChannels > 0 || maxNumOutChannels > 0));

        if (processor->isMidiEffect())
            maxNumInChannels = maxNumOutChannels = 2;

       #ifdef DrxPlugin_PreferredChannelConfigurations
        processor->setPlayConfigDetails (maxNumInChannels, maxNumOutChannels, 44100.0, 1024);
       #endif

        processor->setRateAndBufferSizeDetails (0, 0);
        processor->setPlayHead (this);
        processor->addListener (this);

        if (auto* juceParam = processor->getBypassParameter())
            juceParam->addListener (this);

        juceParameters.update (*processor, false);

        memset (&vstEffect, 0, sizeof (vstEffect));
        vstEffect.magic = 0x56737450 /* 'VstP' */;
        vstEffect.dispatcher = [] (Vst2::AEffect* vstInterface,
                                   Vst2::VstInt32 opCode,
                                   Vst2::VstInt32 index,
                                   Vst2::VstIntPtr value,
                                   uk ptr,
                                   f32 opt) -> Vst2::VstIntPtr
        {
            auto* wrapper = getWrapper (vstInterface);
            VstOpCodeArguments args = { index, value, ptr, opt };

            if (opCode == Vst2::effClose)
            {
                wrapper->dispatcher (opCode, args);
                delete wrapper;
                return 1;
            }

            return wrapper->dispatcher (opCode, args);
        };

        vstEffect.process = nullptr;

        vstEffect.setParameter = [] (Vst2::AEffect* vstInterface, Vst2::VstInt32 index, f32 value)
        {
            getWrapper (vstInterface)->setParameter (index, value);
        };

        vstEffect.getParameter = [] (Vst2::AEffect* vstInterface, Vst2::VstInt32 index) -> f32
        {
            return getWrapper (vstInterface)->getParameter (index);
        };

        vstEffect.numPrograms = jmax (1, processor->getNumPrograms());
        vstEffect.numParams = juceParameters.getNumParameters();
        vstEffect.numInputs = maxNumInChannels;
        vstEffect.numOutputs = maxNumOutChannels;
        vstEffect.initialDelay = processor->getLatencySamples();
        vstEffect.object = this;
        vstEffect.uniqueID = DrxPlugin_VSTUniqueID;

       #ifdef DrxPlugin_VSTChunkStructureVersion
        vstEffect.version = DrxPlugin_VSTChunkStructureVersion;
       #else
        vstEffect.version = DrxPlugin_VersionCode;
       #endif

        vstEffect.processReplacing = [] (Vst2::AEffect* vstInterface,
                                         f32** inputs,
                                         f32** outputs,
                                         Vst2::VstInt32 sampleFrames)
        {
            getWrapper (vstInterface)->processReplacing (inputs, outputs, sampleFrames);
        };

        vstEffect.processDoubleReplacing = [] (Vst2::AEffect* vstInterface,
                                               f64** inputs,
                                               f64** outputs,
                                               Vst2::VstInt32 sampleFrames)
        {
            getWrapper (vstInterface)->processDoubleReplacing (inputs, outputs, sampleFrames);
        };

        vstEffect.flags |= Vst2::effFlagsHasEditor;

        vstEffect.flags |= Vst2::effFlagsCanReplacing;
        if (processor->supportsDoublePrecisionProcessing())
            vstEffect.flags |= Vst2::effFlagsCanDoubleReplacing;

        vstEffect.flags |= Vst2::effFlagsProgramChunks;

       #if DrxPlugin_IsSynth
        vstEffect.flags |= Vst2::effFlagsIsSynth;
       #else
        if (processor->getTailLengthSeconds() == 0.0)
            vstEffect.flags |= Vst2::effFlagsNoSoundInStop;
       #endif

       #if DRX_WINDOWS
        ++numActivePlugins;
       #endif
    }

    ~DrxVSTWrapper() override
    {
        DRX_AUTORELEASEPOOL
        {
           #if DRX_LINUX || DRX_BSD
            MessageManagerLock mmLock;
           #endif

            timedCallback.stopTimer();
            deleteEditor (false);

            hasShutdown = true;

            processor = nullptr;

            jassert (editorComp == nullptr);

            deleteTempChannels();

           #if DRX_WINDOWS
            if (--numActivePlugins == 0)
                messageThreadIsDefinitelyCorrect = false;
           #endif
        }
    }

    Vst2::AEffect* getAEffect() noexcept    { return &vstEffect; }

    template <typename FloatType>
    z0 internalProcessReplacing (FloatType** inputs, FloatType** outputs,
                                   i32 numSamples, VstTempBuffers<FloatType>& tmpBuffers)
    {
        const b8 isMidiEffect = processor->isMidiEffect();

        if (firstProcessCallback)
        {
            firstProcessCallback = false;

            // if this fails, the host hasn't called resume() before processing
            jassert (isProcessing);

            // (tragically, some hosts actually need this, although it's stupid to have
            //  to do it here.)
            if (! isProcessing)
                resume();

            processor->setNonRealtime (isProcessLevelOffline());

           #if DRX_WINDOWS
            if (detail::PluginUtilities::getHostType().isWavelab())
            {
                i32 priority = GetThreadPriority (GetCurrentThread());

                if (priority <= THREAD_PRIORITY_NORMAL && priority >= THREAD_PRIORITY_LOWEST)
                    processor->setNonRealtime (true);
            }
           #endif
        }

        const auto numMidiEventsComingIn = midiEvents.getNumEvents();

        {
            i32k numIn  = processor->getTotalNumInputChannels();
            i32k numOut = processor->getTotalNumOutputChannels();

            const ScopedLock sl (processor->getCallbackLock());

            if (processor->isSuspended())
            {
                for (i32 i = 0; i < numOut; ++i)
                    if (outputs[i] != nullptr)
                        FloatVectorOperations::clear (outputs[i], numSamples);
            }
            else
            {
                updateCallbackContextInfo();

                i32 i;
                for (i = 0; i < numOut; ++i)
                {
                    auto* chan = tmpBuffers.tempChannels.getUnchecked (i);

                    if (chan == nullptr)
                    {
                        chan = outputs[i];

                        b8 bufferPointerReusedForOtherChannels = false;

                        for (i32 j = i; --j >= 0;)
                        {
                            if (outputs[j] == chan)
                            {
                                bufferPointerReusedForOtherChannels = true;
                                break;
                            }
                        }

                        // if some output channels are disabled, some hosts supply the same buffer
                        // for multiple channels or supply a nullptr - this buggers up our method
                        // of copying the inputs over the outputs, so we need to create unique temp
                        // buffers in this case..
                        if (bufferPointerReusedForOtherChannels || chan == nullptr)
                        {
                            chan = new FloatType [(size_t) blockSize * 2];
                            tmpBuffers.tempChannels.set (i, chan);
                        }
                    }

                    if (i < numIn)
                    {
                        if (chan != inputs[i])
                            memcpy (chan, inputs[i], (size_t) numSamples * sizeof (FloatType));
                    }
                    else
                    {
                        FloatVectorOperations::clear (chan, numSamples);
                    }

                    tmpBuffers.channels[i] = chan;
                }

                for (; i < numIn; ++i)
                    tmpBuffers.channels[i] = inputs[i];

                {
                    i32k numChannels = jmax (numIn, numOut);
                    AudioBuffer<FloatType> chans (tmpBuffers.channels, isMidiEffect ? 0 : numChannels, numSamples);

                    if (isBypassed && processor->getBypassParameter() == nullptr)
                        processor->processBlockBypassed (chans, midiEvents);
                    else
                        processor->processBlock (chans, midiEvents);
                }

                // copy back any temp channels that may have been used..
                for (i = 0; i < numOut; ++i)
                    if (auto* chan = tmpBuffers.tempChannels.getUnchecked (i))
                        if (auto* dest = outputs[i])
                            memcpy (dest, chan, (size_t) numSamples * sizeof (FloatType));
            }
        }

        if (! midiEvents.isEmpty())
        {
            if (supportsMidiOut)
            {
                auto numEvents = midiEvents.getNumEvents();

                outgoingEvents.ensureSize (numEvents);
                outgoingEvents.clear();

                for (const auto metadata : midiEvents)
                {
                    jassert (metadata.samplePosition >= 0 && metadata.samplePosition < numSamples);

                    outgoingEvents.addEvent (metadata.data, metadata.numBytes, metadata.samplePosition);
                }

                // Send VST events to the host.
                NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterProcessEvents, 0, 0, outgoingEvents.events, 0.0f);
            }
            else
            {
                /*  This assertion is caused when you've added some events to the
                    midiMessages array in your processBlock() method, which usually means
                    that you're trying to send them somewhere. But in this case they're
                    getting thrown away.

                    If your plugin does want to send midi messages, you'll need to set
                    the DrxPlugin_ProducesMidiOutput macro to 1 in your
                    DrxPluginCharacteristics.h file.

                    If you don't want to produce any midi output, then you should clear the
                    midiMessages array at the end of your processBlock() method, to
                    indicate that you don't want any of the events to be passed through
                    to the output.
                */
                jassertquiet (midiEvents.getNumEvents() <= numMidiEventsComingIn);
            }

            midiEvents.clear();
        }
    }

    z0 processReplacing (f32** inputs, f32** outputs, i32 sampleFrames)
    {
        jassert (! processor->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, floatTempBuffers);
    }

    z0 processDoubleReplacing (f64** inputs, f64** outputs, i32 sampleFrames)
    {
        jassert (processor->isUsingDoublePrecision());
        internalProcessReplacing (inputs, outputs, sampleFrames, doubleTempBuffers);
    }

    //==============================================================================
    z0 resume()
    {
        if (processor != nullptr)
        {
            isProcessing = true;

            auto numInAndOutChannels = static_cast<size_t> (vstEffect.numInputs + vstEffect.numOutputs);
            floatTempBuffers .channels.calloc (numInAndOutChannels);
            doubleTempBuffers.channels.calloc (numInAndOutChannels);

            auto currentRate = sampleRate;
            auto currentBlockSize = blockSize;

            firstProcessCallback = true;

            processor->setNonRealtime (isProcessLevelOffline());
            processor->setRateAndBufferSizeDetails (currentRate, currentBlockSize);

            deleteTempChannels();

            processor->prepareToPlay (currentRate, currentBlockSize);

            midiEvents.ensureSize (2048);
            midiEvents.clear();

            vstEffect.initialDelay = processor->getLatencySamples();

            /** If this plug-in is a synth or it can receive midi events we need to tell the
                host that we want midi. In the SDK this method is marked as deprecated, but
                some hosts rely on this behaviour.
            */
            if (vstEffect.flags & Vst2::effFlagsIsSynth || supportsMidiIn)
                NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterWantMidi, 0, 1, nullptr, 0.0f);

            if (detail::PluginUtilities::getHostType().isAbletonLive()
                 && hostCallback != nullptr
                 && std::isinf (processor->getTailLengthSeconds()))
            {
                AbletonLiveHostSpecific hostCmd;

                hostCmd.magic = 0x41624c69; // 'AbLi'
                hostCmd.cmd = 5;
                hostCmd.commandSize = sizeof (i32);
                hostCmd.flags = AbletonLiveHostSpecific::KCantBeSuspended;

                hostCallback (&vstEffect, Vst2::audioMasterVendorSpecific, 0, 0, &hostCmd, 0.0f);
            }

            if (supportsMidiOut)
                outgoingEvents.ensureSize (512);
        }
    }

    z0 suspend()
    {
        if (processor != nullptr)
        {
            processor->releaseResources();
            outgoingEvents.freeEvents();

            isProcessing = false;
            floatTempBuffers.channels.free();
            doubleTempBuffers.channels.free();

            deleteTempChannels();
        }
    }

    z0 updateCallbackContextInfo()
    {
        const Vst2::VstTimeInfo* ti = nullptr;

        if (hostCallback != nullptr)
        {
            i32 flags = Vst2::kVstPpqPosValid  | Vst2::kVstTempoValid
                          | Vst2::kVstBarsValid    | Vst2::kVstCyclePosValid
                          | Vst2::kVstTimeSigValid | Vst2::kVstSmpteValid
                          | Vst2::kVstClockValid   | Vst2::kVstNanosValid;

            auto result = hostCallback (&vstEffect, Vst2::audioMasterGetTime, 0, flags, nullptr, 0);
            ti = reinterpret_cast<Vst2::VstTimeInfo*> (result);
        }

        if (ti == nullptr || ti->sampleRate <= 0)
        {
            currentPosition.reset();
            return;
        }

        auto& info = currentPosition.emplace();
        info.setBpm ((ti->flags & Vst2::kVstTempoValid) != 0 ? makeOptional (ti->tempo) : nullopt);

        info.setTimeSignature ((ti->flags & Vst2::kVstTimeSigValid) != 0 ? makeOptional (TimeSignature { ti->timeSigNumerator, ti->timeSigDenominator })
                                                                         : nullopt);

        info.setTimeInSamples ((z64) (ti->samplePos + 0.5));
        info.setTimeInSeconds (ti->samplePos / ti->sampleRate);
        info.setPpqPosition ((ti->flags & Vst2::kVstPpqPosValid) != 0 ? makeOptional (ti->ppqPos) : nullopt);
        info.setPpqPositionOfLastBarStart ((ti->flags & Vst2::kVstBarsValid) != 0 ? makeOptional (ti->barStartPos) : nullopt);

        if ((ti->flags & Vst2::kVstSmpteValid) != 0)
        {
            info.setFrameRate ([&]() -> Optional<FrameRate>
            {
                switch (ti->smpteFrameRate)
                {
                    case Vst2::kVstSmpte24fps:          return FrameRate().withBaseRate (24);
                    case Vst2::kVstSmpte239fps:         return FrameRate().withBaseRate (24).withPullDown();

                    case Vst2::kVstSmpte25fps:          return FrameRate().withBaseRate (25);
                    case Vst2::kVstSmpte249fps:         return FrameRate().withBaseRate (25).withPullDown();

                    case Vst2::kVstSmpte30fps:          return FrameRate().withBaseRate (30);
                    case Vst2::kVstSmpte30dfps:         return FrameRate().withBaseRate (30).withDrop();
                    case Vst2::kVstSmpte2997fps:        return FrameRate().withBaseRate (30).withPullDown();
                    case Vst2::kVstSmpte2997dfps:       return FrameRate().withBaseRate (30).withPullDown().withDrop();

                    case Vst2::kVstSmpte60fps:          return FrameRate().withBaseRate (60);
                    case Vst2::kVstSmpte599fps:         return FrameRate().withBaseRate (60).withPullDown();

                    case Vst2::kVstSmpteFilm16mm:
                    case Vst2::kVstSmpteFilm35mm:       return FrameRate().withBaseRate (24);
                }

                return nullopt;
            }());

            const auto effectiveRate = info.getFrameRate().hasValue() ? info.getFrameRate()->getEffectiveRate() : 0.0;
            info.setEditOriginTime (! approximatelyEqual (effectiveRate, 0.0) ? makeOptional (ti->smpteOffset / (80.0 * effectiveRate)) : nullopt);
        }

        info.setIsRecording ((ti->flags & Vst2::kVstTransportRecording) != 0);
        info.setIsPlaying   ((ti->flags & (Vst2::kVstTransportRecording | Vst2::kVstTransportPlaying)) != 0);
        info.setIsLooping   ((ti->flags & Vst2::kVstTransportCycleActive) != 0);

        info.setLoopPoints ((ti->flags & Vst2::kVstCyclePosValid) != 0 ? makeOptional (LoopPoints { ti->cycleStartPos, ti->cycleEndPos })
                                                                       : nullopt);

        info.setHostTimeNs ((ti->flags & Vst2::kVstNanosValid) != 0 ? makeOptional ((zu64) ti->nanoSeconds) : nullopt);
    }

    //==============================================================================
    Optional<PositionInfo> getPosition() const override
    {
        return currentPosition;
    }

    //==============================================================================
    f32 getParameter (i32 index) const
    {
        if (auto* param = juceParameters.getParamForIndex (index))
            return param->getValue();

        return 0.0f;
    }

    z0 setParameter (i32 index, f32 value)
    {
        if (auto* param = juceParameters.getParamForIndex (index))
            setValueAndNotifyIfChanged (*param, value);
    }

    z0 audioProcessorParameterChanged (AudioProcessor*, i32 index, f32 newValue) override
    {
        if (inParameterChangedCallback.get())
        {
            inParameterChangedCallback = false;
            return;
        }

        NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterAutomate, index, 0, nullptr, newValue);
    }

    z0 audioProcessorParameterChangeGestureBegin (AudioProcessor*, i32 index) override
    {
        NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterBeginEdit, index, 0, nullptr, 0.0f);
    }

    z0 audioProcessorParameterChangeGestureEnd (AudioProcessor*, i32 index) override
    {
        NullCheckedInvocation::invoke (hostCallback, &vstEffect, Vst2::audioMasterEndEdit, index, 0, nullptr, 0.0f);
    }

    z0 parameterValueChanged (i32, f32 newValue) override
    {
        // this can only come from the bypass parameter
        isBypassed = (newValue >= 0.5f);
    }

    z0 parameterGestureChanged (i32, b8) override {}

    z0 audioProcessorChanged (AudioProcessor*, const ChangeDetails& details) override
    {
        hostChangeUpdater.update (details);
    }

    b8 getPinProperties (Vst2::VstPinProperties& properties, b8 direction, i32 index) const
    {
        if (processor->isMidiEffect())
            return false;

        i32 channelIdx, busIdx;

        // fill with default
        properties.flags = 0;
        properties.label[0] = 0;
        properties.shortLabel[0] = 0;
        properties.arrangementType = Vst2::kSpeakerArrEmpty;

        if ((channelIdx = processor->getOffsetInBusBufferForAbsoluteChannelIndex (direction, index, busIdx)) >= 0)
        {
            auto& bus = *processor->getBus (direction, busIdx);
            auto& channelSet = bus.getCurrentLayout();
            auto channelType = channelSet.getTypeOfChannel (channelIdx);

            properties.flags = Vst2::kVstPinIsActive | Vst2::kVstPinUseSpeaker;
            properties.arrangementType = SpeakerMappings::channelSetToVstArrangementType (channelSet);
            Txt label = bus.getName();

           #ifdef DrxPlugin_PreferredChannelConfigurations
            label += " " + Txt (channelIdx);
           #else
            if (channelSet.size() > 1)
                label += " " + AudioChannelSet::getAbbreviatedChannelTypeName (channelType);
           #endif

            label.copyToUTF8 (properties.label, (size_t) (Vst2::kVstMaxLabelLen + 1));
            label.copyToUTF8 (properties.shortLabel, (size_t) (Vst2::kVstMaxShortLabelLen + 1));

            if (channelType == AudioChannelSet::left
                || channelType == AudioChannelSet::leftSurround
                || channelType == AudioChannelSet::leftCentre
                || channelType == AudioChannelSet::leftSurroundSide
                || channelType == AudioChannelSet::topFrontLeft
                || channelType == AudioChannelSet::topRearLeft
                || channelType == AudioChannelSet::leftSurroundRear
                || channelType == AudioChannelSet::wideLeft)
                properties.flags |= Vst2::kVstPinIsStereo;

            return true;
        }

        return false;
    }

    //==============================================================================
    z0 setHasEditorFlag (b8 shouldSetHasEditor)
    {
        auto hasEditor = (vstEffect.flags & Vst2::effFlagsHasEditor) != 0;

        if (shouldSetHasEditor == hasEditor)
            return;

        if (shouldSetHasEditor)
            vstEffect.flags |= Vst2::effFlagsHasEditor;
        else
            vstEffect.flags &= ~Vst2::effFlagsHasEditor;
    }

    z0 createEditorComp()
    {
        if (hasShutdown || processor == nullptr)
            return;

        if (editorComp == nullptr)
        {
            if (auto* ed = processor->createEditorIfNeeded())
            {
                setHasEditorFlag (true);
                editorComp.reset (new EditorCompWrapper (*this, *ed, editorScaleFactor));
            }
            else
            {
                setHasEditorFlag (false);
            }
        }

        shouldDeleteEditor = false;
    }

    z0 deleteEditor (b8 canDeleteLaterIfModal)
    {
        DRX_AUTORELEASEPOOL
        {
            PopupMenu::dismissAllActiveMenus();

            jassert (! recursionCheck);
            ScopedValueSetter<b8> svs (recursionCheck, true, false);

            if (editorComp != nullptr)
            {
                if (auto* modalComponent = Component::getCurrentlyModalComponent())
                {
                    modalComponent->exitModalState (0);

                    if (canDeleteLaterIfModal)
                    {
                        shouldDeleteEditor = true;
                        return;
                    }
                }

                editorComp->detachHostWindow();

                if (auto* ed = editorComp->getEditorComp())
                    processor->editorBeingDeleted (ed);

                editorComp = nullptr;

                // there's some kind of component currently modal, but the host
                // is trying to delete our plugin. You should try to avoid this happening..
                jassert (Component::getCurrentlyModalComponent() == nullptr);
            }
        }
    }

    pointer_sized_int dispatcher (i32 opCode, VstOpCodeArguments args)
    {
        if (hasShutdown)
            return 0;

        switch (opCode)
        {
            case Vst2::effOpen:                     return handleOpen (args);
            case Vst2::effClose:                    return handleClose (args);
            case Vst2::effSetProgram:               return handleSetCurrentProgram (args);
            case Vst2::effGetProgram:               return handleGetCurrentProgram (args);
            case Vst2::effSetProgramName:           return handleSetCurrentProgramName (args);
            case Vst2::effGetProgramName:           return handleGetCurrentProgramName (args);
            case Vst2::effGetParamLabel:            return handleGetParameterLabel (args);
            case Vst2::effGetParamDisplay:          return handleGetParameterText (args);
            case Vst2::effGetParamName:             return handleGetParameterName (args);
            case Vst2::effSetSampleRate:            return handleSetSampleRate (args);
            case Vst2::effSetBlockSize:             return handleSetBlockSize (args);
            case Vst2::effMainsChanged:             return handleResumeSuspend (args);
            case Vst2::effEditGetRect:              return handleGetEditorBounds (args);
            case Vst2::effEditOpen:                 return handleOpenEditor (args);
            case Vst2::effEditClose:                return handleCloseEditor (args);
            case Vst2::effIdentify:                 return (pointer_sized_int) ByteOrder::bigEndianInt ("NvEf");
            case Vst2::effGetChunk:                 return handleGetData (args);
            case Vst2::effSetChunk:                 return handleSetData (args);
            case Vst2::effProcessEvents:            return handlePreAudioProcessingEvents (args);
            case Vst2::effCanBeAutomated:           return handleIsParameterAutomatable (args);
            case Vst2::effString2Parameter:         return handleParameterValueForText (args);
            case Vst2::effGetProgramNameIndexed:    return handleGetProgramName (args);
            case Vst2::effGetInputProperties:       return handleGetInputPinProperties (args);
            case Vst2::effGetOutputProperties:      return handleGetOutputPinProperties (args);
            case Vst2::effGetPlugCategory:          return handleGetPlugInCategory (args);
            case Vst2::effSetSpeakerArrangement:    return handleSetSpeakerConfiguration (args);
            case Vst2::effSetBypass:                return handleSetBypass (args);
            case Vst2::effGetEffectName:            return handleGetPlugInName (args);
            case Vst2::effGetProductString:         return handleGetPlugInName (args);
            case Vst2::effGetVendorString:          return handleGetManufacturerName (args);
            case Vst2::effGetVendorVersion:         return handleGetManufacturerVersion (args);
            case Vst2::effVendorSpecific:           return handleManufacturerSpecific (args);
            case Vst2::effCanDo:                    return handleCanPlugInDo (args);
            case Vst2::effGetTailSize:              return handleGetTailSize (args);
            case Vst2::effKeysRequired:             return handleKeyboardFocusRequired (args);
            case Vst2::effGetVstVersion:            return handleGetVstInterfaceVersion (args);
            case Vst2::effGetCurrentMidiProgram:    return handleGetCurrentMidiProgram (args);
            case Vst2::effGetSpeakerArrangement:    return handleGetSpeakerConfiguration (args);
            case Vst2::effSetTotalSampleToProcess:  return handleSetNumberOfSamplesToProcess (args);
            case Vst2::effSetProcessPrecision:      return handleSetSampleFloatType (args);
            case Vst2::effGetNumMidiInputChannels:  return handleGetNumMidiInputChannels();
            case Vst2::effGetNumMidiOutputChannels: return handleGetNumMidiOutputChannels();
            case Vst2::effGetMidiKeyName:           return handleGetMidiKeyName (args);
            case Vst2::effEditIdle:                 return handleEditIdle();
            default:                                return 0;
        }
    }

    //==============================================================================
    // A component to hold the AudioProcessorEditor, and cope with some housekeeping
    // chores when it changes or repaints.
    struct EditorCompWrapper final : public Component
                             #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
                              , public Timer
                             #endif
    {
        EditorCompWrapper (DrxVSTWrapper& w, AudioProcessorEditor& editor, [[maybe_unused]] f32 initialScale)
            : wrapper (w)
        {
            editor.setOpaque (true);
           #if ! DRX_MAC
            editor.setScaleFactor (initialScale);
           #endif
            addAndMakeVisible (editor);

            auto editorBounds = getSizeToContainChild();
            setSize (editorBounds.getWidth(), editorBounds.getHeight());

           #if DRX_WINDOWS
            if (! detail::PluginUtilities::getHostType().isReceptor())
                addMouseListener (this, true);
           #endif

            setOpaque (true);
        }

        ~EditorCompWrapper() override
        {
            deleteAllChildren(); // note that we can't use a std::unique_ptr because the editor may
                                 // have been transferred to another parent which takes over ownership.
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (Colors::black);
        }

        z0 getEditorBounds (Vst2::ERect& bounds)
        {
            auto editorBounds = getSizeToContainChild();
            bounds = convertToHostBounds ({ 0, 0, (i16) editorBounds.getHeight(), (i16) editorBounds.getWidth() });
        }

        z0 attachToHost (VstOpCodeArguments args)
        {
            setVisible (false);

            const auto desktopFlags = detail::PluginUtilities::getDesktopFlags (getEditorComp());

           #if DRX_WINDOWS || DRX_LINUX || DRX_BSD
            addToDesktop (desktopFlags, args.ptr);
            hostWindow = (HostWindowType) args.ptr;

            #if DRX_LINUX || DRX_BSD
             X11Symbols::getInstance()->xReparentWindow (display,
                                                         (Window) getWindowHandle(),
                                                         (HostWindowType) hostWindow,
                                                         0, 0);
             // The host is likely to attempt to move/resize the window directly after this call,
             // and we need to ensure that the X server knows that our window has been attached
             // before that happens.
             X11Symbols::getInstance()->xFlush (display);
            #elif DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
             checkHostWindowScaleFactor (true);
             startTimer (500);
            #endif
           #elif DRX_MAC
            hostWindow = detail::VSTWindowUtilities::attachComponentToWindowRefVST (this, desktopFlags, args.ptr);
           #endif

            setVisible (true);
        }

        z0 detachHostWindow()
        {
           #if DRX_MAC
            if (hostWindow != nullptr)
                detail::VSTWindowUtilities::detachComponentFromWindowRefVST (this, hostWindow);
           #endif

            hostWindow = {};
        }

        AudioProcessorEditor* getEditorComp() const noexcept
        {
            return dynamic_cast<AudioProcessorEditor*> (getChildComponent (0));
        }

        z0 resized() override
        {
            if (auto* pluginEditor = getEditorComp())
            {
                if (! resizingParent)
                {
                    auto newBounds = getLocalBounds();

                    {
                        const ScopedValueSetter<b8> resizingChildSetter (resizingChild, true);
                        pluginEditor->setBounds (pluginEditor->getLocalArea (this, newBounds).withPosition (0, 0));
                    }

                    lastBounds = newBounds;
                }

                updateWindowSize();
            }
        }

        z0 parentSizeChanged() override
        {
            updateWindowSize();
            repaint();
        }

        z0 childBoundsChanged (Component*) override
        {
            if (resizingChild)
                return;

            auto newBounds = getSizeToContainChild();

            if (newBounds != lastBounds)
            {
                updateWindowSize();
                lastBounds = newBounds;
            }
        }

        drx::Rectangle<i32> getSizeToContainChild()
        {
            if (auto* pluginEditor = getEditorComp())
                return getLocalArea (pluginEditor, pluginEditor->getLocalBounds());

            return {};
        }

        z0 resizeHostWindow (drx::Rectangle<i32> bounds)
        {
            auto rect = convertToHostBounds ({ 0, 0, (i16) bounds.getHeight(), (i16) bounds.getWidth() });
            const auto newWidth = rect.right - rect.left;
            const auto newHeight = rect.bottom - rect.top;

            b8 sizeWasSuccessful = false;

            if (auto host = wrapper.hostCallback)
            {
                auto status = host (wrapper.getAEffect(), Vst2::audioMasterCanDo, 0, 0, const_cast<tuk> ("sizeWindow"), 0);

                if (status == (pointer_sized_int) 1 || detail::PluginUtilities::getHostType().isAbletonLive())
                {
                    const ScopedValueSetter<b8> resizingParentSetter (resizingParent, true);

                    sizeWasSuccessful = (host (wrapper.getAEffect(), Vst2::audioMasterSizeWindow,
                                               newWidth, newHeight, nullptr, 0) != 0);
                }
            }

            // some hosts don't support the sizeWindow call, so do it manually..
            if (! sizeWasSuccessful)
            {
                const ScopedValueSetter<b8> resizingParentSetter (resizingParent, true);

               #if DRX_MAC
                detail::VSTWindowUtilities::setNativeHostWindowSizeVST (hostWindow, this, newWidth, newHeight);
               #elif DRX_LINUX || DRX_BSD
                // (Currently, all linux hosts support sizeWindow, so this should never need to happen)
                setSize (newWidth, newHeight);
               #else
                i32 dw = 0;
                i32 dh = 0;
                i32k frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

                HWND w = (HWND) getWindowHandle();

                while (w != nullptr)
                {
                    HWND parent = getWindowParent (w);

                    if (parent == nullptr)
                        break;

                    TCHAR windowType [32] = { 0 };
                    GetClassName (parent, windowType, 31);

                    if (Txt (windowType).equalsIgnoreCase ("MDIClient"))
                        break;

                    RECT windowPos, parentPos;
                    GetWindowRect (w, &windowPos);
                    GetWindowRect (parent, &parentPos);

                    if (w != (HWND) getWindowHandle())
                        SetWindowPos (w, nullptr, 0, 0, newWidth + dw, newHeight + dh,
                                      SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);

                    dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
                    dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

                    w = parent;

                    if (dw == 2 * frameThickness)
                        break;

                    if (dw > 100 || dh > 100)
                        w = nullptr;
                }

                if (w != nullptr)
                    SetWindowPos (w, nullptr, 0, 0, newWidth + dw, newHeight + dh,
                                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
               #endif
            }

           #if DRX_LINUX || DRX_BSD
            X11Symbols::getInstance()->xResizeWindow (display, (Window) getWindowHandle(),
                                                      static_cast<u32> (rect.right - rect.left),
                                                      static_cast<u32> (rect.bottom - rect.top));
           #endif
        }

        z0 setContentScaleFactor (f32 scale)
        {
            if (auto* pluginEditor = getEditorComp())
            {
                auto prevEditorBounds = pluginEditor->getLocalArea (this, lastBounds);

                {
                    const ScopedValueSetter<b8> resizingChildSetter (resizingChild, true);

                    pluginEditor->setScaleFactor (scale);
                    pluginEditor->setBounds (prevEditorBounds.withPosition (0, 0));
                }

                lastBounds = getSizeToContainChild();
                updateWindowSize();
            }
        }

       #if DRX_WINDOWS
        z0 mouseDown (const MouseEvent&) override
        {
            broughtToFront();
        }

        z0 broughtToFront() override
        {
            // for hosts like nuendo, need to also pop the MDI container to the
            // front when our comp is clicked on.
            if (! isCurrentlyBlockedByAnotherModalComponent())
                if (HWND parent = findMDIParentOf ((HWND) getWindowHandle()))
                    SetWindowPos (parent, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        #if DRX_WIN_PER_MONITOR_DPI_AWARE
         z0 checkHostWindowScaleFactor (b8 force = false)
         {
             auto hostWindowScale = (f32) getScaleFactorForWindow ((HostWindowType) hostWindow);

             if (force || (hostWindowScale > 0.0f && ! approximatelyEqual (hostWindowScale, wrapper.editorScaleFactor)))
                 wrapper.handleSetContentScaleFactor (hostWindowScale, force);
         }

         z0 timerCallback() override
         {
             checkHostWindowScaleFactor();
         }
        #endif
       #endif

    private:
        z0 updateWindowSize()
        {
            if (! resizingParent
                && getEditorComp() != nullptr
                && hostWindow != HostWindowType{})
            {
                const auto editorBounds = getSizeToContainChild();
                resizeHostWindow (editorBounds);

                {
                    const ScopedValueSetter<b8> resizingParentSetter (resizingParent, true);

                    // setSize() on linux causes renoise and energyxt to fail.
                    // We'll resize our peer during resizeHostWindow() instead.
                   #if ! (DRX_LINUX || DRX_BSD)
                    setSize (editorBounds.getWidth(), editorBounds.getHeight());
                   #endif

                    if (auto* p = getPeer())
                        p->updateBounds();
                }

               #if DRX_MAC
                resizeHostWindow (editorBounds); // (doing this a second time seems to be necessary in tracktion)
               #endif
            }
        }

        //==============================================================================
        static Vst2::ERect convertToHostBounds (const Vst2::ERect& rect)
        {
            auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();

            if (approximatelyEqual (desktopScale, 1.0f))
                return rect;

            return { (i16) roundToInt (rect.top    * desktopScale),
                     (i16) roundToInt (rect.left   * desktopScale),
                     (i16) roundToInt (rect.bottom * desktopScale),
                     (i16) roundToInt (rect.right  * desktopScale) };
        }

        //==============================================================================
       #if DRX_LINUX || DRX_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostEventLoop;
       #endif

        //==============================================================================
        DrxVSTWrapper& wrapper;
        b8 resizingChild = false, resizingParent = false;

        drx::Rectangle<i32> lastBounds;

       #if DRX_LINUX || DRX_BSD
        using HostWindowType = ::Window;
        ::Display* display = XWindowSystem::getInstance()->getDisplay();
       #elif DRX_WINDOWS
        using HostWindowType = HWND;
        detail::WindowsHooks hooks;
       #else
        using HostWindowType = uk;
       #endif

        HostWindowType hostWindow = {};

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorCompWrapper)
    };

    //==============================================================================
private:
    struct HostChangeUpdater final : private AsyncUpdater
    {
        explicit HostChangeUpdater (DrxVSTWrapper& o)  : owner (o) {}
        ~HostChangeUpdater() override  { cancelPendingUpdate(); }

        z0 update (const ChangeDetails& details)
        {
            if (details.latencyChanged)
            {
                owner.vstEffect.initialDelay = owner.processor->getLatencySamples();
                callbackBits |= audioMasterIOChangedBit;
            }

            if (details.parameterInfoChanged || details.programChanged)
                callbackBits |= audioMasterUpdateDisplayBit;

            triggerAsyncUpdate();
        }

    private:
        z0 handleAsyncUpdate() override
        {
            const auto callbacksToFire = callbackBits.exchange (0);

            if (auto* callback = owner.hostCallback)
            {
                struct FlagPair
                {
                    Vst2::AudioMasterOpcodesX opcode;
                    i32 bit;
                };

                constexpr FlagPair pairs[] { { Vst2::audioMasterUpdateDisplay, audioMasterUpdateDisplayBit },
                                             { Vst2::audioMasterIOChanged,     audioMasterIOChangedBit } };

                for (const auto& pair : pairs)
                    if ((callbacksToFire & pair.bit) != 0)
                        callback (&owner.vstEffect, pair.opcode, 0, 0, nullptr, 0);
            }
        }

        static constexpr auto audioMasterUpdateDisplayBit = 1 << 0;
        static constexpr auto audioMasterIOChangedBit     = 1 << 1;

        DrxVSTWrapper& owner;
        std::atomic<i32> callbackBits { 0 };
    };

    static DrxVSTWrapper* getWrapper (Vst2::AEffect* v) noexcept  { return static_cast<DrxVSTWrapper*> (v->object); }

    b8 isProcessLevelOffline()
    {
        return hostCallback != nullptr
                && (i32) hostCallback (&vstEffect, Vst2::audioMasterGetCurrentProcessLevel, 0, 0, nullptr, 0) == 4;
    }

    static i32 convertHexVersionToDecimal (u32k hexVersion)
    {
       #if DRX_VST_RETURN_HEX_VERSION_NUMBER_DIRECTLY
        return (i32) hexVersion;
       #else
        // Currently, only Cubase displays the version number to the user
        // We are hoping here that when other DAWs start to display the version
        // number, that they do so according to yfede's encoding table in the link
        // below. If not, then this code will need an if (isSteinberg()) in the
        // future.
        i32 major = (hexVersion >> 16) & 0xff;
        i32 minor = (hexVersion >> 8) & 0xff;
        i32 bugfix = hexVersion & 0xff;

        // for details, see: https://forum.drx.com/t/issues-with-version-integer-reported-by-vst2/23867

        // Encoding B
        if (major < 1)
            return major * 1000 + minor * 100 + bugfix * 10;

        // Encoding E
        if (major > 100)
            return major * 10000000 + minor * 100000 + bugfix * 1000;

        // Encoding D
        return static_cast<i32> (hexVersion);
       #endif
    }

    //==============================================================================
   #if DRX_WINDOWS
    // Workarounds for hosts which attempt to open editor windows on a non-GUI thread.. (Grrrr...)
    static z0 checkWhetherMessageThreadIsCorrect()
    {
        auto host = detail::PluginUtilities::getHostType();

        if (host.isWavelab() || host.isCubaseBridged() || host.isPremiere())
        {
            if (! messageThreadIsDefinitelyCorrect)
            {
                MessageManager::getInstance()->setCurrentThreadAsMessageThread();

                struct MessageThreadCallback final : public CallbackMessage
                {
                    MessageThreadCallback (b8& tr) : triggered (tr) {}
                    z0 messageCallback() override     { triggered = true; }

                    b8& triggered;
                };

                (new MessageThreadCallback (messageThreadIsDefinitelyCorrect))->post();
            }
        }
    }
   #else
    static z0 checkWhetherMessageThreadIsCorrect() {}
   #endif

    z0 setValueAndNotifyIfChanged (AudioProcessorParameter& param, f32 newValue)
    {
        if (approximatelyEqual (param.getValue(), newValue))
            return;

        inParameterChangedCallback = true;
        param.setValueNotifyingHost (newValue);
    }

    //==============================================================================
    template <typename FloatType>
    z0 deleteTempChannels (VstTempBuffers<FloatType>& tmpBuffers)
    {
        tmpBuffers.release();

        if (processor != nullptr)
            tmpBuffers.tempChannels.insertMultiple (0, nullptr, vstEffect.numInputs
                                                                 + vstEffect.numOutputs);
    }

    z0 deleteTempChannels()
    {
        deleteTempChannels (floatTempBuffers);
        deleteTempChannels (doubleTempBuffers);
    }

    //==============================================================================
    z0 findMaxTotalChannels (i32& maxTotalIns, i32& maxTotalOuts)
    {
       #ifdef DrxPlugin_PreferredChannelConfigurations
        i32 configs[][2] = { DrxPlugin_PreferredChannelConfigurations };
        maxTotalIns = maxTotalOuts = 0;

        for (auto& config : configs)
        {
            maxTotalIns =  jmax (maxTotalIns,  config[0]);
            maxTotalOuts = jmax (maxTotalOuts, config[1]);
        }
       #else
        auto numInputBuses  = processor->getBusCount (true);
        auto numOutputBuses = processor->getBusCount (false);

        if (numInputBuses > 1 || numOutputBuses > 1)
        {
            maxTotalIns = maxTotalOuts = 0;

            for (i32 i = 0; i < numInputBuses; ++i)
                maxTotalIns  += processor->getChannelCountOfBus (true, i);

            for (i32 i = 0; i < numOutputBuses; ++i)
                maxTotalOuts += processor->getChannelCountOfBus (false, i);
        }
        else
        {
            maxTotalIns  = numInputBuses  > 0 ? processor->getBus (true,  0)->getMaxSupportedChannels (64) : 0;
            maxTotalOuts = numOutputBuses > 0 ? processor->getBus (false, 0)->getMaxSupportedChannels (64) : 0;
        }
       #endif
    }

    b8 pluginHasSidechainsOrAuxs() const  { return (processor->getBusCount (true) > 1 || processor->getBusCount (false) > 1); }

    //==============================================================================
    /** Host to plug-in calls. */

    pointer_sized_int handleOpen (VstOpCodeArguments)
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        setHasEditorFlag (processor->hasEditor());

        return 0;
    }

    pointer_sized_int handleClose (VstOpCodeArguments)
    {
        // Note: most hosts call this on the UI thread, but wavelab doesn't, so be careful in here.
        timedCallback.stopTimer();

        if (MessageManager::getInstance()->isThisTheMessageThread())
            deleteEditor (false);

        return 0;
    }

    pointer_sized_int handleSetCurrentProgram (VstOpCodeArguments args)
    {
        if (processor != nullptr && isPositiveAndBelow ((i32) args.value, processor->getNumPrograms()))
            processor->setCurrentProgram ((i32) args.value);

        return 0;
    }

    pointer_sized_int handleGetCurrentProgram (VstOpCodeArguments)
    {
        return (processor != nullptr && processor->getNumPrograms() > 0 ? processor->getCurrentProgram() : 0);
    }

    pointer_sized_int handleSetCurrentProgramName (VstOpCodeArguments args)
    {
        if (processor != nullptr && processor->getNumPrograms() > 0)
            processor->changeProgramName (processor->getCurrentProgram(), (tuk) args.ptr);

        return 0;
    }

    pointer_sized_int handleGetCurrentProgramName (VstOpCodeArguments args)
    {
        if (processor != nullptr && processor->getNumPrograms() > 0)
            processor->getProgramName (processor->getCurrentProgram()).copyToUTF8 ((tuk) args.ptr, 24 + 1);

        return 0;
    }

    pointer_sized_int handleGetParameterLabel (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            param->getLabel().copyToUTF8 ((tuk) args.ptr, 24 + 1);
        }

        return 0;
    }

    pointer_sized_int handleGetParameterText (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            param->getCurrentValueAsText().copyToUTF8 ((tuk) args.ptr, 24 + 1);
        }

        return 0;
    }

    pointer_sized_int handleGetParameterName (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            // length should technically be kVstMaxParamStrLen, which is 8, but hosts will normally allow a bit more.
            param->getName (32).copyToUTF8 ((tuk) args.ptr, 32 + 1);
        }

        return 0;
    }

    pointer_sized_int handleSetSampleRate (VstOpCodeArguments args)
    {
        sampleRate = args.opt;
        return 0;
    }

    pointer_sized_int handleSetBlockSize (VstOpCodeArguments args)
    {
        blockSize = (i32) args.value;
        return 0;
    }

    pointer_sized_int handleResumeSuspend (VstOpCodeArguments args)
    {
        if (args.value)
            resume();
        else
            suspend();

        return 0;
    }

    pointer_sized_int handleGetEditorBounds (VstOpCodeArguments args)
    {
        checkWhetherMessageThreadIsCorrect();
       #if DRX_LINUX || DRX_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
       #else
        const MessageManagerLock mmLock;
       #endif
        createEditorComp();

        if (editorComp != nullptr)
        {
            editorComp->getEditorBounds (editorRect);
            *((Vst2::ERect**) args.ptr) = &editorRect;
            return (pointer_sized_int) &editorRect;
        }

        return 0;
    }

    pointer_sized_int handleOpenEditor (VstOpCodeArguments args)
    {
        checkWhetherMessageThreadIsCorrect();
       #if DRX_LINUX || DRX_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
       #else
        const MessageManagerLock mmLock;
       #endif
        jassert (! recursionCheck);

        timedCallback.startTimerHz (4); // performs misc housekeeping chores

        deleteEditor (true);
        createEditorComp();

        if (editorComp != nullptr)
        {
            editorComp->attachToHost (args);
            return 1;
        }

        return 0;
    }

    pointer_sized_int handleCloseEditor (VstOpCodeArguments)
    {
        checkWhetherMessageThreadIsCorrect();

       #if DRX_LINUX || DRX_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
       #else
        const MessageManagerLock mmLock;
       #endif

        deleteEditor (true);

        return 0;
    }

    pointer_sized_int handleGetData (VstOpCodeArguments args)
    {
        if (processor == nullptr)
            return 0;

        auto data = (uk*) args.ptr;
        b8 onlyStoreCurrentProgramData = (args.index != 0);

        MemoryBlock block;

        if (onlyStoreCurrentProgramData)
            processor->getCurrentProgramStateInformation (block);
        else
            processor->getStateInformation (block);

        // IMPORTANT! Don't call getStateInfo while holding this lock!
        const ScopedLock lock (stateInformationLock);

        chunkMemory = std::move (block);
        *data = (uk) chunkMemory.getData();

        // because the chunk is only needed temporarily by the host (or at least you'd
        // hope so) we'll give it a while and then free it in the timer callback.
        chunkMemoryTime = drx::Time::getApproximateMillisecondCounter();

        return (i32) chunkMemory.getSize();
    }

    pointer_sized_int handleSetData (VstOpCodeArguments args)
    {
        if (processor != nullptr)
        {
            uk data = args.ptr;
            i32 byteSize = (i32) args.value;
            b8 onlyRestoreCurrentProgramData = (args.index != 0);

            {
                const ScopedLock lock (stateInformationLock);

                chunkMemory.reset();
                chunkMemoryTime = 0;
            }

            if (byteSize > 0 && data != nullptr)
            {
                if (onlyRestoreCurrentProgramData)
                    processor->setCurrentProgramStateInformation (data, byteSize);
                else
                    processor->setStateInformation (data, byteSize);
            }
        }

        return 0;
    }

    pointer_sized_int handlePreAudioProcessingEvents ([[maybe_unused]] VstOpCodeArguments args)
    {
        if (supportsMidiIn)
        {
            VSTMidiEventList::addEventsToMidiBuffer ((Vst2::VstEvents*) args.ptr, midiEvents);
            return 1;
        }

        return 0;
    }

    pointer_sized_int handleIsParameterAutomatable (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            const b8 isMeter = ((((u32) param->getCategory() & 0xffff0000) >> 16) == 2);
            return (param->isAutomatable() && (! isMeter) ? 1 : 0);
        }

        return 0;
    }

    pointer_sized_int handleParameterValueForText (VstOpCodeArguments args)
    {
        if (auto* param = juceParameters.getParamForIndex (args.index))
        {
            if (! LegacyAudioParameter::isLegacy (param))
            {
                setValueAndNotifyIfChanged (*param, param->getValueForText (Txt::fromUTF8 ((tuk) args.ptr)));
                return 1;
            }
        }

        return 0;
    }

    pointer_sized_int handleGetProgramName (VstOpCodeArguments args)
    {
        if (processor != nullptr && isPositiveAndBelow (args.index, processor->getNumPrograms()))
        {
            processor->getProgramName (args.index).copyToUTF8 ((tuk) args.ptr, 24 + 1);
            return 1;
        }

        return 0;
    }

    pointer_sized_int handleGetInputPinProperties (VstOpCodeArguments args)
    {
        return (processor != nullptr && getPinProperties (*(Vst2::VstPinProperties*) args.ptr, true, args.index)) ? 1 : 0;
    }

    pointer_sized_int handleGetOutputPinProperties (VstOpCodeArguments args)
    {
        return (processor != nullptr && getPinProperties (*(Vst2::VstPinProperties*) args.ptr, false, args.index)) ? 1 : 0;
    }

    pointer_sized_int handleGetPlugInCategory (VstOpCodeArguments)
    {
        return Vst2::DrxPlugin_VSTCategory;
    }

    pointer_sized_int handleSetSpeakerConfiguration (VstOpCodeArguments args)
    {
        auto* pluginInput  = reinterpret_cast<Vst2::VstSpeakerArrangement*> (args.value);
        auto* pluginOutput = reinterpret_cast<Vst2::VstSpeakerArrangement*> (args.ptr);

        if (processor->isMidiEffect())
            return 0;

        auto numIns  = processor->getBusCount (true);
        auto numOuts = processor->getBusCount (false);

        if (pluginInput != nullptr && pluginInput->type >= 0)
        {
            // inconsistent request?
            if (SpeakerMappings::vstArrangementTypeToChannelSet (*pluginInput).size() != pluginInput->numChannels)
                return 0;
        }

        if (pluginOutput != nullptr && pluginOutput->type >= 0)
        {
            // inconsistent request?
            if (SpeakerMappings::vstArrangementTypeToChannelSet (*pluginOutput).size() != pluginOutput->numChannels)
                return 0;
        }

        if (pluginInput != nullptr  && pluginInput->numChannels  > 0 && numIns  == 0)
            return 0;

        if (pluginOutput != nullptr && pluginOutput->numChannels > 0 && numOuts == 0)
            return 0;

        auto layouts = processor->getBusesLayout();

        if (pluginInput != nullptr && pluginInput-> numChannels >= 0 && numIns  > 0)
            layouts.getChannelSet (true,  0) = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginInput);

        if (pluginOutput != nullptr && pluginOutput->numChannels >= 0 && numOuts > 0)
            layouts.getChannelSet (false, 0) = SpeakerMappings::vstArrangementTypeToChannelSet (*pluginOutput);

       #ifdef DrxPlugin_PreferredChannelConfigurations
        short configs[][2] = { DrxPlugin_PreferredChannelConfigurations };
        if (! AudioProcessor::containsLayout (layouts, configs))
            return 0;
       #endif

        return processor->setBusesLayout (layouts) ? 1 : 0;
    }

    pointer_sized_int handleSetBypass (VstOpCodeArguments args)
    {
        isBypassed = args.value != 0;

        if (auto* param = processor->getBypassParameter())
            param->setValueNotifyingHost (isBypassed ? 1.0f : 0.0f);

        return 1;
    }

    pointer_sized_int handleGetPlugInName (VstOpCodeArguments args)
    {
        Txt (DrxPlugin_Name).copyToUTF8 ((tuk) args.ptr, 64 + 1);
        return 1;
    }

    pointer_sized_int handleGetManufacturerName (VstOpCodeArguments args)
    {
        Txt (DrxPlugin_Manufacturer).copyToUTF8 ((tuk) args.ptr, 64 + 1);
        return 1;
    }

    pointer_sized_int handleGetManufacturerVersion (VstOpCodeArguments)
    {
        return convertHexVersionToDecimal (DrxPlugin_VersionCode);
    }

    static std::optional<pointer_sized_int> handleVST3Compatibility ([[maybe_unused]] VstOpCodeArguments args)
    {
       #if ! DRX_VST3_CAN_REPLACE_VST2
        return {};
       #else
        if (args.index != (i32) ByteOrder::bigEndianInt ("stCA")
            && args.index != (i32) ByteOrder::bigEndianInt ("stCa"))
            return {};

        if (args.value != (i32) ByteOrder::bigEndianInt ("FUID"))
            return {};

        if (args.ptr == nullptr)
            return 0;

        const auto uid = VST3ClientExtensions::convertVST2PluginId (DrxPlugin_VSTUniqueID, DrxPlugin_Name, VST3ClientExtensions::InterfaceType::component);
        const auto uidString = Txt ((const t8 *) uid.data(), uid.size());
        MemoryBlock uidValue;
        uidValue.loadFromHexString (uidString);
        uidValue.copyTo (args.ptr, 0, uidValue.getSize());
        return 1;
       #endif
    }

    pointer_sized_int handleManufacturerSpecific (VstOpCodeArguments args)
    {
        if (const auto result = handleVST3Compatibility (args))
            return *result;

        if (args.index == (i32) ByteOrder::bigEndianInt ("PreS")
             && args.value == (i32) ByteOrder::bigEndianInt ("AeCs"))
            return handleSetContentScaleFactor (args.opt);

        if (args.index == Vst2::effGetParamDisplay)
            return handleCockosGetParameterText (args.value, args.ptr, args.opt);

        if (auto callbackHandler = processor->getVST2ClientExtensions())
            return callbackHandler->handleVstManufacturerSpecific (args.index, args.value, args.ptr, args.opt);

        return 0;
    }

    pointer_sized_int handleCanPlugInDo (VstOpCodeArguments args)
    {
        auto text = (tukk) args.ptr;
        auto matches = [=] (tukk s) { return strcmp (text, s) == 0; };

        if (matches ("receiveVstEvents")
         || matches ("receiveVstMidiEvent")
         || matches ("receiveVstMidiEvents"))
        {
            return supportsMidiIn ? 1 : -1;
        }

        if (matches ("sendVstEvents")
         || matches ("sendVstMidiEvent")
         || matches ("sendVstMidiEvents"))
        {
            return supportsMidiOut ? 1 : -1;
        }

        if (matches ("receiveVstTimeInfo")
         || matches ("conformsToWindowRules")
         || matches ("supportsViewDpiScaling")
         || matches ("bypass"))
        {
            return 1;
        }

        // This tells Wavelab to use the UI thread to invoke open/close,
        // like all other hosts do.
        if (matches ("openCloseAnyThread"))
            return -1;

        if (matches ("MPE"))
            return processor->supportsMPE() ? 1 : 0;

       #if DRX_MAC
        if (matches ("hasCockosViewAsConfig"))
        {
            return (i32) 0xbeef0000;
        }
       #endif

        if (matches ("hasCockosExtensions"))
            return (i32) 0xbeef0000;

        if (auto callbackHandler = processor->getVST2ClientExtensions())
            return callbackHandler->handleVstPluginCanDo (args.index, args.value, args.ptr, args.opt);

        return 0;
    }

    pointer_sized_int handleGetTailSize (VstOpCodeArguments)
    {
        if (processor != nullptr)
        {
            i32 result;

            auto tailSeconds = processor->getTailLengthSeconds();

            if (std::isinf (tailSeconds))
                result = std::numeric_limits<i32>::max();
            else
                result = static_cast<i32> (tailSeconds * sampleRate);

            return result; // Vst2 expects an i32 upcasted to a intptr_t here
        }

        return 0;
    }

    pointer_sized_int handleKeyboardFocusRequired (VstOpCodeArguments)
    {
        DRX_BEGIN_IGNORE_WARNINGS_MSVC (6326)
        return (DrxPlugin_EditorRequiresKeyboardFocus != 0) ? 1 : 0;
        DRX_END_IGNORE_WARNINGS_MSVC
    }

    pointer_sized_int handleGetVstInterfaceVersion (VstOpCodeArguments)
    {
        return kVstVersion;
    }

    pointer_sized_int handleGetCurrentMidiProgram (VstOpCodeArguments)
    {
        return -1;
    }

    pointer_sized_int handleGetSpeakerConfiguration (VstOpCodeArguments args)
    {
        auto** pluginInput  = reinterpret_cast<Vst2::VstSpeakerArrangement**> (args.value);
        auto** pluginOutput = reinterpret_cast<Vst2::VstSpeakerArrangement**> (args.ptr);

        if (pluginHasSidechainsOrAuxs() || processor->isMidiEffect())
            return false;

        auto inputLayout  = processor->getChannelLayoutOfBus (true, 0);
        auto outputLayout = processor->getChannelLayoutOfBus (false, 0);

        const auto speakerBaseSize = offsetof (Vst2::VstSpeakerArrangement, speakers);

        cachedInArrangement .malloc (speakerBaseSize + (static_cast<std::size_t> (inputLayout. size()) * sizeof (Vst2::VstSpeakerProperties)), 1);
        cachedOutArrangement.malloc (speakerBaseSize + (static_cast<std::size_t> (outputLayout.size()) * sizeof (Vst2::VstSpeakerProperties)), 1);

        *pluginInput  = cachedInArrangement. getData();
        *pluginOutput = cachedOutArrangement.getData();

        if (*pluginInput != nullptr)
            SpeakerMappings::channelSetToVstArrangement (processor->getChannelLayoutOfBus (true,  0), **pluginInput);

        if (*pluginOutput != nullptr)
            SpeakerMappings::channelSetToVstArrangement (processor->getChannelLayoutOfBus (false, 0), **pluginOutput);

        return 1;
    }

    pointer_sized_int handleSetNumberOfSamplesToProcess (VstOpCodeArguments args)
    {
        return args.value;
    }

    pointer_sized_int handleSetSampleFloatType (VstOpCodeArguments args)
    {
        if (! isProcessing)
        {
            if (processor != nullptr)
            {
                processor->setProcessingPrecision ((args.value == Vst2::kVstProcessPrecision64
                                                     && processor->supportsDoublePrecisionProcessing())
                                                         ? AudioProcessor::doublePrecision
                                                         : AudioProcessor::singlePrecision);

                return 1;
            }
        }

        return 0;
    }

    pointer_sized_int handleSetContentScaleFactor ([[maybe_unused]] f32 scale, [[maybe_unused]] b8 force = false)
    {
        checkWhetherMessageThreadIsCorrect();
       #if DRX_LINUX || DRX_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
       #else
        const MessageManagerLock mmLock;
       #endif

       #if ! DRX_MAC
        if (force || ! approximatelyEqual (scale, editorScaleFactor))
        {
            editorScaleFactor = scale;

            if (editorComp != nullptr)
                editorComp->setContentScaleFactor (editorScaleFactor);
        }
       #endif

        return 1;
    }

    pointer_sized_int handleCockosGetParameterText (pointer_sized_int paramIndex,
                                                    uk dest,
                                                    f32 value)
    {
        if (processor != nullptr && dest != nullptr)
        {
            if (auto* param = juceParameters.getParamForIndex ((i32) paramIndex))
            {
                if (! LegacyAudioParameter::isLegacy (param))
                {
                    Txt text (param->getText (value, 1024));
                    memcpy (dest, text.toRawUTF8(), ((size_t) text.length()) + 1);
                    return 0xbeef;
                }
            }
        }

        return 0;
    }

    //==============================================================================
    pointer_sized_int handleGetNumMidiInputChannels()
    {
        if (supportsMidiIn)
        {
           #ifdef DrxPlugin_VSTNumMidiInputs
            return DrxPlugin_VSTNumMidiInputs;
           #else
            return 16;
           #endif
        }

        return 0;
    }

    pointer_sized_int handleGetNumMidiOutputChannels()
    {
        if (supportsMidiOut)
        {
           #ifdef DrxPlugin_VSTNumMidiOutputs
            return DrxPlugin_VSTNumMidiOutputs;
           #else
            return 16;
           #endif
        }

        return 0;
    }

    pointer_sized_int handleGetMidiKeyName (VstOpCodeArguments args)
    {
        if (processor != nullptr)
        {
            auto keyName = (Vst2::MidiKeyName*) args.ptr;

            if (auto name = processor->getNameForMidiNoteNumber (keyName->thisKeyNumber, args.index))
            {
                name->copyToUTF8 (keyName->keyName, Vst2::kVstMaxNameLen);
                return 1;
            }
        }

        return 0;
    }

    pointer_sized_int handleEditIdle()
    {
       #if DRX_LINUX || DRX_BSD
        SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
        hostDrivenEventLoop->processPendingEvents();
       #endif

        return 0;
    }

    //==============================================================================
    ScopedDrxInitialiser_GUI libraryInitialiser;

   #if DRX_LINUX || DRX_BSD
    SharedResourcePointer<detail::MessageThread> messageThread;
   #endif

    TimedCallback timedCallback { [this]
    {
        if (shouldDeleteEditor)
        {
            shouldDeleteEditor = false;
            deleteEditor (true);
        }

        {
            ScopedLock lock (stateInformationLock);

            if (chunkMemoryTime > 0
                && chunkMemoryTime < drx::Time::getApproximateMillisecondCounter() - 2000
                && ! recursionCheck)
            {
                chunkMemory.reset();
                chunkMemoryTime = 0;
            }
        }
    } };

    Vst2::audioMasterCallback hostCallback;
    std::unique_ptr<AudioProcessor> processor;
    f64 sampleRate = 44100.0;
    i32 blockSize = 1024;
    Vst2::AEffect vstEffect;
    CriticalSection stateInformationLock;
    drx::MemoryBlock chunkMemory;
    u32 chunkMemoryTime = 0;
    f32 editorScaleFactor = 1.0f;
    std::unique_ptr<EditorCompWrapper> editorComp;
    Vst2::ERect editorRect;
    MidiBuffer midiEvents;
    VSTMidiEventList outgoingEvents;
    Optional<PositionInfo> currentPosition;

    LegacyAudioParametersWrapper juceParameters;

    b8 isProcessing = false, isBypassed = false, hasShutdown = false;
    b8 firstProcessCallback = true, shouldDeleteEditor = false;
    const b8 supportsMidiIn  = processor->isMidiEffect() || processor->acceptsMidi();
    const b8 supportsMidiOut = processor->isMidiEffect() || processor->producesMidi();

    VstTempBuffers<f32> floatTempBuffers;
    VstTempBuffers<f64> doubleTempBuffers;
    i32 maxNumInChannels = 0, maxNumOutChannels = 0;

    HeapBlock<Vst2::VstSpeakerArrangement> cachedInArrangement, cachedOutArrangement;

    ThreadLocalValue<b8> inParameterChangedCallback;

    HostChangeUpdater hostChangeUpdater { *this };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrxVSTWrapper)
};


//==============================================================================
namespace
{
    Vst2::AEffect* pluginEntryPoint (Vst2::audioMasterCallback audioMaster)
    {
        DRX_AUTORELEASEPOOL
        {
            ScopedDrxInitialiser_GUI libraryInitialiser;

           #if DRX_LINUX || DRX_BSD
            SharedResourcePointer<detail::HostDrivenEventLoop> hostDrivenEventLoop;
           #endif

            try
            {
                if (audioMaster (nullptr, Vst2::audioMasterVersion, 0, 0, nullptr, 0) != 0)
                {
                    std::unique_ptr<AudioProcessor> processor { createPluginFilterOfType (AudioProcessor::wrapperType_VST) };
                    auto* processorPtr = processor.get();
                    auto* wrapper = new DrxVSTWrapper (audioMaster, std::move (processor));
                    auto* aEffect = wrapper->getAEffect();

                    if (auto* callbackHandler = processorPtr->getVST2ClientExtensions())
                    {
                        callbackHandler->handleVstHostCallbackAvailable ([audioMaster, aEffect] (i32 opcode, i32 index, pointer_sized_int value, uk ptr, f32 opt)
                        {
                            return audioMaster (aEffect, opcode, index, value, ptr, opt);
                        });
                    }

                    return aEffect;
                }
            }
            catch (...)
            {}
        }

        return nullptr;
    }
}

#if ! DRX_WINDOWS
 #define DRX_EXPORTED_FUNCTION extern "C" __attribute__ ((visibility ("default")))
#endif

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes")

//==============================================================================
// Mac startup code..
#if DRX_MAC

    DRX_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster);
    DRX_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

    DRX_EXPORTED_FUNCTION Vst2::AEffect* main_macho (Vst2::audioMasterCallback audioMaster);
    DRX_EXPORTED_FUNCTION Vst2::AEffect* main_macho (Vst2::audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

//==============================================================================
// Linux startup code..
#elif DRX_LINUX || DRX_BSD

    DRX_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster);
    DRX_EXPORTED_FUNCTION Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

    DRX_EXPORTED_FUNCTION Vst2::AEffect* main_plugin (Vst2::audioMasterCallback audioMaster) asm ("main");
    DRX_EXPORTED_FUNCTION Vst2::AEffect* main_plugin (Vst2::audioMasterCallback audioMaster)
    {
        return VSTPluginMain (audioMaster);
    }

    // don't put initialiseDrx_GUI or shutdownDrx_GUI in these... it will crash!
    __attribute__ ((constructor)) z0 myPluginInit() {}
    __attribute__ ((destructor))  z0 myPluginFini() {}

//==============================================================================
// Win32 startup code..
#else

    extern "C" __declspec (dllexport) Vst2::AEffect* VSTPluginMain (Vst2::audioMasterCallback audioMaster)
    {
        return pluginEntryPoint (audioMaster);
    }

   #if ! defined (DRX_64BIT) && DRX_MSVC // (can't compile this on win64, but it's not needed anyway with VST2.4)
    extern "C" __declspec (dllexport) i32 main (Vst2::audioMasterCallback audioMaster)
    {
        return (i32) pluginEntryPoint (audioMaster);
    }
   #endif

    extern "C" BOOL WINAPI DllMain (HINSTANCE instance, DWORD reason, LPVOID)
    {
        if (reason == DLL_PROCESS_ATTACH)
            Process::setCurrentModuleInstanceHandle (instance);

        return true;
    }
#endif

DRX_END_IGNORE_WARNINGS_GCC_LIKE

DRX_END_IGNORE_WARNINGS_MSVC

#endif
