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

#if DRX_PLUGINHOST_AU && (DRX_MAC || DRX_IOS)

#if DRX_MAC
#include <AudioUnit/AUCocoaUIView.h>
#include <CoreAudioKit/AUGenericView.h>
#include <AudioToolbox/AudioUnitUtilities.h>

#if DRX_PLUGINHOST_ARA
 #include <ARA_API/ARAAudioUnit.h>
#endif

#endif

#include <CoreMIDI/MIDIServices.h>

#include <CoreAudioKit/AUViewController.h>

#include <drx_audio_basics/native/drx_CoreAudioTimeConversions_mac.h>
#include <drx_audio_basics/native/drx_CoreAudioLayouts_mac.h>
#include "drx_AU_Shared.h"

namespace drx
{

// Change this to disable logging of various activities
#ifndef AU_LOGGING
 #define AU_LOGGING 1
#endif

#if AU_LOGGING
 #define DRX_AU_LOG(a) Logger::writeToLog(a);
#else
 #define DRX_AU_LOG(a)
#endif

namespace AudioUnitFormatHelpers
{
   #if DRX_DEBUG
    static ThreadLocalValue<i32> insideCallback;
   #endif

    static Txt osTypeToString (OSType type) noexcept
    {
        const t32 s[4] = { (t32) ((type >> 24) & 0xff),
                                  (t32) ((type >> 16) & 0xff),
                                  (t32) ((type >> 8) & 0xff),
                                  (t32) (type & 0xff) };
        return Txt (s, 4);
    }

    static OSType stringToOSType (Txt s)
    {
        if (s.trim().length() >= 4) // (to avoid trimming leading spaces)
            s = s.trim();

        s += "    ";

        return (((OSType) (u8) s[0]) << 24)
             | (((OSType) (u8) s[1]) << 16)
             | (((OSType) (u8) s[2]) << 8)
             |  ((OSType) (u8) s[3]);
    }

    static tukk auIdentifierPrefix = "AudioUnit:";

    static Txt createPluginIdentifier (const AudioComponentDescription& desc)
    {
        Txt s (auIdentifierPrefix);

        if (desc.componentType == kAudioUnitType_MusicDevice)
            s << "Synths/";
        else if (desc.componentType == kAudioUnitType_MusicEffect
                  || desc.componentType == kAudioUnitType_Effect)
            s << "Effects/";
        else if (desc.componentType == kAudioUnitType_Generator)
            s << "Generators/";
        else if (desc.componentType == kAudioUnitType_Panner)
            s << "Panners/";
        else if (desc.componentType == kAudioUnitType_Mixer)
            s << "Mixers/";
        else if (desc.componentType == kAudioUnitType_MIDIProcessor)
            s << "MidiEffects/";

        s << osTypeToString (desc.componentType) << ","
          << osTypeToString (desc.componentSubType) << ","
          << osTypeToString (desc.componentManufacturer);

        return s;
    }

    static z0 getNameAndManufacturer (AudioComponent comp, Txt& name, Txt& manufacturer)
    {
        CFObjectHolder<CFStringRef> cfName;

        if (AudioComponentCopyName (comp, &cfName.object) == noErr)
            name = Txt::fromCFString (cfName.object);

        if (name.containsChar (':'))
        {
            manufacturer = name.upToFirstOccurrenceOf (":", false, false).trim();
            name         = name.fromFirstOccurrenceOf (":", false, false).trim();
        }

        if (name.isEmpty())
            name = "<Unknown>";
    }

    static b8 getComponentDescFromIdentifier (const Txt& fileOrIdentifier, AudioComponentDescription& desc,
                                                Txt& name, Txt& version, Txt& manufacturer)
    {
        if (fileOrIdentifier.startsWithIgnoreCase (auIdentifierPrefix))
        {
            Txt s (fileOrIdentifier.substring (jmax (fileOrIdentifier.lastIndexOfChar (':'),
                                                        fileOrIdentifier.lastIndexOfChar ('/')) + 1));

            StringArray tokens;
            tokens.addTokens (s, ",", StringRef());
            tokens.removeEmptyStrings();

            if (tokens.size() == 3)
            {
                zerostruct (desc);
                desc.componentType         = stringToOSType (tokens[0]);
                desc.componentSubType      = stringToOSType (tokens[1]);
                desc.componentManufacturer = stringToOSType (tokens[2]);

                if (AudioComponent comp = AudioComponentFindNext (nullptr, &desc))
                {
                    getNameAndManufacturer (comp, name, manufacturer);

                    if (manufacturer.isEmpty())
                        manufacturer = tokens[2];

                    if (version.isEmpty())
                    {
                        UInt32 versionNum;

                        if (AudioComponentGetVersion (comp, &versionNum) == noErr)
                        {
                            version << (i32) (versionNum >> 16) << "."
                                    << (i32) ((versionNum >> 8) & 0xff) << "."
                                    << (i32) (versionNum & 0xff);
                        }
                    }

                    return true;
                }
            }
        }

        return false;
    }

    static b8 getComponentDescFromFile ([[maybe_unused]] const Txt& fileOrIdentifier,
                                          [[maybe_unused]] AudioComponentDescription& desc,
                                          [[maybe_unused]] Txt& name,
                                          [[maybe_unused]] Txt& version,
                                          [[maybe_unused]] Txt& manufacturer)
    {
        zerostruct (desc);

       #if DRX_IOS
        return false;
       #else
        const File file (fileOrIdentifier);
        if (! file.hasFileExtension (".component") && ! file.hasFileExtension (".appex"))
            return false;

        tukk const utf8 = fileOrIdentifier.toUTF8();

        if (auto url = CFUniquePtr<CFURLRef> (CFURLCreateFromFileSystemRepresentation (nullptr, (const UInt8*) utf8,
                                                                                       (CFIndex) strlen (utf8), file.isDirectory())))
        {
            if (auto bundleRef = CFUniquePtr<CFBundleRef> (CFBundleCreate (kCFAllocatorDefault, url.get())))
            {
                CFTypeRef bundleName = CFBundleGetValueForInfoDictionaryKey (bundleRef.get(), CFSTR ("CFBundleName"));

                if (bundleName != nullptr && CFGetTypeID (bundleName) == CFStringGetTypeID())
                    name = Txt::fromCFString ((CFStringRef) bundleName);

                if (name.isEmpty())
                    name = file.getFileNameWithoutExtension();

                CFTypeRef versionString = CFBundleGetValueForInfoDictionaryKey (bundleRef.get(), CFSTR ("CFBundleVersion"));

                if (versionString != nullptr && CFGetTypeID (versionString) == CFStringGetTypeID())
                    version = Txt::fromCFString ((CFStringRef) versionString);

                CFTypeRef manuString = CFBundleGetValueForInfoDictionaryKey (bundleRef.get(), CFSTR ("CFBundleGetInfoString"));

                if (manuString != nullptr && CFGetTypeID (manuString) == CFStringGetTypeID())
                    manufacturer = Txt::fromCFString ((CFStringRef) manuString);

                NSBundle* bundle = [[NSBundle alloc] initWithPath: (NSString*) fileOrIdentifier.toCFString()];

                NSArray* audioComponents = [bundle objectForInfoDictionaryKey: @"AudioComponents"];
                NSDictionary* dict = [audioComponents objectAtIndex: 0];

                desc.componentManufacturer = stringToOSType (nsStringToDrx ((NSString*) [dict valueForKey: @"manufacturer"]));
                desc.componentType         = stringToOSType (nsStringToDrx ((NSString*) [dict valueForKey: @"type"]));
                desc.componentSubType      = stringToOSType (nsStringToDrx ((NSString*) [dict valueForKey: @"subtype"]));

                [bundle release];
            }
        }

        return desc.componentType != 0 && desc.componentSubType != 0;
       #endif
    }

    static tukk getCategory (OSType type) noexcept
    {
        switch (type)
        {
            case kAudioUnitType_Effect:
            case kAudioUnitType_MusicEffect:    return "Effect";
            case kAudioUnitType_MusicDevice:    return "Synth";
            case kAudioUnitType_Generator:      return "Generator";
            case kAudioUnitType_Panner:         return "Panner";
            case kAudioUnitType_Mixer:          return "Mixer";
            case kAudioUnitType_MIDIProcessor:  return "MidiEffects";
            default: break;
        }

        return nullptr;
    }

    // Audio Unit plugins expect hosts to listen to their view bounds, and to resize
    // the plugin window/view appropriately.
  #if DRX_MAC || DRX_IOS
   #if DRX_IOS
    #define DRX_IOS_MAC_VIEW  UIView
    using ViewComponentBaseClass = UIViewComponent;
   #else
    #define DRX_IOS_MAC_VIEW  NSView
    using ViewComponentBaseClass = NSViewComponent;
   #endif

    struct AutoResizingNSViewComponent final : public ViewComponentBaseClass,
                                               private AsyncUpdater
    {
        z0 childBoundsChanged (Component*) override  { triggerAsyncUpdate(); }
        z0 handleAsyncUpdate() override              { resizeToFitView(); }
    };
  #endif

    template <typename PropertyType>
    static std::optional<PropertyType> tryGetProperty (AudioUnit inUnit,
                                                       AudioUnitPropertyID inID,
                                                       AudioUnitScope inScope,
                                                       AudioUnitElement inElement)
    {
        PropertyType property{};
        auto size = (UInt32) sizeof (property);

        if (AudioUnitGetProperty (inUnit, inID, inScope, inElement, &property, &size) != noErr)
            return {};

        // This may lead to a memory corruption as the plugin has written more data than was provided!
        jassert (size <= sizeof (PropertyType));

        return property;
    }

    template <typename Type>
    class PropertyData
    {
    public:
        explicit PropertyData (MemoryBlock&& inData) : data (std::move (inData)) {}

              Type& get()       { return *static_cast<      Type*> (data.getData()); }
        const Type& get() const { return *static_cast<const Type*> (data.getData()); }
        UInt32 size() const { return (UInt32) data.getSize(); }

    private:
        MemoryBlock data;
    };

    template <typename Type>
    static std::optional<PropertyData<Type>> tryGetPropertyData (AudioUnit inUnit,
                                                                 AudioUnitPropertyID inID,
                                                                 AudioUnitScope inScope,
                                                                 AudioUnitElement inElement)
    {
        UInt32 size{};

        if (AudioUnitGetPropertyInfo (inUnit, inID, inScope, inElement, &size, nullptr) != noErr)
            return {};

        constexpr auto minimumSize = sizeof (Type);
        size = jmax ((UInt32) minimumSize, size);

        MemoryBlock data;
        data.setSize (size, true);

        if (AudioUnitGetProperty (inUnit, inID, inScope, inElement, data.getData(), &size) != noErr)
            return {};

        // This may lead to a memory corruption as the plugin has written more data than was provided!
        jassert (size <= data.getSize());

        return PropertyData<Type> { std::move (data) };
    }

    template <typename Type>
    static b8 trySetProperty (AudioUnit inUnit,
                                AudioUnitPropertyID inID,
                                AudioUnitScope inScope,
                                AudioUnitElement inElement,
                                const Type& newValue)
    {
        return AudioUnitSetProperty (inUnit, inID, inScope, inElement, &newValue, sizeof (Type));
    }

    template <typename Type>
    static b8 trySetProperty (AudioUnit inUnit,
                                AudioUnitPropertyID inID,
                                AudioUnitScope inScope,
                                AudioUnitElement inElement,
                                const PropertyData<Type>& newValue)
    {
        return AudioUnitSetProperty (inUnit, inID, inScope, inElement, newValue.get(), newValue.size());
    }

    static UInt32 getElementCount (AudioUnit comp, AudioUnitScope scope) noexcept
    {
        const auto count = tryGetProperty<UInt32> (comp, kAudioUnitProperty_ElementCount, scope, 0);
        jassert (count);
        return *count;
    }

    /*  The plugin may expect its channels in a particular order, reported to the host
        using kAudioUnitProperty_AudioChannelLayout.
        This remapper allows us to respect the channel order requested by the plugin,
        while still using the DRX channel ordering for the AudioBuffer argument
        of AudioProcessor::processBlock.
    */
    class SingleDirectionChannelMapping
    {
    public:
        z0 setUpMapping (AudioUnit comp, b8 isInput)
        {
            channels.clear();
            busOffset.clear();

            const auto scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
            const auto n = getElementCount (comp, scope);

            for (UInt32 busIndex = 0; busIndex < n; ++busIndex)
            {
                std::vector<size_t> busMap;

                if (const auto layout = tryGetPropertyData<AudioChannelLayout> (comp, kAudioUnitProperty_AudioChannelLayout, scope, busIndex))
                {
                    const auto juceChannelOrder = CoreAudioLayouts::fromCoreAudio (layout->get());
                    const auto auChannelOrder   = CoreAudioLayouts::getCoreAudioLayoutChannels (layout->get());

                    for (auto juceChannelIndex = 0; juceChannelIndex < juceChannelOrder.size(); ++juceChannelIndex)
                        busMap.push_back ((size_t) auChannelOrder.indexOf (juceChannelOrder.getTypeOfChannel (juceChannelIndex)));
                }

                busOffset.push_back (busMap.empty() ? unknownChannelCount : channels.size());
                channels.insert (channels.end(), busMap.begin(), busMap.end());
            }
        }

