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

 name:             NetworkingDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases networking features.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        NetworkingDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class NetworkingDemo final : public Component,
                             private Thread
{
public:
    NetworkingDemo()
        : Thread ("Network Demo")
    {
        setOpaque (true);

        addAndMakeVisible (urlBox);
        urlBox.setText ("https://www.google.com");
        urlBox.onReturnKey = [this] { fetchButton.triggerClick(); };

        addAndMakeVisible (fetchButton);
        fetchButton.onClick = [this] { startThread(); };

        addAndMakeVisible (resultsBox);

        setSize (500, 500);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground));
    }

    z0 resized() override
    {
        auto area = getLocalBounds();

        {
            auto topArea = area.removeFromTop (40);
            fetchButton.setBounds (topArea.removeFromRight (180).reduced (8));
            urlBox     .setBounds (topArea.reduced (8));
        }

        resultsBox.setBounds (area.reduced (8));
    }

    z0 run() override
    {
        auto result = getResultText (urlBox.getText());

        MessageManagerLock mml (this);

        if (mml.lockWasGained())
            resultsBox.loadContent (result);
    }

    Txt getResultText (const URL& url)
    {
        StringPairArray responseHeaders;
        i32 statusCode = 0;

        if (auto stream = url.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                                                 .withConnectionTimeoutMs (10000)
                                                                                 .withResponseHeaders (&responseHeaders)
                                                                                 .withStatusCode (&statusCode)))
        {
            return (statusCode != 0 ? "Status code: " + Txt (statusCode) + newLine : Txt())
                    + "Response headers: " + newLine
                    + responseHeaders.getDescription() + newLine
                    + "----------------------------------------------------" + newLine
                    + stream->readEntireStreamAsString();
        }

        if (statusCode != 0)
            return "Failed to connect, status code = " + Txt (statusCode);

        return "Failed to connect!";
    }

private:
    TextEditor urlBox;
    TextButton fetchButton { "Download URL Contents" };

    CodeDocument resultsDocument;
    CodeEditorComponent resultsBox  { resultsDocument, nullptr };

    z0 lookAndFeelChanged() override
    {
        urlBox.applyFontToAllText (urlBox.getFont());
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkingDemo)
};
