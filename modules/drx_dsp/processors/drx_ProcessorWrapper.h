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
    Acts as a polymorphic base class for processors.
    This exposes the same set of methods that a processor must implement as virtual
    methods, so that you can use the ProcessorWrapper class to wrap an instance of
    a subclass, and then pass that around using ProcessorBase as a base class.
    @see ProcessorWrapper

    @tags{DSP}
*/
struct ProcessorBase
{
    ProcessorBase() = default;
    virtual ~ProcessorBase() = default;

    virtual z0 prepare (const ProcessSpec&)  = 0;
    virtual z0 process (const ProcessContextReplacing<f32>&) = 0;
    virtual z0 reset() = 0;
};


//==============================================================================
/**
    Wraps an instance of a given processor class, and exposes it through the
    ProcessorBase interface.
    @see ProcessorBase

    @tags{DSP}
*/
template <typename ProcessorType>
struct ProcessorWrapper  : public ProcessorBase
{
    z0 prepare (const ProcessSpec& spec) override
    {
        processor.prepare (spec);
    }

    z0 process (const ProcessContextReplacing<f32>& context) override
    {
        processor.process (context);
    }

    z0 reset() override
    {
        processor.reset();
    }

    ProcessorType processor;
};

} // namespace drx::dsp
