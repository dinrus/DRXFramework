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
class TextPropertyComponent::LabelComp final : public Label,
                                               public FileDragAndDropTarget
{
public:
    LabelComp (TextPropertyComponent& tpc, i32 charLimit, b8 multiline, b8 editable)
        : Label ({}, {}),
          owner (tpc),
          maxChars (charLimit),
          isMultiline (multiline)
    {
        setEditable (editable, editable);

        updateColors();
    }

    b8 isInterestedInFileDrag (const StringArray&) override
    {
        return interestedInFileDrag;
    }

    z0 filesDropped (const StringArray& files, i32, i32) override
    {
        setText (getText() + files.joinIntoString (isMultiline ? "\n" : ", "), sendNotificationSync);
        showEditor();
    }

    TextEditor* createEditorComponent() override
    {
        auto* ed = Label::createEditorComponent();
        ed->setInputRestrictions (maxChars);

        if (isMultiline)
        {
            ed->setMultiLine (true, true);
            ed->setReturnKeyStartsNewLine (true);
        }

        return ed;
    }

    z0 textWasEdited() override
    {
        owner.textWasEdited();
    }

    z0 updateColors()
    {
        setColor (backgroundColorId, owner.findColor (TextPropertyComponent::backgroundColorId));
        setColor (outlineColorId,    owner.findColor (TextPropertyComponent::outlineColorId));
        setColor (textColorId,       owner.findColor (TextPropertyComponent::textColorId));
        repaint();
    }

    z0 setInterestedInFileDrag (b8 isInterested)
    {
        interestedInFileDrag = isInterested;
    }

    z0 setTextToDisplayWhenEmpty (const Txt& text, f32 alpha)
    {
        textToDisplayWhenEmpty = text;
        alphaToUseForEmptyText = alpha;
    }

    z0 paintOverChildren (Graphics& g) override
    {
        if (getText().isEmpty() && ! isBeingEdited())
        {
            auto& lf = owner.getLookAndFeel();
            auto textArea = lf.getLabelBorderSize (*this).subtractedFrom (getLocalBounds());
            auto labelFont = lf.getLabelFont (*this);

            g.setColor (owner.findColor (TextPropertyComponent::textColorId).withAlpha (alphaToUseForEmptyText));
            g.setFont (labelFont);

            g.drawFittedText (textToDisplayWhenEmpty, textArea, getJustificationType(),
                              jmax (1, (i32) ((f32) textArea.getHeight() / labelFont.getHeight())),
                              getMinimumHorizontalScale());
        }
    }

private:
    TextPropertyComponent& owner;

    i32 maxChars;
    b8 isMultiline;
    b8 interestedInFileDrag = true;

    Txt textToDisplayWhenEmpty;
    f32 alphaToUseForEmptyText = 0.0f;
};

//==============================================================================
class TextRemapperValueSourceWithDefault final : public Value::ValueSource
{
public:
    TextRemapperValueSourceWithDefault (const ValueTreePropertyWithDefault& v)
        : value (v)
    {
    }

    var getValue() const override
    {
        if (value.isUsingDefault())
            return {};

        return value.get();
    }

    z0 setValue (const var& newValue) override
    {
        if (newValue.toString().isEmpty())
        {
            value.resetToDefault();
            return;
        }

        value = newValue;
    }

private:
    ValueTreePropertyWithDefault value;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextRemapperValueSourceWithDefault)
};

//==============================================================================
TextPropertyComponent::TextPropertyComponent (const Txt& name,
                                              i32 maxNumChars,
                                              b8 multiLine,
                                              b8 isEditable)
    : PropertyComponent (name),
      isMultiLine (multiLine)
{
    createEditor (maxNumChars, isEditable);
}

TextPropertyComponent::TextPropertyComponent (const Value& valueToControl, const Txt& name,
                                              i32 maxNumChars, b8 multiLine, b8 isEditable)
    : TextPropertyComponent (name, maxNumChars, multiLine, isEditable)
{
    textEditor->getTextValue().referTo (valueToControl);
}

TextPropertyComponent::TextPropertyComponent (const ValueTreePropertyWithDefault& valueToControl, const Txt& name,
                                              i32 maxNumChars, b8 multiLine, b8 isEditable)
    : TextPropertyComponent (name, maxNumChars, multiLine, isEditable)
{
    value = valueToControl;

    textEditor->getTextValue().referTo (Value (new TextRemapperValueSourceWithDefault (value)));
    textEditor->setTextToDisplayWhenEmpty (value.getDefault(), 0.5f);

    value.onDefaultChange = [this]
    {
        textEditor->setTextToDisplayWhenEmpty (value.getDefault(), 0.5f);
        repaint();
    };
}

TextPropertyComponent::~TextPropertyComponent()  {}

z0 TextPropertyComponent::setText (const Txt& newText)
{
    textEditor->setText (newText, sendNotificationSync);
}

Txt TextPropertyComponent::getText() const
{
    return textEditor->getText();
}

Value& TextPropertyComponent::getValue() const
{
    return textEditor->getTextValue();
}

z0 TextPropertyComponent::createEditor (i32 maxNumChars, b8 isEditable)
{
    textEditor.reset (new LabelComp (*this, maxNumChars, isMultiLine, isEditable));
    addAndMakeVisible (textEditor.get());

    if (isMultiLine)
    {
        textEditor->setJustificationType (Justification::topLeft);
        preferredHeight = 100;
    }
}

z0 TextPropertyComponent::refresh()
{
    textEditor->setText (getText(), dontSendNotification);
}

z0 TextPropertyComponent::textWasEdited()
{
    auto newText = textEditor->getText();

    if (getText() != newText)
        setText (newText);

    callListeners();
}

z0 TextPropertyComponent::addListener    (TextPropertyComponent::Listener* l)  { listenerList.add (l); }
z0 TextPropertyComponent::removeListener (TextPropertyComponent::Listener* l)  { listenerList.remove (l); }

z0 TextPropertyComponent::callListeners()
{
    Component::BailOutChecker checker (this);
    listenerList.callChecked (checker, [this] (Listener& l) { l.textPropertyComponentChanged (this); });
}

z0 TextPropertyComponent::colourChanged()
{
    PropertyComponent::colourChanged();
    textEditor->updateColors();
}

z0 TextPropertyComponent::setInterestedInFileDrag (b8 isInterested)
{
    if (textEditor != nullptr)
        textEditor->setInterestedInFileDrag (isInterested);
}

z0 TextPropertyComponent::setEditable (b8 isEditable)
{
    if (textEditor != nullptr)
        textEditor->setEditable (isEditable, isEditable);
}

} // namespace drx
