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

 name:             AudioPlaybackDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Plays an audio file.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        AudioPlaybackDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

class DemoThumbnailComp final : public Component,
                                public ChangeListener,
                                public FileDragAndDropTarget,
                                public ChangeBroadcaster,
                                private ScrollBar::Listener,
                                private Timer
{
public:
    DemoThumbnailComp (AudioFormatManager& formatManager,
                       AudioTransportSource& source,
                       Slider& slider)
        : transportSource (source),
          zoomSlider (slider),
          thumbnail (512, formatManager, thumbnailCache)
    {
        thumbnail.addChangeListener (this);

        addAndMakeVisible (scrollbar);
        scrollbar.setRangeLimits (visibleRange);
        scrollbar.setAutoHide (false);
        scrollbar.addListener (this);

        currentPositionMarker.setFill (Colors::white.withAlpha (0.85f));
        addAndMakeVisible (currentPositionMarker);
    }

    ~DemoThumbnailComp() override
    {
        scrollbar.removeListener (this);
        thumbnail.removeChangeListener (this);
    }

    z0 setURL (const URL& url)
    {
        if (auto inputSource = makeInputSource (url))
        {
            thumbnail.setSource (inputSource.release());

            Range<f64> newRange (0.0, thumbnail.getTotalLength());
            scrollbar.setRangeLimits (newRange);
            setRange (newRange);

            startTimerHz (40);
        }
    }

    URL getLastDroppedFile() const noexcept { return lastFileDropped; }

    z0 setZoomFactor (f64 amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            auto newScale = jmax (0.001, thumbnail.getTotalLength() * (1.0 - jlimit (0.0, 0.99, amount)));
            auto timeAtCentre = xToTime ((f32) getWidth() / 2.0f);

            setRange ({ timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5 });
        }
    }

    z0 setRange (Range<f64> newRange)
    {
        visibleRange = newRange;
        scrollbar.setCurrentRange (visibleRange);
        updateCursorPosition();
        repaint();
    }

    z0 setFollowsTransport (b8 shouldFollow)
    {
        isFollowingTransport = shouldFollow;
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::darkgrey);
        g.setColor (Colors::lightblue);

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto thumbArea = getLocalBounds();

            thumbArea.removeFromBottom (scrollbar.getHeight() + 4);
            thumbnail.drawChannels (g, thumbArea.reduced (2),
                                    visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        }
        else
        {
            g.setFont (14.0f);
            g.drawFittedText ("(No audio file selected)", getLocalBounds(), Justification::centred, 2);
        }
    }

    z0 resized() override
    {
        scrollbar.setBounds (getLocalBounds().removeFromBottom (14).reduced (2));
    }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    b8 isInterestedInFileDrag (const StringArray& /*files*/) override
    {
        return true;
    }

    z0 filesDropped (const StringArray& files, i32 /*x*/, i32 /*y*/) override
    {
        lastFileDropped = URL (File (files[0]));
        sendChangeMessage();
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        if (canMoveTransport())
            transportSource.setPosition (jmax (0.0, xToTime ((f32) e.x)));
    }

    z0 mouseUp (const MouseEvent&) override
    {
        transportSource.start();
    }

    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            auto newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
            newStart = jlimit (0.0, jmax (0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

            if (canMoveTransport())
                setRange ({ newStart, newStart + visibleRange.getLength() });

            if (! approximatelyEqual (wheel.deltaY, 0.0f))
                zoomSlider.setValue (zoomSlider.getValue() - wheel.deltaY);

            repaint();
        }
    }

