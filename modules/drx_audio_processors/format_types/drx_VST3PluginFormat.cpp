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

#if DRX_PLUGINHOST_VST3 && (DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD)

#include "drx_VST3Headers.h"
#include "drx_VST3Common.h"
#include "drx_ARACommon.h"

#if DRX_PLUGINHOST_ARA && (DRX_MAC || DRX_WINDOWS || DRX_LINUX)
#include <ARA_API/ARAVST3.h>

namespace ARA
{
DEF_CLASS_IID (IMainFactory)
DEF_CLASS_IID (IPlugInEntryPoint)
DEF_CLASS_IID (IPlugInEntryPoint2)
}
#endif

namespace drx
{

// UB Sanitizer doesn't necessarily have instrumentation for loaded plugins, so
// it won't recognize the dynamic types of pointers to the plugin's interfaces.
DRX_BEGIN_NO_SANITIZE ("vptr")

using namespace Steinberg;

//==============================================================================
#ifndef DRX_VST3_DEBUGGING
 #define DRX_VST3_DEBUGGING 0
#endif

#if DRX_VST3_DEBUGGING
 #define VST3_DBG(a) Logger::writeToLog (a);
#else
 #define VST3_DBG(a)
#endif

#if DRX_DEBUG
static i32 warnOnFailure (i32 result) noexcept
{
    tukk message = "Unknown result!";

    switch (result)
    {
        case kResultOk:         return result;
        case kNotImplemented:   message = "kNotImplemented";  break;
        case kNoInterface:      message = "kNoInterface";     break;
        case kResultFalse:      message = "kResultFalse";     break;
        case kInvalidArgument:  message = "kInvalidArgument"; break;
        case kInternalError:    message = "kInternalError";   break;
        case kNotInitialized:   message = "kNotInitialized";  break;
        case kOutOfMemory:      message = "kOutOfMemory";     break;
        default:                break;
    }

    DBG (message);
    return result;
}

static i32 warnOnFailureIfImplemented (i32 result) noexcept
{
    if (result != kResultOk && result != kNotImplemented)
        return warnOnFailure (result);

    return result;
}
#else
 #define warnOnFailure(x) x
 #define warnOnFailureIfImplemented(x) x
#endif

enum class MediaKind { audio, event };

static Vst::MediaType toVstType (MediaKind x) { return x == MediaKind::audio ? Vst::kAudio : Vst::kEvent; }
static Vst::BusDirection toVstType (Direction x) { return x == Direction::input ? Vst::kInput : Vst::kOutput; }

static std::vector<Vst::ParamID> getAllParamIDs (Vst::IEditController& controller)
{
    std::vector<Vst::ParamID> result;

    auto count = controller.getParameterCount();

    for (decltype (count) i = 0; i < count; ++i)
    {
        Vst::ParameterInfo info{};
        controller.getParameterInfo (i, info);
        result.push_back (info.id);
    }

    return result;
}

//==============================================================================
/*  Allows parameter updates to be queued up without blocking,
    and automatically dispatches these updates on the main thread.
*/
class EditControllerParameterDispatcher final : private Timer
{
public:
    ~EditControllerParameterDispatcher() override { stopTimer(); }

    z0 push (Steinberg::i32 index, f32 value)
    {
        if (controller == nullptr)
            return;

        if (MessageManager::getInstance()->isThisTheMessageThread())
            controller->setParamNormalized (cache.getParamID (index), value);
        else
            cache.set (index, value);
    }

    z0 start (Vst::IEditController& controllerIn)
    {
        controller = &controllerIn;
        cache = CachedParamValues { getAllParamIDs (controllerIn) };
        startTimerHz (60);
    }

    z0 flush()
    {
        cache.ifSet ([this] (Steinberg::i32 index, f32 value)
        {
            controller->setParamNormalized (cache.getParamID (index), value);
        });
    }

private:
    z0 timerCallback() override
    {
        flush();
    }

    CachedParamValues cache;
    Vst::IEditController* controller = nullptr;
};

//==============================================================================
static std::array<u32, 4> getNormalisedTUID (const TUID& tuid) noexcept
{
    const FUID fuid { tuid };
    return { { fuid.getLong1(), fuid.getLong2(), fuid.getLong3(), fuid.getLong4() } };
}

template <typename Range>
static i32 getHashForRange (Range&& range) noexcept
{
    u32 value = 0;

    for (const auto& item : range)
        value = (value * 31) + (u32) item;

    return (i32) value;
}

template <typename ObjectType>
static z0 fillDescriptionWith (PluginDescription& description, ObjectType& object)
{
    description.version  = toString (object.version).trim();
    description.category = toString (object.subCategories).trim();

    if (description.manufacturerName.trim().isEmpty())
        description.manufacturerName = toString (object.vendor).trim();
}

static std::vector<PluginDescription> createPluginDescriptions (const File& pluginFile, const ModuleInfo& info)
{
    std::vector<PluginDescription> result;

    const auto araMainFactoryClassNames = [&]
    {
        std::unordered_set<Txt> factories;

       #if DRX_PLUGINHOST_ARA && (DRX_MAC || DRX_WINDOWS || DRX_LINUX)
        for (const auto& c : info.classes)
            if (c.category == kARAMainFactoryClass)
                factories.insert (CharPointer_UTF8 (c.name.c_str()));
       #endif

        return factories;
    }();

    for (const auto& c : info.classes)
    {
        if (c.category != kVstAudioEffectClass)
            continue;

        PluginDescription description;

        description.fileOrIdentifier    = pluginFile.getFullPathName();
        description.lastFileModTime     = pluginFile.getLastModificationTime();
        description.lastInfoUpdateTime  = Time::getCurrentTime();
        description.manufacturerName    = CharPointer_UTF8 (info.factoryInfo.vendor.c_str());
        description.name                = CharPointer_UTF8 (c.name.c_str());
        description.descriptiveName     = CharPointer_UTF8 (c.name.c_str());
        description.pluginFormatName    = "VST3";
        description.numInputChannels    = 0;
        description.numOutputChannels   = 0;
        description.hasARAExtension     = araMainFactoryClassNames.find (description.name) != araMainFactoryClassNames.end();
        description.version             = CharPointer_UTF8 (c.version.c_str());

        const auto uid = VST3::UID::fromString (c.cid);

        if (! uid)
            continue;

        description.deprecatedUid       = getHashForRange (uid->data());
        description.uniqueId            = getHashForRange (getNormalisedTUID (uid->data()));

        StringArray categories;

        for (const auto& category : c.subCategories)
            categories.add (CharPointer_UTF8 (category.c_str()));

        description.category = categories.joinIntoString ("|");

        description.isInstrument = std::any_of (c.subCategories.begin(),
                                                c.subCategories.end(),
                                                [] (const auto& subcategory) { return subcategory == "Instrument"; });

        result.push_back (description);
    }

    return result;
}

static z0 createPluginDescription (PluginDescription& description,
                                     const File& pluginFile, const Txt& company, const Txt& name,
                                     const PClassInfo& info, PClassInfo2* info2, PClassInfoW* infoW,
                                     i32 numInputs, i32 numOutputs)
{
    description.fileOrIdentifier    = pluginFile.getFullPathName();
    description.lastFileModTime     = pluginFile.getLastModificationTime();
    description.lastInfoUpdateTime  = Time::getCurrentTime();
    description.manufacturerName    = company;
    description.name                = name;
    description.descriptiveName     = name;
    description.pluginFormatName    = "VST3";
    description.numInputChannels    = numInputs;
    description.numOutputChannels   = numOutputs;

    description.deprecatedUid       = getHashForRange (info.cid);
    description.uniqueId            = getHashForRange (getNormalisedTUID (info.cid));

    if (infoW != nullptr)      fillDescriptionWith (description, *infoW);
    else if (info2 != nullptr) fillDescriptionWith (description, *info2);

    if (description.category.isEmpty())
        description.category = toString (info.category).trim();

    description.isInstrument = description.category.containsIgnoreCase ("Instrument"); // This seems to be the only way to find that out! ARGH!
}

static i32 getNumSingleDirectionBusesFor (Vst::IComponent* component,
                                          MediaKind kind,
                                          Direction direction)
{
    jassert (component != nullptr);
    DRX_ASSERT_MESSAGE_THREAD
    return (i32) component->getBusCount (toVstType (kind), toVstType (direction));
}

/** Gives the total number of channels for a particular type of bus direction and media type */
static i32 getNumSingleDirectionChannelsFor (Vst::IComponent* component, Direction busDirection)
{
    jassert (component != nullptr);
    DRX_ASSERT_MESSAGE_THREAD

    const auto direction = toVstType (busDirection);
    const Steinberg::i32 numBuses = component->getBusCount (Vst::kAudio, direction);

    i32 numChannels = 0;

    for (Steinberg::i32 i = numBuses; --i >= 0;)
    {
        Vst::BusInfo busInfo;
        warnOnFailure (component->getBusInfo (Vst::kAudio, direction, i, busInfo));
        numChannels += ((busInfo.flags & Vst::BusInfo::kDefaultActive) != 0 ? (i32) busInfo.channelCount : 0);
    }

    return numChannels;
}

static z0 setStateForAllEventBuses (Vst::IComponent* component,
                                      b8 state,
                                      Direction busDirection)
{
    jassert (component != nullptr);
    DRX_ASSERT_MESSAGE_THREAD

    const auto direction = toVstType (busDirection);
    const Steinberg::i32 numBuses = component->getBusCount (Vst::kEvent, direction);

    for (Steinberg::i32 i = numBuses; --i >= 0;)
        warnOnFailure (component->activateBus (Vst::kEvent, direction, i, state));
}

//==============================================================================
static z0 toProcessContext (Vst::ProcessContext& context,
                              AudioPlayHead* playHead,
                              f64 sampleRate)
{
    jassert (sampleRate > 0.0); //Must always be valid, as stated by the VST3 SDK

    using namespace Vst;

    zerostruct (context);
    context.sampleRate = sampleRate;

    const auto position = playHead != nullptr ? playHead->getPosition()
                                              : nullopt;

    if (position.hasValue())
    {
        if (const auto timeInSamples = position->getTimeInSamples())
            context.projectTimeSamples = *timeInSamples;
        else
            jassertfalse; // The time in samples *must* be valid.

        if (const auto tempo = position->getBpm())
        {
            context.state |= ProcessContext::kTempoValid;
            context.tempo = *tempo;
        }

        if (const auto loop = position->getLoopPoints())
        {
            context.state |= ProcessContext::kCycleValid;
            context.cycleStartMusic     = loop->ppqStart;
            context.cycleEndMusic       = loop->ppqEnd;
        }

        if (const auto sig = position->getTimeSignature())
        {
            context.state |= ProcessContext::kTimeSigValid;
            context.timeSigNumerator    = sig->numerator;
            context.timeSigDenominator  = sig->denominator;
        }

        if (const auto pos = position->getPpqPosition())
        {
            context.state |= ProcessContext::kProjectTimeMusicValid;
            context.projectTimeMusic = *pos;
        }

        if (const auto barStart = position->getPpqPositionOfLastBarStart())
        {
            context.state |= ProcessContext::kBarPositionValid;
            context.barPositionMusic = *barStart;
        }

        if (const auto frameRate = position->getFrameRate())
        {
            if (const auto offset = position->getEditOriginTime())
            {
                context.state |= ProcessContext::kSmpteValid;
                context.smpteOffsetSubframes = (Steinberg::i32) (80.0 * *offset * frameRate->getEffectiveRate());
                context.frameRate.framesPerSecond = (Steinberg::u32) frameRate->getBaseRate();
                context.frameRate.flags = (Steinberg::u32) ((frameRate->isDrop()     ? FrameRate::kDropRate     : 0)
                                                             | (frameRate->isPullDown() ? FrameRate::kPullDownRate : 0));
            }
        }

        if (const auto hostTime = position->getHostTimeNs())
        {
            context.state |= ProcessContext::kSystemTimeValid;
            context.systemTime = (z64) *hostTime;
            jassert (context.systemTime >= 0);
        }

        if (position->getIsPlaying())     context.state |= ProcessContext::kPlaying;
        if (position->getIsRecording())   context.state |= ProcessContext::kRecording;
        if (position->getIsLooping())     context.state |= ProcessContext::kCycleActive;
    }
}

//==============================================================================
#if DRX_LINUX || DRX_BSD

class RunLoop  : public Linux::IRunLoop
{
public:
    RunLoop() = default;

    //==============================================================================
    tresult PLUGIN_API registerEventHandler (Linux::IEventHandler* handler,
                                             Linux::FileDescriptor fd) override
    {
        return impl->registerEventHandler (handler, fd);
    }

    tresult PLUGIN_API unregisterEventHandler (Linux::IEventHandler* handler) override
    {
        return impl->unregisterEventHandler (handler);
    }

    //==============================================================================
    tresult PLUGIN_API registerTimer (Linux::ITimerHandler* handler, Linux::TimerInterval milliseconds) override
    {
        return impl->registerTimer (handler, milliseconds);
    }

    tresult PLUGIN_API unregisterTimer (Linux::ITimerHandler* handler) override
    {
        return impl->unregisterTimer (handler);
    }

private:
    //==============================================================================
    struct TimerCaller final : private Timer
    {
        TimerCaller (Linux::ITimerHandler* h, i32 interval)  : handler (h)  { startTimer (interval); }
        ~TimerCaller() override { stopTimer(); }

        z0 timerCallback() override  { handler->onTimer(); }

        b8 operator== (Linux::ITimerHandler* other) const noexcept { return handler == other; }

        Linux::ITimerHandler* handler = nullptr;
    };

    class Impl
    {
    public:
        ~Impl()
        {
            for (const auto& h : eventHandlerMap)
                LinuxEventLoop::unregisterFdCallback (h.first);
        }

        //==============================================================================
        tresult registerEventHandler (Linux::IEventHandler* handler, Linux::FileDescriptor fd)
        {
            if (handler == nullptr)
                return kInvalidArgument;

            auto& handlers = eventHandlerMap[fd];

            if (handlers.empty())
            {
                LinuxEventLoop::registerFdCallback (fd, [this] (i32 descriptor)
                {
                    for (auto* h : eventHandlerMap[descriptor])
                        h->onFDIsSet (descriptor);

                    return true;
                });
            }

            handlers.push_back (handler);

            return kResultTrue;
        }

        tresult unregisterEventHandler (Linux::IEventHandler* handler)
        {
            if (handler == nullptr)
                return kInvalidArgument;

            for (auto iter = eventHandlerMap.begin(), end = eventHandlerMap.end(); iter != end;)
            {
                auto& handlers = iter->second;

                auto handlersIter = std::find (std::begin (handlers), std::end (handlers), handler);

                if (handlersIter != std::end (handlers))
                {
                    handlers.erase (handlersIter);

                    if (handlers.empty())
                    {
                        LinuxEventLoop::unregisterFdCallback (iter->first);
                        iter = eventHandlerMap.erase (iter);
                        continue;
                    }
                }

                ++iter;
            }

            return kResultTrue;
        }

        //==============================================================================
        tresult registerTimer (Linux::ITimerHandler* handler, Linux::TimerInterval milliseconds)
        {
            if (handler == nullptr || milliseconds <= 0)
                return kInvalidArgument;

            timerCallers.emplace_back (handler, (i32) milliseconds);
            return kResultTrue;
        }

        tresult unregisterTimer (Linux::ITimerHandler* handler)
        {
            auto iter = std::find (timerCallers.begin(), timerCallers.end(), handler);

            if (iter == timerCallers.end())
                return kInvalidArgument;

            timerCallers.erase (iter);
            return kResultTrue;
        }

    private:
        std::unordered_map<Linux::FileDescriptor, std::vector<Linux::IEventHandler*>> eventHandlerMap;
        std::list<TimerCaller> timerCallers;
    };

    SharedResourcePointer<Impl> impl;

    //==============================================================================
    DRX_DECLARE_NON_MOVEABLE (RunLoop)
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RunLoop)
};

#else

class RunLoop {};

#endif

//==============================================================================
class VST3PluginInstance;

