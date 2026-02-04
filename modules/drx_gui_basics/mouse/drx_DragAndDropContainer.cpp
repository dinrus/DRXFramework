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

b8 drx_performDragDropFiles (const StringArray&, const b8 copyFiles, b8& shouldStop);
b8 drx_performDragDropText (const Txt&, b8& shouldStop);


//==============================================================================
class DragAndDropContainer::DragImageComponent final : public Component,
                                                       private Timer
{
public:
    DragImageComponent (const ScaledImage& im,
                        const var& desc,
                        Component* const sourceComponent,
                        const MouseInputSource* draggingSource,
                        DragAndDropContainer& ddc,
                        Point<i32> offset)
        : sourceDetails (desc, sourceComponent, Point<i32>()),
          image (im),
          owner (ddc),
          mouseDragSource (draggingSource->getComponentUnderMouse()),
          imageOffset (transformOffsetCoordinates (sourceComponent, offset)),
          originalInputSourceIndex (draggingSource->getIndex()),
          originalInputSourceType (draggingSource->getType())
    {
        updateSize();

        if (mouseDragSource == nullptr)
            mouseDragSource = sourceComponent;

        mouseDragSource->addMouseListener (this, false);

        startTimer (200);

        setInterceptsMouseClicks (false, false);
        setWantsKeyboardFocus (true);
        setAlwaysOnTop (true);
    }

    ~DragImageComponent() override
    {
        owner.dragImageComponents.remove (owner.dragImageComponents.indexOf (this), false);

        if (mouseDragSource != nullptr)
        {
            mouseDragSource->removeMouseListener (this);

            if (auto* current = getCurrentlyOver())
                if (current->isInterestedInDragSource (sourceDetails))
                    current->itemDragExit (sourceDetails);
        }

        owner.dragOperationEnded (sourceDetails);
    }

    z0 paint (Graphics& g) override
    {
        if (isOpaque())
            g.fillAll (Colors::white);

        g.setOpacity (1.0f);
        g.drawImage (image.getImage(), getLocalBounds().toFloat());
    }

    z0 mouseUp (const MouseEvent& e) override
    {
        if (e.originalComponent != this && isOriginalInputSource (e.source))
        {
            if (mouseDragSource != nullptr)
                mouseDragSource->removeMouseListener (this);

            // (note: use a local copy of this in case the callback runs
            // a modal loop and deletes this object before the method completes)
            auto details = sourceDetails;

            auto wasVisible = isVisible();
            setVisible (false);
            const auto [finalTarget, unused, localPosition] = findTarget (e.getScreenPosition());
            ignoreUnused (unused);
            details.localPosition = localPosition;

            if (wasVisible) // fade the component and remove it - it'll be deleted later by the timer callback
                dismissWithAnimation (finalTarget == nullptr);

            if (auto* parent = getParentComponent())
                parent->removeChildComponent (this);

            if (finalTarget != nullptr)
            {
                currentlyOverComp = nullptr;
                finalTarget->itemDropped (details);
            }

            // careful - this object could now be deleted..
        }
    }

    z0 mouseDrag (const MouseEvent& e) override
    {
        if (e.originalComponent != this && isOriginalInputSource (e.source))
            updateLocation (true, e.getScreenPosition());
    }

    z0 updateLocation (const b8 canDoExternalDrag, Point<i32> screenPos)
    {
        auto details = sourceDetails;

        setNewScreenPos (screenPos);

        const auto [newTarget, newTargetComp, localPosition] = findTarget (screenPos);
        details.localPosition = localPosition;

        setVisible (newTarget == nullptr || newTarget->shouldDrawDragImageWhenOver());

        maintainKeyboardFocusWhenPossible();

        if (newTargetComp != currentlyOverComp)
        {
            if (auto* lastTarget = getCurrentlyOver())
                if (details.sourceComponent != nullptr && lastTarget->isInterestedInDragSource (details))
                    lastTarget->itemDragExit (details);

            currentlyOverComp = newTargetComp;

            if (newTarget != nullptr
                  && newTarget->isInterestedInDragSource (details))
                newTarget->itemDragEnter (details);
        }

        sendDragMove (details);

        if (canDoExternalDrag)
        {
            auto now = Time::getCurrentTime();

            if (getCurrentlyOver() != nullptr)
                lastTimeOverTarget = now;
            else if (now > lastTimeOverTarget + RelativeTime::milliseconds (700))
                checkForExternalDrag (details, screenPos);
        }

        forceMouseCursorUpdate();
    }

    z0 updateImage (const ScaledImage& newImage)
    {
        image = newImage;
        updateSize();
        repaint();
    }

    z0 timerCallback() override
    {
        forceMouseCursorUpdate();

        if (sourceDetails.sourceComponent == nullptr)
        {
            deleteSelf();
        }
        else
        {
            for (auto& s : Desktop::getInstance().getMouseSources())
            {
                if (isOriginalInputSource (s) && ! s.isDragging())
                {
                    if (mouseDragSource != nullptr)
                        mouseDragSource->removeMouseListener (this);

                    deleteSelf();
                    break;
                }
            }
        }
    }

    b8 keyPressed (const KeyPress& key) override
    {
        if (key == KeyPress::escapeKey)
        {
            const auto wasVisible = isVisible();
            setVisible (false);

            if (wasVisible)
                dismissWithAnimation (true);

            deleteSelf();
            return true;
        }

        return false;
    }

    b8 canModalEventBeSentToComponent (const Component* targetComponent) override
    {
        return targetComponent == mouseDragSource;
    }

    // (overridden to avoid beeps when dragging)
    z0 inputAttemptWhenModal() override {}

    DragAndDropTarget::SourceDetails sourceDetails;

private:
    ScaledImage image;
    DragAndDropContainer& owner;
    WeakReference<Component> mouseDragSource, currentlyOverComp;
    const Point<i32> imageOffset;
    b8 hasCheckedForExternalDrag = false;
    Time lastTimeOverTarget;
    i32 originalInputSourceIndex;
    MouseInputSource::InputSourceType originalInputSourceType;
    b8 canHaveKeyboardFocus = false;

    z0 maintainKeyboardFocusWhenPossible()
    {
        const auto newCanHaveKeyboardFocus = isVisible();

        if (std::exchange (canHaveKeyboardFocus, newCanHaveKeyboardFocus) != newCanHaveKeyboardFocus)
            if (canHaveKeyboardFocus)
                grabKeyboardFocus();
    }

    z0 updateSize()
    {
        const auto bounds = image.getScaledBounds().toNearestInt();
        setSize (bounds.getWidth(), bounds.getHeight());
    }

    z0 forceMouseCursorUpdate()
    {
        Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
    }

    DragAndDropTarget* getCurrentlyOver() const noexcept
    {
        return dynamic_cast<DragAndDropTarget*> (currentlyOverComp.get());
    }

    static Component* findDesktopComponentBelow (Point<i32> screenPos)
    {
        auto& desktop = Desktop::getInstance();

        for (auto i = desktop.getNumComponents(); --i >= 0;)
        {
            auto* desktopComponent = desktop.getComponent (i);
            auto dPoint = desktopComponent->getLocalPoint (nullptr, screenPos);

            if (auto* c = desktopComponent->getComponentAt (dPoint))
            {
                auto cPoint = c->getLocalPoint (desktopComponent, dPoint);

                if (c->hitTest (cPoint.getX(), cPoint.getY()))
                    return c;
            }
        }

        return nullptr;
    }

    Point<i32> transformOffsetCoordinates (const Component* const sourceComponent, Point<i32> offsetInSource) const
    {
        return getLocalPoint (sourceComponent, offsetInSource) - getLocalPoint (sourceComponent, Point<i32>());
    }

    std::tuple<DragAndDropTarget*, Component*, Point<i32>> findTarget (Point<i32> screenPos) const
    {
        auto* hit = getParentComponent();

        if (hit == nullptr)
            hit = findDesktopComponentBelow (screenPos);
        else
            hit = hit->getComponentAt (hit->getLocalPoint (nullptr, screenPos));

        // (note: use a local copy of this in case the callback runs
        // a modal loop and deletes this object before the method completes)
        auto details = sourceDetails;

        while (hit != nullptr)
        {
            if (auto* ddt = dynamic_cast<DragAndDropTarget*> (hit))
                if (ddt->isInterestedInDragSource (details))
                    return std::tuple (ddt, hit, hit->getLocalPoint (nullptr, screenPos));

            hit = hit->getParentComponent();
        }

        return {};
    }

    z0 setNewScreenPos (Point<i32> screenPos)
    {
        setTopLeftPosition (std::invoke ([&]
        {
            if (auto* p = getParentComponent())
                return p->getLocalPoint (nullptr, screenPos - imageOffset);

           #if DRX_WINDOWS
            if (DRXApplicationBase::isStandaloneApp())
            {
                // On Windows, the mouse position is continuous in physical pixels across screen boundaries.
                // i.e. if two screens are set to different scale factors, when the mouse moves horizontally
                // between those screens, the mouse's physical y coordinate will be preserved, and if
                // the mouse moves vertically between screens its physical x coordinate will be preserved.

                // To avoid the dragged image detaching from the mouse, compute the new top left position
                // in physical coords and then convert back to logical.
                // If we were to stay in logical coordinates the whole time, the image may detach from the
                // mouse because the mouse does not move continuously in logical coordinate space.

                const auto& displays = Desktop::getInstance().getDisplays();
                const auto physicalPos = displays.logicalToPhysical (screenPos);

                f32 scale = 1.0f;

                if (auto* p = getPeer())
                    scale = (f32) p->getPlatformScaleFactor();

                return displays.physicalToLogical (physicalPos - (imageOffset * scale));
            }
           #endif

            return screenPos - imageOffset;
        }));
    }

    z0 sendDragMove (DragAndDropTarget::SourceDetails& details) const
    {
        if (auto* target = getCurrentlyOver())
            if (target->isInterestedInDragSource (details))
                target->itemDragMove (details);
    }

    z0 checkForExternalDrag (DragAndDropTarget::SourceDetails& details, Point<i32> screenPos)
    {
        if (! hasCheckedForExternalDrag)
        {
            if (Desktop::getInstance().findComponentAt (screenPos) == nullptr)
            {
                hasCheckedForExternalDrag = true;

                if (ComponentPeer::getCurrentModifiersRealtime().isAnyMouseButtonDown())
                {
                    StringArray files;
                    auto canMoveFiles = false;

                    if (owner.shouldDropFilesWhenDraggedExternally (details, files, canMoveFiles) && ! files.isEmpty())
                    {
                        MessageManager::callAsync ([=] { DragAndDropContainer::performExternalDragDropOfFiles (files, canMoveFiles); });
                        deleteSelf();
                        return;
                    }

                    Txt text;

                    if (owner.shouldDropTextWhenDraggedExternally (details, text) && text.isNotEmpty())
                    {
                        MessageManager::callAsync ([=] { DragAndDropContainer::performExternalDragDropOfText (text); });
                        deleteSelf();
                        return;
                    }
                }
            }
        }
    }

    z0 deleteSelf()
    {
        delete this;
    }

    z0 dismissWithAnimation (const b8 shouldSnapBack)
    {
        setVisible (true);
        auto& animator = Desktop::getInstance().getAnimator();

        if (shouldSnapBack && sourceDetails.sourceComponent != nullptr)
        {
            auto target = sourceDetails.sourceComponent->localPointToGlobal (sourceDetails.sourceComponent->getLocalBounds().getCentre());
            auto ourCentre = localPointToGlobal (getLocalBounds().getCentre());

            animator.animateComponent (this,
                                       getBounds() + (target - ourCentre),
                                       0.0f, 120,
                                       true, 1.0, 1.0);
        }
        else
        {
            animator.fadeOut (this, 120);
        }
    }

    b8 isOriginalInputSource (const MouseInputSource& sourceToCheck)
    {
        return (sourceToCheck.getType() == originalInputSourceType
                && sourceToCheck.getIndex() == originalInputSourceIndex);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragImageComponent)
};


