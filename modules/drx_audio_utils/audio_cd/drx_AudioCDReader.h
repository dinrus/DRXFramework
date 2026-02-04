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

#if DRX_USE_CDREADER || DOXYGEN

//==============================================================================
/**
    A type of AudioFormatReader that reads from an audio CD.

    One of these can be used to read a CD as if it's one big audio stream. Use the
    getPositionOfTrackStart() method to find where the individual tracks are
    within the stream.

    @see AudioFormatReader

    @tags{Audio}
*/
class DRX_API  AudioCDReader  : public AudioFormatReader
{
public:
    //==============================================================================
    /** Returns a list of names of Audio CDs currently available for reading.

        If there's a CD drive but no CD in it, this might return an empty list, or
        possibly a device that can be opened but which has no tracks, depending
        on the platform.

        @see createReaderForCD
    */
    static StringArray getAvailableCDNames();

    /** Tries to create an AudioFormatReader that can read from an Audio CD.

        @param index    the index of one of the available CDs - use getAvailableCDNames()
                        to find out how many there are.
        @returns        a new AudioCDReader object, or nullptr if it couldn't be created. The
                        caller will be responsible for deleting the object returned.
    */
    static AudioCDReader* createReaderForCD (i32 index);

    //==============================================================================
    /** Destructor. */
    ~AudioCDReader() override;

    /** Implementation of the AudioFormatReader method. */
    b8 readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                      z64 startSampleInFile, i32 numSamples) override;

    /** Checks whether the CD has been removed from the drive. */
    b8 isCDStillPresent() const;

    /** Returns the total number of tracks (audio + data). */
    i32 getNumTracks() const;

    /** Finds the sample offset of the start of a track.
        @param trackNum     the track number, where trackNum = 0 is the first track
                            and trackNum = getNumTracks() means the end of the CD.
    */
    i32 getPositionOfTrackStart (i32 trackNum) const;

    /** Возвращает true, если a given track is an audio track.
        @param trackNum     the track number, where 0 is the first track.
    */
    b8 isTrackAudio (i32 trackNum) const;

    /** Returns an array of sample offsets for the start of each track, followed by
        the sample position of the end of the CD.
    */
    const Array<i32>& getTrackOffsets() const;

    /** Refreshes the object's table of contents.

        If the disc has been ejected and a different one put in since this
        object was created, this will cause it to update its idea of how many tracks
        there are, etc.
    */
    z0 refreshTrackLengths();

    /** Enables scanning for indexes within tracks.
        @see getLastIndex
    */
    z0 enableIndexScanning (b8 enabled);

    /** Returns the index number found during the last read() call.

        Index scanning is turned off by default - turn it on with enableIndexScanning().

        Then when the read() method is called, if it comes across an index within that
        block, the index number is stored and returned by this method.

        Some devices might not support indexes, of course.

        (If you don't know what CD indexes are, it's unlikely you'll ever need them).

        @see enableIndexScanning
    */
    i32 getLastIndex() const;

    /** Scans a track to find the position of any indexes within it.
        @param trackNumber  the track to look in, where 0 is the first track on the disc
        @returns    an array of sample positions of any index points found (not including
                    the index that marks the start of the track)
    */
    Array<i32> findIndexesInTrack (i32 trackNumber);

    /** Returns the CDDB id number for the CD.
        It's not a great way of identifying a disc, but it's traditional.
    */
    i32 getCDDBId();

    /** Tries to eject the disk.
        Ejecting the disk might not actually be possible, e.g. if some other process is using it.
    */
    z0 ejectDisk();

    //==============================================================================
    enum
    {
        framesPerSecond = 75,
        samplesPerFrame = 44100 / framesPerSecond
    };

private:
    //==============================================================================
    Array<i32> trackStartSamples;

   #if DRX_MAC
    File volumeDir;
    Array<File> tracks;
    i32 currentReaderTrack;
    std::unique_ptr<AudioFormatReader> reader;
    AudioCDReader (const File& volume);

   #elif DRX_WINDOWS
    b8 audioTracks [100];
    uk handle;
    MemoryBlock buffer;
    b8 indexingEnabled;
    i32 lastIndex, firstFrameInBuffer, samplesInBuffer;
    AudioCDReader (uk handle);
    i32 getIndexAt (i32 samplePos);

   #elif DRX_LINUX || DRX_BSD
    AudioCDReader();
   #endif

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioCDReader)
};

#endif

} // namespace drx
