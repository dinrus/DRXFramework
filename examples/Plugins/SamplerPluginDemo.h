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

 name:             SamplerPlugin
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Sampler audio plugin.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_plugin_client, drx_audio_processors,
                   drx_audio_utils, drx_core, drx_data_structures,
                   drx_events, drx_graphics, drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        SamplerAudioProcessor

 useLocalCopy:     1

 pluginCharacteristics: pluginIsSynth, pluginWantsMidiIn

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <tuple>
#include <iomanip>
#include <sstream>
#include <functional>
#include <mutex>

namespace IDs
{

#define DECLARE_ID(name) const drx::Identifier name (#name);

DECLARE_ID (DATA_MODEL)
DECLARE_ID (sampleReader)
DECLARE_ID (centreFrequencyHz)
DECLARE_ID (loopMode)
DECLARE_ID (loopPointsSeconds)

DECLARE_ID (MPE_SETTINGS)
DECLARE_ID (synthVoices)
DECLARE_ID (voiceStealingEnabled)
DECLARE_ID (legacyModeEnabled)
DECLARE_ID (mpeZoneLayout)
DECLARE_ID (legacyFirstChannel)
DECLARE_ID (legacyLastChannel)
DECLARE_ID (legacyPitchbendRange)

DECLARE_ID (VISIBLE_RANGE)
DECLARE_ID (totalRange)
DECLARE_ID (visibleRange)

#undef DECLARE_ID

} // namespace IDs

enum class LoopMode
{
    none,
    forward,
    pingpong
};

// We want to send type-erased commands to the audio thread, but we also
// want those commands to contain move-only resources, so that we can
// construct resources on the gui thread, and then transfer ownership
// cheaply to the audio thread. We can't do this with std::function
// because it enforces that functions are copy-constructible.
// Therefore, we use a very simple templated type-eraser here.
template <typename Proc>
struct Command
{
    virtual ~Command() noexcept                    = default;
    virtual z0 run (Proc& proc) = 0;
};

template <typename Proc, typename Func>
class TemplateCommand final : public Command<Proc>,
                              private Func
{
public:
    template <typename FuncPrime>
    explicit TemplateCommand (FuncPrime&& funcPrime)
        : Func (std::forward<FuncPrime> (funcPrime))
    {}

    z0 run (Proc& proc) override { (*this) (proc); }
};

template <typename Proc>
class CommandFifo final
{
public:
    explicit CommandFifo (i32 size)
        : buffer ((size_t) size),
          abstractFifo (size)
    {}

    CommandFifo()
        : CommandFifo (1024)
    {}

    template <typename Item>
    z0 push (Item&& item) noexcept
    {
        auto command = makeCommand (std::forward<Item> (item));

        abstractFifo.write (1).forEach ([&] (i32 index)
        {
            buffer[size_t (index)] = std::move (command);
        });
    }

    z0 call (Proc& proc) noexcept
    {
        abstractFifo.read (abstractFifo.getNumReady()).forEach ([&] (i32 index)
        {
            buffer[size_t (index)]->run (proc);
        });
    }

private:
    template <typename Func>
    static std::unique_ptr<Command<Proc>> makeCommand (Func&& func)
    {
        using Decayed = std::decay_t<Func>;
        return std::make_unique<TemplateCommand<Proc, Decayed>> (std::forward<Func> (func));
    }

    std::vector<std::unique_ptr<Command<Proc>>> buffer;
    AbstractFifo abstractFifo;
};

//==============================================================================
// Represents the constant parts of an audio sample: its name, sample rate,
// length, and the audio sample data itself.
// Samples might be pretty big, so we'll keep shared_ptrs to them most of the
// time, to reduce duplication and copying.
class Sample final
{
public:
    Sample (AudioFormatReader& source, f64 maxSampleLengthSecs)
        : sourceSampleRate (source.sampleRate),
          length (jmin (i32 (source.lengthInSamples),
                        i32 (maxSampleLengthSecs * sourceSampleRate))),
          data (jmin (2, i32 (source.numChannels)), length + 4)
    {
        if (length == 0)
            throw std::runtime_error ("Unable to load sample");

        source.read (&data, 0, length + 4, 0, true, true);
    }

    f64 getSampleRate() const                    { return sourceSampleRate; }
    i32 getLength() const                           { return length; }
    const AudioBuffer<f32>& getBuffer() const     { return data; }

private:
    f64 sourceSampleRate;
    i32 length;
    AudioBuffer<f32> data;
};

//==============================================================================
// A class which contains all the information related to sample-playback, such
// as sample data, loop points, and loop kind.
// It is expected that multiple sampler voices will maintain pointers to a
// single instance of this class, to avoid redundant duplication of sample
// data in memory.
class MPESamplerSound final
{
public:
    z0 setSample (std::unique_ptr<Sample> value)
    {
        sample = std::move (value);
        setLoopPointsInSeconds (loopPoints);
    }

    Sample* getSample() const
    {
        return sample.get();
    }

    z0 setLoopPointsInSeconds (Range<f64> value)
    {
        loopPoints = sample == nullptr ? value
                                       : Range<f64> (0, sample->getLength() / sample->getSampleRate())
                                                        .constrainRange (value);
    }

    Range<f64> getLoopPointsInSeconds() const
    {
        return loopPoints;
    }

    z0 setCentreFrequencyInHz (f64 centre)
    {
        centreFrequencyInHz = centre;
    }

    f64 getCentreFrequencyInHz() const
    {
        return centreFrequencyInHz;
    }

    z0 setLoopMode (LoopMode type)
    {
        loopMode = type;
    }

    LoopMode getLoopMode() const
    {
        return loopMode;
    }

private:
    std::unique_ptr<Sample> sample;
    f64 centreFrequencyInHz { 440.0 };
    Range<f64> loopPoints;
    LoopMode loopMode { LoopMode::none };
};

//==============================================================================
class MPESamplerVoice final : public MPESynthesiserVoice
{
public:
    explicit MPESamplerVoice (std::shared_ptr<const MPESamplerSound> sound)
        : samplerSound (std::move (sound))
    {
        jassert (samplerSound != nullptr);
    }

    z0 noteStarted() override
    {
        jassert (currentlyPlayingNote.isValid());
        jassert (currentlyPlayingNote.keyState == MPENote::keyDown
              || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);

        level    .setTargetValue (currentlyPlayingNote.noteOnVelocity.asUnsignedFloat());
        frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());

        auto loopPoints = samplerSound->getLoopPointsInSeconds();
        loopBegin.setTargetValue (loopPoints.getStart() * samplerSound->getSample()->getSampleRate());
        loopEnd  .setTargetValue (loopPoints.getEnd()   * samplerSound->getSample()->getSampleRate());

        for (auto smoothed : { &level, &frequency, &loopBegin, &loopEnd })
            smoothed->reset (currentSampleRate, smoothingLengthInSeconds);

        previousPressure = currentlyPlayingNote.pressure.asUnsignedFloat();
        currentSamplePos = 0.0;
        tailOff          = 0.0;
    }

    z0 noteStopped (b8 allowTailOff) override
    {
        jassert (currentlyPlayingNote.keyState == MPENote::off);

        if (allowTailOff && approximatelyEqual (tailOff, 0.0))
            tailOff = 1.0;
        else
            stopNote();
    }

    z0 notePressureChanged() override
    {
        const auto currentPressure = static_cast<f64> (currentlyPlayingNote.pressure.asUnsignedFloat());
        const auto deltaPressure = currentPressure - previousPressure;
        level.setTargetValue (jlimit (0.0, 1.0, level.getCurrentValue() + deltaPressure));
        previousPressure = currentPressure;
    }

    z0 notePitchbendChanged() override
    {
        frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());
    }

    z0 noteTimbreChanged()   override {}
    z0 noteKeyStateChanged() override {}

    z0 renderNextBlock (AudioBuffer<f32>& outputBuffer,
                          i32 startSample,
                          i32 numSamples) override
    {
        render (outputBuffer, startSample, numSamples);
    }

    z0 renderNextBlock (AudioBuffer<f64>& outputBuffer,
                          i32 startSample,
                          i32 numSamples) override
    {
        render (outputBuffer, startSample, numSamples);
    }

    f64 getCurrentSamplePosition() const
    {
        return currentSamplePos;
    }

