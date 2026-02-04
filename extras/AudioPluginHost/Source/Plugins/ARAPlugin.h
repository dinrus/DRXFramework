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

#pragma once

#include <drx_core/system/drx_TargetPlatform.h>

#if DRX_PLUGINHOST_ARA && (DRX_MAC || DRX_WINDOWS || DRX_LINUX)

#include <DrxHeader.h>

#include <ARA_API/ARAInterface.h>
#include <ARA_Library/Dispatch/ARAHostDispatch.h>

class FileAudioSource
{
    auto getAudioSourceProperties() const
    {
        auto properties = ARAHostModel::AudioSource::getEmptyProperties();
        properties.name = formatReader->getFile().getFullPathName().toRawUTF8();
        properties.persistentID = formatReader->getFile().getFullPathName().toRawUTF8();
        properties.sampleCount = formatReader->lengthInSamples;
        properties.sampleRate = formatReader->sampleRate;
        properties.channelCount = (i32) formatReader->numChannels;
        properties.merits64BitSamples = false;
        return properties;
    }

public:
    FileAudioSource (ARA::Host::DocumentController& dc, const drx::File& file)
        : formatReader ([&file]
          {
              auto result = rawToUniquePtr (WavAudioFormat().createMemoryMappedReader (file));
              result->mapEntireFile();
              return result;
          }()),
          audioSource (Converter::toHostRef (this), dc, getAudioSourceProperties())
    {
        audioSource.enableAudioSourceSamplesAccess (true);
    }

    b8 readAudioSamples (f32* const* buffers, z64 startSample, z64 numSamples)
    {
        // TODO: the ARA interface defines numSamples as z64. We should do multiple reads if necessary with the reader.
        if (numSamples > std::numeric_limits<i32>::max())
            return false;

        return formatReader->read (buffers, (i32) formatReader->numChannels, startSample, (i32) (numSamples));
    }

    b8 readAudioSamples (f64* const* buffers, z64 startSample, z64 numSamples)
    {
        ignoreUnused (buffers, startSample, numSamples);
        return false;
    }

    MemoryMappedAudioFormatReader& getFormatReader() const { return *formatReader; }

    auto getPluginRef() const { return audioSource.getPluginRef(); }

    auto& getSource() { return audioSource; }

    using Converter = ARAHostModel::ConversionFunctions<FileAudioSource*, ARA::ARAAudioSourceHostRef>;

private:
    std::unique_ptr<MemoryMappedAudioFormatReader> formatReader;
    ARAHostModel::AudioSource audioSource;
};

//==============================================================================
class MusicalContext
{
    auto getMusicalContextProperties() const
    {
        auto properties = ARAHostModel::MusicalContext::getEmptyProperties();
        properties.name = "MusicalContext";
        properties.orderIndex = 0;
        properties.color = nullptr;
        return properties;
    }

public:
    MusicalContext (ARA::Host::DocumentController& dc)
        : context (Converter::toHostRef (this), dc, getMusicalContextProperties())
    {
    }

    auto getPluginRef() const { return context.getPluginRef(); }

private:
    using Converter = ARAHostModel::ConversionFunctions<MusicalContext*, ARA::ARAMusicalContextHostRef>;

    ARAHostModel::MusicalContext context;
};

//==============================================================================
class RegionSequence
{
    auto getRegionSequenceProperties() const
    {
        auto properties = ARAHostModel::RegionSequence::getEmptyProperties();
        properties.name = name.toRawUTF8();
        properties.orderIndex = 0;
        properties.musicalContextRef = context.getPluginRef();
        properties.color = nullptr;
        return properties;
    }

public:
    RegionSequence (ARA::Host::DocumentController& dc, MusicalContext& contextIn, Txt nameIn)
        : context (contextIn),
          name (std::move (nameIn)),
          sequence (Converter::toHostRef (this), dc, getRegionSequenceProperties())
    {
    }

    auto& getMusicalContext() const { return context; }
    auto getPluginRef() const { return sequence.getPluginRef(); }

private:
    using Converter = ARAHostModel::ConversionFunctions<RegionSequence*, ARA::ARARegionSequenceHostRef>;

    MusicalContext& context;
    Txt name;
    ARAHostModel::RegionSequence sequence;
};

class AudioModification
{
    auto getProperties() const
    {
        auto properties = ARAHostModel::AudioModification::getEmptyProperties();
        properties.persistentID = "x";
        return properties;
    }

public:
    AudioModification (ARA::Host::DocumentController& dc, FileAudioSource& source)
        : modification (Converter::toHostRef (this), dc, source.getSource(), getProperties())
    {
    }

    auto& getModification() { return modification; }

private:
    using Converter = ARAHostModel::ConversionFunctions<AudioModification*, ARA::ARAAudioModificationHostRef>;

    ARAHostModel::AudioModification modification;
};

