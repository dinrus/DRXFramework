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

WinRTWrapper::WinRTWrapper()
{
    winRTHandle = ::LoadLibraryA ("api-ms-win-core-winrt-l1-1-0");

    if (winRTHandle == nullptr)
        return;

    roInitialize           = (RoInitializeFuncPtr)              ::GetProcAddress (winRTHandle, "RoInitialize");
    createHString          = (WindowsCreateStringFuncPtr)       ::GetProcAddress (winRTHandle, "WindowsCreateString");
    deleteHString          = (WindowsDeleteStringFuncPtr)       ::GetProcAddress (winRTHandle, "WindowsDeleteString");
    getHStringRawBuffer    = (WindowsGetStringRawBufferFuncPtr) ::GetProcAddress (winRTHandle, "WindowsGetStringRawBuffer");
    roActivateInstance     = (RoActivateInstanceFuncPtr)        ::GetProcAddress (winRTHandle, "RoActivateInstance");
    roGetActivationFactory = (RoGetActivationFactoryFuncPtr)    ::GetProcAddress (winRTHandle, "RoGetActivationFactory");

    if (roInitialize == nullptr || createHString == nullptr || deleteHString == nullptr
        || getHStringRawBuffer == nullptr || roActivateInstance == nullptr || roGetActivationFactory == nullptr)
        return;

    HRESULT status = roInitialize (1);
    initialised = ! (status != S_OK && status != S_FALSE && status != (HRESULT) 0x80010106L);
}

WinRTWrapper::~WinRTWrapper()
{
    if (winRTHandle != nullptr)
        ::FreeLibrary (winRTHandle);

    clearSingletonInstance();
}

WinRTWrapper::ScopedHString::ScopedHString (Txt str)
{
    if (WinRTWrapper::getInstance()->isInitialised())
        WinRTWrapper::getInstance()->createHString (str.toWideCharPointer(),
                                                    static_cast<u32> (str.length()),
                                                    &hstr);
}

WinRTWrapper::ScopedHString::~ScopedHString()
{
    if (WinRTWrapper::getInstance()->isInitialised() && hstr != nullptr)
        WinRTWrapper::getInstance()->deleteHString (hstr);
}

Txt WinRTWrapper::hStringToString (HSTRING hstr)
{
    if (isInitialised())
        if (const wchar_t* str = getHStringRawBuffer (hstr, nullptr))
            return Txt (str);

    return {};
}

}
