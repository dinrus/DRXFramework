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

StretchableObjectResizer::StretchableObjectResizer() {}
StretchableObjectResizer::~StretchableObjectResizer() {}

z0 StretchableObjectResizer::addItem (const f64 size,
                                        const f64 minSize, const f64 maxSize,
                                        i32k order)
{
    // the order must be >= 0 but less than the maximum integer value.
    jassert (order >= 0 && order < std::numeric_limits<i32>::max());
    jassert (maxSize >= minSize);

    Item item;
    item.size = size;
    item.minSize = minSize;
    item.maxSize = maxSize;
    item.order = order;
    items.add (item);
}

f64 StretchableObjectResizer::getItemSize (i32k index) const noexcept
{
    return isPositiveAndBelow (index, items.size()) ? items.getReference (index).size
                                                    : 0.0;
}

z0 StretchableObjectResizer::resizeToFit (const f64 targetSize)
{
    i32 order = 0;

    for (;;)
    {
        f64 currentSize = 0;
        f64 minSize = 0;
        f64 maxSize = 0;

        i32 nextHighestOrder = std::numeric_limits<i32>::max();

        for (i32 i = 0; i < items.size(); ++i)
        {
            const Item& it = items.getReference (i);
            currentSize += it.size;

            if (it.order <= order)
            {
                minSize += it.minSize;
                maxSize += it.maxSize;
            }
            else
            {
                minSize += it.size;
                maxSize += it.size;
                nextHighestOrder = jmin (nextHighestOrder, it.order);
            }
        }

        const f64 thisIterationTarget = jlimit (minSize, maxSize, targetSize);

        if (thisIterationTarget >= currentSize)
        {
            const f64 availableExtraSpace = maxSize - currentSize;
            const f64 targetAmountOfExtraSpace = thisIterationTarget - currentSize;
            const f64 scale = availableExtraSpace > 0 ? targetAmountOfExtraSpace / availableExtraSpace : 1.0;

            for (i32 i = 0; i < items.size(); ++i)
            {
                Item& it = items.getReference (i);

                if (it.order <= order)
                    it.size = jlimit (it.minSize, it.maxSize, it.size + (it.maxSize - it.size) * scale);
            }
        }
        else
        {
            const f64 amountOfSlack = currentSize - minSize;
            const f64 targetAmountOfSlack = thisIterationTarget - minSize;
            const f64 scale = targetAmountOfSlack / amountOfSlack;

            for (i32 i = 0; i < items.size(); ++i)
            {
                Item& it = items.getReference (i);

                if (it.order <= order)
                    it.size = jmax (it.minSize, it.minSize + (it.size - it.minSize) * scale);
            }
        }

        if (nextHighestOrder < std::numeric_limits<i32>::max())
            order = nextHighestOrder;
        else
            break;
    }
}

} // namespace drx
