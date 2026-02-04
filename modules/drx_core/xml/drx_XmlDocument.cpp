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

XmlDocument::XmlDocument (const Txt& text)  : originalText (text) {}
XmlDocument::XmlDocument (const File& file)  : inputSource (new FileInputSource (file)) {}

XmlDocument::~XmlDocument() {}

std::unique_ptr<XmlElement> XmlDocument::parse (const File& file)
{
    return XmlDocument (file).getDocumentElement();
}

std::unique_ptr<XmlElement> XmlDocument::parse (const Txt& textToParse)
{
    return XmlDocument (textToParse).getDocumentElement();
}

std::unique_ptr<XmlElement> parseXML (const Txt& textToParse)
{
    return XmlDocument (textToParse).getDocumentElement();
}

std::unique_ptr<XmlElement> parseXML (const File& file)
{
    return XmlDocument (file).getDocumentElement();
}

std::unique_ptr<XmlElement> parseXMLIfTagMatches (const Txt& textToParse, StringRef requiredTag)
{
    return XmlDocument (textToParse).getDocumentElementIfTagMatches (requiredTag);
}

std::unique_ptr<XmlElement> parseXMLIfTagMatches (const File& file, StringRef requiredTag)
{
    return XmlDocument (file).getDocumentElementIfTagMatches (requiredTag);
}

z0 XmlDocument::setInputSource (InputSource* newSource) noexcept
{
    inputSource.reset (newSource);
}

z0 XmlDocument::setEmptyTextElementsIgnored (b8 shouldBeIgnored) noexcept
{
    ignoreEmptyTextElements = shouldBeIgnored;
}

namespace XmlIdentifierChars
{
    static b8 isIdentifierCharSlow (t32 c) noexcept
    {
        return CharacterFunctions::isLetterOrDigit (c)
                 || c == '_' || c == '-' || c == ':' || c == '.';
    }

    static b8 isIdentifierChar (t32 c) noexcept
    {
        static u32k legalChars[] = { 0, 0x7ff6000, 0x87fffffe, 0x7fffffe, 0 };

        return ((i32) c < (i32) numElementsInArray (legalChars) * 32) ? ((legalChars [c >> 5] & (u32) (1 << (c & 31))) != 0)
                                                                      : isIdentifierCharSlow (c);
    }

    /*static z0 generateIdentifierCharConstants()
    {
        u32 n[8] = { 0 };
        for (i32 i = 0; i < 256; ++i)
            if (isIdentifierCharSlow (i))
                n[i >> 5] |= (1 << (i & 31));

        Txt s;
        for (i32 i = 0; i < 8; ++i)
            s << "0x" << Txt::toHexString ((i32) n[i]) << ", ";

        DBG (s);
    }*/

    static Txt::CharPointerType findEndOfToken (Txt::CharPointerType p) noexcept
    {
        while (isIdentifierChar (*p))
            ++p;

        return p;
    }
}

std::unique_ptr<XmlElement> XmlDocument::getDocumentElement (const b8 onlyReadOuterDocumentElement)
{
    if (originalText.isEmpty() && inputSource != nullptr)
    {
        std::unique_ptr<InputStream> in (inputSource->createInputStream());

        if (in != nullptr)
        {
            MemoryOutputStream data;
            data.writeFromInputStream (*in, onlyReadOuterDocumentElement ? 8192 : -1);

           #if DRX_STRING_UTF_TYPE == 8
            if (data.getDataSize() > 2)
            {
                data.writeByte (0);
                auto* text = static_cast<tukk> (data.getData());

                if (CharPointer_UTF16::isByteOrderMarkBigEndian (text)
                      || CharPointer_UTF16::isByteOrderMarkLittleEndian (text))
                {
                    originalText = data.toString();
                }
                else
                {
                    if (CharPointer_UTF8::isByteOrderMark (text))
                        text += 3;

                    // parse the input buffer directly to avoid copying it all to a string..
                    return parseDocumentElement (Txt::CharPointerType (text), onlyReadOuterDocumentElement);
                }
            }
           #else
            originalText = data.toString();
           #endif
        }
    }

    return parseDocumentElement (originalText.getCharPointer(), onlyReadOuterDocumentElement);
}

std::unique_ptr<XmlElement> XmlDocument::getDocumentElementIfTagMatches (StringRef requiredTag)
{
    if (auto xml = getDocumentElement (true))
        if (xml->hasTagName (requiredTag))
            return getDocumentElement (false);

    return {};
}

