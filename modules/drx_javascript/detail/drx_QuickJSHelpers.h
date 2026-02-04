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

namespace drx::detail
{

//==============================================================================
// On Linux z64 and z64 don't resolve to the same type and don't have C style casts between
// each other, hence we need a two-step conversion.
template <typename T>
static z64 fromDrxInt64 (const T& convertible) { return (z64) (z64) convertible; }

template <typename T>
static z64 toDrxInt64   (const T& convertible) { return (z64) (z64) convertible; }

//==============================================================================
namespace qjs = choc::javascript::quickjs;

using VarOrError = std::variant<var, Txt>;

static var discardError (VarOrError variant)
{
    const auto* v = std::get_if<var> (&variant);
    return v != nullptr ? *v : var::undefined();
}

static VarOrError quickJSToDrx (const qjs::QuickJSContext::ValuePtr& ptr);

static std::vector<var> quickJSToDrx (Span<qjs::JSValueConst> args, qjs::JSContext* ctx)
{
    std::vector<var> argList;
    argList.reserve (args.size());

    for (const auto& arg : args)
        argList.push_back (discardError (quickJSToDrx ({ qjs::JS_DupValue (ctx, arg), ctx })));

    return argList;
}

static qjs::JSValue juceToQuickJs (const var& v, qjs::JSContext* ctx)
{
    using namespace qjs;

    if (v.isVoid())
        return JS_NULL;

    if (v.isUndefined())
        return JS_UNDEFINED;

    if (v.isInt())
        return JS_NewInt32   (ctx, static_cast<i32> (v));

    if (v.isInt64())
        return JS_NewInt64   (ctx, static_cast<z64> (v));

    if (v.isDouble())
        return JS_NewFloat64 (ctx, static_cast<f64>  (v));

    if (v.isBool())
        return JS_NewBool    (ctx, static_cast<b8>    (v));

    if (v.isString())
    {
        const Txt x = v;
        return JS_NewStringLen (ctx, x.toRawUTF8(), x.getNumBytesAsUTF8());
    }

    if (auto fn = v.getNativeFunction())
    {
        using Fn = var::NativeFunction;
        static constexpr auto size = sizeof (fn);

        const auto cb = [] (JSContext* localContext,
                            JSValueConst thisVal,
                            i32 argc,
                            JSValueConst* argv,
                            i32,
                            JSValue* funcData) -> JSValue
        {
            if (funcData == nullptr)
            {
                jassertfalse;
                return {};
            }

            size_t bufferSize{};
            uk buffer = qjs::JS_GetArrayBuffer (localContext, &bufferSize, *funcData);

            if (buffer == nullptr || bufferSize != size)
            {
                jassertfalse;
                return {};
            }

            const auto thisConverted = discardError (quickJSToDrx ({ qjs::JS_DupValue (localContext, thisVal), localContext }));
            const auto argsConverted = quickJSToDrx ({ argv, (size_t) argc }, localContext);
            const var::NativeFunctionArgs args { thisConverted, argsConverted.data(), (i32) argsConverted.size() };

            const auto resultVar = (*static_cast<Fn*> (buffer)) (args);

            return juceToQuickJs (resultVar, localContext);
        };

        const auto free = [] (JSRuntime*, uk, uk buffer)
        {
            auto* localFn = static_cast<Fn*> (buffer);
            localFn->~Fn();
            delete[] static_cast<u8*> (buffer);
        };

        std::unique_ptr<u8[]> storage { new u8[size] };
        new (storage.get()) Fn { std::move (fn) };

        qjs::QuickJSContext::ValuePtr callbackAsData { qjs::JS_NewArrayBuffer (ctx,
                                                                               storage.release(),
                                                                               size,
                                                                               free,
                                                                               nullptr,
                                                                               false),
                                                       ctx };
        return JS_NewCFunctionData (ctx, cb, 0, 0, 1, &callbackAsData.value);
    }

    if (auto* array = v.getArray())
    {
        auto result = JS_NewArray (ctx);

        for (const auto [index, value] : enumerate (*array, u32{}))
            JS_SetPropertyUint32 (ctx, result, index, juceToQuickJs (value, ctx));

        return result;
    }

    if (auto* obj = v.getDynamicObject())
    {
        auto result = JS_NewObject (ctx);

        for (const auto& pair : obj->getProperties())
        {
            const auto name = pair.name.toString();
            JS_SetPropertyStr (ctx, result, name.toRawUTF8(), juceToQuickJs (pair.value, ctx));
        }

        return result;
    }

    jassertfalse;
    return JS_UNDEFINED;
}

//==============================================================================
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsubobject-linkage")
struct JSFunctionArguments
{
    explicit JSFunctionArguments (qjs::JSContext* contextIn)
        : context (contextIn)
    {
    }

