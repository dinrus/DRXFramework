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

#if (DRX_PLUGINHOST_ARA && (DRX_PLUGINHOST_VST3 || DRX_PLUGINHOST_AU) && (DRX_MAC || DRX_WINDOWS || DRX_LINUX))

#include "drx_ARAHosting.h"

#include <ARA_Library/Dispatch/ARAHostDispatch.cpp>

namespace drx
{
struct ARAEditGuardState
{
public:
    /*  Возвращает true, если this controller wasn't previously present. */
    b8 add (ARA::Host::DocumentController& dc)
    {
        const std::lock_guard<std::mutex> lock (mutex);
        return ++counts[&dc] == 1;
    }

    /*  Возвращает true, если this controller is no longer present. */
    b8 remove (ARA::Host::DocumentController& dc)
    {
        const std::lock_guard<std::mutex> lock (mutex);
        return --counts[&dc] == 0;
    }

private:
    std::map<ARA::Host::DocumentController*, i32> counts;
    std::mutex mutex;
};

static ARAEditGuardState editGuardState;

ARAEditGuard::ARAEditGuard (ARA::Host::DocumentController& dcIn) : dc (dcIn)
{
    if (editGuardState.add (dc))
        dc.beginEditing();
}

ARAEditGuard::~ARAEditGuard()
{
    if (editGuardState.remove (dc))
        dc.endEditing();
}

//==============================================================================
namespace ARAHostModel
{

//==============================================================================
AudioSource::AudioSource (ARA::ARAAudioSourceHostRef hostRef,
                          ARA::Host::DocumentController& dc,
                          const ARA::ARAAudioSourceProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createAudioSource (hostRef, &props);
                        }())
{
}

z0 AudioSource::update (const ARA::ARAAudioSourceProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    getDocumentController().updateAudioSourceProperties (getPluginRef(), &props);
}

z0 AudioSource::enableAudioSourceSamplesAccess (b8 x)
{
    const ARAEditGuard guard (getDocumentController());
    getDocumentController().enableAudioSourceSamplesAccess (getPluginRef(), x);
}

z0 AudioSource::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyAudioSource (ptr);
}

//==============================================================================
AudioModification::AudioModification (ARA::ARAAudioModificationHostRef hostRef,
                                      ARA::Host::DocumentController& dc,
                                      AudioSource& s,
                                      const ARA::ARAAudioModificationProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createAudioModification (s.getPluginRef(), hostRef, &props);
                        }()),
      source (s)
{
}

z0 AudioModification::update (const ARA::ARAAudioModificationProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    getDocumentController().updateAudioModificationProperties (getPluginRef(), &props);
}

z0 AudioModification::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyAudioModification (ptr);
}

//==============================================================================
class PlaybackRegion::Impl  : public ManagedARAHandle<Impl, ARA::ARAPlaybackRegionRef>,
                              public DeletionListener
{
public:
    Impl (ARA::ARAPlaybackRegionHostRef hostRef,
          ARA::Host::DocumentController& dc,
          AudioModification& m,
          const ARA::ARAPlaybackRegionProperties& props);

    ~Impl() override
    {
        for (const auto& l : listeners)
            l->removeListener (*this);
    }

    /*  Updates the state of the corresponding %ARA model object.

        Places the DocumentController in editable state.

        You can use getEmptyProperties() to acquire a properties struct where the `structSize`
        field has already been correctly set.
    */
    z0 update (const ARA::ARAPlaybackRegionProperties& props);

    auto& getAudioModification() const { return modification; }

    static z0 destroy (ARA::Host::DocumentController&, Ptr);

    z0 addListener    (DeletionListener& l)                   { listeners.insert (&l); }
    z0 removeListener (DeletionListener& l) noexcept override { listeners.erase (&l); }

private:
    AudioModification* modification = nullptr;
    std::unordered_set<DeletionListener*> listeners;
};

PlaybackRegion::Impl::Impl (ARA::ARAPlaybackRegionHostRef hostRef,
                            ARA::Host::DocumentController& dc,
                            AudioModification& m,
                            const ARA::ARAPlaybackRegionProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createPlaybackRegion (m.getPluginRef(), hostRef, &props);
                        }()),
      modification (&m)
{
}

