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

#define DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN \
    jassert ((MessageManager::getInstanceWithoutCreating() != nullptr \
               && MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager()) \
              || getPeer() == nullptr);

namespace drx
{

static Component* findFirstEnabledAncestor (Component* in)
{
    if (in == nullptr)
        return nullptr;

    if (in->isEnabled())
        return in;

    return findFirstEnabledAncestor (in->getParentComponent());
}

Component* Component::currentlyFocusedComponent = nullptr;

//==============================================================================
class HierarchyChecker
{
public:
    /*  Creates a bail-out checker for comp and its ancestors, that will return true from
        shouldBailOut() if all of comp's ancestors are destroyed.
        @param comp     a safe pointer to a component. The pointer will be updated to point
                        to the nearest non-null ancestor on each call to shouldBailOut.
    */
    HierarchyChecker (Component::SafePointer<Component>* comp, const MouseEvent& originalEvent)
        : closestAncestor (*comp),
          me (originalEvent)
    {
        for (Component* c = *comp; c != nullptr; c = c->getParentComponent())
            hierarchy.emplace_back (c);
    }

    Component* nearestNonNullParent() const
    {
        return closestAncestor;
    }

    /*  Searches for the closest ancestor, and returns true if the closest ancestor is nullptr. */
    b8 shouldBailOut() const
    {
        closestAncestor = findNearestNonNullParent();
        return closestAncestor == nullptr;
    }

    MouseEvent eventWithNearestParent() const
    {
        return { me.source,
                 me.position.toFloat(),
                 me.mods,
                 me.pressure, me.orientation, me.rotation,
                 me.tiltX, me.tiltY,
                 closestAncestor,
                 closestAncestor,
                 me.eventTime,
                 me.mouseDownPosition.toFloat(),
                 me.mouseDownTime,
                 me.getNumberOfClicks(),
                 me.mouseWasDraggedSinceMouseDown() };
    }

    template <typename Callback>
    z0 forEach (Callback&& callback)
    {
        for (auto& item : hierarchy)
            if (item != nullptr)
                callback (*item);
    }

private:
    Component* findNearestNonNullParent() const
    {
        for (auto& comp : hierarchy)
            if (comp != nullptr)
                return comp;

        return nullptr;
    }

    Component::SafePointer<Component>& closestAncestor;
    std::vector<Component::SafePointer<Component>> hierarchy;
    const MouseEvent me;
};

//==============================================================================
class Component::MouseListenerList
{
public:
    MouseListenerList() noexcept {}

    z0 addListener (MouseListener* newListener, b8 wantsEventsForAllNestedChildComponents)
    {
        if (! listeners.contains (newListener))
        {
            if (wantsEventsForAllNestedChildComponents)
            {
                listeners.insert (0, newListener);
                ++numDeepMouseListeners;
            }
            else
            {
                listeners.add (newListener);
            }
        }
    }

    z0 removeListener (MouseListener* listenerToRemove)
    {
        auto index = listeners.indexOf (listenerToRemove);

        if (index >= 0)
        {
            if (index < numDeepMouseListeners)
                --numDeepMouseListeners;

            listeners.remove (index);
        }
    }

    template <typename EventMethod, typename... Params>
    static z0 sendMouseEvent (HierarchyChecker& checker, EventMethod&& eventMethod, Params&&... params)
    {
        const auto callListeners = [&] (auto& parentComp, const auto findNumListeners)
        {
            if (auto* list = parentComp.mouseListeners.get())
            {
                const WeakReference safePointer { &parentComp };

                for (i32 i = findNumListeners (*list); --i >= 0; i = jmin (i, findNumListeners (*list)))
                {
                    (list->listeners.getUnchecked (i)->*eventMethod) (checker.eventWithNearestParent(), params...);

                    if (checker.shouldBailOut() || safePointer == nullptr)
                        return false;
                }
            }

            return true;
        };

        if (auto* parent = checker.nearestNonNullParent())
            if (! callListeners (*parent, [] (auto& list) { return list.listeners.size(); }))
                return;

        if (auto* parent = checker.nearestNonNullParent())
            for (Component* p = parent->parentComponent; p != nullptr; p = p->parentComponent)
                if (! callListeners (*p, [] (auto& list) { return list.numDeepMouseListeners; }))
                    return;
    }

private:
    Array<MouseListener*> listeners;
    i32 numDeepMouseListeners = 0;

    DRX_DECLARE_NON_COPYABLE (MouseListenerList)
};

class Component::EffectState
{
public:
    explicit EffectState (ImageEffectFilter& i) : effect (&i) {}

    ImageEffectFilter& getEffect() const
    {
        return *effect;
    }

    b8 setEffect (ImageEffectFilter& i)
    {
        return std::exchange (effect, &i) != &i;
    }

    z0 paint (Graphics& g, Component& c, b8 ignoreAlphaLevel)
    {
        auto scale = g.getInternalContext().getPhysicalPixelScaleFactor();
        auto scaledBounds = c.getLocalBounds() * scale;

        if (effectImage.getBounds() != scaledBounds)
            effectImage = Image { c.isOpaque() ? Image::RGB : Image::ARGB, scaledBounds.getWidth(), scaledBounds.getHeight(), false };

        if (! c.isOpaque())
            effectImage.clear (effectImage.getBounds());

        {
            Graphics g2 (effectImage);
            g2.addTransform (AffineTransform::scale ((f32) scaledBounds.getWidth()  / (f32) c.getWidth(),
                                                     (f32) scaledBounds.getHeight() / (f32) c.getHeight()));
            c.paintComponentAndChildren (g2);
        }

        Graphics::ScopedSaveState ss (g);

        g.addTransform (AffineTransform::scale (1.0f / scale));
        effect->applyEffect (effectImage, g, scale, ignoreAlphaLevel ? 1.0f : c.getAlpha());
    }

private:
    Image effectImage;
    ImageEffectFilter* effect;
};

//==============================================================================
Component::Component() noexcept
  : componentFlags (0)
{
}

Component::Component (const Txt& name) noexcept
  : componentName (name), componentFlags (0)
{
}

Component::~Component()
{
    static_assert (sizeof (flags) <= sizeof (componentFlags), "componentFlags has too many bits!");

    componentListeners.call ([this] (ComponentListener& l) { l.componentBeingDeleted (*this); });

    while (childComponentList.size() > 0)
        removeChildComponent (childComponentList.size() - 1, false, true);

    masterReference.clear();

    if (parentComponent != nullptr)
        parentComponent->removeChildComponent (parentComponent->childComponentList.indexOf (this), true, false);
    else
        giveAwayKeyboardFocusInternal (isParentOf (currentlyFocusedComponent));

    if (flags.hasHeavyweightPeerFlag)
        removeFromDesktop();

    // Something has added some children to this component during its destructor! Not a smart idea!
    jassert (childComponentList.size() == 0);
}

//==============================================================================
z0 Component::setName (const Txt& name)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (componentName != name)
    {
        componentName = name;

        if (flags.hasHeavyweightPeerFlag)
            if (auto* peer = getPeer())
                peer->setTitle (name);

        BailOutChecker checker (this);
        componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentNameChanged (*this); });
    }
}

z0 Component::setComponentID (const Txt& newID)
{
    componentID = newID;
}

z0 Component::setVisible (b8 shouldBeVisible)
{
    if (flags.visibleFlag != shouldBeVisible)
    {
        // if component methods are being called from threads other than the message
        // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
        DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

        const WeakReference<Component> safePointer (this);
        flags.visibleFlag = shouldBeVisible;

        if (shouldBeVisible)
            repaint();
        else
            repaintParent();

        sendFakeMouseMove();

        if (! shouldBeVisible)
        {
            detail::ComponentHelpers::releaseAllCachedImageResources (*this);

            if (hasKeyboardFocus (true))
            {
                if (parentComponent != nullptr)
                    parentComponent->grabKeyboardFocus();

                // ensure that keyboard focus is given away if it wasn't taken by parent
                giveAwayKeyboardFocus();
            }
        }

        if (safePointer != nullptr)
        {
            sendVisibilityChangeMessage();

            if (safePointer != nullptr && flags.hasHeavyweightPeerFlag)
            {
                if (auto* peer = getPeer())
                {
                    peer->setVisible (shouldBeVisible);
                    internalHierarchyChanged();
                }
            }
        }
    }
}

z0 Component::visibilityChanged() {}

z0 Component::sendVisibilityChangeMessage()
{
    BailOutChecker checker (this);
    visibilityChanged();

    if (! checker.shouldBailOut())
        componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentVisibilityChanged (*this); });
}

b8 Component::isShowing() const
{
    if (! flags.visibleFlag)
        return false;

    if (parentComponent != nullptr)
        return parentComponent->isShowing();

    if (auto* peer = getPeer())
        return ! peer->isMinimised();

    return false;
}

//==============================================================================
uk Component::getWindowHandle() const
{
    if (auto* peer = getPeer())
        return peer->getNativeHandle();

    return nullptr;
}

