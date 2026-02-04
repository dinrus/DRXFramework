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

class AndroidViewComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    Pimpl (const LocalRef<jobject>& v, Component& comp)
        : ComponentMovementWatcher (&comp),
          view (v),
          owner (comp)
    {
        if (owner.isShowing())
            componentPeerChanged();
    }

    ~Pimpl() override
    {
        removeFromParent();
    }

    z0 componentMovedOrResized (b8 /*wasMoved*/, b8 /*wasResized*/) override
    {
        auto* topComp = owner.getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
            auto pos = topComp->getLocalPoint (&owner, Point<i32>());

            Rectangle<i32> r (pos.x, pos.y, owner.getWidth(), owner.getHeight());
            r *= Desktop::getInstance().getDisplays().getPrimaryDisplay()->scale;

            getEnv()->CallVoidMethod (view, AndroidView.layout, r.getX(), r.getY(),
                                      r.getRight(), r.getBottom());
        }
    }

    z0 componentPeerChanged() override
    {
        auto* peer = owner.getPeer();

        if (currentPeer != peer)
        {
            removeFromParent();
            currentPeer = peer;

            addToParent();
        }

        enum
        {
            VISIBLE   = 0,
            INVISIBLE = 4
        };

        getEnv()->CallVoidMethod (view, AndroidView.setVisibility, owner.isShowing() ? VISIBLE : INVISIBLE);
     }

    z0 componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    z0 componentBroughtToFront (Component& comp) override
    {
        ComponentMovementWatcher::componentBroughtToFront (comp);
    }

    Rectangle<i32> getViewBounds() const
    {
        auto* env = getEnv();

        i32 width  = env->CallIntMethod (view, AndroidView.getWidth);
        i32 height = env->CallIntMethod (view, AndroidView.getHeight);

        return Rectangle<i32> (width, height);
    }

    GlobalRef view;

private:
    z0 addToParent()
    {
        if (currentPeer != nullptr)
        {
            jobject peerView = (jobject) currentPeer->getNativeHandle();

            // NB: Assuming a parent is always of ViewGroup type
            auto* env = getEnv();

            env->CallVoidMethod (peerView, AndroidViewGroup.addView, view.get());
            componentMovedOrResized (false, false);
        }
    }

    z0 removeFromParent()
    {
        auto* env = getEnv();
        auto parentView = env->CallObjectMethod (view, AndroidView.getParent);

        if (parentView != nullptr)
        {
            // Assuming a parent is always of ViewGroup type
            env->CallVoidMethod (parentView, AndroidViewGroup.removeView, view.get());
        }
    }

    Component& owner;
    ComponentPeer* currentPeer = nullptr;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
AndroidViewComponent::AndroidViewComponent()
{
}

AndroidViewComponent::~AndroidViewComponent()
{
    AccessibilityHandler::setNativeChildForComponent (*this, nullptr);
}

z0 AndroidViewComponent::setView (uk view)
{
    if (view != getView())
    {
        pimpl.reset();

        if (view != nullptr)
        {
            // explicitly create a new local ref here so that we don't
            // delete the users pointer
            auto* env = getEnv();
            auto localref = LocalRef<jobject> (env->NewLocalRef ((jobject) view));

            pimpl.reset (new Pimpl (localref, *this));

            AccessibilityHandler::setNativeChildForComponent (*this, getView());
        }
        else
        {
            AccessibilityHandler::setNativeChildForComponent (*this, nullptr);
        }
    }
}

uk AndroidViewComponent::getView() const
{
    return pimpl == nullptr ? nullptr : (uk) pimpl->view;
}

z0 AndroidViewComponent::resizeToFitView()
{
    if (pimpl != nullptr)
        setBounds (pimpl->getViewBounds());
}

z0 AndroidViewComponent::paint (Graphics&) {}

std::unique_ptr<AccessibilityHandler> AndroidViewComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