private:
    template <typename Element>
    z0 render (AudioBuffer<Element>& outputBuffer, i32 startSample, i32 numSamples)
    {
        jassert (samplerSound->getSample() != nullptr);

        auto loopPoints = samplerSound->getLoopPointsInSeconds();
        loopBegin.setTargetValue (loopPoints.getStart() * samplerSound->getSample()->getSampleRate());
        loopEnd  .setTargetValue (loopPoints.getEnd()   * samplerSound->getSample()->getSampleRate());

        auto& data = samplerSound->getSample()->getBuffer();

        auto inL = data.getReadPointer (0);
        auto inR = data.getNumChannels() > 1 ? data.getReadPointer (1) : nullptr;

        auto outL = outputBuffer.getWritePointer (0, startSample);

        if (outL == nullptr)
            return;

        auto outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample)
                                                      : nullptr;

        size_t writePos = 0;

        while (--numSamples >= 0 && renderNextSample (inL, inR, outL, outR, writePos))
            writePos += 1;
    }

    template <typename Element>
    b8 renderNextSample (const f32* inL,
                           const f32* inR,
                           Element* outL,
                           Element* outR,
                           size_t writePos)
    {
        auto currentLevel     = level.getNextValue();
        auto currentFrequency = frequency.getNextValue();
        auto currentLoopBegin = loopBegin.getNextValue();
        auto currentLoopEnd   = loopEnd.getNextValue();

        if (isTailingOff())
        {
            currentLevel *= tailOff;
            tailOff *= 0.9999;

            if (tailOff < 0.005)
            {
                stopNote();
                return false;
            }
        }

        auto pos      = (i32) currentSamplePos;
        auto nextPos  = pos + 1;
        auto alpha    = (Element) (currentSamplePos - pos);
        auto invAlpha = 1.0f - alpha;

        // just using a very simple linear interpolation here..
        auto l = static_cast<Element> (currentLevel * (inL[pos] * invAlpha + inL[nextPos] * alpha));
        auto r = static_cast<Element> ((inR != nullptr) ? currentLevel * (inR[pos] * invAlpha + inR[nextPos] * alpha)
                                                        : l);

        if (outR != nullptr)
        {
            outL[writePos] += l;
            outR[writePos] += r;
        }
        else
        {
            outL[writePos] += (l + r) * 0.5f;
        }

        std::tie (currentSamplePos, currentDirection) = getNextState (currentFrequency,
                                                                      currentLoopBegin,
                                                                      currentLoopEnd);

        if (currentSamplePos > samplerSound->getSample()->getLength())
        {
            stopNote();
            return false;
        }

        return true;
    }

    f64 getSampleValue() const;

    b8 isTailingOff() const
    {
        return ! approximatelyEqual (tailOff, 0.0);
    }

    z0 stopNote()
    {
        clearCurrentNote();
        currentSamplePos = 0.0;
    }

    enum class Direction
    {
        forward,
        backward
    };

    std::tuple<f64, Direction> getNextState (f64 freq,
                                                f64 begin,
                                                f64 end) const
    {
        auto nextPitchRatio = freq / samplerSound->getCentreFrequencyInHz();

        auto nextSamplePos = currentSamplePos;
        auto nextDirection = currentDirection;

        // Move the current sample pos in the correct direction
        switch (currentDirection)
        {
            case Direction::forward:
                nextSamplePos += nextPitchRatio;
                break;

            case Direction::backward:
                nextSamplePos -= nextPitchRatio;
                break;

            default:
                break;
        }

        // Update current sample position, taking loop mode into account
        // If the loop mode was changed while we were travelling backwards, deal
        // with it gracefully.
        if (nextDirection == Direction::backward && nextSamplePos < begin)
        {
            nextSamplePos = begin;
            nextDirection = Direction::forward;

            return std::tuple<f64, Direction> (nextSamplePos, nextDirection);
        }

        if (samplerSound->getLoopMode() == LoopMode::none)
            return std::tuple<f64, Direction> (nextSamplePos, nextDirection);

        if (nextDirection == Direction::forward && end < nextSamplePos && !isTailingOff())
        {
            if (samplerSound->getLoopMode() == LoopMode::forward)
                nextSamplePos = begin;
            else if (samplerSound->getLoopMode() == LoopMode::pingpong)
            {
                nextSamplePos = end;
                nextDirection = Direction::backward;
            }
        }
        return std::tuple<f64, Direction> (nextSamplePos, nextDirection);
    }

    std::shared_ptr<const MPESamplerSound> samplerSound;
    SmoothedValue<f64> level { 0 };
    SmoothedValue<f64> frequency { 0 };
    SmoothedValue<f64> loopBegin;
    SmoothedValue<f64> loopEnd;
    f64 previousPressure { 0 };
    f64 currentSamplePos { 0 };
    f64 tailOff { 0 };
    Direction currentDirection { Direction::forward };
    f64 smoothingLengthInSeconds { 0.01 };
};

template <typename Contents>
class ReferenceCountingAdapter final : public ReferenceCountedObject
{
public:
    template <typename... Args>
    explicit ReferenceCountingAdapter (Args&&... args)
        : contents (std::forward<Args> (args)...)
    {}

    const Contents& get() const
    {
        return contents;
    }

    Contents& get()
    {
        return contents;
    }

private:
    Contents contents;
};

template <typename Contents, typename... Args>
std::unique_ptr<ReferenceCountingAdapter<Contents>>
make_reference_counted (Args&&... args)
{
    auto adapter = new ReferenceCountingAdapter<Contents> (std::forward<Args> (args)...);
    return std::unique_ptr<ReferenceCountingAdapter<Contents>> (adapter);
}

//==============================================================================
inline std::unique_ptr<AudioFormatReader> makeAudioFormatReader (AudioFormatManager& manager,
                                                                 ukk sampleData,
                                                                 size_t dataSize)
{
    return std::unique_ptr<AudioFormatReader> (manager.createReaderFor (std::make_unique<MemoryInputStream> (sampleData,
                                                                                                             dataSize,
                                                                                                             false)));
}

inline std::unique_ptr<AudioFormatReader> makeAudioFormatReader (AudioFormatManager& manager,
                                                                 const File& file)
{
    return std::unique_ptr<AudioFormatReader> (manager.createReaderFor (file));
}

//==============================================================================
class AudioFormatReaderFactory
{
public:
    AudioFormatReaderFactory() = default;
    AudioFormatReaderFactory (const AudioFormatReaderFactory&) = default;
    AudioFormatReaderFactory (AudioFormatReaderFactory&&) = default;
    AudioFormatReaderFactory& operator= (const AudioFormatReaderFactory&) = default;
    AudioFormatReaderFactory& operator= (AudioFormatReaderFactory&&) = default;

    virtual ~AudioFormatReaderFactory() noexcept = default;

    virtual std::unique_ptr<AudioFormatReader> make (AudioFormatManager&) const = 0;
    virtual std::unique_ptr<AudioFormatReaderFactory> clone() const = 0;
};

//==============================================================================
class MemoryAudioFormatReaderFactory final : public AudioFormatReaderFactory
{
public:
    explicit MemoryAudioFormatReaderFactory (MemoryBlock mb)
        : memoryBlock (std::make_shared<MemoryBlock> (std::move (mb)))
    {
    }

    std::unique_ptr<AudioFormatReader> make (AudioFormatManager& manager) const override
    {
        return makeAudioFormatReader (manager, memoryBlock->getData(), memoryBlock->getSize());
    }

    std::unique_ptr<AudioFormatReaderFactory> clone() const override
    {
        return std::unique_ptr<AudioFormatReaderFactory> (new MemoryAudioFormatReaderFactory (*this));
    }

private:
    std::shared_ptr<MemoryBlock> memoryBlock;
};

//==============================================================================
class FileAudioFormatReaderFactory final : public AudioFormatReaderFactory
{
public:
    explicit FileAudioFormatReaderFactory (File fileIn)
        : file (std::move (fileIn))
    {}

    std::unique_ptr<AudioFormatReader> make (AudioFormatManager& manager) const override
    {
        return makeAudioFormatReader (manager, file);
    }

    std::unique_ptr<AudioFormatReaderFactory> clone() const override
    {
        return std::unique_ptr<AudioFormatReaderFactory> (new FileAudioFormatReaderFactory (*this));
    }

private:
    File file;
};

namespace drx
{

template<>
struct VariantConverter<LoopMode>
{
    static LoopMode fromVar (const var& v)
    {
        return static_cast<LoopMode> (i32 (v));
    }

    static var toVar (LoopMode loopMode)
    {
        return static_cast<i32> (loopMode);
    }
};

template <typename Wrapped>
struct GenericVariantConverter
{
    static Wrapped fromVar (const var& v)
    {
        auto cast = dynamic_cast<ReferenceCountingAdapter<Wrapped>*> (v.getObject());
        jassert (cast != nullptr);
        return cast->get();
    }

    static var toVar (Wrapped range)
    {
        return { make_reference_counted<Wrapped> (std::move (range)).release() };
    }
};

template <typename Numeric>
struct VariantConverter<Range<Numeric>> final : GenericVariantConverter<Range<Numeric>> {};

template<>
struct VariantConverter<MPEZoneLayout> final : GenericVariantConverter<MPEZoneLayout> {};

template<>
struct VariantConverter<std::shared_ptr<AudioFormatReaderFactory>> final
    : GenericVariantConverter<std::shared_ptr<AudioFormatReaderFactory>>
{};

} // namespace drx

//==============================================================================
class VisibleRangeDataModel final : private ValueTree::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() noexcept = default;
        virtual z0 totalRangeChanged   (Range<f64>) {}
        virtual z0 visibleRangeChanged (Range<f64>) {}
    };

    VisibleRangeDataModel()
        : VisibleRangeDataModel (ValueTree (IDs::VISIBLE_RANGE))
    {}

    explicit VisibleRangeDataModel (const ValueTree& vt)
        : valueTree (vt),
          totalRange   (valueTree, IDs::totalRange,   nullptr),
          visibleRange (valueTree, IDs::visibleRange, nullptr)
    {
        jassert (valueTree.hasType (IDs::VISIBLE_RANGE));
        valueTree.addListener (this);
    }

    VisibleRangeDataModel (const VisibleRangeDataModel& other)
        : VisibleRangeDataModel (other.valueTree)
    {}

    VisibleRangeDataModel& operator= (const VisibleRangeDataModel& other)
    {
        auto copy (other);
        swap (copy);
        return *this;
    }

    Range<f64> getTotalRange() const
    {
        return totalRange;
    }

    z0 setTotalRange (Range<f64> value, UndoManager* undoManager)
    {
        totalRange.setValue (value, undoManager);
        setVisibleRange (visibleRange, undoManager);
    }

    Range<f64> getVisibleRange() const
    {
        return visibleRange;
    }

    z0 setVisibleRange (Range<f64> value, UndoManager* undoManager)
    {
        visibleRange.setValue (totalRange.get().constrainRange (value), undoManager);
    }

    z0 addListener (Listener& listener)
    {
        listenerList.add (&listener);
    }

    z0 removeListener (Listener& listener)
    {
        listenerList.remove (&listener);
    }

    z0 swap (VisibleRangeDataModel& other) noexcept
    {
        using std::swap;
        swap (other.valueTree, valueTree);
    }

private:
    z0 valueTreePropertyChanged (ValueTree&, const Identifier& property) override
    {
        if (property == IDs::totalRange)
        {
            totalRange.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.totalRangeChanged (totalRange); });
        }
        else if (property == IDs::visibleRange)
        {
            visibleRange.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.visibleRangeChanged (visibleRange); });
        }
    }

    z0 valueTreeChildAdded        (ValueTree&, ValueTree&)      override { jassertfalse; }
    z0 valueTreeChildRemoved      (ValueTree&, ValueTree&, i32) override { jassertfalse; }
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32)        override { jassertfalse; }
    z0 valueTreeParentChanged     (ValueTree&)                  override { jassertfalse; }

    ValueTree valueTree;

    CachedValue<Range<f64>> totalRange;
    CachedValue<Range<f64>> visibleRange;

    ListenerList<Listener> listenerList;
};

