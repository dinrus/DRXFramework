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

#include "DemoContentComponent.h"
#include "SettingsContent.h"
#include "MainComponent.h"

//==============================================================================
struct DemoContent final : public Component
{
    DemoContent() noexcept    {}

    z0 resized() override
    {
        if (comp != nullptr)
            comp->setBounds (getLocalBounds());
    }

    z0 setComponent (Component* newComponent)
    {
        comp.reset (newComponent);

        if (comp != nullptr)
        {
            addAndMakeVisible (comp.get());
            resized();
        }
    }

    Component* getComponent() const noexcept    { return comp.get(); }
    z0 showHomeScreen()                       { setComponent (createIntroDemo()); }

private:
    std::unique_ptr<Component> comp;
};

//==============================================================================
#if ! (DRX_ANDROID || DRX_IOS)
struct CodeContent final : public Component
{
    CodeContent()
    {
        addAndMakeVisible (codeEditor);

        codeEditor.setReadOnly (true);
        codeEditor.setScrollbarThickness (8);

        updateLookAndFeel();
    }

    z0 resized() override
    {
        codeEditor.setBounds (getLocalBounds());
    }

    z0 setDefaultCodeContent()
    {
        document.replaceAllContent ("\n/*******************************************************************************\n"
                                    "          Select one of the demos from the side panel on the left to see\n"
                                    "            its code here and an instance running in the \"Demo\" tab!\n"
                                    "*******************************************************************************/\n");
    }

    z0 updateLookAndFeel()
    {
        auto* v4 = dynamic_cast<LookAndFeel_V4*> (&Desktop::getInstance().getDefaultLookAndFeel());

        if (v4 != nullptr && (v4->getCurrentColorScheme() != LookAndFeel_V4::getLightColorScheme()))
            codeEditor.setColorScheme (getDarkColorScheme());
        else
            codeEditor.setColorScheme (getLightColorScheme());
    }

    z0 lookAndFeelChanged() override
    {
        updateLookAndFeel();
    }

    CodeDocument document;
    CPlusPlusCodeTokeniser cppTokensier;
    CodeEditorComponent codeEditor  { document, &cppTokensier };
};
#endif

//==============================================================================
DemoContentComponent::DemoContentComponent (Component& mainComponent, std::function<z0 (b8)> callback)
    : TabbedComponent (TabbedButtonBar::Orientation::TabsAtTop),
      demoChangedCallback (std::move (callback))
{
    demoContent.reset (new DemoContent());
    addTab ("Demo",     Colors::transparentBlack, demoContent.get(), false);

   #if ! (DRX_ANDROID || DRX_IOS)
    codeContent.reset (new CodeContent());
    addTab ("Code",     Colors::transparentBlack, codeContent.get(), false);
   #endif

    addTab ("Settings", Colors::transparentBlack, new SettingsContent (dynamic_cast<MainComponent&> (mainComponent)), true);

    setTabBarDepth (40);
    updateLookAndFeel();
}

DemoContentComponent::~DemoContentComponent()
{
}

z0 DemoContentComponent::resized()
{
    TabbedComponent::resized();

    if (tabBarIndent > 0)
        getTabbedButtonBar().setBounds (getTabbedButtonBar().getBounds().withTrimmedLeft (tabBarIndent));
}

z0 DemoContentComponent::setDemo (const Txt& category, i32 selectedDemoIndex)
{
    if ((currentDemoCategory == category)
        && (currentDemoIndex == selectedDemoIndex))
        return;

    auto demo = DRXDemos::getCategory (category).demos[(size_t) selectedDemoIndex];

   #if ! (DRX_ANDROID || DRX_IOS)
    codeContent->document.replaceAllContent (trimPIP (demo.demoFile.loadFileAsString()));
    codeContent->codeEditor.scrollToLine (0);
   #endif

    auto* content = demo.callback();
    demoContent->setComponent (content);
    demoChangedCallback (demo.isHeavyweight);

    ensureDemoIsShowing();

    currentDemoCategory = category;
    currentDemoIndex = selectedDemoIndex;
}

b8 DemoContentComponent::isShowingHomeScreen() const noexcept
{
    return isComponentIntroDemo (demoContent->getComponent()) && getCurrentTabIndex() == 0;
}

z0 DemoContentComponent::showHomeScreen()
{
    demoContent->showHomeScreen();

   #if ! (DRX_ANDROID || DRX_IOS)
    codeContent->setDefaultCodeContent();
   #endif

    demoChangedCallback (false);

    ensureDemoIsShowing();

    resized();

    currentDemoCategory = {};
    currentDemoIndex = -1;
}

z0 DemoContentComponent::clearCurrentDemo()
{
    demoContent->setComponent (nullptr);
    demoChangedCallback (false);
}

z0 DemoContentComponent::updateLookAndFeel()
{
    auto backgroundColor = findColor (ResizableWindow::backgroundColorId);

    for (i32 i = 0; i < getNumTabs(); ++i)
        setTabBackgroundColor (i, backgroundColor);
}

z0 DemoContentComponent::lookAndFeelChanged()
{
    updateLookAndFeel();
}

Txt DemoContentComponent::trimPIP (const Txt& fileContents)
{
    auto lines = StringArray::fromLines (fileContents);

    auto metadataEndIndex = lines.indexOf (" END_DRX_PIP_METADATA");

    if (metadataEndIndex == -1)
        return fileContents;

    lines.removeRange (0, metadataEndIndex + 3); // account for newline and comment block end

    return lines.joinIntoString ("\n");
}

z0 DemoContentComponent::ensureDemoIsShowing()
{
    if (getCurrentTabIndex() == (getNumTabs() - 1))
        setCurrentTabIndex (0);
}
