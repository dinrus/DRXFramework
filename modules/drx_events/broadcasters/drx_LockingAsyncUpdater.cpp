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

class LockingAsyncUpdater::Impl : public CallbackMessage
{
public:
    explicit Impl (std::function<z0()> cb)
        : callback (std::move (cb)) {}

    z0 clear()
    {
        const ScopedLock lock (mutex);
        deliver = false;
        callback = nullptr;
    }

    z0 trigger()
    {
        {
            const ScopedLock lock (mutex);

            if (deliver)
                return;

            deliver = true;
        }

        if (! post())
            cancel();
    }

    z0 cancel()
    {
        const ScopedLock lock (mutex);
        deliver = false;
    }

    b8 isPending()
    {
        const ScopedLock lock (mutex);
        return deliver;
    }

    z0 messageCallback() override
    {
        const ScopedLock lock (mutex);

        if (std::exchange (deliver, false))
            NullCheckedInvocation::invoke (callback);
    }

private:
    CriticalSection mutex;
    std::function<z0()> callback;
    b8 deliver = false;
};

//==============================================================================
LockingAsyncUpdater::LockingAsyncUpdater (std::function<z0()> callbackToUse)
    : impl (new Impl (std::move (callbackToUse))) {}

LockingAsyncUpdater::LockingAsyncUpdater (LockingAsyncUpdater&& other) noexcept
    : impl (std::exchange (other.impl, nullptr)) {}

LockingAsyncUpdater& LockingAsyncUpdater::operator= (LockingAsyncUpdater&& other) noexcept
{
    LockingAsyncUpdater temp { std::move (other) };
    std::swap (temp.impl, impl);
    return *this;
}

LockingAsyncUpdater::~LockingAsyncUpdater()
{
    if (impl != nullptr)
        impl->clear();
}

z0 LockingAsyncUpdater::triggerAsyncUpdate()
{
    if (impl != nullptr)
        impl->trigger();
    else
        jassertfalse; // moved-from!
}

z0 LockingAsyncUpdater::cancelPendingUpdate() noexcept
{
    if (impl != nullptr)
        impl->cancel();
    else
        jassertfalse; // moved-from!
}

z0 LockingAsyncUpdater::handleUpdateNowIfNeeded()
{
    if (impl != nullptr)
        impl->messageCallback();
    else
        jassertfalse; // moved-from!
}

b8 LockingAsyncUpdater::isUpdatePending() const noexcept
{
    if (impl != nullptr)
        return impl->isPending();

    jassertfalse; // moved-from!
    return false;
}

} // namespace drx