//==============================================================================
class MPESettingsDataModel final : private ValueTree::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() noexcept = default;
        virtual z0 synthVoicesChanged (i32) {}
        virtual z0 voiceStealingEnabledChanged (b8) {}
        virtual z0 legacyModeEnabledChanged (b8) {}
        virtual z0 mpeZoneLayoutChanged (const MPEZoneLayout&) {}
        virtual z0 legacyFirstChannelChanged (i32) {}
        virtual z0 legacyLastChannelChanged (i32) {}
        virtual z0 legacyPitchbendRangeChanged (i32) {}
    };

    MPESettingsDataModel()
        : MPESettingsDataModel (ValueTree (IDs::MPE_SETTINGS))
    {}

    explicit MPESettingsDataModel (const ValueTree& vt)
        : valueTree (vt),
          synthVoices          (valueTree, IDs::synthVoices,          nullptr, 15),
          voiceStealingEnabled (valueTree, IDs::voiceStealingEnabled, nullptr, false),
          legacyModeEnabled    (valueTree, IDs::legacyModeEnabled,    nullptr, true),
          mpeZoneLayout        (valueTree, IDs::mpeZoneLayout,        nullptr, {}),
          legacyFirstChannel   (valueTree, IDs::legacyFirstChannel,   nullptr, 1),
          legacyLastChannel    (valueTree, IDs::legacyLastChannel,    nullptr, 15),
          legacyPitchbendRange (valueTree, IDs::legacyPitchbendRange, nullptr, 48)
    {
        jassert (valueTree.hasType (IDs::MPE_SETTINGS));
        valueTree.addListener (this);
    }

    MPESettingsDataModel (const MPESettingsDataModel& other)
        : MPESettingsDataModel (other.valueTree)
    {}

    MPESettingsDataModel& operator= (const MPESettingsDataModel& other)
    {
        auto copy (other);
        swap (copy);
        return *this;
    }

    i32 getSynthVoices() const
    {
        return synthVoices;
    }

    z0 setSynthVoices (i32 value, UndoManager* undoManager)
    {
        synthVoices.setValue (Range<i32> (1, 20).clipValue (value), undoManager);
    }

    b8 getVoiceStealingEnabled() const
    {
        return voiceStealingEnabled;
    }

    z0 setVoiceStealingEnabled (b8 value, UndoManager* undoManager)
    {
        voiceStealingEnabled.setValue (value, undoManager);
    }

    b8 getLegacyModeEnabled() const
    {
        return legacyModeEnabled;
    }

    z0 setLegacyModeEnabled (b8 value, UndoManager* undoManager)
    {
        legacyModeEnabled.setValue (value, undoManager);
    }

    MPEZoneLayout getMPEZoneLayout() const
    {
        return mpeZoneLayout;
    }

    z0 setMPEZoneLayout (MPEZoneLayout value, UndoManager* undoManager)
    {
        mpeZoneLayout.setValue (value, undoManager);
    }

    i32 getLegacyFirstChannel() const
    {
        return legacyFirstChannel;
    }

    z0 setLegacyFirstChannel (i32 value, UndoManager* undoManager)
    {
        legacyFirstChannel.setValue (Range<i32> (1, legacyLastChannel).clipValue (value), undoManager);
    }

    i32 getLegacyLastChannel() const
    {
        return legacyLastChannel;
    }

    z0 setLegacyLastChannel (i32 value, UndoManager* undoManager)
    {
        legacyLastChannel.setValue (Range<i32> (legacyFirstChannel, 15).clipValue (value), undoManager);
    }

    i32 getLegacyPitchbendRange() const
    {
        return legacyPitchbendRange;
    }

    z0 setLegacyPitchbendRange (i32 value, UndoManager* undoManager)
    {
        legacyPitchbendRange.setValue (Range<i32> (0, 95).clipValue (value), undoManager);
    }

    z0 addListener (Listener& listener)
    {
        listenerList.add (&listener);
    }

    z0 removeListener (Listener& listener)
    {
        listenerList.remove (&listener);
    }

    z0 swap (MPESettingsDataModel& other) noexcept
    {
        using std::swap;
        swap (other.valueTree, valueTree);
    }

private:
    z0 valueTreePropertyChanged (ValueTree&, const Identifier& property) override
    {
        if (property == IDs::synthVoices)
        {
            synthVoices.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.synthVoicesChanged (synthVoices); });
        }
        else if (property == IDs::voiceStealingEnabled)
        {
            voiceStealingEnabled.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.voiceStealingEnabledChanged (voiceStealingEnabled); });
        }
        else if (property == IDs::legacyModeEnabled)
        {
            legacyModeEnabled.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.legacyModeEnabledChanged (legacyModeEnabled); });
        }
        else if (property == IDs::mpeZoneLayout)
        {
            mpeZoneLayout.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.mpeZoneLayoutChanged (mpeZoneLayout); });
        }
        else if (property == IDs::legacyFirstChannel)
        {
            legacyFirstChannel.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.legacyFirstChannelChanged (legacyFirstChannel); });
        }
        else if (property == IDs::legacyLastChannel)
        {
            legacyLastChannel.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.legacyLastChannelChanged (legacyLastChannel); });
        }
        else if (property == IDs::legacyPitchbendRange)
        {
            legacyPitchbendRange.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.legacyPitchbendRangeChanged (legacyPitchbendRange); });
        }
    }

    z0 valueTreeChildAdded        (ValueTree&, ValueTree&)      override { jassertfalse; }
    z0 valueTreeChildRemoved      (ValueTree&, ValueTree&, i32) override { jassertfalse; }
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32)        override { jassertfalse; }
    z0 valueTreeParentChanged     (ValueTree&)                  override { jassertfalse; }

    ValueTree valueTree;

    CachedValue<i32> synthVoices;
    CachedValue<b8> voiceStealingEnabled;
    CachedValue<b8> legacyModeEnabled;
    CachedValue<MPEZoneLayout> mpeZoneLayout;
    CachedValue<i32> legacyFirstChannel;
    CachedValue<i32> legacyLastChannel;
    CachedValue<i32> legacyPitchbendRange;

    ListenerList<Listener> listenerList;
};

//==============================================================================
class DataModel final : private ValueTree::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() noexcept = default;
        virtual z0 sampleReaderChanged (std::shared_ptr<AudioFormatReaderFactory>) {}
        virtual z0 centreFrequencyHzChanged (f64) {}
        virtual z0 loopModeChanged (LoopMode) {}
        virtual z0 loopPointsSecondsChanged (Range<f64>) {}
    };

    explicit DataModel (AudioFormatManager& audioFormatManagerIn)
        : DataModel (audioFormatManagerIn, ValueTree (IDs::DATA_MODEL))
    {}

    DataModel (AudioFormatManager& audioFormatManagerIn, const ValueTree& vt)
        : audioFormatManager (&audioFormatManagerIn),
          valueTree (vt),
          sampleReader      (valueTree, IDs::sampleReader,      nullptr),
          centreFrequencyHz (valueTree, IDs::centreFrequencyHz, nullptr),
          loopMode          (valueTree, IDs::loopMode,          nullptr, LoopMode::none),
          loopPointsSeconds (valueTree, IDs::loopPointsSeconds, nullptr)
    {
        jassert (valueTree.hasType (IDs::DATA_MODEL));
        valueTree.addListener (this);
    }

    DataModel (const DataModel& other)
        : DataModel (*other.audioFormatManager, other.valueTree)
    {}

    DataModel& operator= (const DataModel& other)
    {
        auto copy (other);
        swap (copy);
        return *this;
    }

    std::unique_ptr<AudioFormatReader> getSampleReader() const
    {
        return sampleReader != nullptr ? sampleReader.get()->make (*audioFormatManager) : nullptr;
    }

    z0 setSampleReader (std::unique_ptr<AudioFormatReaderFactory> readerFactory,
                          UndoManager* undoManager)
    {
        sampleReader.setValue (std::move (readerFactory), undoManager);
        setLoopPointsSeconds (Range<f64> (0, getSampleLengthSeconds()).constrainRange (loopPointsSeconds),
                              undoManager);
    }

    f64 getSampleLengthSeconds() const
    {
        if (auto r = getSampleReader())
            return (f64) r->lengthInSamples / r->sampleRate;

        return 1.0;
    }

    f64 getCentreFrequencyHz() const
    {
        return centreFrequencyHz;
    }

    z0 setCentreFrequencyHz (f64 value, UndoManager* undoManager)
    {
        centreFrequencyHz.setValue (Range<f64> (20, 20000).clipValue (value),
                                    undoManager);
    }

    LoopMode getLoopMode() const
    {
        return loopMode;
    }

    z0 setLoopMode (LoopMode value, UndoManager* undoManager)
    {
        loopMode.setValue (value, undoManager);
    }

    Range<f64> getLoopPointsSeconds() const
    {
        return loopPointsSeconds;
    }

    z0 setLoopPointsSeconds (Range<f64> value, UndoManager* undoManager)
    {
        loopPointsSeconds.setValue (Range<f64> (0, getSampleLengthSeconds()).constrainRange (value),
                                    undoManager);
    }

    MPESettingsDataModel mpeSettings()
    {
        return MPESettingsDataModel (valueTree.getOrCreateChildWithName (IDs::MPE_SETTINGS, nullptr));
    }

    z0 addListener (Listener& listener)
    {
        listenerList.add (&listener);
    }

    z0 removeListener (Listener& listener)
    {
        listenerList.remove (&listener);
    }

    z0 swap (DataModel& other) noexcept
    {
        using std::swap;
        swap (other.valueTree, valueTree);
    }

    AudioFormatManager& getAudioFormatManager() const
    {
        return *audioFormatManager;
    }

