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

#if DRX_PLUGINHOST_LV2 && (! (DRX_ANDROID || DRX_IOS))

#include "drx_LV2Common.h"
#include "drx_LV2Resources.h"

#include <drx_gui_extra/native/drx_NSViewFrameWatcher_mac.h>

#include <thread>

namespace drx
{
namespace lv2_host
{

template <typename Struct, typename Value>
auto with (Struct s, Value Struct::* member, Value value) noexcept
{
    s.*member = std::move (value);
    return s;
}

/*  Converts a uk to an LV2_Atom* if the buffer looks like it holds a well-formed Atom, or
    returns nullptr otherwise.
*/
static const LV2_Atom* convertToAtomPtr (ukk ptr, size_t size)
{
    if (size < sizeof (LV2_Atom))
    {
        jassertfalse;
        return nullptr;
    }

    const auto header = readUnaligned<LV2_Atom> (ptr);

    if (size < header.size + sizeof (LV2_Atom))
    {
        jassertfalse;
        return nullptr;
    }

    // This is UB _if_ the ptr doesn't really point to an LV2_Atom.
    return reinterpret_cast<const LV2_Atom*> (ptr);
}

// Allows mutable access to the items in a vector, without allowing the vector itself
// to be modified.
template <typename T>
class SimpleSpan
{
public:
    constexpr SimpleSpan (T* beginIn, T* endIn) : b (beginIn), e (endIn) {}

    constexpr auto begin() const { return b; }
    constexpr auto end()   const { return e; }

    DRX_BEGIN_IGNORE_WARNINGS_MSVC (4814)
    constexpr auto& operator[] (size_t index)       { return b[index]; }
    DRX_END_IGNORE_WARNINGS_MSVC

    constexpr auto size() const { return (size_t) (e - b); }

private:
    T* b;
    T* e;
};

template <typename T>
constexpr auto makeSimpleSpan (T* b, T* e) { return SimpleSpan<T> { b, e }; }

template <typename R>
constexpr auto makeSimpleSpan (R& r) { return makeSimpleSpan (r.data(), r.data() + r.size()); }

struct PhysicalResizeListener
{
    virtual ~PhysicalResizeListener() = default;
    virtual z0 viewRequestedResizeInPhysicalPixels (i32 width, i32 height) = 0;
};

struct LogicalResizeListener
{
    virtual ~LogicalResizeListener() = default;
    virtual z0 viewRequestedResizeInLogicalPixels (i32 width, i32 height) = 0;
};

#if DRX_WINDOWS
class WindowSizeChangeDetector
{
public:
    WindowSizeChangeDetector()
        : hook (SetWindowsHookEx (WH_CALLWNDPROC,
                                  callWndProc,
                                  (HINSTANCE) drx::Process::getCurrentModuleInstanceHandle(),
                                  GetCurrentThreadId()))
    {}

    ~WindowSizeChangeDetector() noexcept
    {
        UnhookWindowsHookEx (hook);
    }

    static z0 addListener (HWND hwnd, PhysicalResizeListener& listener)
    {
        getActiveEditors().emplace (hwnd, &listener);
    }

    static z0 removeListener (HWND hwnd)
    {
        getActiveEditors().erase (hwnd);
    }

private:
    static std::map<HWND, PhysicalResizeListener*>& getActiveEditors()
    {
        static std::map<HWND, PhysicalResizeListener*> map;
        return map;
    }

    static z0 processMessage (i32 nCode, const CWPSTRUCT* info)
    {
        if (nCode < 0 || info == nullptr)
            return;

        constexpr UINT events[] { WM_SIZING, WM_SIZE, WM_WINDOWPOSCHANGING, WM_WINDOWPOSCHANGED };

        if (std::find (std::begin (events), std::end (events), info->message) == std::end (events))
            return;

        auto& map = getActiveEditors();
        auto iter = map.find (info->hwnd);

        if (iter == map.end())
            return;

        RECT rect;
        GetWindowRect (info->hwnd, &rect);
        iter->second->viewRequestedResizeInPhysicalPixels (rect.right - rect.left, rect.bottom - rect.top);
    }

    static LRESULT CALLBACK callWndProc (i32 nCode, WPARAM wParam, LPARAM lParam)
    {
        processMessage (nCode, lv2_shared::wordCast<CWPSTRUCT*> (lParam));
        return CallNextHookEx ({}, nCode, wParam, lParam);
    }

    HHOOK hook;
};

class WindowSizeChangeListener
{
public:
    WindowSizeChangeListener (HWND hwndIn, PhysicalResizeListener& l)
        : hwnd (hwndIn)
    {
        detector->addListener (hwnd, l);
    }

    ~WindowSizeChangeListener()
    {
        detector->removeListener (hwnd);
    }

private:
    SharedResourcePointer<WindowSizeChangeDetector> detector;
    HWND hwnd;

    DRX_LEAK_DETECTOR (WindowSizeChangeListener)
};
#endif

struct FreeLilvNode
{
    z0 operator() (LilvNode* ptr) const noexcept { lilv_node_free (ptr); }
};

using OwningNode = std::unique_ptr<LilvNode, FreeLilvNode>;

template <typename Traits>
class TypesafeLilvNode
{
public:
    template <typename... Ts>
    TypesafeLilvNode (Ts&&... ts)
        : node (Traits::construct (std::forward<Ts> (ts)...)) {}

    b8 equals (const TypesafeLilvNode& other) const noexcept
    {
        return lilv_node_equals (node.get(), other.node.get());
    }

    const LilvNode* get() const noexcept { return node.get(); }

    auto getTyped() const noexcept -> decltype (Traits::access (nullptr))
    {
        return Traits::access (node.get());
    }

    static TypesafeLilvNode claim (LilvNode* node)
    {
        return TypesafeLilvNode { node };
    }

    static TypesafeLilvNode copy (const LilvNode* node)
    {
        return TypesafeLilvNode { lilv_node_duplicate (node) };
    }

private:
    explicit TypesafeLilvNode (LilvNode* ptr)
        : node (ptr)
    {
        jassert (ptr == nullptr || Traits::verify (node.get()));
    }

    OwningNode node;

    DRX_LEAK_DETECTOR (TypesafeLilvNode)
};

struct UriConstructorTrait
{
    static LilvNode* construct (LilvWorld* world, tukk uri) noexcept
    {
        return lilv_new_uri (world, uri);
    }

    static LilvNode* construct (LilvWorld* world, tukk host, tukk path) noexcept
    {
        return lilv_new_file_uri (world, host, path);
    }

    static constexpr auto verify = lilv_node_is_uri;
    static constexpr auto access = lilv_node_as_uri;
};

struct StringConstructorTrait { static constexpr auto construct  = lilv_new_string;
                                static constexpr auto verify     = lilv_node_is_string;
                                static constexpr auto access     = lilv_node_as_string; };

using NodeUri    = TypesafeLilvNode<UriConstructorTrait>;
using NodeString = TypesafeLilvNode<StringConstructorTrait>;

struct UsefulUris
{
    explicit UsefulUris (LilvWorld* worldIn)
        : world (worldIn) {}

    LilvWorld* const world = nullptr;

   #define X(str) const NodeUri m##str { world, str };
    X (LV2_ATOM__AtomPort)
    X (LV2_ATOM__atomTransfer)
    X (LV2_ATOM__eventTransfer)
    X (LV2_CORE__AudioPort)
    X (LV2_CORE__CVPort)
    X (LV2_CORE__ControlPort)
    X (LV2_CORE__GeneratorPlugin)
    X (LV2_CORE__InputPort)
    X (LV2_CORE__InstrumentPlugin)
    X (LV2_CORE__OutputPort)
    X (LV2_CORE__enumeration)
    X (LV2_CORE__integer)
    X (LV2_CORE__toggled)
    X (LV2_RESIZE_PORT__minimumSize)
    X (LV2_UI__floatProtocol)
    X (LV2_WORKER__interface)
   #undef X
};

template <typename Ptr, typename Free>
struct OwningPtrTraits
{
    using type = std::unique_ptr<Ptr, Free>;
    static const Ptr* get (const type& t) noexcept { return t.get(); }
};

template <typename Ptr>
struct NonOwningPtrTraits
{
    using type = const Ptr*;
    static const Ptr* get (const type& t) noexcept { return t; }
};

struct PluginsIteratorTraits
{
    using Container                 = const LilvPlugins*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_plugins_begin;
    static constexpr auto next      = lilv_plugins_next;
    static constexpr auto isEnd     = lilv_plugins_is_end;
    static constexpr auto get       = lilv_plugins_get;
};

using PluginsIterator = lv2_shared::Iterator<PluginsIteratorTraits>;

struct PluginClassesIteratorTraits
{
    using Container                 = const LilvPluginClasses*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_plugin_classes_begin;
    static constexpr auto next      = lilv_plugin_classes_next;
    static constexpr auto isEnd     = lilv_plugin_classes_is_end;
    static constexpr auto get       = lilv_plugin_classes_get;
};

using PluginClassesIterator = lv2_shared::Iterator<PluginClassesIteratorTraits>;

struct NodesIteratorTraits
{
    using Container                 = const LilvNodes*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_nodes_begin;
    static constexpr auto next      = lilv_nodes_next;
    static constexpr auto isEnd     = lilv_nodes_is_end;
    static constexpr auto get       = lilv_nodes_get;
};

using NodesIterator = lv2_shared::Iterator<NodesIteratorTraits>;

struct ScalePointsIteratorTraits
{
    using Container                 = const LilvScalePoints*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_scale_points_begin;
    static constexpr auto next      = lilv_scale_points_next;
    static constexpr auto isEnd     = lilv_scale_points_is_end;
    static constexpr auto get       = lilv_scale_points_get;
};

using ScalePointsIterator = lv2_shared::Iterator<ScalePointsIteratorTraits>;

struct UisIteratorTraits
{
    using Container                 = const LilvUIs*;
    using Iter                      = LilvIter*;
    static constexpr auto begin     = lilv_uis_begin;
    static constexpr auto next      = lilv_uis_next;
    static constexpr auto isEnd     = lilv_uis_is_end;
    static constexpr auto get       = lilv_uis_get;
};

using UisIterator = lv2_shared::Iterator<UisIteratorTraits>;

template <typename PtrTraits>
class NodesImpl
{
public:
    using type = typename PtrTraits::type;

    template <typename Ptr>
    explicit NodesImpl (Ptr* ptr)
        : nodes (type { ptr }) {}

    explicit NodesImpl (type ptr)
        : nodes (std::move (ptr)) {}

    u32 size() const noexcept { return lilv_nodes_size (PtrTraits::get (nodes)); }

    NodesIterator begin() const noexcept
    {
        return nodes == nullptr ? NodesIterator{}
                                : NodesIterator { PtrTraits::get (nodes) };
    }

    NodesIterator end()   const noexcept { return {}; }

private:
    type nodes{};
};

struct NodesFree
{
    z0 operator() (LilvNodes* ptr) const noexcept { lilv_nodes_free (ptr); }
};

using OwningNodes    = NodesImpl<OwningPtrTraits<LilvNodes, NodesFree>>;
using NonOwningNodes = NodesImpl<NonOwningPtrTraits<LilvNodes>>;

class ScalePoints
{
public:
    explicit ScalePoints (const LilvScalePoints* pt)
        : points (pt) {}

    ScalePointsIterator begin() const noexcept
    {
        return points == nullptr ? ScalePointsIterator{}
                                 : ScalePointsIterator { points };
    }

    ScalePointsIterator end() const noexcept { return {}; }

private:
    const LilvScalePoints* points = nullptr;
};

class ScalePoint
{
public:
    explicit ScalePoint (const LilvScalePoint* pt)
        : point (pt) {}

    const LilvNode* getLabel() const noexcept { return lilv_scale_point_get_label (point); }
    const LilvNode* getValue() const noexcept { return lilv_scale_point_get_value (point); }

private:
    const LilvScalePoint* point = nullptr;
};

struct PortRange
{
    f32 defaultValue, min, max;
};

class Port
{
public:
    enum class Kind
    {
        control,
        audio,
        cv,
        atom,
        unknown,
    };

    enum class Direction
    {
        input,
        output,
        unknown,
    };

    Port (const LilvPlugin* pluginIn, const LilvPort* portIn)
        : plugin (pluginIn), port (portIn) {}

    Direction getDirection (const UsefulUris& uris) const noexcept
    {
        if (isA (uris.mLV2_CORE__InputPort))
            return Direction::input;

        if (isA (uris.mLV2_CORE__OutputPort))
            return Direction::output;

        return Direction::unknown;
    }

    Kind getKind (const UsefulUris& uris) const noexcept
    {
        if (isA (uris.mLV2_CORE__ControlPort))
            return Kind::control;

        if (isA (uris.mLV2_CORE__AudioPort))
            return Kind::audio;

        if (isA (uris.mLV2_CORE__CVPort))
            return Kind::cv;

        if (isA (uris.mLV2_ATOM__AtomPort))
            return Kind::atom;

        return Kind::unknown;
    }

    OwningNode get (const LilvNode* predicate) const noexcept
    {
        return OwningNode { lilv_port_get (plugin, port, predicate) };
    }

    NonOwningNodes getClasses() const noexcept
    {
        return NonOwningNodes { lilv_port_get_classes (plugin, port) };
    }

    NodeString getName() const noexcept
    {
        return NodeString::claim (lilv_port_get_name (plugin, port));
    }

    NodeString getSymbol() const noexcept
    {
        return NodeString::copy (lilv_port_get_symbol (plugin, port));
    }

    OwningNodes getProperties() const noexcept
    {
        return OwningNodes { lilv_port_get_properties (plugin, port) };
    }

    ScalePoints getScalePoints() const noexcept
    {
        return ScalePoints { lilv_port_get_scale_points (plugin, port) };
    }

    b8 hasProperty (const NodeUri& uri) const noexcept
    {
        return lilv_port_has_property (plugin, port, uri.get());
    }

    u32 getIndex() const noexcept { return lilv_port_get_index (plugin, port); }

    static f32 getFloatValue (const LilvNode* node, f32 fallback)
    {
        if (lilv_node_is_float (node) || lilv_node_is_int (node))
            return lilv_node_as_float (node);

        return fallback;
    }

    b8 supportsEvent (const LilvNode* node) const noexcept
    {
        return lilv_port_supports_event (plugin, port, node);
    }

    PortRange getRange() const noexcept
    {
        LilvNode* def = nullptr;
        LilvNode* min = nullptr;
        LilvNode* max = nullptr;

        lilv_port_get_range (plugin, port, &def, &min, &max);

        const OwningNode defOwner { def };
        const OwningNode minOwner { min };
        const OwningNode maxOwner { max };

        return { getFloatValue (def, 0.0f),
                 getFloatValue (min, 0.0f),
                 getFloatValue (max, 1.0f) };
    }

    b8 isValid() const noexcept { return port != nullptr; }

private:
    b8 isA (const NodeUri& uri) const noexcept
    {
        return lilv_port_is_a (plugin, port, uri.get());
    }

    const LilvPlugin* plugin = nullptr;
    const LilvPort* port = nullptr;

    DRX_LEAK_DETECTOR (Port)
};

class Plugin
{
public:
    explicit Plugin (const LilvPlugin* p) : plugin (p) {}

    b8 verify() const noexcept              { return lilv_plugin_verify (plugin); }
    NodeUri getUri() const noexcept           { return NodeUri::copy (lilv_plugin_get_uri (plugin)); }
    NodeUri getBundleUri() const noexcept     { return NodeUri::copy (lilv_plugin_get_bundle_uri (plugin)); }
    NodeUri getLibraryUri() const noexcept    { return NodeUri::copy (lilv_plugin_get_library_uri (plugin)); }
    NodeString getName() const noexcept       { return NodeString::claim (lilv_plugin_get_name (plugin)); }
    NodeString getAuthorName() const noexcept { return NodeString::claim (lilv_plugin_get_author_name (plugin)); }
    u32 getNumPorts() const noexcept { return lilv_plugin_get_num_ports (plugin); }
    const LilvPluginClass* getClass() const noexcept { return lilv_plugin_get_class (plugin); }
    OwningNodes getValue (const LilvNode* predicate) const noexcept { return OwningNodes { lilv_plugin_get_value (plugin, predicate) }; }

    Port getPortByIndex (u32 index) const noexcept
    {
        return Port { plugin, lilv_plugin_get_port_by_index (plugin, index) };
    }

    Port getPortByDesignation (const LilvNode* portClass, const LilvNode* designation) const noexcept
    {
        return Port { plugin, lilv_plugin_get_port_by_designation (plugin, portClass, designation) };
    }

    OwningNodes getRequiredFeatures() const noexcept
    {
        return OwningNodes { lilv_plugin_get_required_features (plugin) };
    }

    OwningNodes getOptionalFeatures() const noexcept
    {
        return OwningNodes { lilv_plugin_get_optional_features (plugin) };
    }

    b8 hasExtensionData (const NodeUri& uri) const noexcept
    {
        return lilv_plugin_has_extension_data (plugin, uri.get());
    }

    b8 hasFeature (const NodeUri& uri) const noexcept
    {
        return lilv_plugin_has_feature (plugin, uri.get());
    }

    template <typename... Classes>
    u32 getNumPortsOfClass (const Classes&... classes) const noexcept
    {
        return lilv_plugin_get_num_ports_of_class (plugin, classes.get()..., 0);
    }

    const LilvPlugin* get() const noexcept { return plugin; }

    b8 hasLatency() const noexcept { return lilv_plugin_has_latency (plugin); }
    u32 getLatencyPortIndex() const noexcept { return lilv_plugin_get_latency_port_index (plugin); }

private:
    const LilvPlugin* plugin = nullptr;

    DRX_LEAK_DETECTOR (Plugin)
};

/*
    This is very similar to the symap implementation in jalv.
*/
class SymbolMap
{
public:
    SymbolMap() = default;

    SymbolMap (std::initializer_list<tukk> uris)
    {
        for (const auto* str : uris)
            map (str);
    }

    LV2_URID map (tukk uri)
    {
        const auto comparator = [this] (size_t index, const Txt& str)
        {
            return strings[index] < str;
        };

        const auto uriString = Txt::fromUTF8 (uri);
        const auto it = std::lower_bound (indices.cbegin(), indices.cend(), uriString, comparator);

        if (it != indices.cend() && strings[*it] == uriString)
            return static_cast<LV2_URID> (*it + 1);

        const auto index = strings.size();
        indices.insert (it, index);
        strings.push_back (uriString);
        return static_cast<LV2_URID> (index + 1);
    }

    tukk unmap (LV2_URID urid) const
    {
        const auto index = urid - 1;
        return index < strings.size() ? strings[index].toRawUTF8()
                                      : nullptr;
    }

    static LV2_URID mapUri (LV2_URID_Map_Handle handle, tukk uri)
    {
        return static_cast<SymbolMap*> (handle)->map (uri);
    }

    static tukk unmapUri (LV2_URID_Unmap_Handle handle, LV2_URID urid)
    {
        return static_cast<SymbolMap*> (handle)->unmap (urid);
    }

    LV2_URID_Map   getMapFeature()      { return { this, mapUri }; }
    LV2_URID_Unmap getUnmapFeature()    { return { this, unmapUri }; }

private:
    std::vector<Txt> strings;
    std::vector<size_t> indices;

    DRX_LEAK_DETECTOR (SymbolMap)
};

struct UsefulUrids
{
    explicit UsefulUrids (SymbolMap& m) : symap (m) {}

    SymbolMap& symap;

   #define X(token) const LV2_URID m##token = symap.map (token);
    X (LV2_ATOM__Bool)
    X (LV2_ATOM__Double)
    X (LV2_ATOM__Float)
    X (LV2_ATOM__Int)
    X (LV2_ATOM__Long)
    X (LV2_ATOM__Object)
    X (LV2_ATOM__Sequence)
    X (LV2_ATOM__atomTransfer)
    X (LV2_ATOM__beatTime)
    X (LV2_ATOM__eventTransfer)
    X (LV2_ATOM__frameTime)
    X (LV2_LOG__Error)
    X (LV2_LOG__Note)
    X (LV2_LOG__Trace)
    X (LV2_LOG__Warning)
    X (LV2_MIDI__MidiEvent)
    X (LV2_PATCH__Set)
    X (LV2_PATCH__property)
    X (LV2_PATCH__value)
    X (LV2_STATE__StateChanged)
    X (LV2_TIME__Position)
    X (LV2_TIME__barBeat)
    X (LV2_TIME__beat)
    X (LV2_TIME__beatUnit)
    X (LV2_TIME__beatsPerBar)
    X (LV2_TIME__beatsPerMinute)
    X (LV2_TIME__frame)
    X (LV2_TIME__speed)
    X (LV2_TIME__bar)
    X (LV2_UI__floatProtocol)
    X (LV2_UNITS__beat)
    X (LV2_UNITS__frame)
   #undef X
};

class Log
{
public:
    explicit Log (const UsefulUrids* u) : urids (u) {}

    LV2_Log_Log* getLogFeature() { return &logFeature; }

private:
    i32 vprintfCallback ([[maybe_unused]] LV2_URID type, tukk fmt, va_list ap) const
    {
        // If this is hit, the plugin has encountered some kind of error
        ignoreUnused (urids);
        jassert (type != urids->mLV2_LOG__Error && type != urids->mLV2_LOG__Warning);
        return std::vfprintf (stderr, fmt, ap);
    }

    static i32 vprintfCallback (LV2_Log_Handle handle,
                                LV2_URID type,
                                tukk fmt,
                                va_list ap)
    {
        return static_cast<const Log*> (handle)->vprintfCallback (type, fmt, ap);
    }

    static i32 printfCallback (LV2_Log_Handle handle, LV2_URID type, tukk fmt, ...)
    {
        va_list list;
        va_start (list, fmt);
        auto result = vprintfCallback (handle, type, fmt, list);
        va_end (list);
        return result;
    }

    const UsefulUrids* urids = nullptr;
    LV2_Log_Log logFeature { this, printfCallback, vprintfCallback };

    DRX_LEAK_DETECTOR (Log)
};

struct Features
{
    explicit Features (std::vector<LV2_Feature>&& f)
        : features (std::move (f)) {}