struct VST3HostContext final : public Vst::IComponentHandler,  // From VST V3.0.0
                               public Vst::IComponentHandler2, // From VST V3.1.0 (a very well named class, of course!)
                               public Vst::IComponentHandler3, // From VST V3.5.0 (also very well named!)
                               public Vst::IContextMenuTarget,
                               public Vst::IHostApplication,
                               public Vst::IUnitHandler,
                               public RunLoop,
                               private ComponentRestarter::Listener
{
    VST3HostContext()
    {
        appName = File::getSpecialLocation (File::currentApplicationFile).getFileNameWithoutExtension();
    }

    ~VST3HostContext() override = default;

    DRX_DECLARE_VST3_COM_REF_METHODS

    FUnknown* getFUnknown()     { return static_cast<Vst::IComponentHandler*> (this); }

    static b8 hasFlag (Steinberg::i32 source, Steinberg::i32 flag) noexcept
    {
        return (source & flag) == flag;
    }

    //==============================================================================
    tresult PLUGIN_API beginEdit (Vst::ParamID paramID) override;
    tresult PLUGIN_API performEdit (Vst::ParamID paramID, Vst::ParamValue valueNormalized) override;
    tresult PLUGIN_API endEdit (Vst::ParamID paramID) override;

    tresult PLUGIN_API restartComponent (Steinberg::i32 flags) override;
    tresult PLUGIN_API setDirty (TBool) override;

    //==============================================================================
    tresult PLUGIN_API requestOpenEditor ([[maybe_unused]] FIDString name) override
    {
        // This request cannot currently be surfaced in the DRX public API
        return kResultFalse;
    }

    tresult PLUGIN_API startGroupEdit() override
    {
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API finishGroupEdit() override
    {
        jassertfalse;
        return kResultFalse;
    }

    z0 setPlugin (VST3PluginInstance* instance)
    {
        jassert (plugin == nullptr);
        plugin = instance;
    }

    //==============================================================================
    struct ContextMenu final : public Vst::IContextMenu
    {
        ContextMenu (VST3PluginInstance& pluginInstance)  : owner (pluginInstance) {}
        virtual ~ContextMenu() {}

        DRX_DECLARE_VST3_COM_REF_METHODS
        DRX_DECLARE_VST3_COM_QUERY_METHODS

        Steinberg::i32 PLUGIN_API getItemCount() override     { return (Steinberg::i32) items.size(); }

        tresult PLUGIN_API addItem (const Item& item, IContextMenuTarget* target) override
        {
            jassert (target != nullptr);

            ItemAndTarget newItem;
            newItem.item = item;
            newItem.target = addVSTComSmartPtrOwner (target);

            items.add (newItem);
            return kResultOk;
        }

        tresult PLUGIN_API removeItem (const Item& toRemove, IContextMenuTarget* target) override
        {
            for (i32 i = items.size(); --i >= 0;)
            {
                auto& item = items.getReference (i);

                if (item.item.tag == toRemove.tag && item.target.get() == target)
                    items.remove (i);
            }

            return kResultOk;
        }

        tresult PLUGIN_API getItem (Steinberg::i32 tag, Item& result, IContextMenuTarget** target) override
        {
            for (i32 i = 0; i < items.size(); ++i)
            {
                auto& item = items.getReference (i);

                if (item.item.tag == tag)
                {
                    result = item.item;

                    if (target != nullptr)
                        *target = item.target.get();

                    return kResultTrue;
                }
            }

            zerostruct (result);
            return kResultFalse;
        }

        tresult PLUGIN_API popup (UCoord x, UCoord y) override;

       #if ! DRX_MODAL_LOOPS_PERMITTED
        static z0 menuFinished (i32 modalResult, VSTComSmartPtr<ContextMenu> menu)  { menu->handleResult (modalResult); }
       #endif

    private:
        enum { zeroTagReplacement = 0x7fffffff };

        Atomic<i32> refCount;
        VST3PluginInstance& owner;

        struct ItemAndTarget
        {
            Item item;
            VSTComSmartPtr<IContextMenuTarget> target;
        };

        Array<ItemAndTarget> items;

        z0 handleResult (i32 result)
        {
            if (result == 0)
                return;

            if (result == zeroTagReplacement)
                result = 0;

            for (i32 i = 0; i < items.size(); ++i)
            {
                auto& item = items.getReference (i);

                if ((i32) item.item.tag == result)
                {
                    if (item.target != nullptr)
                        item.target->executeMenuItem ((Steinberg::i32) result);

                    break;
                }
            }
        }

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContextMenu)
    };

    Vst::IContextMenu* PLUGIN_API createContextMenu (IPlugView*, const Vst::ParamID*) override
    {
        if (plugin == nullptr)
            return nullptr;

        auto* result = new ContextMenu (*plugin);
        result->addRef();
        return result;
    }

    tresult PLUGIN_API executeMenuItem (Steinberg::i32) override
    {
        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API getName (Vst::String128 name) override
    {
        Steinberg::Txt str (appName.toUTF8());
        str.copyTo (name, 0, 127);
        return kResultOk;
    }

    tresult PLUGIN_API createInstance (TUID cid, TUID iid, uk* obj) override
    {
        *obj = nullptr;

        if (! doUIDsMatch (cid, iid))
        {
            jassertfalse;
            return kInvalidArgument;
        }

        if (doUIDsMatch (cid, Vst::IMessage::iid) && doUIDsMatch (iid, Vst::IMessage::iid))
        {
            *obj = new Message;
            return kResultOk;
        }

        if (doUIDsMatch (cid, Vst::IAttributeList::iid) && doUIDsMatch (iid, Vst::IAttributeList::iid))
        {
            *obj = new AttributeList;
            return kResultOk;
        }

        jassertfalse;
        return kNotImplemented;
    }

    //==============================================================================
    tresult PLUGIN_API notifyUnitSelection (Vst::UnitID) override
    {
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API notifyProgramListChange (Vst::ProgramListID, Steinberg::i32) override;

    //==============================================================================
    tresult PLUGIN_API queryInterface (const TUID iid, uk* obj) override
    {
        return testForMultiple (*this,
                                iid,
                                UniqueBase<Vst::IComponentHandler>{},
                                UniqueBase<Vst::IComponentHandler2>{},
                                UniqueBase<Vst::IComponentHandler3>{},
                                UniqueBase<Vst::IContextMenuTarget>{},
                                UniqueBase<Vst::IHostApplication>{},
                                UniqueBase<Vst::IUnitHandler>{},
                               #if DRX_LINUX || DRX_BSD
                                UniqueBase<Linux::IRunLoop>{},
                               #endif
                                SharedBase<FUnknown, Vst::IComponentHandler>{}).extract (obj);
    }

private:
    //==============================================================================
    VST3PluginInstance* plugin = nullptr;
    Atomic<i32> refCount;
    Txt appName;

    ComponentRestarter componentRestarter { *this };

    z0 restartComponentOnMessageThread (i32 flags) override;

    //==============================================================================
    class Attribute
    {
    public:
        using Int    = Steinberg::z64;
        using Float  = f64;
        using Txt = std::vector<Vst::TChar>;
        using Binary = std::vector<t8>;

        explicit Attribute (Int    x) noexcept { constructFrom (std::move (x)); }
        explicit Attribute (Float  x) noexcept { constructFrom (std::move (x)); }
        explicit Attribute (Txt x) noexcept { constructFrom (std::move (x)); }
        explicit Attribute (Binary x) noexcept { constructFrom (std::move (x)); }

        Attribute (Attribute&& other) noexcept
        {
            moveFrom (std::move (other));
        }

        Attribute& operator= (Attribute&& other) noexcept
        {
            reset();
            moveFrom (std::move (other));
            return *this;
        }

        ~Attribute() noexcept
        {
            reset();
        }

        tresult getInt (Steinberg::z64& result) const
        {
            if (kind != Kind::tagInt)
                return kResultFalse;

            result = storage.storedInt;
            return kResultTrue;
        }

        tresult getFloat (f64& result) const
        {
            if (kind != Kind::tagFloat)
                return kResultFalse;

            result = storage.storedFloat;
            return kResultTrue;
        }

        tresult getString (Vst::TChar* data, Steinberg::u32 numBytes) const
        {
            if (kind != Kind::tagString)
                return kResultFalse;

            std::memcpy (data,
                         storage.storedString.data(),
                         jmin (sizeof (Vst::TChar) * storage.storedString.size(), (size_t) numBytes));
            return kResultTrue;
        }

        tresult getBinary (ukk& data, Steinberg::u32& numBytes) const
        {
            if (kind != Kind::tagBinary)
                return kResultFalse;

            data = storage.storedBinary.data();
            numBytes = (Steinberg::u32) storage.storedBinary.size();
            return kResultTrue;
        }

    private:
        z0 constructFrom (Int    x) noexcept { kind = Kind::tagInt;    new (&storage.storedInt)    Int    (std::move (x)); }
        z0 constructFrom (Float  x) noexcept { kind = Kind::tagFloat;  new (&storage.storedFloat)  Float  (std::move (x)); }
        z0 constructFrom (Txt x) noexcept { kind = Kind::tagString; new (&storage.storedString) Txt (std::move (x)); }
        z0 constructFrom (Binary x) noexcept { kind = Kind::tagBinary; new (&storage.storedBinary) Binary (std::move (x)); }

        z0 reset() noexcept
        {
            switch (kind)
            {
                case Kind::tagInt:                                    break;
                case Kind::tagFloat:                                  break;
                case Kind::tagString: storage.storedString.~vector(); break;
                case Kind::tagBinary: storage.storedBinary.~vector(); break;
            }
        }

        z0 moveFrom (Attribute&& other) noexcept
        {
            switch (other.kind)
            {
                case Kind::tagInt:    constructFrom (std::move (other.storage.storedInt));    break;
                case Kind::tagFloat:  constructFrom (std::move (other.storage.storedFloat));  break;
                case Kind::tagString: constructFrom (std::move (other.storage.storedString)); break;
                case Kind::tagBinary: constructFrom (std::move (other.storage.storedBinary)); break;
            }
        }

        enum class Kind { tagInt, tagFloat, tagString, tagBinary };

        union Storage
        {
            Storage() {}
            ~Storage() {}

            Steinberg::z64              storedInt;
            f64                        storedFloat;
            std::vector<Vst::TChar>       storedString;
            std::vector<t8>             storedBinary;
        };

        Storage storage;
        Kind kind;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Attribute)
    };

    //==============================================================================
    class AttributeList final : public Vst::IAttributeList
    {
    public:
        AttributeList() = default;
        virtual ~AttributeList() = default;

        DRX_DECLARE_VST3_COM_REF_METHODS
        DRX_DECLARE_VST3_COM_QUERY_METHODS

        //==============================================================================
        tresult PLUGIN_API setInt (AttrID attr, Steinberg::z64 value) override
        {
            return set (attr, value);
        }

        tresult PLUGIN_API setFloat (AttrID attr, f64 value) override
        {
            return set (attr, value);
        }

        tresult PLUGIN_API setString (AttrID attr, const Vst::TChar* string) override
        {
            return set (attr, std::vector<Vst::TChar> (string, string + 1 + tstrlen (string)));
        }

        tresult PLUGIN_API setBinary (AttrID attr, ukk data, Steinberg::u32 size) override
        {
            const auto* ptr = static_cast<tukk> (data);
            return set (attr, std::vector<t8> (ptr, ptr + size));
        }

        tresult PLUGIN_API getInt (AttrID attr, Steinberg::z64& result) override
        {
            return get (attr, [&] (const auto& x) { return x.getInt (result); });
        }

        tresult PLUGIN_API getFloat (AttrID attr, f64& result) override
        {
            return get (attr, [&] (const auto& x) { return x.getFloat (result); });
        }

        tresult PLUGIN_API getString (AttrID attr, Vst::TChar* result, Steinberg::u32 length) override
        {
            return get (attr, [&] (const auto& x) { return x.getString (result, length); });
        }

        tresult PLUGIN_API getBinary (AttrID attr, ukk& data, Steinberg::u32& size) override
        {
            return get (attr, [&] (const auto& x) { return x.getBinary (data, size); });
        }

    private:
        template <typename Value>
        tresult set (AttrID attr, Value&& value)
        {
            if (attr == nullptr)
                return kInvalidArgument;

            const auto iter = attributes.find (attr);

            if (iter != attributes.end())
                iter->second = Attribute (std::forward<Value> (value));
            else
                attributes.emplace (attr, Attribute (std::forward<Value> (value)));

            return kResultTrue;
        }

        template <typename Visitor>
        tresult get (AttrID attr, Visitor&& visitor)
        {
            if (attr == nullptr)
                return kInvalidArgument;

            const auto iter = attributes.find (attr);

            if (iter == attributes.cend())
                return kResultFalse;

            return visitor (iter->second);
        }

        std::map<std::string, Attribute> attributes;
        Atomic<i32> refCount { 1 };

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeList)
    };

    struct Message final : public Vst::IMessage
    {
        Message() = default;
        virtual ~Message() = default;

        DRX_DECLARE_VST3_COM_REF_METHODS
        DRX_DECLARE_VST3_COM_QUERY_METHODS

        FIDString PLUGIN_API getMessageID() override              { return messageId.toRawUTF8(); }
        z0 PLUGIN_API setMessageID (FIDString id) override      { messageId = toString (id); }
        Vst::IAttributeList* PLUGIN_API getAttributes() override  { return &attributeList; }

    private:
        AttributeList attributeList;
        Txt messageId;
        Atomic<i32> refCount { 1 };

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Message)
    };

    VSTComSmartPtr<AttributeList> attributeList;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3HostContext)
};

//==============================================================================
struct DescriptionLister
{
    static std::vector<PluginDescription> tryLoadFast (const File& file, const File& moduleinfo)
    {
        if (! moduleinfo.existsAsFile())
            return {};

        MemoryBlock mb;

        if (! moduleinfo.loadFileAsData (mb))
            return {};

        const std::string_view blockAsStringView (static_cast<tukk> (mb.getData()), mb.getSize());
        const auto parsed = ModuleInfoLib::parseJson (blockAsStringView, nullptr);

        if (! parsed)
            return {};

        return createPluginDescriptions (file, *parsed);
    }

    static std::vector<PluginDescription> findDescriptionsFast (const File& file)
    {
        const auto moduleinfoNewLocation = file.getChildFile ("Contents").getChildFile ("Resources").getChildFile ("moduleinfo.json");

        if (const auto loaded = tryLoadFast (file, moduleinfoNewLocation); ! loaded.empty())
            return loaded;

        return tryLoadFast (file, file.getChildFile ("Contents").getChildFile ("moduleinfo.json"));
    }

    static std::vector<PluginDescription> findDescriptionsSlow (VST3HostContext& host,
                                                                IPluginFactory& factory,
                                                                const File& file)
    {
        std::vector<PluginDescription> result;

        StringArray foundNames;
        PFactoryInfo factoryInfo;
        factory.getFactoryInfo (&factoryInfo);
        auto companyName = toString (factoryInfo.vendor).trim();

        auto numClasses = factory.countClasses();

        // Every ARA::IMainFactory must have a matching IComponent.
        // The match is determined by the two classes having the same name.
        std::unordered_set<Txt> araMainFactoryClassNames;

       #if DRX_PLUGINHOST_ARA && (DRX_MAC || DRX_WINDOWS || DRX_LINUX)
        for (Steinberg::i32 i = 0; i < numClasses; ++i)
        {
            PClassInfo info;
            factory.getClassInfo (i, &info);
            if (std::strcmp (info.category, kARAMainFactoryClass) == 0)
                araMainFactoryClassNames.insert (info.name);
        }
       #endif

        for (Steinberg::i32 i = 0; i < numClasses; ++i)
        {
            PClassInfo info;
            factory.getClassInfo (i, &info);

            if (std::strcmp (info.category, kVstAudioEffectClass) != 0)
                continue;

            const Txt name (toString (info.name).trim());

            if (foundNames.contains (name, true))
                continue;

            std::unique_ptr<PClassInfo2> info2;
            std::unique_ptr<PClassInfoW> infoW;

            {
                VSTComSmartPtr<IPluginFactory2> pf2;
                VSTComSmartPtr<IPluginFactory3> pf3;

                if (pf2.loadFrom (&factory))
                {
                    info2.reset (new PClassInfo2());
                    pf2->getClassInfo2 (i, info2.get());
                }

                if (pf3.loadFrom (&factory))
                {
                    infoW.reset (new PClassInfoW());
                    pf3->getClassInfoUnicode (i, infoW.get());
                }
            }

            foundNames.add (name);

            PluginDescription desc;

            {
                VSTComSmartPtr<Vst::IComponent> component;

                if (component.loadFrom (&factory, info.cid))
                {
                    if (component->initialize (host.getFUnknown()) == kResultOk)
                    {
                        auto numInputs  = getNumSingleDirectionChannelsFor (component.get(), Direction::input);
                        auto numOutputs = getNumSingleDirectionChannelsFor (component.get(), Direction::output);

                        createPluginDescription (desc, file, companyName, name,
                                                 info, info2.get(), infoW.get(), numInputs, numOutputs);

                        component->terminate();
                    }
                    else
                    {
                        jassertfalse;
                    }
                }
                else
                {
                    jassertfalse;
                }
            }

            if (araMainFactoryClassNames.find (name) != araMainFactoryClassNames.end())
                desc.hasARAExtension = true;

            if (desc.uniqueId != 0)
                result.push_back (desc);
        }

        return result;
    }
};