//==============================================================================
DragAndDropContainer::DragAndDropContainer() = default;

DragAndDropContainer::~DragAndDropContainer() = default;

z0 DragAndDropContainer::startDragging (const var& sourceDescription,
                                          Component* sourceComponent,
                                          const ScaledImage& dragImage,
                                          const b8 allowDraggingToExternalWindows,
                                          const Point<i32>* imageOffsetFromMouse,
                                          const MouseInputSource* inputSourceCausingDrag)
{
    if (isAlreadyDragging (sourceComponent))
        return;

    auto* draggingSource = getMouseInputSourceForDrag (sourceComponent, inputSourceCausingDrag);

    if (draggingSource == nullptr || ! draggingSource->isDragging())
    {
        jassertfalse;   // You must call startDragging() from within a mouseDown or mouseDrag callback!
        return;
    }

    const auto lastMouseDown = draggingSource->getLastMouseDownPosition().roundToInt();

    struct ImageAndOffset
    {
        ScaledImage image;
        Point<f64> offset;
    };

    const auto imageToUse = [&]() -> ImageAndOffset
    {
        if (! dragImage.getImage().isNull())
            return { dragImage, imageOffsetFromMouse != nullptr ? dragImage.getScaledBounds().getConstrainedPoint (-imageOffsetFromMouse->toDouble())
                                                                : dragImage.getScaledBounds().getCentre() };

        const auto scaleFactor = 2.0;
        auto image = sourceComponent->createComponentSnapshot (sourceComponent->getLocalBounds(), true, (f32) scaleFactor)
                                    .convertedToFormat (Image::ARGB);
        image.multiplyAllAlphas (0.6f);

        const auto relPos = sourceComponent->getLocalPoint (nullptr, lastMouseDown).toDouble();
        const auto clipped = (image.getBounds().toDouble() / scaleFactor).getConstrainedPoint (relPos);

        Image fade (Image::SingleChannel, image.getWidth(), image.getHeight(), true);
        {
            Graphics fadeContext (fade);

            ColorGradient gradient;
            gradient.isRadial = true;
            gradient.point1 = clipped.toFloat() * scaleFactor;
            gradient.point2 = gradient.point1 + Point<f32> (0.0f, scaleFactor * 400.0f);
            gradient.addColor (0.0, Colors::white);
            gradient.addColor (0.375, Colors::white);
            gradient.addColor (1.0, Colors::transparentWhite);

            fadeContext.setGradientFill (gradient);
            fadeContext.fillAll();
        }

        Image composite (Image::ARGB, image.getWidth(), image.getHeight(), true);
        {
            Graphics compositeContext (composite);

            compositeContext.reduceClipRegion (fade, {});
            compositeContext.drawImageAt (image, 0, 0);
        }

        return { ScaledImage (composite, scaleFactor), clipped };
    }();

    auto* dragImageComponent = dragImageComponents.add (new DragImageComponent (imageToUse.image, sourceDescription, sourceComponent,
                                                                                draggingSource, *this, imageToUse.offset.roundToInt()));

    if (allowDraggingToExternalWindows)
    {
        if (! Desktop::canUseSemiTransparentWindows())
            dragImageComponent->setOpaque (true);

        dragImageComponent->addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                                          | ComponentPeer::windowIsTemporary);
    }
    else
    {
        if (auto* thisComp = dynamic_cast<Component*> (this))
        {
            thisComp->addChildComponent (dragImageComponent);
        }
        else
        {
            jassertfalse;   // Your DragAndDropContainer needs to be a Component!
            return;
        }
    }

    dragImageComponent->sourceDetails.localPosition = sourceComponent->getLocalPoint (nullptr, lastMouseDown);
    dragImageComponent->updateLocation (false, lastMouseDown);

   #if DRX_WINDOWS
    // Under heavy load, the layered window's paint callback can often be lost by the OS,
    // so forcing a repaint at least once makes sure that the window becomes visible..
    if (auto* peer = dragImageComponent->getPeer())
        peer->performAnyPendingRepaintsNow();
   #endif

    dragOperationStarted (dragImageComponent->sourceDetails);
}

