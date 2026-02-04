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
/**
    An AudioSource that mixes together the output of a set of other AudioSources.

    Input sources can be added and removed while the mixer is running as i64 as their
    prepareToPlay() and releaseResources() methods are called before and after adding
    them to the mixer.

    @tags{Audio}
*/
class DRX_API  MixerAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a MixerAudioSource. */
    MixerAudioSource();

    /** Destructor. */
    ~MixerAudioSource() override;

    //==============================================================================
    /** Adds an input source to the mixer.

        If the mixer is running you'll need to make sure that the input source
        is ready to play by calling its prepareToPlay() method before adding it.
        If the mixer is stopped, then its input sources will be automatically
        prepared when the mixer's prepareToPlay() method is called.

        @param newInput             the source to add to the mixer
        @param deleteWhenRemoved    if true, then this source will be deleted when
                                    no longer needed by the mixer.
    */
    z0 addInputSource (AudioSource* newInput, b8 deleteWhenRemoved);

    /** Removes an input source.
        If the source was added by calling addInputSource() with the deleteWhenRemoved
        flag set, it will be deleted by this method.
    */
    z0 removeInputSource (AudioSource* input);

    /** Removes all the input sources.
        Any sources which were added by calling addInputSource() with the deleteWhenRemoved
        flag set will be deleted by this method.
    */
    z0 removeAllInputs();

    //==============================================================================
    /** Implementation of the AudioSource method.
        This will call prepareToPlay() on all its input sources.
    */
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate) override;

    /** Implementation of the AudioSource method.
        This will call releaseResources() on all its input sources.
    */
    z0 releaseResources() override;

    /** Implementation of the AudioSource method. */
    z0 getNextAudioBlock (const AudioSourceChannelInfo&) override;


private:
    //==============================================================================
    Array<AudioSource*> inputs;
    BigInteger inputsToDelete;
    CriticalSection lock;
    AudioBuffer<f32> tempBuffer;
    f64 currentSampleRate;
    i32 bufferSizeExpected;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerAudioSource)
};

} // namespace drx
