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

FileBrowserComponent::FileBrowserComponent (i32 flags_,
                                            const File& initialFileOrDirectory,
                                            const FileFilter* fileFilter_,
                                            FilePreviewComponent* previewComp_)
   : FileFilter ({}),
     fileFilter (fileFilter_),
     flags (flags_),
     previewComp (previewComp_),
     currentPathBox ("path"),
     fileLabel ("f", TRANS ("file:")),
     thread (SystemStats::getDRXVersion() + ": FileBrowser"),
     wasProcessActive (true)
{
    // You need to specify one or other of the open/save flags..
    jassert ((flags & (saveMode | openMode)) != 0);
    jassert ((flags & (saveMode | openMode)) != (saveMode | openMode));

    // You need to specify at least one of these flags..
    jassert ((flags & (canSelectFiles | canSelectDirectories)) != 0);

    Txt filename;

    if (initialFileOrDirectory == File())
    {
        currentRoot = File::getCurrentWorkingDirectory();
    }
    else if (initialFileOrDirectory.isDirectory())
    {
        currentRoot = initialFileOrDirectory;
    }
    else
    {
        chosenFiles.add (initialFileOrDirectory);
        currentRoot = initialFileOrDirectory.getParentDirectory();
        filename = initialFileOrDirectory.getFileName();
    }

    // The thread must be started before the DirectoryContentsList attempts to scan any file
    thread.startThread (Thread::Priority::low);

    fileList.reset (new DirectoryContentsList (this, thread));
    fileList->setDirectory (currentRoot, true, true);

    if ((flags & useTreeView) != 0)
    {
        auto tree = new FileTreeComponent (*fileList);
        fileListComponent.reset (tree);

        if ((flags & canSelectMultipleItems) != 0)
            tree->setMultiSelectEnabled (true);

        addAndMakeVisible (tree);
    }
    else
    {
        auto list = new FileListComponent (*fileList);
        fileListComponent.reset (list);
        list->setOutlineThickness (1);

        if ((flags & canSelectMultipleItems) != 0)
            list->setMultipleSelectionEnabled (true);

        addAndMakeVisible (list);
    }

    fileListComponent->addListener (this);

    addAndMakeVisible (currentPathBox);
    currentPathBox.setEditableText (true);
    resetRecentPaths();
    currentPathBox.onChange = [this] { updateSelectedPath(); };

    addAndMakeVisible (filenameBox);
    filenameBox.setMultiLine (false);
    filenameBox.setSelectAllWhenFocused (true);
    filenameBox.setText (filename, false);
    filenameBox.onTextChange = [this] { sendListenerChangeMessage(); };
    filenameBox.onReturnKey  = [this] { changeFilename(); };
    filenameBox.onFocusLost  = [this]
    {
        if (! isSaveMode())
            selectionChanged();
    };

    filenameBox.setReadOnly ((flags & (filenameBoxIsReadOnly | canSelectMultipleItems)) != 0);

    addAndMakeVisible (fileLabel);
    fileLabel.attachToComponent (&filenameBox, true);

    if (previewComp != nullptr)
        addAndMakeVisible (previewComp);

    lookAndFeelChanged();

    setRoot (currentRoot);

    if (filename.isNotEmpty())
        setFileName (filename);

    startTimer (2000);
}

FileBrowserComponent::~FileBrowserComponent()
{
    fileListComponent.reset();
    fileList.reset();
    thread.stopThread (10000);
}

//==============================================================================
z0 FileBrowserComponent::addListener (FileBrowserListener* const newListener)
{
    listeners.add (newListener);
}

z0 FileBrowserComponent::removeListener (FileBrowserListener* const listener)
{
    listeners.remove (listener);
}

//==============================================================================
b8 FileBrowserComponent::isSaveMode() const noexcept
{
    return (flags & saveMode) != 0;
}

i32 FileBrowserComponent::getNumSelectedFiles() const noexcept
{
    if (chosenFiles.isEmpty() && currentFileIsValid())
        return 1;

    return chosenFiles.size();
}

File FileBrowserComponent::getSelectedFile (i32 index) const noexcept
{
    if ((flags & canSelectDirectories) != 0 && filenameBox.getText().isEmpty())
        return currentRoot;

    if (! filenameBox.isReadOnly())
        return currentRoot.getChildFile (filenameBox.getText());

    return chosenFiles[index];
}

