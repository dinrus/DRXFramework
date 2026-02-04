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

#include "jucer_MainWindow.h"
#include "../Project/Modules/jucer_Modules.h"
#include "jucer_AutoUpdater.h"
#include "../CodeEditor/jucer_SourceCodeEditor.h"
#include "../Utility/UI/jucer_ProjucerLookAndFeel.h"

//==============================================================================
class ProjucerApplication final : public DRXApplication,
                                  private AsyncUpdater
{
public:
    ProjucerApplication() = default;

    static ProjucerApplication& getApp();
    static ApplicationCommandManager& getCommandManager();

    //==============================================================================
    z0 initialise (const Txt& commandLine) override;
    z0 shutdown() override;
    z0 systemRequestedQuit() override;
    z0 deleteLogger();

    const Txt getApplicationName() override       { return "Projucer"; }
    const Txt getApplicationVersion() override    { return ProjectInfo::versionString; }

    Txt getVersionDescription() const;
    b8 moreThanOneInstanceAllowed() override       { return true; } // this is handled manually in initialise()

    z0 anotherInstanceStarted (const Txt& commandLine) override;

    //==============================================================================
    MenuBarModel* getMenuModel();

    z0 getAllCommands (Array<CommandID>&) override;
    z0 getCommandInfo (CommandID commandID, ApplicationCommandInfo&) override;
    b8 perform (const InvocationInfo&) override;

    //==============================================================================
    z0 openFile (const File&, std::function<z0 (b8)>);
    z0 showPathsWindow (b8 highlightDRXPath = false);
    PropertiesFile::Options getPropertyFileOptionsFor (const Txt& filename, b8 isProjectSettings);
    z0 selectEditorColorSchemeWithName (const Txt& schemeName);

    //==============================================================================
    z0 rescanDRXPathModules();
    z0 rescanUserPathModules();

    AvailableModulesList& getDRXPathModulesList()     { return jucePathModulesList; }
    AvailableModulesList& getUserPathsModulesList()    { return userPathsModulesList; }

    b8 isAutomaticVersionCheckingEnabled() const;
    z0 setAutomaticVersionCheckingEnabled (b8 shouldBeEnabled);

    b8 shouldPromptUserAboutIncorrectDRXPath() const;
    z0 setShouldPromptUserAboutIncorrectDRXPath (b8 shouldPrompt);

    static File getDRXExamplesDirectoryPathFromGlobal() noexcept;
    static Array<File> getSortedExampleDirectories() noexcept;
    static Array<File> getSortedExampleFilesInDirectory (const File&) noexcept;

    //==============================================================================
    ProjucerLookAndFeel lookAndFeel;

    std::unique_ptr<StoredSettings> settings;
    std::unique_ptr<Icons> icons;

    struct MainMenuModel;
    std::unique_ptr<MainMenuModel> menuModel;

    MainWindowList mainWindowList;
    OpenDocumentManager openDocumentManager;
    std::unique_ptr<ApplicationCommandManager> commandManager;

    b8 isRunningCommandLine = false;

private:
    //==============================================================================
    z0 handleAsyncUpdate() override;
    z0 doBasicApplicationSetup();

    z0 initCommandManager();
    b8 initialiseLogger (tukk filePrefix);
    z0 initialiseWindows (const Txt& commandLine);

    z0 createNewProject();
    z0 createNewProjectFromClipboard();
    z0 createNewPIP();
    z0 askUserToOpenFile();
    z0 saveAllDocuments();
    z0 closeAllDocuments (OpenDocumentManager::SaveIfNeeded askUserToSave);
    z0 closeAllMainWindows (std::function<z0 (b8)>);
    z0 closeAllMainWindowsAndQuitIfNeeded();
    z0 clearRecentFiles();

    StringArray getMenuNames();
    PopupMenu createMenu (const Txt& menuName);
    PopupMenu createFileMenu();
    PopupMenu createEditMenu();
    PopupMenu createViewMenu();
    z0 createColorSchemeItems (PopupMenu&);
    PopupMenu createWindowMenu();
    PopupMenu createDocumentMenu();
    PopupMenu createToolsMenu();
    PopupMenu createHelpMenu();
    PopupMenu createExtraAppleMenuItems();
    z0 handleMainMenuCommand (i32 menuItemID);
    PopupMenu createExamplesPopupMenu() noexcept;

    z0 findAndLaunchExample (i32);

    z0 checkIfGlobalDRXPathHasChanged();
    File tryToFindDemoRunnerExecutable();
    File tryToFindDemoRunnerProject();
    z0 launchDemoRunner();

    z0 setColorScheme (i32 index, b8 saveSetting);
    z0 setEditorColorScheme (i32 index, b8 saveSetting);
    z0 updateEditorColorSchemeIfNeeded();

    z0 showUTF8ToolWindow();
    z0 showSVGPathDataToolWindow();
    z0 showAboutWindow();
    z0 showEditorColorSchemeWindow();
    z0 showPIPCreatorWindow();

    z0 launchForumBrowser();
    z0 launchModulesBrowser();
    z0 launchClassesBrowser();
    z0 launchTutorialsBrowser();

    //==============================================================================
   #if DRX_MAC
    class AppleMenuRebuildListener final : private MenuBarModel::Listener
    {
    public:
        AppleMenuRebuildListener()
        {
            if (auto* model = ProjucerApplication::getApp().getMenuModel())
                model->addListener (this);
        }

        ~AppleMenuRebuildListener() override
        {
            if (auto* model = ProjucerApplication::getApp().getMenuModel())
                model->removeListener (this);
        }

    private:
        z0 menuBarItemsChanged (MenuBarModel*) override  {}

        z0 menuCommandInvoked (MenuBarModel*,
                                 const ApplicationCommandTarget::InvocationInfo& info) override
        {
            if (info.commandID == CommandIDs::enableNewVersionCheck)
                Timer::callAfterDelay (50, [] { ProjucerApplication::getApp().rebuildAppleMenu(); });
        }
    };

    z0 rebuildAppleMenu();

    std::unique_ptr<AppleMenuRebuildListener> appleMenuRebuildListener;
   #endif

    //==============================================================================
    std::unique_ptr<TooltipWindow> tooltipWindow;
    AvailableModulesList jucePathModulesList, userPathsModulesList;

    std::unique_ptr<Component> utf8Window, svgPathWindow, aboutWindow, pathsWindow,
                               editorColorSchemeWindow, pipCreatorWindow;

    std::unique_ptr<FileLogger> logger;

    i32 numExamples = 0;
    std::unique_ptr<AlertWindow> demoRunnerAlert;
    b8 hasScannedForDemoRunnerExecutable = false, hasScannedForDemoRunnerProject = false;
    File lastDRXPath, lastDemoRunnerExectuableFile, lastDemoRunnerProjectFile;

    i32 selectedColorSchemeIndex = 0, selectedEditorColorSchemeIndex = 0;

    std::unique_ptr<FileChooser> chooser;
    ScopedMessageBox messageBox;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjucerApplication)
    DRX_DECLARE_WEAK_REFERENCEABLE (ProjucerApplication)
};
