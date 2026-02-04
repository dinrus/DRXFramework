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

#pragma once

using namespace dsp;

//==============================================================================
struct DSPDemoParameterBase    : public ChangeBroadcaster
{
    DSPDemoParameterBase (const Txt& labelName) : name (labelName) {}
    virtual ~DSPDemoParameterBase() = default;

    virtual Component* getComponent() = 0;

    virtual i32 getPreferredHeight()  = 0;
    virtual i32 getPreferredWidth()   = 0;

    Txt name;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DSPDemoParameterBase)
};

//==============================================================================
struct SliderParameter final : public DSPDemoParameterBase
{
    SliderParameter (Range<f64> range, f64 skew, f64 initialValue,
                     const Txt& labelName, const Txt& suffix = {})
        : DSPDemoParameterBase (labelName)
    {
        slider.setRange (range.getStart(), range.getEnd(), 0.01);
        slider.setSkewFactor (skew);
        slider.setValue (initialValue);

        if (suffix.isNotEmpty())
            slider.setTextValueSuffix (suffix);

        slider.onValueChange = [this] { sendChangeMessage(); };
    }

    Component* getComponent() override    { return &slider; }

    i32 getPreferredHeight() override     { return 40; }
    i32 getPreferredWidth()  override     { return 500; }

    f64 getCurrentValue() const        { return slider.getValue(); }

private:
    Slider slider;
};

//==============================================================================
struct ChoiceParameter final : public DSPDemoParameterBase
{
    ChoiceParameter (const StringArray& options, i32 initialId, const Txt& labelName)
        : DSPDemoParameterBase (labelName)
    {
        parameterBox.addItemList (options, 1);
        parameterBox.onChange = [this] { sendChangeMessage(); };

        parameterBox.setSelectedId (initialId);
    }

    Component* getComponent() override    { return &parameterBox; }

    i32 getPreferredHeight() override     { return 25; }
    i32 getPreferredWidth()  override     { return 250; }

    i32 getCurrentSelectedID() const      { return parameterBox.getSelectedId(); }

private:
    ComboBox parameterBox;
};

//==============================================================================
class AudioThumbnailComponent final : public Component,
                                      public FileDragAndDropTarget,
                                      public ChangeBroadcaster,
                                      private ChangeListener,
                                      private Timer
{
public:
    AudioThumbnailComponent (AudioDeviceManager& adm, AudioFormatManager& afm)
        : audioDeviceManager (adm),
          thumbnailCache (5),
          thumbnail (128, afm, thumbnailCache)
    {
        thumbnail.addChangeListener (this);
    }

    ~AudioThumbnailComponent() override
    {
        thumbnail.removeChangeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Color (0xff495358));

        g.setColor (Colors::white);

        if (thumbnail.getTotalLength() > 0.0)
        {
            thumbnail.drawChannels (g, getLocalBounds().reduced (2),
                                    0.0, thumbnail.getTotalLength(), 1.0f);

            g.setColor (Colors::black);
            g.fillRect (static_cast<f32> (currentPosition * getWidth()), 0.0f,
                        1.0f, static_cast<f32> (getHeight()));
        }
        else
        {
            g.drawFittedText ("No audio file loaded.\nDrop a file here or click the \"Load File...\" button.", getLocalBounds(),
                              Justification::centred, 2);
        }
    }

    b8 isInterestedInFileDrag (const StringArray&) override          { return true; }
    z0 filesDropped (const StringArray& files, i32, i32) override    { loadURL (URL (File (files[0])), true); }

    z0 setCurrentURL (const URL& u)
    {
        if (currentURL == u)
            return;

        loadURL (u);
    }

    URL getCurrentURL() const   { return currentURL; }

    z0 setTransportSource (AudioTransportSource* newSource)
    {
        transportSource = newSource;

        struct ResetCallback final : public CallbackMessage
        {
            ResetCallback (AudioThumbnailComponent& o) : owner (o) {}
            z0 messageCallback() override    { owner.reset(); }

            AudioThumbnailComponent& owner;
        };

        (new ResetCallback (*this))->post();
    }

