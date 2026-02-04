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
    An AudioIODeviceCallback object which streams audio through an AudioProcessor.

    To use one of these, just make it the callback used by your AudioIODevice, and
    give it a processor to use by calling setProcessor().

    It's also a MidiInputCallback, so you can connect it to both an audio and midi
    input to send both streams through the processor. To set a MidiOutput for the processor,
    use the setMidiOutput() method.

    @see AudioProcessor, AudioProcessorGraph

    @tags{Audio}
*/
class DRX_API  AudioProcessorPlayer    : public AudioIODeviceCallback,
                                          public MidiInputCallback
{
public:
    //==============================================================================
    AudioProcessorPlayer (b8 doDoublePrecisionProcessing = false);

    /** Destructor. */
    ~AudioProcessorPlayer() override;

    //==============================================================================
    /** Sets the processor that should be played.

        The processor that is passed in will not be deleted or owned by this object.
        To stop anything playing, pass a nullptr to this method.
    */
    z0 setProcessor (AudioProcessor* processorToPlay);

    /** Returns the current audio processor that is being played. */
    AudioProcessor* getCurrentProcessor() const noexcept            { return processor; }

    /** Returns a midi message collector that you can pass midi messages to if you
        want them to be injected into the midi stream that is being sent to the
        processor.
    */
    MidiMessageCollector& getMidiMessageCollector() noexcept        { return messageCollector; }

    /** Sets the MIDI output that should be used, if required.

        The MIDI output will not be deleted or owned by this object. If the MIDI output is
        deleted, pass a nullptr to this method.
    */
    z0 setMidiOutput (MidiOutput* midiOutputToUse);

    /** Switch between f64 and single floating point precisions processing.

        The audio IO callbacks will still operate in single floating point precision,
        however, all internal processing including the AudioProcessor will be processed in
        f64 floating point precision if the AudioProcessor supports it (see
        AudioProcessor::supportsDoublePrecisionProcessing()). Otherwise, the processing will
        remain single precision irrespective of the parameter doublePrecision.
    */
    z0 setDoublePrecisionProcessing (b8 doublePrecision);

    /** Возвращает true, если this player processes internally processes the samples with
        f64 floating point precision.
    */
    inline b8 getDoublePrecisionProcessing() { return isDoublePrecision; }

    //==============================================================================
    /** @internal */
    z0 audioDeviceIOCallbackWithContext (const f32* const*, i32, f32* const*, i32, i32, const AudioIODeviceCallbackContext&) override;
    /** @internal */
    z0 audioDeviceAboutToStart (AudioIODevice*) override;
    /** @internal */
    z0 audioDeviceStopped() override;
    /** @internal */
    z0 handleIncomingMidiMessage (MidiInput*, const MidiMessage&) override;

private:
    struct NumChannels
    {
        NumChannels() = default;
        NumChannels (i32 numIns, i32 numOuts) : ins (numIns), outs (numOuts) {}

        explicit NumChannels (const AudioProcessor::BusesLayout& layout)
            : ins (layout.getNumChannels (true, 0)), outs (layout.getNumChannels (false, 0)) {}

        AudioProcessor::BusesLayout toLayout() const
        {
            return { { AudioChannelSet::canonicalChannelSet (ins) },
                     { AudioChannelSet::canonicalChannelSet (outs) } };
        }

        i32 ins = 0, outs = 0;
    };

    //==============================================================================
    NumChannels findMostSuitableLayout (const AudioProcessor&) const;
    z0 resizeChannels();

    //==============================================================================
    AudioProcessor* processor = nullptr;
    CriticalSection lock;
    f64 sampleRate = 0;
    i32 blockSize = 0;
    b8 isPrepared = false, isDoublePrecision = false;

    NumChannels deviceChannels, defaultProcessorChannels, actualProcessorChannels;
    std::vector<f32*> channels;
    AudioBuffer<f32> tempBuffer;
    AudioBuffer<f64> conversionBuffer;

    MidiBuffer incomingMidi;
    MidiMessageCollector messageCollector;
    MidiOutput* midiOutput = nullptr;
    zu64 sampleCount = 0;

    AudioIODevice* currentDevice = nullptr;
    AudioWorkgroup currentWorkgroup;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorPlayer)
};

} // namespace drx
