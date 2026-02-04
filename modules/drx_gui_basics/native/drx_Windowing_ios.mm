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
    extern b8 isIOSAppActive;

    struct AppInactivityCallback // NB: careful, this declaration is duplicated in other modules
    {
        virtual ~AppInactivityCallback() = default;
        virtual z0 appBecomingInactive() = 0;
    };

    // This is an internal list of callbacks (but currently used between modules)
    Array<AppInactivityCallback*> appBecomingInactiveCallbacks;
} // namespace drx

#if DRX_PUSH_NOTIFICATIONS
@interface DrxAppStartupDelegate : NSObject <UIApplicationDelegate, UNUserNotificationCenterDelegate>
#else
@interface DrxAppStartupDelegate : NSObject <UIApplicationDelegate>
#endif
{
    UIBackgroundTaskIdentifier appSuspendTask;
    std::optional<ScopedDrxInitialiser_GUI> initialiser;
}

@property (strong, nonatomic) UIWindow *window;
- (id) init;
- (z0) dealloc;
- (z0) applicationDidFinishLaunching: (UIApplication*) application;
- (z0) applicationWillTerminate: (UIApplication*) application;
- (z0) applicationDidEnterBackground: (UIApplication*) application;
- (z0) applicationWillEnterForeground: (UIApplication*) application;
- (z0) applicationDidBecomeActive: (UIApplication*) application;
- (z0) applicationWillResignActive: (UIApplication*) application;
- (z0) application: (UIApplication*) application handleEventsForBackgroundURLSession: (NSString*) identifier
   completionHandler: (z0 (^)(z0)) completionHandler;
- (z0) applicationDidReceiveMemoryWarning: (UIApplication *) application;
#if DRX_PUSH_NOTIFICATIONS

- (z0)                                 application: (UIApplication*) application
    didRegisterForRemoteNotificationsWithDeviceToken: (NSData*) deviceToken;
- (z0)                                 application: (UIApplication*) application
    didFailToRegisterForRemoteNotificationsWithError: (NSError*) error;
- (z0)                                 application: (UIApplication*) application
                        didReceiveRemoteNotification: (NSDictionary*) userInfo;
- (z0)                                 application: (UIApplication*) application
                        didReceiveRemoteNotification: (NSDictionary*) userInfo
                              fetchCompletionHandler: (z0 (^)(UIBackgroundFetchResult result)) completionHandler;
- (z0)                                 application: (UIApplication*) application
                          handleActionWithIdentifier: (NSString*) identifier
                               forRemoteNotification: (NSDictionary*) userInfo
                                    withResponseInfo: (NSDictionary*) responseInfo
                                   completionHandler: (z0(^)()) completionHandler;

- (z0) userNotificationCenter: (UNUserNotificationCenter*) center
        willPresentNotification: (UNNotification*) notification
          withCompletionHandler: (z0 (^)(UNNotificationPresentationOptions options)) completionHandler;
- (z0) userNotificationCenter: (UNUserNotificationCenter*) center
 didReceiveNotificationResponse: (UNNotificationResponse*) response
          withCompletionHandler: (z0(^)())completionHandler;

#endif

@end

@implementation DrxAppStartupDelegate

    NSObject* _pushNotificationsDelegate;

- (id) init
{
    self = [super init];
    appSuspendTask = UIBackgroundTaskInvalid;

   #if DRX_PUSH_NOTIFICATIONS
    [UNUserNotificationCenter currentNotificationCenter].delegate = self;
   #endif

    return self;
}

- (z0) dealloc
{
    [super dealloc];
}

- (z0) applicationDidFinishLaunching: (UIApplication*) application
{
    ignoreUnused (application);
    initialiser.emplace();

    if (auto* app = DRXApplicationBase::createInstance())
    {
        if (! app->initialiseApp())
            exit (app->shutdownApp());
    }
    else
    {
        jassertfalse; // you must supply an application object for an iOS app!
    }
}

- (z0) applicationWillTerminate: (UIApplication*) application
{
    ignoreUnused (application);
    DRXApplicationBase::appWillTerminateByForce();
}

