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

 name:             AudioRecordingDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Records audio to a file.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioRecordingDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/AudioLiveScrollingDisplay.h"

//==============================================================================
/** A simple class that acts as an AudioIODeviceCallback and writes the
    incoming audio data to a WAV file.
*/
class AudioRecorder final : public AudioIODeviceCallback
{
public:
    AudioRecorder (AudioThumbnail& thumbnailToUpdate)
        : thumbnail (thumbnailToUpdate)
    {
        backgroundThread.startThread();
    }

    ~AudioRecorder() override
    {
        stop();
    }

    //==============================================================================
    z0 startRecording (const File& file)
    {
        stop();

        if (sampleRate > 0)
        {
            // Create an OutputStream to write to our destination file...
            file.deleteFile();

            if (auto fileStream = std::unique_ptr<FileOutputStream> (file.createOutputStream()))
            {
                // Now create a WAV writer object that writes to our output stream...
                WavAudioFormat wavFormat;

                if (auto writer = wavFormat.createWriterFor (fileStream.get(), sampleRate, 1, 16, {}, 0))
                {
                    fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

                    // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                    // write the data to disk on our background thread.
                    threadedWriter.reset (new AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768));

                    // Reset our recording thumbnail
                    thumbnail.reset (writer->getNumChannels(), writer->getSampleRate());
                    nextSampleNum = 0;

                    // And now, swap over our active writer pointer so that the audio callback will start using it..
                    const ScopedLock sl (writerLock);
                    activeWriter = threadedWriter.get();
                }
            }
        }
    }

    z0 stop()
    {
        // First, clear this pointer to stop the audio callback from using our writer object..
        {
            const ScopedLock sl (writerLock);
            activeWriter = nullptr;
        }

        // Now we can delete the writer object. It's done in this order because the deletion could
        // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
        // the audio callback while this happens.
        threadedWriter.reset();
    }

    b8 isRecording() const
    {
        return activeWriter.load() != nullptr;
    }

    //==============================================================================
    z0 audioDeviceAboutToStart (AudioIODevice* device) override
    {
        sampleRate = device->getCurrentSampleRate();
    }

    z0 audioDeviceStopped() override
    {
        sampleRate = 0;
    }

    z0 audioDeviceIOCallbackWithContext (const f32* const* inputChannelData, i32 numInputChannels,
                                           f32* const* outputChannelData, i32 numOutputChannels,
                                           i32 numSamples, const AudioIODeviceCallbackContext& context) override
    {
        ignoreUnused (context);

        const ScopedLock sl (writerLock);

        if (activeWriter.load() != nullptr && numInputChannels >= thumbnail.getNumChannels())
        {
            activeWriter.load()->write (inputChannelData, numSamples);

            // Create an AudioBuffer to wrap our incoming data, note that this does no allocations or copies, it simply references our input data
            AudioBuffer<f32> buffer (const_cast<f32**> (inputChannelData), thumbnail.getNumChannels(), numSamples);
            thumbnail.addBlock (nextSampleNum, buffer, 0, numSamples);
            nextSampleNum += numSamples;
        }

        // We need to clear the output buffers, in case they're full of junk..
        for (i32 i = 0; i < numOutputChannels; ++i)
            if (outputChannelData[i] != nullptr)
                FloatVectorOperations::clear (outputChannelData[i], numSamples);
    }

private:
    AudioThumbnail& thumbnail;
    TimeSliceThread backgroundThread { "Audio Recorder Thread" }; // the thread that will write our audio data to disk
    std::unique_ptr<AudioFormatWriter::ThreadedWriter> threadedWriter; // the FIFO used to buffer the incoming data
    f64 sampleRate = 0.0;
    z64 nextSampleNum = 0;

    CriticalSection writerLock;
    std::atomic<AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };
};

//==============================================================================
class RecordingThumbnail final : public Component,
                                 private ChangeListener
{
public:
    RecordingThumbnail()
    {
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener (this);
    }

    ~RecordingThumbnail() override
    {
        thumbnail.removeChangeListener (this);
    }

    AudioThumbnail& getAudioThumbnail()     { return thumbnail; }

    z0 setDisplayFullThumbnail (b8 displayFull)
    {
        displayFullThumb = displayFull;
        repaint();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::darkgrey);
        g.setColor (Colors::lightgrey);

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto endTime = displayFullThumb ? thumbnail.getTotalLength()
                                            : jmax (30.0, thumbnail.getTotalLength());

            auto thumbArea = getLocalBounds();
            thumbnail.drawChannels (g, thumbArea.reduced (2), 0.0, endTime, 1.0f);
        }
        else
        {
            g.setFont (14.0f);
            g.drawFittedText ("(No file recorded)", getLocalBounds(), Justification::centred, 2);
        }
    }

