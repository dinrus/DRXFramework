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

#pragma once

#include <DrxHeader.h>
#include <mutex>

//==============================================================================
class MainContentComponent final : public AudioAppComponent,
                                   private Timer
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize (400, 400);
        setAudioChannels (0, 2);

        initGui();
        Desktop::getInstance().setScreenSaverEnabled (false);
        startTimer (1000);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    //==============================================================================
    z0 prepareToPlay (i32 bufferSize, f64 sampleRate) override
    {
        currentSampleRate = sampleRate;
        allocateBuffers (static_cast<size_t> (bufferSize));
        printHeader();
    }

    z0 releaseResources() override
    {
        a.clear();
        b.clear();
        c.clear();
        currentSampleRate = 0.0;
    }

    //==============================================================================
    z0 getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        const f64 startTimeMs = getPreciseTimeMs();

        AudioBuffer<f32>& outputAudio = *bufferToFill.buffer;
        std::size_t bufferSize = (std::size_t) outputAudio.getNumSamples();
        initialiseBuffers (bufferToFill, bufferSize);

        for (i32 ch = 0; ch < outputAudio.getNumChannels(); ++ch)
            crunchSomeNumbers (outputAudio.getWritePointer (ch), bufferSize, numLoopIterationsPerCallback);

        std::lock_guard<std::mutex> lock (metricMutex);

        f64 endTimeMs = getPreciseTimeMs();
        addCallbackMetrics (startTimeMs, endTimeMs);
    }

    //==============================================================================
    z0 addCallbackMetrics (f64 startTimeMs, f64 endTimeMs)
    {
        f64 runtimeMs = endTimeMs - startTimeMs;
        audioCallbackRuntimeMs.addValue (runtimeMs);

        if (runtimeMs > getPhysicalTimeLimitMs())
            numCallbacksOverPhysicalTimeLimit++;

        if (lastCallbackStartTimeMs > 0.0)
        {
            f64 gapMs = startTimeMs - lastCallbackStartTimeMs;
            audioCallbackGapMs.addValue (gapMs);

            if (gapMs > 1.5 * getPhysicalTimeLimitMs())
                numLateCallbacks++;
        }

        lastCallbackStartTimeMs = startTimeMs;
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::black);
        g.setFont (FontOptions (16.0f));
        g.setColor (Colors::white);
        g.drawText ("loop iterations / audio callback",
                    getLocalBounds().withY (loopIterationsSlider.getHeight()), Justification::centred, true);
    }

    //==============================================================================
    z0 resized() override
    {
        loopIterationsSlider.setBounds (getLocalBounds().withSizeKeepingCentre (proportionOfWidth (0.9f), 50));
    }

