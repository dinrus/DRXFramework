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

struct ADSRTests final : public UnitTest
{
    ADSRTests()  : UnitTest ("ADSR", UnitTestCategories::audio)  {}

    z0 runTest() override
    {
        constexpr f64 sampleRate = 44100.0;
        const ADSR::Parameters parameters { 0.1f, 0.1f, 0.5f, 0.1f };

        ADSR adsr;
        adsr.setSampleRate (sampleRate);
        adsr.setParameters (parameters);

        beginTest ("Idle");
        {
            adsr.reset();

            expect (! adsr.isActive());
            expectEquals (adsr.getNextSample(), 0.0f);
        }

        beginTest ("Attack");
        {
            adsr.reset();

            adsr.noteOn();
            expect (adsr.isActive());

            auto buffer = getTestBuffer (sampleRate, parameters.attack);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isIncreasing (buffer));
        }

        beginTest ("Decay");
        {
            adsr.reset();

            adsr.noteOn();
            advanceADSR (adsr, roundToInt (parameters.attack * sampleRate));

            auto buffer = getTestBuffer (sampleRate, parameters.decay);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isDecreasing (buffer));
        }

        beginTest ("Sustain");
        {
            adsr.reset();

            adsr.noteOn();
            advanceADSR (adsr, roundToInt ((parameters.attack + parameters.decay + 0.01) * sampleRate));

            auto random = getRandom();

            for (i32 numTests = 0; numTests < 100; ++numTests)
            {
                const auto sustainLevel = random.nextFloat();
                const auto sustainLength = jmax (0.1f, random.nextFloat());

                adsr.setParameters ({ parameters.attack, parameters.decay, sustainLevel, parameters.release });

                auto buffer = getTestBuffer (sampleRate, sustainLength);
                adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

                expect (isSustained (buffer, sustainLevel));
            }
        }

        beginTest ("Release");
        {
            adsr.reset();

            adsr.noteOn();
            advanceADSR (adsr, roundToInt ((parameters.attack + parameters.decay) * sampleRate));
            adsr.noteOff();

            auto buffer = getTestBuffer (sampleRate, parameters.release);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isDecreasing (buffer));
        }

        beginTest ("Zero-length attack jumps to decay");
        {
            adsr.reset();
            adsr.setParameters ({ 0.0f, parameters.decay, parameters.sustain, parameters.release });

            adsr.noteOn();

            auto buffer = getTestBuffer (sampleRate, parameters.decay);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isDecreasing (buffer));
        }

        beginTest ("Zero-length decay jumps to sustain");
        {
            adsr.reset();
            adsr.setParameters ({ parameters.attack, 0.0f, parameters.sustain, parameters.release });

            adsr.noteOn();
            advanceADSR (adsr, roundToInt (parameters.attack * sampleRate));
            adsr.getNextSample();

            expectEquals (adsr.getNextSample(), parameters.sustain);

            auto buffer = getTestBuffer (sampleRate, 1);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isSustained (buffer, parameters.sustain));
        }

        beginTest ("Zero-length attack and decay jumps to sustain");
        {
            adsr.reset();
            adsr.setParameters ({ 0.0f, 0.0f, parameters.sustain, parameters.release });

            adsr.noteOn();

            expectEquals (adsr.getNextSample(), parameters.sustain);

            auto buffer = getTestBuffer (sampleRate, 1);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isSustained (buffer, parameters.sustain));
        }

        beginTest ("Zero-length attack and decay releases correctly");
        {
            adsr.reset();
            adsr.setParameters ({ 0.0f, 0.0f, parameters.sustain, parameters.release });

            adsr.noteOn();
            adsr.noteOff();

            auto buffer = getTestBuffer (sampleRate, parameters.release);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isDecreasing (buffer));
        }

        beginTest ("Zero-length release resets to idle");
        {
            adsr.reset();
            adsr.setParameters ({ parameters.attack, parameters.decay, parameters.sustain, 0.0f });

            adsr.noteOn();
            advanceADSR (adsr, roundToInt ((parameters.attack + parameters.decay) * sampleRate));
            adsr.noteOff();

            expect (! adsr.isActive());
        }
    }

    static z0 advanceADSR (ADSR& adsr, i32 numSamplesToAdvance)
    {
        while (--numSamplesToAdvance >= 0)
            adsr.getNextSample();
    }

    static AudioBuffer<f32> getTestBuffer (f64 sampleRate, f32 lengthInSeconds)
    {
        AudioBuffer<f32> buffer { 2, roundToInt (lengthInSeconds * sampleRate) };

        for (i32 channel = 0; channel < buffer.getNumChannels(); ++channel)
            for (i32 sample = 0; sample < buffer.getNumSamples(); ++sample)
                buffer.setSample (channel, sample, 1.0f);

        return buffer;
    }

    static b8 isIncreasing (const AudioBuffer<f32>& b)
    {
        jassert (b.getNumChannels() > 0 && b.getNumSamples() > 0);

        for (i32 channel = 0; channel < b.getNumChannels(); ++channel)
        {
            f32 previousSample = -1.0f;

            for (i32 sample = 0; sample < b.getNumSamples(); ++sample)
            {
                const auto currentSample = b.getSample (channel, sample);

                if (currentSample <= previousSample)
                    return false;

                previousSample = currentSample;
            }
        }

        return true;
    }

    static b8 isDecreasing (const AudioBuffer<f32>& b)
    {
        jassert (b.getNumChannels() > 0 && b.getNumSamples() > 0);

        for (i32 channel = 0; channel < b.getNumChannels(); ++channel)
        {
            f32 previousSample = std::numeric_limits<f32>::max();

            for (i32 sample = 0; sample < b.getNumSamples(); ++sample)
            {
                const auto currentSample = b.getSample (channel, sample);

                if (currentSample >= previousSample)
                    return false;

                previousSample = currentSample;
            }
        }

        return true;
    }

    static b8 isSustained (const AudioBuffer<f32>& b, f32 sustainLevel)
    {
        jassert (b.getNumChannels() > 0 && b.getNumSamples() > 0);

        for (i32 channel = 0; channel < b.getNumChannels(); ++channel)
            if (b.findMinMax (channel, 0, b.getNumSamples()) != Range<f32> { sustainLevel, sustainLevel })
                return false;

        return true;
    }
};

static ADSRTests adsrTests;

} // namespace drx
