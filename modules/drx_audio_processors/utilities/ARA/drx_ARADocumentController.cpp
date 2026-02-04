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

class ARADocumentControllerSpecialisation::ARADocumentControllerImpl  : public ARADocumentController
{
public:
    ARADocumentControllerImpl (const ARA::PlugIn::PlugInEntry* entry,
                               const ARA::ARADocumentControllerHostInstance* instance,
                               ARADocumentControllerSpecialisation* spec)
        : ARADocumentController (entry, instance), specialisation (spec)
    {
    }

    template <typename PlaybackRenderer_t = ARAPlaybackRenderer>
    std::vector<PlaybackRenderer_t*> const& getPlaybackRenderers() const noexcept
    {
        return ARA::PlugIn::DocumentController::getPlaybackRenderers<PlaybackRenderer_t>();
    }

    template <typename EditorRenderer_t = ARAEditorRenderer>
    std::vector<EditorRenderer_t*> const& getEditorRenderers() const noexcept
    {
        return ARA::PlugIn::DocumentController::getEditorRenderers<EditorRenderer_t>();
    }

    template <typename EditorView_t = ARAEditorView>
    std::vector<EditorView_t*> const& getEditorViews() const noexcept
    {
        return ARA::PlugIn::DocumentController::getEditorViews<EditorView_t>();
    }

    auto getSpecialisation() { return specialisation; }

protected:
    //==============================================================================
    b8 doRestoreObjectsFromStream (ARAInputStream& input, const ARARestoreObjectsFilter* filter) noexcept
    {
        return specialisation->doRestoreObjectsFromStream (input, filter);
    }

    b8 doStoreObjectsToStream (ARAOutputStream& output, const ARAStoreObjectsFilter* filter) noexcept
    {
        return specialisation->doStoreObjectsToStream (output, filter);
    }

