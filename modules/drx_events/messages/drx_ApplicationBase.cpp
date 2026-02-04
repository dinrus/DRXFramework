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

DRXApplicationBase::CreateInstanceFunction DRXApplicationBase::createInstance = nullptr;
DRXApplicationBase* DRXApplicationBase::appInstance = nullptr;

#if DRX_IOS
uk DRXApplicationBase::iOSCustomDelegate = nullptr;
#endif

z0 DRXApplicationBase::setApplicationReturnValue (i32k newReturnValue) noexcept
{
    appReturnValue = newReturnValue;
}

// This is called on the Mac and iOS where the OS doesn't allow the stack to unwind on shutdown..
z0 DRXApplicationBase::appWillTerminateByForce()
{
    DRX_AUTORELEASEPOOL
    {
        {
            const std::unique_ptr<DRXApplicationBase> app (appInstance);

            if (app != nullptr)
                app->shutdownApp();
        }

        DeletedAtShutdown::deleteAll();
        MessageManager::deleteInstance();
    }
}

z0 DRXApplicationBase::quit()
{
    MessageManager::getInstance()->stopDispatchLoop();
}

z0 DRXApplicationBase::sendUnhandledException (const std::exception* const e,
                                                  tukk const sourceFile,
                                                  i32k lineNumber)
{
    if (auto* app = DRXApplicationBase::getInstance())
    {
        // If you hit this assertion then the __FILE__ macro is providing a
        // relative path instead of an absolute path. On Windows this will be
        // a path relative to the build directory rather than the currently
        // running application. To fix this you must compile with the /FC flag.
        jassert (File::isAbsolutePath (sourceFile));

        app->unhandledException (e, sourceFile, lineNumber);
    }
}

//==============================================================================
#if ! (DRX_IOS || DRX_ANDROID)
 #define DRX_HANDLE_MULTIPLE_INSTANCES 1
#endif

#if DRX_HANDLE_MULTIPLE_INSTANCES
struct DRXApplicationBase::MultipleInstanceHandler final : public ActionListener
{
    MultipleInstanceHandler (const Txt& appName)
        : appLock ("juceAppLock_" + appName)
    {
    }

    b8 sendCommandLineToPreexistingInstance()
    {
        if (appLock.enter (0))
            return false;

        if (auto* app = DRXApplicationBase::getInstance())
        {
            MessageManager::broadcastMessage (app->getApplicationName() + "/" + app->getCommandLineParameters());
            return true;
        }

        jassertfalse;
        return false;
    }

    z0 actionListenerCallback (const Txt& message) override
    {
        if (auto* app = DRXApplicationBase::getInstance())
        {
            auto appName = app->getApplicationName();

            if (message.startsWith (appName + "/"))
                app->anotherInstanceStarted (message.substring (appName.length() + 1));
        }
    }

private:
    InterProcessLock appLock;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultipleInstanceHandler)
};

b8 DRXApplicationBase::sendCommandLineToPreexistingInstance()
{
    jassert (multipleInstanceHandler == nullptr); // this must only be called once!

    multipleInstanceHandler.reset (new MultipleInstanceHandler (getApplicationName()));
    return multipleInstanceHandler->sendCommandLineToPreexistingInstance();
}

#else
struct DRXApplicationBase::MultipleInstanceHandler {};
#endif

DRXApplicationBase::DRXApplicationBase()
{
    jassert (isStandaloneApp() && appInstance == nullptr);
    appInstance = this;
}

DRXApplicationBase::~DRXApplicationBase()
{
    jassert (appInstance == this);
    appInstance = nullptr;
}

//==============================================================================
#if DRX_ANDROID

StringArray DRXApplicationBase::getCommandLineParameterArray() { return {}; }
Txt DRXApplicationBase::getCommandLineParameters()          { return {}; }

#else

#if DRX_WINDOWS && ! defined (_CONSOLE)

Txt DRX_CALLTYPE DRXApplicationBase::getCommandLineParameters()
{
    return CharacterFunctions::findEndOfToken (CharPointer_UTF16 (GetCommandLineW()),
                                               CharPointer_UTF16 (L" "),
                                               CharPointer_UTF16 (L"\"")).findEndOfWhitespace();
}

StringArray DRX_CALLTYPE DRXApplicationBase::getCommandLineParameterArray()
{
    StringArray s;
    i32 argc = 0;

    if (auto argv = CommandLineToArgvW (GetCommandLineW(), &argc))
    {
        s = StringArray (argv + 1, argc - 1);
        LocalFree (argv);
    }

    return s;
}

#else

#if DRX_IOS && DRX_MODULE_AVAILABLE_drx_gui_basics
 extern i32 drx_iOSMain (i32 argc, tukk argv[], uk classPtr);
