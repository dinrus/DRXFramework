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

CallOutBox::CallOutBox (Component& c, Rectangle<i32> area, Component* const parent)
    : content (c)
{
    addAndMakeVisible (content);

    if (parent != nullptr)
    {
        parent->addChildComponent (this);
        updatePosition (area, parent->getLocalBounds());
        setVisible (true);
    }
    else
    {
        setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());
        updatePosition (area, Desktop::getInstance().getDisplays().getDisplayForRect (area)->userArea);
        addToDesktop (ComponentPeer::windowIsTemporary);

        startTimer (100);
    }

    creationTime = Time::getCurrentTime();
}

//==============================================================================
class CallOutBoxCallback final : public ModalComponentManager::Callback,
                                 private Timer
{
public:
    CallOutBoxCallback (std::unique_ptr<Component> c, const Rectangle<i32>& area, Component* parent)
        : content (std::move (c)),
          callout (*content, area, parent)
    {
        callout.setVisible (true);
        callout.enterModalState (true, this);
        startTimer (200);
    }

    z0 modalStateFinished (i32) override {}

    z0 timerCallback() override
    {
        if (! detail::WindowingHelpers::isForegroundOrEmbeddedProcess (&callout))
            callout.dismiss();
    }

    std::unique_ptr<Component> content;
    CallOutBox callout;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CallOutBoxCallback)
};

CallOutBox& CallOutBox::launchAsynchronously (std::unique_ptr<Component> content, Rectangle<i32> area, Component* parent)
{
    jassert (content != nullptr); // must be a valid content component!

    return (new CallOutBoxCallback (std::move (content), area, parent))->callout;
}

//==============================================================================
z0 CallOutBox::setArrowSize (const f32 newSize)
{
    arrowSize = newSize;
    refreshPath();
}

i32 CallOutBox::getBorderSize() const noexcept
{
    return jmax (getLookAndFeel().getCallOutBoxBorderSize (*this), (i32) arrowSize);
}

z0 CallOutBox::lookAndFeelChanged()
{
    resized();
}

z0 CallOutBox::paint (Graphics& g)
{
    getLookAndFeel().drawCallOutBoxBackground (*this, g, outline, background);
}

z0 CallOutBox::resized()
{
    auto borderSpace = getBorderSize();
    content.setTopLeftPosition (borderSpace, borderSpace);
    refreshPath();
}

z0 CallOutBox::moved()
{
    refreshPath();
}

z0 CallOutBox::childBoundsChanged (Component*)
{
    updatePosition (targetArea, availableArea);
}

b8 CallOutBox::hitTest (i32 x, i32 y)
{
    return outline.contains ((f32) x, (f32) y);
}

z0 CallOutBox::inputAttemptWhenModal()
{
    if (dismissalMouseClicksAreAlwaysConsumed
         || targetArea.contains (getMouseXYRelative() + getBounds().getPosition()))
    {
        // if you click on the area that originally popped-up the callout, you expect it
        // to get rid of the box, but deleting the box here allows the click to pass through and
        // probably re-trigger it, so we need to dismiss the box asynchronously to consume the click..

        // For touchscreens, we make sure not to dismiss the CallOutBox immediately,
        // as Windows still sends touch events before the CallOutBox had a chance
        // to really open.

        auto elapsed = Time::getCurrentTime() - creationTime;

        if (elapsed.inMilliseconds() > 200)
            dismiss();
    }
    else
    {
        exitModalState (0);
        setVisible (false);
    }
}

z0 CallOutBox::setDismissalMouseClicksAreAlwaysConsumed (b8 b) noexcept
{
    dismissalMouseClicksAreAlwaysConsumed = b;
}

static constexpr i32 callOutBoxDismissCommandId = 0x4f83a04b;

z0 CallOutBox::handleCommandMessage (i32 commandId)
{
    Component::handleCommandMessage (commandId);

    if (commandId == callOutBoxDismissCommandId)
    {
        exitModalState (0);
        setVisible (false);
    }
}

z0 CallOutBox::dismiss()
{
    postCommandMessage (callOutBoxDismissCommandId);
}

b8 CallOutBox::keyPressed (const KeyPress& key)
{
    if (key.isKeyCode (KeyPress::escapeKey))
    {
        inputAttemptWhenModal();
        return true;
    }

    return false;
}

z0 CallOutBox::updatePosition (const Rectangle<i32>& newAreaToPointTo, const Rectangle<i32>& newAreaToFitIn)
{
    targetArea = newAreaToPointTo;
    availableArea = newAreaToFitIn;

    auto borderSpace = getBorderSize();
    auto newBounds = getLocalArea (&content, Rectangle<i32> (content.getWidth()  + borderSpace * 2,
                                                             content.getHeight() + borderSpace * 2));

    auto hw = newBounds.getWidth() / 2;
    auto hh = newBounds.getHeight() / 2;
    auto hwReduced = (f32) (hw - borderSpace * 2);
    auto hhReduced = (f32) (hh - borderSpace * 2);
    auto arrowIndent = (f32) borderSpace - arrowSize;

    Point<f32> targets[4] = { { (f32) targetArea.getCentreX(), (f32) targetArea.getBottom() },
                                { (f32) targetArea.getRight(),   (f32) targetArea.getCentreY() },
                                { (f32) targetArea.getX(),       (f32) targetArea.getCentreY() },
                                { (f32) targetArea.getCentreX(), (f32) targetArea.getY() } };

    Line<f32> lines[4] = { { targets[0].translated (-hwReduced, hh - arrowIndent),    targets[0].translated (hwReduced, hh - arrowIndent) },
                             { targets[1].translated (hw - arrowIndent, -hhReduced),    targets[1].translated (hw - arrowIndent, hhReduced) },
                             { targets[2].translated (-(hw - arrowIndent), -hhReduced), targets[2].translated (-(hw - arrowIndent), hhReduced) },
                             { targets[3].translated (-hwReduced, -(hh - arrowIndent)), targets[3].translated (hwReduced, -(hh - arrowIndent)) } };

    auto centrePointArea = newAreaToFitIn.reduced (hw, hh).toFloat();
    auto targetCentre = targetArea.getCentre().toFloat();

    f32 nearest = 1.0e9f;

    for (i32 i = 0; i < 4; ++i)
    {
        Line<f32> constrainedLine (centrePointArea.getConstrainedPoint (lines[i].getStart()),
                                     centrePointArea.getConstrainedPoint (lines[i].getEnd()));

        auto centre = constrainedLine.findNearestPointTo (targetCentre);
        auto distanceFromCentre = centre.getDistanceFrom (targets[i]);

        if (! centrePointArea.intersects (lines[i]))
            distanceFromCentre += 1000.0f;

        if (distanceFromCentre < nearest)
        {
            nearest = distanceFromCentre;
            targetPoint = targets[i];

            newBounds.setPosition ((i32) (centre.x - (f32) hw),
                                   (i32) (centre.y - (f32) hh));
        }
    }

    setBounds (newBounds);
}

z0 CallOutBox::refreshPath()
{
    repaint();
    background = {};
    outline.clear();

    const f32 gap = 4.5f;

    outline.addBubble (getLocalArea (&content, content.getLocalBounds().toFloat()).expanded (gap, gap),
                       getLocalBounds().toFloat(),
                       targetPoint - getPosition().toFloat(),
                       getLookAndFeel().getCallOutBoxCornerSize (*this), arrowSize * 0.7f);
}

z0 CallOutBox::timerCallback()
{
    toFront (true);
    stopTimer();
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> CallOutBox::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::dialogWindow);
}

} // namespace drx
