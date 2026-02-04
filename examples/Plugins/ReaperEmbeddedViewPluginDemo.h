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

 name:             ReaperEmbeddedViewDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      An audio plugin which embeds a secondary view in VST2 and
                   VST3 formats in REAPER

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_plugin_client, drx_audio_processors,
                   drx_audio_utils, drx_core, drx_data_structures,
                   drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        ReaperEmbeddedViewDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

/*  This demo shows how to use the VST2ClientExtensions and VST3ClientExtensions
    classes to provide extended functionality in compatible VST/VST3 hosts.

    If this project is built as a VST or VST3 plugin and loaded in REAPER
    6.29 or higher, it will provide an embedded level meter in the track
    control panel. To enable the embedded view, right-click on the plugin
    and select "Show embedded UI in TCP".

    The plugin's editor also include a button which can be used to toggle
    all inserts on and off.
*/

#pragma once

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wshadow-field-in-constructor",
                                     "-Wnon-virtual-dtor")

#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/ivsthostapplication.h>
#include <pluginterfaces/vst2.x/aeffect.h>

DRX_END_IGNORE_WARNINGS_GCC_LIKE

namespace reaper
{
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant",
                                         "-Wunused-parameter",
                                         "-Wnon-virtual-dtor")
    DRX_BEGIN_IGNORE_WARNINGS_MSVC (4100)

    using namespace Steinberg;
    using INT_PTR = pointer_sized_int;
    using u32 = Steinberg::u32;

    #include "extern/reaper_plugin_fx_embed.h"
    #include "extern/reaper_vst3_interfaces.h"

    //==============================================================================
    /*  These should live in a file which is guaranteed to be compiled only once
        (i.e. a .cpp file, normally). This demo is a bit special, because we know
        that this header will only be included in a single translation unit.
     */
    DEF_CLASS_IID (IReaperHostApplication)
    DEF_CLASS_IID (IReaperUIEmbedInterface)

    DRX_END_IGNORE_WARNINGS_MSVC
    DRX_END_IGNORE_WARNINGS_GCC_LIKE
}

//==============================================================================
struct EmbeddedViewListener
{
    virtual ~EmbeddedViewListener() = default;
    virtual Steinberg::TPtrInt handledEmbeddedUIMessage (i32 msg,
                                                         Steinberg::TPtrInt parm2,
                                                         Steinberg::TPtrInt parm3) = 0;

    virtual z0 setGlobalBypassFunction (z0 (*) (i32)) = 0;
};

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnon-virtual-dtor")

//==============================================================================
class EmbeddedUI final : public reaper::IReaperUIEmbedInterface
{
public:
    explicit EmbeddedUI (EmbeddedViewListener& demo) : listener (demo) {}

    Steinberg::TPtrInt embed_message (i32 msg,
                                      Steinberg::TPtrInt parm2,
                                      Steinberg::TPtrInt parm3) override
    {
        return listener.handledEmbeddedUIMessage (msg, parm2, parm3);
    }

    Steinberg::u32 PLUGIN_API addRef() override   { return ++refCount; }
    Steinberg::u32 PLUGIN_API release() override  { return --refCount; }

    Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID tuid, uk* obj) override
    {
        if (std::memcmp (tuid, iid, sizeof (Steinberg::TUID)) == 0)
        {
            ++refCount;
            *obj = this;
            return Steinberg::kResultOk;
        }

        *obj = nullptr;
        return Steinberg::kNoInterface;
    }

private:
    EmbeddedViewListener& listener;
    std::atomic<Steinberg::u32> refCount { 1 };
};

DRX_END_IGNORE_WARNINGS_GCC_LIKE

class VST2Extensions final : public VST2ClientExtensions
{
public:
    explicit VST2Extensions (EmbeddedViewListener& l)
        : listener (l) {}

    pointer_sized_int handleVstPluginCanDo (i32, pointer_sized_int, uk ptr, f32) override
    {
        if (auto* str = static_cast<tukk> (ptr))
            for (auto* key : { "hasCockosEmbeddedUI", "hasCockosExtensions" })
                if (strcmp (str, key) == 0)
                    return (pointer_sized_int) 0xbeef0000;

        return 0;
    }

    pointer_sized_int handleVstManufacturerSpecific (i32 index,
                                                     pointer_sized_int value,
                                                     uk ptr,
                                                     f32 opt) override
    {
        // The docstring at the top of reaper_plugin_fx_embed.h specifies
        // that the index will always be effEditDraw, which is now deprecated.
        if (index != __effEditDrawDeprecated)
            return 0;

        return (pointer_sized_int) listener.handledEmbeddedUIMessage ((i32) opt,
                                                                      (Steinberg::TPtrInt) value,
                                                                      (Steinberg::TPtrInt) ptr);
    }