    JSFunctionArguments (qjs::JSContext* contextIn, const var::NativeFunctionArgs& args)
        : JSFunctionArguments (contextIn, Span { args.arguments, (size_t) args.numArguments })
    {
    }

    JSFunctionArguments (qjs::JSContext* contextIn, Span<const var> args)
        : context (contextIn)
    {
        values.reserve (args.size());

        for (const auto& arg : args)
            values.push_back (juceToQuickJs (arg, context));
    }

    ~JSFunctionArguments()
    {
        for (const auto& value : values)
            qjs::JS_FreeValue (context, value);
    }

    z0 add (const var& arg)
    {
        values.push_back (juceToQuickJs (arg, context));
    }

    qjs::JSValue* getArguments()
    {
        return values.data();
    }

    i32 getSize() const
    {
        return (i32) values.size();
    }

private:
    qjs::JSContext* context;
    std::vector<qjs::JSValue> values;

    DRX_DECLARE_NON_COPYABLE (JSFunctionArguments)
    DRX_DECLARE_NON_MOVEABLE (JSFunctionArguments)
};
DRX_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
// Throws on failure
static var tryQuickJSToDrx (const qjs::QuickJSContext::ValuePtr& ptr,
                             const qjs::JSValue* parent = nullptr)
{
    using namespace qjs;

    jassert (ptr.context != nullptr);

    if (JS_IsUndefined (ptr.value))
        return var::undefined();

    if (JS_IsNull (ptr.value))
        return var{};

    if (JS_IsNumber (ptr.value))
    {
        f64 d = 0;
        JS_ToFloat64 (ptr.context, std::addressof (d), ptr.value);
        return d;
    }

    if (JS_IsBool (ptr.value))
        return JS_ToBool (ptr.context, ptr.value) != 0;

    if (JS_IsString (ptr.value))
    {
        size_t len = 0;
        const auto* s = JS_ToCStringLen2 (ptr.context, std::addressof (len), ptr.value, false);
        const ScopeGuard scope { [&] { JS_FreeCString (ptr.context, s); } };
        return Txt::fromUTF8 (s, (i32) len);
    }

    if (JS_IsArray (ptr.context, ptr.value))
    {
        const auto lengthProp = ptr["length"];
        u32 len = 0;
        JS_ToUint32 (ptr.context, &len, lengthProp.get());

        Array<var> result;
        result.ensureStorageAllocated ((i32) len);

        for (auto i = decltype (len){}; i < len; ++i)
            result.add (tryQuickJSToDrx (ptr[i], &ptr.value));

        return result;
    }

    if (JS_IsFunction (ptr.context, ptr.value))
    {
        // ValuePtr is move-only, so can't be captured into a std::function.
        // Use a custom copyable callable instead.
        struct Callable
        {
            Callable (JSContext* ctxIn, JSValue fnIn, JSValue selfIn)
                : ctx (ctxIn),
                  fn (JS_DupValue (ctx, fnIn)),
                  self (JS_DupValue (ctx, selfIn))
            {
            }

            Callable (const Callable& other)
                : ctx (other.ctx),
                  fn (JS_DupValue (ctx, other.fn)),
                  self (JS_DupValue (ctx, other.self))
            {
            }

            Callable& operator= (const Callable& other)
            {
                Callable { other }.swap (*this);
                return *this;
            }

            ~Callable()
            {
                JS_FreeValue (ctx, fn);
                JS_FreeValue (ctx, self);
            }

            z0 swap (Callable& other) noexcept
            {
                std::swap (other.ctx, ctx);
                std::swap (other.fn, fn);
                std::swap (other.self, self);
            }

            var operator() (const var::NativeFunctionArgs& args) const
            {
                JSFunctionArguments convertedArgs { ctx, args };

                const qjs::QuickJSContext::ValuePtr result { qjs::JS_Call (ctx,
                                                                           fn,
                                                                           self,
                                                                           (i32) convertedArgs.getSize(),
                                                                           convertedArgs.getArguments()),
                                                             ctx };

                return discardError (quickJSToDrx (result));
            }

            JSContext* ctx{};
            JSValue fn, self;
        };

        const qjs::QuickJSContext::ValuePtr parentToUse { parent != nullptr ? JS_DupValue (ptr.context, *parent)
                                                                            : JS_GetGlobalObject (ptr.context),
                                                          ptr.context };

        return var::NativeFunction { Callable { ptr.context, ptr.value, parentToUse.value } };
    }

    if (JS_IsObject (ptr.value))
    {
        std::vector<std::string> propNames;

        for (auto obj = ptr.takeValue (JS_DupValue (ptr.context, ptr.value));;)
        {
            JSPropertyEnum* properties = nullptr;
            u32 numProps = 0;

            if (JS_GetOwnPropertyNames (ptr.context, &properties, &numProps, obj.get(), JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) != 0
                || properties == nullptr)
            {
                return new DynamicObject;
            }

            const ScopeGuard scope { [&] { js_free (ptr.context, properties); } };

            propNames.reserve (numProps);

            for (u32 i = 0; i < numProps; ++i)
            {
                const auto* name = JS_AtomToCString (ptr.context, properties[i].atom);
                std::string nameString (name);

                if (nameString != QuickJSContext::objectNameAttribute)
                    propNames.push_back (std::move (nameString));

                JS_FreeCString (ptr.context, name);
                JS_FreeAtom (ptr.context, properties[i].atom);
            }

            auto proto = ptr.takeValue (JS_GetPrototype (ptr.context, obj.get()));

            if (! JS_IsObject (proto.get()))
                break;

            obj = std::move (proto);
        }

        DynamicObject::Ptr result = new DynamicObject;

        for (auto& propName : propNames)
            result->setProperty (Txt (propName), tryQuickJSToDrx (ptr[propName.c_str()], &ptr.value));

        return result.get();
    }

    ptr.throwIfError();
    return {};
}

static VarOrError quickJSToDrx (const qjs::QuickJSContext::ValuePtr& ptr)
{
    try
    {
        return tryQuickJSToDrx (ptr);
    }
    catch (const choc::javascript::Error& error)
    {
        return Txt (error.what());
    }
}

//==============================================================================
// Any type that references the QuickJS types inside the anonymous namespace added by us requires
// this with GCC. Suppressing this warning is fine, since these classes are only visible and used
// in a single translation unit.
DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsubobject-linkage")
class QuickJSWrapper
{
public:
    explicit QuickJSWrapper (const RelativeTime* maximumExecutionTimeIn)
        : maximumExecutionTime { *maximumExecutionTimeIn }
    {
        qjs::JS_SetInterruptHandler (getQuickJSRuntime(), handleInterrupt, (uk) this);
    }

