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

extern uk getUser32Function (tukk);

namespace IconConverters
{
    extern HICON createHICONFromImage (const Image&, BOOL isIcon, i32 hotspotX, i32 hotspotY);
}

//==============================================================================
class SystemTrayIconComponent::Pimpl
{
public:
    Pimpl (SystemTrayIconComponent& owner_, HICON hicon, HWND hwnd)
        : owner (owner_),
          originalWndProc ((WNDPROC) GetWindowLongPtr (hwnd, GWLP_WNDPROC)),
          taskbarCreatedMessage (RegisterWindowMessage (TEXT ("TaskbarCreated")))
    {
        SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR) hookedWndProc);

        zerostruct (iconData);
        iconData.cbSize = sizeof (iconData);
        iconData.hWnd = hwnd;
        iconData.uID = (UINT) (pointer_sized_int) hwnd;
        iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        iconData.uCallbackMessage = WM_TRAYNOTIFY;
        iconData.hIcon = hicon;

        notify (NIM_ADD);

        // In order to receive the "TaskbarCreated" message, we need to request that it's not filtered out.
        // (Need to load dynamically, as ChangeWindowMessageFilter is only available in Vista and later)
        typedef BOOL (WINAPI* ChangeWindowMessageFilterType) (UINT, DWORD);

        if (ChangeWindowMessageFilterType changeWindowMessageFilter
                = (ChangeWindowMessageFilterType) getUser32Function ("ChangeWindowMessageFilter"))
            changeWindowMessageFilter (taskbarCreatedMessage, 1 /* MSGFLT_ADD */);
    }

    ~Pimpl()
    {
        SetWindowLongPtr (iconData.hWnd, GWLP_WNDPROC, (LONG_PTR) originalWndProc);

        iconData.uFlags = 0;
        notify (NIM_DELETE);
        DestroyIcon (iconData.hIcon);
    }

    z0 updateIcon (HICON hicon)
    {
        HICON oldIcon = iconData.hIcon;

        iconData.hIcon = hicon;
        iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        notify (NIM_MODIFY);

        DestroyIcon (oldIcon);
    }

    z0 setToolTip (const Txt& toolTip)
    {
        iconData.uFlags = NIF_TIP;
        toolTip.copyToUTF16 (iconData.szTip, sizeof (iconData.szTip) - 1);
        notify (NIM_MODIFY);
    }

    z0 handleTaskBarEvent (const LPARAM lParam)
    {
        if (owner.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN
                 || lParam == WM_LBUTTONDBLCLK || lParam == WM_RBUTTONDBLCLK)
            {
                if (auto* current = Component::getCurrentlyModalComponent())
                    current->inputAttemptWhenModal();
            }
        }
        else
        {
            ModifierKeys eventMods (ComponentPeer::getCurrentModifiersRealtime());

            if (lParam == WM_LBUTTONDOWN || lParam == WM_LBUTTONDBLCLK)
                eventMods = eventMods.withFlags (ModifierKeys::leftButtonModifier);
            else if (lParam == WM_RBUTTONDOWN || lParam == WM_RBUTTONDBLCLK)
                eventMods = eventMods.withFlags (ModifierKeys::rightButtonModifier);
            else if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
                eventMods = eventMods.withoutMouseButtons();

            const Time eventTime (getMouseEventTime());

            const MouseEvent e (Desktop::getInstance().getMainMouseSource(), {}, eventMods,
                                MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation,
                                MouseInputSource::defaultRotation, MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                &owner, &owner, eventTime, {}, eventTime, 1, false);

            if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN)
            {
                SetFocus (iconData.hWnd);
                SetForegroundWindow (iconData.hWnd);
                owner.mouseDown (e);
            }
            else if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
            {
                owner.mouseUp (e);
            }
            else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_RBUTTONDBLCLK)
            {
                owner.mouseDoubleClick (e);
            }
            else if (lParam == WM_MOUSEMOVE)
            {
                owner.mouseMove (e);
            }
        }
    }

    static Pimpl* getPimpl (HWND hwnd)
    {
        if (DrxWindowIdentifier::isDRXWindow (hwnd))
            if (ComponentPeer* peer = (ComponentPeer*) GetWindowLongPtr (hwnd, 8))
                if (SystemTrayIconComponent* const iconComp = dynamic_cast<SystemTrayIconComponent*> (&(peer->getComponent())))
                    return iconComp->pimpl.get();

        return nullptr;
    }

    static LRESULT CALLBACK hookedWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (Pimpl* const p = getPimpl (hwnd))
            return p->windowProc  (hwnd, message, wParam, lParam);

        return DefWindowProcW (hwnd, message, wParam, lParam);
   }

    LRESULT windowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_TRAYNOTIFY)
        {
            handleTaskBarEvent (lParam);
        }
        else if (message == taskbarCreatedMessage)
        {
            iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            notify (NIM_ADD);
        }

        return CallWindowProc (originalWndProc, hwnd, message, wParam, lParam);
    }

    z0 showBubble (const Txt& title, const Txt& content)
    {
        iconData.uFlags = 0x10 /*NIF_INFO*/;
        title.copyToUTF16 (iconData.szInfoTitle, sizeof (iconData.szInfoTitle) - 1);
        content.copyToUTF16 (iconData.szInfo, sizeof (iconData.szInfo) - 1);
        notify (NIM_MODIFY);
    }

    SystemTrayIconComponent& owner;
    NOTIFYICONDATA iconData;

private:
    WNDPROC originalWndProc;
    const DWORD taskbarCreatedMessage;
    enum { WM_TRAYNOTIFY = WM_USER + 100 };

    z0 notify (DWORD message) noexcept    { Shell_NotifyIcon (message, &iconData); }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
z0 SystemTrayIconComponent::setIconImage (const Image& colourImage, const Image&)
{
    if (colourImage.isValid())
    {
        HICON hicon = IconConverters::createHICONFromImage (colourImage, TRUE, 0, 0);

        if (pimpl == nullptr)
            pimpl.reset (new Pimpl (*this, hicon, (HWND) getWindowHandle()));
        else
            pimpl->updateIcon (hicon);
    }
    else
    {
        pimpl.reset();
    }
}

z0 SystemTrayIconComponent::setIconTooltip (const Txt& tooltip)
{
    if (pimpl != nullptr)
        pimpl->setToolTip (tooltip);
}

z0 SystemTrayIconComponent::setHighlighted (b8)
{
    // N/A on Windows.
}

z0 SystemTrayIconComponent::showInfoBubble (const Txt& title, const Txt& content)
{
    if (pimpl != nullptr)
        pimpl->showBubble (title, content);
}

z0 SystemTrayIconComponent::hideInfoBubble()
{
    showInfoBubble (Txt(), Txt());
}

uk SystemTrayIconComponent::getNativeHandle() const
{
    return pimpl != nullptr ? &(pimpl->iconData) : nullptr;
}

} // namespace drx
