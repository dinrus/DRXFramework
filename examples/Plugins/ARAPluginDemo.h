/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:                     ARAPluginDemo
 version:                  1.0.0
 vendor:                   DRX
 website:                  http://drx.com
 description:              Audio plugin using the ARA API.

 dependencies:             drx_audio_basics, drx_audio_devices, drx_audio_formats,
                           drx_audio_plugin_client, drx_audio_processors,
                           drx_audio_utils, drx_core, drx_data_structures,
                           drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:                xcode_mac, vs2022

 moduleFlags:              DRX_STRICT_REFCOUNTEDPOINTER=1

 type:                     AudioProcessor
 mainClass:                ARADemoPluginAudioProcessor
 documentControllerClass:  ARADemoPluginDocumentControllerSpecialisation

 useLocalCopy:             1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include <ARA_Library/Utilities/ARAPitchInterpretation.h>
#include <ARA_Library/Utilities/ARATimelineConversion.h>

//==============================================================================
class ARADemoPluginAudioModification final : public ARAAudioModification
{
public:
    ARADemoPluginAudioModification (ARAAudioSource* audioSource,
                                    ARA::ARAAudioModificationHostRef hostRef,
                                    const ARAAudioModification* optionalModificationToClone)
        : ARAAudioModification (audioSource, hostRef, optionalModificationToClone)
    {
        if (optionalModificationToClone != nullptr)
            dimmed = static_cast<const ARADemoPluginAudioModification*> (optionalModificationToClone)->dimmed;
    }

    b8 isDimmed() const           { return dimmed; }
    z0 setDimmed (b8 shouldDim) { dimmed = shouldDim; }

private:
    b8 dimmed = false;
};

//==============================================================================
struct PreviewState
{
    std::atomic<f64> previewTime { 0.0 };
    std::atomic<ARAPlaybackRegion*> previewedRegion { nullptr };
};

class SharedTimeSliceThread final : public TimeSliceThread
{
public:
    SharedTimeSliceThread()
        : TimeSliceThread (Txt (DrxPlugin_Name) + " ARA Sample Reading Thread")
    {
        startThread (Priority::high);  // Above default priority so playback is fluent, but below realtime
    }
};

class AsyncConfigurationCallback final : private AsyncUpdater
{
public:
    explicit AsyncConfigurationCallback (std::function<z0()> callbackIn)
        : callback (std::move (callbackIn)) {}

    ~AsyncConfigurationCallback() override { cancelPendingUpdate(); }

    template <typename RequiresLock>
    auto withLock (RequiresLock&& fn)
    {
        const SpinLock::ScopedTryLockType scope (processingFlag);
        return fn (scope.isLocked());
    }

    z0 startConfigure() { triggerAsyncUpdate(); }

private:
    z0 handleAsyncUpdate() override
    {
        const SpinLock::ScopedLockType scope (processingFlag);
        callback();
    }

    std::function<z0()> callback;
    SpinLock processingFlag;
};

static z0 crossfade (const f32* sourceA,
                       const f32* sourceB,
                       f32 aProportionAtStart,
                       f32 aProportionAtFinish,
                       f32* destinationBuffer,
                       i32 numSamples)
{
    AudioBuffer<f32> destination { &destinationBuffer, 1, numSamples };
    destination.copyFromWithRamp (0, 0, sourceA, numSamples, aProportionAtStart, aProportionAtFinish);
    destination.addFromWithRamp (0, 0, sourceB, numSamples, 1.0f - aProportionAtStart, 1.0f - aProportionAtFinish);
}

class Looper
{
public:
    Looper() : inputBuffer (nullptr), pos (loopRange.getStart())
    {
    }

    Looper (const AudioBuffer<f32>* buffer, Range<z64> range)
        : inputBuffer (buffer), loopRange (range), pos (range.getStart())
    {
    }

    z0 writeInto (AudioBuffer<f32>& buffer)
    {
        if (loopRange.getLength() == 0)
        {
            buffer.clear();
            return;
        }

        const auto numChannelsToCopy = std::min (inputBuffer->getNumChannels(), buffer.getNumChannels());
        const auto actualCrossfadeLengthSamples = std::min (loopRange.getLength() / 2, (z64) desiredCrossfadeLengthSamples);

        for (auto samplesCopied = 0; samplesCopied < buffer.getNumSamples();)
        {
            const auto [needsCrossfade, samplePosOfNextCrossfadeTransition] = [&]() -> std::pair<b8, z64>
            {
                if (const auto endOfFadeIn = loopRange.getStart() + actualCrossfadeLengthSamples; pos < endOfFadeIn)
                    return { true, endOfFadeIn };

                return { false, loopRange.getEnd() - actualCrossfadeLengthSamples };
            }();

            const auto samplesToNextCrossfadeTransition = samplePosOfNextCrossfadeTransition - pos;
            const auto numSamplesToCopy = std::min (buffer.getNumSamples() - samplesCopied,
                                                    (i32) samplesToNextCrossfadeTransition);

            const auto getFadeInGainAtPos = [this, actualCrossfadeLengthSamples] (auto p)
            {
                return jmap ((f32) p, (f32) loopRange.getStart(), (f32) loopRange.getStart() + (f32) actualCrossfadeLengthSamples - 1.0f, 0.0f, 1.0f);
            };

            for (i32 i = 0; i < numChannelsToCopy; ++i)
            {
                if (needsCrossfade)
                {
                    const auto overlapStart = loopRange.getEnd() - actualCrossfadeLengthSamples
                                              + (pos - loopRange.getStart());

                    crossfade (inputBuffer->getReadPointer (i, (i32) pos),
                               inputBuffer->getReadPointer (i, (i32) overlapStart),
                               getFadeInGainAtPos (pos),
                               getFadeInGainAtPos (pos + numSamplesToCopy),
                               buffer.getWritePointer (i, samplesCopied),
                               numSamplesToCopy);
                }
                else
                {
                    buffer.copyFrom (i, samplesCopied, *inputBuffer, i, (i32) pos, numSamplesToCopy);
                }
            }

            samplesCopied += numSamplesToCopy;
            pos += numSamplesToCopy;

            jassert (pos <= loopRange.getEnd() - actualCrossfadeLengthSamples);

            if (pos == loopRange.getEnd() - actualCrossfadeLengthSamples)
                pos = loopRange.getStart();
        }
    }

private:
    static constexpr i32 desiredCrossfadeLengthSamples = 50;

    const AudioBuffer<f32>* inputBuffer;
    Range<z64> loopRange;
    z64 pos;
};

//==============================================================================
// Returns the modified sample range in the output buffer.
inline std::optional<Range<z64>> readPlaybackRangeIntoBuffer (Range<f64> playbackRange,
                                                                const ARAPlaybackRegion* playbackRegion,
                                                                AudioBuffer<f32>& buffer,
                                                                const std::function<AudioFormatReader* (ARAAudioSource*)>& getReader)
{
    const auto rangeInAudioModificationTime = playbackRange - playbackRegion->getStartInPlaybackTime()
                                                            + playbackRegion->getStartInAudioModificationTime();

    const auto audioModification = playbackRegion->getAudioModification<ARADemoPluginAudioModification>();
    const auto audioSource = audioModification->getAudioSource();
    const auto audioModificationSampleRate = audioSource->getSampleRate();

    const Range<z64> sampleRangeInAudioModification {
        ARA::roundSamplePosition (rangeInAudioModificationTime.getStart() * audioModificationSampleRate),
        ARA::roundSamplePosition (rangeInAudioModificationTime.getEnd() * audioModificationSampleRate) - 1
    };

    const auto inputOffset = jlimit ((z64) 0, audioSource->getSampleCount(), sampleRangeInAudioModification.getStart());

    // With the output offset it can always be said of the output buffer, that the zeroth element
    // corresponds to beginning of the playbackRange.
    const auto outputOffset = std::max (-sampleRangeInAudioModification.getStart(), (z64) 0);

    /* TODO: Handle different AudioSource and playback sample rates.

       The conversion should be done inside a specialized AudioFormatReader so that we could use
       playbackSampleRate everywhere in this function and we could still read `readLength` number of samples
       from the source.

       The current implementation will be incorrect when sampling rates differ.
    */
    const auto readLength = [&]
    {
        const auto sourceReadLength =
            std::min (sampleRangeInAudioModification.getEnd(), audioSource->getSampleCount()) - inputOffset;

        const auto outputReadLength =
            std::min (outputOffset + sourceReadLength, (z64) buffer.getNumSamples()) - outputOffset;

        return std::min (sourceReadLength, outputReadLength);
    }();

    if (readLength == 0)
        return Range<z64>();

    auto* reader = getReader (audioSource);

    if (reader != nullptr && reader->read (&buffer, (i32) outputOffset, (i32) readLength, inputOffset, true, true))
    {
        if (audioModification->isDimmed())
            buffer.applyGain ((i32) outputOffset, (i32) readLength, 0.25f);

        return Range<z64>::withStartAndLength (outputOffset, readLength);
    }

    return {};
}

class PossiblyBufferedReader
{
public:
    PossiblyBufferedReader() = default;

    explicit PossiblyBufferedReader (std::unique_ptr<BufferingAudioReader> readerIn)
        : setTimeoutFn ([ptr = readerIn.get()] (i32 ms) { ptr->setReadTimeout (ms); }),
          reader (std::move (readerIn))
    {}

    explicit PossiblyBufferedReader (std::unique_ptr<AudioFormatReader> readerIn)
        : setTimeoutFn(),
          reader (std::move (readerIn))
    {}

    z0 setReadTimeout (i32 ms)
    {
        NullCheckedInvocation::invoke (setTimeoutFn, ms);
    }

