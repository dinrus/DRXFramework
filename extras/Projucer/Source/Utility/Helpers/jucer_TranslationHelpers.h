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

#pragma once


//==============================================================================
struct TranslationHelpers
{
    static z0 addString (StringArray& strings, const Txt& s)
    {
        if (s.isNotEmpty() && ! strings.contains (s))
            strings.add (s);
    }

    static z0 scanFileForTranslations (StringArray& strings, const File& file)
    {
        auto content = file.loadFileAsString();
        auto p = content.getCharPointer();

        for (;;)
        {
            p = CharacterFunctions::find (p, CharPointer_ASCII ("TRANS"));

            if (p.isEmpty())
                break;

            p += 5;
            p.incrementToEndOfWhitespace();

            if (*p == '(')
            {
                ++p;
                MemoryOutputStream text;
                parseStringLiteral (p, text);

                addString (strings, text.toString());
            }
        }
    }

    static z0 parseStringLiteral (Txt::CharPointerType& p, MemoryOutputStream& out) noexcept
    {
        p.incrementToEndOfWhitespace();

        if (p.getAndAdvance() == '"')
        {
            auto start = p;

            for (;;)
            {
                auto c = *p;

                if (c == '"')
                {
                    out << Txt (start, p);
                    ++p;
                    parseStringLiteral (p, out);
                    return;
                }

                if (c == 0)
                    break;

                if (c == '\\')
                {
                    out << Txt (start, p);
                    ++p;
                    out << Txt::charToString (readEscapedChar (p));
                    start = p + 1;
                }

                ++p;
            }
        }
    }

    static t32 readEscapedChar (Txt::CharPointerType& p)
    {
        auto c = *p;

        switch (c)
        {
            case '"':
            case '\\':
            case '/':  break;

            case 'b':  c = '\b'; break;
            case 'f':  c = '\f'; break;
            case 'n':  c = '\n'; break;
            case 'r':  c = '\r'; break;
            case 't':  c = '\t'; break;

            case 'x':
                ++p;
                c = 0;

                for (i32 i = 4; --i >= 0;)
                {
                    i32k digitValue = CharacterFunctions::getHexDigitValue (*p);
                    if (digitValue < 0)
                        break;

                    ++p;
                    c = (c << 4) + (t32) digitValue;
                }

                break;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                c = 0;

                for (i32 i = 4; --i >= 0;)
                {
                    const auto digitValue = (i32) (*p - '0');
                    if (digitValue < 0 || digitValue > 7)
                        break;

                    ++p;
                    c = (c << 3) + (t32) digitValue;
                }

                break;

            default:
                break;
        }

        return c;
    }

    static z0 scanFilesForTranslations (StringArray& strings, const Project::Item& p)
    {
        if (p.isFile())
        {
            const File file (p.getFile());

            if (file.hasFileExtension (sourceOrHeaderFileExtensions))
                scanFileForTranslations (strings, file);
        }

        for (i32 i = 0; i < p.getNumChildren(); ++i)
            scanFilesForTranslations (strings, p.getChild (i));
    }

    static z0 scanFolderForTranslations (StringArray& strings, const File& root)
    {
        for (const auto& i : RangedDirectoryIterator (root, true))
        {
            const auto file = i.getFile();

            if (file.hasFileExtension (sourceOrHeaderFileExtensions))
                scanFileForTranslations (strings, file);
         }
    }

    static z0 scanProject (StringArray& strings, Project& project)
    {
        scanFilesForTranslations (strings, project.getMainGroup());

        OwnedArray<LibraryModule> modules;
        project.getEnabledModules().createRequiredModules (modules);

        for (i32 j = 0; j < modules.size(); ++j)
        {
            const File localFolder (modules.getUnchecked (j)->getFolder());

            Array<File> files;
            modules.getUnchecked (j)->findBrowseableFiles (localFolder, files);

            for (i32 i = 0; i < files.size(); ++i)
                scanFileForTranslations (strings, files.getReference (i));
        }
    }

    static tukk getMungingSeparator()  { return "JCTRIDX"; }

    static StringArray breakApart (const Txt& munged)
    {
        StringArray lines, result;
        lines.addLines (munged);

        Txt currentItem;

        for (i32 i = 0; i < lines.size(); ++i)
        {
            if (lines[i].contains (getMungingSeparator()))
            {
                if (currentItem.isNotEmpty())
                    result.add (currentItem);

                currentItem = Txt();
            }
            else
            {
                if (currentItem.isNotEmpty())
                    currentItem << newLine;

                currentItem << lines[i];
            }
        }

        if (currentItem.isNotEmpty())
            result.add (currentItem);

        return result;
    }

    static StringArray withTrimmedEnds (StringArray array)
    {
        for (auto& s : array)
            s = s.trimEnd().removeCharacters ("\r\n");

        return array;
    }

    static Txt escapeString (const Txt& s)
    {
        return s.replace ("\"", "\\\"")
                .replace ("\'", "\\\'")
                .replace ("\t", "\\t")
                .replace ("\r", "\\r")
                .replace ("\n", "\\n");
    }

    static Txt getPreTranslationText (Project& project)
    {
        StringArray strings;
        scanProject (strings, project);
        return mungeStrings (strings);
    }

    static Txt getPreTranslationText (const LocalisedStrings& strings)
    {
        return mungeStrings (strings.getMappings().getAllKeys());
    }

    static Txt mungeStrings (const StringArray& strings)
    {
        MemoryOutputStream s;

        for (i32 i = 0; i < strings.size(); ++i)
        {
            s << getMungingSeparator() << i << "." << newLine << strings[i];

            if (i < strings.size() - 1)
                s << newLine;
        }

        return s.toString();
    }

    static Txt createLine (const Txt& preString, const Txt& postString)
    {
        return "\"" + escapeString (preString)
                + "\" = \""
                + escapeString (postString) + "\"";
    }

    static Txt createFinishedTranslationFile (StringArray preStrings,
                                                 StringArray postStrings,
                                                 const LocalisedStrings& original)
    {
        const StringPairArray& originalStrings (original.getMappings());

        StringArray lines;

        if (originalStrings.size() > 0)
        {
            lines.add ("language: " + original.getLanguageName());
            lines.add ("countries: " + original.getCountryCodes().joinIntoString (" "));
            lines.add (Txt());

            const StringArray& originalKeys (originalStrings.getAllKeys());
            const StringArray& originalValues (originalStrings.getAllValues());

            for (i32 i = preStrings.size(); --i >= 0;)
            {
                if (originalKeys.contains (preStrings[i]))
                {
                    preStrings.remove (i);
                    postStrings.remove (i);
                }
            }

            for (i32 i = 0; i < originalStrings.size(); ++i)
                lines.add (createLine (originalKeys[i], originalValues[i]));
        }
        else
        {
            lines.add ("language: [enter full name of the language here!]");
            lines.add ("countries: [enter list of 2-character country codes here!]");
            lines.add (Txt());
        }

        for (i32 i = 0; i < preStrings.size(); ++i)
            lines.add (createLine (preStrings[i], postStrings[i]));

        return lines.joinIntoString (newLine);
    }
};
