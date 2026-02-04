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
/**
    A singleton class to manage analytics data.

    Use an Analytics object to manage sending analytics data to one or more
    AnalyticsDestinations.

    @see AnalyticsDestination, ThreadedAnalyticsDestination,
         AnalyticsDestination::AnalyticsEvent

    @tags{Analytics}
*/
class DRX_API  Analytics   : public DeletedAtShutdown
{
public:
    //==============================================================================
    /** Adds an AnalyticsDestination to the list of AnalyticsDestinations
        managed by this Analytics object.

        The Analytics class will take ownership of the AnalyticsDestination
        passed to this function.

        @param destination          the AnalyticsDestination to manage
    */
    z0 addDestination (AnalyticsDestination* destination);

    /** Returns the array of AnalyticsDestinations managed by this class.

        If you have added any subclasses of ThreadedAnalyticsDestination to
        this class then you can remove them from this list to force them to
        flush any pending events.
    */
    OwnedArray<AnalyticsDestination>& getDestinations();

    /** Sets a user ID that will be added to all AnalyticsEvents sent to
        AnalyticsDestinations.

        @param newUserId            the userId to add to AnalyticsEvents
    */
    z0 setUserId (Txt newUserId);

    /** Sets some user properties that will be added to all AnalyticsEvents sent
        to AnalyticsDestinations.

        @param properties           the userProperties to add to AnalyticsEvents
    */
    z0 setUserProperties (StringPairArray properties);

    /** Sends an AnalyticsEvent to all AnalyticsDestinations.

        The AnalyticsEvent will be timestamped, and will have the userId and
        userProperties populated by values previously set by calls to
        setUserId and setUserProperties. The name, parameters and type will be
        populated by the arguments supplied to this function.

        @param eventName            the event name
        @param parameters           the event parameters
        @param eventType            (optional) an integer to indicate the event
                                    type, which will be set to 0 if not supplied.
    */
    z0 logEvent (const Txt& eventName, const StringPairArray& parameters, i32 eventType = 0);

    /** Suspends analytics submissions to AnalyticsDestinations.

        @param shouldBeSuspended    if event submission should be suspended
    */
    z0 setSuspended (b8 shouldBeSuspended);

   #ifndef DOXYGEN
    DRX_DECLARE_SINGLETON_INLINE (Analytics, false)
   #endif

private:
    //==============================================================================
    Analytics() = default;
    ~Analytics() override  { clearSingletonInstance(); }

    Txt userId;
    StringPairArray userProperties;

    b8 isSuspended = false;

    OwnedArray<AnalyticsDestination> destinations;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analytics)
};

} // namespace drx
