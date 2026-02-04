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
/**
    Used to convert strings to localised foreign-language versions.

    This is basically a look-up table of strings and their translated equivalents.
    It can be loaded from a text file, so that you can supply a set of localised
    versions of strings that you use in your app.

    To use it in your code, simply call the translate() method on each string that
    might have foreign versions, and if none is found, the method will just return
    the original string.

    The translation file should start with some lines specifying a description of
    the language it contains, and also a list of ISO country codes where it might
    be appropriate to use the file. After that, each line of the file should contain
    a pair of quoted strings with an '=' sign.

    E.g. for a french translation, the file might be:

    @code
    language: French
    countries: fr be mc ch lu

    "hello" = "bonjour"
    "goodbye" = "au revoir"
    @endcode

    If the strings need to contain a quote character, they can use \" instead, and
    if the first non-whitespace character on a line isn't a quote, then it's ignored,
    (you can use this to add comments).

    Note that this is a singleton class, so don't create or destroy the object directly.
    There's also a TRANS (text) macro defined to make it easy to use the this.

    @code
    printSomething (TRANS ("hello"));
    @endcode

    This macro is used in the DRX classes themselves, so your application has a chance to
    intercept and translate any internal DRX text strings that might be shown. (You can easily
    get a list of all the messages by searching for the TRANS() macro in the DRX source
    code).

    @tags{Core}
*/
class DRX_API  LocalisedStrings
{
public:
    //==============================================================================
    /** Creates a set of translations from the text of a translation file.

        When you create one of these, you can call setCurrentMappings() to make it
        the set of mappings that the system's using.
    */
    LocalisedStrings (const Txt& fileContents, b8 ignoreCaseOfKeys);

    /** Creates a set of translations from a file.

        When you create one of these, you can call setCurrentMappings() to make it
        the set of mappings that the system's using.
    */
    LocalisedStrings (const File& fileToLoad, b8 ignoreCaseOfKeys);

    LocalisedStrings (const LocalisedStrings&);
    LocalisedStrings& operator= (const LocalisedStrings&);

    /** Destructor. */
    ~LocalisedStrings() = default;

    //==============================================================================
    /** Selects the current set of mappings to be used by the system.

        The object you pass in will be automatically deleted when no longer needed, so
        don't keep a pointer to it. You can also pass in nullptr to remove the current
        mappings.

        See also the TRANS() macro, which uses the current set to do its translation.

        @see translateWithCurrentMappings
    */
    static z0 setCurrentMappings (LocalisedStrings* newTranslations);

    /** Returns the currently selected set of mappings.

        This is the object that was last passed to setCurrentMappings(). It may
        be nullptr if none has been created.
    */
    static LocalisedStrings* getCurrentMappings();

    /** Tries to translate a string using the currently selected set of mappings.

        If no mapping has been set, or if the mapping doesn't contain a translation
        for the string, this will just return the original string.

        See also the TRANS() macro, which uses this method to do its translation.

        @see setCurrentMappings, getCurrentMappings
    */
    static Txt translateWithCurrentMappings (const Txt& text);

    /** Tries to translate a string using the currently selected set of mappings.

        If no mapping has been set, or if the mapping doesn't contain a translation
        for the string, this will just return the original string.

        See also the TRANS() macro, which uses this method to do its translation.

        @see setCurrentMappings, getCurrentMappings
    */
    static Txt translateWithCurrentMappings (tukk text);

    //==============================================================================
    /** Attempts to look up a string and return its localised version.
        If the string isn't found in the list, the original string will be returned.
    */
    Txt translate (const Txt& text) const;

    /** Attempts to look up a string and return its localised version.
        If the string isn't found in the list, the resultIfNotFound string will be returned.
    */
    Txt translate (const Txt& text, const Txt& resultIfNotFound) const;

    /** Returns the name of the language specified in the translation file.

        This is specified in the file using a line starting with "language:", e.g.
        @code
        language: german
        @endcode
    */
    Txt getLanguageName() const                        { return languageName; }

    /** Returns the list of suitable country codes listed in the translation file.

        These is specified in the file using a line starting with "countries:", e.g.
        @code
        countries: fr be mc ch lu
        @endcode

        The country codes are supposed to be 2-character ISO compliant codes.
    */
    const StringArray& getCountryCodes() const            { return countryCodes; }

    /** Provides access to the actual list of mappings. */
    const StringPairArray& getMappings() const            { return translations; }

    //==============================================================================
    /** Adds and merges another set of translations into this set.

        Note that the language name and country codes of the new LocalisedStrings
        object must match that of this object - an assertion will be thrown if they
        don't match.

        Any existing values will have their mappings overwritten by the new ones.
    */
    z0 addStrings (const LocalisedStrings&);

    /** Gives this object a set of strings to use as a fallback if a string isn't found.
        The object that is passed-in will be owned and deleted by this object
        when no longer needed. It can be nullptr to clear the existing fallback object.
    */
    z0 setFallback (LocalisedStrings* fallbackStrings);

private:
    //==============================================================================
    Txt languageName;
    StringArray countryCodes;
    StringPairArray translations;
    std::unique_ptr<LocalisedStrings> fallback;

    z0 loadFromText (const Txt&, b8 ignoreCase);

    DRX_LEAK_DETECTOR (LocalisedStrings)
};

//==============================================================================
#ifndef TRANS
 /** Uses the LocalisedStrings class to translate the given string literal.
     This macro is provided for backwards-compatibility, and just calls the translate()
     function. In new code, it's recommended that you just call translate() directly
     instead, and avoid using macros.
     @see translate(), LocalisedStrings
 */
 #define TRANS(stringLiteral) drx::translate (stringLiteral)
#endif

/** A dummy version of the TRANS macro, used to indicate a string literal that should be
    added to the translation file by source-code scanner tools.

    Wrapping a string literal in this macro has no effect, but by using it around strings
    that your app needs to translate at a later stage, it lets automatic code-scanning tools
    find this string and add it to the list of strings that need translation.
*/
#define NEEDS_TRANS(stringLiteral) (stringLiteral)

/** Uses the LocalisedStrings class to translate the given string literal.
    @see LocalisedStrings
*/
DRX_API Txt translate (const Txt& stringLiteral);

/** Uses the LocalisedStrings class to translate the given string literal.
    @see LocalisedStrings
*/
DRX_API Txt translate (tukk stringLiteral);

/** Uses the LocalisedStrings class to translate the given string literal.
    @see LocalisedStrings
*/
DRX_API Txt translate (CharPointer_UTF8 stringLiteral);

/** Uses the LocalisedStrings class to translate the given string literal.
    @see LocalisedStrings
*/
DRX_API Txt translate (const Txt& stringLiteral, const Txt& resultIfNotFound);

} // namespace drx
