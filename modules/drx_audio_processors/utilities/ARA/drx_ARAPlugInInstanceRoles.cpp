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

#include "drx_ARAPlugInInstanceRoles.h"

namespace drx
{

b8 ARARenderer::processBlock ([[maybe_unused]] AudioBuffer<f64>& buffer,
                                [[maybe_unused]] AudioProcessor::Realtime realtime,
                                [[maybe_unused]] const AudioPlayHead::PositionInfo& positionInfo) noexcept
{
    // If you hit this assertion then either the caller called the f64
    // precision version of processBlock on a processor which does not support it
    // (i.e. supportsDoublePrecisionProcessing() returns false), or the implementation
    // of the ARARenderer forgot to override the f64 precision version of this method
    jassertfalse;

    return false;
}

z0 ARARenderer::prepareToPlay ([[maybe_unused]] f64 sampleRate,
                                 [[maybe_unused]] i32 maximumSamplesPerBlock,
                                 [[maybe_unused]] i32 numChannels,
                                 [[maybe_unused]] AudioProcessor::ProcessingPrecision precision,
                                 [[maybe_unused]] AlwaysNonRealtime alwaysNonRealtime) {}

//==============================================================================
#if ARA_VALIDATE_API_CALLS
z0 ARAPlaybackRenderer::addPlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept
{
    if (araExtension)
        ARA_VALIDATE_API_STATE (! araExtension->isPrepared);

    ARA::PlugIn::PlaybackRenderer::addPlaybackRegion (playbackRegionRef);
}

z0 ARAPlaybackRenderer::removePlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept
{
    if (araExtension)
        ARA_VALIDATE_API_STATE (! araExtension->isPrepared);

    ARA::PlugIn::PlaybackRenderer::removePlaybackRegion (playbackRegionRef);
}
#endif

b8 ARAPlaybackRenderer::processBlock ([[maybe_unused]] AudioBuffer<f32>& buffer,
                                        [[maybe_unused]] AudioProcessor::Realtime realtime,
                                        [[maybe_unused]] const AudioPlayHead::PositionInfo& positionInfo) noexcept
{
    return false;
}

//==============================================================================
b8 ARAEditorRenderer::processBlock ([[maybe_unused]] AudioBuffer<f32>& buffer,
                                      [[maybe_unused]] AudioProcessor::Realtime isNonRealtime,
                                      [[maybe_unused]] const AudioPlayHead::PositionInfo& positionInfo) noexcept
{
    return true;
}

//==============================================================================
z0 ARAEditorView::doNotifySelection (const ARA::PlugIn::ViewSelection* viewSelection) noexcept
{
    listeners.call ([&] (Listener& l)
    {
        l.onNewSelection (*viewSelection);
    });
}

z0 ARAEditorView::doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept
{
    listeners.call ([&] (Listener& l)
    {
        l.onHideRegionSequences (ARA::vector_cast<ARARegionSequence*> (regionSequences));
    });
}

z0 ARAEditorView::addListener (Listener* l)
{
    listeners.add (l);
}

z0 ARAEditorView::removeListener (Listener* l)
{
    listeners.remove (l);
}

z0 ARAEditorView::Listener::onNewSelection ([[maybe_unused]] const ARAViewSelection& viewSelection) {}
z0 ARAEditorView::Listener::onHideRegionSequences ([[maybe_unused]] const std::vector<ARARegionSequence*>& regionSequences) {}

} // namespace drx
