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

class ProcessorChainTest final : public UnitTest
{
    template <i32 AddValue>
    struct MockProcessor
    {
        z0 prepare (const ProcessSpec&) noexcept { isPrepared = true; }
        z0 reset() noexcept { isReset = true; }

        template <typename Context>
        z0 process (const Context& context) noexcept
        {
            bufferWasClear = approximatelyEqual (context.getInputBlock().getSample (0, 0), 0.0f);

            if (! context.isBypassed)
                context.getOutputBlock().add (AddValue);
        }

        b8 isPrepared     = false;
        b8 isReset        = false;
        b8 bufferWasClear = false;
    };

public:
    ProcessorChainTest()
        : UnitTest ("ProcessorChain", UnitTestCategories::dsp) {}

    z0 runTest() override
    {
        beginTest ("After calling setBypass, processor is bypassed");
        {
            ProcessorChain<MockProcessor<1>, MockProcessor<2>> chain;

            setBypassed<0> (chain, true);
            expect (isBypassed<0> (chain));
            setBypassed<0> (chain, false);
            expect (! isBypassed<0> (chain));

            setBypassed<1> (chain, true);
            expect (isBypassed<1> (chain));
            setBypassed<1> (chain, false);
            expect (! isBypassed<1> (chain));
        }

        beginTest ("After calling prepare, all processors are prepared");
        {
            ProcessorChain<MockProcessor<1>, MockProcessor<2>> chain;

            expect (! get<0> (chain).isPrepared);
            expect (! get<1> (chain).isPrepared);

            chain.prepare (ProcessSpec{});

            expect (get<0> (chain).isPrepared);
            expect (get<1> (chain).isPrepared);
        }

        beginTest ("After calling reset, all processors are reset");
        {
            ProcessorChain<MockProcessor<1>, MockProcessor<2>> chain;

            expect (! get<0> (chain).isReset);
            expect (! get<1> (chain).isReset);

            chain.reset();

            expect (get<0> (chain).isReset);
            expect (get<1> (chain).isReset);
        }

        beginTest ("After calling process, all processors contribute to processing");
        {
            ProcessorChain<MockProcessor<1>, MockProcessor<2>> chain;

            AudioBuffer<f32> buffer (1, 1);
            AudioBlock<f32> block (buffer);
            ProcessContextReplacing<f32> context (block);

            block.clear();
            chain.process (context);
            expectEquals (buffer.getSample (0, 0), 3.0f);
            expect (get<0> (chain).bufferWasClear);
            expect (! get<1> (chain).bufferWasClear);

            setBypassed<0> (chain, true);
            block.clear();
            chain.process (context);
            expectEquals (buffer.getSample (0, 0), 2.0f);
            expect (get<0> (chain).bufferWasClear);
            expect (get<1> (chain).bufferWasClear);

            setBypassed<1> (chain, true);
            block.clear();
            chain.process (context);
            expectEquals (buffer.getSample (0, 0), 0.0f);
            expect (get<0> (chain).bufferWasClear);
            expect (get<1> (chain).bufferWasClear);

            setBypassed<0> (chain, false);
            block.clear();
            chain.process (context);
            expectEquals (buffer.getSample (0, 0), 1.0f);
            expect (get<0> (chain).bufferWasClear);
            expect (! get<1> (chain).bufferWasClear);
        }

        beginTest ("Chains with trailing items that only support replacing contexts can be built");
        {
            AudioBuffer<f32> inBuf (1, 1), outBuf (1, 1);
            drx::dsp::AudioBlock<f32> in (inBuf), out (outBuf);

            struct OnlyReplacing
            {
                z0 prepare (const drx::dsp::ProcessSpec&) {}
                z0 process (const drx::dsp::ProcessContextReplacing<f32>& c)
                {
                    c.getOutputBlock().multiplyBy (2.0f);
                }
                z0 reset() {}
            };

            {
                drx::dsp::ProcessorChain<drx::dsp::Gain<f32>, OnlyReplacing, OnlyReplacing> c;
                drx::dsp::ProcessContextNonReplacing<f32> context (in, out);
                get<0> (c).setGainLinear (1.0f);
                c.prepare (ProcessSpec{});
                inBuf.setSample (0, 0, 1.0f);
                c.process (context);
                expectEquals (outBuf.getSample (0, 0), 4.0f);
            }
        }
    }
};

static ProcessorChainTest processorChainUnitTest;

} // namespace drx::dsp
