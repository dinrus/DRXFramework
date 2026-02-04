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

class Font::Native
{
public:
    HbFont font{};

    static Typeface::Ptr getDefaultPlatformTypefaceForFont (const Font&);
};

using GetTypefaceForFont = Typeface::Ptr (*)(const Font&);
GetTypefaceForFont drx_getTypefaceForFont = nullptr;

f32 Font::getDefaultMinimumHorizontalScaleFactor() noexcept                { return FontValues::minimumHorizontalScale; }
z0 Font::setDefaultMinimumHorizontalScaleFactor (f32 newValue) noexcept  { FontValues::minimumHorizontalScale = newValue; }

//==============================================================================
class TypefaceCache final : private DeletedAtShutdown
{
public:
    TypefaceCache()
    {
        setSize (10);
    }

    ~TypefaceCache()
    {
        clearSingletonInstance();
    }

    DRX_DECLARE_SINGLETON_INLINE (TypefaceCache, false)

    z0 setSize (i32k numToCache)
    {
        const ScopedWriteLock sl (lock);

        faces.clear();
        faces.insertMultiple (-1, CachedFace(), numToCache);
    }

    z0 clear()
    {
        const ScopedWriteLock sl (lock);

        setSize (faces.size());
        defaultFace = nullptr;
    }

    Typeface::Ptr findTypefaceFor (const Font& font)
    {
        const Key key { font.getTypefaceName(), font.getTypefaceStyle() };

        jassert (key.name.isNotEmpty());

        {
            const ScopedReadLock slr (lock);

            const auto range = makeRange (std::make_reverse_iterator (faces.end()),
                                          std::make_reverse_iterator (faces.begin()));

            for (auto& face : range)
            {
                if (face.key == key && face.typeface != nullptr)
                {
                    face.lastUsageCount = ++counter;
                    return face.typeface;
                }
            }
        }

        const ScopedWriteLock slw (lock);

        auto newFace = CachedFace { key,
                                    ++counter,
                                    drx_getTypefaceForFont != nullptr
                                        ? drx_getTypefaceForFont (font)
                                        : Font::getDefaultTypefaceForFont (font) };

        if (newFace.typeface == nullptr)
            return nullptr;

        const auto replaceIter = std::min_element (faces.begin(),
                                                   faces.end(),
                                                   [] (const auto& a, const auto& b)
                                                   {
                                                       return a.lastUsageCount < b.lastUsageCount;
                                                   });

        jassert (replaceIter != faces.end());
        auto& face = *replaceIter;

        face = std::move (newFace);

        if (defaultFace == nullptr && key == Key{})
            defaultFace = face.typeface;

        return face.typeface;
    }

    Typeface::Ptr getDefaultFace() const noexcept
    {
        const ScopedReadLock slr (lock);
        return defaultFace;
    }

private:
    struct Key
    {
        Txt name = Font::getDefaultSansSerifFontName(), style = Font::getDefaultStyle();

        b8 operator== (const Key& other) const
        {
            const auto tie = [] (const auto& x) { return std::tie (x.name, x.style); };
            return tie (*this) == tie (other);
        }

        b8 operator!= (const Key& other) const
        {
            return ! operator== (other);
        }
    };

    struct CachedFace
    {
        // Although it seems a bit wacky to store the name here, it's because it may be a
        // placeholder rather than a real one, e.g. "<Sans-Serif>" vs the actual typeface name.
        // Since the typeface itself doesn't know that it may have this alias, the name under
        // which it was fetched needs to be stored separately.
        Key key;
        size_t lastUsageCount = 0;
        Typeface::Ptr typeface;
    };

    Typeface::Ptr defaultFace;
    ReadWriteLock lock;
    Array<CachedFace> faces;
    size_t counter = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TypefaceCache)
};

z0 Typeface::setTypefaceCacheSize (i32 numFontsToCache)
{
    TypefaceCache::getInstance()->setSize (numFontsToCache);
}

z0 (*clearOpenGLGlyphCache)() = nullptr;