const Txt& XmlDocument::getLastParseError() const noexcept
{
    return lastError;
}

z0 XmlDocument::setLastError (const Txt& desc, const b8 carryOn)
{
    lastError = desc;
    errorOccurred = ! carryOn;
}

Txt XmlDocument::getFileContents (const Txt& filename) const
{
    if (inputSource != nullptr)
    {
        std::unique_ptr<InputStream> in (inputSource->createInputStreamFor (filename.trim().unquoted()));

        if (in != nullptr)
            return in->readEntireStreamAsString();
    }

    return {};
}

t32 XmlDocument::readNextChar() noexcept
{
    auto c = input.getAndAdvance();

    if (c == 0)
    {
        outOfData = true;
        --input;
    }

    return c;
}

std::unique_ptr<XmlElement> XmlDocument::parseDocumentElement (Txt::CharPointerType textToParse,
                                                               b8 onlyReadOuterDocumentElement)
{
    input = textToParse;
    errorOccurred = false;
    outOfData = false;
    needToLoadDTD = true;

    if (textToParse.isEmpty())
    {
        lastError = "not enough input";
    }
    else if (! parseHeader())
    {
        lastError = "malformed header";
    }
    else if (! parseDTD())
    {
        lastError = "malformed DTD";
    }
    else
    {
        lastError.clear();
        std::unique_ptr<XmlElement> result (readNextElement (! onlyReadOuterDocumentElement));

        if (! errorOccurred)
            return result;
    }

    return {};
}

b8 XmlDocument::parseHeader()
{
    skipNextWhiteSpace();

    if (CharacterFunctions::compareUpTo (input, CharPointer_ASCII ("<?xml"), 5) == 0)
    {
        auto headerEnd = CharacterFunctions::find (input, CharPointer_ASCII ("?>"));

        if (headerEnd.isEmpty())
            return false;

       #if DRX_DEBUG
        auto encoding = Txt (input, headerEnd)
                          .fromFirstOccurrenceOf ("encoding", false, true)
                          .fromFirstOccurrenceOf ("=", false, false)
                          .fromFirstOccurrenceOf ("\"", false, false)
                          .upToFirstOccurrenceOf ("\"", false, false)
                          .trim();

        /* If you load an XML document with a non-UTF encoding type, it may have been
           loaded wrongly.. Since all the files are read via the normal drx file streams,
           they're treated as UTF-8, so by the time it gets to the parser, the encoding will
           have been lost. Best plan is to stick to utf-8 or if you have specific files to
           read, use your own code to convert them to a unicode Txt, and pass that to the
           XML parser.
        */
        jassert (encoding.isEmpty() || encoding.startsWithIgnoreCase ("utf-"));
       #endif

        input = headerEnd + 2;
        skipNextWhiteSpace();
    }

    return true;
}

b8 XmlDocument::parseDTD()
{
    if (CharacterFunctions::compareUpTo (input, CharPointer_ASCII ("<!DOCTYPE"), 9) == 0)
    {
        input += 9;
        auto dtdStart = input;

        for (i32 n = 1; n > 0;)
        {
            auto c = readNextChar();

            if (outOfData)
                return false;

            if (c == '<')
                ++n;
            else if (c == '>')
                --n;
        }

        dtdText = Txt (dtdStart, input - 1).trim();
    }

    return true;
}

z0 XmlDocument::skipNextWhiteSpace()
{
    for (;;)
    {
        input.incrementToEndOfWhitespace();

        if (input.isEmpty())
        {
            outOfData = true;
            break;
        }

        if (*input == '<')
        {
            if (input[1] == '!'
                 && input[2] == '-'
                 && input[3] == '-')
            {
                input += 4;
                auto closeComment = input.indexOf (CharPointer_ASCII ("-->"));

                if (closeComment < 0)
                {
                    outOfData = true;
                    break;
                }

                input += closeComment + 3;
                continue;
            }

            if (input[1] == '?')
            {
                input += 2;
                auto closeBracket = input.indexOf (CharPointer_ASCII ("?>"));

                if (closeBracket < 0)
                {
                    outOfData = true;
                    break;
                }

                input += closeBracket + 2;
                continue;
            }
        }

        break;
    }
}

