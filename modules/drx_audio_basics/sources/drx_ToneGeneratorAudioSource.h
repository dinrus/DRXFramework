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
    A simple AudioSource that generates a sine wave.


    @tags{Audio}
*/
class DRX_API  ToneGeneratorAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a ToneGeneratorAudioSource. */
    ToneGeneratorAudioSource();

    /** Destructor. */
    ~ToneGeneratorAudioSource() override;

    //==============================================================================
    /** Sets the signal's amplitude. */
    z0 setAmplitude (f32 newAmplitude);

    /** Sets the signal's frequency. */
    z0 setFrequency (f64 newFrequencyHz);


    //==============================================================================
    /** Implementation of the AudioSource method. */
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate) override;

    /** Implementation of the AudioSource method. */
    z0 releaseResources() override;

    /** Implementation of the AudioSource method. */
    z0 getNextAudioBlock (const AudioSourceChannelInfo&) override;


private:
    //==============================================================================
    f64 frequency, sampleRate;
    f64 currentPhase, phasePerSample;
    f32 amplitude;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToneGeneratorAudioSource)
};

} // namespace drx
