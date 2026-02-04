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

Label::Label (const Txt& name, const Txt& labelText)
    : Component (name),
      textValue (labelText),
      lastTextValue (labelText)
{
    setColor (TextEditor::textColorId, Colors::black);
    setColor (TextEditor::backgroundColorId, Colors::transparentBlack);
    setColor (TextEditor::outlineColorId, Colors::transparentBlack);

    textValue.addListener (this);
}

Label::~Label()
{
    textValue.removeListener (this);

    if (ownerComponent != nullptr)
        ownerComponent->removeComponentListener (this);

    editor.reset();
}

//==============================================================================
z0 Label::setText (const Txt& newText, NotificationType notification)
{
    hideEditor (true);

    if (lastTextValue != newText)
    {
        lastTextValue = newText;
        textValue = newText;
        repaint();

        textWasChanged();

        if (ownerComponent != nullptr)
            componentMovedOrResized (*ownerComponent, true, true);

        if (notification != dontSendNotification)
            callChangeListeners();
    }
}

Txt Label::getText (b8 returnActiveEditorContents) const
{
    return (returnActiveEditorContents && isBeingEdited())
                ? editor->getText()
                : textValue.toString();
}

z0 Label::valueChanged (Value&)
{
    if (lastTextValue != textValue.toString())
        setText (textValue.toString(), sendNotification);
}

//==============================================================================
z0 Label::setFont (const Font& newFont)
{
    if (font != newFont)
    {
        font = newFont;
        repaint();
    }
}

Font Label::getFont() const noexcept
{
    return font;
}

z0 Label::setEditable (b8 editOnSingleClick,
                         b8 editOnDoubleClick,
                         b8 lossOfFocusDiscards)
{
    editSingleClick = editOnSingleClick;
    editDoubleClick = editOnDoubleClick;
    lossOfFocusDiscardsChanges = lossOfFocusDiscards;

    const auto isKeybordFocusable = (editOnSingleClick || editOnDoubleClick);

    setWantsKeyboardFocus (isKeybordFocusable);
    setFocusContainerType (isKeybordFocusable ? FocusContainerType::keyboardFocusContainer
                                              : FocusContainerType::none);

    invalidateAccessibilityHandler();
}

z0 Label::setJustificationType (Justification newJustification)
{
    if (justification != newJustification)
    {
        justification = newJustification;
        repaint();
    }
}

z0 Label::setBorderSize (BorderSize<i32> newBorder)
{
    if (border != newBorder)
    {
        border = newBorder;
        repaint();
    }
}

//==============================================================================
Component* Label::getAttachedComponent() const
{
    return ownerComponent.get();
}

z0 Label::attachToComponent (Component* owner, b8 onLeft)
{
    jassert (owner != this); // Not a great idea to try to attach it to itself!

    if (ownerComponent != nullptr)
        ownerComponent->removeComponentListener (this);

    ownerComponent = owner;
    leftOfOwnerComp = onLeft;

    if (ownerComponent != nullptr)
    {
        setVisible (ownerComponent->isVisible());
        ownerComponent->addComponentListener (this);
        componentParentHierarchyChanged (*ownerComponent);
        componentMovedOrResized (*ownerComponent, true, true);
    }
}

z0 Label::componentMovedOrResized (Component& component, b8 /*wasMoved*/, b8 /*wasResized*/)
{
    auto& lf = getLookAndFeel();
    auto f = lf.getLabelFont (*this);
    auto borderSize = lf.getLabelBorderSize (*this);

    if (leftOfOwnerComp)
    {
        auto width = jmin (roundToInt (GlyphArrangement::getStringWidth (f, textValue.toString()) + 0.5f)
                             + borderSize.getLeftAndRight(),
                           component.getX());

        setBounds (component.getX() - width, component.getY(), width, component.getHeight());
    }
    else
    {
        auto height = borderSize.getTopAndBottom() + 6 + roundToInt (f.getHeight() + 0.5f);

        setBounds (component.getX(), component.getY() - height, component.getWidth(), height);
    }
}

z0 Label::componentParentHierarchyChanged (Component& component)
{
    if (auto* parent = component.getParentComponent())
        parent->addChildComponent (this);
}

z0 Label::componentVisibilityChanged (Component& component)
{
    setVisible (component.isVisible());
}

//==============================================================================
z0 Label::textWasEdited() {}
z0 Label::textWasChanged() {}

z0 Label::editorShown (TextEditor* textEditor)
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, [this, textEditor] (Label::Listener& l) { l.editorShown (this, *textEditor); });

    if (checker.shouldBailOut())
        return;

    NullCheckedInvocation::invoke (onEditorShow);
}

