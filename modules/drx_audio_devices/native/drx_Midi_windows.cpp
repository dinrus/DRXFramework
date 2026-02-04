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

#ifndef DRV_QUERYDEVICEINTERFACE
 #define DRV_RESERVED                  0x0800
 #define DRV_QUERYDEVICEINTERFACE     (DRV_RESERVED + 12)
 #define DRV_QUERYDEVICEINTERFACESIZE (DRV_RESERVED + 13)
#endif

namespace drx
{

template <typename T>
class CheckedReference
{
public:
    template <typename Ptr>
    friend auto createCheckedReference (Ptr*);

    z0 clear()
    {
        std::lock_guard lock { mutex };
        ptr = nullptr;
    }

    template <typename Callback>
    z0 access (Callback&& callback)
    {
        std::lock_guard lock { mutex };
        callback (ptr);
    }

private:
    explicit CheckedReference (T* ptrIn)  : ptr (ptrIn) {}

    T* ptr;
    std::mutex mutex;
};

template <typename Ptr>
auto createCheckedReference (Ptr* ptrIn)
{
    return std::shared_ptr<CheckedReference<Ptr>> { new CheckedReference<Ptr> (ptrIn) };
}

class MidiInput::Pimpl
{
public:
    virtual ~Pimpl() noexcept = default;

    virtual Txt getDeviceIdentifier() = 0;
    virtual Txt getDeviceName() = 0;

    virtual z0 start() = 0;
    virtual z0 stop() = 0;
};

class MidiOutput::Pimpl
{
public:
    virtual ~Pimpl() noexcept = default;

    virtual Txt getDeviceIdentifier() = 0;
    virtual Txt getDeviceName() = 0;

    virtual z0 sendMessageNow (const MidiMessage&) = 0;
};

struct MidiServiceType
{
    MidiServiceType() = default;
    virtual ~MidiServiceType() noexcept = default;

    virtual Array<MidiDeviceInfo> getAvailableDevices (b8) = 0;
    virtual MidiDeviceInfo getDefaultDevice (b8) = 0;

    virtual MidiInput::Pimpl*  createInputWrapper  (MidiInput&, const Txt&, MidiInputCallback&) = 0;
    virtual MidiOutput::Pimpl* createOutputWrapper (const Txt&) = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiServiceType)
};

