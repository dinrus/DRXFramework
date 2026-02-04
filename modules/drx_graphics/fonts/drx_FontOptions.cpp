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

struct FontValues
{
    static f32 limitFontHeight (const f32 height) noexcept
    {
        return jlimit (0.1f, 10000.0f, height);
    }

    inline static constexpr f32 defaultFontHeight = 14.0f;
    static f32 minimumHorizontalScale;

    FontValues() = delete;
};

f32 FontValues::minimumHorizontalScale = 0.7f;

FontOptions::FontOptions()
    : FontOptions (FontValues::defaultFontHeight, Font::plain)
{
}

FontOptions::FontOptions (f32 fontHeight)
    : FontOptions (fontHeight, Font::plain)
{
}

FontOptions::FontOptions (f32 fontHeight, i32 styleFlags)
    : FontOptions ({}, fontHeight, styleFlags)
{
}

FontOptions::FontOptions (const Txt& typefaceName, f32 fontHeight, i32 styleFlags)
    : FontOptions (typefaceName, FontStyleHelpers::getStyleName (styleFlags), fontHeight)
{
    underlined = (styleFlags & Font::FontStyleFlags::underlined) != 0;
}

FontOptions::FontOptions (const Txt& typefaceName, const Txt& typefaceStyle, f32 fontHeight)
    : name (typefaceName),
      style (typefaceStyle),
      height (FontValues::limitFontHeight (fontHeight))
{
}

FontOptions::FontOptions (const Typeface::Ptr& ptr)
    : name (ptr->getName()),
      style (ptr->getStyle()),
      typeface (ptr),
      height (FontValues::defaultFontHeight)
{
}

auto FontOptions::tie() const
{
    return std::tuple (name,
                       style,
                       typeface.get(),
                       fallbacks,
                       metricsKind,
                       ascentOverride,
                       descentOverride,
                       height,
                       pointHeight,
                       tracking,
                       horizontalScale,
                       fallbackEnabled,
                       underlined);
}

b8 FontOptions::operator== (const FontOptions& other) const { return tie() == other.tie(); }
b8 FontOptions::operator!= (const FontOptions& other) const { return tie() != other.tie(); }
b8 FontOptions::operator<  (const FontOptions& other) const { return tie() <  other.tie(); }
b8 FontOptions::operator<= (const FontOptions& other) const { return tie() <= other.tie(); }
b8 FontOptions::operator>  (const FontOptions& other) const { return tie() >  other.tie(); }
b8 FontOptions::operator>= (const FontOptions& other) const { return tie() >= other.tie(); }

} // namespace drx
