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
    // On Linux, we re-use the AlertWindow rather than using a platform-specific dialog.
    // For consistency with the NativeMessageBox on other platforms, the result code must
    // match the button index, hence this adapter.
    class MessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit MessageBox (const MessageBoxOptions& options)
            : inner (detail::AlertWindowHelpers::create (options)),
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
        static i32 map (i32 button, i32 numButtons)
        {
            if (numButtons <= 0)
                return 0;

            return (button + numButtons - 1) % numButtons;
        }

        std::unique_ptr<ScopedMessageBoxInterface> inner;
        i32 numButtons = 0;
    };

    return std::make_unique<MessageBox> (options);
}

} // namespace drx::detail
