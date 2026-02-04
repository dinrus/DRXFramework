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
class UIARangeValueProvider  : public UIAProviderBase,
                               public ComBaseClassHelper<IRangeValueProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    DRX_COMRESULT SetValue (f64 val) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();

        if (auto* valueInterface = handler.getValueInterface())
        {
            auto range = valueInterface->getRange();

            if (range.isValid())
            {
                if (val < range.getMinimumValue() || val > range.getMaximumValue())
                    return E_INVALIDARG;

                if (! valueInterface->isReadOnly())
                {
                    valueInterface->setValue (val);

                    VARIANT newValue;
                    VariantHelpers::setDouble (valueInterface->getCurrentValue(), &newValue);
                    sendAccessibilityPropertyChangedEvent (handler, UIA_RangeValueValuePropertyId, newValue);

                    return S_OK;
                }
            }
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    DRX_COMRESULT get_Value (f64* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.getCurrentValue();
        });
    }

    DRX_COMRESULT get_IsReadOnly (BOOL* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.isReadOnly();
        });
    }

    DRX_COMRESULT get_Maximum (f64* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.getRange().getMaximumValue();
        });
    }

    DRX_COMRESULT get_Minimum (f64* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.getRange().getMinimumValue();
        });
    }

    DRX_COMRESULT get_LargeChange (f64* pRetVal) override
    {
        return get_SmallChange (pRetVal);
    }

    DRX_COMRESULT get_SmallChange (f64* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.getRange().getInterval();
        });
    }

private:
    template <typename Value, typename Callback>
    DRX_COMRESULT withValueInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* valueInterface = getHandler().getValueInterface())
            {
                if (valueInterface->getRange().isValid())
                {
                    *pRetVal = callback (*valueInterface);
                    return S_OK;
                }
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIARangeValueProvider)
};

} // namespace drx