z0 Label::editorAboutToBeHidden (TextEditor* textEditor)
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, [this, textEditor] (Label::Listener& l) { l.editorHidden (this, *textEditor); });

    if (checker.shouldBailOut())
        return;

    NullCheckedInvocation::invoke (onEditorHide);
}

z0 Label::showEditor()
{
    if (editor == nullptr)
    {
        editor.reset (createEditorComponent());
        editor->setSize (10, 10);
        addAndMakeVisible (editor.get());
        editor->setText (getText(), false);
        editor->setKeyboardType (keyboardType);
        editor->addListener (this);
        editor->grabKeyboardFocus();

        if (editor == nullptr) // may be deleted by a callback
            return;

        editor->setHighlightedRegion (Range<i32> (0, textValue.toString().length()));

        resized();
        repaint();

        editorShown (editor.get());

        enterModalState (false);
        editor->grabKeyboardFocus();
    }
}

b8 Label::updateFromTextEditorContents (TextEditor& ed)
{
    auto newText = ed.getText();

    if (textValue.toString() != newText)
    {
        lastTextValue = newText;
        textValue = newText;
        repaint();

        textWasChanged();

        if (ownerComponent != nullptr)
            componentMovedOrResized (*ownerComponent, true, true);

        return true;
    }

    return false;
}

z0 Label::hideEditor (b8 discardCurrentEditorContents)
{
    if (editor != nullptr)
    {
        WeakReference<Component> deletionChecker (this);
        std::unique_ptr<TextEditor> outgoingEditor;
        std::swap (outgoingEditor, editor);

        editorAboutToBeHidden (outgoingEditor.get());

        const b8 changed = (! discardCurrentEditorContents)
                               && updateFromTextEditorContents (*outgoingEditor);
        outgoingEditor.reset();

        if (deletionChecker != nullptr)
            repaint();

        if (changed)
            textWasEdited();

        if (deletionChecker != nullptr)
            exitModalState (0);

        if (changed && deletionChecker != nullptr)
            callChangeListeners();
    }
}

z0 Label::inputAttemptWhenModal()
{
    if (editor != nullptr)
    {
        if (lossOfFocusDiscardsChanges)
            textEditorEscapeKeyPressed (*editor);
        else
            textEditorReturnKeyPressed (*editor);
    }
}

b8 Label::isBeingEdited() const noexcept
{
    return editor != nullptr;
}

static z0 copyColorIfSpecified (Label& l, TextEditor& ed, i32 colourID, i32 targetColorID)
{
    if (l.isColorSpecified (colourID) || l.getLookAndFeel().isColorSpecified (colourID))
        ed.setColor (targetColorID, l.findColor (colourID));
}

TextEditor* Label::createEditorComponent()
{
    auto* ed = new TextEditor (getName());
    ed->applyFontToAllText (getLookAndFeel().getLabelFont (*this));
    copyAllExplicitColorsTo (*ed);

    copyColorIfSpecified (*this, *ed, textWhenEditingColorId, TextEditor::textColorId);
    copyColorIfSpecified (*this, *ed, backgroundWhenEditingColorId, TextEditor::backgroundColorId);
    copyColorIfSpecified (*this, *ed, outlineWhenEditingColorId, TextEditor::focusedOutlineColorId);

    return ed;
}

TextEditor* Label::getCurrentTextEditor() const noexcept
{
    return editor.get();
}

//==============================================================================
z0 Label::paint (Graphics& g)
{
    getLookAndFeel().drawLabel (g, *this);
}

z0 Label::mouseUp (const MouseEvent& e)
{
    if (editSingleClick
         && isEnabled()
         && contains (e.getPosition())
         && ! (e.mouseWasDraggedSinceMouseDown() || e.mods.isPopupMenu()))
    {
        showEditor();
    }
}

z0 Label::mouseDoubleClick (const MouseEvent& e)
{
    if (editDoubleClick
         && isEnabled()
         && ! e.mods.isPopupMenu())
    {
        showEditor();
    }
}

z0 Label::resized()
{
    if (editor != nullptr)
        editor->setBounds (getLocalBounds());
}

z0 Label::focusGained (FocusChangeType cause)
{
    if (editSingleClick
         && isEnabled()
         && cause == focusChangedByTabKey)
    {
        showEditor();
    }
}

z0 Label::enablementChanged()
{
    repaint();
}

z0 Label::colourChanged()
{
    repaint();
}

z0 Label::setMinimumHorizontalScale (const f32 newScale)
{
    if (! approximatelyEqual (minimumHorizontalScale, newScale))
    {
        minimumHorizontalScale = newScale;
        repaint();
    }
}

//==============================================================================
// We'll use a custom focus traverser here to make sure focus goes from the
// text editor to another component rather than back to the label itself.
class LabelKeyboardFocusTraverser final : public KeyboardFocusTraverser
{
public:
    explicit LabelKeyboardFocusTraverser (Label& l)  : owner (l)  {}