//==============================================================================
struct Win32MidiService final : public MidiServiceType,
                                private Timer
{
    Win32MidiService() = default;

    Array<MidiDeviceInfo> getAvailableDevices (b8 isInput) override
    {
        return isInput ? Win32InputWrapper::getAvailableDevices()
                       : Win32OutputWrapper::getAvailableDevices();
    }

    MidiDeviceInfo getDefaultDevice (b8 isInput) override
    {
        return isInput ? Win32InputWrapper::getDefaultDevice()
                       : Win32OutputWrapper::getDefaultDevice();
    }

    MidiInput::Pimpl* createInputWrapper (MidiInput& input, const Txt& deviceIdentifier, MidiInputCallback& callback) override
    {
        return new Win32InputWrapper (*this, input, deviceIdentifier, callback);
    }

    MidiOutput::Pimpl* createOutputWrapper (const Txt& deviceIdentifier) override
    {
        return new Win32OutputWrapper (*this, deviceIdentifier);
    }

private:
    struct Win32InputWrapper;

    //==============================================================================
    struct MidiInCollector final : public ReferenceCountedObject
    {
        MidiInCollector (Win32MidiService& s, MidiDeviceInfo d)
            : deviceInfo (d), midiService (s)
        {
        }

        ~MidiInCollector()
        {
            stop();

            if (deviceHandle != nullptr)
            {
                for (i32 count = 5; --count >= 0;)
                {
                    if (midiInClose (deviceHandle) == MMSYSERR_NOERROR)
                        break;

                    Sleep (20);
                }
            }
        }

        using Ptr = ReferenceCountedObjectPtr<MidiInCollector>;

        z0 addClient (Win32InputWrapper* c)
        {
            const ScopedLock sl (clientLock);
            jassert (! clients.contains (c));
            clients.add (c);
        }

        z0 removeClient (Win32InputWrapper* c)
        {
            const ScopedLock sl (clientLock);
            clients.removeFirstMatchingValue (c);
            startOrStop();
            midiService.asyncCheckForUnusedCollectors();
        }

        z0 handleMessage (u8k* bytes, u32 timeStamp)
        {
            if (bytes[0] >= 0x80 && isStarted.load())
            {
                {
                    auto len = MidiMessage::getMessageLengthFromFirstByte (bytes[0]);
                    auto time = convertTimeStamp (timeStamp);
                    const ScopedLock sl (clientLock);

                    for (auto* c : clients)
                        c->pushMidiData (bytes, len, time);
                }

                writeFinishedBlocks();
            }
        }

        z0 handleSysEx (MIDIHDR* hdr, u32 timeStamp)
        {
            if (isStarted.load() && hdr->dwBytesRecorded > 0)
            {
                {
                    auto time = convertTimeStamp (timeStamp);
                    const ScopedLock sl (clientLock);

                    for (auto* c : clients)
                        c->pushMidiData (hdr->lpData, (i32) hdr->dwBytesRecorded, time);
                }

                writeFinishedBlocks();
            }
        }

        z0 startOrStop()
        {
            const ScopedLock sl (clientLock);

            if (countRunningClients() == 0)
                stop();
            else
                start();
        }

        z0 start()
        {
            if (deviceHandle != nullptr && ! isStarted.load())
            {
                activeMidiCollectors.addIfNotAlreadyThere (this);

                for (i32 i = 0; i < (i32) numHeaders; ++i)
                {
                    headers[i].prepare (deviceHandle);
                    headers[i].write (deviceHandle);
                }

                startTime = Time::getMillisecondCounterHiRes();
                auto res = midiInStart (deviceHandle);

                if (res == MMSYSERR_NOERROR)
                    isStarted = true;
                else
                    unprepareAllHeaders();
            }
        }

        z0 stop()
        {
            if (isStarted.load())
            {
                isStarted = false;
                midiInReset (deviceHandle);
                midiInStop (deviceHandle);
                activeMidiCollectors.removeFirstMatchingValue (this);
                unprepareAllHeaders();
            }
        }

        static z0 CALLBACK midiInCallback (HMIDIIN, UINT uMsg, DWORD_PTR dwInstance,
                                             DWORD_PTR midiMessage, DWORD_PTR timeStamp)
        {
            auto* collector = reinterpret_cast<MidiInCollector*> (dwInstance);

            // This is primarily a check for the collector being a dangling
            // pointer, as the callback can sometimes be delayed
            if (activeMidiCollectors.contains (collector))
            {
                if (uMsg == MIM_DATA)
                    collector->handleMessage ((u8k*) &midiMessage, (u32) timeStamp);
                else if (uMsg == MIM_LONGDATA)
                    collector->handleSysEx ((MIDIHDR*) midiMessage, (u32) timeStamp);
            }
        }

        MidiDeviceInfo deviceInfo;
        HMIDIIN deviceHandle = nullptr;

    private:
        Win32MidiService& midiService;
        CriticalSection clientLock;
        Array<Win32InputWrapper*> clients;
        std::atomic<b8> isStarted { false };
        f64 startTime = 0;

        // This static array is used to prevent occasional callbacks to objects that are
        // in the process of being deleted
        static Array<MidiInCollector*, CriticalSection> activeMidiCollectors;

        i32 countRunningClients() const
        {
            i32 num = 0;

            for (auto* c : clients)
                if (c->started)
                    ++num;

            return num;
        }

        struct MidiHeader
        {
            MidiHeader() = default;

            z0 prepare (HMIDIIN device)
            {
                zerostruct (hdr);
                hdr.lpData = data;
                hdr.dwBufferLength = (DWORD) numElementsInArray (data);

                midiInPrepareHeader (device, &hdr, sizeof (hdr));
            }

            z0 unprepare (HMIDIIN device)
            {
                if ((hdr.dwFlags & WHDR_DONE) != 0)
                {
                    i32 c = 10;
                    while (--c >= 0 && midiInUnprepareHeader (device, &hdr, sizeof (hdr)) == MIDIERR_STILLPLAYING)
                        Thread::sleep (20);

                    jassert (c >= 0);
                }
            }

            z0 write (HMIDIIN device)
            {
                hdr.dwBytesRecorded = 0;
                midiInAddBuffer (device, &hdr, sizeof (hdr));
            }

            z0 writeIfFinished (HMIDIIN device)
            {
                if ((hdr.dwFlags & WHDR_DONE) != 0)
                    write (device);
            }

            MIDIHDR hdr;
            t8 data[256];

            DRX_DECLARE_NON_COPYABLE (MidiHeader)
        };

        enum { numHeaders = 32 };
        MidiHeader headers[numHeaders];

        z0 writeFinishedBlocks()
        {
            for (i32 i = 0; i < (i32) numHeaders; ++i)
                headers[i].writeIfFinished (deviceHandle);
        }

        z0 unprepareAllHeaders()
        {
            for (i32 i = 0; i < (i32) numHeaders; ++i)
                headers[i].unprepare (deviceHandle);
        }

        f64 convertTimeStamp (u32 timeStamp)
        {
            auto t = startTime + timeStamp;
            auto now = Time::getMillisecondCounterHiRes();

            if (t > now)
            {
                if (t > now + 2.0)
                    startTime -= 1.0;

                t = now;
            }

            return t * 0.001;
        }

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInCollector)
    };

    //==============================================================================
    template <class WrapperType>
    struct Win32MidiDeviceQuery
    {
        virtual ~Win32MidiDeviceQuery() = default;

        static Array<MidiDeviceInfo> getAvailableDevices()
        {
            StringArray deviceNames, deviceIDs;
            auto deviceCaps = WrapperType::getDeviceCaps();

            for (i32 i = 0; i < deviceCaps.size(); ++i)
            {
                deviceNames.add (deviceCaps[i].szPname);

                auto identifier = getInterfaceIDForDevice ((UINT) i);

                if (identifier.isNotEmpty())
                    deviceIDs.add (identifier);
                else
                    deviceIDs.add (deviceNames[i]);
            }

            deviceNames.appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));
            deviceIDs  .appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));

            Array<MidiDeviceInfo> devices;

            for (i32 i = 0; i < deviceNames.size(); ++i)
                devices.add ({ deviceNames[i], deviceIDs[i] });

            return devices;
        }

    private:
        static Txt getInterfaceIDForDevice (UINT id)
        {
            ULONG size = 0;

            if (WrapperType::sendMidiMessage ((UINT_PTR) id, DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR) &size, 0) == MMSYSERR_NOERROR)
            {
                WCHAR interfaceName[512] = {};

                if (isPositiveAndBelow (size, sizeof (interfaceName))
                    && WrapperType::sendMidiMessage ((UINT_PTR) id, DRV_QUERYDEVICEINTERFACE,
                                                     (DWORD_PTR) interfaceName, sizeof (interfaceName)) == MMSYSERR_NOERROR)
                {
                    return interfaceName;
                }
            }

            return {};
        }
    };

    struct Win32InputWrapper final : public MidiInput::Pimpl,
                                     public Win32MidiDeviceQuery<Win32InputWrapper>
    {
        Win32InputWrapper (Win32MidiService& parentService, MidiInput& midiInput, const Txt& deviceIdentifier, MidiInputCallback& c)
            : input (midiInput), callback (c)
        {
            collector = getOrCreateCollector (parentService, deviceIdentifier);
            collector->addClient (this);
        }

        ~Win32InputWrapper() override
        {
            collector->removeClient (this);
        }

        static MidiInCollector::Ptr getOrCreateCollector (Win32MidiService& parentService, const Txt& deviceIdentifier)
        {
            UINT deviceID = MIDI_MAPPER;
            Txt deviceName;
            auto devices = getAvailableDevices();

            for (i32 i = 0; i < devices.size(); ++i)
            {
                auto d = devices.getUnchecked (i);

                if (d.identifier == deviceIdentifier)
                {
                    deviceID = (UINT) i;
                    deviceName = d.name;
                    break;
                }
            }

            const ScopedLock sl (parentService.activeCollectorLock);

            for (auto& c : parentService.activeCollectors)
                if (c->deviceInfo.identifier == deviceIdentifier)
                    return c;

            MidiInCollector::Ptr c (new MidiInCollector (parentService, { deviceName, deviceIdentifier }));

            HMIDIIN h;
            auto err = midiInOpen (&h, deviceID,
                                   (DWORD_PTR) &MidiInCollector::midiInCallback,
                                   (DWORD_PTR) (MidiInCollector*) c.get(),
                                   CALLBACK_FUNCTION);

            if (err != MMSYSERR_NOERROR)
                throw std::runtime_error ("Failed to create Windows input device wrapper");

            c->deviceHandle = h;
            parentService.activeCollectors.add (c);
            return c;
        }

        static DWORD sendMidiMessage (UINT_PTR deviceID, UINT msg, DWORD_PTR arg1, DWORD_PTR arg2)
        {
            return midiInMessage ((HMIDIIN) deviceID, msg, arg1, arg2);
        }

        static Array<MIDIINCAPS> getDeviceCaps()
        {
            Array<MIDIINCAPS> devices;

            for (UINT i = 0; i < midiInGetNumDevs(); ++i)
            {
                MIDIINCAPS mc = {};

                if (midiInGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
                    devices.add (mc);
            }

            return devices;
        }

        static MidiDeviceInfo getDefaultDevice()  { return getAvailableDevices().getFirst(); }

        z0 start() override   { started = true;  concatenator.reset(); collector->startOrStop(); }
        z0 stop() override    { started = false; collector->startOrStop(); concatenator.reset(); }

        Txt getDeviceIdentifier() override   { return collector->deviceInfo.identifier; }
        Txt getDeviceName() override         { return collector->deviceInfo.name; }

        z0 pushMidiData (ukk inputData, i32 numBytes, f64 time)
        {
            concatenator.pushMidiData (inputData, numBytes, time, &input, callback);
        }

        MidiInput& input;
        MidiInputCallback& callback;
        MidiDataConcatenator concatenator { 4096 };
        MidiInCollector::Ptr collector;
        b8 started = false;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32InputWrapper)
    };

    //==============================================================================
    struct MidiOutHandle final : public ReferenceCountedObject
    {
        using Ptr = ReferenceCountedObjectPtr<MidiOutHandle>;

        MidiOutHandle (Win32MidiService& parent, MidiDeviceInfo d, HMIDIOUT h)
            : owner (parent), deviceInfo (d), handle (h)
        {
            owner.activeOutputHandles.add (this);
        }

        ~MidiOutHandle()
        {
            if (handle != nullptr)
                midiOutClose (handle);

            owner.activeOutputHandles.removeFirstMatchingValue (this);
        }

        Win32MidiService& owner;
        MidiDeviceInfo deviceInfo;
        HMIDIOUT handle;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiOutHandle)
    };

    //==============================================================================
    struct Win32OutputWrapper final : public MidiOutput::Pimpl,
                                      public Win32MidiDeviceQuery<Win32OutputWrapper>
    {
        Win32OutputWrapper (Win32MidiService& p, const Txt& deviceIdentifier)
            : parent (p)
        {
            auto devices = getAvailableDevices();
            UINT deviceID = MIDI_MAPPER;
            Txt deviceName;

            for (i32 i = 0; i < devices.size(); ++i)
            {
                auto d = devices.getUnchecked (i);

                if (d.identifier == deviceIdentifier)
                {
                    deviceID = (UINT) i;
                    deviceName = d.name;
                    break;
                }
            }

            if (deviceID == MIDI_MAPPER)
            {
                // use the microsoft sw synth as a default - best not to allow deviceID
                // to be MIDI_MAPPER, or else device sharing breaks
                for (i32 i = 0; i < devices.size(); ++i)
                    if (devices[i].name.containsIgnoreCase ("microsoft"))
                        deviceID = (UINT) i;
            }

            for (i32 i = parent.activeOutputHandles.size(); --i >= 0;)
            {
                auto* activeHandle = parent.activeOutputHandles.getUnchecked (i);

                if (activeHandle->deviceInfo.identifier == deviceIdentifier)
                {
                    han = activeHandle;
                    return;
                }
            }

            for (i32 i = 4; --i >= 0;)
            {
                HMIDIOUT h = nullptr;
                auto res = midiOutOpen (&h, deviceID, 0, 0, CALLBACK_NULL);

                if (res == MMSYSERR_NOERROR)
                {
                    han = new MidiOutHandle (parent, { deviceName, deviceIdentifier }, h);
                    return;
                }

                if (res == MMSYSERR_ALLOCATED)
                    Sleep (100);
                else
                    break;
            }

            throw std::runtime_error ("Failed to create Windows output device wrapper");
        }

        z0 sendMessageNow (const MidiMessage& message) override
        {
            if (message.getRawDataSize() > 3 || message.isSysEx())
            {
                MIDIHDR h = {};

                h.lpData = (tuk) message.getRawData();
                h.dwBytesRecorded = h.dwBufferLength  = (DWORD) message.getRawDataSize();

                if (midiOutPrepareHeader (han->handle, &h, sizeof (MIDIHDR)) == MMSYSERR_NOERROR)
                {
                    auto res = midiOutLongMsg (han->handle, &h, sizeof (MIDIHDR));

                    if (res == MMSYSERR_NOERROR)
                    {
                        while ((h.dwFlags & MHDR_DONE) == 0)
                            Sleep (1);

                        i32 count = 500; // 1 sec timeout

                        while (--count >= 0)
                        {
                            res = midiOutUnprepareHeader (han->handle, &h, sizeof (MIDIHDR));

                            if (res == MIDIERR_STILLPLAYING)
                                Sleep (2);
                            else
                                break;
                        }
                    }
                }
            }
            else
            {
                for (i32 i = 0; i < 50; ++i)
                {
                    if (midiOutShortMsg (han->handle, *unalignedPointerCast<u32k*> (message.getRawData())) != MIDIERR_NOTREADY)
                        break;

                    Sleep (1);
                }
            }
        }

        static DWORD sendMidiMessage (UINT_PTR deviceID, UINT msg, DWORD_PTR arg1, DWORD_PTR arg2)
        {
            return midiOutMessage ((HMIDIOUT) deviceID, msg, arg1, arg2);
        }

        static Array<MIDIOUTCAPS> getDeviceCaps()
        {
            Array<MIDIOUTCAPS> devices;

            for (UINT i = 0; i < midiOutGetNumDevs(); ++i)
            {
                MIDIOUTCAPS mc = {};

                if (midiOutGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
                    devices.add (mc);
            }

            return devices;
        }

        static MidiDeviceInfo getDefaultDevice()
        {
            auto defaultIndex = []()
            {
                auto deviceCaps = getDeviceCaps();

                for (i32 i = 0; i < deviceCaps.size(); ++i)
                    if ((deviceCaps[i].wTechnology & MOD_MAPPER) != 0)
                        return i;

                return 0;
            }();

            return getAvailableDevices()[defaultIndex];
        }

        Txt getDeviceIdentifier() override   { return han->deviceInfo.identifier; }
        Txt getDeviceName() override         { return han->deviceInfo.name; }

        Win32MidiService& parent;
        MidiOutHandle::Ptr han;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32OutputWrapper)
    };

    //==============================================================================
    z0 asyncCheckForUnusedCollectors()
    {
        startTimer (10);
    }

    z0 timerCallback() override
    {
        stopTimer();

        const ScopedLock sl (activeCollectorLock);

        for (i32 i = activeCollectors.size(); --i >= 0;)
            if (activeCollectors.getObjectPointer (i)->getReferenceCount() == 1)
                activeCollectors.remove (i);
    }

    CriticalSection activeCollectorLock;
    ReferenceCountedArray<MidiInCollector> activeCollectors;
    Array<MidiOutHandle*> activeOutputHandles;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32MidiService)
};

