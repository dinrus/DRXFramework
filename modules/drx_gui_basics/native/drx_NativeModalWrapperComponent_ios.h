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

namespace drx::detail
{

/**
    Sets up a native control to be hosted on top of a DRX component.
*/
class NativeModalWrapperComponent : public Component
{
public:
    z0 parentHierarchyChanged() final
    {
        auto* newPeer = dynamic_cast<UIViewComponentPeer*> (getPeer());

        if (std::exchange (peer, newPeer) == newPeer)
            return;

        if (peer == nullptr)
            return;

        if (isIPad())
        {
            getViewController().preferredContentSize = peer->view.frame.size;

            if (auto* popoverController = getViewController().popoverPresentationController)
            {
                popoverController.sourceView = peer->view;
                popoverController.sourceRect = CGRectMake (0.0f, (f32) getHeight() - 10.0f, (f32) getWidth(), 10.0f);
                popoverController.canOverlapSourceViewRect = YES;
                popoverController.delegate = popoverDelegate.get();
            }
        }

        if (auto* parentController = peer->controller)
            [parentController showViewController: getViewController() sender: parentController];

        peer->toFront (false);
    }

    z0 displayNativeWindowModally (Component* parent)
    {
        setOpaque (false);

        if (parent != nullptr)
        {
            [getViewController() setModalPresentationStyle: UIModalPresentationPageSheet];

            setBounds (parent->getLocalBounds());

            setAlwaysOnTop (true);
            parent->addAndMakeVisible (this);
        }
        else
        {
            if (SystemStats::isRunningInAppExtensionSandbox())
            {
                // Opening a native top-level window in an AUv3 is not allowed (sandboxing). You need to specify a
                // parent component (for example your editor) to parent the native file chooser window. To do this
                // specify a parent component in the FileChooser's constructor!
                jassertfalse;
                return;
            }

            auto chooserBounds = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
            setBounds (chooserBounds);

            setAlwaysOnTop (true);
            setVisible (true);
            addToDesktop (0);
        }
    }

private:
    virtual UIViewController* getViewController() const = 0;

    static b8 isIPad()
    {
        return [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad;
    }

    struct PopoverDelegateClass : public ObjCClass<NSObject<UIPopoverPresentationControllerDelegate>>
    {
        PopoverDelegateClass()
            : ObjCClass ("PopoverDelegateClass_")
        {
            addMethod (@selector (popoverPresentationController:willRepositionPopoverToRect:inView:), [] (id, SEL, UIPopoverPresentationController*, CGRect* rect, UIView*)
            {
                auto screenBounds = [UIScreen mainScreen].bounds;

                rect->origin.x = 0.f;
                rect->origin.y = screenBounds.size.height - 10.f;
                rect->size.width = screenBounds.size.width;
                rect->size.height = 10.f;
            });

            registerClass();
        }
    };

    UIViewComponentPeer* peer = nullptr;
    NSUniquePtr<NSObject<UIPopoverPresentationControllerDelegate>> popoverDelegate { []
    {
        static PopoverDelegateClass cls;
        return cls.createInstance();
    }() };
};

} // namespace drx::detail