b8 FileBrowserComponent::currentFileIsValid() const
{
    auto f = getSelectedFile (0);

    if ((flags & canSelectDirectories) == 0 && f.isDirectory())
        return false;

    return isSaveMode() || f.exists();
}

File FileBrowserComponent::getHighlightedFile() const noexcept
{
    return fileListComponent->getSelectedFile (0);
}

z0 FileBrowserComponent::deselectAllFiles()
{
    fileListComponent->deselectAllFiles();
}

//==============================================================================
b8 FileBrowserComponent::isFileSuitable (const File& file) const
{
    return (flags & canSelectFiles) != 0
             && (fileFilter == nullptr || fileFilter->isFileSuitable (file));
}

b8 FileBrowserComponent::isDirectorySuitable (const File&) const
{
    return true;
}

b8 FileBrowserComponent::isFileOrDirSuitable (const File& f) const
{
    if (f.isDirectory())
        return (flags & canSelectDirectories) != 0
                 && (fileFilter == nullptr || fileFilter->isDirectorySuitable (f));

    return (flags & canSelectFiles) != 0 && f.exists()
             && (fileFilter == nullptr || fileFilter->isFileSuitable (f));
}

//==============================================================================
const File& FileBrowserComponent::getRoot() const
{
    return currentRoot;
}

z0 FileBrowserComponent::setRoot (const File& newRootDirectory)
{
    b8 callListeners = false;

    if (currentRoot != newRootDirectory)
    {
        callListeners = true;
        fileListComponent->scrollToTop();

        Txt path (newRootDirectory.getFullPathName());

        if (path.isEmpty())
            path = File::getSeparatorString();

        StringArray rootNames, rootPaths;
        getRoots (rootNames, rootPaths);

        if (! rootPaths.contains (path, true))
        {
            b8 alreadyListed = false;

            for (i32 i = currentPathBox.getNumItems(); --i >= 0;)
            {
                if (currentPathBox.getItemText (i).equalsIgnoreCase (path))
                {
                    alreadyListed = true;
                    break;
                }
            }

            if (! alreadyListed)
                currentPathBox.addItem (path, currentPathBox.getNumItems() + 2);
        }
    }

    currentRoot = newRootDirectory;
    fileList->setDirectory (currentRoot, true, true);

    if (auto* tree = dynamic_cast<FileTreeComponent*> (fileListComponent.get()))
        tree->refresh();

    auto currentRootName = currentRoot.getFullPathName();

    if (currentRootName.isEmpty())
        currentRootName = File::getSeparatorString();

    currentPathBox.setText (currentRootName, dontSendNotification);

    goUpButton->setEnabled (currentRoot.getParentDirectory().isDirectory()
                             && currentRoot.getParentDirectory() != currentRoot);

    if (callListeners)
    {
        Component::BailOutChecker checker (this);
        listeners.callChecked (checker, [&] (FileBrowserListener& l) { l.browserRootChanged (currentRoot); });
    }
}

z0 FileBrowserComponent::setFileName (const Txt& newName)
{
    filenameBox.setText (newName, true);

    fileListComponent->setSelectedFile (currentRoot.getChildFile (newName));
}

z0 FileBrowserComponent::resetRecentPaths()
{
    currentPathBox.clear();

    StringArray rootNames, rootPaths;
    getRoots (rootNames, rootPaths);

    for (i32 i = 0; i < rootNames.size(); ++i)
    {
        if (rootNames[i].isEmpty())
            currentPathBox.addSeparator();
        else
            currentPathBox.addItem (rootNames[i], i + 1);
    }

    currentPathBox.addSeparator();
}

z0 FileBrowserComponent::goUp()
{
    setRoot (getRoot().getParentDirectory());
}

z0 FileBrowserComponent::refresh()
{
    fileList->refresh();
}

z0 FileBrowserComponent::setFileFilter (const FileFilter* const newFileFilter)
{
    if (fileFilter != newFileFilter)
    {
        fileFilter = newFileFilter;
        refresh();
    }
}

Txt FileBrowserComponent::getActionVerb() const
{
    return isSaveMode() ? ((flags & canSelectDirectories) != 0 ? TRANS ("Choose")
                                                               : TRANS ("Save"))
                        : TRANS ("Open");
}

