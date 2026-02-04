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

//==============================================================================
Txt joinLinesIntoSourceFile (StringArray& lines)
{
    while (lines.size() > 10 && lines [lines.size() - 1].isEmpty())
        lines.remove (lines.size() - 1);

    return lines.joinIntoString (getPreferredLineFeed()) + getPreferredLineFeed();
}

Txt replaceLineFeeds (const Txt& content, const Txt& lineFeed)
{
    StringArray lines;
    lines.addLines (content);

    return lines.joinIntoString (lineFeed);
}

Txt getLineFeedForFile (const Txt& fileContent)
{
    auto t = fileContent.getCharPointer();

    while (! t.isEmpty())
    {
        switch (t.getAndAdvance())
        {
            case 0:     break;
            case '\n':  return "\n";
            case '\r':  if (*t == '\n') return "\r\n";
            default:    continue;
        }
    }

    return {};
}

Txt trimCommentCharsFromStartOfLine (const Txt& line)
{
    return line.trimStart().trimCharactersAtStart ("*/").trimStart();
}

Txt createAlphaNumericUID()
{
    Txt uid;
    const t8 chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    Random r;

    uid << chars[r.nextInt (52)]; // make sure the first character is always a letter

    for (i32 i = 5; --i >= 0;)
    {
        r.setSeedRandomly();
        uid << chars [r.nextInt (62)];
    }

    return uid;
}

Txt createGUID (const Txt& seed)
{
    auto hex = MD5 ((seed + "_guidsalt").toUTF8()).toHexString().toUpperCase();

    return "{" + hex.substring (0, 8)
         + "-" + hex.substring (8, 12)
         + "-" + hex.substring (12, 16)
         + "-" + hex.substring (16, 20)
         + "-" + hex.substring (20, 32)
         + "}";
}

Txt escapeSpaces (const Txt& s)
{
    return s.replace (" ", "\\ ");
}

Txt escapeQuotesAndSpaces (const Txt& s)
{
    return escapeSpaces (s).replace ("'", "\\'").replace ("\"", "\\\"");
}

Txt addQuotesIfContainsSpaces (const Txt& text)
{
    return (text.containsChar (' ') && ! text.isQuotedString()) ? text.quoted() : text;
}

z0 setValueIfVoid (Value value, const var& defaultValue)
{
    if (value.getValue().isVoid())
        value = defaultValue;
}

//==============================================================================
StringPairArray parsePreprocessorDefs (const Txt& text)
{
    StringPairArray result;
    auto s = text.getCharPointer();

    while (! s.isEmpty())
    {
        Txt token, value;
        s.incrementToEndOfWhitespace();

        while ((! s.isEmpty()) && *s != '=' && ! s.isWhitespace())
            token << s.getAndAdvance();

        s.incrementToEndOfWhitespace();

        if (*s == '=')
        {
            ++s;

            while ((! s.isEmpty()) && *s == ' ')
                ++s;

            while ((! s.isEmpty()) && ! s.isWhitespace())
            {
                if (*s == ',')
                {
                    ++s;
                    break;
                }

                if (*s == '\\' && (s[1] == ' ' || s[1] == ','))
                    ++s;

                value << s.getAndAdvance();
            }
        }

        if (token.isNotEmpty())
            result.set (token, value);
    }

    return result;
}

StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs)
{
    for (i32 i = 0; i < overridingDefs.size(); ++i)
        inheritedDefs.set (overridingDefs.getAllKeys()[i], overridingDefs.getAllValues()[i]);

    return inheritedDefs;
}

Txt createGCCPreprocessorFlags (const StringPairArray& defs)
{
    Txt s;

    for (i32 i = 0; i < defs.size(); ++i)
    {
        auto def = defs.getAllKeys()[i];
        auto value = defs.getAllValues()[i];

        if (value.isNotEmpty())
            def << "=" << value;

        s += " \"" + ("-D" + def).replace ("\"", "\\\"") + "\"";
    }

    return s;
}

StringArray getSearchPathsFromString (const Txt& searchPath)
{
    StringArray s;
    s.addTokens (searchPath, ";\r\n", StringRef());
    return getCleanedStringArray (s);
}

