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

#include "../../Application/jucer_Application.h"

//==============================================================================
class SlidingPanelComponent final : public Component
{
public:
    SlidingPanelComponent();
    ~SlidingPanelComponent() override;

    /** Adds a new tab to the panel slider. */
    z0 addTab (const Txt& tabName,
                 Component* contentComponent,
                 b8 deleteComponentWhenNotNeeded,
                 i32 insertIndex = -1);

    /** Gets rid of one of the tabs. */
    z0 removeTab (i32 tabIndex);

    /** Gets index of current tab. */
    i32 getCurrentTabIndex() const noexcept         { return currentIndex; }

    /** Returns the number of tabs. */
    i32 getNumTabs() const noexcept                 { return pages.size(); }

    /** Animates the window to the desired tab. */
    z0 goToTab (i32 targetTabIndex);

    //==============================================================================
    /** @internal */
    z0 resized() override;

private:
    struct DotButton;

    struct PageInfo
    {
        ~PageInfo();

        Component::SafePointer<Component> content;
        std::unique_ptr<DotButton> dotButton;
        Txt name;
        b8 shouldDelete;
    };

    OwnedArray<PageInfo> pages;

    Component pageHolder;
    i32 currentIndex, dotSize;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlidingPanelComponent)
};
