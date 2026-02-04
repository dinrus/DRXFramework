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

#pragma once


//==============================================================================
#define UNITY_AUDIO_PLUGIN_API_VERSION 0x010401

#if DRX_WINDOWS
 #define UNITY_INTERFACE_API __stdcall
 #define UNITY_INTERFACE_EXPORT __declspec (dllexport)
#else
 #define UNITY_INTERFACE_API
 #define UNITY_INTERFACE_EXPORT __attribute__ ((visibility ("default")))
#endif

//==============================================================================
struct UnityAudioEffectState;

typedef i32  (UNITY_INTERFACE_API * createCallback)              (UnityAudioEffectState* state);
typedef i32  (UNITY_INTERFACE_API * releaseCallback)             (UnityAudioEffectState* state);
typedef i32  (UNITY_INTERFACE_API * resetCallback)               (UnityAudioEffectState* state);

typedef i32  (UNITY_INTERFACE_API * processCallback)             (UnityAudioEffectState* state, f32* inBuffer, f32* outBuffer, u32 bufferSize,
                                                                  i32 numInChannels, i32 numOutChannels);

typedef i32  (UNITY_INTERFACE_API * setPositionCallback)         (UnityAudioEffectState* state, u32 pos);

typedef i32  (UNITY_INTERFACE_API * setFloatParameterCallback)   (UnityAudioEffectState* state, i32 index, f32 value);
typedef i32  (UNITY_INTERFACE_API * getFloatParameterCallback)   (UnityAudioEffectState* state, i32 index, f32* value, tuk valuestr);
typedef i32  (UNITY_INTERFACE_API * getFloatBufferCallback)      (UnityAudioEffectState* state, tukk name, f32* buffer, i32 numsamples);

typedef i32  (UNITY_INTERFACE_API * distanceAttenuationCallback) (UnityAudioEffectState* state, f32 distanceIn, f32 attenuationIn, f32* attenuationOut);

typedef z0 (UNITY_INTERFACE_API * renderCallback)              (i32 eventId);

//==============================================================================
enum UnityAudioEffectDefinitionFlags
{
    isSideChainTarget = 1,
    isSpatializer = 2,
    isAmbisonicDecoder = 4,
    appliesDistanceAttenuation = 8
};

enum UnityAudioEffectStateFlags
{
    stateIsPlaying = 1,
    stateIsPaused = 2,
    stateIsMuted = 8,
    statIsSideChainTarget = 16
};

enum UnityEventModifiers
{
    shift = 1,
    control = 2,
    alt = 4,
    command = 8,
    numeric = 16,
    capsLock = 32,
    functionKey = 64
};

//==============================================================================
#ifndef DOXYGEN

struct UnityAudioSpatializerData
{
    f32                          listenerMatrix[16];
    f32                          sourceMatrix[16];
    f32                          spatialBlend;
    f32                          reverbZoneMix;
    f32                          spread;
    f32                          stereoPan;
    distanceAttenuationCallback    attenuationCallback;
    f32                          minDistance;
    f32                          maxDistance;
};

struct UnityAudioAmbisonicData
{
    f32                          listenerMatrix[16];
    f32                          sourceMatrix[16];
    f32                          spatialBlend;
    f32                          reverbZoneMix;
    f32                          spread;
    f32                          stereoPan;
    distanceAttenuationCallback    attenuationCallback;
    i32                            ambisonicOutChannels;
    f32                          volume;
};

struct UnityAudioEffectState
{
    drx::u32               structSize;
    drx::u32               sampleRate;
    drx::zu64               dspCurrentTick;
    drx::zu64               dspPreviousTick;
    f32*                     sidechainBuffer;
    uk                      effectData;
    drx::u32               flags;
    uk                      internal;

    UnityAudioSpatializerData* spatializerData;
    drx::u32               dspBufferSize;
    drx::u32               hostAPIVersion;

    UnityAudioAmbisonicData*   ambisonicData;

    template <typename T>
    inline T* getEffectData() const
    {
        jassert (effectData != nullptr);
        jassert (internal != nullptr);

        return (T*) effectData;
    }
};

struct UnityAudioParameterDefinition
{
    t8        name[16];
    t8        unit[16];
    tukk description;
    f32       min;
    f32       max;
    f32       defaultVal;
    f32       displayScale;
    f32       displayExponent;
};

struct UnityAudioEffectDefinition
{
    drx::u32                   structSize;
    drx::u32                   parameterStructSize;
    drx::u32                   apiVersion;
    drx::u32                   pluginVersion;
    drx::u32                   channels;
    drx::u32                   numParameters;
    drx::zu64                   flags;
    t8                           name[32];
    createCallback                 create;
    releaseCallback                release;
    resetCallback                  reset;
    processCallback                process;
    setPositionCallback            setPosition;
    UnityAudioParameterDefinition* parameterDefintions;
    setFloatParameterCallback      setFloatParameter;
    getFloatParameterCallback      getFloatParameter;
    getFloatBufferCallback         getFloatBuffer;
};

#endif

//==============================================================================
// Unity callback
extern "C" UNITY_INTERFACE_EXPORT i32  UNITY_INTERFACE_API UnityGetAudioEffectDefinitions (UnityAudioEffectDefinition*** definitionsPtr);

// GUI script callbacks
extern "C" UNITY_INTERFACE_EXPORT renderCallback UNITY_INTERFACE_API getRenderCallback();

extern "C" UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityInitialiseTexture (i32 id, uk textureHandle, i32 w, i32 h);

extern "C" UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityMouseDown (i32 id, f32 x, f32 y, UnityEventModifiers mods, i32 button);
extern "C" UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityMouseDrag (i32 id, f32 x, f32 y, UnityEventModifiers mods, i32 button);
extern "C" UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityMouseUp   (i32 id, f32 x, f32 y, UnityEventModifiers mods);

extern "C" UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityKeyEvent (i32 id, i32 code, UnityEventModifiers mods, tukk name);

extern "C" UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unitySetScreenBounds (i32 id, f32 x, f32 y, f32 w, f32 h);