//==============================================================================
class PlaybackRegion
{
    auto getPlaybackRegionProperties() const
    {
        auto properties = ARAHostModel::PlaybackRegion::getEmptyProperties();
        properties.transformationFlags = ARA::kARAPlaybackTransformationNoChanges;
        properties.startInModificationTime = 0.0;
        const auto& formatReader = audioSource.getFormatReader();
        properties.durationInModificationTime = (f64) formatReader.lengthInSamples / formatReader.sampleRate;
        properties.startInPlaybackTime = 0.0;
        properties.durationInPlaybackTime = properties.durationInModificationTime;
        properties.musicalContextRef = sequence.getMusicalContext().getPluginRef();
        properties.regionSequenceRef = sequence.getPluginRef();

        properties.name = nullptr;
        properties.color = nullptr;
        return properties;
    }

public:
    PlaybackRegion (ARA::Host::DocumentController& dc,
                    RegionSequence& s,
                    AudioModification& m,
                    FileAudioSource& source)
        : sequence (s),
          audioSource (source),
          region (Converter::toHostRef (this), dc, m.getModification(), getPlaybackRegionProperties())
    {
        jassert (source.getPluginRef() == m.getModification().getAudioSource().getPluginRef());
    }

    auto& getPlaybackRegion() { return region; }

private:
    using Converter = ARAHostModel::ConversionFunctions<PlaybackRegion*, ARA::ARAPlaybackRegionHostRef>;

    RegionSequence& sequence;
    FileAudioSource& audioSource;
    ARAHostModel::PlaybackRegion region;
};

//==============================================================================
class AudioAccessController final : public ARA::Host::AudioAccessControllerInterface
{
public:
    ARA::ARAAudioReaderHostRef createAudioReaderForSource (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                                           b8 use64BitSamples) noexcept override
    {
        auto audioReader = std::make_unique<AudioReader> (audioSourceHostRef, use64BitSamples);
        auto audioReaderHostRef = Converter::toHostRef (audioReader.get());
        auto* readerPtr = audioReader.get();
        audioReaders.emplace (readerPtr, std::move (audioReader));
        return audioReaderHostRef;
    }

    b8 readAudioSamples (ARA::ARAAudioReaderHostRef readerRef,
                           ARA::ARASamplePosition samplePosition,
                           ARA::ARASampleCount samplesPerChannel,
                           uk const* buffers) noexcept override
    {
        const auto use64BitSamples = Converter::fromHostRef (readerRef)->use64Bit;
        auto* audioSource = FileAudioSource::Converter::fromHostRef (Converter::fromHostRef (readerRef)->sourceHostRef);

        if (use64BitSamples)
            return audioSource->readAudioSamples (
                reinterpret_cast<f64* const*> (buffers), samplePosition, samplesPerChannel);

        return audioSource->readAudioSamples (
            reinterpret_cast<f32* const*> (buffers), samplePosition, samplesPerChannel);
    }

    z0 destroyAudioReader (ARA::ARAAudioReaderHostRef readerRef) noexcept override
    {
        audioReaders.erase (Converter::fromHostRef (readerRef));
    }

private:
    struct AudioReader
    {
        AudioReader (ARA::ARAAudioSourceHostRef source, b8 use64BitSamples)
            : sourceHostRef (source), use64Bit (use64BitSamples)
        {
        }

        ARA::ARAAudioSourceHostRef sourceHostRef;
        b8 use64Bit;
    };

    using Converter = ARAHostModel::ConversionFunctions<AudioReader*, ARA::ARAAudioReaderHostRef>;

    std::map<AudioReader*, std::unique_ptr<AudioReader>> audioReaders;
};

class ArchivingController final : public ARA::Host::ArchivingControllerInterface
{
public:
    using ReaderConverter = ARAHostModel::ConversionFunctions<MemoryBlock*, ARA::ARAArchiveReaderHostRef>;
    using WriterConverter = ARAHostModel::ConversionFunctions<MemoryOutputStream*, ARA::ARAArchiveWriterHostRef>;

    ARA::ARASize getArchiveSize (ARA::ARAArchiveReaderHostRef archiveReaderHostRef) noexcept override
    {
        return (ARA::ARASize) ReaderConverter::fromHostRef (archiveReaderHostRef)->getSize();
    }

    b8 readBytesFromArchive (ARA::ARAArchiveReaderHostRef archiveReaderHostRef,
                               ARA::ARASize position,
                               ARA::ARASize length,
                               ARA::ARAByte* buffer) noexcept override
    {
        auto* archiveReader = ReaderConverter::fromHostRef (archiveReaderHostRef);

        if ((position + length) <= archiveReader->getSize())
        {
            std::memcpy (buffer, addBytesToPointer (archiveReader->getData(), position), length);
            return true;
        }

        return false;
    }

    b8 writeBytesToArchive (ARA::ARAArchiveWriterHostRef archiveWriterHostRef,
                              ARA::ARASize position,
                              ARA::ARASize length,
                              const ARA::ARAByte* buffer) noexcept override
    {
        auto* archiveWriter = WriterConverter::fromHostRef (archiveWriterHostRef);

        if (archiveWriter->setPosition ((z64) position) && archiveWriter->write (buffer, length))
            return true;

        return false;
    }

