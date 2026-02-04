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

// This symbol is available on all the platforms we support, but not declared in the CoreText headers on older platforms.
extern "C" CTFontRef CTFontCreateForStringWithLanguage (CTFontRef currentFont,
                                                        CFStringRef string,
                                                        CFRange range,
                                                        CFStringRef language);

class CoreTextTypeface final : public Typeface
{
    static auto& getRegistered()
    {
        class Registered
        {
        public:
            z0 add (CTFontRef ref)
            {
                const std::scoped_lock lock { mutex };
                set.emplace (addOwner (ref));
            }

            z0 remove (CTFontRef ref)
            {
                const std::scoped_lock lock { mutex };
                set.erase (addOwner (ref));
            }

            CFUniquePtr<CTFontRef> findMatch (const Txt& fontName, const Txt& fontStyle) const
            {
                const std::scoped_lock lock { mutex };

                for (auto& item : set)
                {
                    const auto keyAsString = [&] (auto key)
                    {
                        return Txt::fromCFString (CFUniquePtr<CFStringRef> { CTFontCopyName (item.get(), key) }.get());
                    };

                    const auto family = keyAsString (kCTFontFamilyNameKey);
                    const auto style = keyAsString (kCTFontStyleNameKey);

                    if (fontName == family && (style.isEmpty() || fontStyle.equalsIgnoreCase (style)))
                        return addOwner (item.get());
                }

                return nullptr;
            }

            std::vector<CFUniquePtr<CTFontRef>> findAllStylesForFamily (const Txt& fontName) const
            {
                const std::scoped_lock lock { mutex };

                std::vector<CFUniquePtr<CTFontRef>> result;

                for (auto& item : set)
                {
                    const auto keyAsString = [&] (auto key)
                    {
                        return Txt::fromCFString (CFUniquePtr<CFStringRef> { CTFontCopyName (item.get(), key) }.get());
                    };

                    const auto family = keyAsString (kCTFontFamilyNameKey);

                    if (fontName == family)
                        result.emplace_back (addOwner (item.get()));
                }

                return result;
            }

            std::set<Txt> getRegisteredFamilies() const
            {
                const std::scoped_lock lock { mutex };
                std::set<Txt> result;

                for (const auto& item : set)
                {
                    const CFUniquePtr<CFStringRef> family { CTFontCopyName (item.get(), kCTFontFamilyNameKey) };
                    result.insert (Txt::fromCFString (family.get()));
                }

                return result;
            }

        private:
            static CFUniquePtr<CTFontRef> addOwner (CTFontRef reference)
            {
                CFRetain (reference);
                return CFUniquePtr<CTFontRef> { reference };
            }

            std::set<CFUniquePtr<CTFontRef>> set;
            mutable std::mutex mutex;
        };

        static Registered registered;
        return registered;
    }

public:
    static Typeface::Ptr from (const Font& font)
    {
        auto ctFont = [&]() -> CFUniquePtr<CTFontRef>
        {
            if (auto f = getRegistered().findMatch (font.getTypefaceName(), font.getTypefaceStyle()))
                return f;

            CFUniquePtr<CFStringRef> cfFontFamily (FontStyleHelpers::getConcreteFamilyName (font).toCFString());

            if (cfFontFamily == nullptr)
                return {};

            CFUniquePtr<CFStringRef> cfFontStyle (findBestAvailableStyle (font).toCFString());

            if (cfFontStyle == nullptr)
                return {};

            CFStringRef keys[] { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute };
            CFTypeRef values[] { cfFontFamily.get(), cfFontStyle.get() };

            CFUniquePtr<CFDictionaryRef> fontDescAttributes (CFDictionaryCreate (nullptr,
                                                                                 (ukk*) &keys,
                                                                                 (ukk*) &values,
                                                                                 numElementsInArray (keys),
                                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                                 &kCFTypeDictionaryValueCallBacks));

            if (fontDescAttributes == nullptr)
                return {};

            CFUniquePtr<CTFontDescriptorRef> ctFontDescRef (CTFontDescriptorCreateWithAttributes (fontDescAttributes.get()));

            if (ctFontDescRef == nullptr)
                return {};

            return CFUniquePtr<CTFontRef> { CTFontCreateWithFontDescriptor (ctFontDescRef.get(), 1, nullptr) };
        }();

        if (ctFont == nullptr)
            return {};

        HbFont result { hb_coretext_font_create (ctFont.get()) };

        if (result == nullptr)
            return {};

        FontStyleHelpers::initSynthetics (result.get(), font);

        return new CoreTextTypeface (std::move (ctFont), std::move (result), font.getTypefaceName(), font.getTypefaceStyle());
    }

