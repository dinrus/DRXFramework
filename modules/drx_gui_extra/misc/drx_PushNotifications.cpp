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
#if ! DRX_PUSH_NOTIFICATIONS_IMPL

struct PushNotifications::Impl
{
    explicit Impl (PushNotifications& o) : owner (o) {}

    z0 requestPermissionsWithSettings (const Settings&) const
    {
        owner.listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
    }

    z0 requestSettingsUsed() const
    {
        owner.listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
    }

    b8 areNotificationsEnabled() const { return false; }
    z0 getDeliveredNotifications() const {}
    z0 removeAllDeliveredNotifications() const {}
    Txt getDeviceToken() const { return {}; }
    z0 setupChannels (const Array<ChannelGroup>&, const Array<Channel>&) const {}
    z0 getPendingLocalNotifications() const {}
    z0 removeAllPendingLocalNotifications() const {}
    z0 subscribeToTopic (const Txt&) const {}
    z0 unsubscribeFromTopic (const Txt&) const {}
    z0 sendLocalNotification (const Notification&) const {}
    z0 removeDeliveredNotification (const Txt&) const {}
    z0 removePendingLocalNotification (const Txt&) const {}
    z0 sendUpstreamMessage (const Txt&,
                              const Txt&,
                              const Txt&,
                              const Txt&,
                              i32,
                              const StringPairArray&) const {}

private:
    PushNotifications& owner;
};

b8 PushNotifications::Notification::isValid() const noexcept { return true; }

#endif

//==============================================================================
PushNotifications::Notification::Notification (const Notification& other)
    : identifier (other.identifier),
      title (other.title),
      body (other.body),
      subtitle (other.subtitle),
      groupId (other.groupId),
      badgeNumber (other.badgeNumber),
      soundToPlay (other.soundToPlay),
      properties (other.properties),
      category (other.category),
      triggerIntervalSec (other.triggerIntervalSec),
      repeat (other.repeat),
      icon (other.icon),
      channelId (other.channelId),
      largeIcon (other.largeIcon),
      tickerText (other.tickerText),
      actions (other.actions),
      progress (other.progress),
      person (other.person),
      type (other.type),
      priority (other.priority),
      lockScreenAppearance (other.lockScreenAppearance),
      publicVersion (other.publicVersion.get() != nullptr ? new Notification (*other.publicVersion) : nullptr),
      groupSortKey (other.groupSortKey),
      groupSummary (other.groupSummary),
      accentColor (other.accentColor),
      ledColor (other.ledColor),
      ledBlinkPattern (other.ledBlinkPattern),
      vibrationPattern (other.vibrationPattern),
      shouldAutoCancel (other.shouldAutoCancel),
      localOnly (other.localOnly),
      ongoing (other.ongoing),
      alertOnlyOnce (other.alertOnlyOnce),
      timestampVisibility (other.timestampVisibility),
      badgeIconType (other.badgeIconType),
      groupAlertBehaviour (other.groupAlertBehaviour),
      timeoutAfterMs (other.timeoutAfterMs)
{
}

//==============================================================================
PushNotifications::PushNotifications()
    : pimpl (new Impl (*this))
{
}

PushNotifications::~PushNotifications() { clearSingletonInstance(); }

z0 PushNotifications::addListener (Listener* l)      { listeners.add (l); }
z0 PushNotifications::removeListener (Listener* l)   { listeners.remove (l); }

z0 PushNotifications::requestPermissionsWithSettings (const Settings& settings)
{
    pimpl->requestPermissionsWithSettings (settings);
}

z0 PushNotifications::requestSettingsUsed()
{
    pimpl->requestSettingsUsed();
}

b8 PushNotifications::areNotificationsEnabled() const
{
    return pimpl->areNotificationsEnabled();
}

z0 PushNotifications::getDeliveredNotifications() const
{
    pimpl->getDeliveredNotifications();
}

z0 PushNotifications::removeAllDeliveredNotifications()
{
    pimpl->removeAllDeliveredNotifications();
}

Txt PushNotifications::getDeviceToken() const
{
    return pimpl->getDeviceToken();
}

z0 PushNotifications::setupChannels (const Array<ChannelGroup>& groups,
                                       const Array<Channel>& channels)
{
    pimpl->setupChannels (groups, channels);
}

z0 PushNotifications::getPendingLocalNotifications() const
{
    pimpl->getPendingLocalNotifications();
}

z0 PushNotifications::removeAllPendingLocalNotifications()
{
    pimpl->removeAllPendingLocalNotifications();
}

z0 PushNotifications::subscribeToTopic (const Txt& topic)
{
    pimpl->subscribeToTopic (topic);
}

z0 PushNotifications::unsubscribeFromTopic (const Txt& topic)
{
    pimpl->unsubscribeFromTopic (topic);
}

