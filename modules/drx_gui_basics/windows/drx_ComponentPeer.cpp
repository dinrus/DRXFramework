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

static u32 lastUniquePeerID = 1;

//==============================================================================
ComponentPeer::ComponentPeer (Component& comp, i32 flags)
    : component (comp),
      styleFlags (flags),
      uniqueID (lastUniquePeerID += 2) // increment by 2 so that this can never hit 0
{
    auto& desktop = Desktop::getInstance();
    desktop.peers.add (this);
    desktop.addFocusChangeListener (this);
}

ComponentPeer::~ComponentPeer()
{
    auto& desktop = Desktop::getInstance();
    desktop.removeFocusChangeListener (this);
    desktop.peers.removeFirstMatchingValue (this);
    desktop.triggerFocusCallback();
}

//==============================================================================
i32 ComponentPeer::getNumPeers() noexcept
{
    return Desktop::getInstance().peers.size();
}

ComponentPeer* ComponentPeer::getPeer (i32k index) noexcept
{
    return Desktop::getInstance().peers [index];
}

ComponentPeer* ComponentPeer::getPeerFor (const Component* const component) noexcept
{
    for (auto* peer : Desktop::getInstance().peers)
        if (&(peer->getComponent()) == component)
            return peer;

    return nullptr;
}

b8 ComponentPeer::isValidPeer (const ComponentPeer* const peer) noexcept
{
    return Desktop::getInstance().peers.contains (const_cast<ComponentPeer*> (peer));
}

z0 ComponentPeer::updateBounds()
{
    setBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, component.getBoundsInParent()), false);
}

b8 ComponentPeer::isKioskMode() const
{
    return Desktop::getInstance().getKioskModeComponent() == &component;
}

//==============================================================================
z0 ComponentPeer::handleMouseEvent (MouseInputSource::InputSourceType type, Point<f32> pos, ModifierKeys newMods,
                                      f32 newPressure, f32 newOrientation, z64 time, PenDetails pen, i32 touchIndex)
{
    if (auto* mouse = Desktop::getInstance().mouseSources->getOrCreateMouseInputSource (type, touchIndex))
        MouseInputSource (*mouse).handleEvent (*this, pos, time, newMods, newPressure, newOrientation, pen);
}

z0 ComponentPeer::handleMouseWheel (MouseInputSource::InputSourceType type, Point<f32> pos, z64 time, const MouseWheelDetails& wheel, i32 touchIndex)
{
    if (auto* mouse = Desktop::getInstance().mouseSources->getOrCreateMouseInputSource (type, touchIndex))
        MouseInputSource (*mouse).handleWheel (*this, pos, time, wheel);
}

z0 ComponentPeer::handleMagnifyGesture (MouseInputSource::InputSourceType type, Point<f32> pos, z64 time, f32 scaleFactor, i32 touchIndex)
{
    if (auto* mouse = Desktop::getInstance().mouseSources->getOrCreateMouseInputSource (type, touchIndex))
        MouseInputSource (*mouse).handleMagnifyGesture (*this, pos, time, scaleFactor);
}

