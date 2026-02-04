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

#define DRX_PUSH_NOTIFICATIONS_IMPL 1

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

namespace PushNotificationsDelegateDetailsOsx
{
    using Action = PushNotifications::Notification::Action;

    //==============================================================================
    static NSUserNotification* juceNotificationToNSUserNotification (const PushNotifications::Notification& n)
    {
        auto notification = [[NSUserNotification alloc] init];

        notification.title           = juceStringToNS (n.title);
        notification.subtitle        = juceStringToNS (n.subtitle);
        notification.informativeText = juceStringToNS (n.body);
        notification.userInfo = varToNSDictionary (n.properties);

        auto triggerTime = Time::getCurrentTime() + RelativeTime (n.triggerIntervalSec);
        notification.deliveryDate = [NSDate dateWithTimeIntervalSince1970: (f64) triggerTime.toMilliseconds() / 1000.0];

        if (n.repeat && n.triggerIntervalSec >= 60)
        {
            auto dateComponents = [[NSDateComponents alloc] init];
            auto intervalSec = NSInteger (n.triggerIntervalSec);
            dateComponents.second = intervalSec;
            dateComponents.nanosecond = NSInteger ((n.triggerIntervalSec - (f64) intervalSec) * 1000000000);

            notification.deliveryRepeatInterval = dateComponents;

            [dateComponents autorelease];
        }

        auto soundToPlayString = n.soundToPlay.toString (true);

        if (soundToPlayString == "default_os_sound")
        {
            notification.soundName = NSUserNotificationDefaultSoundName;
        }
        else if (soundToPlayString.isNotEmpty())
        {
            auto* soundName = juceStringToNS (soundToPlayString.fromLastOccurrenceOf ("/", false, false)
                                                               .upToLastOccurrenceOf (".", false, false));

            notification.soundName = soundName;
        }

        notification.hasActionButton = n.actions.size() > 0;

        if (n.actions.size() > 0)
            notification.actionButtonTitle = juceStringToNS (n.actions.getReference (0).title);

        notification.identifier = juceStringToNS (n.identifier);

        if (n.actions.size() > 0)
        {
            notification.hasReplyButton = n.actions.getReference (0).style == Action::text;
            notification.responsePlaceholder = juceStringToNS (n.actions.getReference (0).textInputPlaceholder);
        }

        auto* imageDirectory = n.icon.contains ("/")
                             ? juceStringToNS (n.icon.upToLastOccurrenceOf ("/", false, true))
                             : [NSString string];

        auto* imageName      = juceStringToNS (n.icon.fromLastOccurrenceOf ("/", false, false)
                                                     .upToLastOccurrenceOf (".", false, false));
        auto* imageExtension = juceStringToNS (n.icon.fromLastOccurrenceOf (".", false, false));

        NSString* imagePath = nil;

        if ([imageDirectory length] == NSUInteger (0))
        {
            imagePath = [[NSBundle mainBundle] pathForResource: imageName
                                                        ofType: imageExtension];
        }
        else
        {
            imagePath = [[NSBundle mainBundle] pathForResource: imageName
                                                        ofType: imageExtension
                                                   inDirectory: imageDirectory];
        }

        notification.contentImage = [[NSImage alloc] initWithContentsOfFile: imagePath];

        if (n.actions.size() > 1)
        {
            auto additionalActions = [NSMutableArray arrayWithCapacity: (NSUInteger) n.actions.size() - 1];

            for (i32 a = 1; a < n.actions.size(); ++a)
                [additionalActions addObject: [NSUserNotificationAction actionWithIdentifier: juceStringToNS (n.actions[a].identifier)
                                                                                       title: juceStringToNS (n.actions[a].title)]];

            notification.additionalActions = additionalActions;
        }

        [notification autorelease];

        return notification;
    }

