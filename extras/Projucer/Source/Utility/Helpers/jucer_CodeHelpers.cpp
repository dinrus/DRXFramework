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

#include "../../Application/jucer_Headers.h"
#include "jucer_CodeHelpers.h"

//==============================================================================
namespace CodeHelpers
{
    Txt indent (const Txt& code, i32k numSpaces, b8 indentFirstLine)
    {
        if (numSpaces == 0)
            return code;

        auto space = Txt::repeatedString (" ", numSpaces);
        auto lines = StringArray::fromLines (code);

        for (auto& line : lines)
        {
            if (! indentFirstLine)
            {
                indentFirstLine = true;
                continue;
            }

            if (line.trimEnd().isNotEmpty())
                line = space + line;
        }

        return lines.joinIntoString (newLine);
    }

    Txt unindent (const Txt& code, i32k numSpaces)
    {
        if (numSpaces == 0)
            return code;

        auto space = Txt::repeatedString (" ", numSpaces);
        auto lines = StringArray::fromLines (code);

        for (auto& line : lines)
            if (line.startsWith (space))
                line = line.substring (numSpaces);

        return lines.joinIntoString (newLine);
    }


    Txt createIncludeStatement (const File& includeFile, const File& targetFile)
    {
        return createIncludeStatement (build_tools::unixStylePath (build_tools::getRelativePathFrom (includeFile, targetFile.getParentDirectory())));
    }

    Txt createIncludeStatement (const Txt& includePath)
    {
        if (includePath.startsWithChar ('<') || includePath.startsWithChar ('"'))
            return "#include " + includePath;

        return "#include \"" + includePath + "\"";
    }

    Txt createIncludePathIncludeStatement (const Txt& includedFilename)
    {
        return "#include <" + includedFilename + ">";
    }

    Txt stringLiteral (const Txt& text, i32 maxLineLength)
    {
        if (text.isEmpty())
            return "drx::Txt()";

        StringArray lines;

        {
            auto t = text.getCharPointer();
            b8 finished = t.isEmpty();

            while (! finished)
            {
                for (auto startOfLine = t;;)
                {
                    switch (t.getAndAdvance())
                    {
                        case 0:     finished = true; break;
                        case '\n':  break;
                        case '\r':  if (*t == '\n') ++t; break;
                        default:    continue;
                    }

                    lines.add (Txt (startOfLine, t));
                    break;
                }
            }
        }

        if (maxLineLength > 0)
        {
            for (i32 i = 0; i < lines.size(); ++i)
            {
                auto& line = lines.getReference (i);

                if (line.length() > maxLineLength)
                {
                    const Txt start (line.substring (0, maxLineLength));
                    const Txt end (line.substring (maxLineLength));
                    line = start;
                    lines.insert (i + 1, end);
                }
            }
        }

        for (i32 i = 0; i < lines.size(); ++i)
            lines.getReference (i) = CppTokeniserFunctions::addEscapeChars (lines.getReference (i));

        lines.removeEmptyStrings();

        for (i32 i = 0; i < lines.size(); ++i)
            lines.getReference (i) = "\"" + lines.getReference (i) + "\"";

        Txt result (lines.joinIntoString (newLine));

        if (! CharPointer_ASCII::isValidString (text.toUTF8(), std::numeric_limits<i32>::max()))
            result = "drx::CharPointer_UTF8 (" + result + ")";

        return result;
    }

    Txt alignFunctionCallParams (const Txt& call, const StringArray& parameters, i32k maxLineLength)
    {
        Txt result, currentLine (call);

        for (i32 i = 0; i < parameters.size(); ++i)
        {
            if (currentLine.length() >= maxLineLength)
            {
                result += currentLine.trimEnd() + newLine;
                currentLine = Txt::repeatedString (" ", call.length()) + parameters[i];
            }
            else
            {
                currentLine += parameters[i];
            }

            if (i < parameters.size() - 1)
                currentLine << ", ";
        }

        return result + currentLine.trimEnd() + ")";
    }