    z0 notifyDocumentArchivingProgress (f32 value) noexcept override { ignoreUnused (value); }

    z0 notifyDocumentUnarchivingProgress (f32 value) noexcept override { ignoreUnused (value); }

    ARA::ARAPersistentID getDocumentArchiveID (ARA::ARAArchiveReaderHostRef archiveReaderHostRef) noexcept override
    {
        ignoreUnused (archiveReaderHostRef);

        return nullptr;
    }
};

class ContentAccessController final : public ARA::Host::ContentAccessControllerInterface
{
public:
    using Converter = ARAHostModel::ConversionFunctions<ARA::ARAContentType, ARA::ARAContentReaderHostRef>;

    b8 isMusicalContextContentAvailable (ARA::ARAMusicalContextHostRef musicalContextHostRef,
                                           ARA::ARAContentType type) noexcept override
    {
        ignoreUnused (musicalContextHostRef);

        return (type == ARA::kARAContentTypeTempoEntries || type == ARA::kARAContentTypeBarSignatures);
    }

    ARA::ARAContentGrade getMusicalContextContentGrade (ARA::ARAMusicalContextHostRef musicalContextHostRef,
                                                        ARA::ARAContentType type) noexcept override
    {
        ignoreUnused (musicalContextHostRef, type);

        return ARA::kARAContentGradeInitial;
    }

    ARA::ARAContentReaderHostRef
        createMusicalContextContentReader (ARA::ARAMusicalContextHostRef musicalContextHostRef,
                                           ARA::ARAContentType type,
                                           const ARA::ARAContentTimeRange* range) noexcept override
    {
        ignoreUnused (musicalContextHostRef, range);

        return Converter::toHostRef (type);
    }

    b8 isAudioSourceContentAvailable (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                        ARA::ARAContentType type) noexcept override
    {
        ignoreUnused (audioSourceHostRef, type);

        return false;
    }

    ARA::ARAContentGrade getAudioSourceContentGrade (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                                     ARA::ARAContentType type) noexcept override
    {
        ignoreUnused (audioSourceHostRef, type);

        return 0;
    }

    ARA::ARAContentReaderHostRef
        createAudioSourceContentReader (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                        ARA::ARAContentType type,
                                        const ARA::ARAContentTimeRange* range) noexcept override
    {
        ignoreUnused (audioSourceHostRef, type, range);

        return nullptr;
    }

    ARA::ARAInt32 getContentReaderEventCount (ARA::ARAContentReaderHostRef contentReaderHostRef) noexcept override
    {
        const auto contentType = Converter::fromHostRef (contentReaderHostRef);

        if (contentType == ARA::kARAContentTypeTempoEntries || contentType == ARA::kARAContentTypeBarSignatures)
            return 2;

        return 0;
    }

    ukk getContentReaderDataForEvent (ARA::ARAContentReaderHostRef contentReaderHostRef,
                                              ARA::ARAInt32 eventIndex) noexcept override
    {
        if (Converter::fromHostRef (contentReaderHostRef) == ARA::kARAContentTypeTempoEntries)
        {
            if (eventIndex == 0)
            {
                tempoEntry.timePosition = 0.0;
                tempoEntry.quarterPosition = 0.0;
            }
            else if (eventIndex == 1)
            {
                tempoEntry.timePosition = 2.0;
                tempoEntry.quarterPosition = 4.0;
            }

            return &tempoEntry;
        }
        else if (Converter::fromHostRef (contentReaderHostRef) == ARA::kARAContentTypeBarSignatures)
        {
            if (eventIndex == 0)
            {
                barSignature.position = 0.0;
                barSignature.numerator = 4;
                barSignature.denominator = 4;
            }

            if (eventIndex == 1)
            {
                barSignature.position = 1.0;
                barSignature.numerator = 4;
                barSignature.denominator = 4;
            }

            return &barSignature;
        }

        jassertfalse;
        return nullptr;
    }

    z0 destroyContentReader (ARA::ARAContentReaderHostRef contentReaderHostRef) noexcept override
    {
        ignoreUnused (contentReaderHostRef);
    }

    ARA::ARAContentTempoEntry tempoEntry;
    ARA::ARAContentBarSignature barSignature;
};

class ModelUpdateController final : public ARA::Host::ModelUpdateControllerInterface
{
public:
    z0 notifyAudioSourceAnalysisProgress (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                            ARA::ARAAnalysisProgressState state,
                                            f32 value) noexcept override
    {
        ignoreUnused (audioSourceHostRef, state, value);
    }

    z0 notifyAudioSourceContentChanged (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                          const ARA::ARAContentTimeRange* range,
                                          ARA::ContentUpdateScopes scopeFlags) noexcept override
    {
        ignoreUnused (audioSourceHostRef, range, scopeFlags);
    }

    z0 notifyAudioModificationContentChanged (ARA::ARAAudioModificationHostRef audioModificationHostRef,
                                                const ARA::ARAContentTimeRange* range,
                                                ARA::ContentUpdateScopes scopeFlags) noexcept override
    {
        ignoreUnused (audioModificationHostRef, range, scopeFlags);
    }

