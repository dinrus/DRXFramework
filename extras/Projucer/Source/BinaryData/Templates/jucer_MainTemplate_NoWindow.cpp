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
        // Add your application's initialisation code here..
    }

    z0 shutdown() override
    {
        // Add your application's shutdown code here..
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
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_DRX_APPLICATION (%%app_class_name%%)