    static std::vector<Txt> getUris (const std::vector<LV2_Feature>& features)
    {
        std::vector<Txt> result;
        result.reserve (features.size());

        for (const auto& feature : features)
            result.push_back (Txt::fromUTF8 (feature.URI));

        return result;
    }

    std::vector<LV2_Feature> features;
    std::vector<const LV2_Feature*> pointers = makeNullTerminatedArray();

private:
    std::vector<const LV2_Feature*> makeNullTerminatedArray()
    {
        std::vector<const LV2_Feature*> result;
        result.reserve (features.size() + 1);

        for (const auto& feature : features)
            result.push_back (&feature);

        result.push_back (nullptr);

        return result;
    }

    DRX_LEAK_DETECTOR (Features)
};

template <typename Extension>
struct OptionalExtension
{
    OptionalExtension() = default;

    explicit OptionalExtension (Extension extensionIn) : extension (extensionIn), valid (true) {}

    Extension extension;
    b8 valid = false;
};

class Instance
{
    struct Free
    {
        z0 operator() (LilvInstance* ptr) const noexcept { lilv_instance_free (ptr); }
    };

public:
    using Ptr = std::unique_ptr<LilvInstance, Free>;
    using GetExtensionData = ukk (*) (tukk);

    Instance (const Plugin& pluginIn, f64 sampleRate, const LV2_Feature* const* features)
        : plugin (pluginIn),
          instance (lilv_plugin_instantiate (plugin.get(), sampleRate, features)) {}

    z0 activate() { lilv_instance_activate (instance.get()); }
    z0 run (u32 sampleCount) { lilv_instance_run (instance.get(), sampleCount); }
    z0 deactivate() { lilv_instance_deactivate (instance.get()); }

    tukk getUri() const noexcept { return lilv_instance_get_uri (instance.get()); }

    LV2_Handle getHandle() const noexcept { return lilv_instance_get_handle (instance.get()); }

    LilvInstance* get() const noexcept { return instance.get(); }

    z0 connectPort (u32 index, uk data)
    {
        lilv_instance_connect_port (instance.get(), index, data);
    }

    template <typename Extension>
    OptionalExtension<Extension> getExtensionData (const NodeUri& uri) const noexcept
    {
        if (plugin.get() == nullptr || ! plugin.hasExtensionData (uri) || instance.get() == nullptr)
            return {};

        return OptionalExtension<Extension> { readUnaligned<Extension> (lilv_instance_get_extension_data (instance.get(), uri.getTyped())) };
    }

    GetExtensionData getExtensionDataCallback() const noexcept
    {
        return instance->lv2_descriptor->extension_data;
    }

    b8 operator== (std::nullptr_t) const noexcept { return instance == nullptr; }
    b8 operator!= (std::nullptr_t) const noexcept { return ! (*this == nullptr); }

private:
    Plugin plugin;
    Ptr instance;

    DRX_LEAK_DETECTOR (Instance)
};

enum class Realtime { no, yes };

// Must be trivial!
struct WorkResponder
{
    static WorkResponder getDefault() { return { nullptr, nullptr }; }

    LV2_Worker_Status processResponse (u32 size, ukk data) const
    {
        return worker->work_response (handle, size, data);
    }

    b8 isValid() const { return handle != nullptr && worker != nullptr; }

    LV2_Handle handle;
    const LV2_Worker_Interface* worker;
};

struct WorkerResponseListener
{
    virtual ~WorkerResponseListener() = default;
    virtual LV2_Worker_Status responseGenerated (WorkResponder, u32, ukk) = 0;
};

struct RespondHandle
{
    LV2_Worker_Status respond (u32 size, ukk data) const
    {
        if (realtime == Realtime::yes)
            return listener.responseGenerated (responder, size, data);

        return responder.processResponse (size, data);
    }

    static LV2_Worker_Status respond (LV2_Worker_Respond_Handle handle,
                                      u32 size,
                                      ukk data)
    {
        return static_cast<const RespondHandle*> (handle)->respond (size, data);
    }

    WorkResponder responder;
    WorkerResponseListener& listener;
    Realtime realtime;
};

// Must be trivial!
struct WorkSubmitter
{
    static WorkSubmitter getDefault() { return { nullptr, nullptr, nullptr, nullptr }; }

    LV2_Worker_Status doWork (Realtime realtime, u32 size, ukk data) const
    {
        // The Worker spec says that the host "MUST NOT make concurrent calls to [work] from
        // several threads".
        // Taking the work mutex here ensures that only one piece of work is done at a time.
        // If we didn't take the work mutex, there would be a danger of work happening
        // simultaneously on the worker thread and the render thread when switching between
        // realtime/offline modes (in realtime mode, work happens on the worker thread; in
        // offline mode, work happens immediately on the render/audio thread).
        const ScopedLock lock (*workMutex);

        RespondHandle respondHandle { WorkResponder { handle, worker }, *listener, realtime };
        return worker->work (handle, RespondHandle::respond, &respondHandle, size, data);
    }

    b8 isValid() const { return handle != nullptr && worker != nullptr && listener != nullptr && workMutex != nullptr; }

    LV2_Handle handle;
    const LV2_Worker_Interface* worker;
    WorkerResponseListener* listener;
    CriticalSection* workMutex;
};

template <typename Trivial>
static auto toChars (Trivial value)
{
    static_assert (std::is_trivial_v<Trivial>);
    std::array<t8, sizeof (Trivial)> result;
    writeUnaligned (result.data(), value);
    return result;
}

template <typename Context>
class WorkQueue
{
public:
    static_assert (std::is_trivial_v<Context>, "Context must be copyable as bytes");

    explicit WorkQueue (i32 size)
        : fifo (size), data (static_cast<size_t> (size)) {}

    LV2_Worker_Status push (Context context, size_t size, ukk contents)
    {
        const auto* bytes = static_cast<tukk> (contents);
        const auto numToWrite = sizeof (Header) + size;

        if (static_cast<size_t> (fifo.getFreeSpace()) < numToWrite)
            return LV2_WORKER_ERR_NO_SPACE;

        Header header { size, context };
        const auto headerBuffer = toChars (header);

        const auto scope = fifo.write (static_cast<i32> (numToWrite));
        jassert (scope.blockSize1 + scope.blockSize2 == static_cast<i32> (numToWrite));

        size_t index = 0;
        scope.forEach ([&] (i32 i)
        {
            data[static_cast<size_t> (i)] = index < headerBuffer.size() ? headerBuffer[index]
                                                                        : bytes[index - headerBuffer.size()];
            ++index;
        });

        return LV2_WORKER_SUCCESS;
    }

    Context pop (std::vector<t8>& dest)
    {
        // If the vector is too small we'll have to resize it on the audio thread
        jassert (dest.capacity() >= data.size());
        dest.clear();

        const auto numReady = fifo.getNumReady();

        if (static_cast<size_t> (numReady) < sizeof (Header))
        {
            jassert (numReady == 0);
            return Context::getDefault();
        }

        std::array<t8, sizeof (Header)> headerBuffer;

        {
            size_t index = 0;
            fifo.read (sizeof (Header)).forEach ([&] (i32 i)
            {
                headerBuffer[index++] = data[static_cast<size_t> (i)];
            });
        }

        const auto header = readUnaligned<Header> (headerBuffer.data());

        jassert (static_cast<size_t> (fifo.getNumReady()) >= header.size);

        dest.resize (header.size);

        {
            size_t index = 0;
            fifo.read (static_cast<i32> (header.size)).forEach ([&] (i32 i)
            {
                dest[index++] = data[static_cast<size_t> (i)];
            });
        }

        return header.context;
    }

private:
    struct Header
    {
        size_t size;
        Context context;
    };

    AbstractFifo fifo;
    std::vector<t8> data;

    DRX_LEAK_DETECTOR (WorkQueue)
};

/*
    Keeps track of active plugin instances, so that we can avoid sending work
    messages to dead plugins.
*/
class HandleRegistry
{
public:
    z0 insert (LV2_Handle handle)
    {
        const SpinLock::ScopedLockType lock (mutex);
        handles.insert (handle);
    }

    z0 erase (LV2_Handle handle)
    {
        const SpinLock::ScopedLockType lock (mutex);
        handles.erase (handle);
    }

    template <typename Fn>
    LV2_Worker_Status ifContains (LV2_Handle handle, Fn&& callback)
    {
        const SpinLock::ScopedLockType lock (mutex);

        if (handles.find (handle) != handles.cend())
            return callback();

        return LV2_WORKER_ERR_UNKNOWN;
    }

private:
    std::set<LV2_Handle> handles;
    SpinLock mutex;

    DRX_LEAK_DETECTOR (HandleRegistry)
};

/*
    Implements an LV2 Worker, allowing work to be scheduled in realtime
    by the plugin instance.

    IMPORTANT this will die pretty hard if `getExtensionData (LV2_WORKER__interface)`
    returns garbage, so make sure to check that the plugin `hasExtensionData` before
    constructing one of these!
*/
class SharedThreadedWorker final : public WorkerResponseListener
{
public:
    ~SharedThreadedWorker() noexcept override
    {
        shouldExit = true;
        thread.join();
    }

    LV2_Worker_Status schedule (WorkSubmitter submitter,
                                u32 size,
                                ukk data)
    {
        return registry.ifContains (submitter.handle, [&]
        {
            return incoming.push (submitter, size, data);
        });
    }

    LV2_Worker_Status responseGenerated (WorkResponder responder,
                                         u32 size,
                                         ukk data) override
    {
        return registry.ifContains (responder.handle, [&]
        {
            return outgoing.push (responder, size, data);
        });
    }

    z0 processResponses()
    {
        for (;;)
        {
            auto workerResponder = outgoing.pop (message);

            if (! message.empty() && workerResponder.isValid())
                workerResponder.processResponse (static_cast<u32> (message.size()), message.data());
            else
                break;
        }
    }

    z0 registerHandle   (LV2_Handle handle) { registry.insert (handle); }
    z0 deregisterHandle (LV2_Handle handle) { registry.erase  (handle); }

private:
    static constexpr auto queueSize = 8192;
    std::atomic<b8> shouldExit { false };
    WorkQueue<WorkSubmitter> incoming { queueSize };
    WorkQueue<WorkResponder> outgoing { queueSize };
    std::vector<t8> message = std::vector<t8> (queueSize);
    std::thread thread { [this]
    {
        std::vector<t8> buffer (queueSize);

        while (! shouldExit)
        {
            const auto submitter = incoming.pop (buffer);

            if (! buffer.empty() && submitter.isValid())
                submitter.doWork (Realtime::yes, (u32) buffer.size(), buffer.data());
            else
                std::this_thread::sleep_for (std::chrono::milliseconds (1));
        }
    } };
    HandleRegistry registry;

    DRX_LEAK_DETECTOR (SharedThreadedWorker)
};

struct HandleHolder
{
    virtual ~HandleHolder() = default;
    virtual LV2_Handle getHandle() const = 0;
    virtual const LV2_Worker_Interface* getWorkerInterface() const = 0;
};

class WorkScheduler
{
public:
    explicit WorkScheduler (HandleHolder& handleHolderIn)
        : handleHolder (handleHolderIn) {}

    z0 processResponses() { workerThread->processResponses(); }

    LV2_Worker_Schedule& getWorkerSchedule() { return schedule; }

    z0 setNonRealtime (b8 nonRealtime) { realtime = ! nonRealtime; }

    z0 registerHandle   (LV2_Handle handle) { workerThread->registerHandle   (handle); }
    z0 deregisterHandle (LV2_Handle handle) { workerThread->deregisterHandle (handle); }

private:
    LV2_Worker_Status scheduleWork (u32 size, ukk data)
    {
        WorkSubmitter submitter { handleHolder.getHandle(),
                                  handleHolder.getWorkerInterface(),
                                  workerThread,
                                  &workMutex };

        // If we're in realtime mode, the work should go onto a background thread,
        // and we'll process it later.
        // If we're offline, we can just do the work immediately, without worrying about
        // drop-outs
        return realtime ? workerThread->schedule (submitter, size, data)
                        : submitter.doWork (Realtime::no, size, data);
    }

    static LV2_Worker_Status scheduleWork (LV2_Worker_Schedule_Handle handle,
                                           u32 size,
                                           ukk data)
    {
        return static_cast<WorkScheduler*> (handle)->scheduleWork (size, data);
    }

    SharedResourcePointer<SharedThreadedWorker> workerThread;
    HandleHolder& handleHolder;
    LV2_Worker_Schedule schedule { this, scheduleWork };
    CriticalSection workMutex;
    b8 realtime = true;

    DRX_LEAK_DETECTOR (WorkScheduler)
};

struct FeaturesDataListener
{
    virtual ~FeaturesDataListener() = default;
    virtual LV2_Resize_Port_Status resizeCallback (u32 index, size_t size) = 0;
};

class Resize
{
public:
    explicit Resize (FeaturesDataListener& l)
        : listener (l) {}

    LV2_Resize_Port_Resize& getFeature() { return resize; }

private:
    LV2_Resize_Port_Status resizeCallback (u32 index, size_t size)
    {
        return listener.resizeCallback (index, size);
    }

    static LV2_Resize_Port_Status resizeCallback (LV2_Resize_Port_Feature_Data data, u32 index, size_t size)
    {
        return static_cast<Resize*> (data)->resizeCallback (index, size);
    }

    FeaturesDataListener& listener;
    LV2_Resize_Port_Resize resize { this, resizeCallback };
};

class FeaturesData
{
public:
    FeaturesData (HandleHolder& handleHolder,
                  FeaturesDataListener& l,
                  i32 maxBlockSizeIn,
                  i32 sequenceSizeIn,
                  const UsefulUrids* u)
        : urids (u),
          resize (l),
          maxBlockSize (maxBlockSizeIn),
          sequenceSize (sequenceSizeIn),
          workScheduler (handleHolder)
    {}

    LV2_Options_Option* getOptions() noexcept { return options.data(); }

    i32 getMaxBlockSize() const noexcept { return maxBlockSize; }

    z0 setNonRealtime (b8 newValue) { realtime = ! newValue; }

    const LV2_Feature* const* getFeatureArray() const noexcept { return features.pointers.data(); }

    static std::vector<Txt> getFeatureUris()
    {
        return Features::getUris (makeFeatures ({}, {}, {}, {}, {}, {}));
    }

    z0 processResponses() { workScheduler.processResponses(); }

    z0 registerHandle   (LV2_Handle handle) { workScheduler.registerHandle   (handle); }
    z0 deregisterHandle (LV2_Handle handle) { workScheduler.deregisterHandle (handle); }

private:
    static std::vector<LV2_Feature> makeFeatures (LV2_URID_Map* map,
                                                  LV2_URID_Unmap* unmap,
                                                  LV2_Options_Option* options,
                                                  LV2_Worker_Schedule* schedule,
                                                  LV2_Resize_Port_Resize* resize,
                                                  [[maybe_unused]] LV2_Log_Log* log)
    {
        return { LV2_Feature { LV2_STATE__loadDefaultState,         nullptr },
                 LV2_Feature { LV2_BUF_SIZE__boundedBlockLength,    nullptr },
                 LV2_Feature { LV2_URID__map,                       map },
                 LV2_Feature { LV2_URID__unmap,                     unmap },
                 LV2_Feature { LV2_OPTIONS__options,                options },
                 LV2_Feature { LV2_WORKER__schedule,                schedule },
                 LV2_Feature { LV2_STATE__threadSafeRestore,        nullptr },
                #if DRX_DEBUG
                 LV2_Feature { LV2_LOG__log,                        log },
                #endif
                 LV2_Feature { LV2_RESIZE_PORT__resize,             resize } };
    }

    LV2_Options_Option makeOption (tukk uid, const i32* ptr)
    {
        return { LV2_OPTIONS_INSTANCE,
                 0,                         // INSTANCE kinds must have a subject of 0
                 urids->symap.map (uid),
                 sizeof (i32),
                 urids->symap.map (LV2_ATOM__Int),
                 ptr };
    }

    const UsefulUrids* urids;
    Resize resize;
    Log log { urids };

    const i32 minBlockSize = 0, maxBlockSize = 0, sequenceSize = 0;

    std::vector<LV2_Options_Option> options
    {
        makeOption (LV2_BUF_SIZE__minBlockLength, &minBlockSize),
        makeOption (LV2_BUF_SIZE__maxBlockLength, &maxBlockSize),
        makeOption (LV2_BUF_SIZE__sequenceSize,   &sequenceSize),
        { LV2_OPTIONS_INSTANCE, 0, 0, 0, 0, nullptr }, // The final entry must be nulled out
    };

    WorkScheduler workScheduler;

    LV2_URID_Map        map       = urids->symap.getMapFeature();
    LV2_URID_Unmap      unmap     = urids->symap.getUnmapFeature();
    Features features { makeFeatures (&map,
                                      &unmap,
                                      options.data(),
                                      &workScheduler.getWorkerSchedule(),
                                      &resize.getFeature(),
                                      log.getLogFeature()) };

    b8 realtime = true;

    DRX_LEAK_DETECTOR (FeaturesData)
};

//==============================================================================
struct TryLockAndCall
{
    template <typename Fn>
    z0 operator() (SpinLock& mutex, Fn&& fn)
    {
        const SpinLock::ScopedTryLockType lock (mutex);

        if (lock.isLocked())
            fn();
    }
};

struct LockAndCall
{
    template <typename Fn>
    z0 operator() (SpinLock& mutex, Fn&& fn)
    {
        const SpinLock::ScopedLockType lock (mutex);
        fn();
    }
};

struct RealtimeReadTrait
{
    using Read  = TryLockAndCall;
    using Write = LockAndCall;
};

struct RealtimeWriteTrait
{
    using Read  = LockAndCall;
    using Write = TryLockAndCall;
};

struct MessageHeader
{
    u32 portIndex;
    u32 protocol;
};

template <typename Header>
struct MessageBufferInterface
{
    virtual ~MessageBufferInterface() = default;
    virtual z0 pushMessage (Header header, u32 size, ukk buffer) = 0;
};

template <typename Header, typename LockTraits>
class Messages final : public MessageBufferInterface<Header>
{
    using Read  = typename LockTraits::Read;
    using Write = typename LockTraits::Write;

    struct FullHeader
    {
        Header header;
        u32 size;
    };

public:
    Messages() { data.reserve (initialBufferSize); }

    z0 pushMessage (Header header, u32 size, ukk buffer) override
    {
        Write{} (mutex, [&]
        {
            const auto chars = toChars (FullHeader { header, size });
            const auto bufferAsChars = static_cast<tukk> (buffer);
            data.insert (data.end(), chars.begin(), chars.end());
            data.insert (data.end(), bufferAsChars, bufferAsChars + size);
        });
    }

    template <typename Callback>
    z0 readAllAndClear (Callback&& callback)
    {
        Read{} (mutex, [&]
        {
            if (data.empty())
                return;

            const auto end = data.data() + data.size();

            for (auto ptr = data.data(); ptr < end;)
            {
                const auto header = readUnaligned<FullHeader> (ptr);
                callback (header.header, header.size, ptr + sizeof (header));
                ptr += sizeof (header) + header.size;
            }

            data.clear();
        });
    }

private:
    static constexpr auto initialBufferSize = 8192;
    SpinLock mutex;
    std::vector<t8> data;

    DRX_LEAK_DETECTOR (Messages)
};

//==============================================================================
struct UiEventListener : public MessageBufferInterface<MessageHeader>
{
    virtual i32 idle() = 0;
};

struct UiMessageHeader
{
    UiEventListener* listener;
    MessageHeader header;
};

class ProcessorToUi final : public MessageBufferInterface<UiMessageHeader>
{
public:
    ProcessorToUi() { timer.startTimerHz (60); }

    z0 addUi    (UiEventListener& l)      { DRX_ASSERT_MESSAGE_THREAD; activeUis.insert (&l); }
    z0 removeUi (UiEventListener& l)      { DRX_ASSERT_MESSAGE_THREAD; activeUis.erase (&l); }

    z0 pushMessage (UiMessageHeader header, u32 size, ukk buffer) override
    {
        processorToUi.pushMessage (header, size, buffer);
    }

private:
    Messages<UiMessageHeader, RealtimeWriteTrait> processorToUi;
    std::set<UiEventListener*> activeUis;
    TimedCallback timer { [this]
    {
        for (auto* l : activeUis)
            if (l->idle() != 0)
                return;

        processorToUi.readAllAndClear ([&] (const UiMessageHeader& header, u32 size, tukk data)
        {
            if (activeUis.find (header.listener) != activeUis.cend())
                header.listener->pushMessage (header.header, size, data);
        });
    } };
};

/*  These type identifiers may be used to check the type of the incoming data. */
struct StatefulPortUrids
{
    explicit StatefulPortUrids (SymbolMap& map)
        : Float  (map.map (LV2_ATOM__Float)),
          Double (map.map (LV2_ATOM__Double)),
          Int    (map.map (LV2_ATOM__Int)),
          Long   (map.map (LV2_ATOM__Long))
    {}

    const LV2_URID Float, Double, Int, Long;
};

/*
    A bit like SortedSet, but only requires `operator<` and not `operator==`, so
    it behaves a bit more like a std::set.
*/
template <typename Value>
class SafeSortedSet
{
public:
    using       iterator = typename std::vector<Value>::      iterator;
    using const_iterator = typename std::vector<Value>::const_iterator;

    template <typename Other>
    const_iterator find (const Other& other) const noexcept
    {
        const auto it = std::lower_bound (storage.cbegin(), storage.cend(), other);

        if (it != storage.cend() && ! (other < *it))
            return it;

        return storage.cend();
    }

    z0 insert (Value&& value)      { insertImpl (std::move (value)); }
    z0 insert (const Value& value) { insertImpl (value); }

    size_t size()  const noexcept { return storage.size(); }
    b8   empty() const noexcept { return storage.empty(); }

    iterator        begin()       noexcept { return storage. begin(); }
    const_iterator  begin() const noexcept { return storage. begin(); }
    const_iterator cbegin() const noexcept { return storage.cbegin(); }

