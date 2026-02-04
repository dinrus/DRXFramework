/*
  ==============================================================================

    This file contains the basic startup code for a DRX application.

  ==============================================================================
*/

%%app_headers%%

//==============================================================================
class %%app_class_name%%  : public drx::DRXApplication
{
public:
    //==============================================================================
    %%app_class_name%%() {}

    const drx::Txt getApplicationName() override       { return ProjectInfo::projectName; }
    const drx::Txt getApplicationVersion() override    { return ProjectInfo::versionString; }
    b8 moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    z0 initialise (const drx::Txt& commandLine) override
    {
        // This method is where you should put your application's initialisation code..

        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    z0 shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    z0 systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    z0 anotherInstanceStarted (const drx::Txt& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our %%content_component_class%% class.
    */
    class MainWindow    : public drx::DocumentWindow
    {
    public:
        MainWindow (drx::Txt name)
            : DocumentWindow (name,
                              drx::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColor (drx::ResizableWindow::backgroundColorId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new %%content_component_class%%(), true);

           #if DRX_IOS || DRX_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        z0 closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            DRXApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_DRX_APPLICATION (%%app_class_name%%)