    AudioFormatReader* get() const { return reader.get(); }

private:
    std::function<z0 (i32)> setTimeoutFn;
    std::unique_ptr<AudioFormatReader> reader;
};

struct ProcessingLockInterface
{
    virtual ~ProcessingLockInterface() = default;
    virtual ScopedTryReadLock getProcessingLock() = 0;
};

//==============================================================================
class PlaybackRenderer final : public ARAPlaybackRenderer
{
public:
    PlaybackRenderer (ARA::PlugIn::DocumentController* dc, ProcessingLockInterface& lockInterfaceIn)
        : ARAPlaybackRenderer (dc), lockInterface (lockInterfaceIn) {}

    z0 prepareToPlay (f64 sampleRateIn,
                        i32 maximumSamplesPerBlockIn,
                        i32 numChannelsIn,
                        AudioProcessor::ProcessingPrecision,
                        AlwaysNonRealtime alwaysNonRealtime) override
    {
        numChannels = numChannelsIn;
        sampleRate = sampleRateIn;
        maximumSamplesPerBlock = maximumSamplesPerBlockIn;
        tempBuffer.reset (new AudioBuffer<f32> (numChannels, maximumSamplesPerBlock));

        useBufferedAudioSourceReader = alwaysNonRealtime == AlwaysNonRealtime::no;

        for (const auto playbackRegion : getPlaybackRegions())
        {
            auto audioSource = playbackRegion->getAudioModification()->getAudioSource();

            if (audioSourceReaders.find (audioSource) == audioSourceReaders.end())
            {
                auto reader = std::make_unique<ARAAudioSourceReader> (audioSource);

                if (! useBufferedAudioSourceReader)
                {
                    audioSourceReaders.emplace (audioSource,
                                                PossiblyBufferedReader { std::move (reader) });
                }
                else
                {
                    const auto readAheadSize = jmax (4 * maximumSamplesPerBlock,
                                                     roundToInt (2.0 * sampleRate));
                    audioSourceReaders.emplace (audioSource,
                                                PossiblyBufferedReader { std::make_unique<BufferingAudioReader> (reader.release(),
                                                                                                                 *sharedTimesliceThread,
                                                                                                                 readAheadSize) });
                }
            }
        }
    }

    z0 releaseResources() override
    {
        audioSourceReaders.clear();
        tempBuffer.reset();
    }

    b8 processBlock (AudioBuffer<f32>& buffer,
                       AudioProcessor::Realtime realtime,
                       const AudioPlayHead::PositionInfo& positionInfo) noexcept override
    {
        const auto lock = lockInterface.getProcessingLock();

        if (! lock.isLocked())
            return true;

        const auto numSamples = buffer.getNumSamples();
        jassert (numSamples <= maximumSamplesPerBlock);
        jassert (numChannels == buffer.getNumChannels());
        jassert (realtime == AudioProcessor::Realtime::no || useBufferedAudioSourceReader);
        const auto timeInSamples = positionInfo.getTimeInSamples().orFallback (0);
        const auto isPlaying = positionInfo.getIsPlaying();

        b8 success = true;
        b8 didRenderAnyRegion = false;

        if (isPlaying)
        {
            const auto blockRange = Range<z64>::withStartAndLength (timeInSamples, numSamples);

            for (const auto& playbackRegion : getPlaybackRegions())
            {
                // Evaluate region borders in song time, calculate sample range to render in song time.
                // Note that this example does not use head- or tailtime, so the includeHeadAndTail
                // parameter is set to false here - this might need to be adjusted in actual plug-ins.
                const auto playbackSampleRange = playbackRegion->getSampleRange (sampleRate, ARAPlaybackRegion::IncludeHeadAndTail::no);
                auto renderRange = blockRange.getIntersectionWith (playbackSampleRange);

                if (renderRange.isEmpty())
                    continue;

                // Evaluate region borders in modification/source time and calculate offset between
                // song and source samples, then clip song samples accordingly
                // (if an actual plug-in supports time stretching, this must be taken into account here).
                Range<z64> modificationSampleRange { playbackRegion->getStartInAudioModificationSamples(),
                                                       playbackRegion->getEndInAudioModificationSamples() };
                const auto modificationSampleOffset = modificationSampleRange.getStart() - playbackSampleRange.getStart();

                renderRange = renderRange.getIntersectionWith (modificationSampleRange.movedToStartAt (playbackSampleRange.getStart()));

                if (renderRange.isEmpty())
                    continue;

                // Get the audio source for the region and find the reader for that source.
                // This simplified example code only produces audio if sample rate and channel count match -
                // a robust plug-in would need to do conversion, see ARA SDK documentation.
                const auto audioSource = playbackRegion->getAudioModification()->getAudioSource();
                const auto readerIt = audioSourceReaders.find (audioSource);

                if (std::make_tuple (audioSource->getChannelCount(), audioSource->getSampleRate()) != std::make_tuple (numChannels, sampleRate)
                    || (readerIt == audioSourceReaders.end()))
                {
                    success = false;
                    continue;
                }

                auto& reader = readerIt->second;
                reader.setReadTimeout (realtime == AudioProcessor::Realtime::no ? 100 : 0);

                // Calculate buffer offsets.
                i32k numSamplesToRead = (i32) renderRange.getLength();
                i32k startInBuffer = (i32) (renderRange.getStart() - blockRange.getStart());
                auto startInSource = renderRange.getStart() + modificationSampleOffset;

                // Read samples:
                // first region can write directly into output, later regions need to use local buffer.
                auto& readBuffer = (didRenderAnyRegion) ? *tempBuffer : buffer;

                if (! reader.get()->read (&readBuffer, startInBuffer, numSamplesToRead, startInSource, true, true))
                {
                    success = false;
                    continue;
                }

                // Apply dim if enabled
                if (playbackRegion->getAudioModification<ARADemoPluginAudioModification>()->isDimmed())
                    readBuffer.applyGain (startInBuffer, numSamplesToRead, 0.25f);  // dim by about 12 dB

                // Mix output of all regions
                if (didRenderAnyRegion)
                {
                    // Mix local buffer into the output buffer.
                    for (i32 c = 0; c < numChannels; ++c)
                        buffer.addFrom (c, startInBuffer, *tempBuffer, c, startInBuffer, numSamplesToRead);
                }
                else
                {
                    // Clear any excess at start or end of the region.
                    if (startInBuffer != 0)
                        buffer.clear (0, startInBuffer);

                    i32k endInBuffer = startInBuffer + numSamplesToRead;
                    i32k remainingSamples = numSamples - endInBuffer;

                    if (remainingSamples != 0)
                        buffer.clear (endInBuffer, remainingSamples);

                    didRenderAnyRegion = true;
                }
            }
        }

        // If no playback or no region did intersect, clear buffer now.
        if (! didRenderAnyRegion)
            buffer.clear();

        return success;
    }

    using ARAPlaybackRenderer::processBlock;

private:
    //==============================================================================
    ProcessingLockInterface& lockInterface;
    SharedResourcePointer<SharedTimeSliceThread> sharedTimesliceThread;
    std::map<ARAAudioSource*, PossiblyBufferedReader> audioSourceReaders;
    b8 useBufferedAudioSourceReader = true;
    i32 numChannels = 2;
    f64 sampleRate = 48000.0;
    i32 maximumSamplesPerBlock = 128;
    std::unique_ptr<AudioBuffer<f32>> tempBuffer;
};

