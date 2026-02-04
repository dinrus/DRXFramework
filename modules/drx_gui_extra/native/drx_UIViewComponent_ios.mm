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

class UIViewComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    Pimpl (UIView* v, Component& comp)
        : ComponentMovementWatcher (&comp),
          view (v),
          owner (comp)
    {
        [view retain];

        if (owner.isShowing())
            componentPeerChanged();
    }

    ~Pimpl() override
    {
        [view removeFromSuperview];
        [view release];
    }

    z0 componentMovedOrResized (b8 /*wasMoved*/, b8 /*wasResized*/) override
    {
        auto* topComp = owner.getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
            auto pos = topComp->getLocalPoint (&owner, Point<i32>());

            [view setFrame: CGRectMake ((f32) pos.x, (f32) pos.y,
                                        (f32) owner.getWidth(), (f32) owner.getHeight())];
        }
    }

    z0 componentPeerChanged() override
    {
        auto* peer = owner.getPeer();

        if (currentPeer != peer)
        {
            if ([view superview] != nil)
                [view removeFromSuperview]; // Must be careful not to call this unless it's required - e.g. some Apple AU views
                                            // override the call and use it as a sign that they're being deleted, which breaks everything..
            currentPeer = peer;

            if (peer != nullptr)
            {
                UIView* peerView = (UIView*) peer->getNativeHandle();
                [peerView addSubview: view];
                componentMovedOrResized (false, false);
            }
        }

        [view setHidden: ! owner.isShowing()];
     }

    z0 componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    Rectangle<i32> getViewBounds() const
    {
        CGRect r = [view frame];
        return Rectangle<i32> ((i32) r.size.width, (i32) r.size.height);
    }

    UIView* const view;

private:
    Component& owner;
    ComponentPeer* currentPeer = nullptr;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
UIViewComponent::UIViewComponent() {}
UIViewComponent::~UIViewComponent()
{
    AccessibilityHandler::setNativeChildForComponent (*this, nullptr);
}

z0 UIViewComponent::setView (uk view)
{
    if (view != getView())
    {
        pimpl.reset();

        if (view != nullptr)
            pimpl.reset (new Pimpl ((UIView*) view, *this));

        AccessibilityHandler::setNativeChildForComponent (*this, getView());
    }
}

uk UIViewComponent::getView() const
{
    return pimpl == nullptr ? nullptr : pimpl->view;
}

z0 UIViewComponent::resizeToFitView()
{
    if (pimpl != nullptr)
        setBounds (pimpl->getViewBounds());
}

z0 UIViewComponent::paint (Graphics&) {}

std::unique_ptr<AccessibilityHandler> UIViewComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
