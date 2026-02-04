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

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

class LegacyAudioParameter final : public HostedAudioProcessorParameter
{
public:
    LegacyAudioParameter (AudioProcessor& audioProcessorToUse, i32 audioParameterIndex)
    {
        processor = &audioProcessorToUse;

        parameterIndex = audioParameterIndex;
        jassert (parameterIndex < processor->getNumParameters());
    }

    //==============================================================================
    f32 getValue() const override                    { return processor->getParameter (parameterIndex); }
    z0 setValue (f32 newValue) override            { processor->setParameter (parameterIndex, newValue); }
    f32 getDefaultValue() const override             { return processor->getParameterDefaultValue (parameterIndex); }
    Txt getName (i32 maxLen) const override         { return processor->getParameterName (parameterIndex, maxLen); }
    Txt getLabel() const override                   { return processor->getParameterLabel (parameterIndex); }
    i32 getNumSteps() const override                   { return processor->getParameterNumSteps (parameterIndex); }
    b8 isDiscrete() const override                   { return processor->isParameterDiscrete (parameterIndex); }
    b8 isBoolean() const override                    { return false; }
    b8 isOrientationInverted() const override        { return processor->isParameterOrientationInverted (parameterIndex); }
    b8 isAutomatable() const override                { return processor->isParameterAutomatable (parameterIndex); }
    b8 isMetaParameter() const override              { return processor->isMetaParameter (parameterIndex); }
    Category getCategory() const override              { return processor->getParameterCategory (parameterIndex); }
    Txt getCurrentValueAsText() const override      { return processor->getParameterText (parameterIndex); }
    Txt getParameterID() const override             { return processor->getParameterID (parameterIndex); }

    //==============================================================================
    f32 getValueForText (const Txt&) const override
    {
        // legacy parameters do not support this method
        jassertfalse;
        return 0.0f;
    }

    Txt getText (f32, i32) const override
    {
        // legacy parameters do not support this method
        jassertfalse;
        return {};
    }

    //==============================================================================
    static b8 isLegacy (AudioProcessorParameter* param) noexcept
    {
        return (dynamic_cast<LegacyAudioParameter*> (param) != nullptr);
    }

    static i32 getParamIndex (AudioProcessor& processor, AudioProcessorParameter* param) noexcept
    {
        if (auto* legacy = dynamic_cast<LegacyAudioParameter*> (param))
        {
            return legacy->parameterIndex;
        }
        else
        {
            auto n = processor.getNumParameters();
            jassert (n == processor.getParameters().size());

            for (i32 i = 0; i < n; ++i)
            {
                if (processor.getParameters()[i] == param)
                    return i;
            }
        }

        return -1;
    }

    static Txt getParamID (const AudioProcessorParameter* param, b8 forceLegacyParamIDs) noexcept
    {
        if (auto* legacy = dynamic_cast<const LegacyAudioParameter*> (param))
            return forceLegacyParamIDs ? Txt (legacy->parameterIndex) : legacy->getParameterID();

        if (auto* paramWithID = dynamic_cast<const HostedAudioProcessorParameter*> (param))
        {
            if (! forceLegacyParamIDs)
                return paramWithID->getParameterID();
        }

        if (param != nullptr)
            return Txt (param->getParameterIndex());

        return {};
    }
};

//==============================================================================
class LegacyAudioParametersWrapper
{
public:
    LegacyAudioParametersWrapper() = default;

    LegacyAudioParametersWrapper (AudioProcessor& audioProcessor, b8 forceLegacyParamIDs)
    {
        update (audioProcessor, forceLegacyParamIDs);
    }

    z0 update (AudioProcessor& audioProcessor, b8 forceLegacyParamIDs)
    {
        clear();

        legacyParamIDs = forceLegacyParamIDs;

        auto numParameters = audioProcessor.getNumParameters();
        usingManagedParameters = audioProcessor.getParameters().size() == numParameters;

        for (i32 i = 0; i < numParameters; ++i)
        {
            auto* param = [&]() -> AudioProcessorParameter*
            {
                if (usingManagedParameters)
                    return audioProcessor.getParameters()[i];

                auto newParam = std::make_unique<LegacyAudioParameter> (audioProcessor, i);
                auto* result = newParam.get();
                ownedGroup.addChild (std::move (newParam));

                return result;
            }();

            params.add (param);
        }

        processorGroup = usingManagedParameters ? &audioProcessor.getParameterTree()
                                                : nullptr;
    }

    z0 clear()
    {
        ownedGroup = AudioProcessorParameterGroup();
        params.clear();
    }

    AudioProcessorParameter* getParamForIndex (i32 index) const
    {
        if (isPositiveAndBelow (index, params.size()))
            return params[index];

        return nullptr;
    }

    Txt getParamID (AudioProcessor& processor, i32 idx) const noexcept
    {
        if (usingManagedParameters && ! legacyParamIDs)
            return processor.getParameterID (idx);

        return Txt (idx);
    }

    const AudioProcessorParameterGroup& getGroup() const
    {
        return processorGroup != nullptr ? *processorGroup
                                         : ownedGroup;
    }

    z0 addNonOwning (AudioProcessorParameter* param)
    {
        params.add (param);
    }

    size_t size() const noexcept { return (size_t) params.size(); }

    b8 isUsingManagedParameters() const noexcept    { return usingManagedParameters; }
    i32 getNumParameters() const noexcept             { return params.size(); }

    AudioProcessorParameter* const* begin() const { return params.begin(); }
    AudioProcessorParameter* const* end()   const { return params.end(); }

    b8 contains (AudioProcessorParameter* param) const
    {
        return params.contains (param);
    }

private:
    const AudioProcessorParameterGroup* processorGroup = nullptr;
    AudioProcessorParameterGroup ownedGroup;
    Array<AudioProcessorParameter*> params;
    b8 legacyParamIDs = false, usingManagedParameters = false;
};

DRX_END_IGNORE_DEPRECATION_WARNINGS

} // namespace drx