z0 Typeface::clearTypefaceCache()
{
    TypefaceCache::getInstance()->clear();

    RenderingHelpers::SoftwareRendererSavedState::clearGlyphCache();

    NullCheckedInvocation::invoke (clearOpenGLGlyphCache);
}

//==============================================================================
class Font::SharedFontInternal  : public ReferenceCountedObject
{
public:
    explicit SharedFontInternal (FontOptions x)
        : options (x.getName().isEmpty() ? x.withName (getDefaultSansSerifFontName()) : std::move (x))
    {
    }

    ReferenceCountedObjectPtr<SharedFontInternal> copy() const
    {
        const ScopedLock lock (mutex);
        return new SharedFontInternal (typeface, options);
    }

    Typeface::Ptr getTypefacePtr (const Font& f)
    {
        const ScopedLock lock (mutex);

        if (typeface == nullptr)
            typeface = options.getTypeface() != nullptr ? options.getTypeface() : TypefaceCache::getInstance()->findTypefaceFor (f);

        return typeface;
    }

    HbFont getFontPtr (const Font& f)
    {
        const ScopedLock lock (mutex);

        if (auto ptr = getTypefacePtr (f))
            return ptr->getNativeDetails().getFontAtPointSizeAndScale (f.getHeightInPoints(), f.getHorizontalScale());

        return {};
    }

    TypefaceAscentDescent getAscentDescent (const Font& f)
    {
        const ScopedLock lock (mutex);

        if (auto ptr = getTypefacePtr (f))
        {
            const auto ascentDescent = ptr->getNativeDetails().getAscentDescent (f.getMetricsKind());

            auto adjusted = ascentDescent;
            adjusted.ascent = getAscentOverride().value_or (adjusted.ascent);
            adjusted.descent = getDescentOverride().value_or (adjusted.descent);
            return adjusted;
        }

        return {};
    }

    z0 resetTypeface()
    {
        const ScopedLock lock (mutex);
        typeface = nullptr;
    }

    /*  We do not need to lock in these functions, as it's guaranteed
        that these data members can only change if there is a single Font
        instance referencing the shared state.
    */

    StringArray getFallbackFamilies() const
    {
        const auto fallbacks = options.getFallbacks();
        return StringArray (fallbacks.data(), (i32) fallbacks.size());
    }

    Txt getTypefaceName() const             { return options.getName(); }
    Txt getTypefaceStyle() const            { return options.getStyle(); }
    f32 getHeight() const                    { return options.getHeight(); }
    f32 getPointHeight() const               { return options.getPointHeight(); }
    f32 getHorizontalScale() const           { return options.getHorizontalScale(); }
    f32 getKerning() const                   { return options.getKerningFactor(); }
    b8 getUnderline() const                  { return options.getUnderline(); }
    b8 getFallbackEnabled() const            { return options.getFallbackEnabled(); }
    TypefaceMetricsKind getMetricsKind() const { return options.getMetricsKind(); }

    std::optional<f32> getAscentOverride() const  { return options.getAscentOverride(); }
    std::optional<f32> getDescentOverride() const { return options.getDescentOverride(); }

    /*  This shared state may be shared between two or more Font instances that are being
        read/modified from multiple threads.
        Before modifying a shared instance you *must* call dupeInternalIfShared to
        ensure that only one Font instance is pointing to the SharedFontInternal instance
        during the modification.
    */

    z0 setTypeface (Typeface::Ptr newTypeface)
    {
        jassert (getReferenceCount() == 1);
        typeface = newTypeface;

        if (typeface != nullptr)
            options = options.withTypeface (nullptr).withName ("").withStyle ("");

        options = options.withTypeface (typeface);
    }

    z0 setTypefaceName (Txt x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withName (x);
    }

    z0 setTypefaceStyle (Txt x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withStyle (x);
    }

    z0 setHeight (f32 x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withHeight (x);
    }

    z0 setPointHeight (f32 x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withPointHeight (x);
    }

    z0 setHorizontalScale (f32 x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withHorizontalScale (x);
    }

    z0 setKerning (f32 x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withKerningFactor (x);
    }

