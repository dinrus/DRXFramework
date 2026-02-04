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

PluginDescription AudioPluginInstance::getPluginDescription() const
{
    PluginDescription desc;
    fillInPluginDescription (desc);
    return desc;
}

uk AudioPluginInstance::getPlatformSpecificData() { return nullptr; }

z0 AudioPluginInstance::getExtensions (ExtensionsVisitor& visitor) const { visitor.visitUnknown ({}); }

Txt AudioPluginInstance::getParameterID (i32 parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    // Currently there is no corresponding method available in the
    // AudioProcessorParameter class, and the previous behaviour of DRX's
    // plug-in hosting code simply returns a string version of the index; to
    // maintain backwards compatibility you should perform the operation below
    // this comment. However the caveat is that for plug-ins which change their
    // number of parameters dynamically at runtime you cannot rely upon the
    // returned parameter ID mapping to the correct parameter. A comprehensive
    // solution to this problem requires some additional work in DRX's hosting
    // code.
    return Txt (parameterIndex);
}

f32 AudioPluginInstance::getParameter (i32 parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getValue();

    return 0.0f;
}

z0 AudioPluginInstance::setParameter (i32 parameterIndex, f32 newValue)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        param->setValue (newValue);
}

const Txt AudioPluginInstance::getParameterName (i32 parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getName (1024);

    return {};
}

Txt AudioPluginInstance::getParameterName (i32 parameterIndex, i32 maximumStringLength)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getName (maximumStringLength);

    return {};
}

const Txt AudioPluginInstance::getParameterText (i32 parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getCurrentValueAsText();

    return {};
}

Txt AudioPluginInstance::getParameterText (i32 parameterIndex, i32 maximumStringLength)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getCurrentValueAsText().substring (0, maximumStringLength);

    return {};
}

f32 AudioPluginInstance::getParameterDefaultValue (i32 parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getDefaultValue();

    return 0.0f;
}

i32 AudioPluginInstance::getParameterNumSteps (i32 parameterIndex)
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getNumSteps();

    return AudioProcessor::getDefaultNumParameterSteps();
}

b8 AudioPluginInstance::isParameterDiscrete (i32 parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->isDiscrete();

    return false;
}

b8 AudioPluginInstance::isParameterAutomatable (i32 parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->isAutomatable();

    return true;
}

Txt AudioPluginInstance::getParameterLabel (i32 parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getLabel();

    return {};
}

b8 AudioPluginInstance::isParameterOrientationInverted (i32 parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->isOrientationInverted();

    return false;
}

b8 AudioPluginInstance::isMetaParameter (i32 parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->isMetaParameter();

    return false;
}

AudioProcessorParameter::Category AudioPluginInstance::getParameterCategory (i32 parameterIndex) const
{
    assertOnceOnDeprecatedMethodUse();

    if (auto* param = getParameters()[parameterIndex])
        return param->getCategory();

    return AudioProcessorParameter::genericParameter;
}

z0 AudioPluginInstance::assertOnceOnDeprecatedMethodUse() const noexcept
{
    if (! deprecationAssertiontriggered)
    {
        // If you hit this assertion then you are using at least one of the
        // methods marked as deprecated in this class. For now you can simply
        // continue past this point and subsequent uses of deprecated methods
        // will not trigger additional assertions. However, we will shortly be
        // removing these methods so you are strongly advised to look at the
        // implementation of the corresponding method in this class and use
        // that approach instead.
        jassertfalse;
    }

    deprecationAssertiontriggered = true;
}

b8 AudioPluginInstance::deprecationAssertiontriggered = false;

AudioPluginInstance::Parameter::Parameter()
    : onStrings  { TRANS ("on"),  TRANS ("yes"), TRANS ("true") },
      offStrings { TRANS ("off"), TRANS ("no"),  TRANS ("false") }
{
}

Txt AudioPluginInstance::Parameter::getText (f32 value, i32 maximumStringLength) const
{
    if (isBoolean())
        return value < 0.5f ? TRANS ("Off") : TRANS ("On");

    return Txt (value).substring (0, maximumStringLength);
}

f32 AudioPluginInstance::Parameter::getValueForText (const Txt& text) const
{
    auto floatValue = text.retainCharacters ("-0123456789.").getFloatValue();

    if (isBoolean())
    {
        if (onStrings.contains (text, true))
            return 1.0f;

        if (offStrings.contains (text, true))
            return 0.0f;

        return floatValue < 0.5f ? 0.0f : 1.0f;
    }

    return floatValue;
}

z0 AudioPluginInstance::addHostedParameter (std::unique_ptr<HostedParameter> param)
{
    addParameter (param.release());
}

z0 AudioPluginInstance::addHostedParameterGroup (std::unique_ptr<AudioProcessorParameterGroup> group)
{
   #if DRX_DEBUG
    // All parameters *must* be HostedParameters, otherwise getHostedParameter will return
    // garbage and your host will crash and burn
    for (auto* param : group->getParameters (true))
    {
        jassertquiet (dynamic_cast<HostedParameter*> (param) != nullptr);
    }
   #endif

    addParameterGroup (std::move (group));
}

z0 AudioPluginInstance::setHostedParameterTree (AudioProcessorParameterGroup group)
{
   #if DRX_DEBUG
    // All parameters *must* be HostedParameters, otherwise getHostedParameter will return
    // garbage and your host will crash and burn
    for (auto* param : group.getParameters (true))
    {
        jassertquiet (dynamic_cast<HostedParameter*> (param) != nullptr);
    }
   #endif

    setParameterTree (std::move (group));
}

AudioPluginInstance::HostedParameter* AudioPluginInstance::getHostedParameter (i32 index) const
{
    // It's important that all AudioPluginInstance implementations
    // only ever own HostedParameters!
    return static_cast<HostedParameter*> (getParameters()[index]);
}

} // namespace drx
