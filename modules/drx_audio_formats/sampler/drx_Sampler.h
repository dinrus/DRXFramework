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
    A subclass of SynthesiserSound that represents a sampled audio clip.

    This is a pretty basic sampler, and just attempts to load the whole audio stream
    into memory.

    To use it, create a Synthesiser, add some SamplerVoice objects to it, then
    give it some SampledSound objects to play.

    @see SamplerVoice, Synthesiser, SynthesiserSound

    @tags{Audio}
*/
class DRX_API  SamplerSound    : public SynthesiserSound
{
public:
    //==============================================================================
    /** Creates a sampled sound from an audio reader.

        This will attempt to load the audio from the source into memory and store
        it in this object.

        @param name         a name for the sample
        @param source       the audio to load. This object can be safely deleted by the
                            caller after this constructor returns
        @param midiNotes    the set of midi keys that this sound should be played on. This
                            is used by the SynthesiserSound::appliesToNote() method
        @param midiNoteForNormalPitch   the midi note at which the sample should be played
                                        with its natural rate. All other notes will be pitched
                                        up or down relative to this one
        @param attackTimeSecs   the attack (fade-in) time, in seconds
        @param releaseTimeSecs  the decay (fade-out) time, in seconds
        @param maxSampleLengthSeconds   a maximum length of audio to read from the audio
                                        source, in seconds
    */
    SamplerSound (const Txt& name,
                  AudioFormatReader& source,
                  const BigInteger& midiNotes,
                  i32 midiNoteForNormalPitch,
                  f64 attackTimeSecs,
                  f64 releaseTimeSecs,
                  f64 maxSampleLengthSeconds);

    /** Destructor. */
    ~SamplerSound() override;

    //==============================================================================
    /** Returns the sample's name */
    const Txt& getName() const noexcept                  { return name; }

    /** Returns the audio sample data.
        This could return nullptr if there was a problem loading the data.
    */
    AudioBuffer<f32>* getAudioData() const noexcept       { return data.get(); }

    //==============================================================================
    /** Changes the parameters of the ADSR envelope which will be applied to the sample. */
    z0 setEnvelopeParameters (ADSR::Parameters parametersToUse)    { params = parametersToUse; }

    //==============================================================================
    b8 appliesToNote (i32 midiNoteNumber) override;
    b8 appliesToChannel (i32 midiChannel) override;

private:
    //==============================================================================
    friend class SamplerVoice;

    Txt name;
    std::unique_ptr<AudioBuffer<f32>> data;
    f64 sourceSampleRate;
    BigInteger midiNotes;
    i32 length = 0, midiRootNote = 0;

    ADSR::Parameters params;

    DRX_LEAK_DETECTOR (SamplerSound)
};


//==============================================================================
/**
    A subclass of SynthesiserVoice that can play a SamplerSound.

    To use it, create a Synthesiser, add some SamplerVoice objects to it, then
    give it some SampledSound objects to play.

    @see SamplerSound, Synthesiser, SynthesiserVoice

    @tags{Audio}
*/
class DRX_API  SamplerVoice    : public SynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a SamplerVoice. */
    SamplerVoice();

    /** Destructor. */
    ~SamplerVoice() override;

    //==============================================================================
    b8 canPlaySound (SynthesiserSound*) override;

    z0 startNote (i32 midiNoteNumber, f32 velocity, SynthesiserSound*, i32 pitchWheel) override;
    z0 stopNote (f32 velocity, b8 allowTailOff) override;

    z0 pitchWheelMoved (i32 newValue) override;
    z0 controllerMoved (i32 controllerNumber, i32 newValue) override;

    z0 renderNextBlock (AudioBuffer<f32>&, i32 startSample, i32 numSamples) override;
    using SynthesiserVoice::renderNextBlock;

private:
    //==============================================================================
    f64 pitchRatio = 0;
    f64 sourceSamplePosition = 0;
    f32 lgain = 0, rgain = 0;

    ADSR adsr;

    DRX_LEAK_DETECTOR (SamplerVoice)
};

} // namespace drx