private:
    z0 valueTreePropertyChanged (ValueTree&, const Identifier& property) override
    {
        if (property == IDs::sampleReader)
        {
            sampleReader.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.sampleReaderChanged (sampleReader); });
        }
        else if (property == IDs::centreFrequencyHz)
        {
            centreFrequencyHz.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.centreFrequencyHzChanged (centreFrequencyHz); });
        }
        else if (property == IDs::loopMode)
        {
            loopMode.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.loopModeChanged (loopMode); });
        }
        else if (property == IDs::loopPointsSeconds)
        {
            loopPointsSeconds.forceUpdateOfCachedValue();
            listenerList.call ([this] (Listener& l) { l.loopPointsSecondsChanged (loopPointsSeconds); });
        }
    }

    z0 valueTreeChildAdded        (ValueTree&, ValueTree&)      override {}
    z0 valueTreeChildRemoved      (ValueTree&, ValueTree&, i32) override { jassertfalse; }
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32)        override { jassertfalse; }
    z0 valueTreeParentChanged     (ValueTree&)                  override { jassertfalse; }

    AudioFormatManager* audioFormatManager;

    ValueTree valueTree;

    CachedValue<std::shared_ptr<AudioFormatReaderFactory>> sampleReader;
    CachedValue<f64> centreFrequencyHz;
    CachedValue<LoopMode> loopMode;
    CachedValue<Range<f64>> loopPointsSeconds;

    ListenerList<Listener> listenerList;
};

namespace
{
z0 initialiseComboBoxWithConsecutiveIntegers (Component& owner,
                                                ComboBox& comboBox,
                                                Label& label,
                                                i32 firstValue,
                                                i32 numValues,
                                                i32 valueToSelect)
{
    for (auto i = 0; i < numValues; ++i)
        comboBox.addItem (Txt (i + firstValue), i + 1);

    comboBox.setSelectedId (valueToSelect - firstValue + 1);

    label.attachToComponent (&comboBox, true);
    owner.addAndMakeVisible (comboBox);
}

constexpr i32 controlHeight     = 24;
constexpr i32 controlSeparation = 6;

} // namespace

