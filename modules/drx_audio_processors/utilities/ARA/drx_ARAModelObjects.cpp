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
size_t ARADocument::getNumChildren() const noexcept
{
    return getMusicalContexts().size() + getRegionSequences().size() + getAudioSources().size();
}

ARAObject* ARADocument::getChild (size_t index)
{
    auto& musicalContexts = getMusicalContexts();

    if (index < musicalContexts.size())
        return musicalContexts[index];

    const auto numMusicalContexts = musicalContexts.size();
    auto& regionSequences = getRegionSequences();

    if (index < numMusicalContexts + regionSequences.size())
        return regionSequences[index - numMusicalContexts];

    const auto numMusicalContextsAndRegionSequences = numMusicalContexts + regionSequences.size();
    auto& audioSources = getAudioSources();

    if (index < numMusicalContextsAndRegionSequences + audioSources.size())
        return getAudioSources()[index - numMusicalContextsAndRegionSequences];

    return nullptr;
}

//==============================================================================
size_t ARARegionSequence::getNumChildren() const noexcept
{
    return 0;
}

ARAObject* ARARegionSequence::getChild (size_t)
{
    return nullptr;
}

Range<f64> ARARegionSequence::getTimeRange (ARAPlaybackRegion::IncludeHeadAndTail includeHeadAndTail) const
{
    if (getPlaybackRegions().empty())
        return {};

    auto startTime = std::numeric_limits<f64>::max();
    auto endTime = std::numeric_limits<f64>::lowest();
    for (const auto& playbackRegion : getPlaybackRegions())
    {
        const auto regionTimeRange = playbackRegion->getTimeRange (includeHeadAndTail);
        startTime = jmin (startTime, regionTimeRange.getStart());
        endTime = jmax (endTime, regionTimeRange.getEnd());
    }
    return { startTime, endTime };
}

f64 ARARegionSequence::getCommonSampleRate() const
{
    const auto getSampleRate = [] (auto* playbackRegion)
    {
        return playbackRegion->getAudioModification()->getAudioSource()->getSampleRate();
    };

    const auto range = getPlaybackRegions();
    const auto sampleRate = range.size() > 0 ? getSampleRate (range.front()) : 0.0;

    if (std::any_of (range.begin(), range.end(), [&] (auto& x) { return ! exactlyEqual (getSampleRate (x), sampleRate); }))
        return 0.0;

    return sampleRate;
}

//==============================================================================
size_t ARAAudioSource::getNumChildren() const noexcept
{
    return getAudioModifications().size();
}

ARAObject* ARAAudioSource::getChild (size_t index)
{
    return getAudioModifications()[index];
}

z0 ARAAudioSource::notifyAnalysisProgressStarted()
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioSourceAnalysisProgressStarted (this);
}

z0 ARAAudioSource::notifyAnalysisProgressUpdated (f32 progress)
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioSourceAnalysisProgressUpdated (this, progress);
}

z0 ARAAudioSource::notifyAnalysisProgressCompleted()
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioSourceAnalysisProgressCompleted (this);
}

z0 ARAAudioSource::notifyContentChanged (ARAContentUpdateScopes scopeFlags, b8 notifyARAHost)
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioSourceContentChanged (this,
                                                                                             scopeFlags,
                                                                                             notifyARAHost);
}

//==============================================================================
size_t ARAAudioModification::getNumChildren() const noexcept
{
    return getPlaybackRegions().size();
}

ARAObject* ARAAudioModification::getChild (size_t index)
{
    return getPlaybackRegions()[index];
}

z0 ARAAudioModification::notifyContentChanged (ARAContentUpdateScopes scopeFlags, b8 notifyARAHost)
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioModificationContentChanged (this,
                                                                                                   scopeFlags,
                                                                                                   notifyARAHost);
}

//==============================================================================
ARAObject* ARAPlaybackRegion::getParent() { return getAudioModification(); }

Range<f64> ARAPlaybackRegion::getTimeRange (IncludeHeadAndTail includeHeadAndTail) const
{
    auto startTime = getStartInPlaybackTime();
    auto endTime = getEndInPlaybackTime();

    if (includeHeadAndTail == IncludeHeadAndTail::yes)
    {
        ARA::ARATimeDuration headTime {}, tailTime {};
        getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
        startTime -= headTime;
        endTime += tailTime;
    }

    return { startTime, endTime };
}

