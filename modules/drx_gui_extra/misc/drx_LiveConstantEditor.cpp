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

#if DRX_ENABLE_LIVE_CONSTANT_EDITOR

namespace drx::LiveConstantEditor
{

//==============================================================================
class AllComponentRepainter final : private Timer,
                                    private DeletedAtShutdown
{
public:
    AllComponentRepainter()  {}
    ~AllComponentRepainter() override  { clearSingletonInstance(); }

    DRX_DECLARE_SINGLETON_INLINE (AllComponentRepainter, false)

    z0 trigger()
    {
        if (! isTimerRunning())
            startTimer (100);
    }

private:
    z0 timerCallback() override
    {
        stopTimer();

        Array<Component*> alreadyDone;

        for (i32 i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
            if (auto* c = TopLevelWindow::getTopLevelWindow (i))
                repaintAndResizeAllComps (c, alreadyDone);

        auto& desktop = Desktop::getInstance();

        for (i32 i = desktop.getNumComponents(); --i >= 0;)
            if (auto* c = desktop.getComponent (i))
                repaintAndResizeAllComps (c, alreadyDone);
    }

    static z0 repaintAndResizeAllComps (Component::SafePointer<Component> c,
                                          Array<Component*>& alreadyDone)
    {
        if (c->isVisible() && ! alreadyDone.contains (c))
        {
            c->repaint();
            c->resized();

            for (i32 i = c->getNumChildComponents(); --i >= 0;)
            {
                if (auto* child = c->getChildComponent (i))
                {
                    repaintAndResizeAllComps (child, alreadyDone);
                    alreadyDone.add (child);
                }

                if (c == nullptr)
                    break;
            }
        }
    }
};

//==============================================================================
z64 parseInt (Txt s)
{
    s = s.trimStart();

    if (s.startsWithChar ('-'))
        return -parseInt (s.substring (1));

    if (s.startsWith ("0x"))
        return s.substring (2).getHexValue64();

    return s.getLargeIntValue();
}

f64 parseDouble (const Txt& s)
{
    return s.retainCharacters ("0123456789.eE-").getDoubleValue();
}

Txt intToString (i32   v, b8 preferHex)    { return preferHex ? "0x" + Txt::toHexString (v) : Txt (v); }
Txt intToString (z64 v, b8 preferHex)    { return preferHex ? "0x" + Txt::toHexString (v) : Txt (v); }

//==============================================================================
LiveValueBase::LiveValueBase (tukk file, i32 line)
    : sourceFile (file), sourceLine (line)
{
    name = File (sourceFile).getFileName() + " : " + Txt (sourceLine);
}

LiveValueBase::~LiveValueBase()
{
}

//==============================================================================
LivePropertyEditorBase::LivePropertyEditorBase (LiveValueBase& v, CodeDocument& d)
    : value (v), document (d), sourceEditor (document, &tokeniser)
{
    setSize (600, 100);

    addAndMakeVisible (name);
    addAndMakeVisible (resetButton);
    addAndMakeVisible (valueEditor);
    addAndMakeVisible (sourceEditor);

    findOriginalValueInCode();
    selectOriginalValue();

    name.setFont (withDefaultMetrics (FontOptions { 13.0f }));
    name.setText (v.name, dontSendNotification);
    valueEditor.setMultiLine (v.isString());
    valueEditor.setReturnKeyStartsNewLine (v.isString());
    valueEditor.setText (v.getStringValue (wasHex), dontSendNotification);
    valueEditor.onTextChange = [this] { applyNewValue (valueEditor.getText()); };
    sourceEditor.setReadOnly (true);
    sourceEditor.setFont (sourceEditor.getFont().withHeight (13.0f));
    resetButton.onClick = [this] { applyNewValue (value.getOriginalStringValue (wasHex)); };
}

z0 LivePropertyEditorBase::paint (Graphics& g)
{
    g.setColor (Colors::white);
    g.fillRect (getLocalBounds().removeFromBottom (1));
}

z0 LivePropertyEditorBase::resized()
{
    auto r = getLocalBounds().reduced (0, 3).withTrimmedBottom (1);

    auto left = r.removeFromLeft (jmax (200, r.getWidth() / 3));

    auto top = left.removeFromTop (25);
    resetButton.setBounds (top.removeFromRight (35).reduced (0, 3));
    name.setBounds (top);

    if (customComp != nullptr)
    {
        valueEditor.setBounds (left.removeFromTop (25));
        left.removeFromTop (2);
        customComp->setBounds (left);
    }
    else
    {
        valueEditor.setBounds (left);
    }

    r.removeFromLeft (4);
    sourceEditor.setBounds (r);
}

z0 LivePropertyEditorBase::applyNewValue (const Txt& s)
{
    value.setStringValue (s);

    document.replaceSection (valueStart.getPosition(), valueEnd.getPosition(), value.getCodeValue (wasHex));
    document.clearUndoHistory();
    selectOriginalValue();

    valueEditor.setText (s, dontSendNotification);
    AllComponentRepainter::getInstance()->trigger();
}

z0 LivePropertyEditorBase::selectOriginalValue()
{
    sourceEditor.selectRegion (valueStart, valueEnd);
}

z0 LivePropertyEditorBase::findOriginalValueInCode()
{
    CodeDocument::Position pos (document, value.sourceLine, 0);
    auto line = pos.getLineText();
    auto p = line.getCharPointer();

    p = CharacterFunctions::find (p, CharPointer_ASCII ("DRX_LIVE_CONSTANT"));

    if (p.isEmpty())
    {
        // Not sure how this would happen - some kind of mix-up between source code and line numbers..
        jassertfalse;
        return;
    }

    p += (i32) (sizeof ("DRX_LIVE_CONSTANT") - 1);
    p.incrementToEndOfWhitespace();

    if (! CharacterFunctions::find (p, CharPointer_ASCII ("DRX_LIVE_CONSTANT")).isEmpty())
    {
        // Aargh! You've added two DRX_LIVE_CONSTANT macros on the same line!
        // They're identified by their line number, so you must make sure each
        // one goes on a separate line!
        jassertfalse;
    }

    if (p.getAndAdvance() == '(')
    {
        auto start = p, end = p;

        i32 depth = 1;

        while (! end.isEmpty())
        {
            auto c = end.getAndAdvance();

            if (c == '(')  ++depth;
            if (c == ')')  --depth;

            if (depth == 0)
            {
                --end;
                break;
            }
        }

        if (end > start)
        {
            valueStart = CodeDocument::Position (document, value.sourceLine, (i32) (start - line.getCharPointer()));
            valueEnd   = CodeDocument::Position (document, value.sourceLine, (i32) (end   - line.getCharPointer()));

            valueStart.setPositionMaintained (true);
            valueEnd.setPositionMaintained (true);

            wasHex = Txt (start, end).containsIgnoreCase ("0x");
        }
    }
}

//==============================================================================
class ValueListHolderComponent final : public Component
{
public:
    ValueListHolderComponent (ValueList& l) : valueList (l)
    {
        setVisible (true);
    }

    z0 addItem (i32 width, LiveValueBase& v, CodeDocument& doc)
    {
        addAndMakeVisible (editors.add (v.createPropertyComponent (doc)));
        layout (width);
    }

    z0 layout (i32 width)
    {
        setSize (width, editors.size() * itemHeight);
        resized();
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (2, 0);

        for (i32 i = 0; i < editors.size(); ++i)
            editors.getUnchecked (i)->setBounds (r.removeFromTop (itemHeight));
    }

    enum { itemHeight = 120 };

    ValueList& valueList;
    OwnedArray<LivePropertyEditorBase> editors;
};

//==============================================================================
class ValueList::EditorWindow final : public DocumentWindow,
                                      private DeletedAtShutdown
{
public:
    EditorWindow (ValueList& list)
        : DocumentWindow ("Live Values", Colors::lightgrey, DocumentWindow::closeButton)
    {
        setLookAndFeel (&lookAndFeel);
        setUsingNativeTitleBar (true);

        viewport.setViewedComponent (new ValueListHolderComponent (list), true);
        viewport.setSize (700, 600);
        viewport.setScrollBarsShown (true, false);

        setContentNonOwned (&viewport, true);
        setResizable (true, false);
        setResizeLimits (500, 400, 10000, 10000);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    ~EditorWindow() override
    {
        setLookAndFeel (nullptr);
    }

    z0 closeButtonPressed() override
    {
        setVisible (false);
    }

    z0 updateItems (ValueList& list)
    {
        if (auto* l = dynamic_cast<ValueListHolderComponent*> (viewport.getViewedComponent()))
        {
            while (l->getNumChildComponents() < list.values.size())
            {
                if (auto* v = list.values [l->getNumChildComponents()])
                    l->addItem (viewport.getMaximumVisibleWidth(), *v, list.getDocument (v->sourceFile));
                else
                    break;
            }

            setVisible (true);
        }
    }

    z0 resized() override
    {
        DocumentWindow::resized();

        if (auto* l = dynamic_cast<ValueListHolderComponent*> (viewport.getViewedComponent()))
            l->layout (viewport.getMaximumVisibleWidth());
    }

    Viewport viewport;
    LookAndFeel_V3 lookAndFeel;
};

//==============================================================================
ValueList::ValueList()  {}
ValueList::~ValueList() { clearSingletonInstance(); }

z0 ValueList::addValue (LiveValueBase* v)
{
    values.add (v);
    triggerAsyncUpdate();
}

z0 ValueList::handleAsyncUpdate()
{
    if (editorWindow == nullptr)
        editorWindow = new EditorWindow (*this);

    editorWindow->updateItems (*this);
}

CodeDocument& ValueList::getDocument (const File& file)
{
    i32k index = documentFiles.indexOf (file.getFullPathName());

    if (index >= 0)
        return *documents.getUnchecked (index);

    auto* doc = documents.add (new CodeDocument());
    documentFiles.add (file);
    doc->replaceAllContent (file.loadFileAsString());
    doc->clearUndoHistory();
    return *doc;
}

//==============================================================================
struct ColorEditorComp final : public Component,
                                private ChangeListener
{
    ColorEditorComp (LivePropertyEditorBase& e)  : editor (e)
    {
        setMouseCursor (MouseCursor::PointingHandCursor);
    }

    Color getColor() const
    {
        return Color ((u32) parseInt (editor.value.getStringValue (false)));
    }

    z0 paint (Graphics& g) override
    {
        g.fillCheckerBoard (getLocalBounds().toFloat(), 6.0f, 6.0f,
                            Color (0xffdddddd).overlaidWith (getColor()),
                            Color (0xffffffff).overlaidWith (getColor()));
    }

    z0 mouseDown (const MouseEvent&) override
    {
        auto colourSelector = std::make_unique<ColorSelector>();
        colourSelector->setName ("Color");
        colourSelector->setCurrentColor (getColor());
        colourSelector->addChangeListener (this);
        colourSelector->setColor (ColorSelector::backgroundColorId, Colors::transparentBlack);
        colourSelector->setSize (300, 400);

        CallOutBox::launchAsynchronously (std::move (colourSelector), getScreenBounds(), nullptr);
    }

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (auto* cs = dynamic_cast<ColorSelector*> (source))
            editor.applyNewValue (getAsString (cs->getCurrentColor(), true));

        repaint();
    }

    LivePropertyEditorBase& editor;
};

Component* createColorEditor (LivePropertyEditorBase& editor)
{
    return new ColorEditorComp (editor);
}

//==============================================================================
struct SliderComp : public Component
{
    SliderComp (LivePropertyEditorBase& e, b8 useFloat)
        : editor (e), isFloat (useFloat)
    {
        slider.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
        addAndMakeVisible (slider);
        updateRange();
        slider.onDragEnd = [this] { updateRange(); };
        slider.onValueChange = [this]
        {
            editor.applyNewValue (isFloat ? getAsString ((f64) slider.getValue(), editor.wasHex)
                                          : getAsString ((z64)  slider.getValue(), editor.wasHex));
        };
    }

    virtual z0 updateRange()
    {
        f64 v = isFloat ? parseDouble (editor.value.getStringValue (false))
                           : (f64) parseInt (editor.value.getStringValue (false));

        f64 range = isFloat ? 10 : 100;

        slider.setRange (v - range, v + range);
        slider.setValue (v, dontSendNotification);
    }

    z0 resized() override
    {
        slider.setBounds (getLocalBounds().removeFromTop (25));
    }

    LivePropertyEditorBase& editor;
    Slider slider;
    b8 isFloat;
};

//==============================================================================
struct BoolSliderComp final : public SliderComp
{
    BoolSliderComp (LivePropertyEditorBase& e)
        : SliderComp (e, false)
    {
        slider.onValueChange = [this] { editor.applyNewValue (slider.getValue() > 0.5 ? "true" : "false"); };
    }

    z0 updateRange() override
    {
        slider.setRange (0.0, 1.0, dontSendNotification);
        slider.setValue (editor.value.getStringValue (false) == "true", dontSendNotification);
    }
};

Component* createIntegerSlider (LivePropertyEditorBase& editor)  { return new SliderComp (editor, false); }
Component* createFloatSlider   (LivePropertyEditorBase& editor)  { return new SliderComp (editor, true);  }
Component* createBoolSlider    (LivePropertyEditorBase& editor)  { return new BoolSliderComp (editor); }

} // namespace drx::LiveConstantEditor

#endif
