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

 name:             XMLandJSONDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Reads XML and JSON files.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        XMLandJSONDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class XmlTreeItem final : public TreeViewItem
{
public:
    XmlTreeItem (XmlElement& x)  : xml (x)    {}

    Txt getUniqueName() const override
    {
        if (xml.getTagName().isEmpty())
            return "unknown";

        return xml.getTagName();
    }

    b8 mightContainSubItems() override
    {
        return xml.getFirstChildElement() != nullptr;
    }

    z0 paintItem (Graphics& g, i32 width, i32 height) override
    {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (Colors::blue.withAlpha (0.3f));

        // use a "colour" attribute in the xml tag for this node to set the text colour..
        g.setColor (Color::fromString (xml.getStringAttribute ("colour", "ff000000")));
        g.setFont ((f32) height * 0.7f);

        // draw the xml element's tag name..
        g.drawText (xml.getTagName(),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
    }

    z0 itemOpennessChanged (b8 isNowOpen) override
    {
        if (isNowOpen)
        {
            // if we've not already done so, we'll now add the tree's sub-items. You could
            // also choose to delete the existing ones and refresh them if that's more suitable
            // in your app.
            if (getNumSubItems() == 0)
            {
                // create and add sub-items to this node of the tree, corresponding to
                // each sub-element in the XML..

                for (auto* child : xml.getChildIterator())
                    if (child != nullptr)
                        addSubItem (new XmlTreeItem (*child));
            }
        }
        else
        {
            // in this case, we'll leave any sub-items in the tree when the node gets closed,
            // though you could choose to delete them if that's more appropriate for
            // your application.
        }
    }

private:
    XmlElement& xml;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XmlTreeItem)
};

//==============================================================================
class JsonTreeItem final : public TreeViewItem
{
public:
    JsonTreeItem (Identifier i, var value)
        : identifier (i),
          json (value)
    {}

    Txt getUniqueName() const override
    {
        return identifier.toString() + "_id";
    }

    b8 mightContainSubItems() override
    {
        if (auto* obj = json.getDynamicObject())
            return obj->getProperties().size() > 0;

        return json.isArray();
    }

