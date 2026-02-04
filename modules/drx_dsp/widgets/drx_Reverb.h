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
    Processor wrapper around drx::Reverb for easy integration into ProcessorChain.

    @tags{DSP}
*/
class Reverb
{
public:
    //==============================================================================
    /** Creates an uninitialised Reverb processor. Call prepare() before first use. */
    Reverb() = default;

    //==============================================================================
    using Parameters = drx::Reverb::Parameters;

    /** Returns the reverb's current parameters. */
    const Parameters& getParameters() const noexcept    { return reverb.getParameters(); }

    /** Applies a new set of parameters to the reverb.
        Note that this doesn't attempt to lock the reverb, so if you call this in parallel with
        the process method, you may get artifacts.
    */
    z0 setParameters (const Parameters& newParams)    { reverb.setParameters (newParams); }

    /** Возвращает true, если the reverb is enabled. */
    b8 isEnabled() const noexcept                     { return enabled; }

    /** Enables/disables the reverb. */
    z0 setEnabled (b8 newValue) noexcept            { enabled = newValue; }

    //==============================================================================
    /** Initialises the reverb. */
    z0 prepare (const ProcessSpec& spec)
    {
        reverb.setSampleRate (spec.sampleRate);
    }

    /** Resets the reverb's internal state. */
    z0 reset() noexcept
    {
        reverb.reset();
    }

    //==============================================================================
    /** Applies the reverb to a mono or stereo buffer. */
    template <typename ProcessContext>
    z0 process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numInChannels = inputBlock.getNumChannels();
        const auto numOutChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert (inputBlock.getNumSamples() == numSamples);

        outputBlock.copyFrom (inputBlock);

        if (! enabled || context.isBypassed)
            return;

        if (numInChannels == 1 && numOutChannels == 1)
        {
            reverb.processMono (outputBlock.getChannelPointer (0), (i32) numSamples);
        }
        else if (numInChannels == 2 && numOutChannels == 2)
        {
            reverb.processStereo (outputBlock.getChannelPointer (0),
                                  outputBlock.getChannelPointer (1),
                                  (i32) numSamples);
        }
        else
        {
            jassertfalse;   // invalid channel configuration
        }
    }

private:
    //==============================================================================
    drx::Reverb reverb;
    b8 enabled = true;
};

} // namespace drx::dsp