        size_t getAuIndexForDrxChannel (size_t bus, size_t channel) const noexcept
        {
            const auto baseOffset = busOffset[bus];
            return baseOffset != unknownChannelCount ? channels[baseOffset + channel]
                                                     : channel;
        }

    private:
        static constexpr size_t unknownChannelCount = std::numeric_limits<size_t>::max();

        /*  The index (in the channels vector) of the first channel in each bus.
            e.g the index of the first channel in the second bus can be found at busOffset[1].
            It's possible for a bus not to report its channel layout, and in this case a value
            of unknownChannelCount will be stored for that bus.
        */
        std::vector<size_t> busOffset;

        /*  The index in a collection of DRX channels of the AU channel with a matching channel
            type. The mappings for all buses are stored in bus order. To find the start offset for a
            particular bus, use the busOffset vector.
            e.g. the index of the AU channel with the same type as the fifth channel of the third bus
            in DRX layout is found at channels[busOffset[2] + 4].
            If the busOffset for the bus is unknownChannelCount, then assume there is no mapping
            between DRX/AU channel layouts.
        */
        std::vector<size_t> channels;
    };

    static b8 isPluginAUv3 (const AudioComponentDescription& desc)
    {
        return (desc.componentFlags & kAudioComponentFlag_IsV3AudioUnit) != 0;
    }
}

static b8 hasARAExtension ([[maybe_unused]] AudioUnit audioUnit)
{
   #if DRX_PLUGINHOST_ARA
    UInt32 propertySize = sizeof (ARA::ARAAudioUnitFactory);
    Boolean isWriteable = FALSE;

    OSStatus status = AudioUnitGetPropertyInfo (audioUnit,
                                                ARA::kAudioUnitProperty_ARAFactory,
                                                kAudioUnitScope_Global,
                                                0,
                                                &propertySize,
                                                &isWriteable);

    if ((status == noErr) && (propertySize == sizeof (ARA::ARAAudioUnitFactory)) && ! isWriteable)
        return true;
   #endif

    return false;
}

struct AudioUnitDeleter
{
    z0 operator() (AudioUnit au) const { AudioComponentInstanceDispose (au); }
};

using AudioUnitUniquePtr = std::unique_ptr<std::remove_pointer_t<AudioUnit>, AudioUnitDeleter>;
using AudioUnitSharedPtr = std::shared_ptr<std::remove_pointer_t<AudioUnit>>;
using AudioUnitWeakPtr = std::weak_ptr<std::remove_pointer_t<AudioUnit>>;

static std::shared_ptr<const ARA::ARAFactory> getARAFactory ([[maybe_unused]] AudioUnitSharedPtr audioUnit)
{
   #if DRX_PLUGINHOST_ARA
    jassert (audioUnit != nullptr);

    UInt32 propertySize = sizeof (ARA::ARAAudioUnitFactory);
    ARA::ARAAudioUnitFactory audioUnitFactory { ARA::kARAAudioUnitMagic, nullptr };

    if (hasARAExtension (audioUnit.get()))
    {
        OSStatus status = AudioUnitGetProperty (audioUnit.get(),
                                                ARA::kAudioUnitProperty_ARAFactory,
                                                kAudioUnitScope_Global,
                                                0,
                                                &audioUnitFactory,
                                                &propertySize);

        if ((status == noErr)
            && (propertySize == sizeof (ARA::ARAAudioUnitFactory))
            && (audioUnitFactory.inOutMagicNumber == ARA::kARAAudioUnitMagic))
        {
            jassert (audioUnitFactory.outFactory != nullptr);
            return getOrCreateARAFactory (audioUnitFactory.outFactory,
                                          [owningAuPtr = std::move (audioUnit)]() {});
        }
    }
   #endif

    return {};
}

struct VersionedAudioComponent
{
    AudioComponent audioComponent = nullptr;
    b8 isAUv3 = false;

    b8 operator< (const VersionedAudioComponent& other) const { return audioComponent < other.audioComponent; }
};

using AudioUnitCreationCallback = std::function<z0 (AudioUnit, OSStatus)>;

static z0 createAudioUnit (VersionedAudioComponent versionedComponent, AudioUnitCreationCallback callback)
{
    if (versionedComponent.isAUv3)
    {
        AudioComponentInstantiate (versionedComponent.audioComponent,
                                   kAudioComponentInstantiation_LoadOutOfProcess,
                                   ^(AudioComponentInstance audioUnit, OSStatus err)
                                   {
                                       callback (audioUnit, err);
                                   });

        return;
    }

    AudioComponentInstance audioUnit;
    auto err = AudioComponentInstanceNew (versionedComponent.audioComponent, &audioUnit);
    callback (err != noErr ? nullptr : audioUnit, err);
}

struct AudioComponentResult
{
    explicit AudioComponentResult (Txt error) : errorMessage (std::move (error)) {}

    explicit AudioComponentResult (VersionedAudioComponent auComponent) : component (std::move (auComponent)) {}

    b8 isValid() const { return component.audioComponent != nullptr; }

    VersionedAudioComponent component;
    Txt errorMessage;
};

static AudioComponentResult getAudioComponent (AudioUnitPluginFormat& format, const PluginDescription& desc)
{
    using namespace AudioUnitFormatHelpers;

    AudioUnitPluginFormat audioUnitPluginFormat;

    if (! format.fileMightContainThisPluginType (desc.fileOrIdentifier))
        return AudioComponentResult { NEEDS_TRANS ("Plug-in description is not an AudioUnit plug-in") };

    Txt pluginName, version, manufacturer;
    AudioComponentDescription componentDesc;
    AudioComponent auComponent;
    Txt errMessage = NEEDS_TRANS ("Cannot find AudioUnit from description");

    if (! getComponentDescFromIdentifier (desc.fileOrIdentifier, componentDesc, pluginName, version, manufacturer)
        && ! getComponentDescFromFile (desc.fileOrIdentifier, componentDesc, pluginName, version, manufacturer))
    {
        return AudioComponentResult { errMessage };
    }

    if ((auComponent = AudioComponentFindNext (nullptr, &componentDesc)) == nullptr)
    {
        return AudioComponentResult { errMessage };
    }

    if (AudioComponentGetDescription (auComponent, &componentDesc) != noErr)
    {
        return AudioComponentResult { errMessage };
    }

    const b8 isAUv3 = AudioUnitFormatHelpers::isPluginAUv3 (componentDesc);

    return AudioComponentResult { { auComponent, isAUv3 } };
}

static z0 getOrCreateARAAudioUnit (VersionedAudioComponent auComponent, std::function<z0 (AudioUnitSharedPtr)> callback)
{
    static std::map<VersionedAudioComponent, AudioUnitWeakPtr> audioUnitARACache;

    if (auto audioUnit = audioUnitARACache[auComponent].lock())
    {
        callback (std::move (audioUnit));
        return;
    }

    createAudioUnit (auComponent, [auComponent, cb = std::move (callback)] (AudioUnit audioUnit, OSStatus err)
    {
        cb ([auComponent, audioUnit, err]() -> AudioUnitSharedPtr
            {
                if (err != noErr)
                    return nullptr;

                AudioUnitSharedPtr auPtr { AudioUnitUniquePtr { audioUnit } };
                audioUnitARACache[auComponent] = auPtr;
                return auPtr;
            }());
    });
}

//==============================================================================
class AudioUnitPluginWindowCocoa;

//==============================================================================
class AudioUnitPluginInstance final    : public AudioPluginInstance
{
public:
    struct AUInstanceParameter final  : public Parameter
    {
        AUInstanceParameter (AudioUnitPluginInstance& parent,
                             UInt32 parameterID,
                             const Txt& parameterName,
                             AudioUnitParameterValue minParameterValue,
                             AudioUnitParameterValue maxParameterValue,
                             AudioUnitParameterValue defaultParameterValue,
                             b8 parameterIsAutomatable,
                             b8 parameterIsDiscrete,
                             i32 numParameterSteps,
                             b8 isBoolean,
                             const Txt& label,
                             b8 parameterValuesHaveStrings)
            : pluginInstance (parent),
              paramID (parameterID),
              name (parameterName),
              minValue (minParameterValue),
              maxValue (maxParameterValue),
              range (maxValue - minValue),
              automatable (parameterIsAutomatable),
              discrete (parameterIsDiscrete),
              numSteps (numParameterSteps),
              valuesHaveStrings (parameterValuesHaveStrings),
              isSwitch (isBoolean),
              valueLabel (label),
              defaultValue (normaliseParamValue (defaultParameterValue))
        {
            auValueStrings = Parameter::getAllValueStrings();
        }

        f32 getValue() const override
        {
            const ScopedLock sl (pluginInstance.lock);

            AudioUnitParameterValue value = 0;

            if (auto* au = pluginInstance.audioUnit)
            {
                AudioUnitGetParameter (au, paramID, kAudioUnitScope_Global, 0, &value);
                value = normaliseParamValue (value);
            }

            return value;
        }

        z0 setValue (f32 newValue) override
        {
            const ScopedLock sl (pluginInstance.lock);

            if (auto* au = pluginInstance.audioUnit)
            {
                AudioUnitSetParameter (au, paramID, kAudioUnitScope_Global,
                                       0, scaleParamValue (newValue), 0);

                sendParameterChangeEvent();
            }
        }

        f32 getDefaultValue() const override
        {
            return defaultValue;
        }

        Txt getName (i32 /*maximumStringLength*/) const override { return name; }
        Txt getLabel() const override { return valueLabel; }

        Txt getText (f32 value, i32 maximumLength) const override
        {
            if (! auValueStrings.isEmpty())
            {
                auto index = roundToInt (jlimit (0.0f, 1.0f, value) * (f32) (auValueStrings.size() - 1));
                return auValueStrings[index];
            }

            auto scaledValue = scaleParamValue (value);

            if (valuesHaveStrings)
            {
                if (auto* au = pluginInstance.audioUnit)
                {
                    AudioUnitParameterStringFromValue stringValue;
                    stringValue.inParamID = paramID;
                    stringValue.inValue = &scaledValue;
                    stringValue.outString = nullptr;

                    UInt32 propertySize = sizeof (stringValue);

                    auto err = AudioUnitGetProperty (au,
                                                     kAudioUnitProperty_ParameterStringFromValue,
                                                     kAudioUnitScope_Global,
                                                     0,
                                                     &stringValue,
                                                     &propertySize);

                    if (! err && stringValue.outString != nullptr)
                    {
                        const CFUniquePtr<CFStringRef> ownedString { stringValue.outString };
                        return Txt::fromCFString (stringValue.outString).substring (0, maximumLength);
                    }
                }
            }

            return Parameter::getText (scaledValue, maximumLength);
        }

        f32 getValueForText (const Txt& text) const override
        {
            if (! auValueStrings.isEmpty())
            {
                auto index = auValueStrings.indexOf (text);

                if (index != -1)
                    return ((f32) index) / (f32) (auValueStrings.size() - 1);
            }

            if (valuesHaveStrings)
            {
                if (auto* au = pluginInstance.audioUnit)
                {
                    AudioUnitParameterValueFromString pvfs;
                    pvfs.inParamID = paramID;
                    CFUniquePtr<CFStringRef> cfString (text.toCFString());
                    pvfs.inString = cfString.get();
                    UInt32 propertySize = sizeof (pvfs);

                    auto err = AudioUnitGetProperty (au,
                                                     kAudioUnitProperty_ParameterValueFromString,
                                                     kAudioUnitScope_Global,
                                                     0,
                                                     &pvfs,
                                                     &propertySize);

                    if (! err)
                        return normaliseParamValue (pvfs.outValue);
                }
            }

            return Parameter::getValueForText (text);
        }

        b8 isAutomatable() const override         { return automatable; }
        b8 isDiscrete() const override            { return discrete; }
        b8 isBoolean() const override             { return isSwitch; }
        i32 getNumSteps() const override            { return numSteps; }

        StringArray getAllValueStrings() const override
        {
            return auValueStrings;
        }

        Txt getParameterID() const override
        {
            return Txt (paramID);
        }

        z0 sendParameterChangeEvent()
        {
           #if DRX_MAC
            jassert (pluginInstance.audioUnit != nullptr);

            AudioUnitEvent ev;
            ev.mEventType                        = kAudioUnitEvent_ParameterValueChange;
            ev.mArgument.mParameter.mAudioUnit   = pluginInstance.audioUnit;
            ev.mArgument.mParameter.mParameterID = paramID;
            ev.mArgument.mParameter.mScope       = kAudioUnitScope_Global;
            ev.mArgument.mParameter.mElement     = 0;

            AUEventListenerNotify (pluginInstance.eventListenerRef, nullptr, &ev);
           #endif
        }

