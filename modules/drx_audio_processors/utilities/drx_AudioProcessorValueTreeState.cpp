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

namespace drx
{

//==============================================================================

AudioProcessorValueTreeState::Parameter::Parameter (const ParameterID& parameterID,
                                                    const Txt& parameterName,
                                                    NormalisableRange<f32> valueRange,
                                                    f32 defaultParameterValue,
                                                    const AudioProcessorValueTreeStateParameterAttributes& attributes)
    : AudioParameterFloat (parameterID,
                           parameterName,
                           valueRange,
                           defaultParameterValue,
                           attributes.getAudioParameterFloatAttributes()),
      unsnappedDefault (valueRange.convertTo0to1 (defaultParameterValue)),
      discrete (attributes.getDiscrete()),
      boolean (attributes.getBoolean())
{
}

f32 AudioProcessorValueTreeState::Parameter::getDefaultValue() const  { return unsnappedDefault; }
i32 AudioProcessorValueTreeState::Parameter::getNumSteps() const        { return RangedAudioParameter::getNumSteps(); }

b8 AudioProcessorValueTreeState::Parameter::isDiscrete() const        { return discrete; }
b8 AudioProcessorValueTreeState::Parameter::isBoolean() const         { return boolean; }

z0 AudioProcessorValueTreeState::Parameter::valueChanged (f32 newValue)
{
    if (approximatelyEqual ((f32) lastValue, newValue))
        return;

    lastValue = newValue;
    NullCheckedInvocation::invoke (onValueChanged);
}

//==============================================================================
class AudioProcessorValueTreeState::ParameterAdapter final : private AudioProcessorParameter::Listener
{
private:
    using Listener = AudioProcessorValueTreeState::Listener;

public:
    explicit ParameterAdapter (RangedAudioParameter& parameterIn)
        : parameter (parameterIn),
          // For legacy reasons, the unnormalised value should *not* be snapped on construction
          unnormalisedValue (getRange().convertFrom0to1 (parameter.getDefaultValue()))
    {
        parameter.addListener (this);

        if (auto* ptr = dynamic_cast<Parameter*> (&parameter))
            ptr->onValueChanged = [this] { parameterValueChanged ({}, {}); };
    }

    ~ParameterAdapter() override        { parameter.removeListener (this); }

    z0 addListener (Listener* l)      { listeners.add (l); }
    z0 removeListener (Listener* l)   { listeners.remove (l); }

    RangedAudioParameter& getParameter()                { return parameter; }
    const RangedAudioParameter& getParameter() const    { return parameter; }

    const NormalisableRange<f32>& getRange() const    { return parameter.getNormalisableRange(); }

    f32 getDenormalisedDefaultValue() const    { return denormalise (parameter.getDefaultValue()); }

    z0 setDenormalisedValue (f32 value)
    {
        if (! approximatelyEqual (value, (f32) unnormalisedValue))
            setNormalisedValue (normalise (value));
    }

    f32 getDenormalisedValueForText (const Txt& text) const
    {
        return denormalise (parameter.getValueForText (text));
    }

    Txt getTextForDenormalisedValue (f32 value) const
    {
        return parameter.getText (normalise (value), 0);
    }

    f32 getDenormalisedValue() const                { return unnormalisedValue; }
    std::atomic<f32>& getRawDenormalisedValue()     { return unnormalisedValue; }

    b8 flushToTree (const Identifier& key, UndoManager* um)
    {
        auto needsUpdateTestValue = true;

        if (! needsUpdate.compare_exchange_strong (needsUpdateTestValue, false))
            return false;

        if (auto* valueProperty = tree.getPropertyPointer (key))
        {
            if (! approximatelyEqual ((f32) *valueProperty, unnormalisedValue.load()))
            {
                ScopedValueSetter<b8> svs (ignoreParameterChangedCallbacks, true);
                tree.setProperty (key, unnormalisedValue.load(), um);
            }
        }
        else
        {
            tree.setProperty (key, unnormalisedValue.load(), nullptr);
        }

        return true;
    }

    ValueTree tree;

private:
    z0 parameterGestureChanged (i32, b8) override {}