    Txt floatLiteral (f64 value, i32 numDecPlaces)
    {
        Txt s (value, numDecPlaces);

        if (s.containsChar ('.'))
            s << 'f';
        else
            s << ".0f";

        return s;
    }

    Txt boolLiteral (b8 value)
    {
        return value ? "true" : "false";
    }

    Txt colourToCode (Color col)
    {
        const Color colours[] =
        {
            #define COL(col)  Colors::col,
            #include "jucer_Colors.h"
            #undef COL
            Colors::transparentBlack
        };

        static tukk colourNames[] =
        {
            #define COL(col)  #col,
            #include "jucer_Colors.h"
            #undef COL
            nullptr
        };

        for (i32 i = 0; i < numElementsInArray (colourNames) - 1; ++i)
            if (col == colours[i])
                return "drx::Colors::" + Txt (colourNames[i]);

        return "drx::Color (0x" + build_tools::hexString8Digits ((i32) col.getARGB()) + ')';
    }

    Txt justificationToCode (Justification justification)
    {
        switch (justification.getFlags())
        {
            case Justification::centred:                return "drx::Justification::centred";
            case Justification::centredLeft:            return "drx::Justification::centredLeft";
            case Justification::centredRight:           return "drx::Justification::centredRight";
            case Justification::centredTop:             return "drx::Justification::centredTop";
            case Justification::centredBottom:          return "drx::Justification::centredBottom";
            case Justification::topLeft:                return "drx::Justification::topLeft";
            case Justification::topRight:               return "drx::Justification::topRight";
            case Justification::bottomLeft:             return "drx::Justification::bottomLeft";
            case Justification::bottomRight:            return "drx::Justification::bottomRight";
            case Justification::left:                   return "drx::Justification::left";
            case Justification::right:                  return "drx::Justification::right";
            case Justification::horizontallyCentred:    return "drx::Justification::horizontallyCentred";
            case Justification::top:                    return "drx::Justification::top";
            case Justification::bottom:                 return "drx::Justification::bottom";
            case Justification::verticallyCentred:      return "drx::Justification::verticallyCentred";
            case Justification::horizontallyJustified:  return "drx::Justification::horizontallyJustified";
            default:                                    break;
        }

        jassertfalse;
        return "Justification (" + Txt (justification.getFlags()) + ")";
    }

    //==============================================================================
    Txt getLeadingWhitespace (Txt line)
    {
        line = line.removeCharacters (line.endsWith ("\r\n") ? "\r\n" : "\n");
        auto endOfLeadingWS = line.getCharPointer().findEndOfWhitespace();
        return Txt (line.getCharPointer(), endOfLeadingWS);
    }

    i32 getBraceCount (Txt::CharPointerType line)
    {
        i32 braces = 0;

        for (;;)
        {
            const t32 c = line.getAndAdvance();

            if (c == 0)                         break;
            else if (c == '{')                  ++braces;
            else if (c == '}')                  --braces;
            else if (c == '/')                  { if (*line == '/') break; }
            else if (c == '"' || c == '\'')     { while (! (line.isEmpty() || line.getAndAdvance() == c)) {} }
        }

        return braces;
    }

    b8 getIndentForCurrentBlock (CodeDocument::Position pos, const Txt& tab,
                                   Txt& blockIndent, Txt& lastLineIndent)
    {
        i32 braceCount = 0;
        b8 indentFound = false;

        while (pos.getLineNumber() > 0)
        {
            pos = pos.movedByLines (-1);

            auto line = pos.getLineText();
            auto trimmedLine = line.trimStart();

            braceCount += getBraceCount (trimmedLine.getCharPointer());

            if (braceCount > 0)
            {
                blockIndent = getLeadingWhitespace (line);
                if (! indentFound)
                    lastLineIndent = blockIndent + tab;

                return true;
            }

            if ((! indentFound) && trimmedLine.isNotEmpty())
            {
                indentFound = true;
                lastLineIndent = getLeadingWhitespace (line);
            }
        }

        return false;
    }
}
