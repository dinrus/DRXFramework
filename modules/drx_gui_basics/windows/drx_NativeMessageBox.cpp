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

enum class ResultCodeMappingMode
{
    plainIndex,     // The result code is equal to the index of the selected button.
                    // This is used for NativeMessageBox::show, showAsync, and showMessageBox.
    alertWindow,    // The result code is mapped in the same way as AlertWindow, i.e. if there
                    // are N buttons then button X will return ((X + 1) % N).
};

static std::unique_ptr<detail::ScopedMessageBoxInterface> makeNativeMessageBoxWithMappedResult (const MessageBoxOptions& opts,
                                                                                                ResultCodeMappingMode mode)
{
    class Adapter final : public detail::ScopedMessageBoxInterface
    {
    public:
        explicit Adapter (const MessageBoxOptions& options)
            : inner (detail::ScopedMessageBoxInterface::create (options)),
              numButtons (options.getNumButtons()) {}

        z0 runAsync (std::function<z0 (i32)> fn) override
        {
            inner->runAsync ([fn, n = numButtons] (i32 result)
            {
                fn (map (result, n));
            });
        }

        i32 runSync() override
        {
            return map (inner->runSync(), numButtons);
        }

        z0 close() override
        {
            inner->close();
        }

    private:
        static i32 map (i32 button, i32 numButtons) { return (button + 1) % numButtons; }

        std::unique_ptr<detail::ScopedMessageBoxInterface> inner;
        i32 numButtons = 0;
    };

    return mode == ResultCodeMappingMode::plainIndex ? detail::ScopedMessageBoxInterface::create (opts)
                                                     : std::make_unique<Adapter> (opts);
}

static i32 showNativeBoxUnmanaged (const MessageBoxOptions& opts,
                                   ModalComponentManager::Callback* cb,
                                   ResultCodeMappingMode mode)
{
    auto implementation = makeNativeMessageBoxWithMappedResult (opts, mode);
    return detail::ConcreteScopedMessageBoxImpl::showUnmanaged (std::move (implementation), cb);
}

#if DRX_MODAL_LOOPS_PERMITTED
z0 DRX_CALLTYPE NativeMessageBox::showMessageBox (MessageBoxIconType iconType,
                                                     const Txt& title, const Txt& message,
                                                     Component* associatedComponent)
{
    showNativeBoxUnmanaged (MessageBoxOptions().withIconType (iconType)
                                               .withTitle (title)
                                               .withMessage (message)
                                               .withButton (TRANS ("OK"))
                                               .withAssociatedComponent (associatedComponent),
                            nullptr,
                            ResultCodeMappingMode::plainIndex);
}

i32 DRX_CALLTYPE NativeMessageBox::show (const MessageBoxOptions& options)
{
    return showNativeBoxUnmanaged (options, nullptr, ResultCodeMappingMode::plainIndex);
}
#endif

z0 DRX_CALLTYPE NativeMessageBox::showMessageBoxAsync (MessageBoxIconType iconType,
                                                          const Txt& title, const Txt& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsOk (iconType, title, message, {}, associatedComponent);
    showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::alertWindow);
}

b8 DRX_CALLTYPE NativeMessageBox::showOkCancelBox (MessageBoxIconType iconType,
                                                      const Txt& title, const Txt& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsOkCancel (iconType, title, message, {}, {}, associatedComponent);
    return showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::alertWindow) != 0;
}

i32 DRX_CALLTYPE NativeMessageBox::showYesNoCancelBox (MessageBoxIconType iconType,
                                                        const Txt& title, const Txt& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsYesNoCancel (iconType, title, message, {}, {}, {}, associatedComponent);
    return showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::alertWindow);
}

i32 DRX_CALLTYPE NativeMessageBox::showYesNoBox (MessageBoxIconType iconType,
                                                  const Txt& title, const Txt& message,
                                                  Component* associatedComponent,
                                                  ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsYesNo (iconType, title, message, {}, {}, associatedComponent);
    return showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::alertWindow);
}

z0 DRX_CALLTYPE NativeMessageBox::showAsync (const MessageBoxOptions& options,
                                                ModalComponentManager::Callback* callback)
{
    showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::plainIndex);
}

z0 DRX_CALLTYPE NativeMessageBox::showAsync (const MessageBoxOptions& options,
                                                std::function<z0 (i32)> callback)
{
    showAsync (options, ModalCallbackFunction::create (callback));
}

ScopedMessageBox NativeMessageBox::showScopedAsync (const MessageBoxOptions& options, std::function<z0 (i32)> callback)
{
    auto implementation = makeNativeMessageBoxWithMappedResult (options, ResultCodeMappingMode::alertWindow);
    return detail::ConcreteScopedMessageBoxImpl::show (std::move (implementation), std::move (callback));
}

} // namespace drx
