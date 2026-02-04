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

#if DRX_MAC

#include <drx_audio_plugin_client/detail/drx_IncludeModuleHeaders.h>

namespace drx::detail
{

struct VSTWindowUtilities
{
    VSTWindowUtilities() = delete;

    static uk attachComponentToWindowRefVST (Component* comp,
                                                i32 desktopFlags,
                                                uk parentWindowOrView)
    {
        DRX_AUTORELEASEPOOL
        {
            NSView* parentView = [(NSView*) parentWindowOrView retain];

            const auto defaultFlags = DrxPlugin_EditorRequiresKeyboardFocus
                                    ? 0
                                    : ComponentPeer::windowIgnoresKeyPresses;
            comp->addToDesktop (desktopFlags | defaultFlags, parentView);

            // (this workaround is because Wavelab provides a zero-size parent view..)
            if (approximatelyEqual ([parentView frame].size.height, 0.0))
                [((NSView*) comp->getWindowHandle()) setFrameOrigin: NSZeroPoint];

            comp->setVisible (true);
            comp->toFront (false);

            [[parentView window] setAcceptsMouseMovedEvents: YES];
            return parentView;
        }
    }

    static z0 detachComponentFromWindowRefVST (Component* comp,
                                                uk window)
    {
        DRX_AUTORELEASEPOOL
        {
            comp->removeFromDesktop();
            [(id) window release];
        }
    }

    static z0 setNativeHostWindowSizeVST (uk window,
                                            Component* component,
                                            i32 newWidth,
                                            i32 newHeight)
    {
        DRX_AUTORELEASEPOOL
        {
            if (NSView* hostView = (NSView*) window)
            {
                i32k dx = newWidth  - component->getWidth();
                i32k dy = newHeight - component->getHeight();

                NSRect r = [hostView frame];
                r.size.width += dx;
                r.size.height += dy;
                r.origin.y -= dy;
                [hostView setFrame: r];
            }
        }
    }
};

} // namespace drx::detail

#endif
