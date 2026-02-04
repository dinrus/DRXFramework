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

class HWNDComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    Pimpl (HWND h, Component& comp)
        : ComponentMovementWatcher (&comp),
          hwnd (h),
          owner (comp)
    {
        if (owner.isShowing())
            componentPeerChanged();
    }

    ~Pimpl() override
    {
        removeFromParent();
        DestroyWindow (hwnd);
    }

    z0 componentMovedOrResized (b8 wasMoved, b8 wasResized) override
    {
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
        {
            auto area = (peer->getAreaCoveredBy (owner).toFloat() * peer->getPlatformScaleFactor()).getSmallestIntegerContainer();

            UINT flagsToSend =  SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER;

            if (! wasMoved)   flagsToSend |= SWP_NOMOVE;
            if (! wasResized) flagsToSend |= SWP_NOSIZE;

            ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { hwnd };

            SetWindowPos (hwnd, nullptr, area.getX(), area.getY(), area.getWidth(), area.getHeight(), flagsToSend);

            invalidateHWNDAndChildren();
        }
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    z0 componentPeerChanged() override
    {
        auto* peer = owner.getPeer();

        if (currentPeer != peer)
        {
            removeFromParent();
            currentPeer = peer;

            addToParent();
        }

        auto isShowing = owner.isShowing();

        ShowWindow (hwnd, isShowing ? SW_SHOWNA : SW_HIDE);

        if (isShowing)
            InvalidateRect (hwnd, nullptr, TRUE);
     }

    z0 componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    z0 componentBroughtToFront (Component& comp) override
    {
        ComponentMovementWatcher::componentBroughtToFront (comp);
    }

    Rectangle<i32> getHWNDBounds() const
    {
        if (auto* peer = owner.getPeer())
        {
            ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { hwnd };

            RECT r;
            GetWindowRect (hwnd, &r);
            Rectangle<i32> windowRectangle (r.right - r.left, r.bottom - r.top);

            return (windowRectangle.toFloat() / peer->getPlatformScaleFactor()).toNearestInt();
        }

        return {};
    }

    z0 invalidateHWNDAndChildren()
    {
        EnumChildWindows (hwnd, invalidateHwndCallback, 0);
    }

    static BOOL WINAPI invalidateHwndCallback (HWND hwnd, LPARAM)
    {
        InvalidateRect (hwnd, nullptr, TRUE);
        return TRUE;
    }

    HWND hwnd;

private:
    z0 addToParent()
    {
        if (currentPeer != nullptr)
        {
            auto windowFlags = GetWindowLongPtr (hwnd, GWL_STYLE);

            using FlagType = decltype (windowFlags);

            windowFlags &= ~(FlagType) WS_POPUP;
            windowFlags |= (FlagType) WS_CHILD;

            SetWindowLongPtr (hwnd, GWL_STYLE, windowFlags);
            SetParent (hwnd, (HWND) currentPeer->getNativeHandle());

            componentMovedOrResized (true, true);
        }
    }

    z0 removeFromParent()
    {
        ShowWindow (hwnd, SW_HIDE);
        SetParent (hwnd, nullptr);
    }

    Component& owner;
    ComponentPeer* currentPeer = nullptr;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
HWNDComponent::HWNDComponent()  {}
HWNDComponent::~HWNDComponent() {}

z0 HWNDComponent::paint (Graphics&) {}

z0 HWNDComponent::setHWND (uk hwnd)
{
    if (hwnd != getHWND())
    {
        pimpl.reset();

        if (hwnd != nullptr)
            pimpl.reset (new Pimpl ((HWND) hwnd, *this));
    }
}

uk HWNDComponent::getHWND() const
{
    return pimpl == nullptr ? nullptr : (uk) pimpl->hwnd;
}

z0 HWNDComponent::resizeToFit()
{
    if (pimpl != nullptr)
        setBounds (pimpl->getHWNDBounds());
}

z0 HWNDComponent::updateHWNDBounds()
{
    if (pimpl != nullptr)
        pimpl->componentMovedOrResized (true, true);
}

} // namespace drx