z0 FileBrowserComponent::setFilenameBoxLabel (const Txt& name)
{
    fileLabel.setText (name, dontSendNotification);
}

FilePreviewComponent* FileBrowserComponent::getPreviewComponent() const noexcept
{
    return previewComp;
}

DirectoryContentsDisplayComponent* FileBrowserComponent::getDisplayComponent() const noexcept
{
    return fileListComponent.get();
}

//==============================================================================
z0 FileBrowserComponent::resized()
{
    getLookAndFeel()
        .layoutFileBrowserComponent (*this, fileListComponent.get(), previewComp,
                                     &currentPathBox, &filenameBox, goUpButton.get());
}

//==============================================================================
z0 FileBrowserComponent::lookAndFeelChanged()
{
    goUpButton.reset (getLookAndFeel().createFileBrowserGoUpButton());

    if (auto* buttonPtr = goUpButton.get())
    {
        addAndMakeVisible (*buttonPtr);
        buttonPtr->onClick = [this] { goUp(); };
        buttonPtr->setTooltip (TRANS ("Go up to parent directory"));
    }

    currentPathBox.setColor (ComboBox::backgroundColorId,    findColor (currentPathBoxBackgroundColorId));
    currentPathBox.setColor (ComboBox::textColorId,          findColor (currentPathBoxTextColorId));
    currentPathBox.setColor (ComboBox::arrowColorId,         findColor (currentPathBoxArrowColorId));

    filenameBox.setColor (TextEditor::backgroundColorId,     findColor (filenameBoxBackgroundColorId));
    filenameBox.applyColorToAllText (findColor (filenameBoxTextColorId));

    resized();
}

//==============================================================================
z0 FileBrowserComponent::sendListenerChangeMessage()
{
    Component::BailOutChecker checker (this);

    if (previewComp != nullptr)
        previewComp->selectedFileChanged (getSelectedFile (0));

    // You shouldn't delete the browser when the file gets changed!
    jassert (! checker.shouldBailOut());

    listeners.callChecked (checker, [] (FileBrowserListener& l) { l.selectionChanged(); });
}

z0 FileBrowserComponent::selectionChanged()
{
    StringArray newFilenames;
    b8 resetChosenFiles = true;

    for (i32 i = 0; i < fileListComponent->getNumSelectedFiles(); ++i)
    {
        const File f (fileListComponent->getSelectedFile (i));

        if (isFileOrDirSuitable (f))
        {
            if (resetChosenFiles)
            {
                chosenFiles.clear();
                resetChosenFiles = false;
            }

            chosenFiles.add (f);
            newFilenames.add (f.getRelativePathFrom (getRoot()));
        }
    }

    if (newFilenames.size() > 0)
        filenameBox.setText (newFilenames.joinIntoString (", "), false);

    sendListenerChangeMessage();
}

z0 FileBrowserComponent::fileClicked (const File& f, const MouseEvent& e)
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, [&] (FileBrowserListener& l) { l.fileClicked (f, e); });
}

z0 FileBrowserComponent::fileDoubleClicked (const File& f)
{
    if (f.isDirectory())
    {
        setRoot (f);

        if ((flags & canSelectDirectories) != 0 && (flags & doNotClearFileNameOnRootChange) == 0)
            filenameBox.setText ({});
    }
    else
    {
        Component::BailOutChecker checker (this);
        listeners.callChecked (checker, [&] (FileBrowserListener& l) { l.fileDoubleClicked (f); });
    }
}

z0 FileBrowserComponent::browserRootChanged (const File&) {}

b8 FileBrowserComponent::keyPressed ([[maybe_unused]] const KeyPress& key)
{
   #if DRX_LINUX || DRX_BSD || DRX_WINDOWS
    if (key.getModifiers().isCommandDown()
         && (key.getKeyCode() == 'H' || key.getKeyCode() == 'h'))
    {
        fileList->setIgnoresHiddenFiles (! fileList->ignoresHiddenFiles());
        fileList->refresh();
        return true;
    }
   #endif

    return false;
}