//==============================================================================
z0 Component::addToDesktop (i32 styleWanted, uk nativeWindowToAttachTo)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (isOpaque())
        styleWanted &= ~ComponentPeer::windowIsSemiTransparent;
    else
        styleWanted |= ComponentPeer::windowIsSemiTransparent;

    // don't use getPeer(), so that we only get the peer that's specifically
    // for this comp, and not for one of its parents.
    auto* peer = ComponentPeer::getPeerFor (this);

    if (peer == nullptr || styleWanted != peer->getStyleFlags())
    {
        const WeakReference<Component> safePointer (this);

       #if DRX_LINUX || DRX_BSD
        // it's wise to give the component a non-zero size before
        // putting it on the desktop, as X windows get confused by this, and
        // a (1, 1) minimum size is enforced here.
        setSize (jmax (1, getWidth()),
                 jmax (1, getHeight()));
       #endif

        const auto unscaledPosition = detail::ScalingHelpers::scaledScreenPosToUnscaled (getScreenPosition());
        const auto topLeft = detail::ScalingHelpers::unscaledScreenPosToScaled (*this, unscaledPosition);

        b8 wasFullscreen = false;
        b8 wasMinimised = false;
        ComponentBoundsConstrainer* currentConstrainer = nullptr;
        Rectangle<i32> oldNonFullScreenBounds;
        i32 oldRenderingEngine = -1;

        if (peer != nullptr)
        {
            std::unique_ptr<ComponentPeer> oldPeerToDelete (peer);

            wasFullscreen = peer->isFullScreen();
            wasMinimised = peer->isMinimised();
            currentConstrainer = peer->getConstrainer();
            oldNonFullScreenBounds = peer->getNonFullScreenBounds();
            oldRenderingEngine = peer->getCurrentRenderingEngine();

            flags.hasHeavyweightPeerFlag = false;
            Desktop::getInstance().removeDesktopComponent (this);
            internalHierarchyChanged(); // give comps a chance to react to the peer change before the old peer is deleted.

            if (safePointer == nullptr)
                return;

            setTopLeftPosition (topLeft);
        }

        if (parentComponent != nullptr)
            parentComponent->removeChildComponent (this);

        if (safePointer != nullptr)
        {
            flags.hasHeavyweightPeerFlag = true;

            peer = createNewPeer (styleWanted, nativeWindowToAttachTo);

            Desktop::getInstance().addDesktopComponent (this);

            boundsRelativeToParent.setPosition (topLeft);
            peer->updateBounds();

            if (oldRenderingEngine >= 0)
                peer->setCurrentRenderingEngine (oldRenderingEngine);

            peer->setVisible (isVisible());

            peer = ComponentPeer::getPeerFor (this);

            if (peer == nullptr)
                return;

            if (wasFullscreen)
            {
                peer->setFullScreen (true);
                peer->setNonFullScreenBounds (oldNonFullScreenBounds);
            }

            if (wasMinimised)
                peer->setMinimised (true);

           #if DRX_WINDOWS
            if (isAlwaysOnTop())
                peer->setAlwaysOnTop (true);
           #endif

            peer->setConstrainer (currentConstrainer);

            repaint();

           #if DRX_LINUX
            // Creating the peer Image on Linux will change the reported position of the window. If
            // the Image creation is interleaved with the coming configureNotifyEvents the window
            // will appear in the wrong position. To avoid this, we force the Image creation here,
            // before handling any of the configureNotifyEvents. The Linux implementation of
            // performAnyPendingRepaintsNow() will force update the peer position if necessary.
            peer->performAnyPendingRepaintsNow();
           #endif

            internalHierarchyChanged();

            if (auto* handler = getAccessibilityHandler())
                detail::AccessibilityHelpers::notifyAccessibilityEvent (*handler, detail::AccessibilityHelpers::Event::windowOpened);
        }
    }
}

z0 Component::removeFromDesktop()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (flags.hasHeavyweightPeerFlag)
    {
        if (auto* handler = getAccessibilityHandler())
            detail::AccessibilityHelpers::notifyAccessibilityEvent (*handler, detail::AccessibilityHelpers::Event::windowClosed);

        detail::ComponentHelpers::releaseAllCachedImageResources (*this);

        auto* peer = ComponentPeer::getPeerFor (this);
        jassert (peer != nullptr);

        flags.hasHeavyweightPeerFlag = false;
        delete peer;

        Desktop::getInstance().removeDesktopComponent (this);
    }
}

b8 Component::isOnDesktop() const noexcept
{
    return flags.hasHeavyweightPeerFlag;
}

ComponentPeer* Component::getPeer() const
{
    if (flags.hasHeavyweightPeerFlag)
        return ComponentPeer::getPeerFor (this);

    if (parentComponent == nullptr)
        return nullptr;

    return parentComponent->getPeer();
}

z0 Component::userTriedToCloseWindow()
{
    /* This means that the user's trying to get rid of your window with the 'close window' system
       menu option (on windows) or possibly the task manager - you should really handle this
       and delete or hide your component in an appropriate way.

       If you want to ignore the event and don't want to trigger this assertion, just override
       this method and do nothing.
    */
    jassertfalse;
}

z0 Component::minimisationStateChanged (b8) {}

f32 Component::getDesktopScaleFactor() const  { return Desktop::getInstance().getGlobalScaleFactor(); }

//==============================================================================
z0 Component::setOpaque (b8 shouldBeOpaque)
{
    if (shouldBeOpaque != flags.opaqueFlag)
    {
        flags.opaqueFlag = shouldBeOpaque;

        if (flags.hasHeavyweightPeerFlag)
            if (auto* peer = ComponentPeer::getPeerFor (this))
                addToDesktop (peer->getStyleFlags());  // recreates the heavyweight window

        repaint();
    }
}

b8 Component::isOpaque() const noexcept
{
    return flags.opaqueFlag;
}

//==============================================================================
z0 Component::setCachedComponentImage (CachedComponentImage* newCachedImage)
{
    if (cachedImage.get() != newCachedImage)
    {
        cachedImage.reset (newCachedImage);
        repaint();
    }
}

z0 Component::setBufferedToImage (b8 shouldBeBuffered)
{
    // This assertion means that this component is already using a custom CachedComponentImage,
    // so by calling setBufferedToImage, you'll be deleting the custom one - this is almost certainly
    // not what you wanted to happen... If you really do know what you're doing here, and want to
    // avoid this assertion, just call setCachedComponentImage (nullptr) before setBufferedToImage().
    jassert (cachedImage == nullptr || dynamic_cast<detail::StandardCachedComponentImage*> (cachedImage.get()) != nullptr);

    if (shouldBeBuffered)
    {
        if (cachedImage == nullptr)
            cachedImage = std::make_unique<detail::StandardCachedComponentImage> (*this);
    }
    else
    {
        cachedImage.reset();
    }
}

//==============================================================================
z0 Component::reorderChildInternal (i32 sourceIndex, i32 destIndex)
{
    if (sourceIndex != destIndex)
    {
        auto* c = childComponentList.getUnchecked (sourceIndex);
        jassert (c != nullptr);
        c->repaintParent();

        childComponentList.move (sourceIndex, destIndex);

        sendFakeMouseMove();
        internalChildrenChanged();
    }
}

z0 Component::toFront (b8 shouldGrabKeyboardFocus)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (flags.hasHeavyweightPeerFlag)
    {
        if (auto* peer = getPeer())
        {
            peer->toFront (shouldGrabKeyboardFocus);

            if (shouldGrabKeyboardFocus && ! hasKeyboardFocus (true))
                grabKeyboardFocus();
        }
    }
    else if (parentComponent != nullptr)
    {
        auto& childList = parentComponent->childComponentList;

        if (childList.getLast() != this)
        {
            auto index = childList.indexOf (this);

            if (index >= 0)
            {
                i32 insertIndex = -1;

                if (! flags.alwaysOnTopFlag)
                {
                    insertIndex = childList.size() - 1;

                    while (insertIndex > 0 && childList.getUnchecked (insertIndex)->isAlwaysOnTop())
                        --insertIndex;
                }

                parentComponent->reorderChildInternal (index, insertIndex);
            }
        }

        if (shouldGrabKeyboardFocus)
        {
            internalBroughtToFront();

            if (isShowing())
                grabKeyboardFocus();
        }
    }
}

z0 Component::toBehind (Component* other)
{
    if (other != nullptr && other != this)
    {
        // the two components must belong to the same parent..
        jassert (parentComponent == other->parentComponent);

        if (parentComponent != nullptr)
        {
            auto& childList = parentComponent->childComponentList;
            auto index = childList.indexOf (this);

            if (index >= 0 && childList [index + 1] != other)
            {
                auto otherIndex = childList.indexOf (other);

                if (otherIndex >= 0)
                {
                    if (index < otherIndex)
                        --otherIndex;

                    parentComponent->reorderChildInternal (index, otherIndex);
                }
            }
        }
        else if (isOnDesktop())
        {
            jassert (other->isOnDesktop());

            if (other->isOnDesktop())
            {
                auto* us = getPeer();
                auto* them = other->getPeer();
                jassert (us != nullptr && them != nullptr);

                if (us != nullptr && them != nullptr)
                    us->toBehind (them);
            }
        }
    }
}

z0 Component::toBack()
{
    if (isOnDesktop())
    {
        jassertfalse; //xxx need to add this to native window
    }
    else if (parentComponent != nullptr)
    {
        auto& childList = parentComponent->childComponentList;

        if (childList.getFirst() != this)
        {
            auto index = childList.indexOf (this);

            if (index > 0)
            {
                i32 insertIndex = 0;

                if (flags.alwaysOnTopFlag)
                    while (insertIndex < childList.size() && ! childList.getUnchecked (insertIndex)->isAlwaysOnTop())
                        ++insertIndex;

                parentComponent->reorderChildInternal (index, insertIndex);
            }
        }
    }
}

z0 Component::setAlwaysOnTop (b8 shouldStayOnTop)
{
    if (shouldStayOnTop != flags.alwaysOnTopFlag)
    {
        BailOutChecker checker (this);

        flags.alwaysOnTopFlag = shouldStayOnTop;

        if (isOnDesktop())
        {
            if (auto* peer = getPeer())
            {
                if (! peer->setAlwaysOnTop (shouldStayOnTop))
                {
                    // some kinds of peer can't change their always-on-top status, so
                    // for these, we'll need to create a new window
                    auto oldFlags = peer->getStyleFlags();
                    removeFromDesktop();
                    addToDesktop (oldFlags);
                }
            }
        }

        if (shouldStayOnTop && ! checker.shouldBailOut())
            toFront (false);

        if (! checker.shouldBailOut())
            internalHierarchyChanged();
    }
}

b8 Component::isAlwaysOnTop() const noexcept
{
    return flags.alwaysOnTopFlag;
}

//==============================================================================
i32 Component::proportionOfWidth  (f32 proportion) const noexcept   { return roundToInt (proportion * (f32) boundsRelativeToParent.getWidth()); }
i32 Component::proportionOfHeight (f32 proportion) const noexcept   { return roundToInt (proportion * (f32) boundsRelativeToParent.getHeight()); }

i32 Component::getParentWidth() const noexcept
{
    return parentComponent != nullptr ? parentComponent->getWidth()
                                      : getParentMonitorArea().getWidth();
}

i32 Component::getParentHeight() const noexcept
{
    return parentComponent != nullptr ? parentComponent->getHeight()
                                      : getParentMonitorArea().getHeight();
}

Rectangle<i32> Component::getParentMonitorArea() const
{
    return Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds())->userArea;
}

i32 Component::getScreenX() const                       { return getScreenPosition().x; }
i32 Component::getScreenY() const                       { return getScreenPosition().y; }
Point<i32>     Component::getScreenPosition() const     { return localPointToGlobal (Point<i32>()); }
Rectangle<i32> Component::getScreenBounds() const       { return localAreaToGlobal (getLocalBounds()); }

