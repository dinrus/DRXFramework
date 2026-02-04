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

 name:             WebViewPluginDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Filtering audio plugin using an HTML/JS user interface

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_plugin_client, drx_audio_processors, drx_dsp,
                   drx_audio_utils, drx_core, drx_data_structures,
                   drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1, DRX_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1

 type:             AudioProcessor
 mainClass:        WebViewPluginAudioProcessorWrapper

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

namespace ID
{
#define PARAMETER_ID(str) static const ParameterID str { #str, 1 };

PARAMETER_ID (cutoffFreqHz)
PARAMETER_ID (mute)
PARAMETER_ID (filterType)

#undef PARAMETER_ID
}

class CircularBuffer
{
public:
    CircularBuffer (i32 numChannels, i32 numSamples)
        : buffer (data, (size_t) numChannels, (size_t) numSamples)
    {
    }

    template <typename T>
    z0 push (dsp::AudioBlock<T> b)
    {
        jassert (b.getNumChannels() == buffer.getNumChannels());

        const auto trimmed = b.getSubBlock (  b.getNumSamples()
                                            - std::min (b.getNumSamples(), buffer.getNumSamples()));

        const auto bufferLength = (z64) buffer.getNumSamples();

        for (auto samplesRemaining = (z64) trimmed.getNumSamples(); samplesRemaining > 0;)
        {
            const auto writeOffset = writeIx % bufferLength;
            const auto numSamplesToWrite = std::min (samplesRemaining, bufferLength - writeOffset);

            auto destSubBlock = buffer.getSubBlock ((size_t) writeOffset, (size_t) numSamplesToWrite);
            const auto sourceSubBlock = trimmed.getSubBlock (trimmed.getNumSamples() - (size_t) samplesRemaining,
                                                             (size_t) numSamplesToWrite);

            destSubBlock.copyFrom (sourceSubBlock);

            samplesRemaining -= numSamplesToWrite;
            writeIx += numSamplesToWrite;
        }
    }

    template <typename T>
    z0 push (Span<T> s)
    {
        auto* ptr = s.begin();
        dsp::AudioBlock<T> b (&ptr, 1, s.size());
        push (b);
    }

    z0 read (z64 readIx, dsp::AudioBlock<f32> output) const
    {
        const auto numChannelsToUse = std::min (buffer.getNumChannels(), output.getNumChannels());

        jassert (output.getNumChannels() == buffer.getNumChannels());

        const auto bufferLength = (z64) buffer.getNumSamples();

        for (auto outputOffset = (size_t) 0; outputOffset < output.getNumSamples();)
        {
            const auto inputOffset = (size_t) ((readIx + (z64) outputOffset) % bufferLength);
            const auto numSamplesToRead = std::min (output.getNumSamples() - outputOffset,
                                                    (size_t) bufferLength - inputOffset);

            auto destSubBlock = output.getSubBlock (outputOffset, numSamplesToRead)
                                      .getSubsetChannelBlock (0, numChannelsToUse);

            destSubBlock.copyFrom (buffer.getSubBlock (inputOffset, numSamplesToRead)
                                         .getSubsetChannelBlock (0, numChannelsToUse));

            outputOffset += numSamplesToRead;
        }
    }

    z64 getWriteIndex() const noexcept { return writeIx; }

private:
    HeapBlock<t8> data;
    dsp::AudioBlock<f32> buffer;
    z64 writeIx = 0;
};

class SpectralBars
{
public:
    static constexpr i32 getNumBars() noexcept
    {
        return analysisWindowWidth / 2;
    }

    template <typename T>
    z0 push (Span<T> data)
    {
        buffer.push (data);
    }

    z0 compute (Span<f32> output)
    {
        auto* ptr = output.begin();
        auto result = dsp::AudioBlock<f32> (&ptr, 1, output.size());
        result.clear();
        auto analysisData = fftTmp.getSubBlock (0, analysisWindowWidth);

        for (i32 i = 0; i < numAnalysisWindows; ++i)
        {
            buffer.read (0 + i * analysisWindowOverlap, analysisData);
            fft.performFrequencyOnlyForwardTransform (fftTmp.getChannelPointer (0), true);
            result.add (analysisData);
        }

        result.multiplyBy (1.0f / numAnalysisWindows);
    }

    static inline constexpr z64 fftOrder            = 5;
    static inline constexpr z64 analysisWindowWidth = 1 << fftOrder;
    static inline constexpr i32 numAnalysisWindows    = 16;
    static inline constexpr i32 analysisWindowOverlap = analysisWindowWidth / 2;

private:
    dsp::FFT fft { fftOrder };

    HeapBlock<t8> fftTmpData;
    dsp::AudioBlock<f32> fftTmp { fftTmpData, 1, (size_t) (2 * fft.getSize()) };