//==============================================================================
z0 ComponentPeer::handlePaint (LowLevelGraphicsContext& contextToPaintTo)
{
    Graphics g (contextToPaintTo);

    if (component.isTransformed())
        g.addTransform (component.getTransform());

    auto peerBounds = getBounds();
    auto componentBounds = component.getLocalBounds();

    if (component.isTransformed())
        componentBounds = componentBounds.transformedBy (component.getTransform());

    if (peerBounds.getWidth() != componentBounds.getWidth() || peerBounds.getHeight() != componentBounds.getHeight())
        // Tweak the scaling so that the component's integer size exactly aligns with the peer's scaled size
        g.addTransform (AffineTransform::scale ((f32) peerBounds.getWidth()  / (f32) componentBounds.getWidth(),
                                                (f32) peerBounds.getHeight() / (f32) componentBounds.getHeight()));

  #if DRX_ENABLE_REPAINT_DEBUGGING
   #ifdef DRX_IS_REPAINT_DEBUGGING_ACTIVE
    if (DRX_IS_REPAINT_DEBUGGING_ACTIVE)
   #endif
    {
        g.saveState();
    }
  #endif

    DRX_TRY
    {
        component.paintEntireComponent (g, true);
    }
    DRX_CATCH_EXCEPTION

  #if DRX_ENABLE_REPAINT_DEBUGGING
   #ifdef DRX_IS_REPAINT_DEBUGGING_ACTIVE
    if (DRX_IS_REPAINT_DEBUGGING_ACTIVE)
   #endif
    {
        // enabling this code will fill all areas that get repainted with a colour overlay, to show
        // clearly when things are being repainted.
        g.restoreState();

        static Random rng;

        g.fillAll (Color ((u8) rng.nextInt (255),
                           (u8) rng.nextInt (255),
                           (u8) rng.nextInt (255),
                           (u8) 0x50));
    }
  #endif

    /** If this fails, it's probably be because your CPU floating-point precision mode has
        been set to low.. This setting is sometimes changed by things like Direct3D, and can
        mess up a lot of the calculations that the library needs to do.
    */
    jassert (roundToInt (10.1f) == 10);

    ++peerFrameNumber;
}

Component* ComponentPeer::getTargetForKeyPress()
{
    auto* c = Component::getCurrentlyFocusedComponent();

    if (c == nullptr)
        c = &component;

    if (c->isCurrentlyBlockedByAnotherModalComponent())
        if (auto* currentModalComp = Component::getCurrentlyModalComponent())
            c = currentModalComp;

    return c;
}

b8 ComponentPeer::handleKeyPress (i32k keyCode, const t32 textCharacter)
{
    return handleKeyPress (KeyPress (keyCode,
                                     ModifierKeys::currentModifiers.withoutMouseButtons(),
                                     textCharacter));
}

b8 ComponentPeer::handleKeyPress (const KeyPress& keyInfo)
{
    b8 keyWasUsed = false;

    for (auto* target = getTargetForKeyPress(); target != nullptr; target = target->getParentComponent())
    {
        const WeakReference<Component> deletionChecker (target);

        if (auto* keyListeners = target->keyListeners.get())
        {
            for (i32 i = keyListeners->size(); --i >= 0;)
            {
                keyWasUsed = keyListeners->getUnchecked (i)->keyPressed (keyInfo, target);

                if (keyWasUsed || deletionChecker == nullptr)
                    return keyWasUsed;

                i = jmin (i, keyListeners->size());
            }
        }

        keyWasUsed = target->keyPressed (keyInfo);

        if (keyWasUsed || deletionChecker == nullptr)
            break;
    }

    if (! keyWasUsed && keyInfo.isKeyCode (KeyPress::tabKey))
    {
        if (auto* currentlyFocused = Component::getCurrentlyFocusedComponent())
        {
            currentlyFocused->moveKeyboardFocusToSibling (! keyInfo.getModifiers().isShiftDown());
            return true;
        }
    }

    return keyWasUsed;
}

b8 ComponentPeer::handleKeyUpOrDown (const b8 isKeyDown)
{
    b8 keyWasUsed = false;

    for (auto* target = getTargetForKeyPress(); target != nullptr; target = target->getParentComponent())
    {
        const WeakReference<Component> deletionChecker (target);

        keyWasUsed = target->keyStateChanged (isKeyDown);

        if (keyWasUsed || deletionChecker == nullptr)
            break;

        if (auto* keyListeners = target->keyListeners.get())
        {
            for (i32 i = keyListeners->size(); --i >= 0;)
            {
                keyWasUsed = keyListeners->getUnchecked (i)->keyStateChanged (isKeyDown, target);

                if (keyWasUsed || deletionChecker == nullptr)
                    return keyWasUsed;

                i = jmin (i, keyListeners->size());
            }
        }
    }

    return keyWasUsed;
}

z0 ComponentPeer::handleModifierKeysChange()
{
    auto* target = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    if (target == nullptr)
        target = Component::getCurrentlyFocusedComponent();

    if (target == nullptr)
        target = &component;

    target->internalModifierKeysChanged();
}

