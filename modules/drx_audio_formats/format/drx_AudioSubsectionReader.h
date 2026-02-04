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
    This class is used to wrap an AudioFormatReader and only read from a
    subsection of the file.

    So if you have a reader which can read a 1000 sample file, you could wrap it
    in one of these to only access, e.g. samples 100 to 200, and any samples
    outside that will come back as 0. Accessing sample 0 from this reader will
    actually read the first sample from the other's subsection, which might
    be at a non-zero position.

    @see AudioFormatReader

    @tags{Audio}
*/
class DRX_API  AudioSubsectionReader  : public AudioFormatReader
{
public:
    //==============================================================================
    /** Creates an AudioSubsectionReader for a given data source.

        @param sourceReader             the source reader from which we'll be taking data
        @param subsectionStartSample    the sample within the source reader which will be
                                        mapped onto sample 0 for this reader.
        @param subsectionLength         the number of samples from the source that will
                                        make up the subsection. If this reader is asked for
                                        any samples beyond this region, it will return zero.
        @param deleteSourceWhenDeleted  if true, the sourceReader object will be deleted when
                                        this object is deleted.
    */
    AudioSubsectionReader (AudioFormatReader* sourceReader,
                           z64 subsectionStartSample,
                           z64 subsectionLength,
                           b8 deleteSourceWhenDeleted);

    /** Destructor. */
    ~AudioSubsectionReader() override;


    //==============================================================================
    b8 readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                      z64 startSampleInFile, i32 numSamples) override;

    z0 readMaxLevels (z64 startSample, z64 numSamples,
                        Range<f32>* results, i32 numChannelsToRead) override;

    using AudioFormatReader::readMaxLevels;

private:
    //==============================================================================
    AudioFormatReader* const source;
    z64 startSample, length;
    const b8 deleteSourceWhenDeleted;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSubsectionReader)
};

} // namespace drx