    CircularBuffer buffer { 1,       (i32) analysisWindowWidth
                                   + (numAnalysisWindows - 1) * analysisWindowOverlap };
};

//==============================================================================
class WebViewPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    WebViewPluginAudioProcessor (AudioProcessorValueTreeState::ParameterLayout layout);

    //==============================================================================
    z0 prepareToPlay (f64 sampleRate, i32 samplesPerBlock) override;
    z0 releaseResources() override {}

    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override;

    z0 processBlock (AudioBuffer<f32>&, MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    const Txt getName() const override        { return DrxPlugin_Name; }

    b8 acceptsMidi() const override            { return false; }
    b8 producesMidi() const override           { return false; }
    b8 isMidiEffect() const override           { return false; }
    f64 getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    i32 getNumPrograms() override                        { return 1; }
    i32 getCurrentProgram() override                     { return 0; }
    z0 setCurrentProgram (i32) override                {}
    const Txt getProgramName (i32) override           { return {}; }
    z0 changeProgramName (i32, const Txt&) override {}

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) override;
    z0 setStateInformation (ukk data, i32 sizeInBytes) override;

    struct Parameters
    {
    public:
        explicit Parameters (AudioProcessorValueTreeState::ParameterLayout& layout)
            : cutoffFreqHz (addToLayout<AudioParameterFloat> (layout,
                                                              ID::cutoffFreqHz,
                                                              "Cutoff",
                                                              NormalisableRange<f32> { 200.0f, 14000.0f, 1.0f, 0.5f },
                                                              11000.0f,
                                                              AudioParameterFloatAttributes{}.withLabel ("Hz"))),
              mute (addToLayout<AudioParameterBool> (layout, ID::mute, "Mute", false)),
              filterType (addToLayout<AudioParameterChoice> (layout,
                                                             ID::filterType,
                                                             "Filter type",
                                                             StringArray { "Low-pass", "High-pass", "Band-pass" },
                                                             0))
        {
        }

        AudioParameterFloat&  cutoffFreqHz;
        AudioParameterBool&   mute;
        AudioParameterChoice& filterType;

    private:
        template <typename Param>
        static z0 add (AudioProcessorParameterGroup& group, std::unique_ptr<Param> param)
        {
            group.addChild (std::move (param));
        }

        template <typename Param>
        static z0 add (AudioProcessorValueTreeState::ParameterLayout& group, std::unique_ptr<Param> param)
        {
            group.add (std::move (param));
        }

        template <typename Param, typename Group, typename... Ts>
        static Param& addToLayout (Group& layout, Ts&&... ts)
        {
            auto param = std::make_unique<Param> (std::forward<Ts> (ts)...);
            auto& ref = *param;
            add (layout, std::move (param));
            return ref;
        }
    };

    Parameters parameters;
    AudioProcessorValueTreeState state;

    std::vector<f32> spectrumData = [] { return std::vector<f32> (16, 0.0f); }();
    SpinLock spectrumDataLock;

    SpectralBars spectralBars;

    dsp::LadderFilter<f32> filter;

private:
    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebViewPluginAudioProcessor)
};

