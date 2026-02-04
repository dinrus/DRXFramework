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

ComponentBoundsConstrainer::ComponentBoundsConstrainer() noexcept {}
ComponentBoundsConstrainer::~ComponentBoundsConstrainer() {}

//==============================================================================
z0 ComponentBoundsConstrainer::setMinimumWidth  (i32 minimumWidth) noexcept   { minW = minimumWidth; }
z0 ComponentBoundsConstrainer::setMaximumWidth  (i32 maximumWidth) noexcept   { maxW = maximumWidth; }
z0 ComponentBoundsConstrainer::setMinimumHeight (i32 minimumHeight) noexcept  { minH = minimumHeight; }
z0 ComponentBoundsConstrainer::setMaximumHeight (i32 maximumHeight) noexcept  { maxH = maximumHeight; }

z0 ComponentBoundsConstrainer::setMinimumSize (i32 minimumWidth, i32 minimumHeight) noexcept
{
    jassert (maxW >= minimumWidth);
    jassert (maxH >= minimumHeight);
    jassert (minimumWidth > 0 && minimumHeight > 0);

    minW = minimumWidth;
    minH = minimumHeight;

    if (minW > maxW)  maxW = minW;
    if (minH > maxH)  maxH = minH;
}

z0 ComponentBoundsConstrainer::setMaximumSize (i32 maximumWidth, i32 maximumHeight) noexcept
{
    jassert (maximumWidth >= minW);
    jassert (maximumHeight >= minH);
    jassert (maximumWidth > 0 && maximumHeight > 0);

    maxW = jmax (minW, maximumWidth);
    maxH = jmax (minH, maximumHeight);
}

z0 ComponentBoundsConstrainer::setSizeLimits (i32 minimumWidth,
                                                i32 minimumHeight,
                                                i32 maximumWidth,
                                                i32 maximumHeight) noexcept
{
    jassert (maximumWidth >= minimumWidth);
    jassert (maximumHeight >= minimumHeight);
    jassert (maximumWidth > 0 && maximumHeight > 0);
    jassert (minimumWidth > 0 && minimumHeight > 0);

    minW = jmax (0, minimumWidth);
    minH = jmax (0, minimumHeight);
    maxW = jmax (minW, maximumWidth);
    maxH = jmax (minH, maximumHeight);
}

z0 ComponentBoundsConstrainer::setMinimumOnscreenAmounts (i32 minimumWhenOffTheTop,
                                                            i32 minimumWhenOffTheLeft,
                                                            i32 minimumWhenOffTheBottom,
                                                            i32 minimumWhenOffTheRight) noexcept
{
    minOffTop    = minimumWhenOffTheTop;
    minOffLeft   = minimumWhenOffTheLeft;
    minOffBottom = minimumWhenOffTheBottom;
    minOffRight  = minimumWhenOffTheRight;
}

z0 ComponentBoundsConstrainer::setFixedAspectRatio (f64 widthOverHeight) noexcept
{
    aspectRatio = jmax (0.0, widthOverHeight);
}

f64 ComponentBoundsConstrainer::getFixedAspectRatio() const noexcept
{
    return aspectRatio;
}

z0 ComponentBoundsConstrainer::setBoundsForComponent (Component* component,
                                                        Rectangle<i32> targetBounds,
                                                        b8 isStretchingTop,
                                                        b8 isStretchingLeft,
                                                        b8 isStretchingBottom,
                                                        b8 isStretchingRight)
{
    jassert (component != nullptr);

    auto bounds = targetBounds;

    auto limits = [&]() -> Rectangle<i32>
    {
        if (auto* parent = component->getParentComponent())
            return { parent->getWidth(), parent->getHeight() };

        const auto globalBounds = component->localAreaToGlobal (targetBounds - component->getPosition());

        if (auto* display = Desktop::getInstance().getDisplays().getDisplayForPoint (globalBounds.getCentre()))
            return component->getLocalArea (nullptr, display->userArea) + component->getPosition();

        const auto max = std::numeric_limits<i32>::max();
        return { max, max };
    }();

    auto border = [&]() -> BorderSize<i32>
    {
        if (component->getParentComponent() == nullptr)
            if (auto* peer = component->getPeer())
                if (const auto frameSize = peer->getFrameSizeIfPresent())
                    return *frameSize;

        return {};
    }();

    border.addTo (bounds);

    checkBounds (bounds,
                 border.addedTo (component->getBounds()), limits,
                 isStretchingTop, isStretchingLeft,
                 isStretchingBottom, isStretchingRight);

    border.subtractFrom (bounds);

    applyBoundsToComponent (*component, bounds);
}

z0 ComponentBoundsConstrainer::checkComponentBounds (Component* component)
{
    setBoundsForComponent (component, component->getBounds(),
                           false, false, false, false);
}