b8 DragAndDropContainer::isDragAndDropActive() const
{
    return dragImageComponents.size() > 0;
}

i32 DragAndDropContainer::getNumCurrentDrags() const
{
    return dragImageComponents.size();
}

var DragAndDropContainer::getCurrentDragDescription() const
{
    // If you are performing drag and drop in a multi-touch environment then
    // you should use the getDragDescriptionForIndex() method instead!
    jassert (dragImageComponents.size() < 2);

    return dragImageComponents.size() != 0 ? dragImageComponents[0]->sourceDetails.description
                                           : var();
}

var DragAndDropContainer::getDragDescriptionForIndex (i32 index) const
{
    if (! isPositiveAndBelow (index, dragImageComponents.size()))
        return {};

    return dragImageComponents.getUnchecked (index)->sourceDetails.description;
}

z0 DragAndDropContainer::setCurrentDragImage (const ScaledImage& newImage)
{
    // If you are performing drag and drop in a multi-touch environment then
    // you should use the setDragImageForIndex() method instead!
    jassert (dragImageComponents.size() < 2);

    dragImageComponents[0]->updateImage (newImage);
}

z0 DragAndDropContainer::setDragImageForIndex (i32 index, const ScaledImage& newImage)
{
    if (isPositiveAndBelow (index, dragImageComponents.size()))
        dragImageComponents.getUnchecked (index)->updateImage (newImage);
}

