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
    An AudioFormatReader that uses a background thread to pre-read data from
    another reader.

    @see AudioFormatReader

    @tags{Audio}
*/
class DRX_API  BufferingAudioReader  : public AudioFormatReader,
                                        private TimeSliceClient
{
public:
    /** Creates a reader.

        @param sourceReader     the source reader to wrap. This BufferingAudioReader
                                takes ownership of this object and will delete it later
                                when no longer needed
        @param timeSliceThread  the thread that should be used to do the background reading.
                                Make sure that the thread you supply is running, and won't
                                be deleted while the reader object still exists.
        @param samplesToBuffer  the total number of samples to buffer ahead.
    */
    BufferingAudioReader (AudioFormatReader* sourceReader,
                          TimeSliceThread& timeSliceThread,
                          i32 samplesToBuffer);

    ~BufferingAudioReader() override;

    /** Sets a number of milliseconds that the reader can block for in its readSamples()
        method before giving up and returning silence.

        A value of less that 0 means "wait forever". The default timeout is 0.
    */
    z0 setReadTimeout (i32 timeoutMilliseconds) noexcept;

    //==============================================================================
    b8 readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                      z64 startSampleInFile, i32 numSamples) override;

private:
    struct BufferedBlock
    {
        BufferedBlock (AudioFormatReader& reader, z64 pos, i32 numSamples);

        Range<z64> range;
        AudioBuffer<f32> buffer;
        b8 allSamplesRead = false;
    };

    i32 useTimeSlice() override;
    BufferedBlock* getBlockContaining (z64 pos) const noexcept;
    b8 readNextBufferChunk();

    static constexpr i32 samplesPerBlock = 32768;

    std::unique_ptr<AudioFormatReader> source;
    TimeSliceThread& thread;
    std::atomic<z64> nextReadPosition { 0 };
    i32k numBlocks;
    i32 timeoutMs = 0;

    CriticalSection lock;
    OwnedArray<BufferedBlock> blocks;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferingAudioReader)
};

} // namespace drx