    z0 setAscentOverride (std::optional<f32> x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withAscentOverride (x);
    }

    z0 setDescentOverride (std::optional<f32> x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withDescentOverride (x);
    }

    z0 setUnderline (b8 x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withUnderline (x);
    }

    z0 setFallbackFamilies (const StringArray& x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withFallbacks ({ x.begin(), x.end() });
    }

    z0 setFallback (b8 x)
    {
        jassert (getReferenceCount() == 1);
        options = options.withFallbackEnabled (x);
    }

    b8 operator== (const SharedFontInternal& other) const
    {
        return options == other.options;
    }

    b8 operator<  (const SharedFontInternal& other) const
    {
        return options < other.options;
    }

private:
    SharedFontInternal (Typeface::Ptr t, FontOptions o)
        : typeface (t), options (std::move (o))
    {
    }

    Typeface::Ptr typeface;
    FontOptions options;
    CriticalSection mutex;
};

//==============================================================================
Font::Font (FontOptions opt)
    : font (new SharedFontInternal (std::move (opt)))
{
}

template <typename... Args>
auto legacyArgs (Args&&... args)
{
    auto result = FontOptions { std::forward<Args> (args)... }.withMetricsKind (TypefaceMetricsKind::legacy);

    if (result.getName().isEmpty())
        result = result.withName (Font::getDefaultSansSerifFontName());

    return result;
}

Font::Font()                                : font (new SharedFontInternal (legacyArgs())) {}
Font::Font (const Typeface::Ptr& typeface)  : font (new SharedFontInternal (legacyArgs (typeface))) {}
Font::Font (const Font& other) noexcept     : font (other.font) {}

Font::Font (f32 fontHeight, i32 styleFlags)
    : font (new SharedFontInternal (legacyArgs (fontHeight, styleFlags)))
{
}

Font::Font (const Txt& typefaceName, f32 fontHeight, i32 styleFlags)
    : font (new SharedFontInternal (legacyArgs (typefaceName, fontHeight, styleFlags)))
{
}

Font::Font (const Txt& typefaceName, const Txt& typefaceStyle, f32 fontHeight)
    : font (new SharedFontInternal (legacyArgs (typefaceName, typefaceStyle, fontHeight)))
{
}

Font& Font::operator= (const Font& other) noexcept
{
    Font copy { other };
    std::swap (copy.font, font);
    return *this;
}

Font::Font (Font&& other) noexcept
    : font (std::exchange (other.font, {}))
{
}

Font& Font::operator= (Font&& other) noexcept
{
    Font copy { std::move (other) };
    std::swap (copy.font, font);
    return *this;
}

Font::~Font() noexcept = default;

b8 Font::operator== (const Font& other) const noexcept
{
    return font == other.font
            || *font == *other.font;
}

b8 Font::operator!= (const Font& other) const noexcept
{
    return ! operator== (other);
}

b8 Font::compare (const Font& a, const Font& b) noexcept
{
    return *a.font < *b.font;
}

z0 Font::dupeInternalIfShared()
{
    if (font->getReferenceCount() > 1)
        font = font->copy();
}

//==============================================================================
struct FontPlaceholderNames
{
    Txt sans     = "<Sans-Serif>",
           serif    = "<Serif>",
           mono     = "<Monospaced>",
           regular  = "<Regular>",
           systemUi = "system-ui";
};

static const FontPlaceholderNames& getFontPlaceholderNames()
{
    static FontPlaceholderNames names;
    return names;
}

#if DRX_MSVC
// This is a workaround for the lack of thread-safety in MSVC's handling of function-local
// statics - if multiple threads all try to create the first Font object at the same time,
// it can cause a race-condition in creating these placeholder strings.
struct FontNamePreloader { FontNamePreloader() { getFontPlaceholderNames(); } };
static FontNamePreloader fnp;
#endif

const Txt& Font::getDefaultSansSerifFontName()       { return getFontPlaceholderNames().sans; }
const Txt& Font::getSystemUIFontName()               { return getFontPlaceholderNames().systemUi; }
const Txt& Font::getDefaultSerifFontName()           { return getFontPlaceholderNames().serif; }
const Txt& Font::getDefaultMonospacedFontName()      { return getFontPlaceholderNames().mono; }
const Txt& Font::getDefaultStyle()                   { return getFontPlaceholderNames().regular; }