    iterator          end()       noexcept { return storage. end(); }
    const_iterator    end() const noexcept { return storage. end(); }
    const_iterator   cend() const noexcept { return storage.cend(); }

    auto& operator[] (size_t index) const { return storage[index]; }

private:
    template <typename Arg>
    z0 insertImpl (Arg&& value)
    {
        const auto it = std::lower_bound (storage.cbegin(), storage.cend(), value);

        if (it == storage.cend() || value < *it)
            storage.insert (it, std::forward<Arg> (value));
    }

    std::vector<Value> storage;
};

struct StoredScalePoint
{
    Txt label;
    f32 value;

    b8 operator< (const StoredScalePoint& other) const noexcept { return value < other.value; }
};

inline b8 operator< (const StoredScalePoint& a, f32 b) noexcept { return a.value < b; }
inline b8 operator< (f32 a, const StoredScalePoint& b) noexcept { return a < b.value; }

struct ParameterInfo
{
    ParameterInfo() = default;

    ParameterInfo (SafeSortedSet<StoredScalePoint> scalePointsIn,
                   Txt identifierIn,
                   f32 defaultValueIn,
                   f32 minIn,
                   f32 maxIn,
                   b8 isToggleIn,
                   b8 isIntegerIn,
                   b8 isEnumIn)
        : scalePoints (std::move (scalePointsIn)),
          identifier (std::move (identifierIn)),
          defaultValue (defaultValueIn),
          min (minIn),
          max (maxIn),
          isToggle (isToggleIn),
          isInteger (isIntegerIn),
          isEnum (isEnumIn)
    {}

    static SafeSortedSet<StoredScalePoint> getScalePoints (const Port& port)
    {
        SafeSortedSet<StoredScalePoint> scalePoints;

        for (const LilvScalePoint* p : port.getScalePoints())
        {
            const ScalePoint wrapper { p };
            const auto value = wrapper.getValue();
            const auto label = wrapper.getLabel();

            if (lilv_node_is_float (value) || lilv_node_is_int (value))
                scalePoints.insert ({ lilv_node_as_string (label), lilv_node_as_float (value) });
        }

        return scalePoints;
    }

    static ParameterInfo getInfoForPort (const UsefulUris& uris, const Port& port)
    {
        const auto range = port.getRange();

        return { getScalePoints (port),
                 "sym:" + Txt::fromUTF8 (port.getSymbol().getTyped()),
                 range.defaultValue,
                 range.min,
                 range.max,
                 port.hasProperty (uris.mLV2_CORE__toggled),
                 port.hasProperty (uris.mLV2_CORE__integer),
                 port.hasProperty (uris.mLV2_CORE__enumeration) };
    }

    SafeSortedSet<StoredScalePoint> scalePoints;

    /*  This is the 'symbol' of a port, or the 'designation' of a parameter without a symbol. */
    Txt identifier;

    f32 defaultValue = 0.0f, min = 0.0f, max = 1.0f;
    b8 isToggle = false, isInteger = false, isEnum = false;

    DRX_LEAK_DETECTOR (ParameterInfo)
};

struct PortHeader
{
    Txt name;
    Txt symbol;
    u32 index;
    Port::Direction direction;
};

struct ControlPort
{
    ControlPort (const PortHeader& headerIn, const ParameterInfo& infoIn)
        : header (headerIn), info (infoIn) {}

    PortHeader header;
    ParameterInfo info;
    f32 currentValue = info.defaultValue;
};

struct CVPort
{
    PortHeader header;
};

struct AudioPort
{
    PortHeader header;
};

template <size_t Alignment>
class SingleSizeAlignedStorage
{
public:
    SingleSizeAlignedStorage() = default;

    explicit SingleSizeAlignedStorage (size_t sizeInBytes)
        : storage (new t8[sizeInBytes + Alignment]),
          alignedPointer (storage.get()),
          space (sizeInBytes + Alignment)
    {
        alignedPointer = std::align (Alignment, sizeInBytes, alignedPointer, space);
    }

    uk  data() const     { return alignedPointer; }
    size_t size() const     { return space; }

private:
    std::unique_ptr<t8[]> storage;
    uk alignedPointer = nullptr;
    size_t space = 0;
};

template <size_t Alignment>
static SingleSizeAlignedStorage<Alignment> grow (SingleSizeAlignedStorage<Alignment> storage, size_t size)
{
    if (size <= storage.size())
        return storage;

    SingleSizeAlignedStorage<Alignment> newStorage { jmax (size, (storage.size() * 3) / 2) };
    std::memcpy (newStorage.data(), storage.data(), storage.size());
    return newStorage;
}

enum class SupportsTime { no, yes };

class AtomPort
{
public:
    AtomPort (PortHeader h, size_t bytes, SymbolMap& map, SupportsTime supportsTime)
        : header (h), contents (bytes), forge (map.getMapFeature()), time (supportsTime) {}

    PortHeader header;

    z0 replaceWithChunk()
    {
        forge.setBuffer (data(), size());
        forge.writeChunk ((u32) (size() - sizeof (LV2_Atom)));
    }

    z0 replaceBufferWithAtom (const LV2_Atom* atom)
    {
        const auto totalSize = atom->size + sizeof (LV2_Atom);

        if (totalSize <= size())
            std::memcpy (data(), atom, totalSize);
        else
            replaceWithChunk();
    }

    z0 beginSequence()
    {
        forge.setBuffer (data(), size());
        lv2_atom_forge_sequence_head (forge.get(), &frame, 0);
    }

    z0 endSequence()
    {
        lv2_atom_forge_pop (forge.get(), &frame);
    }

    /*  For this to work, the 'atom' pointer must be well-formed.

        It must be followed by an atom header, then at least 'size' bytes of body.
    */
    z0 addAtomToSequence (z64 timestamp, const LV2_Atom* atom)
    {
        // This reinterpret_cast is not UB, casting to a tuk is acceptable.
        // Doing arithmetic on this pointer is dubious, but I can't think of a better alternative
        // given that we don't have any way of knowing the concrete type of the atom.
        addEventToSequence (timestamp,
                            atom->type,
                            atom->size,
                            reinterpret_cast<tukk> (atom) + sizeof (LV2_Atom));
    }

    z0 addEventToSequence (z64 timestamp, u32 type, u32 size, ukk content)
    {
        lv2_atom_forge_frame_time (forge.get(), timestamp);
        lv2_atom_forge_atom (forge.get(), size, type);
        lv2_atom_forge_write (forge.get(), content, size);
    }

    z0 ensureSizeInBytes (size_t size)
    {
        contents = grow (std::move (contents), size);
    }

          tuk data()       noexcept { return data (*this); }
    tukk data() const noexcept { return data (*this); }

    size_t size() const noexcept { return contents.size(); }

          lv2_shared::AtomForge& getForge()       { return forge; }
    const lv2_shared::AtomForge& getForge() const { return forge; }

    b8 getSupportsTime() const { return time == SupportsTime::yes; }

private:
    template <typename This>
    static auto data (This& t) -> decltype (t.data())
    {
        return unalignedPointerCast<decltype (t.data())> (t.contents.data());
    }

    // Atoms are required to be 64-bit aligned
    SingleSizeAlignedStorage<8> contents;
    lv2_shared::AtomForge forge;
    LV2_Atom_Forge_Frame frame;
    SupportsTime time = SupportsTime::no;
};

struct FreeString { z0 operator() (uk ptr) const noexcept { lilv_free (ptr); } };

static File bundlePathFromUri (tukk uri)
{
    return File { std::unique_ptr<t8, FreeString> { lilv_file_uri_parse (uri, nullptr) }.get() };
}

class Plugins
{
public:
    explicit Plugins (const LilvPlugins* list) noexcept : plugins (list) {}

    u32 size() const noexcept { return lilv_plugins_size (plugins); }

    PluginsIterator begin() const noexcept { return PluginsIterator { plugins }; }
    PluginsIterator end()   const noexcept { return PluginsIterator{}; }

    const LilvPlugin* getByUri (const NodeUri& uri) const
    {
        return lilv_plugins_get_by_uri (plugins, uri.get());
    }

    const LilvPlugin* getByFile (const File& file) const
    {
        for (const auto* plugin : *this)
        {
            if (bundlePathFromUri (lilv_node_as_uri (lilv_plugin_get_bundle_uri (plugin))) == file)
                return plugin;
        }

        return nullptr;
    }

private:
    const LilvPlugins* plugins = nullptr;
};

template <typename PtrTraits>
class PluginClassesImpl
{
public:
    using type = typename PtrTraits::type;

    explicit PluginClassesImpl (type ptr)
            : classes (std::move (ptr)) {}

    u32 size() const noexcept { return lilv_plugin_classes_size (PtrTraits::get (classes)); }

    PluginClassesIterator begin() const noexcept { return PluginClassesIterator { PtrTraits::get (classes) }; }
    PluginClassesIterator end()   const noexcept { return PluginClassesIterator{}; }

    const LilvPluginClass* getByUri (const NodeUri& uri) const noexcept
    {
        return lilv_plugin_classes_get_by_uri (PtrTraits::get (classes), uri.get());
    }

private:
    type classes{};
};

struct PluginClassesFree
{
    z0 operator() (LilvPluginClasses* ptr) const noexcept { lilv_plugin_classes_free (ptr); }
};

using OwningPluginClasses    = PluginClassesImpl<OwningPtrTraits<LilvPluginClasses, PluginClassesFree>>;
using NonOwningPluginClasses = PluginClassesImpl<NonOwningPtrTraits<LilvPluginClasses>>;

class World
{
public:
    World() : world (lilv_world_new()) {}

    z0 loadAllFromPaths (const NodeString& paths)
    {
        lilv_world_set_option (world.get(), LILV_OPTION_LV2_PATH, paths.get());
        lilv_world_load_all (world.get());
    }

    z0 loadBundle   (const NodeUri& uri)      { lilv_world_load_bundle   (world.get(), uri.get()); }
    z0 unloadBundle (const NodeUri& uri)      { lilv_world_unload_bundle (world.get(), uri.get()); }

    z0 loadResource   (const NodeUri& uri)    { lilv_world_load_resource   (world.get(), uri.get()); }
    z0 unloadResource (const NodeUri& uri)    { lilv_world_unload_resource (world.get(), uri.get()); }

    z0 loadSpecifications() { lilv_world_load_specifications (world.get()); }
    z0 loadPluginClasses()  { lilv_world_load_plugin_classes (world.get()); }

    Plugins getAllPlugins()                   const { return Plugins { lilv_world_get_all_plugins (world.get()) }; }
    NonOwningPluginClasses getPluginClasses() const { return NonOwningPluginClasses { lilv_world_get_plugin_classes (world.get()) }; }

    NodeUri newUri (tukk uri)                        { return NodeUri    { world.get(), uri }; }
    NodeUri newFileUri (tukk host, tukk path) { return NodeUri    { world.get(), host, path }; }
    NodeString newString (tukk str)                  { return NodeString { world.get(), str }; }

    b8 ask (const LilvNode* subject, const LilvNode* predicate, const LilvNode* object) const
    {
        return lilv_world_ask (world.get(), subject, predicate, object);
    }

    OwningNode get (const LilvNode* subject, const LilvNode* predicate, const LilvNode* object) const
    {
        return OwningNode { lilv_world_get (world.get(), subject, predicate, object) };
    }

    OwningNodes findNodes (const LilvNode* subject, const LilvNode* predicate, const LilvNode* object) const
    {
        return OwningNodes { lilv_world_find_nodes (world.get(), subject, predicate, object) };
    }

    LilvWorld* get() const { return world.get(); }

private:
    struct Free
    {
        z0 operator() (LilvWorld* ptr) const noexcept { lilv_world_free (ptr); }
    };

    std::unique_ptr<LilvWorld, Free> world;
};

class Ports
{
public:
    static constexpr auto sequenceSize = 8192;

    template <typename Callback>
    z0 forEachPort (Callback&& callback) const
    {
        for (const auto& port : controlPorts)
            callback (port.header);

        for (const auto& port : cvPorts)
            callback (port.header);

        for (const auto& port : audioPorts)
            callback (port.header);

        for (const auto& port : atomPorts)
            callback (port.header);
    }

    auto getControlPorts()       { return makeSimpleSpan (controlPorts); }
    auto getControlPorts() const { return makeSimpleSpan (controlPorts); }
    auto getCvPorts()            { return makeSimpleSpan (cvPorts); }
    auto getCvPorts()      const { return makeSimpleSpan (cvPorts); }
    auto getAudioPorts()         { return makeSimpleSpan (audioPorts); }
    auto getAudioPorts()   const { return makeSimpleSpan (audioPorts); }
    auto getAtomPorts()          { return makeSimpleSpan (atomPorts); }
    auto getAtomPorts()    const { return makeSimpleSpan (atomPorts); }

    static Optional<Ports> getPorts (World& world, const UsefulUris& uris, const Plugin& plugin, SymbolMap& symap)
    {
        Ports value;
        b8 successful = true;

        const auto numPorts = plugin.getNumPorts();
        const auto timeNode = world.newUri (LV2_TIME__Position);

        for (u32 i = 0; i != numPorts; ++i)
        {
            const auto port = plugin.getPortByIndex (i);

            const PortHeader header { Txt::fromUTF8 (port.getName().getTyped()),
                                      Txt::fromUTF8 (port.getSymbol().getTyped()),
                                      i,
                                      port.getDirection (uris) };

            switch (port.getKind (uris))
            {
                case Port::Kind::control:
                {
                    value.controlPorts.push_back ({ header, ParameterInfo::getInfoForPort (uris, port) });
                    break;
                }

                case Port::Kind::cv:
                    value.cvPorts.push_back ({ header });
                    break;

                case Port::Kind::audio:
                {
                    value.audioPorts.push_back ({ header });
                    break;
                }

                case Port::Kind::atom:
                {
                    const auto supportsTime = port.supportsEvent (timeNode.get());
                    value.atomPorts.push_back ({ header,
                                                 (size_t) Ports::sequenceSize,
                                                 symap,
                                                 supportsTime ? SupportsTime::yes : SupportsTime::no });
                    break;
                }

                case Port::Kind::unknown:
                    successful = false;
                    break;
            }
        }

        for (auto& atomPort : value.atomPorts)
        {
            const auto port    = plugin.getPortByIndex (atomPort.header.index);
            const auto minSize = port.get (uris.mLV2_RESIZE_PORT__minimumSize.get());

            if (minSize != nullptr)
                atomPort.ensureSizeInBytes ((size_t) lilv_node_as_int (minSize.get()));
        }

        return successful ? makeOptional (std::move (value)) : nullopt;
    }

private:
    std::vector<ControlPort> controlPorts;
    std::vector<CVPort> cvPorts;
    std::vector<AudioPort> audioPorts;
    std::vector<AtomPort> atomPorts;
};

class InstanceWithSupports final : private FeaturesDataListener,
                                   private HandleHolder
{
public:
    InstanceWithSupports (World& world,
                          std::unique_ptr<SymbolMap>&& symapIn,
                          const Plugin& plugin,
                          Ports portsIn,
                          i32 initialBufferSize,
                          f64 sampleRate)
        : symap (std::move (symapIn)),
          ports (std::move (portsIn)),
          features (*this, *this, initialBufferSize, lv2_host::Ports::sequenceSize, &urids),
          instance (plugin, sampleRate, features.getFeatureArray()),
          workerInterface (instance.getExtensionData<LV2_Worker_Interface> (world.newUri (LV2_WORKER__interface)))
    {
        if (instance == nullptr)
            return;

        for (auto& port : ports.getControlPorts())
            instance.connectPort (port.header.index, &port.currentValue);

        for (auto& port : ports.getAtomPorts())
            instance.connectPort (port.header.index, port.data());

        for (auto& port : ports.getCvPorts())
            instance.connectPort (port.header.index, nullptr);

        for (auto& port : ports.getAudioPorts())
            instance.connectPort (port.header.index, nullptr);

        features.registerHandle (instance.getHandle());
    }

    ~InstanceWithSupports() override
    {
        if (instance != nullptr)
            features.deregisterHandle (instance.getHandle());
    }

    std::unique_ptr<SymbolMap> symap;
    const UsefulUrids urids { *symap };
    Ports ports;
    FeaturesData features;
    Instance instance;
    Messages<MessageHeader, RealtimeReadTrait> uiToProcessor;
    SharedResourcePointer<ProcessorToUi> processorToUi;

private:
    LV2_Handle handle = instance == nullptr ? nullptr : instance.getHandle();
    OptionalExtension<LV2_Worker_Interface> workerInterface;

    LV2_Handle getHandle() const override { return handle; }
    const LV2_Worker_Interface* getWorkerInterface() const override { return workerInterface.valid ? &workerInterface.extension : nullptr; }

    LV2_Resize_Port_Status resizeCallback (u32 index, size_t size) override
    {
        if (ports.getAtomPorts().size() <= index)
            return LV2_RESIZE_PORT_ERR_UNKNOWN;

        auto& port = ports.getAtomPorts()[index];

        if (port.header.direction != Port::Direction::output)
            return LV2_RESIZE_PORT_ERR_UNKNOWN;

        port.ensureSizeInBytes (size);
        instance.connectPort (port.header.index, port.data());

        return LV2_RESIZE_PORT_SUCCESS;
    }

    DRX_DECLARE_NON_COPYABLE (InstanceWithSupports)
    DRX_DECLARE_NON_MOVEABLE (InstanceWithSupports)
    DRX_LEAK_DETECTOR (InstanceWithSupports)
};

struct PortState
{
    ukk data;
    u32 size;
    u32 kind;
};

class PortMap
{
public:
    explicit PortMap (Ports& ports)
    {
        for (auto& port : ports.getControlPorts())
            symbolToControlPortMap.emplace (port.header.symbol, &port);
    }

    PortState getState (const Txt& symbol, const StatefulPortUrids& urids)
    {
        if (auto* port = getControlPortForSymbol (symbol))
            return { &port->currentValue, sizeof (f32), urids.Float };

        // At time of writing, lilv_state_new_from_instance did not attempt to store
        // the state of non-control ports. Perhaps that has changed?
        jassertfalse;
        return { nullptr, 0, 0 };
    }

    z0 restoreState (const Txt& symbol, const StatefulPortUrids& urids, PortState ps)
    {
        if (auto* port = getControlPortForSymbol (symbol))
        {
            port->currentValue = [&]() -> f32
            {
                if (ps.kind == urids.Float)
                    return getValueFrom<f32> (ps.data, ps.size);

                if (ps.kind == urids.Double)
                    return getValueFrom<f64> (ps.data, ps.size);

                if (ps.kind == urids.Int)
                    return getValueFrom<i32> (ps.data, ps.size);

                if (ps.kind == urids.Long)
                    return getValueFrom<z64> (ps.data, ps.size);

                jassertfalse;
                return {};
            }();
        }
        else
            jassertfalse; // Restoring state for non-control ports is not currently supported.
    }

private:
    template <typename Value>
    static f32 getValueFrom (ukk data, [[maybe_unused]] u32 size)
    {
        jassert (size == sizeof (Value));
        return (f32) readUnaligned<Value> (data);
    }

    ControlPort* getControlPortForSymbol (const Txt& symbol) const
    {
        const auto iter = symbolToControlPortMap.find (symbol);
        return iter != symbolToControlPortMap.cend() ? iter->second : nullptr;
    }

    std::map<Txt, ControlPort*> symbolToControlPortMap;
    DRX_LEAK_DETECTOR (PortMap)
};

class PluginState
{
public:
    PluginState() = default;

    explicit PluginState (LilvState* ptr)
        : state (ptr) {}

    const LilvState* get() const noexcept { return state.get(); }

    z0 restore (InstanceWithSupports& instance, PortMap& portMap) const
    {
        if (state != nullptr)
            SaveRestoreHandle { instance, portMap }.restore (state.get());
    }

    std::string toString (LilvWorld* world, LV2_URID_Map* map, LV2_URID_Unmap* unmap, tukk uri) const
    {
        std::unique_ptr<t8, FreeString> result { lilv_state_to_string (world,
                                                                         map,
                                                                         unmap,
                                                                         state.get(),
                                                                         uri,
                                                                         nullptr) };
        return std::string { result.get() };
    }

    Txt getLabel() const
    {
        return Txt::fromUTF8 (lilv_state_get_label (state.get()));
    }

    z0 setLabel (const Txt& label)
    {
        lilv_state_set_label (state.get(), label.toRawUTF8());
    }

    class SaveRestoreHandle
    {
    public:
        explicit SaveRestoreHandle (InstanceWithSupports& instanceIn, PortMap& portMap)
            : instance (instanceIn.instance.get()),
              features (instanceIn.features.getFeatureArray()),
              urids (*instanceIn.symap),
              map (portMap)
        {}

        PluginState save (const LilvPlugin* plugin, LV2_URID_Map* mapFeature)
        {
            return PluginState { lilv_state_new_from_instance (plugin,
                                                               instance,
                                                               mapFeature,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               getPortValue,
                                                               this,
                                                               LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE,
                                                               features ) };
        }

        z0 restore (const LilvState* stateIn)
        {
            lilv_state_restore (stateIn,
                                instance,
                                setPortValue,
                                this,
                                0,
                                features);
        }

    private:
        static ukk getPortValue (tukk portSymbol,
                                         uk userData,
                                         u32* size,
                                         u32* type)
        {
            auto& handle = *static_cast<SaveRestoreHandle*> (userData);

            const auto state = handle.map.getState (portSymbol, handle.urids);
            *size = state.size;
            *type = state.kind;
            return state.data;
        }

        static z0 setPortValue (tukk portSymbol,
                                  uk userData,
                                  ukk value,
                                  u32 size,
                                  u32 type)
        {
            const auto& handle = *static_cast<const SaveRestoreHandle*> (userData);
            handle.map.restoreState (portSymbol, handle.urids, { static_cast<tukk> (value), size, type });
        }

        LilvInstance* instance = nullptr;
        const LV2_Feature* const* features = nullptr;
        const StatefulPortUrids urids;
        PortMap& map;
    };

private:
    struct Free
    {
        z0 operator() (LilvState* ptr) const noexcept { lilv_state_free (ptr); }
    };

    std::unique_ptr<LilvState, Free> state;

