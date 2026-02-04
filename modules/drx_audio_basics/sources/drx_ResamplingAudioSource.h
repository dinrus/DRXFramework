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
    A type of AudioSource that takes an input source and changes its sample rate.

    @see AudioSource, LagrangeInterpolator, CatmullRomInterpolator

    @tags{Audio}
*/
class DRX_API  ResamplingAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a ResamplingAudioSource for a given input source.

        @param inputSource              the input source to read from
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
        @param numChannels              the number of channels to process
    */
    ResamplingAudioSource (AudioSource* inputSource,
                           b8 deleteInputWhenDeleted,
                           i32 numChannels = 2);

    /** Destructor. */
    ~ResamplingAudioSource() override;

    /** Changes the resampling ratio.

        (This value can be changed at any time, even while the source is running).

        @param samplesInPerOutputSample     if set to 1.0, the input is passed through; higher
                                            values will speed it up; lower values will slow it
                                            down. The ratio must be greater than 0
    */
    z0 setResamplingRatio (f64 samplesInPerOutputSample);

    /** Returns the current resampling ratio.

        This is the value that was set by setResamplingRatio().
    */
    f64 getResamplingRatio() const noexcept                  { return ratio; }

    /** Clears any buffers and filters that the resampler is using. */
    z0 flushBuffers();

    //==============================================================================
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate) override;
    z0 releaseResources() override;
    z0 getNextAudioBlock (const AudioSourceChannelInfo&) override;

private:
    //==============================================================================
    OptionalScopedPointer<AudioSource> input;
    f64 ratio = 1.0, lastRatio = 1.0;
    AudioBuffer<f32> buffer;
    i32 bufferPos = 0, sampsInBuffer = 0;
    f64 subSampleOffset = 0.0;
    f64 coefficients[6];
    SpinLock ratioLock;
    CriticalSection callbackLock;
    i32k numChannels;
    HeapBlock<f32*> destBuffers;
    HeapBlock<const f32*> srcBuffers;

    z0 setFilterCoefficients (f64 c1, f64 c2, f64 c3, f64 c4, f64 c5, f64 c6);
    z0 createLowPass (f64 proportionalRate);

    struct FilterState
    {
        f64 x1, x2, y1, y2;
    };

    HeapBlock<FilterState> filterStates;
    z0 resetFilters();

    z0 applyFilter (f32* samples, i32 num, FilterState& fs);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResamplingAudioSource)
};

} // namespace drx
