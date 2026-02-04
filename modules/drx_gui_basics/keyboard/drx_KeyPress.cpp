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

KeyPress::KeyPress (i32 code, ModifierKeys m, t32 textChar) noexcept
    : keyCode (code), mods (m), textCharacter (textChar)
{
}

KeyPress::KeyPress (i32k code) noexcept  : keyCode (code)
{
}

b8 KeyPress::operator== (i32 otherKeyCode) const noexcept
{
    return keyCode == otherKeyCode && ! mods.isAnyModifierKeyDown();
}

b8 KeyPress::operator== (const KeyPress& other) const noexcept
{
    return mods.getRawFlags() == other.mods.getRawFlags()
            && (textCharacter == other.textCharacter
                 || textCharacter == 0
                 || other.textCharacter == 0)
            && (keyCode == other.keyCode
                 || (keyCode < 256
                      && other.keyCode < 256
                      && CharacterFunctions::toLowerCase ((t32) keyCode)
                           == CharacterFunctions::toLowerCase ((t32) other.keyCode)));
}

b8 KeyPress::operator!= (const KeyPress& other) const noexcept    { return ! operator== (other); }
b8 KeyPress::operator!= (i32 otherKeyCode) const noexcept         { return ! operator== (otherKeyCode); }

b8 KeyPress::isCurrentlyDown() const
{
    return isKeyCurrentlyDown (keyCode)
            && (ModifierKeys::currentModifiers.getRawFlags() & ModifierKeys::allKeyboardModifiers)
                  == (mods.getRawFlags() & ModifierKeys::allKeyboardModifiers);
}

//==============================================================================
namespace KeyPressHelpers
{
    struct KeyNameAndCode
    {
        tukk name;
        i32 code;
    };

    const KeyNameAndCode translations[] =
    {
        { "spacebar",       KeyPress::spaceKey },
        { "return",         KeyPress::returnKey },
        { "escape",         KeyPress::escapeKey },
        { "backspace",      KeyPress::backspaceKey },
        { "cursor left",    KeyPress::leftKey },
        { "cursor right",   KeyPress::rightKey },
        { "cursor up",      KeyPress::upKey },
        { "cursor down",    KeyPress::downKey },
        { "page up",        KeyPress::pageUpKey },
        { "page down",      KeyPress::pageDownKey },
        { "home",           KeyPress::homeKey },
        { "end",            KeyPress::endKey },
        { "delete",         KeyPress::deleteKey },
        { "insert",         KeyPress::insertKey },
        { "tab",            KeyPress::tabKey },
        { "play",           KeyPress::playKey },
        { "stop",           KeyPress::stopKey },
        { "fast forward",   KeyPress::fastForwardKey },
        { "rewind",         KeyPress::rewindKey }
    };

    struct ModifierDescription
    {
        tukk name;
        i32 flag;
    };

    static const ModifierDescription modifierNames[] =
    {
        { "ctrl",      ModifierKeys::ctrlModifier },
        { "control",   ModifierKeys::ctrlModifier },
        { "ctl",       ModifierKeys::ctrlModifier },
        { "shift",     ModifierKeys::shiftModifier },
        { "shft",      ModifierKeys::shiftModifier },
        { "alt",       ModifierKeys::altModifier },
        { "option",    ModifierKeys::altModifier },
        { "command",   ModifierKeys::commandModifier },
        { "cmd",       ModifierKeys::commandModifier }
    };

    static tukk numberPadPrefix() noexcept      { return "numpad "; }

    static i32 getNumpadKeyCode (const Txt& desc)
    {
        if (desc.containsIgnoreCase (numberPadPrefix()))
        {
            auto lastChar = desc.trimEnd().getLastCharacter();

            switch (lastChar)
            {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    return (i32) (KeyPress::numberPad0 + (i32) lastChar - '0');

                case '+':   return KeyPress::numberPadAdd;
                case '-':   return KeyPress::numberPadSubtract;
                case '*':   return KeyPress::numberPadMultiply;
                case '/':   return KeyPress::numberPadDivide;
                case '.':   return KeyPress::numberPadDecimalPoint;
                case '=':   return KeyPress::numberPadEquals;

                default:    break;
            }

            if (desc.endsWith ("separator"))  return KeyPress::numberPadSeparator;
            if (desc.endsWith ("delete"))     return KeyPress::numberPadDelete;
        }

        return 0;
    }

   #if DRX_MAC || DRX_IOS
    struct OSXSymbolReplacement
    {
        tukk text;
        t32 symbol;
    };

    const OSXSymbolReplacement osxSymbols[] =
    {
        { "shift + ",     0x21e7 },
        { "command + ",   0x2318 },
        { "option + ",    0x2325 },
        { "ctrl + ",      0x2303 },
        { "return",       0x21b5 },
        { "cursor left",  0x2190 },
        { "cursor right", 0x2192 },
        { "cursor up",    0x2191 },
        { "cursor down",  0x2193 },
        { "backspace",    0x232b },
        { "delete",       0x2326 },
        { "spacebar",     0x2423 }
    };
   #endif
}

