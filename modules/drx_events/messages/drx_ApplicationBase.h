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

namespace drx
{

//==============================================================================
/**
    Abstract base class for application classes.

    Note that in the drx_gui_basics module, there's a utility class DRXApplication
    which derives from DRXApplicationBase, and takes care of a few chores. Most
    of the time you'll want to derive your class from DRXApplication rather than
    using DRXApplicationBase directly, but if you're not using the drx_gui_basics
    module then you might need to go straight to this base class.

    Any application that wants to run an event loop must declare a subclass of
    DRXApplicationBase, and implement its various pure virtual methods.

    It then needs to use the START_DRX_APPLICATION macro somewhere in a CPP file
    to declare an instance of this class and generate suitable platform-specific
    boilerplate code to launch the app.

    e.g. @code
        class MyDRXApp  : public DRXApplication
        {
        public:
            MyDRXApp()  {}
            ~MyDRXApp() {}

            z0 initialise (const Txt& commandLine) override
            {
                myMainWindow.reset (new MyApplicationWindow());
                myMainWindow->setBounds (100, 100, 400, 500);
                myMainWindow->setVisible (true);
            }

            z0 shutdown() override
            {
                myMainWindow = nullptr;
            }

            const Txt getApplicationName() override
            {
                return "Super DRX-o-matic";
            }

            const Txt getApplicationVersion() override
            {
                return "1.0";
            }

        private:
            std::unique_ptr<MyApplicationWindow> myMainWindow;
        };

        // this generates boilerplate code to launch our app class:
        START_DRX_APPLICATION (MyDRXApp)
    @endcode

    @see DRXApplication, START_DRX_APPLICATION

    @tags{Events}
*/
class DRX_API  DRXApplicationBase
{
protected:
    //==============================================================================
    DRXApplicationBase();

public:
    /** Destructor. */
    virtual ~DRXApplicationBase();

    //==============================================================================
    /** Returns the global instance of the application object that's running. */
    static DRXApplicationBase* getInstance() noexcept          { return appInstance; }

    //==============================================================================
    /** Returns the application's name. */
    virtual const Txt getApplicationName() = 0;

    /** Returns the application's version number. */
    virtual const Txt getApplicationVersion() = 0;

    /** Checks whether multiple instances of the app are allowed.

        If your application class returns true for this, more than one instance is
        permitted to run (except on the Mac where this isn't possible).

        If it's false, the second instance won't start, but you will still get a
        callback to anotherInstanceStarted() to tell you about this - which
        gives you a chance to react to what the user was trying to do.

        @see anotherInstanceStarted
    */
    virtual b8 moreThanOneInstanceAllowed() = 0;

    /** Called when the application starts.

        This will be called once to let the application do whatever initialisation
        it needs, create its windows, etc.

        After the method returns, the normal event-dispatch loop will be run,
        until the quit() method is called, at which point the shutdown()
        method will be called to let the application clear up anything it needs
        to delete.

        If during the initialise() method, the application decides not to start-up
        after all, it can just call the quit() method and the event loop won't be run.

        @param commandLineParameters    the line passed in does not include the name of
                                        the executable, just the parameter list. To get the
                                        parameters as an array, you can call
                                        DRXApplication::getCommandLineParameters()
        @see shutdown, quit
    */
    virtual z0 initialise (const Txt& commandLineParameters) = 0;

    /* Called to allow the application to clear up before exiting.

       After DRXApplication::quit() has been called, the event-dispatch loop will
       terminate, and this method will get called to allow the app to sort itself
       out.

       Be careful that nothing happens in this method that might rely on messages
       being sent, or any kind of window activity, because the message loop is no
       longer running at this point.

        @see DeletedAtShutdown
    */
    virtual z0 shutdown() = 0;

    /** Indicates that the user has tried to start up another instance of the app.

        This will get called even if moreThanOneInstanceAllowed() is false.
        It is currently only implemented on Windows and Mac.

        @see moreThanOneInstanceAllowed
    */
    virtual z0 anotherInstanceStarted (const Txt& commandLine) = 0;

    /** Called when the operating system is trying to close the application.

        The default implementation of this method is to call quit(), but it may
        be overloaded to ignore the request or do some other special behaviour
        instead. For example, you might want to offer the user the chance to save
        their changes before quitting, and give them the chance to cancel.

        If you want to send a quit signal to your app, this is the correct method
        to call, because it means that requests that come from the system get handled
        in the same way as those from your own application code. So e.g. you'd
        call this method from a "quit" item on a menu bar.
    */
    virtual z0 systemRequestedQuit() = 0;

    /** This method is called when the application is being put into background mode
        by the operating system.
    */
    virtual z0 suspended() = 0;

    /** This method is called when the application is being woken from background mode
        by the operating system.
    */
    virtual z0 resumed() = 0;