    //==============================================================================
    // Model object creation
    ARA::PlugIn::Document*          doCreateDocument          () noexcept override;
    ARA::PlugIn::MusicalContext*    doCreateMusicalContext    (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept override;
    ARA::PlugIn::RegionSequence*    doCreateRegionSequence    (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioSource*       doCreateAudioSource       (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept override;
    ARA::PlugIn::PlaybackRegion*    doCreatePlaybackRegion    (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;

    //==============================================================================
    // Plugin role implementation
    friend class ARAPlaybackRegionReader;
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;
    ARA::PlugIn::EditorRenderer*   doCreateEditorRenderer() noexcept override;
    ARA::PlugIn::EditorView*       doCreateEditorView() noexcept override;

    //==============================================================================
    // ARAAudioSource content access
    b8 doIsAudioSourceContentAvailable (const ARA::PlugIn::AudioSource* audioSource,
                                          ARA::ARAContentType type) noexcept override;
    ARA::ARAContentGrade doGetAudioSourceContentGrade (const ARA::PlugIn::AudioSource* audioSource,
                                                       ARA::ARAContentType type) noexcept override;
    ARA::PlugIn::ContentReader* doCreateAudioSourceContentReader (ARA::PlugIn::AudioSource* audioSource,
                                                                  ARA::ARAContentType type,
                                                                  const ARA::ARAContentTimeRange* range) noexcept override;

    //==============================================================================
    // ARAAudioModification content access
    b8 doIsAudioModificationContentAvailable (const ARA::PlugIn::AudioModification* audioModification,
                                                ARA::ARAContentType type) noexcept override;
    ARA::ARAContentGrade doGetAudioModificationContentGrade (const ARA::PlugIn::AudioModification* audioModification,
                                                             ARA::ARAContentType type) noexcept override;
    ARA::PlugIn::ContentReader* doCreateAudioModificationContentReader (ARA::PlugIn::AudioModification* audioModification,
                                                                        ARA::ARAContentType type,
                                                                        const ARA::ARAContentTimeRange* range) noexcept override;

    //==============================================================================
    // ARAPlaybackRegion content access
    b8 doIsPlaybackRegionContentAvailable (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                             ARA::ARAContentType type) noexcept override;
    ARA::ARAContentGrade doGetPlaybackRegionContentGrade (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                          ARA::ARAContentType type) noexcept override;
    ARA::PlugIn::ContentReader* doCreatePlaybackRegionContentReader (ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                     ARA::ARAContentType type,
                                                                     const ARA::ARAContentTimeRange* range) noexcept override;
    z0 doGetPlaybackRegionHeadAndTailTime (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                             ARA::ARATimeDuration* headTime,
                                             ARA::ARATimeDuration* tailTime) noexcept override;

    //==============================================================================
    // ARAAudioSource analysis
    b8 doIsAudioSourceContentAnalysisIncomplete (const ARA::PlugIn::AudioSource* audioSource,
                                                   ARA::ARAContentType type) noexcept override;
    z0 doRequestAudioSourceContentAnalysis (ARA::PlugIn::AudioSource* audioSource,
                                              std::vector<ARA::ARAContentType> const& contentTypes) noexcept override;

    //==============================================================================
    // Analysis Algorithm selection
    ARA::ARAInt32 doGetProcessingAlgorithmsCount() noexcept override;
    const ARA::ARAProcessingAlgorithmProperties* doGetProcessingAlgorithmProperties (ARA::ARAInt32 algorithmIndex) noexcept override;
    ARA::ARAInt32 doGetProcessingAlgorithmForAudioSource (const ARA::PlugIn::AudioSource* audioSource) noexcept override;
    z0 doRequestProcessingAlgorithmForAudioSource (ARA::PlugIn::AudioSource* audioSource,
                                                     ARA::ARAInt32 algorithmIndex) noexcept override;

#ifndef DOXYGEN

    //==============================================================================
    b8 doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
    b8 doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;

    //==============================================================================
    // Document notifications
    z0 willBeginEditing() noexcept override;
    z0 didEndEditing() noexcept override;
    z0 willNotifyModelUpdates() noexcept override;
    z0 didNotifyModelUpdates() noexcept override;
    z0 willUpdateDocumentProperties (ARA::PlugIn::Document* document, ARADocument::PropertiesPtr newProperties) noexcept override;
    z0 didUpdateDocumentProperties (ARA::PlugIn::Document* document) noexcept override;
    z0 didAddMusicalContextToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    z0 willRemoveMusicalContextFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    z0 didReorderMusicalContextsInDocument (ARA::PlugIn::Document* document) noexcept override;
    z0 didAddRegionSequenceToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    z0 willRemoveRegionSequenceFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    z0 didReorderRegionSequencesInDocument (ARA::PlugIn::Document* document) noexcept override;
    z0 didAddAudioSourceToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::AudioSource* audioSource) noexcept override;
    z0 willRemoveAudioSourceFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::AudioSource* audioSource) noexcept override;
    z0 willDestroyDocument (ARA::PlugIn::Document* document) noexcept override;

    //==============================================================================
    // MusicalContext notifications
    z0 willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept override;
    z0 didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    z0 doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;
    z0 didAddRegionSequenceToMusicalContext (ARA::PlugIn::MusicalContext* musicalContext, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    z0 willRemoveRegionSequenceFromMusicalContext (ARA::PlugIn::MusicalContext* musicalContext, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    z0 didReorderRegionSequencesInMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    z0 willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;

    //==============================================================================
    // RegionSequence notifications, typically not overridden further
    z0 willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept override;
    z0 didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    z0 didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    z0 willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    z0 willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;

    //==============================================================================
    // AudioSource notifications
    z0 willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARAAudioSource::PropertiesPtr newProperties) noexcept override;
    z0 didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept override;
    z0 doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;
    z0 willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, b8 enable) noexcept override;
    z0 didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, b8 enable) noexcept override;
    z0 didAddAudioModificationToAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept override;
    z0 willRemoveAudioModificationFromAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept override;
    z0 willDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, b8 deactivate) noexcept override;
    z0 didDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, b8 deactivate) noexcept override;
    z0 willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept override;

    //==============================================================================
    // AudioModification notifications
    z0 willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARAAudioModification::PropertiesPtr newProperties) noexcept override;
    z0 didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept override;
    z0 didAddPlaybackRegionToAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    z0 willRemovePlaybackRegionFromAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    z0 willDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, b8 deactivate) noexcept override;
    z0 didDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, b8 deactivate) noexcept override;
    z0 willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept override;

    //==============================================================================
    // PlaybackRegion notifications
    z0 willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept override;
    z0 didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    z0 willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

public:
    //==============================================================================
    /** @internal */
    z0 internalNotifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource) override;

    /** @internal */
    z0 internalNotifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource, f32 progress) override;

    /** @internal */
    z0 internalNotifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource) override;

    /** @internal */
    z0 internalDidUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource,
                                                       ARAAudioSource::ARAAnalysisProgressState state,
                                                       f32 progress) override;

    //==============================================================================
    /** @internal */
    z0 internalNotifyAudioSourceContentChanged (ARAAudioSource* audioSource,
                                                  ARAContentUpdateScopes scopeFlags,
                                                  b8 notifyARAHost) override;

    /** @internal */
    z0 internalNotifyAudioModificationContentChanged (ARAAudioModification* audioModification,
                                                        ARAContentUpdateScopes scopeFlags,
                                                        b8 notifyARAHost) override;

    /** @internal */
    z0 internalNotifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion,
                                                     ARAContentUpdateScopes scopeFlags,
                                                     b8 notifyARAHost) override;

#endif

private:
    //==============================================================================
    ARADocumentControllerSpecialisation* specialisation;
    std::atomic<b8> internalAnalysisProgressIsSynced { true };
    ScopedDrxInitialiser_GUI libraryInitialiser;
    i32 activeAudioSourcesCount = 0;
    std::optional<TimedCallback> analysisTimer;

    z0 analysisTimerCallback();

    //==============================================================================
    template <typename ModelObject, typename Function, typename... Ts>
    z0 notifyListeners (Function ModelObject::Listener::* function, ModelObject* modelObject, Ts... ts)
    {
        (specialisation->*function) (modelObject, ts...);
        modelObject->notifyListeners ([&] (auto& l)
                                       {
                                           try
                                           {
                                               (l.*function) (modelObject, ts...);
                                           }
                                           catch (...)
                                           {
                                           }
                                       });
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentControllerImpl)
};

