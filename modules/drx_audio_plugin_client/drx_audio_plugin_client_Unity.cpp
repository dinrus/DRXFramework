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

#include <drx_core/system/drx_TargetPlatform.h>

#if DrxPlugin_Build_Unity

#include <drx_audio_plugin_client/detail/drx_PluginUtilities.h>
#include <drx_audio_processors/format_types/drx_LegacyAudioParameter.cpp>

#if DRX_WINDOWS
 #include <drx_audio_plugin_client/detail/drx_IncludeSystemHeaders.h>
#endif

#include <drx_audio_plugin_client/Unity/drx_UnityPluginInterface.h>

#include <drx_events/native/drx_RunningInUnity.h>

//==============================================================================
namespace drx
{

typedef ComponentPeer* (*createUnityPeerFunctionType) (Component&);
extern createUnityPeerFunctionType drx_createUnityPeerFn;

//==============================================================================
class UnityPeer final : public ComponentPeer,
                        public AsyncUpdater
{
public:
    UnityPeer (Component& ed)
        : ComponentPeer (ed, 0),
          mouseWatcher (*this)
    {
        getEditor().setResizable (false, false);
    }

    //==============================================================================
    Rectangle<i32> getBounds() const override                              { return bounds; }
    Point<f32> localToGlobal (Point<f32> relativePosition) override    { return relativePosition + getBounds().getPosition().toFloat(); }
    Point<f32> globalToLocal (Point<f32> screenPosition) override      { return screenPosition - getBounds().getPosition().toFloat(); }

    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;

    StringArray getAvailableRenderingEngines() override                    { return StringArray ("Software Renderer"); }

    z0 setBounds (const Rectangle<i32>& newBounds, b8) override
    {
        bounds = newBounds;
        mouseWatcher.setBoundsToWatch (bounds);
    }

    b8 contains (Point<i32> localPos, b8) const override
    {
        if (isPositiveAndBelow (localPos.getX(), getBounds().getWidth())
               && isPositiveAndBelow (localPos.getY(), getBounds().getHeight()))
            return true;

        return false;
    }

    z0 handleAsyncUpdate() override
    {
        fillPixels();
    }

    //==============================================================================
    AudioProcessorEditor& getEditor()    { return *dynamic_cast<AudioProcessorEditor*> (&getComponent()); }

    z0 setPixelDataHandle (u8* handle, i32 width, i32 height)
    {
        pixelData = handle;

        textureWidth = width;
        textureHeight = height;

        renderImage = Image (new UnityBitmapImage (pixelData, width, height));
    }

    // N.B. This is NOT an efficient way to do this and you shouldn't use this method in your own code.
    // It works for our purposes here but a much more efficient way would be to use a GL texture.
    z0 fillPixels()
    {
        if (pixelData == nullptr)
            return;

        LowLevelGraphicsSoftwareRenderer renderer (renderImage);
        renderer.addTransform (AffineTransform::verticalFlip ((f32) getComponent().getHeight()));

        handlePaint (renderer);

        for (i32 i = 0; i < textureWidth * textureHeight * 4; i += 4)
        {
            auto r = pixelData[i + 2];
            auto g = pixelData[i + 1];
            auto b = pixelData[i + 0];

            pixelData[i + 0] = r;
            pixelData[i + 1] = g;
            pixelData[i + 2] = b;
        }
    }

    z0 forwardMouseEvent (Point<f32> position, ModifierKeys mods)
    {
        ModifierKeys::currentModifiers = mods;

        handleMouseEvent (drx::MouseInputSource::mouse, position, mods, drx::MouseInputSource::defaultPressure,
                          drx::MouseInputSource::defaultOrientation, drx::Time::currentTimeMillis());
    }

    z0 forwardKeyPress (i32 code, Txt name, ModifierKeys mods)
    {
        ModifierKeys::currentModifiers = mods;

        handleKeyPress (getKeyPress (code, name));
    }

private:
    //==============================================================================
    struct UnityBitmapImage final : public ImagePixelData
    {
        UnityBitmapImage (u8* data, i32 w, i32 h)
            : ImagePixelData (Image::PixelFormat::ARGB, w, h),
              imageData (data),
              lineStride (width * pixelStride)
        {
        }

        std::unique_ptr<ImageType> createType() const override
        {
            return std::make_unique<SoftwareImageType>();
        }

        std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
        {
            return std::make_unique<LowLevelGraphicsSoftwareRenderer> (Image (this));
        }

        z0 initialiseBitmapData (Image::BitmapData& bitmap, i32 x, i32 y, [[maybe_unused]] Image::BitmapData::ReadWriteMode mode) override
        {
            const auto offset = (size_t) x * (size_t) pixelStride + (size_t) y * (size_t) lineStride;
            bitmap.data = imageData + offset;
            bitmap.size = (size_t) (lineStride * height) - offset;
            bitmap.pixelFormat = pixelFormat;
            bitmap.lineStride = lineStride;
            bitmap.pixelStride = pixelStride;
        }

        ImagePixelData::Ptr clone() override
        {
            auto im = new UnityBitmapImage (imageData, width, height);

            for (i32 i = 0; i < height; ++i)
                memcpy (im->imageData + i * lineStride, imageData + i * lineStride, (size_t) lineStride);

            return im;
        }

        u8* imageData;
        i32 pixelStride = 4, lineStride;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnityBitmapImage)
    };

