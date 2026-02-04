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

 name:             AudioWorkgroupDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Simple audio workgroup demo application.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioWorkgroupDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/AudioLiveScrollingDisplay.h"
#include "../Assets/ADSRComponent.h"

constexpr auto NumWorkerThreads = 4;

//==============================================================================
class ThreadBarrier : public ReferenceCountedObject
{
public:
    using Ptr = ReferenceCountedObjectPtr<ThreadBarrier>;

    static Ptr make (i32 numThreadsToSynchronise)
    {
        return { new ThreadBarrier { numThreadsToSynchronise } };
    }

    z0 arriveAndWait()
    {
        std::unique_lock lk { mutex };

        [[maybe_unused]] const auto c = ++blockCount;

        // You've tried to synchronise too many threads!!
        jassert (c <= threadCount);

        if (blockCount == threadCount)
        {
            blockCount = 0;
            cv.notify_all();
            return;
        }

        cv.wait (lk, [this] { return blockCount == 0; });
    }

private:
    std::mutex mutex;
    std::condition_variable cv;
    i32 blockCount{};
    i32k threadCount{};

    explicit ThreadBarrier (i32 numThreadsToSynchronise)
        : threadCount (numThreadsToSynchronise) {}

    DRX_DECLARE_NON_COPYABLE (ThreadBarrier)
    DRX_DECLARE_NON_MOVEABLE (ThreadBarrier)
};

struct Voice
{
    struct Oscillator
    {
        f32 getNextSample()
        {
            const auto s = (2.f * phase - 1.f);
            phase += delta;

            if (phase >= 1.f)
                phase -= 1.f;

            return s;
        }

        f32 delta = 0;
        f32 phase = 0;
    };

    Voice (i32 numSamples, f64 newSampleRate)
        : sampleRate (newSampleRate),
          workBuffer (2, numSamples)
    {
    }

    b8 isActive() const { return adsr.isActive(); }

    z0 startNote (i32 midiNoteNumber, f32 detuneAmount, ADSR::Parameters env)
    {
        constexpr f32 superSawDetuneValues[] = { -1.f, -0.8f, -0.6f, 0.f, 0.5f, 0.7f, 1.f };
        const auto freq = 440.f * std::pow (2.f, ((f32) midiNoteNumber - 69.f) / 12.f);

        for (size_t i = 0; i < 7; i++)
        {
            auto& osc = oscillators[i];

            const auto detune = superSawDetuneValues[i] * detuneAmount;

            osc.delta = (freq + detune) / (f32) sampleRate;
            osc.phase = wobbleGenerator.nextFloat();
        }

        currentNote = midiNoteNumber;

        adsr.setParameters (env);
        adsr.setSampleRate (sampleRate);
        adsr.noteOn();
    }

    z0 stopNote()
    {
        adsr.noteOff();
    }

    z0 run()
    {
        workBuffer.clear();

        constexpr auto oscillatorCount = 7;
        constexpr f32 superSawPanValues[] = { -1.f, -0.7f, -0.3f, 0.f, 0.3f, 0.7f, 1.f };

        constexpr auto spread = 0.8f;
        constexpr auto mix = 1 / 7.f;

        auto* l = workBuffer.getWritePointer (0);
        auto* r = workBuffer.getWritePointer (1);

        for (i32 i = 0; i < workBuffer.getNumSamples(); i++)
        {
            const auto a = adsr.getNextSample();

            f32 left = 0;
            f32 right = 0;

            for (size_t o = 0; o < oscillatorCount; o++)
            {
                auto& osc = oscillators[o];
                const auto s = a * osc.getNextSample();

                left  += s * (1.f - (superSawPanValues[o] * spread));
                right += s * (1.f + (superSawPanValues[o] * spread));
            }

            l[i] += left  * mix;
            r[i] += right * mix;
        }

        workBuffer.applyGain (0.25f);
    }

    const AudioSampleBuffer& getWorkBuffer() const { return workBuffer; }

    ADSR adsr;
    f64 sampleRate;
    std::array<Oscillator, 7> oscillators;
    i32 currentNote = 0;
    Random wobbleGenerator;

private:
    AudioSampleBuffer workBuffer;

    DRX_DECLARE_NON_COPYABLE (Voice)
    DRX_DECLARE_NON_MOVEABLE (Voice)
};

struct AudioWorkerThreadOptions
{
    i32 numChannels;
    i32 numSamples;
    f64 sampleRate;
    AudioWorkgroup workgroup;
    ThreadBarrier::Ptr completionBarrier;
};

