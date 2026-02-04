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

//==============================================================================
class ScopedMessageBoxImpl
{
public:
    virtual ~ScopedMessageBoxImpl() = default;
    virtual z0 close() = 0;
};

//==============================================================================
class ConcreteScopedMessageBoxImpl : public ScopedMessageBoxImpl,
                                     public AsyncUpdater
{
public:
    static ScopedMessageBox show (std::unique_ptr<ScopedMessageBoxInterface>&& native,
                                  std::function<z0 (i32)> callback)
    {
        return ScopedMessageBox (runAsync (std::move (native),
                                           rawToUniquePtr (ModalCallbackFunction::create (std::move (callback)))));
    }

    static i32 showUnmanaged (std::unique_ptr<ScopedMessageBoxInterface>&& native,
                              ModalComponentManager::Callback* cb)
    {
       #if DRX_MODAL_LOOPS_PERMITTED
        if (cb == nullptr)
            return runSync (std::move (native));
       #endif

        runAsync (std::move (native), rawToUniquePtr (cb));

        return 0;
    }

    ~ConcreteScopedMessageBoxImpl() override
    {
        cancelPendingUpdate();
    }

    z0 close() override
    {
        cancelPendingUpdate();
        nativeImplementation->close();
        self.reset();
    }

private:
    static std::shared_ptr<ConcreteScopedMessageBoxImpl> runAsync (std::unique_ptr<ScopedMessageBoxInterface>&& p,
                                                                   std::unique_ptr<ModalComponentManager::Callback>&& c)
    {
        std::shared_ptr<ConcreteScopedMessageBoxImpl> result (new ConcreteScopedMessageBoxImpl (std::move (p), std::move (c)));
        result->self = result;
        result->triggerAsyncUpdate();
        return result;
    }

    static i32 runSync (std::unique_ptr<ScopedMessageBoxInterface>&& p)
    {
        auto local = std::move (p);
        return local != nullptr ? local->runSync() : 0;
    }

    explicit ConcreteScopedMessageBoxImpl (std::unique_ptr<ScopedMessageBoxInterface>&& p)
        : ConcreteScopedMessageBoxImpl (std::move (p), nullptr) {}

    ConcreteScopedMessageBoxImpl (std::unique_ptr<ScopedMessageBoxInterface>&& p,
                                  std::unique_ptr<ModalComponentManager::Callback>&& c)
        : callback (std::move (c)), nativeImplementation (std::move (p)) {}

    z0 handleAsyncUpdate() override
    {
        nativeImplementation->runAsync ([weakRecipient = std::weak_ptr<ConcreteScopedMessageBoxImpl> (self)] (i32 result)
                                        {
                                            const auto notifyRecipient = [result, weakRecipient]
                                            {
                                                if (const auto locked = weakRecipient.lock())
                                                {
                                                    if (auto* cb = locked->callback.get())
                                                        cb->modalStateFinished (result);

                                                    locked->self.reset();
                                                }
                                            };

                                            if (MessageManager::getInstance()->isThisTheMessageThread())
                                                notifyRecipient();
                                            else
                                                MessageManager::callAsync (notifyRecipient);
                                        });
    }

    std::unique_ptr<ModalComponentManager::Callback> callback;
    std::unique_ptr<ScopedMessageBoxInterface> nativeImplementation;

    /*  The 'old' native message box API doesn't have a concept of message box owners.
        Instead, message boxes have to clean up after themselves, once they're done displaying.
        To allow this mode of usage, the implementation keeps an owning reference to itself,
        which is cleared once the message box is closed or asked to quit. To display a native
        message box without a scoped lifetime, just create a Pimpl instance without using
        the ScopedMessageBox wrapper, and the Pimpl will destroy itself after it is dismissed.
    */
    std::shared_ptr<ConcreteScopedMessageBoxImpl> self;
};

} // namespace drx::detail