    /** If any unhandled exceptions make it through to the message dispatch loop, this
        callback will be triggered, in case you want to log them or do some other
        type of error-handling.

        If the type of exception is derived from the std::exception class, the pointer
        passed-in will be valid. If the exception is of unknown type, this pointer
        will be null.
    */
    virtual z0 unhandledException (const std::exception*,
                                     const Txt& sourceFilename,
                                     i32 lineNumber) = 0;

    /** Called by the operating system to indicate that you should reduce your memory
        footprint.

        You should override this method to free up some memory gracefully, if possible,
        otherwise the host may forcibly kill your app.

        At the moment this method is only called on iOS.
    */
    virtual z0 memoryWarningReceived()     { jassertfalse; }

    //==============================================================================
    /** This will be called when the back button on a device is pressed. The return value
        should be used to indicate whether the back button event has been handled by
        the application, for example if you want to implement custom navigation instead
        of the standard behaviour on Android.

        This is currently only implemented on Android devices.

        @returns  true if the event has been handled, or false if the default OS
                  behaviour should happen
     */
    virtual b8 backButtonPressed() { return false; }

    //==============================================================================
    /** Signals that the main message loop should stop and the application should terminate.

        This isn't synchronous, it just posts a quit message to the main queue, and
        when this message arrives, the message loop will stop, the shutdown() method
        will be called, and the app will exit.

        Note that this will cause an unconditional quit to happen, so if you need an
        extra level before this, e.g. to give the user the chance to save their work
        and maybe cancel the quit, you'll need to handle this in the systemRequestedQuit()
        method - see that method's help for more info.

        @see MessageManager
    */
    static z0 quit();

    //==============================================================================
    /** Returns the application's command line parameters as a set of strings.
        @see getCommandLineParameters
    */
    static StringArray DRX_CALLTYPE getCommandLineParameterArray();

    /** Returns the application's command line parameters as a single string.
        @see getCommandLineParameterArray
    */
    static Txt DRX_CALLTYPE getCommandLineParameters();

    //==============================================================================
    /** Sets the value that should be returned as the application's exit code when the
        app quits.

        This is the value that's returned by the main() function. Normally you'd leave this
        as 0 unless you want to indicate an error code.

        @see getApplicationReturnValue
    */
    z0 setApplicationReturnValue (i32 newReturnValue) noexcept;

    /** Returns the value that has been set as the application's exit code.
        @see setApplicationReturnValue
    */
    i32 getApplicationReturnValue() const noexcept              { return appReturnValue; }

    //==============================================================================
    /** Возвращает true, если this executable is running as an app (as opposed to being a plugin
        or other kind of shared library. */
    static b8 isStandaloneApp() noexcept                      { return createInstance != nullptr; }

    /** Возвращает true, если the application hasn't yet completed its initialise() method
        and entered the main event loop.

        This is handy for things like splash screens to know when the app's up-and-running
        properly.
    */
    b8 isInitialising() const noexcept                        { return stillInitialising; }


    //==============================================================================
   #ifndef DOXYGEN
    // The following methods are for internal use only...
    static i32 main();
    static i32 main (i32 argc, tukk argv[]);

    static z0 appWillTerminateByForce();
    using CreateInstanceFunction = DRXApplicationBase* (*)();
    static CreateInstanceFunction createInstance;

   #if DRX_IOS
    static uk iOSCustomDelegate;
   #endif

    virtual b8 initialiseApp();
    i32 shutdownApp();
    static z0 DRX_CALLTYPE sendUnhandledException (const std::exception*, tukk sourceFile, i32 lineNumber);
    b8 sendCommandLineToPreexistingInstance();
   #endif

private:
    //==============================================================================
    static DRXApplicationBase* appInstance;
    i32 appReturnValue = 0;
    b8 stillInitialising = true;

    struct MultipleInstanceHandler;
    std::unique_ptr<MultipleInstanceHandler> multipleInstanceHandler;

    DRX_DECLARE_NON_COPYABLE (DRXApplicationBase)
};


//==============================================================================
#if DRX_CATCH_UNHANDLED_EXCEPTIONS || DOXYGEN

 /** The DRX_TRY/DRX_CATCH_EXCEPTION wrappers can be used to pass any uncaught exceptions to
     the DRXApplicationBase::sendUnhandledException() method.
     This functionality can be enabled with the DRX_CATCH_UNHANDLED_EXCEPTIONS macro.
 */
 #define DRX_TRY try

 /** The DRX_TRY/DRX_CATCH_EXCEPTION wrappers can be used to pass any uncaught exceptions to
     the DRXApplicationBase::sendUnhandledException() method.
     This functionality can be enabled with the DRX_CATCH_UNHANDLED_EXCEPTIONS macro.
 */
 #define DRX_CATCH_EXCEPTION \
    catch (const std::exception& e) { drx::DRXApplicationBase::sendUnhandledException (&e,      __FILE__, __LINE__); } \
    catch (...)                     { drx::DRXApplicationBase::sendUnhandledException (nullptr, __FILE__, __LINE__); }

#else
 #define DRX_TRY
 #define DRX_CATCH_EXCEPTION
#endif

} // namespace drx