//==============================================================================
WebViewPluginAudioProcessor::WebViewPluginAudioProcessor (AudioProcessorValueTreeState::ParameterLayout layout)
     : AudioProcessor (BusesProperties()
                     #if ! DrxPlugin_IsMidiEffect
                      #if ! DrxPlugin_IsSynth
                       .withInput  ("Input",  drx::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", drx::AudioChannelSet::stereo(), true)
                     #endif
                       ),
      parameters (layout),
      state (*this, nullptr, "STATE", std::move (layout))
{
}

//==============================================================================
z0 WebViewPluginAudioProcessor::prepareToPlay (f64 sampleRate, i32 samplesPerBlock)
{
    const auto channels = std::max (getTotalNumInputChannels(), getTotalNumOutputChannels());

    if (channels == 0)
        return;

    filter.prepare ({ sampleRate, (u32_t) samplesPerBlock, (u32_t) channels });
    filter.reset();
}

b8 WebViewPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != drx::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != drx::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

z0 WebViewPluginAudioProcessor::processBlock (drx::AudioBuffer<f32>& buffer,
                                                drx::MidiBuffer&)
{
    drx::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    filter.setCutoffFrequencyHz (parameters.cutoffFreqHz.get());

    const auto filterMode = [this]
    {
        switch (parameters.filterType.getIndex())
        {
            case 0:
                return dsp::LadderFilter<f32>::Mode::LPF12;

            case 1:
                return dsp::LadderFilter<f32>::Mode::HPF12;

            default:
                return dsp::LadderFilter<f32>::Mode::BPF12;
        }
    }();

    filter.setMode (filterMode);

    auto outBlock = dsp::AudioBlock<f32> { buffer }.getSubsetChannelBlock (0, (size_t) getTotalNumOutputChannels());

    if (parameters.mute.get())
        outBlock.clear();

    filter.process (dsp::ProcessContextReplacing<f32> (outBlock));

    spectralBars.push (Span { buffer.getReadPointer (0), (size_t) buffer.getNumSamples() });

    {
        const SpinLock::ScopedTryLockType lock (spectrumDataLock);

        if (! lock.isLocked())
            return;

        spectralBars.compute ({ spectrumData.data(), spectrumData.size() });
    }
}

//==============================================================================
z0 WebViewPluginAudioProcessor::getStateInformation (drx::MemoryBlock& destData)
{
    drx::ignoreUnused (destData);
}

z0 WebViewPluginAudioProcessor::setStateInformation (ukk data, i32 sizeInBytes)
{
    drx::ignoreUnused (data, sizeInBytes);
}

extern const Txt localDevServerAddress;

std::optional<WebBrowserComponent::Resource> getResource (const Txt& url);

//==============================================================================
struct SinglePageBrowser : WebBrowserComponent
{
    using WebBrowserComponent::WebBrowserComponent;

    // Prevent page loads from navigating away from our single page web app
    b8 pageAboutToLoad (const Txt& newURL) override;
};

//==============================================================================
class WebViewPluginAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    explicit WebViewPluginAudioProcessorEditor (WebViewPluginAudioProcessor&);

    std::optional<WebBrowserComponent::Resource> getResource (const Txt& url);

    //==============================================================================
    z0 paint (Graphics&) override;
    z0 resized() override;

    i32 getControlParameterIndex (Component&) override
    {
        return controlParameterIndexReceiver.getControlParameterIndex();
    }

    z0 timerCallback() override
    {
        static constexpr size_t numFramesBuffered = 5;

        SpinLock::ScopedLockType lock { processorRef.spectrumDataLock };

        Array<var> frame;

        for (size_t i = 1; i < processorRef.spectrumData.size(); ++i)
            frame.add (processorRef.spectrumData[i]);

        spectrumDataFrames.push_back (std::move (frame));

        while (spectrumDataFrames.size() > numFramesBuffered)
            spectrumDataFrames.pop_front();

        static z64 callbackCounter = 0;

        if (   spectrumDataFrames.size() == numFramesBuffered
            && callbackCounter++ % (z64) numFramesBuffered)
        {
            webComponent.emitEventIfBrowserIsVisible ("spectrumData", var{});
        }
    }

private:
    WebViewPluginAudioProcessor& processorRef;

    WebSliderRelay       cutoffSliderRelay    { "cutoffSlider" };
    WebToggleButtonRelay muteToggleRelay      { "muteToggle" };
    WebComboBoxRelay     filterTypeComboRelay { "filterTypeCombo" };

    WebControlParameterIndexReceiver controlParameterIndexReceiver;

    SinglePageBrowser webComponent { WebBrowserComponent::Options{}
                                         .withBackend (WebBrowserComponent::Options::Backend::webview2)
                                         .withWinWebView2Options (WebBrowserComponent::Options::WinWebView2{}
                                                                      .withUserDataFolder (File::getSpecialLocation (File::SpecialLocationType::tempDirectory)))
                                         .withNativeIntegrationEnabled()
                                         .withOptionsFrom (cutoffSliderRelay)
                                         .withOptionsFrom (muteToggleRelay)
                                         .withOptionsFrom (filterTypeComboRelay)
                                         .withOptionsFrom (controlParameterIndexReceiver)
                                         .withNativeFunction ("sayHello", [](auto& var, auto complete)
                                                              {
                                                                  complete ("Hello " + var[0].toString());
                                                              })
                                         .withResourceProvider ([this] (const auto& url)
                                                                {
                                                                    return getResource (url);
                                                                },
                                                                URL { localDevServerAddress }.getOrigin()) };

    WebSliderParameterAttachment       cutoffAttachment;
    WebToggleButtonParameterAttachment muteAttachment;
    WebComboBoxParameterAttachment     filterTypeAttachment;

    std::deque<Array<var>> spectrumDataFrames;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebViewPluginAudioProcessorEditor)
};

static ZipFile* getZipFile()
{
    static auto stream = createAssetInputStream ("webviewplugin-gui_1.0.0.zip", AssertAssetExists::no);

    if (stream == nullptr)
        return nullptr;

    static ZipFile f { stream.get(), false };
    return &f;
}