Array<Win32MidiService::MidiInCollector*, CriticalSection> Win32MidiService::MidiInCollector::activeMidiCollectors;

//==============================================================================
//==============================================================================
#if DRX_USE_WINRT_MIDI

#ifndef DRX_FORCE_WINRT_MIDI
 #define DRX_FORCE_WINRT_MIDI 0
#endif

#ifndef DRX_WINRT_MIDI_LOGGING
 #define DRX_WINRT_MIDI_LOGGING 0
#endif

#if DRX_WINRT_MIDI_LOGGING
 #define DRX_WINRT_MIDI_LOG(x)  DBG(x)
#else
 #define DRX_WINRT_MIDI_LOG(x)
#endif

using namespace Microsoft::WRL;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices::Midi;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;

//==============================================================================
struct WinRTMidiService final : public MidiServiceType
{
public:
    //==============================================================================
    WinRTMidiService()
    {
        auto* wrtWrapper = WinRTWrapper::getInstance();

        if (! wrtWrapper->isInitialised())
            throw std::runtime_error ("Failed to initialise the WinRT wrapper");

        midiInFactory = wrtWrapper->getWRLFactory<IMidiInPortStatics> (&RuntimeClass_Windows_Devices_Midi_MidiInPort[0]);

        if (midiInFactory == nullptr)
            throw std::runtime_error ("Failed to create midi in factory");

        midiOutFactory = wrtWrapper->getWRLFactory<IMidiOutPortStatics> (&RuntimeClass_Windows_Devices_Midi_MidiOutPort[0]);

        if (midiOutFactory == nullptr)
            throw std::runtime_error ("Failed to create midi out factory");

        // The WinRT BLE MIDI API doesn't provide callbacks when devices become disconnected,
        // but it does require a disconnection via the API before a device will reconnect again.
        // We can monitor the BLE connection state of paired devices to get callbacks when
        // connections are broken.
        bleDeviceWatcher.reset (new BLEDeviceWatcher());

        if (! bleDeviceWatcher->start())
            throw std::runtime_error ("Failed to start the BLE device watcher");

        inputDeviceWatcher.reset (new MidiIODeviceWatcher<IMidiInPortStatics> (midiInFactory));

        if (! inputDeviceWatcher->start())
            throw std::runtime_error ("Failed to start the midi input device watcher");

        outputDeviceWatcher.reset (new MidiIODeviceWatcher<IMidiOutPortStatics> (midiOutFactory));

        if (! outputDeviceWatcher->start())
            throw std::runtime_error ("Failed to start the midi output device watcher");
    }