    //==============================================================================
    struct MouseWatcher final : public Timer
    {
        MouseWatcher (ComponentPeer& o)    : owner (o)    {}

        z0 timerCallback() override
        {
            auto pos = Desktop::getMousePosition();

            if (boundsToWatch.contains (pos) && pos != lastMousePos)
            {
                auto ms = Desktop::getInstance().getMainMouseSource();

                if (! ms.getCurrentModifiers().isLeftButtonDown())
                    owner.handleMouseEvent (drx::MouseInputSource::mouse, owner.globalToLocal (pos.toFloat()), {},
                                            drx::MouseInputSource::defaultPressure, drx::MouseInputSource::defaultOrientation, drx::Time::currentTimeMillis());

                lastMousePos = pos;
            }

        }

        z0 setBoundsToWatch (Rectangle<i32> b)
        {
            if (boundsToWatch != b)
                boundsToWatch = b;

            startTimer (250);
        }

        ComponentPeer& owner;
        Rectangle<i32> boundsToWatch;
        Point<i32> lastMousePos;
    };

    //==============================================================================
    KeyPress getKeyPress (i32 keyCode, Txt name)
    {
        if (keyCode >= 32 && keyCode <= 64)
            return { keyCode, ModifierKeys::currentModifiers, drx::t32 (keyCode) };

        if (keyCode >= 91 && keyCode <= 122)
            return { keyCode, ModifierKeys::currentModifiers, name[0] };

        if (keyCode >= 256 && keyCode <= 265)
            return { drx::KeyPress::numberPad0 + (keyCode - 256), ModifierKeys::currentModifiers, drx::Txt (keyCode - 256).getCharPointer()[0] };

        if (keyCode == 8)      return { drx::KeyPress::backspaceKey,          ModifierKeys::currentModifiers, {} };
        if (keyCode == 127)    return { drx::KeyPress::deleteKey,             ModifierKeys::currentModifiers, {} };
        if (keyCode == 9)      return { drx::KeyPress::tabKey,                ModifierKeys::currentModifiers, {} };
        if (keyCode == 13)     return { drx::KeyPress::returnKey,             ModifierKeys::currentModifiers, {} };
        if (keyCode == 27)     return { drx::KeyPress::escapeKey,             ModifierKeys::currentModifiers, {} };
        if (keyCode == 32)     return { drx::KeyPress::spaceKey,              ModifierKeys::currentModifiers, {} };
        if (keyCode == 266)    return { drx::KeyPress::numberPadDecimalPoint, ModifierKeys::currentModifiers, {} };
        if (keyCode == 267)    return { drx::KeyPress::numberPadDivide,       ModifierKeys::currentModifiers, {} };
        if (keyCode == 268)    return { drx::KeyPress::numberPadMultiply,     ModifierKeys::currentModifiers, {} };
        if (keyCode == 269)    return { drx::KeyPress::numberPadSubtract,     ModifierKeys::currentModifiers, {} };
        if (keyCode == 270)    return { drx::KeyPress::numberPadAdd,          ModifierKeys::currentModifiers, {} };
        if (keyCode == 272)    return { drx::KeyPress::numberPadEquals,       ModifierKeys::currentModifiers, {} };
        if (keyCode == 273)    return { drx::KeyPress::upKey,                 ModifierKeys::currentModifiers, {} };
        if (keyCode == 274)    return { drx::KeyPress::downKey,               ModifierKeys::currentModifiers, {} };
        if (keyCode == 275)    return { drx::KeyPress::rightKey,              ModifierKeys::currentModifiers, {} };
        if (keyCode == 276)    return { drx::KeyPress::leftKey,               ModifierKeys::currentModifiers, {} };

        return {};
    }