PlaybackRegion::~PlaybackRegion() = default;

z0 PlaybackRegion::Impl::update (const ARA::ARAPlaybackRegionProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    getDocumentController().updatePlaybackRegionProperties (getPluginRef(), &props);
}

z0 PlaybackRegion::Impl::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyPlaybackRegion (ptr);
}

PlaybackRegion::PlaybackRegion (ARA::ARAPlaybackRegionHostRef hostRef,
                                ARA::Host::DocumentController& dc,
                                AudioModification& m,
                                const ARA::ARAPlaybackRegionProperties& props)
    : impl (std::make_unique<Impl> (hostRef, dc, m, props))
{
}

z0 PlaybackRegion::update (const ARA::ARAPlaybackRegionProperties& props) { impl->update (props); }

z0 PlaybackRegion::addListener (DeletionListener& x) { impl->addListener (x); }

auto& PlaybackRegion::getAudioModification() const { return impl->getAudioModification(); }

ARA::ARAPlaybackRegionRef PlaybackRegion::getPluginRef() const noexcept { return impl->getPluginRef(); }

DeletionListener& PlaybackRegion::getDeletionListener() const noexcept { return *impl.get(); }

//==============================================================================
MusicalContext::MusicalContext (ARA::ARAMusicalContextHostRef hostRef,
                                ARA::Host::DocumentController& dc,
                                const ARA::ARAMusicalContextProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createMusicalContext (hostRef, &props);
                        }())
{
}

z0 MusicalContext::update (const ARA::ARAMusicalContextProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    return getDocumentController().updateMusicalContextProperties (getPluginRef(), &props);
}

z0 MusicalContext::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyMusicalContext (ptr);
}

//==============================================================================
RegionSequence::RegionSequence (ARA::ARARegionSequenceHostRef hostRef,
                                ARA::Host::DocumentController& dc,
                                const ARA::ARARegionSequenceProperties& props)
    : ManagedARAHandle (dc, [&]
                        {
                            const ARAEditGuard guard (dc);
                            return dc.createRegionSequence (hostRef, &props);
                        }())
{
}

z0 RegionSequence::update (const ARA::ARARegionSequenceProperties& props)
{
    const ARAEditGuard guard (getDocumentController());
    return getDocumentController().updateRegionSequenceProperties (getPluginRef(), &props);
}

z0 RegionSequence::destroy (ARA::Host::DocumentController& dc, Ptr ptr)
{
    dc.destroyRegionSequence (ptr);
}

//==============================================================================
PlaybackRendererInterface PlugInExtensionInstance::getPlaybackRendererInterface() const
{
    if (instance != nullptr)
        return PlaybackRendererInterface (instance->playbackRendererRef, instance->playbackRendererInterface);

    return {};
}

EditorRendererInterface PlugInExtensionInstance::getEditorRendererInterface() const
{
    if (instance != nullptr)
        return EditorRendererInterface (instance->editorRendererRef, instance->editorRendererInterface);

    return {};
}

} // namespace ARAHostModel

//==============================================================================
class ARAHostDocumentController::Impl
{
public:
    Impl (ARAFactoryWrapper araFactoryIn,
          std::unique_ptr<ARA::Host::DocumentControllerHostInstance>&& dcHostInstanceIn,
          const ARA::ARADocumentControllerInstance* documentControllerInstance,
          std::unique_ptr<ARA::Host::AudioAccessControllerInterface>&& audioAccessControllerIn,
          std::unique_ptr<ARA::Host::ArchivingControllerInterface>&& archivingControllerIn,
          std::unique_ptr<ARA::Host::ContentAccessControllerInterface>&& contentAccessControllerIn,
          std::unique_ptr<ARA::Host::ModelUpdateControllerInterface>&& modelUpdateControllerIn,
          std::unique_ptr<ARA::Host::PlaybackControllerInterface>&& playbackControllerIn)
        : araFactory (std::move (araFactoryIn)),
          audioAccessController (std::move (audioAccessControllerIn)),
          archivingController (std::move (archivingControllerIn)),
          contentAccessController (std::move (contentAccessControllerIn)),
          modelUpdateController (std::move (modelUpdateControllerIn)),
          playbackController (std::move (playbackControllerIn)),
          dcHostInstance (std::move (dcHostInstanceIn)),
          documentController (documentControllerInstance)
    {
    }