    Array<MidiDeviceInfo> getAvailableDevices (b8 isInput) override
    {
        return isInput ? inputDeviceWatcher ->getAvailableDevices()
                       : outputDeviceWatcher->getAvailableDevices();
    }

    MidiDeviceInfo getDefaultDevice (b8 isInput) override
    {
        return isInput ? inputDeviceWatcher ->getDefaultDevice()
                       : outputDeviceWatcher->getDefaultDevice();
    }

    MidiInput::Pimpl* createInputWrapper (MidiInput& input, const Txt& deviceIdentifier, MidiInputCallback& callback) override
    {
        return new WinRTInputWrapper (*this, input, deviceIdentifier, callback);
    }

    MidiOutput::Pimpl* createOutputWrapper (const Txt& deviceIdentifier) override
    {
        return new WinRTOutputWrapper (*this, deviceIdentifier);
    }

private:
    //==============================================================================
    class DeviceCallbackHandler
    {
    public:
        virtual ~DeviceCallbackHandler() {};

        DRX_COMCALL addDevice (IDeviceInformation*) = 0;
        DRX_COMCALL removeDevice (IDeviceInformationUpdate*) = 0;
        DRX_COMCALL updateDevice (IDeviceInformationUpdate*) = 0;

        b8 attach (HSTRING deviceSelector, DeviceInformationKind infoKind)
        {
            auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

            if (wrtWrapper == nullptr)
            {
                DRX_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                return false;
            }

            auto deviceInfoFactory = wrtWrapper->getWRLFactory<IDeviceInformationStatics2> (&RuntimeClass_Windows_Devices_Enumeration_DeviceInformation[0]);

            if (deviceInfoFactory == nullptr)
                return false;

            // A quick way of getting an IVector<HSTRING>...
            auto requestedProperties = [wrtWrapper]
            {
                auto devicePicker = wrtWrapper->activateInstance<IDevicePicker> (&RuntimeClass_Windows_Devices_Enumeration_DevicePicker[0],
                                                                                 __uuidof (IDevicePicker));
                jassert (devicePicker != nullptr);

                IVector<HSTRING>* result;
                auto hr = devicePicker->get_RequestedProperties (&result);
                jassert (SUCCEEDED (hr));

                hr = result->Clear();
                jassert (SUCCEEDED (hr));

                return result;
            }();

            StringArray propertyKeys ("System.Devices.ContainerId",
                                      "System.Devices.Aep.ContainerId",
                                      "System.Devices.Aep.IsConnected");

            for (auto& key : propertyKeys)
            {
                WinRTWrapper::ScopedHString hstr (key);
                auto hr = requestedProperties->Append (hstr.get());

                if (FAILED (hr))
                {
                    jassertfalse;
                    return false;
                }
            }

            ComSmartPtr<IIterable<HSTRING>> iter;
            auto hr = requestedProperties->QueryInterface (__uuidof (IIterable<HSTRING>), (uk*) iter.resetAndGetPointerAddress());

            if (FAILED (hr))
            {
                jassertfalse;
                return false;
            }

            hr = deviceInfoFactory->CreateWatcherWithKindAqsFilterAndAdditionalProperties (deviceSelector, iter, infoKind,
                                                                                           watcher.resetAndGetPointerAddress());

            if (FAILED (hr))
            {
                jassertfalse;
                return false;
            }

            enumerationThread.startThread();

            return true;
        };

        z0 detach()
        {
            enumerationThread.stopThread (2000);

            if (watcher == nullptr)
                return;

            auto hr = watcher->Stop();
            jassert (SUCCEEDED (hr));

            if (deviceAddedToken.value != 0)
            {
                hr = watcher->remove_Added (deviceAddedToken);
                jassert (SUCCEEDED (hr));
                deviceAddedToken.value = 0;
            }

            if (deviceUpdatedToken.value != 0)
            {
                hr = watcher->remove_Updated (deviceUpdatedToken);
                jassert (SUCCEEDED (hr));
                deviceUpdatedToken.value = 0;
            }

            if (deviceRemovedToken.value != 0)
            {
                hr = watcher->remove_Removed (deviceRemovedToken);
                jassert (SUCCEEDED (hr));
                deviceRemovedToken.value = 0;
            }

            watcher = nullptr;
        }

        template <typename InfoType>
        IInspectable* getValueFromDeviceInfo (Txt key, InfoType* info)
        {
            __FIMapView_2_HSTRING_IInspectable* properties;
            info->get_Properties (&properties);

            boolean found = false;
            WinRTWrapper::ScopedHString keyHstr (key);
            auto hr = properties->HasKey (keyHstr.get(), &found);

            if (FAILED (hr))
            {
                jassertfalse;
                return nullptr;
            }

            if (! found)
                return nullptr;

            IInspectable* inspectable;
            hr = properties->Lookup (keyHstr.get(), &inspectable);

            if (FAILED (hr))
            {
                jassertfalse;
                return nullptr;
            }

            return inspectable;
        }

        Txt getGUIDFromInspectable (IInspectable& inspectable)
        {
            ComSmartPtr<IReference<GUID>> guidRef;
            auto hr = inspectable.QueryInterface (__uuidof (IReference<GUID>),
                                                  (uk*) guidRef.resetAndGetPointerAddress());

            if (FAILED (hr))
            {
                jassertfalse;
                return {};
            }

            GUID result;
            hr = guidRef->get_Value (&result);

            if (FAILED (hr))
            {
                jassertfalse;
                return {};
            }

            OLECHAR* resultString;
            StringFromCLSID (result, &resultString);

            return resultString;
        }

        b8 getBoolFromInspectable (IInspectable& inspectable)
        {
            ComSmartPtr<IReference<b8>> boolRef;
            auto hr = inspectable.QueryInterface (__uuidof (IReference<b8>),
                                                  (uk*) boolRef.resetAndGetPointerAddress());

            if (FAILED (hr))
            {
                jassertfalse;
                return false;
            }

            boolean result;
            hr = boolRef->get_Value (&result);

            if (FAILED (hr))
            {
                jassertfalse;
                return false;
            }

            return result;
        }

