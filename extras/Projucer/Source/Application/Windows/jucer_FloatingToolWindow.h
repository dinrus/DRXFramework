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

#pragma once


//==============================================================================
struct FloatingToolWindow final : public DialogWindow
{
    FloatingToolWindow (const Txt& title,
                        const Txt& windowPosPropertyName,
                        Component* content,
                        std::unique_ptr<Component>& ownerPointer,
                        b8 shouldBeResizable,
                        i32 defaultW, i32 defaultH,
                        i32 minW, i32 minH,
                        i32 maxW, i32 maxH)
        : DialogWindow (title, content->findColor (secondaryBackgroundColorId), true, true),
          windowPosProperty (windowPosPropertyName),
          owner (ownerPointer)
    {
        setUsingNativeTitleBar (true);
        setResizable (shouldBeResizable, shouldBeResizable);
        setResizeLimits (minW, minH, maxW, maxH);
        setContentOwned (content, false);

        Txt windowState;
        if (windowPosProperty.isNotEmpty())
            windowState = getGlobalProperties().getValue (windowPosProperty);

        if (windowState.isNotEmpty())
            restoreWindowStateFromString (windowState);
        else
            centreAroundComponent (Component::getCurrentlyFocusedComponent(), defaultW, defaultH);

        setVisible (true);
        owner.reset (this);
    }

    ~FloatingToolWindow() override
    {
        if (windowPosProperty.isNotEmpty())
            getGlobalProperties().setValue (windowPosProperty, getWindowStateAsString());
    }

    z0 closeButtonPressed() override
    {
        owner.reset();
    }

    b8 escapeKeyPressed() override
    {
        closeButtonPressed();
        return true;
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (secondaryBackgroundColorId));
    }

private:
    Txt windowPosProperty;
    std::unique_ptr<Component>& owner;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatingToolWindow)
};