Txt Font::getTypefaceName() const noexcept           { return font->getTypefaceName(); }
Txt Font::getTypefaceStyle() const noexcept          { return font->getTypefaceStyle(); }

z0 Font::setTypefaceName (const Txt& faceName)
{
    if (faceName != font->getTypefaceName())
    {
        jassert (faceName.isNotEmpty());

        dupeInternalIfShared();
        font->setTypeface (nullptr);
        font->setTypefaceName (faceName);
    }
}

z0 Font::setTypefaceStyle (const Txt& typefaceStyle)
{
    if (typefaceStyle != font->getTypefaceStyle())
    {
        dupeInternalIfShared();
        font->setTypeface (nullptr);
        font->setTypefaceStyle (typefaceStyle);
    }
}

Font Font::withTypefaceStyle (const Txt& newStyle) const
{
    Font f (*this);
    f.setTypefaceStyle (newStyle);
    return f;
}

StringArray Font::getAvailableStyles() const
{
    return findAllTypefaceStyles (getTypefacePtr()->getName());
}

z0 Font::setPreferredFallbackFamilies (const StringArray& fallbacks)
{
    if (getPreferredFallbackFamilies() != fallbacks)
    {
        dupeInternalIfShared();
        font->setFallbackFamilies (fallbacks);
    }
}

StringArray Font::getPreferredFallbackFamilies() const
{
    return font->getFallbackFamilies();
}

z0 Font::setFallbackEnabled (b8 enabled)
{
    if (getFallbackEnabled() != enabled)
    {
        dupeInternalIfShared();
        font->setFallback (enabled);
    }
}

b8 Font::getFallbackEnabled() const
{
    return font->getFallbackEnabled();
}

Typeface::Ptr Font::getTypefacePtr() const
{
    return font->getTypefacePtr (*this);
}

//==============================================================================
Font Font::withHeight (const f32 newHeight) const
{
    Font f (*this);
    f.setHeight (newHeight);
    return f;
}

f32 Font::getHeightToPointsFactor() const
{
    return font->getAscentDescent (*this).getHeightToPointsFactor();
}

Font Font::withPointHeight (f32 heightInPoints) const
{
    Font f (*this);
    f.setPointHeight (heightInPoints);
    return f;
}

z0 Font::setHeight (f32 newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getHeight(), newHeight))
    {
        dupeInternalIfShared();
        font->setHeight (newHeight);
        font->resetTypeface();
    }
}

z0 Font::setPointHeight (f32 newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getPointHeight(), newHeight))
    {
        dupeInternalIfShared();
        font->setPointHeight (newHeight);
        font->resetTypeface();
    }
}

z0 Font::setHeightWithoutChangingWidth (f32 newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getHeight(), newHeight))
    {
        dupeInternalIfShared();
        font->setHorizontalScale (font->getHorizontalScale() * (font->getHeight() / newHeight));
        font->setHeight (newHeight);
        font->resetTypeface();
    }
}

i32 Font::getStyleFlags() const noexcept
{
    i32 styleFlags = font->getUnderline() ? underlined : plain;

    if (isBold())    styleFlags |= bold;
    if (isItalic())  styleFlags |= italic;

    return styleFlags;
}

Font Font::withStyle (i32k newFlags) const
{
    Font f (*this);
    f.setStyleFlags (newFlags);
    return f;
}

z0 Font::setStyleFlags (i32k newFlags)
{
    if (getStyleFlags() != newFlags)
    {
        dupeInternalIfShared();
        font->setTypeface (nullptr);
        font->setTypefaceStyle (FontStyleHelpers::getStyleName (newFlags));
        font->setUnderline ((newFlags & underlined) != 0);
    }
}