//==============================================================================
struct DLLHandle
{
    explicit DLLHandle (const File& fileToOpen)
        : dllFile (fileToOpen)
    {
        open();
    }

    ~DLLHandle()
    {
       #if DRX_MAC
        if (bundleRef != nullptr)
       #endif
        {
            factory = nullptr;

            using ExitModuleFn = b8 (PLUGIN_API*)();

            if (auto* exitFn = (ExitModuleFn) getFunction (exitFnName))
                exitFn();

           #if DRX_WINDOWS || DRX_LINUX || DRX_BSD
            library.close();
           #endif
        }
    }

    //==============================================================================
    VSTComSmartPtr<IPluginFactory> getPluginFactory()
    {
        if (factory == nullptr)
            if (auto* proc = (GetFactoryProc) getFunction (factoryFnName))
                factory = becomeVSTComSmartPtrOwner (proc());

        // The plugin NEEDS to provide a factory to be able to be called a VST3!
        // Most likely you are trying to load a 32-bit VST3 from a 64-bit host
        // or vice versa.
        jassert (factory != nullptr);
        return factory;
    }

    uk getFunction (tukk functionName)
    {
       #if DRX_WINDOWS || DRX_LINUX || DRX_BSD
        return library.getFunction (functionName);
       #elif DRX_MAC
        if (bundleRef == nullptr)
            return nullptr;

        CFUniquePtr<CFStringRef> name (Txt (functionName).toCFString());
        return CFBundleGetFunctionPointerForName (bundleRef.get(), name.get());
       #endif
    }

    File getFile() const noexcept  { return dllFile; }

private:
    File dllFile;
    VSTComSmartPtr<IPluginFactory> factory;

    static constexpr tukk factoryFnName = "GetPluginFactory";

   #if DRX_WINDOWS
    static constexpr tukk entryFnName = "InitDll";
    static constexpr tukk exitFnName  = "ExitDll";

    using EntryProc = b8 (PLUGIN_API*)();
   #elif DRX_LINUX || DRX_BSD
    static constexpr tukk entryFnName = "ModuleEntry";
    static constexpr tukk exitFnName  = "ModuleExit";

    using EntryProc = b8 (PLUGIN_API*) (uk);
   #elif DRX_MAC
    static constexpr tukk entryFnName = "bundleEntry";
    static constexpr tukk exitFnName  = "bundleExit";

    using EntryProc = b8 (*) (CFBundleRef);
   #endif

    //==============================================================================
   #if DRX_WINDOWS || DRX_LINUX || DRX_BSD
    DynamicLibrary library;

    b8 open()
    {
        if (library.open (dllFile.getFullPathName()))
        {
            if (auto* proc = (EntryProc) getFunction (entryFnName))
            {
               #if DRX_WINDOWS
                if (proc())
               #else
                if (proc (library.getNativeHandle()))
               #endif
                    return true;
            }
            else
            {
                // this is required for some plug-ins which don't export the dll entry point function
                return true;
            }

            library.close();
        }

        return false;
    }
   #elif DRX_MAC
    CFUniquePtr<CFBundleRef> bundleRef;

    b8 open()
    {
        auto* utf8 = dllFile.getFullPathName().toRawUTF8();

        if (auto url = CFUniquePtr<CFURLRef> (CFURLCreateFromFileSystemRepresentation (nullptr,
                                                                                       (const UInt8*) utf8,
                                                                                       (CFIndex) std::strlen (utf8),
                                                                                       dllFile.isDirectory())))
        {
            bundleRef.reset (CFBundleCreate (kCFAllocatorDefault, url.get()));

            if (bundleRef != nullptr)
            {
                CFObjectHolder<CFErrorRef> error;

                if (CFBundleLoadExecutableAndReturnError (bundleRef.get(), &error.object))
                    if (auto* proc = (EntryProc) getFunction (entryFnName))
                        if (proc (bundleRef.get()))
                            return true;

                if (error.object != nullptr)
                    if (auto failureMessage = CFUniquePtr<CFStringRef> (CFErrorCopyFailureReason (error.object)))
                        DBG (Txt::fromCFString (failureMessage.get()));

                bundleRef = nullptr;
            }
        }

        return false;
    }
   #endif

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DLLHandle)
};

struct RefCountedDllHandle final : public ReferenceCountedObject
{
public:
    using Ptr = ReferenceCountedObjectPtr<RefCountedDllHandle>;

    ~RefCountedDllHandle() override
    {
        getHandles().erase (this);
    }

    VSTComSmartPtr<IPluginFactory> getPluginFactory()
    {
        return handle.getPluginFactory();
    }

    File getFile() const
    {
        return handle.getFile();
    }

    static Ptr getHandle (const Txt& modulePath)
    {
        const auto f = getDLLFileFromBundle (modulePath);

        auto& bundles = getHandles();

        const auto iter = std::find_if (bundles.begin(), bundles.end(), [&] (Ptr x)
        {
            return x->handle.getFile() == f;
        });

        if (iter != bundles.end())
            return *iter;

        return new RefCountedDllHandle { f };
    }

private:
    explicit RefCountedDllHandle (const File& f)
        : handle (f)
    {
        getHandles().insert (this);
    }

    static File getDLLFileFromBundle (const Txt& bundlePath)
    {
       #if DRX_LINUX || DRX_BSD
        const auto machineName = []() -> Txt
        {
            struct utsname unameData;
            const auto res = uname (&unameData);

            if (res != 0)
                return {};

            return unameData.machine;
        }();

        const File file { bundlePath };

        return file.getChildFile ("Contents")
                   .getChildFile (machineName + "-linux")
                   .getChildFile (file.getFileNameWithoutExtension() + ".so");
       #else
        return File { bundlePath };
       #endif
    }

    static std::set<RefCountedDllHandle*>& getHandles()
    {
        static std::set<RefCountedDllHandle*> bundles;
        return bundles;
    }

    DLLHandle handle;
};

//==============================================================================
//==============================================================================
struct VST3ModuleHandle final
{
public:
    static VST3ModuleHandle create (const File& pluginFile, const PluginDescription& desc)
    {
        VST3ModuleHandle result;
        result.handle = RefCountedDllHandle::getHandle (pluginFile.getFullPathName());

        if (result.handle == nullptr)
            return {};

        auto factory = result.handle->getPluginFactory();

        if (factory == nullptr)
            return {};

        const auto numClasses = factory->countClasses();
        result.classIndex = findClassMatchingDescription (factory, desc);

        if (result.classIndex == numClasses)
            return {};

        return result;
    }

    VSTComSmartPtr<IPluginFactory> getPluginFactory() const
    {
        return handle != nullptr ? handle->getPluginFactory() : VSTComSmartPtr<IPluginFactory>{};
    }

    Steinberg::i32 getClassIndex() const
    {
        return classIndex;
    }

    Txt getName() const
    {
        auto factory = getPluginFactory();

        if (factory == nullptr)
            return {};

        PClassInfo info{};
        factory->getClassInfo (classIndex, &info);

        return toString (info.name).trim();
    }

    File getFile() const
    {
        return handle != nullptr ? handle->getFile() : File{};
    }

    b8 isValid() const
    {
        if (handle == nullptr)
            return false;

        const auto factory = handle->getPluginFactory();

        if (factory == nullptr)
            return false;

        return isPositiveAndBelow (classIndex, factory->countClasses());
    }

private:
    static Steinberg::i32 findClassMatchingDescription (VSTComSmartPtr<IPluginFactory> factory, const PluginDescription& desc)
    {
        const auto numClasses = factory->countClasses();

        for (auto i = decltype (numClasses){}; i < numClasses; ++i)
        {
            PClassInfo info{};
            factory->getClassInfo (i, &info);

            if (std::strcmp (info.category, kVstAudioEffectClass) != 0)
                continue;

            const auto uniqueId = getHashForRange (getNormalisedTUID (info.cid));
            const auto deprecatedUid = getHashForRange (info.cid);

            if (toString (info.name).trim() != desc.name)
                continue;

            if (uniqueId != desc.uniqueId && deprecatedUid != desc.deprecatedUid)
                continue;

            return i;
        }

        return numClasses;
    }

    RefCountedDllHandle::Ptr handle;
    Steinberg::i32 classIndex{};

    //==============================================================================
    DRX_LEAK_DETECTOR (VST3ModuleHandle)
};

template <typename Type, size_t N>
static i32 compareWithString (Type (&charArray)[N], const Txt& str)
{
    return std::strncmp (str.toRawUTF8(),
                         charArray,
                         std::min (str.getNumBytesAsUTF8(), (size_t) numElementsInArray (charArray)));
}

template <typename Callback>
static z0 forEachARAFactory ([[maybe_unused]] IPluginFactory* pluginFactory, [[maybe_unused]] Callback&& cb)
{
   #if DRX_PLUGINHOST_ARA && (DRX_MAC || DRX_WINDOWS || DRX_LINUX)
    const auto numClasses = pluginFactory->countClasses();
    for (Steinberg::i32 i = 0; i < numClasses; ++i)
    {
        PClassInfo info;
        pluginFactory->getClassInfo (i, &info);

        if (std::strcmp (info.category, kARAMainFactoryClass) == 0)
        {
            const b8 keepGoing = cb (info);
            if (! keepGoing)
                break;
        }
    }
   #endif
}

static std::shared_ptr<const ARA::ARAFactory> getARAFactory ([[maybe_unused]] IPluginFactory* pluginFactory,
                                                             [[maybe_unused]] const Txt& pluginName)
{
    std::shared_ptr<const ARA::ARAFactory> factory;

   #if DRX_PLUGINHOST_ARA && (DRX_MAC || DRX_WINDOWS || DRX_LINUX)
    forEachARAFactory (pluginFactory,
                       [&pluginFactory, &pluginName, &factory] (const auto& pcClassInfo)
                       {
                           if (compareWithString (pcClassInfo.name, pluginName) == 0)
                           {
                               ARA::IMainFactory* source;
                               if (pluginFactory->createInstance (pcClassInfo.cid, ARA::IMainFactory::iid, (uk*) &source)
                                   == kResultOk)
                               {
                                   factory = getOrCreateARAFactory (source->getFactory(),
                                                                    [source]() { source->release(); });
                                   return false;
                               }
                               jassert (source == nullptr);
                           }

                           return true;
                       });
   #endif

    return factory;
}

static std::shared_ptr<const ARA::ARAFactory> getARAFactory (VST3ModuleHandle& module)
{
    return getARAFactory (module.getPluginFactory().get(), module.getName());
}

//==============================================================================
struct VST3PluginWindow final : public AudioProcessorEditor,
                                public RunLoop,
                                public IPlugFrame,
                                private ComponentMovementWatcher,
                                private ComponentBoundsConstrainer
{
    VST3PluginWindow (AudioPluginInstance* owner, VSTComSmartPtr<IPlugView> pluginView)
        : AudioProcessorEditor (owner),
          ComponentMovementWatcher (this),
          view (pluginView)
         #if DRX_MAC
        , embeddedComponent (*owner)
         #endif
    {
        setSize (10, 10);
        setOpaque (true);
        setVisible (true);
        setConstrainer (this);

        warnOnFailure (view->setFrame (this));
        view->queryInterface (IPlugViewContentScaleSupport::iid, (uk*) &scaleInterface);

        setContentScaleFactor();
        resizeToFit();

        setResizable (view->canResize() == kResultTrue, false);
    }

    ~VST3PluginWindow() override
    {
        if (scaleInterface != nullptr)
            scaleInterface->release();

        #if DRX_LINUX || DRX_BSD
         embeddedComponent.removeClient();
        #endif

        if (attachedCalled)
            warnOnFailure (view->removed());

        warnOnFailure (view->setFrame (nullptr));

        processor.editorBeingDeleted (this);

       #if DRX_MAC
        embeddedComponent.setView (nullptr);
       #endif

        view = nullptr;
    }

    tresult PLUGIN_API queryInterface (const TUID queryIid, uk* obj) override
    {
        return testForMultiple (*this,
                                queryIid,
                               #if DRX_LINUX || DRX_BSD
                                UniqueBase<Linux::IRunLoop>{},
                               #endif
                                UniqueBase<IPlugFrame>{}).extract (obj);
    }

    DRX_DECLARE_VST3_COM_REF_METHODS

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::black);
    }

    z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        view->onWheel (wheel.deltaY);
    }

    z0 focusGained (FocusChangeType) override     { view->onFocus (true); }
    z0 focusLost (FocusChangeType) override       { view->onFocus (false); }

    /** It seems that most, if not all, plugins do their own keyboard hooks,
        but IPlugView does have a set of keyboard related methods...
    */
    b8 keyStateChanged (b8 /*isKeyDown*/) override  { return true; }
    b8 keyPressed (const KeyPress& /*key*/) override  { return true; }