z0 ComponentBoundsConstrainer::applyBoundsToComponent (Component& component, Rectangle<i32> bounds)
{
    if (auto* positioner = component.getPositioner())
        positioner->applyNewBounds (bounds);
    else
        component.setBounds (bounds);
}

//==============================================================================
z0 ComponentBoundsConstrainer::resizeStart()
{
}

z0 ComponentBoundsConstrainer::resizeEnd()
{
}

//==============================================================================
z0 ComponentBoundsConstrainer::checkBounds (Rectangle<i32>& bounds,
                                              const Rectangle<i32>& old,
                                              const Rectangle<i32>& limits,
                                              b8 isStretchingTop,
                                              b8 isStretchingLeft,
                                              b8 isStretchingBottom,
                                              b8 isStretchingRight)
{
    if (isStretchingLeft)
        bounds.setLeft (jlimit (old.getRight() - maxW, old.getRight() - minW, bounds.getX()));
    else
        bounds.setWidth (jlimit (minW, maxW, bounds.getWidth()));

    if (isStretchingTop)
        bounds.setTop (jlimit (old.getBottom() - maxH, old.getBottom() - minH, bounds.getY()));
    else
        bounds.setHeight (jlimit (minH, maxH, bounds.getHeight()));

    if (bounds.isEmpty())
        return;

    if (minOffTop > 0)
    {
        i32k limit = limits.getY() + jmin (minOffTop - bounds.getHeight(), 0);

        if (bounds.getY() < limit)
        {
            if (isStretchingTop)
                bounds.setTop (limits.getY());
            else
                bounds.setY (limit);
        }
    }

    if (minOffLeft > 0)
    {
        i32k limit = limits.getX() + jmin (minOffLeft - bounds.getWidth(), 0);

        if (bounds.getX() < limit)
        {
            if (isStretchingLeft)
                bounds.setLeft (limits.getX());
            else
                bounds.setX (limit);
        }
    }

    if (minOffBottom > 0)
    {
        i32k limit = limits.getBottom() - jmin (minOffBottom, bounds.getHeight());

        if (bounds.getY() > limit)
        {
            if (isStretchingBottom)
                bounds.setBottom (limits.getBottom());
            else
                bounds.setY (limit);
        }
    }

    if (minOffRight > 0)
    {
        i32k limit = limits.getRight() - jmin (minOffRight, bounds.getWidth());

        if (bounds.getX() > limit)
        {
            if (isStretchingRight)
                bounds.setRight (limits.getRight());
            else
                bounds.setX (limit);
        }
    }

    // constrain the aspect ratio if one has been specified..
    if (aspectRatio > 0.0)
    {
        b8 adjustWidth;

        if ((isStretchingTop || isStretchingBottom) && ! (isStretchingLeft || isStretchingRight))
        {
            adjustWidth = true;
        }
        else if ((isStretchingLeft || isStretchingRight) && ! (isStretchingTop || isStretchingBottom))
        {
            adjustWidth = false;
        }
        else
        {
            const f64 oldRatio = (old.getHeight() > 0) ? std::abs (old.getWidth() / (f64) old.getHeight()) : 0.0;
            const f64 newRatio = std::abs (bounds.getWidth() / (f64) bounds.getHeight());

            adjustWidth = (oldRatio > newRatio);
        }

        if (adjustWidth)
        {
            bounds.setWidth (roundToInt (bounds.getHeight() * aspectRatio));

            if (bounds.getWidth() > maxW || bounds.getWidth() < minW)
            {
                bounds.setWidth (jlimit (minW, maxW, bounds.getWidth()));
                bounds.setHeight (roundToInt (bounds.getWidth() / aspectRatio));
            }
        }
        else
        {
            bounds.setHeight (roundToInt (bounds.getWidth() / aspectRatio));

            if (bounds.getHeight() > maxH || bounds.getHeight() < minH)
            {
                bounds.setHeight (jlimit (minH, maxH, bounds.getHeight()));
                bounds.setWidth (roundToInt (bounds.getHeight() * aspectRatio));
            }
        }

        if ((isStretchingTop || isStretchingBottom) && ! (isStretchingLeft || isStretchingRight))
        {
            bounds.setX (old.getX() + (old.getWidth() - bounds.getWidth()) / 2);
        }
        else if ((isStretchingLeft || isStretchingRight) && ! (isStretchingTop || isStretchingBottom))
        {
            bounds.setY (old.getY() + (old.getHeight() - bounds.getHeight()) / 2);
        }
        else
        {
            if (isStretchingLeft)
                bounds.setX (old.getRight() - bounds.getWidth());

            if (isStretchingTop)
                bounds.setY (old.getBottom() - bounds.getHeight());
        }
    }

    jassert (! bounds.isEmpty());
}

} // namespace drx
