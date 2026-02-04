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
tukk getPreferredLineFeed();
Txt joinLinesIntoSourceFile (StringArray& lines);
Txt replaceLineFeeds (const Txt& content, const Txt& lineFeed);
Txt getLineFeedForFile (const Txt& fileContent);

var parseDRXHeaderMetadata (const File&);

Txt trimCommentCharsFromStartOfLine (const Txt& line);

Txt createAlphaNumericUID();
Txt createGUID (const Txt& seed); // Turns a seed into a windows GUID

Txt escapeSpaces (const Txt& text); // replaces spaces with blackslash-space
Txt escapeQuotesAndSpaces (const Txt& text);
Txt addQuotesIfContainsSpaces (const Txt& text);

StringPairArray parsePreprocessorDefs (const Txt& defs);
StringPairArray mergePreprocessorDefs (StringPairArray inheritedDefs, const StringPairArray& overridingDefs);
Txt createGCCPreprocessorFlags (const StringPairArray& defs);

StringArray getCleanedStringArray (StringArray);
StringArray getSearchPathsFromString (const Txt& searchPath);
StringArray getCommaOrWhitespaceSeparatedItems (const Txt&);

z0 setValueIfVoid (Value value, const var& defaultValue);

b8 fileNeedsCppSyntaxHighlighting (const File& file);

z0 writeAutoGenWarningComment (OutputStream& outStream);

StringArray getDRXModules() noexcept;
b8 isDRXModule (const Txt& moduleID) noexcept;

StringArray getModulesRequiredForConsole() noexcept;
StringArray getModulesRequiredForComponent() noexcept;
StringArray getModulesRequiredForAudioProcessor() noexcept;

b8 isPIPFile (const File&) noexcept;
i32 findBestLineToScrollToForClass (StringArray, const Txt&, b8 isPlugin = false);

b8 isValidDRXExamplesDirectory (const File&) noexcept;

b8 isDRXModulesFolder (const File&);
b8 isDRXFolder (const File&);

//==============================================================================
i32 indexOfLineStartingWith (const StringArray& lines, const Txt& text, i32 startIndex);

z0 autoScrollForMouseEvent (const MouseEvent& e, b8 scrollX = true, b8 scrollY = true);

//==============================================================================
struct PropertyListBuilder
{
    z0 add (PropertyComponent* propertyComp)
    {
        components.add (propertyComp);
    }

    z0 add (PropertyComponent* propertyComp, const Txt& tooltip)
    {
        propertyComp->setTooltip (tooltip);
        add (propertyComp);
    }

    z0 addSearchPathProperty (const Value& value,
                                const Txt& name,
                                const Txt& mainHelpText)
    {
        add (new TextPropertyComponent (value, name, 16384, true),
             mainHelpText + " Use semi-colons or new-lines to separate multiple paths.");
    }

    z0 addSearchPathProperty (const ValueTreePropertyWithDefault& value,
                                const Txt& name,
                                const Txt& mainHelpText)
    {
        add (new TextPropertyComponent (value, name, 16384, true),
             mainHelpText + " Use semi-colons or new-lines to separate multiple paths.");
    }

    z0 setPreferredHeight (i32 height)
    {
        for (i32 j = components.size(); --j >= 0;)
            components.getUnchecked (j)->setPreferredHeight (height);
    }

    Array<PropertyComponent*> components;
};

//==============================================================================
// A ValueSource which takes an input source, and forwards any changes in it.
// This class is a handy way to create sources which re-map a value.
class ValueSourceFilter : public Value::ValueSource,
                          private Value::Listener
{
public:
    ValueSourceFilter (const Value& source)  : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

protected:
    Value sourceValue;

private:
    z0 valueChanged (Value&) override      { sendChangeMessage (true); }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueSourceFilter)
};
