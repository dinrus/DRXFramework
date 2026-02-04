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

MessageBoxOptions MessageBoxOptions::makeOptionsOk (MessageBoxIconType iconType,
                                                    const Txt& title,
                                                    const Txt& message,
                                                    const Txt& buttonText,
                                                    Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (buttonText.isEmpty() ? TRANS ("OK") : buttonText)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType iconType,
                                                          const Txt& title,
                                                          const Txt& message,
                                                          const Txt& button1Text,
                                                          const Txt& button2Text,
                                                          Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("OK") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("Cancel") : button2Text)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsYesNo (MessageBoxIconType iconType,
                                                       const Txt& title,
                                                       const Txt& message,
                                                       const Txt& button1Text,
                                                       const Txt& button2Text,
                                                       Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("Yes") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("No") : button2Text)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsYesNoCancel (MessageBoxIconType iconType,
                                                             const Txt& title,
                                                             const Txt& message,
                                                             const Txt& button1Text,
                                                             const Txt& button2Text,
                                                             const Txt& button3Text,
                                                             Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("Yes") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("No") : button2Text)
        .withButton (button3Text.isEmpty() ? TRANS ("Cancel") : button3Text)
        .withAssociatedComponent (associatedComponent);
}

} // namespace drx