class EditorRenderer final : public ARAEditorRenderer,
                             private ARARegionSequence::Listener
{
public:
    EditorRenderer (ARA::PlugIn::DocumentController* documentController,
                    const PreviewState* previewStateIn,
                    ProcessingLockInterface& lockInterfaceIn)
        : ARAEditorRenderer (documentController),
          lockInterface (lockInterfaceIn),
          previewState (previewStateIn)
    {
        jassert (previewState != nullptr);
    }

    ~EditorRenderer() override
    {
        for (const auto& rs : regionSequences)
            rs->removeListener (this);
    }

    z0 didAddPlaybackRegionToRegionSequence (ARARegionSequence*, ARAPlaybackRegion*) override
    {
        asyncConfigCallback.startConfigure();
    }

    z0 didAddRegionSequence (ARA::PlugIn::RegionSequence* rs) noexcept override
    {
        auto* sequence = static_cast<ARARegionSequence*> (rs);
        sequence->addListener (this);
        regionSequences.insert (sequence);
        asyncConfigCallback.startConfigure();
    }

    z0 willRemoveRegionSequence (ARA::PlugIn::RegionSequence* rs) noexcept override
    {
        auto* rsToRemove = static_cast<ARARegionSequence*> (rs);
        rsToRemove->removeListener (this);
        regionSequences.erase (rsToRemove);
    }

    z0 didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion*) noexcept override
    {
        asyncConfigCallback.startConfigure();
    }

    /*  An ARA host could be using either the `addPlaybackRegion()` or `addRegionSequence()` interface
        so we need to check the other side of both.

        The callback must have a signature of `b8 (ARAPlaybackRegion*)`
    */
    template <typename Callback>
    z0 forEachPlaybackRegion (Callback&& cb)
    {
        for (const auto& playbackRegion : getPlaybackRegions())
            if (! cb (playbackRegion))
                return;

        for (const auto& regionSequence : getRegionSequences())
            for (const auto& playbackRegion : regionSequence->getPlaybackRegions())
                if (! cb (playbackRegion))
                    return;
    }

    z0 prepareToPlay (f64 sampleRateIn,
                        i32 maximumExpectedSamplesPerBlock,
                        i32 numChannels,
                        AudioProcessor::ProcessingPrecision,
                        AlwaysNonRealtime alwaysNonRealtime) override
    {
        sampleRate = sampleRateIn;
        previewBuffer = std::make_unique<AudioBuffer<f32>> (numChannels, (i32) (2 * sampleRateIn));

        ignoreUnused (maximumExpectedSamplesPerBlock, alwaysNonRealtime);
    }

    z0 releaseResources() override
    {
        audioSourceReaders.clear();
    }

    z0 reset() override
    {
        previewBuffer->clear();
    }

    b8 processBlock (AudioBuffer<f32>& buffer,
                       AudioProcessor::Realtime realtime,
                       const AudioPlayHead::PositionInfo& positionInfo) noexcept override
    {
        ignoreUnused (realtime);

        const auto lock = lockInterface.getProcessingLock();

        if (! lock.isLocked())
            return true;

        return asyncConfigCallback.withLock ([&] (b8 locked)
        {
            if (! locked)
                return true;

            const auto fadeOutIfNecessary = [this, &buffer]
            {
                if (std::exchange (wasPreviewing, false))
                {
                    previewLooper.writeInto (buffer);
                    const auto fadeOutStart = std::max (0, buffer.getNumSamples() - 50);
                    buffer.applyGainRamp (fadeOutStart, buffer.getNumSamples() - fadeOutStart, 1.0f, 0.0f);
                }
            };

            if (positionInfo.getIsPlaying())
            {
                fadeOutIfNecessary();
                return true;
            }

            if (const auto previewedRegion = previewState->previewedRegion.load())
            {
                const auto regionIsAssignedToEditor = [&]()
                {
                    b8 regionIsAssigned = false;

                    forEachPlaybackRegion ([&previewedRegion, &regionIsAssigned] (const auto& region)
                    {
                        if (region == previewedRegion)
                        {
                            regionIsAssigned = true;
                            return false;
                        }

                        return true;
                    });

                    return regionIsAssigned;
                }();

                if (regionIsAssignedToEditor)
                {
                    const auto previewTime = previewState->previewTime.load();
                    const auto previewDimmed = previewedRegion->getAudioModification<ARADemoPluginAudioModification>()
                                                              ->isDimmed();

                    if (! exactlyEqual (lastPreviewTime, previewTime)
                        || ! exactlyEqual (lastPlaybackRegion, previewedRegion)
                        || ! exactlyEqual (lastPreviewDimmed, previewDimmed))
                    {
                        Range<f64> previewRangeInPlaybackTime { previewTime - 0.25, previewTime + 0.25 };
                        previewBuffer->clear();
                        const auto rangeInOutput = readPlaybackRangeIntoBuffer (previewRangeInPlaybackTime,
                                                                                previewedRegion,
                                                                                *previewBuffer,
                                                                                [this] (auto* source) -> auto*
                                                                                {
                                                                                    const auto iter = audioSourceReaders.find (source);
                                                                                    return iter != audioSourceReaders.end() ? iter->second.get() : nullptr;
                                                                                });

                        if (rangeInOutput)
                        {
                            lastPreviewTime = previewTime;
                            lastPlaybackRegion = previewedRegion;
                            lastPreviewDimmed = previewDimmed;
                            previewLooper = Looper (previewBuffer.get(), *rangeInOutput);
                        }
                    }
                    else
                    {
                        previewLooper.writeInto (buffer);

                        if (! std::exchange (wasPreviewing, true))
                        {
                            const auto fadeInLength = std::min (50, buffer.getNumSamples());
                            buffer.applyGainRamp (0, fadeInLength, 0.0f, 1.0f);
                        }
                    }
                }
            }
            else
            {
                fadeOutIfNecessary();
            }

            return true;
        });
    }

    using ARAEditorRenderer::processBlock;

private:
    z0 configure()
    {
        forEachPlaybackRegion ([this, maximumExpectedSamplesPerBlock = 1000] (const auto& playbackRegion)
        {
            const auto audioSource = playbackRegion->getAudioModification()->getAudioSource();

            if (audioSourceReaders.find (audioSource) == audioSourceReaders.end())
            {
                audioSourceReaders[audioSource] = std::make_unique<BufferingAudioReader> (
                        new ARAAudioSourceReader (playbackRegion->getAudioModification()->getAudioSource()),
                        *timeSliceThread,
                        std::max (4 * maximumExpectedSamplesPerBlock, (i32) sampleRate));
            }

            return true;
        });
    }

    ProcessingLockInterface& lockInterface;
    const PreviewState* previewState = nullptr;
    AsyncConfigurationCallback asyncConfigCallback { [this] { configure(); } };
    f64 lastPreviewTime = 0.0;
    ARAPlaybackRegion* lastPlaybackRegion = nullptr;
    b8 lastPreviewDimmed = false;
    b8 wasPreviewing = false;
    std::unique_ptr<AudioBuffer<f32>> previewBuffer;
    Looper previewLooper;

    f64 sampleRate = 48000.0;
    SharedResourcePointer<SharedTimeSliceThread> timeSliceThread;
    std::map<ARAAudioSource*, std::unique_ptr<BufferingAudioReader>> audioSourceReaders;

    std::set<ARARegionSequence*> regionSequences;
};

//==============================================================================
class ARADemoPluginDocumentControllerSpecialisation final : public ARADocumentControllerSpecialisation,
                                                            private ProcessingLockInterface
{
public:
    using ARADocumentControllerSpecialisation::ARADocumentControllerSpecialisation;

    PreviewState previewState;

protected:
    z0 willBeginEditing (ARADocument*) override
    {
        processBlockLock.enterWrite();
    }

    z0 didEndEditing (ARADocument*) override
    {
        processBlockLock.exitWrite();
    }

    ARAAudioModification* doCreateAudioModification (ARAAudioSource* audioSource,
                                                     ARA::ARAAudioModificationHostRef hostRef,
                                                     const ARAAudioModification* optionalModificationToClone) noexcept override
    {
        return new ARADemoPluginAudioModification (audioSource,
                                                   hostRef,
                                                   static_cast<const ARADemoPluginAudioModification*> (optionalModificationToClone));
    }

    ARAPlaybackRenderer* doCreatePlaybackRenderer() noexcept override
    {
        return new PlaybackRenderer (getDocumentController(), *this);
    }

    EditorRenderer* doCreateEditorRenderer() noexcept override
    {
        return new EditorRenderer (getDocumentController(), &previewState, *this);
    }

    b8 doRestoreObjectsFromStream (ARAInputStream& input,
                                     const ARARestoreObjectsFilter* filter) noexcept override
    {
        // Start reading data from the archive, starting with the number of audio modifications in the archive
        const auto numAudioModifications = input.readInt64();

        // Loop over stored audio modification data
        for (z64 i = 0; i < numAudioModifications; ++i)
        {
            const auto progressVal = (f32) i / (f32) numAudioModifications;
            getDocumentController()->getHostArchivingController()->notifyDocumentUnarchivingProgress (progressVal);

            // Read audio modification persistent ID and analysis result from archive
            const Txt persistentID = input.readString();
            const b8 dimmed = input.readBool();

            // Find audio modification to restore the state to (drop state if not to be loaded)
            auto audioModification = filter->getAudioModificationToRestoreStateWithID<ARADemoPluginAudioModification> (persistentID.getCharPointer());

            if (audioModification == nullptr)
                continue;

            const b8 dimChanged = (dimmed != audioModification->isDimmed());
            audioModification->setDimmed (dimmed);

            // If the dim state changed, send a sample content change notification without notifying the host
            if (dimChanged)
            {
                audioModification->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected(), false);

                for (auto playbackRegion : audioModification->getPlaybackRegions())
                    playbackRegion->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected(), false);
            }
        }

        getDocumentController()->getHostArchivingController()->notifyDocumentUnarchivingProgress (1.0f);

        return ! input.failed();
    }

    b8 doStoreObjectsToStream (ARAOutputStream& output, const ARAStoreObjectsFilter* filter) noexcept override
    {
        // This example implementation only deals with audio modification states
        const auto& audioModificationsToPersist { filter->getAudioModificationsToStore<ARADemoPluginAudioModification>() };

        const auto reportProgress = [archivingController = getDocumentController()->getHostArchivingController()] (f32 p)
        {
            archivingController->notifyDocumentArchivingProgress (p);
        };

        const ScopeGuard scope { [&reportProgress] { reportProgress (1.0f); } };

        // Write the number of audio modifications we are persisting
        const auto numAudioModifications = audioModificationsToPersist.size();

        if (! output.writeInt64 ((z64) numAudioModifications))
            return false;

        // For each audio modification to persist, persist its ID followed by whether it's dimmed
        for (size_t i = 0; i < numAudioModifications; ++i)
        {
            // Write persistent ID and dim state
            if (! output.writeString (audioModificationsToPersist[i]->getPersistentID()))
                return false;

            if (! output.writeBool (audioModificationsToPersist[i]->isDimmed()))
                return false;

            const auto progressVal = (f32) i / (f32) numAudioModifications;
            reportProgress (progressVal);
        }

        return true;
    }

private:
    ScopedTryReadLock getProcessingLock() override
    {
        return ScopedTryReadLock { processBlockLock };
    }

    ReadWriteLock processBlockLock;
};

struct PlayHeadState
{
    z0 update (const Optional<AudioPlayHead::PositionInfo>& info)
    {
        if (info.hasValue())
        {
            isPlaying.store (info->getIsPlaying(), std::memory_order_relaxed);
            timeInSeconds.store (info->getTimeInSeconds().orFallback (0), std::memory_order_relaxed);
            isLooping.store (info->getIsLooping(), std::memory_order_relaxed);
            const auto loopPoints = info->getLoopPoints();

            if (loopPoints.hasValue())
            {
                loopPpqStart = loopPoints->ppqStart;
                loopPpqEnd = loopPoints->ppqEnd;
            }
        }
        else
        {
            isPlaying.store (false, std::memory_order_relaxed);
            isLooping.store (false, std::memory_order_relaxed);
        }
    }