    ~Impl()
    {
        documentController.destroyDocumentController();
    }

    static std::unique_ptr<Impl>
        createImpl (ARAFactoryWrapper araFactory,
                    const Txt& documentName,
                    std::unique_ptr<ARA::Host::AudioAccessControllerInterface>&& audioAccessController,
                    std::unique_ptr<ARA::Host::ArchivingControllerInterface>&& archivingController,
                    std::unique_ptr<ARA::Host::ContentAccessControllerInterface>&& contentAccessController,
                    std::unique_ptr<ARA::Host::ModelUpdateControllerInterface>&& modelUpdateController,
                    std::unique_ptr<ARA::Host::PlaybackControllerInterface>&& playbackController)
    {
        std::unique_ptr<ARA::Host::DocumentControllerHostInstance> dcHostInstance =
            std::make_unique<ARA::Host::DocumentControllerHostInstance> (audioAccessController.get(),
                                                                         archivingController.get(),
                                                                         contentAccessController.get(),
                                                                         modelUpdateController.get(),
                                                                         playbackController.get());

        const auto documentProperties = makeARASizedStruct (&ARA::ARADocumentProperties::name, documentName.toRawUTF8());

        if (auto* dci = araFactory.get()->createDocumentControllerWithDocument (dcHostInstance.get(), &documentProperties))
            return std::make_unique<Impl> (std::move (araFactory),
                                           std::move (dcHostInstance),
                                           dci,
                                           std::move (audioAccessController),
                                           std::move (archivingController),
                                           std::move (contentAccessController),
                                           std::move (modelUpdateController),
                                           std::move (playbackController));

        return {};
    }

    ARAHostModel::PlugInExtensionInstance bindDocumentToPluginInstance (AudioPluginInstance& instance,
                                                                        ARA::ARAPlugInInstanceRoleFlags knownRoles,
                                                                        ARA::ARAPlugInInstanceRoleFlags assignedRoles)
    {

        const auto makeVisitor = [] (auto vst3Fn, auto auFn)
        {
            using Vst3Fn = decltype (vst3Fn);
            using AuFn = decltype (auFn);

            struct Visitor final : public ExtensionsVisitor, Vst3Fn, AuFn
            {
                explicit Visitor (Vst3Fn vst3Fn, AuFn auFn) : Vst3Fn (std::move (vst3Fn)), AuFn (std::move (auFn)) {}
                z0 visitVST3Client (const VST3Client& x) override { Vst3Fn::operator() (x); }
                z0 visitAudioUnitClient (const AudioUnitClient& x) override { AuFn::operator() (x); }
            };

            return Visitor { std::move (vst3Fn), std::move (auFn) };
        };

        const ARA::ARAPlugInExtensionInstance* pei = nullptr;
        auto visitor = makeVisitor ([this, &pei, knownRoles, assignedRoles] (const ExtensionsVisitor::VST3Client& vst3Client)
                                    {
                                        auto* iComponentPtr = vst3Client.getIComponentPtr();
                                        VSTComSmartPtr<ARA::IPlugInEntryPoint2> araEntryPoint;

                                        if (araEntryPoint.loadFrom (iComponentPtr))
                                            pei = araEntryPoint->bindToDocumentControllerWithRoles (documentController.getRef(), knownRoles, assignedRoles);
                                    },
                                   #if DRX_PLUGINHOST_AU && DRX_MAC
                                    [this, &pei, knownRoles, assignedRoles] (const ExtensionsVisitor::AudioUnitClient& auClient)
                                    {
                                        auto audioUnit = auClient.getAudioUnitHandle();
                                        auto propertySize = (UInt32) sizeof (ARA::ARAAudioUnitPlugInExtensionBinding);
                                        const auto expectedPropertySize = propertySize;
                                        ARA::ARAAudioUnitPlugInExtensionBinding audioUnitBinding { ARA::kARAAudioUnitMagic,
                                                                                                   documentController.getRef(),
                                                                                                   nullptr,
                                                                                                   knownRoles,
                                                                                                   assignedRoles };

                                        auto status = AudioUnitGetProperty (audioUnit,
                                                                            ARA::kAudioUnitProperty_ARAPlugInExtensionBindingWithRoles,
                                                                            kAudioUnitScope_Global,
                                                                            0,
                                                                            &audioUnitBinding,
                                                                            &propertySize);

                                        if (status == noErr
                                            && propertySize == expectedPropertySize
                                            && audioUnitBinding.inOutMagicNumber == ARA::kARAAudioUnitMagic
                                            && audioUnitBinding.inDocumentControllerRef == documentController.getRef()
                                            && audioUnitBinding.outPlugInExtension != nullptr)
                                        {
                                            pei = audioUnitBinding.outPlugInExtension;
                                        }
                                        else
                                            jassertfalse;
                                    }
                                   #else
                                    [] (const auto&) {}
                                   #endif
                                    );

        instance.getExtensions (visitor);
        return ARAHostModel::PlugInExtensionInstance { pei };
    }