private:
    z0 checkBounds (Rectangle<i32>& bounds,
                      const Rectangle<i32>&,
                      const Rectangle<i32>&,
                      b8,
                      b8,
                      b8,
                      b8) override
    {
        auto rect = componentToVST3Rect (bounds);
        auto constrainedRect = rect;
        view->checkSizeConstraint (&constrainedRect);

        // Prevent inadvertent window growth while dragging; see componentMovedOrResized below
        if (constrainedRect.getWidth() != rect.getWidth() || constrainedRect.getHeight() != rect.getHeight())
            bounds = vst3ToComponentRect (constrainedRect);
    }

    //==============================================================================
    z0 componentPeerChanged() override {}

    /*  Convert from the component's coordinate system to the hosted VST3's coordinate system. */
    ViewRect componentToVST3Rect (Rectangle<i32> r) const
    {
        const auto combinedScale = nativeScaleFactor * getDesktopScaleFactor();
        const auto physical = (localAreaToGlobal (r.toFloat()) * combinedScale).toNearestInt();
        return { 0, 0, physical.getWidth(), physical.getHeight() };
    }

    /*  Convert from the hosted VST3's coordinate system to the component's coordinate system. */
    Rectangle<i32> vst3ToComponentRect (const ViewRect& vr) const
    {
        const auto combinedScale = nativeScaleFactor * getDesktopScaleFactor();
        const auto floatRect = Rectangle { (f32) vr.right, (f32) vr.bottom } / combinedScale;
        return getLocalArea (nullptr, floatRect).toNearestInt();
    }

    z0 componentMovedOrResized (b8, b8 wasResized) override
    {
        if (recursiveResize || ! wasResized || getTopLevelComponent()->getPeer() == nullptr)
            return;

        if (view->canResize() == kResultTrue)
        {
            // componentToVST3Rect will apply DPI scaling and round to the nearest integer; vst3ToComponentRect
            // will invert the DPI scaling, but the logical size returned by vst3ToComponentRect may be
            // different from the original size due to floating point rounding if the scale factor is > 100%.
            // This can cause the window to unexpectedly grow while it's moving.
            auto scaledRect = componentToVST3Rect (getLocalBounds());

            auto constrainedRect = scaledRect;
            view->checkSizeConstraint (&constrainedRect);

            const auto tieRect = [] (const auto& x) { return std::tuple (x.getWidth(), x.getHeight()); };

            // Only update the size if the constrained size is actually different
            if (tieRect (constrainedRect) != tieRect (scaledRect))
            {
                const ScopedValueSetter recursiveResizeSetter (recursiveResize, true);

                const auto logicalSize = vst3ToComponentRect (constrainedRect);
                setSize (logicalSize.getWidth(), logicalSize.getHeight());
            }

            embeddedComponent.setBounds (getLocalBounds());

            view->onSize (&constrainedRect);
        }
        else
        {
            ViewRect rect;
            warnOnFailure (view->getSize (&rect));

            resizeWithRect (embeddedComponent, rect);
        }

        // Some plugins don't update their cursor correctly when mousing out the window
        Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    z0 componentVisibilityChanged() override
    {
        attachPluginWindow();
        resizeToFit();
        componentMovedOrResized (true, true);
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    z0 resizeToFit()
    {
        ViewRect rect;
        warnOnFailure (view->getSize (&rect));
        resizeWithRect (*this, rect);
    }

    tresult PLUGIN_API resizeView (IPlugView* incomingView, ViewRect* newSize) override
    {
        const ScopedValueSetter<b8> recursiveResizeSetter (recursiveResize, true);

        if (incomingView != nullptr && newSize != nullptr && incomingView == view.get())
        {
            const auto oldPhysicalSize = componentToVST3Rect (getLocalBounds());
            const auto logicalSize = vst3ToComponentRect (*newSize);
            setSize (logicalSize.getWidth(), logicalSize.getHeight());
            embeddedComponent.setSize (logicalSize.getWidth(), logicalSize.getHeight());

           #if DRX_WINDOWS
            embeddedComponent.updateHWNDBounds();
           #elif DRX_LINUX || DRX_BSD
            embeddedComponent.updateEmbeddedBounds();
           #endif

            // According to the VST3 Workflow Diagrams, a resizeView from the plugin should
            // always trigger a response from the host which confirms the new size.
            auto currentPhysicalSize = componentToVST3Rect (getLocalBounds());

            if (currentPhysicalSize.getWidth() != oldPhysicalSize.getWidth()
                || currentPhysicalSize.getHeight() != oldPhysicalSize.getHeight()
                || ! isInOnSize)
            {
                // Guard against plug-ins immediately calling resizeView() with the same size
                const ScopedValueSetter<b8> inOnSizeSetter (isInOnSize, true);
                view->onSize (&currentPhysicalSize);
            }

            return kResultTrue;
        }

        jassertfalse;
        return kInvalidArgument;
    }

    //==============================================================================
    z0 resizeWithRect (Component& comp, const ViewRect& rect) const
    {
        const auto logicalSize = vst3ToComponentRect (rect);
        comp.setSize (jmax (10, logicalSize.getWidth()),
                      jmax (10, logicalSize.getHeight()));
    }

    z0 attachPluginWindow()
    {
        if (pluginHandle == HandleFormat{})
        {
            #if DRX_WINDOWS
             pluginHandle = static_cast<HWND> (embeddedComponent.getHWND());
            #endif

             embeddedComponent.setBounds (getLocalBounds());
             addAndMakeVisible (embeddedComponent);

            #if DRX_MAC
             pluginHandle = (HandleFormat) embeddedComponent.getView();
            #elif DRX_LINUX || DRX_BSD
             pluginHandle = (HandleFormat) embeddedComponent.getHostWindowID();
            #endif

            if (pluginHandle == HandleFormat{})
            {
                jassertfalse;
                return;
            }

            [[maybe_unused]] const auto attachedResult = view->attached ((uk) pluginHandle, defaultVST3WindowType);
            [[maybe_unused]] const auto warning = warnOnFailure (attachedResult);

            if (attachedResult == kResultOk)
                attachedCalled = true;

            updatePluginScale();

           #if DRX_WINDOWS
            // Make sure the embedded component window is the right size
            // and invalidate the embedded HWND and any child windows
            embeddedComponent.updateHWNDBounds();
           #endif
        }
    }

    z0 updatePluginScale()
    {
        if (scaleInterface != nullptr)
            setContentScaleFactor();
        else
            resizeToFit();
    }

    z0 setContentScaleFactor()
    {
        if (scaleInterface != nullptr)
        {
            [[maybe_unused]] const auto result = scaleInterface->setContentScaleFactor ((IPlugViewContentScaleSupport::ScaleFactor) getEffectiveScale());

           #if ! DRX_MAC
            [[maybe_unused]] const auto warning = warnOnFailure (result);
           #endif
        }
    }

    z0 setScaleFactor (f32 s) override
    {
        userScaleFactor = s;
        setContentScaleFactor();
        resizeToFit();
    }

    f32 getEffectiveScale() const
    {
        return nativeScaleFactor * userScaleFactor;
    }

    //==============================================================================
    Atomic<i32> refCount { 1 };
    VSTComSmartPtr<IPlugView> view;

   #if DRX_WINDOWS
    using HandleFormat = HWND;

    struct ViewComponent final : public HWNDComponent
    {
        ViewComponent()
        {
            setOpaque (true);
            inner.addToDesktop (0);

            if (auto* peer = inner.getPeer())
                setHWND (peer->getNativeHandle());
        }

        z0 paint (Graphics& g) override { g.fillAll (Colors::black); }

    private:
        struct Inner final : public Component
        {
            Inner() { setOpaque (true); }
            z0 paint (Graphics& g) override { g.fillAll (Colors::black); }
        };

        Inner inner;
    };

    ViewComponent embeddedComponent;
   #elif DRX_MAC
    NSViewComponentWithParent embeddedComponent;
    using HandleFormat = NSView*;
   #elif DRX_LINUX || DRX_BSD
    XEmbedComponent embeddedComponent { true, false };
    using HandleFormat = Window;
   #else
    Component embeddedComponent;
    using HandleFormat = uk;
   #endif

    HandleFormat pluginHandle = {};
    b8 recursiveResize = false, isInOnSize = false, attachedCalled = false;

    IPlugViewContentScaleSupport* scaleInterface = nullptr;
    f32 nativeScaleFactor = 1.0f;
    f32 userScaleFactor = 1.0f;

    struct ScaleNotifierCallback
    {
        VST3PluginWindow& window;

        z0 operator() (f32 platformScale) const
        {
            MessageManager::callAsync ([ref = Component::SafePointer<VST3PluginWindow> (&window), platformScale]
            {
                if (auto* r = ref.getComponent())
                {
                    r->nativeScaleFactor = platformScale;
                    r->setContentScaleFactor();
                    r->resizeToFit();

                   #if DRX_WINDOWS
                    r->embeddedComponent.updateHWNDBounds();
                   #elif DRX_LINUX || DRX_BSD
                    r->embeddedComponent.updateEmbeddedBounds();
                   #endif
                }
            });
        }
    };

    NativeScaleFactorNotifier scaleNotifier { this, ScaleNotifierCallback { *this } };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginWindow)
};

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

//==============================================================================
static b8 hasARAExtension (IPluginFactory* pluginFactory, const Txt& pluginClassName)
{
    b8 result = false;

    forEachARAFactory (pluginFactory,
                       [&pluginClassName, &result] (const auto& pcClassInfo)
                       {
                           if (compareWithString (pcClassInfo.name, pluginClassName) == 0)
                           {
                               result = true;

                               return false;
                           }

                           return true;
                       });

    return result;
}

//==============================================================================
struct VST3ComponentHolder
{
    explicit VST3ComponentHolder (const VST3ModuleHandle& m)
        : module (m)
    {
        host = addVSTComSmartPtrOwner (new VST3HostContext());
    }

    ~VST3ComponentHolder()
    {
        terminate();
    }

    b8 isIComponentAlsoIEditController() const
    {
        if (component == nullptr)
        {
            jassertfalse;
            return false;
        }

        return VSTComSmartPtr<Vst::IEditController>().loadFrom (component.get());
    }

    b8 fetchController (VSTComSmartPtr<Vst::IEditController>& editController)
    {
        if (! isComponentInitialised && ! initialise())
            return false;

        editController.loadFrom (component.get());

        // Get the IEditController:
        TUID controllerCID = { 0 };

        if (editController == nullptr
            && component->getControllerClassId (controllerCID) == kResultTrue
            && FUID (controllerCID).isValid())
        {
            auto factory = module.getPluginFactory();
            editController.loadFrom (factory.get(), controllerCID);
        }

        if (editController == nullptr)
        {
            // Try finding the IEditController the i64 way around:
            auto factory = module.getPluginFactory();
            auto numClasses = factory->countClasses();

            for (Steinberg::i32 i = 0; i < numClasses; ++i)
            {
                PClassInfo classInfo;
                factory->getClassInfo (i, &classInfo);

                if (std::strcmp (classInfo.category, kVstComponentControllerClass) == 0)
                    editController.loadFrom (factory.get(), classInfo.cid);
            }
        }

        return (editController != nullptr);
    }

    //==============================================================================
    z0 fillInPluginDescription (PluginDescription& description) const
    {
        jassert (module.isValid() && isComponentInitialised);

        const auto factory = module.getPluginFactory();

        if (factory == nullptr)
        {
            jassertfalse;
            return;
        }

        PFactoryInfo factoryInfo;
        factory->getFactoryInfo (&factoryInfo);

        const auto classIdx = module.getClassIndex();

        if (classIdx == factory->countClasses())
        {
            jassertfalse;
            return;
        }

        PClassInfo info;
        [[maybe_unused]] b8 success = (factory->getClassInfo (classIdx, &info) == kResultOk);
        jassert (success);

        VSTComSmartPtr<IPluginFactory2> pf2;
        VSTComSmartPtr<IPluginFactory3> pf3;

        std::unique_ptr<PClassInfo2> info2;
        std::unique_ptr<PClassInfoW> infoW;

        if (pf2.loadFrom (factory.get()))
        {
            info2.reset (new PClassInfo2());
            pf2->getClassInfo2 (classIdx, info2.get());
        }
        else
        {
            info2.reset();
        }

        if (pf3.loadFrom (factory.get()))
        {
            pf3->setHostContext (host->getFUnknown());
            infoW.reset (new PClassInfoW());
            pf3->getClassInfoUnicode (classIdx, infoW.get());
        }
        else
        {
            infoW.reset();
        }

        Vst::BusInfo bus;
        i32 totalNumInputChannels = 0, totalNumOutputChannels = 0;

        i32 n = component->getBusCount (Vst::kAudio, Vst::kInput);
        for (i32 i = 0; i < n; ++i)
            if (component->getBusInfo (Vst::kAudio, Vst::kInput, i, bus) == kResultOk)
                totalNumInputChannels += ((bus.flags & Vst::BusInfo::kDefaultActive) != 0 ? bus.channelCount : 0);

        n = component->getBusCount (Vst::kAudio, Vst::kOutput);
        for (i32 i = 0; i < n; ++i)
            if (component->getBusInfo (Vst::kAudio, Vst::kOutput, i, bus) == kResultOk)
                totalNumOutputChannels += ((bus.flags & Vst::BusInfo::kDefaultActive) != 0 ? bus.channelCount : 0);

        createPluginDescription (description, module.getFile(),
                                 factoryInfo.vendor, module.getName(),
                                 info, info2.get(), infoW.get(),
                                 totalNumInputChannels,
                                 totalNumOutputChannels);

        description.hasARAExtension = hasARAExtension (factory.get(), description.name);
    }

    //==============================================================================
    b8 initialise()
    {
        if (isComponentInitialised)
            return true;

        // It's highly advisable to create your plugins using the message thread.
        // The VST3 spec requires that many of the functions called during
        // initialisation are only called from the message thread.
        DRX_ASSERT_MESSAGE_THREAD

        const auto factory = module.getPluginFactory();

        if (factory == nullptr)
            return false;

        VSTComSmartPtr<IPluginFactory3> pf3;
        pf3.loadFrom (factory.get());

        if (pf3 != nullptr)
            pf3->setHostContext (host->getFUnknown());

        const auto classIdx = module.getClassIndex();

        if (classIdx == factory->countClasses())
            return false;

        PClassInfo info;
        if (factory->getClassInfo (classIdx, &info) != kResultOk)
            return false;

        if (! component.loadFrom (factory.get(), info.cid) || component == nullptr)
            return false;

        cidOfComponent = FUID (info.cid);

        if (warnOnFailure (component->initialize (host->getFUnknown())) != kResultOk)
            return false;

        isComponentInitialised = true;

        return true;
    }

    z0 terminate()
    {
        if (isComponentInitialised)
        {
            component->terminate();
            isComponentInitialised = false;
        }

        component = nullptr;
    }

    //==============================================================================
    VST3ModuleHandle module;
    VSTComSmartPtr<VST3HostContext> host;
    VSTComSmartPtr<Vst::IComponent> component;
    FUID cidOfComponent;

    b8 isComponentInitialised = false;
};

//==============================================================================
class HostToClientParamQueue final : public Vst::IParamValueQueue
{
public:
    struct Item
    {
        Steinberg::i32 offset{};
        f32 value{};
    };

    using ItemsByIndex = std::map<Steinberg::i32, Item>;
    using Node = ItemsByIndex::node_type;
    using NodeStorage = std::vector<Node>;

    static Node makeNode()
    {
        ItemsByIndex container { {} };
        return container.extract (container.begin());
    }

    static NodeStorage makeStorage (size_t numItems)
    {
        NodeStorage result (numItems);
        std::generate (result.begin(), result.end(), makeNode);
        return result;
    }

    HostToClientParamQueue (Vst::ParamID idIn, Steinberg::i32 parameterIndexIn, NodeStorage& items)
        : paramId (idIn), parameterIndex (parameterIndexIn), sharedStorage (items)
    {
    }

    virtual ~HostToClientParamQueue() = default;

    DRX_DECLARE_VST3_COM_REF_METHODS
    DRX_DECLARE_VST3_COM_QUERY_METHODS

    Vst::ParamID PLUGIN_API getParameterId() override { return paramId; }

    Steinberg::i32 getParameterIndex() const noexcept { return parameterIndex; }

    Steinberg::i32 PLUGIN_API getPointCount() override
    {
        return (Steinberg::i32) list.size();
    }

    tresult PLUGIN_API getPoint (Steinberg::i32 index,
                                 Steinberg::i32& offset,
                                 Vst::ParamValue& value) override
    {
        const auto item = getItem (index);

        if (! item.has_value())
            return kResultFalse;

        std::tie (offset, value) = std::tie (item->offset, item->value);
        return kResultTrue;
    }

    tresult PLUGIN_API addPoint (Steinberg::i32, Vst::ParamValue, Steinberg::i32&) override
    {
        // The VST3 SDK uses the IParamValueQueue interface for both input and output of parameter
        // change information. This interface includes the addPoint() function, which allows for
        // new parameter points to be added. However, when communicating parameter information from
        // host to plugin, it doesn't make sense for the plugin to add extra parameter change points
        // to the incoming queues. To enforce that the plugin doesn't attempt to mutate the
        // incoming queues, we always return false from this function. The host adds points to the
        // queue by calling append(), which is not exposed to the plugin, and is therefore
        // effectively private to the host.
        jassertfalse;
        return kResultFalse;
    }

    z0 append (Item item)
    {
        // The host *must* add points in sample-offset order
        jassert (list.empty() || std::prev (list.end())->second.offset <= item.offset);

        auto node = getNodeFromStorage();
        node.key() = (Steinberg::i32) list.size();
        node.mapped() = item;
        list.insert (std::move (node));
    }

    z0 clear()
    {
        while (! list.empty())
            sharedStorage.push_back (list.extract (list.begin()));
    }

private:
    std::optional<Item> getItem (Steinberg::i32 index) const
    {
        if (! isPositiveAndBelow (index, list.size()))
            return {};

        const auto iter = list.find (index);

        if (iter == list.end())
        {
            // Invariant violation
            jassertfalse;
            return {};
        }

        return iter->second;
    }

    Node getNodeFromStorage()
    {
        if (! sharedStorage.empty())
        {
            auto result = std::move (sharedStorage.back());
            sharedStorage.pop_back();
            return result;
        }

        // Allocating!
        jassertfalse;
        return makeNode();
    }

