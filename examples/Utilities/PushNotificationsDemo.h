/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             PushNotificationsDemo
 version:          2.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases push notifications features. To run this demo you must enable the
                   "Push Notifications Capability" option in the Projucer exporter.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra
 exporters:        xcode_mac, xcode_iphone, androidstudio

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1
                   DRX_PUSH_NOTIFICATIONS=1

 type:             Component
 mainClass:        PushNotificationsDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

/*
    To finish the setup of this demo, do the following:

1. Download google_services.json from your Firebase project.
2. Update "Remote Notifications Config File" path in Android exporter (this can be different for debug and release)
   to point to that json file.
3. Add image and sound resources by adding the following to "Extra Android Raw Resources" in Projucer:

../../Assets/Notifications/images/ic_stat_name.png
../../Assets/Notifications/images/ic_stat_name2.png
../../Assets/Notifications/images/ic_stat_name3.png
../../Assets/Notifications/images/ic_stat_name4.png
../../Assets/Notifications/images/ic_stat_name5.png
../../Assets/Notifications/images/ic_stat_name6.png
../../Assets/Notifications/images/ic_stat_name7.png
../../Assets/Notifications/images/ic_stat_name8.png
../../Assets/Notifications/images/ic_stat_name9.png
../../Assets/Notifications/images/ic_stat_name10.png
../../Assets/Notifications/sounds/demonstrative.mp3
../../Assets/Notifications/sounds/isntit.mp3
../../Assets/Notifications/sounds/jinglebellssms.mp3
../../Assets/Notifications/sounds/served.mp3
../../Assets/Notifications/sounds/solemn.mp3

4. Set "Remote Notifications" to enabled in Projucer Android exporter.

To verify that remote notifications are configured properly, go to Remote tab in the demo and press "GetDeviceToken"
button, a dialog with your token (also printed to console in debug build) should show up.


The following steps are only necessary if you have a custom activity defined:

5. Ensure that its launchMode is set to "singleTop" or "singleTask" in Android manifest. This is the default behaviour
   in DRX so you only need to do it if you have custom Android manifest content. You can do it from Projucer by
   ensuring that "Custom Manifest XML Content" contains:

<manifest>
<application>
<activity android:launchMode="singleTask">
</activity>
</application>
</manifest>

6. Ensure that you override onNewIntent() function in the same way as it is done in DrxActivity.java:

package com.rmsl.drx;

import android.app.Activity;
import android.content.Intent;

//==============================================================================
public class DrxActivity   extends Activity
{
    //==============================================================================
    private native z0 appNewIntent (Intent intent);

    @Override
    protected z0 onNewIntent (Intent intent)
    {
        super.onNewIntent (intent);
        setIntent (intent);

        appNewIntent (intent);
    }
}

*/

//==============================================================================
class PushNotificationsDemo final : public Component,
                                    private ChangeListener,
                                    private ComponentListener,
                                    private PushNotifications::Listener
{
public:
    //==============================================================================
    PushNotificationsDemo()
    {
        setupControls();
        distributeControls();

      #if DRX_PUSH_NOTIFICATIONS
        addAndMakeVisible (headerLabel);
        addAndMakeVisible (mainTabs);
        addAndMakeVisible (sendButton);
      #else
        addAndMakeVisible (notAvailableYetLabel);
      #endif

        headerLabel         .setJustificationType (Justification::centred);
        notAvailableYetLabel.setJustificationType (Justification::centred);

      #if DRX_MAC
        StringArray tabNames { "Params1", "Params2", "Params3", "Params4" };
      #else
        StringArray tabNames { "Req. params", "Opt. params1", "Opt. params2", "Opt. params3" };
      #endif

        auto colour = getLookAndFeel().findColor (ResizableWindow::backgroundColorId);
        localNotificationsTabs.addTab (tabNames[0], colour, &paramsOneView, false);
        localNotificationsTabs.addTab (tabNames[1], colour, &paramsTwoView, false);
      #if DRX_ANDROID
        localNotificationsTabs.addTab (tabNames[2], colour, &paramsThreeView, false);
        localNotificationsTabs.addTab (tabNames[3], colour, &paramsFourView,  false);
      #endif
        localNotificationsTabs.addTab ("Aux. actions", colour, &auxActionsView, false);

        mainTabs.addTab ("Local",  colour, &localNotificationsTabs, false);
        mainTabs.addTab ("Remote", colour, &remoteView,             false);

        auto userArea = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
      #if DRX_ANDROID || DRX_IOS
        setSize (userArea.getWidth(), userArea.getHeight());
      #else
        setSize (userArea.getWidth() / 2, userArea.getHeight() / 2);
      #endif

        sendButton.onClick = [this] { sendLocalNotification(); };
        auxActionsView.getDeliveredNotificationsButton .onClick = []
            { PushNotifications::getInstance()->getDeliveredNotifications(); };
        auxActionsView.removeDeliveredNotifWithIdButton.onClick = [this]
            { PushNotifications::getInstance()->removeDeliveredNotification (auxActionsView.deliveredNotifIdentifier.getText()); };
        auxActionsView.removeAllDeliveredNotifsButton  .onClick = []
            { PushNotifications::getInstance()->removeAllDeliveredNotifications(); };
      #if DRX_IOS || DRX_MAC
        auxActionsView.getPendingNotificationsButton .onClick = []
            { PushNotifications::getInstance()->getPendingLocalNotifications(); };
        auxActionsView.removePendingNotifWithIdButton.onClick = [this]
            { PushNotifications::getInstance()->removePendingLocalNotification (auxActionsView.pendingNotifIdentifier.getText()); };
        auxActionsView.removeAllPendingNotifsButton  .onClick = []
            { PushNotifications::getInstance()->removeAllPendingLocalNotifications(); };
      #endif

        remoteView.getDeviceTokenButton.onClick = [this]
        {
            Txt token = PushNotifications::getInstance()->getDeviceToken();

            DBG ("token = " + token);

            if (token.isEmpty())
            {
                showRemoteInstructions();
            }
            else
            {
                auto options = MessageBoxOptions()
                                               .withIconType (MessageBoxIconType::InfoIcon)
                                               .withTitle ("Device token")
                                               .withMessage (token)
                                               .withButton ("OK");
                messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
            }
        };

      #if DRX_ANDROID
        remoteView.sendRemoteMessageButton.onClick = []
        {
            StringPairArray data;
            data.set ("key1", "value1");
            data.set ("key2", "value2");

            static i32 id = 100;
            PushNotifications::getInstance()->sendUpstreamMessage ("872047750958",
                                                                   "com.drx.pushnotificationsdemo",
                                                                   Txt (id++),
                                                                   "standardType",
                                                                   3600,
                                                                   data);
        };

        remoteView.subscribeToSportsButton    .onClick = []
            { PushNotifications::getInstance()->subscribeToTopic ("sports"); };
        remoteView.unsubscribeFromSportsButton.onClick = []
            { PushNotifications::getInstance()->unsubscribeFromTopic ("sports"); };
      #endif

        paramControls.accentColorButton.onClick = [this] { setupAccentColor(); };
        paramControls.ledColorButton   .onClick = [this] { setupLedColor(); };

        jassert (PushNotifications::getInstance()->areNotificationsEnabled());

        PushNotifications::getInstance()->addListener (this);

      #if DRX_IOS || DRX_MAC
        paramControls.fireInComboBox.onChange = [this] { delayNotification(); };
        PushNotifications::getInstance()->requestPermissionsWithSettings (getNotificationSettings());
      #elif DRX_ANDROID
        PushNotifications::ChannelGroup cg { "demoGroup", "demo group" };
        PushNotifications::getInstance()->setupChannels ({ { cg } }, getAndroidChannels());
      #endif

       #if DRX_IOS || DRX_ANDROID
        setPortraitOrientationEnabled (true);
       #endif
    }

    ~PushNotificationsDemo() override
    {
        PushNotifications::getInstance()->removeListener (this);

       #if DRX_IOS || DRX_ANDROID
        setPortraitOrientationEnabled (false);
       #endif
    }

    z0 setPortraitOrientationEnabled (b8 shouldBeEnabled)
    {
        auto allowedOrientations = Desktop::getInstance().getOrientationsEnabled();

        if (shouldBeEnabled)
            allowedOrientations |= Desktop::upright;
        else
            allowedOrientations &= ~Desktop::upright;

        Desktop::getInstance().setOrientationsEnabled (allowedOrientations);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds().reduced (getWidth() / 20, getHeight() / 40);

        headerLabel.setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.1f)));
        mainTabs   .setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.8f)));
        sendButton .setBounds (bounds);

        notAvailableYetLabel.setBounds (getLocalBounds());
    }