        f32 normaliseParamValue (f32 scaledValue) const noexcept
        {
            return (scaledValue - minValue) / range;
        }

        f32 scaleParamValue (f32 normalisedValue) const noexcept
        {
            return minValue + (range * normalisedValue);
        }

        UInt32 getRawParamID() const { return paramID; }

        z0 setName  (Txt&& newName)  { name       = std::move (newName); }
        z0 setLabel (Txt&& newLabel) { valueLabel = std::move (newLabel); }

    private:
        AudioUnitPluginInstance& pluginInstance;
        const UInt32 paramID;
        Txt name;
        const AudioUnitParameterValue minValue, maxValue, range;
        const b8 automatable, discrete;
        i32k numSteps;
        const b8 valuesHaveStrings, isSwitch;
        Txt valueLabel;
        const AudioUnitParameterValue defaultValue;
        StringArray auValueStrings;
    };

    AudioUnitPluginInstance (AudioComponentInstance au)
        : AudioPluginInstance (getBusesProperties (au)),
          auComponent (AudioComponentInstanceGetComponent (au)),
          audioUnit (au),
        #if DRX_MAC
          eventListenerRef (nullptr),
        #endif
          midiConcatenator (2048)
    {
        using namespace AudioUnitFormatHelpers;

        AudioComponentGetDescription (auComponent, &componentDesc);

        isAUv3 = AudioUnitFormatHelpers::isPluginAUv3 (componentDesc);

        wantsMidiMessages = componentDesc.componentType == kAudioUnitType_MusicDevice
                         || componentDesc.componentType == kAudioUnitType_MusicEffect
                         || componentDesc.componentType == kAudioUnitType_MIDIProcessor;

        isMidiEffectPlugin = (componentDesc.componentType == kAudioUnitType_MIDIProcessor);
        AudioComponentDescription ignore;
        getComponentDescFromIdentifier (createPluginIdentifier (componentDesc), ignore, pluginName, version, manufacturer);
        updateSupportedLayouts();
    }

    ~AudioUnitPluginInstance() override
    {
        const ScopedLock sl (lock);

       #if DRX_DEBUG
        // this indicates that some kind of recursive call is getting triggered that's
        // deleting this plugin while it's still under construction.
        jassert (AudioUnitFormatHelpers::insideCallback.get() == 0);
       #endif

        if (audioUnit != nullptr)
        {
            struct AUDeleter final : public CallbackMessage
            {
                AUDeleter (AudioUnitPluginInstance& inInstance, WaitableEvent& inEvent)
                    : auInstance (inInstance), completionSignal (inEvent)
                {}

                z0 messageCallback() override
                {
                    auInstance.cleanup();
                    completionSignal.signal();
                }

                AudioUnitPluginInstance& auInstance;
                WaitableEvent& completionSignal;
            };

            if (MessageManager::getInstance()->isThisTheMessageThread())
            {
                cleanup();
            }
            else
            {
                WaitableEvent completionEvent;
                (new AUDeleter (*this, completionEvent))->post();
                completionEvent.wait();
            }
        }
    }

    // called from the destructor above
    z0 cleanup()
    {
       #if DRX_MAC
        disposeEventListener();
       #endif

        if (prepared)
            releaseResources();

        AudioComponentInstanceDispose (audioUnit);
        audioUnit = nullptr;
    }

    b8 initialise (f64 rate, i32 blockSize)
    {
        producesMidiMessages = canProduceMidiOutput();
        setRateAndBufferSizeDetails (rate, blockSize);
        setLatencySamples (0);
        refreshParameterList();
        setPluginCallbacks();

       #if DRX_MAC
        createEventListener();
       #endif

        return true;
    }

    //==============================================================================
    b8 canAddBus (b8 isInput)    const override                   { return isBusCountWritable (isInput); }
    b8 canRemoveBus (b8 isInput) const override                   { return isBusCountWritable (isInput); }

    b8 canApplyBusCountChange (b8 isInput, b8 isAdding, BusProperties& outProperties) override
    {
        auto currentCount = (UInt32) getBusCount (isInput);
        auto newCount = (UInt32) ((i32) currentCount + (isAdding ? 1 : -1));
        AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;

        if (AudioUnitSetProperty (audioUnit, kAudioUnitProperty_ElementCount, scope, 0, &newCount, sizeof (newCount)) == noErr)
        {
            getBusProperties (isInput, currentCount, outProperties.busName, outProperties.defaultLayout);
            outProperties.isActivatedByDefault = true;
            updateSupportedLayouts();

            return true;
        }

        return false;
    }

    //==============================================================================
    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        if (layouts == getBusesLayout())
            return true;

        for (i32 dir = 0; dir < 2; ++dir)
        {
            const b8 isInput = (dir == 0);
            auto& requestedLayouts         = (isInput ? layouts.inputBuses  : layouts.outputBuses);
            auto& oppositeRequestedLayouts = (isInput ? layouts.outputBuses : layouts.inputBuses);
            auto& supported                = (isInput ? supportedInLayouts : supportedOutLayouts);
            i32k n = getBusCount (isInput);

            for (i32 busIdx = 0; busIdx < n; ++busIdx)
            {
                auto& requested = requestedLayouts.getReference (busIdx);
                i32k oppositeBusIdx = jmin (getBusCount (! isInput) - 1, busIdx);
                const b8 hasOppositeBus = (oppositeBusIdx >= 0);
                auto oppositeRequested = (hasOppositeBus ? oppositeRequestedLayouts.getReference (oppositeBusIdx) : AudioChannelSet());
                auto& possible = supported.getReference (busIdx);

                if (requested.isDisabled())
                    return false;

                if (possible.size() > 0 && ! possible.contains (requested))
                    return false;

                i32 i;
                for (i = 0; i < numChannelInfos; ++i)
                {
                    auto& info = channelInfos[i];
                    auto& thisChannels = (isInput ? info.inChannels  : info.outChannels);
                    auto& opChannels   = (isInput ? info.outChannels : info.inChannels);

                    // this bus
                    if      (thisChannels == 0) continue;
                    else if (thisChannels >  0 && requested.size() != thisChannels)       continue;
                    else if (thisChannels < -2 && requested.size() > (thisChannels * -1)) continue;

                    // opposite bus
                    if      (opChannels == 0 && hasOppositeBus) continue;
                    else if (opChannels >  0 && oppositeRequested.size() != opChannels) continue;
                    else if (opChannels < -2 && oppositeRequested.size() > (opChannels * -1)) continue;

                    // both buses
                    if (thisChannels == -2 && opChannels == -2) continue;
                    if (thisChannels == -1 && opChannels == -1)
                    {
                        i32 numOppositeBuses = getBusCount (! isInput);
                        i32 j;

                        for (j = 0; j < numOppositeBuses; ++j)
                            if (requested.size() != oppositeRequestedLayouts.getReference (j).size())
                                break;

                        if (j < numOppositeBuses)
                            continue;
                    }

                    break;
                }

                if (i >= numChannelInfos)
                    return false;
            }
        }

        return true;
    }

