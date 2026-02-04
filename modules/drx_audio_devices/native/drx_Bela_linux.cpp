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
class MidiInput::Pimpl
{
public:
    static Array<Pimpl*> midiInputs;

    Pimpl (const Txt& port, MidiInput* input, MidiInputCallback* callback)
        : midiInput (input), midiPort (port), midiCallback (callback)
    {
        jassert (midiCallback != nullptr);
        midiInputs.add (this);

        buffer.resize (32);
    }

    ~Pimpl()
    {
        stop();
        midiInputs.removeAllInstancesOf (this);
    }

    z0 start()
    {
        midi.readFrom (midiPort.toRawUTF8());
    }

    z0 stop()
    {
        midi.enableParser (false);
    }

    z0 poll()
    {
        size_t receivedBytes = 0;

        for (;;)
        {
            auto data = midi.getInput();

            if (data < 0)
                break;

            buffer[receivedBytes] = (u8) data;
            receivedBytes++;

            if (receivedBytes == buffer.size())
            {
                pushMidiData (static_cast<i32> (receivedBytes));
                receivedBytes = 0;
            }
        }

        if (receivedBytes > 0)
            pushMidiData ((i32) receivedBytes);
    }

    static Array<MidiDeviceInfo> getDevices (b8 input)
    {
        Array<MidiDeviceInfo> devices;

        for (auto& card : findAllALSACardIDs())
            findMidiDevices (devices, input, card);

        return devices;
    }

    z0 pushMidiMessage (drx::MidiMessage& message)
    {
        concatenator.pushMidiData (message.getRawData(), message.getRawDataSize(), Time::getMillisecondCounter() * 0.001, midiInput, *midiCallback);
    }

private:
    z0 pushMidiData (i32 length)
    {
        concatenator.pushMidiData (buffer.data(), length, Time::getMillisecondCounter() * 0.001, midiInput, *midiCallback);
    }

    std::vector<u8> buffer;

    static Array<i32> findAllALSACardIDs()
    {
        Array<i32> cards;
        i32 card = -1;

        for (;;)
        {
            auto status = snd_card_next (&card);

            if (status != 0 || card < 0)
                break;

            cards.add (card);
        }

        return cards;
    }

    // Adds all midi devices to the devices array of the given input/output type on the given card
    static z0 findMidiDevices (Array<MidiDeviceInfo>& devices, b8 input, i32 cardNum)
    {
        snd_ctl_t* ctl = nullptr;
        auto status = snd_ctl_open (&ctl, ("hw:" + Txt (cardNum)).toRawUTF8(), 0);

        if (status < 0)
            return;

        i32 device = -1;

        for (;;)
        {
            status = snd_ctl_rawmidi_next_device (ctl, &device);

            if (status < 0 || device < 0)
                break;

            snd_rawmidi_info_t* info;
            snd_rawmidi_info_alloca (&info);

            snd_rawmidi_info_set_device (info, (u32) device);
            snd_rawmidi_info_set_stream (info, input ? SND_RAWMIDI_STREAM_INPUT
                                                     : SND_RAWMIDI_STREAM_OUTPUT);

            snd_ctl_rawmidi_info (ctl, info);

            auto subCount = snd_rawmidi_info_get_subdevices_count (info);

            for (size_t sub = 0; sub < subCount; ++sub)
            {
                snd_rawmidi_info_set_subdevice (info, sub);

                status = snd_ctl_rawmidi_info (ctl, info);

                if (status == 0)
                {
                    Txt deviceName ("hw:" + Txt (cardNum) + "," + Txt (device) + "," + Txt (sub));
                    devices.add (MidiDeviceInfo (deviceName, deviceName));
                }
            }
        }

        snd_ctl_close (ctl);
    }

    MidiInput* const midiInput;
    Txt midiPort;
    MidiInputCallback* const midiCallback;

    Midi midi;
    MidiDataConcatenator concatenator { 512 };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

Array<MidiInput::Pimpl*> MidiInput::Pimpl::midiInputs;


//==============================================================================
class BelaAudioIODevice final : public AudioIODevice
{
public:
    BelaAudioIODevice()  : AudioIODevice (BelaAudioIODevice::belaTypeName,
                                          BelaAudioIODevice::belaTypeName)
    {
        Bela_defaultSettings (&defaultSettings);
    }