ARA::PlugIn::DocumentController* ARADocumentControllerSpecialisation::getDocumentController() noexcept
{
    return documentController.get();
}

//==============================================================================
z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressStarted, 0.0f))
        internalAnalysisProgressIsSynced.store (false, std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressStarted (audioSource);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource,
                                                                                                                       f32 progress)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressUpdated,  progress))
        internalAnalysisProgressIsSynced.store (false, std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressUpdated (audioSource, progress);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressCompleted, 1.0f))
        internalAnalysisProgressIsSynced.store (false, std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressCompleted (audioSource);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalDidUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource,
                                                                                                                   ARAAudioSource::ARAAnalysisProgressState state,
                                                                                                                   f32 progress)
{
    specialisation->didUpdateAudioSourceAnalysisProgress (audioSource, state, progress);
}

//==============================================================================
ARADocumentControllerSpecialisation* ARADocumentControllerSpecialisation::getSpecialisedDocumentControllerImpl (ARA::PlugIn::DocumentController* dc)
{
    return static_cast<ARADocumentControllerImpl*> (dc)->getSpecialisation();
}

ARADocument* ARADocumentControllerSpecialisation::getDocumentImpl()
{
    return documentController->getDocument();
}

//==============================================================================
// some helper macros to ease repeated declaration & implementation of notification functions below:
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wgnu-zero-variadic-macro-arguments")

