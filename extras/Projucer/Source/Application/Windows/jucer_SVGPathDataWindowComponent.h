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
class SVGPathDataComponent final : public Component,
                                   public FileDragAndDropTarget

{
public:
    SVGPathDataComponent()
    {
        desc.setJustificationType (Justification::centred);
        addAndMakeVisible (desc);

        userText.setFont (getAppSettings().appearance.getCodeFont().withHeight (13.0f));
        userText.setMultiLine (true, true);
        userText.setReturnKeyStartsNewLine (true);
        addAndMakeVisible (userText);
        userText.onTextChange = [this] { update(); };
        userText.onEscapeKey  = [this] { getTopLevelComponent()->exitModalState (0); };

        resultText.setFont (getAppSettings().appearance.getCodeFont().withHeight (13.0f));
        resultText.setMultiLine (true, true);
        resultText.setReadOnly (true);
        resultText.setSelectAllWhenFocused (true);
        addAndMakeVisible (resultText);

        userText.setText (getLastText());

        addAndMakeVisible (copyButton);
        copyButton.onClick = [this] { SystemClipboard::copyTextToClipboard (resultText.getText()); };

        addAndMakeVisible (closeSubPathButton);
        closeSubPathButton.onClick = [this] { update(); };
        closeSubPathButton.setToggleState (true, NotificationType::dontSendNotification);

        addAndMakeVisible (fillPathButton);
        fillPathButton.onClick = [this] { update(); };
        fillPathButton.setToggleState (true, NotificationType::dontSendNotification);
    }

    z0 update()
    {
        getLastText() = userText.getText();
        auto text = getLastText().trim().unquoted().trim();

        path = Drawable::parseSVGPath (text);

        if (path.isEmpty())
            path = pathFromPoints (text);

        Txt result = "No path generated.. Not a valid SVG path string?";

        if (! path.isEmpty())
        {
            MemoryOutputStream data;
            path.writePathToStream (data);

            MemoryOutputStream out;

            out << "static u8k pathData[] = ";
            build_tools::writeDataAsCppLiteral (data.getMemoryBlock(), out, false, true);
            out << newLine
                << newLine
                << "Path path;" << newLine
                << "path.loadPathFromData (pathData, sizeof (pathData));" << newLine;

            result = out.toString();
        }

        resultText.setText (result, false);
        repaint (previewPathArea);
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (8);

        auto bottomSection = r.removeFromBottom (30);
        copyButton.setBounds (bottomSection.removeFromLeft (50));
        bottomSection.removeFromLeft (25);
        fillPathButton.setBounds (bottomSection.removeFromLeft (bottomSection.getWidth() / 2));
        closeSubPathButton.setBounds (bottomSection);

        r.removeFromBottom (5);
        desc.setBounds (r.removeFromTop (44));
        r.removeFromTop (8);
        userText.setBounds (r.removeFromTop (r.getHeight() / 2));
        r.removeFromTop (8);
        previewPathArea = r.removeFromRight (r.getHeight());
        resultText.setBounds (r);
    }

    z0 paint (Graphics& g) override
    {
        if (dragOver)
        {
            g.setColor (findColor (secondaryBackgroundColorId).brighter());
            g.fillAll();
        }

        g.setColor (findColor (defaultTextColorId));
        path.applyTransform (path.getTransformToScaleToFit (previewPathArea.reduced (4).toFloat(), true));

        if (fillPathButton.getToggleState())
            g.fillPath (path);
        else
            g.strokePath (path, PathStrokeType (2.0f));
    }

    z0 lookAndFeelChanged() override
    {
        userText.applyFontToAllText (userText.getFont());
        resultText.applyFontToAllText (resultText.getFont());
    }

    b8 isInterestedInFileDrag (const StringArray& files) override
    {
        return files.size() == 1
                && File (files[0]).hasFileExtension  ("svg");
    }

    z0 fileDragEnter (const StringArray&, i32, i32) override
    {
        dragOver = true;
        repaint();
    }

    z0 fileDragExit (const StringArray&) override
    {
        dragOver = false;
        repaint();
    }

    z0 filesDropped (const StringArray& files, i32, i32) override
    {
        dragOver = false;
        repaint();

        if (auto element = parseXML (File (files[0])))
        {
            if (auto* ePath = element->getChildByName ("path"))
                userText.setText (ePath->getStringAttribute ("d"), true);
            else if (auto* ePolygon = element->getChildByName ("polygon"))
                userText.setText (ePolygon->getStringAttribute ("points"), true);
        }
    }

    Path pathFromPoints (Txt pointsText)
    {
        auto points = StringArray::fromTokens (pointsText, " ,", "");
        points.removeEmptyStrings();

        jassert (points.size() % 2 == 0);

        Path p;

        for (i32 i = 0; i < points.size() / 2; i++)
        {
            auto x = points[i * 2].getFloatValue();
            auto y = points[i * 2 + 1].getFloatValue();

            if (i == 0)
                p.startNewSubPath ({ x, y });
            else
                p.lineTo ({ x, y });
        }

        if (closeSubPathButton.getToggleState())
            p.closeSubPath();

        return p;
    }

private:
    Label desc { {}, "Paste an SVG path string into the top box, and it'll be converted to some C++ "
                     "code that will load it as a Path object.." };
    TextButton copyButton { "Copy" };
    TextEditor userText, resultText;

    ToggleButton closeSubPathButton { "Close sub-path" };
    ToggleButton fillPathButton     { "Fill path" };

    Rectangle<i32> previewPathArea;
    Path path;
    b8 dragOver = false;

    Txt& getLastText()
    {
        static Txt t;
        return t;
    }
};
