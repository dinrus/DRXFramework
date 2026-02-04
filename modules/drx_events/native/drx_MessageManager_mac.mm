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

using AppFocusChangeCallback = z0 (*)();
AppFocusChangeCallback appFocusChangeCallback = nullptr;

using CheckEventBlockedByModalComps = b8 (*)(NSEvent*);
CheckEventBlockedByModalComps isEventBlockedByModalComps = nullptr;

using MenuTrackingChangedCallback = z0 (*)(b8);
MenuTrackingChangedCallback menuTrackingChangedCallback = nullptr;

//==============================================================================
struct AppDelegateClass final : public ObjCClass<NSObject>
{
    AppDelegateClass()  : ObjCClass ("DRXAppDelegate_")
    {
        addMethod (@selector (applicationWillFinishLaunching:), [] (id self, SEL, NSNotification*)
        {
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            [[NSAppleEventManager sharedAppleEventManager] setEventHandler: self
                                                               andSelector: @selector (getUrl:withReplyEvent:)
                                                             forEventClass: kInternetEventClass
                                                                andEventID: kAEGetURL];
            DRX_END_IGNORE_WARNINGS_GCC_LIKE
        });

        addMethod (@selector (applicationShouldTerminate:), [] (id /*self*/, SEL, NSApplication*)
        {
            if (auto* app = DRXApplicationBase::getInstance())
            {
                app->systemRequestedQuit();

                if (! MessageManager::getInstance()->hasStopMessageBeenSent())
                    return NSTerminateCancel;
            }

            return NSTerminateNow;
        });

        addMethod (@selector (applicationWillTerminate:), [] (id /*self*/, SEL, NSNotification*)
        {
            DRXApplicationBase::appWillTerminateByForce();
        });

        addMethod (@selector (application:openFile:), [] (id /*self*/, SEL, NSApplication*, NSString* filename)
        {
            if (auto* app = DRXApplicationBase::getInstance())
            {
                app->anotherInstanceStarted (quotedIfContainsSpaces (filename));
                return YES;
            }

            return NO;
        });

        addMethod (@selector (application:openFiles:), [] (id /*self*/, SEL, NSApplication*, NSArray* filenames)
        {
            if (auto* app = DRXApplicationBase::getInstance())
            {
                StringArray files;

                for (NSString* f in filenames)
                    files.add (quotedIfContainsSpaces (f));

                if (files.size() > 0)
                    app->anotherInstanceStarted (files.joinIntoString (" "));
            }
        });

        addMethod (@selector (applicationDidBecomeActive:), [] (id /*self*/, SEL, NSNotification*)  { focusChanged(); });
        addMethod (@selector (applicationDidResignActive:), [] (id /*self*/, SEL, NSNotification*)  { focusChanged(); });
        addMethod (@selector (applicationWillUnhide:),      [] (id /*self*/, SEL, NSNotification*)  { focusChanged(); });

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        addMethod (@selector (getUrl:withReplyEvent:), [] (id /*self*/, SEL, NSAppleEventDescriptor* event, NSAppleEventDescriptor*)
        {
            if (auto* app = DRXApplicationBase::getInstance())
                app->anotherInstanceStarted (quotedIfContainsSpaces ([[event paramDescriptorForKeyword: keyDirectObject] stringValue]));
        });

        addMethod (@selector (broadcastMessageCallback:), [] (id /*self*/, SEL, NSNotification* n)
        {
            NSDictionary* dict = (NSDictionary*) [n userInfo];
            auto messageString = nsStringToDrx ((NSString*) [dict valueForKey: nsStringLiteral ("message")]);
            MessageManager::getInstance()->deliverBroadcastMessage (messageString);
        });

        addMethod (@selector (mainMenuTrackingBegan:), [] (id /*self*/, SEL, NSNotification*)
        {
            NullCheckedInvocation::invoke (menuTrackingChangedCallback, true);
        });

        addMethod (@selector (mainMenuTrackingEnded:), [] (id /*self*/, SEL, NSNotification*)
        {
            NullCheckedInvocation::invoke (menuTrackingChangedCallback, false);
        });

        // (used as a way of running a dummy thread)
        addMethod (@selector (dummyMethod), [] (id /*self*/, SEL) {});
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

       #if DRX_PUSH_NOTIFICATIONS
        //==============================================================================
        addIvar<NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>*> ("pushNotificationsDelegate");

        addMethod (@selector (applicationDidFinishLaunching:), [] (id self, SEL, NSNotification* notification)
        {
            if (notification.userInfo != nil)
            {
                DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
                // NSUserNotification is deprecated from macOS 11, but there doesn't seem to be a
                // replacement for NSApplicationLaunchUserNotificationKey returning a non-deprecated type
                NSUserNotification* userNotification = notification.userInfo[NSApplicationLaunchUserNotificationKey];
                DRX_END_IGNORE_DEPRECATION_WARNINGS

                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
                if (userNotification != nil && userNotification.userInfo != nil)
                    [self application: [NSApplication sharedApplication] didReceiveRemoteNotification: userNotification.userInfo];
                DRX_END_IGNORE_WARNINGS_GCC_LIKE
            }
        });

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        addMethod (@selector (setPushNotificationsDelegate:), [] (id self, SEL, NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>* delegate)
        {
            object_setInstanceVariable (self, "pushNotificationsDelegate", delegate);
        });
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

        addMethod (@selector (application:didRegisterForRemoteNotificationsWithDeviceToken:), [] (id self, SEL, NSApplication* application, NSData* deviceToken)
        {
            auto* delegate = getPushNotificationsDelegate (self);

            SEL selector = @selector (application:didRegisterForRemoteNotificationsWithDeviceToken:);

            if (delegate != nil && [delegate respondsToSelector: selector])
            {
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [delegate methodSignatureForSelector: selector]];
                [invocation setSelector: selector];
                [invocation setTarget: delegate];
                [invocation setArgument: &application atIndex:2];
                [invocation setArgument: &deviceToken atIndex:3];

                [invocation invoke];
            }
        });