Point<i32>       Component::getLocalPoint (const Component* source, Point<i32> point) const       { return detail::ComponentHelpers::convertCoordinate (this, source, point); }
Point<f32>     Component::getLocalPoint (const Component* source, Point<f32> point) const     { return detail::ComponentHelpers::convertCoordinate (this, source, point); }
Rectangle<i32>   Component::getLocalArea  (const Component* source, Rectangle<i32> area) const    { return detail::ComponentHelpers::convertCoordinate (this, source, area); }
Rectangle<f32> Component::getLocalArea  (const Component* source, Rectangle<f32> area) const  { return detail::ComponentHelpers::convertCoordinate (this, source, area); }

Point<i32>       Component::localPointToGlobal (Point<i32> point) const       { return detail::ComponentHelpers::convertCoordinate (nullptr, this, point); }
Point<f32>     Component::localPointToGlobal (Point<f32> point) const     { return detail::ComponentHelpers::convertCoordinate (nullptr, this, point); }
Rectangle<i32>   Component::localAreaToGlobal  (Rectangle<i32> area) const    { return detail::ComponentHelpers::convertCoordinate (nullptr, this, area); }
Rectangle<f32> Component::localAreaToGlobal  (Rectangle<f32> area) const  { return detail::ComponentHelpers::convertCoordinate (nullptr, this, area); }

//==============================================================================
z0 Component::setBounds (i32 x, i32 y, i32 w, i32 h)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (w < 0) w = 0;
    if (h < 0) h = 0;

    const b8 wasResized  = (getWidth() != w || getHeight() != h);
    const b8 wasMoved    = (getX() != x || getY() != y);

   #if DRX_DEBUG
    // It's a very bad idea to try to resize a window during its paint() method!
    jassert (! (flags.isInsidePaintCall && wasResized && isOnDesktop()));
   #endif

    if (wasMoved || wasResized)
    {
        const b8 showing = isShowing();

        if (showing)
        {
            // send a fake mouse move to trigger enter/exit messages if needed..
            sendFakeMouseMove();

            if (! flags.hasHeavyweightPeerFlag)
                repaintParent();
        }

        boundsRelativeToParent.setBounds (x, y, w, h);

        if (showing)
        {
            if (wasResized)
                repaint();
            else if (! flags.hasHeavyweightPeerFlag)
                repaintParent();
        }
        else if (cachedImage != nullptr)
        {
            cachedImage->invalidateAll();
        }

        flags.isMoveCallbackPending = wasMoved;
        flags.isResizeCallbackPending = wasResized;

        if (flags.hasHeavyweightPeerFlag)
            if (auto* peer = getPeer())
                peer->updateBounds();

        sendMovedResizedMessagesIfPending();
    }
}

z0 Component::sendMovedResizedMessagesIfPending()
{
    const b8 wasMoved   = flags.isMoveCallbackPending;
    const b8 wasResized = flags.isResizeCallbackPending;

    if (wasMoved || wasResized)
    {
        flags.isMoveCallbackPending = false;
        flags.isResizeCallbackPending = false;

        sendMovedResizedMessages (wasMoved, wasResized);
    }
}

z0 Component::sendMovedResizedMessages (b8 wasMoved, b8 wasResized)
{
    BailOutChecker checker (this);

    if (wasMoved)
    {
        moved();

        if (checker.shouldBailOut())
            return;
    }

    if (wasResized)
    {
        resized();

        if (checker.shouldBailOut())
            return;

        for (i32 i = childComponentList.size(); --i >= 0;)
        {
            childComponentList.getUnchecked (i)->parentSizeChanged();

            if (checker.shouldBailOut())
                return;

            i = jmin (i, childComponentList.size());
        }
    }

    if (parentComponent != nullptr)
        parentComponent->childBoundsChanged (this);

    if (! checker.shouldBailOut())
    {
        componentListeners.callChecked (checker, [this, wasMoved, wasResized] (ComponentListener& l)
        {
            l.componentMovedOrResized (*this, wasMoved, wasResized);
        });
    }

    if ((wasMoved || wasResized) && ! checker.shouldBailOut())
        if (auto* handler = getAccessibilityHandler())
            detail::AccessibilityHelpers::notifyAccessibilityEvent (*handler, detail::AccessibilityHelpers::Event::elementMovedOrResized);
}

z0 Component::setSize (i32 w, i32 h)                  { setBounds (getX(), getY(), w, h); }

z0 Component::setTopLeftPosition (i32 x, i32 y)       { setTopLeftPosition ({ x, y }); }
z0 Component::setTopLeftPosition (Point<i32> pos)     { setBounds (pos.x, pos.y, getWidth(), getHeight()); }

z0 Component::setTopRightPosition (i32 x, i32 y)      { setTopRightPosition ({ x, y }); }
z0 Component::setTopRightPosition (Point<i32> pos)    { setTopLeftPosition (pos.x - getWidth(), pos.y); }
z0 Component::setBounds (Rectangle<i32> r)            { setBounds (r.getX(), r.getY(), r.getWidth(), r.getHeight()); }

z0 Component::setCentrePosition (Point<i32> p)        { setBounds (getBounds().withCentre (p.transformedBy (getTransform().inverted()))); }
z0 Component::setCentrePosition (i32 x, i32 y)        { setCentrePosition ({ x, y }); }

z0 Component::setCentreRelative (f32 x, f32 y)
{
    setCentrePosition (roundToInt ((f32) getParentWidth()  * x),
                       roundToInt ((f32) getParentHeight() * y));
}

z0 Component::setBoundsRelative (Rectangle<f32> target)
{
    setBounds ((target * Point<f32> ((f32) getParentWidth(),
                                       (f32) getParentHeight())).toNearestInt());
}

z0 Component::setBoundsRelative (f32 x, f32 y, f32 w, f32 h)
{
    setBoundsRelative ({ x, y, w, h });
}

z0 Component::centreWithSize (i32 width, i32 height)
{
    auto parentArea = detail::ComponentHelpers::getParentOrMainMonitorBounds (*this)
                          .transformedBy (getTransform().inverted());

    setBounds (parentArea.getCentreX() - width / 2,
               parentArea.getCentreY() - height / 2,
               width, height);
}

z0 Component::setBoundsInset (BorderSize<i32> borders)
{
    setBounds (borders.subtractedFrom (detail::ComponentHelpers::getParentOrMainMonitorBounds (*this)));
}

z0 Component::setBoundsToFit (Rectangle<i32> targetArea, Justification justification, b8 onlyReduceInSize)
{
    if (getLocalBounds().isEmpty() || targetArea.isEmpty())
    {
        // it's no good calling this method unless both the component and
        // target rectangle have a finite size.
        jassertfalse;
        return;
    }

    auto sourceArea = targetArea.withZeroOrigin();

    if (onlyReduceInSize
         && getWidth() <= targetArea.getWidth()
         && getHeight() <= targetArea.getHeight())
    {
        sourceArea = getLocalBounds();
    }
    else
    {
        auto sourceRatio = getHeight() / (f64) getWidth();
        auto targetRatio = targetArea.getHeight() / (f64) targetArea.getWidth();

        if (sourceRatio <= targetRatio)
            sourceArea.setHeight (jmin (targetArea.getHeight(),
                                        roundToInt (targetArea.getWidth() * sourceRatio)));
        else
            sourceArea.setWidth (jmin (targetArea.getWidth(),
                                       roundToInt (targetArea.getHeight() / sourceRatio)));
    }

    if (! sourceArea.isEmpty())
        setBounds (justification.appliedToRectangle (sourceArea, targetArea));
}

//==============================================================================
z0 Component::setTransform (const AffineTransform& newTransform)
{
    // If you pass in a transform with no inverse, the component will have no dimensions,
    // and there will be all sorts of maths errors when converting coordinates.
    jassert (! newTransform.isSingularity());

    if (newTransform.isIdentity())
    {
        if (affineTransform != nullptr)
        {
            repaint();
            affineTransform.reset();
            repaint();
            sendMovedResizedMessages (false, false);
        }
    }
    else if (affineTransform == nullptr)
    {
        repaint();
        affineTransform.reset (new AffineTransform (newTransform));
        repaint();
        sendMovedResizedMessages (false, false);
    }
    else if (*affineTransform != newTransform)
    {
        repaint();
        *affineTransform = newTransform;
        repaint();
        sendMovedResizedMessages (false, false);
    }
}

b8 Component::isTransformed() const noexcept
{
    return affineTransform != nullptr;
}

AffineTransform Component::getTransform() const
{
    return affineTransform != nullptr ? *affineTransform : AffineTransform();
}

f32 Component::getApproximateScaleFactorForComponent (const Component* targetComponent)
{
    AffineTransform transform;

    for (auto* target = targetComponent; target != nullptr; target = target->getParentComponent())
    {
        transform = transform.followedBy (target->getTransform());

        if (target->isOnDesktop())
            transform = transform.scaled (target->getDesktopScaleFactor());
    }

    auto transformScale = std::sqrt (std::abs (transform.getDeterminant()));
    return transformScale / Desktop::getInstance().getGlobalScaleFactor();
}

//==============================================================================
b8 Component::hitTest (i32 x, i32 y)
{
    if (! flags.ignoresMouseClicksFlag)
        return true;

    if (flags.allowChildMouseClicksFlag)
    {
        for (i32 i = childComponentList.size(); --i >= 0;)
        {
            auto& child = *childComponentList.getUnchecked (i);

            if (child.isVisible()
                 && detail::ComponentHelpers::hitTest (child, detail::ComponentHelpers::convertFromParentSpace (child, Point<i32> (x, y).toFloat())))
                return true;
        }
    }

    return false;
}

z0 Component::setInterceptsMouseClicks (b8 allowClicks,
                                          b8 allowClicksOnChildComponents) noexcept
{
    flags.ignoresMouseClicksFlag = ! allowClicks;
    flags.allowChildMouseClicksFlag = allowClicksOnChildComponents;
}

z0 Component::getInterceptsMouseClicks (b8& allowsClicksOnThisComponent,
                                          b8& allowsClicksOnChildComponents) const noexcept
{
    allowsClicksOnThisComponent = ! flags.ignoresMouseClicksFlag;
    allowsClicksOnChildComponents = flags.allowChildMouseClicksFlag;
}

