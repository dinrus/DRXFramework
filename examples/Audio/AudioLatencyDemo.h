/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             AudioLatencyDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Tests the audio latency of a device.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioLatencyDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/AudioLiveScrollingDisplay.h"

//==============================================================================
class LatencyTester final : public AudioIODeviceCallback,
                            private Timer
{
public:
    LatencyTester (TextEditor& editorBox)
        : resultsBox (editorBox)
    {}

    //==============================================================================
    z0 beginTest()
    {
        resultsBox.moveCaretToEnd();
        resultsBox.insertTextAtCaret (newLine + newLine + "Starting test..." + newLine);
        resultsBox.moveCaretToEnd();

        startTimer (50);

        const ScopedLock sl (lock);
        createTestSound();
        recordedSound.clear();
        playingSampleNum = recordedSampleNum = 0;
        testIsRunning = true;
    }

    z0 timerCallback() override
    {
        if (testIsRunning && recordedSampleNum >= recordedSound.getNumSamples())
        {
            testIsRunning = false;
            stopTimer();

            // Test has finished, so calculate the result..
            auto latencySamples = calculateLatencySamples();

            resultsBox.moveCaretToEnd();
            resultsBox.insertTextAtCaret (getMessageDescribingResult (latencySamples));
            resultsBox.moveCaretToEnd();
        }
    }

    Txt getMessageDescribingResult (i32 latencySamples)
    {
        Txt message;

        if (latencySamples >= 0)
        {
            message << newLine
                    << "Results:" << newLine
                    << latencySamples << " samples (" << Txt (latencySamples * 1000.0 / sampleRate, 1)
                    << " milliseconds)" << newLine
                    << "The audio device reports an input latency of "
                    << deviceInputLatency << " samples, output latency of "
                    << deviceOutputLatency << " samples." << newLine
                    << "So the corrected latency = "
                    << (latencySamples - deviceInputLatency - deviceOutputLatency)
                    << " samples (" << Txt ((latencySamples - deviceInputLatency - deviceOutputLatency) * 1000.0 / sampleRate, 2)
                    << " milliseconds)";
        }
        else
        {
            message << newLine
                    << "Couldn't detect the test signal!!" << newLine
                    << "Make sure there's no background noise that might be confusing it..";
        }

        return message;
    }

    //==============================================================================
    z0 audioDeviceAboutToStart (AudioIODevice* device) override
    {
        testIsRunning = false;
        playingSampleNum = recordedSampleNum = 0;

        sampleRate          = device->getCurrentSampleRate();
        deviceInputLatency  = device->getInputLatencyInSamples();
        deviceOutputLatency = device->getOutputLatencyInSamples();

        recordedSound.setSize (1, (i32) (0.9 * sampleRate));
        recordedSound.clear();
    }

    z0 audioDeviceStopped() override {}

    z0 audioDeviceIOCallbackWithContext (const f32* const* inputChannelData, i32 numInputChannels,
                                           f32* const* outputChannelData, i32 numOutputChannels,
                                           i32 numSamples, const AudioIODeviceCallbackContext& context) override
    {
        ignoreUnused (context);

        const ScopedLock sl (lock);

        if (testIsRunning)
        {
            auto* recordingBuffer = recordedSound.getWritePointer (0);
            auto* playBuffer = testSound.getReadPointer (0);

            for (i32 i = 0; i < numSamples; ++i)
            {
                if (recordedSampleNum < recordedSound.getNumSamples())
                {
                    auto inputSamp = 0.0f;

                    for (auto j = numInputChannels; --j >= 0;)
                        if (inputChannelData[j] != nullptr)
                            inputSamp += inputChannelData[j][i];

                    recordingBuffer[recordedSampleNum] = inputSamp;
                }

                ++recordedSampleNum;

                auto outputSamp = (playingSampleNum < testSound.getNumSamples()) ? playBuffer[playingSampleNum] : 0.0f;

                for (auto j = numOutputChannels; --j >= 0;)
                    if (outputChannelData[j] != nullptr)
                        outputChannelData[j][i] = outputSamp;

                ++playingSampleNum;
            }
        }
        else
        {
            // We need to clear the output buffers, in case they're full of junk..
            for (i32 i = 0; i < numOutputChannels; ++i)
                if (outputChannelData[i] != nullptr)
                    zeromem (outputChannelData[i], (size_t) numSamples * sizeof (f32));
        }
    }

private:
    TextEditor& resultsBox;
    AudioBuffer<f32> testSound, recordedSound;
    Array<i32> spikePositions;
    CriticalSection lock;

    i32 playingSampleNum  = 0;
    i32 recordedSampleNum = -1;
    f64 sampleRate     = 0.0;
    b8 testIsRunning    = false;
    i32 deviceInputLatency, deviceOutputLatency;

    //==============================================================================
    // create a test sound which consists of a series of randomly-spaced audio spikes..
    z0 createTestSound()
    {
        auto length = ((i32) sampleRate) / 4;
        testSound.setSize (1, length);
        testSound.clear();

        Random rand;

        for (i32 i = 0; i < length; ++i)
            testSound.setSample (0, i, (rand.nextFloat() - rand.nextFloat() + rand.nextFloat() - rand.nextFloat()) * 0.06f);

        spikePositions.clear();

        i32 spikePos   = 0;
        i32 spikeDelta = 50;

        while (spikePos < length - 1)
        {
            spikePositions.add (spikePos);

            testSound.setSample (0, spikePos,      0.99f);
            testSound.setSample (0, spikePos + 1, -0.99f);

            spikePos += spikeDelta;
            spikeDelta += spikeDelta / 6 + rand.nextInt (5);
        }
    }

    // Searches a buffer for a set of spikes that matches those in the test sound
    i32 findOffsetOfSpikes (const AudioBuffer<f32>& buffer) const
    {
        auto minSpikeLevel = 5.0f;
        auto smooth = 0.975;
        auto* s = buffer.getReadPointer (0);
        i32 spikeDriftAllowed = 5;

        Array<i32> spikesFound;
        spikesFound.ensureStorageAllocated (100);
        auto runningAverage = 0.0;
        i32 lastSpike = 0;

        for (i32 i = 0; i < buffer.getNumSamples() - 10; ++i)
        {
            auto samp = std::abs (s[i]);

            if (samp > runningAverage * minSpikeLevel && i > lastSpike + 20)
            {
                lastSpike = i;
                spikesFound.add (i);
            }

            runningAverage = runningAverage * smooth + (1.0 - smooth) * samp;
        }

        i32 bestMatch = -1;
        auto bestNumMatches = spikePositions.size() / 3; // the minimum number of matches required

        if (spikesFound.size() < bestNumMatches)
            return -1;

        for (i32 offsetToTest = 0; offsetToTest < buffer.getNumSamples() - 2048; ++offsetToTest)
        {
            i32 numMatchesHere = 0;
            i32 foundIndex     = 0;

            for (i32 refIndex = 0; refIndex < spikePositions.size(); ++refIndex)
            {
                auto referenceSpike = spikePositions.getUnchecked (refIndex) + offsetToTest;
                i32 spike = 0;

                while ((spike = spikesFound.getUnchecked (foundIndex)) < referenceSpike - spikeDriftAllowed
                         && foundIndex < spikesFound.size() - 1)
                    ++foundIndex;

                if (spike >= referenceSpike - spikeDriftAllowed && spike <= referenceSpike + spikeDriftAllowed)
                    ++numMatchesHere;
            }

            if (numMatchesHere > bestNumMatches)
            {
                bestNumMatches = numMatchesHere;
                bestMatch = offsetToTest;

                if (numMatchesHere == spikePositions.size())
                    break;
            }
        }

        return bestMatch;
    }

    i32 calculateLatencySamples() const
    {
        // Detect the sound in both our test sound and the recording of it, and measure the difference
        // in their start times..
        auto referenceStart = findOffsetOfSpikes (testSound);
        jassert (referenceStart >= 0);

        auto recordedStart = findOffsetOfSpikes (recordedSound);

        return (recordedStart < 0) ? -1
                                   : (recordedStart - referenceStart);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatencyTester)
};

