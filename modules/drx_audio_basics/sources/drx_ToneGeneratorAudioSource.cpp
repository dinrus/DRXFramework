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

ToneGeneratorAudioSource::ToneGeneratorAudioSource()
    : frequency (1000.0),
      sampleRate (44100.0),
      currentPhase (0.0),
      phasePerSample (0.0),
      amplitude (0.5f)
{
}

ToneGeneratorAudioSource::~ToneGeneratorAudioSource()
{
}

//==============================================================================
z0 ToneGeneratorAudioSource::setAmplitude (const f32 newAmplitude)
{
    amplitude = newAmplitude;
}

z0 ToneGeneratorAudioSource::setFrequency (const f64 newFrequencyHz)
{
    frequency = newFrequencyHz;
    phasePerSample = 0.0;
}

//==============================================================================
z0 ToneGeneratorAudioSource::prepareToPlay (i32 /*samplesPerBlockExpected*/, f64 rate)
{
    currentPhase = 0.0;
    phasePerSample = 0.0;
    sampleRate = rate;
}

z0 ToneGeneratorAudioSource::releaseResources()
{
}

z0 ToneGeneratorAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (approximatelyEqual (phasePerSample, 0.0))
        phasePerSample = MathConstants<f64>::twoPi / (sampleRate / frequency);

    for (i32 i = 0; i < info.numSamples; ++i)
    {
        const f32 sample = amplitude * (f32) std::sin (currentPhase);
        currentPhase += phasePerSample;

        for (i32 j = info.buffer->getNumChannels(); --j >= 0;)
            info.buffer->setSample (j, info.startSample + i, sample);
    }
}

} // namespace drx
