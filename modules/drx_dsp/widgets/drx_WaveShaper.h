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
    Applies waveshaping to audio samples as single samples or AudioBlocks.

    @tags{DSP}
*/
template <typename FloatType, typename Function = FloatType (*) (FloatType)>
struct WaveShaper
{
    Function functionToUse;

    //==============================================================================
    /** Called before processing starts. */
    z0 prepare (const ProcessSpec&) noexcept   {}

    //==============================================================================
    /** Returns the result of processing a single sample. */
    template <typename SampleType>
    SampleType DRX_VECTOR_CALLTYPE processSample (SampleType inputSample) const noexcept
    {
        return functionToUse (inputSample);
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    z0 process (const ProcessContext& context) const noexcept
    {
        if (context.isBypassed)
        {
            if (context.usesSeparateInputAndOutputBlocks())
                context.getOutputBlock().copyFrom (context.getInputBlock());
        }
        else
        {
            AudioBlock<FloatType>::process (context.getInputBlock(),
                                            context.getOutputBlock(),
                                            functionToUse);
        }
    }

    z0 reset() noexcept {}
};

//==============================================================================
#if ! ((DRX_MAC || DRX_IOS) && DRX_CLANG && __clang_major__ < 10)
template <typename Functor>
static WaveShaper<typename std::invoke_result<Functor>, Functor> CreateWaveShaper (Functor functionToUse)   { return {functionToUse}; }
#else
template <typename Functor>
static WaveShaper<typename std::result_of<Functor>, Functor> CreateWaveShaper (Functor functionToUse)   { return {functionToUse}; }
#endif

} // namespace drx::dsp
