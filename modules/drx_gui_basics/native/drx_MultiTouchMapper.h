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

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

namespace drx
{

template <typename IDType>
class MultiTouchMapper
{
public:
    MultiTouchMapper() {}

    i32 getIndexOfTouch (ComponentPeer* peer, IDType touchID)
    {
        jassert (touchID != 0); // need to rethink this if IDs can be 0!
        TouchInfo info {touchID, peer};

        i32 touchIndex = currentTouches.indexOf (info);

        if (touchIndex < 0)
        {
            auto emptyTouchIndex = currentTouches.indexOf ({});
            touchIndex = (emptyTouchIndex >= 0 ? emptyTouchIndex : currentTouches.size());

            currentTouches.set (touchIndex, info);
        }

        return touchIndex;
    }

    z0 clear()
    {
        currentTouches.clear();
    }

    z0 clearTouch (i32 index)
    {
        currentTouches.set (index, {});
    }

    b8 areAnyTouchesActive() const noexcept
    {
        for (auto& t : currentTouches)
            if (t.touchId != 0)
                return true;

        return false;
    }

    z0 deleteAllTouchesForPeer (ComponentPeer* peer)
    {
        for (auto& t : currentTouches)
            if (t.owner == peer)
                t.touchId = 0;
    }

private:
    //==============================================================================
    struct TouchInfo
    {
        TouchInfo() noexcept  : touchId (0), owner (nullptr) {}
        TouchInfo (IDType idToUse, ComponentPeer* peer) noexcept  : touchId (idToUse), owner (peer) {}

        TouchInfo (const TouchInfo&) = default;
        TouchInfo& operator= (const TouchInfo&) = default;
        TouchInfo (TouchInfo&&) noexcept = default;
        TouchInfo& operator= (TouchInfo&&) noexcept = default;

        IDType touchId;
        ComponentPeer* owner;

        b8 operator== (const TouchInfo& o) const noexcept     { return (touchId == o.touchId); }
    };

    //==============================================================================
    Array<TouchInfo> currentTouches;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTouchMapper)
};

} // namespace drx

DRX_END_IGNORE_WARNINGS_GCC_LIKE
