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

struct LuaTokeniserFunctions
{
    static b8 isReservedKeyword (Txt::CharPointerType token, i32k tokenLength) noexcept
    {
        static tukk const keywords2Char[] =
            { "if", "or", "in", "do", nullptr };

        static tukk const keywords3Char[] =
            { "and", "end", "for", "nil", "not", nullptr };

        static tukk const keywords4Char[] =
            { "then", "true", "else", nullptr };

        static tukk const keywords5Char[] =
            {  "false", "local", "until", "while", "break", nullptr };

        static tukk const keywords6Char[] =
            { "repeat", "return", "elseif", nullptr};

        static tukk const keywordsOther[] =
            { "function", "@interface", "@end", "@synthesize", "@dynamic", "@public",
              "@private", "@property", "@protected", "@class", nullptr };

        tukk const* k;

        switch (tokenLength)
        {
            case 2:   k = keywords2Char; break;
            case 3:   k = keywords3Char; break;
            case 4:   k = keywords4Char; break;
            case 5:   k = keywords5Char; break;
            case 6:   k = keywords6Char; break;

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

        while (CppTokeniserFunctions::isIdentifierBody (source.peekNextChar()))
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
                return LuaTokeniser::tokenType_keyword;
        }

        return LuaTokeniser::tokenType_identifier;
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
            auto result = CppTokeniserFunctions::parseNumber (source);

            if (result == LuaTokeniser::tokenType_error)
            {
                source.skip();

                if (firstChar == '.')
                    return LuaTokeniser::tokenType_punctuation;
            }

            return result;
        }

        case ',':
        case ';':
        case ':':
            source.skip();
            return LuaTokeniser::tokenType_punctuation;

        case '(':   case ')':
        case '{':   case '}':
        case '[':   case ']':
            source.skip();
            return LuaTokeniser::tokenType_bracket;

        case '"':
        case '\'':
            CppTokeniserFunctions::skipQuotedString (source);
            return LuaTokeniser::tokenType_string;

        case '+':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '+', '=');
            return LuaTokeniser::tokenType_operator;

        case '-':
        {
            source.skip();
            auto result = CppTokeniserFunctions::parseNumber (source);

            if (source.peekNextChar() == '-')
            {
                source.skipToEndOfLine();
                return LuaTokeniser::tokenType_comment;
            }

            if (result == LuaTokeniser::tokenType_error)
            {
                CppTokeniserFunctions::skipIfNextCharMatches (source, '-', '=');
                return LuaTokeniser::tokenType_operator;
            }

            return result;
        }

        case '*':   case '%':
        case '=':   case '!':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '=');
            return LuaTokeniser::tokenType_operator;

        case '?':
        case '~':
            source.skip();
            return LuaTokeniser::tokenType_operator;

        case '<':   case '>':
        case '|':   case '&':   case '^':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, firstChar);
            CppTokeniserFunctions::skipIfNextCharMatches (source, '=');
            return LuaTokeniser::tokenType_operator;

        default:
            if (CppTokeniserFunctions::isIdentifierStart (firstChar))
                return parseIdentifier (source);

            source.skip();
            break;
        }

        return LuaTokeniser::tokenType_error;
    }
};

//==============================================================================
LuaTokeniser::LuaTokeniser() {}
LuaTokeniser::~LuaTokeniser() {}

i32 LuaTokeniser::readNextToken (CodeDocument::Iterator& source)
{
    return LuaTokeniserFunctions::readNextToken (source);
}

CodeEditorComponent::ColorScheme LuaTokeniser::getDefaultColorScheme()
{
    static const CodeEditorComponent::ColorScheme::TokenType types[] =
    {
        { "Error",          Color (0xffcc0000) },
        { "Comment",        Color (0xff3c3c3c) },
        { "Keyword",        Color (0xff0000cc) },
        { "Operator",       Color (0xff225500) },
        { "Identifier",     Color (0xff000000) },
        { "Integer",        Color (0xff880000) },
        { "Float",          Color (0xff885500) },
        { "Txt",         Color (0xff990099) },
        { "Bracket",        Color (0xff000055) },
        { "Punctuation",    Color (0xff004400) }
    };

    CodeEditorComponent::ColorScheme cs;

    for (auto& t : types)
        cs.set (t.name, Color (t.colour));

    return cs;
}

} // namespace drx
