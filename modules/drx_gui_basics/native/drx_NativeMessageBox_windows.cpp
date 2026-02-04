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

std::unique_ptr<ScopedMessageBoxInterface> ScopedMessageBoxInterface::create (const MessageBoxOptions& options)
{
    class WindowsTaskDialog : public ScopedMessageBoxInterface
    {
    public:
        explicit WindowsTaskDialog (const MessageBoxOptions& opts)
            : associatedComponent (opts.getAssociatedComponent()), options (opts) {}

        z0 runAsync (std::function<z0 (i32)> recipient) override
        {
            future = std::async (std::launch::async, [showMessageBox = getShowMessageBox(), recipient]
            {
                const auto initComResult = CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

                if (initComResult != S_OK)
                    return;

                const ScopeGuard scope { [] { CoUninitialize(); } };

                const auto messageResult = showMessageBox != nullptr ? showMessageBox() : 0;
                NullCheckedInvocation::invoke (recipient, messageResult);
            });
        }

        i32 runSync() override
        {
            if (auto showMessageBox = getShowMessageBox())
                return showMessageBox();

            return 0;
        }

        z0 close() override
        {
            if (auto* toClose = windowHandle.exchange (nullptr))
                EndDialog (toClose, 0);
        }

        z0 setDialogWindowHandle (HWND dialogHandle)
        {
            windowHandle = dialogHandle;
        }

    private:
        std::function<i32()> getShowMessageBox()
        {
            const auto parent = associatedComponent != nullptr ? (HWND) associatedComponent->getWindowHandle() : nullptr;
            return getShowMessageBoxForParent (parent);
        }

        /*  Returns a function that should display a message box and return the result.

            getShowMessageBoxForParent() will be called on the message thread.

            The returned function will be called on a separate thread, in order to avoid blocking the
            message thread.

            'this' is guaranteed to be alive when the returned function is called.
        */
        std::function<i32()> getShowMessageBoxForParent (const HWND parent)
        {
            DRX_ASSERT_MESSAGE_THREAD

            return [this, parent]
            {
                const auto title = options.getTitle();
                const auto message = options.getMessage();

                TASKDIALOGCONFIG config{};

                config.cbSize         = sizeof (config);
                config.hwndParent     = parent;
                config.pszWindowTitle = title.toWideCharPointer();
                config.pszContent     = message.toWideCharPointer();
                config.hInstance      = (HINSTANCE) Process::getCurrentModuleInstanceHandle();
                config.lpCallbackData = reinterpret_cast<LONG_PTR> (this);
                config.pfCallback     = [] (HWND hwnd, UINT msg, WPARAM, LPARAM, LONG_PTR lpRefData)
                {
                    if (auto* t = reinterpret_cast<WindowsTaskDialog*> (lpRefData))
                    {
                        switch (msg)
                        {
                            case TDN_CREATED:
                            case TDN_DIALOG_CONSTRUCTED:
                                t->setDialogWindowHandle (hwnd);
                                break;

                            case TDN_DESTROYED:
                                t->setDialogWindowHandle (nullptr);
                                break;
                        }
                    }

                    return S_OK;
                };

                if (options.getIconType() == MessageBoxIconType::QuestionIcon)
                {
                    if (auto* questionIcon = LoadIcon (nullptr, IDI_QUESTION))
                    {
                        config.hMainIcon = questionIcon;
                        config.dwFlags |= TDF_USE_HICON_MAIN;
                    }
                }
                else
                {
                    config.pszMainIcon = [&]() -> LPWSTR
                    {
                        switch (options.getIconType())
                        {
                            case MessageBoxIconType::WarningIcon:   return TD_WARNING_ICON;
                            case MessageBoxIconType::InfoIcon:      return TD_INFORMATION_ICON;

                            case MessageBoxIconType::QuestionIcon:  DRX_FALLTHROUGH
                            case MessageBoxIconType::NoIcon:
                                break;
                        }

                        return nullptr;
                    }();
                }

                std::vector<Txt> buttonStrings;
                std::vector<TASKDIALOG_BUTTON> buttonLabels;

                for (auto i = 0; i < options.getNumButtons(); ++i)
                    if (const auto buttonText = options.getButtonText (i); buttonText.isNotEmpty())
                        buttonLabels.push_back ({ (i32) buttonLabels.size(), buttonStrings.emplace_back (buttonText).toWideCharPointer() });

                config.pButtons = buttonLabels.data();
                config.cButtons = (UINT) buttonLabels.size();

                i32 buttonIndex = 0;
                TaskDialogIndirect (&config, &buttonIndex, nullptr, nullptr);

                return buttonIndex;
            };
        }

        Component::SafePointer<Component> associatedComponent;
        std::atomic<HWND> windowHandle { nullptr };
        std::future<z0> future;
        MessageBoxOptions options;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsTaskDialog)
    };

    return std::make_unique<WindowsTaskDialog> (options);
}

} // namespace drx::detail