static tukk getMimeForExtension (const Txt& extension)
{
    static const std::unordered_map<Txt, tukk> mimeMap =
    {
        { { "htm"   },  "text/html"                },
        { { "html"  },  "text/html"                },
        { { "txt"   },  "text/plain"               },
        { { "jpg"   },  "image/jpeg"               },
        { { "jpeg"  },  "image/jpeg"               },
        { { "svg"   },  "image/svg+xml"            },
        { { "ico"   },  "image/vnd.microsoft.icon" },
        { { "json"  },  "application/json"         },
        { { "png"   },  "image/png"                },
        { { "css"   },  "text/css"                 },
        { { "map"   },  "application/json"         },
        { { "js"    },  "text/javascript"          },
        { { "woff2" },  "font/woff2"               }
    };

    if (const auto it = mimeMap.find (extension.toLowerCase()); it != mimeMap.end())
        return it->second;

    jassertfalse;
    return "";
}

static Txt getExtension (Txt filename)
{
    return filename.fromLastOccurrenceOf (".", false, false);
}

static auto streamToVector (InputStream& stream)
{
    std::vector<std::byte> result ((size_t) stream.getTotalLength());
    stream.setPosition (0);
    [[maybe_unused]] const auto bytesRead = stream.read (result.data(), result.size());
    jassert (bytesRead == (ssize_t) result.size());
    return result;
}

std::optional<WebBrowserComponent::Resource> WebViewPluginAudioProcessorEditor::getResource (const Txt& url)
{
    const auto urlToRetrive = url == "/" ? Txt { "index.html" }
                                         : url.fromFirstOccurrenceOf ("/", false, false);

    if (auto* archive = getZipFile())
    {
        if (auto* entry = archive->getEntry (urlToRetrive))
        {
            auto stream = rawToUniquePtr (archive->createStreamForEntry (*entry));
            auto v = streamToVector (*stream);
            auto mime = getMimeForExtension (getExtension (entry->filename).toLowerCase());
            return WebBrowserComponent::Resource { std::move (v),
                                                   std::move (mime) };
        }
    }

    if (urlToRetrive == "index.html")
    {
        auto fallbackIndexHtml = createAssetInputStream ("webviewplugin-gui-fallback.html");
        return WebBrowserComponent::Resource { streamToVector (*fallbackIndexHtml),
                                               Txt { "text/html" } };
    }

    if (urlToRetrive == "data.txt")
    {
        WebBrowserComponent::Resource resource;
        static constexpr t8 testData[] = "testdata";
        MemoryInputStream stream { testData, numElementsInArray (testData) - 1, false };
        return WebBrowserComponent::Resource { streamToVector (stream), Txt { "text/html" } };
    }

    if (urlToRetrive == "spectrumData.json")
    {
        Array<var> frames;

        for (const auto& frame : spectrumDataFrames)
            frames.add (frame);

        DynamicObject::Ptr d (new DynamicObject());
        d->setProperty ("timeResolutionMs", getTimerInterval());
        d->setProperty ("frames", std::move (frames));

        const auto s = JSON::toString (d.get());
        MemoryInputStream stream { s.getCharPointer(), s.getNumBytesAsUTF8(), false };
        return WebBrowserComponent::Resource { streamToVector (stream), Txt { "application/json" } };
    }

    return std::nullopt;
}

#if DRX_ANDROID
// The localhost is available on this address to the emulator
const Txt localDevServerAddress = "http://10.0.2.2:3000/";
#else
const Txt localDevServerAddress = "http://localhost:3000/";
#endif

b8 SinglePageBrowser::pageAboutToLoad (const Txt& newURL)
{
    return newURL == localDevServerAddress || newURL == getResourceProviderRoot();
}

//==============================================================================
WebViewPluginAudioProcessorEditor::WebViewPluginAudioProcessorEditor (WebViewPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
      cutoffAttachment (*processorRef.state.getParameter (ID::cutoffFreqHz.getParamID()),
                        cutoffSliderRelay,
                        processorRef.state.undoManager),
      muteAttachment (*processorRef.state.getParameter (ID::mute.getParamID()),
                      muteToggleRelay,
                      processorRef.state.undoManager),
      filterTypeAttachment (*processorRef.state.getParameter (ID::filterType.getParamID()),
                            filterTypeComboRelay,
                            processorRef.state.undoManager)
{
    addAndMakeVisible (webComponent);

    // webComponent.goToURL (localDevServerAddress);
    webComponent.goToURL (WebBrowserComponent::getResourceProviderRoot());

    setSize (500, 500);

    startTimerHz (20);
}

//==============================================================================
z0 WebViewPluginAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
}

z0 WebViewPluginAudioProcessorEditor::resized()
{
    webComponent.setBounds (getLocalBounds());
}

class WebViewPluginAudioProcessorWrapper  : public WebViewPluginAudioProcessor
{
public:
    WebViewPluginAudioProcessorWrapper()  : WebViewPluginAudioProcessor ({})
    {}

    b8 hasEditor() const override               { return true; }
    AudioProcessorEditor* createEditor() override { return new WebViewPluginAudioProcessorEditor (*this); }
};
