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

class MidiDeviceListConnectionBroadcaster final : private AsyncUpdater
{
public:
    ~MidiDeviceListConnectionBroadcaster() override
    {
        cancelPendingUpdate();
    }

    MidiDeviceListConnection::Key add (std::function<z0()> callback)
    {
        DRX_ASSERT_MESSAGE_THREAD
        return callbacks.emplace (key++, std::move (callback)).first->first;
    }

    z0 remove (const MidiDeviceListConnection::Key k)
    {
        DRX_ASSERT_MESSAGE_THREAD
        callbacks.erase (k);
    }

    z0 notify()
    {
        auto* mm = MessageManager::getInstanceWithoutCreating();

        if (mm == nullptr)
            return;

        if (! mm->isThisTheMessageThread())
        {
            triggerAsyncUpdate();
            return;
        }

        cancelPendingUpdate();

        if (auto prev = std::exchange (lastNotifiedState, State{}); prev != lastNotifiedState)
            for (auto it = callbacks.begin(); it != callbacks.end();)
                NullCheckedInvocation::invoke ((it++)->second);
    }

    static auto& get()
    {
        static MidiDeviceListConnectionBroadcaster result;
        return result;
    }

private:
    MidiDeviceListConnectionBroadcaster() = default;

    class State
    {
        Array<MidiDeviceInfo> ins  = MidiInput::getAvailableDevices(),
                              outs = MidiOutput::getAvailableDevices();
        auto tie() const
        {
            return std::tie (ins, outs);
        }

    public:
        b8 operator== (const State& other) const
        {
            return tie() == other.tie();
        }
        b8 operator!= (const State& other) const
        {
            return tie() != other.tie();
        }
    };

    z0 handleAsyncUpdate() override
    {
        notify();
    }

    std::map<MidiDeviceListConnection::Key, std::function<z0()>> callbacks;
    std::optional<State> lastNotifiedState;
    MidiDeviceListConnection::Key key = 0;
};

} // namespace drx