    z0 notifyPlaybackRegionContentChanged (ARA::ARAPlaybackRegionHostRef playbackRegionHostRef,
                                             const ARA::ARAContentTimeRange* range,
                                             ARA::ContentUpdateScopes scopeFlags) noexcept override
    {
        ignoreUnused (playbackRegionHostRef, range, scopeFlags);
    }
};

class PlaybackController final : public ARA::Host::PlaybackControllerInterface
{
public:
    z0 requestStartPlayback() noexcept override {}
    z0 requestStopPlayback() noexcept override {}

    z0 requestSetPlaybackPosition (ARA::ARATimePosition timePosition) noexcept override
    {
        ignoreUnused (timePosition);
    }

    z0 requestSetCycleRange (ARA::ARATimePosition startTime, ARA::ARATimeDuration duration) noexcept override
    {
        ignoreUnused (startTime, duration);
    }

    z0 requestEnableCycle (b8 enable) noexcept override { ignoreUnused (enable); }
};

struct SimplePlayHead final : public drx::AudioPlayHead
{
    Optional<PositionInfo> getPosition() const override
    {
        PositionInfo result;
        result.setTimeInSamples (timeInSamples.load());
        result.setIsPlaying (isPlaying.load());
        return result;
    }

    std::atomic<z64> timeInSamples { 0 };
    std::atomic<b8> isPlaying { false };
};

struct HostPlaybackController
{
    virtual ~HostPlaybackController() = default;

    virtual z0 setPlaying (b8 isPlaying) = 0;
    virtual z0 goToStart() = 0;
    virtual File getAudioSource() const = 0;
    virtual z0 setAudioSource (File audioSourceFile) = 0;
    virtual z0 clearAudioSource() = 0;
};

class AudioSourceComponent final : public Component,
                                   public FileDragAndDropTarget,
                                   public ChangeListener
{
public:
    explicit AudioSourceComponent (HostPlaybackController& controller, drx::ChangeBroadcaster& bc)
        : hostPlaybackController (controller),
          broadcaster (bc),
          waveformComponent (*this)
    {
        audioSourceLabel.setText ("You can drag and drop .wav files here", NotificationType::dontSendNotification);

        addAndMakeVisible (audioSourceLabel);
        addAndMakeVisible (waveformComponent);

        playButton.setButtonText ("Play / Pause");
        playButton.onClick = [this]
        {
            isPlaying = ! isPlaying;
            hostPlaybackController.setPlaying (isPlaying);
        };

        goToStartButton.setButtonText ("Go to start");
        goToStartButton.onClick = [this] { hostPlaybackController.goToStart(); };

        addAndMakeVisible (goToStartButton);
        addAndMakeVisible (playButton);

        broadcaster.addChangeListener (this);

        update();
    }

    ~AudioSourceComponent() override
    {
        broadcaster.removeChangeListener (this);
    }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        update();
    }

    z0 resized() override
    {
        auto localBounds = getLocalBounds();
        auto buttonsArea = localBounds.removeFromBottom (40).reduced (5);
        auto waveformArea = localBounds.removeFromBottom (150).reduced (5);

        drx::FlexBox fb;
        fb.justifyContent = drx::FlexBox::JustifyContent::center;
        fb.alignContent = drx::FlexBox::AlignContent::center;

        fb.items = { drx::FlexItem (goToStartButton).withMinWidth (100.0f).withMinHeight ((f32) buttonsArea.getHeight()),
                     drx::FlexItem (playButton).withMinWidth (100.0f).withMinHeight ((f32) buttonsArea.getHeight()) };

        fb.performLayout (buttonsArea);

        waveformComponent.setBounds (waveformArea);

        audioSourceLabel.setBounds (localBounds);
    }

    b8 isInterestedInFileDrag (const StringArray& files) override
    {
        if (files.size() != 1)
            return false;

        if (files.getReference (0).endsWithIgnoreCase (".wav"))
            return true;

        return false;
    }

    z0 update()
    {
        const auto currentAudioSource = hostPlaybackController.getAudioSource();

        if (currentAudioSource.existsAsFile())
        {
            waveformComponent.setSource (currentAudioSource);
            audioSourceLabel.setText (currentAudioSource.getFullPathName(),
                                      NotificationType::dontSendNotification);
        }
        else
        {
            waveformComponent.clearSource();
            audioSourceLabel.setText ("You can drag and drop .wav files here", NotificationType::dontSendNotification);
        }
    }

    z0 filesDropped (const StringArray& files, i32, i32) override
    {
        hostPlaybackController.setAudioSource (files.getReference (0));
        update();
    }

