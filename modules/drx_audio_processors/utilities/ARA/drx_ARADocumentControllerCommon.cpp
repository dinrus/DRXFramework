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

class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    using ARA::PlugIn::DocumentController::DocumentController;

    template <typename Document_t = ARADocument>
    Document_t* getDocument() const noexcept { return ARA::PlugIn::DocumentController::getDocument<Document_t>(); }

    //==============================================================================
    /** @internal */
    virtual z0 internalNotifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource) = 0;

    /** @internal */
    virtual z0 internalNotifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource, f32 progress) = 0;

    /** @internal */
    virtual z0 internalNotifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource) = 0;

    /** @internal */
    virtual z0 internalDidUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource,
                                                               ARAAudioSource::ARAAnalysisProgressState state,
                                                               f32 progress) = 0;

    //==============================================================================
    /** @internal */
    virtual z0 internalNotifyAudioSourceContentChanged (ARAAudioSource* audioSource,
                                                          ARAContentUpdateScopes scopeFlags,
                                                          b8 notifyARAHost) = 0;

    /** @internal */
    virtual z0 internalNotifyAudioModificationContentChanged (ARAAudioModification* audioModification,
                                                                ARAContentUpdateScopes scopeFlags,
                                                                b8 notifyARAHost) = 0;

    /** @internal */
    virtual z0 internalNotifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion,
                                                             ARAContentUpdateScopes scopeFlags,
                                                             b8 notifyARAHost) = 0;

    friend class ARAPlaybackRegionReader;
};

} // namespace drx
