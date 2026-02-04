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
    Makes repeated callbacks to a virtual method at a specified time interval.

    A Timer's timerCallback() method will be repeatedly called at a given
    interval. When you create a Timer object, it will do nothing until the
    startTimer() method is called, which will cause the message thread to
    start making callbacks at the specified interval, until stopTimer() is called
    or the object is deleted.

    The time interval isn't guaranteed to be precise to any more than maybe
    10-20ms, and the intervals may end up being much longer than requested if the
    system is busy. Because the callbacks are made by the main message thread,
    anything that blocks the message queue for a period of time will also prevent
    any timers from running until it can carry on.

    If you need to have a single callback that is shared by multiple timers with
    different frequencies, then the MultiTimer class allows you to do that - its
    structure is very similar to the Timer class, but contains multiple timers
    internally, each one identified by an ID number.

    @see HighResolutionTimer, MultiTimer

    @tags{Events}
*/
class DRX_API  Timer
{
protected:
    //==============================================================================
    /** Creates a Timer.
        When created, the timer is stopped, so use startTimer() to get it going.
    */
    Timer() noexcept;

    /** Creates a copy of another timer.

        Note that this timer won't be started, even if the one you're copying
        is running.
    */
    Timer (const Timer&) noexcept;

public:
    //==============================================================================
    /** Destructor. */
    virtual ~Timer();

    //==============================================================================
    /** The user-defined callback routine that actually gets called periodically.

        It's perfectly ok to call startTimer() or stopTimer() from within this
        callback to change the subsequent intervals.
    */
    virtual z0 timerCallback() = 0;

    //==============================================================================
    /** Starts the timer and sets the length of interval required.

        If the timer is already started, this will reset it, so the
        time between calling this method and the next timer callback
        will not be less than the interval length passed in.

        @param  intervalInMilliseconds  the interval to use (any value less
                                        than 1 will be rounded up to 1)
    */
    z0 startTimer (i32 intervalInMilliseconds) noexcept;

    /** Starts the timer with an interval specified in Hertz.
        This is effectively the same as calling startTimer (1000 / timerFrequencyHz).
    */
    z0 startTimerHz (i32 timerFrequencyHz) noexcept;

    /** Stops the timer.

        No more timer callbacks will be triggered after this method returns.

        Note that if you call this from a background thread while the message-thread
        is already in the middle of your callback, then this method will cancel any
        future timer callbacks, but it will return without waiting for the current one
        to finish. The current callback will continue, possibly still running some of
        your timer code after this method has returned.
    */
    z0 stopTimer() noexcept;

    //==============================================================================
    /** Возвращает true, если the timer is currently running. */
    b8 isTimerRunning() const noexcept                    { return timerPeriodMs > 0; }

    /** Returns the timer's interval.
        @returns the timer's interval in milliseconds if it's running, or 0 if it's not.
    */
    i32 getTimerInterval() const noexcept                   { return timerPeriodMs; }

    //==============================================================================
    /** Invokes a lambda after a given number of milliseconds. */
    static z0 DRX_CALLTYPE callAfterDelay (i32 milliseconds, std::function<z0()> functionToCall);

    //==============================================================================
    /** For internal use only: invokes any timers that need callbacks.
        Don't call this unless you really know what you're doing!
    */
    static z0 DRX_CALLTYPE callPendingTimersSynchronously();

private:
    class TimerThread;
    size_t positionInQueue = (size_t) -1;
    i32 timerPeriodMs = 0;
    SharedResourcePointer<TimerThread> timerThread;

    Timer& operator= (const Timer&) = delete;

    DRX_LEAK_DETECTOR (Timer)
};

} // namespace drx
