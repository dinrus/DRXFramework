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

 name:             VideoDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Plays video files.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra, drx_video
 exporters:        xcode_mac, vs2022, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        VideoDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

#if DRX_MAC || DRX_WINDOWS
//==============================================================================
// so that we can easily have two video windows each with a file browser, wrap this up as a class..
class MovieComponentWithFileBrowser final : public Component,
                                            public DragAndDropTarget,
                                            private FilenameComponentListener
{
public:
    MovieComponentWithFileBrowser()
        : videoComp (true)
    {
        addAndMakeVisible (videoComp);

        addAndMakeVisible (fileChooser);
        fileChooser.addListener (this);
        fileChooser.setBrowseButtonText ("browse");
    }

    z0 setFile (const File& file)
    {
        fileChooser.setCurrentFile (file, true);
    }

    z0 paintOverChildren (Graphics& g) override
    {
        if (isDragOver)
        {
            g.setColor (Colors::red);
            g.drawRect (fileChooser.getBounds(), 2);
        }
    }

    z0 resized() override
    {
        videoComp.setBounds (getLocalBounds().reduced (10));
    }

    b8 isInterestedInDragSource (const SourceDetails&) override   { return true; }

    z0 itemDragEnter (const SourceDetails&) override
    {
        isDragOver = true;
        repaint();
    }

    z0 itemDragExit (const SourceDetails&) override
    {
        isDragOver = false;
        repaint();
    }

    z0 itemDropped (const SourceDetails& dragSourceDetails) override
    {
        setFile (dragSourceDetails.description.toString());
        isDragOver = false;
        repaint();
    }

private:
    VideoComponent videoComp;

    b8 isDragOver = false;
    FilenameComponent fileChooser  { "movie", {}, true, false, false, "*", {}, "(choose a video file to play)"};

    z0 filenameComponentChanged (FilenameComponent*) override
    {
        auto url = URL (fileChooser.getCurrentFile());

        // this is called when the user changes the filename in the file chooser box
        auto result = videoComp.load (url);
        videoLoadingFinished (url, result);
    }

    z0 videoLoadingFinished (const URL& url, Result result)
    {
        ignoreUnused (url);

        if (result.wasOk())
        {
            // loaded the file ok, so let's start it playing..

            videoComp.play();
            resized(); // update to reflect the video's aspect ratio
        }
        else
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             "Couldn't load the file!",
                                                             result.getErrorMessage());
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
    }

    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MovieComponentWithFileBrowser)
};