//==============================================================================
class MPELegacySettingsComponent final : public Component,
                                         private MPESettingsDataModel::Listener
{
public:
    explicit MPELegacySettingsComponent (const MPESettingsDataModel& model,
                                         UndoManager& um)
        : dataModel (model),
          undoManager (&um)
    {
        dataModel.addListener (*this);

        initialiseComboBoxWithConsecutiveIntegers (*this, legacyStartChannel, legacyStartChannelLabel, 1, 16, 1);
        initialiseComboBoxWithConsecutiveIntegers (*this, legacyEndChannel, legacyEndChannelLabel, 1, 16, 16);
        initialiseComboBoxWithConsecutiveIntegers (*this, legacyPitchbendRange, legacyPitchbendRangeLabel, 0, 96, 2);

        legacyStartChannel.onChange = [this]
        {
            if (isLegacyModeValid())
            {
                undoManager->beginNewTransaction();
                dataModel.setLegacyFirstChannel (getFirstChannel(), undoManager);
            }
        };

        legacyEndChannel.onChange = [this]
        {
            if (isLegacyModeValid())
            {
                undoManager->beginNewTransaction();
                dataModel.setLegacyLastChannel (getLastChannel(), undoManager);
            }
        };

        legacyPitchbendRange.onChange = [this]
        {
            if (isLegacyModeValid())
            {
                undoManager->beginNewTransaction();
                dataModel.setLegacyPitchbendRange (legacyPitchbendRange.getText().getIntValue(), undoManager);
            }
        };
    }

    i32 getMinHeight() const
    {
        return (controlHeight * 3) + (controlSeparation * 2);
    }

private:
    z0 resized() override
    {
        Rectangle<i32> r (proportionOfWidth (0.65f), 0, proportionOfWidth (0.25f), getHeight());

        for (auto& comboBox : { &legacyStartChannel, &legacyEndChannel, &legacyPitchbendRange })
        {
            comboBox->setBounds (r.removeFromTop (controlHeight));
            r.removeFromTop (controlSeparation);
        }
    }

    b8 isLegacyModeValid()
    {
        if (! areLegacyModeParametersValid())
        {
            handleInvalidLegacyModeParameters();
            return false;
        }

        return true;
    }

    z0 legacyFirstChannelChanged (i32 value) override
    {
        legacyStartChannel.setSelectedId (value, dontSendNotification);
    }

    z0 legacyLastChannelChanged (i32 value) override
    {
        legacyEndChannel.setSelectedId (value, dontSendNotification);
    }

    z0 legacyPitchbendRangeChanged (i32 value) override
    {
        legacyPitchbendRange.setSelectedId (value + 1, dontSendNotification);
    }

    i32 getFirstChannel() const
    {
        return legacyStartChannel.getText().getIntValue();
    }

    i32 getLastChannel() const
    {
        return legacyEndChannel.getText().getIntValue();
    }

    b8 areLegacyModeParametersValid() const
    {
        return getFirstChannel() <= getLastChannel();
    }

    z0 handleInvalidLegacyModeParameters()
    {
        auto options = MessageBoxOptions::makeOptionsOk (AlertWindow::WarningIcon,
                                                         "Invalid legacy mode channel layout",
                                                         "Cannot set legacy mode start/end channel:\n"
                                                         "The end channel must not be less than the start channel!",
                                                         "Got it");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    MPESettingsDataModel dataModel;

    ComboBox legacyStartChannel, legacyEndChannel, legacyPitchbendRange;

    Label legacyStartChannelLabel   { {}, "First channel" },
          legacyEndChannelLabel     { {}, "Last channel" },
          legacyPitchbendRangeLabel { {}, "Pitchbend range (semitones)" };

    UndoManager* undoManager;
    ScopedMessageBox messageBox;
};

//==============================================================================
class MPENewSettingsComponent final : public Component,
                                      private MPESettingsDataModel::Listener
{
public:
    MPENewSettingsComponent (const MPESettingsDataModel& model,
                             UndoManager& um)
        : dataModel (model),
          undoManager (&um)
    {
        dataModel.addListener (*this);

        addAndMakeVisible (isLowerZoneButton);
        isLowerZoneButton.setToggleState (true, NotificationType::dontSendNotification);

        initialiseComboBoxWithConsecutiveIntegers (*this, memberChannels, memberChannelsLabel, 0, 16, 15);
        initialiseComboBoxWithConsecutiveIntegers (*this, masterPitchbendRange, masterPitchbendRangeLabel, 0, 96, 2);
        initialiseComboBoxWithConsecutiveIntegers (*this, notePitchbendRange, notePitchbendRangeLabel, 0, 96, 48);

        for (auto& button : { &setZoneButton, &clearAllZonesButton })
            addAndMakeVisible (button);

        setZoneButton.onClick = [this]
        {
            auto isLowerZone = isLowerZoneButton.getToggleState();
            auto numMemberChannels = memberChannels.getText().getIntValue();
            auto perNotePb = notePitchbendRange.getText().getIntValue();
            auto masterPb = masterPitchbendRange.getText().getIntValue();

            if (isLowerZone)
                zoneLayout.setLowerZone (numMemberChannels, perNotePb, masterPb);
            else
                zoneLayout.setUpperZone (numMemberChannels, perNotePb, masterPb);

            undoManager->beginNewTransaction();
            dataModel.setMPEZoneLayout (zoneLayout, undoManager);
        };

        clearAllZonesButton.onClick = [this]
        {
            zoneLayout.clearAllZones();
            undoManager->beginNewTransaction();
            dataModel.setMPEZoneLayout (zoneLayout, undoManager);
        };
    }

    i32 getMinHeight() const
    {
        return (controlHeight * 6) + (controlSeparation * 6);
    }

private:
    z0 resized() override
    {
        Rectangle<i32> r (proportionOfWidth (0.65f), 0, proportionOfWidth (0.25f), getHeight());

        isLowerZoneButton.setBounds (r.removeFromTop (controlHeight));
        r.removeFromTop (controlSeparation);

        for (auto& comboBox : { &memberChannels, &masterPitchbendRange, &notePitchbendRange })
        {
            comboBox->setBounds (r.removeFromTop (controlHeight));
            r.removeFromTop (controlSeparation);
        }

        r.removeFromTop (controlSeparation);

        auto buttonLeft = proportionOfWidth (0.5f);

        setZoneButton.setBounds (r.removeFromTop (controlHeight).withLeft (buttonLeft));
        r.removeFromTop (controlSeparation);
        clearAllZonesButton.setBounds (r.removeFromTop (controlHeight).withLeft (buttonLeft));
    }

    z0 mpeZoneLayoutChanged (const MPEZoneLayout& value) override
    {
        zoneLayout = value;
    }

    MPESettingsDataModel dataModel;
    MPEZoneLayout zoneLayout;

    ComboBox memberChannels, masterPitchbendRange, notePitchbendRange;

    ToggleButton isLowerZoneButton  { "Lower zone" };

    Label memberChannelsLabel       { {}, "Nr. of member channels" },
          masterPitchbendRangeLabel { {}, "Master pitchbend range (semitones)" },
          notePitchbendRangeLabel   { {}, "Note pitchbend range (semitones)" };

    TextButton setZoneButton       { "Set zone" },
               clearAllZonesButton { "Clear all zones" };

    UndoManager* undoManager;
};

//==============================================================================
class MPESettingsComponent final : public Component,
                                   private MPESettingsDataModel::Listener
{
public:
    MPESettingsComponent (const MPESettingsDataModel& model,
                          UndoManager& um)
        : dataModel (model),
          legacySettings (dataModel, um),
          newSettings (dataModel, um),
          undoManager (&um)
    {
        dataModel.addListener (*this);

        addAndMakeVisible (newSettings);
        addChildComponent (legacySettings);

        initialiseComboBoxWithConsecutiveIntegers (*this, numberOfVoices, numberOfVoicesLabel, 1, 20, 15);
        numberOfVoices.onChange = [this]
        {
            undoManager->beginNewTransaction();
            dataModel.setSynthVoices (numberOfVoices.getText().getIntValue(), undoManager);
        };

        for (auto& button : { &legacyModeEnabledToggle, &voiceStealingEnabledToggle })
        {
            addAndMakeVisible (button);
        }

        legacyModeEnabledToggle.onClick = [this]
        {
            undoManager->beginNewTransaction();
            dataModel.setLegacyModeEnabled (legacyModeEnabledToggle.getToggleState(), undoManager);
        };

        voiceStealingEnabledToggle.onClick = [this]
        {
            undoManager->beginNewTransaction();
            dataModel.setVoiceStealingEnabled (voiceStealingEnabledToggle.getToggleState(), undoManager);
        };
    }

private:
    z0 resized() override
    {
        auto topHeight = jmax (legacySettings.getMinHeight(), newSettings.getMinHeight());
        auto r = getLocalBounds();
        r.removeFromTop (15);
        auto top = r.removeFromTop (topHeight);
        legacySettings.setBounds (top);
        newSettings.setBounds (top);

        r.removeFromLeft (proportionOfWidth (0.65f));
        r = r.removeFromLeft (proportionOfWidth (0.25f));

        auto toggleLeft = proportionOfWidth (0.25f);

        legacyModeEnabledToggle.setBounds (r.removeFromTop (controlHeight).withLeft (toggleLeft));
        r.removeFromTop (controlSeparation);
        voiceStealingEnabledToggle.setBounds (r.removeFromTop (controlHeight).withLeft (toggleLeft));
        r.removeFromTop (controlSeparation);
        numberOfVoices.setBounds (r.removeFromTop (controlHeight));
    }

    z0 legacyModeEnabledChanged (b8 value) override
    {
        legacySettings.setVisible (value);
        newSettings.setVisible (! value);
        legacyModeEnabledToggle.setToggleState (value, dontSendNotification);
    }

    z0 voiceStealingEnabledChanged (b8 value) override
    {
        voiceStealingEnabledToggle.setToggleState (value, dontSendNotification);
    }

    z0 synthVoicesChanged (i32 value) override
    {
        numberOfVoices.setSelectedId (value, dontSendNotification);
    }

    MPESettingsDataModel dataModel;
    MPELegacySettingsComponent legacySettings;
    MPENewSettingsComponent newSettings;

    ToggleButton legacyModeEnabledToggle    { "Enable Legacy Mode" },
                 voiceStealingEnabledToggle { "Enable synth voice stealing" };

    ComboBox numberOfVoices;
    Label numberOfVoicesLabel { {}, "Number of synth voices" };

    UndoManager* undoManager;
};

//==============================================================================
class LoopPointMarker final : public Component
{
public:
    using MouseCallback = std::function<z0 (LoopPointMarker&, const MouseEvent&)>;

    LoopPointMarker (Txt marker,
                     MouseCallback onMouseDownIn,
                     MouseCallback onMouseDragIn,
                     MouseCallback onMouseUpIn)
        : text (std::move (marker)),
          onMouseDown (std::move (onMouseDownIn)),
          onMouseDrag (std::move (onMouseDragIn)),
          onMouseUp (std::move (onMouseUpIn))
    {
        setMouseCursor (MouseCursor::LeftRightResizeCursor);
    }

private:
    z0 resized() override
    {
        auto height = 20;
        auto triHeight = 6;

        auto bounds = getLocalBounds();
        Path newPath;
        newPath.addRectangle (bounds.removeFromBottom (height));

        newPath.startNewSubPath (bounds.getBottomLeft().toFloat());
        newPath.lineTo (bounds.getBottomRight().toFloat());
        Point<f32> apex (static_cast<f32> (bounds.getX() + (bounds.getWidth() / 2)),
                           static_cast<f32> (bounds.getBottom() - triHeight));
        newPath.lineTo (apex);
        newPath.closeSubPath();

        newPath.addLineSegment (Line<f32> (apex, Point<f32> (apex.getX(), 0)), 1);

        path = newPath;
    }

    z0 paint (Graphics& g) override
    {
        g.setColor (Colors::deepskyblue);
        g.fillPath (path);

        auto height = 20;
        g.setColor (Colors::white);
        g.drawText (text, getLocalBounds().removeFromBottom (height), Justification::centred);
    }

    b8 hitTest (i32 x, i32 y) override
    {
        return path.contains ((f32) x, (f32) y);
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        onMouseDown (*this, e);
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        onMouseDrag (*this, e);
    }

    z0 mouseUp (const MouseEvent& e) override
    {
        onMouseUp (*this, e);
    }

    Txt text;
    Path path;
    MouseCallback onMouseDown;
    MouseCallback onMouseDrag;
    MouseCallback onMouseUp;
};

//==============================================================================
class Ruler final : public Component,
                    private VisibleRangeDataModel::Listener
{
public:
    explicit Ruler (const VisibleRangeDataModel& model)
        : visibleRange (model)
    {
        visibleRange.addListener (*this);
        setMouseCursor (MouseCursor::LeftRightResizeCursor);
    }

private:
    z0 paint (Graphics& g) override
    {
        auto minDivisionWidth = 50.0f;
        auto maxDivisions     = (f32) getWidth() / minDivisionWidth;

        auto lookFeel = dynamic_cast<LookAndFeel_V4*> (&getLookAndFeel());
        auto bg = lookFeel->getCurrentColorScheme()
                           .getUIColor (LookAndFeel_V4::ColorScheme::UIColor::widgetBackground);

        g.setGradientFill (ColorGradient (bg.brighter(),
                                           0,
                                           0,
                                           bg.darker(),
                                           0,
                                           (f32) getHeight(),
                                           false));

        g.fillAll();
        g.setColor (bg.brighter());
        g.drawHorizontalLine (0, 0.0f, (f32) getWidth());
        g.setColor (bg.darker());
        g.drawHorizontalLine (1, 0.0f, (f32) getWidth());
        g.setColor (Colors::lightgrey);

        auto minLog = std::ceil (std::log10 (visibleRange.getVisibleRange().getLength() / maxDivisions));
        auto precision = 2 + std::abs (minLog);
        auto divisionMagnitude = std::pow (10, minLog);
        auto startingDivision = std::ceil (visibleRange.getVisibleRange().getStart() / divisionMagnitude);

        for (auto div = startingDivision; div * divisionMagnitude < visibleRange.getVisibleRange().getEnd(); ++div)
        {
            auto time = div * divisionMagnitude;
            auto xPos = (time - visibleRange.getVisibleRange().getStart()) * getWidth()
                              / visibleRange.getVisibleRange().getLength();

            std::ostringstream outStream;
            outStream << std::setprecision (roundToInt (precision)) << time;

            const auto bounds = Rectangle<i32> (Point<i32> (roundToInt (xPos) + 3, 0),
                                                Point<i32> (roundToInt (xPos + minDivisionWidth), getHeight()));

            g.drawText (outStream.str(), bounds, Justification::centredLeft, false);

            g.drawVerticalLine (roundToInt (xPos), 2.0f, (f32) getHeight());
        }
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        visibleRangeOnMouseDown = visibleRange.getVisibleRange();
        timeOnMouseDown = visibleRange.getVisibleRange().getStart()
                       + (visibleRange.getVisibleRange().getLength() * e.getMouseDownX()) / getWidth();
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        // Work out the scale of the new range
        auto unitDistance = 100.0f;
        auto scaleFactor  = 1.0 / std::pow (2, (f32) e.getDistanceFromDragStartY() / unitDistance);

        // Now position it so that the mouse continues to point at the same
        // place on the ruler.
        auto visibleLength = std::max (0.12, visibleRangeOnMouseDown.getLength() * scaleFactor);
        auto rangeBegin = timeOnMouseDown - visibleLength * e.x / getWidth();
        const Range<f64> range (rangeBegin, rangeBegin + visibleLength);
        visibleRange.setVisibleRange (range, nullptr);
    }

    z0 visibleRangeChanged (Range<f64>) override
    {
        repaint();
    }

    VisibleRangeDataModel visibleRange;
    Range<f64> visibleRangeOnMouseDown;
    f64 timeOnMouseDown;
};

//==============================================================================
class LoopPointsOverlay final : public Component,
                                private DataModel::Listener,
                                private VisibleRangeDataModel::Listener
{
public:
    LoopPointsOverlay (const DataModel& dModel,
                       const VisibleRangeDataModel& vModel,
                       UndoManager& undoManagerIn)
        : dataModel (dModel),
          visibleRange (vModel),
          beginMarker ("B",
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointMouseDown (m, e); },
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointDragged   (m, e); },
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointMouseUp   (m, e); }),
          endMarker   ("E",
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointMouseDown (m, e); },
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointDragged   (m, e); },
                       [this] (LoopPointMarker& m, const MouseEvent& e) { this->loopPointMouseUp   (m, e); }),
          undoManager (&undoManagerIn)
    {
        dataModel   .addListener (*this);
        visibleRange.addListener (*this);

        for (auto ptr : { &beginMarker, &endMarker })
            addAndMakeVisible (ptr);
    }

private:
    z0 resized() override
    {
        positionLoopPointMarkers();
    }

    z0 loopPointMouseDown (LoopPointMarker&, const MouseEvent&)
    {
        loopPointsOnMouseDown = dataModel.getLoopPointsSeconds();
        undoManager->beginNewTransaction();
    }

    z0 loopPointDragged (LoopPointMarker& marker, const MouseEvent& e)
    {
        auto x = xPositionToTime (e.getEventRelativeTo (this).position.x);
        const Range<f64> newLoopRange (&marker == &beginMarker ? x : loopPointsOnMouseDown.getStart(),
                                          &marker == &endMarker   ? x : loopPointsOnMouseDown.getEnd());

        dataModel.setLoopPointsSeconds (newLoopRange, undoManager);
    }

    z0 loopPointMouseUp (LoopPointMarker& marker, const MouseEvent& e)
    {
        auto x = xPositionToTime (e.getEventRelativeTo (this).position.x);
        const Range<f64> newLoopRange (&marker == &beginMarker ? x : loopPointsOnMouseDown.getStart(),
                                          &marker == &endMarker   ? x : loopPointsOnMouseDown.getEnd());

        dataModel.setLoopPointsSeconds (newLoopRange, undoManager);
    }

    z0 loopPointsSecondsChanged (Range<f64>) override
    {
        positionLoopPointMarkers();
    }

    z0 visibleRangeChanged (Range<f64>) override
    {
        positionLoopPointMarkers();
    }

    f64 timeToXPosition (f64 time) const
    {
        return (time - visibleRange.getVisibleRange().getStart()) * getWidth()
                     / visibleRange.getVisibleRange().getLength();
    }

    f64 xPositionToTime (f64 xPosition) const
    {
        return ((xPosition * visibleRange.getVisibleRange().getLength()) / getWidth())
                           + visibleRange.getVisibleRange().getStart();
    }

    z0 positionLoopPointMarkers()
    {
        auto halfMarkerWidth = 7;

        for (auto tup : { std::make_tuple (&beginMarker, dataModel.getLoopPointsSeconds().getStart()),
                          std::make_tuple (&endMarker,   dataModel.getLoopPointsSeconds().getEnd()) })
        {
            auto ptr  = std::get<0> (tup);
            auto time = std::get<1> (tup);
            ptr->setSize (halfMarkerWidth * 2, getHeight());
            ptr->setTopLeftPosition (roundToInt (timeToXPosition (time) - halfMarkerWidth), 0);
        }
    }

    DataModel dataModel;
    VisibleRangeDataModel visibleRange;
    Range<f64> loopPointsOnMouseDown;
    LoopPointMarker beginMarker, endMarker;
    UndoManager* undoManager;
};