    std::atomic<b8> isPlaying { false },
                      isLooping { false };
    std::atomic<f64> timeInSeconds { 0.0 },
                        loopPpqStart  { 0.0 },
                        loopPpqEnd    { 0.0 };
};

//==============================================================================
class ARADemoPluginAudioProcessorImpl : public AudioProcessor,
                                        public AudioProcessorARAExtension
{
public:
    //==============================================================================
    ARADemoPluginAudioProcessorImpl()
        : AudioProcessor (getBusesProperties())
    {}

    ~ARADemoPluginAudioProcessorImpl() override = default;

    //==============================================================================
    z0 prepareToPlay (f64 sampleRate, i32 samplesPerBlock) override
    {
        playHeadState.update (nullopt);
        prepareToPlayForARA (sampleRate, samplesPerBlock, getMainBusNumOutputChannels(), getProcessingPrecision());
    }

    z0 releaseResources() override
    {
        playHeadState.update (nullopt);
        releaseResourcesForARA();
    }

    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
            && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
            return false;

        return true;
    }

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer& midiMessages) override
    {
        ignoreUnused (midiMessages);

        ScopedNoDenormals noDenormals;

        auto* audioPlayHead = getPlayHead();
        playHeadState.update (audioPlayHead->getPosition());

        if (! processBlockForARA (buffer, isRealtime(), audioPlayHead))
            processBlockBypassed (buffer, midiMessages);
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    const Txt getName() const override                             { return "ARAPluginDemo"; }
    b8 acceptsMidi() const override                                 { return true; }
    b8 producesMidi() const override                                { return true; }

    f64 getTailLengthSeconds() const override
    {
        f64 tail;
        if (getTailLengthSecondsForARA (tail))
            return tail;

        return 0.0;
    }

    //==============================================================================
    i32 getNumPrograms() override                                     { return 0; }
    i32 getCurrentProgram() override                                  { return 0; }
    z0 setCurrentProgram (i32) override                             {}
    const Txt getProgramName (i32) override                        { return "None"; }
    z0 changeProgramName (i32, const Txt&) override              {}

    //==============================================================================
    z0 getStateInformation (MemoryBlock&) override                  {}
    z0 setStateInformation (ukk, i32) override              {}

    PlayHeadState playHeadState;

private:
    //==============================================================================
    static BusesProperties getBusesProperties()
    {
        return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
            .withOutput ("Output", AudioChannelSet::stereo(), true);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADemoPluginAudioProcessorImpl)
};

//==============================================================================
class TimeToViewScaling
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;

        virtual z0 zoomLevelChanged (f64 newPixelPerSecond) = 0;
    };

    z0 addListener (Listener* l)    { listeners.add (l); }
    z0 removeListener (Listener* l) { listeners.remove (l); }

    TimeToViewScaling() = default;

    z0 zoom (f64 factor)
    {
        zoomLevelPixelPerSecond = jlimit (minimumZoom, minimumZoom * 32, zoomLevelPixelPerSecond * factor);
        setZoomLevel (zoomLevelPixelPerSecond);
    }

    z0 setZoomLevel (f64 pixelPerSecond)
    {
        zoomLevelPixelPerSecond = pixelPerSecond;
        listeners.call ([this] (Listener& l) { l.zoomLevelChanged (zoomLevelPixelPerSecond); });
    }

    i32 getXForTime (f64 time) const
    {
        return roundToInt (time * zoomLevelPixelPerSecond);
    }

    f64 getTimeForX (i32 x) const
    {
        return x / zoomLevelPixelPerSecond;
    }

private:
    static constexpr auto minimumZoom = 10.0;

    f64 zoomLevelPixelPerSecond = minimumZoom * 4;
    ListenerList<Listener> listeners;
};

class RulersView final : public Component,
                         public SettableTooltipClient,
                         private Timer,
                         private TimeToViewScaling::Listener,
                         private ARAMusicalContext::Listener
{
public:
    class CycleMarkerComponent final : public Component
    {
        z0 paint (Graphics& g) override
        {
            g.setColor (Colors::yellow.darker (0.2f));
            const auto bounds = getLocalBounds().toFloat();
            g.drawRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 6.0f, 2.0f);
        }
    };

    RulersView (PlayHeadState& playHeadStateIn, TimeToViewScaling& timeToViewScalingIn, ARADocument& document)
        : playHeadState (playHeadStateIn), timeToViewScaling (timeToViewScalingIn), araDocument (document)
    {
        timeToViewScaling.addListener (this);

        addChildComponent (cycleMarker);
        cycleMarker.setInterceptsMouseClicks (false, false);

        setTooltip ("Double-click to start playback, click to stop playback or to reposition, drag horizontal range to set cycle.");

        startTimerHz (30);
    }

    ~RulersView() override
    {
        stopTimer();

        timeToViewScaling.removeListener (this);
        selectMusicalContext (nullptr);
    }

    z0 paint (Graphics& g) override
    {
        auto drawBounds = g.getClipBounds();
        const auto drawStartTime = timeToViewScaling.getTimeForX (drawBounds.getX());
        const auto drawEndTime   = timeToViewScaling.getTimeForX (drawBounds.getRight());

        const auto bounds = getLocalBounds();

        g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
        g.fillRect (bounds);
        g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId).contrasting());
        g.drawRect (bounds);

        const auto rulerHeight = bounds.getHeight() / 3;
        g.drawRect (drawBounds.getX(), rulerHeight, drawBounds.getRight(), rulerHeight);
        g.setFont (FontOptions (12.0f));

        i32k lightLineWidth = 1;
        i32k heavyLineWidth = 3;

        if (selectedMusicalContext != nullptr)
        {
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (selectedMusicalContext);
            const ARA::TempoConverter<decltype (tempoReader)> tempoConverter (tempoReader);

            // chord ruler: one rect per chord, skipping empty "no chords"
            const auto chordBounds = drawBounds.removeFromTop (rulerHeight);
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeSheetChords> chordsReader (selectedMusicalContext);

            if (tempoReader && chordsReader)
            {
                const ARA::ChordInterpreter interpreter (true);
                for (auto itChord = chordsReader.begin(); itChord != chordsReader.end(); ++itChord)
                {
                    if (interpreter.isNoChord (*itChord))
                        continue;

                    const auto chordStartTime = (itChord == chordsReader.begin()) ? 0 : tempoConverter.getTimeForQuarter (itChord->position);

                    if (chordStartTime >= drawEndTime)
                        break;

                    auto chordRect = chordBounds;
                    chordRect.setLeft (timeToViewScaling.getXForTime (chordStartTime));

                    if (std::next (itChord) != chordsReader.end())
                    {
                        const auto nextChordStartTime = tempoConverter.getTimeForQuarter (std::next (itChord)->position);

                        if (nextChordStartTime < drawStartTime)
                            continue;

                        chordRect.setRight (timeToViewScaling.getXForTime (nextChordStartTime));
                    }

                    g.drawRect (chordRect);
                    g.drawText (convertARAString (interpreter.getNameForChord (*itChord).c_str()),
                                chordRect.withTrimmedLeft (2),
                                Justification::centredLeft);
                }
            }

            // beat ruler: evaluates tempo and bar signatures to draw a line for each beat
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures> barSignaturesReader (selectedMusicalContext);

            if (barSignaturesReader)
            {
                const ARA::BarSignaturesConverter<decltype (barSignaturesReader)> barSignaturesConverter (barSignaturesReader);

                const f64 beatStart = barSignaturesConverter.getBeatForQuarter (tempoConverter.getQuarterForTime (drawStartTime));
                const f64 beatEnd   = barSignaturesConverter.getBeatForQuarter (tempoConverter.getQuarterForTime (drawEndTime));
                i32k endBeat = roundToInt (std::floor (beatEnd));
                RectangleList<i32> rects;

                for (i32 beat = roundToInt (std::ceil (beatStart)); beat <= endBeat; ++beat)
                {
                    const auto quarterPos = barSignaturesConverter.getQuarterForBeat (beat);
                    i32k x = timeToViewScaling.getXForTime (tempoConverter.getTimeForQuarter (quarterPos));
                    const auto barSignature = barSignaturesConverter.getBarSignatureForQuarter (quarterPos);
                    i32k lineWidth = (approximatelyEqual (quarterPos, barSignature.position)) ? heavyLineWidth : lightLineWidth;
                    i32k beatsSinceBarStart = roundToInt (barSignaturesConverter.getBeatDistanceFromBarStartForQuarter (quarterPos));
                    i32k lineHeight = (beatsSinceBarStart == 0) ? rulerHeight : rulerHeight / 2;
                    rects.addWithoutMerging (Rectangle<i32> (x - lineWidth / 2, 2 * rulerHeight - lineHeight, lineWidth, lineHeight));
                }

                g.fillRectList (rects);
            }
        }

        // time ruler: one tick for each second
        {
            RectangleList<i32> rects;

            for (auto time = std::floor (drawStartTime); time <= drawEndTime; time += 1.0)
            {
                i32k lineWidth  = (std::fmod (time, 60.0) <= 0.001) ? heavyLineWidth : lightLineWidth;
                i32k lineHeight = (std::fmod (time, 10.0) <= 0.001) ? rulerHeight : rulerHeight / 2;
                rects.addWithoutMerging (Rectangle<i32> (timeToViewScaling.getXForTime (time) - lineWidth / 2,
                                                         bounds.getHeight() - lineHeight,
                                                         lineWidth,
                                                         lineHeight));
            }

            g.fillRectList (rects);
        }
    }

    z0 mouseDrag (const MouseEvent& m) override
    {
        isDraggingCycle = true;

        auto cycleRect = getBounds();
        cycleRect.setLeft  (jmin (m.getMouseDownX(), m.x));
        cycleRect.setRight (jmax (m.getMouseDownX(), m.x));
        cycleMarker.setBounds (cycleRect);
    }

    z0 mouseUp (const MouseEvent& m) override
    {
        auto playbackController = araDocument.getDocumentController()->getHostPlaybackController();

        if (playbackController != nullptr)
        {
            const auto startTime = timeToViewScaling.getTimeForX (jmin (m.getMouseDownX(), m.x));
            const auto endTime   = timeToViewScaling.getTimeForX (jmax (m.getMouseDownX(), m.x));

            if (playHeadState.isPlaying.load (std::memory_order_relaxed))
                playbackController->requestStopPlayback();
            else
                playbackController->requestSetPlaybackPosition (startTime);

            if (isDraggingCycle)
                playbackController->requestSetCycleRange (startTime, endTime - startTime);
        }

        isDraggingCycle = false;
    }

    z0 mouseDoubleClick (const MouseEvent&) override
    {
        if (auto* playbackController = araDocument.getDocumentController()->getHostPlaybackController())
        {
            if (! playHeadState.isPlaying.load (std::memory_order_relaxed))
                playbackController->requestStartPlayback();
        }
    }

    z0 selectMusicalContext (ARAMusicalContext* newSelectedMusicalContext)
    {
        if (auto* oldSelection = std::exchange (selectedMusicalContext, newSelectedMusicalContext);
            oldSelection != selectedMusicalContext)
        {
            if (oldSelection != nullptr)
                oldSelection->removeListener (this);

            if (selectedMusicalContext != nullptr)
                selectedMusicalContext->addListener (this);

            repaint();
        }
    }

    z0 zoomLevelChanged (f64) override
    {
        repaint();
    }

    z0 doUpdateMusicalContextContent (ARAMusicalContext*, ARAContentUpdateScopes) override
    {
        repaint();
    }