    z0 parameterValueChanged (i32, f32) override
    {
        const auto newValue = denormalise (parameter.getValue());

        if (! listenersNeedCalling && approximatelyEqual ((f32) unnormalisedValue, newValue))
            return;

        unnormalisedValue = newValue;
        listeners.call ([this] (Listener& l) { l.parameterChanged (parameter.paramID, unnormalisedValue); });
        listenersNeedCalling = false;
        needsUpdate = true;
    }

    f32 denormalise (f32 normalised) const
    {
        return getParameter().convertFrom0to1 (normalised);
    }

    f32 normalise (f32 denormalised) const
    {
        return getParameter().convertTo0to1 (denormalised);
    }

    z0 setNormalisedValue (f32 value)
    {
        if (ignoreParameterChangedCallbacks)
            return;

        parameter.setValueNotifyingHost (value);
    }

    class LockedListeners
    {
    public:
        template <typename Fn>
        z0 call (Fn&& fn)
        {
            const CriticalSection::ScopedLockType lock (mutex);
            listeners.call (std::forward<Fn> (fn));
        }

        z0 add (Listener* l)
        {
            const CriticalSection::ScopedLockType lock (mutex);
            listeners.add (l);
        }

        z0 remove (Listener* l)
        {
            const CriticalSection::ScopedLockType lock (mutex);
            listeners.remove (l);
        }

    private:
        CriticalSection mutex;
        ListenerList<Listener> listeners;
    };