b8 Component::contains (Point<i32> point)
{
    return contains (point.toFloat());
}

b8 Component::contains (Point<f32> point)
{
    if (detail::ComponentHelpers::hitTest (*this, point))
    {
        if (parentComponent != nullptr)
            return parentComponent->contains (detail::ComponentHelpers::convertToParentSpace (*this, point));

        if (flags.hasHeavyweightPeerFlag)
            if (auto* peer = getPeer())
                return peer->contains (detail::ComponentHelpers::localPositionToRawPeerPos (*this, point).roundToInt(), true);
    }

    return false;
}

b8 Component::reallyContains (Point<i32> point, b8 returnTrueIfWithinAChild)
{
    return reallyContains (point.toFloat(), returnTrueIfWithinAChild);
}

b8 Component::reallyContains (Point<f32> point, b8 returnTrueIfWithinAChild)
{
    if (! contains (point))
        return false;

    auto* top = getTopLevelComponent();
    auto* compAtPosition = top->getComponentAt (top->getLocalPoint (this, point));

    return (compAtPosition == this) || (returnTrueIfWithinAChild && isParentOf (compAtPosition));
}

Component* Component::getComponentAt (Point<i32> position)
{
    return getComponentAt (position.toFloat());
}

Component* Component::getComponentAt (Point<f32> position)
{
    if (flags.visibleFlag && detail::ComponentHelpers::hitTest (*this, position))
    {
        for (i32 i = childComponentList.size(); --i >= 0;)
        {
            auto* child = childComponentList.getUnchecked (i);

            child = child->getComponentAt (detail::ComponentHelpers::convertFromParentSpace (*child, position));

            if (child != nullptr)
                return child;
        }

        return this;
    }

    return nullptr;
}

Component* Component::getComponentAt (i32 x, i32 y)
{
    return getComponentAt (Point<i32> { x, y });
}

//==============================================================================
z0 Component::addChildComponent (Component& child, i32 zOrder)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    jassert (this != &child); // adding a component to itself!?

    if (child.parentComponent != this)
    {
        if (child.parentComponent != nullptr)
            child.parentComponent->removeChildComponent (&child);
        else
            child.removeFromDesktop();

        child.parentComponent = this;

        if (child.isVisible())
            child.repaintParent();

        if (! child.isAlwaysOnTop())
        {
            if (zOrder < 0 || zOrder > childComponentList.size())
                zOrder = childComponentList.size();

            while (zOrder > 0)
            {
                if (! childComponentList.getUnchecked (zOrder - 1)->isAlwaysOnTop())
                    break;

                --zOrder;
            }
        }

        childComponentList.insert (zOrder, &child);

        child.internalHierarchyChanged();
        internalChildrenChanged();
    }
}

z0 Component::addAndMakeVisible (Component& child, i32 zOrder)
{
    child.setVisible (true);
    addChildComponent (child, zOrder);
}

z0 Component::addChildComponent (Component* child, i32 zOrder)
{
    if (child != nullptr)
        addChildComponent (*child, zOrder);
}

z0 Component::addAndMakeVisible (Component* child, i32 zOrder)
{
    if (child != nullptr)
        addAndMakeVisible (*child, zOrder);
}

z0 Component::addChildAndSetID (Component* child, const Txt& childID)
{
    if (child != nullptr)
    {
        child->setComponentID (childID);
        addAndMakeVisible (child);
    }
}

z0 Component::removeChildComponent (Component* child)
{
    removeChildComponent (childComponentList.indexOf (child), true, true);
}

Component* Component::removeChildComponent (i32 index)
{
    return removeChildComponent (index, true, true);
}

Component* Component::removeChildComponent (i32 index, b8 sendParentEvents, b8 sendChildEvents)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED_OR_OFFSCREEN

    if (auto* child = childComponentList [index])
    {
        sendParentEvents = sendParentEvents && child->isShowing();

        if (sendParentEvents)
        {
            sendFakeMouseMove();

            if (child->isVisible())
                child->repaintParent();
        }

        childComponentList.remove (index);
        child->parentComponent = nullptr;

        detail::ComponentHelpers::releaseAllCachedImageResources (*child);

        // (NB: there are obscure situations where child->isShowing() = false, but it still has the focus)
        if (child->hasKeyboardFocus (true))
        {
            const WeakReference<Component> safeThis (this);

            child->giveAwayKeyboardFocusInternal (sendChildEvents || currentlyFocusedComponent != child);

            if (sendParentEvents)
            {
                if (safeThis == nullptr)
                    return child;

                grabKeyboardFocus();
            }
        }

        if (sendChildEvents)
            child->internalHierarchyChanged();

        if (sendParentEvents)
            internalChildrenChanged();

        return child;
    }

    return nullptr;
}

//==============================================================================
z0 Component::removeAllChildren()
{
    while (! childComponentList.isEmpty())
        removeChildComponent (childComponentList.size() - 1);
}

z0 Component::deleteAllChildren()
{
    while (! childComponentList.isEmpty())
        delete (removeChildComponent (childComponentList.size() - 1));
}

i32 Component::getNumChildComponents() const noexcept
{
    return childComponentList.size();
}

Component* Component::getChildComponent (i32 index) const noexcept
{
    return childComponentList[index];
}

i32 Component::getIndexOfChildComponent (const Component* child) const noexcept
{
    return childComponentList.indexOf (const_cast<Component*> (child));
}

Component* Component::findChildWithID (StringRef targetID) const noexcept
{
    for (auto* c : childComponentList)
        if (c->componentID == targetID)
            return c;

    return nullptr;
}

Component* Component::getTopLevelComponent() const noexcept
{
    auto* comp = this;

    while (comp->parentComponent != nullptr)
        comp = comp->parentComponent;

    return const_cast<Component*> (comp);
}

b8 Component::isParentOf (const Component* possibleChild) const noexcept
{
    while (possibleChild != nullptr)
    {
        possibleChild = possibleChild->parentComponent;

        if (possibleChild == this)
            return true;
    }

    return false;
}

//==============================================================================
z0 Component::parentHierarchyChanged() {}
z0 Component::childrenChanged() {}

z0 Component::internalChildrenChanged()
{
    if (componentListeners.isEmpty())
    {
        childrenChanged();
    }
    else
    {
        BailOutChecker checker (this);

        childrenChanged();

        if (! checker.shouldBailOut())
            componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentChildrenChanged (*this); });
    }
}

z0 Component::internalHierarchyChanged()
{
    BailOutChecker checker (this);

    parentHierarchyChanged();

    if (checker.shouldBailOut())
        return;

    componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentParentHierarchyChanged (*this); });

    if (checker.shouldBailOut())
        return;

    for (i32 i = childComponentList.size(); --i >= 0;)
    {
        childComponentList.getUnchecked (i)->internalHierarchyChanged();

        if (checker.shouldBailOut())
        {
            // you really shouldn't delete the parent component during a callback telling you
            // that it's changed..
            jassertfalse;
            return;
        }

        i = jmin (i, childComponentList.size());
    }

    if (flags.hasHeavyweightPeerFlag)
        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::structureChanged);
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
i32 Component::runModalLoop()
{
    if (! MessageManager::getInstance()->isThisTheMessageThread())
    {
        // use a callback so this can be called from non-gui threads
        return (i32) (pointer_sized_int) MessageManager::getInstance()
                                           ->callFunctionOnMessageThread (&detail::ComponentHelpers::runModalLoopCallback, this);
    }

    if (! isCurrentlyModal (false))
        enterModalState (true);

    return ModalComponentManager::getInstance()->runEventLoopForCurrentComponent();
}
#endif

//==============================================================================
z0 Component::enterModalState (b8 shouldTakeKeyboardFocus,
                                 ModalComponentManager::Callback* callback,
                                 b8 deleteWhenDismissed)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    SafePointer safeReference { this };

    if (! isCurrentlyModal (false))
    {
        // While this component is in modal state it may block other components from receiving
        // mouseExit events. To keep mouseEnter and mouseExit calls balanced on these components,
        // we must manually force the mouse to "leave" blocked components.
        detail::ComponentHelpers::sendMouseEventToComponentsThatAreBlockedByModal (*this, &Component::internalMouseExit);

        if (safeReference == nullptr)
        {
            // If you hit this assertion, the mouse-exit event above has caused the modal component to be deleted.
            jassertfalse;
            return;
        }

        auto& mcm = *ModalComponentManager::getInstance();
        mcm.startModal ({}, this, deleteWhenDismissed);
        mcm.attachCallback (this, callback);

        setVisible (true);

        if (shouldTakeKeyboardFocus)
            grabKeyboardFocus();
    }
    else
    {
        // Probably a bad idea to try to make a component modal twice!
        jassertfalse;
    }
}

z0 Component::exitModalState (i32 returnValue)
{
    WeakReference<Component> deletionChecker (this);

    if (isCurrentlyModal (false))
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            auto& mcm = *ModalComponentManager::getInstance();
            mcm.endModal ({}, this, returnValue);
            mcm.bringModalComponentsToFront();

            // While this component is in modal state it may block other components from receiving
            // mouseEnter events. To keep mouseEnter and mouseExit calls balanced on these components,
            // we must manually force the mouse to "enter" blocked components.
            if (deletionChecker != nullptr)
                detail::ComponentHelpers::sendMouseEventToComponentsThatAreBlockedByModal (*deletionChecker, &Component::internalMouseEnter);
        }
        else
        {
            MessageManager::callAsync ([target = WeakReference<Component> { this }, returnValue]
            {
                if (target != nullptr)
                    target->exitModalState (returnValue);
            });
        }
    }
}

b8 Component::isCurrentlyModal (b8 onlyConsiderForemostModalComponent) const noexcept
{
    auto& mcm = *ModalComponentManager::getInstance();

    return onlyConsiderForemostModalComponent ? mcm.isFrontModalComponent (this)
                                              : mcm.isModal (this);
}

b8 Component::isCurrentlyBlockedByAnotherModalComponent() const
{
    return detail::ComponentHelpers::modalWouldBlockComponent (*this, getCurrentlyModalComponent());
}

i32 DRX_CALLTYPE Component::getNumCurrentlyModalComponents() noexcept
{
    if (auto* manager = ModalComponentManager::getInstanceWithoutCreating())
        return manager->getNumModalComponents();

    return {};
}

Component* DRX_CALLTYPE Component::getCurrentlyModalComponent (i32 index) noexcept
{
    if (auto* manager = ModalComponentManager::getInstanceWithoutCreating())
        return manager->getModalComponent (index);

    return {};
}