private:
    z0 delayNotification()
    {
        auto repeatsAllowed = paramControls.fireInComboBox.getSelectedItemIndex() >= 6;

        paramControls.repeatButton.setEnabled (repeatsAllowed);

        if (! repeatsAllowed)
            paramControls.repeatButton.setToggleState (false, NotificationType::sendNotification);
    }

    z0 sendLocalNotification()
    {
        PushNotifications::Notification n;

        fillRequiredParams (n);
        fillOptionalParamsOne (n);
      #if DRX_ANDROID
        fillOptionalParamsTwo (n);
        fillOptionalParamsThree (n);
      #endif

        if (! n.isValid())
        {
          #if DRX_IOS
            Txt requiredFields = "identifier, title, body and category";
          #elif DRX_ANDROID
            Txt requiredFields = "channel ID, title, body and icon";
          #else
            Txt requiredFields = "all required fields";
          #endif

            auto options = MessageBoxOptions()
                                           .withIconType (MessageBoxIconType::InfoIcon)
                                           .withTitle ("Incorrect notifications setup")
                                           .withMessage ("Please make sure that " + requiredFields + " are set.")
                                           .withButton ("OK");
            messageBox = NativeMessageBox::showScopedAsync (options, nullptr);

            return;
        }

        PushNotifications::getInstance()->sendLocalNotification (n);
    }

    z0 fillRequiredParams      (PushNotifications::Notification& n)
    {
        n.identifier = paramControls.identifierEditor.getText();
        n.title      = paramControls.titleEditor     .getText();
        n.body       = paramControls.bodyEditor      .getText();
      #if DRX_IOS
        n.category = paramControls.categoryComboBox.getText();
      #elif DRX_ANDROID || DRX_MAC
       #if DRX_MAC
        Txt prefix = "Notifications/images/";
        Txt extension = ".png";
       #else
        Txt prefix;
        Txt extension;
       #endif
        if (paramControls.iconComboBox.getSelectedItemIndex() == 0)
            n.icon = prefix + "ic_stat_name" + extension;
        else if (paramControls.iconComboBox.getSelectedItemIndex() == 1)
            n.icon = prefix + "ic_stat_name2" + extension;
        else if (paramControls.iconComboBox.getSelectedItemIndex() == 2)
            n.icon = prefix + "ic_stat_name3" + extension;
        else if (paramControls.iconComboBox.getSelectedItemIndex() == 3)
            n.icon = prefix + "ic_stat_name4" + extension;
        else if (paramControls.iconComboBox.getSelectedItemIndex() == 4)
            n.icon = prefix + "ic_stat_name5" + extension;
      #endif

      #if DRX_ANDROID
        // Note: this is not strictly speaking required param, just doing it here because it is the fastest way!
        n.publicVersion.reset (new PushNotifications::Notification());
        n.publicVersion->identifier = "blahblahblah";
        n.publicVersion->title      = "Public title!";
        n.publicVersion->body       = "Public body!";
        n.publicVersion->icon       = n.icon;

        n.channelId = Txt (paramControls.channelIdComboBox.getSelectedItemIndex() + 1);
      #endif
    }

    z0 fillOptionalParamsOne   (PushNotifications::Notification& n)
    {
        n.subtitle = paramControls.subtitleEditor.getText();
        n.badgeNumber = paramControls.badgeNumberComboBox.getSelectedItemIndex();

        if (paramControls.soundToPlayComboBox.getSelectedItemIndex() > 0)
            n.soundToPlay = URL (paramControls.soundToPlayComboBox.getItemText (paramControls.soundToPlayComboBox.getSelectedItemIndex()));

        n.properties = JSON::parse (paramControls.propertiesEditor.getText());

      #if DRX_IOS || DRX_MAC
        n.triggerIntervalSec = f64 (paramControls.fireInComboBox.getSelectedItemIndex() * 10);
        n.repeat = paramControls.repeatButton.getToggleState();
      #elif DRX_ANDROID
        if (paramControls.largeIconComboBox.getSelectedItemIndex() == 1)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name6.png");
        else if (paramControls.largeIconComboBox.getSelectedItemIndex() == 2)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name7.png");
        else if (paramControls.largeIconComboBox.getSelectedItemIndex() == 3)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name8.png");
        else if (paramControls.largeIconComboBox.getSelectedItemIndex() == 4)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name9.png");
        else if (paramControls.largeIconComboBox.getSelectedItemIndex() == 5)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name10.png");

        n.badgeIconType = (PushNotifications::Notification::BadgeIconType) paramControls.badgeIconComboBox.getSelectedItemIndex();
        n.tickerText  = paramControls.tickerTextEditor.getText();

        n.shouldAutoCancel = paramControls.autoCancelButton   .getToggleState();
        n.alertOnlyOnce    = paramControls.alertOnlyOnceButton.getToggleState();
      #endif

      #if DRX_ANDROID || DRX_MAC
        if (paramControls.actionsComboBox.getSelectedItemIndex() == 1)
        {
            PushNotifications::Notification::Action a, a2;
            a .style = PushNotifications::Notification::Action::button;
            a2.style = PushNotifications::Notification::Action::button;
            a .title = a .identifier = "Ok";
            a2.title = a2.identifier = "Cancel";
            n.actions.add (a);
            n.actions.add (a2);
        }
        else if (paramControls.actionsComboBox.getSelectedItemIndex() == 2)
        {
            PushNotifications::Notification::Action a, a2;
            a .title = a .identifier = "Input Text Here";
            a2.title = a2.identifier = "No";
            a .style = PushNotifications::Notification::Action::text;
            a2.style = PushNotifications::Notification::Action::button;
            a .icon = "ic_stat_name4";
            a2.icon = "ic_stat_name5";
            a.textInputPlaceholder = "placeholder text ...";
            n.actions.add (a);
            n.actions.add (a2);
        }
        else if (paramControls.actionsComboBox.getSelectedItemIndex() == 3)
        {
            PushNotifications::Notification::Action a, a2;
            a .title = a .identifier = "Ok";
            a2.title = a2.identifier = "Cancel";
            a .style = PushNotifications::Notification::Action::button;
            a2.style = PushNotifications::Notification::Action::button;
            a .icon = "ic_stat_name4";
            a2.icon = "ic_stat_name5";
            n.actions.add (a);
            n.actions.add (a2);
        }
        else if (paramControls.actionsComboBox.getSelectedItemIndex() == 4)
        {
            PushNotifications::Notification::Action a, a2;
            a .title = a .identifier = "Input Text Here";
            a2.title = a2.identifier = "No";
            a .style = PushNotifications::Notification::Action::text;
            a2.style = PushNotifications::Notification::Action::button;
            a .icon = "ic_stat_name4";
            a2.icon = "ic_stat_name5";
            a.textInputPlaceholder = "placeholder text ...";
            a.allowedResponses.add ("Response 1");
            a.allowedResponses.add ("Response 2");
            a.allowedResponses.add ("Response 3");
            n.actions.add (a);
            n.actions.add (a2);
        }
      #endif
    }

    z0 fillOptionalParamsTwo   (PushNotifications::Notification& n)
    {
        using Notification = PushNotifications::Notification;

        Notification::Progress progress;
        progress.max           = paramControls.progressMaxComboBox    .getSelectedItemIndex() * 10;
        progress.current       = paramControls.progressCurrentComboBox.getSelectedItemIndex() * 10;
        progress.indeterminate = paramControls.progressIndeterminateButton.getToggleState();

        n.progress = progress;
        n.person   = paramControls.personEditor.getText();
        n.type                 = Notification::Type                 (paramControls.categoryComboBox            .getSelectedItemIndex());
        n.priority             = Notification::Priority             (paramControls.priorityComboBox            .getSelectedItemIndex() - 2);
        n.lockScreenAppearance = Notification::LockScreenAppearance (paramControls.lockScreenVisibilityComboBox.getSelectedItemIndex() - 1);
        n.groupId      = paramControls.groupIdEditor.getText();
        n.groupSortKey = paramControls.sortKeyEditor.getText();
        n.groupSummary = paramControls.groupSummaryButton.getToggleState();
        n.groupAlertBehaviour = Notification::GroupAlertBehaviour (paramControls.groupAlertBehaviourComboBox.getSelectedItemIndex());
    }

    z0 fillOptionalParamsThree (PushNotifications::Notification& n)
    {
        n.accentColor = paramControls.accentColorButton.findColor (TextButton::buttonColorId, false);
        n.ledColor    = paramControls.ledColorButton   .findColor (TextButton::buttonColorId, false);

        using Notification = PushNotifications::Notification;
        Notification::LedBlinkPattern ledBlinkPattern;
        ledBlinkPattern.msToBeOn  = paramControls.ledMsToBeOnComboBox .getSelectedItemIndex() * 200;
        ledBlinkPattern.msToBeOff = paramControls.ledMsToBeOffComboBox.getSelectedItemIndex() * 200;
        n.ledBlinkPattern = ledBlinkPattern;

        Array<i32> vibrationPattern;

        if (paramControls.vibratorMsToBeOnComboBox .getSelectedItemIndex() > 0 &&
            paramControls.vibratorMsToBeOffComboBox.getSelectedItemIndex() > 0)
        {
            vibrationPattern.add     (paramControls.vibratorMsToBeOffComboBox.getSelectedItemIndex() * 500);
            vibrationPattern.add     (paramControls.vibratorMsToBeOnComboBox .getSelectedItemIndex() * 500);
            vibrationPattern.add (2 * paramControls.vibratorMsToBeOffComboBox.getSelectedItemIndex() * 500);
            vibrationPattern.add (2 * paramControls.vibratorMsToBeOnComboBox .getSelectedItemIndex() * 500);
        }

        n.vibrationPattern = vibrationPattern;

        n.localOnly = paramControls.localOnlyButton.getToggleState();
        n.ongoing = paramControls.ongoingButton.getToggleState();
        n.timestampVisibility = Notification::TimestampVisibility (paramControls.timestampVisibilityComboBox.getSelectedItemIndex());

        if (paramControls.timeoutAfterComboBox.getSelectedItemIndex() > 0)
        {
            auto index = paramControls.timeoutAfterComboBox.getSelectedItemIndex();
            n.timeoutAfterMs = index * 1000 + 4000;
        }
    }

    z0 setupAccentColor()
    {
        auto accentColorSelector = std::make_unique<ColorSelector>();

        accentColorSelector->setName ("accent colour");
        accentColorSelector->setCurrentColor (paramControls.accentColorButton.findColor (TextButton::buttonColorId));
        accentColorSelector->setColor (ColorSelector::backgroundColorId, Colors::transparentBlack);
        accentColorSelector->setSize (200, 200);
        accentColorSelector->addComponentListener (this);
        accentColorSelector->addChangeListener (this);

        paramControls.accentColorSelector = accentColorSelector.get();

        CallOutBox::launchAsynchronously (std::move (accentColorSelector), paramControls.accentColorButton.getScreenBounds(), nullptr);
    }

    z0 setupLedColor()
    {
        auto ledColorSelector = std::make_unique<ColorSelector>();

        ledColorSelector->setName ("led colour");
        ledColorSelector->setCurrentColor (paramControls.ledColorButton.findColor (TextButton::buttonColorId));
        ledColorSelector->setColor (ColorSelector::backgroundColorId, Colors::transparentBlack);
        ledColorSelector->setSize (200, 200);
        ledColorSelector->addComponentListener (this);
        ledColorSelector->addChangeListener (this);

        paramControls.ledColorSelector = ledColorSelector.get();

        CallOutBox::launchAsynchronously (std::move (ledColorSelector), paramControls.accentColorButton.getScreenBounds(), nullptr);
    }

    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == paramControls.accentColorSelector)
        {
            auto c = paramControls.accentColorSelector->getCurrentColor();
            paramControls.accentColorButton.setColor (TextButton::buttonColorId, c);
        }
        else if (source == paramControls.ledColorSelector)
        {
            auto c = paramControls.ledColorSelector->getCurrentColor();
            paramControls.ledColorButton.setColor (TextButton::buttonColorId, c);
        }
    }

    z0 componentBeingDeleted (Component& component) override
    {
        if (&component == paramControls.accentColorSelector)
            paramControls.accentColorSelector = nullptr;
        else if (&component == paramControls.ledColorSelector)
            paramControls.ledColorSelector = nullptr;
    }

    z0 handleNotification (b8 isLocalNotification, const PushNotifications::Notification& n) override
    {
        ignoreUnused (isLocalNotification);

        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Received notification")
                                       .withMessage ("ID: " + n.identifier
                                                     + ", title: " + n.title
                                                     + ", body: " + n.body)
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
    }

    z0 handleNotificationAction (b8 isLocalNotification,
                                   const PushNotifications::Notification& n,
                                   const drx::Txt& actionIdentifier,
                                   const drx::Txt& optionalResponse) override
    {
        ignoreUnused (isLocalNotification);

        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Received notification action")
                                       .withMessage ("ID: " + n.identifier
                                                     + ", title: " + n.title
                                                     + ", body: " + n.body
                                                     + ", action: " + actionIdentifier
                                                     + ", optionalResponse: " + optionalResponse)
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);

        PushNotifications::getInstance()->removeDeliveredNotification (n.identifier);
    }

    z0 localNotificationDismissedByUser (const PushNotifications::Notification& n) override
    {
        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Notification dismissed by a user")
                                       .withMessage ("ID: " + n.identifier
                                                     + ", title: " + n.title
                                                     + ", body: " + n.body)
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
    }

    z0 deliveredNotificationsListReceived (const Array<PushNotifications::Notification>& notifs) override
    {
        Txt text = "Received notifications: ";

        for (auto& n : notifs)
            text << "(" << n.identifier << ", " << n.title << ", " << n.body << "), ";

        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Received notification list")
                                       .withMessage (text)
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
    }

    z0 pendingLocalNotificationsListReceived (const Array<PushNotifications::Notification>& notifs) override
    {
        Txt text = "Pending notifications: ";

        for (auto& n : notifs)
            text << "(" << n.identifier << ", " << n.title << ", " << n.body << "), ";

        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Pending notification list")
                                       .withMessage (text)
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
    }

    z0 deviceTokenRefreshed (const Txt& token) override
    {
        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Device token refreshed")
                                       .withMessage (token)
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
    }

  #if DRX_ANDROID
    z0 remoteNotificationsDeleted() override
    {
        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Remote notifications deleted")
                                       .withMessage ("Some of the pending messages were removed!")
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
    }

    z0 upstreamMessageSent (const Txt& messageId) override
    {
        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Upstream message sent")
                                       .withMessage ("Message id: " + messageId)
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
    }

    z0 upstreamMessageSendingError (const Txt& messageId, const Txt& error) override
    {
        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Upstream message sending error")
                                       .withMessage ("Message id: " + messageId
                                                     + "\nerror: " + error)
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
    }

    static Array<PushNotifications::Channel> getAndroidChannels()
    {
        using Channel = PushNotifications::Channel;

        Channel ch1, ch2, ch3;

        ch1.identifier = "1";
        ch1.name = "HighImportance";
        ch1.importance = PushNotifications::Channel::max;
        ch1.lockScreenAppearance = PushNotifications::Notification::showCompletely;
        ch1.description = "High Priority Channel for important stuff";
        ch1.groupId = "demoGroup";
        ch1.ledColor = Colors::red;
        ch1.bypassDoNotDisturb = true;
        ch1.canShowBadge = true;
        ch1.enableLights = true;
        ch1.enableVibration = true;
        ch1.soundToPlay = URL ("demonstrative");
        ch1.vibrationPattern = { 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 };

        ch2.identifier = "2";
        ch2.name = "MediumImportance";
        ch2.importance = PushNotifications::Channel::normal;
        ch2.lockScreenAppearance = PushNotifications::Notification::showPartially;
        ch2.description = "Medium Priority Channel for standard stuff";
        ch2.groupId = "demoGroup";
        ch2.ledColor = Colors::yellow;
        ch2.canShowBadge = true;
        ch2.enableLights = true;
        ch2.enableVibration = true;
        ch2.soundToPlay = URL ("default_os_sound");
        ch2.vibrationPattern = { 1000, 1000 };

        ch3.identifier = "3";
        ch3.name = "LowImportance";
        ch3.importance = PushNotifications::Channel::min;
        ch3.lockScreenAppearance = PushNotifications::Notification::dontShow;
        ch3.description = "Low Priority Channel for silly stuff";
        ch3.groupId = "demoGroup";

        return { ch1, ch2, ch3 };
    }

  #elif DRX_IOS || DRX_MAC
    static PushNotifications::Settings getNotificationSettings()
    {
        PushNotifications::Settings settings;
        settings.allowAlert = true;
        settings.allowBadge = true;
        settings.allowSound = true;

      #if DRX_IOS
        using Action   = PushNotifications::Settings::Action;
        using Category = PushNotifications::Settings::Category;

        Action okAction;
        okAction.identifier = "okAction";
        okAction.title = "OK!";
        okAction.style = Action::button;
        okAction.triggerInBackground = true;

        Action cancelAction;
        cancelAction.identifier = "cancelAction";
        cancelAction.title = "Cancel";
        cancelAction.style = Action::button;
        cancelAction.triggerInBackground = true;
        cancelAction.destructive = true;

        Action textAction;
        textAction.identifier = "textAction";
        textAction.title = "Enter text";
        textAction.style = Action::text;
        textAction.triggerInBackground = true;
        textAction.destructive = false;
        textAction.textInputButtonText = "Ok";
        textAction.textInputPlaceholder = "Enter text...";

        Category okCategory;
        okCategory.identifier = "okCategory";
        okCategory.actions = { okAction };

        Category okCancelCategory;
        okCancelCategory.identifier = "okCancelCategory";
        okCancelCategory.actions = { okAction, cancelAction };

        Category textCategory;
        textCategory.identifier = "textCategory";
        textCategory.actions = { textAction };
        textCategory.sendDismissAction = true;

        settings.categories = { okCategory, okCancelCategory, textCategory };
      #endif

        return settings;
    }
  #endif

    struct RowComponent final : public Component
    {
        RowComponent (Label& l, Component& c, i32 u = 1)
            : label (l),
              editor (c),
              rowUnits (u)
        {
            addAndMakeVisible (label);
            addAndMakeVisible (editor);
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds();
            label .setBounds (bounds.removeFromLeft (getWidth() / 3));
            editor.setBounds (bounds);
        }

        Label& label;
        Component& editor;
        i32 rowUnits;
    };

    struct ParamControls
    {
        Label        identifierLabel            { "identifierLabel",            "Identifier" };
        TextEditor   identifierEditor;
        Label        titleLabel                 { "titleLabel",                 "Title" };
        TextEditor   titleEditor;
        Label        bodyLabel                  { "bodyLabel",                  "Body" };
        TextEditor   bodyEditor;

        Label        categoryLabel              { "categoryLabel",              "Category" };
        ComboBox     categoryComboBox;
        Label        channelIdLabel             { "channelIdLabel",             "Channel ID" };
        ComboBox     channelIdComboBox;
        Label        iconLabel                  { "iconLabel",                  "Icon" };
        ComboBox     iconComboBox;

        Label        subtitleLabel              { "subtitleLabel",              "Subtitle" };
        TextEditor   subtitleEditor;
        Label        badgeNumberLabel           { "badgeNumberLabel",           "BadgeNumber" };
        ComboBox     badgeNumberComboBox;
        Label        soundToPlayLabel           { "soundToPlayLabel",           "SoundToPlay" };
        ComboBox     soundToPlayComboBox;
        Label        propertiesLabel            { "propertiesLabel",            "Properties" };
        TextEditor   propertiesEditor;
        Label        fireInLabel                { "fireInLabel",                "Fire in" };
        ComboBox     fireInComboBox;
        Label        repeatLabel                { "repeatLabel",                "Repeat" };
        ToggleButton repeatButton;
        Label        largeIconLabel             { "largeIconLabel",             "Large Icon" };
        ComboBox     largeIconComboBox;
        Label        badgeIconLabel             { "badgeIconLabel",             "Badge Icon" };
        ComboBox     badgeIconComboBox;
        Label        tickerTextLabel            { "tickerTextLabel",            "Ticker Text" };
        TextEditor   tickerTextEditor;
        Label        autoCancelLabel            { "autoCancelLabel",            "AutoCancel" };
        ToggleButton autoCancelButton;
        Label        alertOnlyOnceLabel         { "alertOnlyOnceLabel",         "AlertOnlyOnce" };
        ToggleButton alertOnlyOnceButton;
        Label        actionsLabel               { "actionsLabel",               "Actions" };
        ComboBox     actionsComboBox;

        Label        progressMaxLabel           { "progressMaxLabel",           "ProgressMax" };
        ComboBox     progressMaxComboBox;
        Label        progressCurrentLabel       { "progressCurrentLabel",       "ProgressCurrent" };
        ComboBox     progressCurrentComboBox;
        Label        progressIndeterminateLabel { "progressIndeterminateLabel", "ProgressIndeterminate" };
        ToggleButton progressIndeterminateButton;
        Label        notifCategoryLabel         { "notifCategoryLabel",         "Category" };
        ComboBox     notifCategoryComboBox;
        Label        priorityLabel              { "priorityLabel",              "Priority" };
        ComboBox     priorityComboBox;
        Label        personLabel                { "personLabel",                "Person" };
        TextEditor   personEditor;
        Label        lockScreenVisibilityLabel  { "lockScreenVisibilityLabel",  "LockScreenVisibility" };
        ComboBox     lockScreenVisibilityComboBox;
        Label        groupIdLabel               { "groupIdLabel",               "GroupID" };
        TextEditor   groupIdEditor;
        Label        sortKeyLabel               { "sortKeyLabel",               "SortKey" };
        TextEditor   sortKeyEditor;
        Label        groupSummaryLabel          { "groupSummaryLabel",          "GroupSummary" };
        ToggleButton groupSummaryButton;
        Label        groupAlertBehaviourLabel   { "groupAlertBehaviourLabel",   "GroupAlertBehaviour" };
        ComboBox     groupAlertBehaviourComboBox;

        Label        accentColorLabel          { "accentColorLabel",          "AccentColor" };
        TextButton   accentColorButton;
        Label        ledColorLabel             { "ledColorLabel",             "LedColor" };
        TextButton   ledColorButton;
        Label        ledMsToBeOnLabel           { "ledMsToBeOnLabel",           "LedMsToBeOn" };
        ComboBox     ledMsToBeOnComboBox;
        Label        ledMsToBeOffLabel          { "ledMsToBeOffLabel",          "LedMsToBeOff" };
        ComboBox     ledMsToBeOffComboBox;
        Label        vibratorMsToBeOnLabel      { "vibratorMsToBeOnLabel",      "VibrationMsToBeOn" };
        ComboBox     vibratorMsToBeOnComboBox;
        Label        vibratorMsToBeOffLabel     { "vibratorMsToBeOffLabel",     "VibrationMsToBeOff" };
        ComboBox     vibratorMsToBeOffComboBox;
        Label        localOnlyLabel             { "localOnlyLabel",             "LocalOnly" };
        ToggleButton localOnlyButton;
        Label        ongoingLabel               { "ongoingLabel",               "Ongoing" };
        ToggleButton ongoingButton;
        Label        timestampVisibilityLabel   { "timestampVisibilityLabel",   "TimestampMode" };
        ComboBox     timestampVisibilityComboBox;
        Label        timeoutAfterLabel          { "timeoutAfterLabel",          "Timeout After Ms" };
        ComboBox     timeoutAfterComboBox;

        ColorSelector* accentColorSelector = nullptr;
        ColorSelector* ledColorSelector    = nullptr;
    };

    z0 setupControls()
    {
        auto& pc = paramControls;

        StringArray categories { "okCategory", "okCancelCategory", "textCategory" };

        for (auto& c : categories)
            pc.categoryComboBox.addItem (c, pc.categoryComboBox.getNumItems() + 1);
        pc.categoryComboBox.setSelectedItemIndex (0);

        for (auto i = 1; i <= 3; ++i)
            pc.channelIdComboBox.addItem (Txt (i), i);
        pc.channelIdComboBox.setSelectedItemIndex (0);

        for (auto i = 0; i < 5; ++i)
            pc.iconComboBox.addItem ("icon" + Txt (i + 1), i + 1);
        pc.iconComboBox.setSelectedItemIndex (0);

      #if DRX_MAC
        pc.iconComboBox.addItem ("none", 100);
      #endif

        pc.fireInComboBox.addItem ("Now", 1);

        for (auto i = 1; i < 11; ++i)
            pc.fireInComboBox.addItem (Txt (10 * i) + "seconds", i + 1);
        pc.fireInComboBox.setSelectedItemIndex (0);

        pc.largeIconComboBox.addItem ("none", 1);

        for (auto i = 1; i < 5; ++i)
            pc.largeIconComboBox.addItem ("icon" + Txt (i), i + 1);
        pc.largeIconComboBox.setSelectedItemIndex (0);

        pc.badgeIconComboBox.addItem ("none",  1);
        pc.badgeIconComboBox.addItem ("small", 2);
        pc.badgeIconComboBox.addItem ("large", 3);
        pc.badgeIconComboBox.setSelectedItemIndex (2);

        pc.actionsComboBox.addItem ("none",       1);
        pc.actionsComboBox.addItem ("ok-cancel",  2);
        pc.actionsComboBox.addItem ("text-input", 3);
      #if DRX_ANDROID
        pc.actionsComboBox.addItem ("ok-cancel-icons", 4);
        pc.actionsComboBox.addItem ("text-input-limited_responses", 5);
      #endif
        pc.actionsComboBox.setSelectedItemIndex (0);

        for (auto i = 0; i < 7; ++i)
            pc.badgeNumberComboBox.addItem (Txt (i), i + 1);
        pc.badgeNumberComboBox.setSelectedItemIndex (0);

      #if DRX_IOS
        Txt prefix = "Notifications/sounds/";
        Txt extension = ".caf";
      #else
        Txt prefix;
        Txt extension;
      #endif

        pc.soundToPlayComboBox.addItem ("none", 1);
        pc.soundToPlayComboBox.addItem ("default_os_sound", 2);
        pc.soundToPlayComboBox.addItem (prefix + "demonstrative"  + extension, 3);
        pc.soundToPlayComboBox.addItem (prefix + "isntit"         + extension, 4);
        pc.soundToPlayComboBox.addItem (prefix + "jinglebellssms" + extension, 5);
        pc.soundToPlayComboBox.addItem (prefix + "served"         + extension, 6);
        pc.soundToPlayComboBox.addItem (prefix + "solemn"         + extension, 7);
        pc.soundToPlayComboBox.setSelectedItemIndex (1);

        for (auto i = 0; i < 11; ++i)
        {
            pc.progressMaxComboBox    .addItem (Txt (i * 10) + "%", i + 1);
            pc.progressCurrentComboBox.addItem (Txt (i * 10) + "%", i + 1);
        }

        pc.progressMaxComboBox    .setSelectedItemIndex (0);
        pc.progressCurrentComboBox.setSelectedItemIndex (0);

        pc.notifCategoryComboBox.addItem ("unspecified",     1);
        pc.notifCategoryComboBox.addItem ("alarm",           2);
        pc.notifCategoryComboBox.addItem ("call",            3);
        pc.notifCategoryComboBox.addItem ("email",           4);
        pc.notifCategoryComboBox.addItem ("error",           5);
        pc.notifCategoryComboBox.addItem ("event",           6);
        pc.notifCategoryComboBox.addItem ("message",         7);
        pc.notifCategoryComboBox.addItem ("progress",        8);
        pc.notifCategoryComboBox.addItem ("promo",           9);
        pc.notifCategoryComboBox.addItem ("recommendation", 10);
        pc.notifCategoryComboBox.addItem ("reminder",       11);
        pc.notifCategoryComboBox.addItem ("service",        12);
        pc.notifCategoryComboBox.addItem ("social",         13);
        pc.notifCategoryComboBox.addItem ("status",         14);
        pc.notifCategoryComboBox.addItem ("system",         15);
        pc.notifCategoryComboBox.addItem ("transport",      16);
        pc.notifCategoryComboBox.setSelectedItemIndex (0);

        for (auto i = -2; i < 3; ++i)
            pc.priorityComboBox.addItem (Txt (i), i + 3);
        pc.priorityComboBox.setSelectedItemIndex (2);

        pc.lockScreenVisibilityComboBox.addItem ("don't show",      1);
        pc.lockScreenVisibilityComboBox.addItem ("show partially",  2);
        pc.lockScreenVisibilityComboBox.addItem ("show completely", 3);
        pc.lockScreenVisibilityComboBox.setSelectedItemIndex (1);

        pc.groupAlertBehaviourComboBox.addItem ("alert all",      1);
        pc.groupAlertBehaviourComboBox.addItem ("alert summary",  2);
        pc.groupAlertBehaviourComboBox.addItem ("alert children", 3);
        pc.groupAlertBehaviourComboBox.setSelectedItemIndex (0);

        pc.timeoutAfterComboBox.addItem ("No timeout", 1);

        for (auto i = 0; i < 10; ++i)
        {
            pc.ledMsToBeOnComboBox      .addItem (Txt (i * 200) + "ms", i + 1);
            pc.ledMsToBeOffComboBox     .addItem (Txt (i * 200) + "ms", i + 1);
            pc.vibratorMsToBeOnComboBox .addItem (Txt (i * 500) + "ms", i + 1);
            pc.vibratorMsToBeOffComboBox.addItem (Txt (i * 500) + "ms", i + 1);
            pc.timeoutAfterComboBox     .addItem (Txt (5000 + 1000 * i) + "ms", i + 2);
        }

        pc.ledMsToBeOnComboBox      .setSelectedItemIndex (5);
        pc.ledMsToBeOffComboBox     .setSelectedItemIndex (5);
        pc.vibratorMsToBeOnComboBox .setSelectedItemIndex (0);
        pc.vibratorMsToBeOffComboBox.setSelectedItemIndex (0);
        pc.timeoutAfterComboBox     .setSelectedItemIndex (0);

        pc.timestampVisibilityComboBox.addItem ("off",         1);
        pc.timestampVisibilityComboBox.addItem ("on",          2);
        pc.timestampVisibilityComboBox.addItem ("chronometer", 3);
        pc.timestampVisibilityComboBox.addItem ("count down",  4);
        pc.timestampVisibilityComboBox.setSelectedItemIndex (1);
    }

    z0 distributeControls()
    {
        auto& pc = paramControls;

        paramsOneView  .addRowComponent (new RowComponent (pc.identifierLabel,            pc.identifierEditor));
        paramsOneView  .addRowComponent (new RowComponent (pc.titleLabel,                 pc.titleEditor));
        paramsOneView  .addRowComponent (new RowComponent (pc.bodyLabel,                  pc.bodyEditor, 4));
      #if DRX_IOS
        paramsOneView  .addRowComponent (new RowComponent (pc.categoryLabel,              pc.categoryComboBox));
      #elif DRX_ANDROID
        paramsOneView  .addRowComponent (new RowComponent (pc.channelIdLabel,             pc.channelIdComboBox));
      #endif
      #if DRX_ANDROID || DRX_MAC
        paramsOneView  .addRowComponent (new RowComponent (pc.iconLabel,                  pc.iconComboBox));
      #endif

        paramsTwoView  .addRowComponent (new RowComponent (pc.subtitleLabel,              pc.subtitleEditor));
      #if ! DRX_MAC
        paramsTwoView  .addRowComponent (new RowComponent (pc.badgeNumberLabel,           pc.badgeNumberComboBox));
      #endif
        paramsTwoView  .addRowComponent (new RowComponent (pc.soundToPlayLabel,           pc.soundToPlayComboBox));
        paramsTwoView  .addRowComponent (new RowComponent (pc.propertiesLabel,            pc.propertiesEditor, 3));
      #if DRX_IOS || DRX_MAC
        paramsTwoView  .addRowComponent (new RowComponent (pc.fireInLabel,                pc.fireInComboBox));
        paramsTwoView  .addRowComponent (new RowComponent (pc.repeatLabel,                pc.repeatButton));
      #elif DRX_ANDROID
        paramsTwoView  .addRowComponent (new RowComponent (pc.largeIconLabel,             pc.largeIconComboBox));
        paramsTwoView  .addRowComponent (new RowComponent (pc.badgeIconLabel,             pc.badgeIconComboBox));
        paramsTwoView  .addRowComponent (new RowComponent (pc.tickerTextLabel,            pc.tickerTextEditor));
        paramsTwoView  .addRowComponent (new RowComponent (pc.autoCancelLabel,            pc.autoCancelButton));
        paramsTwoView  .addRowComponent (new RowComponent (pc.alertOnlyOnceLabel,         pc.alertOnlyOnceButton));
      #endif
      #if DRX_ANDROID || DRX_MAC
        paramsTwoView  .addRowComponent (new RowComponent (pc.actionsLabel,               pc.actionsComboBox));
      #endif
      #if DRX_ANDROID
        paramsThreeView.addRowComponent (new RowComponent (pc.progressMaxLabel,           pc.progressMaxComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.progressCurrentLabel,       pc.progressCurrentComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.progressIndeterminateLabel, pc.progressIndeterminateButton));
        paramsThreeView.addRowComponent (new RowComponent (pc.categoryLabel,              pc.categoryComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.priorityLabel,              pc.priorityComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.personLabel,                pc.personEditor));
        paramsThreeView.addRowComponent (new RowComponent (pc.lockScreenVisibilityLabel,  pc.lockScreenVisibilityComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.groupIdLabel,               pc.groupIdEditor));
        paramsThreeView.addRowComponent (new RowComponent (pc.sortKeyLabel,               pc.sortKeyEditor));
        paramsThreeView.addRowComponent (new RowComponent (pc.groupSummaryLabel,          pc.groupSummaryButton));
        paramsThreeView.addRowComponent (new RowComponent (pc.groupAlertBehaviourLabel,   pc.groupAlertBehaviourComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.accentColorLabel,          pc.accentColorButton));
        paramsFourView .addRowComponent (new RowComponent (pc.ledColorLabel,             pc.ledColorButton));
        paramsFourView .addRowComponent (new RowComponent (pc.ledMsToBeOffLabel,          pc.ledMsToBeOffComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.ledMsToBeOnLabel,           pc.ledMsToBeOnComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.vibratorMsToBeOffLabel,     pc.vibratorMsToBeOffComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.vibratorMsToBeOnLabel,      pc.vibratorMsToBeOnComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.localOnlyLabel,             pc.localOnlyButton));
        paramsFourView .addRowComponent (new RowComponent (pc.ongoingLabel,               pc.ongoingButton));
        paramsFourView .addRowComponent (new RowComponent (pc.timestampVisibilityLabel,   pc.timestampVisibilityComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.timeoutAfterLabel,          pc.timeoutAfterComboBox));
      #endif
    }

    struct ParamsView final : public Component
    {
        ParamsView()
        {
            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        z0 addRowComponent (RowComponent* rc)
        {
            rowComponents.add (rc);
            addAndMakeVisible (rc);
        }

        z0 resized() override
        {
            auto totalRowUnits = 0;

            for (auto* rc : rowComponents)
                totalRowUnits += rc->rowUnits;

            auto rowHeight = getHeight() / totalRowUnits;

            auto bounds = getLocalBounds();

            for (auto* rc : rowComponents)
                rc->setBounds (bounds.removeFromTop (rc->rowUnits * rowHeight));

            auto* last = rowComponents[rowComponents.size() - 1];
            last->setBounds (last->getBounds().withHeight (getHeight() - last->getY()));
        }

    private:
        OwnedArray<RowComponent> rowComponents;
    };

    struct AuxActionsView final : public Component
    {
        AuxActionsView()
        {
            addAndMakeVisible (getDeliveredNotificationsButton);
            addAndMakeVisible (removeDeliveredNotifWithIdButton);
            addAndMakeVisible (deliveredNotifIdentifier);
            addAndMakeVisible (removeAllDeliveredNotifsButton);
          #if DRX_IOS || DRX_MAC
            addAndMakeVisible (getPendingNotificationsButton);
            addAndMakeVisible (removePendingNotifWithIdButton);
            addAndMakeVisible (pendingNotifIdentifier);
            addAndMakeVisible (removeAllPendingNotifsButton);
          #endif

            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        z0 resized() override
        {
            auto columnWidth = getWidth();
            auto rowHeight = getHeight() / 6;
            auto bounds = getLocalBounds();

            getDeliveredNotificationsButton .setBounds (bounds.removeFromTop (rowHeight));

            auto rowBounds = bounds.removeFromTop (rowHeight);
            removeDeliveredNotifWithIdButton.setBounds (rowBounds.removeFromLeft (columnWidth / 2));
            deliveredNotifIdentifier        .setBounds (rowBounds);

            removeAllDeliveredNotifsButton  .setBounds (bounds.removeFromTop (rowHeight));

           #if DRX_IOS || DRX_MAC
            getPendingNotificationsButton .setBounds (bounds.removeFromTop (rowHeight));

            rowBounds = bounds.removeFromTop (rowHeight);
            removePendingNotifWithIdButton.setBounds (rowBounds.removeFromLeft (columnWidth / 2));
            pendingNotifIdentifier        .setBounds (rowBounds);

            removeAllPendingNotifsButton  .setBounds (bounds.removeFromTop (rowHeight));
           #endif
        }

        TextButton getDeliveredNotificationsButton  { "Get Delivered Notifications" };
        TextButton removeDeliveredNotifWithIdButton { "Remove Delivered Notif With ID:" };
        TextEditor deliveredNotifIdentifier;
        TextButton removeAllDeliveredNotifsButton   { "Remove All Delivered Notifs" };
        TextButton getPendingNotificationsButton    { "Get Pending Notifications" };
        TextButton removePendingNotifWithIdButton   { "Remove Pending Notif With ID:" };
        TextEditor pendingNotifIdentifier;
        TextButton removeAllPendingNotifsButton     { "Remove All Pending Notifs" };
    };

    struct RemoteView final : public Component
    {
        RemoteView()
        {
            addAndMakeVisible (getDeviceTokenButton);
           #if DRX_ANDROID
            addAndMakeVisible (sendRemoteMessageButton);
            addAndMakeVisible (subscribeToSportsButton);
            addAndMakeVisible (unsubscribeFromSportsButton);
           #endif
        }

        z0 resized()
        {
            auto rowSize = getHeight() / 10;

            auto bounds = getLocalBounds().reduced (getWidth() / 10, getHeight() / 10);

            bounds.removeFromTop (2 * rowSize);

            getDeviceTokenButton       .setBounds (bounds.removeFromTop (rowSize));
            sendRemoteMessageButton    .setBounds (bounds.removeFromTop (rowSize));
            subscribeToSportsButton    .setBounds (bounds.removeFromTop (rowSize));
            unsubscribeFromSportsButton.setBounds (bounds.removeFromTop (rowSize));
        }

        TextButton getDeviceTokenButton        { "GetDeviceToken" };
        TextButton sendRemoteMessageButton     { "SendRemoteMessage" };
        TextButton subscribeToSportsButton     { "SubscribeToSports" };
        TextButton unsubscribeFromSportsButton { "UnsubscribeFromSports" };
    };

    struct DemoTabbedComponent final : public TabbedComponent
    {
        DemoTabbedComponent (PushNotificationsDemo& demoIn, TabbedButtonBar::Orientation orientation)
            : TabbedComponent (orientation), demo (demoIn)
        {
        }

        z0 currentTabChanged (i32, const Txt& newCurrentTabName) override
        {
            if (! showedRemoteInstructions && newCurrentTabName == "Remote")
            {
                demo.showRemoteInstructions();
                showedRemoteInstructions = true;
            }
        }

    private:
        b8 showedRemoteInstructions = false;
        PushNotificationsDemo& demo;
    };

    z0 showRemoteInstructions()
    {
       #if DRX_IOS || DRX_MAC
        auto options = MessageBoxOptions()
                                       .withIconType (MessageBoxIconType::InfoIcon)
                                       .withTitle ("Remote Notifications instructions")
                                       .withMessage ("In order to be able to test remote notifications "
                                                     "ensure that the app is signed and that you register "
                                                     "the bundle ID for remote notifications in "
                                                     "Apple Developer Center.")
                                       .withButton ("OK");
        messageBox = NativeMessageBox::showScopedAsync (options, nullptr);
       #endif
    }

    Label headerLabel { "headerLabel", "Push Notifications Demo" };
    ParamControls paramControls;
    ParamsView paramsOneView, paramsTwoView, paramsThreeView, paramsFourView;
    AuxActionsView auxActionsView;
    TabbedComponent localNotificationsTabs { TabbedButtonBar::TabsAtTop };
    RemoteView remoteView;
    DemoTabbedComponent mainTabs { *this, TabbedButtonBar::TabsAtTop };
    TextButton sendButton      { "Send!" };
    Label notAvailableYetLabel { "notAvailableYetLabel",
                                 "Push Notifications feature is not available on this platform yet!" };
    ScopedMessageBox messageBox;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PushNotificationsDemo)
};
