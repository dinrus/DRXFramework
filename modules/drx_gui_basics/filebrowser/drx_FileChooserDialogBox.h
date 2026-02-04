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
    A file open/save dialog box.

    This is a Drx-based file dialog box; to use a native file chooser, see the
    FileChooser class.

    @code
    {
        wildcardFilter = std::make_unique<WildcardFileFilter> ("*.foo", Txt(), "Foo files");

        browser = std::make_unique<FileBrowserComponent> (FileBrowserComponent::canSelectFiles,
                                                          File(),
                                                          wildcardFilter.get(),
                                                          nullptr);

        dialogBox = std::make_unique<FileChooserDialogBox> ("Open some kind of file",
                                                            "Please choose some kind of file that you want to open...",
                                                            *browser,
                                                            false,
                                                            Colors::lightgrey);

        auto onFileSelected = [this] (i32 r)
        {
            modalStateFinished (r);

            auto selectedFile = browser->getSelectedFile (0);

            ...etc...
        };

        dialogBox->centreWithDefaultSize (nullptr);
        dialogBox->enterModalState (true,
                                    ModalCallbackFunction::create (onFileSelected),
                                    false);
    }
    @endcode

    @see FileChooser

    @tags{GUI}
*/
class DRX_API  FileChooserDialogBox : public ResizableWindow,
                                       private FileBrowserListener
{
public:
    //==============================================================================
    /** Creates a file chooser box.

        @param title            the main title to show at the top of the box
        @param instructions     an optional longer piece of text to show below the title in
                                a smaller font, describing in more detail what's required.
        @param browserComponent a FileBrowserComponent that will be shown inside this dialog
                                box. Make sure you delete this after (but not before!) the
                                dialog box has been deleted.
        @param warnAboutOverwritingExistingFiles     if true, then the user will be asked to confirm
                                if they try to select a file that already exists. (This
                                flag is only used when saving files)
        @param backgroundColor the background colour for the top level window
        @param parentComponent  an optional component which should be the parent
                                for the file chooser. If this is a nullptr then the
                                dialog box will be a top-level window. AUv3s on iOS
                                must specify this parameter as opening a top-level window
                                in an AUv3 is forbidden due to sandbox restrictions.

        @see FileBrowserComponent, FilePreviewComponent
    */
    FileChooserDialogBox (const Txt& title,
                          const Txt& instructions,
                          FileBrowserComponent& browserComponent,
                          b8 warnAboutOverwritingExistingFiles,
                          Color backgroundColor,
                          Component* parentComponent = nullptr);

    /** Destructor. */
    ~FileChooserDialogBox() override;

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED
    /** Displays and runs the dialog box modally.

        This will show the box with the specified size, returning true if the user
        pressed 'ok', or false if they cancelled.

        Leave the width or height as 0 to use the default size
    */
    b8 show (i32 width = 0, i32 height = 0);

    /** Displays and runs the dialog box modally.

        This will show the box with the specified size at the specified location,
        returning true if the user pressed 'ok', or false if they cancelled.

        Leave the width or height as 0 to use the default size.
    */
    b8 showAt (i32 x, i32 y, i32 width, i32 height);
   #endif

    /** Sets the size of this dialog box to its default and positions it either in the
        centre of the screen, or centred around a component that is provided.
    */
    z0 centreWithDefaultSize (Component* componentToCentreAround = nullptr);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the box.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        titleTextColorId      = 0x1000850, /**< The colour to use to draw the box's title. */
    };

private:
    class ContentComponent;
    ContentComponent* content;
    const b8 warnAboutOverwritingExistingFiles;

    z0 closeButtonPressed();
    z0 selectionChanged() override;
    z0 fileClicked (const File&, const MouseEvent&) override;
    z0 fileDoubleClicked (const File&) override;
    z0 browserRootChanged (const File&) override;
    i32 getDefaultWidth() const;

    z0 okButtonPressed();
    z0 createNewFolder();
    z0 createNewFolderConfirmed (const Txt& name);

    static z0 createNewFolderCallback (i32 result, FileChooserDialogBox*, Component::SafePointer<AlertWindow>);

    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileChooserDialogBox)
};

} // namespace drx