//==============================================================================
z0 Component::setBroughtToFrontOnMouseClick (b8 shouldBeBroughtToFront) noexcept
{
    flags.bringToFrontOnClickFlag = shouldBeBroughtToFront;
}

b8 Component::isBroughtToFrontOnMouseClick() const noexcept
{
    return flags.bringToFrontOnClickFlag;
}

//==============================================================================
z0 Component::setMouseCursor (const MouseCursor& newCursor)
{
    if (cursor != newCursor)
    {
        cursor = newCursor;

        if (flags.visibleFlag)
            updateMouseCursor();
    }
}

MouseCursor Component::getMouseCursor()
{
    return cursor;
}

z0 Component::updateMouseCursor() const
{
    Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
}

//==============================================================================
z0 Component::setRepaintsOnMouseActivity (b8 shouldRepaint) noexcept
{
    flags.repaintOnMouseActivityFlag = shouldRepaint;
}

//==============================================================================
f32 Component::getAlpha() const noexcept
{
    return (255 - componentTransparency) / 255.0f;
}

z0 Component::setAlpha (f32 newAlpha)
{
    auto newIntAlpha = (u8) (255 - jlimit (0, 255, roundToInt (newAlpha * 255.0)));

    if (componentTransparency != newIntAlpha)
    {
        componentTransparency = newIntAlpha;
        alphaChanged();
    }
}

z0 Component::alphaChanged()
{
    if (flags.hasHeavyweightPeerFlag)
    {
        if (auto* peer = getPeer())
            peer->setAlpha (getAlpha());
    }
    else
    {
        repaint();
    }
}

//==============================================================================
z0 Component::repaint()
{
    internalRepaintUnchecked (getLocalBounds(), true);
}

z0 Component::repaint (i32 x, i32 y, i32 w, i32 h)
{
    internalRepaint ({ x, y, w, h });
}

z0 Component::repaint (Rectangle<i32> area)
{
    internalRepaint (area);
}

z0 Component::repaintParent()
{
    if (parentComponent != nullptr)
        parentComponent->internalRepaint (detail::ComponentHelpers::convertToParentSpace (*this, getLocalBounds()));
}

z0 Component::internalRepaint (Rectangle<i32> area)
{
    area = area.getIntersection (getLocalBounds());

    if (! area.isEmpty())
        internalRepaintUnchecked (area, false);
}

z0 Component::internalRepaintUnchecked (Rectangle<i32> area, b8 isEntireComponent)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (flags.visibleFlag)
    {
        if (cachedImage != nullptr)
            if (! (isEntireComponent ? cachedImage->invalidateAll()
                                     : cachedImage->invalidate (area)))
                return;

        if (area.isEmpty())
            return;

        if (flags.hasHeavyweightPeerFlag)
        {
            if (auto* peer = getPeer())
            {
                // Tweak the scaling so that the component's integer size exactly aligns with the peer's scaled size
                auto peerBounds = peer->getBounds();
                auto scaled = area * Point<f32> ((f32) peerBounds.getWidth()  / (f32) getWidth(),
                                                   (f32) peerBounds.getHeight() / (f32) getHeight());

                peer->repaint (affineTransform != nullptr ? scaled.transformedBy (*affineTransform) : scaled);
            }
        }
        else
        {
            if (parentComponent != nullptr)
                parentComponent->internalRepaint (detail::ComponentHelpers::convertToParentSpace (*this, area));
        }
    }
}

//==============================================================================
z0 Component::paint (Graphics&)
{
    // if your component is marked as opaque, you must implement a paint
    // method and ensure that its entire area is completely painted.
    jassert (getBounds().isEmpty() || ! isOpaque());
}

z0 Component::paintOverChildren (Graphics&)
{
    // all painting is done in the subclasses
}

//==============================================================================
z0 Component::paintWithinParentContext (Graphics& g)
{
    g.setOrigin (getPosition());

    if (cachedImage != nullptr)
        cachedImage->paint (g);
    else
        paintEntireComponent (g, false);
}

z0 Component::paintComponentAndChildren (Graphics& g)
{
   #if DRX_ETW_TRACELOGGING
    {
        i32 depth = 0;
        auto parent = getParentComponent();
        while (parent)
        {
            parent = parent->getParentComponent();
            depth++;
        }

        DRX_TRACE_LOG_PAINT_COMPONENT_AND_CHILDREN (depth);
    }
   #endif

    auto clipBounds = g.getClipBounds();

    if (flags.dontClipGraphicsFlag && getNumChildComponents() == 0)
    {
        paint (g);
    }
    else
    {
        Graphics::ScopedSaveState ss (g);

        if (! (detail::ComponentHelpers::clipObscuredRegions (*this, g, clipBounds, {}) && g.isClipEmpty()))
            paint (g);
    }

    for (i32 i = 0; i < childComponentList.size(); ++i)
    {
        auto& child = *childComponentList.getUnchecked (i);

        if (child.isVisible())
        {
            if (child.affineTransform != nullptr)
            {
                Graphics::ScopedSaveState ss (g);

                g.addTransform (*child.affineTransform);

                if ((child.flags.dontClipGraphicsFlag && ! g.isClipEmpty()) || g.reduceClipRegion (child.getBounds()))
                    child.paintWithinParentContext (g);
            }
            else if (clipBounds.intersects (child.getBounds()))
            {
                Graphics::ScopedSaveState ss (g);

                if (child.flags.dontClipGraphicsFlag)
                {
                    child.paintWithinParentContext (g);
                }
                else if (g.reduceClipRegion (child.getBounds()))
                {
                    b8 nothingClipped = true;

                    for (i32 j = i + 1; j < childComponentList.size(); ++j)
                    {
                        auto& sibling = *childComponentList.getUnchecked (j);

                        if (sibling.flags.opaqueFlag && sibling.isVisible() && sibling.affineTransform == nullptr)
                        {
                            nothingClipped = false;
                            g.excludeClipRegion (sibling.getBounds());
                        }
                    }

                    if (nothingClipped || ! g.isClipEmpty())
                        child.paintWithinParentContext (g);
                }
            }
        }
    }

    Graphics::ScopedSaveState ss (g);
    paintOverChildren (g);
}

z0 Component::paintEntireComponent (Graphics& g, b8 ignoreAlphaLevel)
{
    // If sizing a top-level-window and the OS paint message is delivered synchronously
    // before resized() is called, then we'll invoke the callback here, to make sure
    // the components inside have had a chance to sort their sizes out..
   #if DRX_DEBUG
    if (! flags.isInsidePaintCall) // (avoids an assertion in plugins hosted in WaveLab)
   #endif
        sendMovedResizedMessagesIfPending();

   #if DRX_DEBUG
    flags.isInsidePaintCall = true;
   #endif

    if (effectState != nullptr)
    {
        effectState->paint (g, *this, ignoreAlphaLevel);
    }
    else if (componentTransparency > 0 && ! ignoreAlphaLevel)
    {
        if (componentTransparency < 255)
        {
            g.beginTransparencyLayer (getAlpha());
            paintComponentAndChildren (g);
            g.endTransparencyLayer();
        }
    }
    else
    {
        paintComponentAndChildren (g);
    }

   #if DRX_DEBUG
    flags.isInsidePaintCall = false;
   #endif
}

z0 Component::setPaintingIsUnclipped (b8 shouldPaintWithoutClipping) noexcept
{
    flags.dontClipGraphicsFlag = shouldPaintWithoutClipping;
}

b8 Component::isPaintingUnclipped() const noexcept
{
    return flags.dontClipGraphicsFlag;
}

//==============================================================================
Image Component::createComponentSnapshot (Rectangle<i32> areaToGrab,
                                          b8 clipImageToComponentBounds, f32 scaleFactor)
{
    auto r = areaToGrab;

    if (clipImageToComponentBounds)
        r = r.getIntersection (getLocalBounds());

    if (r.isEmpty())
        return {};

    auto w = roundToInt (scaleFactor * (f32) r.getWidth());
    auto h = roundToInt (scaleFactor * (f32) r.getHeight());

    Image image (flags.opaqueFlag ? Image::RGB : Image::ARGB, w, h, true);

    Graphics g (image);

    if (w != getWidth() || h != getHeight())
        g.addTransform (AffineTransform::scale ((f32) w / (f32) r.getWidth(),
                                                (f32) h / (f32) r.getHeight()));
    g.setOrigin (-r.getPosition());

    paintEntireComponent (g, true);

    return image;
}

ImageEffectFilter* Component::getComponentEffect() const noexcept
{
    return effectState != nullptr ? &effectState->getEffect() : nullptr;
}

z0 Component::setComponentEffect (ImageEffectFilter* newEffect)
{
    if (newEffect == nullptr && effectState == nullptr)
        return;

    const auto needsRepaint = [&]
    {
        if (newEffect == nullptr)
        {
            effectState.reset();
            return true;
        }

        if (effectState == nullptr)
        {
            effectState = std::make_unique<EffectState> (*newEffect);
            return true;
        }

        return effectState->setEffect (*newEffect);
    }();

    if (needsRepaint)
        repaint();
}

//==============================================================================
LookAndFeel& Component::getLookAndFeel() const noexcept
{
    for (auto* c = this; c != nullptr; c = c->parentComponent)
        if (auto lf = c->lookAndFeel.get())
            return *lf;

    return LookAndFeel::getDefaultLookAndFeel();
}

z0 Component::setLookAndFeel (LookAndFeel* newLookAndFeel)
{
    if (lookAndFeel != newLookAndFeel)
    {
        lookAndFeel = newLookAndFeel;
        sendLookAndFeelChange();
    }
}

FontOptions Component::withDefaultMetrics (FontOptions opt) const
{
    return getLookAndFeel().withDefaultMetrics (std::move (opt));
}

z0 Component::lookAndFeelChanged() {}
z0 Component::colourChanged() {}

z0 Component::sendLookAndFeelChange()
{
    const WeakReference<Component> safePointer (this);
    repaint();
    lookAndFeelChanged();

    if (safePointer != nullptr)
    {
        colourChanged();

        if (safePointer != nullptr)
        {
            for (i32 i = childComponentList.size(); --i >= 0;)
            {
                childComponentList.getUnchecked (i)->sendLookAndFeelChange();

                if (safePointer == nullptr)
                    return;

                i = jmin (i, childComponentList.size());
            }
        }
    }
}

