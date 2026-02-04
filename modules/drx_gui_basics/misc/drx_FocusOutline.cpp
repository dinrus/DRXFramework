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

//==============================================================================
struct OutlineWindowComponent final : public Component
{
    OutlineWindowComponent (Component* c, FocusOutline::OutlineWindowProperties& p)
      : target (c), props (p)
    {
        setVisible (true);
        setInterceptsMouseClicks (false, false);

        if (target->isOnDesktop())
        {
            setSize (1, 1);
            addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                            | ComponentPeer::windowIsTemporary
                            | ComponentPeer::windowIgnoresKeyPresses);
        }
        else if (auto* parent = target->getParentComponent())
        {
            auto targetIndex = parent->getIndexOfChildComponent (target);
            parent->addChildComponent (this, targetIndex + 1);
        }
    }

    z0 paint (Graphics& g) override
    {
        if (target != nullptr)
            props.drawOutline (g, getWidth(), getHeight());
    }

    z0 resized() override
    {
        repaint();
    }

    f32 getDesktopScaleFactor() const override
    {
        return target != nullptr ? target->getDesktopScaleFactor()
                                 : Component::getDesktopScaleFactor();
    }

private:
    WeakReference<Component> target;
    FocusOutline::OutlineWindowProperties& props;

    DRX_DECLARE_NON_COPYABLE (OutlineWindowComponent)
};

//==============================================================================
FocusOutline::FocusOutline (std::unique_ptr<OutlineWindowProperties> props)
    : properties (std::move (props))
{
}

FocusOutline::~FocusOutline()
{
    if (owner != nullptr)
        owner->removeComponentListener (this);

    if (lastParentComp != nullptr)
        lastParentComp->removeComponentListener (this);
}

z0 FocusOutline::setOwner (Component* componentToFollow)
{
    if (componentToFollow != owner)
    {
        if (owner != nullptr)
            owner->removeComponentListener (this);

        owner = componentToFollow;

        if (owner != nullptr)
            owner->addComponentListener (this);

        updateParent();
        updateOutlineWindow();
    }
}

z0 FocusOutline::componentMovedOrResized (Component& c, b8, b8)
{
    if (owner == &c)
        updateOutlineWindow();
}

z0 FocusOutline::componentBroughtToFront (Component& c)
{
    if (owner == &c)
        updateOutlineWindow();
}

z0 FocusOutline::componentParentHierarchyChanged (Component& c)
{
    if (owner == &c)
    {
        updateParent();
        updateOutlineWindow();
    }
}

z0 FocusOutline::componentVisibilityChanged (Component& c)
{
    if (owner == &c)
        updateOutlineWindow();
}

z0 FocusOutline::updateParent()
{
    lastParentComp = (owner != nullptr ? owner->getParentComponent()
                                       : nullptr);
}

z0 FocusOutline::updateOutlineWindow()
{
    if (reentrant)
        return;

    const ScopedValueSetter<b8> setter (reentrant, true);

    if (owner == nullptr)
    {
        outlineWindow = nullptr;
        return;
    }

    if (owner->isShowing()
         && owner->getWidth() > 0 && owner->getHeight() > 0)
    {
        if (outlineWindow == nullptr)
            outlineWindow = std::make_unique<OutlineWindowComponent> (owner, *properties);

        WeakReference<Component> deletionChecker (outlineWindow.get());

        outlineWindow->setAlwaysOnTop (owner->isAlwaysOnTop());

        if (deletionChecker == nullptr)
            return;

        const auto windowBounds = [this]
        {
            const auto bounds = properties->getOutlineBounds (*owner);

            if (lastParentComp != nullptr)
                return lastParentComp->getLocalArea (nullptr, bounds);

            return bounds;
        }();

        outlineWindow->setBounds (windowBounds);
    }
    else
    {
        outlineWindow = nullptr;
    }
}

} // namespace drx