private:
    AudioDeviceManager& audioDeviceManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    AudioTransportSource* transportSource = nullptr;

    URL currentURL;
    f64 currentPosition = 0.0;

    //==============================================================================
    z0 changeListenerCallback (ChangeBroadcaster*) override    { repaint(); }

    z0 reset()
    {
        currentPosition = 0.0;
        repaint();

        if (transportSource == nullptr)
            stopTimer();
        else
            startTimerHz (25);
    }

    z0 loadURL (const URL& u, b8 notify = false)
    {
        if (currentURL == u)
            return;

        currentURL = u;

        thumbnail.setSource (makeInputSource (u).release());

        if (notify)
            sendChangeMessage();
    }

    z0 timerCallback() override
    {
        if (transportSource != nullptr)
        {
            currentPosition = transportSource->getCurrentPosition() / thumbnail.getTotalLength();
            repaint();
        }
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        if (transportSource != nullptr)
        {
            const ScopedLock sl (audioDeviceManager.getAudioCallbackLock());

            transportSource->setPosition ((jmax (static_cast<f64> (e.x), 0.0) / getWidth())
                                            * thumbnail.getTotalLength());
        }
    }
};

//==============================================================================
class DemoParametersComponent final : public Component
{
public:
    DemoParametersComponent (const std::vector<DSPDemoParameterBase*>& demoParams)
    {
        parameters = demoParams;

        for (auto demoParameter : parameters)
        {
            addAndMakeVisible (demoParameter->getComponent());

            auto* paramLabel = new Label ({}, demoParameter->name);

            paramLabel->attachToComponent (demoParameter->getComponent(), true);
            paramLabel->setJustificationType (Justification::centredLeft);
            addAndMakeVisible (paramLabel);
            labels.add (paramLabel);
        }
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft (100);

        for (auto* p : parameters)
        {
            auto* comp = p->getComponent();

            comp->setSize (jmin (bounds.getWidth(), p->getPreferredWidth()), p->getPreferredHeight());

            auto compBounds = bounds.removeFromTop (p->getPreferredHeight());
            comp->setCentrePosition (compBounds.getCentre());
        }
    }

    i32 getHeightNeeded()
    {
        auto height = 0;

        for (auto* p : parameters)
            height += p->getPreferredHeight();

        return height + 10;
    }

private:
    std::vector<DSPDemoParameterBase*> parameters;
    OwnedArray<Label> labels;
};

//==============================================================================
template <class DemoType>
struct DSPDemo final : public AudioSource,
                       public ProcessorWrapper<DemoType>,
                       private ChangeListener
{
    DSPDemo (AudioSource& input)
        : inputSource (&input)
    {
        for (auto* p : getParameters())
            p->addChangeListener (this);
    }

    z0 prepareToPlay (i32 blockSize, f64 sampleRate) override
    {
        inputSource->prepareToPlay (blockSize, sampleRate);
        this->prepare ({ sampleRate, (u32) blockSize, 2 });
    }

    z0 releaseResources() override
    {
        inputSource->releaseResources();
    }

    z0 getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (bufferToFill.buffer == nullptr)
        {
            jassertfalse;
            return;
        }

        inputSource->getNextAudioBlock (bufferToFill);

        AudioBlock<f32> block (*bufferToFill.buffer,
                                 (size_t) bufferToFill.startSample);

        ScopedLock audioLock (audioCallbackLock);
        this->process (ProcessContextReplacing<f32> (block));
    }

    const std::vector<DSPDemoParameterBase*>& getParameters()
    {
        return this->processor.parameters;
    }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        ScopedLock audioLock (audioCallbackLock);
        static_cast<DemoType&> (this->processor).updateParameters();
    }

    CriticalSection audioCallbackLock;

    AudioSource* inputSource;
};

