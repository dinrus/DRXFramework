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

#include <DrxHeader.h>
#include "../../Assets/DemoUtilities.h"

#include "UI/MainComponent.h"

//==============================================================================
#if DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD
 // Just add a simple icon to the Window system tray area or Mac menu bar..
 struct DemoTaskbarComponent final : public SystemTrayIconComponent,
                                     private Timer
 {
     DemoTaskbarComponent()
     {
         setIconImage (getImageFromAssets ("drx_icon.png"),
                       getImageFromAssets ("drx_icon_template.png"));
         setIconTooltip ("DRX demo runner!");
     }

     z0 mouseDown (const MouseEvent&) override
     {
         // On OSX, there can be problems launching a menu when we're not the foreground
         // process, so just in case, we'll first make our process active, and then use a
         // timer to wait a moment before opening our menu, which gives the OS some time to
         // get its act together and bring our windows to the front.

         Process::makeForegroundProcess();
         startTimer (50);
     }

     // This is invoked when the menu is clicked or dismissed
     static z0 menuInvocationCallback (i32 chosenItemID, DemoTaskbarComponent*)
     {
         if (chosenItemID == 1)
             DRXApplication::getInstance()->systemRequestedQuit();
     }

     z0 timerCallback() override
     {
         stopTimer();

         PopupMenu m;
         m.addItem (1, "Quit");

         // It's always better to open menus asynchronously when possible.
         m.showMenuAsync (PopupMenu::Options(),
                          ModalCallbackFunction::forComponent (menuInvocationCallback, this));
     }
 };
#endif

std::unique_ptr<AudioDeviceManager> sharedAudioDeviceManager;

//==============================================================================
class DemoRunnerApplication final : public DRXApplication
{
public:
    //==============================================================================
    DemoRunnerApplication() {}

    ~DemoRunnerApplication() override
    {
        sharedAudioDeviceManager.reset();
    }

    const Txt getApplicationName() override       { return ProjectInfo::projectName; }
    const Txt getApplicationVersion() override    { return ProjectInfo::versionString; }
    b8 moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    z0 initialise (const Txt& commandLine) override
    {
        registerAllDemos();

      #if DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD
        // (This function call is for one of the demos, which involves launching a child process)
        if (invokeChildProcessDemo (commandLine))
            return;
      #else
        ignoreUnused (commandLine);
      #endif

        mainWindow.reset (new MainAppWindow (getApplicationName()));
    }

    b8 backButtonPressed() override    { mainWindow->getMainComponent().getSidePanel().showOrHide (false); return true; }
    z0 shutdown() override             { mainWindow = nullptr; }

    //==============================================================================
    z0 systemRequestedQuit() override                   { quit(); }
    z0 anotherInstanceStarted (const Txt&) override  {}

    ApplicationCommandManager& getGlobalCommandManager()  { return commandManager; }

private:
    class MainAppWindow final : public DocumentWindow
    {
    public:
        MainAppWindow (const Txt& name)
            : DocumentWindow (name, Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColor (ResizableWindow::backgroundColorId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setResizable (true, false);
            setResizeLimits (400, 400, 10000, 10000);

           #if DRX_IOS || DRX_ANDROID
            setFullScreen (true);

            auto& desktop = Desktop::getInstance();

            desktop.setOrientationsEnabled (Desktop::allOrientations);
            desktop.setKioskModeComponent (this);
           #else
            setBounds ((i32) (0.1f * (f32) getParentWidth()),
                       (i32) (0.1f * (f32) getParentHeight()),
                       jmax (850, (i32) (0.5f * (f32) getParentWidth())),
                       jmax (600, (i32) (0.7f * (f32) getParentHeight())));
           #endif

            setContentOwned (new MainComponent(), false);
            setVisible (true);

           #if DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD
            taskbarIcon.reset (new DemoTaskbarComponent());
           #endif
        }

        z0 closeButtonPressed() override    { DRXApplication::getInstance()->systemRequestedQuit(); }

       #if DRX_IOS || DRX_ANDROID
        z0 parentSizeChanged() override
        {
            getMainComponent().resized();
        }
       #endif

        //==============================================================================
        MainComponent& getMainComponent()    { return *dynamic_cast<MainComponent*> (getContentComponent()); }

    private:
        std::unique_ptr<Component> taskbarIcon;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAppWindow)
    };

    std::unique_ptr<MainAppWindow> mainWindow;
    ApplicationCommandManager commandManager;
};

ApplicationCommandManager& getGlobalCommandManager()
{
    return dynamic_cast<DemoRunnerApplication*> (DRXApplication::getInstance())->getGlobalCommandManager();
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_DRX_APPLICATION (DemoRunnerApplication)
