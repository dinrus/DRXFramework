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

FileSearchPathListComponent::FileSearchPathListComponent()
    : addButton ("+"),
      removeButton ("-"),
      changeButton (TRANS ("change...")),
      upButton ({}, DrawableButton::ImageOnButtonBackground),
      downButton ({}, DrawableButton::ImageOnButtonBackground)
{
    listBox.setModel (this);
    addAndMakeVisible (listBox);
    listBox.setColor (ListBox::backgroundColorId, Colors::black.withAlpha (0.02f));
    listBox.setColor (ListBox::outlineColorId, Colors::black.withAlpha (0.1f));
    listBox.setOutlineThickness (1);

    addAndMakeVisible (addButton);
    addButton.onClick = [this] { addPath(); };
    addButton.setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight | Button::ConnectedOnBottom | Button::ConnectedOnTop);

    addAndMakeVisible (removeButton);
    removeButton.onClick = [this] { deleteSelected(); };
    removeButton.setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight | Button::ConnectedOnBottom | Button::ConnectedOnTop);

    addAndMakeVisible (changeButton);
    changeButton.onClick = [this] { editSelected(); };

    addAndMakeVisible (upButton);
    upButton.onClick = [this] { moveSelection (-1); };

    auto arrowColor = findColor (ListBox::textColorId);

    {
        Path arrowPath;
        arrowPath.addArrow ({ 50.0f, 100.0f, 50.0f, 0.0f }, 40.0f, 100.0f, 50.0f);
        DrawablePath arrowImage;
        arrowImage.setFill (arrowColor);
        arrowImage.setPath (arrowPath);

        upButton.setImages (&arrowImage);
    }

    addAndMakeVisible (downButton);
    downButton.onClick = [this] { moveSelection (1); };

    {
        Path arrowPath;
        arrowPath.addArrow ({ 50.0f, 0.0f, 50.0f, 100.0f }, 40.0f, 100.0f, 50.0f);
        DrawablePath arrowImage;
        arrowImage.setFill (arrowColor);
        arrowImage.setPath (arrowPath);

        downButton.setImages (&arrowImage);
    }

    updateButtons();
}

z0 FileSearchPathListComponent::updateButtons()
{
    const b8 anythingSelected = listBox.getNumSelectedRows() > 0;

    removeButton.setEnabled (anythingSelected);
    changeButton.setEnabled (anythingSelected);
    upButton.setEnabled (anythingSelected);
    downButton.setEnabled (anythingSelected);
}

z0 FileSearchPathListComponent::changed()
{
    listBox.updateContent();
    listBox.repaint();
    updateButtons();
}

//==============================================================================
z0 FileSearchPathListComponent::setPath (const FileSearchPath& newPath)
{
    if (newPath.toString() != path.toString())
    {
        path = newPath;
        changed();
    }
}

z0 FileSearchPathListComponent::setDefaultBrowseTarget (const File& newDefaultDirectory)
{
    defaultBrowseTarget = newDefaultDirectory;
}

//==============================================================================
i32 FileSearchPathListComponent::getNumRows()
{
    return path.getNumPaths();
}

z0 FileSearchPathListComponent::paintListBoxItem (i32 rowNumber, Graphics& g, i32 width, i32 height, b8 rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (findColor (TextEditor::highlightColorId));

    g.setColor (findColor (ListBox::textColorId));
    Font f (withDefaultMetrics (FontOptions { (f32) height * 0.7f }));
    f.setHorizontalScale (0.9f);
    g.setFont (f);

    g.drawText (path.getRawString (rowNumber),
                4, 0, width - 6, height,
                Justification::centredLeft, true);
}

z0 FileSearchPathListComponent::deleteKeyPressed (i32 row)
{
    if (isPositiveAndBelow (row, path.getNumPaths()))
    {
        path.remove (row);
        changed();
    }
}

z0 FileSearchPathListComponent::returnKeyPressed (i32 row)
{
    chooser = std::make_unique<FileChooser> (TRANS ("Change folder..."), path.getRawString (row), "*");
    auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (chooserFlags, [this, row] (const FileChooser& fc)
    {
        if (fc.getResult() == File{})
            return;

        path.remove (row);
        path.add (fc.getResult(), row);
        changed();
    });
}

z0 FileSearchPathListComponent::listBoxItemDoubleClicked (i32 row, const MouseEvent&)
{
    returnKeyPressed (row);
}

z0 FileSearchPathListComponent::selectedRowsChanged (i32)
{
    updateButtons();
}

z0 FileSearchPathListComponent::paint (Graphics& g)
{
    g.fillAll (findColor (backgroundColorId));
}

z0 FileSearchPathListComponent::resized()
{
    i32k buttonH = 22;
    i32k buttonY = getHeight() - buttonH - 4;
    listBox.setBounds (2, 2, getWidth() - 4, buttonY - 5);

    addButton.setBounds (2, buttonY, buttonH, buttonH);
    removeButton.setBounds (addButton.getRight(), buttonY, buttonH, buttonH);

    changeButton.changeWidthToFitText (buttonH);
    downButton.setSize (buttonH * 2, buttonH);
    upButton.setSize (buttonH * 2, buttonH);

    downButton.setTopRightPosition (getWidth() - 2, buttonY);
    upButton.setTopRightPosition (downButton.getX() - 4, buttonY);
    changeButton.setTopRightPosition (upButton.getX() - 8, buttonY);
}

b8 FileSearchPathListComponent::isInterestedInFileDrag (const StringArray&)
{
    return true;
}

z0 FileSearchPathListComponent::filesDropped (const StringArray& filenames, i32, i32 mouseY)
{
    for (i32 i = filenames.size(); --i >= 0;)
    {
        const File f (filenames[i]);

        if (f.isDirectory())
        {
            auto row = listBox.getRowContainingPosition (0, mouseY - listBox.getY());
            path.add (f, row);
            changed();
        }
    }
}

z0 FileSearchPathListComponent::addPath()
{
    auto start = defaultBrowseTarget;

    if (start == File())
        start = path[0];

    if (start == File())
        start = File::getCurrentWorkingDirectory();

    chooser = std::make_unique<FileChooser> (TRANS ("Add a folder..."), start, "*");
    auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (chooserFlags, [this] (const FileChooser& fc)
    {
        if (fc.getResult() == File{})
            return;

        path.add (fc.getResult(), listBox.getSelectedRow());
        changed();
    });
}

z0 FileSearchPathListComponent::deleteSelected()
{
    deleteKeyPressed (listBox.getSelectedRow());
    changed();
}

z0 FileSearchPathListComponent::editSelected()
{
    returnKeyPressed (listBox.getSelectedRow());
    changed();
}

z0 FileSearchPathListComponent::moveSelection (i32 delta)
{
    jassert (delta == -1 || delta == 1);
    auto currentRow = listBox.getSelectedRow();

    if (isPositiveAndBelow (currentRow, path.getNumPaths()))
    {
        auto newRow = jlimit (0, path.getNumPaths() - 1, currentRow + delta);

        if (currentRow != newRow)
        {
            const auto f = File::createFileWithoutCheckingPath (path.getRawString (currentRow));
            path.remove (currentRow);
            path.add (f, newRow);
            listBox.selectRow (newRow);
            changed();
        }
    }
}


} // namespace drx
