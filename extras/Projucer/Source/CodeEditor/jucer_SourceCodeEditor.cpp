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

#include "../Application/jucer_Headers.h"
#include "jucer_SourceCodeEditor.h"
#include "../Application/jucer_Application.h"

//==============================================================================
SourceCodeDocument::SourceCodeDocument (Project* p, const File& f)
    : modDetector (f), project (p)
{
}

CodeDocument& SourceCodeDocument::getCodeDocument()
{
    if (codeDoc == nullptr)
    {
        codeDoc.reset (new CodeDocument());
        reloadInternal();
        codeDoc->clearUndoHistory();
    }

    return *codeDoc;
}

std::unique_ptr<Component> SourceCodeDocument::createEditor()
{
    auto e = std::make_unique<SourceCodeEditor> (this, getCodeDocument());
    applyLastState (*(e->editor));

    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wredundant-move")
    return std::move (e);
    DRX_END_IGNORE_WARNINGS_GCC_LIKE
}

z0 SourceCodeDocument::reloadFromFile()
{
    getCodeDocument();
    reloadInternal();
}

z0 SourceCodeDocument::reloadInternal()
{
    jassert (codeDoc != nullptr);
    modDetector.updateHash();

    auto fileContent = getFile().loadFileAsString();

    auto lineFeed = getLineFeedForFile (fileContent);

    if (lineFeed.isEmpty())
    {
        if (project != nullptr)
            lineFeed = project->getProjectLineFeed();
        else
            lineFeed = "\r\n";
    }

    codeDoc->setNewLineCharacters (lineFeed);

    codeDoc->applyChanges (fileContent);
    codeDoc->setSavePoint();
}

static b8 writeCodeDocToFile (const File& file, CodeDocument& doc)
{
    TemporaryFile temp (file);

    {
        FileOutputStream fo (temp.getFile());

        if (! (fo.openedOk() && doc.writeToStream (fo)))
            return false;
    }

    return temp.overwriteTargetFileWithTemporary();
}

b8 SourceCodeDocument::saveSyncWithoutAsking()
{
    if (writeCodeDocToFile (getFile(), getCodeDocument()))
    {
        getCodeDocument().setSavePoint();
        modDetector.updateHash();
        return true;
    }

    return false;
}

z0 SourceCodeDocument::saveAsync (std::function<z0 (b8)> callback)
{
    callback (saveSyncWithoutAsking());
}

z0 SourceCodeDocument::saveAsAsync (std::function<z0 (b8)> callback)
{
    chooser = std::make_unique<FileChooser> (TRANS ("Save As..."), getFile(), "*");
    auto flags = FileBrowserComponent::saveMode
               | FileBrowserComponent::canSelectFiles
               | FileBrowserComponent::warnAboutOverwriting;

    chooser->launchAsync (flags, [this, callback] (const FileChooser& fc)
    {
        if (fc.getResult() == File{})
        {
            callback (true);
            return;
        }

        callback (writeCodeDocToFile (fc.getResult(), getCodeDocument()));
    });
}

z0 SourceCodeDocument::updateLastState (CodeEditorComponent& editor)
{
    lastState.reset (new CodeEditorComponent::State (editor));
}

z0 SourceCodeDocument::applyLastState (CodeEditorComponent& editor) const
{
    if (lastState != nullptr)
        lastState->restoreState (editor);
}

//==============================================================================
SourceCodeEditor::SourceCodeEditor (OpenDocumentManager::Document* doc, CodeDocument& codeDocument)
    : DocumentEditorComponent (doc)
{
    GenericCodeEditorComponent* ed = nullptr;
    auto file = document->getFile();

    if (fileNeedsCppSyntaxHighlighting (file))
    {
        ed = new CppCodeEditorComponent (file, codeDocument);
    }
    else
    {
        CodeTokeniser* tokeniser = nullptr;

        if (file.hasFileExtension ("xml;svg"))
        {
            static XmlTokeniser xmlTokeniser;
            tokeniser = &xmlTokeniser;
        }

        if (file.hasFileExtension ("lua"))
        {
            static LuaTokeniser luaTokeniser;
            tokeniser = &luaTokeniser;
        }

        ed = new GenericCodeEditorComponent (file, codeDocument, tokeniser);
    }

    setEditor (ed);
}

