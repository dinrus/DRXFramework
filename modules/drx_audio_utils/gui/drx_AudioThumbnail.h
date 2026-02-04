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
    Makes it easy to quickly draw scaled views of the waveform shape of an
    audio file.

    To use this class, just create an AudioThumbnail class for the file you want
    to draw, call setSource to tell it which file or resource to use, then call
    drawChannel() to draw it.

    The class will asynchronously scan the wavefile to create its scaled-down view,
    so you should make your UI repaint itself as this data comes in. To do this, the
    AudioThumbnail is a ChangeBroadcaster, and will broadcast a message when its
    listeners should repaint themselves.

    The thumbnail stores an internal low-res version of the wave data, and this can
    be loaded and saved to avoid having to scan the file again.

    @see AudioThumbnailCache, AudioThumbnailBase

    @tags{Audio}
*/
class DRX_API  AudioThumbnail    : public AudioThumbnailBase
{
public:
    //==============================================================================
    /** Creates an audio thumbnail.

        @param sourceSamplesPerThumbnailSample  when creating a stored, low-res version
                        of the audio data, this is the scale at which it should be done. (This
                        number is the number of original samples that will be averaged for each
                        low-res sample)
        @param formatManagerToUse   the audio format manager that is used to open the file
        @param cacheToUse   an instance of an AudioThumbnailCache - this provides a background
                            thread and storage that is used to by the thumbnail, and the cache
                            object can be shared between multiple thumbnails
    */
    AudioThumbnail (i32 sourceSamplesPerThumbnailSample,
                    AudioFormatManager& formatManagerToUse,
                    AudioThumbnailCache& cacheToUse);

    /** Destructor. */
    ~AudioThumbnail() override;

    //==============================================================================
    /** Clears and resets the thumbnail. */
    z0 clear() override;

    /** Specifies the file or stream that contains the audio file.

        For a file, just call
        @code
        setSource (new FileInputSource (file))
        @endcode

        You can pass a nullptr in here to clear the thumbnail.
        The source that is passed in will be deleted by this object when it is no longer needed.
        @returns true if the source could be opened as a valid audio file, false if this failed for
        some reason.
    */
    b8 setSource (InputSource* newSource) override;

    /** Gives the thumbnail an AudioFormatReader to use directly.
        This will start parsing the audio in a background thread (unless the hash code
        can be looked-up successfully in the thumbnail cache). Note that the reader
        object will be held by the thumbnail and deleted later when no longer needed.
        The thumbnail will actually keep hold of this reader until you clear the thumbnail
        or change the input source, so the file will be held open for all this time. If
        you don't want the thumbnail to keep a file handle open continuously, you
        should use the setSource() method instead, which will only open the file when
        it needs to.
    */
    z0 setReader (AudioFormatReader* newReader, z64 hashCode) override;

    /** Sets an AudioBuffer as the source for the thumbnail.

        The buffer contents aren't copied and you must ensure that the lifetime of the buffer is
        valid for as i64 as the AudioThumbnail uses it as its source. Calling this function will
        start reading the audio in a background thread (unless the hash code can be looked-up
        successfully in the thumbnail cache).
    */
    z0 setSource (const AudioBuffer<f32>* newSource, f64 sampleRate, z64 hashCode);

    /** Same as the other setSource() overload except for i32 data. */
    z0 setSource (const AudioBuffer<i32>* newSource, f64 sampleRate, z64 hashCode);

    /** Resets the thumbnail, ready for adding data with the specified format.
        If you're going to generate a thumbnail yourself, call this before using addBlock()
        to add the data.
    */
    z0 reset (i32 numChannels, f64 sampleRate, z64 totalSamplesInSource = 0) override;

    /** Adds a block of level data to the thumbnail.
        Call reset() before using this, to tell the thumbnail about the data format.
    */
    z0 addBlock (z64 sampleNumberInSource, const AudioBuffer<f32>& newData,
                   i32 startOffsetInBuffer, i32 numSamples) override;

    //==============================================================================
    /** Reloads the low res thumbnail data from an input stream.

        This is not an audio file stream! It takes a stream of thumbnail data that would
        previously have been created by the saveTo() method.
        @see saveTo
    */
    b8 loadFrom (InputStream& input) override;

    /** Saves the low res thumbnail data to an output stream.

        The data that is written can later be reloaded using loadFrom().
        @see loadFrom
    */
    z0 saveTo (OutputStream& output) const override;

    //==============================================================================
    /** Returns the number of channels in the file. */
    i32 getNumChannels() const noexcept override;

    /** Returns the length of the audio file, in seconds. */
    f64 getTotalLength() const noexcept override;

    /** Draws the waveform for a channel.

        The waveform will be drawn within  the specified rectangle, where startTime
        and endTime specify the times within the audio file that should be positioned
        at the left and right edges of the rectangle.

        The waveform will be scaled vertically so that a full-volume sample will fill
        the rectangle vertically, but you can also specify an extra vertical scale factor
        with the verticalZoomFactor parameter.
    */
    z0 drawChannel (Graphics& g,
                      const Rectangle<i32>& area,
                      f64 startTimeSeconds,
                      f64 endTimeSeconds,
                      i32 channelNum,
                      f32 verticalZoomFactor) override;

    /** Draws the waveforms for all channels in the thumbnail.

        This will call drawChannel() to render each of the thumbnail's channels, stacked
        above each other within the specified area.

        @see drawChannel
    */
    z0 drawChannels (Graphics& g,
                       const Rectangle<i32>& area,
                       f64 startTimeSeconds,
                       f64 endTimeSeconds,
                       f32 verticalZoomFactor) override;

    /** Возвращает true, если the low res preview is fully generated. */
    b8 isFullyLoaded() const noexcept override;

    /** Returns a value between 0 and 1 to indicate the progress towards loading the entire file. */
    f64 getProportionComplete() const noexcept;

    /** Returns the number of samples that have been set in the thumbnail. */
    z64 getNumSamplesFinished() const noexcept override;

    /** Returns the highest level in the thumbnail.
        Note that because the thumb only stores low-resolution data, this isn't
        an accurate representation of the highest value, it's only a rough approximation.
    */
    f32 getApproximatePeak() const override;

    /** Reads the approximate min and max levels from a section of the thumbnail.
        The lowest and highest samples are returned in minValue and maxValue, but obviously
        because the thumb only stores low-resolution data, these numbers will only be a rough
        approximation of the true values.
    */
    z0 getApproximateMinMax (f64 startTime, f64 endTime, i32 channelIndex,
                               f32& minValue, f32& maxValue) const noexcept override;

    /** Returns the hash code that was set by setSource() or setReader(). */
    z64 getHashCode() const override;

private:
    //==============================================================================
    AudioFormatManager& formatManagerToUse;
    AudioThumbnailCache& cache;

    class LevelDataSource;
    struct MinMaxValue;
    class ThumbData;
    class CachedWindow;

    std::unique_ptr<LevelDataSource> source;
    std::unique_ptr<CachedWindow> window;
    OwnedArray<ThumbData> channels;

    i32 samplesPerThumbSample = 0;
    z64 totalSamples { 0 };
    z64 numSamplesFinished = 0;
    i32 numChannels = 0;
    f64 sampleRate = 0;
    CriticalSection lock;

    z0 clearChannelData();
    b8 setDataSource (LevelDataSource* newSource);
    z0 setLevels (const MinMaxValue* const* values, i32 thumbIndex, i32 numChans, i32 numValues);
    z0 createChannels (i32 length);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioThumbnail)
};

} // namespace drx