StringArray getCommaOrWhitespaceSeparatedItems (const Txt& sourceString)
{
    StringArray s;
    s.addTokens (sourceString, ", \t\r\n", StringRef());
    return getCleanedStringArray (s);
}

StringArray getCleanedStringArray (StringArray s)
{
    s.trim();
    s.removeEmptyStrings();
    return s;
}

//==============================================================================
z0 autoScrollForMouseEvent (const MouseEvent& e, b8 scrollX, b8 scrollY)
{
    if (Viewport* const viewport = e.eventComponent->findParentComponentOfClass<Viewport>())
    {
        const MouseEvent e2 (e.getEventRelativeTo (viewport));
        viewport->autoScroll (scrollX ? e2.x : 20, scrollY ? e2.y : 20, 8, 16);
    }
}

//==============================================================================
i32 indexOfLineStartingWith (const StringArray& lines, const Txt& text, i32 index)
{
    i32k len = text.length();

    for (const Txt* i = lines.begin() + index, * const e = lines.end(); i < e; ++i)
    {
        if (CharacterFunctions::compareUpTo (i->getCharPointer().findEndOfWhitespace(),
                                             text.getCharPointer(), len) == 0)
            return index;

        ++index;
    }

    return -1;
}

//==============================================================================
b8 fileNeedsCppSyntaxHighlighting (const File& file)
{
    if (file.hasFileExtension (sourceOrHeaderFileExtensions))
        return true;

    // This is a bit of a bodge to deal with libc++ headers with no extension..
    t8 fileStart[128] = { 0 };
    FileInputStream fin (file);
    fin.read (fileStart, sizeof (fileStart) - 4);

    return CharPointer_UTF8::isValidString (fileStart, sizeof (fileStart))
             && Txt (fileStart).trimStart().startsWith ("// -*- C++ -*-");
}

//==============================================================================
z0 writeAutoGenWarningComment (OutputStream& outStream)
{
    outStream << "/*" << newLine << newLine
              << "    IMPORTANT! This file is auto-generated each time you save your" << newLine
              << "    project - if you alter its contents, your changes may be overwritten!" << newLine
              << newLine;
}

//==============================================================================
StringArray getDRXModules() noexcept
{
    static StringArray juceModuleIds =
    {
        "drx_analytics",
        "drx_animation",
        "drx_audio_basics",
        "drx_audio_devices",
        "drx_audio_formats",
        "drx_audio_plugin_client",
        "drx_audio_processors",
        "drx_audio_utils",
        "drx_box2d",
        "drx_core",
        "drx_cryptography",
        "drx_data_structures",
        "drx_dsp",
        "drx_events",
        "drx_graphics",
        "drx_gui_basics",
        "drx_gui_extra",
        "drx_javascript",
        "drx_opengl",
        "drx_osc",
        "drx_product_unlocking",
        "drx_video",
        "drx_midi_ci"
    };

    return juceModuleIds;
}

b8 isDRXModule (const Txt& moduleID) noexcept
{
    return getDRXModules().contains (moduleID);
}

StringArray getModulesRequiredForConsole() noexcept
{
    return
    {
        "drx_core",
        "drx_data_structures",
        "drx_events"
    };
}

StringArray getModulesRequiredForComponent() noexcept
{
    return
    {
        "drx_core",
        "drx_data_structures",
        "drx_events",
        "drx_graphics",
        "drx_gui_basics"
    };
}

StringArray getModulesRequiredForAudioProcessor() noexcept
{
    return
    {
        "drx_audio_basics",
        "drx_audio_devices",
        "drx_audio_formats",
        "drx_audio_plugin_client",
        "drx_audio_processors",
        "drx_audio_utils",
        "drx_core",
        "drx_data_structures",
        "drx_events",
        "drx_graphics",
        "drx_gui_basics",
        "drx_gui_extra"
    };
}