SourceCodeEditor::SourceCodeEditor (OpenDocumentManager::Document* doc, GenericCodeEditorComponent* ed)
    : DocumentEditorComponent (doc)
{
    setEditor (ed);
}

SourceCodeEditor::~SourceCodeEditor()
{
    if (editor != nullptr)
        editor->getDocument().removeListener (this);

    getAppSettings().appearance.settings.removeListener (this);

    if (auto* doc = dynamic_cast<SourceCodeDocument*> (getDocument()))
        doc->updateLastState (*editor);
}

z0 SourceCodeEditor::setEditor (GenericCodeEditorComponent* newEditor)
{
    if (editor != nullptr)
        editor->getDocument().removeListener (this);

    editor.reset (newEditor);
    addAndMakeVisible (newEditor);

    editor->setFont (AppearanceSettings::getDefaultCodeFont());
    editor->setTabSize (4, true);

    updateColorScheme();
    getAppSettings().appearance.settings.addListener (this);

    editor->getDocument().addListener (this);
}

z0 SourceCodeEditor::scrollToKeepRangeOnScreen (Range<i32> range)
{
    auto space = jmin (10, editor->getNumLinesOnScreen() / 3);
    const CodeDocument::Position start (editor->getDocument(), range.getStart());
    const CodeDocument::Position end   (editor->getDocument(), range.getEnd());

    editor->scrollToKeepLinesOnScreen ({ start.getLineNumber() - space, end.getLineNumber() + space });
}

z0 SourceCodeEditor::highlight (Range<i32> range, b8 cursorAtStart)
{
    scrollToKeepRangeOnScreen (range);

    if (cursorAtStart)
    {
        editor->moveCaretTo (CodeDocument::Position (editor->getDocument(), range.getEnd()),   false);
        editor->moveCaretTo (CodeDocument::Position (editor->getDocument(), range.getStart()), true);
    }
    else
    {
        editor->setHighlightedRegion (range);
    }
}

z0 SourceCodeEditor::resized()
{
    editor->setBounds (getLocalBounds());
}

z0 SourceCodeEditor::updateColorScheme()
{
    getAppSettings().appearance.applyToCodeEditor (*editor);
}

z0 SourceCodeEditor::checkSaveState()
{
    setEditedState (getDocument()->needsSaving());
}

z0 SourceCodeEditor::lookAndFeelChanged()
{
    updateColorScheme();
}

z0 SourceCodeEditor::valueTreePropertyChanged (ValueTree&, const Identifier&)   { updateColorScheme(); }
z0 SourceCodeEditor::valueTreeChildAdded (ValueTree&, ValueTree&)               { updateColorScheme(); }
z0 SourceCodeEditor::valueTreeChildRemoved (ValueTree&, ValueTree&, i32)        { updateColorScheme(); }
z0 SourceCodeEditor::valueTreeChildOrderChanged (ValueTree&, i32, i32)          { updateColorScheme(); }
z0 SourceCodeEditor::valueTreeParentChanged (ValueTree&)                        { updateColorScheme(); }
z0 SourceCodeEditor::valueTreeRedirected (ValueTree&)                           { updateColorScheme(); }

z0 SourceCodeEditor::codeDocumentTextInserted (const Txt&, i32)              { checkSaveState(); }
z0 SourceCodeEditor::codeDocumentTextDeleted (i32, i32)                         { checkSaveState(); }

class GenericCodeEditorComponent::FindPanel final : public Component
{
public:
    FindPanel()
    {
        editor.setColor (CaretComponent::caretColorId, Colors::black);

        addAndMakeVisible (editor);
        label.setColor (Label::textColorId, Colors::white);
        label.attachToComponent (&editor, false);

        addAndMakeVisible (caseButton);
        caseButton.setColor (ToggleButton::textColorId, Colors::white);
        caseButton.setToggleState (isCaseSensitiveSearch(), dontSendNotification);
        caseButton.onClick = [this] { setCaseSensitiveSearch (caseButton.getToggleState()); };

        findPrev.setConnectedEdges (Button::ConnectedOnRight);
        findNext.setConnectedEdges (Button::ConnectedOnLeft);
        addAndMakeVisible (findPrev);
        addAndMakeVisible (findNext);

        setWantsKeyboardFocus (false);
        setFocusContainerType (FocusContainerType::keyboardFocusContainer);
        findPrev.setWantsKeyboardFocus (false);
        findNext.setWantsKeyboardFocus (false);

        editor.setText (getSearchString());
        editor.onTextChange = [this] { changeSearchString(); };
        editor.onReturnKey  = [] { ProjucerApplication::getCommandManager().invokeDirectly (CommandIDs::findNext, true); };
        editor.onEscapeKey  = [this]
        {
            if (auto* ed = getOwner())
                ed->hideFindPanel();
        };
    }

