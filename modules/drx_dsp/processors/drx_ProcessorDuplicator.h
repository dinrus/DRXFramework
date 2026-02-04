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
    Converts a mono processor class into a multi-channel version by duplicating it
    and applying multichannel buffers across an array of instances.

    When the prepare method is called, it uses the specified number of channels to
    instantiate the appropriate number of instances, which it then uses in its
    process() method.

    @tags{DSP}
*/
template <typename MonoProcessorType, typename StateType>
struct ProcessorDuplicator
{
    ProcessorDuplicator() : state (new StateType()) {}
    ProcessorDuplicator (StateType* stateToUse) : state (stateToUse) {}
    ProcessorDuplicator (typename StateType::Ptr stateToUse) : state (std::move (stateToUse)) {}
    ProcessorDuplicator (const ProcessorDuplicator&) = default;
    ProcessorDuplicator (ProcessorDuplicator&&) = default;

    z0 prepare (const ProcessSpec& spec)
    {
        processors.removeRange ((i32) spec.numChannels, processors.size());

        while (static_cast<size_t> (processors.size()) < spec.numChannels)
            processors.add (new MonoProcessorType (state));

        auto monoSpec = spec;
        monoSpec.numChannels = 1;

        for (auto* p : processors)
            p->prepare (monoSpec);
    }

    z0 reset() noexcept      { for (auto* p : processors) p->reset(); }

    template <typename ProcessContext>
    z0 process (const ProcessContext& context) noexcept
    {
        jassert ((i32) context.getInputBlock().getNumChannels()  <= processors.size());
        jassert ((i32) context.getOutputBlock().getNumChannels() <= processors.size());

        auto numChannels = static_cast<size_t> (jmin (context.getInputBlock().getNumChannels(),
                                                      context.getOutputBlock().getNumChannels()));

        for (size_t chan = 0; chan < numChannels; ++chan)
            processors[(i32) chan]->process (MonoProcessContext<ProcessContext> (context, chan));
    }

    typename StateType::Ptr state;

private:
    template <typename ProcessContext>
    struct MonoProcessContext : public ProcessContext
    {
        MonoProcessContext (const ProcessContext& multiChannelContext, size_t channelToUse)
            : ProcessContext (multiChannelContext), channel (channelToUse)
        {}

        size_t channel;

        typename ProcessContext::ConstAudioBlockType getInputBlock()  const noexcept       { return ProcessContext::getInputBlock() .getSingleChannelBlock (channel); }
        typename ProcessContext::AudioBlockType      getOutputBlock() const noexcept       { return ProcessContext::getOutputBlock().getSingleChannelBlock (channel); }
    };

    drx::OwnedArray<MonoProcessorType> processors;
};

} // namespace drx::dsp