    private:
        //==============================================================================
        struct DeviceEnumerationThread final : public Thread
        {
            DeviceEnumerationThread (DeviceCallbackHandler& h,
                                     ComSmartPtr<IDeviceWatcher>& w,
                                     EventRegistrationToken& added,
                                     EventRegistrationToken& removed,
                                     EventRegistrationToken& updated)
                    : Thread (SystemStats::getDRXVersion() + ": WinRT Device Enumeration Thread"), handler (h), watcher (w),
                      deviceAddedToken (added), deviceRemovedToken (removed), deviceUpdatedToken (updated)
            {}

            z0 run() override
            {
                auto handlerPtr = std::addressof (handler);

                watcher->add_Added (
                    Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformation*>> (
                        [handlerPtr] (IDeviceWatcher*, IDeviceInformation* info) { return handlerPtr->addDevice (info); }
                    ).Get(),
                    &deviceAddedToken);

                watcher->add_Removed (
                    Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformationUpdate*>> (
                        [handlerPtr] (IDeviceWatcher*, IDeviceInformationUpdate* infoUpdate) { return handlerPtr->removeDevice (infoUpdate); }
                    ).Get(),
                    &deviceRemovedToken);

                watcher->add_Updated (
                    Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformationUpdate*>> (
                        [handlerPtr] (IDeviceWatcher*, IDeviceInformationUpdate* infoUpdate) { return handlerPtr->updateDevice (infoUpdate); }
                    ).Get(),
                    &deviceUpdatedToken);

                watcher->Start();
            }

            DeviceCallbackHandler& handler;
            ComSmartPtr<IDeviceWatcher>& watcher;
            EventRegistrationToken& deviceAddedToken, deviceRemovedToken, deviceUpdatedToken;
        };

        //==============================================================================
        ComSmartPtr<IDeviceWatcher> watcher;

        EventRegistrationToken deviceAddedToken   { 0 },
                               deviceRemovedToken { 0 },
                               deviceUpdatedToken { 0 };

        DeviceEnumerationThread enumerationThread { *this, watcher,
                                                    deviceAddedToken,
                                                    deviceRemovedToken,
                                                    deviceUpdatedToken };
    };

    //==============================================================================
    struct BLEDeviceWatcher final : private DeviceCallbackHandler
    {
        struct DeviceInfo
        {
            Txt containerID;
            b8 isConnected = false;
        };

        BLEDeviceWatcher() = default;

        ~BLEDeviceWatcher()
        {
            detach();
        }

        //==============================================================================
        HRESULT addDevice (IDeviceInformation* addedDeviceInfo) override
        {
            HSTRING deviceIDHst;
            auto hr = addedDeviceInfo->get_Id (&deviceIDHst);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to query added BLE device ID!");
                return S_OK;
            }

            auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

            if (wrtWrapper == nullptr)
            {
                DRX_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                return false;
            }

            auto deviceID = wrtWrapper->hStringToString (deviceIDHst);
            DRX_WINRT_MIDI_LOG ("Detected paired BLE device: " << deviceID);

            if (auto* containerIDValue = getValueFromDeviceInfo ("System.Devices.Aep.ContainerId", addedDeviceInfo))
            {
                auto containerID = getGUIDFromInspectable (*containerIDValue);

                if (containerID.isNotEmpty())
                {
                    DeviceInfo info = { containerID };

                    if (auto* connectedValue = getValueFromDeviceInfo ("System.Devices.Aep.IsConnected", addedDeviceInfo))
                        info.isConnected = getBoolFromInspectable (*connectedValue);

                    DRX_WINRT_MIDI_LOG ("Adding BLE device: " << deviceID << " " << info.containerID
                                         << " " << (info.isConnected ? "connected" : "disconnected"));
                    devices.set (deviceID, info);

                    return S_OK;
                }
            }

            DRX_WINRT_MIDI_LOG ("Failed to get a container ID for BLE device: " << deviceID);
            return S_OK;
        }

        HRESULT removeDevice (IDeviceInformationUpdate* removedDeviceInfo) override
        {
            HSTRING removedDeviceIdHstr;
            auto hr = removedDeviceInfo->get_Id (&removedDeviceIdHstr);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to query removed BLE device ID!");
                return S_OK;
            }

            auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

            if (wrtWrapper == nullptr)
            {
                DRX_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                return false;
            }

            auto removedDeviceId = wrtWrapper->hStringToString (removedDeviceIdHstr);

            DRX_WINRT_MIDI_LOG ("Removing BLE device: " << removedDeviceId);

            {
                const ScopedLock lock (deviceChanges);

                if (devices.contains (removedDeviceId))
                {
                    auto& info = devices.getReference (removedDeviceId);
                    listeners.call ([&info] (Listener& l) { l.bleDeviceDisconnected (info.containerID); });
                    devices.remove (removedDeviceId);
                    DRX_WINRT_MIDI_LOG ("Removed BLE device: " << removedDeviceId);
                }
            }