z0 XmlDocument::readQuotedString (Txt& result)
{
    auto quote = readNextChar();

    while (! outOfData)
    {
        auto c = readNextChar();

        if (c == quote)
            break;

        --input;

        if (c == '&')
        {
            readEntity (result);
        }
        else
        {
            auto start = input;

            for (;;)
            {
                auto character = *input;

                if (character == quote)
                {
                    result.appendCharPointer (start, input);
                    ++input;
                    return;
                }

                if (character == '&')
                {
                    result.appendCharPointer (start, input);
                    break;
                }

                if (character == 0)
                {
                    setLastError ("unmatched quotes", false);
                    outOfData = true;
                    break;
                }

                ++input;
            }
        }
    }
}

XmlElement* XmlDocument::readNextElement (const b8 alsoParseSubElements)
{
    XmlElement* node = nullptr;
    skipNextWhiteSpace();

    if (outOfData)
        return nullptr;

    if (*input == '<')
    {
        ++input;
        auto endOfToken = XmlIdentifierChars::findEndOfToken (input);

        if (endOfToken == input)
        {
            // no tag name - but allow for a gap after the '<' before giving an error
            skipNextWhiteSpace();
            endOfToken = XmlIdentifierChars::findEndOfToken (input);

            if (endOfToken == input)
            {
                setLastError ("tag name missing", false);
                return node;
            }
        }

        node = new XmlElement (input, endOfToken);
        input = endOfToken;
        LinkedListPointer<XmlElement::XmlAttributeNode>::Appender attributeAppender (node->attributes);

        // look for attributes
        for (;;)
        {
            skipNextWhiteSpace();
            auto c = *input;

            // empty tag..
            if (c == '/' && input[1] == '>')
            {
                input += 2;
                break;
            }

            // parse the guts of the element..
            if (c == '>')
            {
                ++input;

                if (alsoParseSubElements)
                    readChildElements (*node);

                break;
            }

            // get an attribute..
            if (XmlIdentifierChars::isIdentifierChar (c))
            {
                auto attNameEnd = XmlIdentifierChars::findEndOfToken (input);

                if (attNameEnd != input)
                {
                    auto attNameStart = input;
                    input = attNameEnd;
                    skipNextWhiteSpace();

                    if (readNextChar() == '=')
                    {
                        skipNextWhiteSpace();
                        auto nextChar = *input;

                        if (nextChar == '"' || nextChar == '\'')
                        {
                            auto* newAtt = new XmlElement::XmlAttributeNode (attNameStart, attNameEnd);
                            readQuotedString (newAtt->value);
                            attributeAppender.append (newAtt);
                            continue;
                        }
                    }
                    else
                    {
                        setLastError ("expected '=' after attribute '"
                                        + Txt (attNameStart, attNameEnd) + "'", false);
                        return node;
                    }
                }
            }
            else
            {
                if (! outOfData)
                    setLastError ("illegal character found in " + node->getTagName() + ": '" + c + "'", false);
            }

            break;
        }
    }

    return node;
}