//==============================================================================
class PlaybackPositionOverlay final : public Component,
                                      private Timer,
                                      private VisibleRangeDataModel::Listener
{
public:
    using Provider = std::function<std::vector<f32>()>;
    PlaybackPositionOverlay (const VisibleRangeDataModel& model,
                             Provider providerIn)
        : visibleRange (model),
          provider (std::move (providerIn))
    {
        visibleRange.addListener (*this);
        startTimer (16);
    }

private:
    z0 paint (Graphics& g) override
    {
        g.setColor (Colors::red);

        for (auto position : provider())
        {
            g.drawVerticalLine (roundToInt (timeToXPosition (position)), 0.0f, (f32) getHeight());
        }
    }

    z0 timerCallback() override
    {
        repaint();
    }

    z0 visibleRangeChanged (Range<f64>) override
    {
        repaint();
    }

    f64 timeToXPosition (f64 time) const
    {
        return (time - visibleRange.getVisibleRange().getStart()) * getWidth()
                     / visibleRange.getVisibleRange().getLength();
    }

    VisibleRangeDataModel visibleRange;
    Provider provider;
};

//==============================================================================
class WaveformView final : public Component,
                           private ChangeListener,
                           private DataModel::Listener,
                           private VisibleRangeDataModel::Listener
{
public:
    WaveformView (const DataModel& model,
                  const VisibleRangeDataModel& vr)
        : dataModel (model),
          visibleRange (vr),
          thumbnailCache (4),
          thumbnail (4, dataModel.getAudioFormatManager(), thumbnailCache)
    {
        dataModel   .addListener (*this);
        visibleRange.addListener (*this);
        thumbnail   .addChangeListener (this);
    }

private:
    z0 paint (Graphics& g) override
    {
        // Draw the waveforms
        g.fillAll (Colors::black);
        auto numChannels = thumbnail.getNumChannels();

        if (numChannels == 0)
        {
            g.setColor (Colors::white);
            g.drawFittedText ("No File Loaded", getLocalBounds(), Justification::centred, 1);
            return;
        }

        auto bounds = getLocalBounds();
        auto channelHeight = bounds.getHeight() / numChannels;

        for (auto i = 0; i != numChannels; ++i)
        {
            drawChannel (g, i, bounds.removeFromTop (channelHeight));
        }
    }

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            repaint();
    }

    z0 sampleReaderChanged (std::shared_ptr<AudioFormatReaderFactory> value) override
    {
        if (value != nullptr)
        {
            if (auto reader = value->make (dataModel.getAudioFormatManager()))
            {
                thumbnail.setReader (reader.release(), currentHashCode);
                currentHashCode += 1;

                return;
            }
        }

        thumbnail.clear();
    }

    z0 visibleRangeChanged (Range<f64>) override
    {
        repaint();
    }

    z0 drawChannel (Graphics& g, i32 channel, Rectangle<i32> bounds)
    {
        g.setGradientFill (ColorGradient (Colors::lightblue,
                                           bounds.getTopLeft().toFloat(),
                                           Colors::darkgrey,
                                           bounds.getBottomLeft().toFloat(),
                                           false));
        thumbnail.drawChannel (g,
                               bounds,
                               visibleRange.getVisibleRange().getStart(),
                               visibleRange.getVisibleRange().getEnd(),
                               channel,
                               1.0f);
    }

    DataModel dataModel;
    VisibleRangeDataModel visibleRange;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    z64 currentHashCode = 0;
};

//==============================================================================
class WaveformEditor final : public Component,
                             private DataModel::Listener
{
public:
    WaveformEditor (const DataModel& model,
                    PlaybackPositionOverlay::Provider provider,
                    UndoManager& undoManager)
        : dataModel (model),
          waveformView (model, visibleRange),
          playbackOverlay (visibleRange, std::move (provider)),
          loopPoints (dataModel, visibleRange, undoManager),
          ruler (visibleRange)
    {
        dataModel.addListener (*this);

        addAndMakeVisible (waveformView);
        addAndMakeVisible (playbackOverlay);
        addChildComponent (loopPoints);
        loopPoints.setAlwaysOnTop (true);

        waveformView.toBack();

        addAndMakeVisible (ruler);
    }

private:
    z0 resized() override
    {
        auto bounds = getLocalBounds();
        ruler          .setBounds (bounds.removeFromTop (25));
        waveformView   .setBounds (bounds);
        playbackOverlay.setBounds (bounds);
        loopPoints     .setBounds (bounds);
    }

    z0 loopModeChanged (LoopMode value) override
    {
        loopPoints.setVisible (value != LoopMode::none);
    }

    z0 sampleReaderChanged (std::shared_ptr<AudioFormatReaderFactory>) override
    {
        auto lengthInSeconds = dataModel.getSampleLengthSeconds();
        visibleRange.setTotalRange   (Range<f64> (0, lengthInSeconds), nullptr);
        visibleRange.setVisibleRange (Range<f64> (0, lengthInSeconds), nullptr);
    }

    DataModel dataModel;
    VisibleRangeDataModel visibleRange;
    WaveformView waveformView;
    PlaybackPositionOverlay playbackOverlay;
    LoopPointsOverlay loopPoints;
    Ruler ruler;
};

//==============================================================================
class MainSamplerView final : public Component,
                              private DataModel::Listener,
                              private ChangeListener
{
public:
    MainSamplerView (const DataModel& model,
                     PlaybackPositionOverlay::Provider provider,
                     UndoManager& um)
        : dataModel (model),
          waveformEditor (dataModel, std::move (provider), um),
          undoManager (um)
    {
        dataModel.addListener (*this);

        addAndMakeVisible (waveformEditor);
        addAndMakeVisible (loadNewSampleButton);
        addAndMakeVisible (undoButton);
        addAndMakeVisible (redoButton);

        auto setReader = [this] (const FileChooser& fc)
        {
            const auto result = fc.getResult();

            if (result != File())
            {
                undoManager.beginNewTransaction();
                auto readerFactory = new FileAudioFormatReaderFactory (result);
                dataModel.setSampleReader (std::unique_ptr<AudioFormatReaderFactory> (readerFactory),
                                           &undoManager);
            }
        };

        loadNewSampleButton.onClick = [this, setReader]
        {
            fileChooser.launchAsync (FileBrowserComponent::FileChooserFlags::openMode |
                                     FileBrowserComponent::FileChooserFlags::canSelectFiles,
                                     setReader);
        };

        addAndMakeVisible (centreFrequency);
        centreFrequency.onValueChange = [this]
        {
            undoManager.beginNewTransaction();
            dataModel.setCentreFrequencyHz (centreFrequency.getValue(),
                                            centreFrequency.isMouseButtonDown() ? nullptr : &undoManager);
        };

        centreFrequency.setRange (20, 20000, 1);
        centreFrequency.setSliderStyle (Slider::SliderStyle::IncDecButtons);
        centreFrequency.setIncDecButtonsMode (Slider::IncDecButtonMode::incDecButtonsDraggable_Vertical);

        auto radioGroupId = 1;

        for (auto buttonPtr : { &loopKindNone, &loopKindForward, &loopKindPingpong })
        {
            addAndMakeVisible (buttonPtr);
            buttonPtr->setRadioGroupId (radioGroupId, dontSendNotification);
            buttonPtr->setClickingTogglesState (true);
        }

        loopKindNone.onClick = [this]
        {
            if (loopKindNone.getToggleState())
            {
                undoManager.beginNewTransaction();
                dataModel.setLoopMode (LoopMode::none, &undoManager);
            }
        };

        loopKindForward.onClick = [this]
        {
            if (loopKindForward.getToggleState())
            {
                undoManager.beginNewTransaction();
                dataModel.setLoopMode (LoopMode::forward, &undoManager);
            }
        };

        loopKindPingpong.onClick = [this]
        {
            if (loopKindPingpong.getToggleState())
            {
                undoManager.beginNewTransaction();
                dataModel.setLoopMode (LoopMode::pingpong, &undoManager);
            }
        };

        undoButton.onClick = [this] { undoManager.undo(); };
        redoButton.onClick = [this] { undoManager.redo(); };

        addAndMakeVisible (centreFrequencyLabel);
        addAndMakeVisible (loopKindLabel);

        changeListenerCallback (&undoManager);
        undoManager.addChangeListener (this);
    }

    ~MainSamplerView() override
    {
        undoManager.removeChangeListener (this);
    }

private:
    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &undoManager)
        {
            undoButton.setEnabled (undoManager.canUndo());
            redoButton.setEnabled (undoManager.canRedo());
        }
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();

        auto topBar = bounds.removeFromTop (50);
        auto padding = 4;
        loadNewSampleButton .setBounds (topBar.removeFromRight (100).reduced (padding));
        redoButton          .setBounds (topBar.removeFromRight (100).reduced (padding));
        undoButton          .setBounds (topBar.removeFromRight (100).reduced (padding));
        centreFrequencyLabel.setBounds (topBar.removeFromLeft  (100).reduced (padding));
        centreFrequency     .setBounds (topBar.removeFromLeft  (100).reduced (padding));

        auto bottomBar = bounds.removeFromBottom (50);
        loopKindLabel   .setBounds (bottomBar.removeFromLeft (100).reduced (padding));
        loopKindNone    .setBounds (bottomBar.removeFromLeft (80) .reduced (padding));
        loopKindForward .setBounds (bottomBar.removeFromLeft (80) .reduced (padding));
        loopKindPingpong.setBounds (bottomBar.removeFromLeft (80) .reduced (padding));

        waveformEditor.setBounds (bounds);
    }

    z0 loopModeChanged (LoopMode value) override
    {
        switch (value)
        {
            case LoopMode::none:
                loopKindNone.setToggleState (true, dontSendNotification);
                break;
            case LoopMode::forward:
                loopKindForward.setToggleState (true, dontSendNotification);
                break;
            case LoopMode::pingpong:
                loopKindPingpong.setToggleState (true, dontSendNotification);
                break;

            default:
                break;
        }
    }

    z0 centreFrequencyHzChanged (f64 value) override
    {
        centreFrequency.setValue (value, dontSendNotification);
    }

    DataModel dataModel;
    WaveformEditor waveformEditor;
    TextButton loadNewSampleButton { "Load New Sample" };
    TextButton undoButton { "Undo" };
    TextButton redoButton { "Redo" };
    Slider centreFrequency;

    TextButton loopKindNone        { "None" },
               loopKindForward     { "Forward" },
               loopKindPingpong    { "Ping Pong" };

    Label centreFrequencyLabel     { {}, "Sample Centre Freq / Hz" },
          loopKindLabel            { {}, "Looping Mode" };


    FileChooser fileChooser { "Select a file to load...", File(),
                              dataModel.getAudioFormatManager().getWildcardForAllFormats() };

    UndoManager& undoManager;
};