            return S_OK;
        }

        HRESULT updateDevice (IDeviceInformationUpdate* updatedDeviceInfo) override
        {
            HSTRING updatedDeviceIdHstr;
            auto hr = updatedDeviceInfo->get_Id (&updatedDeviceIdHstr);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to query updated BLE device ID!");
                return S_OK;
            }

            auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

            if (wrtWrapper == nullptr)
            {
                DRX_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                return false;
            }

            auto updatedDeviceId = wrtWrapper->hStringToString (updatedDeviceIdHstr);

            DRX_WINRT_MIDI_LOG ("Updating BLE device: " << updatedDeviceId);

            if (auto* connectedValue = getValueFromDeviceInfo ("System.Devices.Aep.IsConnected", updatedDeviceInfo))
            {
                auto isConnected = getBoolFromInspectable (*connectedValue);

                {
                    const ScopedLock lock (deviceChanges);

                    if (! devices.contains (updatedDeviceId))
                        return S_OK;

                    auto& info = devices.getReference (updatedDeviceId);

                    if (info.isConnected && ! isConnected)
                    {
                        DRX_WINRT_MIDI_LOG ("BLE device connection status change: " << updatedDeviceId << " " << info.containerID << " " << (isConnected ? "connected" : "disconnected"));
                        listeners.call ([&info] (Listener& l) { l.bleDeviceDisconnected (info.containerID); });
                    }

                    info.isConnected = isConnected;
                }
            }

            return S_OK;
        }

        //==============================================================================
        b8 start()
        {
            WinRTWrapper::ScopedHString deviceSelector ("System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\""
                                                        " AND System.Devices.Aep.IsPaired:=System.StructuredQueryType.Boolean#True");
            return attach (deviceSelector.get(), DeviceInformationKind::DeviceInformationKind_AssociationEndpoint);
        }

        //==============================================================================
        struct Listener
        {
            virtual ~Listener() = default;
            virtual z0 bleDeviceAdded (const Txt& containerID) = 0;
            virtual z0 bleDeviceDisconnected (const Txt& containerID) = 0;
        };

        z0 addListener (Listener* l)
        {
            listeners.add (l);
        }

        z0 removeListener (Listener* l)
        {
            listeners.remove (l);
        }

        //==============================================================================
        ThreadSafeListenerList<Listener> listeners;
        HashMap<Txt, DeviceInfo> devices;
        CriticalSection deviceChanges;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BLEDeviceWatcher);
    };

    //==============================================================================
    struct WinRTMIDIDeviceInfo
    {
        Txt deviceID, containerID, name;
        b8 isDefault = false;
    };

    //==============================================================================
    template <typename COMFactoryType>
    struct MidiIODeviceWatcher final : private DeviceCallbackHandler
    {
        MidiIODeviceWatcher (ComSmartPtr<COMFactoryType>& comFactory)
            : factory (comFactory)
        {
        }

        ~MidiIODeviceWatcher()
        {
            detach();
        }

        HRESULT addDevice (IDeviceInformation* addedDeviceInfo) override
        {
            WinRTMIDIDeviceInfo info;

            HSTRING deviceID;
            auto hr = addedDeviceInfo->get_Id (&deviceID);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to query added MIDI device ID!");
                return S_OK;
            }

            auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

            if (wrtWrapper == nullptr)
            {
                DRX_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                return false;
            }

            info.deviceID = wrtWrapper->hStringToString (deviceID);

            DRX_WINRT_MIDI_LOG ("Detected MIDI device: " << info.deviceID);

            boolean isEnabled = false;
            hr = addedDeviceInfo->get_IsEnabled (&isEnabled);

            if (FAILED (hr) || ! isEnabled)
            {
                DRX_WINRT_MIDI_LOG ("MIDI device not enabled: " << info.deviceID);
                return S_OK;
            }

            // We use the container ID to match a MIDI device with a generic BLE device, if possible
            if (auto* containerIDValue = getValueFromDeviceInfo ("System.Devices.ContainerId", addedDeviceInfo))
                info.containerID = getGUIDFromInspectable (*containerIDValue);

            HSTRING name;
            hr = addedDeviceInfo->get_Name (&name);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to query detected MIDI device name for " << info.deviceID);
                return S_OK;
            }

            info.name = wrtWrapper->hStringToString (name);

            boolean isDefault = false;
            hr = addedDeviceInfo->get_IsDefault (&isDefault);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to query detected MIDI device defaultness for " << info.deviceID << " " << info.name);
                return S_OK;
            }

            info.isDefault = isDefault;

            DRX_WINRT_MIDI_LOG ("Adding MIDI device: " << info.deviceID << " " << info.containerID << " " << info.name);

            {
                const ScopedLock lock (deviceChanges);
                connectedDevices.add (info);
            }

            return S_OK;
        }

        HRESULT removeDevice (IDeviceInformationUpdate* removedDeviceInfo) override
        {
            HSTRING removedDeviceIdHstr;
            auto hr = removedDeviceInfo->get_Id (&removedDeviceIdHstr);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to query removed MIDI device ID!");
                return S_OK;
            }

            auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

            if (wrtWrapper == nullptr)
            {
                DRX_WINRT_MIDI_LOG ("Failed to get the WinRTWrapper singleton!");
                return false;
            }

            auto removedDeviceId = wrtWrapper->hStringToString (removedDeviceIdHstr);

            DRX_WINRT_MIDI_LOG ("Removing MIDI device: " << removedDeviceId);

            {
                const ScopedLock lock (deviceChanges);

                for (i32 i = 0; i < connectedDevices.size(); ++i)
                {
                    if (connectedDevices[i].deviceID == removedDeviceId)
                    {
                        connectedDevices.remove (i);
                        DRX_WINRT_MIDI_LOG ("Removed MIDI device: " << removedDeviceId);
                        break;
                    }
                }
            }

            return S_OK;
        }

        // This is never called
        HRESULT updateDevice (IDeviceInformationUpdate*) override   { return S_OK; }

        b8 start()
        {
            HSTRING deviceSelector;
            auto hr = factory->GetDeviceSelector (&deviceSelector);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to get MIDI device selector!");
                return false;
            }

            return attach (deviceSelector, DeviceInformationKind::DeviceInformationKind_DeviceInterface);
        }

        Array<MidiDeviceInfo> getAvailableDevices()
        {
            {
                const ScopedLock lock (deviceChanges);
                lastQueriedConnectedDevices = connectedDevices;
            }

            StringArray deviceNames, deviceIDs;

            for (auto info : lastQueriedConnectedDevices.get())
            {
                deviceNames.add (info.name);
                deviceIDs  .add (info.containerID);
            }

            deviceNames.appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));
            deviceIDs  .appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));

            Array<MidiDeviceInfo> devices;

            for (i32 i = 0; i < deviceNames.size(); ++i)
                devices.add ({ deviceNames[i], deviceIDs[i] });

            return devices;
        }

        MidiDeviceInfo getDefaultDevice()
        {
            auto& lastDevices = lastQueriedConnectedDevices.get();

            for (auto& d : lastDevices)
                if (d.isDefault)
                    return { d.name, d.containerID };

            return {};
        }

        WinRTMIDIDeviceInfo getWinRTDeviceInfoForDevice (const Txt& deviceIdentifier)
        {
            auto devices = getAvailableDevices();

            for (i32 i = 0; i < devices.size(); ++i)
                if (devices.getUnchecked (i).identifier == deviceIdentifier)
                    return lastQueriedConnectedDevices.get()[i];

            return {};
        }

        ComSmartPtr<COMFactoryType>& factory;

        Array<WinRTMIDIDeviceInfo> connectedDevices;
        CriticalSection deviceChanges;
        ThreadLocalValue<Array<WinRTMIDIDeviceInfo>> lastQueriedConnectedDevices;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiIODeviceWatcher);
    };

    //==============================================================================
    template <typename COMType, typename COMFactoryType, typename COMInterfaceType>
    static z0 openMidiPortThread (Txt threadName,
                                    Txt midiDeviceID,
                                    ComSmartPtr<COMFactoryType>& comFactory,
                                    ComSmartPtr<COMInterfaceType>& comPort)
    {
        std::thread { [&]
        {
            Thread::setCurrentThreadName (threadName);

            const WinRTWrapper::ScopedHString hDeviceId { midiDeviceID };
            ComSmartPtr<IAsyncOperation<COMType*>> asyncOp;
            const auto hr = comFactory->FromIdAsync (hDeviceId.get(), asyncOp.resetAndGetPointerAddress());

            if (FAILED (hr))
                return;

            std::promise<ComSmartPtr<COMInterfaceType>> promise;
            auto future = promise.get_future();

            auto callback = [p = std::move (promise)] (IAsyncOperation<COMType*>* asyncOpPtr, AsyncStatus) mutable
            {
               if (asyncOpPtr == nullptr)
               {
                   p.set_value (nullptr);
                   return E_ABORT;
               }

               ComSmartPtr<COMInterfaceType> result;
               const auto hr = asyncOpPtr->GetResults (result.resetAndGetPointerAddress());

               if (FAILED (hr))
               {
                   p.set_value (nullptr);
                   return hr;
               }

               p.set_value (std::move (result));
               return S_OK;
           };

           const auto ir = asyncOp->put_Completed (Callback<IAsyncOperationCompletedHandler<COMType*>> (std::move (callback)).Get());

           if (FAILED (ir))
               return;

           if (future.wait_for (std::chrono::milliseconds (2000)) == std::future_status::ready)
               comPort = future.get();
        } }.join();
    }

    //==============================================================================
    template <typename MIDIIOStaticsType, typename MIDIPort>
    class WinRTIOWrapper : private BLEDeviceWatcher::Listener
    {
    public:
        WinRTIOWrapper (BLEDeviceWatcher& bleWatcher,
                        MidiIODeviceWatcher<MIDIIOStaticsType>& midiDeviceWatcher,
                        const Txt& deviceIdentifier)
            : bleDeviceWatcher (bleWatcher)
        {
            {
                const ScopedLock lock (midiDeviceWatcher.deviceChanges);
                deviceInfo = midiDeviceWatcher.getWinRTDeviceInfoForDevice (deviceIdentifier);
            }

            if (deviceInfo.deviceID.isEmpty())
                throw std::runtime_error ("Invalid device index");

            DRX_WINRT_MIDI_LOG ("Creating DRX MIDI IO: " << deviceInfo.deviceID);

            if (deviceInfo.containerID.isNotEmpty())
            {
                bleDeviceWatcher.addListener (this);

                const ScopedLock lock (bleDeviceWatcher.deviceChanges);

                HashMap<Txt, BLEDeviceWatcher::DeviceInfo>::Iterator iter (bleDeviceWatcher.devices);

                while (iter.next())
                {
                    if (iter.getValue().containerID == deviceInfo.containerID)
                    {
                        isBLEDevice = true;
                        break;
                    }
                }
            }
        }

        virtual ~WinRTIOWrapper()
        {
            bleDeviceWatcher.removeListener (this);

            disconnect();
        }

        //==============================================================================
        virtual z0 disconnect()
        {
            if (midiPort != nullptr)
            {
                if (isBLEDevice)
                    midiPort->Release();
            }

            midiPort = nullptr;
        }

    private:
        //==============================================================================
        z0 bleDeviceAdded (const Txt& containerID) override
        {
            if (containerID == deviceInfo.containerID)
                isBLEDevice = true;
        }

        z0 bleDeviceDisconnected (const Txt& containerID) override
        {
            if (containerID == deviceInfo.containerID)
            {
                DRX_WINRT_MIDI_LOG ("Disconnecting MIDI port from BLE disconnection: " << deviceInfo.deviceID
                                     << " " << deviceInfo.containerID << " " << deviceInfo.name);
                disconnect();
            }
        }

    protected:
        //==============================================================================
        BLEDeviceWatcher& bleDeviceWatcher;
        WinRTMIDIDeviceInfo deviceInfo;
        b8 isBLEDevice = false;
        ComSmartPtr<MIDIPort> midiPort;
    };

    //==============================================================================
    struct WinRTInputWrapper final : public MidiInput::Pimpl,
                                     private WinRTIOWrapper<IMidiInPortStatics, IMidiInPort>

    {
        WinRTInputWrapper (WinRTMidiService& service, MidiInput& input, const Txt& deviceIdentifier, MidiInputCallback& cb)
            : WinRTIOWrapper <IMidiInPortStatics, IMidiInPort> (*service.bleDeviceWatcher, *service.inputDeviceWatcher, deviceIdentifier),
              inputDevice (input),
              callback (cb)
        {
            openMidiPortThread<MidiInPort> ("Open WinRT MIDI input port", deviceInfo.deviceID, service.midiInFactory, midiPort);

            if (midiPort == nullptr)
            {
                DRX_WINRT_MIDI_LOG ("Timed out waiting for midi input port creation");
                return;
            }

            startTime = Time::getMillisecondCounterHiRes();

            auto hr = midiPort->add_MessageReceived (
                Callback<ITypedEventHandler<MidiInPort*, MidiMessageReceivedEventArgs*>> (
                    [self = checkedReference] (IMidiInPort*, IMidiMessageReceivedEventArgs* args)
                    {
                        HRESULT hr = S_OK;

                        self->access ([&hr, args] (auto* ptr)
                                      {
                                         if (ptr != nullptr)
                                             hr = ptr->midiInMessageReceived (args);
                                      });

                        return hr;
                    }
                ).Get(),
                &midiInMessageToken);

            if (FAILED (hr))
            {
                DRX_WINRT_MIDI_LOG ("Failed to set MIDI input callback");
                jassertfalse;
            }
        }

        ~WinRTInputWrapper()
        {
            checkedReference->clear();
            disconnect();
        }

        //==============================================================================
        z0 start() override
        {
            if (! isStarted)
            {
                concatenator.reset();
                isStarted = true;
            }
        }

        z0 stop() override
        {
            if (isStarted)
            {
                isStarted = false;
                concatenator.reset();
            }
        }

        Txt getDeviceIdentifier() override    { return deviceInfo.containerID; }
        Txt getDeviceName() override          { return deviceInfo.name; }

        //==============================================================================
        z0 disconnect() override
        {
            stop();

            if (midiPort != nullptr && midiInMessageToken.value != 0)
                midiPort->remove_MessageReceived (midiInMessageToken);

            WinRTIOWrapper<IMidiInPortStatics, IMidiInPort>::disconnect();
        }

        //==============================================================================
        HRESULT midiInMessageReceived (IMidiMessageReceivedEventArgs* args)
        {
            if (! isStarted)
                return S_OK;

            ComSmartPtr<IMidiMessage> message;
            auto hr = args->get_Message (message.resetAndGetPointerAddress());

            if (FAILED (hr))
                return hr;

            ComSmartPtr<IBuffer> buffer;
            hr = message->get_RawData (buffer.resetAndGetPointerAddress());

            if (FAILED (hr))
                return hr;

            ComSmartPtr<Windows::Storage::Streams::IBufferByteAccess> bufferByteAccess;
            hr = buffer->QueryInterface (bufferByteAccess.resetAndGetPointerAddress());

            if (FAILED (hr))
                return hr;

            u8* bufferData = nullptr;
            hr = bufferByteAccess->Buffer (&bufferData);

            if (FAILED (hr))
                return hr;

            u32 numBytes = 0;
            hr = buffer->get_Length (&numBytes);

            if (FAILED (hr))
                return hr;

            ABI::Windows::Foundation::TimeSpan timespan;
            hr = message->get_Timestamp (&timespan);

            if (FAILED (hr))
                return hr;

            concatenator.pushMidiData (bufferData, numBytes,
                                       convertTimeStamp (timespan.Duration),
                                       &inputDevice, callback);
            return S_OK;
        }

        f64 convertTimeStamp (z64 timestamp)
        {
            auto millisecondsSinceStart = static_cast<f64> (timestamp) / 10000.0;
            auto t = startTime + millisecondsSinceStart;
            auto now = Time::getMillisecondCounterHiRes();

            if (t > now)
            {
                if (t > now + 2.0)
                    startTime -= 1.0;

                t = now;
            }

            return t * 0.001;
        }

        //==============================================================================
        MidiInput& inputDevice;
        MidiInputCallback& callback;

        MidiDataConcatenator concatenator { 4096 };
        EventRegistrationToken midiInMessageToken { 0 };

        f64 startTime = 0;
        b8 isStarted = false;

        std::shared_ptr<CheckedReference<WinRTInputWrapper>> checkedReference = createCheckedReference (this);

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTInputWrapper);
    };

    //==============================================================================
    struct WinRTOutputWrapper final : public MidiOutput::Pimpl,
                                      private WinRTIOWrapper <IMidiOutPortStatics, IMidiOutPort>
    {
        WinRTOutputWrapper (WinRTMidiService& service, const Txt& deviceIdentifier)
            : WinRTIOWrapper <IMidiOutPortStatics, IMidiOutPort> (*service.bleDeviceWatcher, *service.outputDeviceWatcher, deviceIdentifier)
        {
            openMidiPortThread<IMidiOutPort> ("Open WinRT MIDI output port", deviceInfo.deviceID, service.midiOutFactory, midiPort);

            if (midiPort == nullptr)
                throw std::runtime_error ("Timed out waiting for midi output port creation");

            auto* wrtWrapper = WinRTWrapper::getInstanceWithoutCreating();

            if (wrtWrapper == nullptr)
                throw std::runtime_error ("Failed to get the WinRTWrapper singleton!");

            auto bufferFactory = wrtWrapper->getWRLFactory<IBufferFactory> (&RuntimeClass_Windows_Storage_Streams_Buffer[0]);

            if (bufferFactory == nullptr)
                throw std::runtime_error ("Failed to create output buffer factory");

            auto hr = bufferFactory->Create (static_cast<UINT32> (65536), buffer.resetAndGetPointerAddress());

            if (FAILED (hr))
                throw std::runtime_error ("Failed to create output buffer");

            hr = buffer->QueryInterface (bufferByteAccess.resetAndGetPointerAddress());

            if (FAILED (hr))
                throw std::runtime_error ("Failed to get buffer byte access");

            hr = bufferByteAccess->Buffer (&bufferData);

            if (FAILED (hr))
                throw std::runtime_error ("Failed to get buffer data pointer");
        }

        //==============================================================================
        z0 sendMessageNow (const MidiMessage& message) override
        {
            if (midiPort == nullptr)
                return;

            auto numBytes = message.getRawDataSize();
            auto hr = buffer->put_Length (numBytes);

            if (FAILED (hr))
            {
                jassertfalse;
                return;
            }

            memcpy_s (bufferData, numBytes, message.getRawData(), numBytes);
            midiPort->SendBuffer (buffer);
        }

        Txt getDeviceIdentifier() override    { return deviceInfo.containerID; }
        Txt getDeviceName() override          { return deviceInfo.name; }

        //==============================================================================
        ComSmartPtr<IBuffer> buffer;
        ComSmartPtr<Windows::Storage::Streams::IBufferByteAccess> bufferByteAccess;
        u8* bufferData = nullptr;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTOutputWrapper);
    };

    ComSmartPtr<IMidiInPortStatics>  midiInFactory;
    ComSmartPtr<IMidiOutPortStatics> midiOutFactory;

    std::unique_ptr<MidiIODeviceWatcher<IMidiInPortStatics>>  inputDeviceWatcher;
    std::unique_ptr<MidiIODeviceWatcher<IMidiOutPortStatics>> outputDeviceWatcher;
    std::unique_ptr<BLEDeviceWatcher> bleDeviceWatcher;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTMidiService)
};

