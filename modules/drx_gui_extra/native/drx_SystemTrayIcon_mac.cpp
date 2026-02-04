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

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

extern NSMenu* createNSMenu (const PopupMenu&, const Txt& name, i32 topLevelMenuId,
                             i32 topLevelIndex, b8 addDelegate);

//==============================================================================
struct StatusItemContainer : public Timer
{
    //==============================================================================
    StatusItemContainer (SystemTrayIconComponent& iconComp, const Image& im)
        : owner (iconComp), statusIcon (imageToNSImage (ScaledImage (im)))
    {
    }

    virtual z0 configureIcon() = 0;
    virtual z0 setHighlighted (b8 shouldHighlight) = 0;

    //==============================================================================
    z0 setIconSize()
    {
        [statusIcon.get() setSize: NSMakeSize (20.0f, 20.0f)];
    }

    z0 updateIcon (const Image& newImage)
    {
        statusIcon.reset (imageToNSImage (ScaledImage (newImage)));
        setIconSize();
        configureIcon();
    }

    z0 showMenu (const PopupMenu& menu)
    {
        if (NSMenu* m = createNSMenu (menu, "MenuBarItem", -2, -3, true))
        {
            setHighlighted (true);
            stopTimer();

            // There's currently no good alternative to this.
            [statusItem.get() popUpStatusItemMenu: m];

            startTimer (1);
        }
    }

    //==============================================================================
    z0 timerCallback() override
    {
        stopTimer();
        setHighlighted (false);
    }

    //==============================================================================
    SystemTrayIconComponent& owner;

    NSUniquePtr<NSStatusItem> statusItem;
    NSUniquePtr<NSImage> statusIcon;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusItemContainer)
};

