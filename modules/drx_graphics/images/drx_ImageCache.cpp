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

struct ImageCache::Pimpl     : private Timer,
                               private DeletedAtShutdown
{
    Pimpl() = default;

    ~Pimpl() override
    {
        stopTimer();
        clearSingletonInstance();
    }

    DRX_DECLARE_SINGLETON_INLINE (ImageCache::Pimpl, false)

    Image getFromHashCode (const z64 hashCode) noexcept
    {
        const ScopedLock sl (lock);

        for (auto& item : images)
        {
            if (item.hashCode == hashCode)
            {
                item.lastUseTime = Time::getApproximateMillisecondCounter();
                return item.image;
            }
        }

        return {};
     }

    z0 addImageToCache (const Image& image, const z64 hashCode)
    {
        if (image.isValid())
        {
            if (! isTimerRunning())
                startTimer (2000);

            const ScopedLock sl (lock);
            images.add ({ image, hashCode, Time::getApproximateMillisecondCounter() });
        }
    }

    z0 timerCallback() override
    {
        auto now = Time::getApproximateMillisecondCounter();

        const ScopedLock sl (lock);

        for (i32 i = images.size(); --i >= 0;)
        {
            auto& item = images.getReference (i);

            if (item.image.getReferenceCount() <= 1)
            {
                if (now > item.lastUseTime + cacheTimeout || now < item.lastUseTime - 1000)
                    images.remove (i);
            }
            else
            {
                item.lastUseTime = now; // multiply-referenced, so this image is still in use.
            }
        }

        if (images.isEmpty())
            stopTimer();
    }

    z0 releaseUnusedImages()
    {
        const ScopedLock sl (lock);

        for (i32 i = images.size(); --i >= 0;)
            if (images.getReference (i).image.getReferenceCount() <= 1)
                images.remove (i);
    }

    struct Item
    {
        Image image;
        z64 hashCode;
        u32 lastUseTime;
    };

    Array<Item> images;
    CriticalSection lock;
    u32 cacheTimeout = 5000;

    DRX_DECLARE_NON_COPYABLE (Pimpl)
};

//==============================================================================
Image ImageCache::getFromHashCode (const z64 hashCode)
{
    if (Pimpl::getInstanceWithoutCreating() != nullptr)
        return Pimpl::getInstanceWithoutCreating()->getFromHashCode (hashCode);

    return {};
}

z0 ImageCache::addImageToCache (const Image& image, const z64 hashCode)
{
    Pimpl::getInstance()->addImageToCache (image, hashCode);
}

Image ImageCache::getFromFile (const File& file)
{
    auto hashCode = file.hashCode64();
    auto image = getFromHashCode (hashCode);

    if (image.isNull())
    {
        image = ImageFileFormat::loadFrom (file);
        addImageToCache (image, hashCode);
    }

    return image;
}

Image ImageCache::getFromMemory (ukk imageData, i32k dataSize)
{
    auto hashCode = (z64) (pointer_sized_int) imageData;
    auto image = getFromHashCode (hashCode);

    if (image.isNull())
    {
        image = ImageFileFormat::loadFrom (imageData, (size_t) dataSize);
        addImageToCache (image, hashCode);
    }

    return image;
}

z0 ImageCache::setCacheTimeout (i32k millisecs)
{
    jassert (millisecs >= 0);
    Pimpl::getInstance()->cacheTimeout = (u32) millisecs;
}

z0 ImageCache::releaseUnusedImages()
{
    Pimpl::getInstance()->releaseUnusedImages();
}

} // namespace drx