Color Component::findColor (i32 colourID, b8 inheritFromParent) const
{
    if (auto* v = properties.getVarPointer (detail::ComponentHelpers::getColorPropertyID (colourID)))
        return Color ((u32) static_cast<i32> (*v));

    if (inheritFromParent && parentComponent != nullptr
         && (lookAndFeel == nullptr || ! lookAndFeel->isColorSpecified (colourID)))
        return parentComponent->findColor (colourID, true);

    return getLookAndFeel().findColor (colourID);
}

b8 Component::isColorSpecified (i32 colourID) const
{
    return properties.contains (detail::ComponentHelpers::getColorPropertyID (colourID));
}

z0 Component::removeColor (i32 colourID)
{
    if (properties.remove (detail::ComponentHelpers::getColorPropertyID (colourID)))
        colourChanged();
}

z0 Component::setColor (i32 colourID, Color colour)
{
    if (properties.set (detail::ComponentHelpers::getColorPropertyID (colourID), (i32) colour.getARGB()))
        colourChanged();
}

z0 Component::copyAllExplicitColorsTo (Component& target) const
{
    b8 changed = false;

    for (i32 i = properties.size(); --i >= 0;)
    {
        auto name = properties.getName (i);

        if (name.toString().startsWith (detail::colourPropertyPrefix))
            if (target.properties.set (name, properties [name]))
                changed = true;
    }

    if (changed)
        target.colourChanged();
}

//==============================================================================
Component::Positioner::Positioner (Component& c) noexcept  : component (c)
{
}

Component::Positioner* Component::getPositioner() const noexcept
{
    return positioner.get();
}

z0 Component::setPositioner (Positioner* newPositioner)
{
    // You can only assign a positioner to the component that it was created for!
    jassert (newPositioner == nullptr || this == &(newPositioner->getComponent()));
    positioner.reset (newPositioner);
}

//==============================================================================
Rectangle<i32> Component::getLocalBounds() const noexcept
{
    return boundsRelativeToParent.withZeroOrigin();
}

Rectangle<i32> Component::getBoundsInParent() const noexcept
{
    return affineTransform == nullptr ? boundsRelativeToParent
                                      : boundsRelativeToParent.transformedBy (*affineTransform);
}

//==============================================================================
z0 Component::mouseEnter (const MouseEvent&)          {}
z0 Component::mouseExit  (const MouseEvent&)          {}
z0 Component::mouseDown  (const MouseEvent&)          {}
z0 Component::mouseUp    (const MouseEvent&)          {}
z0 Component::mouseDrag  (const MouseEvent&)          {}
z0 Component::mouseMove  (const MouseEvent&)          {}
z0 Component::mouseDoubleClick (const MouseEvent&)    {}

z0 Component::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    // the base class just passes this event up to the nearest enabled ancestor
    if (auto* enabledComponent = findFirstEnabledAncestor (getParentComponent()))
        enabledComponent->mouseWheelMove (e.getEventRelativeTo (enabledComponent), wheel);
}

z0 Component::mouseMagnify (const MouseEvent& e, f32 magnifyAmount)
{
    // the base class just passes this event up to the nearest enabled ancestor
    if (auto* enabledComponent = findFirstEnabledAncestor (getParentComponent()))
        enabledComponent->mouseMagnify (e.getEventRelativeTo (enabledComponent), magnifyAmount);
}

//==============================================================================
z0 Component::resized()                       {}
z0 Component::moved()                         {}
z0 Component::childBoundsChanged (Component*) {}
z0 Component::parentSizeChanged()             {}

z0 Component::addComponentListener (ComponentListener* newListener)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
   #if DRX_DEBUG || DRX_LOG_ASSERTIONS
    if (getParentComponent() != nullptr)
    {
        DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    }
   #endif

    componentListeners.add (newListener);
}

z0 Component::removeComponentListener (ComponentListener* listenerToRemove)
{
    componentListeners.remove (listenerToRemove);
}

//==============================================================================
z0 Component::inputAttemptWhenModal()
{
    ModalComponentManager::getInstance()->bringModalComponentsToFront();
    getLookAndFeel().playAlertSound();
}

b8 Component::canModalEventBeSentToComponent (const Component*)
{
    return false;
}

z0 Component::internalModalInputAttempt()
{
    if (auto* current = getCurrentlyModalComponent())
        current->inputAttemptWhenModal();
}

//==============================================================================
z0 Component::postCommandMessage (i32 commandID)
{
    MessageManager::callAsync ([target = WeakReference<Component> { this }, commandID]
    {
        if (target != nullptr)
            target->handleCommandMessage (commandID);
    });
}

z0 Component::handleCommandMessage (i32)
{
    // used by subclasses
}

//==============================================================================
z0 Component::addMouseListener (MouseListener* newListener,
                                  b8 wantsEventsForAllNestedChildComponents)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    // If you register a component as a mouselistener for itself, it'll receive all the events
    // twice - once via the direct callback that all components get anyway, and then again as a listener!
    jassert ((newListener != this) || wantsEventsForAllNestedChildComponents);

    if (mouseListeners == nullptr)
        mouseListeners.reset (new MouseListenerList());

    mouseListeners->addListener (newListener, wantsEventsForAllNestedChildComponents);
}

z0 Component::removeMouseListener (MouseListener* listenerToRemove)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (mouseListeners != nullptr)
        mouseListeners->removeListener (listenerToRemove);
}

//==============================================================================
z0 Component::internalMouseEnter (SafePointer<Component> target, MouseInputSource source, Point<f32> relativePos, Time time)
{
    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // if something else is modal, always just show a normal mouse cursor
        source.showMouseCursor (MouseCursor::NormalCursor);
        return;
    }

    if (target->flags.repaintOnMouseActivityFlag)
        target->repaint();

    const auto me = makeMouseEvent (source,
                                    detail::PointerState().withPosition (relativePos),
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePos,
                                    time,
                                    0,
                                    false);

    HierarchyChecker checker (&target, me);
    target->mouseEnter (me);

    if (checker.shouldBailOut())
        return;

    target->flags.cachedMouseInsideComponent = true;

    if (checker.shouldBailOut())
        return;

    Desktop::getInstance().getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseEnter (me); });
    MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseEnter);
}

z0 Component::internalMouseExit (SafePointer<Component> target, MouseInputSource source, Point<f32> relativePos, Time time)
{
    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // if something else is modal, always just show a normal mouse cursor
        source.showMouseCursor (MouseCursor::NormalCursor);
        return;
    }

    if (target->flags.repaintOnMouseActivityFlag)
        target->repaint();

    target->flags.cachedMouseInsideComponent = false;

    const auto me = makeMouseEvent (source,
                                    detail::PointerState().withPosition (relativePos),
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePos,
                                    time,
                                    0,
                                    false);

    HierarchyChecker checker (&target, me);
    target->mouseExit (me);

    if (checker.shouldBailOut())
        return;

    Desktop::getInstance().getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseExit (me); });
    MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseExit);
}

z0 Component::internalMouseDown (SafePointer<Component> target,
                                   MouseInputSource source,
                                   const detail::PointerState& relativePointerState,
                                   Time time)
{
    auto& desktop = Desktop::getInstance();

    const auto me = makeMouseEvent (source,
                                    relativePointerState,
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePointerState.position,
                                    time,
                                    source.getNumberOfMultipleClicks(),
                                    false);

    HierarchyChecker checker (&target, me);

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        target->flags.mouseDownWasBlocked = true;
        target->internalModalInputAttempt();

        if (checker.shouldBailOut())
            return;

        // If processing the input attempt has exited the modal loop, we'll allow the event
        // to be delivered..
        if (target->isCurrentlyBlockedByAnotherModalComponent())
        {
            // allow blocked mouse-events to go to global listeners..
            desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseDown (checker.eventWithNearestParent()); });
            return;
        }
    }

    target->flags.mouseDownWasBlocked = false;

    checker.forEach ([] (auto& comp)
    {
        if (comp.isBroughtToFrontOnMouseClick())
            comp.toFront (true);
    });

    if (checker.shouldBailOut())
        return;

    target->grabKeyboardFocusInternal (focusChangedByMouseClick, true, FocusChangeDirection::unknown);

    if (checker.shouldBailOut())
        return;

    if (target->flags.repaintOnMouseActivityFlag)
        target->repaint();

    target->mouseDown (me);

    if (checker.shouldBailOut())
        return;

    desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseDown (checker.eventWithNearestParent()); });

    MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseDown);
}

z0 Component::internalMouseUp (SafePointer<Component> target,
                                 MouseInputSource source,
                                 const detail::PointerState& relativePointerState,
                                 Time time,
                                 const ModifierKeys oldModifiers)
{
    const auto originalTarget = target;

    const auto me = makeMouseEvent (source,
                                    relativePointerState,
                                    oldModifiers,
                                    target,
                                    target,
                                    time,
                                    target->getLocalPoint (nullptr, source.getLastMouseDownPosition()),
                                    source.getLastMouseDownTime(),
                                    source.getNumberOfMultipleClicks(),
                                    source.isLongPressOrDrag());

    HierarchyChecker checker (&target, me);

    if (target->flags.mouseDownWasBlocked && target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // Global listeners still need to know about the mouse up
        auto& desktop = Desktop::getInstance();
        desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseUp (checker.eventWithNearestParent()); });
        return;
    }

    if (target->flags.repaintOnMouseActivityFlag)
        target->repaint();

    target->mouseUp (me);

    if (checker.shouldBailOut())
        return;

    auto& desktop = Desktop::getInstance();
    desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseUp (checker.eventWithNearestParent()); });

    MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseUp);

    if (checker.shouldBailOut())
        return;

    // check for f64-click
    if (me.getNumberOfClicks() >= 2)
    {
        if (target == originalTarget)
            target->mouseDoubleClick (checker.eventWithNearestParent());

        if (checker.shouldBailOut())
            return;

        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseDoubleClick (checker.eventWithNearestParent()); });
        MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseDoubleClick);
    }
}