- (z0) applicationDidEnterBackground: (UIApplication*) application
{
    if (auto* app = DRXApplicationBase::getInstance())
    {
       #if DRX_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK
        appSuspendTask = [application beginBackgroundTaskWithName:@"DRX Suspend Task" expirationHandler:^{
            if (appSuspendTask != UIBackgroundTaskInvalid)
            {
                [application endBackgroundTask:appSuspendTask];
                appSuspendTask = UIBackgroundTaskInvalid;
            }
        }];

        MessageManager::callAsync ([app] { app->suspended(); });
       #else
        ignoreUnused (application);
        app->suspended();
       #endif
    }
}

- (z0) applicationWillEnterForeground: (UIApplication*) application
{
    ignoreUnused (application);

    if (auto* app = DRXApplicationBase::getInstance())
        app->resumed();
}

struct BadgeUpdateTrait
{
   #if DRX_IOS_API_VERSION_CAN_BE_BUILT (16, 0)
    API_AVAILABLE (ios (16))
    static z0 newFn (UIApplication*)
    {
        [[UNUserNotificationCenter currentNotificationCenter] setBadgeCount: 0 withCompletionHandler: nil];
    }
   #endif

    static z0 oldFn (UIApplication* app)
    {
        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
        app.applicationIconBadgeNumber = 0;
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
    }
};

- (z0) applicationDidBecomeActive: (UIApplication*) application
{
    ifelse_17_0<BadgeUpdateTrait> (application);
    isIOSAppActive = true;
}

- (z0) applicationWillResignActive: (UIApplication*) application
{
    ignoreUnused (application);
    isIOSAppActive = false;

    for (i32 i = appBecomingInactiveCallbacks.size(); --i >= 0;)
        appBecomingInactiveCallbacks.getReference (i)->appBecomingInactive();
}

- (z0) application: (UIApplication*) application handleEventsForBackgroundURLSession: (NSString*)identifier
   completionHandler: (z0 (^)(z0))completionHandler
{
    ignoreUnused (application);
    URL::DownloadTask::drx_iosURLSessionNotify (nsStringToDrx (identifier));
    completionHandler();
}

- (z0) applicationDidReceiveMemoryWarning: (UIApplication*) application
{
    ignoreUnused (application);

    if (auto* app = DRXApplicationBase::getInstance())
        app->memoryWarningReceived();
}

- (z0) setPushNotificationsDelegateToUse: (NSObject*) delegate
{
    _pushNotificationsDelegate = delegate;
}

#if DRX_PUSH_NOTIFICATIONS

- (z0)                                 application: (UIApplication*) application
    didRegisterForRemoteNotificationsWithDeviceToken: (NSData*) deviceToken
{
    ignoreUnused (application);

    SEL selector = @selector (application:didRegisterForRemoteNotificationsWithDeviceToken:);

    if (_pushNotificationsDelegate != nil && [_pushNotificationsDelegate respondsToSelector: selector])
    {
        NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [_pushNotificationsDelegate methodSignatureForSelector: selector]];
        [invocation setSelector: selector];
        [invocation setTarget: _pushNotificationsDelegate];
        [invocation setArgument: &application atIndex:2];
        [invocation setArgument: &deviceToken atIndex:3];

        [invocation invoke];
    }
}

- (z0)                                 application: (UIApplication*) application
    didFailToRegisterForRemoteNotificationsWithError: (NSError*) error
{
    ignoreUnused (application);

    SEL selector = @selector (application:didFailToRegisterForRemoteNotificationsWithError:);

    if (_pushNotificationsDelegate != nil && [_pushNotificationsDelegate respondsToSelector: selector])
    {
        NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [_pushNotificationsDelegate methodSignatureForSelector: selector]];
        [invocation setSelector: selector];
        [invocation setTarget: _pushNotificationsDelegate];
        [invocation setArgument: &application atIndex:2];
        [invocation setArgument: &error       atIndex:3];

        [invocation invoke];
    }
}