    //==============================================================================
    Rectangle<i32> bounds;
    MouseWatcher mouseWatcher;

    u8* pixelData = nullptr;
    i32 textureWidth, textureHeight;
    Image renderImage;

    //==============================================================================
    z0 setMinimised (b8) override                                 {}
    b8 isMinimised() const override                                 { return false; }
    b8 isShowing() const override                                   { return true; }
    z0 setFullScreen (b8) override                                {}
    b8 isFullScreen() const override                                { return false; }
    b8 setAlwaysOnTop (b8) override                               { return false; }
    z0 toFront (b8) override                                      {}
    z0 toBehind (ComponentPeer*) override                           {}
    b8 isFocused() const override                                   { return true; }
    z0 grabFocus() override                                         {}
    uk getNativeHandle() const override                            { return nullptr; }
    OptionalBorderSize getFrameSizeIfPresent() const override         { return {}; }
    BorderSize<i32> getFrameSize() const override                     { return {}; }
    z0 setVisible (b8) override                                   {}
    z0 setTitle (const Txt&) override                            {}
    z0 setIcon (const Image&) override                              {}
    z0 textInputRequired (Point<i32>, TextInputTarget&) override    {}
    z0 setAlpha (f32) override                                    {}
    z0 performAnyPendingRepaintsNow() override                      {}
    z0 repaint (const Rectangle<i32>&) override                     {}

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnityPeer)
};

static ComponentPeer* createUnityPeer (Component& c)    { return new UnityPeer (c); }

//==============================================================================
class AudioProcessorUnityWrapper
{
public:
    AudioProcessorUnityWrapper (b8 isTemporary)
    {
        detail::RunningInUnity::state = true;
        pluginInstance = createPluginFilterOfType (AudioProcessor::wrapperType_Unity);

        if (! isTemporary && pluginInstance->hasEditor())
        {
            pluginInstanceEditor.reset (pluginInstance->createEditorIfNeeded());
            pluginInstanceEditor->setVisible (true);
            detail::PluginUtilities::addToDesktop (*pluginInstanceEditor, nullptr);
        }

        juceParameters.update (*pluginInstance, false);
    }

    ~AudioProcessorUnityWrapper()
    {
        if (pluginInstanceEditor != nullptr)
        {
            pluginInstanceEditor->removeFromDesktop();

            PopupMenu::dismissAllActiveMenus();
            pluginInstanceEditor->processor.editorBeingDeleted (pluginInstanceEditor.get());
            pluginInstanceEditor = nullptr;
        }
    }