class AudioWorkerThread final : private Thread
{
public:
    using Ptr = std::unique_ptr<AudioWorkerThread>;
    using Options = AudioWorkerThreadOptions;

    explicit AudioWorkerThread (const Options& workerOptions)
        : Thread ("AudioWorkerThread"),
          options (workerOptions)
    {
        jassert (options.completionBarrier != nullptr);

       #if defined (DRX_MAC)
        jassert (options.workgroup);
       #endif

        startRealtimeThread (RealtimeOptions{}.withApproximateAudioProcessingTime (options.numSamples, options.sampleRate));
    }

    ~AudioWorkerThread() override { stop(); }

    using Thread::notify;
    using Thread::signalThreadShouldExit;
    using Thread::isThreadRunning;

    i32 getJobCount() const { return lastJobCount; }

    i32 queueAudioJobs (Span<Voice*> jobs)
    {
        size_t spanIndex = 0;

        const auto write = jobQueueFifo.write ((i32) jobs.size());
        write.forEach ([&, jobs] (i32 dstIndex)
        {
            jobQueue[(size_t) dstIndex] = jobs[spanIndex++];
        });
        return write.blockSize1 + write.blockSize2;
    }

private:
    z0 stop()
    {
        signalThreadShouldExit();
        stopThread (-1);
    }

    z0 run() override
    {
        WorkgroupToken token;

        options.workgroup.join (token);

        while (wait (-1) && ! threadShouldExit())
        {
            const auto numReady = jobQueueFifo.getNumReady();
            lastJobCount = numReady;

            if (numReady > 0)
            {
                jobQueueFifo.read (jobQueueFifo.getNumReady())
                            .forEach ([this] (i32 srcIndex)
                            {
                                jobQueue[(size_t) srcIndex]->run();
                            });
            }

            // Wait for all our threads to get to this point.
            options.completionBarrier->arriveAndWait();
        }
    }

    static constexpr auto numJobs = 128;

    Options options;
    std::array<Voice*, numJobs> jobQueue;
    AbstractFifo jobQueueFifo { numJobs };
    std::atomic<i32> lastJobCount = 0;

private:
    DRX_DECLARE_NON_COPYABLE (AudioWorkerThread)
    DRX_DECLARE_NON_MOVEABLE (AudioWorkerThread)
};

template <typename ValueType, typename LockType>
struct SharedThreadValue
{
    SharedThreadValue (LockType& lockRef, ValueType initialValue = {})
        : lock (lockRef),
          preSyncValue (initialValue),
          postSyncValue (initialValue)
    {
    }

    z0 set (const ValueType& newValue)
    {
        const typename LockType::ScopedLockType sl { lock };
        preSyncValue = newValue;
    }

    ValueType get() const
    {
        {
            const typename LockType::ScopedTryLockType sl { lock, true };

            if (sl.isLocked())
                postSyncValue = preSyncValue;
        }

        return postSyncValue;
    }

private:
    LockType& lock;
    ValueType preSyncValue{};
    mutable ValueType postSyncValue{};

    DRX_DECLARE_NON_COPYABLE (SharedThreadValue)
    DRX_DECLARE_NON_MOVEABLE (SharedThreadValue)
};

//==============================================================================
class SuperSynth
{
public:
    SuperSynth() = default;

    z0 setEnvelope (ADSR::Parameters params)
    {
        envelope.set (params);
    }

    z0 setThickness (f32 newThickness)
    {
        thickness.set (newThickness);
    }

    z0 prepareToPlay (i32 numSamples, f64 sampleRate)
    {
        activeVoices.reserve (128);

        for (auto& voice : voices)
            voice.reset (new Voice { numSamples, sampleRate });
    }