    DRX_LEAK_DETECTOR (PluginState)
};

/*
    Wraps an LV2 UI bundle, providing access to the descriptor (if available).
*/
struct UiDescriptorLibrary
{
    using GetDescriptor = LV2UI_Descriptor* (*) (u32);

    UiDescriptorLibrary() = default;

    explicit UiDescriptorLibrary (const Txt& libraryPath)
        : library (std::make_unique<DynamicLibrary> (libraryPath)),
          getDescriptor (lv2_shared::wordCast<GetDescriptor> (library->getFunction ("lv2ui_descriptor"))) {}

    std::unique_ptr<DynamicLibrary> library;
    GetDescriptor getDescriptor = nullptr;
};

class UiDescriptorArgs
{
public:
    Txt libraryPath;
    Txt uiUri;

    auto withLibraryPath (Txt v) const noexcept { return with (&UiDescriptorArgs::libraryPath, v); }
    auto withUiUri       (Txt v) const noexcept { return with (&UiDescriptorArgs::uiUri,       v); }

private:
    UiDescriptorArgs with (Txt UiDescriptorArgs::* member, Txt value) const noexcept
    {
        return drx::lv2_host::with (*this, member, std::move (value));
    }
};

/*
    Stores a pointer to the descriptor for a specific UI bundle and UI URI.
*/
class UiDescriptor
{
public:
    UiDescriptor() = default;

    explicit UiDescriptor (const UiDescriptorArgs& args)
        : library (args.libraryPath),
          descriptor (extractUiDescriptor (library, args.uiUri.toRawUTF8()))
    {}

    z0 portEvent (LV2UI_Handle ui,
                    u32 portIndex,
                    u32 bufferSize,
                    u32 format,
                    ukk buffer) const
    {
        DRX_ASSERT_MESSAGE_THREAD

        if (auto* lv2Descriptor = get())
            if (auto* callback = lv2Descriptor->port_event)
                callback (ui, portIndex, bufferSize, format, buffer);
    }

    b8 hasExtensionData (World& world, tukk uid) const
    {
        return world.ask (world.newUri (descriptor->URI).get(),
                          world.newUri (LV2_CORE__extensionData).get(),
                          world.newUri (uid).get());
    }

    template <typename Extension>
    OptionalExtension<Extension> getExtensionData (World& world, tukk uid) const
    {
        if (! hasExtensionData (world, uid))
            return {};

        if (auto* lv2Descriptor = get())
            if (auto* extension = lv2Descriptor->extension_data)
                return OptionalExtension<Extension> (readUnaligned<Extension> (extension (uid)));

        return {};
    }

    const LV2UI_Descriptor* get() const noexcept { return descriptor; }

private:
    static const LV2UI_Descriptor* extractUiDescriptor (const UiDescriptorLibrary& lib, tukk uiUri)
    {
        if (lib.getDescriptor == nullptr)
            return nullptr;

        for (u32 i = 0;; ++i)
        {
            const auto* descriptor = lib.getDescriptor (i);

            if (descriptor == nullptr)
                return nullptr;

            if (strcmp (uiUri, descriptor->URI) == 0)
                return descriptor;
        }
    }

    UiDescriptorLibrary library;
    const LV2UI_Descriptor* descriptor = nullptr;

    DRX_LEAK_DETECTOR (UiDescriptor)
};

enum class Update { no, yes };

/*  A bit like the FlaggedFloatCache used by the VST3 host/client.

    While the FlaggedFloatCache always clears all set flags during the ifSet() call,
    this class stores the "value changed" flags for the processor and UI separately,
    so that they can be read at different rates.
*/
class ParameterValuesAndFlags
{
public:
    ParameterValuesAndFlags() = default;

    explicit ParameterValuesAndFlags (size_t sizeIn)
        : values (sizeIn),
          needsUiUpdate (sizeIn),
          needsProcessorUpdate (sizeIn)
    {
        std::fill (values.begin(), values.end(), 0.0f);
    }

    size_t size() const noexcept { return values.size(); }

    z0 set (size_t index, f32 value, Update update)
    {
        jassert (index < size());
        values[index].store (value, std::memory_order_relaxed);
        needsUiUpdate       .set (index, update == Update::yes ? 1 : 0);
        needsProcessorUpdate.set (index, update == Update::yes ? 1 : 0);
    }

    f32 get (size_t index) const noexcept
    {
        jassert (index < size());
        return values[index].load (std::memory_order_relaxed);
    }

    template <typename Callback>
    z0 ifProcessorValuesChanged (Callback&& callback)
    {
        ifChanged (needsProcessorUpdate, std::forward<Callback> (callback));
    }

    template <typename Callback>
    z0 ifUiValuesChanged (Callback&& callback)
    {
        ifChanged (needsUiUpdate, std::forward<Callback> (callback));
    }

    z0 clearUiFlags() { needsUiUpdate.clear(); }

private:
    template <typename Callback>
    z0 ifChanged (FlagCache<1>& flags, Callback&& callback)
    {
        flags.ifSet ([this, &callback] (size_t groupIndex, u32)
        {
            callback (groupIndex, values[groupIndex].load (std::memory_order_relaxed));
        });
    }

    std::vector<std::atomic<f32>> values;
    FlagCache<1> needsUiUpdate;
    FlagCache<1> needsProcessorUpdate;

    DRX_LEAK_DETECTOR (ParameterValuesAndFlags)
};

class LV2Parameter : public AudioPluginInstance::HostedParameter
{
public:
    LV2Parameter (const Txt& nameIn,
                  const ParameterInfo& infoIn,
                  ParameterValuesAndFlags& floatCache)
        : cache (floatCache),
          info (infoIn),
          range (info.min, info.max),
          name (nameIn),
          normalisedDefault (range.convertTo0to1 (infoIn.defaultValue))
    {}

    f32 getValue() const noexcept override
    {
        return range.convertTo0to1 (getDenormalisedValue());
    }

    z0 setValue (f32 f) override
    {
        cache.set ((size_t) getParameterIndex(), range.convertFrom0to1 (f), Update::yes);
    }

    z0 setDenormalisedValue (f32 denormalised)
    {
        cache.set ((size_t) getParameterIndex(), denormalised, Update::yes);
        sendValueChangedMessageToListeners (range.convertTo0to1 (denormalised));
    }

    z0 setDenormalisedValueWithoutTriggeringUpdate (f32 denormalised)
    {
        cache.set ((size_t) getParameterIndex(), denormalised, Update::no);
        sendValueChangedMessageToListeners (range.convertTo0to1 (denormalised));
    }

    f32 getDenormalisedValue() const noexcept
    {
        return cache.get ((size_t) getParameterIndex());
    }

    f32 getDefaultValue() const override { return normalisedDefault; }
    f32 getDenormalisedDefaultValue() const { return info.defaultValue; }

    f32 getValueForText (const Txt& text) const override
    {
        if (! info.isEnum)
            return range.convertTo0to1 (text.getFloatValue());

        const auto it = std::find_if (info.scalePoints.begin(),
                                      info.scalePoints.end(),
                                      [&] (const StoredScalePoint& stored) { return stored.label == text; });
        return it != info.scalePoints.end() ? range.convertTo0to1 (it->value) : normalisedDefault;
    }

    i32 getNumSteps() const override
    {
        if (info.isToggle)
            return 2;

        if (info.isEnum)
            return static_cast<i32> (info.scalePoints.size());

        if (info.isInteger)
            return static_cast<i32> (range.getRange().getLength()) + 1;

        return AudioProcessorParameter::getNumSteps();
    }

    b8 isDiscrete() const override { return info.isEnum || info.isInteger || info.isToggle; }
    b8 isBoolean() const override { return info.isToggle; }

    StringArray getAllValueStrings() const override
    {
        if (! info.isEnum)
            return {};

        return AudioProcessorParameter::getAllValueStrings();
    }

    Txt getText (f32 normalisedValue, i32) const override
    {
        const auto denormalised = range.convertFrom0to1 (normalisedValue);

        if (info.isEnum && ! info.scalePoints.empty())
        {
            // The normalised value might not correspond to the exact value of a scale point.
            // In this case, we find the closest label by searching the midpoints of the scale
            // point values.
            const auto index = std::distance (midPoints.begin(),
                                              std::lower_bound (midPoints.begin(), midPoints.end(), denormalised));
            jassert (isPositiveAndBelow (index, info.scalePoints.size()));
            return info.scalePoints[(size_t) index].label;
        }

        return getFallbackParameterString (denormalised);
    }

    Txt getParameterID() const override
    {
        return info.identifier;
    }

    Txt getName (i32 maxLength) const override
    {
        return name.substring (0, maxLength);
    }

    Txt getLabel() const override
    {
        // TODO
        return {};
    }

private:
    Txt getFallbackParameterString (f32 denormalised) const
    {
        if (info.isToggle)
            return denormalised > 0.0f ? "On" : "Off";

        if (info.isInteger)
            return Txt { static_cast<i32> (denormalised) };

        return Txt { denormalised };
    }

    static std::vector<f32> findScalePointMidPoints (const SafeSortedSet<StoredScalePoint>& set)
    {
        if (set.size() < 2)
            return {};

        std::vector<f32> result;
        result.reserve (set.size() - 1);

        for (auto it = std::next (set.begin()); it != set.end(); ++it)
            result.push_back ((std::prev (it)->value + it->value) * 0.5f);

        jassert (std::is_sorted (result.begin(), result.end()));
        jassert (result.size() + 1 == set.size());
        return result;
    }

    ParameterValuesAndFlags& cache;
    const ParameterInfo info;
    const std::vector<f32> midPoints = findScalePointMidPoints (info.scalePoints);
    const NormalisableRange<f32> range;
    const Txt name;
    const f32 normalisedDefault;

    DRX_LEAK_DETECTOR (LV2Parameter)
};

class UiInstanceArgs
{
public:
    File bundlePath;
    URL pluginUri;

    auto withBundlePath (File v) const noexcept { return withMember (*this, &UiInstanceArgs::bundlePath, std::move (v)); }
    auto withPluginUri  (URL v)  const noexcept { return withMember (*this, &UiInstanceArgs::pluginUri,  std::move (v)); }
};

/*
    Creates and holds a UI instance for a plugin with a specific URI, using the provided descriptor.
*/
class UiInstance
{
public:
    UiInstance (World& world,
                const UiDescriptor* descriptorIn,
                const UiInstanceArgs& args,
                const LV2_Feature* const* features,
                MessageBufferInterface<MessageHeader>& messagesIn,
                SymbolMap& map,
                PhysicalResizeListener& rl)
        : descriptor (descriptorIn),
          resizeListener (rl),
          uiToProcessor (messagesIn),
          mLV2_UI__floatProtocol   (map.map (LV2_UI__floatProtocol)),
          mLV2_ATOM__atomTransfer  (map.map (LV2_ATOM__atomTransfer)),
          mLV2_ATOM__eventTransfer (map.map (LV2_ATOM__eventTransfer)),
          instance (makeInstance (args, features)),
          idleCallback (getExtensionData<LV2UI_Idle_Interface> (world, LV2_UI__idleInterface))
    {
        jassert (descriptor != nullptr);
        jassert (widget != nullptr);

        ignoreUnused (resizeListener);
    }

    LV2UI_Handle getHandle() const noexcept { return instance.get(); }

    z0 pushMessage (MessageHeader header, u32 size, ukk buffer)
    {
        descriptor->portEvent (getHandle(), header.portIndex, size, header.protocol, buffer);
    }

    i32 idle()
    {
        if (idleCallback.valid && idleCallback.extension.idle != nullptr)
            return idleCallback.extension.idle (getHandle());

        return 0;
    }

    template <typename Extension>
    OptionalExtension<Extension> getExtensionData (World& world, tukk uid) const
    {
        return descriptor->getExtensionData<Extension> (world, uid);
    }

    Rectangle<i32> getDetectedViewBounds() const
    {
       #if DRX_MAC
        const auto frame = [(NSView*) widget frame];
        return { (i32) frame.size.width, (i32) frame.size.height };
       #elif DRX_LINUX || DRX_BSD
        Window root = 0;
        i32 wx = 0, wy = 0;
        u32 ww = 0, wh = 0, bw = 0, bitDepth = 0;

        XWindowSystemUtilities::ScopedXLock xLock;
        auto* display = XWindowSystem::getInstance()->getDisplay();
        X11Symbols::getInstance()->xGetGeometry (display,
                                                 (::Drawable) widget,
                                                 &root,
                                                 &wx,
                                                 &wy,
                                                 &ww,
                                                 &wh,
                                                 &bw,
                                                 &bitDepth);

        return { (i32) ww, (i32) wh };
       #elif DRX_WINDOWS
        RECT rect;
        GetWindowRect ((HWND) widget, &rect);
        return { rect.right - rect.left, rect.bottom - rect.top };
       #else
        return {};
       #endif
    }

    const UiDescriptor* descriptor = nullptr;

private:
    using Instance = std::unique_ptr<z0, z0 (*) (LV2UI_Handle)>;
    using Idle = i32 (*) (LV2UI_Handle);

    Instance makeInstance (const UiInstanceArgs& args, const LV2_Feature* const* features)
    {
        if (descriptor->get() == nullptr)
            return { nullptr, [] (LV2UI_Handle) {} };

        return Instance { descriptor->get()->instantiate (descriptor->get(),
                                                          args.pluginUri.toString (true).toRawUTF8(),
                                                          File::addTrailingSeparator (args.bundlePath.getFullPathName()).toRawUTF8(),
                                                          writeFunction,
                                                          this,
                                                          &widget,
                                                          features),
                          descriptor->get()->cleanup };
    }

    z0 write (u32 portIndex, u32 bufferSize, u32 protocol, ukk buffer)
    {
        const LV2_URID protocols[] { 0, mLV2_UI__floatProtocol, mLV2_ATOM__atomTransfer, mLV2_ATOM__eventTransfer };
        const auto it = std::find (std::begin (protocols), std::end (protocols), protocol);

        if (it != std::end (protocols))
        {
            uiToProcessor.pushMessage ({ portIndex, protocol }, bufferSize, buffer);
        }
    }

    static z0 writeFunction (LV2UI_Controller controller,
                               u32 portIndex,
                               u32 bufferSize,
                               u32 portProtocol,
                               ukk buffer)
    {
        jassert (controller != nullptr);
        static_cast<UiInstance*> (controller)->write (portIndex, bufferSize, portProtocol, buffer);
    }

    PhysicalResizeListener& resizeListener;
    MessageBufferInterface<MessageHeader>& uiToProcessor;
    LV2UI_Widget widget = nullptr;
    const LV2_URID mLV2_UI__floatProtocol;
    const LV2_URID mLV2_ATOM__atomTransfer;
    const LV2_URID mLV2_ATOM__eventTransfer;
    Instance instance;
    OptionalExtension<LV2UI_Idle_Interface> idleCallback;

   #if DRX_MAC
    NSViewFrameWatcher frameWatcher { (NSView*) widget, [this]
    {
        const auto bounds = getDetectedViewBounds();
        resizeListener.viewRequestedResizeInPhysicalPixels (bounds.getWidth(), bounds.getHeight());
    } };
   #elif DRX_WINDOWS
    WindowSizeChangeListener frameWatcher { (HWND) widget, resizeListener };
   #endif

    DRX_LEAK_DETECTOR (UiInstance)
};

struct TouchListener
{
    virtual ~TouchListener() = default;
    virtual z0 controlGrabbed (u32 port, b8 grabbed) = 0;
};

class AsyncFn final : public AsyncUpdater
{
public:
    explicit AsyncFn (std::function<z0()> callbackIn)
        : callback (std::move (callbackIn)) {}

    ~AsyncFn() override { cancelPendingUpdate(); }

    z0 handleAsyncUpdate() override { callback(); }

private:
    std::function<z0()> callback;
};

class UiFeaturesDataOptions
{
public:
    f32 initialScaleFactor = 0.0f, sampleRate = 0.0f;

    auto withInitialScaleFactor (f32 v) const { return with (&UiFeaturesDataOptions::initialScaleFactor, v); }
    auto withSampleRate         (f32 v) const { return with (&UiFeaturesDataOptions::sampleRate,         v); }

private:
    UiFeaturesDataOptions with (f32 UiFeaturesDataOptions::* member, f32 value) const
    {
        return drx::lv2_host::with (*this, member, value);
    }
};

class UiFeaturesData
{
public:
    UiFeaturesData (PhysicalResizeListener& rl,
                    TouchListener& tl,
                    LV2_Handle instanceIn,
                    LV2UI_Widget parentIn,
                    Instance::GetExtensionData getExtensionData,
                    const Ports& ports,
                    SymbolMap& symapIn,
                    const UiFeaturesDataOptions& optIn)
        : opts (optIn),
          resizeListener (rl),
          touchListener (tl),
          instance (instanceIn),
          parent (parentIn),
          symap (symapIn),
          dataAccess { getExtensionData },
          portIndices (makePortIndices (ports))
    {
    }

    const LV2_Feature* const* getFeatureArray() const noexcept { return features.pointers.data(); }

    static std::vector<Txt> getFeatureUris()
    {
        return Features::getUris (makeFeatures ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}));
    }

    Rectangle<i32> getLastRequestedBounds() const   { return { lastRequestedWidth, lastRequestedHeight }; }

private:
    static std::vector<LV2_Feature> makeFeatures (LV2UI_Resize* resize,
                                                  LV2UI_Widget parent,
                                                  LV2_Handle handle,
                                                  LV2_Extension_Data_Feature* data,
                                                  LV2_URID_Map* map,
                                                  LV2_URID_Unmap* unmap,
                                                  LV2UI_Port_Map* portMap,
                                                  LV2UI_Touch* touch,
                                                  LV2_Options_Option* options,
                                                  LV2_Log_Log* log)
    {
        return { LV2_Feature { LV2_UI__resize,          resize },
                 LV2_Feature { LV2_UI__parent,          parent },
                 LV2_Feature { LV2_UI__idleInterface,   nullptr },
                 LV2_Feature { LV2_INSTANCE_ACCESS_URI, handle },
                 LV2_Feature { LV2_DATA_ACCESS_URI,     data },
                 LV2_Feature { LV2_URID__map,           map },
                 LV2_Feature { LV2_URID__unmap,         unmap},
                 LV2_Feature { LV2_UI__portMap,         portMap },
                 LV2_Feature { LV2_UI__touch,           touch },
                 LV2_Feature { LV2_OPTIONS__options,    options },
                 LV2_Feature { LV2_LOG__log,            log } };
    }

    i32 resizeCallback (i32 width, i32 height)
    {
        lastRequestedWidth = width;
        lastRequestedHeight = height;
        resizeListener.viewRequestedResizeInPhysicalPixels (width, height);
        return 0;
    }

    static i32 resizeCallback (LV2UI_Feature_Handle handle, i32 width, i32 height)
    {
        return static_cast<UiFeaturesData*> (handle)->resizeCallback (width, height);
    }

    u32 portIndexCallback (tukk symbol) const
    {
        const auto it = portIndices.find (symbol);
        return it != portIndices.cend() ? it->second : LV2UI_INVALID_PORT_INDEX;
    }

    static u32 portIndexCallback (LV2UI_Feature_Handle handle, tukk symbol)
    {
        return static_cast<const UiFeaturesData*> (handle)->portIndexCallback (symbol);
    }

    z0 touchCallback (u32 portIndex, b8 grabbed) const
    {
        touchListener.controlGrabbed (portIndex, grabbed);
    }

    static z0 touchCallback (LV2UI_Feature_Handle handle, u32 index, b8 b)
    {
        return static_cast<const UiFeaturesData*> (handle)->touchCallback (index, b);
    }

    static std::map<Txt, u32> makePortIndices (const Ports& ports)
    {
        std::map<Txt, u32> result;

        ports.forEachPort ([&] (const PortHeader& header)
        {
            [[maybe_unused]] const auto emplaced = result.emplace (header.symbol, header.index);

            // This will complain if there are duplicate port symbols.
            jassert (emplaced.second);
        });

        return result;
    }

    const UiFeaturesDataOptions opts;
    PhysicalResizeListener& resizeListener;
    TouchListener& touchListener;
    LV2_Handle instance{};
    LV2UI_Widget parent{};
    SymbolMap& symap;
    const UsefulUrids urids { symap };
    Log log { &urids };
    i32 lastRequestedWidth = 0, lastRequestedHeight = 0;
    std::vector<LV2_Options_Option> options { { LV2_OPTIONS_INSTANCE,
                                                0,
                                                symap.map (LV2_UI__scaleFactor),
                                                sizeof (f32),
                                                symap.map (LV2_ATOM__Float),
                                                &opts.initialScaleFactor },
                                              { LV2_OPTIONS_INSTANCE,
                                                0,
                                                symap.map (LV2_PARAMETERS__sampleRate),
                                                sizeof (f32),
                                                symap.map (LV2_ATOM__Float),
                                                &opts.sampleRate },
                                              { LV2_OPTIONS_INSTANCE, 0, 0, 0, 0, nullptr } }; // The final entry must be nulled out
    LV2UI_Resize resize { this, resizeCallback };
    LV2_URID_Map map        = symap.getMapFeature();
    LV2_URID_Unmap unmap    = symap.getUnmapFeature();
    LV2UI_Port_Map portMap { this, portIndexCallback };
    LV2UI_Touch touch { this, touchCallback };
    LV2_Extension_Data_Feature dataAccess;
    std::map<Txt, u32> portIndices;
    Features features { makeFeatures (&resize,
                                      parent,
                                      instance,
                                      &dataAccess,
                                      &map,
                                      &unmap,
                                      &portMap,
                                      &touch,
                                      options.data(),
                                      log.getLogFeature()) };

    DRX_LEAK_DETECTOR (UiFeaturesData)
};