private:
    class WaveformComponent final : public Component,
                                    public ChangeListener
    {
    public:
        WaveformComponent (AudioSourceComponent& p)
            : parent (p),
              thumbCache (7),
              audioThumb (128, formatManager, thumbCache)
        {
            setWantsKeyboardFocus (true);
            formatManager.registerBasicFormats();
            audioThumb.addChangeListener (this);
        }

        ~WaveformComponent() override
        {
            audioThumb.removeChangeListener (this);
        }

        z0 mouseDown (const MouseEvent&) override
        {
            isSelected = true;
            repaint();
        }

        z0 changeListenerCallback (ChangeBroadcaster*) override
        {
            repaint();
        }

        z0 paint (drx::Graphics& g) override
        {
            if (! isEmpty)
            {
                auto rect = getLocalBounds();

                const auto waveformColor = Colors::cadetblue;

                if (rect.getWidth() > 2)
                {
                    g.setColor (isSelected ? drx::Colors::yellow : drx::Colors::black);
                    g.drawRect (rect);
                    rect.reduce (1, 1);
                    g.setColor (waveformColor.darker (1.0f));
                    g.fillRect (rect);
                }

                g.setColor (Colors::cadetblue);
                audioThumb.drawChannels (g, rect, 0.0, audioThumb.getTotalLength(), 1.0f);
            }
        }

        z0 setSource (const File& source)
        {
            isEmpty = false;
            audioThumb.setSource (new FileInputSource (source));
        }

        z0 clearSource()
        {
            isEmpty = true;
            isSelected = false;
            audioThumb.clear();
        }

        b8 keyPressed (const KeyPress& key) override
        {
            if (isSelected && key == KeyPress::deleteKey)
            {
                parent.hostPlaybackController.clearAudioSource();
                return true;
            }

            return false;
        }

    private:
        AudioSourceComponent& parent;

        b8 isEmpty = true;
        b8 isSelected = false;
        AudioFormatManager formatManager;
        AudioThumbnailCache thumbCache;
        AudioThumbnail audioThumb;
    };

    HostPlaybackController& hostPlaybackController;
    drx::ChangeBroadcaster& broadcaster;
    Label audioSourceLabel;
    WaveformComponent waveformComponent;
    b8 isPlaying { false };
    TextButton playButton, goToStartButton;
};

