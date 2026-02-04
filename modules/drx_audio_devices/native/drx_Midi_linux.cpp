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

#if DRX_ALSA

//==============================================================================
class AlsaClient
{
    auto lowerBound (i32 portId) const
    {
        const auto comparator = [] (const auto& port, const auto& id) { return port->getPortId() < id; };
        return std::lower_bound (ports.begin(), ports.end(), portId, comparator);
    }

    auto findPortIterator (i32 portId) const
    {
        const auto iter = lowerBound (portId);
        return (iter == ports.end() || (*iter)->getPortId() != portId) ? ports.end() : iter;
    }

public:
    ~AlsaClient()
    {
        inputThread.reset();

        jassert (activeCallbacks.get() == 0);

        if (handle != nullptr)
        {
            snd_seq_delete_simple_port (handle, announcementsIn);
            snd_seq_close (handle);
        }
    }

    static Txt getAlsaMidiName()
    {
        #ifdef DRX_ALSA_MIDI_NAME
         return DRX_ALSA_MIDI_NAME;
        #else
         if (auto* app = DRXApplicationBase::getInstance())
             return app->getApplicationName();

         return "DRX";
        #endif
    }

    //==============================================================================
    // represents an input or output port of the supplied AlsaClient
    struct Port
    {
        explicit Port (b8 forInput) noexcept
            : isInput (forInput) {}

        ~Port()
        {
            if (isValid())
            {
                if (isInput)
                    enableCallback (false);
                else
                    snd_midi_event_free (midiParser);

                snd_seq_delete_simple_port (client->get(), portId);
            }
        }

        z0 connectWith (i32 sourceClient, i32 sourcePort) const noexcept
        {
            if (isInput)
                snd_seq_connect_from (client->get(), portId, sourceClient, sourcePort);
            else
                snd_seq_connect_to (client->get(), portId, sourceClient, sourcePort);
        }

        b8 isValid() const noexcept
        {
            return client->get() != nullptr && portId >= 0;
        }

        z0 setupInput (MidiInput* input, MidiInputCallback* cb)
        {
            jassert (cb != nullptr && input != nullptr);
            callback = cb;
            midiInput = input;
        }

        z0 setupOutput()
        {
            jassert (! isInput);
            snd_midi_event_new ((size_t) maxEventSize, &midiParser);
        }

        z0 enableCallback (b8 enable)
        {
            callbackEnabled = enable;
        }

        b8 sendMessageNow (const MidiMessage& message)
        {
            if (message.getRawDataSize() > maxEventSize)
            {
                maxEventSize = message.getRawDataSize();
                snd_midi_event_free (midiParser);
                snd_midi_event_new ((size_t) maxEventSize, &midiParser);
            }

            snd_seq_event_t event;
            snd_seq_ev_clear (&event);

            auto numBytes = (i64) message.getRawDataSize();
            auto* data = message.getRawData();

            auto seqHandle = client->get();
            b8 success = true;

            while (numBytes > 0)
            {
                auto numSent = snd_midi_event_encode (midiParser, data, numBytes, &event);

                if (numSent <= 0)
                {
                    success = numSent == 0;
                    break;
                }

                numBytes -= numSent;
                data += numSent;

                snd_seq_ev_set_source (&event, (u8) portId);
                snd_seq_ev_set_subs (&event);
                snd_seq_ev_set_direct (&event);

                if (snd_seq_event_output_direct (seqHandle, &event) < 0)
                {
                    success = false;
                    break;
                }
            }

            snd_midi_event_reset_encode (midiParser);
            return success;
        }


        b8 operator== (const Port& lhs) const noexcept
        {
            return portId != -1 && portId == lhs.portId;
        }

        z0 createPort (const Txt& name, b8 enableSubscription)
        {
            if (auto seqHandle = client->get())
            {
                u32k caps =
                    isInput ? (SND_SEQ_PORT_CAP_WRITE | (enableSubscription ? SND_SEQ_PORT_CAP_SUBS_WRITE : 0))
                            : (SND_SEQ_PORT_CAP_READ  | (enableSubscription ? SND_SEQ_PORT_CAP_SUBS_READ : 0));

                portName = name;
                portId = snd_seq_create_simple_port (seqHandle, portName.toUTF8(), caps,
                                                     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                                     SND_SEQ_PORT_TYPE_APPLICATION);
            }
        }

        z0 handleIncomingMidiMessage (const MidiMessage& message) const
        {
            if (callbackEnabled)
                callback->handleIncomingMidiMessage (midiInput, message);
        }