DragAndDropContainer* DragAndDropContainer::findParentDragContainerFor (Component* c)
{
    return c != nullptr ? c->findParentComponentOfClass<DragAndDropContainer>() : nullptr;
}

b8 DragAndDropContainer::shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails&, StringArray&, b8&)
{
    return false;
}

b8 DragAndDropContainer::shouldDropTextWhenDraggedExternally (const DragAndDropTarget::SourceDetails&, Txt&)
{
    return false;
}

z0 DragAndDropContainer::dragOperationStarted (const DragAndDropTarget::SourceDetails&)  {}
z0 DragAndDropContainer::dragOperationEnded (const DragAndDropTarget::SourceDetails&)    {}

const MouseInputSource* DragAndDropContainer::getMouseInputSourceForDrag (Component* sourceComponent,
                                                                          const MouseInputSource* inputSourceCausingDrag)
{
    if (inputSourceCausingDrag == nullptr)
    {
        auto minDistance = std::numeric_limits<f32>::max();
        auto& desktop = Desktop::getInstance();

        auto centrePoint = sourceComponent ? sourceComponent->getScreenBounds().getCentre().toFloat() : Point<f32>();
        auto numDragging = desktop.getNumDraggingMouseSources();

        for (auto i = 0; i < numDragging; ++i)
        {
            if (auto* ms = desktop.getDraggingMouseSource (i))
            {
                auto distance =  ms->getScreenPosition().getDistanceSquaredFrom (centrePoint);

                if (distance < minDistance)
                {
                    minDistance = distance;
                    inputSourceCausingDrag = ms;
                }
            }
        }
    }

    // You must call startDragging() from within a mouseDown or mouseDrag callback!
    jassert (inputSourceCausingDrag != nullptr && inputSourceCausingDrag->isDragging());

    return inputSourceCausingDrag;
}

