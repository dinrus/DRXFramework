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
class HiddenMessageWindow
{
public:
    HiddenMessageWindow (const TCHAR* const messageWindowName, WNDPROC wndProc)
    {
        Txt className ("DRX_");
        className << Txt::toHexString (Time::getHighResolutionTicks());

        HMODULE moduleHandle = (HMODULE) Process::getCurrentModuleInstanceHandle();

        WNDCLASSEX wc = {};
        wc.cbSize         = sizeof (wc);
        wc.lpfnWndProc    = wndProc;
        wc.cbWndExtra     = 4;
        wc.hInstance      = moduleHandle;
        wc.lpszClassName  = className.toWideCharPointer();

        atom = RegisterClassEx (&wc);
        jassert (atom != 0);

        hwnd = CreateWindow (getClassNameFromAtom(), messageWindowName,
                             0, 0, 0, 0, 0,
                             nullptr, nullptr, moduleHandle, nullptr);
        jassert (hwnd != nullptr);
    }

    ~HiddenMessageWindow()
    {
        DestroyWindow (hwnd);
        UnregisterClass (getClassNameFromAtom(), nullptr);
    }

    inline HWND getHWND() const noexcept     { return hwnd; }

private:
    ATOM atom;
    HWND hwnd;

    LPCTSTR getClassNameFromAtom() noexcept  { return (LPCTSTR) (pointer_sized_uint) atom; }
};

//==============================================================================
class DrxWindowIdentifier
{
public:
    static b8 isDRXWindow (HWND hwnd) noexcept
    {
        return GetWindowLongPtr (hwnd, GWLP_USERDATA) == getImprobableWindowNumber();
    }

    static z0 setAsDRXWindow (HWND hwnd, b8 isDrxWindow) noexcept
    {
        SetWindowLongPtr (hwnd, GWLP_USERDATA, isDrxWindow ? getImprobableWindowNumber() : 0);
    }

private:
    static LONG_PTR getImprobableWindowNumber() noexcept
    {
        static auto number = (LONG_PTR) Random().nextInt64();
        return number;
    }
};

//==============================================================================
class DeviceChangeDetector  : private Timer
{
public:
    DeviceChangeDetector (const wchar_t* const name, std::function<z0()> onChangeIn)
        : messageWindow (name, (WNDPROC) deviceChangeEventCallback),
          onChange (std::move (onChangeIn))
    {
        SetWindowLongPtr (messageWindow.getHWND(), GWLP_USERDATA, (LONG_PTR) this);
    }

    z0 triggerAsyncDeviceChangeCallback()
    {
        // We'll pause before sending a message, because on device removal, the OS hasn't always updated
        // its device lists correctly at this point. This also helps avoid repeated callbacks.
        startTimer (500);
    }

private:
    HiddenMessageWindow messageWindow;
    std::function<z0()> onChange;

    static LRESULT CALLBACK deviceChangeEventCallback (HWND h, const UINT message,
                                                       const WPARAM wParam, const LPARAM lParam)
    {
        if (message == WM_DEVICECHANGE
             && (wParam == 0x8000 /*DBT_DEVICEARRIVAL*/
                  || wParam == 0x8004 /*DBT_DEVICEREMOVECOMPLETE*/
                  || wParam == 0x0007 /*DBT_DEVNODES_CHANGED*/))
        {
            ((DeviceChangeDetector*) GetWindowLongPtr (h, GWLP_USERDATA))
                ->triggerAsyncDeviceChangeCallback();
        }

        return DefWindowProc (h, message, wParam, lParam);
    }

    z0 timerCallback() override
    {
        stopTimer();
        NullCheckedInvocation::invoke (onChange);
    }
};

} // namespace drx