    RangedAudioParameter& parameter;
    LockedListeners listeners;
    std::atomic<f32> unnormalisedValue { 0.0f };
    std::atomic<b8> needsUpdate { true }, listenersNeedCalling { true };
    b8 ignoreParameterChangedCallbacks { false };
};

//==============================================================================
AudioProcessorValueTreeState::AudioProcessorValueTreeState (AudioProcessor& processorToConnectTo,
                                                            UndoManager* undoManagerToUse,
                                                            const Identifier& valueTreeType,
                                                            ParameterLayout parameterLayout)
    : AudioProcessorValueTreeState (processorToConnectTo, undoManagerToUse)
{
    struct PushBackVisitor final : ParameterLayout::Visitor
    {
        explicit PushBackVisitor (AudioProcessorValueTreeState& stateIn)
            : state (&stateIn) {}

        z0 visit (std::unique_ptr<RangedAudioParameter> param) const override
        {
            if (param == nullptr)
            {
                jassertfalse;
                return;
            }

            state->addParameterAdapter (*param);
            state->processor.addParameter (param.release());
        }

        z0 visit (std::unique_ptr<AudioProcessorParameterGroup> group) const override
        {
            if (group == nullptr)
            {
                jassertfalse;
                return;
            }

            for (const auto param : group->getParameters (true))
            {
                if (const auto rangedParam = dynamic_cast<RangedAudioParameter*> (param))
                {
                    state->addParameterAdapter (*rangedParam);
                }
                else
                {
                    // If you hit this assertion then you are attempting to add a parameter that is
                    // not derived from RangedAudioParameter to the AudioProcessorValueTreeState.
                    jassertfalse;
                }
            }

            state->processor.addParameterGroup (std::move (group));
        }

        AudioProcessorValueTreeState* state;
    };

    for (auto& item : parameterLayout.parameters)
        item->accept (PushBackVisitor (*this));

    state = ValueTree (valueTreeType);
}

AudioProcessorValueTreeState::AudioProcessorValueTreeState (AudioProcessor& p, UndoManager* um)
    : processor (p), undoManager (um)
{
    startTimerHz (10);
    state.addListener (this);
}

AudioProcessorValueTreeState::~AudioProcessorValueTreeState()
{
    stopTimer();
}

//==============================================================================
RangedAudioParameter* AudioProcessorValueTreeState::createAndAddParameter (const Txt& paramID,
                                                                           const Txt& paramName,
                                                                           const Txt& labelText,
                                                                           NormalisableRange<f32> range,
                                                                           f32 defaultVal,
                                                                           std::function<Txt (f32)> valueToTextFunction,
                                                                           std::function<f32 (const Txt&)> textToValueFunction,
                                                                           b8 isMetaParameter,
                                                                           b8 isAutomatableParameter,
                                                                           b8 isDiscreteParameter,
                                                                           AudioProcessorParameter::Category category,
                                                                           b8 isBooleanParameter)
{
    auto attributes = AudioProcessorValueTreeStateParameterAttributes()
                          .withLabel (labelText)
                          .withStringFromValueFunction ([fn = std::move (valueToTextFunction)] (f32 v, i32) { return fn (v); })
                          .withValueFromStringFunction (std::move (textToValueFunction))
                          .withMeta (isMetaParameter)
                          .withAutomatable (isAutomatableParameter)
                          .withDiscrete (isDiscreteParameter)
                          .withCategory (category)
                          .withBoolean (isBooleanParameter);

    return createAndAddParameter (std::make_unique<Parameter> (paramID,
                                                               paramName,
                                                               range,
                                                               defaultVal,
                                                               std::move (attributes)));
}

RangedAudioParameter* AudioProcessorValueTreeState::createAndAddParameter (std::unique_ptr<RangedAudioParameter> param)
{
    if (param == nullptr)
        return nullptr;

    // All parameters must be created before giving this manager a ValueTree state!
    jassert (! state.isValid());

    if (getParameter (param->paramID) != nullptr)
        return nullptr;

    addParameterAdapter (*param);

    processor.addParameter (param.get());

    return param.release();
}

//==============================================================================
z0 AudioProcessorValueTreeState::addParameterAdapter (RangedAudioParameter& param)
{
    adapterTable.emplace (param.paramID, std::make_unique<ParameterAdapter> (param));
}

AudioProcessorValueTreeState::ParameterAdapter* AudioProcessorValueTreeState::getParameterAdapter (StringRef paramID) const
{
    auto it = adapterTable.find (paramID);
    return it == adapterTable.end() ? nullptr : it->second.get();
}

z0 AudioProcessorValueTreeState::addParameterListener (StringRef paramID, Listener* listener)
{
    if (auto* p = getParameterAdapter (paramID))
        p->addListener (listener);
}

z0 AudioProcessorValueTreeState::removeParameterListener (StringRef paramID, Listener* listener)
{
    if (auto* p = getParameterAdapter (paramID))
        p->removeListener (listener);
}

Value AudioProcessorValueTreeState::getParameterAsValue (StringRef paramID) const
{
    if (auto* adapter = getParameterAdapter (paramID))
        if (adapter->tree.isValid())
            return adapter->tree.getPropertyAsValue (valuePropertyID, undoManager);

    return {};
}

NormalisableRange<f32> AudioProcessorValueTreeState::getParameterRange (StringRef paramID) const noexcept
{
    if (auto* p = getParameterAdapter (paramID))
        return p->getRange();

    return {};
}

RangedAudioParameter* AudioProcessorValueTreeState::getParameter (StringRef paramID) const noexcept
{
    if (auto adapter = getParameterAdapter (paramID))
        return &adapter->getParameter();

    return nullptr;
}

std::atomic<f32>* AudioProcessorValueTreeState::getRawParameterValue (StringRef paramID) const noexcept
{
    if (auto* p = getParameterAdapter (paramID))
        return &p->getRawDenormalisedValue();

    return nullptr;
}

ValueTree AudioProcessorValueTreeState::copyState()
{
    ScopedLock lock (valueTreeChanging);
    flushParameterValuesToValueTree();
    return state.createCopy();
}

z0 AudioProcessorValueTreeState::replaceState (const ValueTree& newState)
{
    ScopedLock lock (valueTreeChanging);

    state = newState;

    if (undoManager != nullptr)
        undoManager->clearUndoHistory();
}

z0 AudioProcessorValueTreeState::setNewState (ValueTree vt)
{
    jassert (vt.getParent() == state);

    if (auto* p = getParameterAdapter (vt.getProperty (idPropertyID).toString()))
    {
        p->tree = vt;
        p->setDenormalisedValue (p->tree.getProperty (valuePropertyID, p->getDenormalisedDefaultValue()));
    }
}

z0 AudioProcessorValueTreeState::updateParameterConnectionsToChildTrees()
{
    ScopedLock lock (valueTreeChanging);

    for (auto& p : adapterTable)
        p.second->tree = ValueTree();

    for (const auto& child : state)
        setNewState (child);

    for (auto& p : adapterTable)
    {
        auto& adapter = *p.second;

        if (! adapter.tree.isValid())
        {
            adapter.tree = ValueTree (valueType);
            adapter.tree.setProperty (idPropertyID, adapter.getParameter().paramID, nullptr);
            state.appendChild (adapter.tree, nullptr);
        }
    }

    flushParameterValuesToValueTree();
}

z0 AudioProcessorValueTreeState::valueTreePropertyChanged (ValueTree& tree, const Identifier&)
{
    if (tree.hasType (valueType) && tree.getParent() == state)
        setNewState (tree);
}

z0 AudioProcessorValueTreeState::valueTreeChildAdded (ValueTree& parent, ValueTree& tree)
{
    if (parent == state && tree.hasType (valueType))
        setNewState (tree);
}

z0 AudioProcessorValueTreeState::valueTreeRedirected (ValueTree& v)
{
    if (v == state)
        updateParameterConnectionsToChildTrees();
}

b8 AudioProcessorValueTreeState::flushParameterValuesToValueTree()
{
    ScopedLock lock (valueTreeChanging);

    b8 anyUpdated = false;

    for (auto& p : adapterTable)
        anyUpdated |= p.second->flushToTree (valuePropertyID, undoManager);

    return anyUpdated;
}

z0 AudioProcessorValueTreeState::timerCallback()
{
    auto anythingUpdated = flushParameterValuesToValueTree();

    startTimer (anythingUpdated ? 1000 / 50
                                : jlimit (50, 500, getTimerInterval() + 20));
}

//==============================================================================
template <typename Attachment, typename Control>
std::unique_ptr<Attachment> makeAttachment (const AudioProcessorValueTreeState& stateToUse,
                                            const Txt& parameterID,
                                            Control& control)
{
    if (auto* parameter = stateToUse.getParameter (parameterID))
        return std::make_unique<Attachment> (*parameter, control, stateToUse.undoManager);

    jassertfalse;
    return nullptr;
}

AudioProcessorValueTreeState::SliderAttachment::SliderAttachment (AudioProcessorValueTreeState& stateToUse,
                                                                  const Txt& parameterID,
                                                                  Slider& slider)
    : attachment (makeAttachment<SliderParameterAttachment> (stateToUse, parameterID, slider))
{
}

AudioProcessorValueTreeState::ComboBoxAttachment::ComboBoxAttachment (AudioProcessorValueTreeState& stateToUse,
                                                                      const Txt& parameterID,
                                                                      ComboBox& combo)
    : attachment (makeAttachment<ComboBoxParameterAttachment> (stateToUse, parameterID, combo))
{
}

AudioProcessorValueTreeState::ButtonAttachment::ButtonAttachment (AudioProcessorValueTreeState& stateToUse,
                                                                  const Txt& parameterID,
                                                                  Button& button)
    : attachment (makeAttachment<ButtonParameterAttachment> (stateToUse, parameterID, button))
{
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

struct ParameterAdapterTests final : public UnitTest
{
    ParameterAdapterTests()
        : UnitTest ("Parameter Adapter", UnitTestCategories::audioProcessorParameters)
    {}

    z0 runTest() override
    {
        beginTest ("The default value is returned correctly");
        {
            const auto test = [&] (NormalisableRange<f32> range, f32 value)
            {
                AudioParameterFloat param ({}, {}, range, value);

                AudioProcessorValueTreeState::ParameterAdapter adapter (param);

                expectEquals (adapter.getDenormalisedDefaultValue(), value);
            };

            test ({ -100, 100 }, 0);
            test ({ -2.5, 12.5 }, 10);
        }

        beginTest ("Denormalised parameter values can be retrieved");
        {
            const auto test = [&] (NormalisableRange<f32> range, f32 value)
            {
                AudioParameterFloat param ({}, {}, range, {});
                AudioProcessorValueTreeState::ParameterAdapter adapter (param);

                adapter.setDenormalisedValue (value);

                expectEquals (adapter.getDenormalisedValue(), value);
                expectEquals (adapter.getRawDenormalisedValue().load(), value);
            };

            test ({ -20, -10 }, -15);
            test ({ 0, 7.5 }, 2.5);
        }

        beginTest ("Floats can be converted to text");
        {
            const auto test = [&] (NormalisableRange<f32> range, f32 value, Txt expected)
            {
                AudioParameterFloat param ({}, {}, range, {});
                AudioProcessorValueTreeState::ParameterAdapter adapter (param);

                expectEquals (adapter.getTextForDenormalisedValue (value), expected);
            };

            test ({ -100, 100 }, 0, "0.0000000");
            test ({ -2.5, 12.5 }, 10, "10.0000000");
            test ({ -20, -10 }, -15, "-15.0000000");
            test ({ 0, 7.5 }, 2.5, "2.5000000");
        }

        beginTest ("Text can be converted to floats");
        {
            const auto test = [&] (NormalisableRange<f32> range, Txt text, f32 expected)
            {
                AudioParameterFloat param ({}, {}, range, {});
                AudioProcessorValueTreeState::ParameterAdapter adapter (param);

                expectEquals (adapter.getDenormalisedValueForText (text), expected);
            };

            test ({ -100, 100 }, "0.0", 0);
            test ({ -2.5, 12.5 }, "10.0", 10);
            test ({ -20, -10 }, "-15.0", -15);
            test ({ 0, 7.5 }, "2.5", 2.5);
        }
    }
};

static ParameterAdapterTests parameterAdapterTests;

namespace
{
template <typename ValueType>
inline b8 operator== (const NormalisableRange<ValueType>& a,
                        const NormalisableRange<ValueType>& b)
{
    return std::tie (a.start, a.end, a.interval, a.skew, a.symmetricSkew)
           == std::tie (b.start, b.end, b.interval, b.skew, b.symmetricSkew);
}

template <typename ValueType>
inline b8 operator!= (const NormalisableRange<ValueType>& a,
                        const NormalisableRange<ValueType>& b)
{
    return ! (a == b);
}
} // namespace

class AudioProcessorValueTreeStateTests final : public UnitTest
{
private:
    using Parameter = AudioProcessorValueTreeState::Parameter;
    using ParameterGroup = AudioProcessorParameterGroup;
    using ParameterLayout = AudioProcessorValueTreeState::ParameterLayout;
    using Attributes = AudioProcessorValueTreeStateParameterAttributes;

    class TestAudioProcessor final : public AudioProcessor
    {
    public:
        TestAudioProcessor() = default;

        explicit TestAudioProcessor (ParameterLayout layout)
            : state (*this, nullptr, "state", std::move (layout)) {}

        const Txt getName() const override { return {}; }
        z0 prepareToPlay (f64, i32) override {}
        z0 releaseResources() override {}
        z0 processBlock (AudioBuffer<f32>&, MidiBuffer&) override {}
        using AudioProcessor::processBlock;
        f64 getTailLengthSeconds() const override { return {}; }
        b8 acceptsMidi() const override { return {}; }
        b8 producesMidi() const override { return {}; }
        AudioProcessorEditor* createEditor() override { return {}; }
        b8 hasEditor() const override { return {}; }
        i32 getNumPrograms() override { return 1; }
        i32 getCurrentProgram() override { return {}; }
        z0 setCurrentProgram (i32) override {}
        const Txt getProgramName (i32) override { return {}; }
        z0 changeProgramName (i32, const Txt&) override {}
        z0 getStateInformation (MemoryBlock&) override {}
        z0 setStateInformation (ukk, i32) override {}

        AudioProcessorValueTreeState state { *this, nullptr };
    };

    struct Listener final : public AudioProcessorValueTreeState::Listener
    {
        z0 parameterChanged (const Txt& idIn, f32 valueIn) override
        {
            id = idIn;
            value = valueIn;
        }

        Txt id;
        f32 value{};
    };

public:
    AudioProcessorValueTreeStateTests()
        : UnitTest ("Audio Processor Value Tree State", UnitTestCategories::audioProcessorParameters)
    {}

    DRX_BEGIN_IGNORE_WARNINGS_MSVC (6262)
    z0 runTest() override
    {
        ScopedDrxInitialiser_GUI scopedDrxInitialiser_gui;

        beginTest ("After calling createAndAddParameter, the number of parameters increases by one");
        {
            TestAudioProcessor proc;

            proc.state.createAndAddParameter (std::make_unique<Parameter> (
                Txt(),
                Txt(),
                NormalisableRange<f32>(),
                0.0f));

            expectEquals (proc.getParameters().size(), 1);
        }

        beginTest ("After creating a normal named parameter, we can later retrieve that parameter");
        {
            TestAudioProcessor proc;

            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f));

            expect (proc.state.getParameter (key) == param);
        }

        beginTest ("After construction, the value tree has the expected format");
        {
            TestAudioProcessor proc ({
                std::make_unique<AudioProcessorParameterGroup> ("A", "", "",
                    std::make_unique<AudioParameterBool> ("a", "", false),
                    std::make_unique<AudioParameterFloat> ("b", "", NormalisableRange<f32>{}, 0.0f)),
                std::make_unique<AudioProcessorParameterGroup> ("B", "", "",
                    std::make_unique<AudioParameterInt> ("c", "", 0, 1, 0),
                    std::make_unique<AudioParameterChoice> ("d", "", StringArray { "foo", "bar" }, 0)) });

            const auto valueTree = proc.state.copyState();

            expectEquals (valueTree.getNumChildren(), 4);

            for (auto child : valueTree)
            {
                expect (child.hasType ("PARAM"));
                expect (child.hasProperty ("id"));
                expect (child.hasProperty ("value"));
            }
        }

        beginTest ("Meta parameters can be created");
        {
            TestAudioProcessor proc;

            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f,
                                   Attributes().withMeta (true)));

