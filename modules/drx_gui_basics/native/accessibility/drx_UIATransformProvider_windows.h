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
class UIATransformProvider  : public UIAProviderBase,
                              public ComBaseClassHelper<ITransformProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    DRX_COMRESULT Move (f64 x, f64 y) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        if (auto* peer = getPeer())
        {
            RECT rect;
            GetWindowRect ((HWND) peer->getNativeHandle(), &rect);

            rect.left = roundToInt (x);
            rect.top  = roundToInt (y);

            auto bounds = Rectangle<i32>::leftTopRightBottom (rect.left, rect.top, rect.right, rect.bottom);

            peer->setBounds (Desktop::getInstance().getDisplays().physicalToLogical (bounds),
                             peer->isFullScreen());
        }

        return S_OK;
    }

    DRX_COMRESULT Resize (f64 width, f64 height) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        if (auto* peer = getPeer())
        {
            auto scale = peer->getPlatformScaleFactor();

            peer->getComponent().setSize (roundToInt (width  / scale),
                                          roundToInt (height / scale));
        }

        return S_OK;
    }

    DRX_COMRESULT Rotate (f64) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    DRX_COMRESULT get_CanMove (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = true;
            return S_OK;
        });
    }

    DRX_COMRESULT get_CanResize (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            if (auto* peer = getPeer())
                *pRetVal = ((peer->getStyleFlags() & ComponentPeer::windowIsResizable) != 0);

            return S_OK;
        });
    }

    DRX_COMRESULT get_CanRotate (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = false;
            return S_OK;
        });
    }

private:
    ComponentPeer* getPeer() const
    {
        return getHandler().getComponent().getPeer();
    }

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIATransformProvider)
};

} // namespace drx
