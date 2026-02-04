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

#include "../../CodeEditor/jucer_OpenDocumentManager.h"
#include "jucer_HeaderComponent.h"
#include "jucer_ProjectMessagesComponent.h"
#include "jucer_ContentViewComponent.h"

class Sidebar;
struct WizardHolder;

//==============================================================================
class ProjectContentComponent final : public Component,
                                      public ApplicationCommandTarget,
                                      private ChangeListener,
                                      private OpenDocumentManager::DocumentCloseListener
{
public:
    //==============================================================================
    ProjectContentComponent();
    ~ProjectContentComponent() override;

    Project* getProject() const noexcept    { return project; }
    z0 setProject (Project*);

    z0 saveOpenDocumentList();
    z0 reloadLastOpenDocuments();

    b8 showEditorForFile (const File&, b8 grabFocus);
    b8 hasFileInRecentList (const File&) const;
    File getCurrentFile() const;

    b8 showDocument (OpenDocumentManager::Document*, b8 grabFocus);
    z0 hideDocument (OpenDocumentManager::Document*);
    OpenDocumentManager::Document* getCurrentDocument() const    { return currentDocument; }
    z0 closeDocument();
    z0 saveDocumentAsync();
    z0 saveAsAsync();

    z0 hideEditor();
    z0 setScrollableEditorComponent (std::unique_ptr<Component> component);
    z0 setEditorDocument (std::unique_ptr<Component> component, OpenDocumentManager::Document* doc);
    Component* getEditorComponent();

    Component& getSidebarComponent();

    b8 goToPreviousFile();
    b8 goToNextFile();
    b8 canGoToCounterpart() const;
    b8 goToCounterpart();

    z0 saveProjectAsync();
    z0 closeProject();
    z0 openInSelectedIDE (b8 saveFirst);
    z0 showNewExporterMenu();

    z0 showFilesPanel()        { showProjectPanel (0); }
    z0 showModulesPanel()      { showProjectPanel (1); }
    z0 showExportersPanel()    { showProjectPanel (2); }

    z0 showProjectSettings();
    z0 showCurrentExporterSettings();
    z0 showExporterSettings (const Txt& exporterName);
    z0 showModule (const Txt& moduleID);

    z0 deleteSelectedTreeItems();

    z0 refreshProjectTreeFileStatuses();
    z0 updateMissingFileStatuses();

    z0 showBubbleMessage (Rectangle<i32>, const Txt&);

    StringArray getExportersWhichCanLaunch() const;

    static z0 getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails&,
                                                     OwnedArray<Project::Item>& selectedNodes);

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    z0 getAllCommands (Array<CommandID>&) override;
    z0 getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    b8 perform (const InvocationInfo&) override;

    b8 isSaveCommand (CommandID);

    z0 paint (Graphics&) override;
    z0 resized() override;
    z0 childBoundsChanged (Component*) override;
    z0 lookAndFeelChanged() override;

    ProjectMessagesComponent& getProjectMessagesComponent()  { return projectMessagesComponent; }

    static Txt getProjectTabName()    { return "Project"; }

private:
    //==============================================================================
    b8 documentAboutToClose (OpenDocumentManager::Document*) override;
    z0 changeListenerCallback (ChangeBroadcaster*) override;
    z0 showTranslationTool();

    //==============================================================================
    z0 showProjectPanel (i32 index);
    b8 canSelectedProjectBeLaunch();

    //==============================================================================
    Project* project = nullptr;
    OpenDocumentManager::Document* currentDocument = nullptr;
    RecentDocumentList recentDocumentList;

    HeaderComponent headerComponent { this };
    std::unique_ptr<Sidebar> sidebar;
    ProjectMessagesComponent projectMessagesComponent;
    ContentViewComponent contentViewComponent;

    std::unique_ptr<ResizableEdgeComponent> resizerBar;
    ComponentBoundsConstrainer sidebarSizeConstrainer;
    std::unique_ptr<Component> translationTool;
    BubbleMessageComponent bubbleMessage;

    b8 isForeground = false;
    i32 lastViewedTab = 0;

    std::unique_ptr<WizardHolder> wizardHolder;
    ScopedMessageBox messageBox;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectContentComponent)
};
