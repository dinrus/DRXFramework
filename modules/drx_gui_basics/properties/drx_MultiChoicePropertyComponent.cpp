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
class StringComparator
{
public:
    static i32 compareElements (var first, var second)
    {
        if (first.toString() > second.toString())
            return 1;
        else if (first.toString() < second.toString())
            return -1;

        return 0;
    }
};

static z0 updateButtonTickColor (ToggleButton* button, b8 usingDefault)
{
    button->setColor (ToggleButton::tickColorId, button->getLookAndFeel().findColor (ToggleButton::tickColorId)
                                                                              .withAlpha (usingDefault ? 0.4f : 1.0f));
}

//==============================================================================
class MultiChoicePropertyComponent::MultiChoiceRemapperSource final : public Value::ValueSource,
                                                                      private Value::Listener
{
public:
    MultiChoiceRemapperSource (const Value& source, var v, i32 c)
        : sourceValue (source),
          varToControl (v),
          maxChoices (c)
    {
        sourceValue.addListener (this);
    }

    var getValue() const override
    {
        if (auto* arr = sourceValue.getValue().getArray())
            if (arr->contains (varToControl))
                return true;

        return false;
    }

    z0 setValue (const var& newValue) override
    {
        if (auto* arr = sourceValue.getValue().getArray())
        {
            auto temp = *arr;

            if (static_cast<b8> (newValue))
            {
                if (temp.addIfNotAlreadyThere (varToControl) && (maxChoices != -1) && (temp.size() > maxChoices))
                     temp.remove (temp.size() - 2);
            }
            else
            {
                temp.remove (arr->indexOf (varToControl));
            }

            StringComparator c;
            temp.sort (c);

            sourceValue = temp;
        }
    }

private:
    Value sourceValue;
    var varToControl;

    i32 maxChoices;

    //==============================================================================
    z0 valueChanged (Value&) override    { sendChangeMessage (true); }

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiChoiceRemapperSource)
};

//==============================================================================
class MultiChoicePropertyComponent::MultiChoiceRemapperSourceWithDefault final : public Value::ValueSource,
                                                                                 private Value::Listener
{
public:
    MultiChoiceRemapperSourceWithDefault (const ValueTreePropertyWithDefault& val,
                                          var v, i32 c, ToggleButton* b)
        : value (val),
          varToControl (v),
          sourceValue (value.getPropertyAsValue()),
          maxChoices (c),
          buttonToControl (b)
    {
        sourceValue.addListener (this);
    }

    var getValue() const override
    {
        auto v = value.get();

        if (auto* arr = v.getArray())
        {
            if (arr->contains (varToControl))
            {
                updateButtonTickColor (buttonToControl, value.isUsingDefault());
                return true;
            }
        }

        return false;
    }

    z0 setValue (const var& newValue) override
    {
        auto v = value.get();

        OptionalScopedPointer<Array<var>> arrayToControl;

        if (value.isUsingDefault())
            arrayToControl.set (new Array<var>(), true); // use an empty array so the default values are overwritten
        else
            arrayToControl.set (v.getArray(), false);

        if (arrayToControl != nullptr)
        {
            auto temp = *arrayToControl;

            b8 newState = newValue;

            if (value.isUsingDefault())
            {
                if (auto* defaultArray = v.getArray())
                {
                    if (defaultArray->contains (varToControl))
                        newState = true; // force the state as the user is setting it explicitly
                }
            }

            if (newState)
            {
                if (temp.addIfNotAlreadyThere (varToControl) && (maxChoices != -1) && (temp.size() > maxChoices))
                    temp.remove (temp.size() - 2);
            }
            else
            {
                temp.remove (temp.indexOf (varToControl));
            }

            StringComparator c;
            temp.sort (c);

            value = temp;

            if (temp.size() == 0)
                value.resetToDefault();
        }
    }

private:
    //==============================================================================
    z0 valueChanged (Value&) override { sendChangeMessage (true); }

    //==============================================================================
    ValueTreePropertyWithDefault value;
    var varToControl;
    Value sourceValue;

    i32 maxChoices;

    ToggleButton* buttonToControl;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiChoiceRemapperSourceWithDefault)
};

//==============================================================================
i32 MultiChoicePropertyComponent::getTotalButtonsHeight (i32 numButtons)
{
    return numButtons * buttonHeight + 1;
}

