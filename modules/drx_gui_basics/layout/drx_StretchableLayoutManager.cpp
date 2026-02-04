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

StretchableLayoutManager::StretchableLayoutManager() {}
StretchableLayoutManager::~StretchableLayoutManager() {}

//==============================================================================
z0 StretchableLayoutManager::clearAllItems()
{
    items.clear();
    totalSize = 0;
}

z0 StretchableLayoutManager::setItemLayout (i32k itemIndex,
                                              const f64 minimumSize,
                                              const f64 maximumSize,
                                              const f64 preferredSize)
{
    auto* layout = getInfoFor (itemIndex);

    if (layout == nullptr)
    {
        layout = new ItemLayoutProperties();
        layout->itemIndex = itemIndex;

        i32 i;
        for (i = 0; i < items.size(); ++i)
            if (items.getUnchecked (i)->itemIndex > itemIndex)
                break;

        items.insert (i, layout);
    }

    layout->minSize = minimumSize;
    layout->maxSize = maximumSize;
    layout->preferredSize = preferredSize;
    layout->currentSize = 0;
}

b8 StretchableLayoutManager::getItemLayout (i32k itemIndex,
                                              f64& minimumSize,
                                              f64& maximumSize,
                                              f64& preferredSize) const
{
    if (auto* layout = getInfoFor (itemIndex))
    {
        minimumSize = layout->minSize;
        maximumSize = layout->maxSize;
        preferredSize = layout->preferredSize;
        return true;
    }

    return false;
}

//==============================================================================
z0 StretchableLayoutManager::setTotalSize (i32k newTotalSize)
{
    totalSize = newTotalSize;

    fitComponentsIntoSpace (0, items.size(), totalSize, 0);
}

i32 StretchableLayoutManager::getItemCurrentPosition (i32k itemIndex) const
{
    i32 pos = 0;

    for (i32 i = 0; i < itemIndex; ++i)
        if (auto* layout = getInfoFor (i))
            pos += layout->currentSize;

    return pos;
}

i32 StretchableLayoutManager::getItemCurrentAbsoluteSize (i32k itemIndex) const
{
    if (auto* layout = getInfoFor (itemIndex))
        return layout->currentSize;

    return 0;
}

f64 StretchableLayoutManager::getItemCurrentRelativeSize (i32k itemIndex) const
{
    if (auto* layout = getInfoFor (itemIndex))
        return -layout->currentSize / (f64) totalSize;

    return 0;
}

z0 StretchableLayoutManager::setItemPosition (i32k itemIndex,
                                                i32 newPosition)
{
    for (i32 i = items.size(); --i >= 0;)
    {
        auto* layout = items.getUnchecked (i);

        if (layout->itemIndex == itemIndex)
        {
            auto realTotalSize = jmax (totalSize, getMinimumSizeOfItems (0, items.size()));
            auto minSizeAfterThisComp = getMinimumSizeOfItems (i, items.size());
            auto maxSizeAfterThisComp = getMaximumSizeOfItems (i + 1, items.size());

            newPosition = jmax (newPosition, totalSize - maxSizeAfterThisComp - layout->currentSize);
            newPosition = jmin (newPosition, realTotalSize - minSizeAfterThisComp);

            auto endPos = fitComponentsIntoSpace (0, i, newPosition, 0);

            endPos += layout->currentSize;

            fitComponentsIntoSpace (i + 1, items.size(), totalSize - endPos, endPos);
            updatePrefSizesToMatchCurrentPositions();
            break;
        }
    }
}

//==============================================================================
z0 StretchableLayoutManager::layOutComponents (Component** const components,
                                                 i32 numComponents,
                                                 i32 x, i32 y, i32 w, i32 h,
                                                 const b8 vertically,
                                                 const b8 resizeOtherDimension)
{
    setTotalSize (vertically ? h : w);
    i32 pos = vertically ? y : x;

    for (i32 i = 0; i < numComponents; ++i)
    {
        if (auto* layout = getInfoFor (i))
        {
            if (auto* c = components[i])
            {
                if (i == numComponents - 1)
                {
                    // if it's the last item, crop it to exactly fit the available space..
                    if (resizeOtherDimension)
                    {
                        if (vertically)
                            c->setBounds (x, pos, w, jmax (layout->currentSize, h - pos));
                        else
                            c->setBounds (pos, y, jmax (layout->currentSize, w - pos), h);
                    }
                    else
                    {
                        if (vertically)
                            c->setBounds (c->getX(), pos, c->getWidth(), jmax (layout->currentSize, h - pos));
                        else
                            c->setBounds (pos, c->getY(), jmax (layout->currentSize, w - pos), c->getHeight());
                    }
                }
                else
                {
                    if (resizeOtherDimension)
                    {
                        if (vertically)
                            c->setBounds (x, pos, w, layout->currentSize);
                        else
                            c->setBounds (pos, y, layout->currentSize, h);
                    }
                    else
                    {
                        if (vertically)
                            c->setBounds (c->getX(), pos, c->getWidth(), layout->currentSize);
                        else
                            c->setBounds (pos, c->getY(), layout->currentSize, c->getHeight());
                    }
                }
            }

            pos += layout->currentSize;
        }
    }
}


