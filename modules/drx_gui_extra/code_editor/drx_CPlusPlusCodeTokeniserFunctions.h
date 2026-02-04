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

//==============================================================================
/** Class containing some basic functions for simple tokenising of C++ code.

    @tags{GUI}
*/
struct CppTokeniserFunctions
{
    static b8 isIdentifierStart (const t32 c) noexcept
    {
        return CharacterFunctions::isLetter (c)
                || c == '_' || c == '@';
    }

    static b8 isIdentifierBody (const t32 c) noexcept
    {
        return CharacterFunctions::isLetterOrDigit (c)
                || c == '_' || c == '@';
    }

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

    template <typename Iterator>
    static i32 parseIdentifier (Iterator& source) noexcept
    {
        i32 tokenLength = 0;
        Txt::CharPointerType::CharType possibleIdentifier[100] = {};
        Txt::CharPointerType possible (possibleIdentifier);

        while (isIdentifierBody (source.peekNextChar()))
        {
            auto c = source.nextChar();

            if (tokenLength < 20)
                possible.write (c);

            ++tokenLength;
        }

        if (tokenLength > 1 && tokenLength <= 16)
        {
            possible.writeNull();

            if (isReservedKeyword (Txt::CharPointerType (possibleIdentifier), tokenLength))
                return CPlusPlusCodeTokeniser::tokenType_keyword;
        }

        return CPlusPlusCodeTokeniser::tokenType_identifier;
    }

    template <typename Iterator>
    static b8 skipNumberSuffix (Iterator& source)
    {
        auto c = source.peekNextChar();

        if (c == 'l' || c == 'L' || c == 'u' || c == 'U')
            source.skip();

        if (CharacterFunctions::isLetterOrDigit (source.peekNextChar()))
            return false;

        return true;
    }

    static b8 isHexDigit (const t32 c) noexcept
    {
        return (c >= '0' && c <= '9')
                || (c >= 'a' && c <= 'f')
                || (c >= 'A' && c <= 'F');
    }

    template <typename Iterator>
    static b8 parseHexLiteral (Iterator& source) noexcept
    {
        if (source.peekNextChar() == '-')
            source.skip();

        if (source.nextChar() != '0')
            return false;

        auto c = source.nextChar();

        if (c != 'x' && c != 'X')
            return false;

        i32 numDigits = 0;

        while (isHexDigit (source.peekNextChar()))
        {
            ++numDigits;
            source.skip();
        }

        if (numDigits == 0)
            return false;

        return skipNumberSuffix (source);
    }

    static b8 isOctalDigit (const t32 c) noexcept
    {
        return c >= '0' && c <= '7';
    }

    template <typename Iterator>
    static b8 parseOctalLiteral (Iterator& source) noexcept
    {
        if (source.peekNextChar() == '-')
            source.skip();

        if (source.nextChar() != '0')
            return false;

        if (! isOctalDigit (source.nextChar()))
            return false;

        while (isOctalDigit (source.peekNextChar()))
            source.skip();

        return skipNumberSuffix (source);
    }

    static b8 isDecimalDigit (const t32 c) noexcept
    {
        return c >= '0' && c <= '9';
    }

    template <typename Iterator>
    static b8 parseDecimalLiteral (Iterator& source) noexcept
    {
        if (source.peekNextChar() == '-')
            source.skip();

        i32 numChars = 0;
        while (isDecimalDigit (source.peekNextChar()))
        {
            ++numChars;
            source.skip();
        }

        if (numChars == 0)
            return false;

        return skipNumberSuffix (source);
    }