    z0 handleVstHostCallbackAvailable (std::function<VstHostCallbackType>&& hostcb) override
    {
        t8 functionName[] = "BypassFxAllTracks";
        listener.setGlobalBypassFunction (reinterpret_cast<z0 (*) (i32)> (hostcb ((i32) 0xdeadbeef, (i32) 0xdeadf00d, 0, functionName, 0.0)));
    }

private:
    EmbeddedViewListener& listener;
};

class VST3Extensions final : public VST3ClientExtensions
{
public:
    explicit VST3Extensions (EmbeddedViewListener& l)
        : listener (l) {}

    i32 queryIEditController (const Steinberg::TUID tuid, uk* obj) override
    {
        if (embeddedUi.queryInterface (tuid, obj) == Steinberg::kResultOk)
            return Steinberg::kResultOk;

        *obj = nullptr;
        return Steinberg::kNoInterface;
    }

    z0 setIHostApplication (Steinberg::FUnknown* ptr) override
    {
        if (ptr == nullptr)
            return;

        uk objPtr = nullptr;

        if (ptr->queryInterface (reaper::IReaperHostApplication::iid, &objPtr) == Steinberg::kResultOk)
        {
            if (uk fnPtr = static_cast<reaper::IReaperHostApplication*> (objPtr)->getReaperApi ("BypassFxAllTracks"))
                listener.setGlobalBypassFunction (reinterpret_cast<z0 (*) (i32)> (fnPtr));
        }
    }

private:
    EmbeddedViewListener& listener;
    EmbeddedUI embeddedUi { listener };
};

//==============================================================================
class Editor final : public AudioProcessorEditor
{
public:
    explicit Editor (AudioProcessor& proc,
                     AudioParameterFloat& param,
                     z0 (*globalBypass) (i32))
        : AudioProcessorEditor (proc), attachment (param, slider)
    {
        addAndMakeVisible (slider);
        addAndMakeVisible (bypassButton);

        // Clicking will bypass *everything*
        bypassButton.onClick = [globalBypass] { NullCheckedInvocation::invoke (globalBypass, -1); };

        setSize (300, 80);
    }

    z0 resized() override
    {
        auto b = getLocalBounds();
        slider.setBounds (b.removeFromTop (40));
        bypassButton.setBounds (b);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::darkgrey);
    }

private:
    Slider slider;
    TextButton bypassButton { "global bypass" };
    SliderParameterAttachment attachment;
};

//==============================================================================
class ReaperEmbeddedViewDemo final : public AudioProcessor,
                                     private EmbeddedViewListener,
                                     private Timer
{
public:
    ReaperEmbeddedViewDemo()
    {
        addParameter (gain = new AudioParameterFloat ({ "gain", 1 }, "Gain", 0.0f, 1.0f, 0.5f));
        startTimerHz (60);
    }

    z0 prepareToPlay (f64, i32) override {}
    z0 reset() override {}

    z0 releaseResources() override {}

    z0 processBlock (AudioBuffer<f32>&  audio, MidiBuffer&) override { processBlockImpl (audio); }
    z0 processBlock (AudioBuffer<f64>& audio, MidiBuffer&) override { processBlockImpl (audio); }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new Editor (*this, *gain, globalBypassFn); }
    b8 hasEditor() const override               { return true;   }

    //==============================================================================
    const Txt getName() const override { return "ReaperEmbeddedViewDemo"; }

    b8 acceptsMidi()  const override { return false; }
    b8 producesMidi() const override { return false; }
    b8 isMidiEffect() const override { return false; }

    f64 getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    i32 getNumPrograms()    override { return 1; }
    i32 getCurrentProgram() override { return 0; }
    z0 setCurrentProgram (i32) override {}
    const Txt getProgramName (i32) override { return "None"; }

    z0 changeProgramName (i32, const Txt&) override {}

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream (destData, true).writeFloat (*gain);
    }

    z0 setStateInformation (ukk data, i32 sizeInBytes) override
    {
        gain->setValueNotifyingHost (MemoryInputStream (data,
                                                        static_cast<size_t> (sizeInBytes),
                                                        false).readFloat());
    }

    VST2ClientExtensions* getVST2ClientExtensions() override { return &vst2Extensions; }
    VST3ClientExtensions* getVST3ClientExtensions() override { return &vst3Extensions; }