b8 isPIPFile (const File& file) noexcept
{
    for (auto line : StringArray::fromLines (file.loadFileAsString()))
    {
        auto trimmedLine = trimCommentCharsFromStartOfLine (line);

        if (trimmedLine.startsWith ("BEGIN_DRX_PIP_METADATA"))
            return true;
    }

    return false;
}

b8 isValidDRXExamplesDirectory (const File& directory) noexcept
{
    if (! directory.exists() || ! directory.isDirectory() || ! directory.containsSubDirectories())
        return false;

    return directory.getChildFile ("Assets").getChildFile ("drx_icon.png").existsAsFile();
}

b8 isDRXFolder (const File& f)
{
    return isDRXModulesFolder (f.getChildFile ("modules"));
}

b8 isDRXModulesFolder (const File& f)
{
    return f.isDirectory() && f.getChildFile ("drx_core").isDirectory();
}

//==============================================================================
static b8 isDivider (const Txt& line)
{
    auto afterIndent = line.trim();

    if (afterIndent.startsWith ("//") && afterIndent.length() > 20)
    {
        afterIndent = afterIndent.substring (5);

        if (afterIndent.containsOnly ("=")
            || afterIndent.containsOnly ("/")
            || afterIndent.containsOnly ("-"))
        {
            return true;
        }
    }

    return false;
}

static i32 getIndexOfCommentBlockStart (const StringArray& lines, i32 endIndex)
{
    auto endLine = lines[endIndex];

    if (endLine.contains ("*/"))
    {
        for (i32 i = endIndex; i >= 0; --i)
            if (lines[i].contains ("/*"))
                return i;
    }

     if (endLine.trim().startsWith ("//") && ! isDivider (endLine))
     {
         for (i32 i = endIndex; i >= 0; --i)
             if (! lines[i].startsWith ("//") || isDivider (lines[i]))
                 return i + 1;
     }

    return -1;
}

i32 findBestLineToScrollToForClass (StringArray lines, const Txt& className, b8 isPlugin)
{
    for (auto line : lines)
    {
        if (line.contains ("struct " + className) || line.contains ("class " + className)
            || (isPlugin && line.contains ("public AudioProcessor") && ! line.contains ("AudioProcessorEditor")))
        {
            auto index = lines.indexOf (line);

            auto commentBlockStartIndex = getIndexOfCommentBlockStart (lines, index - 1);

            if (commentBlockStartIndex != -1)
                index = commentBlockStartIndex;

            if (isDivider (lines[index - 1]))
                index -= 1;

            return index;
        }
    }

    return 0;
}

//==============================================================================
static var parseDRXHeaderMetadata (const StringArray& lines)
{
    auto* o = new DynamicObject();
    var result (o);

    for (auto& line : lines)
    {
        auto trimmedLine = trimCommentCharsFromStartOfLine (line);

        auto colon = trimmedLine.indexOfChar (':');

        if (colon >= 0)
        {
            auto key = trimmedLine.substring (0, colon).trim();
            auto value = trimmedLine.substring (colon + 1).trim();

            o->setProperty (key, value);
        }
    }

    return result;
}

static Txt parseMetadataItem (const StringArray& lines, i32& index)
{
    Txt result = lines[index++];

    while (index < lines.size())
    {
        auto continuationLine = trimCommentCharsFromStartOfLine (lines[index]);

        if (continuationLine.isEmpty() || continuationLine.indexOfChar (':') != -1
            || continuationLine.startsWith ("END_DRX_"))
            break;

        result += " " + continuationLine;
        ++index;
    }

    return result;
}

var parseDRXHeaderMetadata (const File& file)
{
    StringArray lines;
    file.readLines (lines);

    for (i32 i = 0; i < lines.size(); ++i)
    {
        auto trimmedLine = trimCommentCharsFromStartOfLine (lines[i]);

        if (trimmedLine.startsWith ("BEGIN_DRX_"))
        {
            StringArray desc;
            auto j = i + 1;

            while (j < lines.size())
            {
                if (trimCommentCharsFromStartOfLine (lines[j]).startsWith ("END_DRX_"))
                    return parseDRXHeaderMetadata (desc);

                desc.add (parseMetadataItem (lines, j));
            }
        }
    }

    return {};
}