    z0 create (UnityAudioEffectState* state)
    {
        // only supported in Unity plugin API > 1.0
        if (state->structSize >= sizeof (UnityAudioEffectState))
            samplesPerBlock = static_cast<i32> (state->dspBufferSize);

       #ifdef DrxPlugin_PreferredChannelConfigurations
        short configs[][2] = { DrxPlugin_PreferredChannelConfigurations };
        [[maybe_unused]] i32k numConfigs = sizeof (configs) / sizeof (short[2]);

        jassert (numConfigs > 0 && (configs[0][0] > 0 || configs[0][1] > 0));

        pluginInstance->setPlayConfigDetails (configs[0][0], configs[0][1], state->sampleRate, samplesPerBlock);
       #else
        pluginInstance->setRateAndBufferSizeDetails (state->sampleRate, samplesPerBlock);
       #endif

        pluginInstance->prepareToPlay (state->sampleRate, samplesPerBlock);

        scratchBuffer.setSize (jmax (pluginInstance->getTotalNumInputChannels(), pluginInstance->getTotalNumOutputChannels()), samplesPerBlock);
    }

    z0 release()
    {
        pluginInstance->releaseResources();
    }

    z0 reset()
    {
        pluginInstance->reset();
    }

    z0 process (f32* inBuffer, f32* outBuffer, i32 bufferSize, i32 numInChannels, i32 numOutChannels, b8 isBypassed)
    {
        // If the plugin has a bypass parameter, set it to the current bypass state
        if (auto* param = pluginInstance->getBypassParameter())
            if (isBypassed != (param->getValue() >= 0.5f))
                param->setValueNotifyingHost (isBypassed ? 1.0f : 0.0f);

        for (i32 pos = 0; pos < bufferSize;)
        {
            auto max = jmin (bufferSize - pos, samplesPerBlock);
            processBuffers (inBuffer + (pos * numInChannels), outBuffer + (pos * numOutChannels), max, numInChannels, numOutChannels, isBypassed);

            pos += max;
        }
    }

    z0 declareParameters (UnityAudioEffectDefinition& definition)
    {
        static std::unique_ptr<UnityAudioParameterDefinition> parametersPtr;
        static i32 numParams = 0;

        if (parametersPtr == nullptr)
        {
            numParams = (i32) juceParameters.size();

            parametersPtr.reset (static_cast<UnityAudioParameterDefinition*> (std::calloc (static_cast<size_t> (numParams),
                                                                              sizeof (UnityAudioParameterDefinition))));

            parameterDescriptions.clear();

            for (i32 i = 0; i < numParams; ++i)
            {
                auto* parameter = juceParameters.getParamForIndex (i);
                auto& paramDef = parametersPtr.get()[i];

                const auto nameLength = (size_t) numElementsInArray (paramDef.name);
                const auto unitLength = (size_t) numElementsInArray (paramDef.unit);

                parameter->getName ((i32) nameLength - 1).copyToUTF8 (paramDef.name, nameLength);

                if (parameter->getLabel().isNotEmpty())
                    parameter->getLabel().copyToUTF8 (paramDef.unit, unitLength);

                parameterDescriptions.add (parameter->getName (15));
                paramDef.description = parameterDescriptions[i].toRawUTF8();

                paramDef.defaultVal = parameter->getDefaultValue();
                paramDef.min = 0.0f;
                paramDef.max = 1.0f;
                paramDef.displayScale = 1.0f;
                paramDef.displayExponent = 1.0f;
            }
        }

        definition.numParameters = static_cast<u32> (numParams);
        definition.parameterDefintions = parametersPtr.get();
    }

    z0 setParameter (i32 index, f32 value)       { juceParameters.getParamForIndex (index)->setValueNotifyingHost (value); }
    f32 getParameter (i32 index) const noexcept    { return juceParameters.getParamForIndex (index)->getValue(); }

    Txt getParameterString (i32 index) const noexcept
    {
        auto* param = juceParameters.getParamForIndex (index);
        return param->getText (param->getValue(), 16);
    }

    i32 getNumInputChannels() const noexcept         { return pluginInstance->getTotalNumInputChannels(); }
    i32 getNumOutputChannels() const noexcept        { return pluginInstance->getTotalNumOutputChannels(); }

