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

struct AudioVisualiserComponent::ChannelInfo
{
    ChannelInfo (AudioVisualiserComponent& o, i32 bufferSize) : owner (o)
    {
        setBufferSize (bufferSize);
        clear();
    }

    z0 clear() noexcept
    {
        levels.fill ({});
        value = {};
        subSample = 0;
    }

    z0 pushSamples (const f32* inputSamples, i32 num) noexcept
    {
        for (i32 i = 0; i < num; ++i)
            pushSample (inputSamples[i]);
    }

    z0 pushSample (f32 newSample) noexcept
    {
        if (--subSample <= 0)
        {
            if (++nextSample == levels.size())
                nextSample = 0;

            levels.getReference (nextSample) = value;
            subSample = owner.getSamplesPerBlock();
            value = Range<f32> (newSample, newSample);
        }
        else
        {
            value = value.getUnionWith (newSample);
        }
    }

    z0 setBufferSize (i32 newSize)
    {
        levels.removeRange (newSize, levels.size());
        levels.insertMultiple (-1, {}, newSize - levels.size());

        if (nextSample >= newSize)
            nextSample = 0;
    }

    AudioVisualiserComponent& owner;
    Array<Range<f32>> levels;
    Range<f32> value;
    std::atomic<i32> nextSample { 0 }, subSample { 0 };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelInfo)
};

//==============================================================================
AudioVisualiserComponent::AudioVisualiserComponent (i32 initialNumChannels)
  : numSamples (1024),
    inputSamplesPerBlock (256),
    backgroundColor (Colors::black),
    waveformColor (Colors::white)
{
    setOpaque (true);
    setNumChannels (initialNumChannels);
    setRepaintRate (60);
}

AudioVisualiserComponent::~AudioVisualiserComponent()
{
}

z0 AudioVisualiserComponent::setNumChannels (i32 numChannels)
{
    channels.clear();

    for (i32 i = 0; i < numChannels; ++i)
        channels.add (new ChannelInfo (*this, numSamples));
}

z0 AudioVisualiserComponent::setBufferSize (i32 newNumSamples)
{
    numSamples = newNumSamples;

    for (auto* c : channels)
        c->setBufferSize (newNumSamples);
}

z0 AudioVisualiserComponent::clear()
{
    for (auto* c : channels)
        c->clear();
}

z0 AudioVisualiserComponent::pushBuffer (const f32* const* d, i32 numChannels, i32 num)
{
    numChannels = jmin (numChannels, channels.size());

    for (i32 i = 0; i < numChannels; ++i)
        channels.getUnchecked (i)->pushSamples (d[i], num);
}

z0 AudioVisualiserComponent::pushBuffer (const AudioBuffer<f32>& buffer)
{
    pushBuffer (buffer.getArrayOfReadPointers(),
                buffer.getNumChannels(),
                buffer.getNumSamples());
}

z0 AudioVisualiserComponent::pushBuffer (const AudioSourceChannelInfo& buffer)
{
    auto numChannels = jmin (buffer.buffer->getNumChannels(), channels.size());

    for (i32 i = 0; i < numChannels; ++i)
        channels.getUnchecked (i)->pushSamples (buffer.buffer->getReadPointer (i, buffer.startSample),
                                               buffer.numSamples);
}

z0 AudioVisualiserComponent::pushSample (const f32* d, i32 numChannels)
{
    numChannels = jmin (numChannels, channels.size());

    for (i32 i = 0; i < numChannels; ++i)
        channels.getUnchecked (i)->pushSample (d[i]);
}

z0 AudioVisualiserComponent::setSamplesPerBlock (i32 newSamplesPerPixel) noexcept
{
    inputSamplesPerBlock = newSamplesPerPixel;
}

z0 AudioVisualiserComponent::setRepaintRate (i32 frequencyInHz)
{
    startTimerHz (frequencyInHz);
}

z0 AudioVisualiserComponent::timerCallback()
{
    repaint();
}

z0 AudioVisualiserComponent::setColors (Color bk, Color fg) noexcept
{
    backgroundColor = bk;
    waveformColor = fg;
    repaint();
}

z0 AudioVisualiserComponent::paint (Graphics& g)
{
    g.fillAll (backgroundColor);

    auto r = getLocalBounds().toFloat();
    auto channelHeight = r.getHeight() / (f32) channels.size();

    g.setColor (waveformColor);

    for (auto* c : channels)
        paintChannel (g, r.removeFromTop (channelHeight),
                      c->levels.begin(), c->levels.size(), c->nextSample);
}

z0 AudioVisualiserComponent::getChannelAsPath (Path& path, const Range<f32>* levels,
                                                 i32 numLevels, i32 nextSample)
{
    path.preallocateSpace (4 * numLevels + 8);

    for (i32 i = 0; i < numLevels; ++i)
    {
        auto level = -(levels[(nextSample + i) % numLevels].getEnd());

        if (i == 0)
            path.startNewSubPath (0.0f, level);
        else
            path.lineTo ((f32) i, level);
    }

    for (i32 i = numLevels; --i >= 0;)
        path.lineTo ((f32) i, -(levels[(nextSample + i) % numLevels].getStart()));

    path.closeSubPath();
}

z0 AudioVisualiserComponent::paintChannel (Graphics& g, Rectangle<f32> area,
                                             const Range<f32>* levels, i32 numLevels, i32 nextSample)
{
    Path p;
    getChannelAsPath (p, levels, numLevels, nextSample);

    g.fillPath (p, AffineTransform::fromTargetPoints (0.0f, -1.0f,               area.getX(), area.getY(),
                                                      0.0f, 1.0f,                area.getX(), area.getBottom(),
                                                      (f32) numLevels, -1.0f,  area.getRight(), area.getY()));
}

} // namespace drx