    static Typeface::Ptr from (Span<const std::byte> data)
    {
        // We can't use CFDataCreate here as this triggers a false positive in ASAN
        // so copy the data manually and use CFDataCreateWithBytesNoCopy
        MemoryBlock copy { data.data(), data.size() };

        const CFUniquePtr<CFDataRef> cfData { CFDataCreateWithBytesNoCopy (kCFAllocatorDefault,
                                                                           static_cast<const UInt8*> (copy.getData()),
                                                                           (CFIndex) copy.getSize(),
                                                                           kCFAllocatorNull) };

        if (cfData == nullptr)
            return {};

       #if DRX_IOS
        // Workaround for a an obscure iOS bug which can cause the app to dead-lock
        // when loading custom type faces. See: http://www.openradar.me/18778790 and
        // http://stackoverflow.com/questions/40242370/app-hangs-in-simulator
        [UIFont systemFontOfSize: 12];
       #endif

        const CFUniquePtr<CGDataProviderRef> provider { CGDataProviderCreateWithCFData (cfData.get()) };

        if (provider == nullptr)
            return {};

        const CFUniquePtr<CGFontRef> font { CGFontCreateWithDataProvider (provider.get()) };

        if (font == nullptr)
            return {};

        CFUniquePtr<CTFontRef> ctFont { CTFontCreateWithGraphicsFont (font.get(), 1.0f, {}, {}) };

        if (ctFont == nullptr)
            return {};

        HbFont result { hb_coretext_font_create (ctFont.get()) };

        if (result == nullptr)
            return {};

        const CFUniquePtr<CFStringRef> family { CTFontCopyName (ctFont.get(), kCTFontFamilyNameKey) };
        const CFUniquePtr<CFStringRef> style  { CTFontCopyName (ctFont.get(), kCTFontStyleNameKey) };

        return new CoreTextTypeface (std::move (ctFont),
                                     std::move (result),
                                     Txt::fromCFString (family.get()),
                                     Txt::fromCFString (style.get()),
                                     std::move (copy));
    }

    Native getNativeDetails() const override
    {
        return Native { hb.get(), nonPortableMetrics };
    }

    Typeface::Ptr createSystemFallback (const Txt& c, const Txt& language) const override
    {
        const CFUniquePtr<CFStringRef> cfText { c.toCFString() };
        const CFUniquePtr<CFStringRef> cfLanguage { language.toCFString() };

        auto* old = ctFont.get();
        const CFUniquePtr<CFStringRef> oldName { CTFontCopyFamilyName (old) };
        const CFUniquePtr<CTFontDescriptorRef> oldDescriptor { CTFontCopyFontDescriptor (old) };
        const CFUniquePtr<CFStringRef> oldStyle { (CFStringRef) CTFontDescriptorCopyAttribute (oldDescriptor.get(),
                                                                                               kCTFontStyleNameAttribute) };

        CFUniquePtr<CTFontRef> newFont { CTFontCreateForStringWithLanguage (old,
                                                                            cfText.get(),
                                                                            CFRangeMake (0, CFStringGetLength (cfText.get())),
                                                                            cfLanguage.get()) };

        const CFUniquePtr<CFStringRef> newName { CTFontCopyFamilyName (newFont.get()) };
        const CFUniquePtr<CTFontDescriptorRef> descriptor { CTFontCopyFontDescriptor (newFont.get()) };
        const CFUniquePtr<CFStringRef> newStyle { (CFStringRef) CTFontDescriptorCopyAttribute (descriptor.get(),
                                                                                               kCTFontStyleNameAttribute) };

        HbFont result { hb_coretext_font_create (newFont.get()) };

        if (result == nullptr)
            return {};

        return new CoreTextTypeface { std::move (newFont),
                                      std::move (result),
                                      Txt::fromCFString (newName.get()),
                                      Txt::fromCFString (newStyle.get()),
                                      {} };
    }

