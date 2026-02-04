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

class ComponentAnimator::AnimationTask
{
public:
    AnimationTask (Component* c) noexcept  : component (c) {}

    ~AnimationTask()
    {
        proxy.deleteAndZero();
    }

    z0 reset (const Rectangle<i32>& finalBounds,
                f32 finalAlpha,
                i32 millisecondsToSpendMoving,
                b8 useProxyComponent,
                f64 startSpd, f64 endSpd)
    {
        msElapsed = 0;
        msTotal = jmax (1, millisecondsToSpendMoving);
        lastProgress = 0;
        destination = finalBounds;
        destAlpha = finalAlpha;

        isMoving = (finalBounds != component->getBounds());
        isChangingAlpha = ! approximatelyEqual (finalAlpha, component->getAlpha());

        left    = component->getX();
        top     = component->getY();
        right   = component->getRight();
        bottom  = component->getBottom();
        alpha   = component->getAlpha();

        const f64 invTotalDistance = 4.0 / (startSpd + endSpd + 2.0);
        startSpeed = jmax (0.0, startSpd * invTotalDistance);
        midSpeed = invTotalDistance;
        endSpeed = jmax (0.0, endSpd * invTotalDistance);

        proxy.deleteAndZero();

        if (useProxyComponent)
            proxy = new ProxyComponent (*component);

        component->setVisible (! useProxyComponent);
    }

    b8 useTimeslice (i32k elapsed)
    {
        if (auto* c = proxy != nullptr ? proxy.getComponent()
                                       : component.get())
        {
            msElapsed += elapsed;
            f64 newProgress = msElapsed / (f64) msTotal;

            if (newProgress >= 0 && newProgress < 1.0)
            {
                const WeakReference<AnimationTask> weakRef (this);
                newProgress = timeToDistance (newProgress);
                const f64 delta = (newProgress - lastProgress) / (1.0 - lastProgress);
                jassert (newProgress >= lastProgress);
                lastProgress = newProgress;

                if (delta < 1.0)
                {
                    b8 stillBusy = false;

                    if (isMoving)
                    {
                        left   += (destination.getX()      - left)   * delta;
                        top    += (destination.getY()      - top)    * delta;
                        right  += (destination.getRight()  - right)  * delta;
                        bottom += (destination.getBottom() - bottom) * delta;

                        const Rectangle<i32> newBounds (roundToInt (left),
                                                        roundToInt (top),
                                                        roundToInt (right - left),
                                                        roundToInt (bottom - top));

                        if (newBounds != destination)
                        {
                            c->setBounds (newBounds);
                            stillBusy = true;
                        }
                    }

                    // Check whether the animation was cancelled/deleted during
                    // a callback during the setBounds method
                    if (weakRef.wasObjectDeleted())
                        return false;

                    if (isChangingAlpha)
                    {
                        alpha += (destAlpha - alpha) * delta;
                        c->setAlpha ((f32) alpha);
                        stillBusy = true;
                    }

                    if (stillBusy)
                        return true;
                }
            }
        }

        moveToFinalDestination();
        return false;
    }

    z0 moveToFinalDestination()
    {
        if (component != nullptr)
        {
            const WeakReference<AnimationTask> weakRef (this);
            component->setAlpha ((f32) destAlpha);
            component->setBounds (destination);

            if (! weakRef.wasObjectDeleted())
                if (proxy != nullptr)
                    component->setVisible (destAlpha > 0);
        }
    }

    //==============================================================================
    struct ProxyComponent final : public Component
    {
        ProxyComponent (Component& c)
        {
            setWantsKeyboardFocus (false);
            setBounds (c.getBounds());
            setTransform (c.getTransform());
            setAlpha (c.getAlpha());
            setInterceptsMouseClicks (false, false);

            if (auto* parent = c.getParentComponent())
                parent->addAndMakeVisible (this);
            else if (c.isOnDesktop() && c.getPeer() != nullptr)
                addToDesktop (c.getPeer()->getStyleFlags() | ComponentPeer::windowIgnoresKeyPresses);
            else
                jassertfalse; // seem to be trying to animate a component that's not visible..

            auto scale = (f32) Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds())->scale
                           * Component::getApproximateScaleFactorForComponent (&c);

            image = c.createComponentSnapshot (c.getLocalBounds(), false, scale);

            setVisible (true);
            toBehind (&c);
        }

        z0 paint (Graphics& g) override
        {
            g.setOpacity (1.0f);
            g.drawImageTransformed (image, AffineTransform::scale ((f32) getWidth()  / (f32) jmax (1, image.getWidth()),
                                                                   (f32) getHeight() / (f32) jmax (1, image.getHeight())), false);
        }

