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

class iOSAudioIODeviceType;

class iOSAudioIODevice : public AudioIODevice
{
public:
    //==============================================================================
    Txt open (const BigInteger&, const BigInteger&, f64, i32) override;
    z0 close() override;

    z0 start (AudioIODeviceCallback*) override;
    z0 stop() override;

    Array<f64> getAvailableSampleRates() override;
    Array<i32> getAvailableBufferSizes() override;

    b8 setAudioPreprocessingEnabled (b8) override;

    //==============================================================================
    b8 isPlaying() override;
    b8 isOpen() override;
    Txt getLastError() override;

    //==============================================================================
    StringArray getOutputChannelNames() override;
    StringArray getInputChannelNames() override;

    i32 getDefaultBufferSize() override;
    i32 getCurrentBufferSizeSamples() override;

    f64 getCurrentSampleRate() override;

    i32 getCurrentBitDepth() override;

    BigInteger getActiveOutputChannels() const override;
    BigInteger getActiveInputChannels() const override;

    i32 getOutputLatencyInSamples() override;
    i32 getInputLatencyInSamples() override;

    i32 getXRunCount() const noexcept override;

    AudioWorkgroup getWorkgroup() const override;

    //==============================================================================
    z0 setMidiMessageCollector (MidiMessageCollector*);
    AudioPlayHead* getAudioPlayHead() const;

    //==============================================================================
    b8 isInterAppAudioConnected() const;
   #if DRX_MODULE_AVAILABLE_drx_graphics
    Image getIcon (i32 size);
   #endif
    z0 switchApplication();

private:
    //==============================================================================
    iOSAudioIODevice (iOSAudioIODeviceType*, const Txt&, const Txt&);

    //==============================================================================
    friend class iOSAudioIODeviceType;
    friend struct AudioSessionHolder;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    DRX_DECLARE_NON_COPYABLE (iOSAudioIODevice)
};

} // namespace drx
