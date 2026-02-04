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

#pragma once


//==============================================================================
class TextPropertyComponentWithEnablement final : public TextPropertyComponent,
                                                  private Value::Listener
{
public:
    TextPropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                         ValueTreePropertyWithDefault valueToListenTo,
                                         const Txt& propertyName,
                                         i32 maxNumChars,
                                         b8 multiLine)
        : TextPropertyComponent (valueToControl, propertyName, maxNumChars, multiLine),
          propertyWithDefault (valueToListenTo),
          value (propertyWithDefault.getPropertyAsValue())
    {
        value.addListener (this);
        setEnabled (propertyWithDefault.get());
    }

    ~TextPropertyComponentWithEnablement() override  { value.removeListener (this); }

private:
    ValueTreePropertyWithDefault propertyWithDefault;
    Value value;

    z0 valueChanged (Value&) override  { setEnabled (propertyWithDefault.get()); }
};

//==============================================================================
class ChoicePropertyComponentWithEnablement final : public ChoicePropertyComponent,
                                                    private Value::Listener
{
public:
    ChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                           ValueTreePropertyWithDefault valueToListenTo,
                                           const Txt& propertyName,
                                           const StringArray& choiceToUse,
                                           const Array<var>& correspondingValues)
        : ChoicePropertyComponent (valueToControl, propertyName, choiceToUse, correspondingValues),
          propertyWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        handleValueChanged();
    }

    ChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                           ValueTreePropertyWithDefault valueToListenTo,
                                           const Identifier& multiChoiceID,
                                           const Txt& propertyName,
                                           const StringArray& choicesToUse,
                                           const Array<var>& correspondingValues)
        : ChoicePropertyComponentWithEnablement (valueToControl, valueToListenTo, propertyName, choicesToUse, correspondingValues)
    {
        jassert (valueToListenTo.get().getArray() != nullptr);

        isMultiChoice = true;
        idToCheck = multiChoiceID;

        handleValueChanged();
    }

    ChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                           ValueTreePropertyWithDefault valueToListenTo,
                                           const Txt& propertyName)
        : ChoicePropertyComponent (valueToControl, propertyName),
          propertyWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        handleValueChanged();
    }

    ~ChoicePropertyComponentWithEnablement() override    { value.removeListener (this); }

private:
    ValueTreePropertyWithDefault propertyWithDefault;
    Value value;

    b8 isMultiChoice = false;
    Identifier idToCheck;

    b8 checkMultiChoiceVar() const
    {
        jassert (isMultiChoice);

        auto v = propertyWithDefault.get();

        if (auto* varArray = v.getArray())
            return varArray->contains (idToCheck.toString());

        jassertfalse;
        return false;
    }

    z0 handleValueChanged()
    {
        if (isMultiChoice)
            setEnabled (checkMultiChoiceVar());
        else
            setEnabled (propertyWithDefault.get());
    }

    z0 valueChanged (Value&) override
    {
        handleValueChanged();
    }
};

//==============================================================================
class MultiChoicePropertyComponentWithEnablement final : public MultiChoicePropertyComponent,
                                                         private Value::Listener
{
public:
    MultiChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                                ValueTreePropertyWithDefault valueToListenTo,
                                                const Txt& propertyName,
                                                const StringArray& choices,
                                                const Array<var>& correspondingValues)
        : MultiChoicePropertyComponent (valueToControl,
                                        propertyName,
                                        choices,
                                        correspondingValues),
          propertyWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        valueChanged (value);
    }

    ~MultiChoicePropertyComponentWithEnablement() override    { value.removeListener (this); }

private:
    z0 valueChanged (Value&) override       { setEnabled (propertyWithDefault.get()); }

    ValueTreePropertyWithDefault propertyWithDefault;
    Value value;
};
