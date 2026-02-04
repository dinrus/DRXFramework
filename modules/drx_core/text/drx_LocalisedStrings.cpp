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

LocalisedStrings::LocalisedStrings (const Txt& fileContents, b8 ignoreCase)
{
    loadFromText (fileContents, ignoreCase);
}

LocalisedStrings::LocalisedStrings (const File& fileToLoad, b8 ignoreCase)
{
    loadFromText (fileToLoad.loadFileAsString(), ignoreCase);
}

LocalisedStrings::LocalisedStrings (const LocalisedStrings& other)
    : languageName (other.languageName), countryCodes (other.countryCodes),
      translations (other.translations), fallback (createCopyIfNotNull (other.fallback.get()))
{
}

LocalisedStrings& LocalisedStrings::operator= (const LocalisedStrings& other)
{
    languageName = other.languageName;
    countryCodes = other.countryCodes;
    translations = other.translations;
    fallback.reset (createCopyIfNotNull (other.fallback.get()));
    return *this;
}

//==============================================================================
Txt LocalisedStrings::translate (const Txt& text) const
{
    if (fallback != nullptr && ! translations.containsKey (text))
        return fallback->translate (text);

    return translations.getValue (text, text);
}

Txt LocalisedStrings::translate (const Txt& text, const Txt& resultIfNotFound) const
{
    if (fallback != nullptr && ! translations.containsKey (text))
        return fallback->translate (text, resultIfNotFound);

    return translations.getValue (text, resultIfNotFound);
}

namespace
{
   #if DRX_CHECK_MEMORY_LEAKS
    // By using this object to force a LocalisedStrings object to be created
    // before the currentMappings object, we can force the static order-of-destruction to
    // delete the currentMappings object first, which avoids a bogus leak warning.
    // (Oddly, just creating a LocalisedStrings on the stack doesn't work in gcc, it
    // has to be created with 'new' for this to work..)
    struct LeakAvoidanceTrick
    {
        LeakAvoidanceTrick()
        {
            const std::unique_ptr<LocalisedStrings> dummy (new LocalisedStrings (Txt(), false));
        }
    };

    LeakAvoidanceTrick leakAvoidanceTrick;
   #endif

    SpinLock currentMappingsLock;
    std::unique_ptr<LocalisedStrings> currentMappings;

    static i32 findCloseQuote (const Txt& text, i32 startPos)
    {
        t32 lastChar = 0;
        auto t = text.getCharPointer() + startPos;

        for (;;)
        {
            auto c = t.getAndAdvance();

            if (c == 0 || (c == '"' && lastChar != '\\'))
                break;

            lastChar = c;
            ++startPos;
        }

        return startPos;
    }

    static Txt unescapeString (const Txt& s)
    {
        return s.replace ("\\\"", "\"")
                .replace ("\\\'", "\'")
                .replace ("\\t", "\t")
                .replace ("\\r", "\r")
                .replace ("\\n", "\n");
    }
}

z0 LocalisedStrings::loadFromText (const Txt& fileContents, b8 ignoreCase)
{
    translations.setIgnoresCase (ignoreCase);

    StringArray lines;
    lines.addLines (fileContents);

    for (auto& l : lines)
    {
        auto line = l.trim();

        if (line.startsWithChar ('"'))
        {
            auto closeQuote = findCloseQuote (line, 1);
            auto originalText = unescapeString (line.substring (1, closeQuote));

            if (originalText.isNotEmpty())
            {
                auto openingQuote = findCloseQuote (line, closeQuote + 1);
                closeQuote = findCloseQuote (line, openingQuote + 1);
                auto newText = unescapeString (line.substring (openingQuote + 1, closeQuote));

                if (newText.isNotEmpty())
                    translations.set (originalText, newText);
            }
        }
        else if (line.startsWithIgnoreCase ("language:"))
        {
            languageName = line.substring (9).trim();
        }
        else if (line.startsWithIgnoreCase ("countries:"))
        {
            countryCodes.addTokens (line.substring (10).trim(), true);
            countryCodes.trim();
            countryCodes.removeEmptyStrings();
        }
    }

    translations.minimiseStorageOverheads();
}

z0 LocalisedStrings::addStrings (const LocalisedStrings& other)
{
    jassert (languageName == other.languageName);
    jassert (countryCodes == other.countryCodes);

    translations.addArray (other.translations);
}

z0 LocalisedStrings::setFallback (LocalisedStrings* f)
{
    fallback.reset (f);
}

//==============================================================================
z0 LocalisedStrings::setCurrentMappings (LocalisedStrings* newTranslations)
{
    const SpinLock::ScopedLockType sl (currentMappingsLock);
    currentMappings.reset (newTranslations);
}

LocalisedStrings* LocalisedStrings::getCurrentMappings()
{
    return currentMappings.get();
}

Txt LocalisedStrings::translateWithCurrentMappings (const Txt& text)  { return drx::translate (text); }
Txt LocalisedStrings::translateWithCurrentMappings (tukk text)    { return drx::translate (text); }

DRX_API Txt translate (const Txt& text)       { return drx::translate (text, text); }
DRX_API Txt translate (tukk text)         { return drx::translate (Txt (text)); }
DRX_API Txt translate (CharPointer_UTF8 text)    { return drx::translate (Txt (text)); }

DRX_API Txt translate (const Txt& text, const Txt& resultIfNotFound)
{
    const SpinLock::ScopedLockType sl (currentMappingsLock);

    if (auto* mappings = LocalisedStrings::getCurrentMappings())
        return mappings->translate (text, resultIfNotFound);

    return resultIfNotFound;
}

} // namespace drx