    b8 hasEditor() const noexcept                  { return pluginInstance->hasEditor(); }

    UnityPeer& getEditorPeer() const
    {
        auto* peer = dynamic_cast<UnityPeer*> (pluginInstanceEditor->getPeer());

        jassert (peer != nullptr);
        return *peer;
    }

private:
    //==============================================================================
    z0 processBuffers (f32* inBuffer, f32* outBuffer, i32 bufferSize, i32 numInChannels, i32 numOutChannels, b8 isBypassed)
    {
        i32 ch;
        for (ch = 0; ch < numInChannels; ++ch)
        {
            using DstSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst>;
            using SrcSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::Interleaved,    AudioData::Const>;

            DstSampleType dstData (scratchBuffer.getWritePointer (ch));
            SrcSampleType srcData (inBuffer + ch, numInChannels);
            dstData.convertSamples (srcData, bufferSize);
        }

        for (; ch < numOutChannels; ++ch)
            scratchBuffer.clear (ch, 0, bufferSize);

        {
            const ScopedLock sl (pluginInstance->getCallbackLock());

            if (pluginInstance->isSuspended())
            {
                scratchBuffer.clear();
            }
            else
            {
                MidiBuffer mb;

                if (isBypassed && pluginInstance->getBypassParameter() == nullptr)
                    pluginInstance->processBlockBypassed (scratchBuffer, mb);
                else
                    pluginInstance->processBlock (scratchBuffer, mb);
            }
        }

        for (ch = 0; ch < numOutChannels; ++ch)
        {
            using DstSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::Interleaved,    AudioData::NonConst>;
            using SrcSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const>;

            DstSampleType dstData (outBuffer + ch, numOutChannels);
            SrcSampleType srcData (scratchBuffer.getReadPointer (ch));
            dstData.convertSamples (srcData, bufferSize);
        }
    }

    //==============================================================================
    std::unique_ptr<AudioProcessor> pluginInstance;
    std::unique_ptr<AudioProcessorEditor> pluginInstanceEditor;

    i32 samplesPerBlock = 1024;
    StringArray parameterDescriptions;

    AudioBuffer<f32> scratchBuffer;

    LegacyAudioParametersWrapper juceParameters;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorUnityWrapper)
};

//==============================================================================
static HashMap<i32, AudioProcessorUnityWrapper*>& getWrapperMap()
{
    static HashMap<i32, AudioProcessorUnityWrapper*> wrapperMap;
    return wrapperMap;
}

static z0 onWrapperCreation (AudioProcessorUnityWrapper* wrapperToAdd)
{
    getWrapperMap().set (std::abs (Random::getSystemRandom().nextInt (65536)), wrapperToAdd);
}

static z0 onWrapperDeletion (AudioProcessorUnityWrapper* wrapperToRemove)
{
    getWrapperMap().removeValue (wrapperToRemove);
}

