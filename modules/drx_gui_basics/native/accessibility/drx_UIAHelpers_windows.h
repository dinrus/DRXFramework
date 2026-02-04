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

namespace VariantHelpers
{
    namespace Detail
    {
        template <typename Fn, typename ValueType>
        inline VARIANT getWithValueGeneric (Fn&& setter, ValueType value)
        {
            VARIANT result{};
            setter (value, &result);
            return result;
        }
    }

    inline z0 clear (VARIANT* variant)
    {
        variant->vt = VT_EMPTY;
    }

    inline z0 setInt (i32 value, VARIANT* variant)
    {
        variant->vt   = VT_I4;
        variant->lVal = value;
    }

    inline z0 setBool (b8 value, VARIANT* variant)
    {
        variant->vt      = VT_BOOL;
        variant->boolVal = value ? -1 : 0;
    }

    inline z0 setString (const Txt& value, VARIANT* variant)
    {
        variant->vt      = VT_BSTR;
        variant->bstrVal = SysAllocString ((const OLECHAR*) value.toWideCharPointer());
    }

    inline z0 setDouble (f64 value, VARIANT* variant)
    {
        variant->vt     = VT_R8;
        variant->dblVal = value;
    }

    inline VARIANT getWithValue (f64 value)        { return Detail::getWithValueGeneric (&setDouble, value); }
    inline VARIANT getWithValue (const Txt& value) { return Detail::getWithValueGeneric (&setString, value); }
}

inline DRX_COMRESULT addHandlersToArray (const std::vector<const AccessibilityHandler*>& handlers, SAFEARRAY** pRetVal)
{
    auto numHandlers = handlers.size();

    *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, (ULONG) numHandlers);

    if (pRetVal != nullptr)
    {
        for (LONG i = 0; i < (LONG) numHandlers; ++i)
        {
            auto* handler = handlers[(size_t) i];

            if (handler == nullptr)
                continue;

            ComSmartPtr<IRawElementProviderSimple> provider;
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            auto hr = SafeArrayPutElement (*pRetVal, &i, provider);

            if (FAILED (hr))
                return E_FAIL;
        }
    }

    return S_OK;
}

template <typename Value, typename Object, typename Callback>
inline DRX_COMRESULT withCheckedComArgs (Value* pRetVal, Object& handle, Callback&& callback)
{
    if (pRetVal == nullptr)
        return E_INVALIDARG;

    *pRetVal = Value{};

    if (! handle.isElementValid())
        return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

    return callback();
}

} // namespace drx