    static std::set<Txt> getRegisteredFamilies()
    {
        return getRegistered().getRegisteredFamilies();
    }

    static std::vector<CFUniquePtr<CTFontRef>> findRegisteredStylesForFamily (const Txt& family)
    {
        return getRegistered().findAllStylesForFamily (family);
    }

    ~CoreTextTypeface() override
    {
        getRegistered().remove (ctFont.get());
    }

    CTFontRef getFontRef() const
    {
        return ctFont.get();
    }

    static Typeface::Ptr findSystemTypeface()
    {
        CFUniquePtr<CTFontRef> defaultCtFont (CTFontCreateUIFontForLanguage (kCTFontUIFontSystem, 0.0, nullptr));
        const CFUniquePtr<CFStringRef> newName { CTFontCopyFamilyName (defaultCtFont.get()) };
        const CFUniquePtr<CTFontDescriptorRef> descriptor { CTFontCopyFontDescriptor (defaultCtFont.get()) };
        const CFUniquePtr<CFStringRef> newStyle { (CFStringRef) CTFontDescriptorCopyAttribute (descriptor.get(),
                                                                                               kCTFontStyleNameAttribute) };

        HbFont result { hb_coretext_font_create (defaultCtFont.get()) };

        if (result == nullptr)
            return {};

        return new CoreTextTypeface { std::move (defaultCtFont),
                                      std::move (result),
                                      Txt::fromCFString (newName.get()),
                                      Txt::fromCFString (newStyle.get()),
                                      {} };
    }

private:
    CoreTextTypeface (CFUniquePtr<CTFontRef> nativeFont,
                      HbFont fontIn,
                      const Txt& name,
                      const Txt& style,
                      MemoryBlock data = {})
        : Typeface (name, style),
          ctFont (std::move (nativeFont)),
          hb (std::move (fontIn)),
          storage (std::move (data))
    {
        if (! storage.isEmpty())
            getRegistered().add (ctFont.get());
    }

    static Txt findBestAvailableStyle (const Font& font)
    {
        const auto availableStyles = Font::findAllTypefaceStyles (font.getTypefaceName());
        const auto style = font.getTypefaceStyle();

        if (availableStyles.contains (style))
            return style;

        return availableStyles[0];
    }

    // We store this, rather than calling hb_coretext_font_get_ct_font, because harfbuzz may
    // override the font cascade list in the returned font.
    CFUniquePtr<CTFontRef> ctFont;
    HbFont hb;
    MemoryBlock storage;
    TypefaceAscentDescent nonPortableMetrics = [&]
    {
        const CFUniquePtr<CGFontRef> cgFont { CTFontCopyGraphicsFont (ctFont.get(), nullptr) };
        const auto upem = (f32) CGFontGetUnitsPerEm (cgFont.get());
        return TypefaceAscentDescent { (f32) std::abs (CGFontGetAscent (cgFont.get())  / upem),
                                       (f32) std::abs (CGFontGetDescent (cgFont.get()) / upem) };
    }();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreTextTypeface)
};

//==============================================================================
Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return CoreTextTypeface::from (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (Span<const std::byte> data)
{
    return CoreTextTypeface::from (data);
}

z0 Typeface::scanFolderForFonts (const File& folder)
{
    for (auto& file : folder.findChildFiles (File::findFiles, false, "*.otf;*.ttf"))
        if (auto urlref = CFUniquePtr<CFURLRef> (CFURLCreateWithFileSystemPath (kCFAllocatorDefault, file.getFullPathName().toCFString(), kCFURLPOSIXPathStyle, true)))
            CTFontManagerRegisterFontsForURL (urlref.get(), kCTFontManagerScopeProcess, nullptr);
}

Typeface::Ptr Typeface::findSystemTypeface()
{
    return CoreTextTypeface::findSystemTypeface();
}

StringArray Font::findAllTypefaceNames()
{
    StringArray names;

    auto nameSet = CoreTextTypeface::getRegisteredFamilies();

    CFUniquePtr<CTFontCollectionRef> fontCollectionRef (CTFontCollectionCreateFromAvailableFonts (nullptr));
    CFUniquePtr<CFArrayRef> fontDescriptorArray (CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef.get()));

    for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray.get()); ++i)
    {
        auto ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray.get(), i);
        CFUniquePtr<CFStringRef> cfsFontFamily ((CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontFamilyNameAttribute));

        nameSet.insert (Txt::fromCFString (cfsFontFamily.get()));
    }

    for (auto& item : nameSet)
        names.add (item);

    return names;
}