class UiInstanceWithSupports
{
public:
    UiInstanceWithSupports (World& world,
                            PhysicalResizeListener& resizeListener,
                            TouchListener& touchListener,
                            const UiDescriptor* descriptor,
                            const UiInstanceArgs& args,
                            LV2UI_Widget parent,
                            InstanceWithSupports& engineInstance,
                            const UiFeaturesDataOptions& opts)
        : features (resizeListener,
                    touchListener,
                    engineInstance.instance.getHandle(),
                    parent,
                    engineInstance.instance.getExtensionDataCallback(),
                    engineInstance.ports,
                    *engineInstance.symap,
                    opts),
          instance (world,
                    descriptor,
                    args,
                    features.getFeatureArray(),
                    engineInstance.uiToProcessor,
                    *engineInstance.symap,
                    resizeListener)
    {}

    UiFeaturesData features;
    UiInstance instance;

    DRX_LEAK_DETECTOR (UiInstanceWithSupports)
};

struct RequiredFeatures
{
    explicit RequiredFeatures (OwningNodes nodes)
        : values (std::move (nodes)) {}

    OwningNodes values;
};

struct OptionalFeatures
{
    explicit OptionalFeatures (OwningNodes nodes)
        : values (std::move (nodes)) {}

    OwningNodes values;
};

template <typename Range, typename Predicate>
static b8 noneOf (Range&& range, Predicate&& pred)
{
    // Not a mistake, this is for ADL
    using std::begin;
    using std::end;
    return std::none_of (begin (range), end (range), std::forward<Predicate> (pred));
}

class PeerChangedListener final : private ComponentMovementWatcher
{
public:
    PeerChangedListener (Component& c, std::function<z0()> peerChangedIn)
        : ComponentMovementWatcher (&c), peerChanged (std::move (peerChangedIn))
    {
    }

    z0 componentMovedOrResized (b8, b8) override {}
    z0 componentPeerChanged() override { NullCheckedInvocation::invoke (peerChanged); }
    z0 componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentVisibilityChanged;
    using ComponentMovementWatcher::componentMovedOrResized;

private:
    std::function<z0()> peerChanged;
};

struct ViewSizeListener final : private ComponentMovementWatcher
{
    ViewSizeListener (Component& c, PhysicalResizeListener& l)
        : ComponentMovementWatcher (&c), listener (l)
    {
    }

    z0 componentMovedOrResized (b8, b8 wasResized) override
    {
        if (wasResized)
        {
            const auto physicalSize = Desktop::getInstance().getDisplays()
                                                            .logicalToPhysical (getComponent()->localAreaToGlobal (getComponent()->getLocalBounds()));
            const auto width  = physicalSize.getWidth();
            const auto height = physicalSize.getHeight();

            if (width > 10 && height > 10)
                listener.viewRequestedResizeInPhysicalPixels (width, height);
        }
    }

    z0 componentPeerChanged() override {}
    z0 componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentVisibilityChanged;
    using ComponentMovementWatcher::componentMovedOrResized;

    PhysicalResizeListener& listener;
};

class ConfiguredEditorComponent final : public Component,
                                        private PhysicalResizeListener
{
public:
    ConfiguredEditorComponent (World& world,
                               InstanceWithSupports& instance,
                               UiDescriptor& uiDescriptor,
                               LogicalResizeListener& resizeListenerIn,
                               TouchListener& touchListener,
                               const Txt& uiBundleUri,
                               const UiFeaturesDataOptions& opts)
        : resizeListener (resizeListenerIn),
          floatUrid (instance.symap->map (LV2_ATOM__Float)),
          scaleFactorUrid (instance.symap->map (LV2_UI__scaleFactor)),
          uiInstance (new UiInstanceWithSupports (world,
                                                  *this,
                                                  touchListener,
                                                  &uiDescriptor,
                                                  UiInstanceArgs{}.withBundlePath (bundlePathFromUri (uiBundleUri.toRawUTF8()))
                                                                  .withPluginUri (URL (instance.instance.getUri())),
                                                  viewComponent.getWidget(),
                                                  instance,
                                                  opts)),
          resizeClient (uiInstance->instance.getExtensionData<LV2UI_Resize> (world, LV2_UI__resize)),
          optionsInterface (uiInstance->instance.getExtensionData<LV2_Options_Interface> (world, LV2_OPTIONS__interface))
    {
        jassert (uiInstance != nullptr);

        setOpaque (true);
        addAndMakeVisible (viewComponent);

        const auto boundsToUse = [&]
        {
            const auto requested = uiInstance->features.getLastRequestedBounds();

            if (requested.getWidth() > 10 && requested.getHeight() > 10)
                return requested;

            return uiInstance->instance.getDetectedViewBounds();
        }();

        const auto scaled = lv2ToComponentRect (boundsToUse);
        lastWidth  = scaled.getWidth();
        lastHeight = scaled.getHeight();
        setSize (lastWidth, lastHeight);
    }

    ~ConfiguredEditorComponent() override
    {
        viewComponent.prepareForDestruction();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::black);
    }

    z0 resized() override
    {
        viewComponent.setBounds (getLocalBounds());
    }

    z0 updateViewBounds()
    {
        // If the editor changed size as a result of a request from the client,
        // we shouldn't send a notification back to the client.
        if (uiInstance != nullptr)
        {
            if (resizeClient.valid && resizeClient.extension.ui_resize != nullptr)
            {
                const auto physicalSize = componentToLv2Rect (getLocalBounds());

                resizeClient.extension.ui_resize (uiInstance->instance.getHandle(),
                                                  physicalSize.getWidth(),
                                                  physicalSize.getHeight());
            }
        }
    }

    z0 pushMessage (MessageHeader header, u32 size, ukk buffer)
    {
        if (uiInstance != nullptr)
            uiInstance->instance.pushMessage (header, size, buffer);
    }

    i32 idle()
    {
        if (uiInstance != nullptr)
            return uiInstance->instance.idle();

        return 0;
    }

    z0 childBoundsChanged (Component* c) override
    {
        if (c == nullptr)
            resizeToFitView();
    }

    z0 setUserScaleFactor (f32 userScale) { userScaleFactor = userScale; }

    z0 sendScaleFactorToPlugin()
    {
        const auto factor = getEffectiveScale();

        const LV2_Options_Option options[]
        {
            { LV2_OPTIONS_INSTANCE, 0, scaleFactorUrid, sizeof (f32), floatUrid, &factor },
            { {}, {}, {}, {}, {}, {} }
        };

        if (optionsInterface.valid)
            optionsInterface.extension.set (uiInstance->instance.getHandle(), options);

        applyLastRequestedPhysicalSize();
    }

private:
    z0 viewRequestedResizeInPhysicalPixels (i32 width, i32 height) override
    {
        lastWidth = width;
        lastHeight = height;
        const auto logical = lv2ToComponentRect ({ width, height });
        resizeListener.viewRequestedResizeInLogicalPixels (logical.getWidth(), logical.getHeight());
    }

    z0 resizeToFitView()
    {
        viewComponent.fitToView();
        resizeListener.viewRequestedResizeInLogicalPixels (viewComponent.getWidth(), viewComponent.getHeight());
    }

    z0 applyLastRequestedPhysicalSize()
    {
        viewRequestedResizeInPhysicalPixels (lastWidth, lastHeight);
        viewComponent.forceViewToSize();
    }

    /*  Convert from the component's coordinate system to the hosted LV2's coordinate system. */
    Rectangle<i32> componentToLv2Rect (Rectangle<i32> r) const
    {
        return localAreaToGlobal (r) * nativeScaleFactor * getDesktopScaleFactor();
    }

    /*  Convert from the hosted LV2's coordinate system to the component's coordinate system. */
    Rectangle<i32> lv2ToComponentRect (Rectangle<i32> vr) const
    {
        return getLocalArea (nullptr, vr / (nativeScaleFactor * getDesktopScaleFactor()));
    }

    f32 getEffectiveScale() const     { return nativeScaleFactor * userScaleFactor; }

    // If possible, try to keep platform-specific handing restricted to the implementation of
    // ViewComponent. Keep the interface of ViewComponent consistent on all platforms.
   #if DRX_LINUX || DRX_BSD
    struct InnerHolder
    {
        struct Inner final : public XEmbedComponent
        {
            Inner() : XEmbedComponent (true, true)
            {
                setOpaque (true);
                addToDesktop (0);
            }
        };

        Inner inner;
    };

    struct ViewComponent final : public InnerHolder,
                                 public XEmbedComponent
    {
        explicit ViewComponent (PhysicalResizeListener& l)
            : XEmbedComponent ((u64) inner.getPeer()->getNativeHandle(), true, false),
              listener (inner, l)
        {
            setOpaque (true);
        }

        ~ViewComponent()
        {
            removeClient();
        }

        z0 prepareForDestruction()
        {
            inner.removeClient();
        }

        LV2UI_Widget getWidget() { return lv2_shared::wordCast<LV2UI_Widget> (inner.getHostWindowID()); }
        z0 forceViewToSize() {}
        z0 fitToView() {}

        ViewSizeListener listener;
    };
   #elif DRX_MAC
    struct ViewComponent final : public NSViewComponentWithParent
    {
        explicit ViewComponent (PhysicalResizeListener&)
            : NSViewComponentWithParent (WantsNudge::no) {}
        LV2UI_Widget getWidget() { return getView(); }
        z0 forceViewToSize() {}
        z0 fitToView() { resizeToFitView(); }
        z0 prepareForDestruction() {}
    };
   #elif DRX_WINDOWS
    struct ViewComponent final : public HWNDComponent
    {
        explicit ViewComponent (PhysicalResizeListener&)
        {
            setOpaque (true);
            inner.addToDesktop (0);

            if (auto* peer = inner.getPeer())
                setHWND (peer->getNativeHandle());
        }

        z0 paint (Graphics& g) override { g.fillAll (Colors::black); }

        LV2UI_Widget getWidget() { return getHWND(); }

        z0 forceViewToSize() { updateHWNDBounds(); }
        z0 fitToView() { resizeToFit(); }

        z0 prepareForDestruction() {}

    private:
        struct Inner final : public Component
        {
            Inner() { setOpaque (true); }
            z0 paint (Graphics& g) override { g.fillAll (Colors::black); }
        };

        Inner inner;
    };
   #else
    struct ViewComponent final : public Component
    {
        explicit ViewComponent (PhysicalResizeListener&) {}
        uk getWidget() { return nullptr; }
        z0 forceViewToSize() {}
        z0 fitToView() {}
        z0 prepareForDestruction() {}
    };
   #endif

    struct ScaleNotifierCallback
    {
        ConfiguredEditorComponent& window;

        z0 operator() (f32 platformScale) const
        {
            MessageManager::callAsync ([ref = Component::SafePointer<ConfiguredEditorComponent> (&window), platformScale]
            {
                if (auto* r = ref.getComponent())
                {
                    if (approximatelyEqual (std::exchange (r->nativeScaleFactor, platformScale), platformScale))
                        return;

                    r->nativeScaleFactor = platformScale;
                    r->sendScaleFactorToPlugin();
                }
            });
        }
    };

    LogicalResizeListener& resizeListener;
    i32 lastWidth = 0, lastHeight = 0;
    f32 nativeScaleFactor = 1.0f, userScaleFactor = 1.0f;
    NativeScaleFactorNotifier scaleNotifier { this, ScaleNotifierCallback { *this } };
    ViewComponent viewComponent { *this };
    LV2_URID floatUrid, scaleFactorUrid;
    std::unique_ptr<UiInstanceWithSupports> uiInstance;
    OptionalExtension<LV2UI_Resize> resizeClient;
    OptionalExtension<LV2_Options_Interface> optionsInterface;
    PeerChangedListener peerListener { *this, [this]
    {
        applyLastRequestedPhysicalSize();
    } };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfiguredEditorComponent)
};

//==============================================================================
/*  Interface to receive notifications when the Editor changes. */
struct EditorListener
{
    virtual ~EditorListener() = default;

    /*  The editor needs to be recreated in a few different scenarios, such as:
        - When the scale factor of the window changes, because we can only provide the
          scale factor to the view during construction
        - When the sample rate changes, because the processor also needs to be destroyed
          and recreated in this case

        This function will be called whenever the editor has been recreated, in order to
        allow the processor (or other listeners) to respond, e.g. by sending all of the
        current port/parameter values to the view.
    */
    virtual z0 viewCreated (UiEventListener* newListener) = 0;

    virtual z0 notifyEditorBeingDeleted() = 0;
};

/*  We can't pass the InstanceWithSupports directly to the editor, because
    it might be destroyed and reconstructed if the sample rate changes.
*/
struct InstanceProvider
{
    virtual ~InstanceProvider() noexcept = default;

    virtual InstanceWithSupports* getInstanceWithSupports() const = 0;
};

class Editor final : public AudioProcessorEditor,
                     public UiEventListener,
                     private LogicalResizeListener
{
public:
    Editor (World& worldIn,
            AudioPluginInstance& p,
            InstanceProvider& instanceProviderIn,
            UiDescriptor& uiDescriptorIn,
            TouchListener& touchListenerIn,
            EditorListener& listenerIn,
            const Txt& uiBundleUriIn,
            RequiredFeatures requiredIn,
            OptionalFeatures optionalIn)
        : AudioProcessorEditor (p),
          world (worldIn),
          instanceProvider (&instanceProviderIn),
          uiDescriptor (&uiDescriptorIn),
          touchListener (&touchListenerIn),
          listener (&listenerIn),
          uiBundleUri (uiBundleUriIn),
          required (std::move (requiredIn)),
          optional (std::move (optionalIn))
    {
        setResizable (isResizable (required, optional), false);
        setSize (10, 10);
        setOpaque (true);

        createView();

        instanceProvider->getInstanceWithSupports()->processorToUi->addUi (*this);
    }

    ~Editor() noexcept override
    {
        instanceProvider->getInstanceWithSupports()->processorToUi->removeUi (*this);

        listener->notifyEditorBeingDeleted();
    }

    z0 createView()
    {
        const auto initialScale = userScaleFactor * (f32) [&]
        {
            if (auto* p = getPeer())
                return p->getPlatformScaleFactor();

            return 1.0;
        }();

        const auto opts = UiFeaturesDataOptions{}.withInitialScaleFactor (initialScale)
                                                 .withSampleRate ((f32) processor.getSampleRate());
        configuredEditor = nullptr;
        configuredEditor = rawToUniquePtr (new ConfiguredEditorComponent (world,
                                                                          *instanceProvider->getInstanceWithSupports(),
                                                                          *uiDescriptor,
                                                                          *this,
                                                                          *touchListener,
                                                                          uiBundleUri,
                                                                          opts));
        parentHierarchyChanged();
        const auto initialSize = configuredEditor->getBounds();
        setSize (initialSize.getWidth(), initialSize.getHeight());

        listener->viewCreated (this);
    }

    z0 destroyView()
    {
        configuredEditor = nullptr;
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::black);
    }

    z0 resized() override
    {
        const ScopedValueSetter<b8> scope (resizeFromHost, true);

        if (auto* inner = configuredEditor.get())
        {
            inner->setBounds (getLocalBounds());
            inner->updateViewBounds();
        }
    }

    z0 parentHierarchyChanged() override
    {
        if (auto* comp = configuredEditor.get())
        {
            if (isShowing())
                addAndMakeVisible (comp);
            else
                removeChildComponent (comp);
        }
    }

    z0 pushMessage (MessageHeader header, u32 size, ukk buffer) override
    {
        if (auto* comp = configuredEditor.get())
            comp->pushMessage (header, size, buffer);
    }

    i32 idle() override
    {
        if (auto* comp = configuredEditor.get())
            return comp->idle();

        return 0;
    }

    z0 setScaleFactor (f32 newScale) override
    {
        userScaleFactor = newScale;

        if (configuredEditor != nullptr)
        {
            configuredEditor->setUserScaleFactor (userScaleFactor);
            configuredEditor->sendScaleFactorToPlugin();
        }
    }

private:
    b8 isResizable (const RequiredFeatures& requiredFeatures,
                      const OptionalFeatures& optionalFeatures) const
    {
        const auto uriMatches = [] (const LilvNode* node)
        {
            const auto* uri = lilv_node_as_uri (node);
            return std::strcmp (uri, LV2_UI__noUserResize) == 0;
        };

        return uiDescriptor->hasExtensionData (world, LV2_UI__resize)
               && ! uiDescriptor->hasExtensionData (world, LV2_UI__noUserResize)
               && noneOf (requiredFeatures.values, uriMatches)
               && noneOf (optionalFeatures.values, uriMatches);
    }

    b8 isScalable() const
    {
        return uiDescriptor->hasExtensionData (world, LV2_OPTIONS__interface);
    }

    z0 viewRequestedResizeInLogicalPixels (i32 width, i32 height) override
    {
        if (! resizeFromHost)
            setSize (width, height);
    }

    World& world;
    InstanceProvider* instanceProvider;
    UiDescriptor* uiDescriptor;
    TouchListener* touchListener;
    EditorListener* listener;
    Txt uiBundleUri;
    const RequiredFeatures required;
    const OptionalFeatures optional;
    std::unique_ptr<ConfiguredEditorComponent> configuredEditor;
    f32 userScaleFactor = 1.0f;
    b8 resizeFromHost = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};

class Uis
{
public:
    explicit Uis (const LilvPlugin* plugin) noexcept : uis (lilv_plugin_get_uis (plugin)) {}

    u32 size() const noexcept { return lilv_uis_size (uis.get()); }

    UisIterator begin() const noexcept { return UisIterator { uis.get() }; }
    UisIterator end()   const noexcept { return UisIterator{}; }

    const LilvUI* getByUri (const NodeUri& uri) const
    {
        return lilv_uis_get_by_uri (uis.get(), uri.get());
    }

private:
    struct Free
    {
        z0 operator() (LilvUIs* ptr) const noexcept { lilv_uis_free (ptr); }
    };

    std::unique_ptr<LilvUIs, Free> uis;
};

//==============================================================================
class PluginClass
{
public:
    explicit PluginClass (const LilvPluginClass* c) : pluginClass (c) {}

    NodeUri getParentUri() const noexcept   { return NodeUri::copy (lilv_plugin_class_get_parent_uri (pluginClass)); }
    NodeUri getUri() const noexcept         { return NodeUri::copy (lilv_plugin_class_get_uri (pluginClass)); }
    NodeString getLabel() const noexcept    { return NodeString::copy (lilv_plugin_class_get_label (pluginClass)); }
    OwningPluginClasses getChildren() const noexcept
    {
        return OwningPluginClasses { OwningPluginClasses::type { lilv_plugin_class_get_children (pluginClass) } };
    }

private:
    const LilvPluginClass* pluginClass = nullptr;
};

using FloatWriter = z0 (*) (LV2_Atom_Forge*, f32);

struct ParameterWriterUrids
{
    LV2_URID mLV2_PATCH__Set;
    LV2_URID mLV2_PATCH__property;
    LV2_URID mLV2_PATCH__value;
    LV2_URID mLV2_ATOM__eventTransfer;
};

struct MessageHeaderAndSize
{
    MessageHeader header;
    u32 size;
};

class ParameterWriter
{
public:
    ParameterWriter (ControlPort* p)
        : data (PortBacking { p }), kind (Kind::port) {}

    ParameterWriter (FloatWriter write, LV2_URID urid, u32 controlPortIndex)
        : data (PatchBacking { write, urid, controlPortIndex }), kind (Kind::patch) {}

    z0 writeToProcessor (const ParameterWriterUrids urids, LV2_Atom_Forge* forge, f32 value) const
    {
        switch (kind)
        {
            case Kind::patch:
            {
                if (forge != nullptr)
                {
                    lv2_atom_forge_frame_time (forge, 0);
                    writeSetToForge (urids, *forge, value);
                }

                break;
            }

            case Kind::port:
                data.port.port->currentValue = value;
                break;
        }
    }

    MessageHeaderAndSize writeToUi (const ParameterWriterUrids urids, LV2_Atom_Forge& forge, f32 value) const
    {
        const auto getWrittenBytes = [&]() -> u32
        {
            if (const auto* atom = convertToAtomPtr (forge.buf, forge.size))
                return (u32) (atom->size + sizeof (LV2_Atom));

            jassertfalse;
            return 0;
        };

        switch (kind)
        {
            case Kind::patch:
                writeSetToForge (urids, forge, value);
                return { { data.patch.controlPortIndex, urids.mLV2_ATOM__eventTransfer }, getWrittenBytes() };

            case Kind::port:
                lv2_atom_forge_raw (&forge, &value, sizeof (value));
                return { { data.port.port->header.index, 0 }, sizeof (value) };
        }

        return { { 0, 0 }, 0 };
    }

    const LV2_URID* getUrid() const
    {
        return kind == Kind::patch ? &data.patch.urid : nullptr;
    }

    u32k* getPortIndex() const
    {
        return kind == Kind::port ? &data.port.port->header.index : nullptr;
    }

private:
    z0 writeSetToForge (const ParameterWriterUrids urids, LV2_Atom_Forge& forge, f32 value) const
    {
        lv2_shared::ObjectFrame object { &forge, (u32) 0, urids.mLV2_PATCH__Set };

        lv2_atom_forge_key (&forge, urids.mLV2_PATCH__property);
        lv2_atom_forge_urid (&forge, data.patch.urid);

        lv2_atom_forge_key (&forge, urids.mLV2_PATCH__value);
        data.patch.write (&forge, value);
    }

    struct PortBacking
    {
        ControlPort* port;
    };

    struct PatchBacking
    {
        FloatWriter write;
        LV2_URID urid;
        u32 controlPortIndex;
    };

    union Data
    {
        static_assert (std::is_trivial_v<PortBacking>,  "PortBacking must be trivial");
        static_assert (std::is_trivial_v<PatchBacking>, "PatchBacking must be trivial");

        explicit Data (PortBacking p)  : port  (p) {}
        explicit Data (PatchBacking p) : patch (p) {}

        PortBacking port;
        PatchBacking patch;
    };

    enum class Kind { port, patch };

    Data data;
    Kind kind;

    DRX_LEAK_DETECTOR (ParameterWriter)
};

static Txt lilvNodeToUriString (const LilvNode* node)
{
    return node != nullptr ? Txt::fromUTF8 (lilv_node_as_uri (node)) : Txt{};
}

static Txt lilvNodeToString (const LilvNode* node)
{
    return node != nullptr ? Txt::fromUTF8 (lilv_node_as_string (node)) : Txt{};
}