    auto& getDocumentController()       { return documentController; }

private:
    ARAFactoryWrapper araFactory;

    std::unique_ptr<ARA::Host::AudioAccessControllerInterface>   audioAccessController;
    std::unique_ptr<ARA::Host::ArchivingControllerInterface>     archivingController;
    std::unique_ptr<ARA::Host::ContentAccessControllerInterface> contentAccessController;
    std::unique_ptr<ARA::Host::ModelUpdateControllerInterface>   modelUpdateController;
    std::unique_ptr<ARA::Host::PlaybackControllerInterface>      playbackController;

    std::unique_ptr<ARA::Host::DocumentControllerHostInstance> dcHostInstance;
    ARA::Host::DocumentController documentController;
};

ARAHostDocumentController::ARAHostDocumentController (std::unique_ptr<Impl>&& implIn)
    : impl { std::move (implIn) }
{}

std::unique_ptr<ARAHostDocumentController> ARAHostDocumentController::create (ARAFactoryWrapper factory,
                                                                              const Txt& documentName,
                                                                              std::unique_ptr<ARA::Host::AudioAccessControllerInterface> audioAccessController,
                                                                              std::unique_ptr<ARA::Host::ArchivingControllerInterface> archivingController,
                                                                              std::unique_ptr<ARA::Host::ContentAccessControllerInterface> contentAccessController,
                                                                              std::unique_ptr<ARA::Host::ModelUpdateControllerInterface> modelUpdateController,
                                                                              std::unique_ptr<ARA::Host::PlaybackControllerInterface> playbackController)
{
    if (auto impl = Impl::createImpl (std::move (factory),
                                      documentName,
                                      std::move (audioAccessController),
                                      std::move (archivingController),
                                      std::move (contentAccessController),
                                      std::move (modelUpdateController),
                                      std::move (playbackController)))
    {
        return rawToUniquePtr (new ARAHostDocumentController (std::move (impl)));
    }

    return {};
}

ARAHostDocumentController::~ARAHostDocumentController() = default;

ARA::Host::DocumentController& ARAHostDocumentController::getDocumentController() const
{
    return impl->getDocumentController();
}

ARAHostModel::PlugInExtensionInstance ARAHostDocumentController::bindDocumentToPluginInstance (AudioPluginInstance& instance,
                                                                                               ARA::ARAPlugInInstanceRoleFlags knownRoles,
                                                                                               ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    return impl->bindDocumentToPluginInstance (instance, knownRoles, assignedRoles);
}

z0 createARAFactoryAsync (AudioPluginInstance& instance, std::function<z0 (ARAFactoryWrapper)> cb)
{
    if (! instance.getPluginDescription().hasARAExtension)
        cb (ARAFactoryWrapper{});

    struct Extensions final : public ExtensionsVisitor
    {
        Extensions (std::function<z0 (ARAFactoryWrapper)> callbackIn)
            : callback (std::move (callbackIn))
        {}

        z0 visitARAClient (const ARAClient& araClient) override
        {
            araClient.createARAFactoryAsync (std::move (callback));
        }

        std::function<z0 (ARAFactoryWrapper)> callback;
    };

    Extensions extensions { std::move (cb) };
    instance.getExtensions (extensions);
}

} // namespace drx

#endif
