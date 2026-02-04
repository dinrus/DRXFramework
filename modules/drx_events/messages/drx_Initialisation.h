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
/** Initialises DRX's GUI classes.

    If you're embedding DRX into an application that uses its own event-loop rather
    than using the START_DRX_APPLICATION macro, call this function before making any
    DRX calls, to make sure things are initialised correctly.

    Note that if you're creating a DRX DLL for Windows, you may also need to call the
    Process::setCurrentModuleInstanceHandle() method.

    @see shutdownDrx_GUI()
*/
DRX_API z0 DRX_CALLTYPE  initialiseDrx_GUI();

/** Clears up any static data being used by DRX's GUI classes.

    If you're embedding DRX into an application that uses its own event-loop rather
    than using the START_DRX_APPLICATION macro, call this function in your shutdown
    code to clean up any DRX objects that might be lying around.

    @see initialiseDrx_GUI()
*/
DRX_API z0 DRX_CALLTYPE  shutdownDrx_GUI();


//==============================================================================
/** A utility object that helps you initialise and shutdown DRX correctly
    using an RAII pattern.

    When the first instance of this class is created, it calls initialiseDrx_GUI(),
    and when the last instance is deleted, it calls shutdownDrx_GUI(), so that you
    can easily be sure that as i64 as at least one instance of the class exists, the
    library will be initialised.

    This class is particularly handy to use at the beginning of a console app's
    main() function, because it'll take care of shutting down whenever you return
    from the main() call.

    Be careful with your threading though - to be safe, you should always make sure
    that these objects are created and deleted on the message thread.

    @tags{Events}
*/
class DRX_API  ScopedDrxInitialiser_GUI  final
{
public:
    /** The constructor simply calls initialiseDrx_GUI(). */
    ScopedDrxInitialiser_GUI();

    /** The destructor simply calls shutdownDrx_GUI(). */
    ~ScopedDrxInitialiser_GUI();

    DRX_DECLARE_NON_COPYABLE (ScopedDrxInitialiser_GUI)
    DRX_DECLARE_NON_MOVEABLE (ScopedDrxInitialiser_GUI)
};


//==============================================================================
/**
    To start a DRX app, use this macro: START_DRX_APPLICATION (AppSubClass) where
    AppSubClass is the name of a class derived from DRXApplication or DRXApplicationBase.

    See the DRXApplication and DRXApplicationBase class documentation for more details.
*/
#if DOXYGEN
 #define START_DRX_APPLICATION(AppClass)