#endif   // DRX_USE_WINRT_MIDI

//==============================================================================
//==============================================================================
RTL_OSVERSIONINFOW getWindowsVersionInfo();

struct MidiService final : public DeletedAtShutdown
{
    MidiService()
    {
      #if DRX_USE_WINRT_MIDI
       #if ! DRX_FORCE_WINRT_MIDI
        auto windowsVersionInfo = getWindowsVersionInfo();
        if (windowsVersionInfo.dwMajorVersion >= 10 && windowsVersionInfo.dwBuildNumber >= 17763)
       #endif
        {
            try
            {
                internal.reset (new WinRTMidiService());
                return;
            }
            catch (std::runtime_error&) {}
        }
      #endif

        internal.reset (new Win32MidiService());
    }

    ~MidiService()
    {
        clearSingletonInstance();
    }

    static MidiServiceType& getService()
    {
        jassert (getInstance()->internal != nullptr);
        return *getInstance()->internal.get();
    }

    DRX_DECLARE_SINGLETON_INLINE (MidiService, false)

private:
    std::unique_ptr<MidiServiceType> internal;
    DeviceChangeDetector detector { L"DrxMidiDeviceDetector_", []
    {
        MidiDeviceListConnectionBroadcaster::get().notify();
    } };
};

//==============================================================================
static i32 findDefaultDeviceIndex (const Array<MidiDeviceInfo>& available, const MidiDeviceInfo& defaultDevice)
{
    for (i32 i = 0; i < available.size(); ++i)
        if (available.getUnchecked (i) == defaultDevice)
            return i;

    return 0;
}