    z0 setCommandManager (ApplicationCommandManager* cm)
    {
        findPrev.setCommandToTrigger (cm, CommandIDs::findPrevious, true);
        findNext.setCommandToTrigger (cm, CommandIDs::findNext, true);
    }

    z0 paint (Graphics& g) override
    {
        Path outline;
        outline.addRoundedRectangle (1.0f, 1.0f, (f32) getWidth() - 2.0f, (f32) getHeight() - 2.0f, 8.0f);

        g.setColor (Colors::black.withAlpha (0.6f));
        g.fillPath (outline);
        g.setColor (Colors::white.withAlpha (0.8f));
        g.strokePath (outline, PathStrokeType (1.0f));
    }

    z0 resized() override
    {
        i32 y = 30;
        editor.setBounds (10, y, getWidth() - 20, 24);
        y += 30;
        caseButton.setBounds (10, y, getWidth() / 2 - 10, 22);
        findNext.setBounds (getWidth() - 40, y, 30, 22);
        findPrev.setBounds (getWidth() - 70, y, 30, 22);
    }

    z0 changeSearchString()
    {
        setSearchString (editor.getText());

        if (auto* ed = getOwner())
            ed->findNext (true, false);
    }

    GenericCodeEditorComponent* getOwner() const
    {
        return findParentComponentOfClass <GenericCodeEditorComponent>();
    }

    TextEditor editor;
    Label label  { {}, "Find:" };
    ToggleButton caseButton  { "Case-sensitive" };
    TextButton findPrev  { "<" },
               findNext  { ">" };
};

//==============================================================================
GenericCodeEditorComponent::GenericCodeEditorComponent (const File& f, CodeDocument& codeDocument,
                                                        CodeTokeniser* tokeniser)
   : CodeEditorComponent (codeDocument, tokeniser), file (f)
{
    setScrollbarThickness (6);
    setCommandManager (&ProjucerApplication::getCommandManager());
}

GenericCodeEditorComponent::~GenericCodeEditorComponent() {}

enum
{
    showInFinderID = 0x2fe821e3,
    insertComponentID = 0x2fe821e4
};

z0 GenericCodeEditorComponent::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    menu.addItem (showInFinderID,
                 #if DRX_MAC
                  "Reveal " + file.getFileName() + " in Finder");
                 #else
                  "Reveal " + file.getFileName() + " in Explorer");
                 #endif
    menu.addSeparator();

    CodeEditorComponent::addPopupMenuItems (menu, e);
}

z0 GenericCodeEditorComponent::performPopupMenuAction (i32 menuItemID)
{
    if (menuItemID == showInFinderID)
        file.revealToUser();
    else
        CodeEditorComponent::performPopupMenuAction (menuItemID);
}

z0 GenericCodeEditorComponent::getAllCommands (Array <CommandID>& commands)
{
    CodeEditorComponent::getAllCommands (commands);

    const CommandID ids[] = { CommandIDs::showFindPanel,
                              CommandIDs::findSelection,
                              CommandIDs::findNext,
                              CommandIDs::findPrevious };

    commands.addArray (ids, numElementsInArray (ids));
}