z0 Component::internalMouseDrag (SafePointer<Component> target, MouseInputSource source, const detail::PointerState& relativePointerState, Time time)
{
    if (! target->isCurrentlyBlockedByAnotherModalComponent())
    {
        const auto me = makeMouseEvent (source,
                                        relativePointerState,
                                        source.getCurrentModifiers(),
                                        target,
                                        target,
                                        time,
                                        target->getLocalPoint (nullptr, source.getLastMouseDownPosition()),
                                        source.getLastMouseDownTime(),
                                        source.getNumberOfMultipleClicks(),
                                        source.isLongPressOrDrag());

        HierarchyChecker checker (&target, me);

        target->mouseDrag (me);

        if (checker.shouldBailOut())
            return;

        Desktop::getInstance().getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseDrag (checker.eventWithNearestParent()); });
        MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseDrag);
    }
}

z0 Component::internalMouseMove (SafePointer<Component> target, MouseInputSource source, Point<f32> relativePos, Time time)
{
    auto& desktop = Desktop::getInstance();

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        desktop.sendMouseMove();
    }
    else
    {
        const auto me = makeMouseEvent (source,
                                        detail::PointerState().withPosition (relativePos),
                                        source.getCurrentModifiers(),
                                        target,
                                        target,
                                        time,
                                        relativePos,
                                        time,
                                        0,
                                        false);

        HierarchyChecker checker (&target, me);

        target->mouseMove (me);

        if (checker.shouldBailOut())
            return;

        desktop.getMouseListeners().callChecked (checker, [&] (MouseListener& l) { l.mouseMove (checker.eventWithNearestParent()); });
        MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseMove);
    }
}

z0 Component::internalMouseWheel (SafePointer<Component> target, MouseInputSource source, Point<f32> relativePos,
                                    Time time, const MouseWheelDetails& wheel)
{
    auto& desktop = Desktop::getInstance();

    const auto me = makeMouseEvent (source,
                                    detail::PointerState().withPosition (relativePos),
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePos,
                                    time,
                                    0,
                                    false);

    HierarchyChecker checker (&target, me);

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseWheelMove (me, wheel); });
    }
    else
    {
        target->mouseWheelMove (me, wheel);

        if (checker.shouldBailOut())
            return;

        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseWheelMove (checker.eventWithNearestParent(), wheel); });

        if (! checker.shouldBailOut())
            MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseWheelMove, wheel);
    }
}

z0 Component::internalMagnifyGesture (SafePointer<Component> target, MouseInputSource source, Point<f32> relativePos,
                                        Time time, f32 amount)
{
    auto& desktop = Desktop::getInstance();

    const auto me = makeMouseEvent (source,
                                    detail::PointerState().withPosition (relativePos),
                                    source.getCurrentModifiers(),
                                    target,
                                    target,
                                    time,
                                    relativePos,
                                    time,
                                    0,
                                    false);

    HierarchyChecker checker (&target, me);

    if (target->isCurrentlyBlockedByAnotherModalComponent())
    {
        // allow blocked mouse-events to go to global listeners..
        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseMagnify (me, amount); });
    }
    else
    {
        target->mouseMagnify (me, amount);

        if (checker.shouldBailOut())
            return;

        desktop.mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseMagnify (checker.eventWithNearestParent(), amount); });

        if (! checker.shouldBailOut())
            MouseListenerList::sendMouseEvent (checker, &MouseListener::mouseMagnify, amount);
    }
}

z0 Component::sendFakeMouseMove() const
{
    if (flags.ignoresMouseClicksFlag && ! flags.allowChildMouseClicksFlag)
        return;

    auto mainMouse = Desktop::getInstance().getMainMouseSource();

    if (! mainMouse.isDragging())
        mainMouse.triggerFakeMove();
}

z0 DRX_CALLTYPE Component::beginDragAutoRepeat (i32 interval)
{
    Desktop::getInstance().beginDragAutoRepeat (interval);
}

//==============================================================================
z0 Component::broughtToFront()
{
}

z0 Component::internalBroughtToFront()
{
    if (flags.hasHeavyweightPeerFlag)
        Desktop::getInstance().componentBroughtToFront (this);

    BailOutChecker checker (this);
    broughtToFront();

    if (checker.shouldBailOut())
        return;

    componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentBroughtToFront (*this); });

    if (checker.shouldBailOut())
        return;

    // When brought to the front and there's a modal component blocking this one,
    // we need to bring the modal one to the front instead..
    if (auto* cm = getCurrentlyModalComponent())
        if (cm->getTopLevelComponent() != getTopLevelComponent())
            ModalComponentManager::getInstance()->bringModalComponentsToFront (false); // very important that this is false, otherwise in Windows,
                                                                                       // non-front components can't get focus when another modal comp is
                                                                                       // active, and therefore can't receive mouse-clicks
}

//==============================================================================
z0 Component::focusGained (FocusChangeType)   {}
z0 Component::focusGainedWithDirection (FocusChangeType, FocusChangeDirection) {}
z0 Component::focusLost (FocusChangeType)     {}
z0 Component::focusOfChildComponentChanged (FocusChangeType) {}

z0 Component::internalKeyboardFocusGain (FocusChangeType cause)
{
    internalKeyboardFocusGain (cause, WeakReference<Component> (this), FocusChangeDirection::unknown);
}

z0 Component::internalKeyboardFocusGain (FocusChangeType cause,
                                           const WeakReference<Component>& safePointer,
                                           FocusChangeDirection direction)
{
    focusGainedWithDirection (cause, direction);
    focusGained (cause);

    if (safePointer == nullptr)
        return;

    if (hasKeyboardFocus (false))
        if (auto* handler = getAccessibilityHandler())
            handler->grabFocus();

    if (safePointer == nullptr)
        return;

    internalChildKeyboardFocusChange (cause, safePointer);
}

z0 Component::internalKeyboardFocusLoss (FocusChangeType cause)
{
    const WeakReference<Component> safePointer (this);

    focusLost (cause);

    if (safePointer != nullptr)
    {
        if (auto* handler = getAccessibilityHandler())
            handler->giveAwayFocus();

        internalChildKeyboardFocusChange (cause, safePointer);
    }
}

z0 Component::internalChildKeyboardFocusChange (FocusChangeType cause,
                                                  const WeakReference<Component>& safePointer)
{
    const b8 childIsNowKeyboardFocused = hasKeyboardFocus (true);

    if (flags.childKeyboardFocusedFlag != childIsNowKeyboardFocused)
    {
        flags.childKeyboardFocusedFlag = childIsNowKeyboardFocused;

        focusOfChildComponentChanged (cause);

        if (safePointer == nullptr)
            return;
    }

    if (parentComponent != nullptr)
        parentComponent->internalChildKeyboardFocusChange (cause, parentComponent);
}

z0 Component::setWantsKeyboardFocus (b8 wantsFocus) noexcept
{
    flags.wantsKeyboardFocusFlag = wantsFocus;
}

z0 Component::setMouseClickGrabsKeyboardFocus (b8 shouldGrabFocus)
{
    flags.dontFocusOnMouseClickFlag = ! shouldGrabFocus;
}

b8 Component::getMouseClickGrabsKeyboardFocus() const noexcept
{
    return ! flags.dontFocusOnMouseClickFlag;
}

b8 Component::getWantsKeyboardFocus() const noexcept
{
    return flags.wantsKeyboardFocusFlag && ! flags.isDisabledFlag;
}

z0 Component::setFocusContainerType (FocusContainerType containerType) noexcept
{
    flags.isFocusContainerFlag = (containerType == FocusContainerType::focusContainer
                                  || containerType == FocusContainerType::keyboardFocusContainer);

    flags.isKeyboardFocusContainerFlag = (containerType == FocusContainerType::keyboardFocusContainer);
}

b8 Component::isFocusContainer() const noexcept
{
    return flags.isFocusContainerFlag;
}

b8 Component::isKeyboardFocusContainer() const noexcept
{
    return flags.isKeyboardFocusContainerFlag;
}

template <typename FocusContainerFn>
static Component* findContainer (const Component* child, FocusContainerFn isFocusContainer)
{
    if (auto* parent = child->getParentComponent())
    {
        if ((parent->*isFocusContainer)() || parent->getParentComponent() == nullptr)
            return parent;

        return findContainer (parent, isFocusContainer);
    }

    return nullptr;
}

Component* Component::findFocusContainer() const
{
    return findContainer (this, &Component::isFocusContainer);
}

Component* Component::findKeyboardFocusContainer() const
{
    return findContainer (this, &Component::isKeyboardFocusContainer);
}

static const Identifier explicitFocusOrderId ("_jexfo");

i32 Component::getExplicitFocusOrder() const
{
    return properties [explicitFocusOrderId];
}

z0 Component::setExplicitFocusOrder (i32 newFocusOrderIndex)
{
    properties.set (explicitFocusOrderId, newFocusOrderIndex);
}

std::unique_ptr<ComponentTraverser> Component::createFocusTraverser()
{
    if (flags.isFocusContainerFlag || parentComponent == nullptr)
        return std::make_unique<FocusTraverser>();

    return parentComponent->createFocusTraverser();
}

std::unique_ptr<ComponentTraverser> Component::createKeyboardFocusTraverser()
{
    if (flags.isKeyboardFocusContainerFlag || parentComponent == nullptr)
        return std::make_unique<KeyboardFocusTraverser>();

    return parentComponent->createKeyboardFocusTraverser();
}

z0 Component::takeKeyboardFocus (FocusChangeType cause, FocusChangeDirection direction)
{
    if (currentlyFocusedComponent == this)
        return;

    if (auto* peer = getPeer())
    {
        const WeakReference<Component> safePointer (this);
        peer->grabFocus();

        if (! peer->isFocused() || currentlyFocusedComponent == this)
            return;

        WeakReference<Component> componentLosingFocus (currentlyFocusedComponent);

        if (auto* losingFocus = componentLosingFocus.get())
            if (auto* otherPeer = losingFocus->getPeer())
                otherPeer->closeInputMethodContext();

        currentlyFocusedComponent = this;

        Desktop::getInstance().triggerFocusCallback();

        // call this after setting currentlyFocusedComponent so that the one that's
        // losing it has a chance to see where focus is going
        if (componentLosingFocus != nullptr)
            componentLosingFocus->internalKeyboardFocusLoss (cause);

        if (currentlyFocusedComponent == this)
            internalKeyboardFocusGain (cause, safePointer, direction);
    }
}

