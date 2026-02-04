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
struct ColorPropertyComponent final : public PropertyComponent
{
    ColorPropertyComponent (UndoManager* undoManager, const Txt& name, const Value& colour,
                             Color defaultColor, b8 canResetToDefault)
        : PropertyComponent (name),
          colourEditor (undoManager, colour, defaultColor, canResetToDefault)
    {
        addAndMakeVisible (colourEditor);
    }

    z0 resized() override
    {
        colourEditor.setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
    }

    z0 refresh() override {}

private:
    /**
        A component that shows a colour swatch with hex ARGB value, and which pops up
        a colour selector when you click it.
    */
    struct ColorEditorComponent final : public Component,
                                         private Value::Listener
    {
        ColorEditorComponent (UndoManager* um, const Value& colour,
                               Color defaultCol, const b8 canReset)
            : undoManager (um), colourValue (colour), defaultColor (defaultCol),
              canResetToDefault (canReset)
        {
            colourValue.addListener (this);
        }

        z0 paint (Graphics& g) override
        {
            const Color colour (getColor());

            g.fillAll (Colors::grey);
            g.fillCheckerBoard (getLocalBounds().reduced (2).toFloat(),
                                10.0f, 10.0f,
                                Color (0xffdddddd).overlaidWith (colour),
                                Color (0xffffffff).overlaidWith (colour));

            g.setColor (Colors::white.overlaidWith (colour).contrasting());
            g.setFont (FontOptions ((f32) getHeight() * 0.6f, Font::bold));
            g.drawFittedText (colour.toDisplayString (true), getLocalBounds().reduced (2, 1),
                              Justification::centred, 1);
        }

        Color getColor() const
        {
            if (colourValue.toString().isEmpty())
                return defaultColor;

            return Color::fromString (colourValue.toString());
        }

        z0 setColor (Color newColor)
        {
            if (getColor() != newColor)
            {
                if (newColor == defaultColor && canResetToDefault)
                    colourValue = var();
                else
                    colourValue = newColor.toDisplayString (true);
            }
        }

        z0 resetToDefault()
        {
            setColor (defaultColor);
        }

        z0 refresh()
        {
            const Color col (getColor());

            if (col != lastColor)
            {
                lastColor = col;
                repaint();
            }
        }

        z0 mouseDown (const MouseEvent&) override
        {
            if (undoManager != nullptr)
                undoManager->beginNewTransaction();

            CallOutBox::launchAsynchronously (std::make_unique<PopupColorSelector> (colourValue,
                                                                                     defaultColor,
                                                                                     canResetToDefault),
                                              getScreenBounds(),
                                              nullptr);
        }

    private:
        z0 valueChanged (Value&) override
        {
            refresh();
        }

        UndoManager* undoManager;
        Value colourValue;
        Color lastColor;
        const Color defaultColor;
        const b8 canResetToDefault;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorEditorComponent)
    };

    //==============================================================================
    struct PopupColorSelector final : public Component,
                                       private ChangeListener,
                                       private Value::Listener
    {
        PopupColorSelector (const Value& colour,
                             Color defaultCol,
                             const b8 canResetToDefault)
            : defaultButton ("Reset to Default"),
              colourValue (colour),
              defaultColor (defaultCol)
        {
            addAndMakeVisible (selector);
            selector.setName ("Color");
            selector.setCurrentColor (getColor());
            selector.addChangeListener (this);

            if (canResetToDefault)
            {
                addAndMakeVisible (defaultButton);
                defaultButton.onClick = [this]
                {
                    setColor (defaultColor);
                    selector.setCurrentColor (defaultColor);
                };
            }

            colourValue.addListener (this);
            setSize (300, 400);
        }

        z0 resized() override
        {
            if (defaultButton.isVisible())
            {
                selector.setBounds (0, 0, getWidth(), getHeight() - 30);
                defaultButton.changeWidthToFitText (22);
                defaultButton.setTopLeftPosition (10, getHeight() - 26);
            }
            else
            {
                selector.setBounds (getLocalBounds());
            }
        }

        Color getColor() const
        {
            if (colourValue.toString().isEmpty())
                return defaultColor;

            return Color::fromString (colourValue.toString());
        }

        z0 setColor (Color newColor)
        {
            if (getColor() != newColor)
            {
                if (newColor == defaultColor && defaultButton.isVisible())
                    colourValue = var();
                else
                    colourValue = newColor.toDisplayString (true);
            }
        }

    private:
        z0 changeListenerCallback (ChangeBroadcaster*) override
        {
            if (selector.getCurrentColor() != getColor())
                setColor (selector.getCurrentColor());
        }

        z0 valueChanged (Value&) override
        {
            selector.setCurrentColor (getColor());
        }

        StoredSettings::ColorSelectorWithSwatches selector;
        TextButton defaultButton;
        Value colourValue;
        Color defaultColor;
    };

    ColorEditorComponent colourEditor;
};
