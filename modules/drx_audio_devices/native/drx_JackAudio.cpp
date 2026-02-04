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

static uk drx_libjackHandle = nullptr;

static uk drx_loadJackFunction (tukk const name)
{
    if (drx_libjackHandle == nullptr)
        return nullptr;

   #if DRX_WINDOWS
    return GetProcAddress ((HMODULE) drx_libjackHandle, name);
   #else
    return dlsym (drx_libjackHandle, name);
   #endif
}

#define DRX_DECL_JACK_FUNCTION(return_type, fn_name, argument_types, arguments)  \
  static inline return_type fn_name argument_types                                \
  {                                                                               \
      using ReturnType = return_type;                                             \
      typedef return_type (*fn_type) argument_types;                              \
      static fn_type fn = (fn_type) drx_loadJackFunction (#fn_name);             \
      jassert (fn != nullptr);                                                    \
      return (fn != nullptr) ? ((*fn) arguments) : ReturnType();                  \
  }

#define DRX_DECL_VOID_JACK_FUNCTION(fn_name, argument_types, arguments)          \
  static inline z0 fn_name argument_types                                       \
  {                                                                               \
      typedef z0 (*fn_type) argument_types;                                     \
      static fn_type fn = (fn_type) drx_loadJackFunction (#fn_name);             \
      jassert (fn != nullptr);                                                    \
      if (fn != nullptr) (*fn) arguments;                                         \
  }

//==============================================================================
DRX_DECL_JACK_FUNCTION (jack_client_t*, jack_client_open, (tukk client_name, jack_options_t options, jack_status_t* status, ...), (client_name, options, status))
DRX_DECL_JACK_FUNCTION (i32, jack_client_close, (jack_client_t *client), (client))
DRX_DECL_JACK_FUNCTION (i32, jack_activate, (jack_client_t* client), (client))
DRX_DECL_JACK_FUNCTION (i32, jack_deactivate, (jack_client_t* client), (client))
DRX_DECL_JACK_FUNCTION (jack_nframes_t, jack_get_buffer_size, (jack_client_t* client), (client))
DRX_DECL_JACK_FUNCTION (jack_nframes_t, jack_get_sample_rate, (jack_client_t* client), (client))
DRX_DECL_VOID_JACK_FUNCTION (jack_on_shutdown, (jack_client_t* client, z0 (*function) (uk arg), uk arg), (client, function, arg))
DRX_DECL_VOID_JACK_FUNCTION (jack_on_info_shutdown, (jack_client_t* client, JackInfoShutdownCallback function, uk arg), (client, function, arg))
DRX_DECL_JACK_FUNCTION (uk , jack_port_get_buffer, (jack_port_t* port, jack_nframes_t nframes), (port, nframes))
DRX_DECL_JACK_FUNCTION (jack_nframes_t, jack_port_get_total_latency, (jack_client_t* client, jack_port_t* port), (client, port))
DRX_DECL_JACK_FUNCTION (jack_port_t* , jack_port_register, (jack_client_t* client, tukk port_name, tukk port_type, u64 flags, u64 buffer_size), (client, port_name, port_type, flags, buffer_size))
DRX_DECL_VOID_JACK_FUNCTION (jack_set_error_function, (z0 (*func) (tukk)), (func))
DRX_DECL_JACK_FUNCTION (i32, jack_set_process_callback, (jack_client_t* client, JackProcessCallback process_callback, uk arg), (client, process_callback, arg))
DRX_DECL_JACK_FUNCTION (tukk*, jack_get_ports, (jack_client_t* client, tukk port_name_pattern, tukk type_name_pattern, u64 flags), (client, port_name_pattern, type_name_pattern, flags))
DRX_DECL_JACK_FUNCTION (i32, jack_connect, (jack_client_t* client, tukk source_port, tukk destination_port), (client, source_port, destination_port))
DRX_DECL_JACK_FUNCTION (tukk, jack_port_name, (const jack_port_t* port), (port))
DRX_DECL_JACK_FUNCTION (uk, jack_set_port_connect_callback, (jack_client_t* client, JackPortConnectCallback connect_callback, uk arg), (client, connect_callback, arg))
DRX_DECL_JACK_FUNCTION (i32, jack_port_connected, (const jack_port_t* port), (port))
DRX_DECL_JACK_FUNCTION (i32, jack_set_xrun_callback, (jack_client_t* client, JackXRunCallback xrun_callback, uk arg), (client, xrun_callback, arg))
DRX_DECL_JACK_FUNCTION (i32, jack_port_flags, (const jack_port_t* port), (port))
DRX_DECL_JACK_FUNCTION (jack_port_t*, jack_port_by_name, (jack_client_t* client, tukk name), (client, name))
DRX_DECL_VOID_JACK_FUNCTION (jack_free, (uk ptr), (ptr))

#if DRX_DEBUG
 #define JACK_LOGGING_ENABLED 1
#endif

#if JACK_LOGGING_ENABLED
namespace
{
    z0 jack_Log (const Txt& s)
    {
        std::cerr << s << std::endl;
    }

    tukk getJackErrorMessage (const jack_status_t status)
    {
        if (status & JackServerFailed
             || status & JackServerError)   return "Unable to connect to JACK server";
        if (status & JackVersionError)      return "Client's protocol version does not match";
        if (status & JackInvalidOption)     return "The operation contained an invalid or unsupported option";
        if (status & JackNameNotUnique)     return "The desired client name was not unique";
        if (status & JackNoSuchClient)      return "Requested client does not exist";
        if (status & JackInitFailure)       return "Unable to initialize client";
        return nullptr;
    }
}
 #define DRX_JACK_LOG_STATUS(x)    { if (tukk m = getJackErrorMessage (x)) jack_Log (m); }
 #define DRX_JACK_LOG(x)           jack_Log(x)
#else
 #define DRX_JACK_LOG_STATUS(x)    {}
 #define DRX_JACK_LOG(x)           {}
#endif


//==============================================================================
#ifndef DRX_JACK_CLIENT_NAME
 #ifdef DrxPlugin_Name
  #define DRX_JACK_CLIENT_NAME DrxPlugin_Name
 #else
  #define DRX_JACK_CLIENT_NAME "DRXJack"
 #endif
#endif

struct JackPortIterator
{
    JackPortIterator (jack_client_t* const client, const b8 forInput)
    {
        if (client != nullptr)
            ports.reset (drx::jack_get_ports (client, nullptr, nullptr,
                                               forInput ? JackPortIsInput : JackPortIsOutput));
    }

    b8 next()
    {
        if (ports == nullptr || ports.get()[index + 1] == nullptr)
            return false;

        name = CharPointer_UTF8 (ports.get()[++index]);
        return true;
    }

    Txt getClientName() const
    {
        return name.upToFirstOccurrenceOf (":", false, false);
    }

    Txt getChannelName() const
    {
        return name.fromFirstOccurrenceOf (":", false, false);
    }

    struct Free
    {
        z0 operator() (tukk* ptr) const noexcept { drx::jack_free (ptr); }
    };

    std::unique_ptr<tukk, Free> ports;
    i32 index = -1;
    Txt name;
};

//==============================================================================
class JackAudioIODevice final : public AudioIODevice
{
public:
    JackAudioIODevice (const Txt& inName,
                       const Txt& outName,
                       std::function<z0()> notifyIn)
        : AudioIODevice (outName.isEmpty() ? inName : outName, "JACK"),
          inputName (inName),
          outputName (outName),
          notifyChannelsChanged (std::move (notifyIn))
    {
        jassert (outName.isNotEmpty() || inName.isNotEmpty());

        jack_status_t status = {};
        client = drx::jack_client_open (DRX_JACK_CLIENT_NAME, JackNoStartServer, &status);

        if (client == nullptr)
        {
            DRX_JACK_LOG_STATUS (status);
        }
        else
        {
            drx::jack_set_error_function (errorCallback);

            // open input ports
            const StringArray inputChannels (getInputChannelNames());
            for (i32 i = 0; i < inputChannels.size(); ++i)
            {
                Txt inputChannelName;
                inputChannelName << "in_" << ++totalNumberOfInputChannels;

                inputPorts.add (drx::jack_port_register (client, inputChannelName.toUTF8(),
                                                          JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
            }

            // open output ports
            const StringArray outputChannels (getOutputChannelNames());
            for (i32 i = 0; i < outputChannels.size(); ++i)
            {
                Txt outputChannelName;
                outputChannelName << "out_" << ++totalNumberOfOutputChannels;

                outputPorts.add (drx::jack_port_register (client, outputChannelName.toUTF8(),
                                                           JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
            }

            inChans.calloc (totalNumberOfInputChannels + 2);
            outChans.calloc (totalNumberOfOutputChannels + 2);
        }
    }

    ~JackAudioIODevice() override
    {
        close();
        if (client != nullptr)
        {
            drx::jack_client_close (client);
            client = nullptr;
        }
    }

    StringArray getChannelNames (const Txt& clientName, b8 forInput) const
    {
        StringArray names;

        for (JackPortIterator i (client, forInput); i.next();)
            if (i.getClientName() == clientName)
                names.add (i.getChannelName());

        return names;
    }

    StringArray getOutputChannelNames() override         { return getChannelNames (outputName, true); }
    StringArray getInputChannelNames() override          { return getChannelNames (inputName, false); }

    Array<f64> getAvailableSampleRates() override
    {
        Array<f64> rates;

        if (client != nullptr)
            rates.add (drx::jack_get_sample_rate (client));

        return rates;
    }

    Array<i32> getAvailableBufferSizes() override
    {
        Array<i32> sizes;

        if (client != nullptr)
            sizes.add (static_cast<i32> (drx::jack_get_buffer_size (client)));

        return sizes;
    }

    i32 getDefaultBufferSize() override             { return getCurrentBufferSizeSamples(); }
    i32 getCurrentBufferSizeSamples() override      { return client != nullptr ? static_cast<i32> (drx::jack_get_buffer_size (client)) : 0; }
    f64 getCurrentSampleRate() override          { return client != nullptr ? static_cast<i32> (drx::jack_get_sample_rate (client)) : 0; }

    template <typename Fn>
    z0 forEachClientChannel (const Txt& clientName, b8 isInput, Fn&& fn)
    {
        auto index = 0;

        for (JackPortIterator i (client, isInput); i.next();)
        {
            if (i.getClientName() != clientName)
                continue;

            fn (i.ports.get()[i.index], index);
            index += 1;
        }
    }

    Txt open (const BigInteger& inputChannels, const BigInteger& outputChannels,
                 f64 /* sampleRate */, i32 /* bufferSizeSamples */) override
    {
        if (client == nullptr)
        {
            lastError = "No JACK client running";
            return lastError;
        }

        lastError.clear();
        close();

        xruns.store (0, std::memory_order_relaxed);
        drx::jack_set_process_callback (client, processCallback, this);
        drx::jack_set_port_connect_callback (client, portConnectCallback, this);
        drx::jack_on_shutdown (client, shutdownCallback, this);
        drx::jack_on_info_shutdown (client, infoShutdownCallback, this);
        drx::jack_set_xrun_callback (client, xrunCallback, this);
        drx::jack_activate (client);
        deviceIsOpen = true;

        if (! inputChannels.isZero())
        {
            forEachClientChannel (inputName, false, [&] (tukk portName, i32 index)
            {
                if (! inputChannels[index])
                    return;

                jassert (index < inputPorts.size());

                const auto* source = portName;
                const auto* inputPort = inputPorts[index];

                jassert (drx::jack_port_flags (drx::jack_port_by_name (client, source)) & JackPortIsOutput);
                jassert (drx::jack_port_flags (inputPort) & JackPortIsInput);

                auto error = drx::jack_connect (client, source, drx::jack_port_name (inputPort));
                if (error != 0)
                    DRX_JACK_LOG ("Cannot connect input port " + Txt (index) + " (" + portName + "), error " + Txt (error));
            });
        }

        if (! outputChannels.isZero())
        {
            forEachClientChannel (outputName, true, [&] (tukk portName, i32 index)
            {
                if (! outputChannels[index])
                    return;

                jassert (index < outputPorts.size());

                const auto* outputPort = outputPorts[index];
                const auto* destination = portName;

                jassert (drx::jack_port_flags (outputPort) & JackPortIsOutput);
                jassert (drx::jack_port_flags (drx::jack_port_by_name (client, destination)) & JackPortIsInput);

                auto error = drx::jack_connect (client, drx::jack_port_name (outputPort), destination);
                if (error != 0)
                    DRX_JACK_LOG ("Cannot connect output port " + Txt (index) + " (" + portName + "), error " + Txt (error));
            });
        }

        updateActivePorts();

        return lastError;
    }

    z0 close() override
    {
        stop();

        if (client != nullptr)
        {
            [[maybe_unused]] const auto result = drx::jack_deactivate (client);
            jassert (result == 0);

            drx::jack_set_xrun_callback (client, xrunCallback, nullptr);
            drx::jack_set_process_callback (client, processCallback, nullptr);
            drx::jack_set_port_connect_callback (client, portConnectCallback, nullptr);
            drx::jack_on_shutdown (client, shutdownCallback, nullptr);
            drx::jack_on_info_shutdown (client, infoShutdownCallback, nullptr);
        }

        deviceIsOpen = false;
    }

    z0 start (AudioIODeviceCallback* newCallback) override
    {
        if (deviceIsOpen && newCallback != callback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            AudioIODeviceCallback* const oldCallback = callback;

            {
                const ScopedLock sl (callbackLock);
                callback = newCallback;
            }

            if (oldCallback != nullptr)
                oldCallback->audioDeviceStopped();
        }
    }

    z0 stop() override
    {
        start (nullptr);
    }

    b8 isOpen() override                           { return deviceIsOpen; }
    b8 isPlaying() override                        { return callback != nullptr; }
    i32 getCurrentBitDepth() override                { return 32; }
    Txt getLastError() override                   { return lastError; }
    i32 getXRunCount() const noexcept override       { return xruns.load (std::memory_order_relaxed); }

    BigInteger getActiveOutputChannels() const override  { return activeOutputChannels; }
    BigInteger getActiveInputChannels()  const override  { return activeInputChannels;  }

    i32 getOutputLatencyInSamples() override
    {
        i32 latency = 0;

        for (i32 i = 0; i < outputPorts.size(); i++)
            latency = jmax (latency, (i32) drx::jack_port_get_total_latency (client, outputPorts[i]));

        return latency;
    }

    i32 getInputLatencyInSamples() override
    {
        i32 latency = 0;

        for (i32 i = 0; i < inputPorts.size(); i++)
            latency = jmax (latency, (i32) drx::jack_port_get_total_latency (client, inputPorts[i]));

        return latency;
    }

    Txt inputName, outputName;

private:
    //==============================================================================
    class MainThreadDispatcher final : private AsyncUpdater
    {
    public:
        explicit MainThreadDispatcher (JackAudioIODevice& device)  : ref (device) {}
        ~MainThreadDispatcher() override { cancelPendingUpdate(); }

        z0 updateActivePorts()
        {
            if (MessageManager::getInstance()->isThisTheMessageThread())
                handleAsyncUpdate();
            else
                triggerAsyncUpdate();
        }

    private:
        z0 handleAsyncUpdate() override { ref.updateActivePorts(); }

        JackAudioIODevice& ref;
    };

    //==============================================================================
    z0 process (i32k numSamples)
    {
        i32 numActiveInChans = 0, numActiveOutChans = 0;

        for (i32 i = 0; i < totalNumberOfInputChannels; ++i)
        {
            if (activeInputChannels[i])
                if (auto* in = (jack_default_audio_sample_t*) drx::jack_port_get_buffer (inputPorts.getUnchecked (i),
                                                                                          static_cast<jack_nframes_t> (numSamples)))
                    inChans[numActiveInChans++] = (f32*) in;
        }

        for (i32 i = 0; i < totalNumberOfOutputChannels; ++i)
        {
            if (activeOutputChannels[i])
                if (auto* out = (jack_default_audio_sample_t*) drx::jack_port_get_buffer (outputPorts.getUnchecked (i),
                                                                                           static_cast<jack_nframes_t> (numSamples)))
                    outChans[numActiveOutChans++] = (f32*) out;
        }

        const ScopedLock sl (callbackLock);

        if (callback != nullptr)
        {
            if ((numActiveInChans + numActiveOutChans) > 0)
                callback->audioDeviceIOCallbackWithContext (inChans.getData(),
                                                            numActiveInChans,
                                                            outChans,
                                                            numActiveOutChans,
                                                            numSamples,
                                                            {});
        }
        else
        {
            for (i32 i = 0; i < numActiveOutChans; ++i)
                zeromem (outChans[i], static_cast<size_t> (numSamples) * sizeof (f32));
        }
    }

    static i32 processCallback (jack_nframes_t nframes, uk callbackArgument)
    {
        if (callbackArgument != nullptr)
            ((JackAudioIODevice*) callbackArgument)->process (static_cast<i32> (nframes));

        return 0;
    }

    static i32 xrunCallback (uk callbackArgument)
    {
        if (callbackArgument != nullptr)
            ((JackAudioIODevice*) callbackArgument)->xruns++;

        return 0;
    }

    z0 updateActivePorts()
    {
        BigInteger newOutputChannels, newInputChannels;

        for (i32 i = 0; i < outputPorts.size(); ++i)
            if (drx::jack_port_connected (outputPorts.getUnchecked (i)))
                newOutputChannels.setBit (i);

        for (i32 i = 0; i < inputPorts.size(); ++i)
            if (drx::jack_port_connected (inputPorts.getUnchecked (i)))
                newInputChannels.setBit (i);

        if (newOutputChannels != activeOutputChannels
             || newInputChannels != activeInputChannels)
        {
            AudioIODeviceCallback* const oldCallback = callback;

            stop();

            activeOutputChannels = newOutputChannels;
            activeInputChannels  = newInputChannels;

            if (oldCallback != nullptr)
                start (oldCallback);

            NullCheckedInvocation::invoke (notifyChannelsChanged);
        }
    }

    static z0 portConnectCallback (jack_port_id_t, jack_port_id_t, i32, uk arg)
    {
        if (JackAudioIODevice* device = static_cast<JackAudioIODevice*> (arg))
            device->mainThreadDispatcher.updateActivePorts();
    }

    static z0 threadInitCallback (uk /* callbackArgument */)
    {
        DRX_JACK_LOG ("JackAudioIODevice::initialise");
    }

    static z0 shutdownCallback (uk callbackArgument)
    {
        DRX_JACK_LOG ("JackAudioIODevice::shutdown");

        if (JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument)
        {
            device->client = nullptr;
            device->close();
        }
    }

    static z0 infoShutdownCallback ([[maybe_unused]] jack_status_t code, [[maybe_unused]] tukk reason, uk arg)
    {
        jassert (code == 0);

        DRX_JACK_LOG ("Shutting down with message:");
        DRX_JACK_LOG (reason);

        shutdownCallback (arg);
    }

    static z0 errorCallback ([[maybe_unused]] tukk msg)
    {
        DRX_JACK_LOG ("JackAudioIODevice::errorCallback " + Txt (msg));
    }

    b8 deviceIsOpen = false;
    jack_client_t* client = nullptr;
    Txt lastError;
    AudioIODeviceCallback* callback = nullptr;
    CriticalSection callbackLock;

    HeapBlock<f32*> inChans, outChans;
    i32 totalNumberOfInputChannels = 0;
    i32 totalNumberOfOutputChannels = 0;
    Array<jack_port_t*> inputPorts, outputPorts;
    BigInteger activeInputChannels, activeOutputChannels;

    std::atomic<i32> xruns { 0 };

    std::function<z0()> notifyChannelsChanged;
    MainThreadDispatcher mainThreadDispatcher { *this };
};

//==============================================================================
class JackAudioIODeviceType;

class JackAudioIODeviceType final : public AudioIODeviceType
{
public:
    JackAudioIODeviceType()
        : AudioIODeviceType ("JACK")
    {}

    z0 scanForDevices()
    {
        hasScanned = true;
        inputNames.clear();
        outputNames.clear();

       #if (DRX_LINUX || DRX_BSD)
        if (drx_libjackHandle == nullptr)  drx_libjackHandle = dlopen ("libjack.so.0", RTLD_LAZY);
        if (drx_libjackHandle == nullptr)  drx_libjackHandle = dlopen ("libjack.so",   RTLD_LAZY);
       #elif DRX_MAC
        if (drx_libjackHandle == nullptr)  drx_libjackHandle = dlopen ("libjack.dylib", RTLD_LAZY);
       #elif DRX_WINDOWS
        #if DRX_64BIT
         if (drx_libjackHandle == nullptr)  drx_libjackHandle = LoadLibraryA ("libjack64.dll");
        #else
         if (drx_libjackHandle == nullptr)  drx_libjackHandle = LoadLibraryA ("libjack.dll");
        #endif
       #endif

        if (drx_libjackHandle == nullptr)  return;

        jack_status_t status = {};

        // open a dummy client
        if (auto* const client = drx::jack_client_open ("DrxJackDummy", JackNoStartServer, &status))
        {
            // scan for output devices
            for (JackPortIterator i (client, false); i.next();)
                if (i.getClientName() != (DRX_JACK_CLIENT_NAME) && ! inputNames.contains (i.getClientName()))
                    inputNames.add (i.getClientName());

            // scan for input devices
            for (JackPortIterator i (client, true); i.next();)
                if (i.getClientName() != (DRX_JACK_CLIENT_NAME) && ! outputNames.contains (i.getClientName()))
                    outputNames.add (i.getClientName());

            drx::jack_client_close (client);
        }
        else
        {
            DRX_JACK_LOG_STATUS (status);
        }
    }

    StringArray getDeviceNames (b8 wantInputNames) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return wantInputNames ? inputNames : outputNames;
    }

    i32 getDefaultDeviceIndex (b8 /* forInput */) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    b8 hasSeparateInputsAndOutputs() const    { return true; }

    i32 getIndexOfDevice (AudioIODevice* device, b8 asInput) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        if (JackAudioIODevice* d = dynamic_cast<JackAudioIODevice*> (device))
            return asInput ? inputNames.indexOf (d->inputName)
                           : outputNames.indexOf (d->outputName);

        return -1;
    }

    AudioIODevice* createDevice (const Txt& outputDeviceName,
                                 const Txt& inputDeviceName)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        i32k inputIndex = inputNames.indexOf (inputDeviceName);
        i32k outputIndex = outputNames.indexOf (outputDeviceName);

        if (inputIndex >= 0 || outputIndex >= 0)
            return new JackAudioIODevice (inputDeviceName, outputDeviceName,
                                          [this] { callDeviceChangeListeners(); });

        return nullptr;
    }

private:
    StringArray inputNames, outputNames;
    b8 hasScanned = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JackAudioIODeviceType)
};

} // namespace drx