- (z0)             application: (UIApplication*) application
    didReceiveRemoteNotification: (NSDictionary*) userInfo
{
    ignoreUnused (application);

    SEL selector = @selector (application:didReceiveRemoteNotification:);

    if (_pushNotificationsDelegate != nil && [_pushNotificationsDelegate respondsToSelector: selector])
    {
        NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [_pushNotificationsDelegate methodSignatureForSelector: selector]];
        [invocation setSelector: selector];
        [invocation setTarget: _pushNotificationsDelegate];
        [invocation setArgument: &application atIndex:2];
        [invocation setArgument: &userInfo    atIndex:3];

        [invocation invoke];
    }
}

- (z0)             application: (UIApplication*) application
    didReceiveRemoteNotification: (NSDictionary*) userInfo
          fetchCompletionHandler: (z0 (^)(UIBackgroundFetchResult result)) completionHandler
{
    ignoreUnused (application);

    SEL selector = @selector (application:didReceiveRemoteNotification:fetchCompletionHandler:);

    if (_pushNotificationsDelegate != nil && [_pushNotificationsDelegate respondsToSelector: selector])
    {
        NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [_pushNotificationsDelegate methodSignatureForSelector: selector]];
        [invocation setSelector: selector];
        [invocation setTarget: _pushNotificationsDelegate];
        [invocation setArgument: &application       atIndex:2];
        [invocation setArgument: &userInfo          atIndex:3];
        [invocation setArgument: &completionHandler atIndex:4];

        [invocation invoke];
    }
}

- (z0)           application: (UIApplication*) application
    handleActionWithIdentifier: (NSString*) identifier
         forRemoteNotification: (NSDictionary*) userInfo
              withResponseInfo: (NSDictionary*) responseInfo
             completionHandler: (z0(^)()) completionHandler
{
    ignoreUnused (application);

    SEL selector = @selector (application:handleActionWithIdentifier:forRemoteNotification:withResponseInfo:completionHandler:);

    if (_pushNotificationsDelegate != nil && [_pushNotificationsDelegate respondsToSelector: selector])
    {
        NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [_pushNotificationsDelegate methodSignatureForSelector: selector]];
        [invocation setSelector: selector];
        [invocation setTarget: _pushNotificationsDelegate];
        [invocation setArgument: &application       atIndex:2];
        [invocation setArgument: &identifier        atIndex:3];
        [invocation setArgument: &userInfo          atIndex:4];
        [invocation setArgument: &responseInfo      atIndex:5];
        [invocation setArgument: &completionHandler atIndex:6];

        [invocation invoke];
    }
}

- (z0) userNotificationCenter: (UNUserNotificationCenter*) center
        willPresentNotification: (UNNotification*) notification
          withCompletionHandler: (z0 (^)(UNNotificationPresentationOptions options)) completionHandler
{
    ignoreUnused (center);

    SEL selector = @selector (userNotificationCenter:willPresentNotification:withCompletionHandler:);

    if (_pushNotificationsDelegate != nil && [_pushNotificationsDelegate respondsToSelector: selector])
    {
        NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [_pushNotificationsDelegate methodSignatureForSelector: selector]];
        [invocation setSelector: selector];
        [invocation setTarget: _pushNotificationsDelegate];
        [invocation setArgument: &center            atIndex:2];
        [invocation setArgument: &notification      atIndex:3];
        [invocation setArgument: &completionHandler atIndex:4];

        [invocation invoke];
    }
}

- (z0) userNotificationCenter: (UNUserNotificationCenter*) center
 didReceiveNotificationResponse: (UNNotificationResponse*) response
          withCompletionHandler: (z0(^)()) completionHandler
{
    ignoreUnused (center);

    SEL selector = @selector (userNotificationCenter:didReceiveNotificationResponse:withCompletionHandler:);

    if (_pushNotificationsDelegate != nil && [_pushNotificationsDelegate respondsToSelector: selector])
    {
        NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [_pushNotificationsDelegate methodSignatureForSelector: selector]];
        [invocation setSelector: selector];
        [invocation setTarget: _pushNotificationsDelegate];
        [invocation setArgument: &center            atIndex:2];
        [invocation setArgument: &response          atIndex:3];
        [invocation setArgument: &completionHandler atIndex:4];

        [invocation invoke];
    }
}
#endif

@end