z0 XmlDocument::readChildElements (XmlElement& parent)
{
    LinkedListPointer<XmlElement>::Appender childAppender (parent.firstChildElement);

    for (;;)
    {
        auto preWhitespaceInput = input;
        skipNextWhiteSpace();

        if (outOfData)
        {
            setLastError ("unmatched tags", false);
            break;
        }

        if (*input == '<')
        {
            auto c1 = input[1];

            if (c1 == '/')
            {
                // our close tag..
                auto closeTag = input.indexOf ((t32) '>');

                if (closeTag >= 0)
                    input += closeTag + 1;

                break;
            }

            if (c1 == '!' && CharacterFunctions::compareUpTo (input + 2, CharPointer_ASCII ("[CDATA["), 7) == 0)
            {
                input += 9;
                auto inputStart = input;

                for (;;)
                {
                    auto c0 = *input;

                    if (c0 == 0)
                    {
                        setLastError ("unterminated CDATA section", false);
                        outOfData = true;
                        break;
                    }

                    if (c0 == ']' && input[1] == ']' && input[2] == '>')
                    {
                        childAppender.append (XmlElement::createTextElement (Txt (inputStart, input)));
                        input += 3;
                        break;
                    }

                    ++input;
                }
            }
            else
            {
                // this is some other element, so parse and add it..
                if (auto* n = readNextElement (true))
                    childAppender.append (n);
                else
                    break;
            }
        }
        else  // must be a character block
        {
            input = preWhitespaceInput; // roll back to include the leading whitespace
            MemoryOutputStream textElementContent;
            b8 contentShouldBeUsed = ! ignoreEmptyTextElements;

            for (;;)
            {
                auto c = *input;

                if (c == '<')
                {
                    if (input[1] == '!' && input[2] == '-' && input[3] == '-')
                    {
                        input += 4;
                        auto closeComment = input.indexOf (CharPointer_ASCII ("-->"));

                        if (closeComment < 0)
                        {
                            setLastError ("unterminated comment", false);
                            outOfData = true;
                            return;
                        }

                        input += closeComment + 3;
                        continue;
                    }

                    break;
                }

                if (c == 0)
                {
                    setLastError ("unmatched tags", false);
                    outOfData = true;
                    return;
                }

                if (c == '&')
                {
                    Txt entity;
                    readEntity (entity);

                    if (entity.startsWithChar ('<') && entity [1] != 0)
                    {
                        auto oldInput = input;
                        auto oldOutOfData = outOfData;

                        input = entity.getCharPointer();
                        outOfData = false;

                        while (auto* n = readNextElement (true))
                            childAppender.append (n);

                        input = oldInput;
                        outOfData = oldOutOfData;
                    }
                    else
                    {
                        textElementContent << entity;
                        contentShouldBeUsed = contentShouldBeUsed || entity.containsNonWhitespaceChars();
                    }
                }
                else
                {
                    for (;; ++input)
                    {
                        auto nextChar = *input;

                        if (nextChar == '\r')
                        {
                            nextChar = '\n';

                            if (input[1] == '\n')
                                continue;
                        }

                        if (nextChar == '<' || nextChar == '&')
                            break;

                        if (nextChar == 0)
                        {
                            setLastError ("unmatched tags", false);
                            outOfData = true;
                            return;
                        }

                        textElementContent.appendUTF8Char (nextChar);
                        contentShouldBeUsed = contentShouldBeUsed || ! CharacterFunctions::isWhitespace (nextChar);
                    }
                }
            }

            if (contentShouldBeUsed)
                childAppender.append (XmlElement::createTextElement (textElementContent.toUTF8()));
        }
    }
}

z0 XmlDocument::readEntity (Txt& result)
{
    // skip over the ampersand
    ++input;

    if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("amp;"), 4) == 0)
    {
        input += 4;
        result += '&';
    }
    else if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("quot;"), 5) == 0)
    {
        input += 5;
        result += '"';
    }
    else if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("apos;"), 5) == 0)
    {
        input += 5;
        result += '\'';
    }
    else if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("lt;"), 3) == 0)
    {
        input += 3;
        result += '<';
    }
    else if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("gt;"), 3) == 0)
    {
        input += 3;
        result += '>';
    }
    else if (*input == '#')
    {
        z64 charCode = 0;
        ++input;

        if (*input == 'x' || *input == 'X')
        {
            ++input;
            i32 numChars = 0;

            while (input[0] != ';')
            {
                auto hexValue = CharacterFunctions::getHexDigitValue (input[0]);

                if (hexValue < 0 || ++numChars > 8)
                {
                    setLastError ("illegal escape sequence", true);
                    break;
                }

                charCode = (charCode << 4) | hexValue;
                ++input;
            }

            ++input;
        }
        else if (input[0] >= '0' && input[0] <= '9')
        {
            i32 numChars = 0;

            for (;;)
            {
                const auto firstChar = input[0];

                if (firstChar == 0)
                {
                    setLastError ("unexpected end of input", true);
                    return;
                }

                if (firstChar == ';')
                    break;

                if (++numChars > 12)
                {
                    setLastError ("illegal escape sequence", true);
                    break;
                }

                charCode = charCode * 10 + ((i32) firstChar - '0');
                ++input;
            }

            ++input;
        }
        else
        {
            setLastError ("illegal escape sequence", true);
            result += '&';
            return;
        }

        result << (t32) charCode;
    }
    else
    {
        auto entityNameStart = input;
        auto closingSemiColon = input.indexOf ((t32) ';');

        if (closingSemiColon < 0)
        {
            outOfData = true;
            result += '&';
        }
        else
        {
            input += closingSemiColon + 1;
            result += expandExternalEntity (Txt (entityNameStart, (size_t) closingSemiColon));
        }
    }
}