    z0 process (ThreadBarrier::Ptr barrier, Span<AudioWorkerThread*> workers,
                  AudioSampleBuffer& buffer, MidiBuffer& midiBuffer)
    {
        const auto blockThickness = thickness.get();
        const auto blockEnvelope = envelope.get();

        // We're not trying to be sample accurate.. handle the on/off events in a single block.
        for (auto event : midiBuffer)
        {
            const auto message = event.getMessage();

            if (message.isNoteOn())
            {
                for (auto& voice : voices)
                {
                    if (! voice->isActive())
                    {
                        voice->startNote (message.getNoteNumber(), blockThickness, blockEnvelope);
                        break;
                    }
                }

                continue;
            }

            if (message.isNoteOff())
            {
                for (auto& voice : voices)
                {
                    if (voice->currentNote == message.getNoteNumber())
                        voice->stopNote();
                }

                continue;
            }
        }

        // Queue up all active voices
        for (auto& voice : voices)
            if (voice->isActive())
                activeVoices.push_back (voice.get());

        constexpr auto jobsPerThread = 1;

        // Try and split the voices evenly just for demonstration purposes.
        // You could also do some of the work on this thread instead of waiting.
        for (i32 i = 0; i < (i32) activeVoices.size();)
        {
            for (auto worker : workers)
            {
                if (i >= (i32) activeVoices.size())
                    break;

                const auto jobCount = jmin (jobsPerThread, (i32) activeVoices.size() - i);
                i += worker->queueAudioJobs ({ activeVoices.data() + i, (size_t) jobCount });
            }
        }

        // kick off the work.
        for (auto& worker : workers)
            worker->notify();

        // Wait for our jobs to complete.
        barrier->arriveAndWait();

        // mix the jobs into the main audio thread buffer.
        for (auto* voice : activeVoices)
        {
            buffer.addFrom (0, 0, voice->getWorkBuffer(), 0, 0, buffer.getNumSamples());
            buffer.addFrom (1, 0, voice->getWorkBuffer(), 1, 0, buffer.getNumSamples());
        }

        // Abuse std::vector not reallocating on clear.
        activeVoices.clear();
    }

private:
    std::array<std::unique_ptr<Voice>, 128> voices;
    std::vector<Voice*> activeVoices;

    template <typename T>
    using ThreadValue = SharedThreadValue<T, SpinLock>;

    SpinLock paramLock;
    ThreadValue<ADSR::Parameters> envelope  { paramLock, { 0.f, 0.3f, 1.f, 0.3f } };
    ThreadValue<f32>            thickness { paramLock, 1.f };

    DRX_DECLARE_NON_COPYABLE (SuperSynth)
    DRX_DECLARE_NON_MOVEABLE (SuperSynth)
};