    b8 syncBusLayouts (const BusesLayout& layouts, b8 isInitialized, b8& layoutHasChanged) const
    {
        layoutHasChanged = false;

        for (const auto scope : { kAudioUnitScope_Input, kAudioUnitScope_Output })
        {
            const auto isInput = scope == kAudioUnitScope_Input;
            const auto busCount = (UInt32) getBusCount (isInput);

            if (AudioUnitFormatHelpers::getElementCount (audioUnit, scope) != busCount && isBusCountWritable (isInput))
            {
                layoutHasChanged = true;

                if (! AudioUnitFormatHelpers::trySetProperty (audioUnit, kAudioUnitProperty_ElementCount, scope, 0, busCount))
                    jassertfalse;
            }

            for (UInt32 busIndex = 0; busIndex < busCount; ++busIndex)
            {
                const auto& set = layouts.getChannelSet (isInput, (i32) busIndex);
                const auto currentStream = AudioUnitFormatHelpers::tryGetProperty<AudioStreamBasicDescription> (audioUnit, kAudioUnitProperty_StreamFormat, scope, busIndex);

                if (! currentStream.has_value())
                    return false;

                const auto actualNumChannels = static_cast<i32> (currentStream->mChannelsPerFrame);
                const auto requestedNumChannels = set.size();

                if (actualNumChannels != requestedNumChannels)
                {
                    const auto sampleRate = AudioUnitFormatHelpers::tryGetProperty<Float64> (audioUnit, kAudioUnitProperty_SampleRate, scope, busIndex);
                    jassert (sampleRate.has_value());

                    layoutHasChanged = true;
                    AudioStreamBasicDescription newStream{};
                    newStream.mSampleRate       = *sampleRate;
                    newStream.mFormatID         = kAudioFormatLinearPCM;
                    newStream.mFormatFlags      = (i32) kAudioFormatFlagsNativeFloatPacked | (i32) kAudioFormatFlagIsNonInterleaved | (i32) kAudioFormatFlagsNativeEndian;
                    newStream.mFramesPerPacket  = 1;
                    newStream.mBytesPerPacket   = 4;
                    newStream.mBytesPerFrame    = 4;
                    newStream.mBitsPerChannel   = 32;
                    newStream.mChannelsPerFrame = static_cast<UInt32> (requestedNumChannels);

                    if (AudioUnitFormatHelpers::trySetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, busIndex, newStream))
                        return false;
                }

                if (isInitialized && ! set.isDiscreteLayout())
                {
                    if (const auto layout = AudioUnitFormatHelpers::tryGetPropertyData<AudioChannelLayout> (audioUnit, kAudioUnitProperty_AudioChannelLayout, scope, busIndex))
                    {
                        // This is likely a fault with the plugin. Not enough data has been allocated
                        // to support the AudioChannelLayout being described
                        jassert (layout->get().mChannelLayoutTag != kAudioChannelLayoutTag_UseChannelDescriptions
                                 || layout->size() >= offsetof (AudioChannelLayout, mChannelDescriptions) + layout->get().mNumberChannelDescriptions * sizeof (AudioChannelDescription));

                        const auto requestedTag = CoreAudioLayouts::toCoreAudio (set);
                        const auto actualTag    = CoreAudioLayouts::toCoreAudio (CoreAudioLayouts::fromCoreAudio (layout->get()));

                        if (actualTag != requestedTag)
                        {
                            AudioChannelLayout newLayout{};
                            newLayout.mChannelLayoutTag = requestedTag;

                            // We'll likely need to fill more data in the newLayout for these tags.
                            // Please notify the DRX team if you hit this assertion and provide clear
                            // instructions regarding the plugin being loaded and any other relevant
                            // details required to hit this assertion. Preferably if you can recreate
                            // the assertion using the AudioPluginHost that would be extremely helpful.

                            jassert (requestedTag != kAudioChannelLayoutTag_UseChannelBitmap
                                  && requestedTag != kAudioChannelLayoutTag_UseChannelDescriptions);

                            if (AudioUnitFormatHelpers::trySetProperty (audioUnit, kAudioUnitProperty_AudioChannelLayout, scope, busIndex, newLayout))
                                return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    b8 canApplyBusesLayout (const BusesLayout& layouts) const override
    {
        // You cannot call setBusesLayout when the AudioProcessor is processing.
        // Call releaseResources first!
        jassert (! prepared);

        b8 layoutHasChanged = false;

        if (! syncBusLayouts (layouts, false, layoutHasChanged))
            return false;

        // did anything actually change
        if (! layoutHasChanged)
            return true;

        // Some plug-ins require the LayoutTag to be set after initialization
        const auto success = (AudioUnitInitialize (audioUnit) == noErr)
                             && syncBusLayouts (layouts, true, layoutHasChanged);

        AudioUnitUninitialize (audioUnit);

        if (! success)
            // make sure that the layout is back to its original state
            syncBusLayouts (getBusesLayout(), false, layoutHasChanged);

        return success;
    }

    //==============================================================================
    // AudioPluginInstance methods:

    z0 fillInPluginDescription (PluginDescription& desc) const override
    {
        desc.name = pluginName;
        desc.descriptiveName = pluginName;
        desc.fileOrIdentifier = AudioUnitFormatHelpers::createPluginIdentifier (componentDesc);
        desc.uniqueId = desc.deprecatedUid = ((i32) componentDesc.componentType)
                                           ^ ((i32) componentDesc.componentSubType)
                                           ^ ((i32) componentDesc.componentManufacturer);
        desc.lastFileModTime = Time();
        desc.lastInfoUpdateTime = Time::getCurrentTime();
        desc.pluginFormatName = "AudioUnit";
        desc.category = AudioUnitFormatHelpers::getCategory (componentDesc.componentType);
        desc.manufacturerName = manufacturer;
        desc.version = version;
        desc.numInputChannels = getTotalNumInputChannels();
        desc.numOutputChannels = getTotalNumOutputChannels();
        desc.isInstrument = (componentDesc.componentType == kAudioUnitType_MusicDevice);

       #if DRX_PLUGINHOST_ARA
        desc.hasARAExtension = [&]
        {
            UInt32 propertySize = sizeof (ARA::ARAAudioUnitFactory);
            Boolean isWriteable = FALSE;

            OSStatus status = AudioUnitGetPropertyInfo (audioUnit,
                                                        ARA::kAudioUnitProperty_ARAFactory,
                                                        kAudioUnitScope_Global,
                                                        0,
                                                        &propertySize,
                                                        &isWriteable);

            return (status == noErr) && (propertySize == sizeof (ARA::ARAAudioUnitFactory)) && ! isWriteable;
        }();
       #endif
    }

    z0 getExtensions (ExtensionsVisitor& visitor) const override
    {
        struct Extensions final : public ExtensionsVisitor::AudioUnitClient
        {
            explicit Extensions (const AudioUnitPluginInstance* instanceIn) : instance (instanceIn) {}

            AudioUnit getAudioUnitHandle() const noexcept override   { return instance->audioUnit; }

            const AudioUnitPluginInstance* instance = nullptr;
        };

        visitor.visitAudioUnitClient (Extensions { this });

       #ifdef DRX_PLUGINHOST_ARA
        struct ARAExtensions final : public ExtensionsVisitor::ARAClient
        {
            explicit ARAExtensions (const AudioUnitPluginInstance* instanceIn) : instance (instanceIn) {}

            z0 createARAFactoryAsync (std::function<z0 (ARAFactoryWrapper)> cb) const override
            {
                getOrCreateARAAudioUnit ({ instance->auComponent, instance->isAUv3 },
                                         [origCb = std::move (cb)] (auto dylibKeepAliveAudioUnit)
                                         {
                                             origCb ([&]() -> ARAFactoryWrapper
                                                     {
                                                         if (dylibKeepAliveAudioUnit != nullptr)
                                                             return ARAFactoryWrapper { ::drx::getARAFactory (std::move (dylibKeepAliveAudioUnit)) };

                                                         return ARAFactoryWrapper { nullptr };
                                                     }());
                                         });
            }

            const AudioUnitPluginInstance* instance = nullptr;
        };

        if (hasARAExtension (audioUnit))
            visitor.visitARAClient (ARAExtensions (this));
       #endif
    }

    uk getPlatformSpecificData() override             { return audioUnit; }
    const Txt getName() const override                { return pluginName; }

    f64 getTailLengthSeconds() const override
    {
        Float64 tail = 0;
        UInt32 tailSize = sizeof (tail);

        if (audioUnit != nullptr)
            AudioUnitGetProperty (audioUnit, kAudioUnitProperty_TailTime, kAudioUnitScope_Global,
                                  0, &tail, &tailSize);

        return tail;
    }

    b8 acceptsMidi() const override                    { return wantsMidiMessages; }
    b8 producesMidi() const override                   { return producesMidiMessages; }

    //==============================================================================
    // AudioProcessor methods:

    z0 prepareToPlay (f64 newSampleRate, i32 estimatedSamplesPerBlock) override
    {
        if (audioUnit != nullptr)
        {
            releaseResources();
            setPluginCallbacks();

            for (i32 dir = 0; dir < 2; ++dir)
            {
                const b8 isInput = (dir == 0);
                const AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
                i32k n = getBusCount (isInput);

                for (i32 i = 0; i < n; ++i)
                {
                    Float64 sampleRate;
                    UInt32 sampleRateSize = sizeof (sampleRate);
                    const Float64 sr = newSampleRate;

                    AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SampleRate, scope, static_cast<UInt32> (i), &sampleRate, &sampleRateSize);

                    if (! approximatelyEqual (sampleRate, sr))
                    {
                        if (isAUv3) // setting kAudioUnitProperty_SampleRate fails on AUv3s
                        {
                            AudioStreamBasicDescription stream;
                            UInt32 dataSize = sizeof (stream);
                            auto err = AudioUnitGetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, static_cast<UInt32> (i), &stream, &dataSize);

                            if (err == noErr && dataSize == sizeof (stream))
                            {
                                stream.mSampleRate = sr;
                                AudioUnitSetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, static_cast<UInt32> (i), &stream, sizeof (stream));
                            }
                        }
                        else
                        {
                            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SampleRate, scope, static_cast<UInt32> (i), &sr, sizeof (sr));
                        }
                    }

                    if (isInput)
                    {
                        AURenderCallbackStruct info;
                        zerostruct (info); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)

                        info.inputProcRefCon = this;
                        info.inputProc = renderGetInputCallback;

                        AudioUnitSetProperty (audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
                                              static_cast<UInt32> (i), &info, sizeof (info));
                    }
                    else
                    {
                        outputBufferList.add (new AUBuffer (static_cast<size_t> (getChannelCountOfBus (false, i))));
                    }
                }
            }

            UInt32 frameSize = (UInt32) estimatedSamplesPerBlock;
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0,
                                  &frameSize, sizeof (frameSize));

            setRateAndBufferSizeDetails ((f64) newSampleRate, estimatedSamplesPerBlock);

            zerostruct (timeStamp);
            timeStamp.mSampleTime = 0;
            timeStamp.mHostTime = mach_absolute_time();
            timeStamp.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;

            wasPlaying = false;

            resetBuses();

            b8 ignore;

            if (! syncBusLayouts (getBusesLayout(), false, ignore))
                return;

            prepared = [&]
            {
                if (AudioUnitInitialize (audioUnit) != noErr)
                    return false;

                if (! haveParameterList)
                    refreshParameterList();

                if (! syncBusLayouts (getBusesLayout(), true, ignore))
                {
                    AudioUnitUninitialize (audioUnit);
                    return false;
                }

                updateLatency();
                return true;
            }();

            inMapping .setUpMapping (audioUnit, true);
            outMapping.setUpMapping (audioUnit, false);

            preparedChannels = jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());
            preparedSamples  = estimatedSamplesPerBlock;
            inputBuffer.setSize (preparedChannels, preparedSamples);
        }
    }

    z0 releaseResources() override
    {
        if (prepared)
        {
            resetBuses();
            AudioUnitReset (audioUnit, kAudioUnitScope_Global, 0);
            AudioUnitUninitialize (audioUnit);

            outputBufferList.clear();
            prepared = false;
        }

        incomingMidi.clear();
    }

    z0 resetBuses()
    {
        for (i32 i = 0; i < getBusCount (true);  ++i)  AudioUnitReset (audioUnit, kAudioUnitScope_Input,  static_cast<UInt32> (i));
        for (i32 i = 0; i < getBusCount (false); ++i)  AudioUnitReset (audioUnit, kAudioUnitScope_Output, static_cast<UInt32> (i));
    }

    z0 processAudio (AudioBuffer<f32>& buffer, MidiBuffer& midiMessages, b8 processBlockBypassedCalled)
    {
        auto* playhead = getPlayHead();
        const auto position = playhead != nullptr ? playhead->getPosition() : nullopt;

        if (const auto hostTimeNs = position.hasValue() ? position->getHostTimeNs() : nullopt)
        {
            timeStamp.mHostTime = *hostTimeNs;
            timeStamp.mFlags |= kAudioTimeStampHostTimeValid;
        }
        else
        {
            timeStamp.mHostTime = 0;
            timeStamp.mFlags &= ~kAudioTimeStampHostTimeValid;
        }

        // If these are hit, we might allocate in the process block!
        jassert (buffer.getNumChannels() <= preparedChannels);
        jassert (buffer.getNumSamples()  <= preparedSamples);
        // Copy the input buffer to guard against the case where a bus has more output channels
        // than input channels, so rendering the output for that bus might stamp over the input
        // to the following bus.
        inputBuffer.makeCopyOf (buffer, true);

        const auto numSamples = buffer.getNumSamples();

        if (auSupportsBypass)
        {
            updateBypass (processBlockBypassedCalled);
        }
        else if (processBlockBypassedCalled)
        {
            AudioProcessor::processBlockBypassed (buffer, midiMessages);
            return;
        }

        if (prepared)
        {
            const auto numOutputBuses = getBusCount (false);

            for (i32 i = 0; i < numOutputBuses; ++i)
            {
                if (AUBuffer* buf = outputBufferList[i])
                {
                    AudioBufferList& abl = *buf;
                    const auto* bus = getBus (false, i);
                    const auto channelCount = bus != nullptr ? bus->getNumberOfChannels() : 0;

                    for (auto juceChannel = 0; juceChannel < channelCount; ++juceChannel)
                    {
                        const auto auChannel = outMapping.getAuIndexForDrxChannel ((size_t) i, (size_t) juceChannel);
                        abl.mBuffers[auChannel].mNumberChannels = 1;
                        abl.mBuffers[auChannel].mDataByteSize = (UInt32) ((size_t) numSamples * sizeof (f32));
                        abl.mBuffers[auChannel].mData = buffer.getWritePointer (bus->getChannelIndexInProcessBlockBuffer (juceChannel));
                    }
                }
            }

            if (wantsMidiMessages)
            {
                for (const auto metadata : midiMessages)
                {
                    if (metadata.numBytes <= 3)
                        MusicDeviceMIDIEvent (audioUnit,
                                              metadata.data[0], metadata.data[1], metadata.data[2],
                                              (UInt32) metadata.samplePosition);
                    else
                        MusicDeviceSysEx (audioUnit, metadata.data, (UInt32) metadata.numBytes);
                }

                midiMessages.clear();
            }

            for (i32 i = 0; i < numOutputBuses; ++i)
            {
                AudioUnitRenderActionFlags flags = 0;

                if (AUBuffer* buf = outputBufferList[i])
                    AudioUnitRender (audioUnit, &flags, &timeStamp, static_cast<UInt32> (i), (UInt32) numSamples, buf->bufferList.get());
            }

            timeStamp.mSampleTime += numSamples;
        }
        else
        {
            // Plugin not working correctly, so just bypass..
            for (i32 i = getTotalNumOutputChannels(); --i >= 0;)
                buffer.clear (i, 0, buffer.getNumSamples());
        }

        if (producesMidiMessages)
        {
            const ScopedLock sl (midiInLock);
            midiMessages.swapWith (incomingMidi);
            incomingMidi.clear();
        }
    }

    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer& midiMessages) override
    {
        processAudio (buffer, midiMessages, false);
    }

    z0 processBlockBypassed (AudioBuffer<f32>& buffer, MidiBuffer& midiMessages) override
    {
        processAudio (buffer, midiMessages, true);
    }

    //==============================================================================
    AudioProcessorParameter* getBypassParameter() const override    { return auSupportsBypass ? bypassParam.get() : nullptr; }

    b8 hasEditor() const override
    {
       #if DRX_MAC
        return true;
       #else
        UInt32 dataSize;
        Boolean isWritable;

        return (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_RequestViewController,
                                          kAudioUnitScope_Global, 0, &dataSize, &isWritable) == noErr
                && dataSize == sizeof (uintptr_t) && isWritable != 0);
       #endif
    }

    AudioProcessorEditor* createEditor() override;

    static AudioProcessor::BusesProperties getBusesProperties (AudioComponentInstance comp)
    {
        AudioProcessor::BusesProperties busProperties;

        for (i32 dir = 0; dir < 2; ++dir)
        {
            const auto isInput = (dir == 0);
            const auto n = AudioUnitFormatHelpers::getElementCount (comp, isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output);

            for (UInt32 i = 0; i < n; ++i)
            {
                Txt busName;
                AudioChannelSet currentLayout;

                getBusProperties (comp, isInput, i, busName, currentLayout);
                jassert (! currentLayout.isDisabled());

                busProperties.addBus (isInput, busName, currentLayout, true);
            }
        }

        return busProperties;
    }

    //==============================================================================
    const Txt getInputChannelName (i32 index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumInputChannels()))
            return "Input " + Txt (index + 1);

        return {};
    }

    const Txt getOutputChannelName (i32 index) const override
    {
        if (isPositiveAndBelow (index, getTotalNumOutputChannels()))
            return "Output " + Txt (index + 1);

        return {};
    }

    b8 isInputChannelStereoPair (i32 index) const override    { return isPositiveAndBelow (index, getTotalNumInputChannels()); }
    b8 isOutputChannelStereoPair (i32 index) const override   { return isPositiveAndBelow (index, getTotalNumOutputChannels()); }

    //==============================================================================
    z0 sendAllParametersChangedEvents()
    {
       #if DRX_MAC
        jassert (audioUnit != nullptr);

        AudioUnitParameter param;
        param.mAudioUnit = audioUnit;
        param.mParameterID = kAUParameterListener_AnyParameter;

        AUParameterListenerNotify (nullptr, nullptr, &param);
       #endif
    }

    //==============================================================================
    class ScopedFactoryPresets
    {
    public:
        ScopedFactoryPresets (AudioUnit& au)
        {
            UInt32 sz = sizeof (CFArrayRef);
            AudioUnitGetProperty (au, kAudioUnitProperty_FactoryPresets,
                                  kAudioUnitScope_Global, 0, &presets.object, &sz);
        }

        CFArrayRef get() const noexcept
        {
            return presets.object;
        }

    private:
        CFObjectHolder<CFArrayRef> presets;
    };

    i32 getNumPrograms() override
    {
        ScopedFactoryPresets factoryPresets { audioUnit };

        if (factoryPresets.get() != nullptr)
            return (i32) CFArrayGetCount (factoryPresets.get());

        return 0;
    }

    i32 getCurrentProgram() override
    {
        AUPreset current{};
        UInt32 sz = sizeof (AUPreset);

        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_PresentPreset,
                              kAudioUnitScope_Global, 0, &current, &sz);

        const CFUniquePtr<CFStringRef> ownedString { current.presetName };
        return current.presetNumber;
    }

    z0 setCurrentProgram (i32 newIndex) override
    {
        ScopedFactoryPresets factoryPresets { audioUnit };

        if (factoryPresets.get() != nullptr
            && newIndex < (i32) CFArrayGetCount (factoryPresets.get()))
        {
            AUPreset current{};
            current.presetNumber = newIndex;

            if (auto* p = static_cast<const AUPreset*> (CFArrayGetValueAtIndex (factoryPresets.get(), newIndex)))
                current.presetName = p->presetName;

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_PresentPreset,
                                  kAudioUnitScope_Global, 0, &current, sizeof (AUPreset));

            sendAllParametersChangedEvents();
        }
    }

    const Txt getProgramName (i32 index) override
    {
        if (index == -1)
        {
            AUPreset current{};
            current.presetNumber = -1;

            UInt32 prstsz = sizeof (AUPreset);

            AudioUnitGetProperty (audioUnit, kAudioUnitProperty_PresentPreset,
                                  kAudioUnitScope_Global, 0, &current, &prstsz);

            const CFUniquePtr<CFStringRef> ownedString { current.presetName };
            return current.presetName != nullptr ? Txt::fromCFString (current.presetName) : Txt{};
        }

        ScopedFactoryPresets factoryPresets { audioUnit };

        if (factoryPresets.get() != nullptr)
        {
            for (CFIndex i = 0; i < CFArrayGetCount (factoryPresets.get()); ++i)
                if (auto* p = static_cast<const AUPreset*> (CFArrayGetValueAtIndex (factoryPresets.get(), i)))
                    if (p->presetNumber == index)
                        return Txt::fromCFString (p->presetName);
        }

        return {};
    }

    z0 changeProgramName (i32 /*index*/, const Txt& /*newName*/) override
    {
        jassertfalse; // xxx not implemented!
    }

    //==============================================================================
    z0 updateTrackProperties (const TrackProperties& properties) override
    {
        if (properties.name.has_value())
        {
            CFObjectHolder<CFStringRef> contextName { properties.name->toCFString() };
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_ContextName, kAudioUnitScope_Global,
                                  0, &contextName.object, sizeof (contextName.object));
        }
    }

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) override
    {
        getCurrentProgramStateInformation (destData);
    }

    z0 getCurrentProgramStateInformation (MemoryBlock& destData) override
    {
        CFObjectHolder<CFPropertyListRef> propertyList;
        UInt32 sz = sizeof (propertyList.object);

        if (AudioUnitGetProperty (audioUnit,
                                  kAudioUnitProperty_ClassInfo,
                                  kAudioUnitScope_Global,
                                  0, &propertyList.object, &sz) == noErr)
        {
            CFUniquePtr<CFWriteStreamRef> stream (CFWriteStreamCreateWithAllocatedBuffers (kCFAllocatorDefault, kCFAllocatorDefault));
            CFWriteStreamOpen (stream.get());

            const auto bytesWritten = CFPropertyListWrite (propertyList.object, stream.get(), kCFPropertyListBinaryFormat_v1_0, 0, nullptr);
            CFWriteStreamClose (stream.get());

            CFUniquePtr<CFDataRef> data ((CFDataRef) CFWriteStreamCopyProperty (stream.get(), kCFStreamPropertyDataWritten));

            destData.setSize ((size_t) bytesWritten);
            destData.copyFrom (CFDataGetBytePtr (data.get()), 0, destData.getSize());
        }
    }

    z0 setStateInformation (ukk data, i32 sizeInBytes) override
    {
        setCurrentProgramStateInformation (data, sizeInBytes);
    }

    z0 setCurrentProgramStateInformation (ukk data, i32 sizeInBytes) override
    {
        CFUniquePtr<CFReadStreamRef> stream (CFReadStreamCreateWithBytesNoCopy (kCFAllocatorDefault, (const UInt8*) data,
                                                                                sizeInBytes, kCFAllocatorNull));
        CFReadStreamOpen (stream.get());

        CFPropertyListFormat format = kCFPropertyListBinaryFormat_v1_0;
        CFObjectHolder<CFPropertyListRef> propertyList { CFPropertyListCreateWithStream (kCFAllocatorDefault,
                                                                                         stream.get(),
                                                                                         0,
                                                                                         kCFPropertyListImmutable,
                                                                                         &format,
                                                                                         nullptr) };

        if (propertyList.object != nullptr)
        {
            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_ClassInfo, kAudioUnitScope_Global,
                                  0, &propertyList.object, sizeof (propertyList.object));

            sendAllParametersChangedEvents();
        }
    }

    z0 refreshParameterList() override
    {
        paramIDToParameter.clear();
        AudioProcessorParameterGroup newParameterTree;

        if (audioUnit != nullptr)
        {
            UInt32 paramListSize = 0;
            auto err = AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                                                 0, &paramListSize, nullptr);

            haveParameterList = (paramListSize > 0 && err == noErr);

            if (! haveParameterList)
                return;

            if (paramListSize > 0)
            {
                const size_t numParams = paramListSize / sizeof (i32);

                std::vector<UInt32> ids (numParams, 0);

                AudioUnitGetProperty (audioUnit, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global,
                                      0, ids.data(), &paramListSize);

                std::map<UInt32, AudioProcessorParameterGroup*> groupIDMap;

                for (size_t i = 0; i < numParams; ++i)
                {
                    const ScopedAudioUnitParameterInfo info { audioUnit, ids[i] };

                    if (! info.isValid())
                        continue;

                    const auto paramName = getParamName (info.get());
                    const auto label = getParamLabel (info.get());
                    const auto isDiscrete = (info.get().unit == kAudioUnitParameterUnit_Indexed
                                          || info.get().unit == kAudioUnitParameterUnit_Boolean);
                    const auto isBoolean = info.get().unit == kAudioUnitParameterUnit_Boolean;

                    auto parameter = std::make_unique<AUInstanceParameter> (*this,
                                                                            ids[i],
                                                                            paramName,
                                                                            info.get().minValue,
                                                                            info.get().maxValue,
                                                                            info.get().defaultValue,
                                                                            (info.get().flags & kAudioUnitParameterFlag_NonRealTime) == 0,
                                                                            isDiscrete,
                                                                            isDiscrete ? (i32) (info.get().maxValue - info.get().minValue + 1.0f) : AudioProcessor::getDefaultNumParameterSteps(),
                                                                            isBoolean,
                                                                            label,
                                                                            (info.get().flags & kAudioUnitParameterFlag_ValuesHaveStrings) != 0);

                    paramIDToParameter.emplace (ids[i], parameter.get());

                    if (info.get().flags & kAudioUnitParameterFlag_HasClump)
                    {
                        auto groupInfo = groupIDMap.find (info.get().clumpID);

                        if (groupInfo == groupIDMap.end())
                        {
                            const auto clumpName = [this, &info]
                            {
                                AudioUnitParameterNameInfo clumpNameInfo;
                                UInt32 clumpSz = sizeof (clumpNameInfo);
                                zerostruct (clumpNameInfo);
                                clumpNameInfo.inID = info.get().clumpID;
                                clumpNameInfo.inDesiredLength = (SInt32) 256;

                                if (AudioUnitGetProperty (audioUnit,
                                                          kAudioUnitProperty_ParameterClumpName,
                                                          kAudioUnitScope_Global,
                                                          0,
                                                          &clumpNameInfo,
                                                          &clumpSz) == noErr)
                                {
                                    const CFUniquePtr<CFStringRef> ownedString { clumpNameInfo.outName };
                                    return Txt::fromCFString (clumpNameInfo.outName);
                                }

                                return Txt (info.get().clumpID);
                            }();

                            auto group = std::make_unique<AudioProcessorParameterGroup> (Txt (info.get().clumpID),
                                                                                         clumpName, Txt());
                            group->addChild (std::move (parameter));
                            groupIDMap[info.get().clumpID] = group.get();
                            newParameterTree.addChild (std::move (group));
                        }
                        else
                        {
                            groupInfo->second->addChild (std::move (parameter));
                        }
                    }
                    else
                    {
                        newParameterTree.addChild (std::move (parameter));
                    }
                }
            }
        }

        setHostedParameterTree (std::move (newParameterTree));

        UInt32 propertySize = 0;
        Boolean writable = false;

        auSupportsBypass = (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_BypassEffect,
                                                     kAudioUnitScope_Global, 0, &propertySize, &writable) == noErr
                              && propertySize >= sizeof (UInt32) && writable);
        bypassParam.reset (new AUBypassParameter (*this));
    }

    z0 updateLatency()
    {
        Float64 latencySecs = 0.0;
        UInt32 latencySize = sizeof (latencySecs);
        AudioUnitGetProperty (audioUnit, kAudioUnitProperty_Latency, kAudioUnitScope_Global,
                              0, &latencySecs, &latencySize);

        setLatencySamples (roundToInt (latencySecs * getSampleRate()));
    }

    z0 handleIncomingMidiMessage (uk, const MidiMessage& message)
    {
        const ScopedLock sl (midiInLock);
        incomingMidi.addEvent (message, 0);
    }

    z0 handlePartialSysexMessage (uk, u8k*, i32, f64) {}

    b8 isMidiEffect() const override { return isMidiEffectPlugin; }

