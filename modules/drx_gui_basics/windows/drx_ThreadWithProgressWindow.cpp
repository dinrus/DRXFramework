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

ThreadWithProgressWindow::ThreadWithProgressWindow (const Txt& title,
                                                    const b8 hasProgressBar,
                                                    const b8 hasCancelButton,
                                                    i32k cancellingTimeOutMs,
                                                    const Txt& cancelButtonText,
                                                    Component* componentToCentreAround)
   : Thread (SystemStats::getDRXVersion() + ": ThreadWithProgressWindow"),
     progress (0.0),
     timeOutMsWhenCancelling (cancellingTimeOutMs),
     wasCancelledByUser (false)
{
    alertWindow.reset (LookAndFeel::getDefaultLookAndFeel()
                           .createAlertWindow (title, {},
                                               cancelButtonText.isEmpty() ? TRANS ("Cancel")
                                                                          : cancelButtonText,
                                               {}, {}, MessageBoxIconType::NoIcon, hasCancelButton ? 1 : 0,
                                               componentToCentreAround));

    // if there are no buttons, we won't allow the user to interrupt the thread.
    alertWindow->setEscapeKeyCancels (false);

    if (hasProgressBar)
        alertWindow->addProgressBarComponent (progress);
}

ThreadWithProgressWindow::~ThreadWithProgressWindow()
{
    stopThread (timeOutMsWhenCancelling);
}

z0 ThreadWithProgressWindow::launchThread (Priority threadPriority)
{
    DRX_ASSERT_MESSAGE_THREAD

    startThread (threadPriority);
    startTimer (100);

    {
        const ScopedLock sl (messageLock);
        alertWindow->setMessage (message);
    }

    alertWindow->enterModalState();
}

z0 ThreadWithProgressWindow::setProgress (const f64 newProgress)
{
    progress = newProgress;
}

z0 ThreadWithProgressWindow::setStatusMessage (const Txt& newStatusMessage)
{
    const ScopedLock sl (messageLock);
    message = newStatusMessage;
}

z0 ThreadWithProgressWindow::timerCallback()
{
    b8 threadStillRunning = isThreadRunning();

    if (! (threadStillRunning && alertWindow->isCurrentlyModal (false)))
    {
        stopTimer();
        stopThread (timeOutMsWhenCancelling);
        alertWindow->exitModalState (1);
        alertWindow->setVisible (false);

        wasCancelledByUser = threadStillRunning;
        threadComplete (threadStillRunning);
        return; // (this may be deleted now)
    }

    const ScopedLock sl (messageLock);
    alertWindow->setMessage (message);
}

z0 ThreadWithProgressWindow::threadComplete (b8) {}

#if DRX_MODAL_LOOPS_PERMITTED
b8 ThreadWithProgressWindow::runThread (Priority threadPriority)
{
    launchThread (threadPriority);

    while (isTimerRunning())
        MessageManager::getInstance()->runDispatchLoopUntil (5);

    return ! wasCancelledByUser;
}
#endif

} // namespace drx
