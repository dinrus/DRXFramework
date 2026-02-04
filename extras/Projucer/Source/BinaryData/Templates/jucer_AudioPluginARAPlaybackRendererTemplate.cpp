/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA playback renderer implementation.

  ==============================================================================
*/

%%araplaybackrenderer_headers%%

//==============================================================================
z0 %%araplaybackrenderer_class_name%%::prepareToPlay (f64 sampleRateIn, i32 maximumSamplesPerBlockIn, i32 numChannelsIn, drx::AudioProcessor::ProcessingPrecision, AlwaysNonRealtime alwaysNonRealtime)
{
    numChannels = numChannelsIn;
    sampleRate = sampleRateIn;
    maximumSamplesPerBlock = maximumSamplesPerBlockIn;
    useBufferedAudioSourceReader = alwaysNonRealtime == AlwaysNonRealtime::no;
}

z0 %%araplaybackrenderer_class_name%%::releaseResources()
{
}

//==============================================================================
b8 %%araplaybackrenderer_class_name%%::processBlock (drx::AudioBuffer<f32>& buffer,
                                                       drx::AudioProcessor::Realtime realtime,
                                                       const drx::AudioPlayHead::PositionInfo& positionInfo) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    jassert (numSamples <= maximumSamplesPerBlock);
    jassert (numChannels == buffer.getNumChannels());
    jassert (realtime == drx::AudioProcessor::Realtime::no || useBufferedAudioSourceReader);
    const auto timeInSamples = positionInfo.getTimeInSamples().orFallback (0);
    const auto isPlaying = positionInfo.getIsPlaying();

    b8 success = true;
    b8 didRenderAnyRegion = false;

    if (isPlaying)
    {
        const auto blockRange = drx::Range<drx::z64>::withStartAndLength (timeInSamples, numSamples);

        for (const auto& playbackRegion : getPlaybackRegions())
        {
            // Evaluate region borders in song time, calculate sample range to render in song time.
            // Note that this example does not use head- or tailtime, so the includeHeadAndTail
            // parameter is set to false here - this might need to be adjusted in actual plug-ins.
            const auto playbackSampleRange = playbackRegion->getSampleRange (sampleRate,
                                                                             drx::ARAPlaybackRegion::IncludeHeadAndTail::no);
            auto renderRange = blockRange.getIntersectionWith (playbackSampleRange);

            if (renderRange.isEmpty())
                continue;

            // Evaluate region borders in modification/source time and calculate offset between
            // song and source samples, then clip song samples accordingly
            // (if an actual plug-in supports time stretching, this must be taken into account here).
            drx::Range<drx::z64> modificationSampleRange { playbackRegion->getStartInAudioModificationSamples(),
                                                               playbackRegion->getEndInAudioModificationSamples() };
            const auto modificationSampleOffset = modificationSampleRange.getStart() - playbackSampleRange.getStart();

            renderRange = renderRange.getIntersectionWith (modificationSampleRange.movedToStartAt (playbackSampleRange.getStart()));

            if (renderRange.isEmpty())
                continue;

            // Now calculate the samples in renderRange for this PlaybackRegion based on the ARA model
            // graph. If didRenderAnyRegion is true, add the region's output samples in renderRange to
            // the buffer. Otherwise the buffer needs to be initialised so the sample value must be
            // overwritten.
            i32k numSamplesToRead = (i32) renderRange.getLength();
            i32k startInBuffer = (i32) (renderRange.getStart() - blockRange.getStart());
            const auto startInSource = renderRange.getStart() + modificationSampleOffset;

            for (i32 c = 0; c < numChannels; ++c)
            {
                auto* channelData = buffer.getWritePointer (c);

                for (i32 i = 0; i < numSamplesToRead; ++i)
                {
                    // Calculate region output sample at index startInSource + i ...
                    f32 sample = 0.0f;

                    if (didRenderAnyRegion)
                        channelData[startInBuffer + i] += sample;
                    else
                        channelData[startInBuffer + i] = sample;
                }
            }

            // If rendering first region, clear any excess at start or end of the region.
            if (! didRenderAnyRegion)
            {
                if (startInBuffer != 0)
                    buffer.clear (0, startInBuffer);

                i32k endInBuffer = startInBuffer + numSamples;
                i32k remainingSamples = numSamples - endInBuffer;

                if (remainingSamples != 0)
                    buffer.clear (endInBuffer, remainingSamples);

                didRenderAnyRegion = true;
            }
        }
    }

    if (! didRenderAnyRegion)
        buffer.clear();

    return success;
}
