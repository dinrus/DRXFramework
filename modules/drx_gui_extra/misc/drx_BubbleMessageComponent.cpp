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

BubbleMessageComponent::BubbleMessageComponent (i32 fadeOutLengthMs)
    : fadeOutLength (fadeOutLengthMs), mouseClickCounter (0),
      expiryTime (0), deleteAfterUse (false)
{
}

BubbleMessageComponent::~BubbleMessageComponent()
{
}

z0 BubbleMessageComponent::showAt (const Rectangle<i32>& pos,
                                     const AttributedString& text,
                                     i32k numMillisecondsBeforeRemoving,
                                     const b8 removeWhenMouseClicked,
                                     const b8 deleteSelfAfterUse)
{
    createLayout (text);
    setPosition (pos);
    init (numMillisecondsBeforeRemoving, removeWhenMouseClicked, deleteSelfAfterUse);
}

z0 BubbleMessageComponent::showAt (Component* const component,
                                     const AttributedString& text,
                                     i32k numMillisecondsBeforeRemoving,
                                     const b8 removeWhenMouseClicked,
                                     const b8 deleteSelfAfterUse)
{
    createLayout (text);
    setPosition (component);
    init (numMillisecondsBeforeRemoving, removeWhenMouseClicked, deleteSelfAfterUse);
}

z0 BubbleMessageComponent::createLayout (const AttributedString& text)
{
    textLayout.createLayoutWithBalancedLineLengths (text, 256);
}

z0 BubbleMessageComponent::init (i32k numMillisecondsBeforeRemoving,
                                   const b8 removeWhenMouseClicked,
                                   const b8 deleteSelfAfterUse)
{
    setAlpha (1.0f);
    setVisible (true);
    deleteAfterUse = deleteSelfAfterUse;

    expiryTime = numMillisecondsBeforeRemoving > 0
                    ? (Time::getMillisecondCounter() + (u32) numMillisecondsBeforeRemoving) : 0;

    mouseClickCounter = Desktop::getInstance().getMouseButtonClickCounter();

    if (! (removeWhenMouseClicked && isShowing()))
        mouseClickCounter += 0xfffff;

    startTimer (77);
    repaint();
}

const f32 bubblePaddingX = 20.0f;
const f32 bubblePaddingY = 14.0f;

z0 BubbleMessageComponent::getContentSize (i32& w, i32& h)
{
    w = (i32) (bubblePaddingX + textLayout.getWidth());
    h = (i32) (bubblePaddingY + textLayout.getHeight());
}

z0 BubbleMessageComponent::paintContent (Graphics& g, i32 w, i32 h)
{
    g.setColor (findColor (TooltipWindow::textColorId));

    textLayout.draw (g, Rectangle<f32> (bubblePaddingX / 2.0f, bubblePaddingY / 2.0f,
                                          (f32) w - bubblePaddingX, (f32) h - bubblePaddingY));
}

z0 BubbleMessageComponent::timerCallback()
{
    if (Desktop::getInstance().getMouseButtonClickCounter() > mouseClickCounter)
        hide (false);
    else if (expiryTime != 0 && Time::getMillisecondCounter() > expiryTime)
        hide (true);
}

z0 BubbleMessageComponent::hide (const b8 fadeOut)
{
    stopTimer();

    if (fadeOut)
        Desktop::getInstance().getAnimator().fadeOut (this, fadeOutLength);
    else
        setVisible (false);

    if (deleteAfterUse)
        delete this;
}

} // namespace drx