    qjs::JSContext* getQuickJSContext() const
    {
        return impl->context;
    }

    qjs::JSRuntime* getQuickJSRuntime() const
    {
        return impl->runtime;
    }

    z0 resetTimeout()
    {
        timeout = (z64) Time::getMillisecondCounterHiRes() + maximumExecutionTime.inMilliseconds();
    }

    z0 stop()
    {
        timeout = (z64) Time::getMillisecondCounterHiRes();
    }

private:
    static i32 handleInterrupt (qjs::JSRuntime*, uk opaque)
    {
        auto& self = *static_cast<QuickJSWrapper*> (opaque);
        return (z64) Time::getMillisecondCounterHiRes() >= self.timeout;
    }

    std::unique_ptr<qjs::QuickJSContext> impl = std::make_unique<qjs::QuickJSContext>();
    const RelativeTime& maximumExecutionTime;
    std::atomic<z64> timeout{};
};
DRX_END_IGNORE_WARNINGS_GCC_LIKE

using SetterFn = qjs::JSValue (*) (qjs::JSContext* ctx,
                                   qjs::JSValueConst thisVal,
                                   qjs::JSValueConst val,
                                   i32 magic);
using GetterFn = qjs::JSValue (*) (qjs::JSContext* ctx, qjs::JSValueConst thisVal, i32 magic);

// A replacement for the JS_CGETSET_MAGIC_DEF macro in QuickJS
static qjs::JSCFunctionListEntry makeFunctionListEntry (tukk name,
                                                        GetterFn getter,
                                                        SetterFn setter,
                                                        i16 magic)
{
    qjs::JSCFunctionListEntry e { name, JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE, qjs::JS_DEF_CGETSET_MAGIC, magic, {} };
    e.u.getset.get.getter_magic = getter;
    e.u.getset.set.setter_magic = setter;
    return e;
}

// A replacement for the JS_UNDEFINED macro in QuickJS
static qjs::JSValue makeUndefined()
{
   #if defined(JS_NAN_BOXING) // Differentiates between 32 and 64 bit builds
    return (((zu64)(qjs::JS_TAG_UNDEFINED) << 32) | (u32)(0));
   #else
    return qjs::JSValue (static_cast<i32> (0), qjs::JS_TAG_UNDEFINED);
   #endif
}

static qjs::JSClassID createClassId()
{
    // A passed in value of 0 asks QuickJS to allocate us a new unique ID. QuickJS uses global
    // variables for the bookkeeping, so it's safe to use this function to initialise globals.
    qjs::JSClassID newId = 0;
    return qjs::JS_NewClassID (&newId);
}

//==============================================================================
/*  Attached as an opaque pointer to the corresponding JS object. Its lifetime is managed by the
    QuickJS engine, which calls the finalise function when the corresponding JSValue is deleted.
*/
struct DynamicObjectWrapper
{
    DynamicObjectWrapper (detail::QuickJSWrapper& engineIn, DynamicObject::Ptr objectIn)
        : engine (engineIn), object (objectIn)
    {
        getDynamicObjects().insert (this);
    }

