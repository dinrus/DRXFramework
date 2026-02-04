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

struct PushNotificationsDelegateDetails
{
    //==============================================================================
    using Action   = PushNotifications::Settings::Action;
    using Category = PushNotifications::Settings::Category;

    static uk actionToNSAction (const Action& a)
    {
        if (a.style == Action::text)
        {
            return [UNTextInputNotificationAction actionWithIdentifier: juceStringToNS (a.identifier)
                                                                 title: juceStringToNS (a.title)
                                                               options: NSUInteger (a.destructive << 1 | (! a.triggerInBackground) << 2)
                                                  textInputButtonTitle: juceStringToNS (a.textInputButtonText)
                                                  textInputPlaceholder: juceStringToNS (a.textInputPlaceholder)];
        }

        return [UNNotificationAction actionWithIdentifier: juceStringToNS (a.identifier)
                                                    title: juceStringToNS (a.title)
                                                  options: NSUInteger (a.destructive << 1 | (! a.triggerInBackground) << 2)];
    }

    static uk categoryToNSCategory (const Category& c)
    {
        auto actions = [NSMutableArray arrayWithCapacity: (NSUInteger) c.actions.size()];

        for (const auto& a : c.actions)
        {
            auto* action = (UNNotificationAction*) actionToNSAction (a);
            [actions addObject: action];
        }

        return [UNNotificationCategory categoryWithIdentifier: juceStringToNS (c.identifier)
                                                      actions: actions
                                            intentIdentifiers: @[]
                                                      options: c.sendDismissAction ? UNNotificationCategoryOptionCustomDismissAction : 0];
    }

    //==============================================================================
    static UNNotificationRequest* juceNotificationToUNNotificationRequest (const PushNotifications::Notification& n)
    {
        // content
        auto content = [[UNMutableNotificationContent alloc] init];

        content.title              = juceStringToNS (n.title);
        content.subtitle           = juceStringToNS (n.subtitle);
        content.threadIdentifier   = juceStringToNS (n.groupId);
        content.body               = juceStringToNS (n.body);
        content.categoryIdentifier = juceStringToNS (n.category);
        content.badge              = [NSNumber numberWithInt: n.badgeNumber];

        auto soundToPlayString = n.soundToPlay.toString (true);

        if (soundToPlayString == "default_os_sound")
            content.sound = [UNNotificationSound defaultSound];
        else if (soundToPlayString.isNotEmpty())
            content.sound = [UNNotificationSound soundNamed: juceStringToNS (soundToPlayString)];

        auto* propsDict = (NSMutableDictionary*) [varToNSDictionary (n.properties) mutableCopy];
        [propsDict setObject: juceStringToNS (soundToPlayString) forKey: @"com.drx.soundName"];
        content.userInfo = propsDict;

        // trigger
        UNTimeIntervalNotificationTrigger* trigger = nil;

        if (std::abs (n.triggerIntervalSec) >= 0.001)
        {
            BOOL shouldRepeat = n.repeat && n.triggerIntervalSec >= 60;
            trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval: n.triggerIntervalSec repeats: shouldRepeat];
        }

        // request
        // each notification needs to have an identifier, otherwise it will not show up
        jassert (n.identifier.isNotEmpty());
        UNNotificationRequest* request = [UNNotificationRequest requestWithIdentifier: juceStringToNS (n.identifier)
                                                                              content: content
                                                                              trigger: trigger];

        [content autorelease];