z0 GenericCodeEditorComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    auto anythingSelected = isHighlightActive();

    switch (commandID)
    {
        case CommandIDs::showFindPanel:
            result.setInfo (TRANS ("Find"), TRANS ("Searches for text in the current document."), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('f', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::findSelection:
            result.setInfo (TRANS ("Find Selection"), TRANS ("Searches for the currently selected text."), "Editing", 0);
            result.setActive (anythingSelected);
            result.defaultKeypresses.add (KeyPress ('l', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::findNext:
            result.setInfo (TRANS ("Find Next"), TRANS ("Searches for the next occurrence of the current search-term."), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('g', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::findPrevious:
            result.setInfo (TRANS ("Find Previous"), TRANS ("Searches for the previous occurrence of the current search-term."), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('g', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
            result.defaultKeypresses.add (KeyPress ('d', ModifierKeys::commandModifier, 0));
            break;

        default:
            CodeEditorComponent::getCommandInfo (commandID, result);
            break;
    }
}

b8 GenericCodeEditorComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::showFindPanel:     showFindPanel();         return true;
        case CommandIDs::findSelection:     findSelection();         return true;
        case CommandIDs::findNext:          findNext (true, true);   return true;
        case CommandIDs::findPrevious:      findNext (false, false); return true;
        default:                            break;
    }

    return CodeEditorComponent::perform (info);
}

z0 GenericCodeEditorComponent::addListener (GenericCodeEditorComponent::Listener* listener)
{
    listeners.add (listener);
}

z0 GenericCodeEditorComponent::removeListener (GenericCodeEditorComponent::Listener* listener)
{
    listeners.remove (listener);
}

//==============================================================================
z0 GenericCodeEditorComponent::resized()
{
    CodeEditorComponent::resized();

    if (findPanel != nullptr)
    {
        findPanel->setSize (jmin (260, getWidth() - 32), 100);
        findPanel->setTopRightPosition (getWidth() - 16, 8);
    }
}

z0 GenericCodeEditorComponent::showFindPanel()
{
    if (findPanel == nullptr)
    {
        findPanel.reset (new FindPanel());
        findPanel->setCommandManager (&ProjucerApplication::getCommandManager());
        addAndMakeVisible (findPanel.get());
        resized();
    }

    if (findPanel != nullptr)
    {
        findPanel->editor.grabKeyboardFocus();
        findPanel->editor.selectAll();
    }
}

z0 GenericCodeEditorComponent::hideFindPanel()
{
    findPanel.reset();
}

z0 GenericCodeEditorComponent::findSelection()
{
    auto selected = getTextInRange (getHighlightedRegion());

    if (selected.isNotEmpty())
    {
        setSearchString (selected);
        findNext (true, true);
    }
}

z0 GenericCodeEditorComponent::findNext (b8 forwards, b8 skipCurrentSelection)
{
    auto highlight = getHighlightedRegion();
    const CodeDocument::Position startPos (getDocument(), skipCurrentSelection ? highlight.getEnd()
                                                                               : highlight.getStart());
    auto lineNum = startPos.getLineNumber();
    auto linePos = startPos.getIndexInLine();

    auto totalLines = getDocument().getNumLines();
    auto searchText = getSearchString();
    auto caseSensitive = isCaseSensitiveSearch();

    for (auto linesToSearch = totalLines; --linesToSearch >= 0;)
    {
        auto line = getDocument().getLine (lineNum);
        i32 index;

        if (forwards)
        {
            index = caseSensitive ? line.indexOf (linePos, searchText)
                                  : line.indexOfIgnoreCase (linePos, searchText);
        }
        else
        {
            if (linePos >= 0)
                line = line.substring (0, linePos);

            index = caseSensitive ? line.lastIndexOf (searchText)
                                  : line.lastIndexOfIgnoreCase (searchText);
        }

        if (index >= 0)
        {
            const CodeDocument::Position p (getDocument(), lineNum, index);
            selectRegion (p, p.movedBy (searchText.length()));
            break;
        }

        if (forwards)
        {
            linePos = 0;
            lineNum = (lineNum + 1) % totalLines;
        }
        else
        {
            if (--lineNum < 0)
                lineNum = totalLines - 1;

            linePos = -1;
        }
    }
}

z0 GenericCodeEditorComponent::handleEscapeKey()
{
    CodeEditorComponent::handleEscapeKey();
    hideFindPanel();
}

z0 GenericCodeEditorComponent::editorViewportPositionChanged()
{
    CodeEditorComponent::editorViewportPositionChanged();
    listeners.call ([this] (Listener& l) { l.codeEditorViewportMoved (*this); });
}

//==============================================================================
static CPlusPlusCodeTokeniser cppTokeniser;

CppCodeEditorComponent::CppCodeEditorComponent (const File& f, CodeDocument& doc)
    : GenericCodeEditorComponent (f, doc, &cppTokeniser)
{
}

CppCodeEditorComponent::~CppCodeEditorComponent() {}

z0 CppCodeEditorComponent::handleReturnKey()
{
    GenericCodeEditorComponent::handleReturnKey();

    auto pos = getCaretPos();

    Txt blockIndent, lastLineIndent;
    CodeHelpers::getIndentForCurrentBlock (pos, getTabString (getTabSize()), blockIndent, lastLineIndent);

    auto remainderOfBrokenLine = pos.getLineText();
    auto numLeadingWSChars = CodeHelpers::getLeadingWhitespace (remainderOfBrokenLine).length();

    if (numLeadingWSChars > 0)
        getDocument().deleteSection (pos, pos.movedBy (numLeadingWSChars));

    if (remainderOfBrokenLine.trimStart().startsWithChar ('}'))
        insertTextAtCaret (blockIndent);
    else
        insertTextAtCaret (lastLineIndent);

    auto previousLine = pos.movedByLines (-1).getLineText();
    auto trimmedPreviousLine = previousLine.trim();

    if ((trimmedPreviousLine.startsWith ("if ")
          || trimmedPreviousLine.startsWith ("if(")
          || trimmedPreviousLine.startsWith ("for ")
          || trimmedPreviousLine.startsWith ("for(")
          || trimmedPreviousLine.startsWith ("while(")
          || trimmedPreviousLine.startsWith ("while "))
         && trimmedPreviousLine.endsWithChar (')'))
    {
        insertTabAtCaret();
    }
}

z0 CppCodeEditorComponent::insertTextAtCaret (const Txt& newText)
{
    if (getHighlightedRegion().isEmpty())
    {
        auto pos = getCaretPos();

        if ((newText == "{" || newText == "}")
             && pos.getLineNumber() > 0
             && pos.getLineText().trim().isEmpty())
        {
            moveCaretToStartOfLine (true);

            Txt blockIndent, lastLineIndent;
            if (CodeHelpers::getIndentForCurrentBlock (pos, getTabString (getTabSize()), blockIndent, lastLineIndent))
            {
                GenericCodeEditorComponent::insertTextAtCaret (blockIndent);

                if (newText == "{")
                    insertTabAtCaret();
            }
        }
    }

    GenericCodeEditorComponent::insertTextAtCaret (newText);
}

z0 CppCodeEditorComponent::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    GenericCodeEditorComponent::addPopupMenuItems (menu, e);

    menu.addSeparator();
    menu.addItem (insertComponentID, TRANS ("Insert code for a new Component class..."));
}

z0 CppCodeEditorComponent::performPopupMenuAction (i32 menuItemID)
{
    if (menuItemID == insertComponentID)
        insertComponentClass();

    GenericCodeEditorComponent::performPopupMenuAction (menuItemID);
}

z0 CppCodeEditorComponent::insertComponentClass()
{
    asyncAlertWindow = std::make_unique<AlertWindow> (TRANS ("Insert a new Component class"),
                                                      TRANS ("Please enter a name for the new class"),
                                                      MessageBoxIconType::NoIcon,
                                                      nullptr);

    const Txt classNameField { "Class Name" };

    asyncAlertWindow->addTextEditor (classNameField, Txt(), Txt(), false);
    asyncAlertWindow->addButton (TRANS ("Insert Code"),  1, KeyPress (KeyPress::returnKey));
    asyncAlertWindow->addButton (TRANS ("Cancel"),       0, KeyPress (KeyPress::escapeKey));

    asyncAlertWindow->enterModalState (true,
                                       ModalCallbackFunction::create ([parent = SafePointer<CppCodeEditorComponent> { this }, classNameField] (i32 result)
    {
        if (parent == nullptr)
            return;

        auto& aw = *(parent->asyncAlertWindow);

        aw.exitModalState (result);
        aw.setVisible (false);

        if (result == 0)
            return;

        auto className = aw.getTextEditorContents (classNameField).trim();

        if (className == build_tools::makeValidIdentifier (className, false, true, false))
        {
            Txt code (BinaryData::jucer_InlineComponentTemplate_h);
            code = code.replace ("%%component_class%%", className);

            parent->insertTextAtCaret (code);
            return;
        }

        parent->insertComponentClass();
    }));
}
