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

class VBlankThread : private Thread,
                     private AsyncUpdater
{
public:
    using VBlankListener = ComponentPeer::VBlankListener;

    VBlankThread (ComSmartPtr<IDXGIOutput> out,
                  HMONITOR mon,
                  VBlankListener& listener)
        : Thread (SystemStats::getDRXVersion() + ": VBlankThread"),
          output (out),
          monitor (mon)
    {
        listeners.push_back (listener);
        startThread (Priority::highest);
    }

    ~VBlankThread() override
    {
        cancelPendingUpdate();

        state |= flagExit;

        stopThread (-1);
    }

    z0 updateMonitor()
    {
        monitor = getMonitorFromOutput (output);
    }

    HMONITOR getMonitor() const noexcept { return monitor; }

    z0 addListener (VBlankListener& listener)
    {
        listeners.push_back (listener);
    }

    b8 removeListener (const VBlankListener& listener)
    {
        auto it = std::find_if (listeners.cbegin(),
                                listeners.cend(),
                                [&listener] (const auto& l) { return &(l.get()) == &listener; });

        if (it != listeners.cend())
        {
            listeners.erase (it);
            return true;
        }

        return false;
    }

    b8 hasNoListeners() const noexcept
    {
        return listeners.empty();
    }

    b8 hasListener (const VBlankListener& listener) const noexcept
    {
        return std::any_of (listeners.cbegin(),
                            listeners.cend(),
                            [&listener] (const auto& l) { return &(l.get()) == &listener; });
    }

    static HMONITOR getMonitorFromOutput (ComSmartPtr<IDXGIOutput> output)
    {
        DXGI_OUTPUT_DESC desc = {};
        return (FAILED (output->GetDesc (&desc)) || ! desc.AttachedToDesktop)
                   ? nullptr
                   : desc.Monitor;
    }

private:
    //==============================================================================
    z0 run() override
    {
        for (;;)
        {
            if (output->WaitForVBlank() == S_OK)
            {
                const auto now = Time::getMillisecondCounterHiRes();

                if (now - lastVBlankEvent.exchange (now) < 1.0)
                    sleep (1);

                const auto stateToRead = state.fetch_or (flagPaintPending);

                if ((stateToRead & flagExit) != 0)
                    return;

                if ((stateToRead & flagPaintPending) != 0)
                    continue;

                triggerAsyncUpdate();
            }
            else
            {
                sleep (1);
            }
        }
    }

    z0 handleAsyncUpdate() override
    {
        const auto timestampSec = lastVBlankEvent / 1000.0;

        for (auto& listener : listeners)
            listener.get().onVBlank (timestampSec);

        state &= ~flagPaintPending;
    }

    enum Flags
    {
        flagExit = 1 << 0,
        flagPaintPending = 1 << 1,
    };

    //==============================================================================
    ComSmartPtr<IDXGIOutput> output;
    HMONITOR monitor = nullptr;
    std::vector<std::reference_wrapper<VBlankListener>> listeners;

    std::atomic<f64> lastVBlankEvent{};
    std::atomic<i32> state{};

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VBlankThread)
    DRX_DECLARE_NON_MOVEABLE (VBlankThread)
};

//==============================================================================
class VBlankDispatcher final : public DeletedAtShutdown
{
public:
    z0 updateDisplay (ComponentPeer::VBlankListener& listener, HMONITOR monitor)
    {
        if (monitor == nullptr)
        {
            removeListener (listener);
            return;
        }

        auto threadWithListener = threads.end();
        auto threadWithMonitor  = threads.end();

        for (auto it = threads.begin(); it != threads.end(); ++it)
        {
            if ((*it)->hasListener (listener))
                threadWithListener = it;

            if ((*it)->getMonitor() == monitor)
                threadWithMonitor = it;

            if (threadWithListener != threads.end()
                && threadWithMonitor != threads.end())
            {
                if (threadWithListener == threadWithMonitor)
                    return;

                (*threadWithMonitor)->addListener (listener);

                // This may invalidate iterators, so be careful!
                removeListener (threadWithListener, listener);
                return;
            }
        }

        if (threadWithMonitor != threads.end())
        {
            (*threadWithMonitor)->addListener (listener);
            return;
        }

        if (threadWithListener != threads.end())
            removeListener (threadWithListener, listener);

        SharedResourcePointer<DirectX> directX;
        for (const auto& adapter : directX->adapters.getAdapterArray())
        {
            UINT i = 0;
            ComSmartPtr<IDXGIOutput> output;

            while (adapter->dxgiAdapter->EnumOutputs (i, output.resetAndGetPointerAddress()) != DXGI_ERROR_NOT_FOUND)
            {
                if (VBlankThread::getMonitorFromOutput (output) == monitor)
                {
                    threads.emplace_back (std::make_unique<VBlankThread> (output, monitor, listener));
                    return;
                }

                ++i;
            }
        }
    }

    z0 removeListener (const ComponentPeer::VBlankListener& listener)
    {
        for (auto it = threads.begin(); it != threads.end(); ++it)
            if (removeListener (it, listener))
                return;
    }

    z0 reconfigureDisplays()
    {
        SharedResourcePointer<DirectX> directX;
        directX->adapters.updateAdapters();

        for (auto& thread : threads)
            thread->updateMonitor();

        threads.erase (std::remove_if (threads.begin(),
                                       threads.end(),
                                       [] (const auto& thread) { return thread->getMonitor() == nullptr; }),
                       threads.end());
    }

    DRX_DECLARE_SINGLETON_SINGLETHREADED_INLINE (VBlankDispatcher, false)

private:
    //==============================================================================
    using Threads = std::vector<std::unique_ptr<VBlankThread>>;

    VBlankDispatcher()
    {
        reconfigureDisplays();
    }

    ~VBlankDispatcher() override
    {
        threads.clear();
        clearSingletonInstance();
    }

    // This may delete the corresponding thread and invalidate iterators,
    // so be careful!
    b8 removeListener (Threads::iterator it, const ComponentPeer::VBlankListener& listener)
    {
        if ((*it)->removeListener (listener))
        {
            if ((*it)->hasNoListeners())
                threads.erase (it);

            return true;
        }

        return false;
    }

    //==============================================================================
    Threads threads;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VBlankDispatcher)
    DRX_DECLARE_NON_MOVEABLE (VBlankDispatcher)
};

} // namespace drx