/*  This holds all of the discovered groups in the plugin's manifest, and allows us to
    add parameters to these groups as we discover them.

    Once all the parameters have been added with addParameter(), you can call
    getTree() to convert this class' contents (which are optimised for fast lookup
    and modification) into a plain old AudioProcessorParameterGroup.
*/
class IntermediateParameterTree
{
public:
    explicit IntermediateParameterTree (World& worldIn)
        : world (worldIn)
    {
        const auto groups = getGroups (world);
        const auto symbolNode = world.newUri (LV2_CORE__symbol);
        const auto nameNode   = world.newUri (LV2_CORE__name);

        for (const auto& group : groups)
        {
            const auto symbol = lilvNodeToString (world.get (group.get(), symbolNode.get(), nullptr).get());
            const auto name   = lilvNodeToString (world.get (group.get(), nameNode  .get(), nullptr).get());
            owning.emplace (lilvNodeToUriString (group.get()),
                            std::make_unique<AudioProcessorParameterGroup> (symbol, name, "|"));
        }
    }

    z0 addParameter (StringRef group, std::unique_ptr<LV2Parameter> param)
    {
        if (param == nullptr)
            return;

        const auto it = owning.find (group);
        (it != owning.cend() ? *it->second : topLevel).addChild (std::move (param));
    }

    static AudioProcessorParameterGroup getTree (IntermediateParameterTree tree)
    {
        std::map<Txt, AudioProcessorParameterGroup*> nonowning;

        for (const auto& pair : tree.owning)
            nonowning.emplace (pair.first, pair.second.get());

        const auto groups = getGroups (tree.world);
        const auto subgroupNode = tree.world.newUri (LV2_PORT_GROUPS__subGroupOf);

        for (const auto& group : groups)
        {
            const auto innerIt = tree.owning.find (lilvNodeToUriString (group.get()));

            if (innerIt == tree.owning.cend())
                continue;

            const auto outer = lilvNodeToUriString (tree.world.get (group.get(), subgroupNode.get(), nullptr).get());
            const auto outerIt = nonowning.find (outer);

            if (outerIt != nonowning.cend() && containsParameters (outerIt->second))
                outerIt->second->addChild (std::move (innerIt->second));
        }

        for (auto& subgroup : tree.owning)
            if (containsParameters (subgroup.second.get()))
                tree.topLevel.addChild (std::move (subgroup.second));

        return std::move (tree.topLevel);
    }

private:
    static std::vector<OwningNode> getGroups (World& world)
    {
        std::vector<OwningNode> names;

        for (auto* uri : { LV2_PORT_GROUPS__Group, LV2_PORT_GROUPS__InputGroup, LV2_PORT_GROUPS__OutputGroup })
            for (const auto* group : world.findNodes (nullptr, world.newUri (LILV_NS_RDF "type").get(), world.newUri (uri).get()))
                names.push_back (OwningNode { lilv_node_duplicate (group) });

        return names;
    }

    static b8 containsParameters (const AudioProcessorParameterGroup* g)
    {
        if (g == nullptr)
            return false;

        for (auto* node : *g)
        {
            if (node->getParameter() != nullptr)
                return true;

            if (auto* group = node->getGroup())
                if (containsParameters (group))
                    return true;
        }

        return false;
    }

    World& world;
    AudioProcessorParameterGroup topLevel;
    std::map<Txt, std::unique_ptr<AudioProcessorParameterGroup>> owning;

    DRX_LEAK_DETECTOR (IntermediateParameterTree)
};

struct BypassParameter final : public LV2Parameter
{
    BypassParameter (const ParameterInfo& parameterInfo, ParameterValuesAndFlags& cacheIn)
        : LV2Parameter ("Bypass", parameterInfo, cacheIn) {}

    f32 getValue() const noexcept override
    {
        return LV2Parameter::getValue() > 0.0f ? 0.0f : 1.0f;
    }

    z0 setValue (f32 newValue) override
    {
        LV2Parameter::setValue (newValue > 0.0f ? 0.0f : 1.0f);
    }

    f32 getDefaultValue() const override                              { return 0.0f; }
    b8 isAutomatable() const override                                 { return true; }
    b8 isDiscrete() const override                                    { return true; }
    b8 isBoolean() const override                                     { return true; }
    i32 getNumSteps() const override                                    { return 2; }
    StringArray getAllValueStrings() const override                     { return { TRANS ("Off"), TRANS ("On") }; }
};

struct ParameterData
{
    ParameterInfo info;
    ParameterWriter writer;
    Txt group;
    Txt name;
};

template <typename T>
static auto getPortPointers (SimpleSpan<T> range)
{
    using std::begin;
    std::vector<decltype (&(*begin (range)))> result;

    for (auto& port : range)
    {
        result.resize (std::max ((size_t) (port.header.index + 1), result.size()), nullptr);
        result[port.header.index] = &port;
    }

    return result;
}

static std::unique_ptr<LV2Parameter> makeParameter (u32k* enabledPortIndex,
                                                    const ParameterData& data,
                                                    ParameterValuesAndFlags& cache)
{
    // The bypass parameter is a bit special, in that DRX expects the parameter to be a bypass
    // (where 0 is active, 1 is inactive), but the LV2 version is called "enabled" and has
    // different semantics (0 is inactive, 1 is active).
    // To work around this, we wrap the LV2 parameter in a special inverting DRX parameter.

    if (enabledPortIndex != nullptr)
        if (auto* index = data.writer.getPortIndex())
            if (*index == *enabledPortIndex)
                return std::make_unique<BypassParameter> (data.info, cache);

    return std::make_unique<LV2Parameter> (data.name, data.info, cache);
}

class ControlPortAccelerationStructure
{
public:
    ControlPortAccelerationStructure (SimpleSpan<ControlPort> controlPorts)
        : indexedControlPorts (getPortPointers (controlPorts))
    {
        for (const auto& port : controlPorts)
            if (port.header.direction == Port::Direction::output)
                outputPorts.push_back (&port);
    }

    const std::vector<ControlPort*>& getIndexedControlPorts() { return indexedControlPorts; }

    ControlPort* getControlPortByIndex (u32 index) const
    {
        if (isPositiveAndBelow (index, indexedControlPorts.size()))
            return indexedControlPorts[index];

        return nullptr;
    }

    z0 writeOutputPorts (UiEventListener* target, MessageBufferInterface<UiMessageHeader>& uiMessages) const
    {
        if (target == nullptr)
            return;

        for (const auto* port : outputPorts)
        {
            const auto chars = toChars (port->currentValue);
            uiMessages.pushMessage ({ target, { port->header.index, 0 } }, (u32) chars.size(), chars.data());
        }
    }

private:
    std::vector<ControlPort*> indexedControlPorts;
    std::vector<const ControlPort*> outputPorts;
};

class ParameterValueCache
{
public:
    /*  This takes some information about all the parameters that this plugin wants to expose,
        then builds and installs the actual parameters.
    */
    ParameterValueCache (AudioPluginInstance& processor,
                         World& world,
                         LV2_URID_Map mapFeature,
                         const std::vector<ParameterData>& data,
                         ControlPort* enabledPort)
        : uiForge (mapFeature),
          cache (data.size())
    {
        // Parameter indices are unknown until we add the parameters to the processor.
        // This map lets us keep track of which ParameterWriter corresponds to each parameter.
        // After the parameters have been added to the processor, we'll convert this
        // to a simple vector that stores each ParameterWriter at the same index
        // as the corresponding parameter.
        std::map<AudioProcessorParameter*, ParameterWriter> writerForParameter;

        IntermediateParameterTree tree { world };

        const auto* enabledPortIndex = enabledPort != nullptr ? &enabledPort->header.index
                                                              : nullptr;

        for (const auto& item : data)
        {
            auto param = makeParameter (enabledPortIndex, item, cache);

            if (auto* urid = item.writer.getUrid())
                urids.emplace (*urid, param.get());

            if (auto* index = item.writer.getPortIndex())
                portIndices.emplace (*index, param.get());

            writerForParameter.emplace (param.get(), item.writer);

            tree.addParameter (item.group, std::move (param));
        }

        processor.setHostedParameterTree (IntermediateParameterTree::getTree (std::move (tree)));

        // Build the vector of writers
        writers.reserve (data.size());

        for (auto* param : processor.getParameters())
        {
            const auto it = writerForParameter.find (param);
            jassert (it != writerForParameter.end());
            writers.push_back (it->second); // The writer must exist at the same index as the parameter!
        }

        // Duplicate port indices or urids?
        jassert (processor.getParameters().size() == (i32) (urids.size() + portIndices.size()));

        // Set parameters to default values
        const auto setToDefault = [] (auto& container)
        {
            for (auto& item : container)
                item.second->setDenormalisedValueWithoutTriggeringUpdate (item.second->getDenormalisedDefaultValue());
        };

        setToDefault (urids);
        setToDefault (portIndices);
    }

    z0 postChangedParametersToProcessor (const ParameterWriterUrids helperUrids,
                                           LV2_Atom_Forge* forge)
    {
        cache.ifProcessorValuesChanged ([&] (size_t index, f32 value)
                                        {
                                            writers[index].writeToProcessor (helperUrids, forge, value);
                                        });
    }

    z0 postChangedParametersToUi (UiEventListener* target,
                                    const ParameterWriterUrids helperUrids,
                                    MessageBufferInterface<UiMessageHeader>& uiMessages)
    {
        if (target == nullptr)
            return;

        cache.ifUiValuesChanged ([&] (size_t index, f32 value)
                                 {
                                     writeParameterToUi (target, writers[index], value, helperUrids, uiMessages);
                                 });
    }

    z0 postAllParametersToUi (UiEventListener* target,
                                const ParameterWriterUrids helperUrids,
                                MessageBufferInterface<UiMessageHeader>& uiMessages)
    {
        if (target == nullptr)
            return;

        const auto numWriters = writers.size();

        for (size_t i = 0; i < numWriters; ++i)
            writeParameterToUi (target, writers[i], cache.get (i), helperUrids, uiMessages);

        cache.clearUiFlags();
    }

    LV2Parameter* getParamByUrid (LV2_URID urid) const
    {
        const auto it = urids.find (urid);
        return it != urids.end() ? it->second : nullptr;
    }

    LV2Parameter* getParamByPortIndex (u32 portIndex) const
    {
        const auto it = portIndices.find (portIndex);
        return it != portIndices.end() ? it->second : nullptr;
    }

    z0 updateFromControlPorts (const ControlPortAccelerationStructure& ports) const
    {
        for (const auto& pair : portIndices)
            if (auto* port = ports.getControlPortByIndex (pair.first))
                if (auto* param = pair.second)
                    param->setDenormalisedValueWithoutTriggeringUpdate (port->currentValue);
    }

private:
    z0 writeParameterToUi (UiEventListener* target,
                             const ParameterWriter& writer,
                             f32 value,
                             const ParameterWriterUrids helperUrids,
                             MessageBufferInterface<UiMessageHeader>& uiMessages)
    {
        DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        uiForge.setBuffer (forgeStorage.data(), forgeStorage.size());
        const auto messageHeader = writer.writeToUi (helperUrids, *uiForge.get(), value);
        uiMessages.pushMessage ({ target, messageHeader.header }, messageHeader.size, forgeStorage.data());
    }

    SingleSizeAlignedStorage<8> forgeStorage { 256 };
    lv2_shared::AtomForge uiForge;

    ParameterValuesAndFlags cache;
    std::vector<ParameterWriter> writers;
    std::map<LV2_URID, LV2Parameter*> urids;
    std::map<u32, LV2Parameter*> portIndices;

    DRX_LEAK_DETECTOR (ParameterValueCache)
};

struct PatchSetCallback
{
    explicit PatchSetCallback (ParameterValueCache& x) : cache (x) {}

    // If we receive a patch set from the processor, we can assume that the UI will
    // put itself into the correct state when it receives the message.
    z0 setParameter (LV2_URID property, f32 value) const noexcept
    {
        if (auto* param = cache.getParamByUrid (property))
            param->setDenormalisedValueWithoutTriggeringUpdate (value);
    }

    // TODO gesture support will probably go here, once it's part of the LV2 spec

    ParameterValueCache& cache;
};

struct SupportedParameter
{
    ParameterInfo info;
    b8 supported;
    LV2_URID type;
};

static SupportedParameter getInfoForPatchParameter (World& worldIn,
                                                    const UsefulUrids& urids,
                                                    const NodeUri& property)
{
    const auto rangeUri = worldIn.newUri (LILV_NS_RDFS "range");
    const auto type = worldIn.get (property.get(), rangeUri.get(), nullptr);

    if (type == nullptr)
        return { {}, false, {} };

    const auto typeUrid = urids.symap.map (lilv_node_as_uri (type.get()));

    const LV2_URID types[] { urids.mLV2_ATOM__Int,
                             urids.mLV2_ATOM__Long,
                             urids.mLV2_ATOM__Float,
                             urids.mLV2_ATOM__Double,
                             urids.mLV2_ATOM__Bool };

    if (std::find (std::begin (types), std::end (types), typeUrid) == std::end (types))
        return { {}, false, {} };

    const auto getValue = [&] (tukk uri, f32 fallback)
    {
        return Port::getFloatValue (worldIn.get (property.get(), worldIn.newUri (uri).get(), nullptr).get(), fallback);
    };

    const auto hasPortProperty = [&] (tukk uri)
    {
        return worldIn.ask (property.get(),
                            worldIn.newUri (LV2_CORE__portProperty).get(),
                            worldIn.newUri (uri).get());
    };

    const auto metadataScalePoints = worldIn.findNodes (property.get(),
                                                        worldIn.newUri (LV2_CORE__scalePoint).get(),
                                                        nullptr);
    SafeSortedSet<StoredScalePoint> parsedScalePoints;

    for (const auto* scalePoint : metadataScalePoints)
    {
        const auto label = worldIn.get (scalePoint, worldIn.newUri (LILV_NS_RDFS "label").get(), nullptr);
        const auto value = worldIn.get (scalePoint, worldIn.newUri (LILV_NS_RDF "value").get(), nullptr);

        if (label != nullptr && value != nullptr)
            parsedScalePoints.insert ({ lilv_node_as_string (label.get()), lilv_node_as_float (value.get()) });
        else
            jassertfalse; // A ScalePoint must have both a rdfs:label and a rdf:value
    }

    const auto minimum = getValue (LV2_CORE__minimum, 0.0f);
    const auto maximum = getValue (LV2_CORE__maximum, 1.0f);

    return { { std::move (parsedScalePoints),
               "des:" + Txt::fromUTF8 (property.getTyped()),
               getValue (LV2_CORE__default, (minimum + maximum) * 0.5f),
               minimum,
               maximum,
               typeUrid == urids.mLV2_ATOM__Bool || hasPortProperty (LV2_CORE__toggled),
               typeUrid == urids.mLV2_ATOM__Int || typeUrid == urids.mLV2_ATOM__Long,
               hasPortProperty (LV2_CORE__enumeration) },
             true,
             typeUrid };
}

static std::vector<ParameterData> getPortBasedParameters (World& world,
                                                          const Plugin& plugin,
                                                          std::initializer_list<const ControlPort*> hiddenPorts,
                                                          SimpleSpan<ControlPort> controlPorts)
{
    std::vector<ParameterData> result;

    const auto groupNode = world.newUri (LV2_PORT_GROUPS__group);

    for (auto& port : controlPorts)
    {
        if (port.header.direction != Port::Direction::input)
            continue;

        if (std::find (std::begin (hiddenPorts), std::end (hiddenPorts), &port) != std::end (hiddenPorts))
            continue;

        const auto lilvPort = plugin.getPortByIndex (port.header.index);
        const auto group = lilvNodeToUriString (lilvPort.get (groupNode.get()).get());

        result.push_back ({ port.info, ParameterWriter { &port }, group, port.header.name });
    }

    return result;
}

static z0 writeFloatToForge  (LV2_Atom_Forge* forge, f32 value) { lv2_atom_forge_float  (forge, value); }
static z0 writeDoubleToForge (LV2_Atom_Forge* forge, f32 value) { lv2_atom_forge_double (forge, (f64) value); }
static z0 writeIntToForge    (LV2_Atom_Forge* forge, f32 value) { lv2_atom_forge_int    (forge, (i32) value); }
static z0 writeLongToForge   (LV2_Atom_Forge* forge, f32 value) { lv2_atom_forge_long   (forge, (z64) value); }
static z0 writeBoolToForge   (LV2_Atom_Forge* forge, f32 value) { lv2_atom_forge_bool   (forge, value > 0.5f); }

static std::vector<ParameterData> getPatchBasedParameters (World& world,
                                                           const Plugin& plugin,
                                                           const UsefulUrids& urids,
                                                           u32 controlPortIndex)
{
    // This returns our writable parameters in an indeterminate order.
    // We want our parameters to be in a consistent order between runs, so
    // we'll create all the parameters in one pass, sort them, and then
    // add them in a separate pass.
    const auto writableControls = world.findNodes (plugin.getUri().get(),
                                                   world.newUri (LV2_PATCH__writable).get(),
                                                   nullptr);

    struct DataAndUri
    {
        ParameterData data;
        Txt uri;
    };

    std::vector<DataAndUri> resultWithUris;

    const auto groupNode = world.newUri (LV2_PORT_GROUPS__group);

    for (auto* ctrl : writableControls)
    {
        const auto labelString = [&]
        {
            if (auto label = world.get (ctrl, world.newUri (LILV_NS_RDFS "label").get(), nullptr))
                return Txt::fromUTF8 (lilv_node_as_string (label.get()));

            return Txt();
        }();

        const auto uri = Txt::fromUTF8 (lilv_node_as_uri (ctrl));
        const auto info = getInfoForPatchParameter (world, urids, world.newUri (uri.toRawUTF8()));

        if (! info.supported)
            continue;

        const auto write = [&]
        {
            if (info.type == urids.mLV2_ATOM__Int)
                return writeIntToForge;

            if (info.type == urids.mLV2_ATOM__Long)
                return writeLongToForge;

            if (info.type == urids.mLV2_ATOM__Double)
                return writeDoubleToForge;

            if (info.type == urids.mLV2_ATOM__Bool)
                return writeBoolToForge;

            return writeFloatToForge;
        }();

        const auto group = lilvNodeToUriString (world.get (ctrl, groupNode.get(), nullptr).get());
        resultWithUris.push_back ({ { info.info,
                                      ParameterWriter { write, urids.symap.map (uri.toRawUTF8()), controlPortIndex },
                                      group,
                                      labelString },
                                    uri });
    }

    const auto compareUris = [] (const DataAndUri& a, const DataAndUri& b) { return a.uri < b.uri; };
    std::sort (resultWithUris.begin(), resultWithUris.end(), compareUris);

    std::vector<ParameterData> result;

    for (const auto& item : resultWithUris)
        result.push_back (item.data);

    return result;
}

static std::vector<ParameterData> getDrxParameterInfo (World& world,
                                                        const Plugin& plugin,
                                                        const UsefulUrids& urids,
                                                        std::initializer_list<const ControlPort*> hiddenPorts,
                                                        SimpleSpan<ControlPort> controlPorts,
                                                        u32 controlPortIndex)
{
    auto port  = getPortBasedParameters  (world, plugin, hiddenPorts, controlPorts);
    auto patch = getPatchBasedParameters (world, plugin, urids, controlPortIndex);

    port.insert (port.end(), patch.begin(), patch.end());
    return port;
}

// Rather than sprinkle #ifdef everywhere, risking the wrath of the entire C++
// standards committee, we put all of our conditionally-compiled stuff into a
// specialised template that compiles away to nothing when editor support is
// not available.
#if DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD
 constexpr auto editorFunctionalityEnabled = true;
#else
 constexpr auto editorFunctionalityEnabled = false;
#endif

template <b8 editorEnabled = editorFunctionalityEnabled> class OptionalEditor;

template <>
class OptionalEditor<true>
{
public:
    OptionalEditor (Txt uiBundleUriIn, UiDescriptor uiDescriptorIn, std::function<z0()> timerCallback)
        : uiBundleUri (std::move (uiBundleUriIn)),
          uiDescriptor (std::move (uiDescriptorIn)),
          changedParameterFlusher (std::move (timerCallback)) {}

    z0 createView()
    {
        if (auto* editor = editorPointer.getComponent())
            editor->createView();
    }

    z0 destroyView()
    {
        if (auto* editor = editorPointer.getComponent())
            editor->destroyView();
    }

    std::unique_ptr<AudioProcessorEditor> createEditor (World& world,
                                                        AudioPluginInstance& p,
                                                        InstanceProvider& instanceProviderIn,
                                                        TouchListener& touchListenerIn,
                                                        EditorListener& listenerIn)
    {
        if (! hasEditor())
            return nullptr;

        const auto queryFeatures = [this, &world] (tukk kind)
        {
            return world.findNodes (world.newUri (uiDescriptor.get()->URI).get(),
                                    world.newUri (kind).get(),
                                    nullptr);
        };

        auto newEditor = std::make_unique<Editor> (world,
                                                   p,
                                                   instanceProviderIn,
                                                   uiDescriptor,
                                                   touchListenerIn,
                                                   listenerIn,
                                                   uiBundleUri,
                                                   RequiredFeatures { queryFeatures (LV2_CORE__requiredFeature) },
                                                   OptionalFeatures { queryFeatures (LV2_CORE__optionalFeature) });

        editorPointer = newEditor.get();

        changedParameterFlusher.startTimerHz (60);

        return newEditor;
    }

    b8 hasEditor() const
    {
        return uiDescriptor.get() != nullptr;
    }

    z0 prepareToDestroyEditor()
    {
        changedParameterFlusher.stopTimer();
    }

private:
    Component::SafePointer<Editor> editorPointer = nullptr;
    Txt uiBundleUri;
    UiDescriptor uiDescriptor;
    TimedCallback changedParameterFlusher;
};

template <>
class OptionalEditor<false>
{
public:
    OptionalEditor (Txt, UiDescriptor, std::function<z0()>) {}

    z0 createView() {}
    z0 destroyView() {}

    std::unique_ptr<AudioProcessorEditor> createEditor (World&,
                                                        AudioPluginInstance&,
                                                        InstanceProvider&,
                                                        TouchListener&,
                                                        EditorListener&)
    {
        return nullptr;
    }