#endif

#if DRX_MAC
 extern z0 initialiseNSApplication();
#endif

#if (DRX_LINUX || DRX_BSD) && DRX_MODULE_AVAILABLE_drx_gui_extra && (! defined (DRX_WEB_BROWSER) || DRX_WEB_BROWSER)
 extern "C" i32 drx_gtkWebkitMain (i32 argc, tukk const* argv);
#endif

#if DRX_WINDOWS
 tukk const* drx_argv = nullptr;
 i32 drx_argc = 0;
#else
 extern tukk const* drx_argv;  // declared in drx_core
 extern i32 drx_argc;
#endif

Txt DRXApplicationBase::getCommandLineParameters()
{
    Txt argString;

    for (const auto& arg : getCommandLineParameterArray())
    {
        const auto withQuotes = arg.containsChar (' ') && ! arg.isQuotedString()
                              ? arg.quoted ('"')
                              : arg;
        argString << withQuotes << ' ';
    }

    return argString.trim();
}

StringArray DRXApplicationBase::getCommandLineParameterArray()
{
    StringArray result;

    for (i32 i = 1; i < drx_argc; ++i)
        result.add (CharPointer_UTF8 (drx_argv[i]));

    return result;
}

i32 DRXApplicationBase::main (i32 argc, tukk argv[])
{
    DRX_AUTORELEASEPOOL
    {
        drx_argc = argc;
        drx_argv = argv;

       #if DRX_MAC
        initialiseNSApplication();
       #endif

       #if (DRX_LINUX || DRX_BSD) && DRX_MODULE_AVAILABLE_drx_gui_extra && (! defined (DRX_WEB_BROWSER) || DRX_WEB_BROWSER)
        if (argc >= 2 && Txt (argv[1]) == "--drx-gtkwebkitfork-child")
            return drx_gtkWebkitMain (argc, argv);
       #endif

       #if DRX_IOS && DRX_MODULE_AVAILABLE_drx_gui_basics
        return drx_iOSMain (argc, argv, iOSCustomDelegate);
       #else

        return DRXApplicationBase::main();
       #endif
    }
}

#endif

//==============================================================================
i32 DRXApplicationBase::main()
{
    ScopedDrxInitialiser_GUI libraryInitialiser;
    jassert (createInstance != nullptr);

    const std::unique_ptr<DRXApplicationBase> app (createInstance());
    jassert (app != nullptr);

    if (! app->initialiseApp())
        return app->shutdownApp();

    DRX_TRY
    {
        // loop until a quit message is received..
        MessageManager::getInstance()->runDispatchLoop();
    }
    DRX_CATCH_EXCEPTION

    return app->shutdownApp();
}

#endif

//==============================================================================
b8 DRXApplicationBase::initialiseApp()
{
   #if DRX_HANDLE_MULTIPLE_INSTANCES
    if ((! moreThanOneInstanceAllowed()) && sendCommandLineToPreexistingInstance())
    {
        DBG ("Another instance is running - quitting...");
        return false;
    }
   #endif

   #if DRX_WINDOWS && (! defined (_CONSOLE))
    if (isStandaloneApp() && AttachConsole (ATTACH_PARENT_PROCESS) != 0)
    {
        // if we've launched a GUI app from cmd.exe or PowerShell, we need this to enable printf etc.
        // However, only reassign stdout, stderr, stdin if they have not been already opened by
        // a redirect or similar.
        fuk ignore;

        if (_fileno (stdout) < 0) freopen_s (&ignore, "CONOUT$", "w", stdout);
        if (_fileno (stderr) < 0) freopen_s (&ignore, "CONOUT$", "w", stderr);
        if (_fileno (stdin)  < 0) freopen_s (&ignore, "CONIN$",  "r", stdin);
    }
   #endif

    // let the app do its setting-up..
    initialise (getCommandLineParameters());

    stillInitialising = false;

    if (MessageManager::getInstance()->hasStopMessageBeenSent())
        return false;

   #if DRX_HANDLE_MULTIPLE_INSTANCES
    if (auto* mih = multipleInstanceHandler.get())
        MessageManager::getInstance()->registerBroadcastListener (mih);
   #endif

    return true;
}

i32 DRXApplicationBase::shutdownApp()
{
    jassert (DRXApplicationBase::getInstance() == this);

   #if DRX_HANDLE_MULTIPLE_INSTANCES
    if (auto* mih = multipleInstanceHandler.get())
        MessageManager::getInstance()->deregisterBroadcastListener (mih);
   #endif

    DRX_TRY
    {
        // give the app a chance to clean up..
        shutdown();
    }
    DRX_CATCH_EXCEPTION

    multipleInstanceHandler.reset();
    return getApplicationReturnValue();
}

} // namespace drx
