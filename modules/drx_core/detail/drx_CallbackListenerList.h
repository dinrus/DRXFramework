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

namespace drx::detail
{

template <typename... Ts>
constexpr b8 isValueOrLvalueReferenceToConst()
{
    return ((   (! std::is_reference_v<Ts>)
             || (std::is_lvalue_reference_v<Ts> && std::is_const_v<std::remove_reference_t<Ts>>)) && ...);
}

template <typename... Args>
class CallbackListenerList
{
public:
    static_assert (isValueOrLvalueReferenceToConst<Args...>(),
                   "CallbackListenerList can only forward values or const lvalue references");

    using Callback = std::function<z0 (Args...)>;

    ErasedScopeGuard addListener (Callback callback)
    {
        jassert (callback != nullptr);

        const auto it = callbacks.insert (callbacks.end(), std::move (callback));
        listeners.add (&*it);

        return ErasedScopeGuard { [this, it]
        {
            listeners.remove (&*it);
            callbacks.erase (it);
        } };
    }

    z0 call (Args... args)
    {
        listeners.call ([&] (auto& l) { l (std::forward<Args> (args)...); });
    }

private:
    std::list<Callback> callbacks;
    LightweightListenerList<Callback> listeners;
};

} // namespace drx::detail