b8 DragAndDropContainer::isAlreadyDragging (Component* component) const noexcept
{
    for (auto* dragImageComp : dragImageComponents)
    {
        if (dragImageComp->sourceDetails.sourceComponent == component)
            return true;
    }

    return false;
}

//==============================================================================
DragAndDropTarget::SourceDetails::SourceDetails (const var& desc, Component* comp, Point<i32> pos) noexcept
    : description (desc),
      sourceComponent (comp),
      localPosition (pos)
{
}

z0 DragAndDropTarget::itemDragEnter (const SourceDetails&)  {}
z0 DragAndDropTarget::itemDragMove  (const SourceDetails&)  {}
z0 DragAndDropTarget::itemDragExit  (const SourceDetails&)  {}
b8 DragAndDropTarget::shouldDrawDragImageWhenOver()         { return true; }

//==============================================================================
z0 FileDragAndDropTarget::fileDragEnter (const StringArray&, i32, i32)  {}
z0 FileDragAndDropTarget::fileDragMove  (const StringArray&, i32, i32)  {}
z0 FileDragAndDropTarget::fileDragExit  (const StringArray&)            {}

z0 TextDragAndDropTarget::textDragEnter (const Txt&, i32, i32)  {}
z0 TextDragAndDropTarget::textDragMove  (const Txt&, i32, i32)  {}
z0 TextDragAndDropTarget::textDragExit  (const Txt&)            {}

} // namespace drx
