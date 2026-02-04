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

z0 AudioIODeviceCallback::audioDeviceIOCallbackWithContext ([[maybe_unused]] const f32* const* inputChannelData,
                                                              [[maybe_unused]] i32 numInputChannels,
                                                              [[maybe_unused]] f32* const* outputChannelData,
                                                              [[maybe_unused]] i32 numOutputChannels,
                                                              [[maybe_unused]] i32 numSamples,
                                                              [[maybe_unused]] const AudioIODeviceCallbackContext& context) {}

//==============================================================================
AudioIODevice::AudioIODevice (const Txt& deviceName, const Txt& deviceTypeName)
    : name (deviceName), typeName (deviceTypeName)
{
}

AudioIODevice::~AudioIODevice() {}

z0 AudioIODeviceCallback::audioDeviceError (const Txt&)    {}
b8 AudioIODevice::setAudioPreprocessingEnabled (b8)         { return false; }
b8 AudioIODevice::hasControlPanel() const                     { return false; }
i32  AudioIODevice::getXRunCount() const noexcept               { return -1; }

b8 AudioIODevice::showControlPanel()
{
    jassertfalse;    // this should only be called for devices which return true from
                     // their hasControlPanel() method.
    return false;
}

} // namespace drx