    b8 hasEditor() const { return false; }
    z0 prepareToDestroyEditor() {}
};

//==============================================================================
class LV2AudioPluginInstance final : public AudioPluginInstance,
                                     private TouchListener,
                                     private EditorListener,
                                     private InstanceProvider
{
public:
    LV2AudioPluginInstance (std::shared_ptr<World> worldIn,
                            const Plugin& pluginIn,
                            const UsefulUris& uris,
                            std::unique_ptr<InstanceWithSupports>&& in,
                            PluginDescription&& desc,
                            std::vector<Txt> knownPresetUris,
                            PluginState stateToApply,
                            Txt uiBundleUriIn,
                            UiDescriptor uiDescriptorIn)
        : LV2AudioPluginInstance (worldIn,
                                  pluginIn,
                                  std::move (in),
                                  std::move (desc),
                                  std::move (knownPresetUris),
                                  std::move (stateToApply),
                                  std::move (uiBundleUriIn),
                                  std::move (uiDescriptorIn),
                                  getParsedBuses (*worldIn, pluginIn, uris)) {}

    z0 fillInPluginDescription (PluginDescription& d) const override { d = description; }

    const Txt getName() const override { return description.name; }

    z0 prepareToPlay (f64 sampleRate, i32 numSamples) override
    {
        // In REAPER, changing the sample rate will deactivate the plugin,
        // save its state, destroy it, create a new instance, restore the
        // state, and then activate the new instance.
        // We'll do the same, because there's no way to retroactively change the
        // plugin sample rate.
        // This is a bit expensive, so try to avoid changing the sample rate too
        // frequently.

        // In addition to the above, we also need to destroy the custom view,
        // and recreate it after creating the new plugin instance.
        // Ideally this should all happen in the same Component.

        deactivate();
        destroyView();

        MemoryBlock mb;
        getStateInformation (mb);

        instance = std::make_unique<InstanceWithSupports> (*world,
                                                           std::move (instance->symap),
                                                           plugin,
                                                           std::move (instance->ports),
                                                           numSamples,
                                                           sampleRate);

        // prepareToPlay is *guaranteed* not to be called concurrently with processBlock
        setStateInformationImpl (mb.getData(), (i32) mb.getSize(), ConcurrentWithAudioCallback::no);

        jassert (numSamples == instance->features.getMaxBlockSize());

        optionalEditor.createView();
        activate();
    }

    z0 releaseResources() override { deactivate(); }

    using AudioPluginInstance::processBlock;
    using AudioPluginInstance::processBlockBypassed;

    z0 processBlock (AudioBuffer<f32>& audio, MidiBuffer& midi) override
    {
        processBlockImpl (audio, midi);
    }

    z0 processBlockBypassed (AudioBuffer<f32>& audio, MidiBuffer& midi) override
    {
        if (bypassParam != nullptr)
            processBlockImpl (audio, midi);
        else
            AudioPluginInstance::processBlockBypassed (audio, midi);
    }

    f64 getTailLengthSeconds() const override { return {}; } // TODO

    b8 acceptsMidi() const override
    {
        if (instance == nullptr)
            return false;

        auto ports = instance->ports.getAtomPorts();

        return std::any_of (ports.begin(), ports.end(), [&] (const AtomPort& a)
        {
            if (a.header.direction != Port::Direction::input)
                return false;

            return portAtIndexSupportsMidi (a.header.index);
        });
    }

    b8 producesMidi() const override
    {
        if (instance == nullptr)
            return false;

        auto ports = instance->ports.getAtomPorts();

        return std::any_of (ports.begin(), ports.end(), [&] (const AtomPort& a)
        {
            if (a.header.direction != Port::Direction::output)
                return false;

            return portAtIndexSupportsMidi (a.header.index);
        });
    }

    AudioProcessorEditor* createEditor() override
    {
        return optionalEditor.createEditor (*world, *this, *this, *this, *this).release();
    }

    b8 hasEditor() const override
    {
        return optionalEditor.hasEditor();
    }

    i32 getNumPrograms() override { return (i32) presetUris.size(); }

    i32 getCurrentProgram() override
    {
        return lastAppliedPreset;
    }

    z0 setCurrentProgram (i32 newProgram) override
    {
        DRX_ASSERT_MESSAGE_THREAD;

        if (! isPositiveAndBelow (newProgram, presetUris.size()))
            return;

        lastAppliedPreset = newProgram;
        applyStateWithAppropriateLocking (loadStateWithUri (presetUris[(size_t) newProgram]),
                                          ConcurrentWithAudioCallback::yes);
    }

    const Txt getProgramName (i32 program) override
    {
        DRX_ASSERT_MESSAGE_THREAD;

        if (isPositiveAndBelow (program, presetUris.size()))
            return loadStateWithUri (presetUris[(size_t) program]).getLabel();

        return {};
    }

    z0 changeProgramName (i32 program, const Txt& label) override
    {
        DRX_ASSERT_MESSAGE_THREAD;

        if (isPositiveAndBelow (program, presetUris.size()))
            loadStateWithUri (presetUris[(size_t) program]).setLabel (label);
    }

    z0 getStateInformation (MemoryBlock& block) override
    {
        DRX_ASSERT_MESSAGE_THREAD;

        // TODO where should the state URI come from?
        PortMap portStateManager (instance->ports);
        const auto stateUri = Txt::fromUTF8 (instance->instance.getUri()) + "/savedState";
        auto mapFeature = instance->symap->getMapFeature();
        auto unmapFeature = instance->symap->getUnmapFeature();
        const auto state = PluginState::SaveRestoreHandle (*instance, portStateManager).save (plugin.get(), &mapFeature);
        const auto string = state.toString (world->get(), &mapFeature, &unmapFeature, stateUri.toRawUTF8());
        block.replaceAll (string.data(), string.size());
    }

    z0 setStateInformation (ukk data, i32 size) override
    {
        setStateInformationImpl (data, size, ConcurrentWithAudioCallback::yes);
    }

    z0 setNonRealtime (b8 newValue) noexcept override
    {
        DRX_ASSERT_MESSAGE_THREAD;

        AudioPluginInstance::setNonRealtime (newValue);
        instance->features.setNonRealtime (newValue);
    }

    b8 isBusesLayoutSupported (const BusesLayout& layout) const override
    {
        for (const auto& pair : { std::make_tuple (&layout.inputBuses,  &declaredBusLayout.inputs),
                                  std::make_tuple (&layout.outputBuses, &declaredBusLayout.outputs) })
        {
            const auto& requested = *std::get<0> (pair);
            const auto& allowed   = *std::get<1> (pair);

            if ((size_t) requested.size() != allowed.size())
                return false;

            for (size_t busIndex = 0; busIndex < allowed.size(); ++busIndex)
            {
                const auto& requestedBus = requested[(i32) busIndex];
                const auto& allowedBus = allowed[busIndex];

                if (! allowedBus.isCompatible (requestedBus))
                    return false;
            }
        }

        return true;
    }

    z0 processorLayoutsChanged() override { ioMap = lv2_shared::PortToAudioBufferMap { getBusesLayout(), declaredBusLayout }; }

    AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

private:
    enum class ConcurrentWithAudioCallback { no, yes };

    LV2AudioPluginInstance (std::shared_ptr<World> worldIn,
                            const Plugin& pluginIn,
                            std::unique_ptr<InstanceWithSupports>&& in,
                            PluginDescription&& desc,
                            std::vector<Txt> knownPresetUris,
                            PluginState stateToApply,
                            Txt uiBundleUriIn,
                            UiDescriptor uiDescriptorIn,
                            const lv2_shared::ParsedBuses& parsedBuses)
        : AudioPluginInstance (getBusesProperties (parsedBuses, *worldIn)),
          declaredBusLayout (parsedBuses),
          world (std::move (worldIn)),
          plugin (pluginIn),
          description (std::move (desc)),
          presetUris (std::move (knownPresetUris)),
          instance (std::move (in)),
          optionalEditor (std::move (uiBundleUriIn),
                          std::move (uiDescriptorIn),
                          [this] { postChangedParametersToUi(); })
    {
        applyStateWithAppropriateLocking (std::move (stateToApply), ConcurrentWithAudioCallback::no);
    }

    z0 setStateInformationImpl (ukk data, i32 size, ConcurrentWithAudioCallback concurrent)
    {
        DRX_ASSERT_MESSAGE_THREAD;

        if (data == nullptr || size == 0)
            return;

        auto begin = static_cast<tukk> (data);
        std::vector<t8> copy (begin, begin + size);
        copy.push_back (0);
        auto mapFeature = instance->symap->getMapFeature();
        applyStateWithAppropriateLocking (PluginState { lilv_state_new_from_string (world->get(), &mapFeature, copy.data()) },
                                          concurrent);
    }

    // This does *not* destroy the editor component.
    // If we destroy the processor, the view must also be destroyed to avoid dangling pointers.
    // However, DRX clients expect their editors to remain valid for the duration of the
    // AudioProcessor's lifetime.
    // As a compromise, this will create a new LV2 view into an existing editor component.
    z0 destroyView()
    {
        optionalEditor.destroyView();
    }

    z0 activate()
    {
        if (! active)
            instance->instance.activate();

        active = true;
    }

    z0 deactivate()
    {
        if (active)
            instance->instance.deactivate();

        active = false;
    }

    z0 processBlockImpl (AudioBuffer<f32>& audio, MidiBuffer& midi)
    {
        preparePortsForRun (audio, midi);

        instance->instance.run (static_cast<u32> (audio.getNumSamples()));
        instance->features.processResponses();

        processPortsAfterRun (midi);
    }

    b8 portAtIndexSupportsMidi (u32 index) const noexcept
    {
        const auto port = plugin.getPortByIndex (index);

        if (! port.isValid())
            return false;

        return port.supportsEvent (world->newUri (LV2_MIDI__MidiEvent).get());
    }

    z0 controlGrabbed (u32 port, b8 grabbed) override
    {
        if (auto* param = parameterValues.getParamByPortIndex (port))
        {
            if (grabbed)
                param->beginChangeGesture();
            else
                param->endChangeGesture();
        }
    }

    z0 viewCreated (UiEventListener* newListener) override
    {
        uiEventListener = newListener;
        postAllParametersToUi();
    }

    ParameterWriterUrids getParameterWriterUrids() const
    {
        return { instance->urids.mLV2_PATCH__Set,
                 instance->urids.mLV2_PATCH__property,
                 instance->urids.mLV2_PATCH__value,
                 instance->urids.mLV2_ATOM__eventTransfer };
    }

    z0 postAllParametersToUi()
    {
        parameterValues.postAllParametersToUi (uiEventListener, getParameterWriterUrids(), *instance->processorToUi);
        controlPortStructure.writeOutputPorts (uiEventListener, *instance->processorToUi);
    }

    z0 postChangedParametersToUi()
    {
        parameterValues.postChangedParametersToUi (uiEventListener, getParameterWriterUrids(), *instance->processorToUi);
        controlPortStructure.writeOutputPorts (uiEventListener, *instance->processorToUi);
    }

    z0 notifyEditorBeingDeleted() override
    {
        optionalEditor.prepareToDestroyEditor();
        uiEventListener = nullptr;
        editorBeingDeleted (getActiveEditor());
    }

    InstanceWithSupports* getInstanceWithSupports() const override
    {
        return instance.get();
    }

    z0 applyStateWithAppropriateLocking (PluginState&& state, ConcurrentWithAudioCallback concurrent)
    {
        PortMap portStateManager (instance->ports);

        // If a plugin supports threadSafeRestore, its restore method is thread-safe
        // and may be called concurrently with audio class functions.
        if (hasThreadSafeRestore || concurrent == ConcurrentWithAudioCallback::no)
        {
            state.restore (*instance, portStateManager);
        }
        else
        {
            const ScopedLock lock (getCallbackLock());
            state.restore (*instance, portStateManager);
        }

        parameterValues.updateFromControlPorts (controlPortStructure);
        asyncFullUiParameterUpdate.triggerAsyncUpdate();
    }

    PluginState loadStateWithUri (const Txt& str)
    {
        auto mapFeature = instance->symap->getMapFeature();
        const auto presetUri = world->newUri (str.toRawUTF8());
        lilv_world_load_resource (world->get(), presetUri.get());
        return PluginState { lilv_state_new_from_world (world->get(), &mapFeature, presetUri.get()) };
    }

    z0 connectPorts (AudioBuffer<f32>& audio)
    {
        // Plugins that cannot process in-place will require the feature "inPlaceBroken".
        // We don't support that feature, so if we made it to this point we can assume that
        // in-place processing works.
        for (const auto& port : instance->ports.getAudioPorts())
        {
            const auto channel = ioMap.getChannelForPort (port.header.index);
            auto* ptr = isPositiveAndBelow (channel, audio.getNumChannels()) ? audio.getWritePointer (channel)
                                                                             : nullptr;
            instance->instance.connectPort (port.header.index, ptr);
        }

        for (const auto& port : instance->ports.getCvPorts())
            instance->instance.connectPort (port.header.index, nullptr);

        for (auto& port : instance->ports.getAtomPorts())
            instance->instance.connectPort (port.header.index, port.data());
    }

    z0 writeTimeInfoToPort (AtomPort& port)
    {
        if (port.header.direction != Port::Direction::input || ! port.getSupportsTime())
            return;

        auto* forge = port.getForge().get();
        auto* playhead = getPlayHead();

        if (playhead == nullptr)
            return;

        // Write timing info to the control port
        const auto info = playhead->getPosition();

        if (! info.hasValue())
            return;

        const auto& urids = instance->urids;

        lv2_atom_forge_frame_time (forge, 0);

        lv2_shared::ObjectFrame object { forge, (u32) 0, urids.mLV2_TIME__Position };

        lv2_atom_forge_key (forge, urids.mLV2_TIME__speed);
        lv2_atom_forge_float (forge, info->getIsPlaying() ? 1.0f : 0.0f);

        if (const auto samples = info->getTimeInSamples())
        {
            lv2_atom_forge_key (forge, urids.mLV2_TIME__frame);
            lv2_atom_forge_long (forge, *samples);
        }

        if (const auto bar = info->getBarCount())
        {
            lv2_atom_forge_key (forge, urids.mLV2_TIME__bar);
            lv2_atom_forge_long (forge, *bar);
        }

        if (const auto beat = info->getPpqPosition())
        {
            if (const auto barStart = info->getPpqPositionOfLastBarStart())
            {
                lv2_atom_forge_key (forge, urids.mLV2_TIME__barBeat);
                lv2_atom_forge_float (forge, (f32) (*beat - *barStart));
            }

            lv2_atom_forge_key (forge, urids.mLV2_TIME__beat);
            lv2_atom_forge_double (forge, *beat);
        }

        if (const auto sig = info->getTimeSignature())
        {
            lv2_atom_forge_key (forge, urids.mLV2_TIME__beatUnit);
            lv2_atom_forge_int (forge, sig->denominator);

            lv2_atom_forge_key (forge, urids.mLV2_TIME__beatsPerBar);
            lv2_atom_forge_float (forge, (f32) sig->numerator);
        }

        if (const auto bpm = info->getBpm())
        {
            lv2_atom_forge_key (forge, urids.mLV2_TIME__beatsPerMinute);
            lv2_atom_forge_float (forge, (f32) *bpm);
        }
    }

    z0 preparePortsForRun (AudioBuffer<f32>& audio, MidiBuffer& midiBuffer)
    {
        connectPorts (audio);

        for (auto& port : instance->ports.getAtomPorts())
        {
            switch (port.header.direction)
            {
                case Port::Direction::input:
                    port.beginSequence();
                    break;

                case Port::Direction::output:
                    port.replaceWithChunk();
                    break;

                case Port::Direction::unknown:
                    jassertfalse;
                    break;
            }
        }

        for (auto& port : instance->ports.getAtomPorts())
            writeTimeInfoToPort (port);

        const auto controlPortForge = controlPort != nullptr ? controlPort->getForge().get()
                                                             : nullptr;

        parameterValues.postChangedParametersToProcessor (getParameterWriterUrids(), controlPortForge);

        instance->uiToProcessor.readAllAndClear ([this] (MessageHeader header, u32 size, ukk buffer)
        {
            pushMessage (header, size, buffer);
        });

        for (auto& port : instance->ports.getAtomPorts())
        {
            if (port.header.direction == Port::Direction::input)
            {
                for (const auto meta : midiBuffer)
                {
                    port.addEventToSequence (meta.samplePosition,
                                             instance->urids.mLV2_MIDI__MidiEvent,
                                             static_cast<u32> (meta.numBytes),
                                             meta.data);
                }

                port.endSequence();
            }
        }

        if (freeWheelingPort != nullptr)
            freeWheelingPort->currentValue = isNonRealtime() ? freeWheelingPort->info.max
                                                             : freeWheelingPort->info.min;
    }

    z0 pushMessage (MessageHeader header, [[maybe_unused]] u32 size, ukk data)
    {
        if (header.protocol == 0 || header.protocol == instance->urids.mLV2_UI__floatProtocol)
        {
            const auto value = readUnaligned<f32> (data);

            if (auto* param = parameterValues.getParamByPortIndex (header.portIndex))
            {
                param->setDenormalisedValue (value);
            }
            else if (auto* port = controlPortStructure.getControlPortByIndex (header.portIndex))
            {
                // No parameter corresponds to this port, write to the port directly
                port->currentValue = value;
            }
        }
        else if (auto* atomPort = header.portIndex < atomPorts.size() ? atomPorts[header.portIndex] : nullptr)
        {
            if (header.protocol == instance->urids.mLV2_ATOM__eventTransfer)
            {
                if (const auto* atom = convertToAtomPtr (data, (size_t) size))
                {
                    atomPort->addAtomToSequence (0, atom);

                    // Not UB; LV2_Atom_Object has LV2_Atom as its first member
                    if (atom->type == instance->urids.mLV2_ATOM__Object)
                        patchSetHelper.processPatchSet (reinterpret_cast<const LV2_Atom_Object*> (data), PatchSetCallback { parameterValues });
                }
            }
            else if (header.protocol == instance->urids.mLV2_ATOM__atomTransfer)
            {
                if (const auto* atom = convertToAtomPtr (data, (size_t) size))
                    atomPort->replaceBufferWithAtom (atom);
            }
        }
    }

    z0 processPortsAfterRun (MidiBuffer& midi)
    {
        midi.clear();

        for (auto& port : instance->ports.getAtomPorts())
            processAtomPort (port, midi);

        if (latencyPort != nullptr)
            setLatencySamples ((i32) latencyPort->currentValue);
    }

    z0 processAtomPort (const AtomPort& port, MidiBuffer& midi)
    {
        if (port.header.direction != Port::Direction::output)
            return;

        // The port holds an Atom, by definition
        const auto* atom = reinterpret_cast<const LV2_Atom*> (port.data());

        if (atom->type != instance->urids.mLV2_ATOM__Sequence)
            return;

        // The Atom said that it was of sequence type, so this isn't UB
        const auto* sequence = reinterpret_cast<const LV2_Atom_Sequence*> (port.data());

        // http://lv2plug.in/ns/ext/atom#Sequence - run() stamps are always audio frames
        jassert (sequence->body.unit == 0 || sequence->body.unit == instance->urids.mLV2_UNITS__frame);

        for (const auto* event : lv2_shared::SequenceIterator { lv2_shared::SequenceWithSize { sequence } })
        {
            // At the moment, we forward all outgoing events to the UI.
            instance->processorToUi->pushMessage ({ uiEventListener, { port.header.index, instance->urids.mLV2_ATOM__eventTransfer } },
                                                  (u32) (event->body.size + sizeof (LV2_Atom)),
                                                  &event->body);

            if (event->body.type == instance->urids.mLV2_MIDI__MidiEvent)
                midi.addEvent (event + 1, static_cast<i32> (event->body.size), static_cast<i32> (event->time.frames));

            if (lv2_atom_forge_is_object_type (port.getForge().get(), event->body.type))
                if (reinterpret_cast<const LV2_Atom_Object_Body*> (event + 1)->otype == instance->urids.mLV2_STATE__StateChanged)
                    updateHostDisplay (ChangeDetails{}.withNonParameterStateChanged (true));

            patchSetHelper.processPatchSet (event, PatchSetCallback { parameterValues });
        }
    }

    // Check for duplicate channel designations, and convert the set to a discrete channel layout
    // if any designations are duplicated.
    static std::set<lv2_shared::SinglePortInfo> validateAndRedesignatePorts (std::set<lv2_shared::SinglePortInfo> info)
    {
        const auto channelSet = lv2_shared::ParsedGroup::getEquivalentSet (info);

        if ((i32) info.size() == channelSet.size())
            return info;

        std::set<lv2_shared::SinglePortInfo> result;
        auto designation = (i32) AudioChannelSet::discreteChannel0;

        for (auto& item : info)
        {
            auto copy = item;
            copy.designation = (AudioChannelSet::ChannelType) designation++;
            result.insert (copy);
        }

        return result;
    }

    static AudioChannelSet::ChannelType getPortDesignation (World& world, const Port& port, size_t indexInGroup)
    {
        const auto defaultResult = (AudioChannelSet::ChannelType) (AudioChannelSet::discreteChannel0 + indexInGroup);
        const auto node = port.get (world.newUri (LV2_CORE__designation).get());

        if (node == nullptr)
            return defaultResult;

        const auto it = lv2_shared::channelDesignationMap.find (lilvNodeToUriString (node.get()));

        if (it == lv2_shared::channelDesignationMap.end())
            return defaultResult;

        return it->second;
    }

    static lv2_shared::ParsedBuses getParsedBuses (World& world, const Plugin& p, const UsefulUris& uris)
    {
        const auto groupPropertyUri = world.newUri (LV2_PORT_GROUPS__group);
        const auto optionalUri = world.newUri (LV2_CORE__connectionOptional);

        std::map<Txt, std::set<lv2_shared::SinglePortInfo>> inputGroups, outputGroups;
        std::set<lv2_shared::SinglePortInfo> ungroupedInputs, ungroupedOutputs;

        for (u32 i = 0, numPorts = p.getNumPorts(); i < numPorts; ++i)
        {
            const auto port = p.getPortByIndex (i);

            if (port.getKind (uris) != Port::Kind::audio)
                continue;

            const auto groupUri = lilvNodeToUriString (port.get (groupPropertyUri.get()).get());

            auto& set = [&]() -> auto&
            {
                if (groupUri.isEmpty())
                    return port.getDirection (uris) == Port::Direction::input ? ungroupedInputs : ungroupedOutputs;

                auto& group = port.getDirection (uris) == Port::Direction::input ? inputGroups : outputGroups;
                return group[groupUri];
            }();

            set.insert ({ port.getIndex(), getPortDesignation (world, port, set.size()), port.hasProperty (optionalUri) });
        }

        for (auto* groups : { &inputGroups, &outputGroups })
            for (auto& pair : *groups)
                pair.second = validateAndRedesignatePorts (std::move (pair.second));

        DRX_BEGIN_IGNORE_WARNINGS_MSVC (4702)
        const auto getMainGroupName = [&] (tukk propertyName)
        {
            for (const auto* item : p.getValue (world.newUri (propertyName).get()))
                return lilvNodeToUriString (item);

            return Txt{};
        };
        DRX_END_IGNORE_WARNINGS_MSVC

        return { findStableBusOrder (getMainGroupName (LV2_PORT_GROUPS__mainInput),  inputGroups,  ungroupedInputs),
                 findStableBusOrder (getMainGroupName (LV2_PORT_GROUPS__mainOutput), outputGroups, ungroupedOutputs) };
    }

    static Txt getNameForUri (World& world, StringRef uri)
    {
        if (uri.isEmpty())
            return Txt();

        const auto node = world.get (world.newUri (uri).get(),
                                     world.newUri (LV2_CORE__name).get(),
                                     nullptr);

        if (node == nullptr)
            return Txt();

        return Txt::fromUTF8 (lilv_node_as_string (node.get()));
    }

    static BusesProperties getBusesProperties (const lv2_shared::ParsedBuses& parsedBuses, World& world)
    {
        BusesProperties result;

        for (const auto& pair : { std::make_tuple (&parsedBuses.inputs,  &result.inputLayouts),
                                  std::make_tuple (&parsedBuses.outputs, &result.outputLayouts) })
        {
            const auto& buses = *std::get<0> (pair);
            auto& layout = *std::get<1> (pair);

            for (const auto& bus : buses)
            {
                layout.add (AudioProcessor::BusProperties { getNameForUri (world, bus.uid),
                                                            bus.getEquivalentSet(),
                                                            bus.isRequired() });
            }
        }

        return result;
    }

    LV2_URID map (tukk str) const
    {
        return instance != nullptr ? instance->symap->map (str)
                                   : LV2_URID();
    }

    ControlPort* findControlPortWithIndex (u32 index) const
    {
        auto ports = instance->ports.getControlPorts();
        const auto indexMatches = [&] (const ControlPort& p) { return p.header.index == index; };
        const auto it = std::find_if (ports.begin(), ports.end(), indexMatches);

        return it != ports.end() ? &(*it) : nullptr;
    }

    const lv2_shared::ParsedBuses declaredBusLayout;
    lv2_shared::PortToAudioBufferMap ioMap { getBusesLayout(), declaredBusLayout };
    std::shared_ptr<World> world;
    Plugin plugin;
    PluginDescription description;
    std::vector<Txt> presetUris;
    std::unique_ptr<InstanceWithSupports> instance;
    AsyncFn asyncFullUiParameterUpdate { [this] { postAllParametersToUi(); } };

    std::vector<AtomPort*> atomPorts = getPortPointers (instance->ports.getAtomPorts());

    AtomPort* const controlPort = [&]() -> AtomPort*
    {
        const auto port = plugin.getPortByDesignation (world->newUri (LV2_CORE__InputPort).get(),
                                                       world->newUri (LV2_CORE__control).get());

        if (! port.isValid())
            return nullptr;

        const auto index = port.getIndex();

        if (! isPositiveAndBelow (index, atomPorts.size()))
            return nullptr;

        return atomPorts[index];
    }();

    ControlPort* const latencyPort = [&]() -> ControlPort*
    {
        if (! plugin.hasLatency())
            return nullptr;

        return findControlPortWithIndex (plugin.getLatencyPortIndex());
    }();

    ControlPort* const freeWheelingPort = [&]() -> ControlPort*
    {
        const auto port = plugin.getPortByDesignation (world->newUri (LV2_CORE__InputPort).get(),
                                                       world->newUri (LV2_CORE__freeWheeling).get());

        if (! port.isValid())
            return nullptr;

        return findControlPortWithIndex (port.getIndex());
    }();

    ControlPort* const enabledPort = [&]() -> ControlPort*
    {
        const auto port = plugin.getPortByDesignation (world->newUri (LV2_CORE__InputPort).get(),
                                                       world->newUri (LV2_CORE_PREFIX "enabled").get());

        if (! port.isValid())
            return nullptr;

        return findControlPortWithIndex (port.getIndex());
    }();

    lv2_shared::PatchSetHelper patchSetHelper { instance->symap->getMapFeature(), plugin.getUri().getTyped() };
    ControlPortAccelerationStructure controlPortStructure { instance->ports.getControlPorts() };
    ParameterValueCache parameterValues { *this,
                                          *world,
                                          instance->symap->getMapFeature(),
                                          getDrxParameterInfo (*world,
                                                                plugin,
                                                                instance->urids,
                                                                { latencyPort, freeWheelingPort },
                                                                instance->ports.getControlPorts(),
                                                                controlPort != nullptr ? controlPort->header.index : 0),
                                          enabledPort };
    LV2Parameter* bypassParam = enabledPort != nullptr ? parameterValues.getParamByPortIndex (enabledPort->header.index)
                                                       : nullptr;

    std::atomic<UiEventListener*> uiEventListener { nullptr };
    OptionalEditor<> optionalEditor;
    i32 lastAppliedPreset = 0;
    b8 hasThreadSafeRestore = plugin.hasExtensionData (world->newUri (LV2_STATE__threadSafeRestore));
    b8 active { false };

    DRX_LEAK_DETECTOR (LV2AudioPluginInstance)
};

} // namespace lv2_host