z0 Font::setSizeAndStyle (f32 newHeight,
                            i32k newStyleFlags,
                            const f32 newHorizontalScale,
                            const f32 newKerningAmount)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getHeight(), newHeight)
         || ! approximatelyEqual (font->getHorizontalScale(), newHorizontalScale)
         || ! approximatelyEqual (font->getKerning(), newKerningAmount))
    {
        dupeInternalIfShared();
        font->setHeight (newHeight);
        font->setHorizontalScale (newHorizontalScale);
        font->setKerning (newKerningAmount);
        font->resetTypeface();
    }

    setStyleFlags (newStyleFlags);
}

z0 Font::setSizeAndStyle (f32 newHeight,
                            const Txt& newStyle,
                            const f32 newHorizontalScale,
                            const f32 newKerningAmount)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (! approximatelyEqual (font->getHeight(), newHeight)
         || ! approximatelyEqual (font->getHorizontalScale(), newHorizontalScale)
         || ! approximatelyEqual (font->getKerning(), newKerningAmount))
    {
        dupeInternalIfShared();
        font->setHeight (newHeight);
        font->setHorizontalScale (newHorizontalScale);
        font->setKerning (newKerningAmount);
        font->resetTypeface();
    }

    setTypefaceStyle (newStyle);
}

Font Font::withHorizontalScale (const f32 newHorizontalScale) const
{
    Font f (*this);
    f.setHorizontalScale (newHorizontalScale);
    return f;
}

z0 Font::setHorizontalScale (const f32 scaleFactor)
{
    dupeInternalIfShared();
    font->setHorizontalScale (scaleFactor);
    font->resetTypeface();
}

f32 Font::getHorizontalScale() const noexcept
{
    return font->getHorizontalScale();
}

f32 Font::getExtraKerningFactor() const noexcept
{
    return font->getKerning();
}

Font Font::withExtraKerningFactor (const f32 extraKerning) const
{
    Font f (*this);
    f.setExtraKerningFactor (extraKerning);
    return f;
}

z0 Font::setExtraKerningFactor (const f32 extraKerning)
{
    dupeInternalIfShared();
    font->setKerning (extraKerning);
    font->resetTypeface();
}

std::optional<f32> Font::getAscentOverride() const noexcept
{
    return font->getAscentOverride();
}

z0 Font::setAscentOverride (std::optional<f32> x)
{
    font->setAscentOverride (x);
}

std::optional<f32> Font::getDescentOverride() const noexcept
{
    return font->getDescentOverride();
}

z0 Font::setDescentOverride (std::optional<f32> x)
{
    font->setDescentOverride (x);
}

Font Font::boldened() const                 { return withStyle (getStyleFlags() | bold); }
Font Font::italicised() const               { return withStyle (getStyleFlags() | italic); }

b8 Font::isBold() const noexcept          { return FontStyleHelpers::isBold   (font->getTypefaceStyle()); }
b8 Font::isItalic() const noexcept        { return FontStyleHelpers::isItalic (font->getTypefaceStyle()); }
b8 Font::isUnderlined() const noexcept    { return font->getUnderline(); }

TypefaceMetricsKind Font::getMetricsKind() const noexcept { return font->getMetricsKind(); }

z0 Font::setBold (const b8 shouldBeBold)
{
    auto flags = getStyleFlags();
    setStyleFlags (shouldBeBold ? (flags | bold)
                                : (flags & ~bold));
}

z0 Font::setItalic (const b8 shouldBeItalic)
{
    auto flags = getStyleFlags();
    setStyleFlags (shouldBeItalic ? (flags | italic)
                                  : (flags & ~italic));
}

z0 Font::setUnderline (const b8 shouldBeUnderlined)
{
    dupeInternalIfShared();
    font->setUnderline (shouldBeUnderlined);
    font->resetTypeface();
}

f32 Font::getAscent() const
{
    return font->getAscentDescent (*this).getScaledAscent() * getHeight();
}

f32 Font::getHeight() const noexcept
{
    jassert ((font->getHeight() > 0.0f) != (font->getPointHeight() > 0.0f));
    const auto height = font->getHeight();
    return height > 0.0f ? height : font->getPointHeight() * font->getAscentDescent (*this).getPointsToHeightFactor();
}

