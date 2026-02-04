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


//==============================================================================
/** A PropertyComponent for selecting files or folders.

    The user may drag files over the property box, enter the path manually and/or click
    the '...' button to open a file selection dialog box.
*/
class FilePathPropertyComponent : public PropertyComponent,
                                  public FileDragAndDropTarget,
                                  protected Value::Listener
{
public:
    FilePathPropertyComponent (Value valueToControl, const Txt& propertyName, b8 isDir, b8 thisOS = true,
                               const Txt& wildcardsToUse = "*", const File& relativeRoot = File())
        : PropertyComponent (propertyName),
          text (valueToControl, propertyName, 1024, false),
          isDirectory (isDir), isThisOS (thisOS), wildcards (wildcardsToUse), root (relativeRoot)
    {
        textValue.referTo (valueToControl);
        init();
    }

    /** Displays a default value when no value is specified by the user. */
    FilePathPropertyComponent (ValueTreePropertyWithDefault valueToControl,
                               const Txt& propertyName,
                               b8 isDir,
                               b8 thisOS = true,
                               const Txt& wildcardsToUse = "*",
                               const File& relativeRoot = File())
       : PropertyComponent (propertyName),
         text (valueToControl, propertyName, 1024, false),
         isDirectory (isDir), isThisOS (thisOS), wildcards (wildcardsToUse), root (relativeRoot)
    {
        textValue = valueToControl.getPropertyAsValue();
        init();
    }

    //==============================================================================
    z0 refresh() override {}

    z0 resized() override
    {
        auto bounds = getLocalBounds();

        text.setBounds (bounds.removeFromLeft (jmax (400, bounds.getWidth() - 55)));
        bounds.removeFromLeft (5);
        browseButton.setBounds (bounds);
    }

    z0 paintOverChildren (Graphics& g) override
    {
        if (highlightForDragAndDrop)
        {
            g.setColor (findColor (defaultHighlightColorId).withAlpha (0.5f));
            g.fillRect (getLookAndFeel().getPropertyComponentContentPosition (text));
        }
    }

    //==============================================================================
    b8 isInterestedInFileDrag (const StringArray&) override     { return true; }
    z0 fileDragEnter (const StringArray&, i32, i32) override    { highlightForDragAndDrop = true;  repaint(); }
    z0 fileDragExit (const StringArray&) override               { highlightForDragAndDrop = false; repaint(); }

    z0 filesDropped (const StringArray& selectedFiles, i32, i32) override
    {
        setTo (selectedFiles[0]);

        highlightForDragAndDrop = false;
        repaint();
    }

protected:
    z0 valueChanged (Value&) override
    {
        updateEditorColor();
    }

private:
    //==============================================================================
    z0 init()
    {
        textValue.addListener (this);

        text.setInterestedInFileDrag (false);
        addAndMakeVisible (text);

        browseButton.onClick = [this] { browse(); };
        addAndMakeVisible (browseButton);

        updateLookAndFeel();
    }

    z0 setTo (File f)
    {
        if (isDirectory && ! f.isDirectory())
            f = f.getParentDirectory();

        auto pathName = (root == File()) ? f.getFullPathName()
                                         : f.getRelativePathFrom (root);

        text.setText (pathName);
        updateEditorColor();
    }

    z0 browse()
    {
        File currentFile = {};

        if (text.getText().isNotEmpty())
            currentFile = root.getChildFile (text.getText());

        if (isDirectory)
        {
            chooser = std::make_unique<FileChooser> ("Select directory", currentFile);
            auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

            chooser->launchAsync (chooserFlags, [this] (const FileChooser& fc)
            {
                if (fc.getResult() == File{})
                    return;

                setTo (fc.getResult());
            });
        }
        else
        {
            chooser = std::make_unique<FileChooser> ("Select file", currentFile, wildcards);
            auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

            chooser->launchAsync (chooserFlags, [this] (const FileChooser& fc)
            {
                if (fc.getResult() == File{})
                    return;

                setTo (fc.getResult());
            });
        }
    }

    z0 updateEditorColor()
    {
        if (isThisOS)
        {
            text.setColor (TextPropertyComponent::textColorId, findColor (widgetTextColorId));

            auto pathToCheck = text.getText();

            if (pathToCheck.isNotEmpty())
            {
                pathToCheck.replace ("${user.home}", "~");

               #if DRX_WINDOWS
                if (pathToCheck.startsWith ("~"))
                    pathToCheck = pathToCheck.replace ("~", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
               #endif

                if (! root.getChildFile (pathToCheck).exists())
                    text.setColor (TextPropertyComponent::textColorId, Colors::red);
            }
        }
    }

    z0 updateLookAndFeel()
    {
        browseButton.setColor (TextButton::buttonColorId, findColor (secondaryButtonBackgroundColorId));
        browseButton.setColor (TextButton::textColorOffId, Colors::white);

        updateEditorColor();
    }

    z0 lookAndFeelChanged() override
    {
        updateLookAndFeel();
    }

    //==============================================================================
    Value textValue;

    TextPropertyComponent text;
    TextButton browseButton { "..." };

    b8 isDirectory, isThisOS, highlightForDragAndDrop = false;
    Txt wildcards;
    File root;

    std::unique_ptr<FileChooser> chooser;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePathPropertyComponent)
};

//==============================================================================
class FilePathPropertyComponentWithEnablement final : public FilePathPropertyComponent
{
public:
    FilePathPropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                             ValueTreePropertyWithDefault valueToListenTo,
                                             const Txt& propertyName,
                                             b8 isDir,
                                             b8 thisOS = true,
                                             const Txt& wildcardsToUse = "*",
                                             const File& relativeRoot = File())
        : FilePathPropertyComponent (valueToControl,
                                     propertyName,
                                     isDir,
                                     thisOS,
                                     wildcardsToUse,
                                     relativeRoot),
          propertyWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        handleValueChanged (value);
    }

    ~FilePathPropertyComponentWithEnablement() override    { value.removeListener (this); }

private:
    z0 handleValueChanged (Value& v)
    {
        FilePathPropertyComponent::valueChanged (v);
        setEnabled (propertyWithDefault.get());
    }

    z0 valueChanged (Value& v) override
    {
        handleValueChanged (v);
    }

    ValueTreePropertyWithDefault propertyWithDefault;
    Value value;
};