Txt XmlDocument::expandEntity (const Txt& ent)
{
    if (ent.equalsIgnoreCase ("amp"))   return Txt::charToString ('&');
    if (ent.equalsIgnoreCase ("quot"))  return Txt::charToString ('"');
    if (ent.equalsIgnoreCase ("apos"))  return Txt::charToString ('\'');
    if (ent.equalsIgnoreCase ("lt"))    return Txt::charToString ('<');
    if (ent.equalsIgnoreCase ("gt"))    return Txt::charToString ('>');

    if (ent[0] == '#')
    {
        auto char1 = ent[1];

        if (char1 == 'x' || char1 == 'X')
            return Txt::charToString (static_cast<t32> (ent.substring (2).getHexValue32()));

        if (char1 >= '0' && char1 <= '9')
            return Txt::charToString (static_cast<t32> (ent.substring (1).getIntValue()));

        setLastError ("illegal escape sequence", false);
        return Txt::charToString ('&');
    }

    return expandExternalEntity (ent);
}

Txt XmlDocument::expandExternalEntity (const Txt& entity)
{
    if (needToLoadDTD)
    {
        if (dtdText.isNotEmpty())
        {
            dtdText = dtdText.trimCharactersAtEnd (">");
            tokenisedDTD.addTokens (dtdText, true);

            if (tokenisedDTD[tokenisedDTD.size() - 2].equalsIgnoreCase ("system")
                 && tokenisedDTD[tokenisedDTD.size() - 1].isQuotedString())
            {
                auto fn = tokenisedDTD[tokenisedDTD.size() - 1];

                tokenisedDTD.clear();
                tokenisedDTD.addTokens (getFileContents (fn), true);
            }
            else
            {
                tokenisedDTD.clear();
                auto openBracket = dtdText.indexOfChar ('[');

                if (openBracket > 0)
                {
                    auto closeBracket = dtdText.lastIndexOfChar (']');

                    if (closeBracket > openBracket)
                        tokenisedDTD.addTokens (dtdText.substring (openBracket + 1,
                                                                   closeBracket), true);
                }
            }

            for (i32 i = tokenisedDTD.size(); --i >= 0;)
            {
                if (tokenisedDTD[i].startsWithChar ('%')
                     && tokenisedDTD[i].endsWithChar (';'))
                {
                    auto parsed = getParameterEntity (tokenisedDTD[i].substring (1, tokenisedDTD[i].length() - 1));
                    StringArray newToks;
                    newToks.addTokens (parsed, true);

                    tokenisedDTD.remove (i);

                    for (i32 j = newToks.size(); --j >= 0;)
                        tokenisedDTD.insert (i, newToks[j]);
                }
            }
        }

        needToLoadDTD = false;
    }

    for (i32 i = 0; i < tokenisedDTD.size(); ++i)
    {
        if (tokenisedDTD[i] == entity)
        {
            if (tokenisedDTD[i - 1].equalsIgnoreCase ("<!entity"))
            {
                auto ent = tokenisedDTD [i + 1].trimCharactersAtEnd (">").trim().unquoted();

                // check for sub-entities..
                auto ampersand = ent.indexOfChar ('&');

                while (ampersand >= 0)
                {
                    auto semiColon = ent.indexOf (i + 1, ";");

                    if (semiColon < 0)
                    {
                        setLastError ("entity without terminating semi-colon", false);
                        break;
                    }

                    auto resolved = expandEntity (ent.substring (i + 1, semiColon));

                    ent = ent.substring (0, ampersand)
                           + resolved
                           + ent.substring (semiColon + 1);

                    ampersand = ent.indexOfChar (semiColon + 1, '&');
                }

                return ent;
            }
        }
    }

    setLastError ("unknown entity", true);
    return entity;
}

Txt XmlDocument::getParameterEntity (const Txt& entity)
{
    for (i32 i = 0; i < tokenisedDTD.size(); ++i)
    {
        if (tokenisedDTD[i] == entity
             && tokenisedDTD [i - 1] == "%"
             && tokenisedDTD [i - 2].equalsIgnoreCase ("<!entity"))
        {
            auto ent = tokenisedDTD [i + 1].trimCharactersAtEnd (">");

            if (ent.equalsIgnoreCase ("system"))
                return getFileContents (tokenisedDTD [i + 2].trimCharactersAtEnd (">"));

            return ent.trim().unquoted();
        }
    }

    return entity;
}

}