        addMethod (@selector (application:didFailToRegisterForRemoteNotificationsWithError:), [] (id self, SEL, NSApplication* application, NSError* error)
        {
            auto* delegate = getPushNotificationsDelegate (self);

            SEL selector =  @selector (application:didFailToRegisterForRemoteNotificationsWithError:);

            if (delegate != nil && [delegate respondsToSelector: selector])
            {
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [delegate methodSignatureForSelector: selector]];
                [invocation setSelector: selector];
                [invocation setTarget: delegate];
                [invocation setArgument: &application atIndex:2];
                [invocation setArgument: &error       atIndex:3];

                [invocation invoke];
            }
        });

        addMethod (@selector (application:didReceiveRemoteNotification:), [] (id self, SEL, NSApplication* application, NSDictionary* userInfo)
        {
            auto* delegate = getPushNotificationsDelegate (self);

            SEL selector =  @selector (application:didReceiveRemoteNotification:);

            if (delegate != nil && [delegate respondsToSelector: selector])
            {
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature: [delegate methodSignatureForSelector: selector]];
                [invocation setSelector: selector];
                [invocation setTarget: delegate];
                [invocation setArgument: &application atIndex:2];
                [invocation setArgument: &userInfo    atIndex:3];

                [invocation invoke];
            }
        });
       #endif

        registerClass();
    }

private:
    static z0 focusChanged()
    {
        if (appFocusChangeCallback != nullptr)
            (*appFocusChangeCallback)();
    }

    static Txt quotedIfContainsSpaces (NSString* file)
    {
        Txt s (nsStringToDrx (file));
        s = s.unquoted().replace ("\"", "\\\"");

        if (s.containsChar (' '))
            s = s.quoted();

        return s;
    }

    //==============================================================================
   #if DRX_PUSH_NOTIFICATIONS
    static NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>* getPushNotificationsDelegate (id self)
    {
        return getIvar<NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>*> (self, "pushNotificationsDelegate");
    }
   #endif
};