//==============================================================================
class AudioWorkgroupDemo  : public Component,
                            private Timer,
                            private AudioSource,
                            private MidiInputCallback
{
public:
    AudioWorkgroupDemo()
    {
        addAndMakeVisible (keyboardComponent);
        addAndMakeVisible (liveAudioDisplayComp);
        addAndMakeVisible (envelopeComponent);
        addAndMakeVisible (keyboardComponent);
        addAndMakeVisible (thicknessSlider);
        addAndMakeVisible (voiceCountLabel);

        std::generate (threadLabels.begin(), threadLabels.end(), &std::make_unique<Label>);

        for (auto& label : threadLabels)
        {
            addAndMakeVisible (*label);
            label->setEditable (false);
        }

        thicknessSlider.textFromValueFunction = [] (f64) { return "Phatness"; };
        thicknessSlider.onValueChange = [this] { synthesizer.setThickness ((f32) thicknessSlider.getValue()); };
        thicknessSlider.setRange (0.5, 15, 0.1);
        thicknessSlider.setValue (7, dontSendNotification);
        thicknessSlider.setTextBoxIsEditable (false);

        envelopeComponent.onChange  = [this] { synthesizer.setEnvelope (envelopeComponent.getParameters()); };

        voiceCountLabel.setEditable (false);

        audioSourcePlayer.setSource (this);

       #ifndef DRX_DEMO_RUNNER
        audioDeviceManager.initialise (0, 2, nullptr, true, {}, nullptr);
       #endif

        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioDeviceManager.addMidiInputDeviceCallback ({}, this);

        setOpaque (true);
        setSize (640, 480);
        startTimerHz (10);
    }

    ~AudioWorkgroupDemo() override
    {
        audioSourcePlayer.setSource (nullptr);
        audioDeviceManager.removeMidiInputDeviceCallback ({}, this);
        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();

        liveAudioDisplayComp.setBounds (bounds.removeFromTop (60));
        keyboardComponent.setBounds (bounds.removeFromBottom (150));
        envelopeComponent.setBounds (bounds.removeFromBottom (150));

        thicknessSlider.setBounds (bounds.removeFromTop (30));
        voiceCountLabel.setBounds (bounds.removeFromTop (30));

        const auto maxLabelWidth = bounds.getWidth() / 4;
        auto currentBounds = bounds.removeFromLeft (maxLabelWidth);

        for (auto& l : threadLabels)
        {
            if (currentBounds.getHeight() < 30)
                currentBounds = bounds.removeFromLeft (maxLabelWidth);

            l->setBounds (currentBounds.removeFromTop (30));
        }
    }

    z0 timerCallback() override
    {
        Txt text;
        i32 totalVoices = 0;

        {
            const SpinLock::ScopedLockType sl { threadArrayUiLock };

            for (size_t i = 0; i < NumWorkerThreads; i++)
            {
                const auto& thread = workerThreads[i];
                auto& label        = threadLabels[i];

                if (thread != nullptr)
                {
                    const auto count = thread->getJobCount();

                    text = "Thread ";
                    text << (i32) i << ": " << count << " jobs";
                    label->setText (text, dontSendNotification);
                    totalVoices += count;
                }
            }
        }

        text = {};
        text << "Voices: " << totalVoices << " (" << totalVoices * 7 << " oscs)";
        voiceCountLabel.setText (text, dontSendNotification);
    }

    //==============================================================================
    z0 prepareToPlay (i32 samplesPerBlockExpected, f64 sampleRate) override
    {
        completionBarrier = ThreadBarrier::make ((i32) NumWorkerThreads + 1);

        const auto numChannels = 2;
        const auto workerOptions = AudioWorkerThreadOptions
        {
            numChannels,
            samplesPerBlockExpected,
            sampleRate,
            audioDeviceManager.getDeviceAudioWorkgroup(),
            completionBarrier,
        };

        {
            const SpinLock::ScopedLockType sl { threadArrayUiLock };

            for (auto& worker : workerThreads)
                 worker.reset (new AudioWorkerThread { workerOptions });
        }

        synthesizer.prepareToPlay (samplesPerBlockExpected, sampleRate);
        liveAudioDisplayComp.audioDeviceAboutToStart (audioDeviceManager.getCurrentAudioDevice());
        waveformBuffer.setSize (1, samplesPerBlockExpected);
    }

    z0 releaseResources() override
    {
        {
            const SpinLock::ScopedLockType sl { threadArrayUiLock };

            for (auto& thread : workerThreads)
                thread.reset();
        }

        liveAudioDisplayComp.audioDeviceStopped();
    }

    z0 getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        midiBuffer.clear();

        bufferToFill.clearActiveBufferRegion();
        keyboardState.processNextMidiBuffer (midiBuffer, bufferToFill.startSample, bufferToFill.numSamples, true);

        AudioWorkerThread* workers[NumWorkerThreads]{};
        std::transform (workerThreads.begin(), workerThreads.end(), workers,
                        [] (auto& worker) { return worker.get(); });

        synthesizer.process (completionBarrier, Span { workers }, *bufferToFill.buffer, midiBuffer);

        // LiveAudioScrollingDisplay applies a 10x gain to the input signal, we need to reduce the gain on our signal.
        waveformBuffer.copyFrom (0, 0,
                                 bufferToFill.buffer->getReadPointer (0),
                                 bufferToFill.numSamples,
                                 1 / 10.f);
        liveAudioDisplayComp.audioDeviceIOCallbackWithContext (waveformBuffer.getArrayOfReadPointers(), 1,
                                                               nullptr, 0, bufferToFill.numSamples, {});
    }

    z0 handleIncomingMidiMessage (MidiInput*, const MidiMessage& message) override
    {
        if (message.isNoteOn())
            keyboardState.noteOn (message.getChannel(), message.getNoteNumber(), 1);
        else if (message.isNoteOff())
            keyboardState.noteOff (message.getChannel(), message.getNoteNumber(), 1);
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef DRX_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    MidiBuffer                midiBuffer;
    MidiKeyboardState         keyboardState;
    AudioSourcePlayer         audioSourcePlayer;
    SuperSynth                synthesizer;
    AudioSampleBuffer         waveformBuffer;

    MidiKeyboardComponent     keyboardComponent { keyboardState, MidiKeyboardComponent::horizontalKeyboard };
    LiveScrollingAudioDisplay liveAudioDisplayComp;
    ADSRComponent             envelopeComponent;
    Slider                    thicknessSlider { Slider::SliderStyle::LinearHorizontal, Slider::TextBoxLeft };
    Label                     voiceCountLabel;

    SpinLock                  threadArrayUiLock;
    ThreadBarrier::Ptr        completionBarrier;

    std::array<std::unique_ptr<Label>, NumWorkerThreads> threadLabels;
    std::array<AudioWorkerThread::Ptr, NumWorkerThreads> workerThreads;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWorkgroupDemo)
};