//==============================================================================
class AudioLatencyDemo final : public Component
{
public:
    AudioLatencyDemo()
    {
        setOpaque (true);

        liveAudioScroller.reset (new LiveScrollingAudioDisplay());
        addAndMakeVisible (liveAudioScroller.get());

        addAndMakeVisible (resultsBox);
        resultsBox.setMultiLine (true);
        resultsBox.setReturnKeyStartsNewLine (true);
        resultsBox.setReadOnly (true);
        resultsBox.setScrollbarsShown (true);
        resultsBox.setCaretVisible (false);
        resultsBox.setPopupMenuEnabled (true);

        resultsBox.setColor (TextEditor::outlineColorId, Color (0x1c000000));
        resultsBox.setColor (TextEditor::shadowColorId,  Color (0x16000000));

        resultsBox.setText ("Running this test measures the round-trip latency between the audio output and input "
                            "devices you\'ve got selected.\n\n"
                            "It\'ll play a sound, then try to measure the time at which the sound arrives "
                            "back at the audio input. Obviously for this to work you need to have your "
                            "microphone somewhere near your speakers...");

        addAndMakeVisible (startTestButton);
        startTestButton.onClick = [this] { startTest(); };

       #ifndef DRX_DEMO_RUNNER
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (b8 granted)
                                     {
                                         i32 numInputChannels = granted ? 2 : 0;
                                         audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                     });
       #endif

        audioDeviceManager.addAudioCallback (liveAudioScroller.get());

        setSize (500, 500);
    }

    ~AudioLatencyDemo() override
    {
        audioDeviceManager.removeAudioCallback (liveAudioScroller.get());
        audioDeviceManager.removeAudioCallback (latencyTester    .get());
        latencyTester    .reset();
        liveAudioScroller.reset();
    }

    z0 startTest()
    {
        if (latencyTester.get() == nullptr)
        {
            latencyTester.reset (new LatencyTester (resultsBox));
            audioDeviceManager.addAudioCallback (latencyTester.get());
        }

        latencyTester->beginTest();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (ResizableWindow::backgroundColorId));
    }

    z0 resized() override
    {
        auto b = getLocalBounds().reduced (5);

        if (liveAudioScroller.get() != nullptr)
        {
            liveAudioScroller->setBounds (b.removeFromTop (b.getHeight() / 5));
            b.removeFromTop (10);
        }

        startTestButton.setBounds (b.removeFromBottom (b.getHeight() / 10));
        b.removeFromBottom (10);

        resultsBox.setBounds (b);
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef DRX_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (1, 2) };
   #endif

    std::unique_ptr<LatencyTester> latencyTester;
    std::unique_ptr<LiveScrollingAudioDisplay> liveAudioScroller;

    TextButton startTestButton  { "Test Latency" };
    TextEditor resultsBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioLatencyDemo)
};
