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
class UIAGridProvider  : public UIAProviderBase,
                         public ComBaseClassHelper<IGridProvider, ITableProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    DRX_COMRESULT GetItem (i32 row, i32 column, IRawElementProviderSimple** pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityTableInterface& tableInterface)
        {
            if (! isPositiveAndBelow (row, tableInterface.getNumRows())
                || ! isPositiveAndBelow (column, tableInterface.getNumColumns()))
                return E_INVALIDARG;

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

            if (auto* cellHandler = tableInterface.getCellHandler (row, column))
            {
                cellHandler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
                return S_OK;
            }

            if (auto* rowHandler = tableInterface.getRowHandler (row))
            {
                rowHandler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
                return S_OK;
            }

            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            return E_FAIL;
        });
    }

    DRX_COMRESULT get_RowCount (i32* pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityTableInterface& tableInterface)
        {
            *pRetVal = tableInterface.getNumRows();
            return S_OK;
        });
    }

    DRX_COMRESULT get_ColumnCount (i32* pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityTableInterface& tableInterface)
        {
            *pRetVal = tableInterface.getNumColumns();
            return S_OK;
        });
    }

    DRX_COMRESULT GetRowHeaders (SAFEARRAY**) override
    {
        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    DRX_COMRESULT GetColumnHeaders (SAFEARRAY** pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityTableInterface& tableInterface)
        {
            if (auto* header = tableInterface.getHeaderHandler())
            {
                const auto children = header->getChildren();

                *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, (ULONG) children.size());

                LONG index = 0;

                for (const auto& child : children)
                {
                    ComSmartPtr<IRawElementProviderSimple> provider;

                    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
                    if (child != nullptr)
                        child->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));
                    DRX_END_IGNORE_WARNINGS_GCC_LIKE

                    if (provider == nullptr)
                        return E_FAIL;

                    const auto hr = SafeArrayPutElement (*pRetVal, &index, provider);

                    if (FAILED (hr))
                        return E_FAIL;

                    ++index;
                }

                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    DRX_COMRESULT get_RowOrColumnMajor (RowOrColumnMajor* pRetVal) override
    {
        *pRetVal = RowOrColumnMajor_RowMajor;
        return S_OK;
    }

private:
    template <typename Value, typename Callback>
    DRX_COMRESULT withTableInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (&getHandler(), &AccessibilityHandler::getTableInterface))
                if (auto* tableInterface = tableHandler->getTableInterface())
                    return callback (*tableInterface);

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAGridProvider)
};

} // namespace drx