private:
    template <typename Float>
    z0 processBlockImpl (AudioBuffer<Float>& audio)
    {
        audio.applyGain (*gain);

        const auto minMax = audio.findMinMax (0, 0, audio.getNumSamples());
        const auto newMax = (f32) std::max (std::abs (minMax.getStart()), std::abs (minMax.getEnd()));

        auto loaded = storedLevel.load();
        while (loaded < newMax && ! storedLevel.compare_exchange_weak (loaded, newMax)) {}
    }

    z0 timerCallback() override
    {
        levelToDraw = std::max (levelToDraw * 0.95f, storedLevel.exchange (0.0f));
    }

    Steinberg::TPtrInt getSizeInfo (reaper::REAPER_FXEMBED_SizeHints* sizeHints)
    {
        if (sizeHints == nullptr)
            return 0;

        sizeHints->preferred_aspect = 1 << 16;
        sizeHints->minimum_aspect   = 1 << 16;
        sizeHints->min_height = sizeHints->min_width = 50;
        sizeHints->max_height = sizeHints->max_width = 1000;
        return 1;
    }

    Steinberg::TPtrInt doPaint (reaper::REAPER_FXEMBED_IBitmap* bitmap,
                                reaper::REAPER_FXEMBED_DrawInfo* drawInfo)
    {
        if (bitmap == nullptr || drawInfo == nullptr || bitmap->getWidth() <= 0 || bitmap->getHeight() <= 0)
            return 0;

        Image img (drx::Image::PixelFormat::ARGB, bitmap->getWidth(), bitmap->getHeight(), true);
        Graphics g (img);

        g.fillAll (Colors::black);

        const auto bounds = g.getClipBounds();
        const auto corner = 3.0f;

        g.setColor (Colors::darkgrey);
        g.fillRoundedRectangle (bounds.withSizeKeepingCentre (20, bounds.getHeight() - 6).toFloat(),
                                corner);

        const auto minDb = -50.0f;
        const auto maxDb = 6.0f;
        const auto levelInDb = Decibels::gainToDecibels (levelToDraw, minDb);
        const auto fractionOfHeight = jmap (levelInDb, minDb, maxDb, 0.0f, 1.0f);
        const auto trackBounds = bounds.withSizeKeepingCentre (16, bounds.getHeight() - 10).toFloat();

        g.setColor (Colors::black);
        const auto zeroDbIndicatorY = trackBounds.proportionOfHeight (jmap (0.0f,
                                                                            minDb,
                                                                            maxDb,
                                                                            0.0f,
                                                                            1.0f));
        g.drawHorizontalLine ((i32) (trackBounds.getBottom() - zeroDbIndicatorY),
                              trackBounds.getX(),
                              trackBounds.getRight());

        g.setGradientFill (ColorGradient (Colors::darkgreen,
                                           { 0.0f, (f32) bounds.getHeight() },
                                           Colors::darkred,
                                           { 0.0f, 0.0f },
                                           false));

        g.fillRoundedRectangle (trackBounds.withHeight (trackBounds.proportionOfHeight (fractionOfHeight))
                                           .withBottomY (trackBounds.getBottom()),
                                corner);

        Image::BitmapData imgData { img, Image::BitmapData::readOnly };
        const auto pixelsWidth = imgData.pixelStride * imgData.width;

        auto* px = bitmap->getBits();
        const auto rowSpan = bitmap->getRowSpan();
        const auto numRows = bitmap->getHeight();

        for (i32 y = 0; y < numRows; ++y)
            std::memcpy (px + (y * rowSpan), imgData.getLinePointer (y), (size_t) pixelsWidth);

        return 1;
    }

    Steinberg::TPtrInt handledEmbeddedUIMessage (i32 msg,
                                                 Steinberg::TPtrInt parm2,
                                                 Steinberg::TPtrInt parm3) override
    {
        switch (msg)
        {
            case REAPER_FXEMBED_WM_IS_SUPPORTED:
                return 1;

            case REAPER_FXEMBED_WM_PAINT:
                return doPaint (reinterpret_cast<reaper::REAPER_FXEMBED_IBitmap*> (parm2),
                                reinterpret_cast<reaper::REAPER_FXEMBED_DrawInfo*> (parm3));

            case REAPER_FXEMBED_WM_GETMINMAXINFO:
                return getSizeInfo (reinterpret_cast<reaper::REAPER_FXEMBED_SizeHints*> (parm3));

            // Implementing mouse behaviour is left as an exercise for the reaper, I mean reader
            case REAPER_FXEMBED_WM_CREATE:          break;
            case REAPER_FXEMBED_WM_DESTROY:         break;
            case REAPER_FXEMBED_WM_SETCURSOR:       break;
            case REAPER_FXEMBED_WM_MOUSEMOVE:       break;
            case REAPER_FXEMBED_WM_LBUTTONDOWN:     break;
            case REAPER_FXEMBED_WM_LBUTTONUP:       break;
            case REAPER_FXEMBED_WM_LBUTTONDBLCLK:   break;
            case REAPER_FXEMBED_WM_RBUTTONDOWN:     break;
            case REAPER_FXEMBED_WM_RBUTTONUP:       break;
            case REAPER_FXEMBED_WM_RBUTTONDBLCLK:   break;
            case REAPER_FXEMBED_WM_MOUSEWHEEL:      break;
        }

        return 0;
    }

    z0 setGlobalBypassFunction (z0 (*fn) (i32)) override { globalBypassFn = fn; }

    AudioParameterFloat* gain = nullptr;
    z0 (*globalBypassFn) (i32) = nullptr;

    std::atomic<f32> storedLevel { 0.0f };
    f32 levelToDraw = 0.0f;

    VST2Extensions vst2Extensions { *this };
    VST3Extensions vst3Extensions { *this };
};