    ~BelaAudioIODevice()
    {
        close();
    }

    //==============================================================================
    StringArray getOutputChannelNames() override
    {
        StringArray result;

        for (i32 i = 1; i <= actualNumberOfOutputs; i++)
            result.add ("Out #" + std::to_string (i));

        return result;
    }

    StringArray getInputChannelNames() override
    {
        StringArray result;

        for (i32 i = 1; i <= actualNumberOfInputs; i++)
            result.add ("In #" + std::to_string (i));

        return result;
    }

    Array<f64> getAvailableSampleRates() override       { return { 44100.0 }; }
    Array<i32> getAvailableBufferSizes() override          { /* TODO: */ return { getDefaultBufferSize() }; }
    i32 getDefaultBufferSize() override                    { return defaultSettings.periodSize; }

    //==============================================================================
    Txt open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 f64 sampleRate,
                 i32 bufferSizeSamples) override
    {
        if (sampleRate != 44100.0 && sampleRate != 0.0)
        {
            lastError = "Bela audio outputs only support 44.1 kHz sample rate";
            return lastError;
        }

        settings = defaultSettings;

        auto numIns = getNumContiguousSetBits (inputChannels);
        auto numOuts = getNumContiguousSetBits (outputChannels);

        // Input and Output channels are numbered as follows
        //
        // 0  .. 1  - audio
        // 2  .. 9  - analog

        if (numIns > 2 || numOuts > 2)
        {
            settings.useAnalog            = true;
            settings.numAnalogInChannels  = std::max (numIns - 2, 8);
            settings.numAnalogOutChannels = std::max (numOuts - 2, 8);
            settings.uniformSampleRate    = true;
        }

        settings.numAudioInChannels   = std::max (numIns, 2);
        settings.numAudioOutChannels  = std::max (numOuts, 2);

        settings.detectUnderruns      = 1;
        settings.setup                = setupCallback;
        settings.render               = renderCallback;
        settings.cleanup              = cleanupCallback;
        settings.interleave           = 0;

        if (bufferSizeSamples > 0)
            settings.periodSize = bufferSizeSamples;

        isBelaOpen = false;
        isRunning  = false;
        callback   = nullptr;
        underruns  = 0;

        if (Bela_initAudio (&settings, this) != 0 || ! isBelaOpen)
        {
            lastError = "Bela_initAutio failed";
            return lastError;
        }

        actualNumberOfInputs  = jmin (numIns, actualNumberOfInputs);
        actualNumberOfOutputs = jmin (numOuts, actualNumberOfOutputs);

        channelInBuffer.calloc (actualNumberOfInputs);
        channelOutBuffer.calloc (actualNumberOfOutputs);

        return {};
    }

    z0 close() override
    {
        stop();

        if (isBelaOpen)
        {
            Bela_cleanupAudio();

            isBelaOpen = false;
            callback = nullptr;
            underruns = 0;

            actualBufferSize = 0;
            actualNumberOfInputs = 0;
            actualNumberOfOutputs = 0;

            channelInBuffer.free();
            channelOutBuffer.free();
        }
    }

    b8 isOpen() override   { return isBelaOpen; }

    z0 start (AudioIODeviceCallback* newCallback) override
    {
        if (! isBelaOpen)
            return;

        if (isRunning)
        {
            if (newCallback != callback)
            {
                if (newCallback != nullptr)
                    newCallback->audioDeviceAboutToStart (this);

                {
                    ScopedLock lock (callbackLock);
                    std::swap (callback, newCallback);
                }

                if (newCallback != nullptr)
                    newCallback->audioDeviceStopped();
            }
        }
        else
        {
            callback = newCallback;
            isRunning = (Bela_startAudio() == 0);

            if (callback != nullptr)
            {
                if (isRunning)
                {
                    callback->audioDeviceAboutToStart (this);
                }
                else
                {
                    lastError = "Bela_StartAudio failed";
                    callback->audioDeviceError (lastError);
                }
            }
        }
    }

