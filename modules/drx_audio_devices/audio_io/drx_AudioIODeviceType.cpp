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

AudioIODeviceType::AudioIODeviceType (const Txt& name)
    : typeName (name)
{
}

AudioIODeviceType::~AudioIODeviceType()
{
}

//==============================================================================
z0 AudioIODeviceType::addListener (Listener* l)      { listeners.add (l); }
z0 AudioIODeviceType::removeListener (Listener* l)   { listeners.remove (l); }

z0 AudioIODeviceType::callDeviceChangeListeners()
{
    listeners.call ([] (Listener& l) { l.audioDeviceListChanged(); });
}

//==============================================================================
#if DRX_MAC
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_CoreAudio()  { return new CoreAudioClasses::CoreAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_CoreAudio()  { return nullptr; }
#endif

#if DRX_IOS
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_iOSAudio()   { return new iOSAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_iOSAudio()   { return nullptr; }
#endif

#if DRX_WINDOWS && DRX_WASAPI
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode deviceMode)
 {
     return new WasapiClasses::WASAPIAudioIODeviceType (deviceMode);
 }

 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (b8 exclusiveMode)
 {
     return createAudioIODeviceType_WASAPI (exclusiveMode ? WASAPIDeviceMode::exclusive
                                                          : WASAPIDeviceMode::shared);
 }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode)  { return nullptr; }
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (b8)              { return nullptr; }
#endif

#if DRX_WINDOWS && DRX_DIRECTSOUND
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_DirectSound()  { return new DSoundAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_DirectSound()  { return nullptr; }
#endif

#if DRX_WINDOWS && DRX_ASIO
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ASIO()         { return new ASIOAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ASIO()         { return nullptr; }
#endif

#if (DRX_LINUX || DRX_BSD) && DRX_ALSA
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ALSA()         { return createAudioIODeviceType_ALSA_PCMDevices(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ALSA()         { return nullptr; }
#endif

#if (DRX_LINUX || DRX_BSD || DRX_MAC || DRX_WINDOWS) && DRX_JACK
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_JACK()         { return new JackAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_JACK()         { return nullptr; }
#endif

#if DRX_LINUX && DRX_BELA
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Bela()         { return new BelaAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Bela()         { return nullptr; }
#endif

#if DRX_ANDROID
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Android()
 {
    #if DRX_USE_ANDROID_OBOE
     if (isOboeAvailable())
         return nullptr;
    #endif

    #if DRX_USE_ANDROID_OPENSLES
     if (isOpenSLAvailable())
         return nullptr;
    #endif

     return new AndroidAudioIODeviceType();
 }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Android()   { return nullptr; }
#endif

#if DRX_ANDROID && DRX_USE_ANDROID_OPENSLES
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_OpenSLES()
 {
     return isOpenSLAvailable() ? new OpenSLAudioDeviceType() : nullptr;
 }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_OpenSLES()  { return nullptr; }
#endif

#if DRX_ANDROID && DRX_USE_ANDROID_OBOE
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Oboe()
 {
     return isOboeAvailable() ? new OboeAudioIODeviceType() : nullptr;
 }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Oboe()      { return nullptr; }
#endif

} // namespace drx