#else
 #if DRX_WINDOWS && ! defined (_CONSOLE)
  #define DRX_MAIN_FUNCTION                                                        \
      DRX_BEGIN_IGNORE_WARNINGS_MSVC (28251)                                       \
      i32 __stdcall WinMain (struct HINSTANCE__*, struct HINSTANCE__*, tuk, i32)  \
      DRX_END_IGNORE_WARNINGS_MSVC
  #define DRX_MAIN_FUNCTION_ARGS
 #else
  #define DRX_MAIN_FUNCTION       i32 main (i32 argc, tuk argv[])
  #define DRX_MAIN_FUNCTION_ARGS  argc, (tukk*) argv
 #endif

 #if DRX_IOS

  #define DRX_CREATE_APPLICATION_DEFINE(AppClass) \
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes") \
    drx::DRXApplicationBase* drx_CreateApplication() { return new AppClass(); } \
    uk drx_GetIOSCustomDelegateClass()              { return nullptr; } \
    DRX_END_IGNORE_WARNINGS_GCC_LIKE

  #define DRX_CREATE_APPLICATION_DEFINE_CUSTOM_DELEGATE(AppClass, DelegateClass) \
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes") \
    drx::DRXApplicationBase* drx_CreateApplication() { return new AppClass(); } \
    uk drx_GetIOSCustomDelegateClass()              { return [DelegateClass class]; } \
    DRX_END_IGNORE_WARNINGS_GCC_LIKE

  #define DRX_MAIN_FUNCTION_DEFINITION \
    DRX_MAIN_FUNCTION \
    { \
       drx::DRXApplicationBase::createInstance = &drx_CreateApplication; \
       drx::DRXApplicationBase::iOSCustomDelegate = drx_GetIOSCustomDelegateClass(); \
       return drx::DRXApplicationBase::main (DRX_MAIN_FUNCTION_ARGS); \
    }

 #elif DRX_ANDROID

  #define DRX_CREATE_APPLICATION_DEFINE(AppClass) \
    extern "C" drx::DRXApplicationBase* drx_CreateApplication() { return new AppClass(); }

  #define DRX_MAIN_FUNCTION_DEFINITION

 #else

  #define DRX_CREATE_APPLICATION_DEFINE(AppClass) \
    drx::DRXApplicationBase* drx_CreateApplication(); \
    drx::DRXApplicationBase* drx_CreateApplication() { return new AppClass(); }

  #define DRX_MAIN_FUNCTION_DEFINITION \
    DRX_MAIN_FUNCTION \
    { \
       drx::DRXApplicationBase::createInstance = &drx_CreateApplication; \
       return drx::DRXApplicationBase::main (DRX_MAIN_FUNCTION_ARGS); \
    }

 #endif

 #if DrxPlugin_Build_Standalone
  #if DRX_USE_CUSTOM_PLUGIN_STANDALONE_APP
    #define START_DRX_APPLICATION(AppClass) DRX_CREATE_APPLICATION_DEFINE(AppClass)
    #if DRX_IOS
     #define START_DRX_APPLICATION_WITH_CUSTOM_DELEGATE(AppClass, DelegateClass) DRX_CREATE_APPLICATION_DEFINE_CUSTOM_DELEGATE(AppClass, DelegateClass)
    #endif
  #else
   #define START_DRX_APPLICATION(AppClass) static_assert(false, "You are trying to use START_DRX_APPLICATION in an audio plug-in. Define DRX_USE_CUSTOM_PLUGIN_STANDALONE_APP=1 if you want to use a custom standalone target app.");
   #if DRX_IOS
    #define START_DRX_APPLICATION_WITH_CUSTOM_DELEGATE(AppClass, DelegateClass) static_assert(false, "You are trying to use START_DRX_APPLICATION in an audio plug-in. Define DRX_USE_CUSTOM_PLUGIN_STANDALONE_APP=1 if you want to use a custom standalone target app.");
   #endif
  #endif
 #else

  #define START_DRX_APPLICATION(AppClass) \
     DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes") \
     DRX_CREATE_APPLICATION_DEFINE(AppClass) \
     DRX_MAIN_FUNCTION_DEFINITION \
     DRX_END_IGNORE_WARNINGS_GCC_LIKE

  #if DRX_IOS
    /**
       You can instruct DRX to use a custom iOS app delegate class instead of DRX's default
       app delegate. For DRX to work you must pass all messages to DRX's internal app delegate.
       Below is an example of minimal forwarding custom delegate. Note that you are at your own
       risk if you decide to use your own delegate and subtle, hard to debug bugs may occur.

       @interface MyCustomDelegate : NSObject <UIApplicationDelegate> { NSObject<UIApplicationDelegate>* juceDelegate; } @end

       @implementation MyCustomDelegate

       -(id) init
       {
           self = [super init];
           juceDelegate = reinterpret_cast<NSObject<UIApplicationDelegate>*> ([[NSClassFromString (@"DrxAppStartupDelegate") alloc] init]);
           return self;
       }

       -(z0) dealloc
       {
           [juceDelegate release];
           [super dealloc];
       }

       - (z0) forwardInvocation: (NSInvocation*) anInvocation
       {
           if (juceDelegate != nullptr && [juceDelegate respondsToSelector: [anInvocation selector]])
               [anInvocation invokeWithTarget: juceDelegate];
           else
               [super forwardInvocation: anInvocation];
       }

       -(BOOL) respondsToSelector: (SEL) aSelector
       {
           if (juceDelegate != nullptr && [juceDelegate respondsToSelector: aSelector])
               return YES;

           return [super respondsToSelector: aSelector];
       }
       @end
   */
   #define START_DRX_APPLICATION_WITH_CUSTOM_DELEGATE(AppClass, DelegateClass) \
      DRX_CREATE_APPLICATION_DEFINE_CUSTOM_DELEGATE(AppClass, DelegateClass) \
      DRX_MAIN_FUNCTION_DEFINITION
  #endif
 #endif
#endif

} // namespace drx