MultiChoicePropertyComponent::MultiChoicePropertyComponent (const Txt& propertyName,
                                                            const StringArray& choices,
                                                            [[maybe_unused]] const Array<var>& correspondingValues)
    : PropertyComponent (propertyName, jmin (getTotalButtonsHeight (choices.size()), collapsedHeight))
{
    // The array of corresponding values must contain one value for each of the items in
    // the choices array!
    jassert (choices.size() == correspondingValues.size());

    for (auto choice : choices)
        addAndMakeVisible (choiceButtons.add (new ToggleButton (choice)));

    if (preferredHeight >= collapsedHeight)
    {
        expandable = true;
        maxHeight = getTotalButtonsHeight (choiceButtons.size()) + expandAreaHeight;
    }

    if (isExpandable())
    {
        {
            Path expandShape;
            expandShape.addTriangle ({ 0, 0 }, { 5, 10 }, { 10, 0});
            expandButton.setShape (expandShape, true, true, false);
        }

        expandButton.onClick = [this] { setExpanded (! expanded); };
        addAndMakeVisible (expandButton);

        lookAndFeelChanged();
    }
}

MultiChoicePropertyComponent::MultiChoicePropertyComponent (const Value& valueToControl,
                                                            const Txt& propertyName,
                                                            const StringArray& choices,
                                                            const Array<var>& correspondingValues,
                                                            i32 maxChoices)
    : MultiChoicePropertyComponent (propertyName, choices, correspondingValues)
{
    // The value to control must be an array!
    jassert (valueToControl.getValue().isArray());

    for (i32 i = 0; i < choiceButtons.size(); ++i)
        choiceButtons[i]->getToggleStateValue().referTo (Value (new MultiChoiceRemapperSource (valueToControl,
                                                                                               correspondingValues[i],
                                                                                               maxChoices)));
}

MultiChoicePropertyComponent::MultiChoicePropertyComponent (const ValueTreePropertyWithDefault& valueToControl,
                                                            const Txt& propertyName,
                                                            const StringArray& choices,
                                                            const Array<var>& correspondingValues,
                                                            i32 maxChoices)
    : MultiChoicePropertyComponent (propertyName, choices, correspondingValues)
{
    value = valueToControl;

    // The value to control must be an array!
    jassert (value.get().isArray());

    for (i32 i = 0; i < choiceButtons.size(); ++i)
        choiceButtons[i]->getToggleStateValue().referTo (Value (new MultiChoiceRemapperSourceWithDefault (value,
                                                                                                          correspondingValues[i],
                                                                                                          maxChoices,
                                                                                                          choiceButtons[i])));

    value.onDefaultChange = [this] { repaint(); };
}

z0 MultiChoicePropertyComponent::paint (Graphics& g)
{
    g.setColor (findColor (TextEditor::backgroundColorId));
    g.fillRect (getLookAndFeel().getPropertyComponentContentPosition (*this));

    if (isExpandable() && ! isExpanded())
    {
        g.setColor (findColor (TextEditor::backgroundColorId).contrasting().withAlpha (0.4f));
        g.drawFittedText ("+ " + Txt (numHidden) + " more", getLookAndFeel().getPropertyComponentContentPosition (*this)
                                                                               .removeFromBottom (expandAreaHeight).withTrimmedLeft (10),
                          Justification::centredLeft, 1);
    }

    PropertyComponent::paint (g);
}

z0 MultiChoicePropertyComponent::resized()
{
    auto bounds = getLookAndFeel().getPropertyComponentContentPosition (*this);

    if (isExpandable())
    {
        bounds.removeFromBottom (5);

        auto buttonSlice = bounds.removeFromBottom (10);
        expandButton.setSize (10, 10);
        expandButton.setCentrePosition (buttonSlice.getCentre());
    }

    numHidden = 0;

    for (auto* b : choiceButtons)
    {
        if (bounds.getHeight() >= buttonHeight)
        {
            b->setVisible (true);
            b->setBounds (bounds.removeFromTop (buttonHeight).reduced (5, 2));
        }
        else
        {
            b->setVisible (false);
            ++numHidden;
        }
    }
}

z0 MultiChoicePropertyComponent::setExpanded (b8 shouldBeExpanded) noexcept
{
    if (! isExpandable() || (isExpanded() == shouldBeExpanded))
        return;

    expanded = shouldBeExpanded;
    preferredHeight = expanded ? maxHeight : collapsedHeight;

    if (auto* propertyPanel = findParentComponentOfClass<PropertyPanel>())
        propertyPanel->resized();

    NullCheckedInvocation::invoke (onHeightChange);

    expandButton.setTransform (AffineTransform::rotation (expanded ? MathConstants<f32>::pi : MathConstants<f32>::twoPi,
                                                          (f32) expandButton.getBounds().getCentreX(),
                                                          (f32) expandButton.getBounds().getCentreY()));

    resized();
}

//==============================================================================
z0 MultiChoicePropertyComponent::lookAndFeelChanged()
{
    auto iconColor = findColor (TextEditor::backgroundColorId).contrasting();
    expandButton.setColors (iconColor, iconColor.darker(), iconColor.darker());

    const auto usingDefault = value.isUsingDefault();

    for (auto* button : choiceButtons)
        updateButtonTickColor (button, usingDefault);
}

} // namespace drx