//==============================================================================
struct ProcessorState
{
    i32 synthVoices;
    b8 legacyModeEnabled;
    Range<i32> legacyChannels;
    i32 legacyPitchbendRange;
    b8 voiceStealingEnabled;
    MPEZoneLayout mpeZoneLayout;
    std::unique_ptr<AudioFormatReaderFactory> readerFactory;
    Range<f64> loopPointsSeconds;
    f64 centreFrequencyHz;
    LoopMode loopMode;
};

//==============================================================================
class SamplerAudioProcessor final : public AudioProcessor
{
public:
    SamplerAudioProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true))
    {
        if (auto inputStream = createAssetInputStream ("cello.wav"))
        {
            MemoryBlock mb;
            inputStream->readIntoMemoryBlock (mb);
            readerFactory = std::make_unique<MemoryAudioFormatReaderFactory> (std::move (mb));
        }

        if (readerFactory != nullptr)
        {
            AudioFormatManager manager;
            manager.registerBasicFormats();

            if (auto reader = readerFactory->make (manager))
            {
                auto sample = std::make_unique<Sample> (*reader, 10.0);
                auto lengthInSeconds = sample->getLength() / sample->getSampleRate();
                samplerSound->setLoopPointsInSeconds ({ lengthInSeconds * 0.1, lengthInSeconds * 0.9 });
                samplerSound->setSample (std::move (sample));
            }
        }

        // Start with the max number of voices
        for (auto i = 0; i != maxVoices; ++i)
            synthesiser.addVoice (new MPESamplerVoice (samplerSound));
    }

    z0 prepareToPlay (f64 sampleRate, i32) override
    {
        synthesiser.setCurrentPlaybackSampleRate (sampleRate);
    }

    z0 releaseResources() override {}

    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == AudioChannelSet::mono()
            || layouts.getMainOutputChannelSet() == AudioChannelSet::stereo();
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override
    {
        // This function will be called from the message thread. We lock the command
        // queue to ensure that no messages are processed for the duration of this
        // call.
        SpinLock::ScopedLockType lock (commandQueueMutex);

        ProcessorState state;
        state.synthVoices          = synthesiser.getNumVoices();
        state.legacyModeEnabled    = synthesiser.isLegacyModeEnabled();
        state.legacyChannels       = synthesiser.getLegacyModeChannelRange();
        state.legacyPitchbendRange = synthesiser.getLegacyModePitchbendRange();
        state.voiceStealingEnabled = synthesiser.isVoiceStealingEnabled();
        state.mpeZoneLayout        = synthesiser.getZoneLayout();
        state.readerFactory        = readerFactory == nullptr ? nullptr : readerFactory->clone();

        auto sound = samplerSound;
        state.loopPointsSeconds = sound->getLoopPointsInSeconds();
        state.centreFrequencyHz = sound->getCentreFrequencyInHz();
        state.loopMode          = sound->getLoopMode();

        return new SamplerAudioProcessorEditor (*this, std::move (state));
    }

    b8 hasEditor() const override                                       { return true; }

    //==============================================================================
    const Txt getName() const override                                 { return "SamplerPlugin"; }
    b8 acceptsMidi() const override                                     { return true; }
    b8 producesMidi() const override                                    { return false; }
    b8 isMidiEffect() const override                                    { return false; }
    f64 getTailLengthSeconds() const override                          { return 0.0; }

    //==============================================================================
    i32 getNumPrograms() override                                         { return 1; }
    i32 getCurrentProgram() override                                      { return 0; }
    z0 setCurrentProgram (i32) override                                 {}
    const Txt getProgramName (i32) override                            { return "None"; }
    z0 changeProgramName (i32, const Txt&) override                  {}

    //==============================================================================
    z0 getStateInformation (MemoryBlock&) override                      {}
    z0 setStateInformation (ukk, i32) override                  {}

    //==============================================================================
    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer& midi) override
    {
        process (buffer, midi);
    }

    z0 processBlock (AudioBuffer<f64>& buffer, MidiBuffer& midi) override
    {
        process (buffer, midi);
    }

    // These should be called from the GUI thread, and will block until the
    // command buffer has enough room to accept a command.
    z0 setSample (std::unique_ptr<AudioFormatReaderFactory> fact, AudioFormatManager& formatManager)
    {
        class SetSampleCommand
        {
        public:
            SetSampleCommand (std::unique_ptr<AudioFormatReaderFactory> r,
                              std::unique_ptr<Sample> sampleIn,
                              std::vector<std::unique_ptr<MPESamplerVoice>> newVoicesIn)
                : readerFactory (std::move (r)),
                  sample (std::move (sampleIn)),
                  newVoices (std::move (newVoicesIn))
            {}

            z0 operator() (SamplerAudioProcessor& proc)
            {
                proc.readerFactory = std::move (readerFactory);
                auto sound = proc.samplerSound;
                sound->setSample (std::move (sample));
                auto numberOfVoices = proc.synthesiser.getNumVoices();
                proc.synthesiser.clearVoices();

                for (auto it = begin (newVoices); proc.synthesiser.getNumVoices() < numberOfVoices; ++it)
                {
                    proc.synthesiser.addVoice (it->release());
                }
            }

        private:
            std::unique_ptr<AudioFormatReaderFactory> readerFactory;
            std::unique_ptr<Sample> sample;
            std::vector<std::unique_ptr<MPESamplerVoice>> newVoices;
        };

        // Note that all allocation happens here, on the main message thread. Then,
        // we transfer ownership across to the audio thread.
        auto loadedSamplerSound = samplerSound;
        std::vector<std::unique_ptr<MPESamplerVoice>> newSamplerVoices;
        newSamplerVoices.reserve (maxVoices);

        for (auto i = 0; i != maxVoices; ++i)
            newSamplerVoices.emplace_back (new MPESamplerVoice (loadedSamplerSound));

        if (fact == nullptr)
        {
            commands.push (SetSampleCommand (std::move (fact),
                                             nullptr,
                                             std::move (newSamplerVoices)));
        }
        else if (auto reader = fact->make (formatManager))
        {
            commands.push (SetSampleCommand (std::move (fact),
                                             std::unique_ptr<Sample> (new Sample (*reader, 10.0)),
                                             std::move (newSamplerVoices)));
        }
    }

    z0 setCentreFrequency (f64 centreFrequency)
    {
        commands.push ([centreFrequency] (SamplerAudioProcessor& proc)
                       {
                           auto loaded = proc.samplerSound;
                           if (loaded != nullptr)
                               loaded->setCentreFrequencyInHz (centreFrequency);
                       });
    }

    z0 setLoopMode (LoopMode loopMode)
    {
        commands.push ([loopMode] (SamplerAudioProcessor& proc)
                       {
                           auto loaded = proc.samplerSound;
                           if (loaded != nullptr)
                               loaded->setLoopMode (loopMode);
                       });
    }

    z0 setLoopPoints (Range<f64> loopPoints)
    {
        commands.push ([loopPoints] (SamplerAudioProcessor& proc)
                       {
                           auto loaded = proc.samplerSound;
                           if (loaded != nullptr)
                               loaded->setLoopPointsInSeconds (loopPoints);
                       });
    }

    z0 setMPEZoneLayout (MPEZoneLayout layout)
    {
        commands.push ([layout] (SamplerAudioProcessor& proc)
                       {
                           // setZoneLayout will lock internally, so we don't care too much about
                           // ensuring that the layout doesn't get copied or destroyed on the
                           // audio thread. If the audio glitches while updating midi settings
                           // it doesn't matter too much.
                           proc.synthesiser.setZoneLayout (layout);
                       });
    }

    z0 setLegacyModeEnabled (i32 pitchbendRange, Range<i32> channelRange)
    {
        commands.push ([pitchbendRange, channelRange] (SamplerAudioProcessor& proc)
                       {
                           proc.synthesiser.enableLegacyMode (pitchbendRange, channelRange);
                       });
    }

    z0 setVoiceStealingEnabled (b8 voiceStealingEnabled)
    {
        commands.push ([voiceStealingEnabled] (SamplerAudioProcessor& proc)
                       {
                           proc.synthesiser.setVoiceStealingEnabled (voiceStealingEnabled);
                       });
    }

    z0 setNumberOfVoices (i32 numberOfVoices)
    {
        // We don't want to call 'new' on the audio thread. Normally, we'd
        // construct things here, on the GUI thread, and then move them into the
        // command lambda. Unfortunately, C++11 doesn't have extended lambda
        // capture, so we use a custom struct instead.

        class SetNumVoicesCommand
        {
        public:
            SetNumVoicesCommand (std::vector<std::unique_ptr<MPESamplerVoice>> newVoicesIn)
                : newVoices (std::move (newVoicesIn))
            {}

            z0 operator() (SamplerAudioProcessor& proc)
            {
                if ((i32) newVoices.size() < proc.synthesiser.getNumVoices())
                    proc.synthesiser.reduceNumVoices (i32 (newVoices.size()));
                else
                    for (auto it = begin (newVoices); (size_t) proc.synthesiser.getNumVoices() < newVoices.size(); ++it)
                        proc.synthesiser.addVoice (it->release());
            }

        private:
            std::vector<std::unique_ptr<MPESamplerVoice>> newVoices;
        };

        numberOfVoices = std::min ((i32) maxVoices, numberOfVoices);
        auto loadedSamplerSound = samplerSound;
        std::vector<std::unique_ptr<MPESamplerVoice>> newSamplerVoices;
        newSamplerVoices.reserve ((size_t) numberOfVoices);

        for (auto i = 0; i != numberOfVoices; ++i)
            newSamplerVoices.emplace_back (new MPESamplerVoice (loadedSamplerSound));

        commands.push (SetNumVoicesCommand (std::move (newSamplerVoices)));
    }

    // These accessors are just for an 'overview' and won't give the exact
    // state of the audio engine at a particular point in time.
    // If you call getNumVoices(), get the result '10', and then call
    // getPlaybackPosiiton (9), there's a chance the audio engine will have
    // been updated to remove some voices in the meantime, so the returned
    // value won't correspond to an existing voice.
    i32 getNumVoices() const                    { return synthesiser.getNumVoices(); }
    f32 getPlaybackPosition (i32 voice) const { return playbackPositions.at ((size_t) voice); }