//==============================================================================
StretchableLayoutManager::ItemLayoutProperties* StretchableLayoutManager::getInfoFor (i32k itemIndex) const
{
    for (auto* i : items)
        if (i->itemIndex == itemIndex)
            return i;

    return nullptr;
}

i32 StretchableLayoutManager::fitComponentsIntoSpace (i32k startIndex,
                                                      i32k endIndex,
                                                      i32k availableSpace,
                                                      i32 startPos)
{
    // calculate the total sizes
    f64 totalIdealSize = 0.0;
    i32 totalMinimums = 0;

    for (i32 i = startIndex; i < endIndex; ++i)
    {
        auto* layout = items.getUnchecked (i);

        layout->currentSize = sizeToRealSize (layout->minSize, totalSize);

        totalMinimums += layout->currentSize;
        totalIdealSize += sizeToRealSize (layout->preferredSize, totalSize);
   }

    if (totalIdealSize <= 0)
        totalIdealSize = 1.0;

    // now calc the best sizes..
    i32 extraSpace = availableSpace - totalMinimums;

    while (extraSpace > 0)
    {
        i32 numWantingMoreSpace = 0;
        i32 numHavingTakenExtraSpace = 0;

        // first figure out how many comps want a slice of the extra space..
        for (i32 i = startIndex; i < endIndex; ++i)
        {
            auto* layout = items.getUnchecked (i);

            auto sizeWanted = sizeToRealSize (layout->preferredSize, totalSize);

            auto bestSize = jlimit (layout->currentSize,
                                    jmax (layout->currentSize,
                                          sizeToRealSize (layout->maxSize, totalSize)),
                                    roundToInt (sizeWanted * availableSpace / totalIdealSize));

            if (bestSize > layout->currentSize)
                ++numWantingMoreSpace;
        }

        // ..share out the extra space..
        for (i32 i = startIndex; i < endIndex; ++i)
        {
            auto* layout = items.getUnchecked (i);

            auto sizeWanted = sizeToRealSize (layout->preferredSize, totalSize);

            auto bestSize = jlimit (layout->currentSize,
                                    jmax (layout->currentSize, sizeToRealSize (layout->maxSize, totalSize)),
                                    roundToInt (sizeWanted * availableSpace / totalIdealSize));

            auto extraWanted = bestSize - layout->currentSize;

            if (extraWanted > 0)
            {
                auto extraAllowed = jmin (extraWanted,
                                          extraSpace / jmax (1, numWantingMoreSpace));

                if (extraAllowed > 0)
                {
                    ++numHavingTakenExtraSpace;
                    --numWantingMoreSpace;

                    layout->currentSize += extraAllowed;
                    extraSpace -= extraAllowed;
                }
            }
        }

        if (numHavingTakenExtraSpace <= 0)
            break;
    }

    // ..and calculate the end position
    for (i32 i = startIndex; i < endIndex; ++i)
    {
        auto* layout = items.getUnchecked (i);
        startPos += layout->currentSize;
    }

    return startPos;
}

i32 StretchableLayoutManager::getMinimumSizeOfItems (i32k startIndex,
                                                     i32k endIndex) const
{
    i32 totalMinimums = 0;

    for (i32 i = startIndex; i < endIndex; ++i)
        totalMinimums += sizeToRealSize (items.getUnchecked (i)->minSize, totalSize);

    return totalMinimums;
}

i32 StretchableLayoutManager::getMaximumSizeOfItems (i32k startIndex, i32k endIndex) const
{
    i32 totalMaximums = 0;

    for (i32 i = startIndex; i < endIndex; ++i)
        totalMaximums += sizeToRealSize (items.getUnchecked (i)->maxSize, totalSize);

    return totalMaximums;
}

z0 StretchableLayoutManager::updatePrefSizesToMatchCurrentPositions()
{
    for (i32 i = 0; i < items.size(); ++i)
    {
        auto* layout = items.getUnchecked (i);

        layout->preferredSize
            = (layout->preferredSize < 0) ? getItemCurrentRelativeSize (i)
                                          : getItemCurrentAbsoluteSize (i);
    }
}

i32 StretchableLayoutManager::sizeToRealSize (f64 size, i32 totalSpace)
{
    if (size < 0)
        size *= -totalSpace;

    return roundToInt (jmax (1.0, size));
}

} // namespace drx