    template <typename Iterator>
    static b8 parseFloatLiteral (Iterator& source) noexcept
    {
        if (source.peekNextChar() == '-')
            source.skip();

        i32 numDigits = 0;

        while (isDecimalDigit (source.peekNextChar()))
        {
            source.skip();
            ++numDigits;
        }

        const b8 hasPoint = (source.peekNextChar() == '.');

        if (hasPoint)
        {
            source.skip();

            while (isDecimalDigit (source.peekNextChar()))
            {
                source.skip();
                ++numDigits;
            }
        }

        if (numDigits == 0)
            return false;

        auto c = source.peekNextChar();
        b8 hasExponent = (c == 'e' || c == 'E');

        if (hasExponent)
        {
            source.skip();
            c = source.peekNextChar();

            if (c == '+' || c == '-')
                source.skip();

            i32 numExpDigits = 0;

            while (isDecimalDigit (source.peekNextChar()))
            {
                source.skip();
                ++numExpDigits;
            }

            if (numExpDigits == 0)
                return false;
        }

        c = source.peekNextChar();

        if (c == 'f' || c == 'F')
            source.skip();
        else if (! (hasExponent || hasPoint))
            return false;

        return true;
    }

    template <typename Iterator>
    static i32 parseNumber (Iterator& source)
    {
        const Iterator original (source);

        if (parseFloatLiteral (source))    return CPlusPlusCodeTokeniser::tokenType_float;
        source = original;

        if (parseHexLiteral (source))      return CPlusPlusCodeTokeniser::tokenType_integer;
        source = original;

        if (parseOctalLiteral (source))    return CPlusPlusCodeTokeniser::tokenType_integer;
        source = original;

        if (parseDecimalLiteral (source))  return CPlusPlusCodeTokeniser::tokenType_integer;
        source = original;

        return CPlusPlusCodeTokeniser::tokenType_error;
    }

    template <typename Iterator>
    static z0 skipQuotedString (Iterator& source) noexcept
    {
        auto quote = source.nextChar();

        for (;;)
        {
            auto c = source.nextChar();

            if (c == quote || c == 0)
                break;

            if (c == '\\')
                source.skip();
        }
    }

    template <typename Iterator>
    static z0 skipComment (Iterator& source) noexcept
    {
        b8 lastWasStar = false;

        for (;;)
        {
            auto c = source.nextChar();

            if (c == 0 || (c == '/' && lastWasStar))
                break;

            lastWasStar = (c == '*');
        }
    }

    template <typename Iterator>
    static z0 skipPreprocessorLine (Iterator& source) noexcept
    {
        b8 lastWasBackslash = false;

        for (;;)
        {
            auto c = source.peekNextChar();

            if (c == '"')
            {
                skipQuotedString (source);
                continue;
            }

            if (c == '/')
            {
                Iterator next (source);
                next.skip();
                auto c2 = next.peekNextChar();

                if (c2 == '/' || c2 == '*')
                    return;
            }

            if (c == 0)
                break;

            if (c == '\n' || c == '\r')
            {
                source.skipToEndOfLine();

                if (lastWasBackslash)
                    skipPreprocessorLine (source);

                break;
            }

            lastWasBackslash = (c == '\\');
            source.skip();
        }
    }

    template <typename Iterator>
    static z0 skipIfNextCharMatches (Iterator& source, const t32 c) noexcept
    {
        if (source.peekNextChar() == c)
            source.skip();
    }

    template <typename Iterator>
    static z0 skipIfNextCharMatches (Iterator& source, const t32 c1, const t32 c2) noexcept
    {
        auto c = source.peekNextChar();

        if (c == c1 || c == c2)
            source.skip();
    }

