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

class DropShadower::ShadowWindow final : public Component
{
public:
    ShadowWindow (Component* comp, const DropShadow& ds)
        : target (comp), shadow (ds)
    {
        setVisible (true);
        setAccessible (false);
        setInterceptsMouseClicks (false, false);

        if (comp->isOnDesktop())
        {
           #if DRX_WINDOWS
            const auto scope = [&]() -> std::optional<ScopedThreadDPIAwarenessSetter>
            {
                if (comp != nullptr)
                    if (auto* handle = comp->getWindowHandle())
                        return ScopedThreadDPIAwarenessSetter (handle);

                return {};
            }();
           #endif

            setSize (1, 1); // to keep the OS happy by not having zero-size windows
            addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                            | ComponentPeer::windowIsTemporary
                            | ComponentPeer::windowIgnoresKeyPresses);
        }
        else if (Component* const parent = comp->getParentComponent())
        {
            parent->addChildComponent (this);
        }
    }

    z0 paint (Graphics& g) override
    {
        if (Component* c = target)
            shadow.drawForRectangle (g, getLocalArea (c, c->getLocalBounds()));
    }

    z0 resized() override
    {
        repaint();  // (needed for correct repainting)
    }

    f32 getDesktopScaleFactor() const override
    {
        if (target != nullptr)
            return target->getDesktopScaleFactor();

        return Component::getDesktopScaleFactor();
    }

private:
    WeakReference<Component> target;
    DropShadow shadow;

    DRX_DECLARE_NON_COPYABLE (ShadowWindow)
};

class DropShadower::VirtualDesktopWatcher final  : public ComponentListener,
                                                   private Timer
{
public:
    //==============================================================================
    VirtualDesktopWatcher (Component& c) : component (&c)
    {
        component->addComponentListener (this);
        update();
    }

    ~VirtualDesktopWatcher() override
    {
        stopTimer();

        if (auto* c = component.get())
            c->removeComponentListener (this);
    }

    b8 shouldHideDropShadow() const
    {
        return hasReasonToHide;
    }

    z0 addListener (uk listener, std::function<z0()> cb)
    {
        listeners[listener] = std::move (cb);
    }

    z0 removeListener (uk listener)
    {
        listeners.erase (listener);
    }

    //==============================================================================
    z0 componentParentHierarchyChanged (Component& c) override
    {
        if (component.get() == &c)
            update();
    }

private:
    //==============================================================================
    z0 update()
    {
        b8 newHasReasonToHide = false;

        if (! component.wasObjectDeleted() && isWindows && component->isOnDesktop())
        {
            startTimerHz (5);

            WeakReference<VirtualDesktopWatcher> weakThis (this);

            // During scaling changes this call can trigger a call to HWNDComponentPeer::handleDPIChanging()
            // which deletes this VirtualDesktopWatcher.
            newHasReasonToHide = ! detail::WindowingHelpers::isWindowOnCurrentVirtualDesktop (component->getWindowHandle());

            if (weakThis == nullptr)
                return;
        }
        else
        {
            stopTimer();
        }

        if (std::exchange (hasReasonToHide, newHasReasonToHide) != newHasReasonToHide)
            for (auto& l : listeners)
                l.second();
    }

    z0 timerCallback() override
    {
        update();
    }

    //==============================================================================
    WeakReference<Component> component;
    const b8 isWindows = (SystemStats::getOperatingSystemType() & SystemStats::Windows) != 0;
    b8 hasReasonToHide = false;
    std::map<uk, std::function<z0()>> listeners;

    DRX_DECLARE_WEAK_REFERENCEABLE (VirtualDesktopWatcher)
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VirtualDesktopWatcher)
};

class DropShadower::ParentVisibilityChangedListener final : public ComponentListener
{
public:
    ParentVisibilityChangedListener (Component& r, ComponentListener& l)
        : root (&r), listener (&l)
    {
        updateParentHierarchy();
    }

    ~ParentVisibilityChangedListener() override
    {
        for (auto& compEntry : observedComponents)
            if (auto* comp = compEntry.get())
                comp->removeComponentListener (this);
    }

    z0 componentVisibilityChanged (Component& component) override
    {
        if (root != &component)
            listener->componentVisibilityChanged (*root);
    }

    z0 componentParentHierarchyChanged (Component& component) override
    {
        if (root == &component)
            updateParentHierarchy();
    }

private:
    class ComponentWithWeakReference
    {
    public:
        explicit ComponentWithWeakReference (Component& c)
            : ptr (&c), ref (&c) {}

        Component* get() const { return ref.get(); }

        b8 operator< (const ComponentWithWeakReference& other) const { return ptr < other.ptr; }

    private:
        Component* ptr;
        WeakReference<Component> ref;
    };

