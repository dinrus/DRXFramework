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

SidePanel::SidePanel (StringRef title, i32 width, b8 positionOnLeft,
                      Component* contentToDisplay, b8 deleteComponentWhenNoLongerNeeded)
    : titleLabel ("titleLabel", title),
      isOnLeft (positionOnLeft),
      panelWidth (width)
{
    lookAndFeelChanged();

    addAndMakeVisible (titleLabel);

    dismissButton.onClick = [this] { showOrHide (false); };
    addAndMakeVisible (dismissButton);

    auto& desktop = Desktop::getInstance();

    desktop.addGlobalMouseListener (this);
    desktop.getAnimator().addChangeListener (this);

    if (contentToDisplay != nullptr)
        setContent (contentToDisplay, deleteComponentWhenNoLongerNeeded);

    setOpaque (false);
    setVisible (false);
    setAlwaysOnTop (true);
}

SidePanel::~SidePanel()
{
    auto& desktop = Desktop::getInstance();

    desktop.removeGlobalMouseListener (this);
    desktop.getAnimator().removeChangeListener (this);

    if (parent != nullptr)
        parent->removeComponentListener (this);
}

z0 SidePanel::setContent (Component* newContent, b8 deleteComponentWhenNoLongerNeeded)
{
    if (contentComponent.get() != newContent)
    {
        if (deleteComponentWhenNoLongerNeeded)
            contentComponent.setOwned (newContent);
        else
            contentComponent.setNonOwned (newContent);

        addAndMakeVisible (contentComponent);

        resized();
    }
}

z0 SidePanel::setTitleBarComponent (Component* titleBarComponentToUse,
                                      b8 keepDismissButton,
                                      b8 deleteComponentWhenNoLongerNeeded)
{
    if (titleBarComponent.get() != titleBarComponentToUse)
    {
        if (deleteComponentWhenNoLongerNeeded)
            titleBarComponent.setOwned (titleBarComponentToUse);
        else
            titleBarComponent.setNonOwned (titleBarComponentToUse);

        addAndMakeVisible (titleBarComponent);

        resized();
    }

    shouldShowDismissButton = keepDismissButton;
}

z0 SidePanel::showOrHide (b8 show)
{
    if (parent != nullptr)
    {
        isShowing = show;

        Desktop::getInstance().getAnimator().animateComponent (this, calculateBoundsInParent (*parent),
                                                               1.0f, 250, true, 1.0, 0.0);

        if (isShowing && ! isVisible())
            setVisible (true);
    }
}

z0 SidePanel::moved()
{
    NullCheckedInvocation::invoke (onPanelMove);
}

z0 SidePanel::resized()
{
    auto bounds = getLocalBounds();

    calculateAndRemoveShadowBounds (bounds);

    auto titleBounds = bounds.removeFromTop (titleBarHeight);

    if (titleBarComponent != nullptr)
    {
        if (shouldShowDismissButton)
            dismissButton.setBounds (isOnLeft ? titleBounds.removeFromRight (30).withTrimmedRight (10)
                                              : titleBounds.removeFromLeft  (30).withTrimmedLeft  (10));

        titleBarComponent->setBounds (titleBounds);
    }
    else
    {
        dismissButton.setBounds (isOnLeft ? titleBounds.removeFromRight (30).withTrimmedRight (10)
                                          : titleBounds.removeFromLeft  (30).withTrimmedLeft  (10));

        titleLabel.setBounds (isOnLeft ? titleBounds.withTrimmedRight (40)
                                       : titleBounds.withTrimmedLeft (40));
    }

    if (contentComponent != nullptr)
        contentComponent->setBounds (bounds);
}

z0 SidePanel::paint (Graphics& g)
{
    auto& lf = getLookAndFeel();

    auto bgColor     = lf.findColor (SidePanel::backgroundColor);
    auto shadowColor = lf.findColor (SidePanel::shadowBaseColor);

    g.setGradientFill (ColorGradient (shadowColor.withAlpha (0.7f), (isOnLeft ? shadowArea.getTopLeft()
                                                                                : shadowArea.getTopRight()).toFloat(),
                                       shadowColor.withAlpha (0.0f), (isOnLeft ? shadowArea.getTopRight()
                                                                                : shadowArea.getTopLeft()).toFloat(), false));
    g.fillRect (shadowArea);

    g.reduceClipRegion (getLocalBounds().withTrimmedRight (shadowArea.getWidth())
                                        .withX (isOnLeft ? 0 : shadowArea.getWidth()));
    g.fillAll (bgColor);
}