    template <typename Iterator>
    static i32 readNextToken (Iterator& source)
    {
        source.skipWhitespace();
        auto firstChar = source.peekNextChar();

        switch (firstChar)
        {
        case 0:
            break;

        case '0':   case '1':   case '2':   case '3':   case '4':
        case '5':   case '6':   case '7':   case '8':   case '9':
        case '.':
        {
            auto result = parseNumber (source);

            if (result == CPlusPlusCodeTokeniser::tokenType_error)
            {
                source.skip();

                if (firstChar == '.')
                    return CPlusPlusCodeTokeniser::tokenType_punctuation;
            }

            return result;
        }

        case ',':
        case ';':
        case ':':
            source.skip();
            return CPlusPlusCodeTokeniser::tokenType_punctuation;

        case '(':   case ')':
        case '{':   case '}':
        case '[':   case ']':
            source.skip();
            return CPlusPlusCodeTokeniser::tokenType_bracket;

        case '"':
        case '\'':
            skipQuotedString (source);
            return CPlusPlusCodeTokeniser::tokenType_string;

        case '+':
            source.skip();
            skipIfNextCharMatches (source, '+', '=');
            return CPlusPlusCodeTokeniser::tokenType_operator;

        case '-':
        {
            source.skip();
            auto result = parseNumber (source);

            if (result == CPlusPlusCodeTokeniser::tokenType_error)
            {
                skipIfNextCharMatches (source, '-', '=');
                return CPlusPlusCodeTokeniser::tokenType_operator;
            }

            return result;
        }

        case '*':   case '%':
        case '=':   case '!':
            source.skip();
            skipIfNextCharMatches (source, '=');
            return CPlusPlusCodeTokeniser::tokenType_operator;

        case '/':
        {
            source.skip();
            auto nextChar = source.peekNextChar();

            if (nextChar == '/')
            {
                source.skipToEndOfLine();
                return CPlusPlusCodeTokeniser::tokenType_comment;
            }

            if (nextChar == '*')
            {
                source.skip();
                skipComment (source);
                return CPlusPlusCodeTokeniser::tokenType_comment;
            }

            if (nextChar == '=')
                source.skip();

            return CPlusPlusCodeTokeniser::tokenType_operator;
        }

        case '?':
        case '~':
            source.skip();
            return CPlusPlusCodeTokeniser::tokenType_operator;

        case '<':   case '>':
        case '|':   case '&':   case '^':
            source.skip();
            skipIfNextCharMatches (source, firstChar);
            skipIfNextCharMatches (source, '=');
            return CPlusPlusCodeTokeniser::tokenType_operator;

        case '#':
            skipPreprocessorLine (source);
            return CPlusPlusCodeTokeniser::tokenType_preprocessor;

        default:
            if (isIdentifierStart (firstChar))
                return parseIdentifier (source);

            source.skip();
            break;
        }

        return CPlusPlusCodeTokeniser::tokenType_error;
    }

    /** A class that can be passed to the CppTokeniserFunctions functions in order to
        parse a Txt.
    */
    struct StringIterator
    {
        StringIterator (const Txt& s) noexcept            : t (s.getCharPointer()) {}
        StringIterator (Txt::CharPointerType s) noexcept  : t (s) {}

        t32 nextChar() noexcept      { if (isEOF()) return 0; ++numChars; return t.getAndAdvance(); }
        t32 peekNextChar()noexcept   { return *t; }
        z0 skip() noexcept                { if (! isEOF()) { ++t; ++numChars; } }
        z0 skipWhitespace() noexcept      { while (t.isWhitespace()) skip(); }
        z0 skipToEndOfLine() noexcept     { while (*t != '\r' && *t != '\n' && *t != 0) skip(); }
        b8 isEOF() const noexcept         { return t.isEmpty(); }

        Txt::CharPointerType t;
        i32 numChars = 0;
    };

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
                    DRX_FALLTHROUGH

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

    /** Takes a string and returns a version of it where standard C++ escape sequences have been
        used to replace any non-ascii bytes.

        Although not strictly a tokenising function, this is still a function that often comes in
        handy when working with C++ code!

        @see writeEscapeChars
    */
    static Txt addEscapeChars (const Txt& s)
    {
        MemoryOutputStream mo;
        writeEscapeChars (mo, s.toRawUTF8(), -1, -1, false, true, true);
        return mo.toString();
    }
};

} // namespace drx