    z0 updateParentHierarchy()
    {
        const auto lastSeenComponents = std::exchange (observedComponents, [&]
        {
            std::set<ComponentWithWeakReference> result;

            for (auto node = root; node != nullptr; node = node->getParentComponent())
                result.emplace (*node);

            return result;
        }());

        const auto withDifference = [] (const auto& rangeA, const auto& rangeB, auto&& callback)
        {
            std::vector<ComponentWithWeakReference> result;
            std::set_difference (rangeA.begin(), rangeA.end(), rangeB.begin(), rangeB.end(), std::back_inserter (result));

            for (const auto& item : result)
                if (auto* c = item.get())
                    callback (*c);
        };

        withDifference (lastSeenComponents, observedComponents, [this] (auto& comp) { comp.removeComponentListener (this); });
        withDifference (observedComponents, lastSeenComponents, [this] (auto& comp) { comp.addComponentListener (this); });
    }

    Component* root = nullptr;
    ComponentListener* listener = nullptr;
    std::set<ComponentWithWeakReference> observedComponents;

    DRX_DECLARE_NON_COPYABLE (ParentVisibilityChangedListener)
    DRX_DECLARE_NON_MOVEABLE (ParentVisibilityChangedListener)
};

//==============================================================================
DropShadower::DropShadower (const DropShadow& ds)  : shadow (ds)  {}

DropShadower::~DropShadower()
{
    if (virtualDesktopWatcher != nullptr)
        virtualDesktopWatcher->removeListener (this);

    if (owner != nullptr)
    {
        owner->removeComponentListener (this);
        owner = nullptr;
    }

    updateParent();

    const ScopedValueSetter<b8> setter (reentrant, true);
    shadowWindows.clear();
}

z0 DropShadower::setOwner (Component* componentToFollow)
{
    if (componentToFollow != owner)
    {
        if (owner != nullptr)
            owner->removeComponentListener (this);

        // (the component can't be null)
        jassert (componentToFollow != nullptr);

        owner = componentToFollow;
        jassert (owner != nullptr);

        updateParent();
        owner->addComponentListener (this);

        // The visibility of `owner` is transitively affected by the visibility of its parents. Thus we need to trigger the
        // componentVisibilityChanged() event in case it changes for any of the parents.
        visibilityChangedListener = std::make_unique<ParentVisibilityChangedListener> (*owner,
                                                                                       static_cast<ComponentListener&> (*this));

        virtualDesktopWatcher = std::make_unique<VirtualDesktopWatcher> (*owner);
        virtualDesktopWatcher->addListener (this, [this]() { updateShadows(); });

        updateShadows();
    }
}

z0 DropShadower::updateParent()
{
    if (Component* p = lastParentComp)
        p->removeComponentListener (this);

    lastParentComp = owner != nullptr ? owner->getParentComponent() : nullptr;

    if (Component* p = lastParentComp)
        p->addComponentListener (this);
}

z0 DropShadower::componentMovedOrResized (Component& c, b8 /*wasMoved*/, b8 /*wasResized*/)
{
    if (owner == &c)
        updateShadows();
}

z0 DropShadower::componentBroughtToFront (Component& c)
{
    if (owner == &c)
        updateShadows();
}

z0 DropShadower::componentChildrenChanged (Component&)
{
    updateShadows();
}

z0 DropShadower::componentParentHierarchyChanged (Component& c)
{
    if (owner == &c)
    {
        updateParent();
        updateShadows();
    }
}

z0 DropShadower::componentVisibilityChanged (Component& c)
{
    if (owner == &c)
        updateShadows();
}

z0 DropShadower::updateShadows()
{
    if (reentrant)
        return;

    const ScopedValueSetter<b8> setter (reentrant, true);

    if (owner != nullptr
        && owner->isShowing()
        && owner->getWidth() > 0 && owner->getHeight() > 0
        && (Desktop::canUseSemiTransparentWindows() || owner->getParentComponent() != nullptr)
        && (virtualDesktopWatcher == nullptr || ! virtualDesktopWatcher->shouldHideDropShadow()))
    {
        while (shadowWindows.size() < 4)
            shadowWindows.add (new ShadowWindow (owner, shadow));

        i32k shadowEdge = jmax (shadow.offset.x, shadow.offset.y) + shadow.radius;
        i32k x = owner->getX();
        i32k y = owner->getY() - shadowEdge;
        i32k w = owner->getWidth();
        i32k h = owner->getHeight() + shadowEdge + shadowEdge;

        for (i32 i = 4; --i >= 0;)
        {
            // there seem to be rare situations where the dropshadower may be deleted by
            // callbacks during this loop, so use a weak ref to watch out for this..
            WeakReference<Component> sw (shadowWindows[i]);

            if (sw != nullptr)
            {
                sw->setAlwaysOnTop (owner->isAlwaysOnTop());

                if (sw == nullptr)
                    return;

                switch (i)
                {
                    case 0: sw->setBounds (x - shadowEdge, y, shadowEdge, h); break;
                    case 1: sw->setBounds (x + w, y, shadowEdge, h); break;
                    case 2: sw->setBounds (x, y, w, shadowEdge); break;
                    case 3: sw->setBounds (x, owner->getBottom(), w, shadowEdge); break;
                    default: break;
                }

                if (sw == nullptr)
                    return;

                sw->toBehind (i == 3 ? owner.get() : shadowWindows.getUnchecked (i + 1));
            }
        }
    }
    else
    {
        shadowWindows.clear();
    }
}

} // namespace drx