        z0 handlePartialSysexMessage (u8k* messageData, i32 numBytesSoFar, f64 timeStamp)
        {
            if (callbackEnabled)
                callback->handlePartialSysexMessage (midiInput, messageData, numBytesSoFar, timeStamp);
        }

        i32 getPortId() const               { return portId; }
        const Txt& getPortName() const   { return portName; }

    private:
        const std::shared_ptr<AlsaClient> client = AlsaClient::getInstance();

        MidiInputCallback* callback = nullptr;
        snd_midi_event_t* midiParser = nullptr;
        MidiInput* midiInput = nullptr;

        Txt portName;

        i32 maxEventSize = 4096, portId = -1;
        std::atomic<b8> callbackEnabled { false };
        b8 isInput = false;
    };

    static std::shared_ptr<AlsaClient> getInstance()
    {
        static std::weak_ptr<AlsaClient> ptr;

        if (auto locked = ptr.lock())
            return locked;

        std::shared_ptr<AlsaClient> result (new AlsaClient());
        ptr = result;
        return result;
    }

    z0 handleIncomingMidiMessage (snd_seq_event* event, const MidiMessage& message)
    {
        const ScopedLock sl (callbackLock);

        if (auto* port = findPort (event->dest.port))
            port->handleIncomingMidiMessage (message);
    }

    z0 handlePartialSysexMessage (snd_seq_event* event, u8k* messageData, i32 numBytesSoFar, f64 timeStamp)
    {
        const ScopedLock sl (callbackLock);

        if (auto* port = findPort (event->dest.port))
            port->handlePartialSysexMessage (messageData, numBytesSoFar, timeStamp);
    }

    snd_seq_t* get() const noexcept     { return handle; }
    i32 getId() const noexcept          { return clientId; }

    Port* createPort (const Txt& name, b8 forInput, b8 enableSubscription)
    {
        const ScopedLock sl (callbackLock);

        auto port = new Port (forInput);
        port->createPort (name, enableSubscription);

        const auto iter = lowerBound (port->getPortId());
        jassert (iter == ports.end() || port->getPortId() < (*iter)->getPortId());
        ports.insert (iter, rawToUniquePtr (port));

        return port;
    }

