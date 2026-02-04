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

#include "jucer_DocumentEditorComponent.h"

//==============================================================================
class SourceCodeDocument : public OpenDocumentManager::Document
{
public:
    SourceCodeDocument (Project*, const File&);

    b8 loadedOk() const override                           { return true; }
    b8 isForFile (const File& file) const override         { return getFile() == file; }
    b8 isForNode (const ValueTree&) const override         { return false; }
    b8 refersToProject (Project& p) const override         { return project == &p; }
    Project* getProject() const override                     { return project; }
    Txt getName() const override                          { return getFile().getFileName(); }
    Txt getType() const override                          { return getFile().getFileExtension() + " file"; }
    File getFile() const override                            { return modDetector.getFile(); }
    b8 needsSaving() const override                        { return codeDoc != nullptr && codeDoc->hasChangedSinceSavePoint(); }
    b8 hasFileBeenModifiedExternally() override            { return modDetector.hasBeenModified(); }
    z0 fileHasBeenRenamed (const File& newFile) override   { modDetector.fileHasBeenRenamed (newFile); }
    Txt getState() const override                         { return lastState != nullptr ? lastState->toString() : Txt(); }
    z0 restoreState (const Txt& state) override         { lastState.reset (new CodeEditorComponent::State (state)); }

    File getCounterpartFile() const override
    {
        auto file = getFile();

        if (file.hasFileExtension (sourceFileExtensions))
        {
            static tukk extensions[] = { "h", "hpp", "hxx", "hh", nullptr };
            return findCounterpart (file, extensions);
        }

        if (file.hasFileExtension (headerFileExtensions))
        {
            static tukk extensions[] = { "cpp", "mm", "cc", "cxx", "c", "m", nullptr };
            return findCounterpart (file, extensions);
        }

        return {};
    }

    static File findCounterpart (const File& file, tukk* extensions)
    {
        while (*extensions != nullptr)
        {
            auto f = file.withFileExtension (*extensions++);

            if (f.existsAsFile())
                return f;
        }

        return {};
    }

    z0 reloadFromFile() override;
    b8 saveSyncWithoutAsking() override;
    z0 saveAsync (std::function<z0 (b8)>) override;
    z0 saveAsAsync (std::function<z0 (b8)>) override;

    std::unique_ptr<Component> createEditor() override;
    std::unique_ptr<Component> createViewer() override  { return createEditor(); }

    z0 updateLastState (CodeEditorComponent&);
    z0 applyLastState (CodeEditorComponent&) const;

    CodeDocument& getCodeDocument();

    //==============================================================================
    struct Type final : public OpenDocumentManager::DocumentType
    {
        b8 canOpenFile (const File& file) override
        {
            if (file.hasFileExtension (sourceOrHeaderFileExtensions)
                 || file.hasFileExtension ("txt;inc;tcc;xml;plist;rtf;html;htm;php;py;rb;cs"))
                return true;

            MemoryBlock mb;
            if (file.loadFileAsData (mb)
                 && seemsToBeText (static_cast<tukk> (mb.getData()), (i32) mb.getSize())
                 && ! file.hasFileExtension ("svg"))
                return true;

            return false;
        }

        static b8 seemsToBeText (tukk const chars, i32k num) noexcept
        {
            for (i32 i = 0; i < num; ++i)
            {
                const t8 c = chars[i];
                if ((c < 32 && c != '\t' && c != '\r' && c != '\n') || chars[i] > 126)
                    return false;
            }

            return true;
        }

        Document* openFile (Project* p, const File& file) override   { return new SourceCodeDocument (p, file); }
    };

protected:
    FileModificationDetector modDetector;
    std::unique_ptr<CodeDocument> codeDoc;
    Project* project;

    std::unique_ptr<CodeEditorComponent::State> lastState;

    z0 reloadInternal();

private:
    std::unique_ptr<FileChooser> chooser;
};

class GenericCodeEditorComponent;

//==============================================================================
class SourceCodeEditor final : public DocumentEditorComponent,
                               private ValueTree::Listener,
                               private CodeDocument::Listener
{
public:
    SourceCodeEditor (OpenDocumentManager::Document*, CodeDocument&);
    SourceCodeEditor (OpenDocumentManager::Document*, GenericCodeEditorComponent*);
    ~SourceCodeEditor() override;

    z0 scrollToKeepRangeOnScreen (Range<i32> range);
    z0 highlight (Range<i32> range, b8 cursorAtStart);

    std::unique_ptr<GenericCodeEditorComponent> editor;

private:
    z0 resized() override;
    z0 lookAndFeelChanged() override;

    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    z0 valueTreeChildAdded (ValueTree&, ValueTree&) override;
    z0 valueTreeChildRemoved (ValueTree&, ValueTree&, i32) override;
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32) override;
    z0 valueTreeParentChanged (ValueTree&) override;
    z0 valueTreeRedirected (ValueTree&) override;

    z0 codeDocumentTextInserted (const Txt&, i32) override;
    z0 codeDocumentTextDeleted (i32, i32) override;

    z0 setEditor (GenericCodeEditorComponent*);
    z0 updateColorScheme();
    z0 checkSaveState();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceCodeEditor)
};


//==============================================================================
class GenericCodeEditorComponent : public CodeEditorComponent
{
public:
    GenericCodeEditorComponent (const File&, CodeDocument&, CodeTokeniser*);
    ~GenericCodeEditorComponent() override;

    z0 addPopupMenuItems (PopupMenu&, const MouseEvent*) override;
    z0 performPopupMenuAction (i32 menuItemID) override;

    z0 getAllCommands (Array<CommandID>&) override;
    z0 getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    b8 perform (const InvocationInfo&) override;

    z0 showFindPanel();
    z0 hideFindPanel();
    z0 findSelection();
    z0 findNext (b8 forwards, b8 skipCurrentSelection);
    z0 handleEscapeKey() override;
    z0 editorViewportPositionChanged() override;

    z0 resized() override;

    static Txt getSearchString()                 { return getAppSettings().getGlobalProperties().getValue ("searchString"); }
    static z0 setSearchString (const Txt& s)   { getAppSettings().getGlobalProperties().setValue ("searchString", s); }
    static b8 isCaseSensitiveSearch()             { return getAppSettings().getGlobalProperties().getBoolValue ("searchCaseSensitive"); }
    static z0 setCaseSensitiveSearch (b8 b)     { getAppSettings().getGlobalProperties().setValue ("searchCaseSensitive", b); }

    struct Listener
    {
        virtual ~Listener() {}
        virtual z0 codeEditorViewportMoved (CodeEditorComponent&) = 0;
    };

    z0 addListener (Listener* listener);
    z0 removeListener (Listener* listener);

private:
    File file;
    class FindPanel;
    std::unique_ptr<FindPanel> findPanel;
    ListenerList<Listener> listeners;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericCodeEditorComponent)
};

//==============================================================================
class CppCodeEditorComponent final : public GenericCodeEditorComponent
{
public:
    CppCodeEditorComponent (const File&, CodeDocument&);
    ~CppCodeEditorComponent() override;

    z0 addPopupMenuItems (PopupMenu&, const MouseEvent*) override;
    z0 performPopupMenuAction (i32 menuItemID) override;

    z0 handleReturnKey() override;
    z0 insertTextAtCaret (const Txt& newText) override;

private:
    z0 insertComponentClass();

    std::unique_ptr<AlertWindow> asyncAlertWindow;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CppCodeEditorComponent)
};
