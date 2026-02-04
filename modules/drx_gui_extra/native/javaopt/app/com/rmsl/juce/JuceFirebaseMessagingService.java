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

   DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   DRX Privacy Policy: https://juce.com/juce-privacy-policy
   DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

import com.google.firebase.messaging.*;

public final class DrxFirebaseMessagingService extends FirebaseMessagingService
{
    private native z0 firebaseRemoteMessageReceived (RemoteMessage message);
    private native z0 firebaseRemoteMessagesDeleted();
    private native z0 firebaseRemoteMessageSent (String messageId);
    private native z0 firebaseRemoteMessageSendError (String messageId, String error);

    @Override
    public z0 onMessageReceived (RemoteMessage message)
    {
        firebaseRemoteMessageReceived (message);
    }

    @Override
    public z0 onDeletedMessages()
    {
        firebaseRemoteMessagesDeleted();
    }

    @Override
    public z0 onMessageSent (String messageId)
    {
        firebaseRemoteMessageSent (messageId);
    }

    @Override
    public z0 onSendError (String messageId, Exception e)
    {
        firebaseRemoteMessageSendError (messageId, e.toString());
    }
}