private:
    z0 updateCyclePosition()
    {
        if (selectedMusicalContext != nullptr)
        {
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (selectedMusicalContext);
            const ARA::TempoConverter<decltype (tempoReader)> tempoConverter (tempoReader);

            const auto loopStartTime = tempoConverter.getTimeForQuarter (playHeadState.loopPpqStart.load (std::memory_order_relaxed));
            const auto loopEndTime   = tempoConverter.getTimeForQuarter (playHeadState.loopPpqEnd.load   (std::memory_order_relaxed));

            auto cycleRect = getBounds();
            cycleRect.setLeft  (timeToViewScaling.getXForTime (loopStartTime));
            cycleRect.setRight (timeToViewScaling.getXForTime (loopEndTime));
            cycleMarker.setVisible (true);
            cycleMarker.setBounds (cycleRect);
        }
        else
        {
            cycleMarker.setVisible (false);
        }
    }

    z0 timerCallback() override
    {
        if (! isDraggingCycle)
            updateCyclePosition();
    }

private:
    PlayHeadState& playHeadState;
    TimeToViewScaling& timeToViewScaling;
    ARADocument& araDocument;
    ARAMusicalContext* selectedMusicalContext = nullptr;
    CycleMarkerComponent cycleMarker;
    b8 isDraggingCycle = false;
};

class RulersHeader final : public Component
{
public:
    RulersHeader()
    {
        chordsLabel.setText ("Chords", NotificationType::dontSendNotification);
        addAndMakeVisible (chordsLabel);

        barsLabel.setText ("Bars", NotificationType::dontSendNotification);
        addAndMakeVisible (barsLabel);

        timeLabel.setText ("Time", NotificationType::dontSendNotification);
        addAndMakeVisible (timeLabel);
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();
        const auto rulerHeight = bounds.getHeight() / 3;

        for (auto* label : { &chordsLabel, &barsLabel, &timeLabel })
            label->setBounds (bounds.removeFromTop (rulerHeight));
    }

    z0 paint (Graphics& g) override
    {
        auto bounds = getLocalBounds();
        const auto rulerHeight = bounds.getHeight() / 3;
        g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
        g.fillRect (bounds);
        g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId).contrasting());
        g.drawRect (bounds);
        bounds.removeFromTop (rulerHeight);
        g.drawRect (bounds.removeFromTop (rulerHeight));
    }

private:
    Label chordsLabel, barsLabel, timeLabel;
};

//==============================================================================
struct WaveformCache final : private ARAAudioSource::Listener
{
    WaveformCache() : thumbnailCache (20)
    {
    }

    ~WaveformCache() override
    {
        for (const auto& entry : thumbnails)
        {
            entry.first->removeListener (this);
        }
    }

    //==============================================================================
    z0 willDestroyAudioSource (ARAAudioSource* audioSource) override
    {
        removeAudioSource (audioSource);
    }

    AudioThumbnail& getOrCreateThumbnail (ARAAudioSource* audioSource)
    {
        const auto iter = thumbnails.find (audioSource);

        if (iter != std::end (thumbnails))
            return *iter->second;

        auto thumb = std::make_unique<AudioThumbnail> (128, dummyManager, thumbnailCache);
        auto& result = *thumb;

        ++hash;
        thumb->setReader (new ARAAudioSourceReader (audioSource), hash);

        audioSource->addListener (this);
        thumbnails.emplace (audioSource, std::move (thumb));
        return result;
    }

private:
    z0 removeAudioSource (ARAAudioSource* audioSource)
    {
        audioSource->removeListener (this);
        thumbnails.erase (audioSource);
    }

    z64 hash = 0;
    AudioFormatManager dummyManager;
    AudioThumbnailCache thumbnailCache;
    std::map<ARAAudioSource*, std::unique_ptr<AudioThumbnail>> thumbnails;
};

class PlaybackRegionView final : public Component,
                                 public ChangeListener,
                                 public SettableTooltipClient,
                                 private ARAAudioSource::Listener,
                                 private ARAPlaybackRegion::Listener,
                                 private ARAEditorView::Listener
{
public:
    PlaybackRegionView (ARAEditorView& editorView, ARAPlaybackRegion& region, WaveformCache& cache)
        : araEditorView (editorView), playbackRegion (region), waveformCache (cache), previewRegionOverlay (*this)
    {
        auto* audioSource = playbackRegion.getAudioModification()->getAudioSource();

        waveformCache.getOrCreateThumbnail (audioSource).addChangeListener (this);

        audioSource->addListener (this);
        playbackRegion.addListener (this);
        araEditorView.addListener (this);
        addAndMakeVisible (previewRegionOverlay);

        setTooltip ("Double-click to toggle dim state of the region, click and hold to prelisten region near click.");
    }

    ~PlaybackRegionView() override
    {
        auto* audioSource = playbackRegion.getAudioModification()->getAudioSource();

        audioSource->removeListener (this);
        playbackRegion.removeListener (this);
        araEditorView.removeListener (this);

        waveformCache.getOrCreateThumbnail (audioSource).removeChangeListener (this);
    }

    z0 mouseDown (const MouseEvent& m) override
    {
        const auto relativeTime = (f64) m.getMouseDownX() / getLocalBounds().getWidth();
        const auto previewTime = playbackRegion.getStartInPlaybackTime()
                                 + relativeTime * playbackRegion.getDurationInPlaybackTime();
        auto& previewState = ARADocumentControllerSpecialisation::getSpecialisedDocumentController<ARADemoPluginDocumentControllerSpecialisation> (playbackRegion.getDocumentController())->previewState;
        previewState.previewTime.store (previewTime);
        previewState.previewedRegion.store (&playbackRegion);
        previewRegionOverlay.update();
    }

    z0 mouseUp (const MouseEvent&) override
    {
        auto& previewState = ARADocumentControllerSpecialisation::getSpecialisedDocumentController<ARADemoPluginDocumentControllerSpecialisation> (playbackRegion.getDocumentController())->previewState;
        previewState.previewTime.store (0.0);
        previewState.previewedRegion.store (nullptr);
        previewRegionOverlay.update();
    }

    z0 mouseDoubleClick (const MouseEvent&) override
    {
        // Set the dim flag on our region's audio modification when f64-clicked
        auto audioModification = playbackRegion.getAudioModification<ARADemoPluginAudioModification>();
        audioModification->setDimmed (! audioModification->isDimmed());

        // Send a content change notification for the modification and all associated playback regions
        audioModification->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected(), true);
        for (auto region : audioModification->getPlaybackRegions())
            region->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected(), true);
    }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        repaint();
    }

    z0 didEnableAudioSourceSamplesAccess (ARAAudioSource*, b8) override
    {
        repaint();
    }

    z0 willUpdatePlaybackRegionProperties (ARAPlaybackRegion*,
                                             ARAPlaybackRegion::PropertiesPtr newProperties) override
    {
        if (playbackRegion.getName() != newProperties->name
            || playbackRegion.getColor() != newProperties->color)
        {
            repaint();
        }
    }

    z0 didUpdatePlaybackRegionContent (ARAPlaybackRegion*, ARAContentUpdateScopes) override
    {
        repaint();
    }

    z0 onNewSelection (const ARAViewSelection& viewSelection) override
    {
        const auto& selectedPlaybackRegions = viewSelection.getPlaybackRegions();
        const b8 selected = std::find (selectedPlaybackRegions.begin(), selectedPlaybackRegions.end(), &playbackRegion) != selectedPlaybackRegions.end();
        if (selected != isSelected)
        {
            isSelected = selected;
            repaint();
        }
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (convertOptionalARAColor (playbackRegion.getEffectiveColor(), Colors::black));

        const auto* audioModification = playbackRegion.getAudioModification<ARADemoPluginAudioModification>();
        g.setColor (audioModification->isDimmed() ? Colors::darkgrey.darker() : Colors::darkgrey.brighter());

        if (audioModification->getAudioSource()->isSampleAccessEnabled())
        {
            auto& thumbnail = waveformCache.getOrCreateThumbnail (playbackRegion.getAudioModification()->getAudioSource());
            thumbnail.drawChannels (g,
                                    getLocalBounds(),
                                    playbackRegion.getStartInAudioModificationTime(),
                                    playbackRegion.getEndInAudioModificationTime(),
                                    1.0f);
        }
        else
        {
            g.setFont (FontOptions (12.0f));
            g.drawText ("Audio Access Disabled", getLocalBounds(), Justification::centred);
        }

        g.setColor (Colors::white.withMultipliedAlpha (0.9f));
        g.setFont (FontOptions (12.0f));
        g.drawText (convertOptionalARAString (playbackRegion.getEffectiveName()),
                    getLocalBounds(),
                    Justification::topLeft);

        if (audioModification->isDimmed())
            g.drawText ("DIMMED", getLocalBounds(), Justification::bottomLeft);

        g.setColor (isSelected ? Colors::white : Colors::black);
        g.drawRect (getLocalBounds());
    }

    z0 resized() override
    {
        repaint();
    }

