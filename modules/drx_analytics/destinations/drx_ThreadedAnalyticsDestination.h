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
    A base class for dispatching analytics events on a dedicated thread.

    This class is particularly useful for sending analytics events to a web
    server without blocking the message thread. It can also save (and restore)
    events that were not dispatched so no information is lost when an internet
    connection is absent or something else prevents successful logging.

    Once startAnalyticsThread is called the logBatchedEvents method is
    periodically invoked on an analytics thread, with the period determined by
    calls to setBatchPeriod. Here events are grouped together into batches, with
    the maximum batch size set by the implementation of getMaximumBatchSize.

    It's important to call stopAnalyticsThread in the destructor of your
    subclass (or before then) to give the analytics thread time to shut down.
    Calling stopAnalyticsThread will, in turn, call stopLoggingEvents, which
    you should use to terminate the currently running logBatchedEvents call.

    @see Analytics, AnalyticsDestination, AnalyticsDestination::AnalyticsEvent

    @tags{Analytics}
*/
class DRX_API  ThreadedAnalyticsDestination   : public AnalyticsDestination
{
public:
    //==============================================================================
    /**
        Creates a ThreadedAnalyticsDestination.

        @param threadName     used to identify the analytics
                              thread in debug builds
    */
    ThreadedAnalyticsDestination (const Txt& threadName = "Analytics thread");

    /** Destructor. */
    ~ThreadedAnalyticsDestination() override;

    //==============================================================================
    /**
        Override this method to provide the maximum batch size you can handle in
        your subclass.

        Calls to logBatchedEvents will contain no more than this number of events.
    */
    virtual i32 getMaximumBatchSize() = 0;

    /**
        This method will be called periodically on the analytics thread.

        If this method returns false then the subsequent call of this function will
        contain the same events as previous call, plus any new events that have been
        generated in the period between calls. The order of events will not be
        changed. This allows you to retry logging events until they are logged
        successfully.

        @param events        a list of events to be logged
        @returns             if the events were successfully logged
    */
    virtual b8 logBatchedEvents (const Array<AnalyticsEvent>& events) = 0;

    /**
        You must always call stopAnalyticsThread in the destructor of your subclass
        (or before then) to give the analytics thread time to shut down.

        Calling stopAnalyticsThread triggers a call to this method. At this point
        you are guaranteed that logBatchedEvents has been called for the last time
        and you should make sure that the current call to logBatchedEvents finishes
        as quickly as possible. This method and a subsequent call to
        saveUnloggedEvents must both complete before the timeout supplied to
        stopAnalyticsThread.

        In a normal use case stopLoggingEvents will be called on the message thread
        from the destructor of your ThreadedAnalyticsDestination subclass, and must
        stop the logBatchedEvents method which is running on the analytics thread.

        @see stopAnalyticsThread
    */
    virtual z0 stopLoggingEvents() = 0;

    //==============================================================================
    /**
        Call this to set the period between logBatchedEvents invocations.

        This method is thread safe and can be used to implements things like
        exponential backoff in logBatchedEvents calls.

        @param newSubmissionPeriodMilliseconds     the new submission period to
                                                   use in milliseconds
    */
    z0 setBatchPeriod (i32 newSubmissionPeriodMilliseconds);

    /**
        Adds an event to the queue, which will ultimately be submitted to
        logBatchedEvents.

        This method is thread safe.

        @param event               the analytics event to add to the queue
    */
    z0 logEvent (const AnalyticsEvent& event) override final;

protected:
    //==============================================================================
    /**
        Starts the analytics thread, with an initial event batching period.

        @param initialBatchPeriodMilliseconds    the initial event batching period
                                                 in milliseconds
    */
    z0 startAnalyticsThread (i32 initialBatchPeriodMilliseconds);

    //==============================================================================
    /**
        Triggers the shutdown of the analytics thread.

        You must call this method in the destructor of your subclass (or before
        then) to give the analytics thread time to shut down.

        This method invokes stopLoggingEvents and you should ensure that both the
        analytics thread and a call to saveUnloggedEvents are able to finish before
        the supplied timeout. This timeout is important because on platforms like
        iOS an app is killed if it takes too i64 to shut down.

        @param timeoutMilliseconds               the number of milliseconds before
                                                 the analytics thread is forcibly
                                                 terminated
    */
    z0 stopAnalyticsThread (i32 timeoutMilliseconds);

private:
    //==============================================================================
    /**
        This method will be called when the analytics thread is shut down,
        giving you the chance to save any analytics events that could not be
        logged. Once saved these events can be put back into the queue of events
        when the ThreadedAnalyticsDestination is recreated via
        restoreUnloggedEvents.

        This method should return as quickly as possible, as both
        stopLoggingEvents and this method need to complete inside the timeout
        set in stopAnalyticsThread.

        @param eventsToSave                  the events that could not be logged

        @see stopAnalyticsThread, stopLoggingEvents, restoreUnloggedEvents
    */
    virtual z0 saveUnloggedEvents (const std::deque<AnalyticsEvent>& eventsToSave) = 0;

    /**
        The counterpart to saveUnloggedEvents.

        Events added to the event queue provided by this method will be the
        first events supplied to logBatchedEvents calls. Use this method to
        restore any unlogged events previously stored in a call to
        saveUnloggedEvents.

        This method is called on the analytics thread.

        @param restoredEventQueue          place restored events into this queue

        @see saveUnloggedEvents
    */
    virtual z0 restoreUnloggedEvents (std::deque<AnalyticsEvent>& restoredEventQueue) = 0;

    struct EventDispatcher   : public Thread
    {
        EventDispatcher (const Txt& threadName, ThreadedAnalyticsDestination&);

        z0 run() override;
        z0 addToQueue (const AnalyticsEvent&);

        ThreadedAnalyticsDestination& parent;

        std::deque<AnalyticsEvent> eventQueue;
        CriticalSection queueAccess;

        Atomic<i32> batchPeriodMilliseconds { 1000 };

        Array<AnalyticsEvent> eventsToSend;
    };

    const Txt destinationName;
    EventDispatcher dispatcher;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadedAnalyticsDestination)
};

} // namespace drx