    //==============================================================================
    static PushNotifications::Notification nsUserNotificationToDrxNotification (NSUserNotification* n)
    {
        PushNotifications::Notification notif;

        notif.title       = nsStringToDrx (n.title);
        notif.subtitle    = nsStringToDrx (n.subtitle);
        notif.body        = nsStringToDrx (n.informativeText);

        notif.repeat = n.deliveryRepeatInterval != nil;

        if (n.deliveryRepeatInterval != nil)
        {
            notif.triggerIntervalSec = (f64) n.deliveryRepeatInterval.second + ((f64) n.deliveryRepeatInterval.nanosecond / 1000000000.0);
        }
        else
        {
            NSDate* dateNow = [NSDate date];
            NSDate* deliveryDate = n.deliveryDate;

            notif.triggerIntervalSec = [dateNow timeIntervalSinceDate: deliveryDate];
        }

        notif.soundToPlay = URL (nsStringToDrx (n.soundName));
        notif.properties  = nsDictionaryToVar (n.userInfo);

        notif.identifier = nsStringToDrx (n.identifier);

        if (n.contentImage != nil)
            notif.icon = nsStringToDrx ([n.contentImage name]);

        Array<Action> actions;

        if (n.actionButtonTitle != nil)
        {
            Action action;
            action.title = nsStringToDrx (n.actionButtonTitle);

            if (n.hasReplyButton)
                action.style = Action::text;

            if (n.responsePlaceholder != nil)
                action.textInputPlaceholder = nsStringToDrx (n.responsePlaceholder);

            actions.add (action);
        }

        if (n.additionalActions != nil)
        {
            for (NSUserNotificationAction* a in n.additionalActions)
            {
                Action action;
                action.identifier = nsStringToDrx (a.identifier);
                action.title      = nsStringToDrx (a.title);

                actions.add (action);
            }
        }

        return notif;
    }

    //==============================================================================
    static var getNotificationPropertiesFromDictionaryVar (const var& dictionaryVar)
    {
        auto* dictionaryVarObject = dictionaryVar.getDynamicObject();

        if (dictionaryVarObject == nullptr)
            return {};

        const auto& properties = dictionaryVarObject->getProperties();

        DynamicObject::Ptr propsVarObject = new DynamicObject();

        for (i32 i = 0; i < properties.size(); ++i)
        {
            auto propertyName = properties.getName (i).toString();

            if (propertyName == "aps")
                continue;

            propsVarObject->setProperty (propertyName, properties.getValueAt (i));
        }

        return var (propsVarObject.get());
    }

    static PushNotifications::Notification nsDictionaryToDrxNotification (NSDictionary* dictionary)
    {
        const var dictionaryVar = nsDictionaryToVar (dictionary);

        const var apsVar = dictionaryVar.getProperty ("aps", {});

        if (! apsVar.isObject())
            return {};

        var alertVar = apsVar.getProperty ("alert", {});

        const var titleVar = alertVar.getProperty ("title", {});
        const var bodyVar  = alertVar.isObject() ? alertVar.getProperty ("body", {}) : alertVar;

        const var categoryVar = apsVar.getProperty ("category", {});
        const var soundVar    = apsVar.getProperty ("sound", {});
        const var badgeVar    = apsVar.getProperty ("badge", {});
        const var threadIdVar = apsVar.getProperty ("thread-id", {});

        PushNotifications::Notification notification;

        notification.title       = titleVar   .toString();
        notification.body        = bodyVar    .toString();
        notification.groupId     = threadIdVar.toString();
        notification.category    = categoryVar.toString();
        notification.soundToPlay = URL (soundVar.toString());
        notification.badgeNumber = (i32) badgeVar;
        notification.properties  = getNotificationPropertiesFromDictionaryVar (dictionaryVar);

        return notification;
    }
}

//==============================================================================
struct PushNotificationsDelegate
{
    PushNotificationsDelegate() : delegate ([getClass().createInstance() init])
    {
        Class::setThis (delegate.get(), this);

        id<NSApplicationDelegate> appDelegate = [[NSApplication sharedApplication] delegate];

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        if ([appDelegate respondsToSelector: @selector (setPushNotificationsDelegate:)])
            [appDelegate performSelector: @selector (setPushNotificationsDelegate:) withObject: delegate.get()];
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

        [NSUserNotificationCenter defaultUserNotificationCenter].delegate = delegate.get();
    }

    virtual ~PushNotificationsDelegate()
    {
        [NSUserNotificationCenter defaultUserNotificationCenter].delegate = nil;
    }

    virtual z0 registeredForRemoteNotifications (NSData* deviceToken) = 0;

    virtual z0 failedToRegisterForRemoteNotifications (NSError* error) = 0;

    virtual z0 didReceiveRemoteNotification (NSDictionary* userInfo) = 0;

    virtual z0 didDeliverNotification (NSUserNotification* notification) = 0;

    virtual z0 didActivateNotification (NSUserNotification* notification) = 0;