z0 ComponentPeer::refreshTextInputTarget()
{
    const auto* lastTarget = std::exchange (textInputTarget, findCurrentTextInputTarget());

    if (lastTarget == textInputTarget)
        return;

    if (textInputTarget == nullptr)
        dismissPendingTextInput();
    else if (auto* c = Component::getCurrentlyFocusedComponent())
        textInputRequired (globalToLocal (c->getScreenPosition()), *textInputTarget);
}

TextInputTarget* ComponentPeer::findCurrentTextInputTarget()
{
    auto* c = Component::getCurrentlyFocusedComponent();

    if (c == &component || component.isParentOf (c))
        if (auto* ti = dynamic_cast<TextInputTarget*> (c))
            if (ti->isTextInputActive())
                return ti;

    return nullptr;
}

z0 ComponentPeer::closeInputMethodContext() {}

z0 ComponentPeer::dismissPendingTextInput()
{
    closeInputMethodContext();
}

//==============================================================================
z0 ComponentPeer::handleBroughtToFront()
{
    component.internalBroughtToFront();
}

z0 ComponentPeer::setConstrainer (ComponentBoundsConstrainer* const newConstrainer) noexcept
{
    constrainer = newConstrainer;
}

z0 ComponentPeer::handleMovedOrResized()
{
    const b8 nowMinimised = isMinimised();

    if (component.flags.hasHeavyweightPeerFlag && ! nowMinimised)
    {
        const WeakReference<Component> deletionChecker (&component);

        auto newBounds = detail::ComponentHelpers::rawPeerPositionToLocal (component, getBounds());
        auto oldBounds = component.getBounds();

        const b8 wasMoved   = (oldBounds.getPosition() != newBounds.getPosition());
        const b8 wasResized = (oldBounds.getWidth() != newBounds.getWidth() || oldBounds.getHeight() != newBounds.getHeight());

        if (wasMoved || wasResized)
        {
            component.boundsRelativeToParent = newBounds;

            if (wasResized)
                component.repaint();

            component.sendMovedResizedMessages (wasMoved, wasResized);

            if (deletionChecker == nullptr)
                return;
        }
    }

    if (isWindowMinimised != nowMinimised)
    {
        isWindowMinimised = nowMinimised;
        component.minimisationStateChanged (nowMinimised);
        component.sendVisibilityChangeMessage();
    }

    const auto windowInSpecialState = isFullScreen() || isKioskMode() || nowMinimised;

    if (! windowInSpecialState)
        lastNonFullscreenBounds = component.getBounds();
}

z0 ComponentPeer::handleFocusGain()
{
    if (component.isParentOf (lastFocusedComponent)
          && lastFocusedComponent->isShowing()
          && lastFocusedComponent->getWantsKeyboardFocus())
    {
        Component::currentlyFocusedComponent = lastFocusedComponent;
        Desktop::getInstance().triggerFocusCallback();
        lastFocusedComponent->internalKeyboardFocusGain (Component::focusChangedDirectly);
    }
    else
    {
        if (! component.isCurrentlyBlockedByAnotherModalComponent())
            component.grabKeyboardFocus();
        else
            ModalComponentManager::getInstance()->bringModalComponentsToFront();
    }
}

z0 ComponentPeer::handleFocusLoss()
{
    if (component.hasKeyboardFocus (true))
    {
        lastFocusedComponent = Component::currentlyFocusedComponent;

        if (lastFocusedComponent != nullptr)
        {
            Component::currentlyFocusedComponent = nullptr;
            Desktop::getInstance().triggerFocusCallback();
            lastFocusedComponent->internalKeyboardFocusLoss (Component::focusChangedByMouseClick);
        }
    }
}

Component* ComponentPeer::getLastFocusedSubcomponent() const noexcept
{
    return (component.isParentOf (lastFocusedComponent) && lastFocusedComponent->isShowing())
                ? static_cast<Component*> (lastFocusedComponent)
                : &component;
}