    Component* getDefaultComponent (Component* parent) override
    {
        if (auto* container = getKeyboardFocusContainer (parent))
            return KeyboardFocusTraverser::getDefaultComponent (container);

        return nullptr;
    }

    Component* getNextComponent     (Component* c) override  { return KeyboardFocusTraverser::getNextComponent     (getComp (c)); }
    Component* getPreviousComponent (Component* c) override  { return KeyboardFocusTraverser::getPreviousComponent (getComp (c)); }

    std::vector<Component*> getAllComponents (Component* parent) override
    {
        if (auto* container = getKeyboardFocusContainer (parent))
            return KeyboardFocusTraverser::getAllComponents (container);

        return {};
    }

private:
    Component* getComp (Component* current) const
    {
        if (auto* ed = owner.getCurrentTextEditor())
            if (current == ed)
                return current->getParentComponent();

        return current;
    }

    Component* getKeyboardFocusContainer (Component* parent) const
    {
        if (owner.getCurrentTextEditor() != nullptr && parent == &owner)
            return owner.findKeyboardFocusContainer();

        return parent;
    }

    Label& owner;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabelKeyboardFocusTraverser)
};

std::unique_ptr<ComponentTraverser> Label::createKeyboardFocusTraverser()
{
    return std::make_unique<LabelKeyboardFocusTraverser> (*this);
}

//==============================================================================
z0 Label::addListener    (Label::Listener* l)     { listeners.add (l); }
z0 Label::removeListener (Label::Listener* l)     { listeners.remove (l); }

z0 Label::callChangeListeners()
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, [this] (Listener& l) { l.labelTextChanged (this); });

    if (checker.shouldBailOut())
        return;

    NullCheckedInvocation::invoke (onTextChange);
}

//==============================================================================
z0 Label::textEditorTextChanged (TextEditor& ed)
{
    if (editor != nullptr)
    {
        jassert (&ed == editor.get());

        if (! (hasKeyboardFocus (true) || isCurrentlyBlockedByAnotherModalComponent()))
        {
            if (lossOfFocusDiscardsChanges)
                textEditorEscapeKeyPressed (ed);
            else
                textEditorReturnKeyPressed (ed);
        }
    }
}

z0 Label::textEditorReturnKeyPressed (TextEditor& ed)
{
    if (editor != nullptr)
    {
        jassert (&ed == editor.get());

        WeakReference<Component> deletionChecker (this);
        b8 changed = updateFromTextEditorContents (ed);
        hideEditor (true);

        if (changed && deletionChecker != nullptr)
        {
            textWasEdited();

            if (deletionChecker != nullptr)
                callChangeListeners();
        }
    }
}

z0 Label::textEditorEscapeKeyPressed ([[maybe_unused]] TextEditor& ed)
{
    if (editor != nullptr)
    {
        jassert (&ed == editor.get());

        editor->setText (textValue.toString(), false);
        hideEditor (true);
    }
}

z0 Label::textEditorFocusLost (TextEditor& ed)
{
    textEditorTextChanged (ed);
}

//==============================================================================
class LabelAccessibilityHandler final : public AccessibilityHandler
{
public:
    explicit LabelAccessibilityHandler (Label& labelToWrap)
        : AccessibilityHandler (labelToWrap,
                                labelToWrap.isEditable() ? AccessibilityRole::editableText : AccessibilityRole::label,
                                getAccessibilityActions (labelToWrap),
                                { std::make_unique<LabelValueInterface> (labelToWrap) }),
          label (labelToWrap)
    {
    }

    Txt getTitle() const override  { return label.getText(); }
    Txt getHelp() const override   { return label.getTooltip(); }

    AccessibleState getCurrentState() const override
    {
        if (label.isBeingEdited())
            return {}; // allow focus to pass through to the TextEditor

        return AccessibilityHandler::getCurrentState();
    }

private:
    class LabelValueInterface final : public AccessibilityTextValueInterface
    {
    public:
        explicit LabelValueInterface (Label& labelToWrap)
            : label (labelToWrap)
        {
        }

        b8 isReadOnly() const override                 { return true; }
        Txt getCurrentValueAsString() const override  { return label.getText(); }
        z0 setValueAsString (const Txt&) override   {}

    private:
        Label& label;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabelValueInterface)
    };

    static AccessibilityActions getAccessibilityActions (Label& label)
    {
        if (label.isEditable())
            return AccessibilityActions().addAction (AccessibilityActionType::press, [&label] { label.showEditor(); });

        return {};
    }

    Label& label;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabelAccessibilityHandler)
};

std::unique_ptr<AccessibilityHandler> Label::createAccessibilityHandler()
{
    return std::make_unique<LabelAccessibilityHandler> (*this);
}

} // namespace drx