// no notification arguments
#define OVERRIDE_TO_NOTIFY_1(function, ModelObjectType, modelObject) \
    z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::function (ARA::PlugIn::ModelObjectType* modelObject) noexcept \
    { \
        notifyListeners (&ARA##ModelObjectType::Listener::function, static_cast<ARA##ModelObjectType*> (modelObject)); \
    }

// single notification argument, model object version
#define OVERRIDE_TO_NOTIFY_2(function, ModelObjectType, modelObject, ArgumentType, argument) \
    z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::function (ARA::PlugIn::ModelObjectType* modelObject, ARA::PlugIn::ArgumentType argument) noexcept \
    { \
        notifyListeners (&ARA##ModelObjectType::Listener::function, static_cast<ARA##ModelObjectType*> (modelObject), static_cast<ARA##ArgumentType> (argument)); \
    }

// single notification argument, non-model object version
#define OVERRIDE_TO_NOTIFY_3(function, ModelObjectType, modelObject, ArgumentType, argument) \
    z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::function (ARA::PlugIn::ModelObjectType* modelObject, ArgumentType argument) noexcept \
    { \
        notifyListeners (&ARA##ModelObjectType::Listener::function, static_cast<ARA##ModelObjectType*> (modelObject), argument); \
    }

//==============================================================================
ARA::PlugIn::Document* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateDocument() noexcept
{
    auto* document = specialisation->doCreateDocument();

    // Your Document subclass must inherit from drx::ARADocument
    jassert (dynamic_cast<ARADocument*> (document));

    return document;
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::willBeginEditing() noexcept
{
    notifyListeners (&ARADocument::Listener::willBeginEditing, static_cast<ARADocument*> (getDocument()));
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::didEndEditing() noexcept
{
    notifyListeners (&ARADocument::Listener::didEndEditing, static_cast<ARADocument*> (getDocument()));

    if (activeAudioSourcesCount == 0)
    {
        analysisTimer.reset();
    }
    else if (! analysisTimer.has_value() && (activeAudioSourcesCount > 0))
    {
        analysisTimer.emplace ([this] { analysisTimerCallback(); });
        analysisTimer->startTimerHz (20);
    }
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::willNotifyModelUpdates() noexcept
{
    notifyListeners (&ARADocument::Listener::willNotifyModelUpdates, static_cast<ARADocument*> (getDocument()));
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::didNotifyModelUpdates() noexcept
{
    notifyListeners (&ARADocument::Listener::didNotifyModelUpdates, static_cast<ARADocument*> (getDocument()));
}

//==============================================================================
b8 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader,
                                                                                                  const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept
{
    ARAInputStream reader (archiveReader);
    return doRestoreObjectsFromStream (reader, filter);
}

b8 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter,
                                                                                              const ARA::PlugIn::StoreObjectsFilter* filter) noexcept
{
    ARAOutputStream writer (archiveWriter);
    return doStoreObjectsToStream (writer, filter);
}

//==============================================================================
OVERRIDE_TO_NOTIFY_3 (willUpdateDocumentProperties, Document, document, ARADocument::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateDocumentProperties, Document, document)
OVERRIDE_TO_NOTIFY_2 (didAddMusicalContextToDocument, Document, document, MusicalContext*, musicalContext)
OVERRIDE_TO_NOTIFY_2 (willRemoveMusicalContextFromDocument, Document, document, MusicalContext*, musicalContext)
OVERRIDE_TO_NOTIFY_1 (didReorderMusicalContextsInDocument, Document, document)
OVERRIDE_TO_NOTIFY_2 (didAddRegionSequenceToDocument, Document, document, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_2 (willRemoveRegionSequenceFromDocument, Document, document, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_1 (didReorderRegionSequencesInDocument, Document, document)
OVERRIDE_TO_NOTIFY_2 (didAddAudioSourceToDocument, Document, document, AudioSource*, audioSource)
OVERRIDE_TO_NOTIFY_2 (willRemoveAudioSourceFromDocument, Document, document, AudioSource*, audioSource)
OVERRIDE_TO_NOTIFY_1 (willDestroyDocument, Document, document)

//==============================================================================
ARA::PlugIn::MusicalContext* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateMusicalContext (ARA::PlugIn::Document* document,
                                                                                                                     ARA::ARAMusicalContextHostRef hostRef) noexcept
{
    return specialisation->doCreateMusicalContext (static_cast<ARADocument*> (document), hostRef);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext,
                                                                                                    const ARA::ARAContentTimeRange*,
                                                                                                    ARA::ContentUpdateScopes flags) noexcept
{
    notifyListeners (&ARAMusicalContext::Listener::doUpdateMusicalContextContent,
                     static_cast<ARAMusicalContext*> (musicalContext),
                     flags);
}

OVERRIDE_TO_NOTIFY_3 (willUpdateMusicalContextProperties, MusicalContext, musicalContext, ARAMusicalContext::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateMusicalContextProperties, MusicalContext, musicalContext)
OVERRIDE_TO_NOTIFY_2 (didAddRegionSequenceToMusicalContext, MusicalContext, musicalContext, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_2 (willRemoveRegionSequenceFromMusicalContext, MusicalContext, musicalContext, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_1 (didReorderRegionSequencesInMusicalContext, MusicalContext, musicalContext)
OVERRIDE_TO_NOTIFY_1 (willDestroyMusicalContext, MusicalContext, musicalContext)

//==============================================================================
ARA::PlugIn::RegionSequence* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return specialisation->doCreateRegionSequence (static_cast<ARADocument*> (document), hostRef);
}

OVERRIDE_TO_NOTIFY_3 (willUpdateRegionSequenceProperties, RegionSequence, regionSequence, ARARegionSequence::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateRegionSequenceProperties, RegionSequence, regionSequence)
OVERRIDE_TO_NOTIFY_2 (didAddPlaybackRegionToRegionSequence, RegionSequence, regionSequence, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_2 (willRemovePlaybackRegionFromRegionSequence, RegionSequence, regionSequence, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_1 (willDestroyRegionSequence, RegionSequence, regionSequence)

//==============================================================================
ARA::PlugIn::AudioSource* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    ++activeAudioSourcesCount;
    return specialisation->doCreateAudioSource (static_cast<ARADocument*> (document), hostRef);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource,
                                                                                                 const ARA::ARAContentTimeRange*,
                                                                                                 ARA::ContentUpdateScopes flags) noexcept
{
    notifyListeners (&ARAAudioSource::Listener::doUpdateAudioSourceContent, static_cast<ARAAudioSource*> (audioSource), flags);
}

OVERRIDE_TO_NOTIFY_3 (willUpdateAudioSourceProperties, AudioSource, audioSource, ARAAudioSource::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateAudioSourceProperties, AudioSource, audioSource)
OVERRIDE_TO_NOTIFY_3 (willEnableAudioSourceSamplesAccess, AudioSource, audioSource, b8, enable)
OVERRIDE_TO_NOTIFY_3 (didEnableAudioSourceSamplesAccess, AudioSource, audioSource, b8, enable)
OVERRIDE_TO_NOTIFY_2 (didAddAudioModificationToAudioSource, AudioSource, audioSource, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_2 (willRemoveAudioModificationFromAudioSource, AudioSource, audioSource, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_3 (willDeactivateAudioSourceForUndoHistory, AudioSource, audioSource, b8, deactivate)

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::didDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource,
                                                                                                             b8 deactivate) noexcept
{
    activeAudioSourcesCount += (deactivate ? -1 : 1);
    notifyListeners (&ARAAudioSource::Listener::didDeactivateAudioSourceForUndoHistory,
                     static_cast<ARAAudioSource*> (audioSource),
                     deactivate);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    if (! audioSource->isDeactivatedForUndoHistory())
        --activeAudioSourcesCount;

    notifyListeners (&ARAAudioSource::Listener::willDestroyAudioSource, static_cast<ARAAudioSource*> (audioSource));
}
//==============================================================================
ARA::PlugIn::AudioModification* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource,
                                                                                                                           ARA::ARAAudioModificationHostRef hostRef,
                                                                                                                           const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
{
    return specialisation->doCreateAudioModification (static_cast<ARAAudioSource*> (audioSource),
                                                      hostRef,
                                                      static_cast<const ARAAudioModification*> (optionalModificationToClone));
}

OVERRIDE_TO_NOTIFY_3 (willUpdateAudioModificationProperties, AudioModification, audioModification, ARAAudioModification::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateAudioModificationProperties, AudioModification, audioModification)
OVERRIDE_TO_NOTIFY_2 (didAddPlaybackRegionToAudioModification, AudioModification, audioModification, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_2 (willRemovePlaybackRegionFromAudioModification, AudioModification, audioModification, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_3 (willDeactivateAudioModificationForUndoHistory, AudioModification, audioModification, b8, deactivate)
OVERRIDE_TO_NOTIFY_3 (didDeactivateAudioModificationForUndoHistory, AudioModification, audioModification, b8, deactivate)
OVERRIDE_TO_NOTIFY_1 (willDestroyAudioModification, AudioModification, audioModification)

//==============================================================================
ARA::PlugIn::PlaybackRegion* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification,
                                                                                                                     ARA::ARAPlaybackRegionHostRef hostRef) noexcept
{
    return specialisation->doCreatePlaybackRegion (static_cast<ARAAudioModification*> (modification), hostRef);
}

OVERRIDE_TO_NOTIFY_3 (willUpdatePlaybackRegionProperties, PlaybackRegion, playbackRegion, ARAPlaybackRegion::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdatePlaybackRegionProperties, PlaybackRegion, playbackRegion)
OVERRIDE_TO_NOTIFY_1 (willDestroyPlaybackRegion, PlaybackRegion, playbackRegion)

//==============================================================================
z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioSourceContentChanged (ARAAudioSource* audioSource,
                                                                                                              ARAContentUpdateScopes scopeFlags,
                                                                                                              b8 notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyAudioSourceContentChanged (audioSource, scopeFlags);

    notifyListeners (&ARAAudioSource::Listener::doUpdateAudioSourceContent, audioSource, scopeFlags);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioModificationContentChanged (ARAAudioModification* audioModification,
                                                                                                                    ARAContentUpdateScopes scopeFlags,
                                                                                                                    b8 notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyAudioModificationContentChanged (audioModification, scopeFlags);

    notifyListeners (&ARAAudioModification::Listener::didUpdateAudioModificationContent, audioModification, scopeFlags);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion,
                                                                                                                 ARAContentUpdateScopes scopeFlags,
                                                                                                                 b8 notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyPlaybackRegionContentChanged (playbackRegion, scopeFlags);

    notifyListeners (&ARAPlaybackRegion::Listener::didUpdatePlaybackRegionContent, playbackRegion, scopeFlags);
}

//==============================================================================
DRX_END_IGNORE_WARNINGS_GCC_LIKE

#undef OVERRIDE_TO_NOTIFY_1
#undef OVERRIDE_TO_NOTIFY_2
#undef OVERRIDE_TO_NOTIFY_3

//==============================================================================
ARADocument* ARADocumentControllerSpecialisation::doCreateDocument()
{
    return new ARADocument (static_cast<ARADocumentControllerImpl*> (getDocumentController()));
}

ARAMusicalContext* ARADocumentControllerSpecialisation::doCreateMusicalContext (ARADocument* document,
                                                                                ARA::ARAMusicalContextHostRef hostRef)
{
    return new ARAMusicalContext (static_cast<ARADocument*> (document), hostRef);
}

ARARegionSequence* ARADocumentControllerSpecialisation::doCreateRegionSequence (ARADocument* document,
                                                                                ARA::ARARegionSequenceHostRef hostRef)
{
    return new ARARegionSequence (static_cast<ARADocument*> (document), hostRef);
}

ARAAudioSource* ARADocumentControllerSpecialisation::doCreateAudioSource (ARADocument* document,
                                                                          ARA::ARAAudioSourceHostRef hostRef)
{
     return new ARAAudioSource (static_cast<ARADocument*> (document), hostRef);
}

ARAAudioModification* ARADocumentControllerSpecialisation::doCreateAudioModification (
    ARAAudioSource* audioSource,
    ARA::ARAAudioModificationHostRef hostRef,
    const ARAAudioModification* optionalModificationToClone)
{
    return new ARAAudioModification (static_cast<ARAAudioSource*> (audioSource),
                                     hostRef,
                                     static_cast<const ARAAudioModification*> (optionalModificationToClone));
}

ARAPlaybackRegion*
    ARADocumentControllerSpecialisation::doCreatePlaybackRegion (ARAAudioModification* modification,
                                                                 ARA::ARAPlaybackRegionHostRef hostRef)
{
    return new ARAPlaybackRegion (static_cast<ARAAudioModification*> (modification), hostRef);
}

//==============================================================================
ARA::PlugIn::PlaybackRenderer* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreatePlaybackRenderer() noexcept
{
    return specialisation->doCreatePlaybackRenderer();
}

ARA::PlugIn::EditorRenderer* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateEditorRenderer() noexcept
{
    return specialisation->doCreateEditorRenderer();
}

ARA::PlugIn::EditorView* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateEditorView() noexcept
{
    return specialisation->doCreateEditorView();
}

b8 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doIsAudioSourceContentAvailable (const ARA::PlugIn::AudioSource* audioSource,
                                                                                                      ARA::ARAContentType type) noexcept
{
    return specialisation->doIsAudioSourceContentAvailable (audioSource, type);
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetAudioSourceContentGrade (const ARA::PlugIn::AudioSource* audioSource,
                                                                                                                   ARA::ARAContentType type) noexcept
{
    return specialisation->doGetAudioSourceContentGrade (audioSource, type);
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateAudioSourceContentReader (ARA::PlugIn::AudioSource* audioSource,
                                                                                                                              ARA::ARAContentType type,
                                                                                                                              const ARA::ARAContentTimeRange* range) noexcept
{
    return specialisation->doCreateAudioSourceContentReader (audioSource, type, range);
}

b8 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doIsAudioModificationContentAvailable (const ARA::PlugIn::AudioModification* audioModification,
                                                                                                            ARA::ARAContentType type) noexcept
{
    return specialisation->doIsAudioModificationContentAvailable (audioModification, type);
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetAudioModificationContentGrade (const ARA::PlugIn::AudioModification* audioModification,
                                                                                                                         ARA::ARAContentType type) noexcept
{
    return specialisation->doGetAudioModificationContentGrade (audioModification, type);
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateAudioModificationContentReader (ARA::PlugIn::AudioModification* audioModification,
                                                                                                                                    ARA::ARAContentType type,
                                                                                                                                    const ARA::ARAContentTimeRange* range) noexcept
{
    return specialisation->doCreateAudioModificationContentReader (audioModification, type, range);
}

b8 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doIsPlaybackRegionContentAvailable (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                         ARA::ARAContentType type) noexcept
{
    return specialisation->doIsPlaybackRegionContentAvailable (playbackRegion, type);
}

ARA::ARAContentGrade
    ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetPlaybackRegionContentGrade (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                     ARA::ARAContentType type) noexcept
{
    return specialisation->doGetPlaybackRegionContentGrade (playbackRegion, type);
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreatePlaybackRegionContentReader (ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                                                 ARA::ARAContentType type,
                                                                                                                                 const ARA::ARAContentTimeRange* range) noexcept
{
    return specialisation->doCreatePlaybackRegionContentReader (playbackRegion, type, range);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetPlaybackRegionHeadAndTailTime (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                         ARA::ARATimeDuration* headTime,
                                                                                                         ARA::ARATimeDuration* tailTime) noexcept
{
    specialisation->doGetPlaybackRegionHeadAndTailTime (playbackRegion, headTime, tailTime);
}

b8 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doIsAudioSourceContentAnalysisIncomplete (const ARA::PlugIn::AudioSource* audioSource,
                                                                                                               ARA::ARAContentType type) noexcept
{
    return specialisation->doIsAudioSourceContentAnalysisIncomplete (audioSource, type);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doRequestAudioSourceContentAnalysis (ARA::PlugIn::AudioSource* audioSource,
                                                                                                          std::vector<ARA::ARAContentType> const& contentTypes) noexcept
{
    specialisation->doRequestAudioSourceContentAnalysis (audioSource, contentTypes);
}

ARA::ARAInt32 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetProcessingAlgorithmsCount() noexcept
{
    return specialisation->doGetProcessingAlgorithmsCount();
}

const ARA::ARAProcessingAlgorithmProperties* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetProcessingAlgorithmProperties (ARA::ARAInt32 algorithmIndex) noexcept
{
    return specialisation->doGetProcessingAlgorithmProperties (algorithmIndex);
}

ARA::ARAInt32 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetProcessingAlgorithmForAudioSource (const ARA::PlugIn::AudioSource* audioSource) noexcept
{
    return specialisation->doGetProcessingAlgorithmForAudioSource (audioSource);
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doRequestProcessingAlgorithmForAudioSource (ARA::PlugIn::AudioSource* audioSource,
                                                                                                                 ARA::ARAInt32 algorithmIndex) noexcept
{
    return specialisation->doRequestProcessingAlgorithmForAudioSource (audioSource, algorithmIndex);
}

//==============================================================================
// Helper code for ARADocumentControllerSpecialisation::ARADocumentControllerImpl::timerCallback() to
// rewire the host-related ARA SDK's progress tracker to our internal update mechanism.
namespace ModelUpdateControllerProgressAdapter
{
    using namespace ARA;

    static z0 ARA_CALL notifyAudioSourceAnalysisProgress (ARAModelUpdateControllerHostRef /*controllerHostRef*/,
                                                            ARAAudioSourceHostRef audioSourceHostRef, ARAAnalysisProgressState state, f32 value) noexcept
    {
        auto audioSource = reinterpret_cast<ARAAudioSource*> (audioSourceHostRef);
        audioSource->getDocumentController<ARADocumentController>()->internalDidUpdateAudioSourceAnalysisProgress (audioSource, state, value);
        audioSource->notifyListeners ([&] (ARAAudioSource::Listener& l) { l.didUpdateAudioSourceAnalysisProgress (audioSource, state, value); });
    }

    static z0 ARA_CALL notifyAudioSourceContentChanged (ARAModelUpdateControllerHostRef, ARAAudioSourceHostRef,
                                                          const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassertfalse;    // not to be called - this adapter only forwards analysis progress
    }

    static z0 ARA_CALL notifyAudioModificationContentChanged (ARAModelUpdateControllerHostRef, ARAAudioModificationHostRef,
                                                                const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassertfalse;    // not to be called - this adapter only forwards analysis progress
    }

    static z0 ARA_CALL notifyPlaybackRegionContentChanged (ARAModelUpdateControllerHostRef, ARAPlaybackRegionHostRef,
                                                             const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassertfalse;    // not to be called - this adapter only forwards analysis progress
    }

    static ARA::PlugIn::HostModelUpdateController* get()
    {
        static const auto modelUpdateControllerInterface = makeARASizedStruct (&ARA::ARAModelUpdateControllerInterface::notifyPlaybackRegionContentChanged,
                                                                               ModelUpdateControllerProgressAdapter::notifyAudioSourceAnalysisProgress,
                                                                               ModelUpdateControllerProgressAdapter::notifyAudioSourceContentChanged,
                                                                               ModelUpdateControllerProgressAdapter::notifyAudioModificationContentChanged,
                                                                               ModelUpdateControllerProgressAdapter::notifyPlaybackRegionContentChanged);

        static const auto instance = makeARASizedStruct (&ARA::ARADocumentControllerHostInstance::playbackControllerInterface,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         &modelUpdateControllerInterface,
                                                         nullptr,
                                                         nullptr);

        static auto progressAdapter = ARA::PlugIn::HostModelUpdateController { &instance };
        return &progressAdapter;
    }
}

z0 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::analysisTimerCallback()
{
    if (! internalAnalysisProgressIsSynced.exchange (true, std::memory_order_release))
        for (auto& audioSource : getDocument()->getAudioSources())
            audioSource->internalAnalysisProgressTracker.notifyProgress (ModelUpdateControllerProgressAdapter::get(),
                                                                         reinterpret_cast<ARA::ARAAudioSourceHostRef> (audioSource));
}

//==============================================================================
ARAInputStream::ARAInputStream (ARA::PlugIn::HostArchiveReader* reader)
    : archiveReader (reader),
      size ((z64) reader->getArchiveSize())
{}

i32 ARAInputStream::read (uk destBuffer, i32 maxBytesToRead)
{
    const auto bytesToRead = std::min ((z64) maxBytesToRead, size - position);

    if (bytesToRead > 0 && ! archiveReader->readBytesFromArchive ((ARA::ARASize) position, (ARA::ARASize) bytesToRead,
                                                                  static_cast<ARA::ARAByte*> (destBuffer)))
    {
        failure = true;
        return 0;
    }

    position += bytesToRead;
    return (i32) bytesToRead;
}

b8 ARAInputStream::setPosition (z64 newPosition)
{
    position = jlimit ((z64) 0, size, newPosition);
    return true;
}

b8 ARAInputStream::isExhausted()
{
    return position >= size;
}

ARAOutputStream::ARAOutputStream (ARA::PlugIn::HostArchiveWriter* writer)
    : archiveWriter (writer)
{}

b8 ARAOutputStream::write (ukk dataToWrite, size_t numberOfBytes)
{
    if (! archiveWriter->writeBytesToArchive ((ARA::ARASize) position, numberOfBytes, (const ARA::ARAByte*) dataToWrite))
        return false;

    position += (z64) numberOfBytes;
    return true;
}

b8 ARAOutputStream::setPosition (z64 newPosition)
{
    position = newPosition;
    return true;
}

//==============================================================================
ARADocumentControllerSpecialisation::ARADocumentControllerSpecialisation (
    const ARA::PlugIn::PlugInEntry* entry,
    const ARA::ARADocumentControllerHostInstance* instance)
    : documentController (std::make_unique<ARADocumentControllerImpl> (entry, instance, this))
{
}

ARADocumentControllerSpecialisation::~ARADocumentControllerSpecialisation() = default;

ARAPlaybackRenderer* ARADocumentControllerSpecialisation::doCreatePlaybackRenderer()
{
    return new ARAPlaybackRenderer (getDocumentController());
}

ARAEditorRenderer* ARADocumentControllerSpecialisation::doCreateEditorRenderer()
{
    return new ARAEditorRenderer (getDocumentController());
}

ARAEditorView* ARADocumentControllerSpecialisation::doCreateEditorView()
{
    return new ARAEditorView (getDocumentController());
}

b8 ARADocumentControllerSpecialisation::doIsAudioSourceContentAvailable ([[maybe_unused]] const ARA::PlugIn::AudioSource* audioSource,
                                                                           [[maybe_unused]] ARA::ARAContentType type)
{
    return false;
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::doGetAudioSourceContentGrade ([[maybe_unused]] const ARA::PlugIn::AudioSource* audioSource,
                                                                                        [[maybe_unused]] ARA::ARAContentType type)
{
    // Overriding doIsAudioSourceContentAvailable() requires overriding
    // doGetAudioSourceContentGrade() accordingly!
    jassertfalse;

    return ARA::kARAContentGradeInitial;
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::doCreateAudioSourceContentReader ([[maybe_unused]] ARA::PlugIn::AudioSource* audioSource,
                                                                                                   [[maybe_unused]] ARA::ARAContentType type,
                                                                                                   [[maybe_unused]] const ARA::ARAContentTimeRange* range)
{
    // Overriding doIsAudioSourceContentAvailable() requires overriding
    // doCreateAudioSourceContentReader() accordingly!
    jassertfalse;

    return nullptr;
}

b8 ARADocumentControllerSpecialisation::doIsAudioModificationContentAvailable ([[maybe_unused]] const ARA::PlugIn::AudioModification* audioModification,
                                                                                 [[maybe_unused]] ARA::ARAContentType type)
{
    return false;
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::doGetAudioModificationContentGrade ([[maybe_unused]] const ARA::PlugIn::AudioModification* audioModification,
                                                                                              [[maybe_unused]] ARA::ARAContentType type)
{
    // Overriding doIsAudioModificationContentAvailable() requires overriding
    // doGetAudioModificationContentGrade() accordingly!
    jassertfalse;

    return ARA::kARAContentGradeInitial;
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::doCreateAudioModificationContentReader ([[maybe_unused]] ARA::PlugIn::AudioModification* audioModification,
                                                                                                         [[maybe_unused]] ARA::ARAContentType type,
                                                                                                         [[maybe_unused]] const ARA::ARAContentTimeRange* range)
{
    // Overriding doIsAudioModificationContentAvailable() requires overriding
    // doCreateAudioModificationContentReader() accordingly!
    jassertfalse;

    return nullptr;
}

b8 ARADocumentControllerSpecialisation::doIsPlaybackRegionContentAvailable ([[maybe_unused]] const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                              [[maybe_unused]] ARA::ARAContentType type)
{
    return false;
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::doGetPlaybackRegionContentGrade ([[maybe_unused]] const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                           [[maybe_unused]] ARA::ARAContentType type)
{
    // Overriding doIsPlaybackRegionContentAvailable() requires overriding
    // doGetPlaybackRegionContentGrade() accordingly!
    jassertfalse;

    return ARA::kARAContentGradeInitial;
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::doCreatePlaybackRegionContentReader ([[maybe_unused]] ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                      [[maybe_unused]] ARA::ARAContentType type,
                                                                                                      [[maybe_unused]] const ARA::ARAContentTimeRange* range)
{
    // Overriding doIsPlaybackRegionContentAvailable() requires overriding
    // doCreatePlaybackRegionContentReader() accordingly!
    jassertfalse;

    return nullptr;
}

z0 ARADocumentControllerSpecialisation::doGetPlaybackRegionHeadAndTailTime ([[maybe_unused]] const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                              ARA::ARATimeDuration* headTime,
                                                                              ARA::ARATimeDuration* tailTime)
{
    *headTime = 0.0;
    *tailTime = 0.0;
}

b8 ARADocumentControllerSpecialisation::doIsAudioSourceContentAnalysisIncomplete ([[maybe_unused]] const ARA::PlugIn::AudioSource* audioSource,
                                                                                    [[maybe_unused]] ARA::ARAContentType type)
{
    return false;
}

z0 ARADocumentControllerSpecialisation::doRequestAudioSourceContentAnalysis ([[maybe_unused]] ARA::PlugIn::AudioSource* audioSource,
                                                                               [[maybe_unused]] std::vector<ARA::ARAContentType> const& contentTypes)
{
}

ARA::ARAInt32 ARADocumentControllerSpecialisation::doGetProcessingAlgorithmsCount() { return 0; }

const ARA::ARAProcessingAlgorithmProperties*
    ARADocumentControllerSpecialisation::doGetProcessingAlgorithmProperties ([[maybe_unused]] ARA::ARAInt32 algorithmIndex)
{
    return nullptr;
}

ARA::ARAInt32 ARADocumentControllerSpecialisation::doGetProcessingAlgorithmForAudioSource ([[maybe_unused]] const ARA::PlugIn::AudioSource* audioSource)
{
    // doGetProcessingAlgorithmForAudioSource() must be implemented if the supported
    // algorithm count is greater than zero.
    if (getDocumentController()->getProcessingAlgorithmsCount() > 0)
        jassertfalse;

    return 0;
}

z0 ARADocumentControllerSpecialisation::doRequestProcessingAlgorithmForAudioSource ([[maybe_unused]] ARA::PlugIn::AudioSource* audioSource,
                                                                                      [[maybe_unused]] ARA::ARAInt32 algorithmIndex)
{
    // doRequestProcessingAlgorithmForAudioSource() must be implemented if the supported
    // algorithm count is greater than zero.
    jassert (getDocumentController()->getProcessingAlgorithmsCount() <= 0);
}

} // namespace drx