//==============================================================================
struct ButtonBasedStatusItem  final : public StatusItemContainer
{
    //==============================================================================
    ButtonBasedStatusItem (SystemTrayIconComponent& iconComp, const Image& im)
        : StatusItemContainer (iconComp, im)
    {
        static ButtonEventForwarderClass cls;
        eventForwarder.reset ([cls.createInstance() init]);
        ButtonEventForwarderClass::setOwner (eventForwarder.get(), this);

        setIconSize();
        configureIcon();

        statusItem.reset ([[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain]);
        auto button = [statusItem.get() button];
        button.image = statusIcon.get();
        button.target = eventForwarder.get();
        button.action = @selector (handleEvent:);
        [button sendActionOn: NSEventMaskLeftMouseDown | NSEventMaskRightMouseDown | NSEventMaskScrollWheel];
    }

    ~ButtonBasedStatusItem() override
    {
        [statusItem.get() button].image = nullptr;
    }

    z0 configureIcon() override
    {
        [statusIcon.get() setTemplate: true];
        [statusItem.get() button].image = statusIcon.get();
    }

    z0 setHighlighted (b8 shouldHighlight) override
    {
        [[statusItem.get() button] setHighlighted: shouldHighlight];
    }

    //==============================================================================
    z0 handleEvent()
    {
        auto e = [NSApp currentEvent];
        NSEventType type = [e type];

        const b8 isLeft  = (type == NSEventTypeLeftMouseDown);
        const b8 isRight = (type == NSEventTypeRightMouseDown);

        if (owner.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (isLeft || isRight)
                if (auto* current = Component::getCurrentlyModalComponent())
                    current->inputAttemptWhenModal();
        }
        else
        {
            auto eventMods = ComponentPeer::getCurrentModifiersRealtime();

            if (([e modifierFlags] & NSEventModifierFlagCommand) != 0)
                eventMods = eventMods.withFlags (ModifierKeys::commandModifier);

            auto now = Time::getCurrentTime();
            auto mouseSource = Desktop::getInstance().getMainMouseSource();
            auto pressure = (f32) e.pressure;

            if (isLeft || isRight)
            {
                owner.mouseDown ({ mouseSource, {},
                                   eventMods.withFlags (isLeft ? ModifierKeys::leftButtonModifier
                                                               : ModifierKeys::rightButtonModifier),
                                   pressure,
                                   MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                   MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                   &owner, &owner, now, {}, now, 1, false });

                owner.mouseUp   ({ mouseSource, {},
                                   eventMods.withoutMouseButtons(),
                                   pressure,
                                   MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                   MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                   &owner, &owner, now, {}, now, 1, false });
            }
            else if (type == NSEventTypeMouseMoved)
            {
                owner.mouseMove (MouseEvent (mouseSource, {}, eventMods, pressure,
                                             MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                             MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                             &owner, &owner, now, {}, now, 1, false));
            }
        }
    }

    //==============================================================================
    class ButtonEventForwarderClass final : public ObjCClass<NSObject>
    {
    public:
        ButtonEventForwarderClass() : ObjCClass<NSObject> ("DRXButtonEventForwarderClass_")
        {
            addIvar<ButtonBasedStatusItem*> ("owner");

            addMethod (@selector (handleEvent:), handleEvent);

            registerClass();
        }

        static ButtonBasedStatusItem* getOwner (id self)               { return getIvar<ButtonBasedStatusItem*> (self, "owner"); }
        static z0 setOwner (id self, ButtonBasedStatusItem* owner)   { object_setInstanceVariable (self, "owner", owner); }

    private:
        static z0 handleEvent (id self, SEL, id)
        {
            if (auto* owner = getOwner (self))
                owner->handleEvent();
        }
    };

    //==============================================================================
    NSUniquePtr<NSObject> eventForwarder;
};

//==============================================================================
struct ViewBasedStatusItem final : public StatusItemContainer
{
    //==============================================================================
    ViewBasedStatusItem (SystemTrayIconComponent& iconComp, const Image& im)
        : StatusItemContainer (iconComp, im)
    {
        static SystemTrayViewClass cls;
        view.reset ([cls.createInstance() init]);
        SystemTrayViewClass::setOwner (view.get(), this);
        SystemTrayViewClass::setImage (view.get(), statusIcon.get());

        setIconSize();

        statusItem.reset ([[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain]);
        [statusItem.get() setView: view.get()];

        SystemTrayViewClass::frameChanged (view.get(), SEL(), nullptr);

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [[NSNotificationCenter defaultCenter]  addObserver: view.get()
                                                  selector: @selector (frameChanged:)
                                                      name: NSWindowDidMoveNotification
                                                    object: nil];
        DRX_END_IGNORE_WARNINGS_GCC_LIKE
    }

    ~ViewBasedStatusItem() override
    {
        [[NSNotificationCenter defaultCenter] removeObserver: view.get()];
        [[NSStatusBar systemStatusBar] removeStatusItem: statusItem.get()];
        SystemTrayViewClass::setOwner (view.get(), nullptr);
        SystemTrayViewClass::setImage (view.get(), nil);
    }

    z0 configureIcon() override
    {
        SystemTrayViewClass::setImage (view.get(), statusIcon.get());
        [statusItem.get() setView: view.get()];
    }

    z0 setHighlighted (b8 shouldHighlight) override
    {
        isHighlighted = shouldHighlight;
        [view.get() setNeedsDisplay: true];
    }

    //==============================================================================
    z0 handleStatusItemAction (NSEvent* e)
    {
        NSEventType type = [e type];

        const b8 isLeft  = (type == NSEventTypeLeftMouseDown  || type == NSEventTypeLeftMouseUp);
        const b8 isRight = (type == NSEventTypeRightMouseDown || type == NSEventTypeRightMouseUp);

        if (owner.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (isLeft || isRight)
                if (auto* current = Component::getCurrentlyModalComponent())
                    current->inputAttemptWhenModal();
        }
        else
        {
            auto eventMods = ComponentPeer::getCurrentModifiersRealtime();

            if (([e modifierFlags] & NSEventModifierFlagCommand) != 0)
                eventMods = eventMods.withFlags (ModifierKeys::commandModifier);

            auto now = Time::getCurrentTime();
            auto mouseSource = Desktop::getInstance().getMainMouseSource();
            auto pressure = (f32) e.pressure;

            if (isLeft || isRight)  // Only mouse up is sent by the OS, so simulate a down/up
            {
                setHighlighted (true);
                startTimer (150);

                owner.mouseDown (MouseEvent (mouseSource, {},
                                             eventMods.withFlags (isLeft ? ModifierKeys::leftButtonModifier
                                                                         : ModifierKeys::rightButtonModifier),
                                             pressure, MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                             MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                             &owner, &owner, now, {}, now, 1, false));

                owner.mouseUp (MouseEvent (mouseSource, {}, eventMods.withoutMouseButtons(), pressure,
                                           MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                           MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                           &owner, &owner, now, {}, now, 1, false));
            }
            else if (type == NSEventTypeMouseMoved)
            {
                owner.mouseMove (MouseEvent (mouseSource, {}, eventMods, pressure,
                                             MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                             MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                             &owner, &owner, now, {}, now, 1, false));
            }
        }
    }

    //==============================================================================
    struct SystemTrayViewClass final : public ObjCClass<NSControl>
    {
        SystemTrayViewClass()  : ObjCClass<NSControl> ("DRXSystemTrayView_")
        {
            addIvar<ViewBasedStatusItem*> ("owner");
            addIvar<NSImage*> ("image");

            addMethod (@selector (mouseDown:),      handleEventDown);
            addMethod (@selector (rightMouseDown:), handleEventDown);
            addMethod (@selector (drawRect:),       drawRect);

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (frameChanged:),   frameChanged);
            DRX_END_IGNORE_WARNINGS_GCC_LIKE

            registerClass();
        }

        static ViewBasedStatusItem* getOwner (id self)               { return getIvar<ViewBasedStatusItem*> (self, "owner"); }
        static NSImage* getImage (id self)                           { return getIvar<NSImage*> (self, "image"); }
        static z0 setOwner (id self, ViewBasedStatusItem* owner)   { object_setInstanceVariable (self, "owner", owner); }
        static z0 setImage (id self, NSImage* image)               { object_setInstanceVariable (self, "image", image); }

        static z0 frameChanged (id self, SEL, NSNotification*)
        {
            if (auto* owner = getOwner (self))
            {
                NSRect r = [[[owner->statusItem.get() view] window] frame];
                NSRect sr = [[[NSScreen screens] objectAtIndex: 0] frame];
                r.origin.y = sr.size.height - r.origin.y - r.size.height;
                owner->owner.setBounds (convertToRectInt (r));
            }
        }

    private:
        static z0 handleEventDown (id self, SEL, NSEvent* e)
        {
            if (auto* owner = getOwner (self))
                owner->handleStatusItemAction (e);
        }

        static z0 drawRect (id self, SEL, NSRect)
        {
            NSRect bounds = [self bounds];

            if (auto* owner = getOwner (self))
                [owner->statusItem.get() drawStatusBarBackgroundInRect: bounds
                                                         withHighlight: owner->isHighlighted];

            if (NSImage* const im = getImage (self))
            {
                NSSize imageSize = [im size];

                [im drawInRect: NSMakeRect (bounds.origin.x + ((bounds.size.width  - imageSize.width)  / 2.0f),
                                            bounds.origin.y + ((bounds.size.height - imageSize.height) / 2.0f),
                                            imageSize.width, imageSize.height)
                      fromRect: NSZeroRect
                     operation: NSCompositingOperationSourceOver
                      fraction: 1.0f];
            }
        }
    };

