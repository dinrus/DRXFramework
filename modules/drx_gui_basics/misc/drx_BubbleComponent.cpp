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

BubbleComponent::BubbleComponent()
  : allowablePlacements (above | below | left | right)
{
    setInterceptsMouseClicks (false, false);
    lookAndFeelChanged();
}

BubbleComponent::~BubbleComponent() {}

//==============================================================================
z0 BubbleComponent::lookAndFeelChanged()
{
    getLookAndFeel().setComponentEffectForBubbleComponent (*this);
}

z0 BubbleComponent::paint (Graphics& g)
{
    getLookAndFeel().drawBubble (g, *this, arrowTip.toFloat(), content.toFloat());

    g.reduceClipRegion (content);
    g.setOrigin (content.getPosition());

    paintContent (g, content.getWidth(), content.getHeight());
}

z0 BubbleComponent::setAllowedPlacement (i32k newPlacement)
{
    allowablePlacements = newPlacement;
}

//==============================================================================
z0 BubbleComponent::setPosition (Component* componentToPointTo, i32 distanceFromTarget, i32 arrowLength)
{
    jassert (componentToPointTo != nullptr);

    Rectangle<i32> target;

    if (Component* p = getParentComponent())
        target = p->getLocalArea (componentToPointTo, componentToPointTo->getLocalBounds());
    else
        target = componentToPointTo->getScreenBounds().transformedBy (getTransform().inverted());

    setPosition (target, distanceFromTarget, arrowLength);
}

z0 BubbleComponent::setPosition (Point<i32> arrowTipPos, i32 arrowLength)
{
    setPosition (Rectangle<i32> (arrowTipPos.x, arrowTipPos.y, 1, 1), arrowLength, arrowLength);
}

z0 BubbleComponent::setPosition (Rectangle<i32> rectangleToPointTo,
                                   i32 distanceFromTarget, i32 arrowLength)
{
    {
        i32 contentW = 150, contentH = 30;
        getContentSize (contentW, contentH);
        content.setBounds (distanceFromTarget, distanceFromTarget, contentW, contentH);
    }

    i32k totalW = content.getWidth()  + distanceFromTarget * 2;
    i32k totalH = content.getHeight() + distanceFromTarget * 2;

    auto availableSpace = (getParentComponent() != nullptr ? getParentComponent()->getLocalBounds()
                                                           : getParentMonitorArea().transformedBy (getTransform().inverted()));

    i32 spaceAbove = ((allowablePlacements & above) != 0) ? jmax (0, rectangleToPointTo.getY()  - availableSpace.getY()) : -1;
    i32 spaceBelow = ((allowablePlacements & below) != 0) ? jmax (0, availableSpace.getBottom() - rectangleToPointTo.getBottom()) : -1;
    i32 spaceLeft  = ((allowablePlacements & left)  != 0) ? jmax (0, rectangleToPointTo.getX()  - availableSpace.getX()) : -1;
    i32 spaceRight = ((allowablePlacements & right) != 0) ? jmax (0, availableSpace.getRight()  - rectangleToPointTo.getRight()) : -1;

    // look at whether the component is elongated, and if so, try to position next to its longer dimension.
    if (rectangleToPointTo.getWidth() > rectangleToPointTo.getHeight() * 2
         && (spaceAbove > totalH + 20 || spaceBelow > totalH + 20))
    {
        spaceLeft = spaceRight = 0;
    }
    else if (rectangleToPointTo.getWidth() < rectangleToPointTo.getHeight() / 2
              && (spaceLeft > totalW + 20 || spaceRight > totalW + 20))
    {
        spaceAbove = spaceBelow = 0;
    }

    i32 targetX, targetY;

    if (jmax (spaceAbove, spaceBelow) >= jmax (spaceLeft, spaceRight))
    {
        targetX = rectangleToPointTo.getCentre().x;
        arrowTip.x = totalW / 2;

        if (spaceAbove >= spaceBelow)
        {
            // above
            targetY = rectangleToPointTo.getY();
            arrowTip.y = content.getBottom() + arrowLength;
        }
        else
        {
            // below
            targetY = rectangleToPointTo.getBottom();
            arrowTip.y = content.getY() - arrowLength;
        }
    }
    else
    {
        targetY = rectangleToPointTo.getCentre().y;
        arrowTip.y = totalH / 2;

        if (spaceLeft > spaceRight)
        {
            // on the left
            targetX = rectangleToPointTo.getX();
            arrowTip.x = content.getRight() + arrowLength;
        }
        else
        {
            // on the right
            targetX = rectangleToPointTo.getRight();
            arrowTip.x = content.getX() - arrowLength;
        }
    }

    setBounds (targetX - arrowTip.x, targetY - arrowTip.y, totalW, totalH);
}

} // namespace drx