//==============================================================================
class LV2PluginFormat::Pimpl
{
public:
    Pimpl()
    {
        loadAllPluginsFromPaths (getDefaultLocationsToSearch());

        const auto tempFile = lv2ResourceFolder.getFile();

        if (tempFile.createDirectory())
        {
            for (const auto& bundle : lv2::Bundle::getAllBundles())
            {
                const auto pathToBundle = tempFile.getChildFile (bundle.name + Txt (".lv2"));

                if (! pathToBundle.createDirectory())
                    continue;

                for (const auto& resource : bundle.contents)
                    pathToBundle.getChildFile (resource.name).replaceWithText (resource.contents);

                const auto pathString = File::addTrailingSeparator (pathToBundle.getFullPathName());
                world->loadBundle (world->newFileUri (nullptr, pathString.toRawUTF8()));
            }
        }
    }

    ~Pimpl()
    {
        lv2ResourceFolder.getFile().deleteRecursively();
    }

    z0 findAllTypesForFile (OwnedArray<PluginDescription>& result,
                              const Txt& identifier)
    {
        if (File::isAbsolutePath (identifier))
            world->loadBundle (world->newFileUri (nullptr, File::addTrailingSeparator (identifier).toRawUTF8()));

        for (const auto& plugin : { findPluginByUri (identifier), findPluginByFile (identifier) })
        {
            if (auto desc = getDescription (plugin); desc.fileOrIdentifier.isNotEmpty())
            {
                result.add (std::make_unique<PluginDescription> (desc));
                break;
            }
        }
    }

    b8 fileMightContainThisPluginType (const Txt& file) const
    {
        // If the string looks like a URI, then it could be a valid LV2 identifier
        const auto* data = file.toRawUTF8();
        const auto numBytes = file.getNumBytesAsUTF8();
        std::vector<u8> vec (numBytes + 1, 0);
        std::copy (data, data + numBytes, vec.begin());
        return serd_uri_string_has_scheme (vec.data()) || file.endsWith (".lv2");
    }

    Txt getNameOfPluginFromIdentifier (const Txt& identifier)
    {
        // We would have to actually load the bundle to get its name,
        // and the bundle may contain multiple plugins
        return identifier;
    }

    b8 pluginNeedsRescanning (const PluginDescription&)
    {
        return true;
    }

    b8 doesPluginStillExist (const PluginDescription& description)
    {
        return findPluginByUri (description.fileOrIdentifier) != nullptr;
    }

    StringArray searchPathsForPlugins (const FileSearchPath& paths, b8, b8)
    {
        loadAllPluginsFromPaths (paths);

        StringArray result;

        for (const auto* plugin : world->getAllPlugins())
            result.add (lv2_host::Plugin { plugin }.getUri().getTyped());

        return result;
    }

    FileSearchPath getDefaultLocationsToSearch()
    {
      #if DRX_MAC
        return { "~/Library/Audio/Plug-Ins/LV2;"
                 "~/.lv2;"
                 "/usr/local/lib/lv2;"
                 "/usr/lib/lv2;"
                 "/Library/Audio/Plug-Ins/LV2;" };
      #elif DRX_WINDOWS
        return { "%APPDATA%\\LV2;"
                 "%COMMONPROGRAMFILES%\\LV2" };
      #else
       #if DRX_64BIT
        if (File ("/usr/lib64/lv2").exists() || File ("/usr/local/lib64/lv2").exists())
            return { "~/.lv2;"
                     "/usr/lib64/lv2;"
                     "/usr/local/lib64/lv2" };
       #endif

        return { "~/.lv2;"
                 "/usr/lib/lv2;"
                 "/usr/local/lib/lv2" };
      #endif
    }

    const LilvUI* findEmbeddableUi (const lv2_host::Uis* pluginUis, std::true_type)
    {
        if (pluginUis == nullptr)
            return nullptr;

        const std::vector<const LilvUI*> allUis (pluginUis->begin(), pluginUis->end());

        if (allUis.empty())
            return nullptr;

        constexpr tukk rawUri =
               #if DRX_MAC
                LV2_UI__CocoaUI;
               #elif DRX_WINDOWS
                LV2_UI__WindowsUI;
               #elif DRX_LINUX || DRX_BSD
                LV2_UI__X11UI;
               #else
                nullptr;
               #endif

        jassert (rawUri != nullptr);
        const auto nativeUiUri = world->newUri (rawUri);

        struct UiWithSuitability
        {
            const LilvUI* ui;
            u32 suitability;

            b8 operator< (const UiWithSuitability& other) const noexcept
            {
                return suitability < other.suitability;
            }

            static u32 uiIsSupported (tukk hostUri, tukk pluginUri)
            {
                if (strcmp (hostUri, pluginUri) == 0)
                    return 1;

                return 0;
            }
        };

        std::vector<UiWithSuitability> uisWithSuitability;
        uisWithSuitability.reserve (allUis.size());

        std::transform (allUis.cbegin(), allUis.cend(), std::back_inserter (uisWithSuitability), [&] (const LilvUI* ui)
        {
            const LilvNode* type = nullptr;
            return UiWithSuitability { ui, lilv_ui_is_supported (ui, UiWithSuitability::uiIsSupported, nativeUiUri.get(), &type) };
        });

        std::sort (uisWithSuitability.begin(), uisWithSuitability.end());

        if (uisWithSuitability.back().suitability != 0)
            return uisWithSuitability.back().ui;

        return nullptr;
    }

    const LilvUI* findEmbeddableUi (const lv2_host::Uis*, std::false_type)
    {
        return nullptr;
    }

    const LilvUI* findEmbeddableUi (const lv2_host::Uis* pluginUis)
    {
        return findEmbeddableUi (pluginUis, std::integral_constant<b8, lv2_host::editorFunctionalityEnabled>{});
    }

    static lv2_host::UiDescriptor getUiDescriptor (const LilvUI* ui)
    {
        if (ui == nullptr)
            return {};

        const auto libraryFile = StringPtr { lilv_file_uri_parse (lilv_node_as_uri (lilv_ui_get_binary_uri (ui)), nullptr) };

        return lv2_host::UiDescriptor { lv2_host::UiDescriptorArgs{}.withLibraryPath (libraryFile.get())
                                                                    .withUiUri (lilv_node_as_uri (lilv_ui_get_uri (ui))) };
    }

    // Returns the name of a missing feature, if any.
    template <typename RequiredFeatures, typename AvailableFeatures>
    static std::vector<Txt> findMissingFeatures (RequiredFeatures&& required,
                                                    AvailableFeatures&& available)
    {
        std::vector<Txt> result;

        for (const auto* node : required)
        {
            const auto nodeString = Txt::fromUTF8 (lilv_node_as_uri (node));

            if (std::find (std::begin (available), std::end (available), nodeString) == std::end (available))
                result.push_back (nodeString);
        }

        return result;
    }

    z0 createPluginInstance (const PluginDescription& desc,
                               f64 initialSampleRate,
                               i32 initialBufferSize,
                               PluginCreationCallback callback)
    {
        const auto* pluginPtr = findPluginByUri (desc.fileOrIdentifier);

        if (pluginPtr == nullptr)
            return callback (nullptr, "Unable to locate plugin with the requested URI");

        const lv2_host::Plugin plugin { pluginPtr };

        auto symap = std::make_unique<lv2_host::SymbolMap>();

        const auto missingFeatures = findMissingFeatures (plugin.getRequiredFeatures(),
                                                          lv2_host::FeaturesData::getFeatureUris());

        if (! missingFeatures.empty())
        {
            const auto missingFeaturesString = StringArray (missingFeatures.data(), (i32) missingFeatures.size()).joinIntoString (", ");

            return callback (nullptr, "plugin requires missing features: " + missingFeaturesString);
        }

        auto stateToApply = [&]
        {
            if (! plugin.hasFeature (world->newUri (LV2_STATE__loadDefaultState)))
                return lv2_host::PluginState{};

            auto map = symap->getMapFeature();
            return lv2_host::PluginState { lilv_state_new_from_world (world->get(), &map, plugin.getUri().get()) };
        }();

        auto ports = lv2_host::Ports::getPorts (*world, uris, plugin, *symap);

        if (! ports.hasValue())
            return callback (nullptr, "Plugin has ports of an unsupported type");

        auto instance = std::make_unique<lv2_host::InstanceWithSupports> (*world,
                                                                          std::move (symap),
                                                                          plugin,
                                                                          std::move (*ports),
                                                                          (i32) initialBufferSize,
                                                                          initialSampleRate);

        if (instance->instance == nullptr)
            return callback (nullptr, "Plugin was located, but could not be opened");

        auto potentialPresets = world->findNodes (nullptr,
                                                  world->newUri (LV2_CORE__appliesTo).get(),
                                                  plugin.getUri().get());

        const lv2_host::Uis pluginUis { plugin.get() };

        const auto uiToUse = [&]() -> const LilvUI*
        {
            const auto bestMatch = findEmbeddableUi (&pluginUis);

            if (bestMatch == nullptr)
                return bestMatch;

            const auto uiUri = lilv_ui_get_uri (bestMatch);
            lilv_world_load_resource (world->get(), uiUri);

            const auto queryUi = [&] (tukk featureUri)
            {
                const auto featureUriNode = world->newUri (featureUri);
                return world->findNodes (uiUri, featureUriNode.get(), nullptr);
            };

            const auto missingUiFeatures = findMissingFeatures (queryUi (LV2_CORE__requiredFeature),
                                                                lv2_host::UiFeaturesData::getFeatureUris());

            return missingUiFeatures.empty() ? bestMatch : nullptr;
        }();

        auto uiBundleUri = uiToUse != nullptr ? Txt::fromUTF8 (lilv_node_as_uri (lilv_ui_get_bundle_uri (uiToUse)))
                                              : Txt();

        auto wrapped = std::make_unique<lv2_host::LV2AudioPluginInstance> (world,
                                                                           plugin,
                                                                           uris,
                                                                           std::move (instance),
                                                                           getDescription (pluginPtr),
                                                                           findPresetUrisForPlugin (plugin.get()),
                                                                           std::move (stateToApply),
                                                                           std::move (uiBundleUri),
                                                                           getUiDescriptor (uiToUse));
        callback (std::move (wrapped), {});
    }

private:
    z0 loadAllPluginsFromPaths (const FileSearchPath& path)
    {
        const auto joined = path.toStringWithSeparator (LILV_PATH_SEP);
        world->loadAllFromPaths (world->newString (joined.toRawUTF8()));
    }

    struct Free { z0 operator() (tuk ptr) const noexcept { free (ptr); } };
    using StringPtr = std::unique_ptr<t8, Free>;

    const LilvPlugin* findPluginByUri (const Txt& s)
    {
        return world->getAllPlugins().getByUri (world->newUri (s.toRawUTF8()));
    }

    const LilvPlugin* findPluginByFile (const File& f)
    {
        return world->getAllPlugins().getByFile (f);
    }

    template <typename Fn>
    z0 visitParentClasses (const LilvPluginClass* c, Fn&& fn) const
    {
        if (c == nullptr)
            return;

        const lv2_host::PluginClass wrapped { c };
        fn (wrapped);

        const auto parentUri = wrapped.getParentUri();

        if (parentUri.get() != nullptr)
            visitParentClasses (world->getPluginClasses().getByUri (parentUri), fn);
    }

    std::vector<lv2_host::NodeUri> collectPluginClassUris (const LilvPluginClass* c) const
    {
        std::vector<lv2_host::NodeUri> results;

        visitParentClasses (c, [&results] (const lv2_host::PluginClass& wrapped)
        {
            results.emplace_back (wrapped.getUri());
        });

        return results;
    }

    PluginDescription getDescription (const LilvPlugin* plugin)
    {
        if (plugin == nullptr)
            return {};

        const auto wrapped      = lv2_host::Plugin { plugin };
        const auto bundle       = wrapped.getBundleUri().getTyped();
        const auto bundleFile   = File { StringPtr { lilv_file_uri_parse (bundle, nullptr) }.get() };

        const auto numInputs  = wrapped.getNumPortsOfClass (uris.mLV2_CORE__AudioPort, uris.mLV2_CORE__InputPort);
        const auto numOutputs = wrapped.getNumPortsOfClass (uris.mLV2_CORE__AudioPort, uris.mLV2_CORE__OutputPort);

        PluginDescription result;
        result.name                 = wrapped.getName().getTyped();
        result.descriptiveName      = wrapped.getName().getTyped();
        result.lastFileModTime      = bundleFile.getLastModificationTime();
        result.lastInfoUpdateTime   = Time::getCurrentTime();
        result.manufacturerName     = wrapped.getAuthorName().getTyped();
        result.pluginFormatName     = LV2PluginFormat::getFormatName();
        result.numInputChannels     = static_cast<i32> (numInputs);
        result.numOutputChannels    = static_cast<i32> (numOutputs);

        const auto classPtr     = wrapped.getClass();
        const auto classes      = collectPluginClassUris (classPtr);
        const auto isInstrument = std::any_of (classes.cbegin(),
                                               classes.cend(),
                                               [this] (const lv2_host::NodeUri& uri)
                                               {
                                                   return uri.equals (uris.mLV2_CORE__GeneratorPlugin);
                                               });

        result.category         = lv2_host::PluginClass { classPtr }.getLabel().getTyped();
        result.isInstrument     = isInstrument;

        // The plugin URI is required to be globally unique, so a hash of it should be too
        result.fileOrIdentifier = wrapped.getUri().getTyped();

        const auto uid = DefaultHashFunctions::generateHash (result.fileOrIdentifier, std::numeric_limits<i32>::max());;
        result.deprecatedUid = result.uniqueId = uid;
        return result;
    }

    std::vector<Txt> findPresetUrisForPlugin (const LilvPlugin* plugin)
    {
        std::vector<Txt> presetUris;

        lv2_host::OwningNodes potentialPresets { lilv_plugin_get_related (plugin, world->newUri (LV2_PRESETS__Preset).get()) };

        for (const auto* potentialPreset : potentialPresets)
            presetUris.push_back (lilv_node_as_string (potentialPreset));

        return presetUris;
    }

    TemporaryFile lv2ResourceFolder;
    std::shared_ptr<lv2_host::World> world = std::make_shared<lv2_host::World>();
    lv2_host::UsefulUris uris { world->get() };
};

//==============================================================================
LV2PluginFormat::LV2PluginFormat()
    : pimpl (std::make_unique<Pimpl>()) {}

LV2PluginFormat::~LV2PluginFormat() = default;

z0 LV2PluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                           const Txt& fileOrIdentifier)
{
    pimpl->findAllTypesForFile (results, fileOrIdentifier);
}

b8 LV2PluginFormat::fileMightContainThisPluginType (const Txt& fileOrIdentifier)
{
    return pimpl->fileMightContainThisPluginType (fileOrIdentifier);
}

Txt LV2PluginFormat::getNameOfPluginFromIdentifier (const Txt& fileOrIdentifier)
{
    return pimpl->getNameOfPluginFromIdentifier (fileOrIdentifier);
}

b8 LV2PluginFormat::pluginNeedsRescanning (const PluginDescription& desc)
{
    return pimpl->pluginNeedsRescanning (desc);
}

b8 LV2PluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    return pimpl->doesPluginStillExist (desc);
}

b8 LV2PluginFormat::canScanForPlugins() const { return true; }
b8 LV2PluginFormat::isTrivialToScan() const { return true; }

StringArray LV2PluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                                    b8 recursive,
                                                    b8 allowAsync)
{
    return pimpl->searchPathsForPlugins (directoriesToSearch, recursive, allowAsync);
}

FileSearchPath LV2PluginFormat::getDefaultLocationsToSearch()
{
    return pimpl->getDefaultLocationsToSearch();
}

b8 LV2PluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

z0 LV2PluginFormat::createPluginInstance (const PluginDescription& desc,
                                            f64 sampleRate,
                                            i32 bufferSize,
                                            PluginCreationCallback callback)
{
    pimpl->createPluginInstance (desc, sampleRate, bufferSize, std::move (callback));
}

} // namespace drx

#endif