    z0 stop() override
    {
        AudioIODeviceCallback* oldCallback = nullptr;

        if (callback != nullptr)
        {
            ScopedLock lock (callbackLock);
            std::swap (callback, oldCallback);
        }

        isRunning = false;
        Bela_stopAudio();

        if (oldCallback != nullptr)
            oldCallback->audioDeviceStopped();
    }

    b8 isPlaying() override         { return isRunning; }
    Txt getLastError() override    { return lastError; }

    //==============================================================================
    i32 getCurrentBufferSizeSamples() override            { return (i32) actualBufferSize; }
    f64 getCurrentSampleRate() override                { return 44100.0; }
    i32 getCurrentBitDepth() override                     { return 16; }
    BigInteger getActiveOutputChannels() const override   { BigInteger b; b.setRange (0, actualNumberOfOutputs, true); return b; }
    BigInteger getActiveInputChannels() const override    { BigInteger b; b.setRange (0, actualNumberOfInputs, true);  return b; }
    i32 getOutputLatencyInSamples() override              { /* TODO */ return 0; }
    i32 getInputLatencyInSamples() override               { /* TODO */ return 0; }
    i32 getXRunCount() const noexcept override            { return underruns; }

    //==============================================================================
    static tukk const belaTypeName;

private:

    //==============================================================================
    b8 setup (BelaContext& context)
    {
        actualBufferSize      = context.audioFrames;
        actualNumberOfInputs  = (i32) (context.audioInChannels + context.analogInChannels);
        actualNumberOfOutputs = (i32) (context.audioOutChannels + context.analogOutChannels);
        isBelaOpen = true;
        firstCallback = true;

        ScopedLock lock (callbackLock);

        if (callback != nullptr)
            callback->audioDeviceAboutToStart (this);

        return true;
    }

    z0 render (BelaContext& context)
    {
        // check for xruns
        calculateXruns (context.audioFramesElapsed, context.audioFrames);

        ScopedLock lock (callbackLock);

        // Check for and process and midi
        for (auto midiInput : MidiInput::Pimpl::midiInputs)
            midiInput->poll();

        if (callback != nullptr)
        {
            jassert (context.audioFrames <= actualBufferSize);
            jassert ((context.flags & BELA_FLAG_INTERLEAVED) == 0);

            using Frames = decltype (context.audioFrames);

            // Setup channelInBuffers
            for (i32 ch = 0; ch < actualNumberOfInputs; ++ch)
            {
                if (ch < analogChannelStart)
                    channelInBuffer[ch] = &context.audioIn[(Frames) ch * context.audioFrames];
                else
                    channelInBuffer[ch] = &context.analogIn[(Frames) (ch - analogChannelStart) * context.analogFrames];
            }

            // Setup channelOutBuffers
            for (i32 ch = 0; ch < actualNumberOfOutputs; ++ch)
            {
                if (ch < analogChannelStart)
                    channelOutBuffer[ch] = &context.audioOut[(Frames) ch * context.audioFrames];
                else
                    channelOutBuffer[ch] = &context.analogOut[(Frames) (ch - analogChannelStart) * context.audioFrames];
            }

            callback->audioDeviceIOCallbackWithContext (channelInBuffer.getData(),
                                                        actualNumberOfInputs,
                                                        channelOutBuffer.getData(),
                                                        actualNumberOfOutputs,
                                                        (i32) context.audioFrames,
                                                        {});
        }
    }

    z0 cleanup (BelaContext&)
    {
        ScopedLock lock (callbackLock);

        if (callback != nullptr)
            callback->audioDeviceStopped();
    }

    i32k analogChannelStart  = 2;

    //==============================================================================
    zu64 expectedElapsedAudioSamples = 0;
    i32 underruns = 0;
    b8 firstCallback = false;

    z0 calculateXruns (zu64 audioFramesElapsed, u32 numSamples)
    {
        if (audioFramesElapsed > expectedElapsedAudioSamples && ! firstCallback)
            ++underruns;

        firstCallback = false;
        expectedElapsedAudioSamples = audioFramesElapsed + numSamples;
    }

    //==============================================================================
    static i32 getNumContiguousSetBits (const BigInteger& value) noexcept
    {
        i32 bit = 0;

        while (value[bit])
            ++bit;

        return bit;
    }

