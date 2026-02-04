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

class AudioThumbnailCache;

//==============================================================================
/**
    Provides a base for classes that can store and draw scaled views of an
    audio waveform.

    Typically, you'll want to use the derived class AudioThumbnail, which provides
    a concrete implementation.

    @see AudioThumbnail, AudioThumbnailCache

    @tags{Audio}
*/
class DRX_API  AudioThumbnailBase    : public ChangeBroadcaster,
                                        public AudioFormatWriter::ThreadedWriter::IncomingDataReceiver
{
public:
    //==============================================================================
    AudioThumbnailBase() = default;
    ~AudioThumbnailBase() override = default;

    //==============================================================================
    /** Clears and resets the thumbnail. */
    virtual z0 clear() = 0;

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
    virtual b8 setSource (InputSource* newSource) = 0;

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
    virtual z0 setReader (AudioFormatReader* newReader, z64 hashCode) = 0;

    //==============================================================================
    /** Reloads the low res thumbnail data from an input stream.

        This is not an audio file stream! It takes a stream of thumbnail data that would
        previously have been created by the saveTo() method.
        @see saveTo
    */
    virtual b8 loadFrom (InputStream& input) = 0;

    /** Saves the low res thumbnail data to an output stream.

        The data that is written can later be reloaded using loadFrom().
        @see loadFrom
    */
    virtual z0 saveTo (OutputStream& output) const = 0;

    //==============================================================================
    /** Returns the number of channels in the file. */
    virtual i32 getNumChannels() const noexcept = 0;

    /** Returns the length of the audio file, in seconds. */
    virtual f64 getTotalLength() const noexcept = 0;

    /** Draws the waveform for a channel.

        The waveform will be drawn within  the specified rectangle, where startTime
        and endTime specify the times within the audio file that should be positioned
        at the left and right edges of the rectangle.

        The waveform will be scaled vertically so that a full-volume sample will fill
        the rectangle vertically, but you can also specify an extra vertical scale factor
        with the verticalZoomFactor parameter.
    */
    virtual z0 drawChannel (Graphics& g,
                              const Rectangle<i32>& area,
                              f64 startTimeSeconds,
                              f64 endTimeSeconds,
                              i32 channelNum,
                              f32 verticalZoomFactor) = 0;

    /** Draws the waveforms for all channels in the thumbnail.

        This will call drawChannel() to render each of the thumbnail's channels, stacked
        above each other within the specified area.

        @see drawChannel
    */
    virtual z0 drawChannels (Graphics& g,
                               const Rectangle<i32>& area,
                               f64 startTimeSeconds,
                               f64 endTimeSeconds,
                               f32 verticalZoomFactor) = 0;

    /** Возвращает true, если the low res preview is fully generated. */
    virtual b8 isFullyLoaded() const noexcept = 0;

    /** Returns the number of samples that have been set in the thumbnail. */
    virtual z64 getNumSamplesFinished() const noexcept = 0;

    /** Returns the highest level in the thumbnail.
        Note that because the thumb only stores low-resolution data, this isn't
        an accurate representation of the highest value, it's only a rough approximation.
    */
    virtual f32 getApproximatePeak() const = 0;

    /** Reads the approximate min and max levels from a section of the thumbnail.
        The lowest and highest samples are returned in minValue and maxValue, but obviously
        because the thumb only stores low-resolution data, these numbers will only be a rough
        approximation of the true values.
    */
    virtual z0 getApproximateMinMax (f64 startTime, f64 endTime, i32 channelIndex,
                                       f32& minValue, f32& maxValue) const noexcept = 0;

    /** Returns the hash code that was set by setSource() or setReader(). */
    virtual z64 getHashCode() const = 0;
};

} // namespace drx