    const Vst::ParamID paramId;
    const Steinberg::i32 parameterIndex;
    NodeStorage& sharedStorage;
    ItemsByIndex list;
    Atomic<i32> refCount;
};

class ClientToHostParamQueue final : public Vst::IParamValueQueue
{
public:
    ClientToHostParamQueue (Vst::ParamID idIn, Steinberg::i32 parameterIndexIn)
        : paramId (idIn), parameterIndex (parameterIndexIn)
    {
    }

    virtual ~ClientToHostParamQueue() = default;

    DRX_DECLARE_VST3_COM_REF_METHODS
    DRX_DECLARE_VST3_COM_QUERY_METHODS

    Vst::ParamID PLUGIN_API getParameterId() override { return paramId; }

    Steinberg::i32 getParameterIndex() const noexcept { return parameterIndex; }

    Steinberg::i32 PLUGIN_API getPointCount() override
    {
        return size;
    }

    tresult PLUGIN_API getPoint (Steinberg::i32 index,
                                 Steinberg::i32& sampleOffset,
                                 Vst::ParamValue& value) override
    {
        if (! isPositiveAndBelow (index, size))
            return kResultFalse;

        sampleOffset = 0;
        value = cachedValue;

        return kResultTrue;
    }

    tresult PLUGIN_API addPoint (Steinberg::i32,
                                 Vst::ParamValue value,
                                 Steinberg::i32& index) override
    {
        index = size++;
        set ((f32) value);

        return kResultTrue;
    }

    z0 set (f32 valueIn)
    {
        cachedValue = valueIn;
        size = 1;
    }

    z0 clear() { size = 0; }

    std::optional<f32> getValue() const
    {
        return size > 0 ? std::optional<f32> (cachedValue) : std::nullopt;
    }

private:
    const Vst::ParamID paramId;
    const Steinberg::i32 parameterIndex;
    f32 cachedValue{};
    Steinberg::i32 size{};
    Atomic<i32> refCount;
};

//==============================================================================
/*  An implementation of IParameterChanges with some important characteristics:
    - Lookup by index is O(1)
    - Lookup by paramID is also O(1)
    - addParameterData never allocates, as i64 you pass a paramID already passed to initialise
*/
template <typename Queue>
class ParameterChanges final : public Vst::IParameterChanges
{
    static constexpr Steinberg::i32 notInVector = -1;

    struct Entry
    {
        explicit Entry (std::unique_ptr<Queue> queue) : ptr (addVSTComSmartPtrOwner (queue.release())) {}

        VSTComSmartPtr<Queue> ptr;
        Steinberg::i32 index = notInVector;
    };

    using Map = std::unordered_map<Vst::ParamID, Entry>;
    using Queues = std::vector<Entry*>;

public:
    virtual ~ParameterChanges() = default;

    DRX_DECLARE_VST3_COM_REF_METHODS
    DRX_DECLARE_VST3_COM_QUERY_METHODS

    Steinberg::i32 PLUGIN_API getParameterCount() override
    {
        return (Steinberg::i32) queues.size();
    }

    Queue* PLUGIN_API getParameterData (Steinberg::i32 index) override
    {
        if (isPositiveAndBelow (index, queues.size()))
        {
            auto& entry = queues[(size_t) index];
            // If this fails, our container has become internally inconsistent
            jassert (entry->index == index);
            return entry->ptr.get();
        }

        return nullptr;
    }

    Queue* PLUGIN_API addParameterData (const Vst::ParamID& id, Steinberg::i32& index) override
    {
        const auto it = map.find (id);

        if (it == map.end())
            return nullptr;

        auto& result = it->second;

        if (result.index == notInVector)
        {
            result.index = (Steinberg::i32) queues.size();
            queues.push_back (&result);
        }

        index = result.index;
        return result.ptr.get();
    }

    z0 set (Vst::ParamID id, f32 value, Steinberg::i32 offset)
    {
        Steinberg::i32 indexOut = notInVector;

        if (auto* queue = addParameterData (id, indexOut))
            queue->append ({ offset, value });
    }

    z0 clear()
    {
        for (auto* item : queues)
        {
            item->index = notInVector;
            item->ptr->clear();
        }

        queues.clear();
    }

    template <typename... Args>
    z0 initialise (const std::vector<Vst::ParamID>& idsIn, Args&&... args)
    {
        for (const auto [index, id] : enumerate (idsIn))
        {
            map.emplace (id,
                         Entry { std::make_unique<Queue> (id,
                                                          (Steinberg::i32) index,
                                                          std::forward<Args> (args)...) });
        }

        queues.reserve (map.size());
        queues.clear();
    }

    template <typename Callback>
    z0 forEach (Callback&& callback) const
    {
        for (const auto* item : queues)
        {
            auto* ptr = item->ptr.get();

            if (ptr == nullptr)
                continue;

            if (const auto finalValue = ptr->getValue())
                callback (ptr->getParameterIndex(), ptr->getParameterId(), *finalValue);
        }
    }

private:
    Map map;
    Queues queues;
    Atomic<i32> refCount;
};

//==============================================================================
class VST3PluginInstance final  : public AudioPluginInstance
{
public:
    //==============================================================================
    struct VST3Parameter final  : public Parameter
    {
        VST3Parameter (VST3PluginInstance& parent, Steinberg::i32 vstParameterIndex)
            : pluginInstance (parent),
              vstParamIndex (vstParameterIndex)
        {
        }

        f32 getValue() const override
        {
            return pluginInstance.cachedParamValues.get (vstParamIndex);
        }

        /*  The 'normal' setValue call, which will update both the processor and editor.
        */
        z0 setValue (f32 newValue) override
        {
            pluginInstance.cachedParamValues.set (vstParamIndex, newValue);
            pluginInstance.parameterDispatcher.push (vstParamIndex, newValue);
        }

        /*  If we're syncing the editor to the processor, the processor won't need to
            be notified about the parameter updates, so we can avoid flagging the
            change when updating the f32 cache.
        */
        z0 setValueWithoutUpdatingProcessor (f32 newValue)
        {
            if (! exactlyEqual (pluginInstance.cachedParamValues.exchangeWithoutNotifying (vstParamIndex, newValue), newValue))
                sendValueChangedMessageToListeners (newValue);
        }

        Txt getText (f32 value, i32 maximumLength) const override
        {
            MessageManagerLock lock;

            if (pluginInstance.editController != nullptr)
            {
                Vst::String128 result;

                if (pluginInstance.editController->getParamStringByValue (cachedInfo.id, value, result) == kResultOk)
                    return toString (result).substring (0, maximumLength);
            }

            return Parameter::getText (value, maximumLength);
        }

        f32 getValueForText (const Txt& text) const override
        {
            MessageManagerLock lock;

            if (pluginInstance.editController != nullptr)
            {
                Vst::ParamValue result;

                if (pluginInstance.editController->getParamValueByString (cachedInfo.id, toString (text), result) == kResultOk)
                    return (f32) result;
            }

            return Parameter::getValueForText (text);
        }

        f32 getDefaultValue() const override
        {
            return (f32) cachedInfo.defaultNormalizedValue;
        }

        Txt getName (i32 /*maximumStringLength*/) const override
        {
            return toString (cachedInfo.title);
        }

        Txt getLabel() const override
        {
            return toString (cachedInfo.units);
        }

        b8 isAutomatable() const override
        {
            return (cachedInfo.flags & Vst::ParameterInfo::kCanAutomate) != 0;
        }

        b8 isDiscrete() const override
        {
            return getNumSteps() != AudioProcessor::getDefaultNumParameterSteps();
        }

        i32 getNumSteps() const override
        {
            const auto stepCount = cachedInfo.stepCount;
            return stepCount == 0 ? AudioProcessor::getDefaultNumParameterSteps()
                                  : stepCount + 1;
        }

        StringArray getAllValueStrings() const override
        {
            return {};
        }

        Txt getParameterID() const override
        {
            return Txt (cachedInfo.id);
        }

        Vst::ParamID getParamID() const noexcept { return cachedInfo.id; }

        z0 updateCachedInfo()
        {
            cachedInfo = fetchParameterInfo();
        }

        Vst::ParameterInfo getParameterInfo() const
        {
            return cachedInfo;
        }

        Steinberg::i32 getVstParamIndex() const
        {
            return vstParamIndex;
        }

    private:
        Vst::ParameterInfo fetchParameterInfo() const
        {
            DRX_ASSERT_MESSAGE_THREAD
            return pluginInstance.getParameterInfoForIndex (vstParamIndex);
        }

        VST3PluginInstance& pluginInstance;
        const Steinberg::i32 vstParamIndex;
        Vst::ParameterInfo cachedInfo = fetchParameterInfo();
    };

    //==============================================================================
    explicit VST3PluginInstance (std::unique_ptr<VST3ComponentHolder> componentHolder)
        : AudioPluginInstance (getBusProperties (componentHolder->component)),
          holder (std::move (componentHolder))
    {
        jassert (holder->isComponentInitialised);
        holder->host->setPlugin (this);
    }

    ~VST3PluginInstance() override
    {
        MessageManager::callSync ([this] { cleanup(); });
    }

    z0 cleanup()
    {
        jassert (getActiveEditor() == nullptr); // You must delete any editors before deleting the plugin instance!

        releaseResources();

        if (editControllerConnection != nullptr && componentConnection != nullptr)
        {
            editControllerConnection->disconnect (componentConnection.get());
            componentConnection->disconnect (editControllerConnection.get());
        }

        editController->setComponentHandler (nullptr);

        if (isControllerInitialised && ! holder->isIComponentAlsoIEditController())
            editController->terminate();

        holder->terminate();

        componentConnection = nullptr;
        editControllerConnection = nullptr;
        unitData = nullptr;
        unitInfo = nullptr;
        programListData = nullptr;
        componentHandler2 = nullptr;
        componentHandler = nullptr;
        processor = nullptr;
        midiMapping = nullptr;
        editController2 = nullptr;
        editController = nullptr;
    }

    //==============================================================================
    b8 initialise()
    {
        // It's highly advisable to create your plugins using the message thread.
        // The VST3 spec requires that many of the functions called during
        // initialisation are only called from the message thread.
        DRX_ASSERT_MESSAGE_THREAD

        if (! holder->initialise())
            return false;

        if (! (isControllerInitialised || holder->fetchController (editController)))
            return false;

        // If the IComponent and IEditController are the same, we will have
        // already initialized the object at this point and should avoid doing so again.
        if (! holder->isIComponentAlsoIEditController())
            editController->initialize (holder->host->getFUnknown());

        isControllerInitialised = true;
        editController->setComponentHandler (holder->host.get());
        grabInformationObjects();
        interconnectComponentAndController();

        auto configureParameters = [this]
        {
            initialiseParameterList();
            synchroniseStates();
            syncProgramNames();
        };

        configureParameters();
        setupIO();

        // Some plug-ins don't present their parameters until after the IO has been
        // configured, so we need to jump though all these hoops again
        if (getParameters().isEmpty() && editController->getParameterCount() > 0)
            configureParameters();

        updateMidiMappings();

        parameterDispatcher.start (*editController);

        return true;
    }

    z0 getExtensions (ExtensionsVisitor& visitor) const override
    {
        struct Extensions final :  public ExtensionsVisitor::VST3Client,
                                   public ExtensionsVisitor::ARAClient
        {
            explicit Extensions (const VST3PluginInstance* instanceIn) : instance (instanceIn) {}

            Vst::IComponent* getIComponentPtr() const noexcept override   { return instance->holder->component.get(); }

            MemoryBlock getPreset() const override             { return instance->getStateForPresetFile(); }

            b8 setPreset (const MemoryBlock& rawData) const override
            {
                return instance->setStateFromPresetFile (rawData);
            }

            z0 createARAFactoryAsync (std::function<z0 (ARAFactoryWrapper)> cb) const noexcept override
            {
                cb (ARAFactoryWrapper { ::drx::getARAFactory (instance->holder->module) });
            }

            const VST3PluginInstance* instance = nullptr;
        };

        Extensions extensions { this };
        visitor.visitVST3Client (extensions);

        if (::drx::getARAFactory (holder->module))
        {
            visitor.visitARAClient (extensions);
        }
    }

    uk getPlatformSpecificData() override   { return holder->component.get(); }

    z0 updateMidiMappings()
    {
        // MIDI mappings will always be updated on the main thread, but we need to ensure
        // that we're not simultaneously reading them on the audio thread.
        const SpinLock::ScopedLockType processLock (processMutex);

        if (midiMapping != nullptr)
            storedMidiMapping.storeMappings (*midiMapping);
    }

    //==============================================================================
    const Txt getName() const override
    {
        return holder->module.getName();
    }

    std::vector<Vst::SpeakerArrangement> getActualArrangements (b8 isInput) const
    {
        std::vector<Vst::SpeakerArrangement> result;

        const auto numBuses = getBusCount (isInput);

        for (auto i = 0; i < numBuses; ++i)
            result.push_back (getArrangementForBus (processor.get(), isInput, i));

        return result;
    }

    std::optional<std::vector<Vst::SpeakerArrangement>> busLayoutsToArrangements (b8 isInput) const
    {
        std::vector<Vst::SpeakerArrangement> result;

        const auto numBuses = getBusCount (isInput);

        for (auto i = 0; i < numBuses; ++i)
        {
            if (const auto arr = getVst3SpeakerArrangement (getBus (isInput, i)->getLastEnabledLayout()))
                result.push_back (*arr);
            else
                return {};
        }

        return result;
    }

    z0 prepareToPlay (f64 newSampleRate, i32 estimatedSamplesPerBlock) override
    {
        // The VST3 spec requires that IComponent::setupProcessing() is called on the message
        // thread. If you call it from a different thread, some plugins may break.
        DRX_ASSERT_MESSAGE_THREAD
        MessageManagerLock lock;

        const SpinLock::ScopedLockType processLock (processMutex);

        // Avoid redundantly calling things like setActive, which can be a heavy-duty call for some plugins:
        if (isActive
              && approximatelyEqual (getSampleRate(), newSampleRate)
              && getBlockSize() == estimatedSamplesPerBlock)
            return;

        using namespace Vst;

        // If the plugin has already been activated (prepareToPlay has been called twice without
        // a matching releaseResources call) deactivate it so that the speaker layout and bus
        // activation can be updated safely.
        deactivate();

        ProcessSetup setup;
        setup.symbolicSampleSize    = isUsingDoublePrecision() ? kSample64 : kSample32;
        setup.maxSamplesPerBlock    = estimatedSamplesPerBlock;
        setup.sampleRate            = newSampleRate;
        setup.processMode           = isNonRealtime() ? kOffline : kRealtime;

        warnOnFailure (processor->setupProcessing (setup));

        holder->initialise();

        auto inArrangements  = busLayoutsToArrangements (true) .value_or (std::vector<SpeakerArrangement>{});
        auto outArrangements = busLayoutsToArrangements (false).value_or (std::vector<SpeakerArrangement>{});

        // Some plug-ins will crash if you pass a nullptr to setBusArrangements!
        SpeakerArrangement nullArrangement = {};
        auto* inData  = inArrangements .empty() ? &nullArrangement : inArrangements .data();
        auto* outData = outArrangements.empty() ? &nullArrangement : outArrangements.data();

        warnOnFailure (processor->setBusArrangements (inData,  static_cast<i32> (inArrangements .size()),
                                                      outData, static_cast<i32> (outArrangements.size())));

        const auto inArrActual  = getActualArrangements (true);
        const auto outArrActual = getActualArrangements (false);

        jassert (inArrActual == inArrangements && outArrActual == outArrangements);

        // Needed for having the same sample rate in processBlock(); some plugins need this!
        setRateAndBufferSizeDetails (newSampleRate, estimatedSamplesPerBlock);

        auto numInputBuses  = getBusCount (true);
        auto numOutputBuses = getBusCount (false);

        for (i32 i = 0; i < numInputBuses; ++i)
            warnOnFailure (holder->component->activateBus (Vst::kAudio, Vst::kInput,  i, getBus (true,  i)->isEnabled() ? 1 : 0));

        for (i32 i = 0; i < numOutputBuses; ++i)
            warnOnFailure (holder->component->activateBus (Vst::kAudio, Vst::kOutput, i, getBus (false, i)->isEnabled() ? 1 : 0));

        setLatencySamples (jmax (0, (i32) processor->getLatencySamples()));

        inputBusMap .prepare (createChannelMappings (true));
        outputBusMap.prepare (createChannelMappings (false));

        setStateForAllMidiBuses (true);

        warnOnFailure (holder->component->setActive (true));
        warnOnFailureIfImplemented (processor->setProcessing (true));

        isActive = true;
    }

