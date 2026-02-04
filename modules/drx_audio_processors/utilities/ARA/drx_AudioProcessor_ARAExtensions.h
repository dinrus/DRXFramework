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

namespace drx
{

class AudioProcessor;
class ARAPlaybackRenderer;
class ARAEditorRenderer;
class ARAEditorView;

//==============================================================================
/** Extension class meant to be subclassed by the plugin's implementation of @see AudioProcessor.

    Subclassing AudioProcessorARAExtension allows access to the three possible plugin instance
    roles as defined by the ARA SDK. Hosts can assign any subset of roles to each plugin instance.

    @tags{ARA}
*/
class DRX_API  AudioProcessorARAExtension  : public ARA::PlugIn::PlugInExtension
{
public:
    AudioProcessorARAExtension() = default;

    //==============================================================================
    /** Returns the result of ARA::PlugIn::PlugInExtension::getPlaybackRenderer() with the pointer
        cast to ARAPlaybackRenderer*.

        If you have overridden ARADocumentControllerSpecialisation::doCreatePlaybackRenderer(),
        then you can use the template parameter to cast the pointers to your subclass of
        ARAPlaybackRenderer.
    */
    template <typename PlaybackRenderer_t = ARAPlaybackRenderer>
    PlaybackRenderer_t* getPlaybackRenderer() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getPlaybackRenderer<PlaybackRenderer_t>();
    }

    /** Returns the result of ARA::PlugIn::PlugInExtension::getEditorRenderer() with the pointer
        cast to ARAEditorRenderer*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateEditorRenderer(),
        then you can use the template parameter to cast the pointers to your subclass of
        ARAEditorRenderer.
    */
    template <typename EditorRenderer_t = ARAEditorRenderer>
    EditorRenderer_t* getEditorRenderer() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getEditorRenderer<EditorRenderer_t>();
    }

    /** Returns the result of ARA::PlugIn::PlugInExtension::getEditorView() with the pointer
        cast to ARAEditorView*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateEditorView(),
        then you can use the template parameter to cast the pointers to your subclass of
        ARAEditorView.
    */
    template <typename EditorView_t = ARAEditorView>
    EditorView_t* getEditorView() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getEditorView<EditorView_t>();
    }

    //==============================================================================
    /** Возвращает true, если plugin instance fulfills the ARAPlaybackRenderer role. */
    b8 isPlaybackRenderer() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getPlaybackRenderer() != nullptr;
    }

    /** Возвращает true, если plugin instance fulfills the ARAEditorRenderer role. */
    b8 isEditorRenderer() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getEditorRenderer() != nullptr;
    }

    /** Возвращает true, если plugin instance fulfills the ARAEditorView role. */
    b8 isEditorView() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getEditorView() != nullptr;
    }

    //==============================================================================
#if ARA_VALIDATE_API_CALLS
    b8 isPrepared { false };
#endif

protected:
    /** Implementation helper for AudioProcessor::getTailLengthSeconds().

        If bound to ARA, this traverses the instance roles to retrieve the respective tail time
        and returns true. Otherwise returns false and leaves tailLength unmodified.
    */
    b8 getTailLengthSecondsForARA (f64& tailLength) const;

    /** Implementation helper for AudioProcessor::prepareToPlay().

        If bound to ARA, this traverses the instance roles to prepare them for play and returns
        true. Otherwise returns false and does nothing.
    */
    b8 prepareToPlayForARA (f64 sampleRate,
                              i32 samplesPerBlock,
                              i32 numChannels,
                              AudioProcessor::ProcessingPrecision precision);

    /** Implementation helper for AudioProcessor::releaseResources().

        If bound to ARA, this traverses the instance roles to let them release resources and returns
        true. Otherwise returns false and does nothing.
    */
    b8 releaseResourcesForARA();

    /** Implementation helper for AudioProcessor::processBlock().

        If bound to ARA, this traverses the instance roles to let them process the block and returns
        true. Otherwise returns false and does nothing.

        Use this overload if your rendering code already has a current positionInfo available.
    */
    b8 processBlockForARA (AudioBuffer<f32>& buffer,
                             AudioProcessor::Realtime realtime,
                             const AudioPlayHead::PositionInfo& positionInfo);

    /** Implementation helper for AudioProcessor::processBlock().

        If bound to ARA, this traverses the instance roles to let them process the block and returns
        true. Otherwise returns false and does nothing.

        Use this overload if your rendering code does not have a current positionInfo available.
    */
    b8 processBlockForARA (AudioBuffer<f32>& buffer, AudioProcessor::Realtime isNonRealtime, AudioPlayHead* playhead);

    //==============================================================================
    /** Optional hook for derived classes to perform any additional initialization that may be needed.

        If overriding this, make sure you call the base class implementation from your override.
    */
    z0 didBindToARA() noexcept override;

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorARAExtension)
};

//==============================================================================
/** Extension class meant to be subclassed by the plugin's implementation of @see AudioProcessorEditor.

    Subclassing AudioProcessorARAExtension allows access to the ARAEditorView instance role as
    described by the ARA SDK.

    @tags{ARA}
*/
class DRX_API  AudioProcessorEditorARAExtension
{
public:
    /** Constructor. */
    explicit AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor);

    /** \copydoc AudioProcessorARAExtension::getEditorView */
    template <typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept
    {
        return (this->araProcessorExtension != nullptr) ? this->araProcessorExtension->getEditorView<EditorView_t>()
                                                        : nullptr;
    }

    /** \copydoc AudioProcessorARAExtension::isEditorView */
    b8 isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

protected:
    /** Destructor. */
    ~AudioProcessorEditorARAExtension();

private:
    AudioProcessorARAExtension* araProcessorExtension;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorEditorARAExtension)
};

} // namespace drx
