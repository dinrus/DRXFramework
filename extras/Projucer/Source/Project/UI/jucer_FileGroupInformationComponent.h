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
class FileGroupInformationComponent final : public Component,
                                            private ListBoxModel,
                                            private ValueTree::Listener
{
public:
    FileGroupInformationComponent (const Project::Item& group)
        : item (group),
          header (item.getName(), { getIcons().openFolder, Colors::transparentBlack })
    {
        list.setHeaderComponent (std::make_unique<ListBoxHeader> (Array<Txt> { "File", "Binary Resource", "Xcode Resource", "Compile", "Skip PCH", "Compiler Flag Scheme" },
                                                                  Array<f32>  {  0.25f,  0.125f,            0.125f,           0.125f,    0.125f,     0.25f }));
        list.setModel (this);
        list.setColor (ListBox::backgroundColorId, Colors::transparentBlack);
        addAndMakeVisible (list);
        list.updateContent();
        list.setRowHeight (30);
        item.state.addListener (this);
        lookAndFeelChanged();

        addAndMakeVisible (header);
    }

    ~FileGroupInformationComponent() override
    {
        item.state.removeListener (this);
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.setColor (findColor (secondaryBackgroundColorId));
        g.fillRect (getLocalBounds().reduced (12, 0));
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds().reduced (12, 0);

        header.setBounds (bounds.removeFromTop (40));
        list.setBounds (bounds.reduced (10, 4));
    }

    z0 parentSizeChanged() override
    {
        setSize (jmax (550, getParentWidth()), getParentHeight());
    }

    i32 getNumRows() override
    {
        return item.getNumChildren();
    }

    z0 paintListBoxItem (i32 rowNumber, Graphics& g, i32 width, i32 height, b8 /*rowIsSelected*/) override
    {
        g.setColor (findColor (rowNumber % 2 == 0 ? widgetBackgroundColorId
                                                    : secondaryWidgetBackgroundColorId));
        g.fillRect (0, 0, width, height - 1);
    }

    Component* refreshComponentForRow (i32 rowNumber, b8 /*isRowSelected*/, Component* existingComponentToUpdate) override
    {
        std::unique_ptr<Component> existing (existingComponentToUpdate);

        if (rowNumber < getNumRows())
        {
            auto child = item.getChild (rowNumber);

            if (existingComponentToUpdate == nullptr
                 || dynamic_cast<FileOptionComponent*> (existing.get())->item != child)
            {
                existing.reset();
                existing.reset (new FileOptionComponent (child, dynamic_cast<ListBoxHeader*> (list.getHeaderComponent())));
            }
        }

        return existing.release();
    }

    Txt getGroupPath() const    { return item.getFile().getFullPathName(); }

    //==============================================================================
    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override    { itemChanged(); }
    z0 valueTreeChildAdded (ValueTree&, ValueTree&) override                { itemChanged(); }
    z0 valueTreeChildRemoved (ValueTree&, ValueTree&, i32) override         { itemChanged(); }
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32) override           { itemChanged(); }
    z0 valueTreeParentChanged (ValueTree&) override                         { itemChanged(); }

