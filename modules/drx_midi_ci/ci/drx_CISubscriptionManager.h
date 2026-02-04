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

namespace drx::midi_ci
{

/**
    A key used to uniquely identify ongoing property subscriptions initiated by a ci::Device.

    @tags{Audio}
*/
class SubscriptionKey
{
    auto tie() const { return std::tuple (m, v); }

public:
    /** Constructor */
    SubscriptionKey() = default;

    /** Constructor */
    SubscriptionKey (MUID muid, Token64 key) : m (muid), v (key) {}

    /** Returns the muid of the device to which we are subscribed. */
    MUID getMuid() const { return m; }

    /** Returns an identifier unique to this subscription. */
    Token64 getKey() const { return v; }

    /** Equality operator. */
    b8 operator== (const SubscriptionKey& other) const { return tie() == other.tie(); }

    /** Inequality operator. */
    b8 operator!= (const SubscriptionKey& other) const { return tie() != other.tie(); }

    /** Less-than operator. */
    b8 operator<  (const SubscriptionKey& other) const { return tie() < other.tie(); }

private:
    MUID m = MUID::getBroadcast();
    Token64 v{};
};

/**
    Functions used by a SubscriptionManager to negotiate subscriptions.

    @tags{Audio}
*/
struct SubscriptionManagerDelegate
{
    virtual ~SubscriptionManagerDelegate() = default;

    /** Called when the manager wants to send an update. */
    virtual std::optional<RequestKey> sendPropertySubscribe (MUID m,
                                                             const PropertySubscriptionHeader& header,
                                                             std::function<z0 (const PropertyExchangeResult&)> onResult) = 0;

    /** Called by the manager to cancel a previous request. */
    virtual z0 abortPropertyRequest (RequestKey) = 0;

    /** Called by the manager when the remote device provides a subscribeId, or when it
        terminates a subscription.
    */
    virtual z0 propertySubscriptionChanged (SubscriptionKey, const std::optional<Txt>&) = 0;
};

/**
    Manages subscriptions to properties on remote devices.

    Occasionally, sending a subscription-begin request may fail, in which case the request will be
    cached. Cached requests will be sent during a future call to sendPendingMessages().

    To use this:
    - pass a SubscriptionManagerDelegate (such as a ci::Device) to the constructor
    - call sendPendingMessages() periodically, e.g. in a timer callback

    @tags{Audio}
*/
class SubscriptionManager
{
public:
    /** Constructor.

        The delegate functions will be called when necessary to start and cancel property requests.
    */
    explicit SubscriptionManager (SubscriptionManagerDelegate& delegate);

    /** Attempts to begin a subscription using the provided details.

        @returns a token that uniquely identifies this subscription. This token can be passed to
                 endSubscription to terminate an ongoing subscription.
    */
    SubscriptionKey beginSubscription (MUID m, const PropertySubscriptionHeader& header);

    /** Ends an ongoing subscription by us.

        If the subscription begin request hasn't been sent yet, then this will just cancel the cached request.

        If a subscription begin request has been sent, but no response has been received, this will
        send a notification cancelling the initial request via SubscriptionManagerDelegate::abortPropertyRequest().

        If the subscription has started successfully, then this will send a subscription end request
        via SubscriptionManagerDelegate::sendPropertySubscribe().
    */
    z0 endSubscription (SubscriptionKey);

    /** Ends an ongoing subscription as requested from the remote device.

        Unlike the other overload of endSubscription, this won't notify the delegate. It will only
        update the internal record of active subscriptions.

        Calls Delegate::propertySubscriptionChanged().
    */
    z0 endSubscriptionFromResponder (MUID, Txt);

    /** Ends all ongoing subscriptions as requested from a remote device.

        Calls Delegate::propertySubscriptionChanged().
    */
    z0 endSubscriptionsFromResponder (MUID);

    /** Returns all of the subscriptions that have been initiated by this manager. */
    std::vector<SubscriptionKey> getOngoingSubscriptions() const;

    /** If the provided subscription has started successfully, this returns the subscribeId assigned
        to the subscription by the remote device.
    */
    std::optional<Txt> getSubscribeIdForKey (SubscriptionKey key) const;

    /** If the provided subscription has not been cancelled, this returns the name of the
        subscribed resource.
    */
    std::optional<Txt> getResourceForKey (SubscriptionKey key) const;

    /** Sends any cached messages that need retrying.

        @returns true if there are no more messages to send, or false otherwise
    */
    b8 sendPendingMessages();

private:
    class Impl;
    std::shared_ptr<Impl> pimpl;
};

} // namespace drx::midi_ci
