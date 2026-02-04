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

#include "../jucer_Headers.h"
#include "../jucer_Application.h"

#include "jucer_StartPageComponent.h"
#include "jucer_StartPageTreeHolder.h"
#include "jucer_NewProjectTemplates.h"
#include "jucer_ContentComponents.h"

#include <drx/Core/Core.h>

using Txt = drx::WTxt;

//==============================================================================
struct ContentComponent final : public Component
{
    ContentComponent()
    {
        setTitle (~Txt("Контент"));
        setFocusContainerType (FocusContainerType::focusContainer);
    }

    z0 resized() override
    {
        if (content != nullptr)
            content->setBounds (getLocalBounds());
    }

    z0 setContent (std::unique_ptr<Component>&& newContent)
    {
        if (content.get() != newContent.get())
        {
            content = std::move (newContent);
            addAndMakeVisible (content.get());
            resized();
        }
    }

private:
    std::unique_ptr<Component> content;

    //==============================================================================
    DRX_LEAK_DETECTOR (ContentComponent)
};

//==============================================================================
static File findExampleFile (i32 dirIndex, i32 index)
{
    auto dir = ProjucerApplication::getSortedExampleDirectories()[dirIndex];
    return ProjucerApplication::getSortedExampleFilesInDirectory (dir)[index];
}

static std::unique_ptr<Component> createExampleProjectsTab (ContentComponent& content, std::function<z0 (const File&)> cb)
{
    StringArray exampleCategories;
    std::vector<StringArray> examples;

    for (auto& dir : ProjucerApplication::getSortedExampleDirectories())
    {
        exampleCategories.add (dir.getFileName());

        StringArray ex;
        for (auto& f : ProjucerApplication::getSortedExampleFilesInDirectory (dir))
            ex.add (f.getFileNameWithoutExtension());

        examples.push_back (ex);
    }

    if (exampleCategories.isEmpty())
        return nullptr;

    auto selectedCallback = [&, cb] (i32 category, i32 index) mutable
    {
        content.setContent (std::make_unique<ExampleComponent> (findExampleFile (category, index), cb));
    };

    return std::make_unique<StartPageTreeHolder> (~Txt("Примеры"),
                                                  exampleCategories,
                                                  examples,
                                                  std::move (selectedCallback),
                                                  StartPageTreeHolder::Open::no);
}

//==============================================================================
static StringArray getAllTemplateCategoryStrings()
{
    StringArray categories;

    for (auto& t : NewProjectTemplates::getAllTemplates())
        categories.addIfNotAlreadyThere (NewProjectTemplates::getProjectCategoryString (t.category));

    return categories;
}

static std::vector<NewProjectTemplates::ProjectTemplate> getTemplatesInCategory (const Txt& category)
{
    std::vector<NewProjectTemplates::ProjectTemplate> templates;

    for (auto& t : NewProjectTemplates::getAllTemplates())
        if (NewProjectTemplates::getProjectCategoryString (t.category) == category)
            templates.push_back (t);

    return templates;
}

static StringArray getAllTemplateNamesForCategory (const Txt& category)
{
    StringArray types;

    for (auto& t : getTemplatesInCategory (category))
        types.add (t.displayName);

    return types;
}

static std::unique_ptr<Component> createProjectTemplatesTab (ContentComponent& content,
                                                             std::function<z0 (std::unique_ptr<Project>&&)>&& cb)
{
    auto categories = getAllTemplateCategoryStrings();

    std::vector<StringArray> templateNames;

    for (auto& c : categories)
        templateNames.push_back (getAllTemplateNamesForCategory (c));

    auto selectedCallback = [&, cb] (i32 category, i32 index)
    {
        auto categoryString = getAllTemplateCategoryStrings()[category];
        auto templates = getTemplatesInCategory (categoryString);

        content.setContent (std::make_unique<TemplateComponent> (templates[(size_t) index], std::move (cb)));
    };

    auto holder = std::make_unique<StartPageTreeHolder> (~Txt("Шаблоны"),
                                                         categories,
                                                         templateNames,
                                                         std::move (selectedCallback),
                                                         StartPageTreeHolder::Open::yes);
    holder->setSelectedItem (categories[0], 1);

    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wredundant-move")
    return std::move (holder);
    DRX_END_IGNORE_WARNINGS_GCC_LIKE
}

