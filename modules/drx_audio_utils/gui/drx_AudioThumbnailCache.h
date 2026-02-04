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

//==============================================================================
/**
    An instance of this class is used to manage multiple AudioThumbnail objects.

    The cache runs a single background thread that is shared by all the thumbnails
    that need it, and it maintains a set of low-res previews in memory, to avoid
    having to re-scan audio files too often.

    @see AudioThumbnail

    @tags{Audio}
*/
class DRX_API  AudioThumbnailCache
{
public:
    //==============================================================================
    /** Creates a cache object.

        The maxNumThumbsToStore parameter lets you specify how many previews should
        be kept in memory at once.
    */
    explicit AudioThumbnailCache (i32 maxNumThumbsToStore);

    /** Destructor. */
    virtual ~AudioThumbnailCache();

    //==============================================================================
    /** Clears out any stored thumbnails. */
    z0 clear();

    /** Reloads the specified thumb if this cache contains the appropriate stored
        data.

        This is called automatically by the AudioThumbnail class, so you shouldn't
        normally need to call it directly.
    */
    b8 loadThumb (AudioThumbnailBase& thumb, z64 hashCode);

    /** Stores the cacheable data from the specified thumb in this cache.

        This is called automatically by the AudioThumbnail class, so you shouldn't
        normally need to call it directly.
    */
    z0 storeThumb (const AudioThumbnailBase& thumb, z64 hashCode);

    /** Tells the cache to forget about the thumb with the given hashcode. */
    z0 removeThumb (z64 hashCode);

    //==============================================================================
    /** Attempts to re-load a saved cache of thumbnails from a stream.
        The cache data must have been written by the writeToStream() method.
        This will replace all currently-loaded thumbnails with the new data.
    */
    b8 readFromStream (InputStream& source);

    /** Writes all currently-loaded cache data to a stream.
        The resulting data can be re-loaded with readFromStream().
    */
    z0 writeToStream (OutputStream& stream);

    /** Returns the thread that client thumbnails can use. */
    TimeSliceThread& getTimeSliceThread() noexcept      { return thread; }

protected:
    /** This can be overridden to provide a custom callback for saving thumbnails
        once they have finished being loaded.
    */
    virtual z0 saveNewlyFinishedThumbnail (const AudioThumbnailBase&, z64 hashCode);

    /** This can be overridden to provide a custom callback for loading thumbnails
        from pre-saved files to save the cache the trouble of having to create them.
    */
    virtual b8 loadNewThumb (AudioThumbnailBase&, z64 hashCode);

private:
    //==============================================================================
    TimeSliceThread thread;

    class ThumbnailCacheEntry;
    OwnedArray<ThumbnailCacheEntry> thumbs;
    CriticalSection lock;
    i32 maxNumThumbsToStore;

    ThumbnailCacheEntry* findThumbFor (z64 hash) const;
    i32 findOldestThumb() const;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioThumbnailCache)
};

} // namespace drx
