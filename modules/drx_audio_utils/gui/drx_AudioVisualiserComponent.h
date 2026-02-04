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
    A simple component that can be used to show a scrolling waveform of audio data.

    This is a handy way to get a quick visualisation of some audio data. Just create
    one of these, set its size and oversampling rate, and then feed it with incoming
    data by calling one of its pushBuffer() or pushSample() methods.

    You can override its paint method for more customised views, but it's only designed
    as a quick-and-dirty class for simple tasks, so please don't send us feature requests
    for fancy additional features that you'd like it to support! If you're building a
    real-world app that requires more powerful waveform display, you'll probably want to
    create your own component instead.

    @tags{Audio}
*/
class DRX_API AudioVisualiserComponent  : public Component,
                                           private Timer
{
public:
    /** Creates a visualiser with the given number of channels. */
    AudioVisualiserComponent (i32 initialNumChannels);

    /** Destructor. */
    ~AudioVisualiserComponent() override;

    /** Changes the number of channels that the visualiser stores. */
    z0 setNumChannels (i32 numChannels);

    /** Changes the number of samples that the visualiser keeps in its history.
        Note that this value refers to the number of averaged sample blocks, and each
        block is calculated as the peak of a number of incoming audio samples. To set
        the number of incoming samples per block, use setSamplesPerBlock().
     */
    z0 setBufferSize (i32 bufferSize);

    /** */
    z0 setSamplesPerBlock (i32 newNumInputSamplesPerBlock) noexcept;

    /** */
    i32 getSamplesPerBlock() const noexcept                         { return inputSamplesPerBlock; }

    /** Clears the contents of the buffers. */
    z0 clear();

    /** Pushes a buffer of channels data.
        The number of channels provided here is expected to match the number of channels
        that this AudioVisualiserComponent has been told to use.
    */
    z0 pushBuffer (const AudioBuffer<f32>& bufferToPush);

    /** Pushes a buffer of channels data.
        The number of channels provided here is expected to match the number of channels
        that this AudioVisualiserComponent has been told to use.
    */
    z0 pushBuffer (const AudioSourceChannelInfo& bufferToPush);

    /** Pushes a buffer of channels data.
        The number of channels provided here is expected to match the number of channels
        that this AudioVisualiserComponent has been told to use.
    */
    z0 pushBuffer (const f32* const* channelData, i32 numChannels, i32 numSamples);

    /** Pushes a single sample (per channel).
        The number of channels provided here is expected to match the number of channels
        that this AudioVisualiserComponent has been told to use.
    */
    z0 pushSample (const f32* samplesForEachChannel, i32 numChannels);

    /** Sets the colours used to paint the */
    z0 setColors (Color backgroundColor, Color waveformColor) noexcept;

    /** Sets the frequency at which the component repaints itself. */
    z0 setRepaintRate (i32 frequencyInHz);

    /** Draws a channel of audio data in the given bounds.
        The default implementation just calls getChannelAsPath() and fits this into the given
        area. You may want to override this to draw things differently.
    */
    virtual z0 paintChannel (Graphics&, Rectangle<f32> bounds,
                               const Range<f32>* levels, i32 numLevels, i32 nextSample);

    /** Creates a path which contains the waveform shape of a given set of range data.
        The path is normalised so that -1 and +1 are its upper and lower bounds, and it
        goes from 0 to numLevels on the X axis.
    */
    z0 getChannelAsPath (Path& result, const Range<f32>* levels, i32 numLevels, i32 nextSample);

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;

private:
    struct ChannelInfo;

    OwnedArray<ChannelInfo> channels;
    i32 numSamples, inputSamplesPerBlock;
    Color backgroundColor, waveformColor;

    z0 timerCallback() override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioVisualiserComponent)
};

} // namespace drx
