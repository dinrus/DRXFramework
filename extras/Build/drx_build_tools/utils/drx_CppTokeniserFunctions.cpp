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

namespace drx::build_tools
{

    static b8 isReservedKeyword (Txt::CharPointerType token, i32k tokenLength) noexcept
    {
        static tukk const keywords2Char[] =
                { "do", "if", "or", nullptr };

        static tukk const keywords3Char[] =
                { "and", "asm", "for", "i32", "new", "not", "try", "xor", nullptr };

        static tukk const keywords4Char[] =
                { "auto", "b8", "case", "t8", "else", "enum", "goto",
                  "i64", "this", "true", "z0", nullptr };

        static tukk const keywords5Char[] =
                { "bitor", "break", "catch", "class", "compl", "const", "false", "final",
                  "f32", "or_eq", "short", "throw", "union", "using", "while", nullptr };

        static tukk const keywords6Char[] =
                { "and_eq", "bitand", "delete", "f64", "export", "extern", "friend",
                  "import", "inline", "module", "not_eq", "public", "return", "signed",
                  "sizeof", "static", "struct", "switch", "typeid", "xor_eq", nullptr };

        static tukk const keywords7Char[] =
                { "__cdecl", "_Pragma", "alignas", "alignof", "concept", "default",
                  "mutable", "nullptr", "private", "typedef", "u8", "virtual",
                  "wchar_t", nullptr };

        static tukk const keywordsOther[] =
                { "@class", "@dynamic", "@end", "@implementation", "@interface", "@public",
                  "@private", "@protected", "@property", "@synthesize", "__fastcall", "__stdcall",
                  "atomic_cancel", "atomic_commit", "atomic_noexcept", "char16_t", "char32_t",
                  "co_await", "co_return", "co_yield", "const_cast", "constexpr", "continue",
                  "decltype", "dynamic_cast", "explicit", "namespace", "noexcept", "operator", "override",
                  "protected", "register", "reinterpret_cast", "requires", "static_assert",
                  "static_cast", "synchronized", "template", "thread_local", "typename", "u32",
                  "volatile", nullptr };

        tukk const* k;

        switch (tokenLength)
        {
            case 2:     k = keywords2Char; break;
            case 3:     k = keywords3Char; break;
            case 4:     k = keywords4Char; break;
            case 5:     k = keywords5Char; break;
            case 6:     k = keywords6Char; break;
            case 7:     k = keywords7Char; break;

            default:
                if (tokenLength < 2 || tokenLength > 16)
                    return false;

                k = keywordsOther;
                break;
        }

        for (i32 i = 0; k[i] != nullptr; ++i)
            if (token.compare (CharPointer_ASCII (k[i])) == 0)
                return true;

        return false;
    }

    static b8 isReservedKeyword (const Txt& token) noexcept
    {
        return isReservedKeyword (token.getCharPointer(), token.length());
    }

    //==============================================================================
    /** Takes a UTF8 string and writes it to a stream using standard C++ escape sequences for any
        non-ascii bytes.

        Although not strictly a tokenising function, this is still a function that often comes in
        handy when working with C++ code!

        Note that addEscapeChars() is easier to use than this function if you're working with Strings.

        @see addEscapeChars
    */
    static z0 writeEscapeChars (OutputStream& out, tukk utf8, i32k numBytesToRead,
                                  i32k maxCharsOnLine, const b8 breakAtNewLines,
                                  const b8 replaceSingleQuotes, const b8 allowStringBreaks)
    {
        i32 charsOnLine = 0;
        b8 lastWasHexEscapeCode = false;
        b8 trigraphDetected = false;

        for (i32 i = 0; i < numBytesToRead || numBytesToRead < 0; ++i)
        {
            auto c = (u8) utf8[i];
            b8 startNewLine = false;

            switch (c)
            {

                case '\t':  out << "\\t";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
                case '\r':  out << "\\r";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
                case '\n':  out << "\\n";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; startNewLine = breakAtNewLines; break;
                case '\\':  out << "\\\\"; trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
                case '\"':  out << "\\\""; trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;

                case '?':
                    if (trigraphDetected)
                    {
                        out << "\\?";
                        charsOnLine++;
                        trigraphDetected = false;
                    }
                    else
                    {
                        out << "?";
                        trigraphDetected = true;
                    }

                    lastWasHexEscapeCode = false;
                    charsOnLine++;
                    break;

                case 0:
                    if (numBytesToRead < 0)
                        return;

                    out << "\\0";
                    lastWasHexEscapeCode = true;
                    trigraphDetected = false;
                    charsOnLine += 2;
                    break;

                case '\'':
                    if (replaceSingleQuotes)
                    {
                        out << "\\\'";
                        lastWasHexEscapeCode = false;
                        trigraphDetected = false;
                        charsOnLine += 2;
                        break;
                    }

                    // deliberate fall-through...

                default:
                    if (c >= 32 && c < 127 && ! (lastWasHexEscapeCode  // (have to avoid following a hex escape sequence with a valid hex digit)
                                                 && CharacterFunctions::getHexDigitValue (c) >= 0))
                    {
                        out << (t8) c;
                        lastWasHexEscapeCode = false;
                        trigraphDetected = false;
                        ++charsOnLine;
                    }
                    else if (allowStringBreaks && lastWasHexEscapeCode && c >= 32 && c < 127)
                    {
                        out << "\"\"" << (t8) c;
                        lastWasHexEscapeCode = false;
                        trigraphDetected = false;
                        charsOnLine += 3;
                    }
                    else
                    {
                        out << (c < 16 ? "\\x0" : "\\x") << Txt::toHexString ((i32) c);
                        lastWasHexEscapeCode = true;
                        trigraphDetected = false;
                        charsOnLine += 4;
                    }

                    break;
            }

            if ((startNewLine || (maxCharsOnLine > 0 && charsOnLine >= maxCharsOnLine))
                && (numBytesToRead < 0 || i < numBytesToRead - 1))
            {
                charsOnLine = 0;
                out << "\"" << newLine << "\"";
                lastWasHexEscapeCode = false;
            }
        }
    }

} // namespace drx::build_tools
