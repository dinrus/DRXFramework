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

#include "../Utility/PIPs/jucer_PIPGenerator.h"
#include "../Project/jucer_Project.h"
#include "../CodeEditor/jucer_OpenDocumentManager.h"

class ProjectContentComponent;

//==============================================================================
/**
    The big top-level window where everything happens.
*/
class MainWindow final : public DocumentWindow,
                         public ApplicationCommandTarget,
                         public FileDragAndDropTarget,
                         public DragAndDropContainer,
                         private Value::Listener,
                         private ChangeListener
{
public:
    //==============================================================================
    MainWindow();
    ~MainWindow() override;

    enum class OpenInIDE { no, yes };

    //==============================================================================
    z0 closeButtonPressed() override;

    //==============================================================================
    b8 canOpenFile (const File& file) const;
    z0 openFile (const File& file, std::function<z0 (b8)> callback);

    z0 setProject (std::unique_ptr<Project> newProject);
    Project* getProject() const  { return currentProject.get(); }

    z0 makeVisible();
    z0 restoreWindowPosition();
    z0 updateTitleBarIcon();
    z0 closeCurrentProject (OpenDocumentManager::SaveIfNeeded askToSave, std::function<z0 (b8)> callback);
    z0 moveProject (File newProjectFile, OpenInIDE openInIDE);

    z0 showStartPage();

    b8 isInterestedInFileDrag (const StringArray& files) override;
    z0 filesDropped (const StringArray& filenames, i32 mouseX, i32 mouseY) override;

    z0 activeWindowStatusChanged() override;

    ProjectContentComponent* getProjectContentComponent() const;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    z0 getAllCommands (Array <CommandID>& commands) override;
    z0 getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    b8 perform (const InvocationInfo& info) override;

    b8 shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                               StringArray& files, b8& canMoveFiles) override;
private:
    z0 valueChanged (Value&) override;
    z0 changeListenerCallback (ChangeBroadcaster* source) override;

    static tukk getProjectWindowPosName()   { return "projectWindowPos"; }
    z0 createProjectContentCompIfNeeded();

    z0 openPIP (const File&, std::function<z0 (b8)> callback);
    z0 setupTemporaryPIPProject (PIPGenerator&);

    z0 initialiseProjectWindow();

    std::unique_ptr<Project> currentProject;
    Value projectNameValue;

    std::unique_ptr<Component> blurOverlayComponent;

    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

//==============================================================================
class MainWindowList
{
public:
    MainWindowList();

    z0 forceCloseAllWindows();
    z0 askAllWindowsToClose (std::function<z0 (b8)> callback);
    z0 closeWindow (MainWindow*);

    z0 goToSiblingWindow (MainWindow*, i32 delta);

    z0 createWindowIfNoneAreOpen();
    z0 openDocument (OpenDocumentManager::Document*, b8 grabFocus);
    z0 openFile (const File& file, std::function<z0 (b8)> callback, b8 openInBackground = false);

    MainWindow* createNewMainWindow();
    MainWindow* getFrontmostWindow (b8 createIfNotFound = true);
    MainWindow* getOrCreateEmptyWindow();
    MainWindow* getMainWindowForFile (const File&);

    Project* getFrontmostProject();

    z0 reopenLastProjects();
    z0 saveCurrentlyOpenProjectList();

    z0 checkWindowBounds (MainWindow&);

    z0 sendLookAndFeelChange();

    OwnedArray<MainWindow> windows;

private:
    b8 isInReopenLastProjects = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindowList)
    DRX_DECLARE_WEAK_REFERENCEABLE (MainWindowList)
};