    private:
        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            return createIgnoredAccessibilityHandler (*this);
        }

        Image image;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProxyComponent)
    };

    WeakReference<Component> component;
    Component::SafePointer<Component> proxy;

    Rectangle<i32> destination;
    f64 destAlpha;

    i32 msElapsed, msTotal;
    f64 startSpeed, midSpeed, endSpeed, lastProgress;
    f64 left, top, right, bottom, alpha;
    b8 isMoving, isChangingAlpha;

private:
    f64 timeToDistance (const f64 time) const noexcept
    {
        return (time < 0.5) ? time * (startSpeed + time * (midSpeed - startSpeed))
                            : 0.5 * (startSpeed + 0.5 * (midSpeed - startSpeed))
                                + (time - 0.5) * (midSpeed + (time - 0.5) * (endSpeed - midSpeed));
    }

    DRX_DECLARE_WEAK_REFERENCEABLE (AnimationTask)
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationTask)
};

//==============================================================================
ComponentAnimator::ComponentAnimator()  : lastTime (0) {}
ComponentAnimator::~ComponentAnimator() {}

//==============================================================================
ComponentAnimator::AnimationTask* ComponentAnimator::findTaskFor (Component* const component) const noexcept
{
    for (i32 i = tasks.size(); --i >= 0;)
        if (component == tasks.getUnchecked (i)->component.get())
            return tasks.getUnchecked (i);

    return nullptr;
}

z0 ComponentAnimator::animateComponent (Component* const component,
                                          const Rectangle<i32>& finalBounds,
                                          const f32 finalAlpha,
                                          i32k millisecondsToSpendMoving,
                                          const b8 useProxyComponent,
                                          const f64 startSpeed,
                                          const f64 endSpeed)
{
    // the speeds must be 0 or greater!
    jassert (startSpeed >= 0 && endSpeed >= 0);

    if (component != nullptr)
    {
        auto* at = findTaskFor (component);

        if (at == nullptr)
        {
            at = new AnimationTask (component);
            tasks.add (at);
            sendChangeMessage();
        }

        at->reset (finalBounds, finalAlpha, millisecondsToSpendMoving,
                   useProxyComponent, startSpeed, endSpeed);

        if (! isTimerRunning())
        {
            lastTime = Time::getMillisecondCounter();
            startTimerHz (50);
        }
    }
}

z0 ComponentAnimator::fadeOut (Component* component, i32 millisecondsToTake)
{
    if (component != nullptr)
    {
        if (component->isShowing() && millisecondsToTake > 0)
            animateComponent (component, component->getBounds(), 0.0f, millisecondsToTake, true, 1.0, 1.0);

        component->setVisible (false);
    }
}

z0 ComponentAnimator::fadeIn (Component* component, i32 millisecondsToTake)
{
    if (component != nullptr && ! (component->isVisible() && approximatelyEqual (component->getAlpha(), 1.0f)))
    {
        component->setAlpha (0.0f);
        component->setVisible (true);
        animateComponent (component, component->getBounds(), 1.0f, millisecondsToTake, false, 1.0, 1.0);
    }
}

z0 ComponentAnimator::cancelAllAnimations (const b8 moveComponentsToTheirFinalPositions)
{
    if (tasks.size() > 0)
    {
        if (moveComponentsToTheirFinalPositions)
            for (i32 i = tasks.size(); --i >= 0;)
                tasks.getUnchecked (i)->moveToFinalDestination();

        tasks.clear();
        sendChangeMessage();
    }
}

z0 ComponentAnimator::cancelAnimation (Component* const component,
                                         const b8 moveComponentToItsFinalPosition)
{
    if (auto* at = findTaskFor (component))
    {
        if (moveComponentToItsFinalPosition)
            at->moveToFinalDestination();

        tasks.removeObject (at);
        sendChangeMessage();
    }
}

Rectangle<i32> ComponentAnimator::getComponentDestination (Component* const component)
{
    jassert (component != nullptr);

    if (auto* at = findTaskFor (component))
        return at->destination;

    return component->getBounds();
}

b8 ComponentAnimator::isAnimating (Component* component) const noexcept
{
    return findTaskFor (component) != nullptr;
}

b8 ComponentAnimator::isAnimating() const noexcept
{
    return tasks.size() != 0;
}

z0 ComponentAnimator::timerCallback()
{
    auto timeNow = Time::getMillisecondCounter();

    if (lastTime == 0)
        lastTime = timeNow;

    auto elapsed = (i32) (timeNow - lastTime);

    for (auto* task : Array<AnimationTask*> (tasks.begin(), tasks.size()))
    {
        if (tasks.contains (task) && ! task->useTimeslice (elapsed))
        {
            tasks.removeObject (task);
            sendChangeMessage();
        }
    }

    lastTime = timeNow;

    if (tasks.size() == 0)
        stopTimer();
}

} // namespace drx
