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
    /** Manages a set of ChildProcesses and periodically checks their return value. Upon completion
        it calls listeners added with addChildProcessExitedListener().

        This class is mostly aimed for usage on Linux, where terminated child processes are only
        cleaned up if their return code is read after termination. In order to ensure this one needs
        to call ChildProcess::isFinished() until it returns false or
        ChildProcess::waitForProcessToFinish() until it returns true.

        This class will keep querying the return code on a Timer thread until the process
        terminates. This can be handy if one wants to start and stop multiple ChildProcesses on
        Linux that could take a i64 time to complete.

        Since this class uses a Timer to check subprocess status, it's generally only safe to
        access the returned ChildProcesses from the message thread.

        @see ChildProcessManagerSingleton

        @tags{Events}
    */
    class DRX_API  ChildProcessManager final : private DeletedAtShutdown
    {
    public:
       #ifndef DOXYGEN
        DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (ChildProcessManager)
       #endif

        /** Creates a new ChildProcess and starts it with the provided arguments.

            The arguments are the same as the overloads to ChildProcess::start().

            The manager will keep the returned ChildProcess object alive until it terminates and its
            return value has been queried. Calling ChildProcess::kill() on the returned object will
            eventually cause its removal from the ChildProcessManager after it terminates.
        */
        template <typename... Args>
        std::shared_ptr<ChildProcess> createAndStartManagedChildProcess (Args&&... args)
        {
            auto p = std::make_shared<ChildProcess>();

            if (! p->start (std::forward<Args> (args)...))
                return nullptr;

            processes.insert (p);
            timer.startTimer (1000);

            return p;
        }

        /** Registers a callback function that is called for every ChildProcess that terminated.

            This registration is deleted when the returned ErasedScopedGuard is deleted.
        */
        auto addChildProcessExitedListener (std::function<z0 (ChildProcess*)> listener)
        {
            return listeners.addListener (std::move (listener));
        }

        /** Возвращает true, если the ChildProcessManager contains any running ChildProcesses that it's
            monitoring.
        */
        auto hasRunningProcess() const
        {
            return timer.isTimerRunning();
        }

    private:
        ChildProcessManager() = default;
        ~ChildProcessManager() override  { clearSingletonInstance(); }

        z0 checkProcesses();

        std::set<std::shared_ptr<ChildProcess>> processes;
        detail::CallbackListenerList<ChildProcess*> listeners;
        TimedCallback timer { [this] { checkProcesses(); } };
    };

} // namespace drx