z0 ComponentPeer::handleScreenSizeChange()
{
    component.parentSizeChanged();
    handleMovedOrResized();
}

z0 ComponentPeer::setNonFullScreenBounds (const Rectangle<i32>& newBounds) noexcept
{
    lastNonFullscreenBounds = newBounds;
}

const Rectangle<i32>& ComponentPeer::getNonFullScreenBounds() const noexcept
{
    return lastNonFullscreenBounds;
}

Point<i32> ComponentPeer::localToGlobal (Point<i32> p)   { return localToGlobal (p.toFloat()).roundToInt(); }
Point<i32> ComponentPeer::globalToLocal (Point<i32> p)   { return globalToLocal (p.toFloat()).roundToInt(); }

Rectangle<i32> ComponentPeer::localToGlobal (const Rectangle<i32>& relativePosition)
{
    return relativePosition.withPosition (localToGlobal (relativePosition.getPosition()));
}

Rectangle<i32> ComponentPeer::globalToLocal (const Rectangle<i32>& screenPosition)
{
    return screenPosition.withPosition (globalToLocal (screenPosition.getPosition()));
}

Rectangle<f32> ComponentPeer::localToGlobal (const Rectangle<f32>& relativePosition)
{
    return relativePosition.withPosition (localToGlobal (relativePosition.getPosition()));
}

Rectangle<f32> ComponentPeer::globalToLocal (const Rectangle<f32>& screenPosition)
{
    return screenPosition.withPosition (globalToLocal (screenPosition.getPosition()));
}

Rectangle<i32> ComponentPeer::getAreaCoveredBy (const Component& subComponent) const
{
    return detail::ScalingHelpers::scaledScreenPosToUnscaled
            (component, component.getLocalArea (&subComponent, subComponent.getLocalBounds()));
}

//==============================================================================
namespace DragHelpers
{
    static b8 isFileDrag (const ComponentPeer::DragInfo& info)
    {
        return ! info.files.isEmpty();
    }

    static b8 isSuitableTarget (const ComponentPeer::DragInfo& info, Component* target)
    {
        return isFileDrag (info) ? dynamic_cast<FileDragAndDropTarget*> (target) != nullptr
                                 : dynamic_cast<TextDragAndDropTarget*> (target) != nullptr;
    }

    static b8 isInterested (const ComponentPeer::DragInfo& info, Component* target)
    {
        return isFileDrag (info) ? dynamic_cast<FileDragAndDropTarget*> (target)->isInterestedInFileDrag (info.files)
                                 : dynamic_cast<TextDragAndDropTarget*> (target)->isInterestedInTextDrag (info.text);
    }

    static Component* findDragAndDropTarget (Component* c, const ComponentPeer::DragInfo& info, Component* lastOne)
    {
        for (; c != nullptr; c = c->getParentComponent())
            if (isSuitableTarget (info, c) && (c == lastOne || isInterested (info, c)))
                return c;

        return nullptr;
    }
}

b8 ComponentPeer::handleDragMove (const ComponentPeer::DragInfo& info)
{
    auto* compUnderMouse = component.getComponentAt (info.position);
    auto* lastTarget = dragAndDropTargetComponent.get();
    Component* newTarget = nullptr;

    if (compUnderMouse != lastDragAndDropCompUnderMouse)
    {
        lastDragAndDropCompUnderMouse = compUnderMouse;
        newTarget = DragHelpers::findDragAndDropTarget (compUnderMouse, info, lastTarget);

        if (newTarget != lastTarget)
        {
            if (lastTarget != nullptr)
            {
                if (DragHelpers::isFileDrag (info))
                    dynamic_cast<FileDragAndDropTarget*> (lastTarget)->fileDragExit (info.files);
                else
                    dynamic_cast<TextDragAndDropTarget*> (lastTarget)->textDragExit (info.text);
            }

            dragAndDropTargetComponent = nullptr;

            if (DragHelpers::isSuitableTarget (info, newTarget))
            {
                dragAndDropTargetComponent = newTarget;
                auto pos = newTarget->getLocalPoint (&component, info.position);

                if (DragHelpers::isFileDrag (info))
                    dynamic_cast<FileDragAndDropTarget*> (newTarget)->fileDragEnter (info.files, pos.x, pos.y);
                else
                    dynamic_cast<TextDragAndDropTarget*> (newTarget)->textDragEnter (info.text, pos.x, pos.y);
            }
        }
    }
    else
    {
        newTarget = lastTarget;
    }

    if (! DragHelpers::isSuitableTarget (info, newTarget))
        return false;

    auto pos = newTarget->getLocalPoint (&component, info.position);

    if (DragHelpers::isFileDrag (info))
        dynamic_cast<FileDragAndDropTarget*> (newTarget)->fileDragMove (info.files, pos.x, pos.y);
    else
        dynamic_cast<TextDragAndDropTarget*> (newTarget)->textDragMove (info.text, pos.x, pos.y);

    return true;
}

