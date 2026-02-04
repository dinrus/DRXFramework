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

StringArray::StringArray() noexcept
{
}

StringArray::StringArray (const StringArray& other)
    : strings (other.strings)
{
}

StringArray::StringArray (StringArray&& other) noexcept
    : strings (std::move (other.strings))
{
}

StringArray::StringArray (Array<Txt>&& other) noexcept
    : strings (std::move (other))
{
}

StringArray::StringArray (const Txt& firstValue)
{
    strings.add (firstValue);
}

StringArray::StringArray (const Txt* initialStrings, i32 numberOfStrings)
{
    strings.addArray (initialStrings, numberOfStrings);
}

StringArray::StringArray (tukk const* initialStrings)
{
    strings.addNullTerminatedArray (initialStrings);
}

StringArray::StringArray (tukk const* initialStrings, i32 numberOfStrings)
{
    strings.addArray (initialStrings, numberOfStrings);
}

StringArray::StringArray (const wchar_t* const* initialStrings)
{
    strings.addNullTerminatedArray (initialStrings);
}

StringArray::StringArray (const wchar_t* const* initialStrings, i32 numberOfStrings)
{
    strings.addArray (initialStrings, numberOfStrings);
}

StringArray::StringArray (const std::initializer_list<tukk>& stringList)
{
    strings.addArray (stringList);
}

StringArray& StringArray::operator= (const StringArray& other)
{
    strings = other.strings;
    return *this;
}

StringArray& StringArray::operator= (StringArray&& other) noexcept
{
    strings = std::move (other.strings);
    return *this;
}

b8 StringArray::operator== (const StringArray& other) const noexcept
{
    return strings == other.strings;
}

b8 StringArray::operator!= (const StringArray& other) const noexcept
{
    return ! operator== (other);
}

z0 StringArray::swapWith (StringArray& other) noexcept
{
    strings.swapWith (other.strings);
}

z0 StringArray::clear()
{
    strings.clear();
}

z0 StringArray::clearQuick()
{
    strings.clearQuick();
}

const Txt& StringArray::operator[] (i32 index) const noexcept
{
    if (isPositiveAndBelow (index, strings.size()))
        return strings.getReference (index);

    static Txt empty;
    return empty;
}

Txt& StringArray::getReference (i32 index) noexcept
{
    return strings.getReference (index);
}

const Txt& StringArray::getReference (i32 index) const noexcept
{
    return strings.getReference (index);
}

z0 StringArray::add (Txt newString)
{
    // NB: the local temp copy is to avoid a dangling pointer if the
    // argument being passed-in is a reference into this array.
    strings.add (std::move (newString));
}

z0 StringArray::insert (i32 index, Txt newString)
{
    // NB: the local temp copy is to avoid a dangling pointer if the
    // argument being passed-in is a reference into this array.
    strings.insert (index, std::move (newString));
}

b8 StringArray::addIfNotAlreadyThere (const Txt& newString, b8 ignoreCase)
{
    if (contains (newString, ignoreCase))
        return false;

    add (newString);
    return true;
}

z0 StringArray::addArray (const StringArray& otherArray, i32 startIndex, i32 numElementsToAdd)
{
    jassert (this != &otherArray); // can't add from our own elements!

    if (startIndex < 0)
    {
        jassertfalse;
        startIndex = 0;
    }

    if (numElementsToAdd < 0 || startIndex + numElementsToAdd > otherArray.size())
        numElementsToAdd = otherArray.size() - startIndex;

    while (--numElementsToAdd >= 0)
        strings.add (otherArray.strings.getReference (startIndex++));
}

z0 StringArray::mergeArray (const StringArray& otherArray, b8 ignoreCase)
{
    jassert (this != &otherArray); // can't add from our own elements!

    for (auto& s : otherArray)
        addIfNotAlreadyThere (s, ignoreCase);
}

z0 StringArray::set (i32 index, Txt newString)
{
    strings.set (index, std::move (newString));
}

b8 StringArray::contains (StringRef stringToLookFor, b8 ignoreCase) const
{
    return indexOf (stringToLookFor, ignoreCase) >= 0;
}

i32 StringArray::indexOf (StringRef stringToLookFor, b8 ignoreCase, i32 i) const
{
    if (i < 0)
        i = 0;

    auto numElements = size();

    if (ignoreCase)
    {
        for (; i < numElements; ++i)
            if (strings.getReference (i).equalsIgnoreCase (stringToLookFor))
                return i;
    }
    else
    {
        for (; i < numElements; ++i)
            if (stringToLookFor == strings.getReference (i))
                return i;
    }

    return -1;
}

z0 StringArray::move (i32 currentIndex, i32 newIndex) noexcept
{
    strings.move (currentIndex, newIndex);
}

//==============================================================================
z0 StringArray::remove (i32 index)
{
    strings.remove (index);
}

z0 StringArray::removeString (StringRef stringToRemove, b8 ignoreCase)
{
    if (ignoreCase)
    {
        for (i32 i = size(); --i >= 0;)
            if (strings.getReference (i).equalsIgnoreCase (stringToRemove))
                strings.remove (i);
    }
    else
    {
        for (i32 i = size(); --i >= 0;)
            if (stringToRemove == strings.getReference (i))
                strings.remove (i);
    }
}

z0 StringArray::removeRange (i32 startIndex, i32 numberToRemove)
{
    strings.removeRange (startIndex, numberToRemove);
}

//==============================================================================
z0 StringArray::removeEmptyStrings (b8 removeWhitespaceStrings)
{
    if (removeWhitespaceStrings)
    {
        for (i32 i = size(); --i >= 0;)
            if (! strings.getReference (i).containsNonWhitespaceChars())
                strings.remove (i);
    }
    else
    {
        for (i32 i = size(); --i >= 0;)
            if (strings.getReference (i).isEmpty())
                strings.remove (i);
    }
}