//==============================================================================
class VideoDemo final : public Component,
                        public DragAndDropContainer,
                        private FileBrowserListener
{
public:
    VideoDemo()
    {
        setOpaque (true);

        movieList.setDirectory (File::getSpecialLocation (File::userMoviesDirectory), true, true);
        directoryThread.startThread (Thread::Priority::background);

        fileTree.setTitle ("Files");
        fileTree.addListener (this);
        fileTree.setColor (FileTreeComponent::backgroundColorId, Colors::lightgrey.withAlpha (0.6f));
        addAndMakeVisible (fileTree);

        addAndMakeVisible (resizerBar);

        loadLeftButton .onClick = [this] { movieCompLeft .setFile (fileTree.getSelectedFile (0)); };
        loadRightButton.onClick = [this] { movieCompRight.setFile (fileTree.getSelectedFile (0)); };

        addAndMakeVisible (loadLeftButton);
        addAndMakeVisible (loadRightButton);

        addAndMakeVisible (movieCompLeft);
        addAndMakeVisible (movieCompRight);

        // we have to set up our StretchableLayoutManager so it know the limits and preferred sizes of it's contents
        stretchableManager.setItemLayout (0,            // for the fileTree
                                          -0.1, -0.9,   // must be between 50 pixels and 90% of the available space
                                          -0.3);        // and its preferred size is 30% of the total available space

        stretchableManager.setItemLayout (1,            // for the resize bar
                                          5, 5, 5);     // hard limit to 5 pixels

        stretchableManager.setItemLayout (2,            // for the movie components
                                          -0.1, -0.9,   // size must be between 50 pixels and 90% of the available space
                                          -0.7);        // and its preferred size is 70% of the total available space

        setSize (500, 500);
    }

    ~VideoDemo() override
    {
        fileTree.removeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        // make a list of two of our child components that we want to reposition
        Component* comps[] = { &fileTree, &resizerBar, nullptr };

        // this will position the 3 components, one above the other, to fit
        // vertically into the rectangle provided.
        stretchableManager.layOutComponents (comps, 3,
                                             0, 0, getWidth(), getHeight(),
                                             true, true);

        // now position out two video components in the space that's left
        auto area = getLocalBounds().removeFromBottom (getHeight() - resizerBar.getBottom());

        {
            auto buttonArea = area.removeFromTop (30);
            loadLeftButton .setBounds (buttonArea.removeFromLeft (buttonArea.getWidth() / 2).reduced (5));
            loadRightButton.setBounds (buttonArea.reduced (5));
        }

        movieCompLeft .setBounds (area.removeFromLeft (area.getWidth() / 2).reduced (5));
        movieCompRight.setBounds (area.reduced (5));
    }

private:
    std::unique_ptr<FileChooser> fileChooser;
    WildcardFileFilter moviesWildcardFilter  { "*", "*", "Movies File Filter" };
    TimeSliceThread directoryThread          { "Movie File Scanner Thread" };
    DirectoryContentsList movieList          { &moviesWildcardFilter, directoryThread };
    FileTreeComponent fileTree               { movieList };

    StretchableLayoutManager stretchableManager;
    StretchableLayoutResizerBar resizerBar   { &stretchableManager, 1, false };

    TextButton loadLeftButton   { "Load Left" },
               loadRightButton  { "Load Right" };
    MovieComponentWithFileBrowser movieCompLeft, movieCompRight;

    z0 selectionChanged() override
    {
        // we're just going to update the drag description of out tree so that rows can be dragged onto the file players
        fileTree.setDragAndDropDescription (fileTree.getSelectedFile().getFullPathName());
    }

    z0 fileClicked (const File&, const MouseEvent&) override {}
    z0 fileDoubleClicked (const File&)              override {}
    z0 browserRootChanged (const File&)             override {}

    z0 selectVideoFile()
    {
        fileChooser.reset (new FileChooser ("Choose a file to open...", File::getCurrentWorkingDirectory(),
                                            "*", false));

        fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                  [this] (const FileChooser& chooser)
                                  {
                                      Txt chosen;
                                      auto results = chooser.getURLResults();

                                      // TODO: support non local files too
                                      if (results.size() > 0)
                                          movieCompLeft.setFile (results[0].getLocalFile());
                                  });
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoDemo)
};
#elif DRX_IOS || DRX_ANDROID
//==============================================================================
class VideoDemo final : public Component,
                        private Timer
{
public:
    VideoDemo()
        : videoCompWithNativeControls (true),
          videoCompNoNativeControls (false)
    {
        loadLocalButton  .onClick = [this] { selectVideoFile(); };
        loadUrlButton    .onClick = [this] { showVideoUrlPrompt(); };
        seekToStartButton.onClick = [this] { seekVideoToStart(); };
        playButton       .onClick = [this] { playVideo(); };
        pauseButton      .onClick = [this] { pauseVideo(); };
        unloadButton     .onClick = [this] { unloadVideoFile(); };

        volumeLabel         .setColor (Label::textColorId, Colors::white);
        currentPositionLabel.setColor (Label::textColorId, Colors::white);

        volumeLabel         .setJustificationType (Justification::right);
        currentPositionLabel.setJustificationType (Justification::right);

        volumeSlider  .setRange (0.0, 1.0);
        positionSlider.setRange (0.0, 1.0);

        volumeSlider  .setSliderSnapsToMousePosition (false);
        positionSlider.setSliderSnapsToMousePosition (false);

        volumeSlider.setSkewFactor (1.5);
        volumeSlider.setValue (1.0, dontSendNotification);
       #if DRX_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
        curVideoComp->onGlobalMediaVolumeChanged = [this]() { volumeSlider.setValue (curVideoComp->getAudioVolume(), dontSendNotification); };
       #endif

        volumeSlider  .onValueChange = [this]() { curVideoComp->setAudioVolume ((f32) volumeSlider.getValue()); };
        positionSlider.onValueChange = [this]() { seekVideoToNormalisedPosition (positionSlider.getValue()); };

        positionSlider.onDragStart = [this]()
                                     {
                                         positionSliderDragging = true;
                                         wasPlayingBeforeDragStart = curVideoComp->isPlaying();

                                         if (wasPlayingBeforeDragStart)
                                             curVideoComp->stop();
                                     };

        positionSlider.onDragEnd   = [this]()
                                     {
                                         if (wasPlayingBeforeDragStart)
                                             curVideoComp->play();

                                         wasPlayingBeforeDragStart = false;

                                         // Ensure the slider does not temporarily jump back on consecutive timer callback.
                                         Timer::callAfterDelay (500, [this]() { positionSliderDragging = false; });
                                     };

        playSpeedComboBox.addItem ("25%", 25);
        playSpeedComboBox.addItem ("50%", 50);
        playSpeedComboBox.addItem ("100%", 100);
        playSpeedComboBox.addItem ("200%", 200);
        playSpeedComboBox.addItem ("400%", 400);
        playSpeedComboBox.setSelectedId (100, dontSendNotification);
        playSpeedComboBox.onChange = [this]() { curVideoComp->setPlaySpeed (playSpeedComboBox.getSelectedId() / 100.0); };

        setTransportControlsEnabled (false);

        addAndMakeVisible (loadLocalButton);
        addAndMakeVisible (loadUrlButton);
        addAndMakeVisible (volumeLabel);
        addAndMakeVisible (volumeSlider);
        addChildComponent (videoCompWithNativeControls);
        addChildComponent (videoCompNoNativeControls);
        addAndMakeVisible (positionSlider);
        addAndMakeVisible (currentPositionLabel);

        addAndMakeVisible (playSpeedComboBox);
        addAndMakeVisible (seekToStartButton);
        addAndMakeVisible (playButton);
        addAndMakeVisible (unloadButton);
        addChildComponent (pauseButton);

        setSize (500, 500);

        RuntimePermissions::request (RuntimePermissions::readExternalStorage,
                                     [] (b8 granted)
                                     {
                                         if (! granted)
                                         {
                                             AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                                                               "Permissions warning",
                                                                               "External storage access permission not granted, some files"
                                                                               " may be inaccessible.");
                                         }
                                     });

        setPortraitOrientationEnabled (true);
    }

    ~VideoDemo() override
    {
        curVideoComp->onPlaybackStarted = nullptr;
        curVideoComp->onPlaybackStopped = nullptr;
        curVideoComp->onErrorOccurred   = nullptr;
        curVideoComp->onGlobalMediaVolumeChanged = nullptr;

        setPortraitOrientationEnabled (false);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto area = getLocalBounds();

        i32 marginSize = 5;
        i32 buttonHeight = 20;

        area.reduce (0, marginSize);

        auto topArea = area.removeFromTop (buttonHeight);
        loadLocalButton.setBounds (topArea.removeFromLeft (topArea.getWidth() / 6));
        loadUrlButton.setBounds (topArea.removeFromLeft (loadLocalButton.getWidth()));
        volumeLabel.setBounds (topArea.removeFromLeft (loadLocalButton.getWidth()));
        volumeSlider.setBounds (topArea.reduced (10, 0));

        auto transportArea = area.removeFromBottom (buttonHeight);
        auto positionArea  = area.removeFromBottom (buttonHeight).reduced (marginSize, 0);

        playSpeedComboBox.setBounds (transportArea.removeFromLeft (jmax (50, transportArea.getWidth() / 5)));

        auto controlWidth = transportArea.getWidth() / 3;

        currentPositionLabel.setBounds (positionArea.removeFromRight (jmax (150, controlWidth)));
        positionSlider.setBounds (positionArea);

        seekToStartButton.setBounds (transportArea.removeFromLeft (controlWidth));
        playButton       .setBounds (transportArea.removeFromLeft (controlWidth));
        unloadButton     .setBounds (transportArea.removeFromLeft (controlWidth));
        pauseButton.setBounds (playButton.getBounds());

        area.removeFromTop (marginSize);
        area.removeFromBottom (marginSize);

        videoCompWithNativeControls.setBounds (area);
        videoCompNoNativeControls.setBounds (area);

        if (positionSlider.getWidth() > 0)
            positionSlider.setMouseDragSensitivity (positionSlider.getWidth());
    }