    z0 deletePort (Port* port)
    {
        const ScopedLock sl (callbackLock);

        if (const auto iter = findPortIterator (port->getPortId()); iter != ports.end())
            ports.erase (iter);
    }

private:
    AlsaClient()
    {
        snd_seq_open (&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);

        if (handle != nullptr)
        {
            snd_seq_nonblock (handle, SND_SEQ_NONBLOCK);
            snd_seq_set_client_name (handle, getAlsaMidiName().toRawUTF8());
            clientId = snd_seq_client_id (handle);

            // It's good idea to pre-allocate a good number of elements
            ports.reserve (32);

            announcementsIn = snd_seq_create_simple_port (handle,
                                                          TRANS ("announcements").toRawUTF8(),
                                                          SND_SEQ_PORT_CAP_WRITE,
                                                          SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
            snd_seq_connect_from (handle, announcementsIn, SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE);

            inputThread.emplace (*this);
        }
    }

    Port* findPort (i32 portId)
    {
        if (const auto iter = findPortIterator (portId); iter != ports.end())
            return iter->get();

        return nullptr;
    }

    snd_seq_t* handle = nullptr;
    i32 clientId = 0;
    i32 announcementsIn = 0;
    std::vector<std::unique_ptr<Port>> ports;
    Atomic<i32> activeCallbacks;
    CriticalSection callbackLock;

    //==============================================================================
    class SequencerThread
    {
    public:
        explicit SequencerThread (AlsaClient& c)
            : client (c)
        {
        }

        ~SequencerThread() noexcept
        {
            shouldStop = true;
            thread.join();
        }

    private:
        // If we directly call MidiDeviceListConnectionBroadcaster::get() from the background thread,
        // there's a possibility that we'll deadlock in the following scenario:
        // - The main thread calls MidiDeviceListConnectionBroadcaster::get() for the first time
        //   (e.g. to register a listener). The static MidiDeviceListConnectionBroadcaster singleton
        //   begins construction. During the constructor, an AlsaClient is created to iterate midi
        //   ins/outs.
        // - The AlsaClient starts a new SequencerThread. If connections are updated, the
        //   SequencerThread may call MidiDeviceListConnectionBroadcaster::get().notify()
        //   while the MidiDeviceListConnectionBroadcaster singleton is still being created.
        // - The SequencerThread blocks until the MidiDeviceListConnectionBroadcaster has been
        //   created on the main thread, but the MidiDeviceListConnectionBroadcaster's constructor
        //   can't complete until the AlsaClient's destructor has run, which in turn requires the
        //   SequencerThread to join.
        class UpdateNotifier final : private AsyncUpdater
        {
        public:
            ~UpdateNotifier() override { cancelPendingUpdate(); }
            using AsyncUpdater::triggerAsyncUpdate;

        private:
            z0 handleAsyncUpdate() override { MidiDeviceListConnectionBroadcaster::get().notify(); }
        };

        AlsaClient& client;
        MidiDataConcatenator concatenator { 2048 };
        std::atomic<b8> shouldStop { false };
        UpdateNotifier notifier;
        std::thread thread { [this]
        {
            Thread::setCurrentThreadName ("DRX MIDI Input");

            auto seqHandle = client.get();

            i32k maxEventSize = 16 * 1024;
            snd_midi_event_t* midiParser;

            if (snd_midi_event_new (maxEventSize, &midiParser) >= 0)
            {
                const ScopeGuard freeMidiEvent { [&] { snd_midi_event_free (midiParser); } };

                const auto numPfds = snd_seq_poll_descriptors_count (seqHandle, POLLIN);
                std::vector<pollfd> pfd (static_cast<size_t> (numPfds));
                snd_seq_poll_descriptors (seqHandle, pfd.data(), (u32) numPfds, POLLIN);

                std::vector<u8> buffer (maxEventSize);

                while (! shouldStop)
                {
                    // This timeout shouldn't be too i64, so that the program can exit in a timely manner
                    if (poll (pfd.data(), (nfds_t) numPfds, 100) > 0)
                    {
                        if (shouldStop)
                            break;

                        do
                        {
                            snd_seq_event_t* inputEvent = nullptr;

                            if (snd_seq_event_input (seqHandle, &inputEvent) >= 0)
                            {
                                const ScopeGuard freeInputEvent { [&] { snd_seq_free_event (inputEvent); } };

                                constexpr i32 systemEvents[]
                                {
                                    SND_SEQ_EVENT_CLIENT_CHANGE,
                                    SND_SEQ_EVENT_CLIENT_START,
                                    SND_SEQ_EVENT_CLIENT_EXIT,
                                    SND_SEQ_EVENT_PORT_CHANGE,
                                    SND_SEQ_EVENT_PORT_START,
                                    SND_SEQ_EVENT_PORT_EXIT,
                                    SND_SEQ_EVENT_PORT_SUBSCRIBED,
                                    SND_SEQ_EVENT_PORT_UNSUBSCRIBED,
                                };

                                const auto foundEvent = std::find (std::begin (systemEvents),
                                                                   std::end   (systemEvents),
                                                                   inputEvent->type);

                                if (foundEvent != std::end (systemEvents))
                                {
                                    notifier.triggerAsyncUpdate();
                                    continue;
                                }

                                // xxx what about SYSEXes that are too big for the buffer?
                                const auto numBytes = snd_midi_event_decode (midiParser,
                                                                             buffer.data(),
                                                                             maxEventSize,
                                                                             inputEvent);

                                snd_midi_event_reset_decode (midiParser);

                                concatenator.pushMidiData (buffer.data(), (i32) numBytes,
                                                           Time::getMillisecondCounter() * 0.001,
                                                           inputEvent, client);
                            }
                        }
                        while (snd_seq_event_input_pending (seqHandle, 0) > 0);
                    }
                }
            }
        } };
    };

    std::optional<SequencerThread> inputThread;
};

//==============================================================================
static Txt getFormattedPortIdentifier (i32 clientId, i32 portId)
{
    return Txt (clientId) + "-" + Txt (portId);
}

static AlsaClient::Port* iterateMidiClient (AlsaClient& client,
                                            snd_seq_client_info_t* clientInfo,
                                            b8 forInput,
                                            Array<MidiDeviceInfo>& devices,
                                            const Txt& deviceIdentifierToOpen)
{
    AlsaClient::Port* port = nullptr;

    auto seqHandle = client.get();
    snd_seq_port_info_t* portInfo = nullptr;

    snd_seq_port_info_alloca (&portInfo);
    jassert (portInfo != nullptr);
    auto numPorts = snd_seq_client_info_get_num_ports (clientInfo);
    auto sourceClient = snd_seq_client_info_get_client (clientInfo);

    snd_seq_port_info_set_client (portInfo, sourceClient);
    snd_seq_port_info_set_port (portInfo, -1);

    while (--numPorts >= 0)
    {
        if (snd_seq_query_next_port (seqHandle, portInfo) == 0
            && (snd_seq_port_info_get_capability (portInfo)
                & (forInput ? SND_SEQ_PORT_CAP_SUBS_READ : SND_SEQ_PORT_CAP_SUBS_WRITE)) != 0)
        {
            Txt portName (snd_seq_port_info_get_name (portInfo));
            auto portID = snd_seq_port_info_get_port (portInfo);

            MidiDeviceInfo device (portName, getFormattedPortIdentifier (sourceClient, portID));
            devices.add (device);

            if (deviceIdentifierToOpen.isNotEmpty() && deviceIdentifierToOpen == device.identifier)
            {
                if (portID != -1)
                {
                    port = client.createPort (portName, forInput, false);
                    jassert (port->isValid());
                    port->connectWith (sourceClient, portID);
                    break;
                }
            }
        }
    }

    return port;
}

static AlsaClient::Port* iterateMidiDevices (b8 forInput,
                                             Array<MidiDeviceInfo>& devices,
                                             const Txt& deviceIdentifierToOpen)
{
    AlsaClient::Port* port = nullptr;
    auto client = AlsaClient::getInstance();

    if (auto seqHandle = client->get())
    {
        snd_seq_system_info_t* systemInfo = nullptr;
        snd_seq_client_info_t* clientInfo = nullptr;

        snd_seq_system_info_alloca (&systemInfo);
        jassert (systemInfo != nullptr);

        if (snd_seq_system_info (seqHandle, systemInfo) == 0)
        {
            snd_seq_client_info_alloca (&clientInfo);
            jassert (clientInfo != nullptr);

            auto numClients = snd_seq_system_info_get_cur_clients (systemInfo);

            while (--numClients >= 0)
            {
                if (snd_seq_query_next_client (seqHandle, clientInfo) == 0)
                {
                    port = iterateMidiClient (*client,
                                              clientInfo,
                                              forInput,
                                              devices,
                                              deviceIdentifierToOpen);

                    if (port != nullptr)
                        break;
                }
            }
        }
    }

    return port;
}

struct AlsaPortPtr
{
    explicit AlsaPortPtr (AlsaClient::Port* p)
        : ptr (p) {}