b8 ComponentPeer::handleDragExit (const ComponentPeer::DragInfo& info)
{
    DragInfo info2 (info);
    info2.position.setXY (-1, -1);
    const b8 used = handleDragMove (info2);

    jassert (dragAndDropTargetComponent == nullptr);
    lastDragAndDropCompUnderMouse = nullptr;
    return used;
}

b8 ComponentPeer::handleDragDrop (const ComponentPeer::DragInfo& info)
{
    handleDragMove (info);

    if (WeakReference<Component> targetComp = dragAndDropTargetComponent)
    {
        dragAndDropTargetComponent = nullptr;
        lastDragAndDropCompUnderMouse = nullptr;

        if (DragHelpers::isSuitableTarget (info, targetComp))
        {
            if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
            {
                targetComp->internalModalInputAttempt();

                if (targetComp->isCurrentlyBlockedByAnotherModalComponent())
                    return true;
            }

            ComponentPeer::DragInfo infoCopy (info);
            infoCopy.position = targetComp->getLocalPoint (&component, info.position);

            // We'll use an async message to deliver the drop, because if the target decides
            // to run a modal loop, it can gum-up the operating system..
            MessageManager::callAsync ([=]
            {
                if (auto* c = targetComp.get())
                {
                    if (DragHelpers::isFileDrag (info))
                        dynamic_cast<FileDragAndDropTarget*> (c)->filesDropped (infoCopy.files, infoCopy.position.x, infoCopy.position.y);
                    else
                        dynamic_cast<TextDragAndDropTarget*> (c)->textDropped (infoCopy.text, infoCopy.position.x, infoCopy.position.y);
                }
            });

            return true;
        }
    }

    return false;
}

//==============================================================================
z0 ComponentPeer::handleUserClosingWindow()
{
    component.userTriedToCloseWindow();
}

b8 ComponentPeer::setDocumentEditedStatus (b8)
{
    return false;
}

z0 ComponentPeer::setRepresentedFile (const File&)
{
}

//==============================================================================
i32 ComponentPeer::getCurrentRenderingEngine() const                             { return 0; }
z0 ComponentPeer::setCurrentRenderingEngine ([[maybe_unused]] i32 index)       { jassert (index == 0); }

//==============================================================================
std::function<ModifierKeys()> ComponentPeer::getNativeRealtimeModifiers = nullptr;

ModifierKeys ComponentPeer::getCurrentModifiersRealtime() noexcept
{
    if (getNativeRealtimeModifiers != nullptr)
        return getNativeRealtimeModifiers();

    return ModifierKeys::currentModifiers;
}

//==============================================================================
z0 ComponentPeer::forceDisplayUpdate()
{
    Desktop::getInstance().displays->refresh();
}

z0 ComponentPeer::callVBlankListeners (f64 timestampSec)
{
    vBlankListeners.call ([timestampSec] (auto& l) { l.onVBlank (timestampSec); });
}

z0 ComponentPeer::globalFocusChanged ([[maybe_unused]] Component* comp)
{
    refreshTextInputTarget();
}

} // namespace drx