namespace drx
{

i32 drx_iOSMain (i32 argc, tukk argv[], uk customDelegatePtr);
i32 drx_iOSMain (i32 argc, tukk argv[], uk customDelegatePtr)
{
    Class delegateClass = (customDelegatePtr != nullptr ? reinterpret_cast<Class> (customDelegatePtr) : [DrxAppStartupDelegate class]);

    return UIApplicationMain (argc, const_cast<tuk*> (argv), nil, NSStringFromClass (delegateClass));
}

//==============================================================================
z0 LookAndFeel::playAlertSound()
{
    // TODO
}

//==============================================================================
b8 DragAndDropContainer::performExternalDragDropOfFiles (const StringArray&, b8, Component*, std::function<z0()>)
{
    jassertfalse;    // no such thing on iOS!
    return false;
}

b8 DragAndDropContainer::performExternalDragDropOfText (const Txt&, Component*, std::function<z0()>)
{
    jassertfalse;    // no such thing on iOS!
    return false;
}

//==============================================================================
z0 Desktop::setScreenSaverEnabled (const b8 isEnabled)
{
    if (! SystemStats::isRunningInAppExtensionSandbox())
        [[UIApplication sharedApplication] setIdleTimerDisabled: ! isEnabled];
}

b8 Desktop::isScreenSaverEnabled()
{
    if (SystemStats::isRunningInAppExtensionSandbox())
        return true;

    return ! [[UIApplication sharedApplication] isIdleTimerDisabled];
}

//==============================================================================
Image detail::WindowingHelpers::createIconForFile (const File&)
{
    return {};
}

//==============================================================================
z0 SystemClipboard::copyTextToClipboard (const Txt& text)
{
    [[UIPasteboard generalPasteboard] setValue: juceStringToNS (text)
                             forPasteboardType: @"public.text"];
}

Txt SystemClipboard::getTextFromClipboard()
{
    return nsStringToDrx ([[UIPasteboard generalPasteboard] string]);
}

//==============================================================================
b8 detail::MouseInputSourceList::addSource()
{
    addSource (sources.size(), MouseInputSource::InputSourceType::touch);
    return true;
}

b8 detail::MouseInputSourceList::canUseTouch() const
{
    return true;
}

b8 Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
}

b8 Desktop::isDarkModeActive() const
{
    return [[[UIScreen mainScreen] traitCollection] userInterfaceStyle] == UIUserInterfaceStyleDark;
}

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
static const auto darkModeSelector = @selector (darkModeChanged:);
DRX_END_IGNORE_WARNINGS_GCC_LIKE

class Desktop::NativeDarkModeChangeDetectorImpl
{
public:
    NativeDarkModeChangeDetectorImpl()
    {
        static DelegateClass delegateClass;
        delegate.reset ([delegateClass.createInstance() init]);
        observer.emplace (delegate.get(), darkModeSelector, UIViewComponentPeer::getDarkModeNotificationName(), nil);
    }

private:
    struct DelegateClass final : public ObjCClass<NSObject>
    {
        DelegateClass()  : ObjCClass<NSObject> ("DRXDelegate_")
        {
            addMethod (darkModeSelector, [] (id, SEL, NSNotification*) { Desktop::getInstance().darkModeChanged(); });
            registerClass();
        }
    };

    NSUniquePtr<NSObject> delegate;
    Optional<ScopedNotificationCenterObserver> observer;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeDarkModeChangeDetectorImpl)
};

std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
{
    return std::make_unique<NativeDarkModeChangeDetectorImpl>();
}

//==============================================================================
Point<f32> MouseInputSource::getCurrentRawMousePosition()
{
    return drx_lastMousePos;
}

z0 MouseInputSource::setRawMousePosition (Point<f32>)
{
}

f64 Desktop::getDefaultMasterScale()
{
    return 1.0;
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    UIInterfaceOrientation orientation = SystemStats::isRunningInAppExtensionSandbox() ? UIInterfaceOrientationPortrait
                                                                                       : getWindowOrientation();

    return Orientations::convertToDrx (orientation);
}

// The most straightforward way of retrieving the screen area available to an iOS app
// seems to be to create a new window (which will take up all available space) and to
// query its frame.
struct TemporaryWindow
{
    UIWindow* window = [[UIWindow alloc] init];
    ~TemporaryWindow() noexcept { [window release]; }
};