private:
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache  { 10 };
    AudioThumbnail thumbnail            { 512, formatManager, thumbnailCache };

    b8 displayFullThumb = false;

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            repaint();
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordingThumbnail)
};

//==============================================================================
class AudioRecordingDemo final : public Component
{
public:
    AudioRecordingDemo()
    {
        setOpaque (true);
        addAndMakeVisible (liveAudioScroller);

        addAndMakeVisible (explanationLabel);
        explanationLabel.setFont (FontOptions (15.0f, Font::plain));
        explanationLabel.setJustificationType (Justification::topLeft);
        explanationLabel.setEditable (false, false, false);
        explanationLabel.setColor (TextEditor::textColorId, Colors::black);
        explanationLabel.setColor (TextEditor::backgroundColorId, Color (0x00000000));

        addAndMakeVisible (recordButton);
        recordButton.setColor (TextButton::buttonColorId, Color (0xffff5c5c));
        recordButton.setColor (TextButton::textColorOnId, Colors::black);

        recordButton.onClick = [this]
        {
            if (recorder.isRecording())
                stopRecording();
            else
                startRecording();
        };

        addAndMakeVisible (recordingThumbnail);

       #ifndef DRX_DEMO_RUNNER
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (b8 granted)
                                     {
                                         i32 numInputChannels = granted ? 2 : 0;
                                         audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                     });
       #endif

        audioDeviceManager.addAudioCallback (&liveAudioScroller);
        audioDeviceManager.addAudioCallback (&recorder);

        setSize (500, 500);
    }

    ~AudioRecordingDemo() override
    {
        audioDeviceManager.removeAudioCallback (&recorder);
        audioDeviceManager.removeAudioCallback (&liveAudioScroller);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto area = getLocalBounds();

        liveAudioScroller .setBounds (area.removeFromTop (80).reduced (8));
        recordingThumbnail.setBounds (area.removeFromTop (80).reduced (8));
        recordButton      .setBounds (area.removeFromTop (36).removeFromLeft (140).reduced (8));
        explanationLabel  .setBounds (area.reduced (8));
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef DRX_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (1, 0) };
   #endif

    LiveScrollingAudioDisplay liveAudioScroller;
    RecordingThumbnail recordingThumbnail;
    AudioRecorder recorder { recordingThumbnail.getAudioThumbnail() };

    Label explanationLabel { {},
                             "This page demonstrates how to record a wave file from the live audio input.\n\n"
                             "After you are done with your recording you can choose where to save it." };
    TextButton recordButton { "Record" };
    File lastRecording;
    FileChooser chooser { "Output file...", File::getCurrentWorkingDirectory().getChildFile ("recording.wav"), "*.wav" };

    z0 startRecording()
    {
        if (! RuntimePermissions::isGranted (RuntimePermissions::writeExternalStorage))
        {
            SafePointer<AudioRecordingDemo> safeThis (this);

            RuntimePermissions::request (RuntimePermissions::writeExternalStorage,
                                         [safeThis] (b8 granted) mutable
                                         {
                                             if (granted)
                                                 safeThis->startRecording();
                                         });
            return;
        }

       #if (DRX_ANDROID || DRX_IOS)
        auto parentDir = File::getSpecialLocation (File::tempDirectory);
       #else
        auto parentDir = File::getSpecialLocation (File::userDocumentsDirectory);
       #endif

        lastRecording = parentDir.getNonexistentChildFile ("DRX Demo Audio Recording", ".wav");

        recorder.startRecording (lastRecording);

        recordButton.setButtonText ("Stop");
        recordingThumbnail.setDisplayFullThumbnail (false);
    }

    z0 stopRecording()
    {
        recorder.stop();

        chooser.launchAsync (  FileBrowserComponent::saveMode
                             | FileBrowserComponent::canSelectFiles
                             | FileBrowserComponent::warnAboutOverwriting,
                             [this] (const FileChooser& c)
                             {
                                 if (FileInputStream inputStream (lastRecording); inputStream.openedOk())
                                    if (const auto outputStream = makeOutputStream (c.getURLResult()))
                                        outputStream->writeFromInputStream (inputStream, -1);

                                 recordButton.setButtonText ("Record");
                                 recordingThumbnail.setDisplayFullThumbnail (true);
                             });
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioRecordingDemo)
};