class ARAPluginInstanceWrapper final : public AudioPluginInstance
{
public:
    class ARATestHost final : public HostPlaybackController,
                              public drx::ChangeBroadcaster
    {
    public:
        class Editor final : public AudioProcessorEditor
        {
        public:
            explicit Editor (ARATestHost& araTestHost)
                : AudioProcessorEditor (araTestHost.getAudioPluginInstance()),
                  audioSourceComponent (araTestHost, araTestHost)
            {
                audioSourceComponent.update();
                addAndMakeVisible (audioSourceComponent);
                setSize (512, 220);
            }

            ~Editor() override { getAudioProcessor()->editorBeingDeleted (this); }

            z0 resized() override { audioSourceComponent.setBounds (getLocalBounds()); }

        private:
            AudioSourceComponent audioSourceComponent;
        };

        explicit ARATestHost (ARAPluginInstanceWrapper& instanceIn)
            : instance (instanceIn)
        {
            if (instance.inner->getPluginDescription().hasARAExtension)
            {
                instance.inner->setPlayHead (&playHead);

                createARAFactoryAsync (*instance.inner, [this] (ARAFactoryWrapper araFactory)
                                                        {
                                                            init (std::move (araFactory));
                                                        });
            }
        }

        z0 init (ARAFactoryWrapper araFactory)
        {
            if (araFactory.get() != nullptr)
            {
                documentController = ARAHostDocumentController::create (std::move (araFactory),
                                                                        "AudioPluginHostDocument",
                                                                        std::make_unique<AudioAccessController>(),
                                                                        std::make_unique<ArchivingController>(),
                                                                        std::make_unique<ContentAccessController>(),
                                                                        std::make_unique<ModelUpdateController>(),
                                                                        std::make_unique<PlaybackController>());

                if (documentController != nullptr)
                {
                    const auto allRoles = ARA::kARAPlaybackRendererRole | ARA::kARAEditorRendererRole | ARA::kARAEditorViewRole;
                    const auto plugInExtensionInstance = documentController->bindDocumentToPluginInstance (*instance.inner,
                                                                                                           allRoles,
                                                                                                           allRoles);
                    playbackRenderer = plugInExtensionInstance.getPlaybackRendererInterface();
                    editorRenderer   = plugInExtensionInstance.getEditorRendererInterface();
                    synchronizeStateWithDocumentController();
                }
                else
                    jassertfalse;
            }
            else
                jassertfalse;
        }

        z0 getStateInformation (drx::MemoryBlock& b)
        {
            std::lock_guard<std::mutex> configurationLock (instance.innerMutex);

            if (context != nullptr)
                context->getStateInformation (b);
        }

        z0 setStateInformation (ukk d, i32 s)
        {
            {
                std::lock_guard<std::mutex> lock { contextUpdateSourceMutex };
                contextUpdateSource = ContextUpdateSource { d, s };
            }

            synchronise();
        }

        ~ARATestHost() override { instance.inner->releaseResources(); }

        z0 afterProcessBlock (i32 numSamples)
        {
            const auto isPlayingNow = isPlaying.load();
            playHead.isPlaying.store (isPlayingNow);

            if (isPlayingNow)
            {
                const auto currentAudioSourceLength = audioSourceLength.load();
                const auto currentPlayHeadPosition = playHead.timeInSamples.load();

                // Rudimentary attempt to not seek beyond our sample data, assuming a fairly stable numSamples
                // value. We should gain control over calling the AudioProcessorGraph's processBlock() calls so
                // that we can do sample precise looping.
                if (currentAudioSourceLength - currentPlayHeadPosition < numSamples)
                    playHead.timeInSamples.store (0);
                else
                    playHead.timeInSamples.fetch_add (numSamples);
            }

            if (goToStartSignal.exchange (false))
                playHead.timeInSamples.store (0);
        }

        File getAudioSource() const override
        {
            std::lock_guard<std::mutex> lock { instance.innerMutex };

            if (context != nullptr)
                return context->audioFile;

            return {};
        }

        z0 setAudioSource (File audioSourceFile) override
        {
            if (audioSourceFile.existsAsFile())
            {
                {
                    std::lock_guard<std::mutex> lock { contextUpdateSourceMutex };
                    contextUpdateSource = ContextUpdateSource (std::move (audioSourceFile));
                }

                synchronise();
            }
        }

        z0 clearAudioSource() override
        {
            {
                std::lock_guard<std::mutex> lock { contextUpdateSourceMutex };
                contextUpdateSource = ContextUpdateSource (ContextUpdateSource::Type::reset);
            }

            synchronise();
        }

        z0 setPlaying (b8 isPlayingIn) override { isPlaying.store (isPlayingIn); }

        z0 goToStart() override { goToStartSignal.store (true); }

        Editor* createEditor() { return new Editor (*this); }

        AudioPluginInstance& getAudioPluginInstance() { return instance; }

    private:
        /**  Use this to put the plugin in an unprepared state for the duration of adding and removing PlaybackRegions
             to and from Renderers.
        */
        class ScopedPluginDeactivator
        {
        public:
            explicit ScopedPluginDeactivator (ARAPluginInstanceWrapper& inst) : instance (inst)
            {
                if (instance.prepareToPlayParams.isValid)
                    instance.inner->releaseResources();
            }

            ~ScopedPluginDeactivator()
            {
                if (instance.prepareToPlayParams.isValid)
                    instance.inner->prepareToPlay (instance.prepareToPlayParams.sampleRate,
                                                   instance.prepareToPlayParams.samplesPerBlock);
            }

        private:
            ARAPluginInstanceWrapper& instance;

            DRX_DECLARE_NON_COPYABLE (ScopedPluginDeactivator)
        };

        class ContextUpdateSource
        {
        public:
            enum class Type
            {
                empty,
                audioSourceFile,
                stateInformation,
                reset
            };

            ContextUpdateSource() = default;

            explicit ContextUpdateSource (const File& file)
                : type (Type::audioSourceFile),
                  audioSourceFile (file)
            {
            }

            ContextUpdateSource (ukk d, i32 s)
                : type (Type::stateInformation),
                  stateInformation (d, (size_t) s)
            {
            }

            ContextUpdateSource (Type t) : type (t)
            {
                jassert (t == Type::reset);
            }

            Type getType() const { return type; }

            const File& getAudioSourceFile() const
            {
                jassert (type == Type::audioSourceFile);

                return audioSourceFile;
            }

            const MemoryBlock& getStateInformation() const
            {
                jassert (type == Type::stateInformation);

                return stateInformation;
            }

        private:
            Type type = Type::empty;

            File audioSourceFile;
            MemoryBlock stateInformation;
        };

        z0 synchronise()
        {
            const SpinLock::ScopedLockType scope (instance.innerProcessBlockFlag);
            std::lock_guard<std::mutex> configurationLock (instance.innerMutex);
            synchronizeStateWithDocumentController();
        }

        z0 synchronizeStateWithDocumentController()
        {
            b8 resetContext = false;

            auto newContext = [&]() -> std::unique_ptr<Context>
            {
                std::lock_guard<std::mutex> lock { contextUpdateSourceMutex };

                switch (contextUpdateSource.getType())
                {
                    case ContextUpdateSource::Type::empty:
                        return {};

                    case ContextUpdateSource::Type::audioSourceFile:
                        if (! (contextUpdateSource.getAudioSourceFile().existsAsFile()))
                            return {};

                        {
                            const ARAEditGuard editGuard (documentController->getDocumentController());
                            return std::make_unique<Context> (documentController->getDocumentController(),
                                                              contextUpdateSource.getAudioSourceFile());
                        }

                    case ContextUpdateSource::Type::stateInformation:
                        jassert (contextUpdateSource.getStateInformation().getSize() <= std::numeric_limits<i32>::max());

                        return Context::createFromStateInformation (documentController->getDocumentController(),
                                                                    contextUpdateSource.getStateInformation().getData(),
                                                                    (i32) contextUpdateSource.getStateInformation().getSize());

                    case ContextUpdateSource::Type::reset:
                        resetContext = true;
                        return {};
                }

                jassertfalse;
                return {};
            }();

            if (newContext != nullptr)
            {
                {
                    ScopedPluginDeactivator deactivator (instance);

                    context = std::move (newContext);
                    audioSourceLength.store (context->fileAudioSource.getFormatReader().lengthInSamples);

                    auto& region = context->playbackRegion.getPlaybackRegion();
                    playbackRenderer.add (region);
                    editorRenderer.add (region);
                }

                sendChangeMessage();
            }

            if (resetContext)
            {
                {
                    ScopedPluginDeactivator deactivator (instance);

                    context.reset();
                    audioSourceLength.store (0);
                }

                sendChangeMessage();
            }
        }

        struct Context
        {
            Context (ARA::Host::DocumentController& dc, const File& audioFileIn)
                : audioFile (audioFileIn),
                  musicalContext    (dc),
                  regionSequence    (dc, musicalContext, "track 1"),
                  fileAudioSource   (dc, audioFile),
                  audioModification (dc, fileAudioSource),
                  playbackRegion    (dc, regionSequence, audioModification, fileAudioSource)
            {
            }

            static std::unique_ptr<Context> createFromStateInformation (ARA::Host::DocumentController& dc, ukk d, i32 s)
            {
                if (auto xml = getXmlFromBinary (d, s))
                {
                    if (xml->hasTagName (xmlRootTag))
                    {
                        File file { xml->getStringAttribute (xmlAudioFileAttrib) };

                        if (file.existsAsFile())
                            return std::make_unique<Context> (dc, std::move (file));
                    }
                }

                return {};
            }

            z0 getStateInformation (drx::MemoryBlock& b)
            {
                XmlElement root { xmlRootTag };
                root.setAttribute (xmlAudioFileAttrib, audioFile.getFullPathName());
                copyXmlToBinary (root, b);
            }

            const static Identifier xmlRootTag;
            const static Identifier xmlAudioFileAttrib;

            File audioFile;

            MusicalContext musicalContext;
            RegionSequence regionSequence;
            FileAudioSource fileAudioSource;
            AudioModification audioModification;
            PlaybackRegion playbackRegion;
        };

        SimplePlayHead playHead;
        ARAPluginInstanceWrapper& instance;

        std::unique_ptr<ARAHostDocumentController> documentController;
        ARAHostModel::PlaybackRendererInterface playbackRenderer;
        ARAHostModel::EditorRendererInterface editorRenderer;

        std::unique_ptr<Context> context;

        mutable std::mutex contextUpdateSourceMutex;
        ContextUpdateSource contextUpdateSource;

        std::atomic<b8> isPlaying { false };
        std::atomic<b8> goToStartSignal { false };
        std::atomic<z64> audioSourceLength { 0 };
    };