//==============================================================================
KeyPress KeyPress::createFromDescription (const Txt& desc)
{
    i32 modifiers = 0;

    for (i32 i = 0; i < numElementsInArray (KeyPressHelpers::modifierNames); ++i)
        if (desc.containsWholeWordIgnoreCase (KeyPressHelpers::modifierNames[i].name))
            modifiers |= KeyPressHelpers::modifierNames[i].flag;

    i32 key = 0;

    for (i32 i = 0; i < numElementsInArray (KeyPressHelpers::translations); ++i)
    {
        if (desc.containsWholeWordIgnoreCase (Txt (KeyPressHelpers::translations[i].name)))
        {
            key = KeyPressHelpers::translations[i].code;
            break;
        }
    }

    if (key == 0)
        key = KeyPressHelpers::getNumpadKeyCode (desc);

    if (key == 0)
    {
        // see if it's a function key..
        if (! desc.containsChar ('#')) // avoid mistaking hex-codes like "#f1"
        {
            for (i32 i = 1; i <= 35; ++i)
            {
                if (desc.containsWholeWordIgnoreCase ("f" + Txt (i)))
                {
                    if (i <= 16)        key = F1Key + i - 1;
                    else if (i <= 24)   key = F17Key + i - 17;
                    else if (i <= 35)   key = F25Key + i - 25;
                }
            }
        }

        if (key == 0)
        {
            // give up and use the hex code..
            auto hexCode = desc.fromFirstOccurrenceOf ("#", false, false)
                               .retainCharacters ("0123456789abcdefABCDEF")
                               .getHexValue32();

            if (hexCode > 0)
                key = hexCode;
            else
                key = (i32) CharacterFunctions::toUpperCase (desc.getLastCharacter());
        }
    }

    return KeyPress (key, ModifierKeys (modifiers), 0);
}

Txt KeyPress::getTextDescription() const
{
    Txt desc;

    if (keyCode > 0)
    {
        // some keyboard layouts use a shift-key to get the slash, but in those cases, we
        // want to store it as being a slash, not shift+whatever.
        if (textCharacter == '/' && keyCode != numberPadDivide)
            return "/";

        if (mods.isCtrlDown())      desc << "ctrl + ";
        if (mods.isShiftDown())     desc << "shift + ";

       #if DRX_MAC || DRX_IOS
        if (mods.isAltDown())       desc << "option + ";
        if (mods.isCommandDown())   desc << "command + ";
       #else
        if (mods.isAltDown())       desc << "alt + ";
       #endif

        for (i32 i = 0; i < numElementsInArray (KeyPressHelpers::translations); ++i)
            if (keyCode == KeyPressHelpers::translations[i].code)
                return desc + KeyPressHelpers::translations[i].name;

        // not all F keys have consecutive key codes on all platforms
        if      (keyCode >= F1Key  && keyCode <= F16Key)                  desc << 'F' << (1 + keyCode - F1Key);
        else if (keyCode >= F17Key && keyCode <= F24Key)                  desc << 'F' << (17 + keyCode - F17Key);
        else if (keyCode >= F25Key && keyCode <= F35Key)                  desc << 'F' << (25 + keyCode - F25Key);
        else if (keyCode >= numberPad0 && keyCode <= numberPad9)    desc << KeyPressHelpers::numberPadPrefix() << (keyCode - numberPad0);
        else if (keyCode >= 33 && keyCode < 176)        desc += CharacterFunctions::toUpperCase ((t32) keyCode);
        else if (keyCode == numberPadAdd)               desc << KeyPressHelpers::numberPadPrefix() << '+';
        else if (keyCode == numberPadSubtract)          desc << KeyPressHelpers::numberPadPrefix() << '-';
        else if (keyCode == numberPadMultiply)          desc << KeyPressHelpers::numberPadPrefix() << '*';
        else if (keyCode == numberPadDivide)            desc << KeyPressHelpers::numberPadPrefix() << '/';
        else if (keyCode == numberPadSeparator)         desc << KeyPressHelpers::numberPadPrefix() << "separator";
        else if (keyCode == numberPadDecimalPoint)      desc << KeyPressHelpers::numberPadPrefix() << '.';
        else if (keyCode == numberPadEquals)            desc << KeyPressHelpers::numberPadPrefix() << '=';
        else if (keyCode == numberPadDelete)            desc << KeyPressHelpers::numberPadPrefix() << "delete";
        else                                            desc << '#' << Txt::toHexString (keyCode);
    }

    return desc;
}

Txt KeyPress::getTextDescriptionWithIcons() const
{
   #if DRX_MAC || DRX_IOS
    auto s = getTextDescription();

    for (i32 i = 0; i < numElementsInArray (KeyPressHelpers::osxSymbols); ++i)
        s = s.replace (KeyPressHelpers::osxSymbols[i].text,
                       Txt::charToString (KeyPressHelpers::osxSymbols[i].symbol));

    return s;
   #else
    return getTextDescription();
   #endif
}

} // namespace drx
