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
    A very simple ADSR envelope class.

    To use it, call setSampleRate() with the current sample rate and give it some parameters
    with setParameters() then call getNextSample() to get the envelope value to be applied
    to each audio sample or applyEnvelopeToBuffer() to apply the envelope to a whole buffer.

    Do not change the parameters during playback. If you change the parameters before the
    release stage has completed then you must call reset() before the next call to
    noteOn().

    @tags{Audio}
*/
class DRX_API  ADSR
{
public:
    //==============================================================================
    ADSR()
    {
        recalculateRates();
    }

    //==============================================================================
    /**
        Holds the parameters being used by an ADSR object.

        @tags{Audio}
    */
    struct DRX_API  Parameters
    {
        Parameters() = default;

        Parameters (f32 attackTimeSeconds,
                    f32 decayTimeSeconds,
                    f32 sustainLevel,
                    f32 releaseTimeSeconds)
            : attack (attackTimeSeconds),
              decay (decayTimeSeconds),
              sustain (sustainLevel),
              release (releaseTimeSeconds)
        {
        }

        f32 attack = 0.1f, decay = 0.1f, sustain = 1.0f, release = 0.1f;
    };

    /** Sets the parameters that will be used by an ADSR object.

        You must have called setSampleRate() with the correct sample rate before
        this otherwise the values may be incorrect!

        @see getParameters
    */
    z0 setParameters (const Parameters& newParameters)
    {
        // need to call setSampleRate() first!
        jassert (sampleRate > 0.0);

        parameters = newParameters;
        recalculateRates();
    }

    /** Returns the parameters currently being used by an ADSR object.

        @see setParameters
    */
    const Parameters& getParameters() const noexcept  { return parameters; }

    /** Возвращает true, если the envelope is in its attack, decay, sustain or release stage. */
    b8 isActive() const noexcept                    { return state != State::idle; }

    //==============================================================================
    /** Sets the sample rate that will be used for the envelope.

        This must be called before the getNextSample() or setParameters() methods.
    */
    z0 setSampleRate (f64 newSampleRate) noexcept
    {
        jassert (newSampleRate > 0.0);
        sampleRate = newSampleRate;
    }

    //==============================================================================
    /** Resets the envelope to an idle state. */
    z0 reset() noexcept
    {
        envelopeVal = 0.0f;
        state = State::idle;
    }

    /** Starts the attack phase of the envelope. */
    z0 noteOn() noexcept
    {
        if (attackRate > 0.0f)
        {
            state = State::attack;
        }
        else if (decayRate > 0.0f)
        {
            envelopeVal = 1.0f;
            state = State::decay;
        }
        else
        {
            envelopeVal = parameters.sustain;
            state = State::sustain;
        }
    }

    /** Starts the release phase of the envelope. */
    z0 noteOff() noexcept
    {
        if (state != State::idle)
        {
            if (parameters.release > 0.0f)
            {
                releaseRate = (f32) (envelopeVal / (parameters.release * sampleRate));
                state = State::release;
            }
            else
            {
                reset();
            }
        }
    }

    //==============================================================================
    /** Returns the next sample value for an ADSR object.

        @see applyEnvelopeToBuffer
    */
    f32 getNextSample() noexcept
    {
        switch (state)
        {
            case State::idle:
            {
                return 0.0f;
            }

            case State::attack:
            {
                envelopeVal += attackRate;

                if (envelopeVal >= 1.0f)
                {
                    envelopeVal = 1.0f;
                    goToNextState();
                }

                break;
            }

            case State::decay:
            {
                envelopeVal -= decayRate;

                if (envelopeVal <= parameters.sustain)
                {
                    envelopeVal = parameters.sustain;
                    goToNextState();
                }

                break;
            }

            case State::sustain:
            {
                envelopeVal = parameters.sustain;
                break;
            }

            case State::release:
            {
                envelopeVal -= releaseRate;

                if (envelopeVal <= 0.0f)
                    goToNextState();

                break;
            }
        }

        return envelopeVal;
    }

    /** This method will conveniently apply the next numSamples number of envelope values
        to an AudioBuffer.

        @see getNextSample
    */
    template <typename FloatType>
    z0 applyEnvelopeToBuffer (AudioBuffer<FloatType>& buffer, i32 startSample, i32 numSamples)
    {
        jassert (startSample + numSamples <= buffer.getNumSamples());

        if (state == State::idle)
        {
            buffer.clear (startSample, numSamples);
            return;
        }

        if (state == State::sustain)
        {
            buffer.applyGain (startSample, numSamples, parameters.sustain);
            return;
        }

        auto numChannels = buffer.getNumChannels();

        while (--numSamples >= 0)
        {
            auto env = getNextSample();

            for (i32 i = 0; i < numChannels; ++i)
                buffer.getWritePointer (i)[startSample] *= env;

            ++startSample;
        }
    }

private:
    //==============================================================================
    z0 recalculateRates() noexcept
    {
        auto getRate = [] (f32 distance, f32 timeInSeconds, f64 sr)
        {
            return timeInSeconds > 0.0f ? (f32) (distance / (timeInSeconds * sr)) : -1.0f;
        };

        attackRate  = getRate (1.0f, parameters.attack, sampleRate);
        decayRate   = getRate (1.0f - parameters.sustain, parameters.decay, sampleRate);
        releaseRate = getRate (parameters.sustain, parameters.release, sampleRate);

        if ((state == State::attack && attackRate <= 0.0f)
            || (state == State::decay && (decayRate <= 0.0f || envelopeVal <= parameters.sustain))
            || (state == State::release && releaseRate <= 0.0f))
        {
            goToNextState();
        }
    }

    z0 goToNextState() noexcept
    {
        if (state == State::attack)
        {
            state = (decayRate > 0.0f ? State::decay : State::sustain);
            return;
        }

        if (state == State::decay)
        {
            state = State::sustain;
            return;
        }

        if (state == State::release)
            reset();
    }

    //==============================================================================
    enum class State { idle, attack, decay, sustain, release };

    State state = State::idle;
    Parameters parameters;

    f64 sampleRate = 44100.0;
    f32 envelopeVal = 0.0f, attackRate = 0.0f, decayRate = 0.0f, releaseRate = 0.0f;
};

} // namespace drx
