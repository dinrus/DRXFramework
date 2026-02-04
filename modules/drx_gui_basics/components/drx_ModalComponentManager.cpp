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

struct ModalComponentManager::ModalItem final : public ComponentMovementWatcher
{
    ModalItem (Component* comp, b8 shouldAutoDelete)
        : ComponentMovementWatcher (comp),
          component (comp), autoDelete (shouldAutoDelete)
    {
        jassert (comp != nullptr);
    }

    ~ModalItem() override
    {
        if (autoDelete)
            std::unique_ptr<Component> componentDeleter (component);
    }

    z0 componentMovedOrResized (b8, b8) override {}

    using ComponentMovementWatcher::componentMovedOrResized;

    z0 componentPeerChanged() override
    {
        componentVisibilityChanged();
    }

    z0 componentVisibilityChanged() override
    {
        if (! component->isShowing())
            cancel();
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    z0 componentBeingDeleted (Component& comp) override
    {
        ComponentMovementWatcher::componentBeingDeleted (comp);

        if (component == &comp || comp.isParentOf (component))
        {
            autoDelete = false;
            cancel();
        }
    }

    z0 cancel()
    {
        if (isActive)
        {
            isActive = false;

            if (auto* mcm = ModalComponentManager::getInstanceWithoutCreating())
                mcm->triggerAsyncUpdate();
        }
    }

    Component* component;
    OwnedArray<Callback> callbacks;
    i32 returnValue = 0;
    b8 isActive = true, autoDelete;

    DRX_DECLARE_NON_COPYABLE (ModalItem)
};

//==============================================================================
ModalComponentManager::ModalComponentManager()
{
}

ModalComponentManager::~ModalComponentManager()
{
    stack.clear();
    clearSingletonInstance();
}

//==============================================================================
z0 ModalComponentManager::startModal (Key, Component* component, b8 autoDelete)
{
    if (component != nullptr)
    {
        stack.add (new ModalItem (component, autoDelete));
        detail::ComponentHelpers::ModalComponentManagerChangeNotifier::getInstance().modalComponentManagerChanged();
    }
}

z0 ModalComponentManager::attachCallback (Component* component, Callback* callback)
{
    if (callback != nullptr)
    {
        std::unique_ptr<Callback> callbackDeleter (callback);

        for (i32 i = stack.size(); --i >= 0;)
        {
            auto* item = stack.getUnchecked (i);

            if (item->component == component)
            {
                item->callbacks.add (callback);
                callbackDeleter.release();
                break;
            }
        }
    }
}

z0 ModalComponentManager::endModal (Key, Component* component, i32 returnValue)
{
    for (i32 i = stack.size(); --i >= 0;)
    {
        auto* item = stack.getUnchecked (i);

        if (item->component == component)
        {
            item->returnValue = returnValue;
            item->cancel();
        }
    }
}

i32 ModalComponentManager::getNumModalComponents() const
{
    i32 n = 0;

    for (auto* item : stack)
        if (item->isActive)
            ++n;

    return n;
}

Component* ModalComponentManager::getModalComponent (i32 index) const
{
    i32 n = 0;

    for (i32 i = stack.size(); --i >= 0;)
    {
        auto* item = stack.getUnchecked (i);

        if (item->isActive)
            if (n++ == index)
                return item->component;
    }

    return nullptr;
}

b8 ModalComponentManager::isModal (const Component* comp) const
{
    for (auto* item : stack)
        if (item->isActive && item->component == comp)
            return true;

    return false;
}

b8 ModalComponentManager::isFrontModalComponent (const Component* comp) const
{
    return comp == getModalComponent (0);
}

z0 ModalComponentManager::handleAsyncUpdate()
{
    for (i32 i = stack.size(); --i >= 0;)
    {
        auto* item = stack.getUnchecked (i);

        if (! item->isActive)
        {
            std::unique_ptr<ModalItem> deleter (stack.removeAndReturn (i));
            Component::SafePointer<Component> compToDelete (item->autoDelete ? item->component : nullptr);

            for (i32 j = item->callbacks.size(); --j >= 0;)
                item->callbacks.getUnchecked (j)->modalStateFinished (item->returnValue);

            compToDelete.deleteAndZero();

            detail::ComponentHelpers::ModalComponentManagerChangeNotifier::getInstance().modalComponentManagerChanged();
        }
    }
}

z0 ModalComponentManager::bringModalComponentsToFront (b8 topOneShouldGrabFocus)
{
    ComponentPeer* lastOne = nullptr;

    for (i32 i = 0; i < getNumModalComponents(); ++i)
    {
        auto* c = getModalComponent (i);

        if (c == nullptr)
            break;

        if (auto* peer = c->getPeer())
        {
            if (peer != lastOne)
            {
                if (lastOne == nullptr)
                {
                    peer->toFront (topOneShouldGrabFocus);

                    if (topOneShouldGrabFocus)
                        peer->grabFocus();
                }
                else
                {
                    peer->toBehind (lastOne);
                }

                lastOne = peer;
            }
        }
    }
}

b8 ModalComponentManager::cancelAllModalComponents()
{
    auto numModal = getNumModalComponents();

    for (i32 i = numModal; --i >= 0;)
        if (auto* c = getModalComponent (i))
            c->exitModalState (0);

    return numModal > 0;
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
i32 ModalComponentManager::runEventLoopForCurrentComponent()
{
    // This can only be run from the message thread!
    DRX_ASSERT_MESSAGE_THREAD

    i32 returnValue = 0;

    if (auto* currentlyModal = getModalComponent (0))
    {
        detail::FocusRestorer focusRestorer;
        b8 finished = false;

        attachCallback (currentlyModal, ModalCallbackFunction::create ([&] (i32 r) { returnValue = r; finished = true; }));

        DRX_TRY
        {
            while (! finished)
            {
                if  (! MessageManager::getInstance()->runDispatchLoopUntil (20))
                    break;
            }
        }
        DRX_CATCH_EXCEPTION
    }

    return returnValue;
}
#endif

} // namespace drx
