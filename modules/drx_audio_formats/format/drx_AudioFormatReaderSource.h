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
    A type of AudioSource that will read from an AudioFormatReader.

    @see PositionableAudioSource, AudioTransportSource, BufferingAudioSource

    @tags{Audio}
*/
class DRX_API  AudioFormatReaderSource  : public PositionableAudioSource
{
public:
    //==============================================================================
    /** Creates an AudioFormatReaderSource for a given reader.

        @param sourceReader                     the reader to use as the data source - this must
                                                not be null
        @param deleteReaderWhenThisIsDeleted    if true, the reader passed-in will be deleted
                                                when this object is deleted; if false it will be
                                                left up to the caller to manage its lifetime
    */
    AudioFormatReaderSource (AudioFormatReader* sourceReader,
                             b8 deleteReaderWhenThisIsDeleted);

    /** Destructor. */
    ~AudioFormatReaderSource() override;

    //==============================================================================
    /** Toggles loop-mode.

        If set to true, it will continuously loop the input source. If false,
        it will just emit silence after the source has finished.

        @see isLooping
    */
    z0 setLooping (b8 shouldLoop) override;

    /** Returns whether loop-mode is turned on or not. */
    b8 isLooping() const override                             { return looping; }

    /** Returns the reader that's being used. */
    AudioFormatReader* getAudioFormatReader() const noexcept    { return reader; }

    //==============================================================================
    /** Implementation of the AudioSource method. */
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate) override;

    /** Implementation of the AudioSource method. */
    z0 releaseResources() override;

    /** Implementation of the AudioSource method. */
    z0 getNextAudioBlock (const AudioSourceChannelInfo&) override;

    //==============================================================================
    /** Implements the PositionableAudioSource method. */
    z0 setNextReadPosition (z64 newPosition) override;

    /** Implements the PositionableAudioSource method. */
    z64 getNextReadPosition() const override;

    /** Implements the PositionableAudioSource method. */
    z64 getTotalLength() const override;

private:
    //==============================================================================
    OptionalScopedPointer<AudioFormatReader> reader;

    z64 nextPlayPos;
    b8 looping;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFormatReaderSource)
};

} // namespace drx
