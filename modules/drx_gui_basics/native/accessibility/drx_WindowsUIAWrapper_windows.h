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

class WindowsUIAWrapper  : public DeletedAtShutdown
{
public:
    b8 isLoaded() const noexcept
    {
        return uiaReturnRawElementProvider            != nullptr
            && uiaHostProviderFromHwnd                != nullptr
            && uiaRaiseAutomationPropertyChangedEvent != nullptr
            && uiaRaiseAutomationEvent                != nullptr
            && uiaClientsAreListening                 != nullptr
            && uiaDisconnectProvider                  != nullptr
            && uiaDisconnectAllProviders              != nullptr;
    }

    //==============================================================================
    LRESULT returnRawElementProvider (HWND hwnd, WPARAM wParam, LPARAM lParam, IRawElementProviderSimple* provider)
    {
        return uiaReturnRawElementProvider != nullptr ? uiaReturnRawElementProvider (hwnd, wParam, lParam, provider)
                                                      : (LRESULT) nullptr;
    }

    DRX_COMRESULT hostProviderFromHwnd (HWND hwnd, IRawElementProviderSimple** provider)
    {
        return uiaHostProviderFromHwnd != nullptr ? uiaHostProviderFromHwnd (hwnd, provider)
                                                  : (HRESULT) UIA_E_NOTSUPPORTED;
    }

    DRX_COMRESULT raiseAutomationPropertyChangedEvent (IRawElementProviderSimple* provider, PROPERTYID propID, VARIANT oldValue, VARIANT newValue)
    {
        return uiaRaiseAutomationPropertyChangedEvent != nullptr ? uiaRaiseAutomationPropertyChangedEvent (provider, propID, oldValue, newValue)
                                                                 : (HRESULT) UIA_E_NOTSUPPORTED;
    }

    DRX_COMRESULT raiseAutomationEvent (IRawElementProviderSimple* provider, EVENTID eventID)
    {
        return uiaRaiseAutomationEvent != nullptr ? uiaRaiseAutomationEvent (provider, eventID)
                                                  : (HRESULT) UIA_E_NOTSUPPORTED;
    }

    BOOL clientsAreListening()
    {
        return uiaClientsAreListening != nullptr ? uiaClientsAreListening()
                                                 : false;
    }

    DRX_COMRESULT disconnectProvider (IRawElementProviderSimple* provider)
    {
        if (uiaDisconnectProvider != nullptr)
        {
            const ScopedValueSetter<IRawElementProviderSimple*> disconnectingProviderSetter (disconnectingProvider, provider);
            return uiaDisconnectProvider (provider);
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    DRX_COMRESULT disconnectAllProviders()
    {
        if (uiaDisconnectAllProviders != nullptr)
        {
            const ScopedValueSetter<b8> disconnectingAllProvidersSetter (disconnectingAllProviders, true);
            return uiaDisconnectAllProviders();
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    //==============================================================================
    b8 isProviderDisconnecting (IRawElementProviderSimple* provider)
    {
        return disconnectingProvider == provider || disconnectingAllProviders;
    }

    //==============================================================================
    DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (WindowsUIAWrapper)

private:
    //==============================================================================
    WindowsUIAWrapper()
    {
        // force UIA COM library initialisation here to prevent an exception when calling methods from SendMessage()
        if (isLoaded())
            returnRawElementProvider (nullptr, 0, 0, nullptr);
        else
            jassertfalse; // UIAutomationCore could not be loaded!
    }

    ~WindowsUIAWrapper()
    {
        disconnectAllProviders();

        if (uiaHandle != nullptr)
            ::FreeLibrary (uiaHandle);

        clearSingletonInstance();
    }

    //==============================================================================
    template <typename FuncType>
    static FuncType getUiaFunction (HMODULE module, LPCSTR funcName)
    {
        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-function-type")
        return (FuncType) GetProcAddress (module, funcName);
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
    }

    //==============================================================================
    using UiaReturnRawElementProviderFunc            = LRESULT (WINAPI*) (HWND, WPARAM, LPARAM, IRawElementProviderSimple*);
    using UiaHostProviderFromHwndFunc                = HRESULT (WINAPI*) (HWND, IRawElementProviderSimple**);
    using UiaRaiseAutomationPropertyChangedEventFunc = HRESULT (WINAPI*) (IRawElementProviderSimple*, PROPERTYID, VARIANT, VARIANT);
    using UiaRaiseAutomationEventFunc                = HRESULT (WINAPI*) (IRawElementProviderSimple*, EVENTID);
    using UiaClientsAreListeningFunc                 = BOOL    (WINAPI*) ();
    using UiaDisconnectProviderFunc                  = HRESULT (WINAPI*) (IRawElementProviderSimple*);
    using UiaDisconnectAllProvidersFunc              = HRESULT (WINAPI*) ();

    HMODULE uiaHandle = ::LoadLibraryA ("UIAutomationCore.dll");
    UiaReturnRawElementProviderFunc            uiaReturnRawElementProvider            = getUiaFunction<UiaReturnRawElementProviderFunc>            (uiaHandle, "UiaReturnRawElementProvider");
    UiaHostProviderFromHwndFunc                uiaHostProviderFromHwnd                = getUiaFunction<UiaHostProviderFromHwndFunc>                (uiaHandle, "UiaHostProviderFromHwnd");
    UiaRaiseAutomationPropertyChangedEventFunc uiaRaiseAutomationPropertyChangedEvent = getUiaFunction<UiaRaiseAutomationPropertyChangedEventFunc> (uiaHandle, "UiaRaiseAutomationPropertyChangedEvent");
    UiaRaiseAutomationEventFunc                uiaRaiseAutomationEvent                = getUiaFunction<UiaRaiseAutomationEventFunc>                (uiaHandle, "UiaRaiseAutomationEvent");
    UiaClientsAreListeningFunc                 uiaClientsAreListening                 = getUiaFunction<UiaClientsAreListeningFunc>                 (uiaHandle, "UiaClientsAreListening");
    UiaDisconnectProviderFunc                  uiaDisconnectProvider                  = getUiaFunction<UiaDisconnectProviderFunc>                  (uiaHandle, "UiaDisconnectProvider");
    UiaDisconnectAllProvidersFunc              uiaDisconnectAllProviders              = getUiaFunction<UiaDisconnectAllProvidersFunc>              (uiaHandle, "UiaDisconnectAllProviders");

    IRawElementProviderSimple* disconnectingProvider = nullptr;
    b8 disconnectingAllProviders = false;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsUIAWrapper)
};

} // namespace drx