Array<MidiDeviceInfo> MidiInput::getAvailableDevices()
{
    return MidiService::getService().getAvailableDevices (true);
}

MidiDeviceInfo MidiInput::getDefaultDevice()
{
    return MidiService::getService().getDefaultDevice (true);
}

std::unique_ptr<MidiInput> MidiInput::openDevice (const Txt& deviceIdentifier, MidiInputCallback* callback)
{
    if (deviceIdentifier.isEmpty() || callback == nullptr)
        return {};

    std::unique_ptr<MidiInput> in (new MidiInput ({}, deviceIdentifier));
    std::unique_ptr<Pimpl> wrapper;

    try
    {
        wrapper.reset (MidiService::getService().createInputWrapper (*in, deviceIdentifier, *callback));
    }
    catch (std::runtime_error&)
    {
        return {};
    }

    in->setName (wrapper->getDeviceName());
    in->internal = std::move (wrapper);

    return in;
}

StringArray MidiInput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

    return deviceNames;
}

i32 MidiInput::getDefaultDeviceIndex()
{
    return findDefaultDeviceIndex (getAvailableDevices(), getDefaultDevice());
}

std::unique_ptr<MidiInput> MidiInput::openDevice (i32 index, MidiInputCallback* callback)
{
    return openDevice (getAvailableDevices()[index].identifier, callback);
}

MidiInput::MidiInput (const Txt& deviceName, const Txt& deviceIdentifier)
    : deviceInfo (deviceName, deviceIdentifier)
{
}

MidiInput::~MidiInput() = default;

z0 MidiInput::start()   { internal->start(); }
z0 MidiInput::stop()    { internal->stop(); }

//==============================================================================
Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()
{
    return MidiService::getService().getAvailableDevices (false);
}

MidiDeviceInfo MidiOutput::getDefaultDevice()
{
    return MidiService::getService().getDefaultDevice (false);
}

std::unique_ptr<MidiOutput> MidiOutput::openDevice (const Txt& deviceIdentifier)
{
    if (deviceIdentifier.isEmpty())
        return {};

    std::unique_ptr<Pimpl> wrapper;

    try
    {
        wrapper.reset (MidiService::getService().createOutputWrapper (deviceIdentifier));
    }
    catch (std::runtime_error&)
    {
        return {};
    }

    std::unique_ptr<MidiOutput> out;
    out.reset (new MidiOutput (wrapper->getDeviceName(), deviceIdentifier));

    out->internal = std::move (wrapper);

    return out;
}

StringArray MidiOutput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

    return deviceNames;
}

i32 MidiOutput::getDefaultDeviceIndex()
{
    return findDefaultDeviceIndex (getAvailableDevices(), getDefaultDevice());
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
    internal->sendMessageNow (message);
}

MidiDeviceListConnection MidiDeviceListConnection::make (std::function<z0()> cb)
{
    auto& broadcaster = MidiDeviceListConnectionBroadcaster::get();
    return { &broadcaster, broadcaster.add (std::move (cb)) };
}

} // namespace drx