Range<z64> ARAPlaybackRegion::getSampleRange (f64 sampleRate, IncludeHeadAndTail includeHeadAndTail) const
{
    const auto timeRange = getTimeRange (includeHeadAndTail);

    return { ARA::samplePositionAtTime (timeRange.getStart(), sampleRate),
             ARA::samplePositionAtTime (timeRange.getEnd(), sampleRate) };
}

f64 ARAPlaybackRegion::getHeadTime() const
{
    ARA::ARATimeDuration headTime {}, tailTime {};
    getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
    return headTime;
}

f64 ARAPlaybackRegion::getTailTime() const
{
    ARA::ARATimeDuration headTime {}, tailTime {};
    getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
    return tailTime;
}

z0 ARAPlaybackRegion::notifyContentChanged (ARAContentUpdateScopes scopeFlags, b8 notifyARAHost)
{
    getDocumentController<ARADocumentController>()->internalNotifyPlaybackRegionContentChanged (this,
                                                                                                scopeFlags,
                                                                                                notifyARAHost);
}

//==============================================================================
z0 ARADocumentListener::willBeginEditing ([[maybe_unused]] ARADocument* document) {}
z0 ARADocumentListener::didEndEditing ([[maybe_unused]] ARADocument* document) {}
z0 ARADocumentListener::willNotifyModelUpdates ([[maybe_unused]] ARADocument* document) {}
z0 ARADocumentListener::didNotifyModelUpdates ([[maybe_unused]] ARADocument* document) {}
z0 ARADocumentListener::willUpdateDocumentProperties ([[maybe_unused]] ARADocument* document,
                                                        [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARADocumentProperties> newProperties) {}
z0 ARADocumentListener::didUpdateDocumentProperties ([[maybe_unused]] ARADocument* document) {}
z0 ARADocumentListener::didAddMusicalContextToDocument ([[maybe_unused]] ARADocument* document,
                                                          [[maybe_unused]] ARAMusicalContext* musicalContext) {}
z0 ARADocumentListener::willRemoveMusicalContextFromDocument ([[maybe_unused]] ARADocument* document,
                                                                [[maybe_unused]] ARAMusicalContext* musicalContext) {}
z0 ARADocumentListener::didReorderMusicalContextsInDocument ([[maybe_unused]] ARADocument* document) {}
z0 ARADocumentListener::didAddRegionSequenceToDocument ([[maybe_unused]] ARADocument* document,
                                                          [[maybe_unused]] ARARegionSequence* regionSequence) {}
z0 ARADocumentListener::willRemoveRegionSequenceFromDocument ([[maybe_unused]] ARADocument* document,
                                                                [[maybe_unused]] ARARegionSequence* regionSequence) {}
z0 ARADocumentListener::didReorderRegionSequencesInDocument ([[maybe_unused]] ARADocument* document) {}
z0 ARADocumentListener::didAddAudioSourceToDocument ([[maybe_unused]] ARADocument* document,
                                                       [[maybe_unused]] ARAAudioSource* audioSource) {}
z0 ARADocumentListener::willRemoveAudioSourceFromDocument ([[maybe_unused]] ARADocument* document,
                                                             [[maybe_unused]] ARAAudioSource* audioSource) {}
z0 ARADocumentListener::willDestroyDocument ([[maybe_unused]] ARADocument* document) {}

//==============================================================================
z0 ARAMusicalContextListener::willUpdateMusicalContextProperties ([[maybe_unused]] ARAMusicalContext* musicalContext,
                                                                    [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARAMusicalContextProperties> newProperties) {}
z0 ARAMusicalContextListener::didUpdateMusicalContextProperties ([[maybe_unused]] ARAMusicalContext* musicalContext) {}
z0 ARAMusicalContextListener::doUpdateMusicalContextContent ([[maybe_unused]] ARAMusicalContext* musicalContext,
                                                               [[maybe_unused]] ARAContentUpdateScopes scopeFlags) {}
z0 ARAMusicalContextListener::didAddRegionSequenceToMusicalContext ([[maybe_unused]] ARAMusicalContext* musicalContext,
                                                                      [[maybe_unused]] ARARegionSequence* regionSequence) {}
z0 ARAMusicalContextListener::willRemoveRegionSequenceFromMusicalContext ([[maybe_unused]] ARAMusicalContext* musicalContext,
                                                                            [[maybe_unused]] ARARegionSequence* regionSequence) {}
z0 ARAMusicalContextListener::didReorderRegionSequencesInMusicalContext ([[maybe_unused]] ARAMusicalContext* musicalContext) {}
z0 ARAMusicalContextListener::willDestroyMusicalContext ([[maybe_unused]] ARAMusicalContext* musicalContext) {}

//==============================================================================
z0 ARAPlaybackRegionListener::willUpdatePlaybackRegionProperties ([[maybe_unused]] ARAPlaybackRegion* playbackRegion,
                                                                    [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) {}
z0 ARAPlaybackRegionListener::didUpdatePlaybackRegionProperties ([[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
z0 ARAPlaybackRegionListener::didUpdatePlaybackRegionContent ([[maybe_unused]] ARAPlaybackRegion* playbackRegion,
                                                                [[maybe_unused]] ARAContentUpdateScopes scopeFlags) {}
z0 ARAPlaybackRegionListener::willDestroyPlaybackRegion ([[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}

//==============================================================================
z0 ARARegionSequenceListener::willUpdateRegionSequenceProperties ([[maybe_unused]] ARARegionSequence* regionSequence,
                                                                    [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties) {}
z0 ARARegionSequenceListener::didUpdateRegionSequenceProperties ([[maybe_unused]] ARARegionSequence* regionSequence) {}
z0 ARARegionSequenceListener::willRemovePlaybackRegionFromRegionSequence ([[maybe_unused]] ARARegionSequence* regionSequence,
                                                                            [[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
z0 ARARegionSequenceListener::didAddPlaybackRegionToRegionSequence ([[maybe_unused]] ARARegionSequence* regionSequence,
                                                                      [[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
z0 ARARegionSequenceListener::willDestroyRegionSequence ([[maybe_unused]] ARARegionSequence* regionSequence) {}

//==============================================================================
z0 ARAAudioSourceListener::willUpdateAudioSourceProperties ([[maybe_unused]] ARAAudioSource* audioSource,
                                                              [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> newProperties) {}
z0 ARAAudioSourceListener::didUpdateAudioSourceProperties ([[maybe_unused]] ARAAudioSource* audioSource) {}
z0 ARAAudioSourceListener::doUpdateAudioSourceContent ([[maybe_unused]] ARAAudioSource* audioSource,
                                                         [[maybe_unused]] ARAContentUpdateScopes scopeFlags) {}
z0 ARAAudioSourceListener::didUpdateAudioSourceAnalysisProgress ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                   [[maybe_unused]] ARA::ARAAnalysisProgressState state,
                                                                   [[maybe_unused]] f32 progress) {}
z0 ARAAudioSourceListener::willEnableAudioSourceSamplesAccess ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                 [[maybe_unused]] b8 enable) {}
z0 ARAAudioSourceListener::didEnableAudioSourceSamplesAccess ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                [[maybe_unused]] b8 enable) {}
z0 ARAAudioSourceListener::willDeactivateAudioSourceForUndoHistory ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                      [[maybe_unused]] b8 deactivate) {}
z0 ARAAudioSourceListener::didDeactivateAudioSourceForUndoHistory ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                     [[maybe_unused]] b8 deactivate) {}
z0 ARAAudioSourceListener::didAddAudioModificationToAudioSource ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                   [[maybe_unused]] ARAAudioModification* audioModification) {}
z0 ARAAudioSourceListener::willRemoveAudioModificationFromAudioSource ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                         [[maybe_unused]] ARAAudioModification* audioModification) {}
z0 ARAAudioSourceListener::willDestroyAudioSource ([[maybe_unused]] ARAAudioSource* audioSource) {}

//==============================================================================
z0 ARAAudioModificationListener::willUpdateAudioModificationProperties ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                          [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARAAudioModificationProperties> newProperties) {}
z0 ARAAudioModificationListener::didUpdateAudioModificationProperties ([[maybe_unused]] ARAAudioModification* audioModification) {}
z0 ARAAudioModificationListener::didUpdateAudioModificationContent ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                      [[maybe_unused]] ARAContentUpdateScopes scopeFlags) {}
z0 ARAAudioModificationListener::willDeactivateAudioModificationForUndoHistory ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                                  [[maybe_unused]] b8 deactivate) {}
z0 ARAAudioModificationListener::didDeactivateAudioModificationForUndoHistory ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                                 [[maybe_unused]] b8 deactivate) {}
z0 ARAAudioModificationListener::didAddPlaybackRegionToAudioModification ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                            [[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
z0 ARAAudioModificationListener::willRemovePlaybackRegionFromAudioModification ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                                  [[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
z0 ARAAudioModificationListener::willDestroyAudioModification ([[maybe_unused]] ARAAudioModification* audioModification) {}

} // namespace drx