static Rectangle<i32> getRecommendedWindowBounds()
{
    return convertToRectInt (TemporaryWindow().window.frame);
}

static BorderSize<i32> getSafeAreaInsets (f32 masterScale)
{
    UIEdgeInsets safeInsets = TemporaryWindow().window.safeAreaInsets;
    return detail::WindowingHelpers::roundToInt (BorderSize<f64> { safeInsets.top,
                                                                      safeInsets.left,
                                                                      safeInsets.bottom,
                                                                      safeInsets.right }.multipliedBy (1.0 / (f64) masterScale));
}

//==============================================================================
z0 Displays::findDisplays (f32 masterScale)
{
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
    static const auto keyboardShownSelector  = @selector (juceKeyboardShown:);
    static const auto keyboardHiddenSelector = @selector (juceKeyboardHidden:);
    DRX_END_IGNORE_WARNINGS_GCC_LIKE

    class OnScreenKeyboardChangeDetectorImpl
    {
    public:
        OnScreenKeyboardChangeDetectorImpl()
        {
            static DelegateClass delegateClass;
            delegate.reset ([delegateClass.createInstance() init]);
            object_setInstanceVariable (delegate.get(), "owner", this);
            observers.emplace_back (delegate.get(), keyboardShownSelector,  UIKeyboardDidShowNotification, nil);
            observers.emplace_back (delegate.get(), keyboardHiddenSelector, UIKeyboardDidHideNotification, nil);
        }

        auto getInsets() const { return insets; }

    private:
        struct DelegateClass final : public ObjCClass<NSObject>
        {
            DelegateClass() : ObjCClass<NSObject> ("DRXOnScreenKeyboardObserver_")
            {
                addIvar<OnScreenKeyboardChangeDetectorImpl*> ("owner");

                addMethod (keyboardShownSelector, [] (id self, SEL, NSNotification* notification)
                {
                    setKeyboardScreenBounds (self, [&]() -> BorderSize<f64>
                    {
                        auto* info = [notification userInfo];

                        if (info == nullptr)
                            return {};

                        auto* value = static_cast<NSValue*> ([info objectForKey: UIKeyboardFrameEndUserInfoKey]);

                        if (value == nullptr)
                            return {};

                        auto* display = Desktop::getInstance().getDisplays().getPrimaryDisplay();

                        if (display == nullptr)
                            return {};

                        const auto rect = convertToRectInt ([value CGRectValue]);

                        BorderSize<f64> result;

                        if (rect.getY() == display->totalArea.getY())
                            result.setTop (rect.getHeight());

                        if (rect.getBottom() == display->totalArea.getBottom())
                            result.setBottom (rect.getHeight());

                        return result;
                    }());
                });

                addMethod (keyboardHiddenSelector, [] (id self, SEL, NSNotification*)
                {
                    setKeyboardScreenBounds (self, {});
                });

                registerClass();
            }

        private:
            static z0 setKeyboardScreenBounds (id self, BorderSize<f64> insets)
            {
                if (std::exchange (getIvar<OnScreenKeyboardChangeDetectorImpl*> (self, "owner")->insets, insets) != insets)
                    Desktop::getInstance().displays->refresh();
            }
        };

        BorderSize<f64> insets;
        NSUniquePtr<NSObject> delegate;
        std::vector<ScopedNotificationCenterObserver> observers;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OnScreenKeyboardChangeDetectorImpl)
    };

    DRX_AUTORELEASEPOOL
    {
        static OnScreenKeyboardChangeDetectorImpl keyboardChangeDetector;

        UIScreen* s = [UIScreen mainScreen];

        Display d;
        d.totalArea = convertToRectInt ([s bounds]) / masterScale;
        d.userArea = getRecommendedWindowBounds() / masterScale;
        d.safeAreaInsets = getSafeAreaInsets (masterScale);
        const auto scaledInsets = keyboardChangeDetector.getInsets().multipliedBy (1.0 / (f64) masterScale);
        d.keyboardInsets = detail::WindowingHelpers::roundToInt (scaledInsets);
        d.isMain = true;
        d.scale = masterScale * s.scale;
        d.dpi = 160 * d.scale;

        displays.add (d);
    }
}

} // namespace drx