    //==============================================================================
    NSUniquePtr<NSControl> view;
    b8 isHighlighted = false;
};

//==============================================================================
class SystemTrayIconComponent::Pimpl
{
public:
    //==============================================================================
    Pimpl (SystemTrayIconComponent& iconComp, const Image& im)
        : statusItemHolder (std::make_unique<ButtonBasedStatusItem> (iconComp, im))
    {
    }

    //==============================================================================
    std::unique_ptr<StatusItemContainer> statusItemHolder;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
z0 SystemTrayIconComponent::setIconImage (const Image&, const Image& templateImage)
{
    if (templateImage.isValid())
    {
        if (pimpl == nullptr)
            pimpl.reset (new Pimpl (*this, templateImage));
        else
            pimpl->statusItemHolder->updateIcon (templateImage);
    }
    else
    {
        pimpl.reset();
    }
}

z0 SystemTrayIconComponent::setIconTooltip (const Txt&)
{
    // xxx not yet implemented!
}

z0 SystemTrayIconComponent::setHighlighted (b8 shouldHighlight)
{
    if (pimpl != nullptr)
        pimpl->statusItemHolder->setHighlighted (shouldHighlight);
}

z0 SystemTrayIconComponent::showInfoBubble (const Txt& /*title*/, const Txt& /*content*/)
{
    // xxx Not implemented!
}

z0 SystemTrayIconComponent::hideInfoBubble()
{
    // xxx Not implemented!
}

uk SystemTrayIconComponent::getNativeHandle() const
{
    return pimpl != nullptr ? pimpl->statusItemHolder->statusItem.get() : nullptr;
}

z0 SystemTrayIconComponent::showDropdownMenu (const PopupMenu& menu)
{
    if (pimpl != nullptr)
        pimpl->statusItemHolder->showMenu (menu);
}

DRX_END_IGNORE_DEPRECATION_WARNINGS

} // namespace drx