    virtual b8 shouldPresentNotification (NSUserNotification* notification) = 0;

protected:
    NSUniquePtr<NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>> delegate;

private:
    struct Class final : public ObjCClass<NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>>
    {
        Class() : ObjCClass<NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>> ("DrxPushNotificationsDelegate_")
        {
            addIvar<PushNotificationsDelegate*> ("self");

            addMethod (@selector (application:didRegisterForRemoteNotificationsWithDeviceToken:), registeredForRemoteNotifications);
            addMethod (@selector (application:didFailToRegisterForRemoteNotificationsWithError:), failedToRegisterForRemoteNotifications);
            addMethod (@selector (application:didReceiveRemoteNotification:),                     didReceiveRemoteNotification);
            addMethod (@selector (userNotificationCenter:didDeliverNotification:),                didDeliverNotification);
            addMethod (@selector (userNotificationCenter:didActivateNotification:),               didActivateNotification);
            addMethod (@selector (userNotificationCenter:shouldPresentNotification:),             shouldPresentNotification);

            registerClass();
        }

        //==============================================================================
        static PushNotificationsDelegate& getThis (id self)         { return *getIvar<PushNotificationsDelegate*> (self, "self"); }
        static z0 setThis (id self, PushNotificationsDelegate* d) { object_setInstanceVariable (self, "self", d); }

        //==============================================================================
        static z0 registeredForRemoteNotifications       (id self, SEL, NSApplication*,
                                                            NSData* deviceToken)                { getThis (self).registeredForRemoteNotifications (deviceToken); }

        static z0 failedToRegisterForRemoteNotifications (id self, SEL, NSApplication*,
                                                            NSError* error)                     { getThis (self).failedToRegisterForRemoteNotifications (error); }

        static z0 didReceiveRemoteNotification           (id self, SEL, NSApplication*,
                                                            NSDictionary* userInfo)             { getThis (self).didReceiveRemoteNotification (userInfo); }

        static z0 didDeliverNotification          (id self, SEL, NSUserNotificationCenter*,
                                                     NSUserNotification* notification)          { getThis (self).didDeliverNotification (notification); }

        static z0 didActivateNotification         (id self, SEL, NSUserNotificationCenter*,
                                                     NSUserNotification* notification)          { getThis (self).didActivateNotification (notification); }

        static b8 shouldPresentNotification       (id self, SEL, NSUserNotificationCenter*,
                                                     NSUserNotification* notification)          { return getThis (self).shouldPresentNotification (notification); }
    };

    //==============================================================================
    static Class& getClass()
    {
        static Class c;
        return c;
    }
};

//==============================================================================
b8 PushNotifications::Notification::isValid() const noexcept { return true; }

//==============================================================================
struct PushNotifications::Impl : private PushNotificationsDelegate
{
    explicit Impl (PushNotifications& p)
        : owner (p)
    {
    }

    z0 requestPermissionsWithSettings (const PushNotifications::Settings& settingsToUse)
    {
        settings = settingsToUse;

        NSRemoteNotificationType types = NSUInteger ((b8) settings.allowBadge);

        types |= (NSUInteger) ((settings.allowSound ? NSRemoteNotificationTypeSound : 0)
                             | (settings.allowAlert ? NSRemoteNotificationTypeAlert : 0));

        [[NSApplication sharedApplication] registerForRemoteNotificationTypes: types];
    }

    z0 requestSettingsUsed()
    {
        settings.allowBadge = [NSApplication sharedApplication].enabledRemoteNotificationTypes & NSRemoteNotificationTypeBadge;

        settings.allowSound = [NSApplication sharedApplication].enabledRemoteNotificationTypes & NSRemoteNotificationTypeSound;
        settings.allowAlert = [NSApplication sharedApplication].enabledRemoteNotificationTypes & NSRemoteNotificationTypeAlert;

        owner.listeners.call ([&] (Listener& l) { l.notificationSettingsReceived (settings); });
    }

    b8 areNotificationsEnabled() const { return true; }