private:
    class PreviewRegionOverlay final : public Component
    {
        static constexpr auto previewLength = 0.5;

    public:
        PreviewRegionOverlay (PlaybackRegionView& ownerIn) : owner (ownerIn)
        {
        }

        z0 update()
        {
            const auto& previewState = owner.getDocumentController()->previewState;

            if (previewState.previewedRegion.load() == &owner.playbackRegion)
            {
                const auto previewStartTime = previewState.previewTime.load() - owner.playbackRegion.getStartInPlaybackTime();
                const auto pixelPerSecond = owner.getWidth() / owner.playbackRegion.getDurationInPlaybackTime();

                setBounds (roundToInt ((previewStartTime - previewLength / 2) * pixelPerSecond),
                           0,
                           roundToInt (previewLength * pixelPerSecond),
                           owner.getHeight());

                setVisible (true);
            }
            else
            {
                setVisible (false);
            }

            repaint();
        }

        z0 paint (Graphics& g) override
        {
            g.setColor (Colors::yellow.withAlpha (0.5f));
            g.fillRect (getLocalBounds());
        }

    private:
        PlaybackRegionView& owner;
    };

    ARADemoPluginDocumentControllerSpecialisation* getDocumentController() const
    {
        return ARADocumentControllerSpecialisation::getSpecialisedDocumentController<ARADemoPluginDocumentControllerSpecialisation> (playbackRegion.getDocumentController());
    }

    ARAEditorView& araEditorView;
    ARAPlaybackRegion& playbackRegion;
    WaveformCache& waveformCache;
    PreviewRegionOverlay previewRegionOverlay;
    b8 isSelected = false;
};

class RegionSequenceView final : public Component,
                                 public ChangeBroadcaster,
                                 private TimeToViewScaling::Listener,
                                 private ARARegionSequence::Listener,
                                 private ARAPlaybackRegion::Listener
{
public:
    RegionSequenceView (ARAEditorView& editorView, TimeToViewScaling& scaling, ARARegionSequence& rs, WaveformCache& cache)
        : araEditorView (editorView), timeToViewScaling (scaling), regionSequence (rs), waveformCache (cache)
    {
        regionSequence.addListener (this);

        for (auto* playbackRegion : regionSequence.getPlaybackRegions())
            createAndAddPlaybackRegionView (playbackRegion);

        updatePlaybackDuration();

        timeToViewScaling.addListener (this);
    }

    ~RegionSequenceView() override
    {
        timeToViewScaling.removeListener (this);

        regionSequence.removeListener (this);

        for (const auto& it : playbackRegionViews)
            it.first->removeListener (this);
    }

    //==============================================================================
    // ARA Document change callback overrides
    z0 willUpdateRegionSequenceProperties (ARARegionSequence*,
                                             ARARegionSequence::PropertiesPtr newProperties) override
    {
        if (regionSequence.getColor() != newProperties->color)
        {
            for (auto& pbr : playbackRegionViews)
                pbr.second->repaint();
        }
    }

    z0 willRemovePlaybackRegionFromRegionSequence (ARARegionSequence*,
                                                     ARAPlaybackRegion* playbackRegion) override
    {
        playbackRegion->removeListener (this);
        removeChildComponent (playbackRegionViews[playbackRegion].get());
        playbackRegionViews.erase (playbackRegion);
        updatePlaybackDuration();
    }

    z0 didAddPlaybackRegionToRegionSequence (ARARegionSequence*, ARAPlaybackRegion* playbackRegion) override
    {
        createAndAddPlaybackRegionView (playbackRegion);
        updatePlaybackDuration();
    }

    z0 willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) override
    {
        playbackRegion->removeListener (this);
        removeChildComponent (playbackRegionViews[playbackRegion].get());
        playbackRegionViews.erase (playbackRegion);
        updatePlaybackDuration();
    }

    z0 didUpdatePlaybackRegionProperties (ARAPlaybackRegion*) override
    {
        updatePlaybackDuration();
    }

    z0 zoomLevelChanged (f64) override
    {
        resized();
    }

    z0 resized() override
    {
        for (auto& pbr : playbackRegionViews)
        {
            const auto playbackRegion = pbr.first;
            pbr.second->setBounds (
                getLocalBounds()
                    .withTrimmedLeft (timeToViewScaling.getXForTime (playbackRegion->getStartInPlaybackTime()))
                    .withWidth (timeToViewScaling.getXForTime (playbackRegion->getDurationInPlaybackTime())));
        }
    }

    auto getPlaybackDuration() const noexcept
    {
        return playbackDuration;
    }

private:
    z0 createAndAddPlaybackRegionView (ARAPlaybackRegion* playbackRegion)
    {
        playbackRegionViews[playbackRegion] = std::make_unique<PlaybackRegionView> (araEditorView,
                                                                                    *playbackRegion,
                                                                                    waveformCache);
        playbackRegion->addListener (this);
        addAndMakeVisible (*playbackRegionViews[playbackRegion]);
    }

    z0 updatePlaybackDuration()
    {
        const auto iter = std::max_element (
            playbackRegionViews.begin(),
            playbackRegionViews.end(),
            [] (const auto& a, const auto& b) { return a.first->getEndInPlaybackTime() < b.first->getEndInPlaybackTime(); });

        playbackDuration = iter != playbackRegionViews.end() ? iter->first->getEndInPlaybackTime()
                                                             : 0.0;

        sendChangeMessage();
    }

    ARAEditorView& araEditorView;
    TimeToViewScaling& timeToViewScaling;
    ARARegionSequence& regionSequence;
    WaveformCache& waveformCache;
    std::unordered_map<ARAPlaybackRegion*, std::unique_ptr<PlaybackRegionView>> playbackRegionViews;
    f64 playbackDuration = 0.0;
};

class ZoomControls final : public Component
{
public:
    ZoomControls()
    {
        addAndMakeVisible (zoomInButton);
        addAndMakeVisible (zoomOutButton);
    }

    z0 setZoomInCallback  (std::function<z0()> cb)   { zoomInButton.onClick  = std::move (cb); }
    z0 setZoomOutCallback (std::function<z0()> cb)   { zoomOutButton.onClick = std::move (cb); }

    z0 resized() override
    {
        FlexBox fb;
        fb.justifyContent = FlexBox::JustifyContent::flexEnd;

        for (auto* button : { &zoomInButton, &zoomOutButton })
            fb.items.add (FlexItem (*button).withMinHeight (30.0f).withMinWidth (30.0f).withMargin ({ 5, 5, 5, 0 }));

        fb.performLayout (getLocalBounds());
    }

private:
    TextButton zoomInButton { "+" }, zoomOutButton { "-" };
};

