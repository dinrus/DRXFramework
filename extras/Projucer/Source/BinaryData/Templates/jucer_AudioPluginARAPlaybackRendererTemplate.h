/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA playback renderer implementation.

  ==============================================================================
*/

#pragma once

#include <drx_audio_processors/drx_audio_processors.h>

//==============================================================================
/**
*/
class %%araplaybackrenderer_class_name%%  : public drx::ARAPlaybackRenderer
{
public:
    //==============================================================================
    using drx::ARAPlaybackRenderer::ARAPlaybackRenderer;

    //==============================================================================
    z0 prepareToPlay (f64 sampleRate,
                        i32 maximumSamplesPerBlock,
                        i32 numChannels,
                        drx::AudioProcessor::ProcessingPrecision,
                        AlwaysNonRealtime alwaysNonRealtime) override;
    z0 releaseResources() override;

    //==============================================================================
    b8 processBlock (drx::AudioBuffer<f32>& buffer,
                       drx::AudioProcessor::Realtime realtime,
                       const drx::AudioPlayHead::PositionInfo& positionInfo) noexcept override;

private:
    //==============================================================================
    f64 sampleRate = 44100.0;
    i32 maximumSamplesPerBlock = 4096;
    i32 numChannels = 1;
    b8 useBufferedAudioSourceReader = true;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%araplaybackrenderer_class_name%%)
};
