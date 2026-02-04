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

namespace drx::dsp
{

//==============================================================================
/**
    A processor that performs multi-channel oversampling.

    This class can be configured to do a factor of 2, 4, 8 or 16 times
    oversampling, using multiple stages, with polyphase allpass IIR filters or FIR
    filters, and latency compensation.

    The principle of oversampling is to increase the sample rate of a given
    non-linear process to prevent it from creating aliasing. Oversampling works
    by upsampling the input signal N times, processing the upsampled signal
    with the increased internal sample rate, then downsampling the result to get
    back to the original sample rate.

    Choose between FIR or IIR filtering depending on your needs in terms of
    latency and phase distortion. With FIR filters the phase is linear but the
    latency is maximised. With IIR filtering the phase is compromised around the
    Nyquist frequency but the latency is minimised.

    @see FilterDesign.

    @tags{DSP}
*/
template <typename SampleType>
class DRX_API  Oversampling
{
public:
    /** The type of filter that can be used for the oversampling processing. */
    enum FilterType
    {
        filterHalfBandFIREquiripple = 0,
        filterHalfBandPolyphaseIIR,
        numFilterTypes
    };

    //==============================================================================
    /** The default constructor.

        Note: This creates a "dummy" oversampling stage, which needs to be removed
        before adding proper oversampling stages.

        @param numChannels    the number of channels to process with this object

        @see clearOversamplingStages, addOversamplingStage
    */
    explicit Oversampling (size_t numChannels = 1);

    /** Constructor.

        @param numChannels          the number of channels to process with this object
        @param factor               the processing will perform 2 ^ factor times oversampling
        @param type                 the type of filter design employed for filtering during
                                    oversampling
        @param isMaxQuality         if the oversampling is done using the maximum quality, where
                                    the filters will be more efficient but the CPU load will
                                    increase as well
        @param useIntegerLatency    if true this processor will add some fractional delay at the
                                    end of the signal path to ensure that the overall latency of
                                    the oversampling is an integer
    */
    Oversampling (size_t numChannels,
                  size_t factor,
                  FilterType type,
                  b8 isMaxQuality = true,
                  b8 useIntegerLatency = false);

    /** Destructor. */
    ~Oversampling();

    //==============================================================================
    /* Sets if this processor should add some fractional delay at the end of the signal
       path to ensure that the overall latency of the oversampling is an integer.
    */
    z0 setUsingIntegerLatency (b8 shouldUseIntegerLatency) noexcept;

    /** Returns the latency in samples of the overall processing. You can use this
        information in your main processor to compensate the additional latency
        involved with the oversampling, for example with a dry / wet mixer, and to
        report the latency to the DAW.

        Note: If you have not opted to use an integer latency then the latency may not be
        integer, so you might need to round its value or to compensate it properly in
        your processing code since plug-ins can only report integer latency values in
        samples to the DAW.
    */
    SampleType getLatencyInSamples() const noexcept;

    /** Returns the current oversampling factor. */
    size_t getOversamplingFactor() const noexcept;

    //==============================================================================
    /** Must be called before any processing, to set the buffer sizes of the internal
        buffers of the oversampling processing.
    */
    z0 initProcessing (size_t maximumNumberOfSamplesBeforeOversampling);

    /** Resets the processing pipeline, ready to oversample a new stream of data. */
    z0 reset() noexcept;

    /** Must be called to perform the upsampling, prior to any oversampled processing.

        Returns an AudioBlock referencing the oversampled input signal, which must be
        used to perform the non-linear processing which needs the higher sample rate.
        Don't forget to set the sample rate of that processing to N times the original
        sample rate.
    */
    AudioBlock<SampleType> processSamplesUp (const AudioBlock<const SampleType>& inputBlock) noexcept;

    /** Must be called to perform the downsampling, after the upsampling and the
        non-linear processing. The output signal is probably delayed by the internal
        latency of the whole oversampling behaviour, so don't forget to take this
        into account.
    */
    z0 processSamplesDown (AudioBlock<SampleType>& outputBlock) noexcept;

    //==============================================================================
    /** Adds a new oversampling stage to the Oversampling class, multiplying the
        current oversampling factor by two. This is used with the default constructor
        to create custom oversampling chains, requiring a call to the
        clearOversamplingStages before any addition.

        Note: Upsampling and downsampling filtering have different purposes, the
        former removes upsampling artefacts while the latter removes useless frequency
        content created by the oversampled process, so usually the attenuation is
        increased when upsampling compared to downsampling.

        @param normalisedTransitionWidthUp     a value between 0 and 0.5 which specifies how much
                                               the transition between passband and stopband is
                                               steep, for upsampling filtering (the lower the better)
        @param stopbandAmplitudedBUp           the amplitude in dB in the stopband for upsampling
                                               filtering, must be negative
        @param normalisedTransitionWidthDown   a value between 0 and 0.5 which specifies how much
                                               the transition between passband and stopband is
                                               steep, for downsampling filtering (the lower the better)
        @param stopbandAmplitudedBDown         the amplitude in dB in the stopband for downsampling
                                               filtering, must be negative

        @see clearOversamplingStages
    */
    z0 addOversamplingStage (FilterType,
                               f32 normalisedTransitionWidthUp,   f32 stopbandAmplitudedBUp,
                               f32 normalisedTransitionWidthDown, f32 stopbandAmplitudedBDown);

    /** Adds a new "dummy" oversampling stage, which does nothing to the signal. Using
        one can be useful if your application features a customisable oversampling factor
        and if you want to select the current one from an OwnedArray without changing
        anything in the processing code.

        @see OwnedArray, clearOversamplingStages, addOversamplingStage
    */
    z0 addDummyOversamplingStage();

    /** Removes all the previously registered oversampling stages, so you can add
        your own from scratch.

        @see addOversamplingStage, addDummyOversamplingStage
    */
    z0 clearOversamplingStages();

    //==============================================================================
    size_t factorOversampling = 1;
    size_t numChannels = 1;

   #ifndef DOXYGEN
    struct OversamplingStage;
   #endif

private:
    //==============================================================================
    z0 updateDelayLine();
    SampleType getUncompensatedLatency() const noexcept;

    //==============================================================================
    OwnedArray<OversamplingStage> stages;
    b8 isReady = false, shouldUseIntegerLatency = false;
    DelayLine<SampleType, DelayLineInterpolationTypes::Thiran> delay { 8 };
    SampleType fractionalDelay = 0;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oversampling)
};

} // namespace drx::dsp
