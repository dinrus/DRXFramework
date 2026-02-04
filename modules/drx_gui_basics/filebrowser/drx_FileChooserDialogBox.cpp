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

class FileChooserDialogBox::ContentComponent final : public Component
{
public:
    ContentComponent (const Txt& name, const Txt& desc, FileBrowserComponent& chooser)
        : Component (name),
          chooserComponent (chooser),
          okButton (chooser.getActionVerb()),
          cancelButton (TRANS ("Cancel")),
          newFolderButton (TRANS ("New Folder")),
          instructions (desc)
    {
        addAndMakeVisible (chooserComponent);

        addAndMakeVisible (okButton);
        okButton.addShortcut (KeyPress (KeyPress::returnKey));

        addAndMakeVisible (cancelButton);
        cancelButton.addShortcut (KeyPress (KeyPress::escapeKey));

        addChildComponent (newFolderButton);

        setInterceptsMouseClicks (false, true);
    }

    z0 paint (Graphics& g) override
    {
        text.draw (g, getLocalBounds().reduced (6)
                        .removeFromTop ((i32) text.getHeight()).toFloat());
    }

    z0 resized() override
    {
        i32k buttonHeight = 26;

        auto area = getLocalBounds();

        text.createLayout (getLookAndFeel().createFileChooserHeaderText (getName(), instructions),
                           (f32) getWidth() - 12.0f);

        area.removeFromTop (roundToInt (text.getHeight()) + 10);

        chooserComponent.setBounds (area.removeFromTop (area.getHeight() - buttonHeight - 20));
        auto buttonArea = area.reduced (16, 10);

        okButton.changeWidthToFitText (buttonHeight);
        okButton.setBounds (buttonArea.removeFromRight (okButton.getWidth() + 16));

        buttonArea.removeFromRight (16);

        cancelButton.changeWidthToFitText (buttonHeight);
        cancelButton.setBounds (buttonArea.removeFromRight (cancelButton.getWidth()));

        newFolderButton.changeWidthToFitText (buttonHeight);
        newFolderButton.setBounds (buttonArea.removeFromLeft (newFolderButton.getWidth()));
    }

    FileBrowserComponent& chooserComponent;
    TextButton okButton, cancelButton, newFolderButton;
    Txt instructions;
    TextLayout text;
};

//==============================================================================
FileChooserDialogBox::FileChooserDialogBox (const Txt& name,
                                            const Txt& instructions,
                                            FileBrowserComponent& chooserComponent,
                                            b8 shouldWarn,
                                            Color backgroundColor,
                                            Component* parentComp)
    : ResizableWindow (name, backgroundColor, parentComp == nullptr),
      warnAboutOverwritingExistingFiles (shouldWarn)
{
    content = new ContentComponent (name, instructions, chooserComponent);
    setContentOwned (content, false);

    setResizable (true, true);
    setResizeLimits (300, 300, 1200, 1000);

    content->okButton.onClick        = [this] { okButtonPressed(); };
    content->cancelButton.onClick    = [this] { closeButtonPressed(); };
    content->newFolderButton.onClick = [this] { createNewFolder(); };

    content->chooserComponent.addListener (this);

    FileChooserDialogBox::selectionChanged();

    if (parentComp != nullptr)
        parentComp->addAndMakeVisible (this);
    else
        setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());
}

FileChooserDialogBox::~FileChooserDialogBox()
{
    content->chooserComponent.removeListener (this);
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
b8 FileChooserDialogBox::show (i32 w, i32 h)
{
    return showAt (-1, -1, w, h);
}

b8 FileChooserDialogBox::showAt (i32 x, i32 y, i32 w, i32 h)
{
    if (w <= 0)  w = getDefaultWidth();
    if (h <= 0)  h = 500;

    if (x < 0 || y < 0)
        centreWithSize (w, h);
    else
        setBounds (x, y, w, h);

    const b8 ok = (runModalLoop() != 0);
    setVisible (false);
    return ok;
}
#endif

z0 FileChooserDialogBox::centreWithDefaultSize (Component* componentToCentreAround)
{
    centreAroundComponent (componentToCentreAround, getDefaultWidth(), 500);
}

i32 FileChooserDialogBox::getDefaultWidth() const
{
    if (auto* previewComp = content->chooserComponent.getPreviewComponent())
        return 400 + previewComp->getWidth();

    return 600;
}

//==============================================================================
z0 FileChooserDialogBox::closeButtonPressed()
{
    setVisible (false);
}

z0 FileChooserDialogBox::selectionChanged()
{
    content->okButton.setEnabled (content->chooserComponent.currentFileIsValid());

    content->newFolderButton.setVisible (content->chooserComponent.isSaveMode()
                                          && content->chooserComponent.getRoot().isDirectory());
}

z0 FileChooserDialogBox::fileDoubleClicked (const File&)
{
    selectionChanged();
    content->okButton.triggerClick();
}

z0 FileChooserDialogBox::fileClicked (const File&, const MouseEvent&) {}
z0 FileChooserDialogBox::browserRootChanged (const File&) {}

z0 FileChooserDialogBox::okButtonPressed()
{
    if (warnAboutOverwritingExistingFiles
         && content->chooserComponent.isSaveMode()
         && content->chooserComponent.getSelectedFile (0).exists())
    {
        auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                               TRANS ("File already exists"),
                                                               TRANS ("There's already a file called: FLNM")
                                                                  .replace ("FLNM", content->chooserComponent.getSelectedFile (0).getFullPathName())
                                                                 + "\n\n"
                                                                 + TRANS ("Are you sure you want to overwrite it?"),
                                                               TRANS ("Overwrite"),
                                                               TRANS ("Cancel"),
                                                               this);
        messageBox = AlertWindow::showScopedAsync (options, [this] (i32 result)
        {
            if (result != 0)
                exitModalState (1);
        });
    }
    else
    {
        exitModalState (1);
    }
}

z0 FileChooserDialogBox::createNewFolderCallback (i32 result, FileChooserDialogBox* box,
                                                    Component::SafePointer<AlertWindow> alert)
{
    if (result != 0 && alert != nullptr && box != nullptr)
    {
        alert->setVisible (false);
        box->createNewFolderConfirmed (alert->getTextEditorContents ("Folder Name"));
    }
}

z0 FileChooserDialogBox::createNewFolder()
{
    auto parent = content->chooserComponent.getRoot();

    if (parent.isDirectory())
    {
        auto* aw = new AlertWindow (TRANS ("New Folder"),
                                    TRANS ("Please enter the name for the folder"),
                                    MessageBoxIconType::NoIcon, this);

        aw->addTextEditor ("Folder Name", Txt(), Txt(), false);
        aw->addButton (TRANS ("Create Folder"), 1, KeyPress (KeyPress::returnKey));
        aw->addButton (TRANS ("Cancel"),        0, KeyPress (KeyPress::escapeKey));

        aw->enterModalState (true,
                             ModalCallbackFunction::forComponent (createNewFolderCallback, this,
                                                                  Component::SafePointer<AlertWindow> (aw)),
                             true);
    }
}

z0 FileChooserDialogBox::createNewFolderConfirmed (const Txt& nameFromDialog)
{
    auto name = File::createLegalFileName (nameFromDialog);

    if (! name.isEmpty())
    {
        auto parent = content->chooserComponent.getRoot();

        if (! parent.getChildFile (name).createDirectory())
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             TRANS ("New Folder"),
                                                             TRANS ("Couldn't create the folder!"));
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }

        content->chooserComponent.refresh();
    }
}

} // namespace drx