class PlayheadPositionLabel final : public Label,
                                    private Timer
{
public:
    PlayheadPositionLabel (PlayHeadState& playHeadStateIn)
        : playHeadState (playHeadStateIn)
    {
        startTimerHz (30);
    }

    ~PlayheadPositionLabel() override
    {
        stopTimer();
    }

    z0 selectMusicalContext (ARAMusicalContext* newSelectedMusicalContext)
    {
        selectedMusicalContext = newSelectedMusicalContext;
    }

private:
    z0 timerCallback() override
    {
        const auto timePosition = playHeadState.timeInSeconds.load (std::memory_order_relaxed);

        auto text = timeToTimecodeString (timePosition);

        if (playHeadState.isPlaying.load (std::memory_order_relaxed))
            text += " (playing)";
        else
            text += " (stopped)";

        if (selectedMusicalContext != nullptr)
        {
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (selectedMusicalContext);
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures> barSignaturesReader (selectedMusicalContext);

            if (tempoReader && barSignaturesReader)
            {
                const ARA::TempoConverter<decltype (tempoReader)> tempoConverter (tempoReader);
                const ARA::BarSignaturesConverter<decltype (barSignaturesReader)> barSignaturesConverter (barSignaturesReader);
                const auto quarterPosition = tempoConverter.getQuarterForTime (timePosition);
                const auto barIndex = barSignaturesConverter.getBarIndexForQuarter (quarterPosition);
                const auto beatDistance = barSignaturesConverter.getBeatDistanceFromBarStartForQuarter (quarterPosition);
                const auto quartersPerBeat = 4.0 / (f64) barSignaturesConverter.getBarSignatureForQuarter (quarterPosition).denominator;
                const auto beatIndex = (i32) beatDistance;
                const auto tickIndex = drx::roundToInt ((beatDistance - beatIndex) * quartersPerBeat * 960.0);

                text += newLine;
                text += Txt::formatted ("bar %d | beat %d | tick %03d", (barIndex >= 0) ? barIndex + 1 : barIndex, beatIndex + 1, tickIndex + 1);
                text += "  -  ";

                const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeSheetChords> chordsReader (selectedMusicalContext);

                if (chordsReader && chordsReader.getEventCount() > 0)
                {
                    const auto begin = chordsReader.begin();
                    const auto end = chordsReader.end();
                    auto it = begin;

                    while (it != end && it->position <= quarterPosition)
                        ++it;

                    if (it != begin)
                        --it;

                    const ARA::ChordInterpreter interpreter (true);
                    text += "chord ";
                    text += Txt (interpreter.getNameForChord (*it));
                }
                else
                {
                    text += "(no chords provided)";
                }
            }
        }

        setText (text, NotificationType::dontSendNotification);
    }

    // Copied from AudioPluginDemo.h: quick-and-dirty function to format a timecode string
    static Txt timeToTimecodeString (f64 seconds)
    {
        auto millisecs = roundToInt (seconds * 1000.0);
        auto absMillisecs = std::abs (millisecs);

        return Txt::formatted ("%02d:%02d:%02d.%03d",
                                  millisecs / 3600000,
                                  (absMillisecs / 60000) % 60,
                                  (absMillisecs / 1000)  % 60,
                                  absMillisecs % 1000);
    }

    PlayHeadState& playHeadState;
    ARAMusicalContext* selectedMusicalContext = nullptr;
};

class TrackHeader final : public Component,
                          private ARARegionSequence::Listener,
                          private ARAEditorView::Listener
{
public:
    TrackHeader (ARAEditorView& editorView, ARARegionSequence& regionSequenceIn)
        : araEditorView (editorView), regionSequence (regionSequenceIn)
    {
        updateTrackName (regionSequence.getName());
        onNewSelection (araEditorView.getViewSelection());

        addAndMakeVisible (trackNameLabel);

        regionSequence.addListener (this);
        araEditorView.addListener (this);
    }

    ~TrackHeader() override
    {
        araEditorView.removeListener (this);
        regionSequence.removeListener (this);
    }

    z0 willUpdateRegionSequenceProperties (ARARegionSequence*, ARARegionSequence::PropertiesPtr newProperties) override
    {
        if (regionSequence.getName() != newProperties->name)
            updateTrackName (newProperties->name);
        if (regionSequence.getColor() != newProperties->color)
            repaint();
    }

    z0 resized() override
    {
        trackNameLabel.setBounds (getLocalBounds().reduced (2));
    }

    z0 paint (Graphics& g) override
    {
        const auto backgroundColor = getLookAndFeel().findColor (ResizableWindow::backgroundColorId);
        g.setColor (isSelected ? backgroundColor.brighter() : backgroundColor);
        g.fillRoundedRectangle (getLocalBounds().reduced (2).toFloat(), 6.0f);
        g.setColor (backgroundColor.contrasting());
        g.drawRoundedRectangle (getLocalBounds().reduced (2).toFloat(), 6.0f, 1.0f);

        if (auto colour = regionSequence.getColor())
        {
            g.setColor (convertARAColor (colour));
            g.fillRect (getLocalBounds().removeFromTop (16).reduced (6));
            g.fillRect (getLocalBounds().removeFromBottom (16).reduced (6));
        }
    }

    z0 onNewSelection (const ARAViewSelection& viewSelection) override
    {
        const auto& selectedRegionSequences = viewSelection.getRegionSequences();
        const b8 selected = std::find (selectedRegionSequences.begin(), selectedRegionSequences.end(), &regionSequence) != selectedRegionSequences.end();

        if (selected != isSelected)
        {
            isSelected = selected;
            repaint();
        }
    }

private:
    z0 updateTrackName (ARA::ARAUtf8String optionalName)
    {
        trackNameLabel.setText (optionalName ? optionalName : "No track name",
                                NotificationType::dontSendNotification);
    }

    ARAEditorView& araEditorView;
    ARARegionSequence& regionSequence;
    Label trackNameLabel;
    b8 isSelected = false;
};

constexpr auto trackHeight = 60;

class VerticalLayoutViewportContent final : public Component
{
public:
    z0 resized() override
    {
        auto bounds = getLocalBounds();

        for (auto* component : getChildren())
        {
            component->setBounds (bounds.removeFromTop (trackHeight));
            component->resized();
        }
    }
};

class VerticalLayoutViewport final : public Viewport
{
public:
    VerticalLayoutViewport()
    {
        setViewedComponent (&content, false);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId).brighter());
    }

    std::function<z0 (Rectangle<i32>)> onVisibleAreaChanged;

    VerticalLayoutViewportContent content;

private:
    z0 visibleAreaChanged (const Rectangle<i32>& newVisibleArea) override
    {
        NullCheckedInvocation::invoke (onVisibleAreaChanged, newVisibleArea);
    }
};

class OverlayComponent final : public Component,
                               private Timer,
                               private TimeToViewScaling::Listener
{
public:
    class PlayheadMarkerComponent final : public Component
    {
        z0 paint (Graphics& g) override { g.fillAll (Colors::yellow.darker (0.2f)); }
    };

    OverlayComponent (PlayHeadState& playHeadStateIn, TimeToViewScaling& timeToViewScalingIn)
        : playHeadState (playHeadStateIn), timeToViewScaling (timeToViewScalingIn)
    {
        addChildComponent (playheadMarker);
        setInterceptsMouseClicks (false, false);
        startTimerHz (30);

        timeToViewScaling.addListener (this);
    }

    ~OverlayComponent() override
    {
        timeToViewScaling.removeListener (this);

        stopTimer();
    }

    z0 resized() override
    {
        updatePlayHeadPosition();
    }

    z0 setHorizontalOffset (i32 offset)
    {
        horizontalOffset = offset;
    }

    z0 setSelectedTimeRange (std::optional<ARA::ARAContentTimeRange> timeRange)
    {
        selectedTimeRange = timeRange;
        repaint();
    }

    z0 zoomLevelChanged (f64) override
    {
        updatePlayHeadPosition();
        repaint();
    }

    z0 paint (Graphics& g) override
    {
        if (selectedTimeRange)
        {
            auto bounds = getLocalBounds();
            bounds.setLeft (timeToViewScaling.getXForTime (selectedTimeRange->start));
            bounds.setRight (timeToViewScaling.getXForTime (selectedTimeRange->start + selectedTimeRange->duration));
            g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId).brighter().withAlpha (0.3f));
            g.fillRect (bounds);
            g.setColor (Colors::whitesmoke.withAlpha (0.5f));
            g.drawRect (bounds);
        }
    }

private:
    z0 updatePlayHeadPosition()
    {
        if (playHeadState.isPlaying.load (std::memory_order_relaxed))
        {
            const auto markerX = timeToViewScaling.getXForTime (playHeadState.timeInSeconds.load (std::memory_order_relaxed));
            const auto playheadLine = getLocalBounds().withTrimmedLeft ((i32) (markerX - markerWidth / 2.0) - horizontalOffset)
                                                      .removeFromLeft ((i32) markerWidth);
            playheadMarker.setVisible (true);
            playheadMarker.setBounds (playheadLine);
        }
        else
        {
            playheadMarker.setVisible (false);
        }
    }

    z0 timerCallback() override
    {
        updatePlayHeadPosition();
    }

    static constexpr f64 markerWidth = 2.0;

    PlayHeadState& playHeadState;
    TimeToViewScaling& timeToViewScaling;
    i32 horizontalOffset = 0;
    std::optional<ARA::ARAContentTimeRange> selectedTimeRange;
    PlayheadMarkerComponent playheadMarker;
};