//==============================================================================
template <class DemoType>
class AudioFileReaderComponent final : public Component,
                                       private TimeSliceThread,
                                       private Value::Listener,
                                       private ChangeListener
{
public:
    //==============================================================================
    AudioFileReaderComponent()
        : TimeSliceThread ("Audio File Reader Thread"),
          header (audioDeviceManager, formatManager, *this)
    {
        loopState.addListener (this);

        formatManager.registerBasicFormats();
        audioDeviceManager.addAudioCallback (&audioSourcePlayer);

       #ifndef DRX_DEMO_RUNNER
        audioDeviceManager.initialiseWithDefaultDevices (0, 2);
       #endif

        init();
        startThread();

        setOpaque (true);

        addAndMakeVisible (header);

        setSize (800, 250);
    }

    ~AudioFileReaderComponent() override
    {
        signalThreadShouldExit();
        stop();
        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
        waitForThreadToExit (10000);
    }

    z0 paint (Graphics& g) override
    {
        g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
        g.fillRect (getLocalBounds());
    }

    z0 resized() override
    {
        auto r = getLocalBounds();

        header.setBounds (r.removeFromTop (120));

        r.removeFromTop (20);

        if (parametersComponent != nullptr)
            parametersComponent->setBounds (r.removeFromTop (parametersComponent->getHeightNeeded()).reduced (20, 0));
    }

    //==============================================================================
    b8 loadURL (const URL& fileToPlay)
    {
        stop();

        audioSourcePlayer.setSource (nullptr);
        getThumbnailComponent().setTransportSource (nullptr);
        transportSource.reset();
        readerSource.reset();

        auto source = makeInputSource (fileToPlay);

        if (source == nullptr)
            return false;

        auto stream = rawToUniquePtr (source->createInputStream());

        if (stream == nullptr)
            return false;

        reader = rawToUniquePtr (formatManager.createReaderFor (std::move (stream)));

        if (reader == nullptr)
            return false;

        readerSource.reset (new AudioFormatReaderSource (reader.get(), false));
        readerSource->setLooping (loopState.getValue());

        init();
        resized();

        return true;
    }

    z0 togglePlay()
    {
        if (playState.getValue())
            stop();
        else
            play();
    }

    z0 stop()
    {
        playState = false;

        if (transportSource.get() != nullptr)
        {
            transportSource->stop();
            transportSource->setPosition (0);
        }
    }

    z0 init()
    {
        if (transportSource.get() == nullptr)
        {
            transportSource.reset (new AudioTransportSource());
            transportSource->addChangeListener (this);

            if (readerSource != nullptr)
            {
                if (auto* device = audioDeviceManager.getCurrentAudioDevice())
                {
                    transportSource->setSource (readerSource.get(), roundToInt (device->getCurrentSampleRate()), this, reader->sampleRate);

                    getThumbnailComponent().setTransportSource (transportSource.get());
                }
            }
        }

        audioSourcePlayer.setSource (nullptr);
        currentDemo.reset();

        if (currentDemo.get() == nullptr)
            currentDemo.reset (new DSPDemo<DemoType> (*transportSource));

        audioSourcePlayer.setSource (currentDemo.get());

        auto& parameters = currentDemo->getParameters();

        parametersComponent.reset();

        if (! parameters.empty())
        {
            parametersComponent = std::make_unique<DemoParametersComponent> (parameters);
            addAndMakeVisible (parametersComponent.get());
        }
    }

    z0 play()
    {
        if (readerSource == nullptr)
            return;

        if (transportSource->getCurrentPosition() >= transportSource->getLengthInSeconds()
             || transportSource->getCurrentPosition() < 0)
            transportSource->setPosition (0);

        transportSource->start();
        playState = true;
    }

    z0 setLooping (b8 shouldLoop)
    {
        if (readerSource != nullptr)
            readerSource->setLooping (shouldLoop);
    }

    AudioThumbnailComponent& getThumbnailComponent()    { return header.thumbnailComp; }

private:
    //==============================================================================
    class AudioPlayerHeader final : public Component,
                                    private ChangeListener,
                                    private Value::Listener
    {
    public:
        AudioPlayerHeader (AudioDeviceManager& adm,
                           AudioFormatManager& afm,
                           AudioFileReaderComponent& afr)
            : thumbnailComp (adm, afm),
              audioFileReader (afr)
        {
            setOpaque (true);

            addAndMakeVisible (loadButton);
            addAndMakeVisible (playButton);
            addAndMakeVisible (loopButton);

            playButton.setColor (TextButton::buttonColorId, Color (0xff79ed7f));
            playButton.setColor (TextButton::textColorOffId, Colors::black);

            loadButton.setColor (TextButton::buttonColorId, Color (0xff797fed));
            loadButton.setColor (TextButton::textColorOffId, Colors::black);

            loadButton.onClick = [this] { openFile(); };
            playButton.onClick = [this] { audioFileReader.togglePlay(); };

            addAndMakeVisible (thumbnailComp);
            thumbnailComp.addChangeListener (this);

            audioFileReader.playState.addListener (this);
            loopButton.getToggleStateValue().referTo (audioFileReader.loopState);
        }

        ~AudioPlayerHeader() override
        {
            audioFileReader.playState.removeListener (this);
        }

        z0 paint (Graphics& g) override
        {
            g.setColor (getLookAndFeel().findColor (ResizableWindow::backgroundColorId).darker());
            g.fillRect (getLocalBounds());
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds();

            auto buttonBounds = bounds.removeFromLeft (jmin (250, bounds.getWidth() / 4));
            auto loopBounds = buttonBounds.removeFromBottom (30);

            loadButton.setBounds (buttonBounds.removeFromTop (buttonBounds.getHeight() / 2));
            playButton.setBounds (buttonBounds);

            loopButton.setSize (0, 25);
            loopButton.changeWidthToFitText();
            loopButton.setCentrePosition (loopBounds.getCentre());

            thumbnailComp.setBounds (bounds);
        }

        AudioThumbnailComponent thumbnailComp;

    private:
        //==============================================================================
        z0 openFile()
        {
            audioFileReader.stop();

            if (fileChooser != nullptr)
                return;

            if (! RuntimePermissions::isGranted (RuntimePermissions::readExternalStorage))
            {
                SafePointer<AudioPlayerHeader> safeThis (this);
                RuntimePermissions::request (RuntimePermissions::readExternalStorage,
                                             [safeThis] (b8 granted) mutable
                                             {
                                                 if (safeThis != nullptr && granted)
                                                     safeThis->openFile();
                                             });
                return;
            }

            fileChooser.reset (new FileChooser ("Select an audio file...", File(), "*.wav;*.mp3;*.aif"));

            fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                      [this] (const FileChooser& fc) mutable
                                      {
                                          if (fc.getURLResults().size() > 0)
                                          {
                                              const auto u = fc.getURLResult();

                                              if (! audioFileReader.loadURL (u))
                                              {
                                                  auto options = MessageBoxOptions().withIconType (MessageBoxIconType::WarningIcon)
                                                                                    .withTitle ("Error loading file")
                                                                                    .withMessage ("Unable to load audio file")
                                                                                    .withButton ("OK");
                                                  messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
                                              }
                                              else
                                              {
                                                  thumbnailComp.setCurrentURL (u);
                                              }
                                          }

                                          fileChooser = nullptr;
                                      }, nullptr);
        }

        z0 changeListenerCallback (ChangeBroadcaster*) override
        {
            if (audioFileReader.playState.getValue())
                audioFileReader.stop();

            audioFileReader.loadURL (thumbnailComp.getCurrentURL());
        }

        z0 valueChanged (Value& v) override
        {
            playButton.setButtonText (v.getValue() ? "Stop" : "Play");
            playButton.setColor (TextButton::buttonColorId, v.getValue() ? Color (0xffed797f) : Color (0xff79ed7f));
        }

        //==============================================================================
        TextButton loadButton { "Load File..." }, playButton { "Play" };
        ToggleButton loopButton { "Loop File" };

        AudioFileReaderComponent& audioFileReader;
        std::unique_ptr<FileChooser> fileChooser;
        ScopedMessageBox messageBox;
    };

    //==============================================================================
    z0 valueChanged (Value& v) override
    {
        if (readerSource != nullptr)
            readerSource->setLooping (v.getValue());
    }

    z0 changeListenerCallback (ChangeBroadcaster*) override
    {
        if (playState.getValue() && ! transportSource->isPlaying())
            stop();
    }

    //==============================================================================
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef DRX_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    AudioFormatManager formatManager;
    Value playState { var (false) };
    Value loopState { var (false) };

    f64 currentSampleRate = 44100.0;
    u32 currentBlockSize = 512;
    u32 currentNumChannels = 2;

    std::unique_ptr<AudioFormatReader> reader;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    std::unique_ptr<AudioTransportSource> transportSource;
    std::unique_ptr<DSPDemo<DemoType>> currentDemo;

    AudioSourcePlayer audioSourcePlayer;

    AudioPlayerHeader header;

    AudioBuffer<f32> fileReadBuffer;

    std::unique_ptr<DemoParametersComponent> parametersComponent;
};