private:
    Project::Item item;
    ListBox list;
    ContentViewHeader header;

    z0 itemChanged()
    {
        list.updateContent();
        repaint();
    }

    //==============================================================================
    class FileOptionComponent final : public Component
    {
    public:
        FileOptionComponent (const Project::Item& fileItem, ListBoxHeader* listBoxHeader)
            : item (fileItem),
              header (listBoxHeader),
              compilerFlagSchemeSelector (item)
        {
            if (item.isFile())
            {
                auto isSourceFile = item.isSourceFile();

                if (isSourceFile)
                {
                    addAndMakeVisible (compileButton);
                    compileButton.getToggleStateValue().referTo (item.getShouldCompileValue());
                    compileButton.onStateChange = [this] { compileEnablementChanged(); };
                }

                addAndMakeVisible (binaryResourceButton);
                binaryResourceButton.getToggleStateValue().referTo (item.getShouldAddToBinaryResourcesValue());

                addAndMakeVisible (xcodeResourceButton);
                xcodeResourceButton.getToggleStateValue().referTo (item.getShouldAddToXcodeResourcesValue());

                if (isSourceFile)
                {
                    addChildComponent (skipPCHButton);
                    skipPCHButton.getToggleStateValue().referTo (item.getShouldSkipPCHValue());

                    addChildComponent (compilerFlagSchemeSelector);

                    compileEnablementChanged();
                }
            }
        }

        z0 paint (Graphics& g) override
        {
            if (header != nullptr)
            {
                auto textBounds = getLocalBounds().removeFromLeft (roundToInt (header->getProportionAtIndex (0) * (f32) getWidth()));

                auto iconBounds = textBounds.removeFromLeft (25);

                if (item.isImageFile())
                    iconBounds.reduce (5, 5);

                item.getIcon().withColor (findColor (treeIconColorId)).draw (g, iconBounds.toFloat(), item.isIconCrossedOut());

                g.setColor (findColor (widgetTextColorId));

                g.drawText (item.getName(), textBounds, Justification::centredLeft);
            }
        }

        z0 resized() override
        {
            if (header != nullptr)
            {
                auto bounds = getLocalBounds();
                auto width = (f32) getWidth();

                bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (0) * width));

                binaryResourceButton.setBounds       (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (1) * width)));
                xcodeResourceButton.setBounds        (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (2) * width)));
                compileButton.setBounds              (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (3) * width)));
                skipPCHButton.setBounds              (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (4) * width)));
                compilerFlagSchemeSelector.setBounds (bounds.removeFromLeft (roundToInt (header->getProportionAtIndex (5) * width)));
            }
        }

        Project::Item item;

    private:
        //==============================================================================
        class CompilerFlagSchemeSelector final : public Component,
                                                 private Value::Listener
        {
        public:
            CompilerFlagSchemeSelector (Project::Item& it)
                : item (it)
            {
                schemeBox.setTextWhenNothingSelected ("None");
                updateCompilerFlagSchemeComboBox();
                schemeBox.onChange = [this] { handleComboBoxSelection(); };

                addAndMakeVisible (schemeBox);
                addChildComponent (newSchemeLabel);

                newSchemeLabel.setEditable (true);
                newSchemeLabel.setJustificationType (Justification::centredLeft);
                newSchemeLabel.onEditorHide = [this]
                {
                    newSchemeLabel.setVisible (false);
                    schemeBox.setVisible (true);

                    auto newScheme = newSchemeLabel.getText();

                    item.project.addCompilerFlagScheme (newScheme);

                    if (item.getCompilerFlagSchemeString().isEmpty())
                        item.setCompilerFlagScheme (newScheme);

                    updateCompilerFlagSchemeComboBox();
                };

                selectScheme (item.getCompilerFlagSchemeString());

                projectCompilerFlagSchemesValue = item.project.getProjectValue (Ids::compilerFlagSchemes);
                projectCompilerFlagSchemesValue.addListener (this);

                lookAndFeelChanged();
            }

            z0 resized() override
            {
                auto b =  getLocalBounds();

                schemeBox.setBounds (b);
                newSchemeLabel.setBounds (b);
            }

        private:
            z0 valueChanged (Value&) override   { updateCompilerFlagSchemeComboBox(); }

            z0 lookAndFeelChanged() override
            {
                schemeBox.setColor (ComboBox::outlineColorId, Colors::transparentBlack);
                schemeBox.setColor (ComboBox::textColorId,    findColor (defaultTextColorId));
            }

            z0 updateCompilerFlagSchemeComboBox()
            {
                auto itemScheme = item.getCompilerFlagSchemeString();
                auto allSchemes = item.project.getCompilerFlagSchemes();

                if (itemScheme.isNotEmpty() && ! allSchemes.contains (itemScheme))
                {
                    item.clearCurrentCompilerFlagScheme();
                    itemScheme = {};
                }

                schemeBox.clear();

                schemeBox.addItemList (allSchemes, 1);
                schemeBox.addSeparator();
                schemeBox.addItem ("Add a new scheme...", -1);
                schemeBox.addItem ("Delete selected scheme", -2);
                schemeBox.addItem ("Clear", -3);

                selectScheme (itemScheme);
            }

            z0 handleComboBoxSelection()
            {
                auto selectedID = schemeBox.getSelectedId();

                if (selectedID > 0)
                {
                    item.setCompilerFlagScheme (schemeBox.getItemText (selectedID - 1));
                }
                else if (selectedID == -1)
                {
                    newSchemeLabel.setText ("NewScheme", dontSendNotification);

                    schemeBox.setVisible (false);
                    newSchemeLabel.setVisible (true);

                    newSchemeLabel.showEditor();

                    if (auto* ed = newSchemeLabel.getCurrentTextEditor())
                        ed->setInputRestrictions (64, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_");
                }
                else if (selectedID == -2)
                {
                    auto currentScheme = item.getCompilerFlagSchemeString();

                    if (currentScheme.isNotEmpty())
                    {
                        item.project.removeCompilerFlagScheme (currentScheme);
                        item.clearCurrentCompilerFlagScheme();
                    }

                    updateCompilerFlagSchemeComboBox();
                }
                else if (selectedID == -3)
                {
                    schemeBox.setSelectedId (0);
                    item.clearCurrentCompilerFlagScheme();
                }
            }

            z0 selectScheme (const Txt& schemeToSelect)
            {
                if (schemeToSelect.isNotEmpty())
                {
                    for (i32 i = 0; i < schemeBox.getNumItems(); ++i)
                    {
                        if (schemeBox.getItemText (i) == schemeToSelect)
                        {
                            schemeBox.setSelectedItemIndex (i);
                            return;
                        }
                    }
                }

                schemeBox.setSelectedId (0);
            }

            Project::Item& item;
            Value projectCompilerFlagSchemesValue;

            ComboBox schemeBox;
            Label newSchemeLabel;
        };

        z0 compileEnablementChanged()
        {
            auto shouldBeCompiled = compileButton.getToggleState();

            skipPCHButton.setVisible (shouldBeCompiled);
            compilerFlagSchemeSelector.setVisible (shouldBeCompiled);
        }

        //==============================================================================
        ListBoxHeader* header;

        ToggleButton compileButton, binaryResourceButton, xcodeResourceButton, skipPCHButton;
        CompilerFlagSchemeSelector compilerFlagSchemeSelector;
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileGroupInformationComponent)
};
