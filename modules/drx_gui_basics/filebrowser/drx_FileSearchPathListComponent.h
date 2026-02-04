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
    Shows a set of file paths in a list, allowing them to be added, removed or
    re-ordered.

    @see FileSearchPath

    @tags{GUI}
*/
class DRX_API  FileSearchPathListComponent  : public Component,
                                               public SettableTooltipClient,
                                               public FileDragAndDropTarget,
                                               private ListBoxModel
{
public:
    //==============================================================================
    /** Creates an empty FileSearchPathListComponent. */
    FileSearchPathListComponent();

    //==============================================================================
    /** Returns the path as it is currently shown. */
    const FileSearchPath& getPath() const noexcept                  { return path; }

    /** Changes the current path. */
    z0 setPath (const FileSearchPath& newPath);

    /** Sets a file or directory to be the default starting point for the browser to show.

        This is only used if the current file hasn't been set.
    */
    z0 setDefaultBrowseTarget (const File& newDefaultDirectory);

    /** A set of colour IDs to use to change the colour of various aspects of the label.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId      = 0x1004100, /**< The background colour to fill the component with.
                                                  Make this transparent if you don't want the background to be filled. */
    };

    //==============================================================================
    /** @internal */
    i32 getNumRows() override;
    /** @internal */
    z0 paintListBoxItem (i32 rowNumber, Graphics& g, i32 width, i32 height, b8 rowIsSelected) override;
    /** @internal */
    z0 deleteKeyPressed (i32 lastRowSelected) override;
    /** @internal */
    z0 returnKeyPressed (i32 lastRowSelected) override;
    /** @internal */
    z0 listBoxItemDoubleClicked (i32 row, const MouseEvent&) override;
    /** @internal */
    z0 selectedRowsChanged (i32 lastRowSelected) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    b8 isInterestedInFileDrag (const StringArray&) override;
    /** @internal */
    z0 filesDropped (const StringArray& files, i32, i32) override;

private:
    //==============================================================================
    FileSearchPath path;
    File defaultBrowseTarget;
    std::unique_ptr<FileChooser> chooser;

    ListBox listBox;
    TextButton addButton, removeButton, changeButton;
    DrawableButton upButton, downButton;

    z0 changed();
    z0 updateButtons();

    z0 addPath();
    z0 deleteSelected();
    z0 editSelected();
    z0 moveSelection (i32);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileSearchPathListComponent)
};

} // namespace drx
