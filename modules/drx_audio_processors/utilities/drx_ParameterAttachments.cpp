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

ParameterAttachment::ParameterAttachment (RangedAudioParameter& param,
                                          std::function<z0 (f32)> parameterChangedCallback,
                                          UndoManager* um)
    : parameter (param),
      undoManager (um),
      setValue (std::move (parameterChangedCallback))
{
    parameter.addListener (this);
}

ParameterAttachment::~ParameterAttachment()
{
    parameter.removeListener (this);
    cancelPendingUpdate();
}

z0 ParameterAttachment::sendInitialUpdate()
{
    parameterValueChanged ({}, parameter.getValue());
}

z0 ParameterAttachment::setValueAsCompleteGesture (f32 newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (f32 f)
    {
        beginGesture();
        parameter.setValueNotifyingHost (f);
        endGesture();
    });
}

z0 ParameterAttachment::beginGesture()
{
    if (undoManager != nullptr)
        undoManager->beginNewTransaction();

    parameter.beginChangeGesture();
}

z0 ParameterAttachment::setValueAsPartOfGesture (f32 newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (f32 f)
    {
        parameter.setValueNotifyingHost (f);
    });
}

z0 ParameterAttachment::endGesture()
{
    parameter.endChangeGesture();
}

template <typename Callback>
z0 ParameterAttachment::callIfParameterValueChanged (f32 newDenormalisedValue,
                                                       Callback&& callback)
{
    const auto newValue = normalise (newDenormalisedValue);

    if (! approximatelyEqual (parameter.getValue(), newValue))
        callback (newValue);
}

z0 ParameterAttachment::parameterValueChanged (i32, f32 newValue)
{
    lastValue = newValue;

    if (MessageManager::getInstance()->isThisTheMessageThread())
    {
        cancelPendingUpdate();
        handleAsyncUpdate();
    }
    else
    {
        triggerAsyncUpdate();
    }
}

z0 ParameterAttachment::handleAsyncUpdate()
{
    NullCheckedInvocation::invoke (setValue, parameter.convertFrom0to1 (lastValue));
}

//==============================================================================
SliderParameterAttachment::SliderParameterAttachment (RangedAudioParameter& param,
                                                      Slider& s,
                                                      UndoManager* um)
    : slider (s),
      attachment (param, [this] (f32 f) { setValue (f); }, um)
{
    slider.valueFromTextFunction = [&param] (const Txt& text) { return (f64) param.convertFrom0to1 (param.getValueForText (text)); };
    slider.textFromValueFunction = [&param] (f64 value) { return param.getText (param.convertTo0to1 ((f32) value), 0); };
    slider.setDoubleClickReturnValue (true, param.convertFrom0to1 (param.getDefaultValue()));

    auto range = param.getNormalisableRange();

    auto convertFrom0To1Function = [range] (f64 currentRangeStart,
                                            f64 currentRangeEnd,
                                            f64 normalisedValue) mutable
    {
        range.start = (f32) currentRangeStart;
        range.end = (f32) currentRangeEnd;
        return (f64) range.convertFrom0to1 ((f32) normalisedValue);
    };

    auto convertTo0To1Function = [range] (f64 currentRangeStart,
                                          f64 currentRangeEnd,
                                          f64 mappedValue) mutable
    {
        range.start = (f32) currentRangeStart;
        range.end = (f32) currentRangeEnd;
        return (f64) range.convertTo0to1 ((f32) mappedValue);
    };

    auto snapToLegalValueFunction = [range] (f64 currentRangeStart,
                                             f64 currentRangeEnd,
                                             f64 mappedValue) mutable
    {
        range.start = (f32) currentRangeStart;
        range.end = (f32) currentRangeEnd;
        return (f64) range.snapToLegalValue ((f32) mappedValue);
    };

    NormalisableRange<f64> newRange { (f64) range.start,
                                         (f64) range.end,
                                         std::move (convertFrom0To1Function),
                                         std::move (convertTo0To1Function),
                                         std::move (snapToLegalValueFunction) };
    newRange.interval = range.interval;
    newRange.skew = range.skew;
    newRange.symmetricSkew = range.symmetricSkew;

    slider.setNormalisableRange (newRange);

    sendInitialUpdate();
    slider.valueChanged();
    slider.addListener (this);
}

