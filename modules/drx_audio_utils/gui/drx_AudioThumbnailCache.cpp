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

class AudioThumbnailCache::ThumbnailCacheEntry
{
public:
    ThumbnailCacheEntry (const z64 hashCode)
        : hash (hashCode),
          lastUsed (Time::getMillisecondCounter())
    {
    }

    ThumbnailCacheEntry (InputStream& in)
        : hash (in.readInt64()),
          lastUsed (0)
    {
        const z64 len = in.readInt64();
        in.readIntoMemoryBlock (data, (ssize_t) len);
    }

    z0 write (OutputStream& out)
    {
        out.writeInt64 (hash);
        out.writeInt64 ((z64) data.getSize());
        out << data;
    }

    z64 hash;
    u32 lastUsed;
    MemoryBlock data;

private:
    DRX_LEAK_DETECTOR (ThumbnailCacheEntry)
};

//==============================================================================
AudioThumbnailCache::AudioThumbnailCache (i32k maxNumThumbs)
    : thread (SystemStats::getDRXVersion() + ": thumb cache"),
      maxNumThumbsToStore (maxNumThumbs)
{
    jassert (maxNumThumbsToStore > 0);
    thread.startThread (Thread::Priority::low);
}

AudioThumbnailCache::~AudioThumbnailCache()
{
}

AudioThumbnailCache::ThumbnailCacheEntry* AudioThumbnailCache::findThumbFor (const z64 hash) const
{
    for (i32 i = thumbs.size(); --i >= 0;)
        if (thumbs.getUnchecked (i)->hash == hash)
            return thumbs.getUnchecked (i);

    return nullptr;
}

i32 AudioThumbnailCache::findOldestThumb() const
{
    i32 oldest = 0;
    u32 oldestTime = Time::getMillisecondCounter() + 1;

    for (i32 i = thumbs.size(); --i >= 0;)
    {
        const ThumbnailCacheEntry* const te = thumbs.getUnchecked (i);

        if (te->lastUsed < oldestTime)
        {
            oldest = i;
            oldestTime = te->lastUsed;
        }
    }

    return oldest;
}

b8 AudioThumbnailCache::loadThumb (AudioThumbnailBase& thumb, const z64 hashCode)
{
    const ScopedLock sl (lock);

    if (ThumbnailCacheEntry* te = findThumbFor (hashCode))
    {
        te->lastUsed = Time::getMillisecondCounter();

        MemoryInputStream in (te->data, false);
        thumb.loadFrom (in);
        return true;
    }

    return loadNewThumb (thumb, hashCode);
}

z0 AudioThumbnailCache::storeThumb (const AudioThumbnailBase& thumb,
                                      const z64 hashCode)
{
    const ScopedLock sl (lock);
    ThumbnailCacheEntry* te = findThumbFor (hashCode);

    if (te == nullptr)
    {
        te = new ThumbnailCacheEntry (hashCode);

        if (thumbs.size() < maxNumThumbsToStore)
            thumbs.add (te);
        else
            thumbs.set (findOldestThumb(), te);
    }

    {
        MemoryOutputStream out (te->data, false);
        thumb.saveTo (out);
    }

    saveNewlyFinishedThumbnail (thumb, hashCode);
}

z0 AudioThumbnailCache::clear()
{
    const ScopedLock sl (lock);
    thumbs.clear();
}

z0 AudioThumbnailCache::removeThumb (const z64 hashCode)
{
    const ScopedLock sl (lock);

    for (i32 i = thumbs.size(); --i >= 0;)
        if (thumbs.getUnchecked (i)->hash == hashCode)
            thumbs.remove (i);
}

static i32 getThumbnailCacheFileMagicHeader() noexcept
{
    return (i32) ByteOrder::littleEndianInt ("ThmC");
}

b8 AudioThumbnailCache::readFromStream (InputStream& source)
{
    if (source.readInt() != getThumbnailCacheFileMagicHeader())
        return false;

    const ScopedLock sl (lock);
    clear();
    i32 numThumbnails = jmin (maxNumThumbsToStore, source.readInt());

    while (--numThumbnails >= 0 && ! source.isExhausted())
        thumbs.add (new ThumbnailCacheEntry (source));

    return true;
}

z0 AudioThumbnailCache::writeToStream (OutputStream& out)
{
    const ScopedLock sl (lock);

    out.writeInt (getThumbnailCacheFileMagicHeader());
    out.writeInt (thumbs.size());

    for (i32 i = 0; i < thumbs.size(); ++i)
        thumbs.getUnchecked (i)->write (out);
}

z0 AudioThumbnailCache::saveNewlyFinishedThumbnail (const AudioThumbnailBase&, z64)
{
}

b8 AudioThumbnailCache::loadNewThumb (AudioThumbnailBase&, z64)
{
    return false;
}

} // namespace drx