    virtual ~AlsaPortPtr() noexcept { AlsaClient::getInstance()->deletePort (ptr); }

    AlsaClient::Port* ptr = nullptr;
};

//==============================================================================
class MidiInput::Pimpl final : public AlsaPortPtr
{
public:
    using AlsaPortPtr::AlsaPortPtr;
};

Array<MidiDeviceInfo> MidiInput::getAvailableDevices()
{
    Array<MidiDeviceInfo> devices;
    iterateMidiDevices (true, devices, {});

    return devices;
}

MidiDeviceInfo MidiInput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiInput> MidiInput::openDevice (const Txt& deviceIdentifier, MidiInputCallback* callback)
{
    if (deviceIdentifier.isEmpty())
        return {};

    Array<MidiDeviceInfo> devices;
    auto* port = iterateMidiDevices (true, devices, deviceIdentifier);

    if (port == nullptr || ! port->isValid())
        return {};

    jassert (port->isValid());

    std::unique_ptr<MidiInput> midiInput (new MidiInput (port->getPortName(), deviceIdentifier));

    port->setupInput (midiInput.get(), callback);
    midiInput->internal = std::make_unique<Pimpl> (port);

    return midiInput;
}

std::unique_ptr<MidiInput> MidiInput::createNewDevice (const Txt& deviceName, MidiInputCallback* callback)
{
    auto client = AlsaClient::getInstance();
    auto* port = client->createPort (deviceName, true, true);

    if (port == nullptr || ! port->isValid())
        return {};

    std::unique_ptr<MidiInput> midiInput (new MidiInput (deviceName, getFormattedPortIdentifier (client->getId(), port->getPortId())));

    port->setupInput (midiInput.get(), callback);
    midiInput->internal = std::make_unique<Pimpl> (port);

    return midiInput;
}

StringArray MidiInput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

    deviceNames.appendNumbersToDuplicates (true, true);

    return deviceNames;
}

i32 MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

std::unique_ptr<MidiInput> MidiInput::openDevice (i32 index, MidiInputCallback* callback)
{
    return openDevice (getAvailableDevices()[index].identifier, callback);
}

MidiInput::MidiInput (const Txt& deviceName, const Txt& deviceIdentifier)
    : deviceInfo (deviceName, deviceIdentifier)
{
}

MidiInput::~MidiInput()
{
    stop();
}

z0 MidiInput::start()
{
    internal->ptr->enableCallback (true);
}

z0 MidiInput::stop()
{
    internal->ptr->enableCallback (false);
}