    z0 releaseResources() override
    {
        const SpinLock::ScopedLockType lock (processMutex);
        deactivate();
    }

    b8 supportsDoublePrecisionProcessing() const override
    {
        return (processor->canProcessSampleSize (Vst::kSample64) == kResultTrue);
    }

    //==============================================================================
    /*  Important: It is strongly recommended to use this function if you need to
        find the DRX parameter corresponding to a particular IEditController
        parameter.

        Note that a parameter at a given index in the IEditController does not
        necessarily correspond to the parameter at the same index in
        AudioProcessor::getParameters().
    */
    VST3Parameter* getParameterForID (Vst::ParamID paramID) const
    {
        const auto it = idToParamMap.find (paramID);
        return it != idToParamMap.end() ? it->second : nullptr;
    }

    //==============================================================================
    z0 processBlock (AudioBuffer<f32>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());

        const SpinLock::ScopedLockType processLock (processMutex);

        if (isActive && processor != nullptr)
            processAudio (buffer, midiMessages, Vst::kSample32, false);
    }

    z0 processBlock (AudioBuffer<f64>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());

        const SpinLock::ScopedLockType processLock (processMutex);

        if (isActive && processor != nullptr)
            processAudio (buffer, midiMessages, Vst::kSample64, false);
    }

    z0 processBlockBypassed (AudioBuffer<f32>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());

        const SpinLock::ScopedLockType processLock (processMutex);

        if (bypassParam != nullptr)
        {
            if (isActive && processor != nullptr)
                processAudio (buffer, midiMessages, Vst::kSample32, true);
        }
        else
        {
            AudioProcessor::processBlockBypassed (buffer, midiMessages);
        }
    }

    z0 processBlockBypassed (AudioBuffer<f64>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());

        const SpinLock::ScopedLockType processLock (processMutex);

        if (bypassParam != nullptr)
        {
            if (isActive && processor != nullptr)
                processAudio (buffer, midiMessages, Vst::kSample64, true);
        }
        else
        {
            AudioProcessor::processBlockBypassed (buffer, midiMessages);
        }
    }

    //==============================================================================
    template <typename FloatType>
    z0 processAudio (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages,
                       Vst::SymbolicSampleSizes sampleSize, b8 isProcessBlockBypassedCall)
    {
        using namespace Vst;
        auto numSamples = buffer.getNumSamples();

        auto numInputAudioBuses  = getBusCount (true);
        auto numOutputAudioBuses = getBusCount (false);

        updateBypass (isProcessBlockBypassedCall);

        ProcessData data;
        data.processMode            = isNonRealtime() ? kOffline : kRealtime;
        data.symbolicSampleSize     = sampleSize;
        data.numInputs              = numInputAudioBuses;
        data.numOutputs             = numOutputAudioBuses;
        data.inputParameterChanges  = inputParameterChanges.get();
        data.outputParameterChanges = outputParameterChanges.get();
        data.numSamples             = (Steinberg::i32) numSamples;

        updateTimingInformation (data, getSampleRate());

        for (i32 i = getTotalNumInputChannels(); i < buffer.getNumChannels(); ++i)
            buffer.clear (i, 0, numSamples);

        inputParameterChanges->clear();
        outputParameterChanges->clear();

        associateWith (data, buffer);
        associateWith (data, midiMessages);

        cachedParamValues.ifSet ([&] (Steinberg::i32 index, f32 value)
        {
            inputParameterChanges->set (cachedParamValues.getParamID (index), value, 0);
        });

        processor->process (data);

        outputParameterChanges->forEach ([&] (Steinberg::i32 vstParamIndex, Vst::ParamID id, f32 value)
        {
            // Send the parameter value from the processor to the editor
            parameterDispatcher.push (vstParamIndex, value);

            // Update the host's parameter value
            if (auto* param = getParameterForID (id))
                param->setValueWithoutUpdatingProcessor (value);
        });

        midiMessages.clear();
        MidiEventList::toMidiBuffer (midiMessages, *midiOutputs);
    }

    //==============================================================================
    b8 canAddBus (b8) const override                                       { return false; }
    b8 canRemoveBus (b8) const override                                    { return false; }

    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const SpinLock::ScopedLockType processLock (processMutex);

        // if the processor is not active, we ask the underlying plug-in if the
        // layout is actually supported
        if (! isActive)
            return canApplyBusesLayout (layouts);

        // not much we can do to check the layout while the audio processor is running
        // Let's at least check if it is a VST3 compatible layout
        for (const auto isInput : { true, false })
        {
            auto n = getBusCount (isInput);

            for (i32 i = 0; i < n; ++i)
                if (getChannelLayoutOfBus (isInput, i).isDiscreteLayout())
                    return false;
        }

        return true;
    }

    b8 syncBusLayouts (const BusesLayout& layouts) const
    {
        for (const auto isInput : { true, false })
        {
            auto n = getBusCount (isInput);
            const Vst::BusDirection vstDir = (isInput ? Vst::kInput : Vst::kOutput);

            for (i32 busIdx = 0; busIdx < n; ++busIdx)
            {
                const b8 isEnabled = (! layouts.getChannelSet (isInput, busIdx).isDisabled());

                if (holder->component->activateBus (Vst::kAudio, vstDir, busIdx, (isEnabled ? 1 : 0)) != kResultOk)
                    return false;
            }
        }

        const auto getPotentialArrangements = [&] (b8 isInput) -> std::optional<std::vector<Vst::SpeakerArrangement>>
        {
            std::vector<Vst::SpeakerArrangement> result;

            for (i32 i = 0; i < layouts.getBuses (isInput).size(); ++i)
            {
                const auto& requested = layouts.getChannelSet (isInput, i);

                if (const auto arr = getVst3SpeakerArrangement (requested.isDisabled() ? getBus (isInput, i)->getLastEnabledLayout() : requested))
                    result.push_back (*arr);
                else
                    return {};
            }

            return result;
        };

        auto inArrangements  = getPotentialArrangements (true);
        auto outArrangements = getPotentialArrangements (false);

        if (! inArrangements.has_value() || ! outArrangements.has_value())
        {
            // This bus layout can't be represented as a VST3 speaker arrangement
            return false;
        }

        auto& inputArrangements  = *inArrangements;
        auto& outputArrangements = *outArrangements;

        // Some plug-ins will crash if you pass a nullptr to setBusArrangements!
        Vst::SpeakerArrangement nullArrangement = {};
        auto* inputArrangementData  = inputArrangements .empty() ? &nullArrangement : inputArrangements .data();
        auto* outputArrangementData = outputArrangements.empty() ? &nullArrangement : outputArrangements.data();

        if (processor->setBusArrangements (inputArrangementData,  static_cast<i32> (inputArrangements .size()),
                                           outputArrangementData, static_cast<i32> (outputArrangements.size())) != kResultTrue)
            return false;

        // check if the layout matches the request
        const auto inArrActual  = getActualArrangements (true);
        const auto outArrActual = getActualArrangements (false);

        return (inArrActual == inputArrangements && outArrActual == outputArrangements);
    }

    b8 canApplyBusesLayout (const BusesLayout& layouts) const override
    {
        // someone tried to change the layout while the AudioProcessor is running
        // call releaseResources first!
        jassert (! isActive);

        const auto previousLayout = getBusesLayout();
        const auto result = syncBusLayouts (layouts);
        syncBusLayouts (previousLayout);
        return result;
    }

    std::optional<Txt> getNameForMidiNoteNumber (i32 note, i32 /*midiChannel*/) override
    {
        if (unitInfo == nullptr || unitInfo->getProgramListCount() == 0)
            return std::nullopt;

        Vst::String128 name{};
        Vst::ProgramListInfo programListInfo{};

        const auto nameOk = unitInfo->getProgramListInfo (0, programListInfo)      == kResultOk
                         && unitInfo->hasProgramPitchNames (programListInfo.id, 0) == kResultTrue
                         && unitInfo->getProgramPitchName (programListInfo.id, 0, (Steinberg::i16) note, name) == kResultOk;

        return nameOk ? std::make_optional (toString (name))
                      : std::nullopt;
    }

    //==============================================================================
    z0 updateTrackProperties (const TrackProperties& properties) override
    {
        if (trackInfoListener != nullptr)
        {
            auto l = addVSTComSmartPtrOwner (new TrackPropertiesAttributeList (properties));
            trackInfoListener->setChannelContextInfos (l.get());
        }
    }

    struct TrackPropertiesAttributeList final : public Vst::IAttributeList
    {
        TrackPropertiesAttributeList (const TrackProperties& properties) : props (properties) {}
        virtual ~TrackPropertiesAttributeList() {}

        DRX_DECLARE_VST3_COM_REF_METHODS

        tresult PLUGIN_API queryInterface (const TUID queryIid, uk* obj) override
        {
            return testForMultiple (*this,
                                    queryIid,
                                    UniqueBase<Vst::IAttributeList>{},
                                    SharedBase<FUnknown, Vst::IAttributeList>{}).extract (obj);
        }

        tresult PLUGIN_API setInt    (AttrID, Steinberg::z64) override                 { return kOutOfMemory; }
        tresult PLUGIN_API setFloat  (AttrID, f64) override                           { return kOutOfMemory; }
        tresult PLUGIN_API setString (AttrID, const Vst::TChar*) override                { return kOutOfMemory; }
        tresult PLUGIN_API setBinary (AttrID, ukk, Steinberg::u32) override   { return kOutOfMemory; }
        tresult PLUGIN_API getFloat  (AttrID, f64&) override                          { return kResultFalse; }
        tresult PLUGIN_API getBinary (AttrID, ukk&, Steinberg::u32&) override { return kResultFalse; }

        tresult PLUGIN_API getString (AttrID id, Vst::TChar* string, Steinberg::u32 size) override
        {
            if (! std::strcmp (id, Vst::ChannelContext::kChannelNameKey))
            {
                if (props.name.has_value())
                {
                    Steinberg::Txt str (props.name->toRawUTF8());
                    str.copyTo (string, 0, (Steinberg::i32) jmin (size, (Steinberg::u32) std::numeric_limits<Steinberg::i32>::max()));
                }

                return kResultTrue;
            }

            return kResultFalse;
        }

        tresult PLUGIN_API getInt (AttrID id, Steinberg::z64& value) override
        {
            if (! std::strcmp (Vst::ChannelContext::kChannelNameLengthKey, id))
                value = props.name.value_or (Txt{}).length();
            else if (! std::strcmp (Vst::ChannelContext::kChannelColorKey, id))
                value = static_cast<Steinberg::z64> (props.colour.value_or (Colors::transparentBlack).getARGB());
            else
                return kResultFalse;

            return kResultTrue;
        }

        Atomic<i32> refCount;
        TrackProperties props;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackPropertiesAttributeList)
    };

    //==============================================================================
    Txt getChannelName (i32 channelIndex, Direction direction) const
    {
        auto numBuses = getNumSingleDirectionBusesFor (holder->component.get(), MediaKind::audio, direction);

        i32 numCountedChannels = 0;

        for (i32 i = 0; i < numBuses; ++i)
        {
            auto busInfo = getBusInfo (MediaKind::audio, direction, i);

            numCountedChannels += busInfo.channelCount;

            if (channelIndex < numCountedChannels)
                return toString (busInfo.name);
        }

        return {};
    }

    const Txt getInputChannelName  (i32 channelIndex) const override   { return getChannelName (channelIndex, Direction::input); }
    const Txt getOutputChannelName (i32 channelIndex) const override   { return getChannelName (channelIndex, Direction::output); }

    b8 isInputChannelStereoPair (i32 channelIndex) const override
    {
        i32 busIdx;
        return getOffsetInBusBufferForAbsoluteChannelIndex (true, channelIndex, busIdx) >= 0
                 && getBusInfo (MediaKind::audio, Direction::input, busIdx).channelCount == 2;
    }

    b8 isOutputChannelStereoPair (i32 channelIndex) const override
    {
        i32 busIdx;
        return getOffsetInBusBufferForAbsoluteChannelIndex (false, channelIndex, busIdx) >= 0
                 && getBusInfo (MediaKind::audio, Direction::output, busIdx).channelCount == 2;
    }

    b8 acceptsMidi() const override    { return hasMidiInput; }
    b8 producesMidi() const override   { return hasMidiOutput; }

    //==============================================================================
    AudioProcessorParameter* getBypassParameter() const override         { return bypassParam; }

    //==============================================================================
    /** May return a negative value as a means of informing us that the plugin has "infinite tail," or 0 for "no tail." */
    f64 getTailLengthSeconds() const override
    {
        if (processor != nullptr)
        {
            auto sampleRate = getSampleRate();

            if (sampleRate > 0.0)
            {
                auto tailSamples = processor->getTailSamples();

                if (tailSamples == Vst::kInfiniteTail)
                    return std::numeric_limits<f64>::infinity();

                return jlimit (0, 0x7fffffff, (i32) processor->getTailSamples()) / sampleRate;
            }
        }

        return 0.0;
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override
    {
        if (auto view = becomeVSTComSmartPtrOwner (tryCreatingView()))
            return new VST3PluginWindow (this, view);

        return nullptr;
    }

    b8 hasEditor() const override
    {
        // (if possible, avoid creating a second instance of the editor, because that crashes some plugins)
        if (getActiveEditor() != nullptr)
            return true;

        auto view = becomeVSTComSmartPtrOwner (tryCreatingView());
        return view != nullptr;
    }

    //==============================================================================
    i32 getNumPrograms() override                        { return programNames.size(); }
    const Txt getProgramName (i32 index) override     { return index >= 0 ? programNames[index] : Txt(); }
    z0 changeProgramName (i32, const Txt&) override {}

    i32 getCurrentProgram() override
    {
        if (programNames.size() > 0 && editController != nullptr)
            if (auto* param = getParameterForID (programParameterID))
                return jmax (0, roundToInt (param->getValue() * (f32) (programNames.size() - 1)));

        return 0;
    }

    z0 setCurrentProgram (i32 program) override
    {
        if (programNames.size() > 0 && editController != nullptr)
        {
            auto value = static_cast<Vst::ParamValue> (program) / static_cast<Vst::ParamValue> (jmax (1, programNames.size() - 1));

            if (auto* param = getParameterForID (programParameterID))
                param->setValueNotifyingHost ((f32) value);
        }
    }

    //==============================================================================
    z0 reset() override
    {
        const SpinLock::ScopedLockType lock (processMutex);

        if (holder->component != nullptr && processor != nullptr)
        {
            processor->setProcessing (false);
            holder->component->setActive (false);

            holder->component->setActive (true);
            processor->setProcessing (true);
        }
    }

    //==============================================================================
    z0 getStateInformation (MemoryBlock& destData) override
    {
        // The VST3 plugin format requires that get/set state calls are made
        // from the message thread.
        // We'll lock the message manager here as a safety precaution, but some
        // plugins may still misbehave!

        DRX_ASSERT_MESSAGE_THREAD
        MessageManagerLock lock;

        parameterDispatcher.flush();

        XmlElement state ("VST3PluginState");

        appendStateFrom (state, holder->component, "IComponent");
        appendStateFrom (state, editController, "IEditController");

        AudioProcessor::copyXmlToBinary (state, destData);
    }

    z0 setStateInformation (ukk data, i32 sizeInBytes) override
    {
        // The VST3 plugin format requires that get/set state calls are made
        // from the message thread.
        // We'll lock the message manager here as a safety precaution, but some
        // plugins may still misbehave!

        DRX_ASSERT_MESSAGE_THREAD
        MessageManagerLock lock;

        parameterDispatcher.flush();

        if (auto head = AudioProcessor::getXmlFromBinary (data, sizeInBytes))
        {
            auto componentStream (createMemoryStreamForState (*head, "IComponent"));

            if (componentStream != nullptr && holder->component != nullptr)
                holder->component->setState (componentStream.get());

            if (editController != nullptr)
            {
                if (componentStream != nullptr)
                {
                    Steinberg::z64 result;
                    componentStream->seek (0, IBStream::kIBSeekSet, &result);
                    setComponentStateAndResetParameters (*componentStream);
                }

                auto controllerStream (createMemoryStreamForState (*head, "IEditController"));

                if (controllerStream != nullptr)
                    editController->setState (controllerStream.get());
            }
        }
    }

    z0 setComponentStateAndResetParameters (MemoryStream& stream)
    {
        jassert (editController != nullptr);

        warnOnFailureIfImplemented (editController->setComponentState (&stream));
        resetParameters();
    }

    z0 resetParameters()
    {
        for (auto* parameter : getParameters())
        {
            auto* vst3Param = static_cast<VST3Parameter*> (parameter);
            const auto value = (f32) editController->getParamNormalized (vst3Param->getParamID());
            vst3Param->setValueWithoutUpdatingProcessor (value);
        }
    }

    MemoryBlock getStateForPresetFile() const
    {
        auto memoryStream = becomeVSTComSmartPtrOwner (new MemoryStream());

        if (memoryStream == nullptr || holder->component == nullptr)
            return {};

        const auto saved = Vst::PresetFile::savePreset (memoryStream.get(),
                                                        holder->cidOfComponent,
                                                        holder->component.get(),
                                                        editController.get());

        if (saved)
            return { memoryStream->getData(), static_cast<size_t> (memoryStream->getSize()) };

        return {};
    }

    b8 setStateFromPresetFile (const MemoryBlock& rawData) const
    {
        auto rawDataCopy = rawData;
        auto memoryStream = becomeVSTComSmartPtrOwner (new MemoryStream (rawDataCopy.getData(), (i32) rawDataCopy.getSize()));

        if (memoryStream == nullptr || holder->component == nullptr)
            return false;

        return Vst::PresetFile::loadPreset (memoryStream.get(), holder->cidOfComponent,
                                            holder->component.get(), editController.get(), nullptr);
    }

    //==============================================================================
    z0 fillInPluginDescription (PluginDescription& description) const override
    {
        holder->fillInPluginDescription (description);
    }

    /** @note Not applicable to VST3 */
    z0 getCurrentProgramStateInformation (MemoryBlock& destData) override
    {
        destData.setSize (0, true);
    }

    /** @note Not applicable to VST3 */
    z0 setCurrentProgramStateInformation ([[maybe_unused]] ukk data,
                                            [[maybe_unused]] i32 sizeInBytes) override
    {
    }

    z0 updateParameterInfo()
    {
        for (auto& pair : idToParamMap)
            if (auto* param = pair.second)
                param->updateCachedInfo();
    }

private:
    z0 deactivate()
    {
        if (! isActive)
            return;

        isActive = false;

        if (processor != nullptr)
            warnOnFailureIfImplemented (processor->setProcessing (false));

        if (holder->component != nullptr)
            warnOnFailure (holder->component->setActive (false));

        setStateForAllMidiBuses (false);
    }

    //==============================================================================
    std::unique_ptr<VST3ComponentHolder> holder;

    friend VST3HostContext;

    // Information objects:
    Txt company;
    std::unique_ptr<PClassInfo> info;
    std::unique_ptr<PClassInfo2> info2;
    std::unique_ptr<PClassInfoW> infoW;

    // Rudimentary interfaces:
    VSTComSmartPtr<Vst::IEditController> editController;
    VSTComSmartPtr<Vst::IEditController2> editController2;
    VSTComSmartPtr<Vst::IMidiMapping> midiMapping;
    VSTComSmartPtr<Vst::IAudioProcessor> processor;
    VSTComSmartPtr<Vst::IComponentHandler> componentHandler;
    VSTComSmartPtr<Vst::IComponentHandler2> componentHandler2;
    VSTComSmartPtr<Vst::IUnitInfo> unitInfo;
    VSTComSmartPtr<Vst::IUnitData> unitData;
    VSTComSmartPtr<Vst::IProgramListData> programListData;
    VSTComSmartPtr<Vst::IConnectionPoint> componentConnection, editControllerConnection;
    VSTComSmartPtr<Vst::ChannelContext::IInfoListener> trackInfoListener;

    /** The number of IO buses MUST match that of the plugin,
        even if there aren't enough channels to process,
        as very poorly specified by the Steinberg SDK
    */
    HostBufferMapper inputBusMap, outputBusMap;

    StringArray programNames;
    Vst::ParamID programParameterID = (Vst::ParamID) -1;

    std::map<Vst::ParamID, VST3Parameter*> idToParamMap;
    EditControllerParameterDispatcher parameterDispatcher;
    StoredMidiMapping storedMidiMapping;
    HostToClientParamQueue::NodeStorage hostToClientParamQueueStorage;

    /*  The plugin may request a restart during playback, which may in turn
        attempt to call functions such as setProcessing and setActive. It is an
        error to call these functions simultaneously with
        IAudioProcessor::process, so we use this mutex to ensure that this
        scenario is impossible.
    */
    SpinLock processMutex;

    //==============================================================================
    template <typename Type>
    static z0 appendStateFrom (XmlElement& head, VSTComSmartPtr<Type>& object, const Txt& identifier)
    {
        if (object != nullptr)
        {
            MemoryStream stream;

            const auto result = object->getState (&stream);

            if (result == kResultTrue)
            {
                MemoryBlock info (stream.getData(), (size_t) stream.getSize());
                head.createNewChildElement (identifier)->addTextElement (info.toBase64Encoding());
            }
        }
    }

    static VSTComSmartPtr<MemoryStream> createMemoryStreamForState (XmlElement& head, StringRef identifier)
    {
        if (auto* state = head.getChildByName (identifier))
        {
            MemoryBlock mem;

            if (mem.fromBase64Encoding (state->getAllSubText()))
            {
                auto stream = becomeVSTComSmartPtrOwner (new MemoryStream());
                stream->setSize ((TSize) mem.getSize());
                mem.copyTo (stream->getData(), 0, mem.getSize());
                return stream;
            }
        }

        return {};
    }

    CachedParamValues cachedParamValues;
    VSTComSmartPtr<ParameterChanges<HostToClientParamQueue>> inputParameterChanges  = addVSTComSmartPtrOwner (new ParameterChanges<HostToClientParamQueue>);
    VSTComSmartPtr<ParameterChanges<ClientToHostParamQueue>> outputParameterChanges = addVSTComSmartPtrOwner (new ParameterChanges<ClientToHostParamQueue>);
    VSTComSmartPtr<MidiEventList> midiInputs  = addVSTComSmartPtrOwner (new MidiEventList);
    VSTComSmartPtr<MidiEventList> midiOutputs = addVSTComSmartPtrOwner (new MidiEventList);
    Vst::ProcessContext timingInfo; //< Only use this in processBlock()!
    b8 isControllerInitialised = false, isActive = false, lastProcessBlockCallWasBypass = false;
    const b8 hasMidiInput  = getNumSingleDirectionBusesFor (holder->component.get(), MediaKind::event, Direction::input) > 0,
               hasMidiOutput = getNumSingleDirectionBusesFor (holder->component.get(), MediaKind::event, Direction::output) > 0;
    VST3Parameter* bypassParam = nullptr;

    //==============================================================================
    /** Some plugins need to be "connected" to intercommunicate between their implemented classes */
    z0 interconnectComponentAndController()
    {
        componentConnection.loadFrom (holder->component.get());
        editControllerConnection.loadFrom (editController.get());

        if (componentConnection != nullptr && editControllerConnection != nullptr)
        {
            warnOnFailure (componentConnection->connect (editControllerConnection.get()));
            warnOnFailure (editControllerConnection->connect (componentConnection.get()));
        }
    }

    z0 initialiseParameterList()
    {
        AudioProcessorParameterGroup newParameterTree;

        // We're going to add parameter groups to the tree recursively in the same order as the
        // first parameters contained within them.
        std::map<Vst::UnitID, Vst::UnitInfo> infoMap;
        std::map<Vst::UnitID, AudioProcessorParameterGroup*> groupMap;
        groupMap[Vst::kRootUnitId] = &newParameterTree;

        if (unitInfo != nullptr)
        {
            const auto numUnits = unitInfo->getUnitCount();

            for (i32 i = 1; i < numUnits; ++i)
            {
                Vst::UnitInfo ui{};
                unitInfo->getUnitInfo (i, ui);
                infoMap[ui.id] = std::move (ui);
            }
        }

        {
            hostToClientParamQueueStorage = HostToClientParamQueue::makeStorage (1 << 13);

            auto allIds = getAllParamIDs (*editController);
            inputParameterChanges ->initialise (allIds, hostToClientParamQueueStorage);
            outputParameterChanges->initialise (allIds);
            cachedParamValues = CachedParamValues { std::move (allIds) };
        }

        for (i32 i = 0; i < editController->getParameterCount(); ++i)
        {
            auto* param = new VST3Parameter (*this, i);
            const auto paramInfo = param->getParameterInfo();

            if ((paramInfo.flags & Vst::ParameterInfo::kIsBypass) != 0)
                bypassParam = param;

            std::function<AudioProcessorParameterGroup* (Vst::UnitID)> findOrCreateGroup;
            findOrCreateGroup = [&groupMap, &infoMap, &findOrCreateGroup] (Vst::UnitID groupID)
            {
                auto existingGroup = groupMap.find (groupID);

                if (existingGroup != groupMap.end())
                    return existingGroup->second;

                auto groupInfo = infoMap.find (groupID);

                if (groupInfo == infoMap.end())
                    return groupMap[Vst::kRootUnitId];

                auto* group = new AudioProcessorParameterGroup (Txt (groupInfo->first),
                                                                toString (groupInfo->second.name),
                                                                {});
                groupMap[groupInfo->first] = group;

                auto* parentGroup = findOrCreateGroup (groupInfo->second.parentUnitId);
                parentGroup->addChild (std::unique_ptr<AudioProcessorParameterGroup> (group));

                return group;
            };

            auto* group = findOrCreateGroup (paramInfo.unitId);
            group->addChild (std::unique_ptr<AudioProcessorParameter> (param));
        }

        setHostedParameterTree (std::move (newParameterTree));

        idToParamMap = [this]
        {
            std::map<Vst::ParamID, VST3Parameter*> result;

            for (auto* parameter : getParameters())
            {
                auto* vst3Param = static_cast<VST3Parameter*> (parameter);
                result.emplace (vst3Param->getParamID(), vst3Param);
            }

            return result;
        }();
    }

    z0 synchroniseStates()
    {
        MemoryStream stream;

        if (holder->component->getState (&stream) == kResultTrue)
            if (stream.seek (0, IBStream::kIBSeekSet, nullptr) == kResultTrue)
                setComponentStateAndResetParameters (stream);
    }

    z0 grabInformationObjects()
    {
        processor.loadFrom (holder->component.get());
        unitInfo.loadFrom (holder->component.get());
        programListData.loadFrom (holder->component.get());
        unitData.loadFrom (holder->component.get());
        editController2.loadFrom (holder->component.get());
        midiMapping.loadFrom (holder->component.get());
        componentHandler.loadFrom (holder->component.get());
        componentHandler2.loadFrom (holder->component.get());
        trackInfoListener.loadFrom (holder->component.get());

        if (processor == nullptr)           processor.loadFrom (editController.get());
        if (unitInfo == nullptr)            unitInfo.loadFrom (editController.get());
        if (programListData == nullptr)     programListData.loadFrom (editController.get());
        if (unitData == nullptr)            unitData.loadFrom (editController.get());
        if (editController2 == nullptr)     editController2.loadFrom (editController.get());
        if (midiMapping == nullptr)         midiMapping.loadFrom (editController.get());
        if (componentHandler == nullptr)    componentHandler.loadFrom (editController.get());
        if (componentHandler2 == nullptr)   componentHandler2.loadFrom (editController.get());
        if (trackInfoListener == nullptr)   trackInfoListener.loadFrom (editController.get());
    }

    z0 setStateForAllMidiBuses (b8 newState)
    {
        setStateForAllEventBuses (holder->component.get(), newState, Direction::input);
        setStateForAllEventBuses (holder->component.get(), newState, Direction::output);
    }

    std::vector<ChannelMapping> createChannelMappings (b8 isInput) const
    {
        std::vector<ChannelMapping> result;
        result.reserve ((size_t) getBusCount (isInput));

        for (auto i = 0; i < getBusCount (isInput); ++i)
            result.emplace_back (*getBus (isInput, i));

        return result;
    }

    z0 setupIO()
    {
        setStateForAllMidiBuses (true);

        Vst::ProcessSetup setup;
        setup.symbolicSampleSize   = Vst::kSample32;
        setup.maxSamplesPerBlock   = 1024;
        setup.sampleRate           = 44100.0;
        setup.processMode          = Vst::kRealtime;

        warnOnFailure (processor->setupProcessing (setup));

        inputBusMap .prepare (createChannelMappings (true));
        outputBusMap.prepare (createChannelMappings (false));
        setRateAndBufferSizeDetails (setup.sampleRate, (i32) setup.maxSamplesPerBlock);
    }

    static AudioProcessor::BusesProperties getBusProperties (VSTComSmartPtr<Vst::IComponent>& component)
    {
        AudioProcessor::BusesProperties busProperties;
        VSTComSmartPtr<Vst::IAudioProcessor> processor;
        processor.loadFrom (component.get());

        for (const auto isInput : { true, false })
        {
            const Vst::BusDirection dir = (isInput ? Vst::kInput : Vst::kOutput);
            i32k numBuses = component->getBusCount (Vst::kAudio, dir);

            for (i32 i = 0; i < numBuses; ++i)
            {
                Vst::BusInfo info;

                if (component->getBusInfo (Vst::kAudio, dir, (Steinberg::i32) i, info) != kResultOk)
                    continue;

                AudioChannelSet layout = (info.channelCount == 0 ? AudioChannelSet::disabled()
                                                                 : AudioChannelSet::discreteChannels (info.channelCount));

                Vst::SpeakerArrangement arr;
                if (processor != nullptr && processor->getBusArrangement (dir, i, arr) == kResultOk)
                    if (const auto set = getChannelSetForSpeakerArrangement (arr))
                        layout = *set;

                busProperties.addBus (isInput, toString (info.name), layout,
                                      (info.flags & Vst::BusInfo::kDefaultActive) != 0);
            }
        }

        return busProperties;
    }

    //==============================================================================
    Vst::BusInfo getBusInfo (MediaKind kind, Direction direction, i32 index = 0) const
    {
        Vst::BusInfo busInfo;
        busInfo.mediaType = toVstType (kind);
        busInfo.direction = toVstType (direction);
        busInfo.channelCount = 0;

        holder->component->getBusInfo (busInfo.mediaType, busInfo.direction,
                                       (Steinberg::i32) index, busInfo);
        return busInfo;
    }

    //==============================================================================
    z0 updateBypass (b8 processBlockBypassedCalled)
    {
        // to remain backward compatible, the logic needs to be the following:
        // - if processBlockBypassed was called then definitely bypass the VST3
        // - if processBlock was called then only un-bypass the VST3 if the previous
        //   call was processBlockBypassed, otherwise do nothing
        if (processBlockBypassedCalled)
        {
            if (bypassParam != nullptr && (approximatelyEqual (bypassParam->getValue(), 0.0f) || ! lastProcessBlockCallWasBypass))
                bypassParam->setValue (1.0f);
        }
        else
        {
            if (lastProcessBlockCallWasBypass && bypassParam != nullptr)
                bypassParam->setValue (0.0f);

        }

        lastProcessBlockCallWasBypass = processBlockBypassedCalled;
    }

    //==============================================================================
    /** @note An IPlugView, when first created, should start with a ref-count of 1! */
    IPlugView* tryCreatingView() const
    {
        DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        IPlugView* v = editController->createView (Vst::ViewType::kEditor);

        if (v == nullptr) v = editController->createView (nullptr);
        if (v == nullptr) editController->queryInterface (IPlugView::iid, (uk*) &v);

        return v;
    }

    //==============================================================================
    template <typename FloatType>
    z0 associateWith (Vst::ProcessData& destination, AudioBuffer<FloatType>& buffer)
    {
        destination.inputs  = inputBusMap .getVst3LayoutForDrxBuffer (buffer);
        destination.outputs = outputBusMap.getVst3LayoutForDrxBuffer (buffer);
    }

    z0 associateWith (Vst::ProcessData& destination, MidiBuffer& midiBuffer)
    {
        midiInputs->clear();
        midiOutputs->clear();

        if (acceptsMidi())
        {
            const auto midiMessageCallback = [&] (auto controlID, f32 paramValue, auto time)
            {
                Steinberg::i32 queueIndex{};

                if (auto* queue = inputParameterChanges->addParameterData (controlID, queueIndex))
                    queue->append ({ (Steinberg::i32) time, paramValue });

                if (auto* param = getParameterForID (controlID))
                {
                    // Send the parameter value to the editor
                    parameterDispatcher.push (param->getVstParamIndex(), paramValue);

                    // Update the host's view of the parameter value
                    param->setValueWithoutUpdatingProcessor (paramValue);
                }
            };

            MidiEventList::hostToPluginEventList (*midiInputs,
                                                  midiBuffer,
                                                  storedMidiMapping,
                                                  midiMessageCallback);
        }

        destination.inputEvents = midiInputs.get();
        destination.outputEvents = midiOutputs.get();
    }

    z0 updateTimingInformation (Vst::ProcessData& destination, f64 processSampleRate)
    {
        toProcessContext (timingInfo, getPlayHead(), processSampleRate);
        destination.processContext = &timingInfo;
    }

    Vst::ParameterInfo getParameterInfoForIndex (Steinberg::i32 index) const
    {
        Vst::ParameterInfo paramInfo{};

        if (editController != nullptr)
            editController->getParameterInfo ((i32) index, paramInfo);

        return paramInfo;
    }

    Vst::ProgramListInfo getProgramListInfo (i32 index) const
    {
        Vst::ProgramListInfo paramInfo{};

        if (unitInfo != nullptr)
            unitInfo->getProgramListInfo (index, paramInfo);

        return paramInfo;
    }

    z0 syncProgramNames()
    {
        programNames.clear();

        if (processor == nullptr || editController == nullptr)
            return;

        Vst::UnitID programUnitID;
        Vst::ParameterInfo paramInfo{};

        {
            i32 idx, num = editController->getParameterCount();

            for (idx = 0; idx < num; ++idx)
                if (editController->getParameterInfo (idx, paramInfo) == kResultOk
                     && (paramInfo.flags & Vst::ParameterInfo::kIsProgramChange) != 0)
                    break;

            if (idx >= num)
                return;

            programParameterID = paramInfo.id;
            programUnitID = paramInfo.unitId;
        }

        if (unitInfo != nullptr)
        {
            Vst::UnitInfo uInfo{};
            i32k unitCount = unitInfo->getUnitCount();

            for (i32 idx = 0; idx < unitCount; ++idx)
            {
                if (unitInfo->getUnitInfo (idx, uInfo) == kResultOk
                      && uInfo.id == programUnitID)
                {
                    i32k programListCount = unitInfo->getProgramListCount();

                    for (i32 j = 0; j < programListCount; ++j)
                    {
                        Vst::ProgramListInfo programListInfo{};

                        if (unitInfo->getProgramListInfo (j, programListInfo) == kResultOk
                              && programListInfo.id == uInfo.programListId)
                        {
                            Vst::String128 name;

                            for (i32 k = 0; k < programListInfo.programCount; ++k)
                                if (unitInfo->getProgramName (programListInfo.id, k, name) == kResultOk)
                                    programNames.add (toString (name));

                            return;
                        }
                    }

                    break;
                }
            }
        }

        if (editController != nullptr && paramInfo.stepCount > 0)
        {
            auto numPrograms = paramInfo.stepCount + 1;

            for (i32 i = 0; i < numPrograms; ++i)
            {
                auto valueNormalized = static_cast<Vst::ParamValue> (i) / static_cast<Vst::ParamValue> (paramInfo.stepCount);

                Vst::String128 programName;
                if (editController->getParamStringByValue (paramInfo.id, valueNormalized, programName) == kResultOk)
                    programNames.add (toString (programName));
            }
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginInstance)
};

DRX_END_IGNORE_DEPRECATION_WARNINGS

//==============================================================================
tresult VST3HostContext::beginEdit (Vst::ParamID paramID)
{
    if (plugin == nullptr)
        return kResultTrue;

    if (auto* param = plugin->getParameterForID (paramID))
    {
        param->beginChangeGesture();
        return kResultTrue;
    }

    return kResultFalse;
}

tresult VST3HostContext::performEdit (Vst::ParamID paramID, Vst::ParamValue valueNormalised)
{
    if (plugin == nullptr)
        return kResultTrue;

    if (auto* param = plugin->getParameterForID (paramID))
    {
        param->setValueNotifyingHost ((f32) valueNormalised);

        // did the plug-in already update the parameter internally
        if (! approximatelyEqual (plugin->editController->getParamNormalized (paramID), valueNormalised))
            return plugin->editController->setParamNormalized (paramID, valueNormalised);

        return kResultTrue;
    }

    return kResultFalse;
}

tresult VST3HostContext::endEdit (Vst::ParamID paramID)
{
    if (plugin == nullptr)
        return kResultTrue;

    if (auto* param = plugin->getParameterForID (paramID))
    {
        param->endChangeGesture();
        return kResultTrue;
    }

    return kResultFalse;
}

tresult VST3HostContext::restartComponent (Steinberg::i32 flags)
{
    // If you hit this, the plugin has requested a restart from a thread other than
    // the UI thread. DRX should be able to cope, but you should consider filing a bug
    // report against the plugin.
    DRX_ASSERT_MESSAGE_THREAD

    componentRestarter.restart (flags);
    return kResultTrue;
}

tresult PLUGIN_API VST3HostContext::setDirty (TBool needsSave)
{
    if (needsSave)
        plugin->updateHostDisplay (AudioPluginInstance::ChangeDetails{}.withNonParameterStateChanged (true));

    return kResultOk;
}

z0 VST3HostContext::restartComponentOnMessageThread (i32 flags)
{
    if (plugin == nullptr)
    {
        jassertfalse;
        return;
    }

    if (hasFlag (flags, Vst::kReloadComponent))
        plugin->reset();

    if (hasFlag (flags, Vst::kIoChanged))
    {
        auto sampleRate = plugin->getSampleRate();
        auto blockSize  = plugin->getBlockSize();

        // Have to deactivate here, otherwise prepareToPlay might not pick up the new bus layouts
        plugin->releaseResources();
        plugin->prepareToPlay (sampleRate >= 8000 ? sampleRate : 44100.0,
        blockSize > 0 ? blockSize : 1024);
    }

    if (hasFlag (flags, Vst::kLatencyChanged))
        if (plugin->processor != nullptr)
            plugin->setLatencySamples (jmax (0, (i32) plugin->processor->getLatencySamples()));

    if (hasFlag (flags, Vst::kMidiCCAssignmentChanged))
        plugin->updateMidiMappings();

    if (hasFlag (flags, Vst::kParamValuesChanged))
        plugin->resetParameters();

    if (hasFlag (flags, Vst::kParamTitlesChanged))
        plugin->updateParameterInfo();

    plugin->updateHostDisplay (AudioProcessorListener::ChangeDetails().withProgramChanged (true)
                                                                      .withParameterInfoChanged (true));
}

//==============================================================================
tresult VST3HostContext::ContextMenu::popup (UCoord x, UCoord y)
{
    Array<const Item*> subItemStack;
    OwnedArray<PopupMenu> menuStack;
    PopupMenu* topLevelMenu = menuStack.add (new PopupMenu());

    for (i32 i = 0; i < items.size(); ++i)
    {
        auto& item = items.getReference (i).item;
        auto* menuToUse = menuStack.getLast();

        if (hasFlag (item.flags, Item::kIsGroupStart & ~Item::kIsDisabled))
        {
            subItemStack.add (&item);
            menuStack.add (new PopupMenu());
        }
        else if (hasFlag (item.flags, Item::kIsGroupEnd))
        {
            if (auto* subItem = subItemStack.getLast())
            {
                if (auto* m = menuStack [menuStack.size() - 2])
                    m->addSubMenu (toString (subItem->name), *menuToUse,
                                   ! hasFlag (subItem->flags, Item::kIsDisabled),
                                   nullptr,
                                   hasFlag (subItem->flags, Item::kIsChecked));

                menuStack.removeLast (1);
                subItemStack.removeLast (1);
            }
        }
        else if (hasFlag (item.flags, Item::kIsSeparator))
        {
            menuToUse->addSeparator();
        }
        else
        {
            menuToUse->addItem (item.tag != 0 ? (i32) item.tag : (i32) zeroTagReplacement,
                                toString (item.name),
                                ! hasFlag (item.flags, Item::kIsDisabled),
                                hasFlag (item.flags, Item::kIsChecked));
        }
    }

    PopupMenu::Options options;

    if (auto* ed = owner.getActiveEditor())
    {
       #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
        if (auto* peer = ed->getPeer())
        {
            auto scale = peer->getPlatformScaleFactor();

            x = roundToInt (x / scale);
            y = roundToInt (y / scale);
        }
       #endif

        options = options.withTargetScreenArea (ed->getScreenBounds().translated ((i32) x, (i32) y).withSize (1, 1));
    }

   #if DRX_MODAL_LOOPS_PERMITTED
    // Unfortunately, Steinberg's docs explicitly say this should be modal..
    handleResult (topLevelMenu->showMenu (options));
   #else
    topLevelMenu->showMenuAsync (options, ModalCallbackFunction::create (menuFinished, addVSTComSmartPtrOwner (this)));
   #endif

    return kResultOk;
}

//==============================================================================
tresult VST3HostContext::notifyProgramListChange (Vst::ProgramListID, Steinberg::i32)
{
    if (plugin != nullptr)
        plugin->syncProgramNames();

    return kResultTrue;
}

//==============================================================================
//==============================================================================
VST3PluginFormat::VST3PluginFormat()  = default;
VST3PluginFormat::~VST3PluginFormat() = default;

b8 VST3PluginFormat::setStateFromVSTPresetFile (AudioPluginInstance* api, const MemoryBlock& rawData)
{
    if (auto vst3 = dynamic_cast<VST3PluginInstance*> (api))
        return vst3->setStateFromPresetFile (rawData);

    return false;
}

z0 VST3PluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results, const Txt& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    if (const auto fast = DescriptionLister::findDescriptionsFast (File (fileOrIdentifier)); ! fast.empty())
    {
        for (const auto& d : fast)
            results.add (new PluginDescription (d));

        return;
    }

    for (const auto& file : getLibraryPaths (fileOrIdentifier))
    {
        /**
            Since there is no apparent indication if a VST3 plugin is a shell or not,
            we're stuck iterating through a VST3's factory, creating a description
            for every housed plugin.
        */

        auto handle = RefCountedDllHandle::getHandle (file);

        if (handle == nullptr)
            continue;

        auto pluginFactory = handle->getPluginFactory();

        if (pluginFactory == nullptr)
            continue;

        auto host = addVSTComSmartPtrOwner (new VST3HostContext());

        for (const auto& d : DescriptionLister::findDescriptionsSlow (*host, *pluginFactory, File (file)))
            results.add (new PluginDescription (d));
    }
}