    i16 getOrdinal (const Identifier& identifier)
    {
        if (const auto& it = ordinals.find (identifier); it != ordinals.end())
            return it->second;

        identifiers.emplace_back (identifier);
        const auto newSize = identifiers.size() - 1;
        jassert (newSize <= (size_t) std::numeric_limits<i16>::max());
        const auto newOrdinal = (i16) newSize;
        ordinals[identifier] = newOrdinal;
        return newOrdinal;
    }

    auto getIdentifier (i32 ordinal) const
    {
        jassert ((size_t) ordinal < identifiers.size());
        return identifiers[(size_t) ordinal];
    }

    NamedValueSet& getProperties() const
    {
        return object->getProperties();
    }

    static z0 finaliser (qjs::JSRuntime*, qjs::JSValue val)
    {
        auto* wrapper = static_cast<DynamicObjectWrapper*> (qjs::JS_GetOpaque (val, getClassId()));
        wrapper->finalise();
    }

    z0 finalise()
    {
        getDynamicObjects().erase (this);
        delete this;
    }

    static z0 createClass (qjs::JSRuntime* runtime)
    {
        qjs::JSClassDef classDef {};
        classDef.class_name = "drx_DynamicObject";
        classDef.finalizer  = finaliser;
        qjs::JS_NewClass (runtime, getClassId(), &classDef);
    }

    //==============================================================================
    static qjs::JSValue callDispatcher (qjs::JSContext* ctx,
                                        qjs::JSValueConst thisValue,
                                        i32 numArgs,
                                        qjs::JSValueConst* args,
                                        i32 ordinal)
    {
        auto& self = *static_cast<DynamicObjectWrapper*> (qjs::JS_GetOpaque2 (ctx, thisValue, getClassId()));
        const auto argList = quickJSToDrx (Span { args, (size_t) numArgs }, ctx);
        const auto identifier = self.getIdentifier (ordinal);
        auto result = self.object->invokeMethod (identifier,
                                                 { self.object.get(), argList.data(), (i32) argList.size() });
        return juceToQuickJs (result, ctx);
    }

    static qjs::JSValue setDispatcher (qjs::JSContext* ctx,
                                       qjs::JSValueConst thisVal,
                                       qjs::JSValueConst val,
                                       i32 ordinal)
    {
        auto& self = *static_cast<DynamicObjectWrapper*> (qjs::JS_GetOpaque2 (ctx, thisVal, getClassId()));
        self.object->setProperty (self.getIdentifier (ordinal), discardError (quickJSToDrx ({ qjs::JS_DupValue (ctx, val), ctx })));

        // In case there is a problem we could return e.g. `JS_EXCEPTION` or
        // `JS_ThrowRangeError(ctx, "invalid precision");` here.
        return makeUndefined();
    }

    static qjs::JSValue getDispatcher (qjs::JSContext* ctx, qjs::JSValueConst thisVal, i32 ordinal)
    {
        auto& self = *static_cast<DynamicObjectWrapper*> (qjs::JS_GetOpaque2 (ctx, thisVal, getClassId()));
        return juceToQuickJs (self.object->getProperty (self.getIdentifier (ordinal)), ctx);
    }

    static qjs::JSClassID getClassId()
    {
        static qjs::JSClassID classId = createClassId();
        return classId;
    }

    static std::set<uk>& getDynamicObjects()
    {
        // Used to check if an opaque ptr attached to a JSValue is a DynamicObjectWrapper
        static std::set<uk> dynamicObjects;
        return dynamicObjects;
    }

    //==============================================================================
    QuickJSWrapper& engine;
    DynamicObject::Ptr object;
    std::map<Identifier, i16> ordinals;
    std::vector<Identifier> identifiers;
};

//==============================================================================
static b8 hasProperty (qjs::JSContext* ctx, qjs::JSValueConst object, tukk name)
{
    qjs::JSAtom atom = JS_NewAtom (ctx, name);
    ScopeGuard freeAtom { [&] { qjs::JS_FreeAtom (ctx, atom); } };

    return JS_HasProperty (ctx, object, atom) > 0;
}

static qjs::JSValue getOrCreateProperty (qjs::JSContext* ctx, qjs::JSValueConst object, tukk name)
{
    if (! hasProperty (ctx, object, name))
        qjs::JS_SetPropertyStr (ctx, object, name, JS_NewObject (ctx));

    return qjs::JS_GetPropertyStr (ctx, object, name);
}

static u32 toUint32 (z64 value)
{
    jassert (0 <= value && value <= (z64) std::numeric_limits<u32>::max());
    return (u32) value;
}

} // namespace drx::detail