z0 SidePanel::parentHierarchyChanged()
{
    auto* newParent = getParentComponent();

    if ((newParent != nullptr) && (parent != newParent))
    {
        if (parent != nullptr)
            parent->removeComponentListener (this);

        parent = newParent;
        parent->addComponentListener (this);
    }
}

z0 SidePanel::mouseDrag (const MouseEvent& e)
{
    if (shouldResize)
    {
        Point<i32> convertedPoint;

        if (getParentComponent() == nullptr)
            convertedPoint = e.eventComponent->localPointToGlobal (e.getPosition());
        else
            convertedPoint = getParentComponent()->getLocalPoint (e.eventComponent, e.getPosition());

        auto currentMouseDragX = convertedPoint.x;

        if (isOnLeft)
        {
            amountMoved = startingBounds.getRight() - currentMouseDragX;
            setBounds (getBounds().withX (startingBounds.getX() - jmax (amountMoved, 0)));
        }
        else
        {
            amountMoved = currentMouseDragX - startingBounds.getX();
            setBounds (getBounds().withX (startingBounds.getX() + jmax (amountMoved, 0)));
        }
    }
    else if (isShowing)
    {
        auto relativeMouseDownPosition = getLocalPoint (e.eventComponent, e.getMouseDownPosition());
        auto relativeMouseDragPosition = getLocalPoint (e.eventComponent, e.getPosition());

        if (! getLocalBounds().contains (relativeMouseDownPosition)
              && getLocalBounds().contains (relativeMouseDragPosition))
        {
            shouldResize = true;
            startingBounds = getBounds();
        }
    }
}

z0 SidePanel::mouseUp (const MouseEvent&)
{
    if (shouldResize)
    {
        showOrHide (amountMoved < (panelWidth / 2));

        amountMoved = 0;
        shouldResize = false;
    }
}

//==============================================================================
z0 SidePanel::lookAndFeelChanged()
{
    auto& lf = getLookAndFeel();

    dismissButton.setShape (lf.getSidePanelDismissButtonShape (*this), false, true, false);

    dismissButton.setColors (lf.findColor (SidePanel::dismissButtonNormalColor),
                              lf.findColor (SidePanel::dismissButtonOverColor),
                              lf.findColor (SidePanel::dismissButtonDownColor));

    titleLabel.setFont (lf.getSidePanelTitleFont (*this));
    titleLabel.setColor (Label::textColorId, findColor (SidePanel::titleTextColor));
    titleLabel.setJustificationType (lf.getSidePanelTitleJustification (*this));
}

z0 SidePanel::componentMovedOrResized (Component& component, [[maybe_unused]] b8 wasMoved, b8 wasResized)
{
    if (wasResized && (&component == parent))
        setBounds (calculateBoundsInParent (component));
}

z0 SidePanel::changeListenerCallback (ChangeBroadcaster*)
{
    if (! Desktop::getInstance().getAnimator().isAnimating (this))
    {
        NullCheckedInvocation::invoke (onPanelShowHide, isShowing);

        if (isVisible() && ! isShowing)
            setVisible (false);
    }
}

Rectangle<i32> SidePanel::calculateBoundsInParent (Component& parentComp) const
{
    auto parentBounds = parentComp.getLocalBounds();

    if (isOnLeft)
    {
        return isShowing ? parentBounds.removeFromLeft (panelWidth)
                         : parentBounds.withX (parentBounds.getX() - panelWidth).withWidth (panelWidth);
    }

    return isShowing ? parentBounds.removeFromRight (panelWidth)
                     : parentBounds.withX (parentBounds.getRight()).withWidth (panelWidth);
}

z0 SidePanel::calculateAndRemoveShadowBounds (Rectangle<i32>& bounds)
{
    shadowArea = isOnLeft ? bounds.removeFromRight (shadowWidth)
                          : bounds.removeFromLeft  (shadowWidth);
}

b8 SidePanel::isMouseEventInThisOrChildren (Component* eventComponent)
{
    if (eventComponent == this)
        return true;

    for (auto& child : getChildren())
        if (eventComponent == child)
            return true;

    return false;
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> SidePanel::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