SliderParameterAttachment::~SliderParameterAttachment()
{
    slider.removeListener (this);
}

z0 SliderParameterAttachment::sendInitialUpdate() { attachment.sendInitialUpdate(); }

z0 SliderParameterAttachment::setValue (f32 newValue)
{
    const ScopedValueSetter<b8> svs (ignoreCallbacks, true);
    slider.setValue (newValue, sendNotificationSync);
}

z0 SliderParameterAttachment::sliderValueChanged (Slider*)
{
    if (! ignoreCallbacks)
        attachment.setValueAsPartOfGesture ((f32) slider.getValue());
}

//==============================================================================
ComboBoxParameterAttachment::ComboBoxParameterAttachment (RangedAudioParameter& param,
                                                          ComboBox& c,
                                                          UndoManager* um)
    : comboBox (c),
      storedParameter (param),
      attachment (param, [this] (f32 f) { setValue (f); }, um)
{
    sendInitialUpdate();
    comboBox.addListener (this);
}

ComboBoxParameterAttachment::~ComboBoxParameterAttachment()
{
    comboBox.removeListener (this);
}

z0 ComboBoxParameterAttachment::sendInitialUpdate()
{
    attachment.sendInitialUpdate();
}

z0 ComboBoxParameterAttachment::setValue (f32 newValue)
{
    const auto normValue = storedParameter.convertTo0to1 (newValue);
    const auto index = roundToInt (normValue * (f32) (comboBox.getNumItems() - 1));

    if (index == comboBox.getSelectedItemIndex())
        return;

    const ScopedValueSetter<b8> svs (ignoreCallbacks, true);
    comboBox.setSelectedItemIndex (index, sendNotificationSync);
}

z0 ComboBoxParameterAttachment::comboBoxChanged (ComboBox*)
{
    if (ignoreCallbacks)
        return;

    const auto numItems = comboBox.getNumItems();
    const auto selected = (f32) comboBox.getSelectedItemIndex();
    const auto newValue = numItems > 1 ? selected / (f32) (numItems - 1)
                                       : 0.0f;

    attachment.setValueAsCompleteGesture (storedParameter.convertFrom0to1 (newValue));
}

//==============================================================================
ButtonParameterAttachment::ButtonParameterAttachment (RangedAudioParameter& param,
                                                      Button& b,
                                                      UndoManager* um)
    : button (b),
      attachment (param, [this] (f32 f) { setValue (f); }, um)
{
    sendInitialUpdate();
    button.addListener (this);
}

ButtonParameterAttachment::~ButtonParameterAttachment()
{
    button.removeListener (this);
}

z0 ButtonParameterAttachment::sendInitialUpdate()
{
    attachment.sendInitialUpdate();
}

z0 ButtonParameterAttachment::setValue (f32 newValue)
{
    const ScopedValueSetter<b8> svs (ignoreCallbacks, true);
    button.setToggleState (newValue >= 0.5f, sendNotificationSync);
}

z0 ButtonParameterAttachment::buttonClicked (Button*)
{
    if (ignoreCallbacks)
        return;

    attachment.setValueAsCompleteGesture (button.getToggleState() ? 1.0f : 0.0f);
}

//==============================================================================
#if DRX_WEB_BROWSER
WebSliderParameterAttachment::WebSliderParameterAttachment (RangedAudioParameter& parameterIn,
                                                            WebSliderRelay& sliderStateIn,
                                                            UndoManager* undoManager)
    : sliderState (sliderStateIn),
      parameter (parameterIn),
      attachment (parameter, [this] (f32 newValue) { setValue (newValue); }, undoManager)
{
    sendInitialUpdate();
    sliderState.addListener (this);
}

WebSliderParameterAttachment::~WebSliderParameterAttachment()
{
    sliderState.removeListener (this);
}

z0 WebSliderParameterAttachment::sendInitialUpdate()
{
    const auto range = parameter.getNormalisableRange();
    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (detail::WebSliderRelayEvents::Event::eventTypeKey, "propertiesChanged");
    object->setProperty ("start", range.start);
    object->setProperty ("end", range.end);
    object->setProperty ("skew", range.skew);
    object->setProperty ("name", parameter.getName (100));
    object->setProperty ("label", parameter.getLabel());

    // We use the NormalisableRange defined num steps even for an AudioParameterFloat.
    const auto numSteps = range.interval > 0
                        ? static_cast<i32> ((range.end - range.start) / range.interval) + 1
                        : AudioProcessor::getDefaultNumParameterSteps();

    object->setProperty ("numSteps", numSteps);
    object->setProperty ("interval", range.interval);
    object->setProperty ("parameterIndex", parameter.getParameterIndex());
    sliderState.emitEvent (object.get());
    attachment.sendInitialUpdate();
}

