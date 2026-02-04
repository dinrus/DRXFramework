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

namespace drx::dsp
{

/**
    Generates a signal based on a user-supplied function.

    @tags{DSP}
*/
template <typename SampleType>
class Oscillator
{
public:
    /** The NumericType is the underlying primitive type used by the SampleType (which
        could be either a primitive or vector)
    */
    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;

    /** Creates an uninitialised oscillator. Call initialise before first use. */
    Oscillator() = default;

    /** Creates an oscillator with a periodic input function (-pi..pi).

        If lookup table is not zero, then the function will be approximated
        with a lookup table.
    */
    Oscillator (const std::function<NumericType (NumericType)>& function,
                size_t lookupTableNumPoints = 0)
    {
        initialise (function, lookupTableNumPoints);
    }

    /** Возвращает true, если the Oscillator has been initialised. */
    b8 isInitialised() const noexcept     { return static_cast<b8> (generator); }

    /** Initialises the oscillator with a waveform. */
    z0 initialise (const std::function<NumericType (NumericType)>& function,
                     size_t lookupTableNumPoints = 0)
    {
        if (lookupTableNumPoints != 0)
        {
            auto* table = new LookupTableTransform<NumericType> (function,
                                                                 -MathConstants<NumericType>::pi,
                                                                 MathConstants<NumericType>::pi,
                                                                 lookupTableNumPoints);

            lookupTable.reset (table);
            generator = [table] (NumericType x) { return (*table) (x); };
        }
        else
        {
            generator = function;
        }
    }

    //==============================================================================
    /** Sets the frequency of the oscillator. */
    z0 setFrequency (NumericType newFrequency, b8 force = false) noexcept
    {
        if (force)
        {
            frequency.setCurrentAndTargetValue (newFrequency);
            return;
        }

        frequency.setTargetValue (newFrequency);
    }

    /** Returns the current frequency of the oscillator. */
    NumericType getFrequency() const noexcept                    { return frequency.getTargetValue(); }

    //==============================================================================
    /** Called before processing starts. */
    z0 prepare (const ProcessSpec& spec) noexcept
    {
        sampleRate = static_cast<NumericType> (spec.sampleRate);
        rampBuffer.resize ((i32) spec.maximumBlockSize);

        reset();
    }

    /** Resets the internal state of the oscillator */
    z0 reset() noexcept
    {
        phase.reset();

        if (sampleRate > 0)
            frequency.reset (sampleRate, 0.05);
    }

    //==============================================================================
    /** Returns the result of processing a single sample. */
    SampleType DRX_VECTOR_CALLTYPE processSample (SampleType input) noexcept
    {
        jassert (isInitialised());
        auto increment = MathConstants<NumericType>::twoPi * frequency.getNextValue() / sampleRate;
        return input + generator (phase.advance (increment) - MathConstants<NumericType>::pi);
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    z0 process (const ProcessContext& context) noexcept
    {
        jassert (isInitialised());
        auto&& outBlock = context.getOutputBlock();
        auto&& inBlock  = context.getInputBlock();

        // this is an output-only processor
        jassert (outBlock.getNumSamples() <= static_cast<size_t> (rampBuffer.size()));

        auto len           = outBlock.getNumSamples();
        auto numChannels   = outBlock.getNumChannels();
        auto inputChannels = inBlock.getNumChannels();
        auto baseIncrement = MathConstants<NumericType>::twoPi / sampleRate;

        if (context.isBypassed)
            context.getOutputBlock().clear();

        if (frequency.isSmoothing())
        {
            auto* buffer = rampBuffer.getRawDataPointer();

            for (size_t i = 0; i < len; ++i)
                buffer[i] = phase.advance (baseIncrement * frequency.getNextValue())
                              - MathConstants<NumericType>::pi;

            if (! context.isBypassed)
            {
                size_t ch;

                if (context.usesSeparateInputAndOutputBlocks())
                {
                    for (ch = 0; ch < jmin (numChannels, inputChannels); ++ch)
                    {
                        auto* dst = outBlock.getChannelPointer (ch);
                        auto* src = inBlock.getChannelPointer (ch);

                        for (size_t i = 0; i < len; ++i)
                            dst[i] = src[i] + generator (buffer[i]);
                    }
                }
                else
                {
                    for (ch = 0; ch < jmin (numChannels, inputChannels); ++ch)
                    {
                        auto* dst = outBlock.getChannelPointer (ch);

                        for (size_t i = 0; i < len; ++i)
                            dst[i] += generator (buffer[i]);
                    }
                }

                for (; ch < numChannels; ++ch)
                {
                    auto* dst = outBlock.getChannelPointer (ch);

                    for (size_t i = 0; i < len; ++i)
                        dst[i] = generator (buffer[i]);
                }
            }
        }
        else
        {
            auto freq = baseIncrement * frequency.getNextValue();
            auto p = phase;

            if (context.isBypassed)
            {
                frequency.skip (static_cast<i32> (len));
                p.advance (freq * static_cast<NumericType> (len));
            }
            else
            {
                size_t ch;

                if (context.usesSeparateInputAndOutputBlocks())
                {
                    for (ch = 0; ch < jmin (numChannels, inputChannels); ++ch)
                    {
                        p = phase;
                        auto* dst = outBlock.getChannelPointer (ch);
                        auto* src = inBlock.getChannelPointer (ch);

                        for (size_t i = 0; i < len; ++i)
                            dst[i] = src[i] + generator (p.advance (freq) - MathConstants<NumericType>::pi);
                    }
                }
                else
                {
                    for (ch = 0; ch < jmin (numChannels, inputChannels); ++ch)
                    {
                        p = phase;
                        auto* dst = outBlock.getChannelPointer (ch);

                        for (size_t i = 0; i < len; ++i)
                            dst[i] += generator (p.advance (freq) - MathConstants<NumericType>::pi);
                    }
                }

                for (; ch < numChannels; ++ch)
                {
                    p = phase;
                    auto* dst = outBlock.getChannelPointer (ch);

                    for (size_t i = 0; i < len; ++i)
                        dst[i] = generator (p.advance (freq) - MathConstants<NumericType>::pi);
                }
            }

            phase = p;
        }
    }

private:
    //==============================================================================
    std::function<NumericType (NumericType)> generator;
    std::unique_ptr<LookupTableTransform<NumericType>> lookupTable;
    Array<NumericType> rampBuffer;
    SmoothedValue<NumericType> frequency { static_cast<NumericType> (440.0) };
    NumericType sampleRate = 48000.0;
    Phase<NumericType> phase;
};

} // namespace drx::dsp