class DocumentView final : public Component,
                           public ChangeListener,
                           public ARAMusicalContext::Listener,
                           private ARADocument::Listener,
                           private ARAEditorView::Listener
{
public:
    DocumentView (ARAEditorView& editorView, PlayHeadState& playHeadState)
        : araEditorView (editorView),
          araDocument (*editorView.getDocumentController()->getDocument<ARADocument>()),
          rulersView (playHeadState, timeToViewScaling, araDocument),
          overlay (playHeadState, timeToViewScaling),
          playheadPositionLabel (playHeadState)
    {
        if (araDocument.getMusicalContexts().size() > 0)
            selectMusicalContext (araDocument.getMusicalContexts().front());

        addAndMakeVisible (rulersHeader);

        viewport.content.addAndMakeVisible (rulersView);

        viewport.onVisibleAreaChanged = [this] (const auto& r)
        {
            viewportHeightOffset = r.getY();
            overlay.setHorizontalOffset (r.getX());
            resized();
        };

        addAndMakeVisible (viewport);
        addAndMakeVisible (overlay);
        addAndMakeVisible (playheadPositionLabel);

        zoomControls.setZoomInCallback  ([this] { zoom (2.0); });
        zoomControls.setZoomOutCallback ([this] { zoom (0.5); });
        addAndMakeVisible (zoomControls);

        invalidateRegionSequenceViews();

        araDocument.addListener (this);
        araEditorView.addListener (this);
    }

    ~DocumentView() override
    {
        araEditorView.removeListener (this);
        araDocument.removeListener (this);
        selectMusicalContext (nullptr);
    }

    //==============================================================================
    // ARADocument::Listener overrides
    z0 didAddMusicalContextToDocument (ARADocument*, ARAMusicalContext* musicalContext) override
    {
        if (selectedMusicalContext == nullptr)
            selectMusicalContext (musicalContext);
    }

    z0 willDestroyMusicalContext (ARAMusicalContext* musicalContext) override
    {
        if (selectedMusicalContext == musicalContext)
            selectMusicalContext (nullptr);
    }

    z0 didReorderRegionSequencesInDocument (ARADocument*) override
    {
        invalidateRegionSequenceViews();
    }

    z0 didAddRegionSequenceToDocument (ARADocument*, ARARegionSequence*) override
    {
        invalidateRegionSequenceViews();
    }

    z0 willRemoveRegionSequenceFromDocument (ARADocument*, ARARegionSequence* regionSequence) override
    {
        removeRegionSequenceView (regionSequence);
    }

    z0 didEndEditing (ARADocument*) override
    {
        rebuildRegionSequenceViews();
        update();
    }

    //==============================================================================
    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        update();
    }

    //==============================================================================
    // ARAEditorView::Listener overrides
    z0 onNewSelection (const ARAViewSelection& viewSelection) override
    {
        auto getNewSelectedMusicalContext = [&viewSelection]() -> ARAMusicalContext*
        {
            if (! viewSelection.getRegionSequences().empty())
                return viewSelection.getRegionSequences<ARARegionSequence>().front()->getMusicalContext();
            else if (! viewSelection.getPlaybackRegions().empty())
                return viewSelection.getPlaybackRegions<ARAPlaybackRegion>().front()->getRegionSequence()->getMusicalContext();

            return nullptr;
        };

        if (auto* newSelectedMusicalContext = getNewSelectedMusicalContext())
            if (newSelectedMusicalContext != selectedMusicalContext)
                selectMusicalContext (newSelectedMusicalContext);

        if (const auto timeRange = viewSelection.getTimeRange())
            overlay.setSelectedTimeRange (*timeRange);
        else
            overlay.setSelectedTimeRange (std::nullopt);
    }

    z0 onHideRegionSequences (const std::vector<ARARegionSequence*>& regionSequences) override
    {
        hiddenRegionSequences = regionSequences;
        invalidateRegionSequenceViews();
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId).darker());
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();

        FlexBox fb;
        fb.justifyContent = FlexBox::JustifyContent::spaceBetween;
        fb.items.add (FlexItem (playheadPositionLabel).withWidth (450.0f).withMinWidth (250.0f));
        fb.items.add (FlexItem (zoomControls).withMinWidth (80.0f));
        fb.performLayout (bounds.removeFromBottom (40));

        auto headerBounds = bounds.removeFromLeft (headerWidth);
        rulersHeader.setBounds (headerBounds.removeFromTop (trackHeight));
        layOutVertically (headerBounds, trackHeaders, viewportHeightOffset);

        viewport.setBounds (bounds);
        overlay.setBounds (bounds.reduced (1));

        const auto width = jmax (timeToViewScaling.getXForTime (timelineLength), viewport.getWidth());
        const auto height = (i32) (regionSequenceViews.size() + 1) * trackHeight;
        viewport.content.setSize (width, height);
        viewport.content.resized();
    }

    //==============================================================================
    static constexpr i32 headerWidth = 120;

private:
    struct RegionSequenceViewKey
    {
        explicit RegionSequenceViewKey (ARARegionSequence* regionSequence)
            : orderIndex (regionSequence->getOrderIndex()), sequence (regionSequence)
        {
        }

        b8 operator< (const RegionSequenceViewKey& other) const
        {
            return std::tie (orderIndex, sequence) < std::tie (other.orderIndex, other.sequence);
        }

        ARA::ARAInt32 orderIndex;
        ARARegionSequence* sequence;
    };

    z0 selectMusicalContext (ARAMusicalContext* newSelectedMusicalContext)
    {
        if (auto oldContext = std::exchange (selectedMusicalContext, newSelectedMusicalContext);
            oldContext != selectedMusicalContext)
        {
            if (oldContext != nullptr)
                oldContext->removeListener (this);

            if (selectedMusicalContext != nullptr)
                selectedMusicalContext->addListener (this);

            rulersView.selectMusicalContext (selectedMusicalContext);
            playheadPositionLabel.selectMusicalContext (selectedMusicalContext);
        }
    }

    z0 zoom (f64 factor)
    {
        timeToViewScaling.zoom (factor);
        update();
    }

    template <typename T>
    z0 layOutVertically (Rectangle<i32> bounds, T& components, i32 verticalOffset = 0)
    {
        bounds = bounds.withY (bounds.getY() - verticalOffset).withHeight (bounds.getHeight() + verticalOffset);

        for (auto& component : components)
        {
            component.second->setBounds (bounds.removeFromTop (trackHeight));
            component.second->resized();
        }
    }

    z0 update()
    {
        timelineLength = 0.0;

        for (const auto& view : regionSequenceViews)
            timelineLength = std::max (timelineLength, view.second->getPlaybackDuration());

        resized();
    }

    z0 addTrackViews (ARARegionSequence* regionSequence)
    {
        const auto insertIntoMap = [] (auto& map, auto key, auto value) -> auto&
        {
            auto it = map.insert ({ std::move (key), std::move (value) });
            return *(it.first->second);
        };

        auto& regionSequenceView = insertIntoMap (
            regionSequenceViews,
            RegionSequenceViewKey { regionSequence },
            std::make_unique<RegionSequenceView> (araEditorView, timeToViewScaling, *regionSequence, waveformCache));

        regionSequenceView.addChangeListener (this);
        viewport.content.addAndMakeVisible (regionSequenceView);

        auto& trackHeader = insertIntoMap (trackHeaders,
                                           RegionSequenceViewKey { regionSequence },
                                           std::make_unique<TrackHeader> (araEditorView, *regionSequence));

        addAndMakeVisible (trackHeader);
    }

    z0 removeRegionSequenceView (ARARegionSequence* regionSequence)
    {
        const auto& view = regionSequenceViews.find (RegionSequenceViewKey { regionSequence });

        if (view != regionSequenceViews.cend())
        {
            removeChildComponent (view->second.get());
            regionSequenceViews.erase (view);
        }

        invalidateRegionSequenceViews();
    }

    z0 invalidateRegionSequenceViews()
    {
        regionSequenceViewsAreValid = false;
        rebuildRegionSequenceViews();
    }

    z0 rebuildRegionSequenceViews()
    {
        if (! regionSequenceViewsAreValid && ! araDocument.getDocumentController()->isHostEditingDocument())
        {
            for (auto& view : regionSequenceViews)
                removeChildComponent (view.second.get());

            regionSequenceViews.clear();

            for (auto& view : trackHeaders)
                removeChildComponent (view.second.get());

            trackHeaders.clear();

            for (auto* regionSequence : araDocument.getRegionSequences())
                if (std::find (hiddenRegionSequences.begin(), hiddenRegionSequences.end(), regionSequence) == hiddenRegionSequences.end())
                    addTrackViews (regionSequence);

            update();

            regionSequenceViewsAreValid = true;
        }
    }

    ARAEditorView& araEditorView;
    ARADocument& araDocument;

    b8 regionSequenceViewsAreValid = false;

    TimeToViewScaling timeToViewScaling;
    f64 timelineLength = 0.0;

    ARAMusicalContext* selectedMusicalContext = nullptr;

    std::vector<ARARegionSequence*> hiddenRegionSequences;

    WaveformCache waveformCache;
    std::map<RegionSequenceViewKey, std::unique_ptr<TrackHeader>> trackHeaders;
    std::map<RegionSequenceViewKey, std::unique_ptr<RegionSequenceView>> regionSequenceViews;
    RulersHeader rulersHeader;
    RulersView rulersView;
    VerticalLayoutViewport viewport;
    OverlayComponent overlay;
    ZoomControls zoomControls;
    PlayheadPositionLabel playheadPositionLabel;
    TooltipWindow tooltip;

    i32 viewportHeightOffset = 0;
};


class ARADemoPluginProcessorEditor final : public AudioProcessorEditor,
                                           public AudioProcessorEditorARAExtension
{
public:
    explicit ARADemoPluginProcessorEditor (ARADemoPluginAudioProcessorImpl& p)
        : AudioProcessorEditor (&p),
          AudioProcessorEditorARAExtension (&p)
    {
        if (auto* editorView = getARAEditorView())
            documentView = std::make_unique<DocumentView> (*editorView, p.playHeadState);

        addAndMakeVisible (documentView.get());

        // ARA requires that plugin editors are resizable to support tight integration
        // into the host UI
        setResizable (true, false);
        setSize (800, 300);
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));

        if (! isARAEditorView())
        {
            g.setColor (Colors::white);
            g.setFont (FontOptions (15.0f));
            g.drawFittedText ("ARA host isn't detected. This plugin only supports ARA mode",
                              getLocalBounds(),
                              Justification::centred,
                              1);
        }
    }

    z0 resized() override
    {
        if (documentView != nullptr)
            documentView->setBounds (getLocalBounds());
    }

private:
    std::unique_ptr<Component> documentView;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADemoPluginProcessorEditor)
};

class ARADemoPluginAudioProcessor final : public ARADemoPluginAudioProcessorImpl
{
public:
    b8 hasEditor() const override               { return true; }
    AudioProcessorEditor* createEditor() override { return new ARADemoPluginProcessorEditor (*this); }
};