            expect (param->isMetaParameter());
        }

        beginTest ("Automatable parameters can be created");
        {
            TestAudioProcessor proc;

            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f,
                                   Attributes().withAutomatable (true)));

            expect (param->isAutomatable());
        }

        beginTest ("Discrete parameters can be created");
        {
            TestAudioProcessor proc;

            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f,
                                   Attributes().withDiscrete (true)));

            expect (param->isDiscrete());
        }

        beginTest ("Custom category parameters can be created");
        {
            TestAudioProcessor proc;

            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f,
                                   Attributes().withCategory (AudioProcessorParameter::Category::inputMeter)));

            expect (param->category == AudioProcessorParameter::Category::inputMeter);
        }

        beginTest ("Boolean parameters can be created");
        {
            TestAudioProcessor proc;

            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f,
                                   Attributes().withBoolean (true)));

            expect (param->isBoolean());
        }

        beginTest ("After creating a custom named parameter, we can later retrieve that parameter");
        {
            const auto key = "id";
            auto param = std::make_unique<AudioParameterBool> (key, "", false);
            const auto paramPtr = param.get();

            TestAudioProcessor proc (std::move (param));

            expect (proc.state.getParameter (key) == paramPtr);
        }

        beginTest ("After adding a normal parameter that already exists, the AudioProcessor parameters are unchanged");
        {
            TestAudioProcessor proc;
            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f));

            proc.state.createAndAddParameter (std::make_unique<Parameter> (
                key,
                Txt(),
                NormalisableRange<f32>(),
                0.0f));

            expectEquals (proc.getParameters().size(), 1);
            expect (proc.getParameters().getFirst() == param);
        }

        beginTest ("After setting a parameter value, that value is reflected in the state");
        {
            TestAudioProcessor proc;
            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f));

            const auto value = 0.5f;
            param->setValueNotifyingHost (value);

            expectEquals (proc.state.getRawParameterValue (key)->load(), value);
        }

        beginTest ("After adding an APVTS::Parameter, its value is the default value");
        {
            TestAudioProcessor proc;
            const auto key = "id";
            const auto value = 5.0f;

            proc.state.createAndAddParameter (std::make_unique<Parameter> (
                key,
                Txt(),
                NormalisableRange<f32> (0.0f, 100.0f, 10.0f),
                value));

            expectEquals (proc.state.getRawParameterValue (key)->load(), value);
        }

        beginTest ("Listeners receive notifications when parameters change");
        {
            Listener listener;
            TestAudioProcessor proc;
            const auto key = "id";
            const auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                                   key,
                                   Txt(),
                                   NormalisableRange<f32>(),
                                   0.0f));
            proc.state.addParameterListener (key, &listener);

            const auto value = 0.5f;
            param->setValueNotifyingHost (value);

            expectEquals (listener.id, Txt { key });
            expectEquals (listener.value, value);
        }

        beginTest ("Bool parameters have a range of 0-1");
        {
            const auto key = "id";

            TestAudioProcessor proc (std::make_unique<AudioParameterBool> (key, "", false));

            expect (proc.state.getParameterRange (key) == NormalisableRange<f32> (0.0f, 1.0f, 1.0f));
        }

        beginTest ("Float parameters retain their specified range");
        {
            const auto key = "id";
            const auto range = NormalisableRange<f32> { -100, 100, 0.7f, 0.2f, true };

            TestAudioProcessor proc (std::make_unique<AudioParameterFloat> (key, "", range, 0.0f));

            expect (proc.state.getParameterRange (key) == range);
        }

        beginTest ("Int parameters retain their specified range");
        {
            const auto key = "id";
            const auto min = -27;
            const auto max = 53;

            TestAudioProcessor proc (std::make_unique<AudioParameterInt> (key, "", min, max, 0));

            expect (proc.state.getParameterRange (key) == NormalisableRange<f32> (f32 (min), f32 (max), 1.0f));
        }

        beginTest ("Choice parameters retain their specified range");
        {
            const auto key = "id";
            const auto choices = StringArray { "", "", "" };

            TestAudioProcessor proc (std::make_unique<AudioParameterChoice> (key, "", choices, 0));

            expect (proc.state.getParameterRange (key) == NormalisableRange<f32> (0.0f, (f32) (choices.size() - 1), 1.0f));
            expect (proc.state.getParameter (key)->getNumSteps() == choices.size());
        }

        beginTest ("When the parameter value is changed, normal parameter values are updated");
        {
            TestAudioProcessor proc;
            const auto key = "id";
            const auto initialValue = 0.2f;
            auto param = proc.state.createAndAddParameter (std::make_unique<Parameter> (
                             key,
                             Txt(),
                             NormalisableRange<f32>(),
                             initialValue));
            proc.state.state = ValueTree { "state" };

            auto value = proc.state.getParameterAsValue (key);
            expectEquals (f32 (value.getValue()), initialValue);

            const auto newValue = 0.75f;
            value = newValue;

            expectEquals (param->getValue(), newValue);
            expectEquals (proc.state.getRawParameterValue (key)->load(), newValue);
        }

        beginTest ("When the parameter value is changed, custom parameter values are updated");
        {
            const auto key = "id";
            const auto choices = StringArray ("foo", "bar", "baz");
            auto param = std::make_unique<AudioParameterChoice> (key, "", choices, 0);
            const auto paramPtr = param.get();
            TestAudioProcessor proc (std::move (param));

            const auto newValue = 2.0f;
            auto value = proc.state.getParameterAsValue (key);
            value = newValue;

            expectEquals (paramPtr->getCurrentChoiceName(), choices[i32 (newValue)]);
            expectEquals (proc.state.getRawParameterValue (key)->load(), newValue);
        }

        beginTest ("When the parameter value is changed, listeners are notified");
        {
            Listener listener;
            TestAudioProcessor proc;
            const auto key = "id";
            proc.state.createAndAddParameter (std::make_unique<Parameter> (
                key,
                Txt(),
                NormalisableRange<f32>(),
                0.0f));
            proc.state.addParameterListener (key, &listener);
            proc.state.state = ValueTree { "state" };

            const auto newValue = 0.75f;
            proc.state.getParameterAsValue (key) = newValue;

            expectEquals (listener.value, newValue);
            expectEquals (listener.id, Txt { key });
        }

        beginTest ("When the parameter value is changed, listeners are notified");
        {
            const auto key = "id";
            const auto choices = StringArray { "foo", "bar", "baz" };
            Listener listener;
            TestAudioProcessor proc (std::make_unique<AudioParameterChoice> (key, "", choices, 0));
            proc.state.addParameterListener (key, &listener);

            const auto newValue = 2.0f;
            proc.state.getParameterAsValue (key) = newValue;

            expectEquals (listener.value, newValue);
            expectEquals (listener.id, Txt (key));
        }
    }
    DRX_END_IGNORE_WARNINGS_MSVC
};

static AudioProcessorValueTreeStateTests audioProcessorValueTreeStateTests;

#endif

} // namespace drx