f32 Font::getDescent() const              { return getHeight() - getAscent(); }

f32 Font::getHeightInPoints() const
{
    jassert ((font->getHeight() > 0.0f) != (font->getPointHeight() > 0.0f));
    const auto pointHeight = font->getPointHeight();

    if (pointHeight > 0.0f)
        return pointHeight;

    const auto factor = font->getAscentDescent (*this).getPointsToHeightFactor();

    if (factor > 0.0f)
        return font->getHeight() / factor;

    jassertfalse;
    return 0.0f;
}

f32 Font::getAscentInPoints() const       { return getAscent()  * getHeightToPointsFactor(); }
f32 Font::getDescentInPoints() const      { return getDescent() * getHeightToPointsFactor(); }

i32 Font::getStringWidth (const Txt& text) const
{
    DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
    return (i32) std::ceil (getStringWidthFloat (text));
    DRX_END_IGNORE_DEPRECATION_WARNINGS
}

f32 Font::getStringWidthFloat (const Txt& text) const
{
    if (auto typeface = getTypefacePtr())
    {
        const auto w = typeface->getStringWidth (getMetricsKind(), text, getHeight(), getHorizontalScale());
        return w + (getHeight() * getHorizontalScale() * getExtraKerningFactor() * (f32) text.length());
    }

    return 0;
}

z0 Font::findFonts (Array<Font>& destArray)
{
    for (auto& name : findAllTypefaceNames())
    {
        auto styles = findAllTypefaceStyles (name);

        Txt style ("Regular");

        if (! styles.contains (style, true))
            style = styles[0];

        destArray.add (FontOptions (name, style, FontValues::defaultFontHeight));
    }
}

static b8 characterNotRendered (u32 c)
{
    constexpr u32 points[]
    {
        // Control points
        0x0000, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x001A, 0x001B, 0x0085,

        // BIDI control points
        0x061C, 0x200E, 0x200F, 0x202A, 0x202B, 0x202C, 0x202D, 0x202E, 0x2066, 0x2067, 0x2068, 0x2069
    };

    return std::find (std::begin (points), std::end (points), c) != std::end (points);
}

static b8 isFontSuitableForCodepoint (const Font& font, t32 c)
{
    const auto& hbFont = font.getNativeDetails().font;

    if (hbFont == nullptr)
        return false;

    hb_codepoint_t glyph{};

    return characterNotRendered ((u32) c)
           || hb_font_get_nominal_glyph (hbFont.get(), (hb_codepoint_t) c, &glyph);
}

static b8 isFontSuitableForText (const Font& font, const Txt& str)
{
    for (const auto c : str)
        if (! isFontSuitableForCodepoint (font, c))
            return false;

    return true;
}

Font Font::findSuitableFontForText (const Txt& text, const Txt& language) const
{
    if (! getFallbackEnabled() || isFontSuitableForText (*this, text))
        return *this;

    for (const auto& fallback : getPreferredFallbackFamilies())
    {
        auto copy = *this;
        copy.setTypefaceName (fallback);

        if (isFontSuitableForText (copy, text))
            return copy;
    }

    const auto fallbackTypefacePtr = std::invoke ([&]
    {
        if (auto current = getTypefacePtr())
            return current;

        auto copy = *this;
        copy.setTypefaceName (Font::getDefaultSansSerifFontName());
        return copy.getTypefacePtr();
    });

    if (fallbackTypefacePtr != nullptr)
    {
        if (auto suggested = fallbackTypefacePtr->createSystemFallback (text, language))
        {
            auto copy = *this;

            if (copy.getTypefacePtr() != suggested)
            {
                copy.dupeInternalIfShared();
                copy.font->setTypeface (suggested);
            }

            return copy;
        }
    }

    return *this;
}

//==============================================================================
Txt Font::toString() const
{
    Txt s;

    if (getTypefaceName() != getDefaultSansSerifFontName())
        s << getTypefaceName() << "; ";

    s << Txt (getHeight(), 1);

    if (getTypefaceStyle() != getDefaultStyle())
        s << ' ' << getTypefaceStyle();

    return s;
}