private:
    TextButton loadLocalButton { "Load Local" };
    TextButton loadUrlButton { "Load URL" };
    Label volumeLabel { "volumeLabel", "Vol:" };
    Slider volumeSlider { Slider::LinearHorizontal, Slider::NoTextBox };

    VideoComponent videoCompWithNativeControls;
    VideoComponent videoCompNoNativeControls;
   #if DRX_IOS || DRX_MAC
    VideoComponent* curVideoComp = &videoCompWithNativeControls;
   #else
    VideoComponent* curVideoComp = &videoCompNoNativeControls;
   #endif
    b8 isFirstSetup = true;

    Slider positionSlider { Slider::LinearHorizontal, Slider::NoTextBox };
    b8 positionSliderDragging = false;
    b8 wasPlayingBeforeDragStart = false;

    Label currentPositionLabel { "currentPositionLabel", "-:- / -:-" };

    ComboBox playSpeedComboBox { "playSpeedComboBox" };
    TextButton seekToStartButton { "|<" };
    TextButton playButton { "Play" };
    TextButton pauseButton { "Pause" };
    TextButton unloadButton { "Unload" };

    std::unique_ptr<FileChooser> fileChooser;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoDemo)
    DRX_DECLARE_WEAK_REFERENCEABLE (VideoDemo)

    //==============================================================================
    z0 setPortraitOrientationEnabled (b8 shouldBeEnabled)
    {
        auto allowedOrientations = Desktop::getInstance().getOrientationsEnabled();

        if (shouldBeEnabled)
            allowedOrientations |= Desktop::upright;
        else
            allowedOrientations &= ~Desktop::upright;

        Desktop::getInstance().setOrientationsEnabled (allowedOrientations);
    }

    z0 setTransportControlsEnabled (b8 shouldBeEnabled)
    {
        positionSlider   .setEnabled (shouldBeEnabled);
        playSpeedComboBox.setEnabled (shouldBeEnabled);
        seekToStartButton.setEnabled (shouldBeEnabled);
        playButton       .setEnabled (shouldBeEnabled);
        unloadButton     .setEnabled (shouldBeEnabled);
        pauseButton      .setEnabled (shouldBeEnabled);
    }

    z0 selectVideoFile()
    {
        fileChooser.reset (new FileChooser ("Choose a video file to open...", File::getCurrentWorkingDirectory(),
                                            "*", true));

        fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                  [this] (const FileChooser& chooser)
                                  {
                                      auto results = chooser.getURLResults();

                                      if (results.size() > 0)
                                          loadVideo (results[0]);
                                  });
    }

    z0 loadVideo (const URL& url)
    {
        unloadVideoFile();

       #if DRX_IOS || DRX_MAC
        askIfUseNativeControls (url);
       #else
        loadUrl (url);
        setupVideoComp (false);
       #endif
    }

    z0 askIfUseNativeControls (const URL& url)
    {
        auto* aw = new AlertWindow ("Choose viewer type", {}, MessageBoxIconType::NoIcon);

        aw->addButton ("Yes", 1, KeyPress (KeyPress::returnKey));
        aw->addButton ("No", 0, KeyPress (KeyPress::escapeKey));
        aw->addTextBlock ("Do you want to use the viewer with native controls?");

        auto callback = ModalCallbackFunction::forComponent (videoViewerTypeChosen, this, url);
        aw->enterModalState (true, callback, true);
    }

    static z0 videoViewerTypeChosen (i32 result, VideoDemo* owner, URL url)
    {
        if (owner != nullptr)
        {
            owner->setupVideoComp (result != 0);
            owner->loadUrl (url);
        }
    }

    z0 setupVideoComp (b8 useNativeViewerWithNativeControls)
    {
        auto* oldVideoComp = curVideoComp;

        if (useNativeViewerWithNativeControls)
            curVideoComp = &videoCompWithNativeControls;
        else
            curVideoComp = &videoCompNoNativeControls;

        if (isFirstSetup || oldVideoComp != curVideoComp)
        {
            oldVideoComp->onPlaybackStarted = nullptr;
            oldVideoComp->onPlaybackStopped = nullptr;
            oldVideoComp->onErrorOccurred   = nullptr;
            oldVideoComp->setVisible (false);

            curVideoComp->onPlaybackStarted = [this]() { processPlaybackStarted(); };
            curVideoComp->onPlaybackStopped = [this]() { processPlaybackPaused(); };
            curVideoComp->onErrorOccurred   = [this] (const Txt& errorMessage) { errorOccurred (errorMessage); };
            curVideoComp->setVisible (true);

           #if DRX_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
            oldVideoComp->onGlobalMediaVolumeChanged = nullptr;
            curVideoComp->onGlobalMediaVolumeChanged = [this]() { volumeSlider.setValue (curVideoComp->getAudioVolume(), dontSendNotification); };
           #endif
        }

        isFirstSetup = false;
    }

    z0 loadUrl (const URL& url)
    {
        curVideoComp->loadAsync (url, [this] (const URL& u, Result r) { videoLoadingFinished (u, r); });
    }

    z0 showVideoUrlPrompt()
    {
        auto* aw = new AlertWindow ("Enter URL for video to load", {}, MessageBoxIconType::NoIcon);

        aw->addButton ("OK", 1, KeyPress (KeyPress::returnKey));
        aw->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
        aw->addTextEditor ("videoUrlTextEditor", "https://www.rmp-streaming.com/media/bbb-360p.mp4");

        auto callback = ModalCallbackFunction::forComponent (videoUrlPromptClosed, this, Component::SafePointer<AlertWindow> (aw));
        aw->enterModalState (true, callback, true);
    }

    static z0 videoUrlPromptClosed (i32 result, VideoDemo* owner, Component::SafePointer<AlertWindow> aw)
    {
        if (result != 0 && owner != nullptr && aw != nullptr)
        {
            auto url = aw->getTextEditorContents ("videoUrlTextEditor");

            if (url.isNotEmpty())
                owner->loadVideo (url);
        }
    }

    z0 videoLoadingFinished (const URL& url, Result result)
    {
        ignoreUnused (url);

        if (result.wasOk())
        {
            resized(); // update to reflect the video's aspect ratio

            setTransportControlsEnabled (true);

            currentPositionLabel.setText (getPositionString (0.0, curVideoComp->getVideoDuration()), sendNotification);
            positionSlider.setValue (0.0, dontSendNotification);
            playSpeedComboBox.setSelectedId (100, dontSendNotification);
        }
        else
        {
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                              "Couldn't load the file!",
                                              result.getErrorMessage());
        }
    }

    static Txt getPositionString (f64 playPositionSeconds, f64 durationSeconds)
    {
        auto positionMs = static_cast<i32> (1000 * playPositionSeconds);
        i32 posMinutes = positionMs / 60000;
        i32 posSeconds = (positionMs % 60000) / 1000;
        i32 posMillis = positionMs % 1000;

        auto totalMs = static_cast<i32> (1000 * durationSeconds);
        i32 totMinutes = totalMs / 60000;
        i32 totSeconds = (totalMs % 60000) / 1000;
        i32 totMillis = totalMs % 1000;

        return Txt::formatted ("%02d:%02d:%03d / %02d:%02d:%03d",
                                  posMinutes, posSeconds, posMillis,
                                  totMinutes, totSeconds, totMillis);
    }

    z0 updatePositionSliderAndLabel()
    {
        auto position = curVideoComp->getPlayPosition();
        auto duration = curVideoComp->getVideoDuration();

        currentPositionLabel.setText (getPositionString (position, duration), sendNotification);

        if (! positionSliderDragging)
            positionSlider.setValue (approximatelyEqual (duration, 0.0) ? 0.0 : (position / duration), dontSendNotification);
    }

    z0 seekVideoToStart()
    {
        seekVideoToNormalisedPosition (0.0);
    }

    z0 seekVideoToNormalisedPosition (f64 normalisedPos)
    {
        normalisedPos = jlimit (0.0, 1.0, normalisedPos);

        auto duration = curVideoComp->getVideoDuration();
        auto newPos = jlimit (0.0, duration, duration * normalisedPos);

        curVideoComp->setPlayPosition (newPos);
        currentPositionLabel.setText (getPositionString (newPos, curVideoComp->getVideoDuration()), sendNotification);
        positionSlider.setValue (normalisedPos, dontSendNotification);
    }

    z0 playVideo()
    {
        curVideoComp->play();
    }

    z0 processPlaybackStarted()
    {
        playButton.setVisible (false);
        pauseButton.setVisible (true);

        startTimer (20);
    }

    z0 pauseVideo()
    {
        curVideoComp->stop();
    }

    z0 processPlaybackPaused()
    {
        // On seeking to a new pos, the playback may be temporarily paused.
        if (positionSliderDragging)
            return;

        pauseButton.setVisible (false);
        playButton.setVisible (true);
    }

    z0 errorOccurred (const Txt& errorMessage)
    {
        AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon,
                                          "An error has occurred",
                                          errorMessage + ", video will be unloaded.");

        unloadVideoFile();
    }

    z0 unloadVideoFile()
    {
        curVideoComp->closeVideo();

        setTransportControlsEnabled (false);
        stopTimer();

        pauseButton.setVisible (false);
        playButton.setVisible (true);

        currentPositionLabel.setText ("-:- / -:-", sendNotification);
        positionSlider.setValue (0.0, dontSendNotification);
    }

    z0 timerCallback() override
    {
        updatePositionSliderAndLabel();
    }
};
#elif DRX_LINUX || DRX_BSD
 #error "This demo не поддерживается on Linux!"
#endif