private:
    //==============================================================================
    friend class AudioUnitPluginWindowCocoa;
    friend class AudioUnitPluginFormat;

    CoreAudioTimeConversions timeConversions;

    AudioComponentDescription componentDesc;
    AudioComponent auComponent;
    Txt pluginName, manufacturer, version;
    Txt fileOrIdentifier;
    CriticalSection lock;

    b8 wantsMidiMessages = false, producesMidiMessages = false,
         wasPlaying = false, prepared = false,
         isAUv3 = false, isMidiEffectPlugin = false;

    struct AUBuffer
    {
        AUBuffer (size_t numBuffers)
        {
            bufferList.calloc (1, (sizeof (AudioBufferList) - sizeof (::AudioBuffer)) + (sizeof (::AudioBuffer) * numBuffers));
            AudioBufferList& buffer = *bufferList.get();

            buffer.mNumberBuffers = static_cast<UInt32> (numBuffers);
        }

        operator AudioBufferList&()
        {
            return *bufferList.get();
        }

        HeapBlock<AudioBufferList> bufferList;
    };

    //==============================================================================
    struct AUBypassParameter final : public Parameter
    {
        AUBypassParameter (AudioUnitPluginInstance& effectToUse)
             : parent (effectToUse), currentValue (getCurrentHostValue())
        {}

        b8 getCurrentHostValue()
        {
            if (parent.auSupportsBypass)
            {
                UInt32 dataSize = sizeof (UInt32);
                UInt32 value = 0;

                if (AudioUnitGetProperty (parent.audioUnit, kAudioUnitProperty_BypassEffect,
                                          kAudioUnitScope_Global, 0, &value, &dataSize) == noErr
                               && dataSize == sizeof (UInt32))
                    return value != 0;
            }

            return false;
        }

        f32 getValue() const override
        {
            return currentValue ? 1.0f : 0.0f;
        }

        z0 setValue (f32 newValue) override
        {
            auto newBypassValue = (newValue != 0.0f);

            const ScopedLock sl (parent.lock);

            if (newBypassValue != currentValue)
            {
                currentValue = newBypassValue;

                if (parent.auSupportsBypass)
                {
                    UInt32 value = (newValue != 0.0f ? 1 : 0);
                    AudioUnitSetProperty (parent.audioUnit, kAudioUnitProperty_BypassEffect,
                                          kAudioUnitScope_Global, 0, &value, sizeof (UInt32));

                   #if DRX_MAC
                    jassert (parent.audioUnit != nullptr);

                    AudioUnitEvent ev;
                    ev.mEventType                       = kAudioUnitEvent_PropertyChange;
                    ev.mArgument.mProperty.mAudioUnit   = parent.audioUnit;
                    ev.mArgument.mProperty.mPropertyID  = kAudioUnitProperty_BypassEffect;
                    ev.mArgument.mProperty.mScope       = kAudioUnitScope_Global;
                    ev.mArgument.mProperty.mElement     = 0;

                    AUEventListenerNotify (parent.eventListenerRef, nullptr, &ev);
                   #endif
                }
            }
        }

        f32 getValueForText (const Txt& text) const override
        {
            Txt lowercaseText (text.toLowerCase());

            for (auto& testText : auOnStrings)
                if (lowercaseText == testText)
                    return 1.0f;

            for (auto& testText : auOffStrings)
                if (lowercaseText == testText)
                    return 0.0f;

            return text.getIntValue() != 0 ? 1.0f : 0.0f;
        }

        f32 getDefaultValue() const override                              { return 0.0f; }
        Txt getName (i32 /*maximumStringLength*/) const override         { return "Bypass"; }
        Txt getText (f32 value, i32) const override                    { return (value != 0.0f ? TRANS ("On") : TRANS ("Off")); }
        b8 isAutomatable() const override                                 { return true; }
        b8 isDiscrete() const override                                    { return true; }
        b8 isBoolean() const override                                     { return true; }
        i32 getNumSteps() const override                                    { return 2; }
        StringArray getAllValueStrings() const override                     { return values; }
        Txt getLabel() const override                                    { return {}; }

        Txt getParameterID() const override                              { return {}; }

        AudioUnitPluginInstance& parent;
        const StringArray auOnStrings  { TRANS ("on"),  TRANS ("yes"), TRANS ("true") };
        const StringArray auOffStrings { TRANS ("off"), TRANS ("no"),  TRANS ("false") };
        const StringArray values { TRANS ("Off"), TRANS ("On") };

        b8 currentValue = false;
    };

    OwnedArray<AUBuffer> outputBufferList;
    AudioTimeStamp timeStamp;
    AudioBuffer<f32> inputBuffer;
    Array<Array<AudioChannelSet>> supportedInLayouts, supportedOutLayouts;

    i32 numChannelInfos, preparedChannels = 0, preparedSamples = 0;
    HeapBlock<AUChannelInfo> channelInfos;

    AudioUnit audioUnit;
   #if DRX_MAC
    AUEventListenerRef eventListenerRef;
   #endif

    std::map<UInt32, AUInstanceParameter*> paramIDToParameter;

    AudioUnitFormatHelpers::SingleDirectionChannelMapping inMapping, outMapping;
    MidiDataConcatenator midiConcatenator;
    CriticalSection midiInLock;
    MidiBuffer incomingMidi;
    std::unique_ptr<AUBypassParameter> bypassParam;
    b8 lastProcessBlockCallWasBypass = false, auSupportsBypass = false;
    b8 haveParameterList = false;

    z0 setPluginCallbacks()
    {
        if (audioUnit != nullptr)
        {
           #if DRX_MAC
            if (producesMidiMessages)
            {
                AUMIDIOutputCallbackStruct info;
                zerostruct (info);

                info.userData = this;
                info.midiOutputCallback = renderMidiOutputCallback;

                producesMidiMessages = (AudioUnitSetProperty (audioUnit, kAudioUnitProperty_MIDIOutputCallback,
                                                              kAudioUnitScope_Global, 0, &info, sizeof (info)) == noErr);
            }
           #endif

            HostCallbackInfo info;
            zerostruct (info);

            info.hostUserData = this;
            info.beatAndTempoProc = getBeatAndTempoCallback;
            info.musicalTimeLocationProc = getMusicalTimeLocationCallback;
            info.transportStateProc = getTransportStateCallback;

            AudioUnitSetProperty (audioUnit, kAudioUnitProperty_HostCallbacks,
                                  kAudioUnitScope_Global, 0, &info, sizeof (info));
        }
    }

   #if DRX_MAC
    z0 disposeEventListener()
    {
        if (eventListenerRef != nullptr)
        {
            AUListenerDispose (eventListenerRef);
            eventListenerRef = nullptr;
        }
    }

    z0 createEventListener()
    {
        if (audioUnit == nullptr)
            return;

        disposeEventListener();

        AUEventListenerCreate (eventListenerCallback, this, CFRunLoopGetMain(),
                               kCFRunLoopDefaultMode, 0, 0, &eventListenerRef);

        for (auto* param : getParameters())
        {
            jassert (dynamic_cast<AUInstanceParameter*> (param) != nullptr);

            AudioUnitEvent event;
            event.mArgument.mParameter.mAudioUnit = audioUnit;
            event.mArgument.mParameter.mParameterID = static_cast<AUInstanceParameter*> (param)->getRawParamID();
            event.mArgument.mParameter.mScope = kAudioUnitScope_Global;
            event.mArgument.mParameter.mElement = 0;

            event.mEventType = kAudioUnitEvent_ParameterValueChange;
            AUEventListenerAddEventType (eventListenerRef, nullptr, &event);

            event.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
            AUEventListenerAddEventType (eventListenerRef, nullptr, &event);

            event.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
            AUEventListenerAddEventType (eventListenerRef, nullptr, &event);
        }

        addPropertyChangeListener (kAudioUnitProperty_PresentPreset);
        addPropertyChangeListener (kAudioUnitProperty_ParameterList);
        addPropertyChangeListener (kAudioUnitProperty_Latency);
        addPropertyChangeListener (kAudioUnitProperty_BypassEffect);
    }

    z0 addPropertyChangeListener (AudioUnitPropertyID type) const
    {
        AudioUnitEvent event;
        event.mEventType = kAudioUnitEvent_PropertyChange;
        event.mArgument.mProperty.mPropertyID = type;
        event.mArgument.mProperty.mAudioUnit = audioUnit;
        event.mArgument.mProperty.mScope = kAudioUnitScope_Global;
        event.mArgument.mProperty.mElement = 0;
        AUEventListenerAddEventType (eventListenerRef, nullptr, &event);
    }

    z0 eventCallback (const AudioUnitEvent& event, AudioUnitParameterValue newValue)
    {
        if (event.mEventType == kAudioUnitEvent_PropertyChange)
        {
            respondToPropertyChange (event.mArgument.mProperty);
            return;
        }

        const auto iter = paramIDToParameter.find (event.mArgument.mParameter.mParameterID);
        auto* param = iter != paramIDToParameter.end() ? iter->second : nullptr;
        jassert (param != nullptr); // Invalid parameter index

        if (param == nullptr)
            return;

        if (event.mEventType == kAudioUnitEvent_ParameterValueChange)
            param->sendValueChangedMessageToListeners (param->normaliseParamValue (newValue));
        else if (event.mEventType == kAudioUnitEvent_BeginParameterChangeGesture)
            param->beginChangeGesture();
        else if (event.mEventType == kAudioUnitEvent_EndParameterChangeGesture)
            param->endChangeGesture();
    }

    z0 respondToPropertyChange (const AudioUnitProperty& prop)
    {
        switch (prop.mPropertyID)
        {
            case kAudioUnitProperty_ParameterList:
                updateParameterInfo();
                updateHostDisplay (AudioProcessorListener::ChangeDetails().withParameterInfoChanged (true));
                break;

            case kAudioUnitProperty_PresentPreset:
                sendAllParametersChangedEvents();
                updateHostDisplay (AudioProcessorListener::ChangeDetails().withProgramChanged (true));
                break;

            case kAudioUnitProperty_Latency:
                updateLatency();
                break;

            case kAudioUnitProperty_BypassEffect:
                if (bypassParam != nullptr)
                    bypassParam->setValueNotifyingHost (bypassParam->getValue());

                break;
        }
    }

    static z0 eventListenerCallback (uk userRef, uk, const AudioUnitEvent* event,
                                       UInt64, AudioUnitParameterValue value)
    {
        DRX_ASSERT_MESSAGE_THREAD
        jassert (event != nullptr);
        static_cast<AudioUnitPluginInstance*> (userRef)->eventCallback (*event, value);
    }

    z0 updateParameterInfo()
    {
        for (const auto& idAndParam : paramIDToParameter)
        {
            const auto& id    = idAndParam.first;
            const auto& param = idAndParam.second;

            const ScopedAudioUnitParameterInfo info { audioUnit, id };

            if (! info.isValid())
                continue;

            param->setName  (getParamName  (info.get()));
            param->setLabel (getParamLabel (info.get()));
        }
    }
   #endif

    /*  Some fields in the AudioUnitParameterInfo may need to be released after use,
        so we'll do that using RAII.
    */
    class ScopedAudioUnitParameterInfo
    {
    public:
        ScopedAudioUnitParameterInfo (AudioUnit au, UInt32 paramId)
        {
            auto sz = (UInt32) sizeof (info);
            valid = noErr == AudioUnitGetProperty (au,
                                                   kAudioUnitProperty_ParameterInfo,
                                                   kAudioUnitScope_Global,
                                                   paramId,
                                                   &info,
                                                   &sz);
        }

        ScopedAudioUnitParameterInfo (const ScopedAudioUnitParameterInfo&) = delete;
        ScopedAudioUnitParameterInfo (ScopedAudioUnitParameterInfo&&) = delete;
        ScopedAudioUnitParameterInfo& operator= (const ScopedAudioUnitParameterInfo&) = delete;
        ScopedAudioUnitParameterInfo& operator= (ScopedAudioUnitParameterInfo&&) = delete;

        ~ScopedAudioUnitParameterInfo() noexcept
        {
            if ((info.flags & kAudioUnitParameterFlag_CFNameRelease) == 0)
                return;

            if (info.cfNameString != nullptr)
                CFRelease (info.cfNameString);

            if (info.unit == kAudioUnitParameterUnit_CustomUnit && info.unitName != nullptr)
                CFRelease (info.unitName);
        }

        b8 isValid() const { return valid; }

        const AudioUnitParameterInfo& get() const noexcept { return info; }

    private:
        AudioUnitParameterInfo info;
        b8 valid = false;
    };

    static Txt getParamName (const AudioUnitParameterInfo& info)
    {
        if ((info.flags & kAudioUnitParameterFlag_HasCFNameString) == 0)
            return { info.name, sizeof (info.name) };

        return Txt::fromCFString (info.cfNameString);
    }

    static Txt getParamLabel (const AudioUnitParameterInfo& info)
    {
        if (info.unit == kAudioUnitParameterUnit_CustomUnit)    return Txt::fromCFString (info.unitName);
        if (info.unit == kAudioUnitParameterUnit_Percent)       return "%";
        if (info.unit == kAudioUnitParameterUnit_Seconds)       return "s";
        if (info.unit == kAudioUnitParameterUnit_Hertz)         return "Hz";
        if (info.unit == kAudioUnitParameterUnit_Decibels)      return "dB";
        if (info.unit == kAudioUnitParameterUnit_Milliseconds)  return "ms";

        return {};
    }

    //==============================================================================
    OSStatus renderGetInput (AudioUnitRenderActionFlags*,
                             const AudioTimeStamp*,
                             UInt32 inBusNumber,
                             UInt32 inNumberFrames,
                             AudioBufferList* ioData)
    {
        if (inputBuffer.getNumChannels() <= 0)
        {
            jassertfalse;
            return noErr;
        }

        // if this ever happens, might need to add extra handling
        if (inputBuffer.getNumSamples() != (i32) inNumberFrames)
        {
            jassertfalse;
            return noErr;
        }

        const auto buffer = static_cast<i32> (inBusNumber) < getBusCount (true)
                          ? getBusBuffer (inputBuffer, true, static_cast<i32> (inBusNumber))
                          : AudioBuffer<f32>();

        for (i32 juceChannel = 0; juceChannel < buffer.getNumChannels(); ++juceChannel)
        {
            const auto auChannel = (i32) inMapping.getAuIndexForDrxChannel (inBusNumber, (size_t) juceChannel);

            if (auChannel < buffer.getNumChannels())
                memcpy (ioData->mBuffers[auChannel].mData, buffer.getReadPointer (juceChannel), sizeof (f32) * inNumberFrames);
            else
                zeromem (ioData->mBuffers[auChannel].mData, sizeof (f32) * inNumberFrames);
        }

        return noErr;
    }

    OSStatus renderMidiOutput (const MIDIPacketList* pktlist)
    {
        if (pktlist != nullptr && pktlist->numPackets)
        {
            auto time = Time::getMillisecondCounterHiRes() * 0.001;
            const MIDIPacket* packet = &pktlist->packet[0];

            for (UInt32 i = 0; i < pktlist->numPackets; ++i)
            {
                midiConcatenator.pushMidiData (packet->data, (i32) packet->length, time, (uk) nullptr, *this);
                packet = MIDIPacketNext (packet);
            }
        }

        return noErr;
    }

    template <typename Type1, typename Type2>
    static z0 setIfNotNull (Type1* p, Type2 value) noexcept
    {
        if (p != nullptr) *p = value;
    }

    /*  If the AudioPlayHead is available, and has valid PositionInfo, this will return the result
        of calling the specified getter on that PositionInfo. Otherwise, this will return a
        default-constructed instance of the same type.

        For getters that return an Optional, this function will return a nullopt if the playhead or
        position info is invalid.

        For getters that return a b8, this function will return false if the playhead or position
        info is invalid.
    */
    template <typename Result>
    Result getFromPlayHead (Result (AudioPlayHead::PositionInfo::* member)() const) const
    {
        if (auto* ph = getPlayHead())
            if (const auto pos = ph->getPosition())
                return ((*pos).*member)();

        return {};
    }

    OSStatus getBeatAndTempo (Float64* outCurrentBeat, Float64* outCurrentTempo) const
    {
        setIfNotNull (outCurrentBeat,  getFromPlayHead (&AudioPlayHead::PositionInfo::getPpqPosition).orFallback (0));
        setIfNotNull (outCurrentTempo, getFromPlayHead (&AudioPlayHead::PositionInfo::getBpm).orFallback (120.0));
        return noErr;
    }

    OSStatus getMusicalTimeLocation (UInt32* outDeltaSampleOffsetToNextBeat, Float32* outTimeSig_Numerator,
                                     UInt32* outTimeSig_Denominator, Float64* outCurrentMeasureDownBeat) const
    {
        setIfNotNull (outDeltaSampleOffsetToNextBeat, (UInt32) 0); //xxx
        setIfNotNull (outCurrentMeasureDownBeat, getFromPlayHead (&AudioPlayHead::PositionInfo::getPpqPositionOfLastBarStart).orFallback (0.0));

        const auto signature = getFromPlayHead (&AudioPlayHead::PositionInfo::getTimeSignature).orFallback (AudioPlayHead::TimeSignature{});
        setIfNotNull (outTimeSig_Numerator,   (Float32) signature.numerator);
        setIfNotNull (outTimeSig_Denominator, (UInt32)  signature.denominator);

        return noErr;
    }

    OSStatus getTransportState (Boolean* outIsPlaying, Boolean* outTransportStateChanged,
                                Float64* outCurrentSampleInTimeLine, Boolean* outIsCycling,
                                Float64* outCycleStartBeat, Float64* outCycleEndBeat)
    {
        const auto nowPlaying = getFromPlayHead (&AudioPlayHead::PositionInfo::getIsPlaying);
        setIfNotNull (outIsPlaying, nowPlaying);
        setIfNotNull (outTransportStateChanged, std::exchange (wasPlaying, nowPlaying) != nowPlaying);
        setIfNotNull (outCurrentSampleInTimeLine, (f64) getFromPlayHead (&AudioPlayHead::PositionInfo::getTimeInSamples).orFallback (0));
        setIfNotNull (outIsCycling, getFromPlayHead (&AudioPlayHead::PositionInfo::getIsLooping));

        const auto loopPoints = getFromPlayHead (&AudioPlayHead::PositionInfo::getLoopPoints).orFallback (AudioPlayHead::LoopPoints{});
        setIfNotNull (outCycleStartBeat, loopPoints.ppqStart);
        setIfNotNull (outCycleEndBeat, loopPoints.ppqEnd);

        return noErr;
    }

    //==============================================================================
    static OSStatus renderGetInputCallback (uk hostRef, AudioUnitRenderActionFlags* ioActionFlags,
                                            const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                                            UInt32 inNumberFrames, AudioBufferList* ioData)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)
                 ->renderGetInput (ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    }

    static OSStatus renderMidiOutputCallback (uk hostRef, const AudioTimeStamp*, UInt32 /*midiOutNum*/,
                                              const MIDIPacketList* pktlist)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)->renderMidiOutput (pktlist);
    }

    static OSStatus getBeatAndTempoCallback (uk hostRef, Float64* outCurrentBeat, Float64* outCurrentTempo)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)->getBeatAndTempo (outCurrentBeat, outCurrentTempo);
    }

    static OSStatus getMusicalTimeLocationCallback (uk hostRef, UInt32* outDeltaSampleOffsetToNextBeat,
                                                    Float32* outTimeSig_Numerator, UInt32* outTimeSig_Denominator,
                                                    Float64* outCurrentMeasureDownBeat)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)
                    ->getMusicalTimeLocation (outDeltaSampleOffsetToNextBeat, outTimeSig_Numerator,
                                              outTimeSig_Denominator, outCurrentMeasureDownBeat);
    }

    static OSStatus getTransportStateCallback (uk hostRef, Boolean* outIsPlaying, Boolean* outTransportStateChanged,
                                               Float64* outCurrentSampleInTimeLine, Boolean* outIsCycling,
                                               Float64* outCycleStartBeat, Float64* outCycleEndBeat)
    {
        return static_cast<AudioUnitPluginInstance*> (hostRef)
                    ->getTransportState (outIsPlaying, outTransportStateChanged, outCurrentSampleInTimeLine,
                                         outIsCycling, outCycleStartBeat, outCycleEndBeat);
    }

    //==============================================================================
    b8 isBusCountWritable (b8 isInput) const noexcept
    {
        UInt32 countSize;
        Boolean writable;
        AudioUnitScope scope = (isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output);

        auto err = AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_ElementCount, scope, 0, &countSize, &writable);

        return (err == noErr && writable != 0 && countSize == sizeof (UInt32));
    }

    //==============================================================================
    z0 getBusProperties (b8 isInput, UInt32 busIdx, Txt& busName, AudioChannelSet& currentLayout) const
    {
        getBusProperties (audioUnit, isInput, busIdx, busName, currentLayout);
    }

    static z0 getBusProperties (AudioUnit comp, b8 isInput, UInt32 busIdx, Txt& busName, AudioChannelSet& currentLayout)
    {
        const AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
        busName = (isInput ? "Input #" : "Output #") + Txt (busIdx + 1);

        {
            CFObjectHolder<CFStringRef> busNameCF;
            UInt32 propertySize = sizeof (busNameCF.object);

            if (AudioUnitGetProperty (comp, kAudioUnitProperty_ElementName, scope, busIdx, &busNameCF.object, &propertySize) == noErr)
                if (busNameCF.object != nullptr)
                    busName = nsStringToDrx ((NSString*) busNameCF.object);

            if (const auto auLayout = AudioUnitFormatHelpers::tryGetPropertyData<AudioChannelLayout> (comp, kAudioUnitProperty_AudioChannelLayout, scope, busIdx))
                currentLayout = CoreAudioLayouts::fromCoreAudio (auLayout->get());

            if (currentLayout.isDisabled())
            {
                AudioStreamBasicDescription descr;
                propertySize = sizeof (descr);

                if (AudioUnitGetProperty (comp, kAudioUnitProperty_StreamFormat, scope, busIdx, &descr, &propertySize) == noErr)
                    currentLayout = AudioChannelSet::canonicalChannelSet (static_cast<i32> (descr.mChannelsPerFrame));
            }
        }
    }

    //==============================================================================
    z0 numBusesChanged() override
    {
        updateSupportedLayouts();
    }

    z0 updateSupportedLayouts()
    {
        supportedInLayouts.clear();
        supportedOutLayouts.clear();
        numChannelInfos = 0;
        channelInfos.free();

        for (i32 dir = 0; dir < 2; ++dir)
        {
            const b8 isInput = (dir == 0);
            const AudioUnitScope scope = isInput ? kAudioUnitScope_Input : kAudioUnitScope_Output;
            const auto n = AudioUnitFormatHelpers::getElementCount (audioUnit, scope);

            for (UInt32 busIdx = 0; busIdx < n; ++busIdx)
            {
                Array<AudioChannelSet> supported;
                AudioChannelSet currentLayout;

                if (const auto auLayout = AudioUnitFormatHelpers::tryGetPropertyData<AudioChannelLayout> (audioUnit, kAudioUnitProperty_AudioChannelLayout, scope, busIdx))
                    currentLayout = CoreAudioLayouts::fromCoreAudio (auLayout->get());

                if (currentLayout.isDisabled())
                {
                    AudioStreamBasicDescription descr;
                    UInt32 propertySize = sizeof (descr);

                    if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_StreamFormat, scope, busIdx, &descr, &propertySize) == noErr)
                        currentLayout = AudioChannelSet::canonicalChannelSet (static_cast<i32> (descr.mChannelsPerFrame));
                }

                supported.clear();
                {
                    UInt32 propertySize = 0;
                    Boolean writable;

                    if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_SupportedChannelLayoutTags, scope, busIdx, &propertySize, &writable) == noErr
                        && propertySize > 0)
                    {
                        const size_t numElements = propertySize / sizeof (AudioChannelLayoutTag);
                        HeapBlock<AudioChannelLayoutTag> layoutTags (numElements);
                        propertySize = static_cast<UInt32> (sizeof (AudioChannelLayoutTag) * numElements);

                        if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SupportedChannelLayoutTags, scope,
                                                  static_cast<UInt32> (busIdx), layoutTags.get(), &propertySize) == noErr)
                        {
                            for (i32 j = 0; j < static_cast<i32> (numElements); ++j)
                            {
                                const AudioChannelLayoutTag tag = layoutTags[j];

                                if (tag != kAudioChannelLayoutTag_UseChannelDescriptions)
                                {
                                    AudioChannelLayout caLayout;

                                    caLayout.mChannelLayoutTag = tag;
                                    supported.addIfNotAlreadyThere (CoreAudioLayouts::fromCoreAudio (caLayout));
                                }
                            }

                            if (supported.size() > 0)
                                supported.addIfNotAlreadyThere (currentLayout);
                        }
                    }
                }

                (isInput ? supportedInLayouts : supportedOutLayouts).add (supported);
            }
        }

        {
            UInt32 propertySize = 0;
            Boolean writable;

            if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_SupportedNumChannels, kAudioUnitScope_Global, 0, &propertySize, &writable) == noErr
                && propertySize > 0)
            {
                numChannelInfos = propertySize / sizeof (AUChannelInfo);
                channelInfos.malloc (static_cast<size_t> (numChannelInfos));
                propertySize = static_cast<UInt32> (sizeof (AUChannelInfo) * static_cast<size_t> (numChannelInfos));

                if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SupportedNumChannels, kAudioUnitScope_Global, 0, channelInfos.get(), &propertySize) != noErr)
                    numChannelInfos = 0;
            }
            else
            {
                numChannelInfos = 1;
                channelInfos.malloc (static_cast<size_t> (numChannelInfos));
                channelInfos.get()->inChannels  = -1;
                channelInfos.get()->outChannels = -1;
            }
        }
    }

    b8 canProduceMidiOutput()
    {
       #if DRX_MAC
        UInt32 dataSize = 0;
        Boolean isWritable = false;

        if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_MIDIOutputCallbackInfo,
                                      kAudioUnitScope_Global, 0, &dataSize, &isWritable) == noErr
             && dataSize != 0)
        {
            CFObjectHolder<CFArrayRef> midiArray;

            if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_MIDIOutputCallbackInfo,
                                      kAudioUnitScope_Global, 0, &midiArray.object, &dataSize) == noErr)
                return (CFArrayGetCount (midiArray.object) > 0);
        }
       #endif

        return false;
    }

    b8 supportsMPE() const override
    {
        UInt32 dataSize = 0;
        Boolean isWritable = false;

        if (AudioUnitGetPropertyInfo (audioUnit, kAudioUnitProperty_SupportsMPE,
                                      kAudioUnitScope_Global, 0, &dataSize, &isWritable) == noErr
            && dataSize == sizeof (UInt32))
        {
            UInt32 result = 0;

            if (AudioUnitGetProperty (audioUnit, kAudioUnitProperty_SupportsMPE,
                                      kAudioUnitScope_Global, 0, &result, &dataSize) == noErr)
            {
                return result > 0;
            }
        }

        return false;
    }

    //==============================================================================
    z0 updateBypass (b8 processBlockBypassedCalled)
    {
        if (processBlockBypassedCalled && bypassParam != nullptr)
        {
            if (bypassParam->getValue() == 0.0f || ! lastProcessBlockCallWasBypass)
                bypassParam->setValue (1.0f);
        }
        else
        {
            if (lastProcessBlockCallWasBypass && bypassParam != nullptr)
                bypassParam->setValue (0.0f);
        }

        lastProcessBlockCallWasBypass = processBlockBypassedCalled;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUnitPluginInstance)
};