        return request;
    }

    static Txt getUserResponseFromNSDictionary (NSDictionary* dictionary)
    {
        if (dictionary == nil || dictionary.count == 0)
            return {};

        jassert (dictionary.count == 1);

        for (NSString* key in dictionary)
        {
            const auto keyString = nsStringToDrx (key);

            id value = dictionary[key];

            if ([value isKindOfClass: [NSString class]])
                return nsStringToDrx ((NSString*) value);
        }

        jassertfalse;
        return {};
    }

    //==============================================================================
    static var getNotificationPropertiesFromDictionaryVar (const var& dictionaryVar)
    {
        DynamicObject* dictionaryVarObject = dictionaryVar.getDynamicObject();

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

    //==============================================================================
    static f64 getIntervalSecFromUNNotificationTrigger (UNNotificationTrigger* t)
    {
        if (t != nil)
        {
            if ([t isKindOfClass: [UNTimeIntervalNotificationTrigger class]])
            {
                auto* trigger = (UNTimeIntervalNotificationTrigger*) t;
                return trigger.timeInterval;
            }
            else if ([t isKindOfClass: [UNCalendarNotificationTrigger class]])
            {
                auto* trigger = (UNCalendarNotificationTrigger*) t;
                NSDate* date    = [trigger.dateComponents date];
                NSDate* dateNow = [NSDate date];
                return [dateNow timeIntervalSinceDate: date];
            }
        }

        return 0.;
    }

    static PushNotifications::Notification unNotificationRequestToDrxNotification (UNNotificationRequest* r)
    {
        PushNotifications::Notification n;

        n.identifier = nsStringToDrx (r.identifier);
        n.title      = nsStringToDrx (r.content.title);
        n.subtitle   = nsStringToDrx (r.content.subtitle);
        n.body       = nsStringToDrx (r.content.body);
        n.groupId    = nsStringToDrx (r.content.threadIdentifier);
        n.category   = nsStringToDrx (r.content.categoryIdentifier);
        n.badgeNumber = r.content.badge.intValue;

        auto userInfoVar = nsDictionaryToVar (r.content.userInfo);

        if (auto* object = userInfoVar.getDynamicObject())
        {
            static const Identifier soundName ("com.drx.soundName");
            n.soundToPlay = URL (object->getProperty (soundName).toString());
            object->removeProperty (soundName);
        }

        n.properties = userInfoVar;

        n.triggerIntervalSec = getIntervalSecFromUNNotificationTrigger (r.trigger);
        n.repeat = r.trigger != nil && r.trigger.repeats;

        return n;
    }

    static PushNotifications::Notification unNotificationToDrxNotification (UNNotification* n)
    {
        return unNotificationRequestToDrxNotification (n.request);
    }

    static Action unNotificationActionToAction (UNNotificationAction* a)
    {
        Action action;

        action.identifier = nsStringToDrx (a.identifier);
        action.title      = nsStringToDrx (a.title);
        action.triggerInBackground = ! (a.options & UNNotificationActionOptionForeground);
        action.destructive         =    a.options & UNNotificationActionOptionDestructive;

        if ([a isKindOfClass: [UNTextInputNotificationAction class]])
        {
            auto* textAction = (UNTextInputNotificationAction*)a;

            action.style = Action::text;
            action.textInputButtonText  = nsStringToDrx (textAction.textInputButtonTitle);
            action.textInputPlaceholder = nsStringToDrx (textAction.textInputPlaceholder);
        }
        else
        {
            action.style = Action::button;
        }

        return action;
    }

    static Category unNotificationCategoryToCategory (UNNotificationCategory* c)
    {
        Category category;

        category.identifier = nsStringToDrx (c.identifier);
        category.sendDismissAction = c.options & UNNotificationCategoryOptionCustomDismissAction;

        for (UNNotificationAction* a in c.actions)
            category.actions.add (unNotificationActionToAction (a));

        return category;
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

private:
    ~PushNotificationsDelegateDetails() = delete;
};

//==============================================================================
b8 PushNotifications::Notification::isValid() const noexcept
{
    return title.isNotEmpty() && body.isNotEmpty() && identifier.isNotEmpty() && category.isNotEmpty();
}

//==============================================================================
struct PushNotifications::Impl
{
    explicit Impl (PushNotifications& p)
        : owner (p)
    {
        Class::setThis (delegate.get(), this);

        auto appDelegate = [[UIApplication sharedApplication] delegate];

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        if ([appDelegate respondsToSelector: @selector (setPushNotificationsDelegateToUse:)])
            [appDelegate performSelector: @selector (setPushNotificationsDelegateToUse:) withObject: delegate.get()];
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
    }

    z0 requestPermissionsWithSettings (const PushNotifications::Settings& settingsToUse)
    {
        settings = settingsToUse;

        auto categories = [NSMutableSet setWithCapacity: (NSUInteger) settings.categories.size()];

        for (const auto& c : settings.categories)
        {
            auto* category = (UNNotificationCategory*) PushNotificationsDelegateDetails::categoryToNSCategory (c);
            [categories addObject: category];
        }

        UNAuthorizationOptions authOptions = NSUInteger ((b8)settings.allowBadge << 0
                                                       | (b8)settings.allowSound << 1
                                                       | (b8)settings.allowAlert << 2);

        [[UNUserNotificationCenter currentNotificationCenter] setNotificationCategories: categories];
        [[UNUserNotificationCenter currentNotificationCenter] requestAuthorizationWithOptions: authOptions
                                                                            completionHandler: ^(BOOL /*granted*/, NSError* /*error*/)
                                                                                               {
                                                                                                   requestSettingsUsed();
                                                                                               }];

        [[UIApplication sharedApplication] registerForRemoteNotifications];
    }

    z0 requestSettingsUsed()
    {
        [[UNUserNotificationCenter currentNotificationCenter] getNotificationSettingsWithCompletionHandler:
         ^(UNNotificationSettings* s)
         {
             [[UNUserNotificationCenter currentNotificationCenter] getNotificationCategoriesWithCompletionHandler:
              ^(NSSet<UNNotificationCategory*>* categories)
              {
                  settings.allowBadge = s.badgeSetting == UNNotificationSettingEnabled;
                  settings.allowSound = s.soundSetting == UNNotificationSettingEnabled;
                  settings.allowAlert = s.alertSetting == UNNotificationSettingEnabled;

                  for (UNNotificationCategory* c in categories)
                      settings.categories.add (PushNotificationsDelegateDetails::unNotificationCategoryToCategory (c));

                  owner.listeners.call ([&] (Listener& l) { l.notificationSettingsReceived (settings); });
              }
             ];
         }];
    }

    b8 areNotificationsEnabled() const { return true; }

    z0 sendLocalNotification (const Notification& n)
    {
        UNNotificationRequest* request = PushNotificationsDelegateDetails::juceNotificationToUNNotificationRequest (n);

        [[UNUserNotificationCenter currentNotificationCenter] addNotificationRequest: request
                                                               withCompletionHandler: ^(NSError* error)
                                                                                      {
                                                                                          jassert (error == nil);

                                                                                          if (error != nil)
                                                                                              NSLog (nsStringLiteral ("addNotificationRequest error: %@"), error);
                                                                                      }];
    }

    z0 getDeliveredNotifications() const
    {
        [[UNUserNotificationCenter currentNotificationCenter] getDeliveredNotificationsWithCompletionHandler:
         ^(NSArray<UNNotification*>* notifications)
         {
            Array<PushNotifications::Notification> notifs;

            for (UNNotification* n in notifications)
                notifs.add (PushNotificationsDelegateDetails::unNotificationToDrxNotification (n));

            owner.listeners.call ([&] (Listener& l) { l.deliveredNotificationsListReceived (notifs); });
         }];
    }

    z0 removeAllDeliveredNotifications()
    {
        [[UNUserNotificationCenter currentNotificationCenter] removeAllDeliveredNotifications];
    }

    z0 removeDeliveredNotification ([[maybe_unused]] const Txt& identifier)
    {
        NSArray<NSString*>* identifiers = [NSArray arrayWithObject: juceStringToNS (identifier)];

        [[UNUserNotificationCenter currentNotificationCenter] removeDeliveredNotificationsWithIdentifiers: identifiers];
    }

    z0 setupChannels ([[maybe_unused]] const Array<ChannelGroup>& groups, [[maybe_unused]] const Array<Channel>& channels)
    {
    }

    z0 getPendingLocalNotifications() const
    {
        [[UNUserNotificationCenter currentNotificationCenter] getPendingNotificationRequestsWithCompletionHandler:
         ^(NSArray<UNNotificationRequest*>* requests)
         {
             Array<PushNotifications::Notification> notifs;

             for (UNNotificationRequest* r : requests)
                 notifs.add (PushNotificationsDelegateDetails::unNotificationRequestToDrxNotification (r));

             owner.listeners.call ([&] (Listener& l) { l.pendingLocalNotificationsListReceived (notifs); });
         }];
    }

    z0 removePendingLocalNotification (const Txt& identifier)
    {
        NSArray<NSString*>* identifiers = [NSArray arrayWithObject: juceStringToNS (identifier)];
        [[UNUserNotificationCenter currentNotificationCenter] removePendingNotificationRequestsWithIdentifiers: identifiers];
    }

    z0 removeAllPendingLocalNotifications()
    {
        [[UNUserNotificationCenter currentNotificationCenter] removeAllPendingNotificationRequests];
    }

    Txt getDeviceToken()
    {
        // You need to call requestPermissionsWithSettings() first.
        jassert (initialised);

        return deviceToken;
    }

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
    //==============================================================================
    z0 registeredForRemoteNotifications (NSData* deviceTokenToUse)
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

    z0 failedToRegisterForRemoteNotifications ([[maybe_unused]] NSError* error)
    {
        deviceToken.clear();
    }

    z0 didReceiveRemoteNotification (NSDictionary* userInfo)
    {
        auto n = PushNotificationsDelegateDetails::nsDictionaryToDrxNotification (userInfo);

        owner.listeners.call ([&] (Listener& l) { l.handleNotification (false, n); });
    }

    z0 didReceiveRemoteNotificationFetchCompletionHandler (NSDictionary* userInfo,
                                                             z0 (^completionHandler)(UIBackgroundFetchResult result))
    {
        didReceiveRemoteNotification (userInfo);
        completionHandler (UIBackgroundFetchResultNewData);
    }

    z0 handleActionForRemoteNotificationCompletionHandler (NSString* actionIdentifier,
                                                             NSDictionary* userInfo,
                                                             NSDictionary* responseInfo,
                                                             z0 (^completionHandler)())
    {
        auto n = PushNotificationsDelegateDetails::nsDictionaryToDrxNotification (userInfo);
        auto actionString = nsStringToDrx (actionIdentifier);
        auto response = PushNotificationsDelegateDetails::getUserResponseFromNSDictionary (responseInfo);

        owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (false, n, actionString, response); });

        completionHandler();
    }

    z0 willPresentNotificationWithCompletionHandler ([[maybe_unused]] UNNotification* notification,
                                                       z0 (^completionHandler)(UNNotificationPresentationOptions options))
    {
        NSUInteger options = NSUInteger ((i32)settings.allowBadge << 0
                                       | (i32)settings.allowSound << 1
                                       | (i32)settings.allowAlert << 2);

        completionHandler (options);
    }

    z0 didReceiveNotificationResponseWithCompletionHandler (UNNotificationResponse* response,
                                                              z0 (^completionHandler)())
    {
        const b8 remote = [response.notification.request.trigger isKindOfClass: [UNPushNotificationTrigger class]];

        auto actionString = nsStringToDrx (response.actionIdentifier);

        if (actionString == "com.apple.UNNotificationDefaultActionIdentifier")
            actionString.clear();
        else if (actionString == "com.apple.UNNotificationDismissActionIdentifier")
            actionString = "com.drx.NotificationDeleted";

        auto n = PushNotificationsDelegateDetails::unNotificationToDrxNotification (response.notification);

        Txt responseString;

        if ([response isKindOfClass: [UNTextInputNotificationResponse class]])
        {
            UNTextInputNotificationResponse* textResponse = (UNTextInputNotificationResponse*)response;
            responseString = nsStringToDrx (textResponse.userText);
        }

        owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (! remote, n, actionString, responseString); });
        completionHandler();
    }

    //==============================================================================
    struct Class final : public ObjCClass<NSObject<UIApplicationDelegate, UNUserNotificationCenterDelegate>>
    {
        Class()
            : ObjCClass ("DrxPushNotificationsDelegate_")
        {
            addIvar<Impl*> ("self");

            addMethod (@selector (application:didRegisterForRemoteNotificationsWithDeviceToken:), [] (id self, SEL, UIApplication*, NSData* data)
            {
                getThis (self).registeredForRemoteNotifications (data);
            });

            addMethod (@selector (application:didFailToRegisterForRemoteNotificationsWithError:), [] (id self, SEL, UIApplication*, NSError* error)
            {
                getThis (self).failedToRegisterForRemoteNotifications (error);
            });

            addMethod (@selector (application:didReceiveRemoteNotification:), [] (id self, SEL, UIApplication*, NSDictionary* userInfo)
            {
                getThis (self).didReceiveRemoteNotification (userInfo);
            });

            addMethod (@selector (application:didReceiveRemoteNotification:fetchCompletionHandler:), [] (id self, SEL, UIApplication*, NSDictionary* userInfo, z0 (^completionHandler)(UIBackgroundFetchResult result))
            {
                getThis (self).didReceiveRemoteNotificationFetchCompletionHandler (userInfo, completionHandler);
            });

            addMethod (@selector (application:handleActionWithIdentifier:forRemoteNotification:withResponseInfo:completionHandler:), [] (id self, SEL, UIApplication*, NSString* actionIdentifier, NSDictionary* userInfo, NSDictionary* responseInfo, z0 (^completionHandler)())
            {
                getThis (self).handleActionForRemoteNotificationCompletionHandler (actionIdentifier, userInfo, responseInfo, completionHandler);
            });

            addMethod (@selector (userNotificationCenter:willPresentNotification:withCompletionHandler:), [] (id self, SEL, UNUserNotificationCenter*, UNNotification* notification, z0 (^completionHandler)(UNNotificationPresentationOptions options))
            {
                getThis (self).willPresentNotificationWithCompletionHandler (notification, completionHandler);
            });

            addMethod (@selector (userNotificationCenter:didReceiveNotificationResponse:withCompletionHandler:), [] (id self, SEL, UNUserNotificationCenter*, UNNotificationResponse* response, z0 (^completionHandler)())
            {
                getThis (self).didReceiveNotificationResponseWithCompletionHandler (response, completionHandler);
            });

            registerClass();
        }

        //==============================================================================
        static Impl& getThis (id self)          { return *getIvar<Impl*> (self, "self"); }
        static z0 setThis (id self, Impl* d)  { object_setInstanceVariable (self, "self", d); }
    };

    //==============================================================================
    static Class& getClass()
    {
        static Class c;
        return c;
    }

    NSUniquePtr<NSObject<UIApplicationDelegate, UNUserNotificationCenterDelegate>> delegate { [getClass().createInstance() init] };

    PushNotifications& owner;

    b8 initialised = false;
    Txt deviceToken;

    PushNotifications::Settings settings;
};

} // namespace drx