z0 WebSliderParameterAttachment::setValue (f32 newValue)
{
    const ScopedValueSetter<b8> svs (ignoreCallbacks, true);
    sliderState.setValue (newValue);
}

z0 WebSliderParameterAttachment::sliderValueChanged (WebSliderRelay* slider)
{
    if (ignoreCallbacks)
    {
        jassertfalse;
        return;
    }

    attachment.setValueAsPartOfGesture (slider->getValue());
}

//==============================================================================
WebToggleButtonParameterAttachment::WebToggleButtonParameterAttachment (RangedAudioParameter& parameterIn,
                                                                        WebToggleButtonRelay& button,
                                                                        UndoManager* undoManager)
    : relay (button),
      parameter (parameterIn),
      attachment (parameter, [this] (f32 f) { setValue (f); }, undoManager)
{
    sendInitialUpdate();
    relay.addListener (this);
}

WebToggleButtonParameterAttachment::~WebToggleButtonParameterAttachment()
{
    relay.removeListener (this);
}

z0 WebToggleButtonParameterAttachment::sendInitialUpdate()
{
    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (detail::WebSliderRelayEvents::Event::eventTypeKey, "propertiesChanged");
    object->setProperty ("name", parameter.getName (100));
    object->setProperty ("parameterIndex", parameter.getParameterIndex());
    relay.emitEvent (object.get());
    attachment.sendInitialUpdate();
}

z0 WebToggleButtonParameterAttachment::setValue (f32 newValue)
{
    const ScopedValueSetter<b8> svs (ignoreCallbacks, true);
    relay.setToggleState (newValue >= 0.5f);
}

z0 WebToggleButtonParameterAttachment::toggleStateChanged (b8 newValue)
{
    if (ignoreCallbacks)
    {
        jassertfalse;
        return;
    }

    attachment.setValueAsCompleteGesture (newValue ? 1.0f : 0.0f);
}

z0 WebToggleButtonParameterAttachment::initialUpdateRequested()
{
    sendInitialUpdate();
}

//==============================================================================
WebComboBoxParameterAttachment::WebComboBoxParameterAttachment (RangedAudioParameter& parameterIn,
                                                                WebComboBoxRelay& combo,
                                                                UndoManager* undoManager)
    : relay (combo),
      parameter (parameterIn),
      attachment (parameter, [this] (f32 f) { setValue (f); }, undoManager)
{
    sendInitialUpdate();
    relay.addListener (this);
}

WebComboBoxParameterAttachment::~WebComboBoxParameterAttachment()
{
    relay.removeListener (this);
}

z0 WebComboBoxParameterAttachment::sendInitialUpdate()
{
    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (detail::WebSliderRelayEvents::Event::eventTypeKey, "propertiesChanged");
    object->setProperty ("name", parameter.getName (100));
    object->setProperty ("parameterIndex", parameter.getParameterIndex());

    if (auto* choiceParameter = dynamic_cast<AudioParameterChoice*> (&parameter))
        object->setProperty ("choices", choiceParameter->choices);
    else
        object->setProperty ("choices", StringArray{});

    relay.emitEvent (object.get());
    attachment.sendInitialUpdate();
}

z0 WebComboBoxParameterAttachment::setValue (f32 newValue)
{
    const auto normValue = parameter.convertTo0to1 (newValue);

    const ScopedValueSetter<b8> svs (ignoreCallbacks, true);
    relay.setValue (normValue);
}

z0 WebComboBoxParameterAttachment::valueChanged (f32 newValue)
{
    if (ignoreCallbacks)
    {
        jassertfalse;
        return;
    }

    attachment.setValueAsCompleteGesture (parameter.convertFrom0to1 (newValue));
}

z0 WebComboBoxParameterAttachment::initialUpdateRequested()
{
    sendInitialUpdate();
}
#endif

} // namespace drx