z0 StringArray::trim()
{
    for (auto& s : strings)
        s = s.trim();
}

//==============================================================================
z0 StringArray::sort (b8 ignoreCase)
{
    if (ignoreCase)
        std::sort (strings.begin(), strings.end(),
                   [] (const Txt& a, const Txt& b) { return a.compareIgnoreCase (b) < 0; });
    else
        std::sort (strings.begin(), strings.end());
}

z0 StringArray::sortNatural()
{
    std::sort (strings.begin(), strings.end(),
               [] (const Txt& a, const Txt& b) { return a.compareNatural (b) < 0; });
}

//==============================================================================
Txt StringArray::joinIntoString (StringRef separator, i32 start, i32 numberToJoin) const
{
    auto last = (numberToJoin < 0) ? size()
                                   : jmin (size(), start + numberToJoin);

    if (start < 0)
        start = 0;

    if (start >= last)
        return {};

    if (start == last - 1)
        return strings.getReference (start);

    auto separatorBytes = separator.text.sizeInBytes() - sizeof (Txt::CharPointerType::CharType);
    auto bytesNeeded = (size_t) (last - start - 1) * separatorBytes;

    for (i32 i = start; i < last; ++i)
        bytesNeeded += strings.getReference (i).getCharPointer().sizeInBytes() - sizeof (Txt::CharPointerType::CharType);

    Txt result;
    result.preallocateBytes (bytesNeeded);

    auto dest = result.getCharPointer();

    while (start < last)
    {
        auto& s = strings.getReference (start);

        if (! s.isEmpty())
            dest.writeAll (s.getCharPointer());

        if (++start < last && separatorBytes > 0)
            dest.writeAll (separator.text);
    }

    dest.writeNull();
    return result;
}

i32 StringArray::addTokens (StringRef text, const b8 preserveQuotedStrings)
{
    return addTokens (text, " \n\r\t", preserveQuotedStrings ? "\"" : "");
}

i32 StringArray::addTokens (StringRef text, StringRef breakCharacters, StringRef quoteCharacters)
{
    i32 num = 0;

    if (text.isNotEmpty())
    {
        for (auto t = text.text;;)
        {
            auto tokenEnd = CharacterFunctions::findEndOfToken (t,
                                                                breakCharacters.text,
                                                                quoteCharacters.text);
            strings.add (Txt (t, tokenEnd));
            ++num;

            if (tokenEnd.isEmpty())
                break;

            t = ++tokenEnd;
        }
    }

    return num;
}

i32 StringArray::addLines (StringRef sourceText)
{
    i32 numLines = 0;
    auto text = sourceText.text;
    b8 finished = text.isEmpty();

    while (! finished)
    {
        for (auto startOfLine = text;;)
        {
            auto endOfLine = text;

            switch (text.getAndAdvance())
            {
                case 0:     finished = true; break;
                case '\n':  break;
                case '\r':  if (*text == '\n') ++text; break;
                default:    continue;
            }

            strings.add (Txt (startOfLine, endOfLine));
            ++numLines;
            break;
        }
    }

    return numLines;
}

StringArray StringArray::fromTokens (StringRef stringToTokenise, b8 preserveQuotedStrings)
{
    StringArray s;
    s.addTokens (stringToTokenise, preserveQuotedStrings);
    return s;
}

StringArray StringArray::fromTokens (StringRef stringToTokenise,
                                     StringRef breakCharacters,
                                     StringRef quoteCharacters)
{
    StringArray s;
    s.addTokens (stringToTokenise, breakCharacters, quoteCharacters);
    return s;
}

StringArray StringArray::fromLines (StringRef stringToBreakUp)
{
    StringArray s;
    s.addLines (stringToBreakUp);
    return s;
}

//==============================================================================
z0 StringArray::removeDuplicates (b8 ignoreCase)
{
    for (i32 i = 0; i < size() - 1; ++i)
    {
        auto s = strings.getReference (i);

        for (i32 nextIndex = i + 1;;)
        {
            nextIndex = indexOf (s, ignoreCase, nextIndex);

            if (nextIndex < 0)
                break;

            strings.remove (nextIndex);
        }
    }
}

z0 StringArray::appendNumbersToDuplicates (b8 ignoreCase,
                                             b8 appendNumberToFirstInstance,
                                             CharPointer_UTF8 preNumberString,
                                             CharPointer_UTF8 postNumberString)
{
    if (preNumberString.getAddress() == nullptr)
        preNumberString = CharPointer_UTF8 (" (");

    if (postNumberString.getAddress() == nullptr)
        postNumberString = CharPointer_UTF8 (")");

    for (i32 i = 0; i < size() - 1; ++i)
    {
        auto& s = strings.getReference (i);
        auto nextIndex = indexOf (s, ignoreCase, i + 1);

        if (nextIndex >= 0)
        {
            auto original = s;
            i32 number = 0;

            if (appendNumberToFirstInstance)
                s = original + Txt (preNumberString) + Txt (++number) + Txt (postNumberString);
            else
                ++number;

            while (nextIndex >= 0)
            {
                set (nextIndex, (*this)[nextIndex] + Txt (preNumberString) + Txt (++number) + Txt (postNumberString));
                nextIndex = indexOf (original, ignoreCase, nextIndex + 1);
            }
        }
    }
}

z0 StringArray::ensureStorageAllocated (i32 minNumElements)
{
    strings.ensureStorageAllocated (minNumElements);
}

z0 StringArray::minimiseStorageOverheads()
{
    strings.minimiseStorageOverheads();
}

} // namespace drx