//==============================================================================
z0 FileBrowserComponent::changeFilename()
{
    if (filenameBox.getText().containsChar (File::getSeparatorChar()))
    {
        auto f = currentRoot.getChildFile (filenameBox.getText());

        if (f.isDirectory())
        {
            setRoot (f);
            chosenFiles.clear();

            if ((flags & doNotClearFileNameOnRootChange) == 0)
                filenameBox.setText ({});
        }
        else
        {
            setRoot (f.getParentDirectory());
            chosenFiles.clear();
            chosenFiles.add (f);
            filenameBox.setText (f.getFileName());
        }
    }
    else
    {
        fileDoubleClicked (getSelectedFile (0));
    }
}

//==============================================================================
z0 FileBrowserComponent::updateSelectedPath()
{
    auto newText = currentPathBox.getText().trim().unquoted();

    if (newText.isNotEmpty())
    {
        auto index = currentPathBox.getSelectedId() - 1;

        StringArray rootNames, rootPaths;
        getRoots (rootNames, rootPaths);

        if (rootPaths[index].isNotEmpty())
        {
            setRoot (File (rootPaths[index]));
        }
        else
        {
            File f (newText);

            for (;;)
            {
                if (f.isDirectory())
                {
                    setRoot (f);
                    break;
                }

                if (f.getParentDirectory() == f)
                    break;

                f = f.getParentDirectory();
            }
        }
    }
}

z0 FileBrowserComponent::getDefaultRoots (StringArray& rootNames, StringArray& rootPaths)
{
   #if DRX_WINDOWS
    Array<File> roots;
    File::findFileSystemRoots (roots);
    rootPaths.clear();

    for (i32 i = 0; i < roots.size(); ++i)
    {
        const File& drive = roots.getReference (i);

        Txt name (drive.getFullPathName());
        rootPaths.add (name);

        if (drive.isOnHardDisk())
        {
            Txt volume (drive.getVolumeLabel());

            if (volume.isEmpty())
                volume = TRANS ("Hard Drive");

            name << " [" << volume << ']';
        }
        else if (drive.isOnCDRomDrive())
        {
            name << " [" << TRANS ("CD/DVD drive") << ']';
        }

        rootNames.add (name);
    }

    rootPaths.add ({});
    rootNames.add ({});

    rootPaths.add (File::getSpecialLocation (File::userDocumentsDirectory).getFullPathName());
    rootNames.add (TRANS ("Documents"));
    rootPaths.add (File::getSpecialLocation (File::userMusicDirectory).getFullPathName());
    rootNames.add (TRANS ("Music"));
    rootPaths.add (File::getSpecialLocation (File::userPicturesDirectory).getFullPathName());
    rootNames.add (TRANS ("Pictures"));
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add (TRANS ("Desktop"));

   #elif DRX_MAC
    rootPaths.add (File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    rootNames.add (TRANS ("Home folder"));
    rootPaths.add (File::getSpecialLocation (File::userDocumentsDirectory).getFullPathName());
    rootNames.add (TRANS ("Documents"));
    rootPaths.add (File::getSpecialLocation (File::userMusicDirectory).getFullPathName());
    rootNames.add (TRANS ("Music"));
    rootPaths.add (File::getSpecialLocation (File::userPicturesDirectory).getFullPathName());
    rootNames.add (TRANS ("Pictures"));
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add (TRANS ("Desktop"));

    rootPaths.add ({});
    rootNames.add ({});

    for (auto& volume : File ("/Volumes").findChildFiles (File::findDirectories, false))
    {
        if (volume.isDirectory() && ! volume.getFileName().startsWithChar ('.'))
        {
            rootPaths.add (volume.getFullPathName());
            rootNames.add (volume.getFileName());
        }
    }

   #else
    rootPaths.add ("/");
    rootNames.add ("/");
    rootPaths.add (File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    rootNames.add (TRANS ("Home folder"));
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add (TRANS ("Desktop"));
   #endif
}

z0 FileBrowserComponent::getRoots (StringArray& rootNames, StringArray& rootPaths)
{
    getDefaultRoots (rootNames, rootPaths);
}

z0 FileBrowserComponent::timerCallback()
{
    const auto isProcessActive = detail::WindowingHelpers::isForegroundOrEmbeddedProcess (this);

    if (wasProcessActive != isProcessActive)
    {
        wasProcessActive = isProcessActive;

        if (isProcessActive && fileList != nullptr)
            refresh();
    }
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> FileBrowserComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