//==============================================================================
static UnityAudioEffectDefinition getEffectDefinition()
{
    const auto wrapper = std::make_unique<AudioProcessorUnityWrapper> (true);
    const Txt originalName { DrxPlugin_Name };
    const auto name = (! originalName.startsWithIgnoreCase ("audioplugin") ? "audioplugin_" : "") + originalName;

    UnityAudioEffectDefinition result{};
    name.copyToUTF8 (result.name, (size_t) numElementsInArray (result.name));

    result.structSize = sizeof (UnityAudioEffectDefinition);
    result.parameterStructSize = sizeof (UnityAudioParameterDefinition);

    result.apiVersion = UNITY_AUDIO_PLUGIN_API_VERSION;
    result.pluginVersion = DrxPlugin_VersionCode;

    // effects must set this to 0, generators > 0
    result.channels = (wrapper->getNumInputChannels() != 0 ? 0
                                                           : static_cast<u32> (wrapper->getNumOutputChannels()));

    wrapper->declareParameters (result);

    result.create = [] (UnityAudioEffectState* state)
    {
        auto* pluginInstance = new AudioProcessorUnityWrapper (false);
        pluginInstance->create (state);

        state->effectData = pluginInstance;

        onWrapperCreation (pluginInstance);

        return 0;
    };

    result.release = [] (UnityAudioEffectState* state)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();
        pluginInstance->release();

        onWrapperDeletion (pluginInstance);
        delete pluginInstance;

        if (getWrapperMap().size() == 0)
            shutdownDrx_GUI();

        return 0;
    };

    result.reset = [] (UnityAudioEffectState* state)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();
        pluginInstance->reset();

        return 0;
    };

    result.setPosition = [] (UnityAudioEffectState* state, u32 pos)
    {
        ignoreUnused (state, pos);
        return 0;
    };

    result.process = [] (UnityAudioEffectState* state,
                         f32* inBuffer,
                         f32* outBuffer,
                         u32 bufferSize,
                         i32 numInChannels,
                         i32 numOutChannels)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();

        if (pluginInstance != nullptr)
        {
            auto isPlaying = ((state->flags & stateIsPlaying) != 0);
            auto isMuted   = ((state->flags & stateIsMuted)   != 0);
            auto isPaused  = ((state->flags & stateIsPaused)  != 0);

            const auto bypassed = ! isPlaying || (isMuted || isPaused);
            pluginInstance->process (inBuffer, outBuffer, static_cast<i32> (bufferSize), numInChannels, numOutChannels, bypassed);
        }
        else
        {
            FloatVectorOperations::clear (outBuffer, static_cast<i32> (bufferSize) * numOutChannels);
        }

        return 0;
    };

    result.setFloatParameter = [] (UnityAudioEffectState* state, i32 index, f32 value)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();
        pluginInstance->setParameter (index, value);

        return 0;
    };

    result.getFloatParameter = [] (UnityAudioEffectState* state, i32 index, f32* value, tuk valueStr)
    {
        auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();
        *value = pluginInstance->getParameter (index);

        pluginInstance->getParameterString (index).copyToUTF8 (valueStr, 15);

        return 0;
    };

    result.getFloatBuffer = [] (UnityAudioEffectState* state, tukk kind, f32* buffer, i32 numSamples)
    {
        ignoreUnused (numSamples);

        const StringRef kindStr { kind };

        if (kindStr == StringRef ("Editor"))
        {
            auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();

            buffer[0] = pluginInstance->hasEditor() ? 1.0f : 0.0f;
        }
        else if (kindStr == StringRef ("ID"))
        {
            auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();

            for (HashMap<i32, AudioProcessorUnityWrapper*>::Iterator i (getWrapperMap()); i.next();)
            {
                if (i.getValue() == pluginInstance)
                {
                    buffer[0] = (f32) i.getKey();
                    break;
                }
            }

            return 0;
        }
        else if (kindStr == StringRef ("Size"))
        {
            auto* pluginInstance = state->getEffectData<AudioProcessorUnityWrapper>();

            auto& editor = pluginInstance->getEditorPeer().getEditor();

            buffer[0] = (f32) editor.getBounds().getWidth();
            buffer[1] = (f32) editor.getBounds().getHeight();
            buffer[2] = (f32) editor.getConstrainer()->getMinimumWidth();
            buffer[3] = (f32) editor.getConstrainer()->getMinimumHeight();
            buffer[4] = (f32) editor.getConstrainer()->getMaximumWidth();
            buffer[5] = (f32) editor.getConstrainer()->getMaximumHeight();
        }

        return 0;
    };

    return result;
}

} // namespace drx

// From reading the example code, it seems that the triple indirection indicates
// an out-value of an array of pointers. That is, after calling this function, definitionsPtr
// should point to a pre-existing/static array of pointer-to-effect-definition.
UNITY_INTERFACE_EXPORT i32 UNITY_INTERFACE_API UnityGetAudioEffectDefinitions (UnityAudioEffectDefinition*** definitionsPtr)
{
    if (drx::getWrapperMap().size() == 0)
        drx::initialiseDrx_GUI();

    static std::once_flag flag;
    std::call_once (flag, [] { drx::drx_createUnityPeerFn = drx::createUnityPeer; });

    static auto definition = drx::getEffectDefinition();
    static UnityAudioEffectDefinition* definitions[] { &definition };
    *definitionsPtr = definitions;

    return 1;
}

