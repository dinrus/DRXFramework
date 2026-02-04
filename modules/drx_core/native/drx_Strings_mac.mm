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

Txt Txt::fromCFString (CFStringRef cfString)
{
    if (cfString == nullptr)
        return {};

    CFRange range = { 0, CFStringGetLength (cfString) };
    CFIndex bytesNeeded = 0;
    CFStringGetBytes (cfString, range, kCFStringEncodingUTF8, 0, false, nullptr, 0, &bytesNeeded);

    HeapBlock<UInt8> utf8 (bytesNeeded + 1);
    CFStringGetBytes (cfString, range, kCFStringEncodingUTF8, 0, false, utf8, bytesNeeded + 1, nullptr);

    return Txt (CharPointer_UTF8 ((const CharPointer_UTF8::CharType*) utf8.get()),
                   CharPointer_UTF8 ((const CharPointer_UTF8::CharType*) utf8.get() + bytesNeeded));
}

CFStringRef Txt::toCFString() const
{
    tukk const utf8 = toRawUTF8();

    if (CFStringRef result = CFStringCreateWithBytes (kCFAllocatorDefault, (const UInt8*) utf8,
                                                      (CFIndex) strlen (utf8), kCFStringEncodingUTF8, false))
        return result;

    // If CFStringCreateWithBytes fails, it probably means there was a UTF8 format
    // error, so we'll return an empty string rather than a null pointer.
    return Txt().toCFString();
}

Txt Txt::convertToPrecomposedUnicode() const
{
   #if DRX_IOS
    DRX_AUTORELEASEPOOL
    {
        return nsStringToDrx ([juceStringToNS (*this) precomposedStringWithCanonicalMapping]);
    }
   #else
    UnicodeMapping map;

    map.unicodeEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                              kUnicodeNoSubset,
                                              kTextEncodingDefaultFormat);

    map.otherEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                            kUnicodeCanonicalCompVariant,
                                            kTextEncodingDefaultFormat);

    map.mappingVersion = kUnicodeUseLatestMapping;

    UnicodeToTextInfo conversionInfo = {};
    Txt result;

    if (CreateUnicodeToTextInfo (&map, &conversionInfo) == noErr)
    {
        const size_t bytesNeeded = CharPointer_UTF16::getBytesRequiredFor (getCharPointer());

        HeapBlock<t8> tempOut;
        tempOut.calloc (bytesNeeded + 4);

        ByteCount bytesRead = 0;
        ByteCount outputBufferSize = 0;

        if (ConvertFromUnicodeToText (conversionInfo,
                                      bytesNeeded, (ConstUniCharArrayPtr) toUTF16().getAddress(),
                                      kUnicodeDefaultDirectionMask,
                                      0, {}, {}, {},
                                      bytesNeeded, &bytesRead,
                                      &outputBufferSize, tempOut) == noErr)
        {
            result = Txt (CharPointer_UTF16 (reinterpret_cast<CharPointer_UTF16::CharType*> (tempOut.get())));
        }

        DisposeUnicodeToTextInfo (&conversionInfo);
    }

    return result;
   #endif
}

} // namespace drx
