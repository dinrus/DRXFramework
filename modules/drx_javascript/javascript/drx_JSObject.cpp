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

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsubobject-linkage")
class JSObject::Impl
{
public:
    using ValuePtr = detail::qjs::QuickJSContext::ValuePtr;

    explicit Impl (detail::QuickJSWrapper* engineIn)
        : Impl (engineIn,
                { detail::qjs::JS_GetGlobalObject (engineIn->getQuickJSContext()), engineIn->getQuickJSContext() })
    {
    }

    Impl (const Impl& other)
        : Impl (other.engine,
                { detail::qjs::JS_DupValue (other.engine->getQuickJSContext(), other.valuePtr.get()),
                  other.engine->getQuickJSContext() })
    {
    }

    std::unique_ptr<Impl> getChild (const Identifier& prop) const
    {
        return rawToUniquePtr (new Impl (engine, { detail::getOrCreateProperty (engine->getQuickJSContext(),
                                                                                valuePtr.get(),
                                                                                prop.toString().toRawUTF8()),
                                                   engine->getQuickJSContext() }));
    }

    std::unique_ptr<Impl> getChild (z64 index) const
    {
        jassert (isArray());
        return rawToUniquePtr (new Impl (engine, valuePtr[detail::toUint32 (index)]));
    }

    b8 hasProperty (const Identifier& name) const
    {
        return detail::hasProperty (engine->getQuickJSContext(), valuePtr.get(), name.toString().toRawUTF8());
    }

    z0 setProperty (const Identifier& name, const var& value) const
    {
        auto* ctx = engine->getQuickJSContext();

        detail::qjs::JS_SetPropertyStr (ctx, valuePtr.get(), name.toString().toRawUTF8(), detail::juceToQuickJs (value, ctx));
    }

    z0 setProperty (z64 index, const var& value) const
    {
        auto* ctx = engine->getQuickJSContext();

        detail::qjs::JS_SetPropertyInt64 (ctx, valuePtr.get(), index, detail::juceToQuickJs (value, ctx));
    }

    var get() const
    {
        if (auto* opaque = detail::qjs::JS_GetOpaque (valuePtr.get(), detail::DynamicObjectWrapper::getClassId()))
            if (detail::DynamicObjectWrapper::getDynamicObjects().count (opaque) != 0)
                return { static_cast<detail::DynamicObjectWrapper*> (opaque)->object.get() };

        auto* ctx = engine->getQuickJSContext();
        return detail::discardError (detail::quickJSToDrx ({ detail::qjs::JS_DupValue (ctx, valuePtr.get()), ctx }));
    }

    detail::VarOrError invokeMethod (const Identifier& methodName, Span<const var> args) const
    {
        engine->resetTimeout();

        if (! hasProperty (methodName))
        {
            jassertfalse;
            return {};
        }

        auto* ctx = engine->getQuickJSContext();
        const auto methodAtom = JS_NewAtom (ctx, methodName.toString().toRawUTF8());
        ScopeGuard scope { [&] { detail::qjs::JS_FreeAtom (ctx, methodAtom); } };

        detail::JSFunctionArguments arguments { ctx, args };

        ValuePtr returnVal { detail::qjs::JS_Invoke (ctx,
                                                     valuePtr.get(),
                                                     methodAtom,
                                                     arguments.getSize(),
                                                     arguments.getArguments()),
                             ctx };

        return detail::quickJSToDrx (returnVal);
    }

    NamedValueSet getProperties() const
    {
        NamedValueSet result;

        auto* ctx = engine->getQuickJSContext();
        ValuePtr names { detail::qjs::JS_GetOwnPropertyNames2 (ctx,
                                                               valuePtr.get(),
                                                               detail::qjs::JS_GPN_ENUM_ONLY | detail::qjs::JS_GPN_STRING_MASK,
                                                               detail::qjs::JS_ITERATOR_KIND_KEY),
                         ctx };


        if (auto v = detail::discardError (detail::quickJSToDrx (names)); const auto* propertyNames = v.getArray())
        {
            for (const auto& name : *propertyNames)
            {
                if (name.isString())
                {
                    const Identifier prop { name.toString() };
                    result.set (prop, getChild (prop)->get());
                }
            }
        }

        return result;
    }

    b8 isArray() const
    {
        return detail::qjs::JS_IsArray (engine->getQuickJSContext(), valuePtr.get());
    }

    z64 getSize() const
    {
        if (! isArray())
        {
            jassertfalse;
            return 0;
        }

        auto lengthProp = valuePtr["length"];
        u32 length = 0;
        detail::qjs::JS_ToUint32 (engine->getQuickJSContext(), &length, lengthProp.get());
        return (z64) length;
    }

private:
    Impl (detail::QuickJSWrapper* e, ValuePtr&& ptr)
        : engine (e), valuePtr (std::move (ptr))
    {
    }

    detail::QuickJSWrapper* engine = nullptr;
    ValuePtr valuePtr;
};
DRX_END_IGNORE_WARNINGS_GCC_LIKE

JSObject::JSObject (detail::QuickJSWrapper* engine)
    : impl (new Impl (engine))
{
}

JSObject::JSObject (std::unique_ptr<Impl> implIn)
    : impl (std::move (implIn))
{
}

JSObject::JSObject (const JSObject& other)
    : impl (new Impl (*other.impl))
{
}

JSObject::~JSObject() = default;

JSObject::JSObject (JSObject&&) noexcept = default;

JSObject& JSObject::operator= (const JSObject& other)
{
    JSObject { other }.swap (*this);
    return *this;
}

JSObject& JSObject::operator= (JSObject&& other) noexcept = default;

JSObject JSObject::getChild (const Identifier& name) const
{
    return JSObject { impl->getChild (name) };
}

JSObject JSObject::operator[] (const Identifier& name) const
{
    return getChild (name);
}

b8 JSObject::isArray() const
{
    return impl->isArray();
}

z64 JSObject::getSize() const
{
    return impl->getSize();
}

JSObject JSObject::getChild (z64 index) const
{
    jassert (isArray());
    return JSObject { impl->getChild (index) };
}

JSObject JSObject::operator[] (z64 index) const
{
    return getChild (index);
}

b8 JSObject::hasProperty (const Identifier& name) const
{
    return impl->hasProperty (name);
}

var JSObject::get() const
{
    return impl->get();
}

z0 JSObject::setProperty (const Identifier& name, const var& value) const
{
    impl->setProperty (name, value);
}

z0 JSObject::setProperty (z64 index, const var& value) const
{
    impl->setProperty (index, value);
}

var JSObject::invokeMethod (const Identifier& methodName,
                            Span<const var> args,
                            Result* result) const
{
    const auto varOrError = impl->invokeMethod (methodName, args);

    if (result != nullptr)
    {
        const auto* e = std::get_if<Txt> (&varOrError);
        *result = e != nullptr ? Result::fail (*e) : Result::ok();
    }

    return detail::discardError (varOrError);
}

NamedValueSet JSObject::getProperties() const
{
    return impl->getProperties();
}

z0 JSObject::swap (JSObject& other) noexcept
{
    std::swap (impl, other.impl);
}

} // namespace drx
