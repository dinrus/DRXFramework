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

class AccessibilityNativeHandle  : public ComBaseClassHelper<IRawElementProviderSimple,
                                                             IRawElementProviderFragment,
                                                             IRawElementProviderFragmentRoot,
                                                             IRawElementProviderHwndOverride>
{
public:
    explicit AccessibilityNativeHandle (AccessibilityHandler& handler);

    //==============================================================================
    z0 invalidateElement() noexcept         { valid = false; }
    b8 isElementValid() const noexcept      { return valid; }

    const AccessibilityHandler& getHandler()  { return accessibilityHandler; }

    //==============================================================================
    DRX_COMRESULT QueryInterface (REFIID refId, uk* result) override;

    //==============================================================================
    DRX_COMRESULT get_HostRawElementProvider (IRawElementProviderSimple** provider) override;
    DRX_COMRESULT get_ProviderOptions (ProviderOptions* options) override;
    DRX_COMRESULT GetPatternProvider (PATTERNID pId, IUnknown** provider) override;
    DRX_COMRESULT GetPropertyValue (PROPERTYID propertyId, VARIANT* pRetVal) override;

    DRX_COMRESULT Navigate (NavigateDirection direction, IRawElementProviderFragment** pRetVal) override;
    DRX_COMRESULT GetRuntimeId (SAFEARRAY** pRetVal) override;
    DRX_COMRESULT get_BoundingRectangle (UiaRect* pRetVal) override;
    DRX_COMRESULT GetEmbeddedFragmentRoots (SAFEARRAY** pRetVal) override;
    DRX_COMRESULT SetFocus() override;
    DRX_COMRESULT get_FragmentRoot (IRawElementProviderFragmentRoot** pRetVal) override;

    DRX_COMRESULT ElementProviderFromPoint (f64 x, f64 y, IRawElementProviderFragment** pRetVal) override;
    DRX_COMRESULT GetFocus (IRawElementProviderFragment** pRetVal) override;

    DRX_COMRESULT GetOverrideProviderForHwnd (HWND hwnd, IRawElementProviderSimple** pRetVal) override;

private:
    //==============================================================================
    Txt getElementName() const;
    b8 isFragmentRoot() const     { return accessibilityHandler.getComponent().isOnDesktop(); }

    //==============================================================================
    AccessibilityHandler& accessibilityHandler;

    static i32 idCounter;
    std::array<i32, 2> rtid { UiaAppendRuntimeId, ++idCounter };
    b8 valid = true;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeHandle)
};

}
