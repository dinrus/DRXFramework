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

GridItem::Property::Property() noexcept : isAuto (true)
{
}

GridItem::Property::Property (GridItem::Keyword keyword) noexcept : isAuto (keyword == GridItem::Keyword::autoValue)
{
    jassert (keyword == GridItem::Keyword::autoValue);
}

GridItem::Property::Property (tukk lineNameToUse) noexcept : GridItem::Property (Txt (lineNameToUse))
{
}

GridItem::Property::Property (const Txt& lineNameToUse) noexcept : name (lineNameToUse), number (1)
{
}

GridItem::Property::Property (i32 numberToUse) noexcept : number (numberToUse)
{
}

GridItem::Property::Property (i32 numberToUse, const Txt& lineNameToUse) noexcept
    : name (lineNameToUse), number (numberToUse)
{
}

GridItem::Property::Property (Span spanToUse) noexcept
    : name (spanToUse.name), number (spanToUse.number), isSpan (true)
{
}


//==============================================================================
GridItem::Margin::Margin() noexcept : left(), right(), top(), bottom() {}

GridItem::Margin::Margin (i32 v) noexcept : GridItem::Margin::Margin (static_cast<f32> (v)) {}

GridItem::Margin::Margin (f32 v) noexcept : left (v), right (v), top (v), bottom (v) {}

GridItem::Margin::Margin (f32 t, f32 r, f32 b, f32 l) noexcept : left (l), right (r), top (t), bottom (b) {}

//==============================================================================
GridItem::GridItem() noexcept = default;
GridItem::GridItem (Component& componentToUse) noexcept  : associatedComponent (&componentToUse) {}
GridItem::GridItem (Component* componentToUse) noexcept  : associatedComponent (componentToUse) {}

z0 GridItem::setArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd)
{
    column.start = columnStart;
    column.end = columnEnd;
    row.start = rowStart;
    row.end = rowEnd;
}

z0 GridItem::setArea (Property rowStart, Property columnStart)
{
    column.start = columnStart;
    row.start = rowStart;
}

z0 GridItem::setArea (const Txt& areaName)
{
    area = areaName;
}

GridItem GridItem::withArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd) const noexcept
{
    auto gi = *this;
    gi.setArea (rowStart, columnStart, rowEnd, columnEnd);
    return gi;
}

GridItem GridItem::withArea (Property rowStart, Property columnStart) const noexcept
{
    auto gi = *this;
    gi.setArea (rowStart, columnStart);
    return gi;
}

GridItem GridItem::withArea (const Txt& areaName) const noexcept
{
    auto gi = *this;
    gi.setArea (areaName);
    return gi;
}

GridItem GridItem::withRow (StartAndEndProperty newRow) const noexcept
{
    auto gi = *this;
    gi.row = newRow;
    return gi;
}

GridItem GridItem::withColumn (StartAndEndProperty newColumn) const noexcept
{
    auto gi = *this;
    gi.column = newColumn;
    return gi;
}

GridItem GridItem::withAlignSelf (AlignSelf newAlignSelf) const noexcept
{
    auto gi = *this;
    gi.alignSelf = newAlignSelf;
    return gi;
}

GridItem GridItem::withJustifySelf (JustifySelf newJustifySelf) const noexcept
{
    auto gi = *this;
    gi.justifySelf = newJustifySelf;
    return gi;
}

GridItem GridItem::withWidth (f32 newWidth) const noexcept
{
    auto gi = *this;
    gi.width = newWidth;
    return gi;
}

GridItem GridItem::withHeight (f32 newHeight) const noexcept
{
    auto gi = *this;
    gi.height = newHeight;
    return gi;
}

GridItem GridItem::withSize (f32 newWidth, f32 newHeight) const noexcept
{
    auto gi = *this;
    gi.width = newWidth;
    gi.height = newHeight;
    return gi;
}

GridItem GridItem::withMargin (Margin newHeight) const noexcept
{
    auto gi = *this;
    gi.margin = newHeight;
    return gi;
}

GridItem GridItem::withOrder (i32 newOrder) const noexcept
{
    auto gi = *this;
    gi.order = newOrder;
    return gi;
}

} // namespace drx