// This is declared at file scope, so that it's guaranteed to be
// constructed before and destructed after `appDelegate` (below)
static AppDelegateClass appDelegateClass;

//==============================================================================
struct AppDelegate
{
public:
    AppDelegate()
    {
        delegate = [appDelegateClass.createInstance() init];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [center addObserver: delegate selector: @selector (mainMenuTrackingBegan:)
                       name: NSMenuDidBeginTrackingNotification object: nil];
        [center addObserver: delegate selector: @selector (mainMenuTrackingEnded:)
                       name: NSMenuDidEndTrackingNotification object: nil];
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

        if (DRXApplicationBase::isStandaloneApp())
        {
            [NSApp setDelegate: delegate];

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            [[NSDistributedNotificationCenter defaultCenter] addObserver: delegate
                                                                selector: @selector (broadcastMessageCallback:)
                                                                    name: getBroadcastEventName()
                                                                  object: nil
                                                      suspensionBehavior: NSNotificationSuspensionBehaviorDeliverImmediately];
            DRX_END_IGNORE_WARNINGS_GCC_LIKE
        }
        else
        {
            [center addObserver: delegate selector: @selector (applicationDidResignActive:)
                           name: NSApplicationDidResignActiveNotification object: NSApp];

            [center addObserver: delegate selector: @selector (applicationDidBecomeActive:)
                           name: NSApplicationDidBecomeActiveNotification object: NSApp];

            [center addObserver: delegate selector: @selector (applicationWillUnhide:)
                           name: NSApplicationWillUnhideNotification object: NSApp];
        }
    }

    ~AppDelegate()
    {
        [[NSRunLoop currentRunLoop] cancelPerformSelectorsWithTarget: delegate];
        [[NSNotificationCenter defaultCenter] removeObserver: delegate];

        if (DRXApplicationBase::isStandaloneApp())
        {
            [NSApp setDelegate: nil];

            [[NSDistributedNotificationCenter defaultCenter] removeObserver: delegate
                                                                       name: getBroadcastEventName()
                                                                     object: nil];
        }

        [delegate release];
    }

    static NSString* getBroadcastEventName()
    {
        return juceStringToNS ("drx_" + Txt::toHexString (File::getSpecialLocation (File::currentExecutableFile).hashCode64()));
    }

    MessageQueue messageQueue;
    id delegate;
};

//==============================================================================
z0 MessageManager::runDispatchLoop()
{
    if (quitMessagePosted.get() == 0) // check that the quit message wasn't already posted..
    {
        DRX_AUTORELEASEPOOL
        {
            // must only be called by the message thread!
            jassert (isThisTheMessageThread());

           #if DRX_CATCH_UNHANDLED_EXCEPTIONS
            @try
            {
                [NSApp run];
            }
            @catch (NSException* e)
            {
                // An AppKit exception will kill the app, but at least this provides a chance to log it.,
                std::runtime_error ex (std::string ("NSException: ") + [[e name] UTF8String] + ", Reason:" + [[e reason] UTF8String]);
                DRXApplicationBase::sendUnhandledException (&ex, __FILE__, __LINE__);
            }
            @finally
            {
            }
           #else
            [NSApp run];
           #endif
        }
    }
}

static z0 shutdownNSApp()
{
    [NSApp stop: nil];
    [NSEvent startPeriodicEventsAfterDelay: 0  withPeriod: 0.1];
}

z0 MessageManager::stopDispatchLoop()
{
    if (isThisTheMessageThread())
    {
        quitMessagePosted = true;
        shutdownNSApp();
    }
    else
    {
        struct QuitCallback final : public CallbackMessage
        {
            QuitCallback() {}
            z0 messageCallback() override    { MessageManager::getInstance()->stopDispatchLoop(); }
        };

        (new QuitCallback())->post();
    }
}