//==============================================================================
class AudioUnitPluginWindowCocoa final : public AudioProcessorEditor
{
public:
    AudioUnitPluginWindowCocoa (AudioUnitPluginInstance& p, b8 createGenericViewIfNeeded)
        : AudioProcessorEditor (&p),
          plugin (p)
    {
        addAndMakeVisible (wrapper);

        setOpaque (true);
        setVisible (true);
        setSize (100, 100);

        createView (createGenericViewIfNeeded);
    }

    ~AudioUnitPluginWindowCocoa() override
    {
        if (wrapper.getView() != nil)
        {
            wrapper.setVisible (false);
            removeChildComponent (&wrapper);
            wrapper.setView (nil);
            plugin.editorBeingDeleted (this);
        }
    }

    z0 embedViewController (DRX_IOS_MAC_VIEW* pluginView, [[maybe_unused]] const CGSize& size)
    {
        wrapper.setView (pluginView);
        waitingForViewCallback = false;

      #if DRX_MAC
        if (pluginView != nil)
            wrapper.resizeToFitView();
      #else
        [pluginView setBounds: CGRectMake (0.f, 0.f, static_cast<i32> (size.width), static_cast<i32> (size.height))];
        wrapper.setSize (static_cast<i32> (size.width), static_cast<i32> (size.height));
      #endif
    }

