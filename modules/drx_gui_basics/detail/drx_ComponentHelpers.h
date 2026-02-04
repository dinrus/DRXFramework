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

namespace drx::detail
{

constexpr t8 colourPropertyPrefix[] = "jcclr_";

//==============================================================================
struct ComponentHelpers
{
    using SH = ScalingHelpers;

   #if DRX_MODAL_LOOPS_PERMITTED
    static uk runModalLoopCallback (uk userData)
    {
        return (uk) (pointer_sized_int) static_cast<Component*> (userData)->runModalLoop();
    }
   #endif

    static Identifier getColorPropertyID (i32 colourID)
    {
        t8 buffer[32];
        auto* end = buffer + numElementsInArray (buffer) - 1;
        auto* t = end;
        *t = 0;

        for (auto v = (u32) colourID;;)
        {
            *--t = "0123456789abcdef" [v & 15];
            v >>= 4;

            if (v == 0)
                break;
        }

        for (i32 i = (i32) sizeof (colourPropertyPrefix) - 1; --i >= 0;)
            *--t = colourPropertyPrefix[i];

        return t;
    }

    //==============================================================================
    static b8 hitTest (Component& comp, Point<f32> localPoint)
    {
        const auto intPoint = localPoint.roundToInt();
        return Rectangle<i32> { comp.getWidth(), comp.getHeight() }.contains (intPoint)
            && comp.hitTest (intPoint.x, intPoint.y);
    }

    // converts an unscaled position within a peer to the local position within that peer's component
    template <typename PointOrRect>
    static PointOrRect rawPeerPositionToLocal (const Component& comp, PointOrRect pos) noexcept
    {
        if (comp.isTransformed())
            pos = pos.transformedBy (comp.getTransform().inverted());

        return SH::unscaledScreenPosToScaled (comp, pos);
    }

    // converts a position within a peer's component to the unscaled position within the peer
    template <typename PointOrRect>
    static PointOrRect localPositionToRawPeerPos (const Component& comp, PointOrRect pos) noexcept
    {
        if (comp.isTransformed())
            pos = pos.transformedBy (comp.getTransform());

        return SH::scaledScreenPosToUnscaled (comp, pos);
    }

    template <typename PointOrRect>
    static PointOrRect convertFromParentSpace (const Component& comp, const PointOrRect pointInParentSpace)
    {
        const auto transformed = comp.affineTransform != nullptr ? pointInParentSpace.transformedBy (comp.affineTransform->inverted())
                                                                 : pointInParentSpace;

        if (comp.isOnDesktop())
        {
            if (auto* peer = comp.getPeer())
                return SH::unscaledScreenPosToScaled (comp, peer->globalToLocal (SH::scaledScreenPosToUnscaled (transformed)));

            jassertfalse;
            return transformed;
        }

        if (comp.getParentComponent() == nullptr)
            return SH::subtractPosition (SH::unscaledScreenPosToScaled (comp, SH::scaledScreenPosToUnscaled (transformed)), comp);

        return SH::subtractPosition (transformed, comp);
    }

    template <typename PointOrRect>
    static PointOrRect convertToParentSpace (const Component& comp, const PointOrRect pointInLocalSpace)
    {
        const auto preTransform = [&]
        {
            if (comp.isOnDesktop())
            {
                if (auto* peer = comp.getPeer())
                    return SH::unscaledScreenPosToScaled (peer->localToGlobal (SH::scaledScreenPosToUnscaled (comp, pointInLocalSpace)));

                jassertfalse;
                return pointInLocalSpace;
            }

            if (comp.getParentComponent() == nullptr)
                return SH::unscaledScreenPosToScaled (SH::scaledScreenPosToUnscaled (comp, SH::addPosition (pointInLocalSpace, comp)));

            return SH::addPosition (pointInLocalSpace, comp);
        }();

        return comp.affineTransform != nullptr ? preTransform.transformedBy (*comp.affineTransform)
                                               : preTransform;
    }

