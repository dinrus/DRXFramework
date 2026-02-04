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

//==============================================================================
#ifndef DOXYGEN
/** The contents of this namespace are used to implement ProcessorChain and should
    not be used elsewhere. Their interfaces (and existence) are liable to change!
*/
namespace detail
{
    template <typename Fn, typename Tuple, size_t... Ix>
    constexpr z0 forEachInTuple (Fn&& fn, Tuple&& tuple, std::index_sequence<Ix...>)
    {
        (fn (std::get<Ix> (tuple), std::integral_constant<size_t, Ix>()), ...);
    }

    template <typename T>
    using TupleIndexSequence = std::make_index_sequence<std::tuple_size_v<std::remove_cv_t<std::remove_reference_t<T>>>>;

    template <typename Fn, typename Tuple>
    constexpr z0 forEachInTuple (Fn&& fn, Tuple&& tuple)
    {
        forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple>{});
    }

    template <typename Context, size_t Ix>
    inline constexpr auto useContextDirectly = ! Context::usesSeparateInputAndOutputBlocks() || Ix == 0;
}
#endif

/** This variadically-templated class lets you join together any number of processor
    classes into a single processor which will call process() on them all in sequence.

    @tags{DSP}
*/
template <typename... Processors>
class ProcessorChain
{
public:
    /** Get a reference to the processor at index `Index`. */
    template <i32 Index>       auto& get()       noexcept { return std::get<Index> (processors); }

    /** Get a reference to the processor at index `Index`. */
    template <i32 Index> const auto& get() const noexcept { return std::get<Index> (processors); }

    /** Set the processor at index `Index` to be bypassed or enabled. */
    template <i32 Index>
    z0 setBypassed (b8 b) noexcept  { bypassed[(size_t) Index] = b; }

    /** Query whether the processor at index `Index` is bypassed. */
    template <i32 Index>
    b8 isBypassed() const noexcept    { return bypassed[(size_t) Index]; }

    /** Prepare all inner processors with the provided `ProcessSpec`. */
    z0 prepare (const ProcessSpec& spec)
    {
        detail::forEachInTuple ([&] (auto& proc, auto) { proc.prepare (spec); }, processors);
    }

    /** Reset all inner processors. */
    z0 reset()
    {
        detail::forEachInTuple ([] (auto& proc, auto) { proc.reset(); }, processors);
    }

    /** Process `context` through all inner processors in sequence. */
    template <typename ProcessContext>
    z0 process (const ProcessContext& context) noexcept
    {
        detail::forEachInTuple ([this, &context] (auto& proc, auto index) noexcept { this->processOne (context, proc, index); },
                                processors);
    }

private:
    template <typename Context, typename Proc, size_t Ix>
    z0 processOne (const Context& context, Proc& proc, std::integral_constant<size_t, Ix>) noexcept
    {
        if constexpr (detail::useContextDirectly<Context, Ix>)
        {
            auto contextCopy = context;
            contextCopy.isBypassed = (bypassed[Ix] || context.isBypassed);

            proc.process (contextCopy);
        }
        else
        {
            jassert (context.getOutputBlock().getNumChannels() == context.getInputBlock().getNumChannels());
            ProcessContextReplacing<typename Context::SampleType> replacingContext (context.getOutputBlock());
            replacingContext.isBypassed = (bypassed[Ix] || context.isBypassed);

            proc.process (replacingContext);
        }
    }

    std::tuple<Processors...> processors;
    std::array<b8, sizeof... (Processors)> bypassed { {} };
};

/** Non-member equivalent of ProcessorChain::get which avoids awkward
    member template syntax.
*/
template <i32 Index, typename... Processors>
inline auto& get (ProcessorChain<Processors...>& chain) noexcept
{
    return chain.template get<Index>();
}

/** Non-member equivalent of ProcessorChain::get which avoids awkward
    member template syntax.
*/
template <i32 Index, typename... Processors>
inline auto& get (const ProcessorChain<Processors...>& chain) noexcept
{
    return chain.template get<Index>();
}

/** Non-member equivalent of ProcessorChain::setBypassed which avoids awkward
    member template syntax.
*/
template <i32 Index, typename... Processors>
inline z0 setBypassed (ProcessorChain<Processors...>& chain, b8 bypassed) noexcept
{
    chain.template setBypassed<Index> (bypassed);
}

/** Non-member equivalent of ProcessorChain::isBypassed which avoids awkward
    member template syntax.
*/
template <i32 Index, typename... Processors>
inline b8 isBypassed (const ProcessorChain<Processors...>& chain) noexcept
{
    return chain.template isBypassed<Index>();
}

} // namespace drx::dsp

#ifndef DOXYGEN
namespace std
{

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmismatched-tags")

/** Adds support for C++17 structured bindings. */
template <typename... Processors>
struct tuple_size<::drx::dsp::ProcessorChain<Processors...>> : integral_constant<size_t, sizeof... (Processors)> {};

/** Adds support for C++17 structured bindings. */
template <size_t I, typename... Processors>
struct tuple_element<I, ::drx::dsp::ProcessorChain<Processors...>> : tuple_element<I, tuple<Processors...>> {};

DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace std
#endif