#if DRX_MODAL_LOOPS_PERMITTED
b8 MessageManager::runDispatchLoopUntil (i32 millisecondsToRunFor)
{
    jassert (millisecondsToRunFor >= 0);
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    auto endTime = Time::currentTimeMillis() + millisecondsToRunFor;

    while (quitMessagePosted.get() == 0)
    {
        DRX_AUTORELEASEPOOL
        {
            auto msRemaining = endTime - Time::currentTimeMillis();

            if (msRemaining <= 0)
                break;

            CFRunLoopRunInMode (kCFRunLoopDefaultMode, jmin (1.0, msRemaining * 0.001), true);

            if (NSEvent* e = [NSApp nextEventMatchingMask: NSEventMaskAny
                                                untilDate: [NSDate dateWithTimeIntervalSinceNow: 0.001]
                                                   inMode: NSDefaultRunLoopMode
                                                  dequeue: YES])
                if (isEventBlockedByModalComps == nullptr || ! (*isEventBlockedByModalComps) (e))
                    [NSApp sendEvent: e];
        }
    }

    return quitMessagePosted.get() == 0;
}
#endif

//==============================================================================
z0 initialiseNSApplication();
z0 initialiseNSApplication()
{
    DRX_AUTORELEASEPOOL
    {
        [NSApplication sharedApplication];
    }
}

static std::unique_ptr<AppDelegate> appDelegate;

z0 MessageManager::doPlatformSpecificInitialisation()
{
    if (appDelegate == nil)
        appDelegate.reset (new AppDelegate());
}

z0 MessageManager::doPlatformSpecificShutdown()
{
    appDelegate = nullptr;
}

b8 MessageManager::postMessageToSystemQueue (MessageBase* message)
{
    jassert (appDelegate != nil);
    appDelegate->messageQueue.post (message);
    return true;
}

z0 MessageManager::broadcastMessage (const Txt& message)
{
    NSDictionary* info = [NSDictionary dictionaryWithObject: juceStringToNS (message)
                                                     forKey: nsStringLiteral ("message")];

    [[NSDistributedNotificationCenter defaultCenter] postNotificationName: AppDelegate::getBroadcastEventName()
                                                                   object: nil
                                                                 userInfo: info];
}

//==============================================================================
#if DRX_MAC
struct MountedVolumeListChangeDetector::Pimpl
{
    Pimpl (MountedVolumeListChangeDetector& d)  : owner (d)
    {
        static ObserverClass cls;
        delegate = [cls.createInstance() init];
        ObserverClass::setOwner (delegate, this);

        NSNotificationCenter* nc = [[NSWorkspace sharedWorkspace] notificationCenter];

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [nc addObserver: delegate selector: @selector (changed:) name: NSWorkspaceDidMountNotification   object: nil];
        [nc addObserver: delegate selector: @selector (changed:) name: NSWorkspaceDidUnmountNotification object: nil];
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
    }

    ~Pimpl()
    {
        [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver: delegate];
        [delegate release];
    }

private:
    MountedVolumeListChangeDetector& owner;
    id delegate;

    struct ObserverClass final : public ObjCClass<NSObject>
    {
        ObserverClass()  : ObjCClass<NSObject> ("DRXDriveObserver_")
        {
            addIvar<Pimpl*> ("owner");

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (changed:), changed);
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            addProtocol (@protocol (NSTextInput));
            registerClass();
        }

        static Pimpl* getOwner (id self)                { return getIvar<Pimpl*> (self, "owner"); }
        static z0 setOwner (id self, Pimpl* owner)    { object_setInstanceVariable (self, "owner", owner); }

        static z0 changed (id self, SEL, NSNotification*)
        {
            getOwner (self)->owner.mountedVolumeListChanged();
        }
    };
};

MountedVolumeListChangeDetector::MountedVolumeListChangeDetector()  { pimpl.reset (new Pimpl (*this)); }
MountedVolumeListChangeDetector::~MountedVolumeListChangeDetector() {}
#endif

} // namespace drx