    template <typename PointOrRect>
    static PointOrRect convertFromDistantParentSpace (const Component* parent, const Component& target, PointOrRect coordInParent)
    {
        auto* directParent = target.getParentComponent();
        jassert (directParent != nullptr);

        if (directParent == parent)
            return convertFromParentSpace (target, coordInParent);

        DRX_BEGIN_IGNORE_WARNINGS_MSVC (6011)
        return convertFromParentSpace (target, convertFromDistantParentSpace (parent, *directParent, coordInParent));
        DRX_END_IGNORE_WARNINGS_MSVC
    }

    template <typename PointOrRect>
    static PointOrRect convertCoordinate (const Component* target, const Component* source, PointOrRect p)
    {
        while (source != nullptr)
        {
            if (source == target)
                return p;

            DRX_BEGIN_IGNORE_WARNINGS_MSVC (6011)

            if (source->isParentOf (target))
                return convertFromDistantParentSpace (source, *target, p);

            DRX_END_IGNORE_WARNINGS_MSVC

            p = convertToParentSpace (*source, p);
            source = source->getParentComponent();
        }

        jassert (source == nullptr);
        if (target == nullptr)
            return p;

        auto* topLevelComp = target->getTopLevelComponent();

        p = convertFromParentSpace (*topLevelComp, p);

        if (topLevelComp == target)
            return p;

        return convertFromDistantParentSpace (topLevelComp, *target, p);
    }

    static b8 clipChildComponent (const Component& child,
                                    Graphics& g,
                                    const Rectangle<i32> clipRect,
                                    Point<i32> delta)
    {
        if (! child.isVisible() || child.isTransformed())
            return false;

        const auto newClip = clipRect.getIntersection (child.boundsRelativeToParent);

        if (newClip.isEmpty())
            return false;

        if (child.isOpaque() && child.componentTransparency == 0)
        {
            g.excludeClipRegion (newClip + delta);
            return true;
        }

        const auto childPos = child.getPosition();
        return clipObscuredRegions (child, g, newClip - childPos, childPos + delta);
    }

    static b8 clipObscuredRegions (const Component& comp,
                                     Graphics& g,
                                     const Rectangle<i32> clipRect,
                                     Point<i32> delta)
    {
        auto wasClipped = false;

        for (i32 i = comp.childComponentList.size(); --i >= 0;)
            wasClipped |= clipChildComponent (*comp.childComponentList.getUnchecked (i), g, clipRect, delta);

        return wasClipped;
    }

    static Rectangle<i32> getParentOrMainMonitorBounds (const Component& comp)
    {
        if (auto* p = comp.getParentComponent())
            return p->getLocalBounds();

        return Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
    }

    static z0 releaseAllCachedImageResources (Component& c)
    {
        if (auto* cached = c.getCachedComponentImage())
            cached->releaseResources();

        for (auto* child : c.childComponentList)
            releaseAllCachedImageResources (*child);
    }

    //==============================================================================
    static b8 modalWouldBlockComponent (const Component& maybeBlocked, Component* modal)
    {
        return modal != nullptr
            && modal != &maybeBlocked
            && ! modal->isParentOf (&maybeBlocked)
            && ! modal->canModalEventBeSentToComponent (&maybeBlocked);
    }

    template <typename Function>
    static z0 sendMouseEventToComponentsThatAreBlockedByModal (Component& modal, Function&& function)
    {
        for (auto& ms : Desktop::getInstance().getMouseSources())
            if (auto* c = ms.getComponentUnderMouse())
                if (modalWouldBlockComponent (*c, &modal))
                    function (c, ms, SH::screenPosToLocalPos (*c, ms.getScreenPosition()), Time::getCurrentTime());
    }

    class ModalComponentManagerChangeNotifier
    {
    public:
        static auto& getInstance()
        {
            static ModalComponentManagerChangeNotifier instance;
            return instance;
        }

        ErasedScopeGuard addListener (std::function<z0()> l)
        {
            return listeners.addListener (std::move (l));
        }

        z0 modalComponentManagerChanged()
        {
            listeners.call();
        }

    private:
        ModalComponentManagerChangeNotifier() = default;

        detail::CallbackListenerList<> listeners;
    };
};

} // namespace drx::detail
