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

//==============================================================================
class JavascriptEngine::Impl
{
public:
    using ValuePtr = detail::qjs::QuickJSContext::ValuePtr;

    //==============================================================================
    explicit Impl (const RelativeTime* maximumExecutionTimeIn)
        : engine (std::make_unique<detail::QuickJSWrapper> (maximumExecutionTimeIn))
    {
        detail::DynamicObjectWrapper::createClass (engine->getQuickJSRuntime());
    }

    z0 registerNativeObject (const Identifier& name,
                               DynamicObject::Ptr dynamicObject,
                               std::optional<detail::qjs::JSValue> parent = std::nullopt)
    {
        auto wrapper  = std::make_unique<detail::DynamicObjectWrapper> (*engine, dynamicObject);
        auto* ctx     = engine->getQuickJSContext();
        auto jsObject = JS_NewObjectClass (ctx, (i32) detail::DynamicObjectWrapper::getClassId());
        detail::qjs::JS_SetOpaque (jsObject, (uk) wrapper.get());

        std::vector<detail::qjs::JSCFunctionListEntry> propertyFunctionList;

        for (const auto& [identifier, prop] : wrapper->getProperties())
        {
            auto* jsIdentifier = identifier.toString().toRawUTF8();

            if (prop.isMethod())
            {
                detail::qjs::JS_SetPropertyStr (ctx,
                                                jsObject,
                                                jsIdentifier,
                                                JS_NewCFunctionMagic (ctx,
                                                                      detail::DynamicObjectWrapper::callDispatcher,
                                                                      jsIdentifier,
                                                                      0,
                                                                      detail::qjs::JS_CFUNC_generic_magic,
                                                                      wrapper->getOrdinal (identifier)));
            }
            else if (prop.isObject())
            {
                if (auto* embeddedObject = prop.getDynamicObject())
                    registerNativeObject (identifier, embeddedObject, jsObject);
            }
            else
            {
                const auto entry = detail::makeFunctionListEntry (jsIdentifier,
                                                                  detail::DynamicObjectWrapper::getDispatcher,
                                                                  detail::DynamicObjectWrapper::setDispatcher,
                                                                  wrapper->getOrdinal (identifier));
                propertyFunctionList.push_back (entry);
            }
        }

        if (! propertyFunctionList.empty())
        {
            detail::qjs::JS_SetPropertyFunctionList (ctx,
                                             jsObject,
                                             propertyFunctionList.data(),
                                             (i32) propertyFunctionList.size());
        }

        const auto jsObjectName = name.toString().toRawUTF8();

        if (parent.has_value())
        {
            detail::qjs::JS_SetPropertyStr (ctx, *parent, jsObjectName, jsObject);
        }
        else
        {
            ValuePtr globalObject { detail::qjs::JS_GetGlobalObject (ctx), ctx };
            detail::qjs::JS_SetPropertyStr (ctx, globalObject.get(), jsObjectName, jsObject);
        }

        wrapper.release();
    }

    var evaluate (const Txt& code, Result* errorMessage)
    {
        engine->resetTimeout();

        if (errorMessage != nullptr)
            *errorMessage = Result::ok();

        const auto result = detail::quickJSToDrx ({ JS_Eval (engine->getQuickJSContext(),
                                                              code.toRawUTF8(),
                                                              code.getNumBytesAsUTF8(),
                                                              "",
                                                              JS_EVAL_TYPE_GLOBAL),
                                                     engine->getQuickJSContext() });

        if (auto* v = std::get_if<var> (&result))
            return *v;

        if (auto* e = std::get_if<Txt> (&result))
            if (errorMessage != nullptr)
                *errorMessage = Result::fail (*e);

        return var::undefined();
    }

    Result execute (const Txt& code)
    {
        auto result = Result::ok();
        evaluate (code, &result);
        return result;
    }

    var callFunction (const Identifier& function,
                      const var::NativeFunctionArgs& args,
                      Result* errorMessage)
    {
        engine->resetTimeout();

        auto* ctx = engine->getQuickJSContext();
        const auto functionStr = function.toString();

        const auto fn = detail::qjs::JS_NewAtomLen (ctx, functionStr.toRawUTF8(), functionStr.getNumBytesAsUTF8());

        detail::JSFunctionArguments argList { ctx, args };

        detail::qjs::QuickJSContext::ValuePtr global { JS_GetGlobalObject (ctx), ctx };
        detail::qjs::QuickJSContext::ValuePtr returnVal { JS_Invoke (ctx, global.get(), fn, argList.getSize(), argList.getArguments()), ctx };

        JS_FreeAtom (ctx, fn);

        if (errorMessage != nullptr)
            *errorMessage = Result::ok();

        const auto result = detail::quickJSToDrx (returnVal);

        if (auto* v = std::get_if<var> (&result))
            return *v;

        if (auto* e = std::get_if<Txt> (&result))
            if (errorMessage != nullptr)
                *errorMessage = Result::fail (*e);

        return var::undefined();
    }

    z0 stop() noexcept
    {
        engine->stop();
    }

    JSObject getRootObject() const
    {
        return JSObject { engine.get() };
    }

private:
    std::unique_ptr<detail::QuickJSWrapper> engine;
};

//==============================================================================
JavascriptEngine::JavascriptEngine()
    : maximumExecutionTime (15.0),
      impl (std::make_unique<Impl> (&maximumExecutionTime))
{
}

JavascriptEngine::~JavascriptEngine() = default;

z0 JavascriptEngine::registerNativeObject (const Identifier& name, DynamicObject* object)
{
    impl->registerNativeObject (name, object);
}

Result JavascriptEngine::execute (const Txt& javascriptCode)
{
    return impl->execute (javascriptCode);
}

var JavascriptEngine::evaluate (const Txt& javascriptCode, Result* errorMessage)
{
    return impl->evaluate (javascriptCode, errorMessage);
}

var JavascriptEngine::callFunction (const Identifier& function,
                                    const var::NativeFunctionArgs& args,
                                    Result* errorMessage)
{
    return impl->callFunction (function, args, errorMessage);
}

z0 JavascriptEngine::stop() noexcept
{
    impl->stop();
}

JSObject JavascriptEngine::getRootObject() const
{
    return impl->getRootObject();
}

NamedValueSet JavascriptEngine::getRootObjectProperties() const
{
    return getRootObject().getProperties();
}

} // namespace drx