private:
    //==============================================================================
    class SamplerAudioProcessorEditor final : public AudioProcessorEditor,
                                              public FileDragAndDropTarget,
                                              private DataModel::Listener,
                                              private MPESettingsDataModel::Listener
    {
    public:
        SamplerAudioProcessorEditor (SamplerAudioProcessor& p, ProcessorState state)
            : AudioProcessorEditor (&p),
              samplerAudioProcessor (p),
              mainSamplerView (dataModel,
                               [&p]
                               {
                                   std::vector<f32> ret;
                                   auto voices = p.getNumVoices();
                                   ret.reserve ((size_t) voices);

                                   for (auto i = 0; i != voices; ++i)
                                       ret.emplace_back (p.getPlaybackPosition (i));

                                   return ret;
                               },
                               undoManager)
        {
            dataModel.addListener (*this);
            mpeSettings.addListener (*this);

            formatManager.registerBasicFormats();

            addAndMakeVisible (tabbedComponent);

            auto lookFeel = dynamic_cast<LookAndFeel_V4*> (&getLookAndFeel());
            auto bg = lookFeel->getCurrentColorScheme()
                               .getUIColor (LookAndFeel_V4::ColorScheme::UIColor::widgetBackground);

            tabbedComponent.addTab ("Sample Editor", bg, &mainSamplerView, false);
            tabbedComponent.addTab ("MPE Settings", bg, &settingsComponent, false);

            mpeSettings.setSynthVoices          (state.synthVoices,               nullptr);
            mpeSettings.setLegacyModeEnabled    (state.legacyModeEnabled,         nullptr);
            mpeSettings.setLegacyFirstChannel   (state.legacyChannels.getStart(), nullptr);
            mpeSettings.setLegacyLastChannel    (state.legacyChannels.getEnd(),   nullptr);
            mpeSettings.setLegacyPitchbendRange (state.legacyPitchbendRange,      nullptr);
            mpeSettings.setVoiceStealingEnabled (state.voiceStealingEnabled,      nullptr);
            mpeSettings.setMPEZoneLayout        (state.mpeZoneLayout,             nullptr);

            dataModel.setSampleReader (std::move (state.readerFactory), nullptr);

            dataModel.setLoopPointsSeconds  (state.loopPointsSeconds, nullptr);
            dataModel.setCentreFrequencyHz  (state.centreFrequencyHz, nullptr);
            dataModel.setLoopMode           (state.loopMode,          nullptr);

            // Make sure that before the constructor has finished, you've set the
            // editor's size to whatever you need it to be.
            setResizable (true, true);
            setResizeLimits (640, 480, 2560, 1440);
            setSize (640, 480);
        }

    private:
        z0 resized() override
        {
            tabbedComponent.setBounds (getLocalBounds());
        }

        b8 keyPressed (const KeyPress& key) override
        {
            if (key == KeyPress ('z', ModifierKeys::commandModifier, 0))
            {
                undoManager.undo();
                return true;
            }

            if (key == KeyPress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
            {
                undoManager.redo();
                return true;
            }

            return Component::keyPressed (key);
        }

        b8 isInterestedInFileDrag (const StringArray& files) override
        {
            WildcardFileFilter filter (formatManager.getWildcardForAllFormats(), {}, "Known Audio Formats");
            return files.size() == 1 && filter.isFileSuitable (files[0]);
        }

        z0 filesDropped (const StringArray& files, i32, i32) override
        {
            jassert (files.size() == 1);
            undoManager.beginNewTransaction();
            auto r = new FileAudioFormatReaderFactory (files[0]);
            dataModel.setSampleReader (std::unique_ptr<AudioFormatReaderFactory> (r),
                                       &undoManager);

        }

        z0 sampleReaderChanged (std::shared_ptr<AudioFormatReaderFactory> value) override
        {
            samplerAudioProcessor.setSample (value == nullptr ? nullptr : value->clone(),
                                             dataModel.getAudioFormatManager());
        }

        z0 centreFrequencyHzChanged (f64 value) override
        {
            samplerAudioProcessor.setCentreFrequency (value);
        }

        z0 loopPointsSecondsChanged (Range<f64> value) override
        {
            samplerAudioProcessor.setLoopPoints (value);
        }

        z0 loopModeChanged (LoopMode value) override
        {
            samplerAudioProcessor.setLoopMode (value);
        }

        z0 synthVoicesChanged (i32 value) override
        {
            samplerAudioProcessor.setNumberOfVoices (value);
        }

        z0 voiceStealingEnabledChanged (b8 value) override
        {
            samplerAudioProcessor.setVoiceStealingEnabled (value);
        }

        z0 legacyModeEnabledChanged (b8 value) override
        {
            if (value)
                setProcessorLegacyMode();
            else
                setProcessorMPEMode();
        }

        z0 mpeZoneLayoutChanged (const MPEZoneLayout&) override
        {
            setProcessorMPEMode();
        }

        z0 legacyFirstChannelChanged (i32) override
        {
            setProcessorLegacyMode();
        }

        z0 legacyLastChannelChanged (i32) override
        {
            setProcessorLegacyMode();
        }

        z0 legacyPitchbendRangeChanged (i32) override
        {
            setProcessorLegacyMode();
        }

        z0 setProcessorLegacyMode()
        {
            samplerAudioProcessor.setLegacyModeEnabled (mpeSettings.getLegacyPitchbendRange(),
                                                        Range<i32> (mpeSettings.getLegacyFirstChannel(),
                                                        mpeSettings.getLegacyLastChannel()));
        }

        z0 setProcessorMPEMode()
        {
            samplerAudioProcessor.setMPEZoneLayout (mpeSettings.getMPEZoneLayout());
        }

        SamplerAudioProcessor& samplerAudioProcessor;
        AudioFormatManager formatManager;
        DataModel dataModel { formatManager };
        UndoManager undoManager;
        MPESettingsDataModel mpeSettings { dataModel.mpeSettings() };

        TabbedComponent tabbedComponent { TabbedButtonBar::Orientation::TabsAtTop };
        MPESettingsComponent settingsComponent { dataModel.mpeSettings(), undoManager };
        MainSamplerView mainSamplerView;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessorEditor)
    };

    //==============================================================================
    template <typename Element>
    z0 process (AudioBuffer<Element>& buffer, MidiBuffer& midiMessages)
    {
        // Try to acquire a lock on the command queue.
        // If we were successful, we pop all pending commands off the queue and
        // apply them to the processor.
        // If we weren't able to acquire the lock, it's because someone called
        // createEditor, which requires that the processor data model stays in
        // a valid state for the duration of the call.
        const GenericScopedTryLock<SpinLock> lock (commandQueueMutex);

        if (lock.isLocked())
            commands.call (*this);

        synthesiser.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

        auto loadedSamplerSound = samplerSound;

        if (loadedSamplerSound->getSample() == nullptr)
            return;

        auto numVoices = synthesiser.getNumVoices();

        // Update the current playback positions
        for (auto i = 0; i < maxVoices; ++i)
        {
            auto* voicePtr = dynamic_cast<MPESamplerVoice*> (synthesiser.getVoice (i));

            if (i < numVoices && voicePtr != nullptr)
                playbackPositions[(size_t) i] = static_cast<f32> (voicePtr->getCurrentSamplePosition() / loadedSamplerSound->getSample()->getSampleRate());
            else
                playbackPositions[(size_t) i] = 0.0f;
        }

    }

    CommandFifo<SamplerAudioProcessor> commands;

    std::unique_ptr<AudioFormatReaderFactory> readerFactory;
    std::shared_ptr<MPESamplerSound> samplerSound = std::make_shared<MPESamplerSound>();
    MPESynthesiser synthesiser;

    // This mutex is used to ensure we don't modify the processor state during
    // a call to createEditor, which would cause the UI to become desynched
    // with the real state of the processor.
    SpinLock commandQueueMutex;

    enum { maxVoices = 20 };

    // This is used for visualising the current playback position of each voice.
    std::array<std::atomic<f32>, maxVoices> playbackPositions;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerAudioProcessor)
};