Font Font::fromString (const Txt& fontDescription)
{
    i32k separator = fontDescription.indexOfChar (';');
    Txt name;

    if (separator > 0)
        name = fontDescription.substring (0, separator).trim();

    if (name.isEmpty())
        name = getDefaultSansSerifFontName();

    Txt sizeAndStyle (fontDescription.substring (separator + 1).trimStart());

    f32 height = sizeAndStyle.getFloatValue();
    if (height <= 0)
        height = 10.0f;

    const Txt style (sizeAndStyle.fromFirstOccurrenceOf (" ", false, false));

    return FontOptions (name, style, height);
}

Font::Native Font::getNativeDetails() const
{
    return { font->getFontPtr (*this) };
}

Typeface::Ptr Font::getDefaultTypefaceForFont (const Font& font)
{
    const auto resolvedTypeface = [&]() -> Typeface::Ptr
    {
        if (font.getTypefaceName() != getSystemUIFontName())
            return {};

        const auto systemTypeface = Typeface::findSystemTypeface();

        if (systemTypeface == nullptr)
            return {};

        if (systemTypeface->getStyle() == font.getTypefaceStyle())
            return systemTypeface;

        auto copy = font;
        copy.setTypefaceName (systemTypeface->getName());
        return getDefaultTypefaceForFont (copy);
    }();

    if (resolvedTypeface != nullptr)
        return resolvedTypeface;

    return Native::getDefaultPlatformTypefaceForFont (font);
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class FontTests : public UnitTest
{
public:
    FontTests() : UnitTest ("Font", UnitTestCategories::graphics) {}

    z0 runTest() override
    {
        const Span data { FontBinaryData::Karla_Regular_Typo_On_Offsets_Off };
        const auto face = Typeface::createSystemTypefaceFor (data.data(), data.size());

        beginTest ("Old constructor from Typeface");
        {
            DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
            Font f { face };
            DRX_END_IGNORE_DEPRECATION_WARNINGS

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == face->getStyle());
            expect (f.getTypefacePtr() == face);

            f.setTypefaceStyle ("Italic");

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == "Italic");
            expect (f.getTypefacePtr() != face);
        }

        beginTest ("FontOptions constructor from Typeface");
        {
            const FontOptions opt { face };
            expect (opt.getName() == face->getName());
            expect (opt.getStyle() == face->getStyle());
            expect (opt.getTypeface() == face);

            Font f { opt };

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == face->getStyle());
            expect (f.getTypefacePtr() == face);

            f.setTypefaceStyle ("Italic");

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == "Italic");
            expect (f.getTypefacePtr() != face);
        }

        beginTest ("FontOptions constructor from Typeface with style and name set");
        {
            const auto opt = FontOptions { face }.withName ("placeholder").withStyle ("Italic");
            expect (opt.getName() == face->getName());
            expect (opt.getStyle() == face->getStyle());
            expect (opt.getTypeface() == face);

            Font f { opt };

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == face->getStyle());
            expect (f.getTypefacePtr() == face);

            f.setTypefaceStyle ("Italic");

            expect (f.getTypefaceName() == face->getName());
            expect (f.getTypefaceStyle() == "Italic");
            expect (f.getTypefacePtr() != face);
        }

        auto a = FontOptions().withName ("placeholder").withStyle ("Italic");

        beginTest ("Setting Typeface on FontOptions replaces previous name/style");
        {
            auto b = a.withTypeface (face);

            expect (b.getName() == face->getName());
            expect (b.getStyle() == face->getStyle());
        }

        beginTest ("Setting a name or style on a FontOptions holding a typeface has no effect");
        {
            auto b = a.withTypeface (face).withName ("name").withStyle ("style");
            expect (b.getName() == face->getName());
            expect (b.getStyle() == face->getStyle());

            auto c = b.withTypeface (nullptr).withName ("name").withStyle ("style");
            expect (c.getName() == "name");
            expect (c.getStyle() == "style");
        }
    }
};

static FontTests fontTests;

#endif

} // namespace drx
