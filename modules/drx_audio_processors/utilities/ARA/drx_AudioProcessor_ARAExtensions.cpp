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

#include "drx_AudioProcessor_ARAExtensions.h"

namespace drx
{

//==============================================================================
b8 AudioProcessorARAExtension::getTailLengthSecondsForARA (f64& tailLength) const
{
    if (! isBoundToARA())
        return false;

    tailLength = 0.0;

    if (auto playbackRenderer = getPlaybackRenderer())
        for (const auto& playbackRegion : playbackRenderer->getPlaybackRegions())
            tailLength = jmax (tailLength, playbackRegion->getTailTime());

    return true;
}

b8 AudioProcessorARAExtension::prepareToPlayForARA (f64 sampleRate,
                                                      i32 samplesPerBlock,
                                                      i32 numChannels,
                                                      AudioProcessor::ProcessingPrecision precision)
{
#if ARA_VALIDATE_API_CALLS
    isPrepared = true;
#endif

    if (! isBoundToARA())
        return false;

    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->prepareToPlay (sampleRate, samplesPerBlock, numChannels, precision);

    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->prepareToPlay (sampleRate, samplesPerBlock, numChannels, precision);

    return true;
}

b8 AudioProcessorARAExtension::releaseResourcesForARA()
{
#if ARA_VALIDATE_API_CALLS
    isPrepared = false;
#endif

    if (! isBoundToARA())
        return false;

    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->releaseResources();

    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->releaseResources();

    return true;
}

b8 AudioProcessorARAExtension::processBlockForARA (AudioBuffer<f32>& buffer,
                                                     AudioProcessor::Realtime realtime,
                                                     const AudioPlayHead::PositionInfo& positionInfo)
{
    // validate that the host has prepared us before processing
    ARA_VALIDATE_API_STATE (isPrepared);

    if (! isBoundToARA())
        return false;

    // Render our ARA playback regions for this buffer.
    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->processBlock (buffer, realtime, positionInfo);

    // Render our ARA editor regions and sequences for this buffer.
    // This example does not support editor rendering and thus uses the default implementation,
    // which is a no-op and could be omitted in actual plug-ins to optimize performance.
    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->processBlock (buffer, realtime, positionInfo);

    return true;
}

b8 AudioProcessorARAExtension::processBlockForARA (AudioBuffer<f32>& buffer,
                                                     drx::AudioProcessor::Realtime realtime,
                                                     AudioPlayHead* playhead)
{
    return processBlockForARA (buffer,
                               realtime,
                               playhead != nullptr ? playhead->getPosition().orFallback (AudioPlayHead::PositionInfo{})
                                                   : AudioPlayHead::PositionInfo{});
}

//==============================================================================
z0 AudioProcessorARAExtension::didBindToARA() noexcept
{
    // validate that the ARA binding is not established by the host while prepared to play
#if ARA_VALIDATE_API_CALLS
    ARA_VALIDATE_API_STATE (! isPrepared);
    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->araExtension = this;
#endif

#if DRX_ASSERTIONS_ENABLED_OR_LOGGED
    // validate proper subclassing of the instance role classes
    if (auto playbackRenderer = getPlaybackRenderer())
        jassert (dynamic_cast<ARAPlaybackRenderer*> (playbackRenderer) != nullptr);
    if (auto editorRenderer = getEditorRenderer())
        jassert (dynamic_cast<ARAEditorRenderer*> (editorRenderer) != nullptr);
    if (auto editorView = getEditorView())
        jassert (dynamic_cast<ARAEditorView*> (editorView) != nullptr);
#endif
}

//==============================================================================

AudioProcessorEditorARAExtension::AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor)
    : araProcessorExtension (dynamic_cast<AudioProcessorARAExtension*> (audioProcessor))
{
    if (isARAEditorView())
        getARAEditorView()->setEditorOpen (true);
}

AudioProcessorEditorARAExtension::~AudioProcessorEditorARAExtension()
{
    if (isARAEditorView())
        getARAEditorView()->setEditorOpen (false);
}

} // namespace drx