z0 Component::grabKeyboardFocusInternal (FocusChangeType cause, b8 canTryParent, FocusChangeDirection direction)
{
    if (flags.dontFocusOnMouseClickFlag && cause == FocusChangeType::focusChangedByMouseClick)
        return;

    if (! isShowing())
        return;

    if (flags.wantsKeyboardFocusFlag
        && (isEnabled() || parentComponent == nullptr))
    {
        takeKeyboardFocus (cause, direction);
        return;
    }

    if (isParentOf (currentlyFocusedComponent) && currentlyFocusedComponent->isShowing())
        return;

    if (auto traverser = createKeyboardFocusTraverser())
    {
        if (auto* defaultComp = traverser->getDefaultComponent (this))
        {
            defaultComp->grabKeyboardFocusInternal (cause, false, direction);
            return;
        }
    }

    // if no children want it and we're allowed to try our parent comp,
    // then pass up to parent, which will try our siblings.
    if (canTryParent && parentComponent != nullptr)
        parentComponent->grabKeyboardFocusInternal (cause, true, direction);
}

z0 Component::grabKeyboardFocus()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    grabKeyboardFocusInternal (focusChangedDirectly, true, FocusChangeDirection::unknown);

    // A component can only be focused when it's actually on the screen!
    // If this fails then you're probably trying to grab the focus before you've
    // added the component to a parent or made it visible. Or maybe one of its parent
    // components isn't yet visible.
    jassert (isShowing() || isOnDesktop());
}

z0 Component::giveAwayKeyboardFocusInternal (b8 sendFocusLossEvent)
{
    if (hasKeyboardFocus (true))
    {
        if (auto* componentLosingFocus = currentlyFocusedComponent)
        {
            if (auto* otherPeer = componentLosingFocus->getPeer())
                otherPeer->closeInputMethodContext();

            currentlyFocusedComponent = nullptr;

            if (sendFocusLossEvent && componentLosingFocus != nullptr)
                componentLosingFocus->internalKeyboardFocusLoss (focusChangedDirectly);

            Desktop::getInstance().triggerFocusCallback();
        }
    }
}

z0 Component::giveAwayKeyboardFocus()
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    giveAwayKeyboardFocusInternal (true);
}

z0 Component::moveKeyboardFocusToSibling (b8 moveToNext)
{
    // if component methods are being called from threads other than the message
    // thread, you'll need to use a MessageManagerLock object to make sure it's thread-safe.
    DRX_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (parentComponent != nullptr)
    {
        if (auto traverser = createKeyboardFocusTraverser())
        {
            auto findComponentToFocus = [&]() -> Component*
            {
                if (auto* comp = (moveToNext ? traverser->getNextComponent (this)
                                             : traverser->getPreviousComponent (this)))
                    return comp;

                if (auto* focusContainer = findKeyboardFocusContainer())
                {
                    auto allFocusableComponents = traverser->getAllComponents (focusContainer);

                    if (! allFocusableComponents.empty())
                        return moveToNext ? allFocusableComponents.front()
                                          : allFocusableComponents.back();
                }

                return nullptr;
            };

            if (auto* nextComp = findComponentToFocus())
            {
                if (nextComp->isCurrentlyBlockedByAnotherModalComponent())
                {
                    const WeakReference<Component> nextCompPointer (nextComp);
                    internalModalInputAttempt();

                    if (nextCompPointer == nullptr || nextComp->isCurrentlyBlockedByAnotherModalComponent())
                        return;
                }

                nextComp->grabKeyboardFocusInternal (focusChangedByTabKey,
                                                     true,
                                                     moveToNext ? FocusChangeDirection::forward
                                                                : FocusChangeDirection::backward);
                return;
            }
        }

        parentComponent->moveKeyboardFocusToSibling (moveToNext);
    }
}

b8 Component::hasKeyboardFocus (b8 trueIfChildIsFocused) const
{
    return (currentlyFocusedComponent == this)
            || (trueIfChildIsFocused && isParentOf (currentlyFocusedComponent));
}

Component* DRX_CALLTYPE Component::getCurrentlyFocusedComponent() noexcept
{
    return currentlyFocusedComponent;
}

z0 DRX_CALLTYPE Component::unfocusAllComponents()
{
    if (currentlyFocusedComponent != nullptr)
        currentlyFocusedComponent->giveAwayKeyboardFocus();
}

//==============================================================================
b8 Component::isEnabled() const noexcept
{
    return (! flags.isDisabledFlag)
            && (parentComponent == nullptr || parentComponent->isEnabled());
}

z0 Component::setEnabled (b8 shouldBeEnabled)
{
    if (flags.isDisabledFlag == shouldBeEnabled)
    {
        flags.isDisabledFlag = ! shouldBeEnabled;

        // if any parent components are disabled, setting our flag won't make a difference,
        // so no need to send a change message
        if (parentComponent == nullptr || parentComponent->isEnabled())
            sendEnablementChangeMessage();

        BailOutChecker checker (this);
        componentListeners.callChecked (checker, [this] (ComponentListener& l) { l.componentEnablementChanged (*this); });

        if (! shouldBeEnabled && hasKeyboardFocus (true))
        {
            if (parentComponent != nullptr)
                parentComponent->grabKeyboardFocus();

            // ensure that keyboard focus is given away if it wasn't taken by parent
            giveAwayKeyboardFocus();
        }
    }
}

z0 Component::enablementChanged() {}

z0 Component::sendEnablementChangeMessage()
{
    const WeakReference<Component> safePointer (this);

    enablementChanged();

    if (safePointer == nullptr)
        return;

    for (i32 i = getNumChildComponents(); --i >= 0;)
    {
        if (auto* c = getChildComponent (i))
        {
            c->sendEnablementChangeMessage();

            if (safePointer == nullptr)
                return;
        }
    }
}

//==============================================================================
b8 Component::isMouseOver (b8 includeChildren) const
{
    if (! MessageManager::getInstance()->isThisTheMessageThread())
        return flags.cachedMouseInsideComponent;

    for (auto& ms : Desktop::getInstance().getMouseSources())
    {
        auto* c = ms.getComponentUnderMouse();

        if (c != nullptr && (c == this || (includeChildren && isParentOf (c))))
            if (ms.isDragging() || ! (ms.isTouch() || ms.isPen()))
                if (c->reallyContains (c->getLocalPoint (nullptr, ms.getScreenPosition()), false))
                    return true;
    }

    return false;
}

b8 Component::isMouseButtonDown (b8 includeChildren) const
{
    for (auto& ms : Desktop::getInstance().getMouseSources())
    {
        auto* c = ms.getComponentUnderMouse();

        if (c == this || (includeChildren && isParentOf (c)))
            if (ms.isDragging())
                return true;
    }

    return false;
}

b8 Component::isMouseOverOrDragging (b8 includeChildren) const
{
    for (auto& ms : Desktop::getInstance().getMouseSources())
    {
        auto* c = ms.getComponentUnderMouse();

        if (c == this || (includeChildren && isParentOf (c)))
            if (ms.isDragging() || ! ms.isTouch())
                return true;
    }

    return false;
}

b8 DRX_CALLTYPE Component::isMouseButtonDownAnywhere() noexcept
{
    return ModifierKeys::currentModifiers.isAnyMouseButtonDown();
}

Point<i32> Component::getMouseXYRelative() const
{
    return getLocalPoint (nullptr, Desktop::getMousePositionFloat()).roundToInt();
}

//==============================================================================
z0 Component::addKeyListener (KeyListener* newListener)
{
    if (keyListeners == nullptr)
        keyListeners.reset (new Array<KeyListener*>());

    keyListeners->addIfNotAlreadyThere (newListener);
}

z0 Component::removeKeyListener (KeyListener* listenerToRemove)
{
    if (keyListeners != nullptr)
        keyListeners->removeFirstMatchingValue (listenerToRemove);
}

b8 Component::keyPressed (const KeyPress&)            { return false; }
b8 Component::keyStateChanged (b8 /*isKeyDown*/)    { return false; }

z0 Component::modifierKeysChanged (const ModifierKeys& modifiers)
{
    if (parentComponent != nullptr)
        parentComponent->modifierKeysChanged (modifiers);
}

z0 Component::internalModifierKeysChanged()
{
    sendFakeMouseMove();
    modifierKeysChanged (ModifierKeys::currentModifiers);
}

//==============================================================================
Component::BailOutChecker::BailOutChecker (Component* component)
    : safePointer (component)
{
    jassert (component != nullptr);
}

b8 Component::BailOutChecker::shouldBailOut() const noexcept
{
    return safePointer == nullptr;
}

//==============================================================================
z0 Component::setTitle (const Txt& newTitle)
{
    componentTitle = newTitle;
}

z0 Component::setDescription (const Txt& newDescription)
{
    componentDescription = newDescription;
}

z0 Component::setHelpText (const Txt& newHelpText)
{
    componentHelpText = newHelpText;
}

z0 Component::setAccessible (b8 shouldBeAccessible)
{
    flags.accessibilityIgnoredFlag = ! shouldBeAccessible;

    if (flags.accessibilityIgnoredFlag)
        invalidateAccessibilityHandler();
}

b8 Component::isAccessible() const noexcept
{
    return (! flags.accessibilityIgnoredFlag
            && (parentComponent == nullptr || parentComponent->isAccessible()));
}

std::unique_ptr<AccessibilityHandler> Component::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::unspecified);
}

std::unique_ptr<AccessibilityHandler> Component::createIgnoredAccessibilityHandler (Component& comp)
{
    return std::make_unique<AccessibilityHandler> (comp, AccessibilityRole::ignored);
}

z0 Component::invalidateAccessibilityHandler()
{
    accessibilityHandler = nullptr;
}

AccessibilityHandler* Component::getAccessibilityHandler()
{
    if (! isAccessible() || getWindowHandle() == nullptr)
        return nullptr;

    if (accessibilityHandler == nullptr
        || accessibilityHandler->getTypeIndex() != std::type_index (typeid (*this)))
    {
        accessibilityHandler = createAccessibilityHandler();

        // On Android, notifying that an element was created can cause the system to request
        // the accessibility node info for the new element. If we're not careful, this will lead
        // to recursive calls, as each time an element is created, new node info will be requested,
        // causing an element to be created, causing a new info request...
        // By assigning the accessibility handler before notifying the system that an element was
        // created, the if() predicate above should evaluate to false on recursive calls,
        // terminating the recursion.
        if (accessibilityHandler != nullptr)
            detail::AccessibilityHelpers::notifyAccessibilityEvent (*accessibilityHandler, detail::AccessibilityHelpers::Event::elementCreated);
        else
            jassertfalse; // createAccessibilityHandler must return non-null
    }

    return accessibilityHandler.get();
}

} // namespace drx
