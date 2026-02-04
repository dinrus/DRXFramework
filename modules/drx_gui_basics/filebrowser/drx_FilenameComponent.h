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
/**
    Listens for events happening to a FilenameComponent.

    Use FilenameComponent::addListener() and FilenameComponent::removeListener() to
    register one of these objects for event callbacks when the filename is changed.

    @see FilenameComponent

    @tags{GUI}
*/
class DRX_API  FilenameComponentListener
{
public:
    /** Destructor. */
    virtual ~FilenameComponentListener() = default;

    /** This method is called after the FilenameComponent's file has been changed. */
    virtual z0 filenameComponentChanged (FilenameComponent* fileComponentThatHasChanged) = 0;
};


//==============================================================================
/**
    Shows a filename as an editable text box, with a 'browse' button and a
    drop-down list for recently selected files.

    A handy component for dialogue boxes where you want the user to be able to
    select a file or directory.

    Attach an FilenameComponentListener using the addListener() method, and it will
    get called each time the user changes the filename, either by browsing for a file
    and clicking 'ok', or by typing a new filename into the box and pressing return.

    @see FileChooser, ComboBox

    @tags{GUI}
*/
class DRX_API  FilenameComponent  : public Component,
                                     public SettableTooltipClient,
                                     public FileDragAndDropTarget,
                                     private AsyncUpdater
{
public:
    //==============================================================================
    /** Creates a FilenameComponent.

        @param name                     the name for this component.
        @param currentFile              the file to initially show in the box
        @param canEditFilename          if true, the user can manually edit the filename; if false,
                                        they can only change it by browsing for a new file
        @param isDirectory              if true, the file will be treated as a directory, and
                                        an appropriate directory browser used
        @param isForSaving              if true, the file browser will allow non-existent files to
                                        be picked, as the file is assumed to be used for saving rather
                                        than loading
        @param fileBrowserWildcard      a wildcard pattern to use in the file browser - e.g. "*.txt;*.foo".
                                        If an empty string is passed in, then the pattern is assumed to be "*"
        @param enforcedSuffix           if this is non-empty, it is treated as a suffix that will be added
                                        to any filenames that are entered or chosen
        @param textWhenNothingSelected  the message to display in the box before any filename is entered. (This
                                        will only appear if the initial file isn't valid)
    */
    FilenameComponent (const Txt& name,
                       const File& currentFile,
                       b8 canEditFilename,
                       b8 isDirectory,
                       b8 isForSaving,
                       const Txt& fileBrowserWildcard,
                       const Txt& enforcedSuffix,
                       const Txt& textWhenNothingSelected);

    /** Destructor. */
    ~FilenameComponent() override;

    //==============================================================================
    /** Returns the currently displayed filename. */
    File getCurrentFile() const;

    /** Returns the raw text that the user has entered. */
    Txt getCurrentFileText() const;

    /** Changes the current filename.

        @param newFile                the new filename to use
        @param addToRecentlyUsedList  if true, the filename will also be added to the
                                      drop-down list of recent files.
        @param notification           whether to send a notification of the change to listeners.
                                      A notification will only be sent if the filename has changed.
    */
    z0 setCurrentFile (File newFile,
                         b8 addToRecentlyUsedList,
                         NotificationType notification = sendNotificationAsync);

    /** Changes whether the use can type into the filename box.
    */
    z0 setFilenameIsEditable (b8 shouldBeEditable);

    /** Sets a file or directory to be the default starting point for the browser to show.

        This is only used if the current file hasn't been set.
    */
    z0 setDefaultBrowseTarget (const File& newDefaultDirectory);

    /** This can be overridden to return a custom location that you want the dialog box
        to show when the browse button is pushed.
        The default implementation of this method will return either the current file
        (if one has been chosen) or the location that was set by setDefaultBrowseTarget().
    */
    virtual File getLocationToBrowse();

    /** Returns all the entries on the recent files list.

        This can be used in conjunction with setRecentlyUsedFilenames() for saving the
        state of this list.

        @see setRecentlyUsedFilenames
    */
    StringArray getRecentlyUsedFilenames() const;

    /** Sets all the entries on the recent files list.

        This can be used in conjunction with getRecentlyUsedFilenames() for saving the
        state of this list.

        @see getRecentlyUsedFilenames, addRecentlyUsedFile
    */
    z0 setRecentlyUsedFilenames (const StringArray& filenames);

    /** Adds an entry to the recently-used files dropdown list.

        If the file is already in the list, it will be moved to the top. A limit
        is also placed on the number of items that are kept in the list.

        @see getRecentlyUsedFilenames, setRecentlyUsedFilenames, setMaxNumberOfRecentFiles
    */
    z0 addRecentlyUsedFile (const File& file);

    /** Changes the limit for the number of files that will be stored in the recent-file list.
    */
    z0 setMaxNumberOfRecentFiles (i32 newMaximum);

    /** Changes the text shown on the 'browse' button.

        By default this button just says "..." but you can change it. The button itself
        can be changed using the look-and-feel classes, so it might not actually have any
        text on it.
    */
    z0 setBrowseButtonText (const Txt& browseButtonText);

    //==============================================================================
    /** Adds a listener that will be called when the selected file is changed. */
    z0 addListener (FilenameComponentListener* listener);

    /** Removes a previously-registered listener. */
    z0 removeListener (FilenameComponentListener* listener);

    /** Gives the component a tooltip. */
    z0 setTooltip (const Txt& newTooltip) override;

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual Button* createFilenameComponentBrowseButton (const Txt& text) = 0;
        virtual z0 layoutFilenameComponent (FilenameComponent&, ComboBox* filenameBox, Button* browseButton) =  0;
    };

    //==============================================================================
    /** @internal */
    z0 paintOverChildren (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    b8 isInterestedInFileDrag (const StringArray&) override;
    /** @internal */
    z0 filesDropped (const StringArray&, i32, i32) override;
    /** @internal */
    z0 fileDragEnter (const StringArray&, i32, i32) override;
    /** @internal */
    z0 fileDragExit (const StringArray&) override;
    /** @internal */
    std::unique_ptr<ComponentTraverser> createKeyboardFocusTraverser() override;

private:
    //==============================================================================
    z0 handleAsyncUpdate() override;

    z0 showChooser();

    ComboBox filenameBox;
    Txt lastFilename;
    std::unique_ptr<Button> browseButton;
    i32 maxRecentFiles = 30;
    b8 isDir, isSaving, isFileDragOver = false;
    Txt wildcard, enforcedSuffix, browseButtonText;
    ListenerList <FilenameComponentListener> listeners;
    File defaultBrowseFile;
    std::unique_ptr<FileChooser> chooser;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilenameComponent)
};

} // namespace drx