//==============================================================================
struct ProjectTemplatesAndExamples final : public TabbedComponent
{
    ProjectTemplatesAndExamples (ContentComponent& c,
                                 std::function<z0 (std::unique_ptr<Project>&&)>&& newProjectCb,
                                 std::function<z0 (const File&)>&& exampleCb)
        : TabbedComponent (TabbedButtonBar::Orientation::TabsAtTop),
          content (c),
          exampleSelectedCallback (std::move (exampleCb))
    {
        setTitle (~Txt("Шаблоны и Примеры"));
        setFocusContainerType (FocusContainerType::focusContainer);

        addTab (~Txt("Новый Проект"),
                Colors::transparentBlack,
                createProjectTemplatesTab (content, std::move (newProjectCb)).release(),
                true);

        refreshExamplesTab();
    }

    z0 refreshExamplesTab()
    {
        auto wasOpen = (getCurrentTabIndex() == 1);

        removeTab (1);

        auto exampleTabs = createExampleProjectsTab (content, exampleSelectedCallback);

        addTab (~Txt("Открыть Пример"),
                Colors::transparentBlack,
                exampleTabs == nullptr ? new SetDRXPathComponent (*this) : exampleTabs.release(),
                true);

        if (wasOpen)
            setCurrentTabIndex (1);
    }

private:
    //==============================================================================
    struct SetDRXPathComponent final : public Component,
                                        private ChangeListener
    {
        explicit SetDRXPathComponent (ProjectTemplatesAndExamples& o)
            : owner (o)
        {
            getGlobalProperties().addChangeListener (this);

            setPathButton.setButtonText ("Set path to DRX...");
            setPathButton.onClick = [] { ProjucerApplication::getApp().showPathsWindow (true); };

            addAndMakeVisible (setPathButton);
        }

        ~SetDRXPathComponent() override
        {
            getGlobalProperties().removeChangeListener (this);
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (findColor (secondaryBackgroundColorId));
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds().reduced (5);
            bounds.removeFromTop (25);

            setPathButton.setBounds (bounds.removeFromTop (25));
        }

    private:
        z0 changeListenerCallback (ChangeBroadcaster*) override
        {
            if (isValidDRXExamplesDirectory (ProjucerApplication::getDRXExamplesDirectoryPathFromGlobal()))
                owner.refreshExamplesTab();
        }

        ProjectTemplatesAndExamples& owner;
        TextButton setPathButton;
    };

    ContentComponent& content;
    std::function<z0 (const File&)> exampleSelectedCallback;
};

//==============================================================================
StartPageComponent::StartPageComponent (std::function<z0 (std::unique_ptr<Project>&&)>&& newProjectCb,
                                        std::function<z0 (const File&)>&& exampleCb)
    : content (std::make_unique<ContentComponent>()),
      tabs (std::make_unique<ProjectTemplatesAndExamples> (*content, std::move (newProjectCb), std::move (exampleCb)))
{
    tabs->setOutline (0);
    addAndMakeVisible (*tabs);

    addAndMakeVisible (openExistingButton);
    openExistingButton.setCommandToTrigger (&ProjucerApplication::getCommandManager(), CommandIDs::open, true);

    addAndMakeVisible (*content);

    setSize (900, 600);
}

z0 StartPageComponent::paint (Graphics& g)
{
    g.fillAll (findColor (backgroundColorId));
}

z0 StartPageComponent::resized()
{
    auto bounds = getLocalBounds().reduced (10);

    auto tabBounds = bounds.removeFromLeft (bounds.getWidth() / 3);

    openExistingButton.setBounds (tabBounds.removeFromBottom (30).reduced (10, 0));
    tabBounds.removeFromBottom (5);

    tabs->setBounds (tabBounds);
    bounds.removeFromLeft (10);

    content->setBounds (bounds);
}