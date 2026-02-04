/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             MDIDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Displays and edits MDI files.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MDIDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** The Note class contains text editor used to display and edit the note's contents and will
    also listen to changes in the text and mark the FileBasedDocument as 'dirty'. This 'dirty'
    flag is used to prompt the user to save the note when it is closed.
 */
class Note final : public Component,
                   public FileBasedDocument
{
public:
    Note (const Txt& name, const Txt& contents)
        : FileBasedDocument (".jnote", "*.jnote",
                             "Browse for note to load",
                             "Choose file to save note to"),
          textValueObject (contents)
    {
        // we need to use an separate Value object as our text source so it doesn't get marked
        // as changed immediately
        setName (name);

        editor.setMultiLine (true);
        editor.setReturnKeyStartsNewLine (true);
        editor.getTextValue().referTo (textValueObject);
        addAndMakeVisible (editor);
        editor.onTextChange = [this] { changed(); };
    }

    z0 resized() override
    {
        editor.setBounds (getLocalBounds());
    }

    Txt getDocumentTitle() override
    {
        return getName();
    }

    Result loadDocument (const File& file) override
    {
        editor.setText (file.loadFileAsString());
        return Result::ok();
    }

    Result saveDocument (const File& file) override
    {
        // attempt to save the contents into the given file
        if (file.replaceWithText (editor.getText()))
            return Result::ok();

        return Result::fail ("Can't write to file");
    }

    File getLastDocumentOpened() override
    {
        // not interested in this for now
        return {};
    }

    z0 setLastDocumentOpened (const File& /*file*/) override
    {
        // not interested in this for now
    }

    File getSuggestedSaveAsFile (const File&) override
    {
        return File::getSpecialLocation (File::userDesktopDirectory)
                    .getChildFile (getName())
                    .withFileExtension ("jnote");
    }

private:
    Value textValueObject;
    TextEditor editor;

    z0 lookAndFeelChanged() override
    {
        editor.applyFontToAllText (editor.getFont());
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Note)
};


//==============================================================================
/** Simple MultiDocumentPanel that just tries to save our notes when they are closed.
 */
class DemoMultiDocumentPanel final : public MultiDocumentPanel
{
public:
    DemoMultiDocumentPanel() = default;

    z0 tryToCloseDocumentAsync (Component* component, std::function<z0 (b8)> callback) override
    {
        if (auto* note = dynamic_cast<Note*> (component))
        {
            SafePointer<DemoMultiDocumentPanel> parent { this };
            note->saveIfNeededAndUserAgreesAsync ([parent, callback] (FileBasedDocument::SaveResult result)
            {
                if (parent != nullptr)
                    callback (result == FileBasedDocument::savedOk);
            });
        }
    }

    z0 activeDocumentChanged() override
    {
        if (auto* activeDoc = getActiveDocument())
            Logger::outputDebugString ("activeDocumentChanged() to " + activeDoc->getName());
    }

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoMultiDocumentPanel)
};

//==============================================================================
/** Simple multi-document panel that manages a number of notes that you can store to files.
    By default this will look for notes saved to the desktop and load them up.
 */
class MDIDemo final : public Component,
                      public FileDragAndDropTarget
{
public:
    MDIDemo()
    {
        setOpaque (true);

        showInTabsButton.setToggleState (false, dontSendNotification);
        showInTabsButton.onClick = [this] { updateLayoutMode(); };
        addAndMakeVisible (showInTabsButton);

        oneDocShouldBeFullscreenButton.onClick = [this]
        {
            multiDocumentPanel.useFullscreenWhenOneDocument (oneDocShouldBeFullscreenButton.getToggleState());
        };
        addAndMakeVisible (oneDocShouldBeFullscreenButton);
        oneDocShouldBeFullscreenButton.setToggleState (false, drx::sendNotification);

        addNoteButton.onClick = [this]
        {
            addNote ("Note " + Txt (noteCounter), "Hello World! " + Txt (noteCounter));
            ++noteCounter;
        };
        addAndMakeVisible (addNoteButton);

        closeActiveDocumentButton.onClick = [this]
        {
            multiDocumentPanel.closeDocumentAsync (multiDocumentPanel.getActiveDocument(), false, [] (auto) {});
        };
        addAndMakeVisible (closeActiveDocumentButton);

        closeApplicationButton.onClick = [this]
        {
            multiDocumentPanel.closeAllDocumentsAsync (true, [] (b8 allSaved)
            {
                if (allSaved)
                    DRXApplicationBase::quit();
            });
        };
        addAndMakeVisible (closeApplicationButton);

        addAndMakeVisible (multiDocumentPanel);
        multiDocumentPanel.setBackgroundColor (Colors::transparentBlack);

        updateLayoutMode();
        addNote ("Notes Demo", "You can drag-and-drop text files onto this page to open them as notes..");
        addExistingNotes();

        setSize (650, 500);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto area = getLocalBounds();

        auto topButtonRow = area.removeFromTop (28).reduced (2);

        showInTabsButton              .setBounds (topButtonRow.removeFromLeft (150));

        closeApplicationButton        .setBounds (topButtonRow.removeFromRight (150));
        addNoteButton                 .setBounds (topButtonRow.removeFromRight (150));
        closeActiveDocumentButton     .setBounds (topButtonRow.removeFromRight (150));

        oneDocShouldBeFullscreenButton.setBounds (area.removeFromTop (28).reduced (2).removeFromLeft (240));

        multiDocumentPanel.setBounds (area);
    }

    b8 isInterestedInFileDrag (const StringArray&) override
    {
        return true;
    }

    z0 filesDropped (const StringArray& filenames, i32 /*x*/, i32 /*y*/) override
    {
        Array<File> files;

        for (auto& f : filenames)
            files.add ({ f });

        createNotesForFiles (files);
    }

    z0 createNotesForFiles (const Array<File>& files)
    {
        for (auto& file : files)
        {
            auto content = file.loadFileAsString();

            if (content.length() > 20000)
                content = "Too i64!";

            addNote (file.getFileName(), content);
        }
    }

private:
    z0 updateLayoutMode()
    {
        multiDocumentPanel.setLayoutMode (showInTabsButton.getToggleState() ? MultiDocumentPanel::MaximisedWindowsWithTabs
                                                                            : MultiDocumentPanel::FloatingWindows);
    }

    z0 addNote (const Txt& name, const Txt& content)
    {
        auto* newNote = new Note (name, content);
        newNote->setSize (200, 200);

        multiDocumentPanel.addDocument (newNote, Colors::lightblue.withAlpha (0.6f), true);
    }

    z0 addExistingNotes()
    {
        Array<File> files;
        File::getSpecialLocation (File::userDesktopDirectory).findChildFiles (files, File::findFiles, false, "*.jnote");
        createNotesForFiles (files);
    }

    ToggleButton showInTabsButton               { "Show with tabs" };
    ToggleButton oneDocShouldBeFullscreenButton { "Fill screen when only one note is open" };
    TextButton   addNoteButton                  { "Create a new note" },
                 closeApplicationButton         { "Close app" },
                 closeActiveDocumentButton      { "Close active document" };

    DemoMultiDocumentPanel multiDocumentPanel;
    i32 noteCounter = 1;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDIDemo)
};
