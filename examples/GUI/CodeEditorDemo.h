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

 name:             CodeEditorDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Displays a code editor.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        CodeEditorDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

#if DRX_ANDROID
 #error "This demo не поддерживается on Android!"
#endif

//==============================================================================
class CodeEditorDemo final : public Component,
                             private FilenameComponentListener
{
public:
    CodeEditorDemo()
    {
        setOpaque (true);

        // Create the editor..
        editor.reset (new CodeEditorComponent (codeDocument, &cppTokeniser));
        addAndMakeVisible (editor.get());

        editor->loadContent ("\n"
                             "/* Code editor demo!\n"
                             "\n"
                             "   To see a real-world example of the code editor\n"
                             "   in action, have a look at the Projucer!\n"
                             "\n"
                             "*/\n"
                             "\n");

        // Create a file chooser control to load files into it..
        addAndMakeVisible (fileChooser);
        fileChooser.addListener (this);

        updateLookAndFeel();

        setSize (500, 500);
    }

    ~CodeEditorDemo() override
    {
        fileChooser.removeListener (this);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Colors::lightgrey));
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (8);

        fileChooser.setBounds (r.removeFromTop (25));
        editor->setBounds     (r.withTrimmedTop (8));
    }

private:
    // this is the document that the editor component is showing
    CodeDocument codeDocument;

    // this is a tokeniser to apply the C++ syntax highlighting
    CPlusPlusCodeTokeniser cppTokeniser;

    // the editor component
    std::unique_ptr<CodeEditorComponent> editor;

    FilenameComponent fileChooser { "File", {}, true, false, false, "*.cpp;*.h;*.hpp;*.c;*.mm;*.m", {},
                                    "Choose a C++ file to open it in the editor" };

    //==============================================================================
    z0 filenameComponentChanged (FilenameComponent*) override
    {
        editor->loadContent (fileChooser.getCurrentFile().loadFileAsString());
    }

    z0 updateLookAndFeel()
    {
        if (auto* v4 = dynamic_cast<LookAndFeel_V4*> (&LookAndFeel::getDefaultLookAndFeel()))
        {
            auto useLight = v4->getCurrentColorScheme() == LookAndFeel_V4::getLightColorScheme();
            editor->setColorScheme (useLight ? getLightCodeEditorColorScheme()
                                              : getDarkCodeEditorColorScheme());
        }
        else
        {
            editor->setColorScheme (cppTokeniser.getDefaultColorScheme());
        }
    }

    z0 lookAndFeelChanged() override
    {
        updateLookAndFeel();
    }

    CodeEditorComponent::ColorScheme getDarkCodeEditorColorScheme()
    {
        struct Type
        {
            tukk name;
            drx::u32 colour;
        };

        const Type types[] =
        {
            { "Error",              0xffe60000 },
            { "Comment",            0xff72d20c },
            { "Keyword",            0xffee6f6f },
            { "Operator",           0xffc4eb19 },
            { "Identifier",         0xffcfcfcf },
            { "Integer",            0xff42c8c4 },
            { "Float",              0xff885500 },
            { "Txt",             0xffbc45dd },
            { "Bracket",            0xff058202 },
            { "Punctuation",        0xffcfbeff },
            { "Preprocessor Text",  0xfff8f631 }
        };

        CodeEditorComponent::ColorScheme cs;

        for (auto& t : types)
            cs.set (t.name, Color (t.colour));

        return cs;
    }

    CodeEditorComponent::ColorScheme getLightCodeEditorColorScheme()
    {
        struct Type
        {
            tukk name;
            drx::u32 colour;
        };

        const Type types[] =
        {
            { "Error",              0xffcc0000 },
            { "Comment",            0xff00aa00 },
            { "Keyword",            0xff0000cc },
            { "Operator",           0xff225500 },
            { "Identifier",         0xff000000 },
            { "Integer",            0xff880000 },
            { "Float",              0xff885500 },
            { "Txt",             0xff990099 },
            { "Bracket",            0xff000055 },
            { "Punctuation",        0xff004400 },
            { "Preprocessor Text",  0xff660000 }
        };

        CodeEditorComponent::ColorScheme cs;

        for (auto& t : types)
            cs.set (t.name, Color (t.colour));

        return cs;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorDemo)
};