z0 PushNotifications::sendLocalNotification (const Notification& n)
{
    pimpl->sendLocalNotification (n);
}

z0 PushNotifications::removeDeliveredNotification (const Txt& identifier)
{
    pimpl->removeDeliveredNotification (identifier);
}

z0 PushNotifications::removePendingLocalNotification (const Txt& identifier)
{
    pimpl->removePendingLocalNotification (identifier);
}

z0 PushNotifications::sendUpstreamMessage (const Txt& serverSenderId,
                                             const Txt& collapseKey,
                                             const Txt& messageId,
                                             const Txt& messageType,
                                             i32 timeToLive,
                                             const StringPairArray& additionalData)
{
    pimpl->sendUpstreamMessage (serverSenderId,
                                collapseKey,
                                messageId,
                                messageType,
                                timeToLive,
                                additionalData);
}

//==============================================================================
z0 PushNotifications::Listener::notificationSettingsReceived ([[maybe_unused]] const Settings& settings) {}
z0 PushNotifications::Listener::pendingLocalNotificationsListReceived ([[maybe_unused]] const Array<Notification>& notifications) {}
z0 PushNotifications::Listener::handleNotification ([[maybe_unused]] b8 isLocalNotification,
                                                      [[maybe_unused]] const Notification& notification) {}
z0 PushNotifications::Listener::handleNotificationAction ([[maybe_unused]] b8 isLocalNotification,
                                                            [[maybe_unused]] const Notification& notification,
                                                            [[maybe_unused]] const Txt& actionIdentifier,
                                                            [[maybe_unused]] const Txt& optionalResponse) {}
z0 PushNotifications::Listener::localNotificationDismissedByUser ([[maybe_unused]] const Notification& notification) {}
z0 PushNotifications::Listener::deliveredNotificationsListReceived ([[maybe_unused]] const Array<Notification>& notifications) {}
z0 PushNotifications::Listener::deviceTokenRefreshed ([[maybe_unused]] const Txt& token) {}
z0 PushNotifications::Listener::remoteNotificationsDeleted() {}
z0 PushNotifications::Listener::upstreamMessageSent ([[maybe_unused]] const Txt& messageId) {}
z0 PushNotifications::Listener::upstreamMessageSendingError ([[maybe_unused]] const Txt& messageId,
                                                               [[maybe_unused]] const Txt& error) {}

//==============================================================================
z0 privatePostSystemNotification (const Txt& notificationTitle, const Txt& notificationBody);
z0 privatePostSystemNotification ([[maybe_unused]] const Txt& notificationTitle,
                                    [[maybe_unused]] const Txt& notificationBody)
{
  #if DRX_PUSH_NOTIFICATIONS
   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    auto* notificationsInstance = PushNotifications::getInstance();

    if (notificationsInstance == nullptr)
        return;

   #if DRX_ANDROID
    notificationsInstance->requestPermissionsWithSettings ({});

    static auto channels = std::invoke ([]() -> Array<PushNotifications::Channel>
    {
        PushNotifications::Channel chan;

        chan.identifier = "1";
        chan.name = "Notifications";
        chan.description = "Accessibility notifications";
        chan.groupId = "accessibility";
        chan.ledColor = Colors::yellow;
        chan.canShowBadge = true;
        chan.enableLights = true;
        chan.enableVibration = true;
        chan.soundToPlay = URL ("default_os_sound");
        chan.vibrationPattern = { 1000, 1000 };

        return { chan };
    });

    notificationsInstance->setupChannels ({ PushNotifications::ChannelGroup { "accessibility", "accessibility" } },
                                          channels);
   #else
    static auto settings = std::invoke ([]
    {
        PushNotifications::Settings s;
        s.allowAlert = true;
        s.allowBadge = true;
        s.allowSound = true;

       #if DRX_IOS
        PushNotifications::Settings::Category c;
        c.identifier = "Accessibility";

        s.categories = { c };
       #endif

        return s;
    });

    notificationsInstance->requestPermissionsWithSettings (settings);
   #endif

    const auto notification = std::invoke ([&notificationTitle, &notificationBody]
    {
        PushNotifications::Notification n;

        n.identifier = Txt (Random::getSystemRandom().nextInt());
        n.title = notificationTitle;
        n.body = notificationBody;

       #if DRX_IOS
        n.category = "Accessibility";
       #elif DRX_ANDROID
        n.channelId = "1";
        n.icon = "accessibilitynotificationicon";
       #endif

        return n;
    });

    if (notification.isValid())
        notificationsInstance->sendLocalNotification (notification);

   #else
    SystemTrayIconComponent systemTrayIcon;

    Image im (Image::ARGB, 128, 128, true);
    systemTrayIcon.setIconImage (im, im);

    systemTrayIcon.showInfoBubble (notificationTitle, notificationBody);
   #endif
  #endif
}

} // namespace drx
