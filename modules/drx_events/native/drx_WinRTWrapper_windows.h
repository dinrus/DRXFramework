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

class WinRTWrapper :   public DeletedAtShutdown
{
public:
    //==============================================================================
    ~WinRTWrapper();
    b8 isInitialised() const noexcept  { return initialised; }

    DRX_DECLARE_SINGLETON_INLINE (WinRTWrapper, false)

    //==============================================================================
    template <class ComClass>
    ComSmartPtr<ComClass> activateInstance (const wchar_t* runtimeClassID, REFCLSID classUUID)
    {
        ComSmartPtr<ComClass> result;

        if (isInitialised())
        {
            ComSmartPtr<IInspectable> inspectable;
            ScopedHString runtimeClass (runtimeClassID);
            auto hr = roActivateInstance (runtimeClass.get(), inspectable.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
                inspectable->QueryInterface (classUUID, (uk*) result.resetAndGetPointerAddress());
        }

        return result;
    }

    template <class ComClass>
    ComSmartPtr<ComClass> getWRLFactory (const wchar_t* runtimeClassID)
    {
        ComSmartPtr<ComClass> comPtr;

        if (isInitialised())
        {
            ScopedHString classID (runtimeClassID);

            if (classID.get() != nullptr)
                roGetActivationFactory (classID.get(), __uuidof (ComClass), (uk*) comPtr.resetAndGetPointerAddress());
        }

        return comPtr;
    }

    //==============================================================================
    class ScopedHString
    {
    public:
        ScopedHString (Txt);
        ~ScopedHString();

        HSTRING get() const noexcept          { return hstr; }

    private:
        HSTRING hstr = nullptr;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedHString)
    };

    Txt hStringToString (HSTRING);

private:
    WinRTWrapper();

    //==============================================================================
    HMODULE winRTHandle = nullptr;
    b8 initialised = false;

    typedef HRESULT (WINAPI* RoInitializeFuncPtr) (i32);
    typedef HRESULT (WINAPI* WindowsCreateStringFuncPtr) (LPCWSTR, UINT32, HSTRING*);
    typedef HRESULT (WINAPI* WindowsDeleteStringFuncPtr) (HSTRING);
    typedef PCWSTR  (WINAPI* WindowsGetStringRawBufferFuncPtr) (HSTRING, UINT32*);
    typedef HRESULT (WINAPI* RoActivateInstanceFuncPtr) (HSTRING, IInspectable**);
    typedef HRESULT (WINAPI* RoGetActivationFactoryFuncPtr) (HSTRING, REFIID, uk*);

    RoInitializeFuncPtr roInitialize = nullptr;
    WindowsCreateStringFuncPtr createHString = nullptr;
    WindowsDeleteStringFuncPtr deleteHString = nullptr;
    WindowsGetStringRawBufferFuncPtr getHStringRawBuffer = nullptr;
    RoActivateInstanceFuncPtr roActivateInstance = nullptr;
    RoGetActivationFactoryFuncPtr roGetActivationFactory = nullptr;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTWrapper)
};

} // namespace drx