private:
    AudioTransportSource& transportSource;
    Slider& zoomSlider;
    ScrollBar scrollbar  { false };

    AudioThumbnailCache thumbnailCache  { 5 };
    AudioThumbnail thumbnail;
    Range<f64> visibleRange;
    b8 isFollowingTransport = false;
    URL lastFileDropped;

    DrawableRectangle currentPositionMarker;

    f32 timeToX (const f64 time) const
    {
        if (visibleRange.getLength() <= 0)
            return 0;

        return (f32) getWidth() * (f32) ((time - visibleRange.getStart()) / visibleRange.getLength());
    }

    f64 xToTime (const f32 x) const
    {
        return (x / (f32) getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
    }

    b8 canMoveTransport() const noexcept
    {
        return ! (isFollowingTransport && transportSource.isPlaying());
    }

    z0 scrollBarMoved (ScrollBar* scrollBarThatHasMoved, f64 newRangeStart) override
    {
        if (scrollBarThatHasMoved == &scrollbar)
            if (! (isFollowingTransport && transportSource.isPlaying()))
                setRange (visibleRange.movedToStartAt (newRangeStart));
    }

    z0 timerCallback() override
    {
        if (canMoveTransport())
            updateCursorPosition();
        else
            setRange (visibleRange.movedToStartAt (transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
    }

    z0 updateCursorPosition()
    {
        currentPositionMarker.setVisible (transportSource.isPlaying() || isMouseButtonDown());

        currentPositionMarker.setRectangle (Rectangle<f32> (timeToX (transportSource.getCurrentPosition()) - 0.75f, 0,
                                                              1.5f, (f32) (getHeight() - scrollbar.getHeight())));
    }
};

//==============================================================================
class AudioPlaybackDemo final : public Component,
                               #if (DRX_ANDROID || DRX_IOS)
                                private Button::Listener,
                               #else
                                private FileBrowserListener,
                               #endif
                                private ChangeListener
{
public:
    AudioPlaybackDemo()
    {
        addAndMakeVisible (zoomLabel);
        zoomLabel.setFont (FontOptions (15.00f, Font::plain));
        zoomLabel.setJustificationType (Justification::centredRight);
        zoomLabel.setEditable (false, false, false);
        zoomLabel.setColor (TextEditor::textColorId, Colors::black);
        zoomLabel.setColor (TextEditor::backgroundColorId, Color (0x00000000));

        addAndMakeVisible (followTransportButton);
        followTransportButton.onClick = [this] { updateFollowTransportState(); };

       #if (DRX_ANDROID || DRX_IOS)
        addAndMakeVisible (chooseFileButton);
        chooseFileButton.addListener (this);
       #else
        addAndMakeVisible (fileTreeComp);

        directoryList.setDirectory (File::getSpecialLocation (File::userHomeDirectory), true, true);

        fileTreeComp.setTitle ("Files");
        fileTreeComp.setColor (FileTreeComponent::backgroundColorId, Colors::lightgrey.withAlpha (0.6f));
        fileTreeComp.addListener (this);

        addAndMakeVisible (explanation);
        explanation.setFont (FontOptions (14.00f, Font::plain));
        explanation.setJustificationType (Justification::bottomRight);
        explanation.setEditable (false, false, false);
        explanation.setColor (TextEditor::textColorId, Colors::black);
        explanation.setColor (TextEditor::backgroundColorId, Color (0x00000000));
       #endif

        addAndMakeVisible (zoomSlider);
        zoomSlider.setRange (0, 1, 0);
        zoomSlider.onValueChange = [this] { thumbnail->setZoomFactor (zoomSlider.getValue()); };
        zoomSlider.setSkewFactor (2);

        thumbnail = std::make_unique<DemoThumbnailComp> (formatManager, transportSource, zoomSlider);
        addAndMakeVisible (thumbnail.get());
        thumbnail->addChangeListener (this);

        addAndMakeVisible (startStopButton);
        startStopButton.setColor (TextButton::buttonColorId, Color (0xff79ed7f));
        startStopButton.setColor (TextButton::textColorOffId, Colors::black);
        startStopButton.onClick = [this] { startOrStop(); };

        // audio setup
        formatManager.registerBasicFormats();

        thread.startThread (Thread::Priority::normal);

       #ifndef DRX_DEMO_RUNNER
        audioDeviceManager.initialise (0, 2, nullptr, true, {}, nullptr);
       #endif

        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&transportSource);

        setOpaque (true);
        setSize (500, 500);
    }

    ~AudioPlaybackDemo() override
    {
        transportSource  .setSource (nullptr);
        audioSourcePlayer.setSource (nullptr);

        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);

       #if (DRX_ANDROID || DRX_IOS)
        chooseFileButton.removeListener (this);
       #else
        fileTreeComp.removeListener (this);
       #endif

        thumbnail->removeChangeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (4);

        auto controls = r.removeFromBottom (90);

        auto controlRightBounds = controls.removeFromRight (controls.getWidth() / 3);

       #if (DRX_ANDROID || DRX_IOS)
        chooseFileButton.setBounds (controlRightBounds.reduced (10));
       #else
        explanation.setBounds (controlRightBounds);
       #endif

        auto zoom = controls.removeFromTop (25);
        zoomLabel .setBounds (zoom.removeFromLeft (50));
        zoomSlider.setBounds (zoom);

        followTransportButton.setBounds (controls.removeFromTop (25));
        startStopButton      .setBounds (controls);

        r.removeFromBottom (6);

       #if DRX_ANDROID || DRX_IOS
        thumbnail->setBounds (r);
       #else
        thumbnail->setBounds (r.removeFromBottom (140));
        r.removeFromBottom (6);

        fileTreeComp.setBounds (r);
       #endif
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef DRX_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    AudioFormatManager formatManager;
    TimeSliceThread thread  { "audio file preview" };

   #if (DRX_ANDROID || DRX_IOS)
    std::unique_ptr<FileChooser> fileChooser;
    TextButton chooseFileButton {"Choose Audio File...", "Choose an audio file for playback"};
   #else
    DirectoryContentsList directoryList {nullptr, thread};
    FileTreeComponent fileTreeComp {directoryList};
    Label explanation { {}, "Select an audio file in the treeview above, and this page will display its waveform, and let you play it.." };
   #endif

    URL currentAudioFile;
    AudioSourcePlayer audioSourcePlayer;
    AudioTransportSource transportSource;
    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;

    std::unique_ptr<DemoThumbnailComp> thumbnail;
    Label zoomLabel                     { {}, "zoom:" };
    Slider zoomSlider                   { Slider::LinearHorizontal, Slider::NoTextBox };
    ToggleButton followTransportButton  { "Follow Transport" };
    TextButton startStopButton          { "Play/Stop" };

    //==============================================================================
    z0 showAudioResource (URL resource)
    {
        if (! loadURLIntoTransport (resource))
        {
            // Failed to load the audio file!
            jassertfalse;
            return;
        }

        currentAudioFile = std::move (resource);
        zoomSlider.setValue (0, dontSendNotification);
        thumbnail->setURL (currentAudioFile);
    }

    b8 loadURLIntoTransport (const URL& audioURL)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        currentAudioFileSource.reset();

        const auto source = makeInputSource (audioURL);

        if (source == nullptr)
            return false;

        auto stream = rawToUniquePtr (source->createInputStream());

        if (stream == nullptr)
            return false;

        auto reader = rawToUniquePtr (formatManager.createReaderFor (std::move (stream)));

        if (reader == nullptr)
            return false;

        currentAudioFileSource = std::make_unique<AudioFormatReaderSource> (reader.release(), true);

        // ..and plug it into our transport source
        transportSource.setSource (currentAudioFileSource.get(),
                                   32768,                   // tells it to buffer this many samples ahead
                                   &thread,                 // this is the background thread to use for reading-ahead
                                   currentAudioFileSource->getAudioFormatReader()->sampleRate);     // allows for sample rate correction

        return true;
    }

    z0 startOrStop()
    {
        if (transportSource.isPlaying())
        {
            transportSource.stop();
        }
        else
        {
            transportSource.setPosition (0);
            transportSource.start();
        }
    }

    z0 updateFollowTransportState()
    {
        thumbnail->setFollowsTransport (followTransportButton.getToggleState());
    }

   #if (DRX_ANDROID || DRX_IOS)
    z0 buttonClicked (Button* btn) override
    {
        if (btn == &chooseFileButton && fileChooser.get() == nullptr)
        {
            if (! RuntimePermissions::isGranted (RuntimePermissions::readExternalStorage))
            {
                SafePointer<AudioPlaybackDemo> safeThis (this);
                RuntimePermissions::request (RuntimePermissions::readExternalStorage,
                                             [safeThis] (b8 granted) mutable
                                             {
                                                 if (safeThis != nullptr && granted)
                                                     safeThis->buttonClicked (&safeThis->chooseFileButton);
                                             });
                return;
            }

            if (FileChooser::isPlatformDialogAvailable())
            {
                fileChooser = std::make_unique<FileChooser> ("Select an audio file...", File(), "*.wav;*.flac;*.aif");

                fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                          [this] (const FileChooser& fc) mutable
                                          {
                                              if (fc.getURLResults().size() > 0)
                                              {
                                                  auto u = fc.getURLResult();

                                                  showAudioResource (std::move (u));
                                              }

                                              fileChooser = nullptr;
                                          }, nullptr);
            }
            else
            {
                NativeMessageBox::showAsync (MessageBoxOptions()
                                               .withIconType (MessageBoxIconType::WarningIcon)
                                               .withTitle ("Enable Code Signing")
                                               .withMessage ("You need to enable code-signing for your iOS project and enable \"iCloud Documents\" "
                                                             "permissions to be able to open audio files on your iDevice. See: "
                                                             "https://forum.drx.com/t/native-ios-android-file-choosers"),
                                             nullptr);
            }
        }
    }
   #else
    z0 selectionChanged() override
    {
        showAudioResource (URL (fileTreeComp.getSelectedFile()));
    }

    z0 fileClicked (const File&, const MouseEvent&) override          {}
    z0 fileDoubleClicked (const File&) override                       {}
    z0 browserRootChanged (const File&) override                      {}
   #endif

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == thumbnail.get())
            showAudioResource (URL (thumbnail->getLastDroppedFile()));
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackDemo)
};