    z0 paintItem (Graphics& g, i32 width, i32 height) override
    {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (Colors::blue.withAlpha (0.3f));

        g.setColor (Colors::black);
        g.setFont ((f32) height * 0.7f);

        // draw the element's tag name..
        g.drawText (getText(),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
    }

    z0 itemOpennessChanged (b8 isNowOpen) override
    {
        if (isNowOpen)
        {
            // if we've not already done so, we'll now add the tree's sub-items. You could
            // also choose to delete the existing ones and refresh them if that's more suitable
            // in your app.
            if (getNumSubItems() == 0)
            {
                // create and add sub-items to this node of the tree, corresponding to
                // the type of object this var represents

                if (json.isArray())
                {
                    for (i32 i = 0; i < json.size(); ++i)
                    {
                        auto& child = json[i];
                        jassert (! child.isVoid());
                        addSubItem (new JsonTreeItem ({}, child));
                    }
                }
                else if (auto* obj = json.getDynamicObject())
                {
                    auto& props = obj->getProperties();

                    for (i32 i = 0; i < props.size(); ++i)
                    {
                        auto id = props.getName (i);

                        auto child = props[id];
                        jassert (! child.isVoid());

                        addSubItem (new JsonTreeItem (id, child));
                    }
                }
            }
        }
        else
        {
            // in this case, we'll leave any sub-items in the tree when the node gets closed,
            // though you could choose to delete them if that's more appropriate for
            // your application.
        }
    }

private:
    Identifier identifier;
    var json;

    /** Returns the text to display in the tree.
        This is a little more complex for JSON than XML as nodes can be strings, objects or arrays.
     */
    Txt getText() const
    {
        Txt text;

        if (identifier.isValid())
            text << identifier.toString();

        if (! json.isVoid())
        {
            if (text.isNotEmpty() && (! json.isArray()))
                text << ": ";

            if (json.isObject() && (! identifier.isValid()))
                text << "[Array]";
            else if (! json.isArray())
                text << json.toString();
        }

        return text;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JsonTreeItem)
};

//==============================================================================
class XMLandJSONDemo final : public Component,
                             private CodeDocument::Listener
{
public:
    /** The type of database to parse. */
    enum Type
    {
        xml,
        json
    };

    XMLandJSONDemo()
    {
        setOpaque (true);

        addAndMakeVisible (typeBox);
        typeBox.addItem ("XML",  1);
        typeBox.addItem ("JSON", 2);

        typeBox.onChange = [this]
        {
            if (typeBox.getSelectedId() == 1)
                reset (xml);
            else
                reset (json);
        };

        comboBoxLabel.attachToComponent (&typeBox, true);

        addAndMakeVisible (codeDocumentComponent);
        codeDocument.addListener (this);

        resultsTree.setTitle ("Results");
        addAndMakeVisible (resultsTree);
        resultsTree.setColor (TreeView::backgroundColorId, Colors::white);
        resultsTree.setDefaultOpenness (true);

        addAndMakeVisible (errorMessage);
        errorMessage.setReadOnly (true);
        errorMessage.setMultiLine (true);
        errorMessage.setCaretVisible (false);
        errorMessage.setColor (TextEditor::outlineColorId, Colors::transparentWhite);
        errorMessage.setColor (TextEditor::shadowColorId,  Colors::transparentWhite);

        typeBox.setSelectedId (1);

        setSize (500, 500);
    }

    ~XMLandJSONDemo() override
    {
        resultsTree.setRootItem (nullptr);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto area = getLocalBounds();

        typeBox.setBounds (area.removeFromTop (36).removeFromRight (150).reduced (8));
        codeDocumentComponent.setBounds (area.removeFromTop (area.getHeight() / 2).reduced (8));
        resultsTree          .setBounds (area.reduced (8));
        errorMessage         .setBounds (resultsTree.getBounds());
    }

private:
    ComboBox typeBox;
    Label comboBoxLabel { {}, "Database Type:" };
    CodeDocument codeDocument;
    CodeEditorComponent codeDocumentComponent  { codeDocument, nullptr };
    TreeView resultsTree;

    std::unique_ptr<TreeViewItem> rootItem;
    std::unique_ptr<XmlElement> parsedXml;
    TextEditor errorMessage;

    z0 rebuildTree()
    {
        std::unique_ptr<XmlElement> openness;

        if (rootItem != nullptr)
            openness = rootItem->getOpennessState();

        createNewRootNode();

        if (openness != nullptr && rootItem != nullptr)
            rootItem->restoreOpennessState (*openness);
    }

    z0 createNewRootNode()
    {
        // clear the current tree
        resultsTree.setRootItem (nullptr);
        rootItem.reset();

        // try and parse the editor's contents
        switch (typeBox.getSelectedItemIndex())
        {
            case xml:           rootItem.reset (rebuildXml());        break;
            case json:          rootItem.reset (rebuildJson());       break;
            default:            rootItem.reset();                     break;
        }

        // if we have a valid TreeViewItem hide any old error messages and set our TreeView to use it
        if (rootItem != nullptr)
            errorMessage.clear();

        errorMessage.setVisible (! errorMessage.isEmpty());

        resultsTree.setRootItem (rootItem.get());
    }

    /** Parses the editor's contents as XML. */
    TreeViewItem* rebuildXml()
    {
        parsedXml.reset();

        XmlDocument doc (codeDocument.getAllContent());
        parsedXml = doc.getDocumentElement();

        if (parsedXml.get() == nullptr)
        {
            auto error = doc.getLastParseError();

            if (error.isEmpty())
                error = "Unknown error";

            errorMessage.setText ("Error parsing XML: " + error, dontSendNotification);

            return nullptr;
        }

        return new XmlTreeItem (*parsedXml);
    }

    /** Parses the editor's contents as JSON. */
    TreeViewItem* rebuildJson()
    {
        var parsedJson;
        auto result = JSON::parse (codeDocument.getAllContent(), parsedJson);

        if (! result.wasOk())
        {
            errorMessage.setText ("Error parsing JSON: " + result.getErrorMessage());
            return nullptr;
        }

        return new JsonTreeItem (Identifier(), parsedJson);
    }

    /** Clears the editor and loads some default text. */
    z0 reset (Type type)
    {
        switch (type)
        {
            case xml:   codeDocument.replaceAllContent (loadEntireAssetIntoString ("treedemo.xml")); break;
            case json:  codeDocument.replaceAllContent (loadEntireAssetIntoString ("drx_module_info")); break;
            default:    codeDocument.replaceAllContent ({}); break;
        }
    }

    z0 codeDocumentTextInserted (const Txt&, i32) override     { rebuildTree(); }
    z0 codeDocumentTextDeleted (i32, i32) override                { rebuildTree(); }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XMLandJSONDemo)
};