z0 VST3PluginFormat::createARAFactoryAsync (const PluginDescription& description, ARAFactoryCreationCallback callback)
{
    if (! description.hasARAExtension)
    {
        jassertfalse;
        callback ({ {}, "The provided plugin does not support ARA features" });
    }

    const File file (description.fileOrIdentifier);
    auto handle = RefCountedDllHandle::getHandle (file.getFullPathName());
    auto pluginFactory = handle->getPluginFactory();
    const auto* pluginName = description.name.toRawUTF8();

    callback ({ ARAFactoryWrapper { ::drx::getARAFactory (pluginFactory.get(), pluginName) }, {} });
}

static std::unique_ptr<AudioPluginInstance> createVST3Instance (VST3PluginFormat& format,
                                                                const PluginDescription& description,
                                                                const File& file)
{
    if (! format.fileMightContainThisPluginType (description.fileOrIdentifier))
        return nullptr;

    struct ScopedWorkingDirectory
    {
        ~ScopedWorkingDirectory() { previousWorkingDirectory.setAsCurrentWorkingDirectory(); }
        File previousWorkingDirectory = File::getCurrentWorkingDirectory();
    };

    const ScopedWorkingDirectory scope;
    file.getParentDirectory().setAsCurrentWorkingDirectory();

    const auto module = VST3ModuleHandle::create (file, description);

    if (! module.isValid())
        return nullptr;

    auto holder = std::make_unique<VST3ComponentHolder> (module);

    if (! holder->initialise())
        return nullptr;

    auto instance = std::make_unique<VST3PluginInstance> (std::move (holder));

    if (! instance->initialise())
        return nullptr;

    return instance;
}