    b8 isValid() const        { return wrapper.getView() != nil || waitingForViewCallback; }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::white);
    }

    z0 resized() override
    {
        wrapper.setSize (getWidth(), getHeight());
    }

    z0 childBoundsChanged (Component*) override
    {
        setSize (wrapper.getWidth(), wrapper.getHeight());
    }

private:

    AudioUnitPluginInstance& plugin;
    AudioUnitFormatHelpers::AutoResizingNSViewComponent wrapper;

    typedef z0 (^ViewControllerCallbackBlock)(AUViewControllerBase *);

    b8 waitingForViewCallback = false;

    b8 createView ([[maybe_unused]] b8 createGenericViewIfNeeded)
    {
        DRX_IOS_MAC_VIEW* pluginView = nil;
        UInt32 dataSize = 0;
        Boolean isWritable = false;

       #if DRX_MAC
        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, &dataSize, &isWritable) == noErr
             && dataSize != 0
             && AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                          0, &dataSize, &isWritable) == noErr)
        {
            HeapBlock<AudioUnitCocoaViewInfo> info;
            info.calloc (dataSize, 1);

            if (AudioUnitGetProperty (plugin.audioUnit, kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, info, &dataSize) == noErr)
            {
                NSString* viewClassName = (NSString*) (info->mCocoaAUViewClass[0]);
                CFUniquePtr<CFStringRef> path (CFURLCopyPath (info->mCocoaAUViewBundleLocation));
                NSString* unescapedPath = (NSString*) CFURLCreateStringByReplacingPercentEscapes (nullptr, path.get(), CFSTR (""));
                NSBundle* viewBundle = [NSBundle bundleWithPath: [unescapedPath autorelease]];
                Class viewClass = [viewBundle classNamed: viewClassName];

                if ([viewClass conformsToProtocol: @protocol (AUCocoaUIBase)]
                     && [viewClass instancesRespondToSelector: @selector (interfaceVersion)]
                     && [viewClass instancesRespondToSelector: @selector (uiViewForAudioUnit: withSize:)])
                {
                    id factory = [[[viewClass alloc] init] autorelease];
                    pluginView = [factory uiViewForAudioUnit: plugin.audioUnit
                                                    withSize: NSMakeSize (getWidth(), getHeight())];
                }

                for (i32 i = (dataSize - sizeof (CFURLRef)) / sizeof (CFStringRef); --i >= 0;)
                    CFRelease (info->mCocoaAUViewClass[i]);

                CFRelease (info->mCocoaAUViewBundleLocation);
            }
        }
       #endif

        dataSize = 0;
        isWritable = false;

        if (AudioUnitGetPropertyInfo (plugin.audioUnit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global,
                                          0, &dataSize, &isWritable) == noErr
                && dataSize == sizeof (ViewControllerCallbackBlock))
        {
            waitingForViewCallback = true;
            auto callback = ^(AUViewControllerBase* controller) { this->requestViewControllerCallback (controller); };

            if (noErr == AudioUnitSetProperty (plugin.audioUnit, kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global, 0, &callback, dataSize))
                return true;

            waitingForViewCallback = false;
        }

       #if DRX_MAC
        if (createGenericViewIfNeeded && (pluginView == nil))
        {
            {
                // This forces CoreAudio.component to be loaded, otherwise the AUGenericView will assert
                AudioComponentDescription desc;
                Txt name, version, manufacturer;
                AudioUnitFormatHelpers::getComponentDescFromIdentifier ("AudioUnit:Output/auou,genr,appl",
                                                                        desc, name, version, manufacturer);
            }

            pluginView = [[AUGenericView alloc] initWithAudioUnit: plugin.audioUnit];
        }
       #endif

        wrapper.setView (pluginView);

        if (pluginView != nil)
            wrapper.resizeToFitView();

        return pluginView != nil;
    }

    z0 requestViewControllerCallback (AUViewControllerBase* controller)
    {
        const auto viewSize = [&controller]
        {
            auto size = CGSizeZero;

            size = [controller preferredContentSize];

            if (approximatelyEqual (size.width, 0.0) || approximatelyEqual (size.height, 0.0))
                size = controller.view.frame.size;

            return CGSizeMake (jmax ((CGFloat) 20.0f, size.width),
                               jmax ((CGFloat) 20.0f, size.height));
        }();

        if (! MessageManager::getInstance()->isThisTheMessageThread())
        {
            struct AsyncViewControllerCallback final : public CallbackMessage
            {
                AudioUnitPluginWindowCocoa* owner;
                DRX_IOS_MAC_VIEW* controllerView;
                CGSize size;

                AsyncViewControllerCallback (AudioUnitPluginWindowCocoa* plugInWindow, DRX_IOS_MAC_VIEW* inView,
                                             const CGSize& preferredSize)
                    : owner (plugInWindow), controllerView ([inView retain]), size (preferredSize)
                {}

                z0 messageCallback() override
                {
                    owner->embedViewController (controllerView, size);
                    [controllerView release];
                }
            };

            (new AsyncViewControllerCallback (this, [controller view], viewSize))->post();
        }
        else
        {
            embedViewController ([controller view], viewSize);
        }
    }
};

