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
class UIAGridItemProvider  : public UIAProviderBase,
                             public ComBaseClassHelper<IGridItemProvider, ITableItemProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    DRX_COMRESULT get_Row (i32* pRetVal) override
    {
        return withTableSpan (pRetVal,
                              &AccessibilityTableInterface::getRowSpan,
                              &AccessibilityTableInterface::Span::begin);
    }

    DRX_COMRESULT get_Column (i32* pRetVal) override
    {
        return withTableSpan (pRetVal,
                              &AccessibilityTableInterface::getColumnSpan,
                              &AccessibilityTableInterface::Span::begin);
    }

    DRX_COMRESULT get_RowSpan (i32* pRetVal) override
    {
        return withTableSpan (pRetVal,
                              &AccessibilityTableInterface::getRowSpan,
                              &AccessibilityTableInterface::Span::num);
    }

    DRX_COMRESULT get_ColumnSpan (i32* pRetVal) override
    {
        return withTableSpan (pRetVal,
                              &AccessibilityTableInterface::getColumnSpan,
                              &AccessibilityTableInterface::Span::num);
    }

    DRX_COMRESULT get_ContainingGrid (IRawElementProviderSimple** pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityHandler& tableHandler)
        {
            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            tableHandler.getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            return true;
        });
    }

    DRX_COMRESULT GetRowHeaderItems (SAFEARRAY**) override
    {
        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    DRX_COMRESULT GetColumnHeaderItems (SAFEARRAY** pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityHandler& tableHandler)
        {
            if (auto* tableInterface = tableHandler.getTableInterface())
            {
                if (const auto column = tableInterface->getColumnSpan (getHandler()))
                {
                    if (auto* header = tableInterface->getHeaderHandler())
                    {
                        const auto children = header->getChildren();

                        if (isPositiveAndBelow (column->begin, children.size()))
                        {
                            ComSmartPtr<IRawElementProviderSimple> provider;

                            if (auto* child = children[(size_t) column->begin])
                            {
                                DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
                                if (child->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress())) == S_OK && provider != nullptr)
                                {
                                    *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, 1);
                                    LONG index = 0;
                                    const auto hr = SafeArrayPutElement (*pRetVal, &index, provider);

                                    return ! FAILED (hr);
                                }
                                DRX_END_IGNORE_WARNINGS_GCC_LIKE
                            }
                        }
                    }
                }
            }

            return false;
        });
    }
private:
    template <typename Value, typename Callback>
    DRX_COMRESULT withTableInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* handler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (&getHandler(), &AccessibilityHandler::getTableInterface))
                if (handler->getTableInterface() != nullptr && callback (*handler))
                    return S_OK;

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    DRX_COMRESULT withTableSpan (i32* pRetVal,
                                  Optional<AccessibilityTableInterface::Span> (AccessibilityTableInterface::* getSpan) (const AccessibilityHandler&) const,
                                  i32 AccessibilityTableInterface::Span::* spanMember) const
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityHandler& handler)
        {
            if (const auto span = ((handler.getTableInterface())->*getSpan) (getHandler()))
            {
                *pRetVal = (*span).*spanMember;
                return true;
            }

            return false;
        });
    }

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAGridItemProvider)
};

} // namespace drx
