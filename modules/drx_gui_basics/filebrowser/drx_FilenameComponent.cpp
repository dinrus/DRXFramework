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

FilenameComponent::FilenameComponent (const Txt& name,
                                      const File& currentFile,
                                      b8 canEditFilename,
                                      b8 isDirectory,
                                      b8 isForSaving,
                                      const Txt& fileBrowserWildcard,
                                      const Txt& suffix,
                                      const Txt& textWhenNothingSelected)
    : Component (name),
      isDir (isDirectory),
      isSaving (isForSaving),
      wildcard (fileBrowserWildcard),
      enforcedSuffix (suffix)
{
    addAndMakeVisible (filenameBox);
    filenameBox.setEditableText (canEditFilename);
    filenameBox.setTextWhenNothingSelected (textWhenNothingSelected);
    filenameBox.setTextWhenNoChoicesAvailable (TRANS ("(no recently selected files)"));
    filenameBox.onChange = [this] { setCurrentFile (getCurrentFile(), true); };

    setBrowseButtonText ("...");

    setCurrentFile (currentFile, true, dontSendNotification);
}

FilenameComponent::~FilenameComponent()
{
}

//==============================================================================
z0 FilenameComponent::paintOverChildren (Graphics& g)
{
    if (isFileDragOver)
    {
        g.setColor (Colors::red.withAlpha (0.2f));
        g.drawRect (getLocalBounds(), 3);
    }
}

z0 FilenameComponent::resized()
{
    getLookAndFeel().layoutFilenameComponent (*this, &filenameBox, browseButton.get());
}

std::unique_ptr<ComponentTraverser> FilenameComponent::createKeyboardFocusTraverser()
{
    // This prevents the sub-components from grabbing focus if the
    // FilenameComponent has been set to refuse focus.
    return getWantsKeyboardFocus() ? Component::createKeyboardFocusTraverser() : nullptr;
}

z0 FilenameComponent::setBrowseButtonText (const Txt& newBrowseButtonText)
{
    browseButtonText = newBrowseButtonText;
    lookAndFeelChanged();
}

z0 FilenameComponent::lookAndFeelChanged()
{
    browseButton.reset();
    browseButton.reset (getLookAndFeel().createFilenameComponentBrowseButton (browseButtonText));
    addAndMakeVisible (browseButton.get());
    browseButton->setConnectedEdges (Button::ConnectedOnLeft);
    browseButton->onClick = [this] { showChooser(); };
    resized();
}

z0 FilenameComponent::setTooltip (const Txt& newTooltip)
{
    SettableTooltipClient::setTooltip (newTooltip);
    filenameBox.setTooltip (newTooltip);
}

z0 FilenameComponent::setDefaultBrowseTarget (const File& newDefaultDirectory)
{
    defaultBrowseFile = newDefaultDirectory;
}

File FilenameComponent::getLocationToBrowse()
{
    if (lastFilename.isEmpty() && defaultBrowseFile != File())
        return defaultBrowseFile;

    return getCurrentFile();
}

z0 FilenameComponent::showChooser()
{
    chooser = std::make_unique<FileChooser> (isDir ? TRANS ("Choose a new directory")
                                                   : TRANS ("Choose a new file"),
                                             getLocationToBrowse(),
                                             wildcard);

    auto chooserFlags = isDir ? FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories
                              : FileBrowserComponent::canSelectFiles | (isSaving ? FileBrowserComponent::saveMode
                                                                                 : FileBrowserComponent::openMode);

    chooser->launchAsync (chooserFlags, [this] (const FileChooser&)
    {
        if (chooser->getResult() == File{})
            return;

        setCurrentFile (chooser->getResult(), true);
    });
}

b8 FilenameComponent::isInterestedInFileDrag (const StringArray&)
{
    return true;
}

z0 FilenameComponent::filesDropped (const StringArray& filenames, i32, i32)
{
    isFileDragOver = false;
    repaint();

    const File f (filenames[0]);

    if (f.exists() && (f.isDirectory() == isDir))
        setCurrentFile (f, true);
}

z0 FilenameComponent::fileDragEnter (const StringArray&, i32, i32)
{
    isFileDragOver = true;
    repaint();
}

z0 FilenameComponent::fileDragExit (const StringArray&)
{
    isFileDragOver = false;
    repaint();
}

//==============================================================================
Txt FilenameComponent::getCurrentFileText() const
{
    return filenameBox.getText();
}

File FilenameComponent::getCurrentFile() const
{
    auto f = File::getCurrentWorkingDirectory().getChildFile (getCurrentFileText());

    if (enforcedSuffix.isNotEmpty())
        f = f.withFileExtension (enforcedSuffix);

    return f;
}

z0 FilenameComponent::setCurrentFile (File newFile,
                                        const b8 addToRecentlyUsedList,
                                        NotificationType notification)
{
    if (enforcedSuffix.isNotEmpty())
        newFile = newFile.withFileExtension (enforcedSuffix);

    if (newFile.getFullPathName() != lastFilename)
    {
        lastFilename = newFile.getFullPathName();

        if (addToRecentlyUsedList)
            addRecentlyUsedFile (newFile);

        filenameBox.setText (lastFilename, dontSendNotification);

        if (notification != dontSendNotification)
        {
            triggerAsyncUpdate();

            if (notification == sendNotificationSync)
                handleUpdateNowIfNeeded();
        }
    }
}

z0 FilenameComponent::setFilenameIsEditable (const b8 shouldBeEditable)
{
    filenameBox.setEditableText (shouldBeEditable);
}

StringArray FilenameComponent::getRecentlyUsedFilenames() const
{
    StringArray names;

    for (i32 i = 0; i < filenameBox.getNumItems(); ++i)
        names.add (filenameBox.getItemText (i));

    return names;
}

z0 FilenameComponent::setRecentlyUsedFilenames (const StringArray& filenames)
{
    if (filenames != getRecentlyUsedFilenames())
    {
        filenameBox.clear();

        for (i32 i = 0; i < jmin (filenames.size(), maxRecentFiles); ++i)
            filenameBox.addItem (filenames[i], i + 1);
    }
}

z0 FilenameComponent::setMaxNumberOfRecentFiles (i32k newMaximum)
{
    maxRecentFiles = jmax (1, newMaximum);

    setRecentlyUsedFilenames (getRecentlyUsedFilenames());
}

z0 FilenameComponent::addRecentlyUsedFile (const File& file)
{
    auto files = getRecentlyUsedFilenames();

    if (file.getFullPathName().isNotEmpty())
    {
        files.removeString (file.getFullPathName(), true);
        files.insert (0, file.getFullPathName());

        setRecentlyUsedFilenames (files);
    }
}

//==============================================================================
z0 FilenameComponent::addListener (FilenameComponentListener* const listener)
{
    listeners.add (listener);
}

z0 FilenameComponent::removeListener (FilenameComponentListener* const listener)
{
    listeners.remove (listener);
}

z0 FilenameComponent::handleAsyncUpdate()
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, [this] (FilenameComponentListener& l) { l.filenameComponentChanged (this); });
}

} // namespace drx