//==============================================================================
static drx::ModifierKeys unityModifiersToDRX (UnityEventModifiers mods, b8 mouseDown, i32 mouseButton = -1)
{
    i32 flags = 0;

    if (mouseDown)
    {
        if (mouseButton == 0)
            flags |= drx::ModifierKeys::leftButtonModifier;
        else if (mouseButton == 1)
            flags |= drx::ModifierKeys::rightButtonModifier;
        else if (mouseButton == 2)
            flags |= drx::ModifierKeys::middleButtonModifier;
    }

    if (mods == 0)
        return flags;

    if ((mods & UnityEventModifiers::shift) != 0)        flags |= drx::ModifierKeys::shiftModifier;
    if ((mods & UnityEventModifiers::control) != 0)      flags |= drx::ModifierKeys::ctrlModifier;
    if ((mods & UnityEventModifiers::alt) != 0)          flags |= drx::ModifierKeys::altModifier;
    if ((mods & UnityEventModifiers::command) != 0)      flags |= drx::ModifierKeys::commandModifier;

    return { flags };
}

//==============================================================================
static drx::AudioProcessorUnityWrapper* getWrapperChecked (i32 id)
{
    auto* wrapper = drx::getWrapperMap()[id];
    jassert (wrapper != nullptr);

    return wrapper;
}

//==============================================================================
static z0 UNITY_INTERFACE_API onRenderEvent (i32 id)
{
    getWrapperChecked (id)->getEditorPeer().triggerAsyncUpdate();
}

UNITY_INTERFACE_EXPORT renderCallback UNITY_INTERFACE_API getRenderCallback()
{
    return onRenderEvent;
}

UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityInitialiseTexture (i32 id, uk data, i32 w, i32 h)
{
    getWrapperChecked (id)->getEditorPeer().setPixelDataHandle (reinterpret_cast<drx::u8*> (data), w, h);
}

UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityMouseDown (i32 id, f32 x, f32 y, UnityEventModifiers unityMods, i32 button)
{
    getWrapperChecked (id)->getEditorPeer().forwardMouseEvent ({ x, y }, unityModifiersToDRX (unityMods, true, button));
}

UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityMouseDrag (i32 id, f32 x, f32 y, UnityEventModifiers unityMods, i32 button)
{
    getWrapperChecked (id)->getEditorPeer().forwardMouseEvent ({ x, y }, unityModifiersToDRX (unityMods, true, button));
}

UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityMouseUp (i32 id, f32 x, f32 y, UnityEventModifiers unityMods)
{
    getWrapperChecked (id)->getEditorPeer().forwardMouseEvent ({ x, y }, unityModifiersToDRX (unityMods, false));
}

UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unityKeyEvent (i32 id, i32 code, UnityEventModifiers mods, tukk name)
{
    getWrapperChecked (id)->getEditorPeer().forwardKeyPress (code, name, unityModifiersToDRX (mods, false));
}

UNITY_INTERFACE_EXPORT z0 UNITY_INTERFACE_API unitySetScreenBounds (i32 id, f32 x, f32 y, f32 w, f32 h)
{
    getWrapperChecked (id)->getEditorPeer().getEditor().setBounds ({ (i32) x, (i32) y, (i32) w, (i32) h });
}

//==============================================================================
#if DRX_WINDOWS
 DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes")

 extern "C" BOOL WINAPI DllMain (HINSTANCE instance, DWORD reason, LPVOID)
 {
     if (reason == DLL_PROCESS_ATTACH)
         drx::Process::setCurrentModuleInstanceHandle (instance);

     return true;
 }

 DRX_END_IGNORE_WARNINGS_GCC_LIKE
#endif

#endif
