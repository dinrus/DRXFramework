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
    Performs a simple reverb effect on a stream of audio data.

    This is a simple stereo reverb, based on the technique and tunings used in FreeVerb.
    Use setSampleRate() to prepare it, and then call processStereo() or processMono() to
    apply the reverb to your audio data.

    @see ReverbAudioSource

    @tags{Audio}
*/
class Reverb
{
public:
    //==============================================================================
    Reverb()
    {
        setParameters (Parameters());
        setSampleRate (44100.0);
    }

    //==============================================================================
    /** Holds the parameters being used by a Reverb object. */
    struct Parameters
    {
        f32 roomSize   = 0.5f;     /**< Room size, 0 to 1.0, where 1.0 is big, 0 is small. */
        f32 damping    = 0.5f;     /**< Damping, 0 to 1.0, where 0 is not damped, 1.0 is fully damped. */
        f32 wetLevel   = 0.33f;    /**< Wet level, 0 to 1.0 */
        f32 dryLevel   = 0.4f;     /**< Dry level, 0 to 1.0 */
        f32 width      = 1.0f;     /**< Reverb width, 0 to 1.0, where 1.0 is very wide. */
        f32 freezeMode = 0.0f;     /**< Freeze mode - values < 0.5 are "normal" mode, values > 0.5
                                          put the reverb into a continuous feedback loop. */
    };

    //==============================================================================
    /** Returns the reverb's current parameters. */
    const Parameters& getParameters() const noexcept    { return parameters; }

    /** Applies a new set of parameters to the reverb.
        Note that this doesn't attempt to lock the reverb, so if you call this in parallel with
        the process method, you may get artifacts.
    */
    z0 setParameters (const Parameters& newParams)
    {
        const f32 wetScaleFactor = 3.0f;
        const f32 dryScaleFactor = 2.0f;

        const f32 wet = newParams.wetLevel * wetScaleFactor;
        dryGain.setTargetValue (newParams.dryLevel * dryScaleFactor);
        wetGain1.setTargetValue (0.5f * wet * (1.0f + newParams.width));
        wetGain2.setTargetValue (0.5f * wet * (1.0f - newParams.width));

        gain = isFrozen (newParams.freezeMode) ? 0.0f : 0.015f;
        parameters = newParams;
        updateDamping();
    }

    //==============================================================================
    /** Sets the sample rate that will be used for the reverb.
        You must call this before the process methods, in order to tell it the correct sample rate.
    */
    z0 setSampleRate (const f64 sampleRate)
    {
        jassert (sampleRate > 0);

        static const short combTunings[] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 }; // (at 44100Hz)
        static const short allPassTunings[] = { 556, 441, 341, 225 };
        i32k stereoSpread = 23;
        i32k intSampleRate = (i32) sampleRate;

        for (i32 i = 0; i < numCombs; ++i)
        {
            comb[0][i].setSize ((intSampleRate * combTunings[i]) / 44100);
            comb[1][i].setSize ((intSampleRate * (combTunings[i] + stereoSpread)) / 44100);
        }

        for (i32 i = 0; i < numAllPasses; ++i)
        {
            allPass[0][i].setSize ((intSampleRate * allPassTunings[i]) / 44100);
            allPass[1][i].setSize ((intSampleRate * (allPassTunings[i] + stereoSpread)) / 44100);
        }