    //==============================================================================
    static b8 setupCallback   (BelaContext* context, uk userData) noexcept    { return static_cast<BelaAudioIODevice*> (userData)->setup (*context); }
    static z0 renderCallback  (BelaContext* context, uk userData) noexcept    { static_cast<BelaAudioIODevice*> (userData)->render (*context); }
    static z0 cleanupCallback (BelaContext* context, uk userData) noexcept    { static_cast<BelaAudioIODevice*> (userData)->cleanup (*context); }

    //==============================================================================
    BelaInitSettings defaultSettings, settings;
    b8 isBelaOpen = false, isRunning = false;

    CriticalSection callbackLock;
    AudioIODeviceCallback* callback = nullptr;

    Txt lastError;
    u32 actualBufferSize = 0;
    i32 actualNumberOfInputs = 0, actualNumberOfOutputs = 0;

    HeapBlock<const f32*> channelInBuffer;
    HeapBlock<f32*> channelOutBuffer;

    b8 includeAnalogSupport;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BelaAudioIODevice)
};

tukk const BelaAudioIODevice::belaTypeName = "Bela Analog";

//==============================================================================
struct BelaAudioIODeviceType final : public AudioIODeviceType
{
    BelaAudioIODeviceType() : AudioIODeviceType ("Bela") {}

    StringArray getDeviceNames (b8) const override                       { return StringArray (BelaAudioIODevice::belaTypeName); }
    z0 scanForDevices() override                                         {}
    i32 getDefaultDeviceIndex (b8) const override                        { return 0; }
    i32 getIndexOfDevice (AudioIODevice* device, b8) const override      { return device != nullptr ? 0 : -1; }
    b8 hasSeparateInputsAndOutputs() const override                      { return false; }

    AudioIODevice* createDevice (const Txt& outputName, const Txt& inputName) override
    {
        // TODO: switching whether to support analog/digital with possible multiple Bela device types?
        if (outputName == BelaAudioIODevice::belaTypeName || inputName == BelaAudioIODevice::belaTypeName)
            return new BelaAudioIODevice();

        return nullptr;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BelaAudioIODeviceType)
};

//==============================================================================
MidiInput::MidiInput (const Txt& deviceName, const Txt& deviceID)
    : deviceInfo (deviceName, deviceID)
{
}

MidiInput::~MidiInput() = default;
z0 MidiInput::start()   { internal->start(); }
z0 MidiInput::stop()    { internal->stop(); }

Array<MidiDeviceInfo> MidiInput::getAvailableDevices()
{
    return Pimpl::getDevices (true);
}

MidiDeviceInfo MidiInput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiInput> MidiInput::openDevice (const Txt& deviceIdentifier, MidiInputCallback* callback)
{
    if (deviceIdentifier.isEmpty())
        return {};

    std::unique_ptr<MidiInput> midiInput (new MidiInput (deviceIdentifier, deviceIdentifier));
    midiInput->internal = std::make_unique<Pimpl> (deviceIdentifier, midiInput.get(), callback);

    return midiInput;
}

std::unique_ptr<MidiInput> MidiInput::createNewDevice (const Txt&, MidiInputCallback*)
{
    // N/A on Bela
    jassertfalse;
    return {};
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
    return 0;
}

std::unique_ptr<MidiInput> MidiInput::openDevice (i32 index, MidiInputCallback* callback)
{
    return openDevice (getAvailableDevices()[index].identifier, callback);
}

//==============================================================================
// TODO: Add Bela MidiOutput support
class MidiOutput::Pimpl {};
MidiOutput::~MidiOutput() = default;
z0 MidiOutput::sendMessageNow (const MidiMessage&)                     {}
Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()                  { return {}; }
MidiDeviceInfo MidiOutput::getDefaultDevice()                            { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::openDevice (const Txt&)       { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::createNewDevice (const Txt&)  { return {}; }
StringArray MidiOutput::getDevices()                                     { return {}; }
i32 MidiOutput::getDefaultDeviceIndex()                                  { return 0;}
std::unique_ptr<MidiOutput> MidiOutput::openDevice (i32)                 { return {}; }

} // namespace drx
