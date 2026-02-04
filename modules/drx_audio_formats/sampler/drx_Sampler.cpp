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

SamplerSound::SamplerSound (const Txt& soundName,
                            AudioFormatReader& source,
                            const BigInteger& notes,
                            i32 midiNoteForNormalPitch,
                            f64 attackTimeSecs,
                            f64 releaseTimeSecs,
                            f64 maxSampleLengthSeconds)
    : name (soundName),
      sourceSampleRate (source.sampleRate),
      midiNotes (notes),
      midiRootNote (midiNoteForNormalPitch)
{
    if (sourceSampleRate > 0 && source.lengthInSamples > 0)
    {
        length = jmin ((i32) source.lengthInSamples,
                       (i32) (maxSampleLengthSeconds * sourceSampleRate));

        data.reset (new AudioBuffer<f32> (jmin (2, (i32) source.numChannels), length + 4));

        source.read (data.get(), 0, length + 4, 0, true, true);

        params.attack  = static_cast<f32> (attackTimeSecs);
        params.release = static_cast<f32> (releaseTimeSecs);
    }
}

SamplerSound::~SamplerSound()
{
}

b8 SamplerSound::appliesToNote (i32 midiNoteNumber)
{
    return midiNotes[midiNoteNumber];
}

b8 SamplerSound::appliesToChannel (i32 /*midiChannel*/)
{
    return true;
}

//==============================================================================
SamplerVoice::SamplerVoice() {}
SamplerVoice::~SamplerVoice() {}

b8 SamplerVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast<const SamplerSound*> (sound) != nullptr;
}

z0 SamplerVoice::startNote (i32 midiNoteNumber, f32 velocity, SynthesiserSound* s, i32 /*currentPitchWheelPosition*/)
{
    if (auto* sound = dynamic_cast<const SamplerSound*> (s))
    {
        pitchRatio = std::pow (2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
                        * sound->sourceSampleRate / getSampleRate();

        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;

        adsr.setSampleRate (sound->sourceSampleRate);
        adsr.setParameters (sound->params);

        adsr.noteOn();
    }
    else
    {
        jassertfalse; // this object can only play SamplerSounds!
    }
}

z0 SamplerVoice::stopNote (f32 /*velocity*/, b8 allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

z0 SamplerVoice::pitchWheelMoved (i32 /*newValue*/) {}
z0 SamplerVoice::controllerMoved (i32 /*controllerNumber*/, i32 /*newValue*/) {}

//==============================================================================
z0 SamplerVoice::renderNextBlock (AudioBuffer<f32>& outputBuffer, i32 startSample, i32 numSamples)
{
    if (auto* playingSound = static_cast<SamplerSound*> (getCurrentlyPlayingSound().get()))
    {
        auto& data = *playingSound->data;
        const f32* const inL = data.getReadPointer (0);
        const f32* const inR = data.getNumChannels() > 1 ? data.getReadPointer (1) : nullptr;

        f32* outL = outputBuffer.getWritePointer (0, startSample);
        f32* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            auto pos = (i32) sourceSamplePosition;
            auto alpha = (f32) (sourceSamplePosition - pos);
            auto invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            f32 l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
            f32 r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
                                       : l;

            auto envelopeValue = adsr.getNextSample();

            l *= lgain * envelopeValue;
            r *= rgain * envelopeValue;

            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            sourceSamplePosition += pitchRatio;

            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (0.0f, false);
                break;
            }
        }
    }
}

} // namespace drx