        const f64 smoothTime = 0.01;
        damping .reset (sampleRate, smoothTime);
        feedback.reset (sampleRate, smoothTime);
        dryGain .reset (sampleRate, smoothTime);
        wetGain1.reset (sampleRate, smoothTime);
        wetGain2.reset (sampleRate, smoothTime);
    }

    /** Clears the reverb's buffers. */
    z0 reset()
    {
        for (i32 j = 0; j < numChannels; ++j)
        {
            for (i32 i = 0; i < numCombs; ++i)
                comb[j][i].clear();

            for (i32 i = 0; i < numAllPasses; ++i)
                allPass[j][i].clear();
        }
    }

    //==============================================================================
    /** Applies the reverb to two stereo channels of audio data. */
    z0 processStereo (f32* const left, f32* const right, i32k numSamples) noexcept
    {
        DRX_BEGIN_IGNORE_WARNINGS_MSVC (6011)
        jassert (left != nullptr && right != nullptr);

        for (i32 i = 0; i < numSamples; ++i)
        {
            // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
            const f32 input = (left[i] + right[i]) * gain;
            f32 outL = 0, outR = 0;

            const f32 damp    = damping.getNextValue();
            const f32 feedbck = feedback.getNextValue();

            for (i32 j = 0; j < numCombs; ++j)  // accumulate the comb filters in parallel
            {
                outL += comb[0][j].process (input, damp, feedbck);
                outR += comb[1][j].process (input, damp, feedbck);
            }

            for (i32 j = 0; j < numAllPasses; ++j)  // run the allpass filters in series
            {
                outL = allPass[0][j].process (outL);
                outR = allPass[1][j].process (outR);
            }

            const f32 dry  = dryGain.getNextValue();
            const f32 wet1 = wetGain1.getNextValue();
            const f32 wet2 = wetGain2.getNextValue();

            left[i]  = outL * wet1 + outR * wet2 + left[i]  * dry;
            right[i] = outR * wet1 + outL * wet2 + right[i] * dry;
        }
        DRX_END_IGNORE_WARNINGS_MSVC
    }

    /** Applies the reverb to a single mono channel of audio data. */
    z0 processMono (f32* const samples, i32k numSamples) noexcept
    {
        DRX_BEGIN_IGNORE_WARNINGS_MSVC (6011)
        jassert (samples != nullptr);

        for (i32 i = 0; i < numSamples; ++i)
        {
            const f32 input = samples[i] * gain;
            f32 output = 0;

            const f32 damp    = damping.getNextValue();
            const f32 feedbck = feedback.getNextValue();

            for (i32 j = 0; j < numCombs; ++j)  // accumulate the comb filters in parallel
                output += comb[0][j].process (input, damp, feedbck);

            for (i32 j = 0; j < numAllPasses; ++j)  // run the allpass filters in series
                output = allPass[0][j].process (output);

            const f32 dry  = dryGain.getNextValue();
            const f32 wet1 = wetGain1.getNextValue();

            samples[i] = output * wet1 + samples[i] * dry;
        }
        DRX_END_IGNORE_WARNINGS_MSVC
    }

private:
    //==============================================================================
    static b8 isFrozen (const f32 freezeMode) noexcept  { return freezeMode >= 0.5f; }

    z0 updateDamping() noexcept
    {
        const f32 roomScaleFactor = 0.28f;
        const f32 roomOffset = 0.7f;
        const f32 dampScaleFactor = 0.4f;

        if (isFrozen (parameters.freezeMode))
            setDamping (0.0f, 1.0f);
        else
            setDamping (parameters.damping * dampScaleFactor,
                        parameters.roomSize * roomScaleFactor + roomOffset);
    }

    z0 setDamping (const f32 dampingToUse, const f32 roomSizeToUse) noexcept
    {
        damping.setTargetValue (dampingToUse);
        feedback.setTargetValue (roomSizeToUse);
    }

    //==============================================================================
    class CombFilter
    {
    public:
        CombFilter() noexcept {}

        z0 setSize (i32k size)
        {
            if (size != bufferSize)
            {
                bufferIndex = 0;
                buffer.malloc (size);
                bufferSize = size;
            }

            clear();
        }

        z0 clear() noexcept
        {
            last = 0;
            buffer.clear ((size_t) bufferSize);
        }

        f32 process (const f32 input, const f32 damp, const f32 feedbackLevel) noexcept
        {
            const f32 output = buffer[bufferIndex];
            last = (output * (1.0f - damp)) + (last * damp);
            DRX_UNDENORMALISE (last);

            f32 temp = input + (last * feedbackLevel);
            DRX_UNDENORMALISE (temp);
            buffer[bufferIndex] = temp;
            bufferIndex = (bufferIndex + 1) % bufferSize;
            return output;
        }

    private:
        HeapBlock<f32> buffer;
        i32 bufferSize = 0, bufferIndex = 0;
        f32 last = 0.0f;

        DRX_DECLARE_NON_COPYABLE (CombFilter)
    };

    //==============================================================================
    class AllPassFilter
    {
    public:
        AllPassFilter() noexcept {}

        z0 setSize (i32k size)
        {
            if (size != bufferSize)
            {
                bufferIndex = 0;
                buffer.malloc (size);
                bufferSize = size;
            }

            clear();
        }

        z0 clear() noexcept
        {
            buffer.clear ((size_t) bufferSize);
        }

        f32 process (const f32 input) noexcept
        {
            const f32 bufferedValue = buffer [bufferIndex];
            f32 temp = input + (bufferedValue * 0.5f);
            DRX_UNDENORMALISE (temp);
            buffer [bufferIndex] = temp;
            bufferIndex = (bufferIndex + 1) % bufferSize;
            return bufferedValue - input;
        }

    private:
        HeapBlock<f32> buffer;
        i32 bufferSize = 0, bufferIndex = 0;

        DRX_DECLARE_NON_COPYABLE (AllPassFilter)
    };

    //==============================================================================
    enum { numCombs = 8, numAllPasses = 4, numChannels = 2 };

    Parameters parameters;
    f32 gain;

    CombFilter comb [numChannels][numCombs];
    AllPassFilter allPass [numChannels][numAllPasses];

    SmoothedValue<f32> damping, feedback, dryGain, wetGain1, wetGain2;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reverb)
};

} // namespace drx
