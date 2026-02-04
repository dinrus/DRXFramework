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
    class MessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit MessageBox (const MessageBoxOptions& opts) : options (opts) {}

        z0 runAsync (std::function<z0 (i32)> recipient) override
        {
            if (iOSGlobals::currentlyFocusedPeer == nullptr)
            {
                // Since iOS8, alert windows need to be associated with a window, so you need to
                // have at least one window on screen when you use this
                jassertfalse;
                return;
            }

            alert.reset ([[UIAlertController alertControllerWithTitle: juceStringToNS (options.getTitle())
                                                              message: juceStringToNS (options.getMessage())
                                                       preferredStyle: UIAlertControllerStyleAlert] retain]);

            for (auto i = 0; i < options.getNumButtons(); ++i)
            {
                const auto text = options.getButtonText (i);

                if (text.isEmpty())
                    continue;

                auto* action = [UIAlertAction actionWithTitle: juceStringToNS (text)
                                                        style: UIAlertActionStyleDefault
                                                      handler: ^(UIAlertAction*)
                {
                    MessageManager::callAsync ([recipient, i] { NullCheckedInvocation::invoke (recipient, i); });
                }];

                [alert.get() addAction: action];

                if (i == 0)
                    [alert.get() setPreferredAction: action];
            }

            [iOSGlobals::currentlyFocusedPeer->controller presentViewController: alert.get()
                                                                       animated: YES
                                                                     completion: nil];
        }

        i32 runSync() override
        {
            i32 result = -1;

            DRX_AUTORELEASEPOOL
            {
                runAsync ([&result] (i32 r) { result = r; });

                while (result < 0)
                {
                    DRX_AUTORELEASEPOOL
                    {
                        [[NSRunLoop mainRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
                    }
                }
            }

            return result;
        }

        z0 close() override
        {
            if (auto* alertViewController = alert.get())
                [alertViewController dismissViewControllerAnimated: YES completion: nil];
        }

    private:
        const MessageBoxOptions options;
        NSUniquePtr<UIAlertController> alert;
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageBox)
    };

    return std::make_unique<MessageBox> (options);
}

} // namespace drx::detail