    z0 sendLocalNotification (const Notification& n)
    {
        auto* notification = PushNotificationsDelegateDetailsOsx::juceNotificationToNSUserNotification (n);

        [[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification: notification];
    }

    z0 getDeliveredNotifications() const
    {
        Array<PushNotifications::Notification> notifs;

        for (NSUserNotification* n in [NSUserNotificationCenter defaultUserNotificationCenter].deliveredNotifications)
            notifs.add (PushNotificationsDelegateDetailsOsx::nsUserNotificationToDrxNotification (n));

        owner.listeners.call ([&] (Listener& l) { l.deliveredNotificationsListReceived (notifs); });
    }

    z0 removeAllDeliveredNotifications()
    {
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
    }

    z0 removeDeliveredNotification (const Txt& identifier)
    {
        PushNotifications::Notification n;
        n.identifier = identifier;

        auto nsNotification = PushNotificationsDelegateDetailsOsx::juceNotificationToNSUserNotification (n);

        [[NSUserNotificationCenter defaultUserNotificationCenter] removeDeliveredNotification: nsNotification];
    }

    z0 setupChannels ([[maybe_unused]] const Array<ChannelGroup>& groups, [[maybe_unused]] const Array<Channel>& channels)
    {
    }

    z0 getPendingLocalNotifications() const
    {
        Array<PushNotifications::Notification> notifs;

        for (NSUserNotification* n in [NSUserNotificationCenter defaultUserNotificationCenter].scheduledNotifications)
            notifs.add (PushNotificationsDelegateDetailsOsx::nsUserNotificationToDrxNotification (n));

        owner.listeners.call ([&] (Listener& l) { l.pendingLocalNotificationsListReceived (notifs); });
    }

    z0 removePendingLocalNotification (const Txt& identifier)
    {
        PushNotifications::Notification n;
        n.identifier = identifier;

        auto nsNotification = PushNotificationsDelegateDetailsOsx::juceNotificationToNSUserNotification (n);

        [[NSUserNotificationCenter defaultUserNotificationCenter] removeScheduledNotification: nsNotification];
    }

    z0 removeAllPendingLocalNotifications()
    {
        for (NSUserNotification* n in [NSUserNotificationCenter defaultUserNotificationCenter].scheduledNotifications)
            [[NSUserNotificationCenter defaultUserNotificationCenter] removeScheduledNotification: n];
    }

    Txt getDeviceToken()
    {
        // You need to call requestPermissionsWithSettings() first.
        jassert (initialised);

        return deviceToken;
    }

    //==============================================================================
    //PushNotificationsDelegate
    z0 registeredForRemoteNotifications (NSData* deviceTokenToUse) override
    {
        deviceToken = [deviceTokenToUse]() -> Txt
        {
            auto length = deviceTokenToUse.length;

            if (auto* buffer = (u8k*) deviceTokenToUse.bytes)
            {
                NSMutableString* hexString = [NSMutableString stringWithCapacity: (length * 2)];

                for (NSUInteger i = 0; i < length; ++i)
                    [hexString appendFormat:@"%02x", buffer[i]];

                return nsStringToDrx ([hexString copy]);
            }

            return {};
        }();

        initialised = true;

        owner.listeners.call ([&] (Listener& l) { l.deviceTokenRefreshed (deviceToken); });
    }

    z0 failedToRegisterForRemoteNotifications ([[maybe_unused]] NSError* error) override
    {
        deviceToken.clear();
    }

    z0 didReceiveRemoteNotification (NSDictionary* userInfo) override
    {
        auto n = PushNotificationsDelegateDetailsOsx::nsDictionaryToDrxNotification (userInfo);
        owner.listeners.call ([&] (Listener& l) { l.handleNotification (true, n); });
    }

    z0 didDeliverNotification ([[maybe_unused]] NSUserNotification* notification) override
    {
    }

    z0 didActivateNotification (NSUserNotification* notification) override
    {
        auto n = PushNotificationsDelegateDetailsOsx::nsUserNotificationToDrxNotification (notification);

        if (notification.activationType == NSUserNotificationActivationTypeContentsClicked)
        {
            owner.listeners.call ([&] (Listener& l) { l.handleNotification (notification.remote, n); });
        }
        else
        {
            const auto actionIdentifier = nsStringToDrx ([&]
            {
                if (notification.additionalActivationAction != nil)
                    return notification.additionalActivationAction.identifier;

                return notification.actionButtonTitle;
            }());

            auto reply = notification.activationType == NSUserNotificationActivationTypeReplied
                            ? nsStringToDrx ([notification.response string])
                            : Txt();

            owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (notification.remote, n, actionIdentifier, reply); });
        }
    }

    b8 shouldPresentNotification (NSUserNotification*) override { return true; }

    z0 subscribeToTopic ([[maybe_unused]] const Txt& topic)     {}
    z0 unsubscribeFromTopic ([[maybe_unused]] const Txt& topic) {}

    z0 sendUpstreamMessage ([[maybe_unused]] const Txt& serverSenderId,
                              [[maybe_unused]] const Txt& collapseKey,
                              [[maybe_unused]] const Txt& messageId,
                              [[maybe_unused]] const Txt& messageType,
                              [[maybe_unused]] i32 timeToLive,
                              [[maybe_unused]] const StringPairArray& additionalData)
    {
    }

private:
    PushNotifications& owner;

    b8 initialised = false;
    Txt deviceToken;

    PushNotifications::Settings settings;
};

DRX_END_IGNORE_DEPRECATION_WARNINGS

} // namespace drx