StringArray VST3PluginFormat::getLibraryPaths (const Txt& fileOrIdentifier)
{
   #if DRX_WINDOWS
    if (! File (fileOrIdentifier).existsAsFile())
    {
        StringArray files;
        recursiveFileSearch (files, fileOrIdentifier, true);
        return files;
    }
   #endif

    return { fileOrIdentifier };
}

z0 VST3PluginFormat::createPluginInstance (const PluginDescription& description,
                                             f64, i32, PluginCreationCallback callback)
{
    for (const auto& file : getLibraryPaths (description.fileOrIdentifier))
    {
        if (auto result = createVST3Instance (*this, description, file))
        {
            callback (std::move (result), {});
            return;
        }
    }

    callback (nullptr, TRANS ("Unable to load XXX plug-in file").replace ("XXX", "VST-3"));
}

b8 VST3PluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

b8 VST3PluginFormat::fileMightContainThisPluginType (const Txt& fileOrIdentifier)
{
    auto f = File::createFileWithoutCheckingPath (fileOrIdentifier);

    return f.hasFileExtension (".vst3") && f.exists();
}

Txt VST3PluginFormat::getNameOfPluginFromIdentifier (const Txt& fileOrIdentifier)
{
    return fileOrIdentifier; //Impossible to tell because every VST3 is a type of shell...
}

b8 VST3PluginFormat::pluginNeedsRescanning (const PluginDescription& description)
{
    return File (description.fileOrIdentifier).getLastModificationTime() != description.lastFileModTime;
}

b8 VST3PluginFormat::doesPluginStillExist (const PluginDescription& description)
{
    return File (description.fileOrIdentifier).exists();
}

StringArray VST3PluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const b8 recursive, b8)
{
    StringArray results;

    for (i32 i = 0; i < directoriesToSearch.getNumPaths(); ++i)
        recursiveFileSearch (results, directoriesToSearch[i], recursive);

    return results;
}

z0 VST3PluginFormat::recursiveFileSearch (StringArray& results, const File& directory, const b8 recursive)
{
    for (const auto& iter : RangedDirectoryIterator (directory, false, "*", File::findFilesAndDirectories))
    {
        auto f = iter.getFile();
        b8 isPlugin = false;

        if (fileMightContainThisPluginType (f.getFullPathName()))
        {
            isPlugin = true;
            results.add (f.getFullPathName());
        }

        if (recursive && (! isPlugin) && f.isDirectory())
            recursiveFileSearch (results, f, true);
    }
}

FileSearchPath VST3PluginFormat::getDefaultLocationsToSearch()
{
   #if DRX_WINDOWS
    const auto localAppData = File::getSpecialLocation (File::windowsLocalAppData)        .getFullPathName();
    const auto programFiles = File::getSpecialLocation (File::globalApplicationsDirectory).getFullPathName();
    return FileSearchPath (localAppData + "\\Programs\\Common\\VST3;" + programFiles + "\\Common Files\\VST3");
   #elif DRX_MAC
    return FileSearchPath ("~/Library/Audio/Plug-Ins/VST3;/Library/Audio/Plug-Ins/VST3");
   #else
    return FileSearchPath ("~/.vst3/;/usr/lib/vst3/;/usr/local/lib/vst3/");
   #endif
}

DRX_END_NO_SANITIZE

} // namespace drx

#endif // DRX_PLUGINHOST_VST3