    explicit ARAPluginInstanceWrapper (std::unique_ptr<AudioPluginInstance> innerIn)
        : inner (std::move (innerIn)), araHost (*this)
    {
        jassert (inner != nullptr);

        for (auto isInput : { true, false })
            matchBuses (isInput);

        setBusesLayout (inner->getBusesLayout());
    }

    //==============================================================================
    AudioProcessorEditor* createARAHostEditor() { return araHost.createEditor(); }

    //==============================================================================
    const Txt getName() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getName();
    }

    StringArray getAlternateDisplayNames() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getAlternateDisplayNames();
    }

    f64 getTailLengthSeconds() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getTailLengthSeconds();
    }

    b8 acceptsMidi() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->acceptsMidi();
    }

    b8 producesMidi() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->producesMidi();
    }

    AudioProcessorEditor* createEditor() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->createEditorIfNeeded();
    }

    b8 hasEditor() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->hasEditor();
    }

    i32 getNumPrograms() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getNumPrograms();
    }

    i32 getCurrentProgram() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getCurrentProgram();
    }

    z0 setCurrentProgram (i32 i) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->setCurrentProgram (i);
    }

    const Txt getProgramName (i32 i) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getProgramName (i);
    }

    z0 changeProgramName (i32 i, const Txt& n) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->changeProgramName (i, n);
    }

    z0 getStateInformation (drx::MemoryBlock& b) override
    {
        XmlElement state ("ARAPluginInstanceWrapperState");

        {
            MemoryBlock m;
            araHost.getStateInformation (m);
            state.createNewChildElement ("host")->addTextElement (m.toBase64Encoding());
        }

        {
            std::lock_guard<std::mutex> lock (innerMutex);

            MemoryBlock m;
            inner->getStateInformation (m);
            state.createNewChildElement ("plugin")->addTextElement (m.toBase64Encoding());
        }

        copyXmlToBinary (state, b);
    }

    z0 setStateInformation (ukk d, i32 s) override
    {
        if (auto xml = getXmlFromBinary (d, s))
        {
            if (xml->hasTagName ("ARAPluginInstanceWrapperState"))
            {
                if (auto* hostState = xml->getChildByName ("host"))
                {
                    MemoryBlock m;
                    m.fromBase64Encoding (hostState->getAllSubText());
                    jassert (m.getSize() <= std::numeric_limits<i32>::max());
                    araHost.setStateInformation (m.getData(), (i32) m.getSize());
                }

                if (auto* pluginState = xml->getChildByName ("plugin"))
                {
                    std::lock_guard<std::mutex> lock (innerMutex);

                    MemoryBlock m;
                    m.fromBase64Encoding (pluginState->getAllSubText());
                    jassert (m.getSize() <= std::numeric_limits<i32>::max());
                    inner->setStateInformation (m.getData(), (i32) m.getSize());
                }
            }
        }
    }

    z0 getCurrentProgramStateInformation (drx::MemoryBlock& b) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->getCurrentProgramStateInformation (b);
    }

    z0 setCurrentProgramStateInformation (ukk d, i32 s) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->setCurrentProgramStateInformation (d, s);
    }

    z0 prepareToPlay (f64 sr, i32 bs) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->setRateAndBufferSizeDetails (sr, bs);
        inner->prepareToPlay (sr, bs);
        prepareToPlayParams = { sr, bs };
    }

    z0 releaseResources() override { inner->releaseResources(); }

    z0 memoryWarningReceived() override { inner->memoryWarningReceived(); }

    z0 processBlock (AudioBuffer<f32>& a, MidiBuffer& m) override
    {
        const SpinLock::ScopedTryLockType scope (innerProcessBlockFlag);

        if (! scope.isLocked())
            return;

        inner->processBlock (a, m);
        araHost.afterProcessBlock (a.getNumSamples());
    }

    z0 processBlock (AudioBuffer<f64>& a, MidiBuffer& m) override
    {
        const SpinLock::ScopedTryLockType scope (innerProcessBlockFlag);

        if (! scope.isLocked())
            return;

        inner->processBlock (a, m);
        araHost.afterProcessBlock (a.getNumSamples());
    }

    z0 processBlockBypassed (AudioBuffer<f32>& a, MidiBuffer& m) override
    {
        const SpinLock::ScopedTryLockType scope (innerProcessBlockFlag);

        if (! scope.isLocked())
            return;

        inner->processBlockBypassed (a, m);
        araHost.afterProcessBlock (a.getNumSamples());
    }

    z0 processBlockBypassed (AudioBuffer<f64>& a, MidiBuffer& m) override
    {
        const SpinLock::ScopedTryLockType scope (innerProcessBlockFlag);

        if (! scope.isLocked())
            return;

        inner->processBlockBypassed (a, m);
        araHost.afterProcessBlock (a.getNumSamples());
    }

    b8 supportsDoublePrecisionProcessing() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->supportsDoublePrecisionProcessing();
    }

    b8 supportsMPE() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->supportsMPE();
    }

    b8 isMidiEffect() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->isMidiEffect();
    }

    z0 reset() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->reset();
    }

    z0 setNonRealtime (b8 b) noexcept override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->setNonRealtime (b);
    }

    z0 refreshParameterList() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->refreshParameterList();
    }

    z0 numChannelsChanged() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->numChannelsChanged();
    }

    z0 numBusesChanged() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->numBusesChanged();
    }

    z0 processorLayoutsChanged() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->processorLayoutsChanged();
    }

    z0 setPlayHead (AudioPlayHead* p) override { ignoreUnused (p); }

    z0 updateTrackProperties (const TrackProperties& p) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->updateTrackProperties (p);
    }

    b8 isBusesLayoutSupported (const BusesLayout& layout) const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->checkBusesLayoutSupported (layout);
    }

    b8 canAddBus (b8) const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return true;
    }
    b8 canRemoveBus (b8) const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return true;
    }

    //==============================================================================
    z0 fillInPluginDescription (PluginDescription& description) const override
    {
        return inner->fillInPluginDescription (description);
    }

private:
    z0 matchBuses (b8 isInput)
    {
        const auto inBuses = inner->getBusCount (isInput);

        while (getBusCount (isInput) < inBuses)
            addBus (isInput);

        while (inBuses < getBusCount (isInput))
            removeBus (isInput);
    }

    // Used for mutual exclusion between the audio and other threads
    SpinLock innerProcessBlockFlag;

    // Used for mutual exclusion on non-audio threads
    mutable std::mutex innerMutex;

    std::unique_ptr<AudioPluginInstance> inner;

    ARATestHost araHost;

    struct PrepareToPlayParams
    {
        PrepareToPlayParams() : isValid (false) {}

        PrepareToPlayParams (f64 sampleRateIn, i32 samplesPerBlockIn)
            : isValid (true), sampleRate (sampleRateIn), samplesPerBlock (samplesPerBlockIn)
        {
        }

        b8 isValid;
        f64 sampleRate;
        i32 samplesPerBlock;
    };

    PrepareToPlayParams prepareToPlayParams;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginInstanceWrapper)
};
#endif