//==============================================================================
AudioProcessorEditor* AudioUnitPluginInstance::createEditor()
{
    std::unique_ptr<AudioProcessorEditor> w (new AudioUnitPluginWindowCocoa (*this, false));

    if (! static_cast<AudioUnitPluginWindowCocoa*> (w.get())->isValid())
        w.reset (new AudioUnitPluginWindowCocoa (*this, true)); // use AUGenericView as a fallback

    return w.release();
}

//==============================================================================
AudioUnitPluginFormat::AudioUnitPluginFormat()
{
}

AudioUnitPluginFormat::~AudioUnitPluginFormat()
{
}

z0 AudioUnitPluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                                 const Txt& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    PluginDescription desc;
    desc.fileOrIdentifier = fileOrIdentifier;
    desc.uniqueId = desc.deprecatedUid = 0;

    if (MessageManager::getInstance()->isThisTheMessageThread()
          && requiresUnblockedMessageThreadDuringCreation (desc))
        return;

    try
    {
        auto createdInstance = createInstanceFromDescription (desc, 44100.0, 512);

        if (auto auInstance = dynamic_cast<AudioUnitPluginInstance*> (createdInstance.get()))
            results.add (new PluginDescription (auInstance->getPluginDescription()));
    }
    catch (...)
    {
        // crashed while loading...
    }
}

z0 AudioUnitPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                  f64 rate, i32 blockSize,
                                                  PluginCreationCallback callback)
{
    auto auComponentResult = getAudioComponent (*this, desc);

    if (! auComponentResult.isValid())
    {
        callback (nullptr, std::move (auComponentResult.errorMessage));
        return;
    }

    createAudioUnit (auComponentResult.component,
                     [rate, blockSize, origCallback = std::move (callback)] (AudioUnit audioUnit, OSStatus err)
                     {
                        if (err == noErr)
                        {
                            auto instance = std::make_unique<AudioUnitPluginInstance> (audioUnit);

                            if (instance->initialise (rate, blockSize))
                                origCallback (std::move (instance), {});
                            else
                                origCallback (nullptr, NEEDS_TRANS ("Unable to initialise the AudioUnit plug-in"));
                        }
                        else
                        {
                            auto errMsg = TRANS ("An OS error occurred during initialisation of the plug-in (XXX)");
                            origCallback (nullptr, errMsg.replace ("XXX", Txt (err)));
                        }
                    });
}

z0 AudioUnitPluginFormat::createARAFactoryAsync (const PluginDescription& desc, ARAFactoryCreationCallback callback)
{
    auto auComponentResult = getAudioComponent (*this, desc);

    if (! auComponentResult.isValid())
    {
        callback ({ {}, "Failed to create AudioComponent for " + desc.descriptiveName });
        return;
    }

    getOrCreateARAAudioUnit (auComponentResult.component, [cb = std::move (callback)] (auto dylibKeepAliveAudioUnit)
    {
        cb ([&]() -> ARAFactoryResult
            {
                if (dylibKeepAliveAudioUnit != nullptr)
                    return { ARAFactoryWrapper { ::drx::getARAFactory (std::move (dylibKeepAliveAudioUnit)) }, "" };

                return { {}, "Failed to create ARAFactory from the provided AudioUnit" };
            }());
    });
}

b8 AudioUnitPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription& desc) const
{
    Txt pluginName, version, manufacturer;
    AudioComponentDescription componentDesc;

    if (AudioUnitFormatHelpers::getComponentDescFromIdentifier (desc.fileOrIdentifier, componentDesc,
                                                                pluginName, version, manufacturer)
           || AudioUnitFormatHelpers::getComponentDescFromFile (desc.fileOrIdentifier, componentDesc,
                                                                pluginName, version, manufacturer))
    {
        if (AudioComponent auComp = AudioComponentFindNext (nullptr, &componentDesc))
        {
            if (AudioComponentGetDescription (auComp, &componentDesc) == noErr)
                return AudioUnitFormatHelpers::isPluginAUv3 (componentDesc);
        }
    }

    return false;
}

StringArray AudioUnitPluginFormat::searchPathsForPlugins (const FileSearchPath&, b8 /*recursive*/, b8 allowPluginsWhichRequireAsynchronousInstantiation)
{
    StringArray result;
    AudioComponent comp = nullptr;

    for (;;)
    {
        AudioComponentDescription desc;
        zerostruct (desc);

        comp = AudioComponentFindNext (comp, &desc);

        if (comp == nullptr)
            break;

        if (AudioComponentGetDescription (comp, &desc) != noErr)
            continue;

        if (desc.componentType == kAudioUnitType_MusicDevice
             || desc.componentType == kAudioUnitType_MusicEffect
             || desc.componentType == kAudioUnitType_Effect
             || desc.componentType == kAudioUnitType_Generator
             || desc.componentType == kAudioUnitType_Panner
             || desc.componentType == kAudioUnitType_Mixer
             || desc.componentType == kAudioUnitType_MIDIProcessor)
        {
            if (allowPluginsWhichRequireAsynchronousInstantiation || ! AudioUnitFormatHelpers::isPluginAUv3 (desc))
                result.add (AudioUnitFormatHelpers::createPluginIdentifier (desc));
        }
    }

    return result;
}

b8 AudioUnitPluginFormat::fileMightContainThisPluginType (const Txt& fileOrIdentifier)
{
    AudioComponentDescription desc;
    Txt name, version, manufacturer;

    if (AudioUnitFormatHelpers::getComponentDescFromIdentifier (fileOrIdentifier, desc, name, version, manufacturer))
        return AudioComponentFindNext (nullptr, &desc) != nullptr;

    auto f = File::createFileWithoutCheckingPath (fileOrIdentifier);

    return (f.hasFileExtension (".component") || f.hasFileExtension (".appex"))
             && f.isDirectory();
}

Txt AudioUnitPluginFormat::getNameOfPluginFromIdentifier (const Txt& fileOrIdentifier)
{
    AudioComponentDescription desc;
    Txt name, version, manufacturer;
    AudioUnitFormatHelpers::getComponentDescFromIdentifier (fileOrIdentifier, desc, name, version, manufacturer);

    if (name.isEmpty())
        name = fileOrIdentifier;

    return name;
}

b8 AudioUnitPluginFormat::pluginNeedsRescanning (const PluginDescription& desc)
{
    AudioComponentDescription newDesc;
    Txt name, version, manufacturer;

    return ! (AudioUnitFormatHelpers::getComponentDescFromIdentifier (desc.fileOrIdentifier, newDesc,
                                                                      name, version, manufacturer)
               && version == desc.version);
}

b8 AudioUnitPluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    if (desc.fileOrIdentifier.startsWithIgnoreCase (AudioUnitFormatHelpers::auIdentifierPrefix))
        return fileMightContainThisPluginType (desc.fileOrIdentifier);

    return File (desc.fileOrIdentifier).exists();
}

FileSearchPath AudioUnitPluginFormat::getDefaultLocationsToSearch()
{
    return {};
}

#undef DRX_AU_LOG

} // namespace drx

#endif
