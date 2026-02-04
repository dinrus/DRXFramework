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
    class AndroidMessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit AndroidMessageBox (const MessageBoxOptions& o) : opts (o) {}

        z0 runAsync (std::function<z0 (i32)> recipient) override
        {
            const auto makeDialogListener = [&recipient] (i32 result)
            {
                return new DialogListener ([recipient, result] { recipient (result); });
            };

            auto* env = getEnv();

            LocalRef<jobject> builder (env->NewObject (AndroidAlertDialogBuilder, AndroidAlertDialogBuilder.construct, getMainActivity().get()));

            const auto setText = [&] (auto method, const Txt& text)
            {
                builder = LocalRef<jobject> (env->CallObjectMethod (builder, method, javaString (text).get()));
            };

            setText (AndroidAlertDialogBuilder.setTitle,   opts.getTitle());
            setText (AndroidAlertDialogBuilder.setMessage, opts.getMessage());
            builder = LocalRef<jobject> (env->CallObjectMethod (builder, AndroidAlertDialogBuilder.setCancelable, true));

            builder = LocalRef<jobject> (env->CallObjectMethod (builder, AndroidAlertDialogBuilder.setOnCancelListener,
                                                                CreateJavaInterface (makeDialogListener (0),
                                                                                     "android/content/DialogInterface$OnCancelListener").get()));

            const auto addButton = [&] (auto method, i32 index)
            {
                builder = LocalRef<jobject> (env->CallObjectMethod (builder,
                                                                    method,
                                                                    javaString (opts.getButtonText (index)).get(),
                                                                    CreateJavaInterface (makeDialogListener (index),
                                                                                         "android/content/DialogInterface$OnClickListener").get()));
            };

            addButton (AndroidAlertDialogBuilder.setPositiveButton, 0);

            if (opts.getButtonText (1).isNotEmpty())
                addButton (AndroidAlertDialogBuilder.setNegativeButton, 1);

            if (opts.getButtonText (2).isNotEmpty())
                addButton (AndroidAlertDialogBuilder.setNeutralButton, 2);

            dialog = GlobalRef (LocalRef<jobject> (env->CallObjectMethod (builder, AndroidAlertDialogBuilder.create)));

            LocalRef<jobject> window (env->CallObjectMethod (dialog, AndroidDialog.getWindow));

            if (Desktop::getInstance().getKioskModeComponent() != nullptr)
            {
                env->CallVoidMethod (window, AndroidWindow.setFlags, FLAG_NOT_FOCUSABLE, FLAG_NOT_FOCUSABLE);
                LocalRef<jobject> decorView (env->CallObjectMethod (window, AndroidWindow.getDecorView));
                env->CallVoidMethod (decorView, AndroidView.setSystemUiVisibility, fullScreenFlags);
            }

            env->CallVoidMethod (dialog, AndroidDialog.show);

            if (Desktop::getInstance().getKioskModeComponent() != nullptr)
                env->CallVoidMethod (window, AndroidWindow.clearFlags, FLAG_NOT_FOCUSABLE);
        }

        i32 runSync() override
        {
            // Not implemented on this platform.
            jassertfalse;
            return 0;
        }

        z0 close() override
        {
            if (dialog != nullptr)
                getEnv()->CallVoidMethod (dialog, AndroidDialogInterface.dismiss);
        }

    private:
        const MessageBoxOptions opts;
        GlobalRef dialog;
    };

    return std::make_unique<AndroidMessageBox> (options);
}

} // namespace drx::detail