StringArray Font::findAllTypefaceStyles (const Txt& family)
{
    if (FontStyleHelpers::isPlaceholderFamilyName (family))
        return findAllTypefaceStyles (FontStyleHelpers::getConcreteFamilyNameFromPlaceholder (family));

    std::set<Txt> uniqueResults;
    StringArray orderedResults;

    for (const auto& font : CoreTextTypeface::findRegisteredStylesForFamily (family))
    {
        const CFUniquePtr<CTFontDescriptorRef> descriptor (CTFontCopyFontDescriptor (font.get()));
        const CFUniquePtr<CFStringRef> cfsFontStyle ((CFStringRef) CTFontDescriptorCopyAttribute (descriptor.get(), kCTFontStyleNameAttribute));
        const auto name = Txt::fromCFString (cfsFontStyle.get());

        if (uniqueResults.insert (name).second)
            orderedResults.add (name);
    }

    CFUniquePtr<CFStringRef> cfsFontFamily (family.toCFString());
    CFStringRef keys[] { kCTFontFamilyNameAttribute };
    CFTypeRef values[] { cfsFontFamily.get() };

    CFUniquePtr<CFDictionaryRef> fontDescAttributes (CFDictionaryCreate (nullptr, (ukk*) &keys, (ukk*) &values, numElementsInArray (keys),
                                                                         &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

    CFUniquePtr<CTFontDescriptorRef> ctFontDescRef (CTFontDescriptorCreateWithAttributes (fontDescAttributes.get()));
    CFUniquePtr<CFArrayRef> fontFamilyArray (CFArrayCreate (kCFAllocatorDefault, (ukk*) &ctFontDescRef, 1, &kCFTypeArrayCallBacks));

    CFUniquePtr<CTFontCollectionRef> fontCollectionRef (CTFontCollectionCreateWithFontDescriptors (fontFamilyArray.get(), nullptr));

    if (auto fontDescriptorArray = CFUniquePtr<CFArrayRef> (CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef.get())))
    {
        for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray.get()); ++i)
        {
            auto ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray.get(), i);
            CFUniquePtr<CFStringRef> cfsFontStyle ((CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontStyleNameAttribute));
            const auto name = Txt::fromCFString (cfsFontStyle.get());

            if (uniqueResults.insert (name).second)
                orderedResults.add (name);
        }
    }

    return orderedResults;
}

struct DefaultFontNames
{
   #if DRX_IOS
    Txt defaultSans  { "Helvetica" },
           defaultSerif { "Times New Roman" },
           defaultFixed { "Courier New" };
   #else
    Txt defaultSans  { "Lucida Grande" },
           defaultSerif { "Times New Roman" },
           defaultFixed { "Menlo" };
   #endif
};

Typeface::Ptr Font::Native::getDefaultPlatformTypefaceForFont (const Font& font)
{
    static DefaultFontNames defaultNames;

    auto newFont = font;
    auto faceName = font.getTypefaceName();

    if (faceName == getDefaultSansSerifFontName())       newFont.setTypefaceName (defaultNames.defaultSans);
    else if (faceName == getDefaultSerifFontName())      newFont.setTypefaceName (defaultNames.defaultSerif);
    else if (faceName == getDefaultMonospacedFontName()) newFont.setTypefaceName (defaultNames.defaultFixed);

    if (font.getTypefaceStyle() == getDefaultStyle())
        newFont.setTypefaceStyle ("Regular");

    return Typeface::createSystemTypefaceFor (newFont);
}

} // namespace drx