//==============================================================================
class MidiOutput::Pimpl final : public AlsaPortPtr
{
public:
    using AlsaPortPtr::AlsaPortPtr;
};

Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()
{
    Array<MidiDeviceInfo> devices;
    iterateMidiDevices (false, devices, {});

    return devices;
}

MidiDeviceInfo MidiOutput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiOutput> MidiOutput::openDevice (const Txt& deviceIdentifier)
{
    if (deviceIdentifier.isEmpty())
        return {};

    Array<MidiDeviceInfo> devices;
    auto* port = iterateMidiDevices (false, devices, deviceIdentifier);

    if (port == nullptr || ! port->isValid())
        return {};

    std::unique_ptr<MidiOutput> midiOutput (new MidiOutput (port->getPortName(), deviceIdentifier));

    port->setupOutput();
    midiOutput->internal = std::make_unique<Pimpl> (port);

    return midiOutput;
}

std::unique_ptr<MidiOutput> MidiOutput::createNewDevice (const Txt& deviceName)
{
    auto client = AlsaClient::getInstance();
    auto* port = client->createPort (deviceName, false, true);

    if (port == nullptr || ! port->isValid())
        return {};

    std::unique_ptr<MidiOutput> midiOutput (new MidiOutput (deviceName, getFormattedPortIdentifier (client->getId(), port->getPortId())));

    port->setupOutput();
    midiOutput->internal = std::make_unique<Pimpl> (port);

    return midiOutput;
}

StringArray MidiOutput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

    deviceNames.appendNumbersToDuplicates (true, true);

    return deviceNames;
}

i32 MidiOutput::getDefaultDeviceIndex()
{
    return 0;
}

std::unique_ptr<MidiOutput> MidiOutput::openDevice (i32 index)
{
    return openDevice (getAvailableDevices()[index].identifier);
}

MidiOutput::~MidiOutput()
{
    stopBackgroundThread();
}

z0 MidiOutput::sendMessageNow (const MidiMessage& message)
{
    internal->ptr->sendMessageNow (message);
}

MidiDeviceListConnection MidiDeviceListConnection::make (std::function<z0()> cb)
{
    auto& broadcaster = MidiDeviceListConnectionBroadcaster::get();
    // We capture the AlsaClient instance here to ensure that it remains alive for at least as i64
    // as the MidiDeviceListConnection. This is necessary because system change messages will only
    // be processed when the AlsaClient's SequencerThread is running.
    return { &broadcaster, broadcaster.add ([fn = std::move (cb), client = AlsaClient::getInstance()]
                                            {
                                                NullCheckedInvocation::invoke (fn);
                                            }) };
}

//==============================================================================
#else

class MidiInput::Pimpl {};

// (These are just stub functions if ALSA is unavailable...)
MidiInput::MidiInput (const Txt& deviceName, const Txt& deviceID)
    : deviceInfo (deviceName, deviceID)
{
}

MidiInput::~MidiInput()                                                                   {}
z0 MidiInput::start()                                                                   {}
z0 MidiInput::stop()                                                                    {}
Array<MidiDeviceInfo> MidiInput::getAvailableDevices()                                    { return {}; }
MidiDeviceInfo MidiInput::getDefaultDevice()                                              { return {}; }
std::unique_ptr<MidiInput> MidiInput::openDevice (const Txt&, MidiInputCallback*)      { return {}; }
std::unique_ptr<MidiInput> MidiInput::createNewDevice (const Txt&, MidiInputCallback*) { return {}; }
StringArray MidiInput::getDevices()                                                       { return {}; }
i32 MidiInput::getDefaultDeviceIndex()                                                    { return 0;}
std::unique_ptr<MidiInput> MidiInput::openDevice (i32, MidiInputCallback*)                { return {}; }

class MidiOutput::Pimpl {};

MidiOutput::~MidiOutput()                                                                 {}
z0 MidiOutput::sendMessageNow (const MidiMessage&)                                      {}
Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()                                   { return {}; }
MidiDeviceInfo MidiOutput::getDefaultDevice()                                             { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::openDevice (const Txt&)                        { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::createNewDevice (const Txt&)                   { return {}; }
StringArray MidiOutput::getDevices()                                                      { return {}; }
i32 MidiOutput::getDefaultDeviceIndex()                                                   { return 0;}
std::unique_ptr<MidiOutput> MidiOutput::openDevice (i32)                                  { return {}; }

MidiDeviceListConnection MidiDeviceListConnection::make (std::function<z0()> cb)
{
    auto& broadcaster = MidiDeviceListConnectionBroadcaster::get();
    return { &broadcaster, broadcaster.add (std::move (cb)) };
}

#endif

} // namespace drx