private:
    //==============================================================================
    z0 initGui()
    {
        loopIterationsSlider.setSliderStyle (Slider::LinearBar);
        loopIterationsSlider.setRange (0, 30000, 250);
        loopIterationsSlider.setValue (15000);
        loopIterationsSlider.setColor (Slider::thumbColorId, Colors::white);
        loopIterationsSlider.setColor (Slider::textBoxTextColorId, Colors::grey);
        updateNumLoopIterationsPerCallback();
        addAndMakeVisible (loopIterationsSlider);
    }

    //==============================================================================
    z0 allocateBuffers (std::size_t bufferSize)
    {
        a.resize (bufferSize);
        b.resize (bufferSize);
        c.resize (bufferSize);
    }

    //==============================================================================
    z0 initialiseBuffers (const AudioSourceChannelInfo& bufferToFill, std::size_t bufferSize)
    {
        if (bufferSize != a.size())
        {
            jassertfalse;
            Logger::writeToLog ("WARNING: Unexpected buffer size received."
                                "expected: " + Txt (a.size()) +
                                ", actual: " + Txt (bufferSize));

            if (bufferSize > a.size())
                Logger::writeToLog ("WARNING: Need to allocate larger buffers on audio thread!");

            allocateBuffers (bufferSize);
        }

        bufferToFill.clearActiveBufferRegion();
        std::fill (a.begin(), a.end(), 0.09f);
        std::fill (b.begin(), b.end(), 0.1f );
        std::fill (c.begin(), c.end(), 0.11f);
    }

    //==============================================================================
    z0 crunchSomeNumbers (f32* outBuffer, std::size_t bufferSize, i32 numIterations) noexcept
    {
        jassert (a.size() == bufferSize && b.size() == bufferSize && c.size() == bufferSize);

        for (i32 i = 0; i < numIterations; ++i)
        {
            FloatVectorOperations::multiply (c.data(), a.data(), b.data(), (i32) bufferSize);
            FloatVectorOperations::addWithMultiply (outBuffer, b.data(), c.data(), (i32) bufferSize);
        }
    }

    //==============================================================================
    z0 timerCallback() override
    {
        printAndResetPerformanceMetrics();
    }

    //==============================================================================
    z0 printHeader() const
    {
        Logger::writeToLog ("buffer size = " + Txt (a.size()) + " samples");
        Logger::writeToLog ("sample rate = " + Txt (currentSampleRate) + " Hz");
        Logger::writeToLog ("physical time limit / callback = " + Txt (getPhysicalTimeLimitMs() )+ " ms");
        Logger::writeToLog ("");
        Logger::writeToLog ("         | callback exec time / physLimit   | callback time gap / physLimit    | callback counters        ");
        Logger::writeToLog ("numLoops | avg     min     max     stddev   | avg     min     max     stddev   | called  late    >limit   ");
        Logger::writeToLog ("-----    | -----   -----   -----   -----    | -----   -----   -----   -----    | ---     ---     ---      ");
    }

    //==============================================================================
    z0 printAndResetPerformanceMetrics()
    {
        std::unique_lock<std::mutex> lock (metricMutex);

        auto runtimeMetric = audioCallbackRuntimeMs;
        auto gapMetric = audioCallbackGapMs;
        auto late = numLateCallbacks;
        auto overLimit = numCallbacksOverPhysicalTimeLimit;

        resetPerformanceMetrics();
        updateNumLoopIterationsPerCallback();

        lock.unlock();

        Logger::writeToLog (Txt (numLoopIterationsPerCallback).paddedRight (' ', 8) + " | "
                            + getPercentFormattedMetricString (runtimeMetric) + " | "
                            + getPercentFormattedMetricString (gapMetric) + " | "
                            + Txt (runtimeMetric.getCount()).paddedRight (' ', 8)
                            + Txt (late).paddedRight (' ', 8)
                            + Txt (overLimit).paddedRight (' ', 8) + " | ");
    }

    //==============================================================================
    Txt getPercentFormattedMetricString (const StatisticsAccumulator<f64> metric) const
    {
        auto physTimeLimit = getPhysicalTimeLimitMs();

        return (Txt (100.0 * metric.getAverage()  / physTimeLimit, 1) + "%").paddedRight (' ', 8)
             + (Txt (100.0 * metric.getMinValue() / physTimeLimit, 1) + "%").paddedRight (' ', 8)
             + (Txt (100.0 * metric.getMaxValue() / physTimeLimit, 1) + "%").paddedRight (' ', 8)
             + Txt (metric.getStandardDeviation(), 3).paddedRight (' ', 8);
    }

    //==============================================================================
    z0 resetPerformanceMetrics()
    {
        audioCallbackRuntimeMs.reset();
        audioCallbackGapMs.reset();
        numLateCallbacks = 0;
        numCallbacksOverPhysicalTimeLimit = 0;
    }

    //==============================================================================
    z0 updateNumLoopIterationsPerCallback()
    {
        numLoopIterationsPerCallback = (i32) loopIterationsSlider.getValue();
    }

    //==============================================================================
    static f64 getPreciseTimeMs() noexcept
    {
        return 1000.0 * (f64) Time::getHighResolutionTicks() / (f64) Time::getHighResolutionTicksPerSecond();
    }

    //==============================================================================
    f64 getPhysicalTimeLimitMs() const noexcept
    {
        return 1000.0 * (f64) a.size() / currentSampleRate;
    }

    //==============================================================================
    std::vector<f32> a, b, c; // must always be of size == current bufferSize
    f64 currentSampleRate = 0.0;

    StatisticsAccumulator<f64> audioCallbackRuntimeMs;
    StatisticsAccumulator<f64> audioCallbackGapMs;
    f64 lastCallbackStartTimeMs = 0.0;
    i32 numLateCallbacks = 0;
    i32 numCallbacksOverPhysicalTimeLimit = 0;
    i32 numLoopIterationsPerCallback;

    Slider loopIterationsSlider;
    std::mutex metricMutex;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
