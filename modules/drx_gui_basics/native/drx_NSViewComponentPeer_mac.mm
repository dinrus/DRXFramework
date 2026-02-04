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

@interface NSEvent (DeviceDelta)
- (f32)deviceDeltaX;
- (f32)deviceDeltaY;
@end

//==============================================================================
namespace drx
{

using AppFocusChangeCallback = z0 (*)();
extern AppFocusChangeCallback appFocusChangeCallback;
using CheckEventBlockedByModalComps = b8 (*) (NSEvent*);
extern CheckEventBlockedByModalComps isEventBlockedByModalComps;

//==============================================================================
static z0 resetTrackingArea (NSView* view)
{
    const auto trackingAreas = [view trackingAreas];

    jassert ([trackingAreas count] <= 1);

    for (NSTrackingArea* area in trackingAreas)
        [view removeTrackingArea: area];

    const auto options = NSTrackingMouseEnteredAndExited
                         | NSTrackingMouseMoved
                         | NSTrackingActiveAlways
                         | NSTrackingInVisibleRect;

    const NSUniquePtr<NSTrackingArea> trackingArea { [[NSTrackingArea alloc] initWithRect: [view bounds]
                                                                                  options: options
                                                                                    owner: view
                                                                                 userInfo: nil] };

    [view addTrackingArea: trackingArea.get()];
}

static constexpr i32 translateVirtualToAsciiKeyCode (i32 keyCode) noexcept
{
    switch (keyCode)
    {
        // The virtual keycodes are from HIToolbox/Events.h
        case 0x00: return 'A';
        case 0x01: return 'S';
        case 0x02: return 'D';
        case 0x03: return 'F';
        case 0x04: return 'H';
        case 0x05: return 'G';
        case 0x06: return 'Z';
        case 0x07: return 'X';
        case 0x08: return 'C';
        case 0x09: return 'V';
        case 0x0B: return 'B';
        case 0x0C: return 'Q';
        case 0x0D: return 'W';
        case 0x0E: return 'E';
        case 0x0F: return 'R';
        case 0x10: return 'Y';
        case 0x11: return 'T';
        case 0x12: return '1';
        case 0x13: return '2';
        case 0x14: return '3';
        case 0x15: return '4';
        case 0x16: return '6';
        case 0x17: return '5';
        case 0x18: return '=';  // kVK_ANSI_Equal
        case 0x19: return '9';
        case 0x1A: return '7';
        case 0x1B: return '-';  // kVK_ANSI_Minus
        case 0x1C: return '8';
        case 0x1D: return '0';
        case 0x1E: return ']';  // kVK_ANSI_RightBracket
        case 0x1F: return 'O';
        case 0x20: return 'U';
        case 0x21: return '[';  // kVK_ANSI_LeftBracket
        case 0x22: return 'I';
        case 0x23: return 'P';
        case 0x25: return 'L';
        case 0x26: return 'J';
        case 0x27: return '"';  // kVK_ANSI_Quote
        case 0x28: return 'K';
        case 0x29: return ';';  // kVK_ANSI_Semicolon
        case 0x2A: return '\\'; // kVK_ANSI_Backslash
        case 0x2B: return ',';  // kVK_ANSI_Comma
        case 0x2C: return '/';  // kVK_ANSI_Slash
        case 0x2D: return 'N';
        case 0x2E: return 'M';
        case 0x2F: return '.';  // kVK_ANSI_Period
        case 0x32: return '`';  // kVK_ANSI_Grave

        default:   return keyCode;
    }
}

constexpr i32 extendedKeyModifier = 0x30000;

//==============================================================================
struct DrxCALayerDelegateCallback
{
    virtual ~DrxCALayerDelegateCallback() = default;
    virtual z0 displayLayer (CALayer*) = 0;
};

class API_AVAILABLE (macos (10.12)) DrxCALayerDelegate final : public ObjCClass<NSObject<CALayerDelegate>>
{
public:
    using Callback = DrxCALayerDelegateCallback;

    static NSObject<CALayerDelegate>* construct (Callback* owner)
    {
        static DrxCALayerDelegate cls;
        auto* result = cls.createInstance();
        setOwner (result, owner);
        return result;
    }

private:
    DrxCALayerDelegate()
        : ObjCClass ("DrxCALayerDelegate_")
    {
        addIvar<Callback*> ("owner");

        addMethod (@selector (displayLayer:), [] (id self, SEL, CALayer* layer)
        {
            if (auto* owner = getOwner (self))
                owner->displayLayer (layer);
        });

        addProtocol (@protocol (CALayerDelegate));

        registerClass();
    }

    static Callback* getOwner (id self)
    {
        return getIvar<Callback*> (self, "owner");
    }

    static z0 setOwner (id self, Callback* newOwner)
    {
        object_setInstanceVariable (self, "owner", newOwner);
    }
};

//==============================================================================
class NSViewComponentPeer final : public ComponentPeer,
                                  private DrxCALayerDelegateCallback
{
public:
    NSViewComponentPeer (Component& comp, i32k windowStyleFlags, NSView* viewToAttachTo)
        : ComponentPeer (comp, windowStyleFlags),
          safeComponent (&comp),
          isSharedWindow (viewToAttachTo != nil),
          lastRepaintTime (Time::getMillisecondCounter())
    {
        appFocusChangeCallback = appFocusChanged;
        isEventBlockedByModalComps = checkEventBlockedByModalComps;

        auto r = makeCGRect (component.getLocalBounds());

        view = [createViewInstance() initWithFrame: r];
        setOwner (view, this);

        [view registerForDraggedTypes: getSupportedDragTypes()];

        resetTrackingArea (view);

        scopedObservers.emplace_back (view, frameChangedSelector, NSViewFrameDidChangeNotification, view);

        [view setPostsFrameChangedNotifications: YES];

      #if USE_COREGRAPHICS_RENDERING
        // Creating a metal renderer may fail on some systems.
        // We need to try creating the renderer before first creating a backing layer
        // so that we know whether to use a metal layer or the system default layer
        // (setWantsLayer: YES will call through to makeBackingLayer, where we check
        // whether metalRenderer is non-null).
        // The system overwrites the layer delegate set during makeBackingLayer,
        // so that must be set separately, after the layer has been created and
        // configured.
       #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
        if (@available (macOS 10.14, *))
        {
            metalRenderer = CoreGraphicsMetalLayerRenderer::create();
            layerDelegate.reset (DrxCALayerDelegate::construct (this));
        }
       #endif

        if ((windowStyleFlags & ComponentPeer::windowRequiresSynchronousCoreGraphicsRendering) == 0)
        {
            [view setWantsLayer: YES];
            [view setLayerContentsRedrawPolicy: NSViewLayerContentsRedrawDuringViewResize];
            [view layer].drawsAsynchronously = YES;
        }

       #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
        if (@available (macOS 10.14, *))
            if (metalRenderer != nullptr)
                view.layer.delegate = layerDelegate.get();
       #endif
      #endif

        if (isSharedWindow)
        {
            window = [viewToAttachTo window];
            [viewToAttachTo addSubview: view];
        }
        else
        {
            r.origin.x = (CGFloat) component.getX();
            r.origin.y = (CGFloat) component.getY();
            r = flippedScreenRect (r);

            window = [createWindowInstance() initWithContentRect: r
                                                       styleMask: getNSWindowStyleMask (windowStyleFlags)
                                                         backing: NSBackingStoreBuffered
                                                           defer: YES];
            [window setColorSpace: [NSColorSpace sRGBColorSpace]];
            setOwner (window, this);

            [window setAccessibilityElement: YES];

            [window orderOut: nil];
            [window setDelegate: (id<NSWindowDelegate>) window];

            [window setOpaque: component.isOpaque()];

            if (! [window isOpaque])
                [window setBackgroundColor: [NSColor clearColor]];

            [view setAppearance: [NSAppearance appearanceNamed: NSAppearanceNameAqua]];

            [window setHasShadow: ((windowStyleFlags & windowHasDropShadow) != 0)];

            if (component.isAlwaysOnTop())
                setAlwaysOnTop (true);

            [window setContentView: view];

            // We'll both retain and also release this on closing because plugin hosts can unexpectedly
            // close the window for us, and also tend to get cause trouble if setReleasedWhenClosed is NO.
            [window setReleasedWhenClosed: YES];
            [window retain];

            [window setExcludedFromWindowsMenu: (windowStyleFlags & windowIsTemporary) != 0];
            [window setIgnoresMouseEvents: (windowStyleFlags & windowIgnoresMouseClicks) != 0];

            setCollectionBehaviour (false);

            [window setRestorable: NO];

            if (@available (macOS 10.12, *))
                [window setTabbingMode: NSWindowTabbingModeDisallowed];

            scopedObservers.emplace_back (view, frameChangedSelector, NSWindowDidMoveNotification, window);
            scopedObservers.emplace_back (view, frameChangedSelector, NSWindowDidMiniaturizeNotification, window);
            scopedObservers.emplace_back (view, @selector (windowWillMiniaturize:), NSWindowWillMiniaturizeNotification, window);
            scopedObservers.emplace_back (view, @selector (windowDidDeminiaturize:), NSWindowDidDeminiaturizeNotification, window);
        }

        auto alpha = component.getAlpha();

        if (alpha < 1.0f)
            setAlpha (alpha);

        setTitle (component.getName());

        getNativeRealtimeModifiers = []
        {
            if ([NSEvent respondsToSelector: @selector (modifierFlags)])
                NSViewComponentPeer::updateModifiers ([NSEvent modifierFlags]);

            return ModifierKeys::currentModifiers;
        };
    }

    ~NSViewComponentPeer() override
    {
        scopedObservers.clear();

        setOwner (view, nullptr);

        if ([view superview] != nil)
        {
            redirectWillMoveToWindow (nullptr);
            [view removeFromSuperview];
        }

        if (! isSharedWindow)
        {
            setOwner (window, nullptr);
            [window setContentView: nil];
            [window close];
            [window release];
        }

        [view release];
    }

    //==============================================================================
    uk getNativeHandle() const override    { return view; }

    z0 setVisible (b8 shouldBeVisible) override
    {
        if (isSharedWindow)
        {
            if (shouldBeVisible)
                [view setHidden: false];
            else if ([window firstResponder] != view || ([window firstResponder] == view && [window makeFirstResponder: nil]))
                [view setHidden: true];
        }
        else
        {
            if (shouldBeVisible)
            {
                ++insideToFrontCall;
                [window orderFront: nil];
                --insideToFrontCall;
                handleBroughtToFront();
            }
            else
            {
                [window orderOut: nil];
            }
        }
    }

    z0 setTitle (const Txt& title) override
    {
        DRX_AUTORELEASEPOOL
        {
            if (! isSharedWindow)
                [window setTitle: juceStringToNS (title)];
        }
    }

    b8 setDocumentEditedStatus (b8 edited) override
    {
        if (! hasNativeTitleBar())
            return false;

        [window setDocumentEdited: edited];
        return true;
    }

    z0 setRepresentedFile (const File& file) override
    {
        if (! isSharedWindow)
        {
            [window setRepresentedFilename: juceStringToNS (file != File()
                                                                ? file.getFullPathName()
                                                                : Txt())];

            windowRepresentsFile = (file != File());
        }
    }

    z0 setBounds (const Rectangle<i32>& newBounds, b8) override
    {
        auto r = makeCGRect (newBounds);
        auto oldViewSize = [view frame].size;

        if (isSharedWindow)
        {
            [view setFrame: r];
        }
        else
        {
            [window setFrame: [window frameRectForContentRect: flippedScreenRect (r)]
                     display: false];
        }

        if (! CGSizeEqualToSize (oldViewSize, r.size))
            [view setNeedsDisplay: true];
    }

    Rectangle<i32> getBounds (const b8 global) const
    {
        auto r = [view frame];
        NSWindow* viewWindow = [view window];

        if (global && viewWindow != nil)
        {
            r = [[view superview] convertRect: r toView: nil];
            r = [viewWindow convertRectToScreen: r];

            r = flippedScreenRect (r);
        }

        return convertToRectInt (r);
    }

    Rectangle<i32> getBounds() const override
    {
        return getBounds (! isSharedWindow);
    }

    Point<f32> localToGlobal (Point<f32> relativePosition) override
    {
        return relativePosition + getBounds (true).getPosition().toFloat();
    }

    using ComponentPeer::localToGlobal;

    Point<f32> globalToLocal (Point<f32> screenPosition) override
    {
        return screenPosition - getBounds (true).getPosition().toFloat();
    }

    using ComponentPeer::globalToLocal;

    z0 setAlpha (f32 newAlpha) override
    {
        if (isSharedWindow)
            [view setAlphaValue: (CGFloat) newAlpha];
        else
            [window setAlphaValue: (CGFloat) newAlpha];
    }

    z0 setMinimised (b8 shouldBeMinimised) override
    {
        if (! isSharedWindow)
        {
            if (shouldBeMinimised)
                [window miniaturize: nil];
            else
                [window deminiaturize: nil];
        }
    }

    b8 isMinimised() const override
    {
        return [window isMiniaturized];
    }

    b8 isShowing() const override
    {
        return [window isVisible] && ! isMinimised();
    }

    NSWindowCollectionBehavior getCollectionBehavior (b8 forceFullScreen) const
    {
        if (forceFullScreen)
            return NSWindowCollectionBehaviorFullScreenPrimary;

        // Some SDK versions don't define NSWindowCollectionBehaviorFullScreenAuxiliary
        constexpr auto fullScreenAux = (NSUInteger) (1 << 8);

        return (getStyleFlags() & (windowHasMaximiseButton | windowIsResizable)) == (windowHasMaximiseButton | windowIsResizable)
             ? NSWindowCollectionBehaviorFullScreenPrimary
             : fullScreenAux;
    }

    z0 setCollectionBehaviour (b8 forceFullScreen) const
    {
        [window setCollectionBehavior: getCollectionBehavior (forceFullScreen)];
    }

    z0 setFullScreen (b8 shouldBeFullScreen) override
    {
        if (isSharedWindow)
            return;

        if (shouldBeFullScreen)
            setCollectionBehaviour (true);

        if (isMinimised())
            setMinimised (false);

        if (shouldBeFullScreen != isFullScreen())
            [window toggleFullScreen: nil];
    }

    b8 isFullScreen() const override
    {
        return ([window styleMask] & NSWindowStyleMaskFullScreen) != 0;
    }

    b8 isKioskMode() const override
    {
        return isFullScreen() && ComponentPeer::isKioskMode();
    }

    static b8 isWindowAtPoint (NSWindow* w, NSPoint screenPoint)
    {
        if ([NSWindow respondsToSelector: @selector (windowNumberAtPoint:belowWindowWithWindowNumber:)])
            return [NSWindow windowNumberAtPoint: screenPoint belowWindowWithWindowNumber: 0] == [w windowNumber];

        return true;
    }

    b8 contains (Point<i32> localPos, b8 trueIfInAChildWindow) const override
    {
        NSRect viewFrame = [view frame];

        if (! (isPositiveAndBelow (localPos.getX(), viewFrame.size.width)
            && isPositiveAndBelow (localPos.getY(), viewFrame.size.height)))
            return false;

        if (! SystemStats::isRunningInAppExtensionSandbox())
        {
            if (NSWindow* const viewWindow = [view window])
            {
                NSRect windowFrame = [viewWindow frame];
                NSPoint windowPoint = [view convertPoint: NSMakePoint (localPos.x, localPos.y) toView: nil];
                NSPoint screenPoint = NSMakePoint (windowFrame.origin.x + windowPoint.x,
                                                   windowFrame.origin.y + windowPoint.y);

                if (! isWindowAtPoint (viewWindow, screenPoint))
                    return false;
            }
        }

        const auto pointInSuperview = std::invoke ([&]
        {
            const auto local = NSMakePoint (localPos.x, localPos.y);

            if (auto* superview = [view superview])
                return [view convertPoint: local toView: superview];

            return local;
        });

        NSView* v = [view hitTest: pointInSuperview];

        return trueIfInAChildWindow ? (v != nil)
                                    : (v == view);
    }

    OptionalBorderSize getFrameSizeIfPresent() const override
    {
        if (! isSharedWindow)
        {
            BorderSize<i32> b;

            NSRect v = [view convertRect: [view frame] toView: nil];
            NSRect w = [window frame];

            b.setTop ((i32) (w.size.height - (v.origin.y + v.size.height)));
            b.setBottom ((i32) v.origin.y);
            b.setLeft ((i32) v.origin.x);
            b.setRight ((i32) (w.size.width - (v.origin.x + v.size.width)));

            return OptionalBorderSize { b };
        }

        return {};
    }

    BorderSize<i32> getFrameSize() const override
    {
        if (const auto frameSize = getFrameSizeIfPresent())
            return *frameSize;

        return {};
    }

    b8 hasNativeTitleBar() const
    {
        return (getStyleFlags() & windowHasTitleBar) != 0;
    }

    b8 setAlwaysOnTop (b8 alwaysOnTop) override
    {
        if (! isSharedWindow)
        {
            [window setLevel: alwaysOnTop ? ((getStyleFlags() & windowIsTemporary) != 0 ? NSPopUpMenuWindowLevel
                                                                                        : NSFloatingWindowLevel)
                                          : NSNormalWindowLevel];

            isAlwaysOnTop = alwaysOnTop;
        }

        return true;
    }

    z0 toFront (b8 makeActiveWindow) override
    {
        if (isSharedWindow)
        {
            NSView* superview = [view superview];
            NSMutableArray* subviews = [NSMutableArray arrayWithArray: [superview subviews]];

            const auto isFrontmost = [[subviews lastObject] isEqual: view];

            if (! isFrontmost)
            {
                [view retain];
                [subviews removeObject: view];
                [subviews addObject: view];

                [superview setSubviews: subviews];
                [view release];
            }
        }

        if (window != nil && component.isVisible())
        {
            ++insideToFrontCall;

            if (makeActiveWindow && ! inBecomeKeyWindow && [window canBecomeKeyWindow])
                [window makeKeyAndOrderFront: nil];
            else
                [window orderFront: nil];

            if (insideToFrontCall <= 1)
            {
                Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
                handleBroughtToFront();
            }

            --insideToFrontCall;
        }
    }

    z0 toBehind (ComponentPeer* other) override
    {
        if (auto* otherPeer = dynamic_cast<NSViewComponentPeer*> (other))
        {
            if (isSharedWindow)
            {
                NSView* superview = [view superview];
                NSMutableArray* subviews = [NSMutableArray arrayWithArray: [superview subviews]];

                const auto otherViewIndex = [subviews indexOfObject: otherPeer->view];

                if (otherViewIndex == NSNotFound)
                    return;

                const auto isBehind = [subviews indexOfObject: view] < otherViewIndex;

                if (! isBehind)
                {
                    [view retain];
                    [subviews removeObject: view];
                    [subviews insertObject: view
                                   atIndex: otherViewIndex];

                    [superview setSubviews: subviews];
                    [view release];
                }
            }
            else if (component.isVisible())
            {
                [window orderWindow: NSWindowBelow
                         relativeTo: [otherPeer->window windowNumber]];
            }
        }
        else
        {
            jassertfalse; // wrong type of window?
        }
    }

    z0 setIcon (const Image& newIcon) override
    {
        if (! isSharedWindow)
        {
            // need to set a dummy represented file here to show the file icon (which we then set to the new icon)
            if (! windowRepresentsFile)
                [window setRepresentedFilename: juceStringToNS (" ")]; // can't just use an empty string for some reason...

            auto img = NSUniquePtr<NSImage> { imageToNSImage (ScaledImage (newIcon)) };
            [[window standardWindowButton: NSWindowDocumentIconButton] setImage: img.get()];
        }
    }

    StringArray getAvailableRenderingEngines() override
    {
        StringArray s ("Software Renderer");

       #if USE_COREGRAPHICS_RENDERING
        s.add ("CoreGraphics Renderer");
       #endif

        return s;
    }

    i32 getCurrentRenderingEngine() const override
    {
        return usingCoreGraphics ? 1 : 0;
    }

    z0 setCurrentRenderingEngine ([[maybe_unused]] i32 index) override
    {
       #if USE_COREGRAPHICS_RENDERING
        if (usingCoreGraphics != (index > 0))
        {
            usingCoreGraphics = index > 0;
            [view setNeedsDisplay: true];
        }
       #endif
    }

    z0 redirectMouseDown (NSEvent* ev)
    {
        if (! Process::isForegroundProcess())
            Process::makeForegroundProcess();

        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (getModifierForButtonNumber ([ev buttonNumber]));
        sendMouseEvent (ev);
    }

    z0 redirectMouseUp (NSEvent* ev)
    {
        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutFlags (getModifierForButtonNumber ([ev buttonNumber]));
        sendMouseEvent (ev);
        showArrowCursorIfNeeded();
    }

    z0 redirectMouseDrag (NSEvent* ev)
    {
        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (getModifierForButtonNumber ([ev buttonNumber]));
        sendMouseEvent (ev);
    }

    z0 redirectMouseMove (NSEvent* ev)
    {
        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons();

        NSPoint windowPos = [ev locationInWindow];
        NSPoint screenPos = [[ev window] convertRectToScreen: NSMakeRect (windowPos.x, windowPos.y, 1.0f, 1.0f)].origin;

        if (isWindowAtPoint ([ev window], screenPos))
        {
            if (contains (getMousePos (ev, view).roundToInt(), false))
                sendMouseEvent (ev);
        }
        else
            // moved into another window which overlaps this one, so trigger an exit
            handleMouseEvent (MouseInputSource::InputSourceType::mouse, MouseInputSource::offscreenMousePos, ModifierKeys::currentModifiers,
                              getMousePressure (ev), MouseInputSource::defaultOrientation, getMouseTime (ev));

        showArrowCursorIfNeeded();
    }

    z0 redirectMouseEnter (NSEvent* ev)
    {
        sendMouseEnterExit (ev);
    }

    z0 redirectMouseExit (NSEvent* ev)
    {
        sendMouseEnterExit (ev);
    }

    static f32 checkDeviceDeltaReturnValue (f32 v) noexcept
    {
        // (deviceDeltaX can fail and return NaN, so need to sanity-check the result)
        v *= 0.5f / 256.0f;
        return (v > -1000.0f && v < 1000.0f) ? v : 0.0f;
    }

    z0 redirectMouseWheel (NSEvent* ev)
    {
        updateModifiers (ev);

        MouseWheelDetails wheel;
        wheel.deltaX = 0;
        wheel.deltaY = 0;
        wheel.isReversed = false;
        wheel.isSmooth = false;
        wheel.isInertial = false;

        @try
        {
            if ([ev respondsToSelector: @selector (isDirectionInvertedFromDevice)])
                wheel.isReversed = [ev isDirectionInvertedFromDevice];

            wheel.isInertial = ([ev momentumPhase] != NSEventPhaseNone);

            if ([ev respondsToSelector: @selector (hasPreciseScrollingDeltas)])
            {
                if ([ev hasPreciseScrollingDeltas])
                {
                    const f32 scale = 0.5f / 256.0f;
                    wheel.deltaX = scale * (f32) [ev scrollingDeltaX];
                    wheel.deltaY = scale * (f32) [ev scrollingDeltaY];
                    wheel.isSmooth = true;
                }
            }
            else if ([ev respondsToSelector: @selector (deviceDeltaX)])
            {
                wheel.deltaX = checkDeviceDeltaReturnValue ([ev deviceDeltaX]);
                wheel.deltaY = checkDeviceDeltaReturnValue ([ev deviceDeltaY]);
            }
        }
        @catch (...)
        {}

        if (wheel.deltaX == 0.0f && wheel.deltaY == 0.0f)
        {
            const f32 scale = 10.0f / 256.0f;
            wheel.deltaX = scale * (f32) [ev deltaX];
            wheel.deltaY = scale * (f32) [ev deltaY];
        }

        handleMouseWheel (MouseInputSource::InputSourceType::mouse, getMousePos (ev, view), getMouseTime (ev), wheel);
    }

    z0 redirectMagnify (NSEvent* ev)
    {
        const f32 invScale = 1.0f - (f32) [ev magnification];

        if (invScale > 0.0f)
            handleMagnifyGesture (MouseInputSource::InputSourceType::mouse, getMousePos (ev, view), getMouseTime (ev), 1.0f / invScale);
    }

    z0 redirectCopy      (NSObject*) { handleKeyPress (KeyPress ('c', ModifierKeys (ModifierKeys::commandModifier), 'c')); }
    z0 redirectPaste     (NSObject*) { handleKeyPress (KeyPress ('v', ModifierKeys (ModifierKeys::commandModifier), 'v')); }
    z0 redirectCut       (NSObject*) { handleKeyPress (KeyPress ('x', ModifierKeys (ModifierKeys::commandModifier), 'x')); }
    z0 redirectSelectAll (NSObject*) { handleKeyPress (KeyPress ('a', ModifierKeys (ModifierKeys::commandModifier), 'a')); }

    z0 redirectWillMoveToWindow (NSWindow* newWindow)
    {
        windowObservers.clear();

        if (isSharedWindow && [view window] == window && newWindow == nullptr)
        {
            if (auto* comp = safeComponent.get())
                comp->setVisible (false);
        }
    }

    z0 sendMouseEvent (NSEvent* ev)
    {
        updateModifiers (ev);
        handleMouseEvent (MouseInputSource::InputSourceType::mouse, getMousePos (ev, view), ModifierKeys::currentModifiers,
                          getMousePressure (ev), MouseInputSource::defaultOrientation, getMouseTime (ev));
    }

    b8 handleKeyEvent (NSEvent* ev, b8 isKeyDown)
    {
        auto unicode = nsStringToDrx ([ev characters]);
        auto keyCode = getKeyCodeFromEvent (ev);

       #if DRX_DEBUG_KEYCODES
        DBG ("unicode: " + unicode + " " + Txt::toHexString ((i32) unicode[0]));
        auto unmodified = nsStringToDrx ([ev charactersIgnoringModifiers]);
        DBG ("unmodified: " + unmodified + " " + Txt::toHexString ((i32) unmodified[0]));
       #endif

        if (keyCode != 0 || unicode.isNotEmpty())
        {
            if (isKeyDown)
            {
                b8 used = false;

                for (auto textCharacter : unicode)
                {
                    switch (keyCode)
                    {
                        case NSLeftArrowFunctionKey:
                        case NSRightArrowFunctionKey:
                        case NSUpArrowFunctionKey:
                        case NSDownArrowFunctionKey:
                        case NSPageUpFunctionKey:
                        case NSPageDownFunctionKey:
                        case NSEndFunctionKey:
                        case NSHomeFunctionKey:
                        case NSDeleteFunctionKey:
                            textCharacter = 0;
                            break; // (these all seem to generate unwanted garbage unicode strings)

                        default:
                            if (([ev modifierFlags] & NSEventModifierFlagCommand) != 0
                                 || (keyCode >= NSF1FunctionKey && keyCode <= NSF35FunctionKey))
                                textCharacter = 0;
                            break;
                    }

                    used |= handleKeyUpOrDown (true);
                    used |= handleKeyPress (keyCode, textCharacter);
                }

                return used;
            }

            if (handleKeyUpOrDown (false))
                return true;
        }

        return false;
    }

    b8 redirectKeyDown (NSEvent* ev)
    {
        // (need to retain this in case a modal loop runs in handleKeyEvent and
        // our event object gets lost)
        const NSUniquePtr<NSEvent> r ([ev retain]);

        updateKeysDown (ev, true);
        b8 used = handleKeyEvent (ev, true);

        if (([ev modifierFlags] & NSEventModifierFlagCommand) != 0)
        {
            // for command keys, the key-up event is thrown away, so simulate one..
            updateKeysDown (ev, false);
            used = (isValidPeer (this) && handleKeyEvent (ev, false)) || used;
        }

        // (If we're running modally, don't allow unused keystrokes to be passed
        // along to other blocked views..)
        if (Component::getCurrentlyModalComponent() != nullptr)
            used = true;

        return used;
    }

    b8 redirectKeyUp (NSEvent* ev)
    {
        updateKeysDown (ev, false);
        return handleKeyEvent (ev, false)
                || Component::getCurrentlyModalComponent() != nullptr;
    }

    z0 redirectModKeyChange (NSEvent* ev)
    {
        // (need to retain this in case a modal loop runs and our event object gets lost)
        const NSUniquePtr<NSEvent> r ([ev retain]);

        keysCurrentlyDown.clear();
        handleKeyUpOrDown (true);

        updateModifiers (ev);
        handleModifierKeysChange();
    }

    //==============================================================================
    z0 drawRect (NSRect r)
    {
        if (r.size.width < 1.0f || r.size.height < 1.0f)
            return;

        auto* cg = (CGContextRef) [[NSGraphicsContext currentContext] CGContext];
        drawRectWithContext (cg, r);
    }

    z0 drawRectWithContext (CGContextRef cg, NSRect r)
    {
        if (! component.isOpaque())
            CGContextClearRect (cg, CGContextGetClipBoundingBox (cg));

        f32 displayScale = 1.0f;
        NSScreen* screen = [[view window] screen];

        if ([screen respondsToSelector: @selector (backingScaleFactor)])
            displayScale = (f32) screen.backingScaleFactor;

        auto invalidateTransparentWindowShadow = [this]
        {
            // transparent NSWindows with a drop-shadow need to redraw their shadow when the content
            // changes to avoid stale shadows being drawn behind the window
            if (! isSharedWindow && ! [window isOpaque] && [window hasShadow])
                [window invalidateShadow];
        };

       #if USE_COREGRAPHICS_RENDERING && DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
        // This was a workaround for a CGContext not having a way of finding whether a rectangle
        // falls within its clip region. However, Apple removed the capability of
        // [view getRectsBeingDrawn: ...] sometime around 10.13, so on later versions of macOS
        // numRects will always be 1, and you'll need to use a CoreGraphicsMetalLayerRenderer
        // to avoid CoreGraphics consolidating disparate rects.
        if (usingCoreGraphics && metalRenderer == nullptr)
        {
            const NSRect* rects = nullptr;
            NSInteger numRects = 0;
            [view getRectsBeingDrawn: &rects count: &numRects];

            if (numRects > 1)
            {
                for (i32 i = 0; i < numRects; ++i)
                {
                    NSRect rect = rects[i];
                    CGContextSaveGState (cg);
                    CGContextClipToRect (cg, CGRectMake (rect.origin.x, rect.origin.y, rect.size.width, rect.size.height));
                    renderRect (cg, rect, displayScale);
                    CGContextRestoreGState (cg);
                }

                invalidateTransparentWindowShadow();
                return;
            }
        }
       #endif

        renderRect (cg, r, displayScale);
        invalidateTransparentWindowShadow();
    }

    z0 renderRect (CGContextRef cg, NSRect r, f32 displayScale)
    {
       #if USE_COREGRAPHICS_RENDERING
        if (usingCoreGraphics)
        {
            const auto height = getComponent().getHeight();
            CGContextConcatCTM (cg, CGAffineTransformMake (1, 0, 0, -1, 0, height));
            CoreGraphicsContext context (cg, (f32) height);
            handlePaint (context);
        }
        else
       #endif
        {
            const Point<i32> offset (-roundToInt (r.origin.x), -roundToInt (r.origin.y));
            auto clipW = (i32) (r.size.width  + 0.5f);
            auto clipH = (i32) (r.size.height + 0.5f);

            RectangleList<i32> clip;
            getClipRects (clip, offset, clipW, clipH);

            if (! clip.isEmpty())
            {
                Image temp (component.isOpaque() ? Image::RGB : Image::ARGB,
                            roundToInt ((f32) clipW * displayScale),
                            roundToInt ((f32) clipH * displayScale),
                            ! component.isOpaque());

                {
                    auto intScale = roundToInt (displayScale);

                    if (intScale != 1)
                        clip.scaleAll (intScale);

                    auto context = component.getLookAndFeel()
                                            .createGraphicsContext (temp, offset * intScale, clip);

                    if (intScale != 1)
                        context->addTransform (AffineTransform::scale (displayScale));

                    handlePaint (*context);
                }

                detail::ColorSpacePtr colourSpace { CGColorSpaceCreateWithName (kCGColorSpaceSRGB) };
                CGImageRef image = drx_createCoreGraphicsImage (temp, colourSpace.get());
                CGContextConcatCTM (cg, CGAffineTransformMake (1, 0, 0, -1, r.origin.x, r.origin.y + clipH));
                CGContextDrawImage (cg, CGRectMake (0.0f, 0.0f, clipW, clipH), image);
                CGImageRelease (image);
            }
        }
    }

    z0 repaint (const Rectangle<i32>& area) override
    {
        // In 10.11 changes were made to the way the OS handles repaint regions, and it seems that it can
        // no longer be trusted to coalesce all the regions, or to even remember them all without losing
        // a few when there's a lot of activity.
        // As a workaround for this, we use a RectangleList to do our own coalescing of regions before
        // asynchronously asking the OS to repaint them.
        deferredRepaints.add (area.toFloat());
    }

    static b8 shouldThrottleRepaint()
    {
        return areAnyWindowsInLiveResize();
    }

    z0 onVBlank (f64 timestampSec)
    {
        callVBlankListeners (timestampSec);
        setNeedsDisplayRectangles();
    }

    z0 setNeedsDisplayRectangles()
    {
        if (deferredRepaints.isEmpty())
            return;

        auto now = Time::getMillisecondCounter();
        auto msSinceLastRepaint = (lastRepaintTime >= now) ? now - lastRepaintTime
                                                           : (std::numeric_limits<u32>::max() - lastRepaintTime) + now;

        constexpr u32 minimumRepaintInterval = 1000 / 30; // 30fps

        // When windows are being resized, artificially throttling high-frequency repaints helps
        // to stop the event queue getting clogged, and keeps everything working smoothly.
        // For some reason Logic also needs this throttling to record parameter events correctly.
        if (msSinceLastRepaint < minimumRepaintInterval && shouldThrottleRepaint())
            return;

        const auto frameSize = view.frame.size;
        const Rectangle currentBounds { (f32) frameSize.width, (f32) frameSize.height };

        for (auto& i : deferredRepaints)
            [view setNeedsDisplayInRect: makeCGRect (i)];

        lastRepaintTime = Time::getMillisecondCounter();

       #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
        if (metalRenderer == nullptr)
       #endif
            deferredRepaints.clear();
    }

    z0 performAnyPendingRepaintsNow() override
    {
        [view displayIfNeeded];
    }

    static b8 areAnyWindowsInLiveResize() noexcept
    {
        for (NSWindow* w in [NSApp windows])
            if ([w inLiveResize])
                return true;

        return false;
    }

    //==============================================================================
    b8 isBlockedByModalComponent()
    {
        if (auto* modal = Component::getCurrentlyModalComponent())
        {
            if (insideToFrontCall == 0
                 && (! getComponent().isParentOf (modal))
                 && getComponent().isCurrentlyBlockedByAnotherModalComponent())
            {
                return true;
            }
        }

        return false;
    }

    enum class KeyWindowChanged { no, yes };

    z0 sendModalInputAttemptIfBlocked (KeyWindowChanged keyChanged)
    {
        if (! isBlockedByModalComponent())
            return;

        if (auto* modal = Component::getCurrentlyModalComponent())
        {
            if (auto* otherPeer = modal->getPeer())
            {
                const auto modalPeerIsTemporary = (otherPeer->getStyleFlags() & ComponentPeer::windowIsTemporary) != 0;

                if (! modalPeerIsTemporary)
                    return;

                // When a peer resigns key status, it might be because we just created a modal
                // component that is now key.
                // In this case, we should only dismiss the modal component if it isn't key,
                // implying that a third window has become key.
                const auto modalPeerIsKey = [NSApp keyWindow] == static_cast<NSViewComponentPeer*> (otherPeer)->window;

                if (keyChanged == KeyWindowChanged::yes && modalPeerIsKey)
                    return;

                modal->inputAttemptWhenModal();
            }
        }
    }

    b8 canBecomeKeyWindow()
    {
        return component.isVisible() && (getStyleFlags() & ComponentPeer::windowIgnoresKeyPresses) == 0;
    }

    b8 canBecomeMainWindow()
    {
        return component.isVisible() && dynamic_cast<ResizableWindow*> (&component) != nullptr;
    }

    b8 worksWhenModal() const
    {
        // In plugins, the host could put our plugin window inside a modal window, so this
        // allows us to successfully open other popups. Feels like there could be edge-case
        // problems caused by this, so let us know if you spot any issues..
        return ! DRXApplication::isStandaloneApp();
    }

    z0 becomeKeyWindow()
    {
        handleBroughtToFront();
        grabFocus();
    }

    z0 resignKeyWindow()
    {
        viewFocusLoss();
    }

    b8 windowShouldClose()
    {
        if (! isValidPeer (this))
            return YES;

        handleUserClosingWindow();
        return NO;
    }

    z0 redirectMovedOrResized()
    {
        handleMovedOrResized();
        setNeedsDisplayRectangles();
    }

    z0 viewMovedToWindow()
    {
        if (isSharedWindow)
        {
            auto newWindow = [view window];
            b8 shouldSetVisible = (window == nullptr && newWindow != nullptr);

            window = newWindow;

            if (shouldSetVisible)
                getComponent().setVisible (true);
        }

        if (auto* currentWindow = [view window])
        {
            windowObservers.emplace_back (view, dismissModalsSelector, NSWindowWillMoveNotification, currentWindow);
            windowObservers.emplace_back (view, dismissModalsSelector, NSWindowWillMiniaturizeNotification, currentWindow);
            windowObservers.emplace_back (view, becomeKeySelector, NSWindowDidBecomeKeyNotification, currentWindow);
            windowObservers.emplace_back (view, resignKeySelector, NSWindowDidResignKeyNotification, currentWindow);
        }
    }

    z0 dismissModals()
    {
        if (hasNativeTitleBar() || isSharedWindow)
            sendModalInputAttemptIfBlocked (KeyWindowChanged::no);
    }

    z0 becomeKey()
    {
        component.repaint();
    }

    z0 resignKey()
    {
        viewFocusLoss();
        sendModalInputAttemptIfBlocked (KeyWindowChanged::yes);
    }

    z0 liveResizingStart()
    {
        if (constrainer == nullptr)
            return;

        constrainer->resizeStart();
        isFirstLiveResize = true;

        setFullScreenSizeConstraints (*constrainer);
    }

    z0 liveResizingEnd()
    {
        if (constrainer != nullptr)
            constrainer->resizeEnd();
    }

    NSRect constrainRect (const NSRect r)
    {
        if (constrainer == nullptr || isKioskMode() || isFullScreen())
            return r;

        const auto scale = getComponent().getDesktopScaleFactor();

        auto pos            = detail::ScalingHelpers::unscaledScreenPosToScaled (scale, convertToRectInt (flippedScreenRect (r)));
        const auto original = detail::ScalingHelpers::unscaledScreenPosToScaled (scale, convertToRectInt (flippedScreenRect ([window frame])));

        const auto screenBounds = Desktop::getInstance().getDisplays().getTotalBounds (true);

        const b8 inLiveResize = [window inLiveResize];

        if (! inLiveResize || isFirstLiveResize)
        {
            isFirstLiveResize = false;

            isStretchingTop    = (pos.getY() != original.getY() && pos.getBottom() == original.getBottom());
            isStretchingLeft   = (pos.getX() != original.getX() && pos.getRight()  == original.getRight());
            isStretchingBottom = (pos.getY() == original.getY() && pos.getBottom() != original.getBottom());
            isStretchingRight  = (pos.getX() == original.getX() && pos.getRight()  != original.getRight());
        }

        constrainer->checkBounds (pos, original, screenBounds,
                                  isStretchingTop, isStretchingLeft, isStretchingBottom, isStretchingRight);

        return flippedScreenRect (makeCGRect (detail::ScalingHelpers::scaledScreenPosToUnscaled (scale, pos)));
    }

    static z0 showArrowCursorIfNeeded()
    {
        auto& desktop = Desktop::getInstance();
        auto mouse = desktop.getMainMouseSource();

        if (mouse.getComponentUnderMouse() == nullptr
             && desktop.findComponentAt (mouse.getScreenPosition().roundToInt()) == nullptr)
        {
            [[NSCursor arrowCursor] set];
        }
    }

    static z0 updateModifiers (NSEvent* e)
    {
        updateModifiers ([e modifierFlags]);
    }

    static z0 updateModifiers (const NSUInteger flags)
    {
        i32 m = 0;

        if ((flags & NSEventModifierFlagShift) != 0)        m |= ModifierKeys::shiftModifier;
        if ((flags & NSEventModifierFlagControl) != 0)      m |= ModifierKeys::ctrlModifier;
        if ((flags & NSEventModifierFlagOption) != 0)       m |= ModifierKeys::altModifier;
        if ((flags & NSEventModifierFlagCommand) != 0)      m |= ModifierKeys::commandModifier;

        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withOnlyMouseButtons().withFlags (m);
    }

    static z0 updateKeysDown (NSEvent* ev, b8 isKeyDown)
    {
        updateModifiers (ev);

        if (auto keyCode = getKeyCodeFromEvent (ev))
        {
            if (isKeyDown)
                keysCurrentlyDown.insert (keyCode);
            else
                keysCurrentlyDown.erase (keyCode);
        }
    }

    static i32 getKeyCodeFromEvent (NSEvent* ev)
    {
        // Unfortunately, charactersIgnoringModifiers does not ignore the shift key.
        // Using [ev keyCode] is not a solution either as this will,
        // for example, return VK_KEY_Y if the key is pressed which
        // is typically located at the Y key position on a QWERTY
        // keyboard. However, on international keyboards this might not
        // be the key labeled Y (for example, on German keyboards this key
        // has a Z label). Therefore, we need to query the current keyboard
        // layout to figure out what character the key would have produced
        // if the shift key was not pressed
        Txt unmodified = nsStringToDrx ([ev charactersIgnoringModifiers]);
        auto keyCode = (i32) unmodified[0];

        if (keyCode == 0x19) // (backwards-tab)
            keyCode = '\t';
        else if (keyCode == 0x03) // (enter)
            keyCode = '\r';
        else
            keyCode = (i32) CharacterFunctions::toUpperCase ((t32) keyCode);

        // The purpose of the keyCode is to provide information about non-printing characters to facilitate
        // keyboard control over the application.
        //
        // So when keyCode is decoded as a printing character outside the ASCII range we need to replace it.
        // This holds when the keyCode is larger than 0xff and not part one of the two MacOS specific
        // non-printing ranges.
        if (keyCode > 0xff
            && ! (keyCode >= NSUpArrowFunctionKey && keyCode <= NSModeSwitchFunctionKey)
            && ! (keyCode >= extendedKeyModifier))
        {
            keyCode = translateVirtualToAsciiKeyCode ([ev keyCode]);
        }

        if (([ev modifierFlags] & NSEventModifierFlagNumericPad) != 0)
        {
            i32k numPadConversions[] = { '0', KeyPress::numberPad0, '1', KeyPress::numberPad1,
                                              '2', KeyPress::numberPad2, '3', KeyPress::numberPad3,
                                              '4', KeyPress::numberPad4, '5', KeyPress::numberPad5,
                                              '6', KeyPress::numberPad6, '7', KeyPress::numberPad7,
                                              '8', KeyPress::numberPad8, '9', KeyPress::numberPad9,
                                              '+', KeyPress::numberPadAdd, '-', KeyPress::numberPadSubtract,
                                              '*', KeyPress::numberPadMultiply, '/', KeyPress::numberPadDivide,
                                              '.', KeyPress::numberPadDecimalPoint,
                                              ',', KeyPress::numberPadDecimalPoint, // (to deal with non-english kbds)
                                              '=', KeyPress::numberPadEquals };

            for (i32 i = 0; i < numElementsInArray (numPadConversions); i += 2)
                if (keyCode == numPadConversions [i])
                    keyCode = numPadConversions [i + 1];
        }

        return keyCode;
    }

    static z64 getMouseTime (NSEvent* e) noexcept
    {
        return (Time::currentTimeMillis() - Time::getMillisecondCounter())
                 + (z64) ([e timestamp] * 1000.0);
    }

    static f32 getMousePressure (NSEvent* e) noexcept
    {
        @try
        {
            if (e.type != NSEventTypeMouseEntered && e.type != NSEventTypeMouseExited)
                return (f32) e.pressure;
        }
        @catch (NSException* e) {}
        @finally {}

        return 0.0f;
    }

    static Point<f32> getMousePos (NSEvent* e, NSView* view)
    {
        NSPoint p = [view convertPoint: [e locationInWindow] fromView: nil];
        return { (f32) p.x, (f32) p.y };
    }

    static i32 getModifierForButtonNumber (const NSInteger num)
    {
        return num == 0 ? ModifierKeys::leftButtonModifier
                        : (num == 1 ? ModifierKeys::rightButtonModifier
                                    : (num == 2 ? ModifierKeys::middleButtonModifier : 0));
    }

    static u32 getNSWindowStyleMask (i32k flags) noexcept
    {
        u32 style = (flags & windowHasTitleBar) != 0 ? NSWindowStyleMaskTitled
                                                              : NSWindowStyleMaskBorderless;

        if ((flags & windowHasMinimiseButton) != 0)  style |= NSWindowStyleMaskMiniaturizable;
        if ((flags & windowHasCloseButton) != 0)     style |= NSWindowStyleMaskClosable;
        if ((flags & windowIsResizable) != 0)        style |= NSWindowStyleMaskResizable;
        return style;
    }

    static NSArray* getSupportedDragTypes()
    {
        const auto type = []
        {
            if (@available (macOS 10.13, *))
                return NSPasteboardTypeFileURL;

            DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
            return (NSString*) kUTTypeFileURL;
            DRX_END_IGNORE_DEPRECATION_WARNINGS
        }();

        return [NSArray arrayWithObjects: type, (NSString*) kPasteboardTypeFileURLPromise, NSPasteboardTypeString, nil];
    }

    BOOL sendDragCallback (b8 (ComponentPeer::* callback) (const DragInfo&), id <NSDraggingInfo> sender)
    {
        NSPasteboard* pasteboard = [sender draggingPasteboard];
        NSString* contentType = [pasteboard availableTypeFromArray: getSupportedDragTypes()];

        if (contentType == nil)
            return false;

        const auto p = localToGlobal (convertToPointFloat ([view convertPoint: [sender draggingLocation] fromView: nil]));

        ComponentPeer::DragInfo dragInfo;
        dragInfo.position = detail::ScalingHelpers::screenPosToLocalPos (component, p).roundToInt();

        if (contentType == NSPasteboardTypeString)
            dragInfo.text = nsStringToDrx ([pasteboard stringForType: NSPasteboardTypeString]);
        else
            dragInfo.files = getDroppedFiles (pasteboard, contentType);

        if (! dragInfo.isEmpty())
            return (this->*callback) (dragInfo);

        return false;
    }

    StringArray getDroppedFiles (NSPasteboard* pasteboard, NSString* contentType)
    {
        StringArray files;
        NSString* iTunesPasteboardType = nsStringLiteral ("CorePasteboardFlavorType 0x6974756E"); // 'itun'

        if ([contentType isEqualToString: (NSString*) kPasteboardTypeFileURLPromise]
             && [[pasteboard types] containsObject: iTunesPasteboardType])
        {
            id list = [pasteboard propertyListForType: iTunesPasteboardType];

            if ([list isKindOfClass: [NSDictionary class]])
            {
                NSDictionary* iTunesDictionary = (NSDictionary*) list;
                NSArray* tracks = [iTunesDictionary valueForKey: nsStringLiteral ("Tracks")];
                NSEnumerator* enumerator = [tracks objectEnumerator];
                NSDictionary* track;

                while ((track = [enumerator nextObject]) != nil)
                {
                    if (id value = [track valueForKey: nsStringLiteral ("Location")])
                    {
                        NSURL* url = [NSURL URLWithString: value];

                        if ([url isFileURL])
                            files.add (nsStringToDrx ([url path]));
                    }
                }
            }
        }
        else
        {
            NSArray* items = [pasteboard readObjectsForClasses:@[[NSURL class]] options: nil];

            for (u32 i = 0; i < [items count]; ++i)
            {
                NSURL* url = [items objectAtIndex: i];

                if ([url isFileURL])
                    files.add (nsStringToDrx ([url path]));
            }
        }

        return files;
    }

    //==============================================================================
    z0 viewFocusGain()
    {
        if (currentlyFocusedPeer != this)
        {
            if (ComponentPeer::isValidPeer (currentlyFocusedPeer))
                currentlyFocusedPeer->handleFocusLoss();

            currentlyFocusedPeer = this;
            handleFocusGain();
        }
    }

    z0 viewFocusLoss()
    {
        if (currentlyFocusedPeer == this)
        {
            currentlyFocusedPeer = nullptr;
            handleFocusLoss();
        }
    }

    b8 isFocused() const override
    {
        return (isSharedWindow || ! DRXApplication::isStandaloneApp())
                    ? this == currentlyFocusedPeer
                    : [window isKeyWindow];
    }

    z0 grabFocus() override
    {
        if (window != nil)
        {
            if (! inBecomeKeyWindow && [window canBecomeKeyWindow])
                [window makeKeyWindow];

            [window makeFirstResponder: view];

            viewFocusGain();
        }
    }

    z0 textInputRequired (Point<i32>, TextInputTarget&) override {}

    z0 closeInputMethodContext() override
    {
        stringBeingComposed.clear();

        if (const auto* inputContext = [view inputContext])
            [inputContext discardMarkedText];
    }

    z0 resetWindowPresentation()
    {
        [window setStyleMask: (NSViewComponentPeer::getNSWindowStyleMask (getStyleFlags()))];

        if (hasNativeTitleBar())
            setTitle (getComponent().getName()); // required to force the OS to update the title

        [NSApp setPresentationOptions: NSApplicationPresentationDefault];
        setCollectionBehaviour (isFullScreen());
    }

    z0 setHasChangedSinceSaved (b8 b) override
    {
        if (! isSharedWindow)
            [window setDocumentEdited: b];
    }

    b8 sendEventToInputContextOrComponent (NSEvent* ev)
    {
        // In the case that an event was processed unsuccessfully in performKeyEquivalent and then
        // posted back to keyDown by the system, this check will ensure that we don't attempt to
        // process the same event a second time.
        const auto newEvent = KeyEventAttributes::make (ev);

        if (std::exchange (lastSeenKeyEvent, newEvent) == newEvent)
            return false;

        // We assume that the event will be handled by the IME.
        // Occasionally, the inputContext may be sent key events like cmd+Q, which it will turn
        // into a noop: call and forward to doCommandBySelector:.
        // In this case, the event will be extracted from keyEventBeingHandled and passed to the
        // focused component, and viewCannotHandleEvent will be set depending on whether the event
        // was handled by the component.
        // If the event was *not* handled by the component, and was also not consumed completely by
        // the IME, it's important to return the event to the system for further handling, so that
        // the main menu works as expected.
        viewCannotHandleEvent = false;
        keyEventBeingHandled.reset ([ev retain]);
        const WeakReference ref { this };
        // redirectKeyDown may delete this peer!
        const ScopeGuard scope { [&ref] { if (ref != nullptr) ref->keyEventBeingHandled = nullptr; } };

        const auto handled = [&]() -> b8
        {
            if (findCurrentTextInputTarget() != nullptr)
                if (const auto* inputContext = [view inputContext])
                    return [inputContext handleEvent: ev] && ! viewCannotHandleEvent;

            return false;
        }();

        if (handled)
            return true;

        stringBeingComposed.clear();
        return redirectKeyDown (ev);
    }

    //==============================================================================
    class KeyEventAttributes
    {
        auto tie() const
        {
            return std::tie (type,
                             modifierFlags,
                             timestamp,
                             windowNumber,
                             characters,
                             charactersIgnoringModifiers,
                             keyCode,
                             isRepeat);
        }

    public:
        static std::optional<KeyEventAttributes> make (NSEvent* event)
        {
            const auto type = [event type];

            if (type != NSEventTypeKeyDown && type != NSEventTypeKeyUp)
                return {};

            return KeyEventAttributes
            {
                type,
                [event modifierFlags],
                [event timestamp],
                [event windowNumber],
                nsStringToDrx ([event characters]),
                nsStringToDrx ([event charactersIgnoringModifiers]),
                [event keyCode],
                static_cast<b8> ([event isARepeat])
            };
        }

        b8 operator== (const KeyEventAttributes& other) const { return tie() == other.tie(); }
        b8 operator!= (const KeyEventAttributes& other) const { return tie() != other.tie(); }

    private:
        KeyEventAttributes (NSEventType typeIn,
                            NSEventModifierFlags flagsIn,
                            NSTimeInterval timestampIn,
                            NSInteger windowNumberIn,
                            Txt charactersIn,
                            Txt charactersIgnoringModifiersIn,
                            u16 keyCodeIn,
                            b8 isRepeatIn)
            : type (typeIn),
              modifierFlags (flagsIn),
              timestamp (timestampIn),
              windowNumber (windowNumberIn),
              characters (charactersIn),
              charactersIgnoringModifiers (charactersIgnoringModifiersIn),
              keyCode (keyCodeIn),
              isRepeat (isRepeatIn)
        {}

        NSEventType type;
        NSEventModifierFlags modifierFlags;
        NSTimeInterval timestamp;
        NSInteger windowNumber;
        Txt characters;
        Txt charactersIgnoringModifiers;
        u16 keyCode;
        b8 isRepeat;
    };

    NSWindow* window = nil;
    NSView* view = nil;
    WeakReference<Component> safeComponent;
    const b8 isSharedWindow = false;
   #if USE_COREGRAPHICS_RENDERING
    b8 usingCoreGraphics = true;
   #else
    b8 usingCoreGraphics = false;
   #endif
    NSUniquePtr<NSEvent> keyEventBeingHandled;
    b8 isFirstLiveResize = false, viewCannotHandleEvent = false;
    b8 isStretchingTop = false, isStretchingLeft = false, isStretchingBottom = false, isStretchingRight = false;
    b8 windowRepresentsFile = false;
    b8 isAlwaysOnTop = false, wasAlwaysOnTop = false, inBecomeKeyWindow = false;
    b8 inPerformKeyEquivalent = false;
    Txt stringBeingComposed;
    i32 startOfMarkedTextInTextInputTarget = 0;

    Rectangle<f32> lastSizeBeforeZoom;
    RectangleList<f32> deferredRepaints;
    u32 lastRepaintTime;

    std::optional<KeyEventAttributes> lastSeenKeyEvent;

    static inline ComponentPeer* currentlyFocusedPeer;
    static inline std::set<i32> keysCurrentlyDown;
    static inline i32 insideToFrontCall;

    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
    static inline const auto dismissModalsSelector  = @selector (dismissModals);
    static inline const auto frameChangedSelector   = @selector (frameChanged:);
    static inline const auto asyncMouseDownSelector = @selector (asyncMouseDown:);
    static inline const auto asyncMouseUpSelector   = @selector (asyncMouseUp:);
    static inline const auto becomeKeySelector      = @selector (becomeKey:);
    static inline const auto resignKeySelector      = @selector (resignKey:);
    DRX_END_IGNORE_WARNINGS_GCC_LIKE

   #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    std::unique_ptr<CoreGraphicsMetalLayerRenderer> metalRenderer;
    NSUniquePtr<NSObject<CALayerDelegate>> layerDelegate;
   #endif

private:
    DRX_DECLARE_WEAK_REFERENCEABLE (NSViewComponentPeer)

    // Note: the OpenGLContext also has a SharedResourcePointer<PerScreenDisplayLinks> to
    // avoid unnecessarily duplicating display-link threads.
    SharedResourcePointer<PerScreenDisplayLinks> sharedDisplayLinks;

    class AsyncRepainter final : private AsyncUpdater
    {
    public:
        explicit AsyncRepainter (NSViewComponentPeer& o) : owner (o) {}
        ~AsyncRepainter() override { cancelPendingUpdate(); }

        z0 markUpdated (const CGDirectDisplayID x, f64 timestampSec)
        {
            {
                const std::scoped_lock lock { mutex };

                if (const auto it = std::find_if (backgroundVBlankEvents.cbegin(),
                                                  backgroundVBlankEvents.cend(),
                                                  [&x] (const auto& event) { return event.display == x; });
                    it == backgroundVBlankEvents.cend())
                {
                    backgroundVBlankEvents.push_back ({ x, timestampSec });
                }
            }

            triggerAsyncUpdate();
        }

    private:
        z0 handleAsyncUpdate() override
        {
            {
                const std::scoped_lock lock { mutex };
                mainThreadVBlankEvents = backgroundVBlankEvents;
                backgroundVBlankEvents.clear();
            }

            for (const auto& event : mainThreadVBlankEvents)
            {
                if (auto* peerView = owner.view)
                    if (auto* peerWindow = [peerView window])
                        if (event.display == ScopedDisplayLink::getDisplayIdForScreen ([peerWindow screen]))
                            owner.onVBlank (event.timeSec);
            }
        }

        struct VBlankEvent
        {
            CGDirectDisplayID display{};
            f64 timeSec{};
        };

        NSViewComponentPeer& owner;
        std::mutex mutex;
        std::vector<VBlankEvent> backgroundVBlankEvents, mainThreadVBlankEvents;
    };

    AsyncRepainter asyncRepainter { *this };

    /*  Creates a function object that can be called from an arbitrary thread (probably a CVLink
        thread). When called, this function object will trigger a call to setNeedsDisplayRectangles
        as soon as possible on the main thread, for any peers currently on the provided NSScreen.
    */
    PerScreenDisplayLinks::Connection connection
    {
        sharedDisplayLinks->registerFactory ([this] (CGDirectDisplayID display)
        {
            return [this, display] (f64 timestampSec)
                   {
                       asyncRepainter.markUpdated (display, timestampSec);
                   };
        })
    };

    static NSView* createViewInstance();
    static NSWindow* createWindowInstance();

    z0 sendMouseEnterExit (NSEvent* ev)
    {
        if (auto* area = [ev trackingArea])
            if (! [[view trackingAreas] containsObject: area])
                return;

        if ([NSEvent pressedMouseButtons] == 0)
            sendMouseEvent (ev);
    }

    static z0 setOwner (id viewOrWindow, NSViewComponentPeer* newOwner)
    {
        object_setInstanceVariable (viewOrWindow, "owner", newOwner);
    }

    z0 getClipRects (RectangleList<i32>& clip, Point<i32> offset, i32 clipW, i32 clipH)
    {
        const NSRect* rects = nullptr;
        NSInteger numRects = 0;
        [view getRectsBeingDrawn: &rects count: &numRects];

        const Rectangle<i32> clipBounds (clipW, clipH);

        clip.ensureStorageAllocated ((i32) numRects);

        for (i32 i = 0; i < numRects; ++i)
            clip.addWithoutMerging (clipBounds.getIntersection (Rectangle<i32> (roundToInt (rects[i].origin.x) + offset.x,
                                                                                roundToInt (rects[i].origin.y) + offset.y,
                                                                                roundToInt (rects[i].size.width),
                                                                                roundToInt (rects[i].size.height))));
    }

    static z0 appFocusChanged()
    {
        keysCurrentlyDown.clear();

        if (isValidPeer (currentlyFocusedPeer))
        {
            if (Process::isForegroundProcess())
            {
                currentlyFocusedPeer->handleFocusGain();
                ModalComponentManager::getInstance()->bringModalComponentsToFront();
            }
            else
            {
                currentlyFocusedPeer->handleFocusLoss();
            }
        }
    }

    static b8 checkEventBlockedByModalComps (NSEvent* e)
    {
        if (Component::getNumCurrentlyModalComponents() == 0)
            return false;

        NSWindow* const w = [e window];

        if (w == nil || [w worksWhenModal])
            return false;

        b8 isKey = false, isInputAttempt = false;

        switch ([e type])
        {
            case NSEventTypeKeyDown:
            case NSEventTypeKeyUp:
                isKey = isInputAttempt = true;
                break;

            case NSEventTypeLeftMouseDown:
            case NSEventTypeRightMouseDown:
            case NSEventTypeOtherMouseDown:
                isInputAttempt = true;
                break;

            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeLeftMouseUp:
            case NSEventTypeRightMouseUp:
            case NSEventTypeOtherMouseUp:
            case NSEventTypeOtherMouseDragged:
                if (Desktop::getInstance().getDraggingMouseSource (0) != nullptr)
                    return false;
                break;

            case NSEventTypeMouseMoved:
            case NSEventTypeMouseEntered:
            case NSEventTypeMouseExited:
            case NSEventTypeCursorUpdate:
            case NSEventTypeScrollWheel:
            case NSEventTypeTabletPoint:
            case NSEventTypeTabletProximity:
                break;

            case NSEventTypeFlagsChanged:
            case NSEventTypeAppKitDefined:
            case NSEventTypeSystemDefined:
            case NSEventTypeApplicationDefined:
            case NSEventTypePeriodic:
            case NSEventTypeGesture:
            case NSEventTypeMagnify:
            case NSEventTypeSwipe:
            case NSEventTypeRotate:
            case NSEventTypeBeginGesture:
            case NSEventTypeEndGesture:
            case NSEventTypeQuickLook:
            case NSEventTypeSmartMagnify:
            case NSEventTypePressure:
            case NSEventTypeDirectTouch:
            case NSEventTypeChangeMode:
            default:
                return false;
        }

        for (i32 i = ComponentPeer::getNumPeers(); --i >= 0;)
        {
            if (auto* peer = dynamic_cast<NSViewComponentPeer*> (ComponentPeer::getPeer (i)))
            {
                if ([peer->view window] == w)
                {
                    if (isKey)
                    {
                        if (peer->view == [w firstResponder])
                            return false;
                    }
                    else
                    {
                        if (peer->isSharedWindow
                               ? NSPointInRect ([peer->view convertPoint: [e locationInWindow] fromView: nil], [peer->view bounds])
                               : NSPointInRect ([e locationInWindow], NSMakeRect (0, 0, [w frame].size.width, [w frame].size.height)))
                            return false;
                    }
                }
            }
        }

        if (isInputAttempt)
        {
            if (! [NSApp isActive])
                [NSApp activateIgnoringOtherApps: YES];

            if (auto* modal = Component::getCurrentlyModalComponent())
                modal->inputAttemptWhenModal();
        }

        return true;
    }

    z0 setFullScreenSizeConstraints (const ComponentBoundsConstrainer& c)
    {
        const auto minSize = NSMakeSize (static_cast<f32> (c.getMinimumWidth()), 0.0f);
        [window setMinFullScreenContentSize: minSize];
        [window setMaxFullScreenContentSize: NSMakeSize (100000, 100000)];
    }

    z0 displayLayer ([[maybe_unused]] CALayer* layer) override
    {
       #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
        if (metalRenderer == nullptr)
            return;

        const auto scale = [this]
        {
            if (auto* viewWindow = [view window])
                return (f32) viewWindow.backingScaleFactor;

            return 1.0f;
        }();

        deferredRepaints = metalRenderer->drawRectangleList (static_cast<CAMetalLayer*> (layer),
                                                             scale,
                                                             [this] (auto&&... args) { drawRectWithContext (args...); },
                                                             std::move (deferredRepaints),
                                                             [view inLiveResize]);
       #endif
    }

    /*  Used to store and restore the values of the NSWindowStyleMaskClosable and
        NSWindowStyleMaskMiniaturizable flags.
    */
    struct StoredStyleFlags
    {
        StoredStyleFlags (NSWindowStyleMask m)
            : stored { m }
        {}

        static auto getStoredFlags()
        {
            return std::array<NSWindowStyleMask, 2> { NSWindowStyleMaskClosable,
                                                      NSWindowStyleMaskMiniaturizable };
        }

        auto withFlagsRestored (NSWindowStyleMask m) const
        {
            for (const auto& f : getStoredFlags())
                m = withFlagFromStored (m, f);

            return m;
        }

    private:
        NSWindowStyleMask withFlagFromStored (NSWindowStyleMask m, NSWindowStyleMask flag) const
        {
            return (m & ~flag) | (stored & flag);
        }

        NSWindowStyleMask stored;
    };

    z0 modalComponentManagerChanged()
    {
        // We are only changing the style flags if we absolutely have to. Plugin windows generally
        // don't like to be modified. Windows created under plugin hosts running in an external
        // subprocess are particularly touchy, and may make the window invisible even if we call
        // [window setStyleMask [window setStyleMask]].
        if (isSharedWindow || ! hasNativeTitleBar())
            return;

        const auto newStyleMask = [&]() -> std::optional<NSWindowStyleMask>
        {
            const auto currentStyleMask = [window styleMask];

            if (ModalComponentManager::getInstance()->getNumModalComponents() > 0)
            {
                if (! storedFlags)
                    storedFlags.emplace (currentStyleMask);

                auto updatedMask = (storedFlags->withFlagsRestored (currentStyleMask)) & ~NSWindowStyleMaskMiniaturizable;

                if (component.isCurrentlyBlockedByAnotherModalComponent())
                    updatedMask &= ~NSWindowStyleMaskClosable;

                return updatedMask;
            }

            if (storedFlags)
            {
                const auto flagsToApply = storedFlags->withFlagsRestored (currentStyleMask);
                storedFlags.reset();
                return flagsToApply;
            }

            return {};
        }();

        if (newStyleMask && *newStyleMask != [window styleMask])
            [window setStyleMask: *newStyleMask];
    }

    //==============================================================================
    std::vector<ScopedNotificationCenterObserver> scopedObservers;
    std::vector<ScopedNotificationCenterObserver> windowObservers;

    std::optional<StoredStyleFlags> storedFlags;
    ErasedScopeGuard modalChangeListenerScope =
        detail::ComponentHelpers::ModalComponentManagerChangeNotifier::getInstance().addListener ([this]
                                                                                                  {
                                                                                                      modalComponentManagerChanged();
                                                                                                  });

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewComponentPeer)
};

//==============================================================================
template <typename Base>
struct NSViewComponentPeerWrapper : public Base
{
    explicit NSViewComponentPeerWrapper (tukk baseName)
        : Base (baseName)
    {
        Base::template addIvar<NSViewComponentPeer*> ("owner");
    }

    static NSViewComponentPeer* getOwner (id self)
    {
        return getIvar<NSViewComponentPeer*> (self, "owner");
    }

    static id getAccessibleChild (id self)
    {
        if (auto* owner = getOwner (self))
            if (auto* handler = owner->getComponent().getAccessibilityHandler())
                return (id) handler->getNativeImplementation();

        return nil;
    }
};

//==============================================================================
struct DrxNSViewClass final : public NSViewComponentPeerWrapper<ObjCClass<NSView>>
{
    DrxNSViewClass()  : NSViewComponentPeerWrapper ("DRXView_")
    {
        addMethod (@selector (isOpaque), [] (id self, SEL)
        {
            auto* owner = getOwner (self);
            return owner == nullptr || owner->getComponent().isOpaque();
        });

        addMethod (@selector (updateTrackingAreas), [] (id self, SEL)
        {
            sendSuperclassMessage<z0> (self, @selector (updateTrackingAreas));

            resetTrackingArea (static_cast<NSView*> (self));
        });

        addMethod (@selector (becomeFirstResponder), [] (id self, SEL)
        {
            callOnOwner (self, &NSViewComponentPeer::viewFocusGain);
            return YES;
        });

        addMethod (@selector (resignFirstResponder), [] (id self, SEL)
        {
            callOnOwner (self, &NSViewComponentPeer::viewFocusLoss);
            return YES;
        });

        addMethod (NSViewComponentPeer::dismissModalsSelector,  [] (id self, SEL)                    { callOnOwner (self, &NSViewComponentPeer::dismissModals); });
        addMethod (NSViewComponentPeer::frameChangedSelector,   [] (id self, SEL, NSNotification*)   { callOnOwner (self, &NSViewComponentPeer::redirectMovedOrResized); });
        addMethod (NSViewComponentPeer::becomeKeySelector,      [] (id self, SEL)                    { callOnOwner (self, &NSViewComponentPeer::becomeKey); });
        addMethod (NSViewComponentPeer::resignKeySelector,      [] (id self, SEL)                    { callOnOwner (self, &NSViewComponentPeer::resignKey); });

        addMethod (@selector (paste:),                          [] (id self, SEL, NSObject* s)       { callOnOwner (self, &NSViewComponentPeer::redirectPaste,            s);  });
        addMethod (@selector (copy:),                           [] (id self, SEL, NSObject* s)       { callOnOwner (self, &NSViewComponentPeer::redirectCopy,             s);  });
        addMethod (@selector (cut:),                            [] (id self, SEL, NSObject* s)       { callOnOwner (self, &NSViewComponentPeer::redirectCut,              s);  });
        addMethod (@selector (selectAll:),                      [] (id self, SEL, NSObject* s)       { callOnOwner (self, &NSViewComponentPeer::redirectSelectAll,        s);  });

        addMethod (@selector (viewWillMoveToWindow:),           [] (id self, SEL, NSWindow* w)       { callOnOwner (self, &NSViewComponentPeer::redirectWillMoveToWindow, w); });

        addMethod (@selector (drawRect:),                       [] (id self, SEL, NSRect r)          { callOnOwner (self, &NSViewComponentPeer::drawRect, r); });
        addMethod (@selector (viewDidMoveToWindow),             [] (id self, SEL)                    { callOnOwner (self, &NSViewComponentPeer::viewMovedToWindow); });
        addMethod (@selector (flagsChanged:),                   [] (id self, SEL, NSEvent* ev)       { callOnOwner (self, &NSViewComponentPeer::redirectModKeyChange, ev); });
        addMethod (@selector (mouseMoved:),                     [] (id self, SEL, NSEvent* ev)       { callOnOwner (self, &NSViewComponentPeer::redirectMouseMove,    ev); });
        addMethod (@selector (mouseEntered:),                   [] (id self, SEL, NSEvent* ev)       { callOnOwner (self, &NSViewComponentPeer::redirectMouseEnter,   ev); });
        addMethod (@selector (mouseExited:),                    [] (id self, SEL, NSEvent* ev)       { callOnOwner (self, &NSViewComponentPeer::redirectMouseExit,    ev); });
        addMethod (@selector (scrollWheel:),                    [] (id self, SEL, NSEvent* ev)       { callOnOwner (self, &NSViewComponentPeer::redirectMouseWheel,   ev); });
        addMethod (@selector (magnifyWithEvent:),               [] (id self, SEL, NSEvent* ev)       { callOnOwner (self, &NSViewComponentPeer::redirectMagnify,      ev); });

        addMethod (@selector (mouseDragged:),                   mouseDragged);
        addMethod (@selector (rightMouseDragged:),              mouseDragged);
        addMethod (@selector (otherMouseDragged:),              mouseDragged);

        addMethod (NSViewComponentPeer::asyncMouseDownSelector, asyncMouseDown);
        addMethod (NSViewComponentPeer::asyncMouseUpSelector,   asyncMouseUp);

        addMethod (@selector (mouseDown:),                      mouseDown);
        addMethod (@selector (rightMouseDown:),                 mouseDown);
        addMethod (@selector (otherMouseDown:),                 mouseDown);

        addMethod (@selector (mouseUp:),                        mouseUp);
        addMethod (@selector (rightMouseUp:),                   mouseUp);
        addMethod (@selector (otherMouseUp:),                   mouseUp);

        addMethod (@selector (draggingEntered:),                draggingUpdated);
        addMethod (@selector (draggingUpdated:),                draggingUpdated);

        addMethod (@selector (draggingEnded:),                  draggingExited);
        addMethod (@selector (draggingExited:),                 draggingExited);

        DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        addMethod (@selector (clipsToBounds), [] (id, SEL) { return YES; });
        DRX_END_IGNORE_WARNINGS_GCC_LIKE

        addMethod (@selector (acceptsFirstMouse:), [] (id, SEL, NSEvent*) { return YES; });

       #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
        addMethod (@selector (makeBackingLayer), [] (id self, SEL) -> CALayer*
        {
            if (auto* owner = getOwner (self))
            {
                if (owner->metalRenderer != nullptr)
                {
                    auto* layer = [CAMetalLayer layer];

                    layer.device = MTLCreateSystemDefaultDevice();
                    layer.framebufferOnly = NO;
                    layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
                    layer.opaque = getOwner (self)->getComponent().isOpaque();
                    layer.needsDisplayOnBoundsChange = YES;
                    layer.drawsAsynchronously = YES;
                    layer.delegate = owner->layerDelegate.get();

                    if (@available (macOS 10.13, *))
                        layer.allowsNextDrawableTimeout = NO;

                    return layer;
                }
            }

            return sendSuperclassMessage<CALayer*> (self, @selector (makeBackingLayer));
        });
       #endif

        addMethod (@selector (windowWillMiniaturize:), [] (id self, SEL, NSNotification*)
        {
            if (auto* p = getOwner (self))
            {
                if (p->isAlwaysOnTop)
                {
                    // there is a bug when restoring minimised always on top windows so we need
                    // to remove this behaviour before minimising and restore it afterwards
                    p->setAlwaysOnTop (false);
                    p->wasAlwaysOnTop = true;
                }
            }
        });

        addMethod (@selector (windowDidDeminiaturize:), [] (id self, SEL, NSNotification*)
        {
            if (auto* p = getOwner (self))
            {
                if (p->wasAlwaysOnTop)
                    p->setAlwaysOnTop (true);

                p->redirectMovedOrResized();
            }
        });

        addMethod (@selector (wantsDefaultClipping), [] (id, SEL) { return YES; }); // (this is the default, but may want to customise it in future)

        addMethod (@selector (worksWhenModal), [] (id self, SEL)
        {
            if (auto* p = getOwner (self))
                return p->worksWhenModal();

            return false;
        });

        addMethod (@selector (viewWillDraw), [] (id self, SEL)
        {
            // Without setting contentsFormat macOS Big Sur will always set the invalid area
            // to be the entire frame.
            if (@available (macOS 10.12, *))
            {
                CALayer* layer = ((NSView*) self).layer;
                layer.contentsFormat = kCAContentsFormatRGBA8Uint;
            }

            sendSuperclassMessage<z0> (self, @selector (viewWillDraw));
        });

        addMethod (@selector (keyDown:), [] (id self, SEL, NSEvent* ev)
        {
            const auto handled = [&]
            {
                if (auto* owner = getOwner (self))
                    return owner->sendEventToInputContextOrComponent (ev);

                return false;
            }();

            if (! handled)
                sendSuperclassMessage<z0> (self, @selector (keyDown:), ev);
        });

        addMethod (@selector (keyUp:), [] (id self, SEL, NSEvent* ev)
        {
            auto* owner = getOwner (self);

            if (! owner->redirectKeyUp (ev))
                sendSuperclassMessage<z0> (self, @selector (keyUp:), ev);
        });

        // See "The Path of Key Events" on this page:
        // https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/EventOverview/EventArchitecture/EventArchitecture.html
        // Normally, 'special' key presses (cursor keys, shortcuts, return, delete etc.) will be
        // sent down the view hierarchy to this function before any of the other keyboard handling
        // functions.
        // If any object returns YES from performKeyEquivalent, then the event is consumed.
        // If no object handles the key equivalent, then the event will be sent to the main menu.
        // If the menu is also unable to respond to the event, then the event will be sent
        // to keyDown:/keyUp: via sendEvent:, but this time the event will be sent to the first
        // responder and propagated back up the responder chain.
        // This architecture presents some issues in DRX apps, which always expect the focused
        // Component to be sent all key presses *including* special keys.
        // There are also some slightly pathological cases that DRX needs to support, for example
        // the situation where one of the cursor keys is bound to a main menu item. By default,
        // macOS would send the cursor event to performKeyEquivalent on each NSResponder, then send
        // the event to the main menu if no responder handled it. This would mean that the focused
        // Component would never see the event, which would break widgets like the TextEditor, which
        // expect to take precedence over menu items when they have focus.
        // Another layer of subtlety is that some IMEs require cursor key input. When i64-pressing
        // the 'e' key to bring up the accent menu, the popup menu should receive cursor events
        // before the focused component.
        // To fulfil all of these requirements, we handle special keys ('key equivalents') like any
        // other key event, and send these events firstly to the NSTextInputContext (if there's an
        // active TextInputTarget), and then on to the focused Component in the case that the
        // input handler is unable to use the keypress. If the event still hasn't been used, then
        // it will be sent to the superclass's performKeyEquivalent: function, which will give the
        // OS a chance to handle events like cmd+Q, cmd+`, cmd+H etc.
        addMethod (@selector (performKeyEquivalent:), [] (id self, SEL s, NSEvent* ev) -> BOOL
        {
            if (auto* owner = getOwner (self))
            {
                const auto ref = owner->safeComponent;
                const auto prev = std::exchange (owner->inPerformKeyEquivalent, true);

                const ScopeGuard scope { [&ref, owner, prev]
                {
                    if (ref != nullptr)
                        owner->inPerformKeyEquivalent = prev;
                } };

                if (owner->sendEventToInputContextOrComponent (ev))
                {
                    if (ref == nullptr)
                        return YES;

                    const auto isFirstResponder = [&]
                    {
                        if (auto* v = owner->view)
                            if (auto* w = v.window)
                                return w.firstResponder == self;

                        return false;
                    }();

                    // If the view isn't the first responder, but the view has successfully
                    // performed the key equivalent, then the key event must have been passed down
                    // the view hierarchy to this point. In that case, the view won't be sent a
                    // matching keyUp event, so we simulate it here.
                    if (! isFirstResponder)
                        owner->redirectKeyUp (ev);

                    return YES;
                }
            }

            return sendSuperclassMessage<BOOL> (self, s, ev);
        });

        addMethod (@selector (insertText:replacementRange:), [] (id self, SEL, id aString, NSRange replacementRange)
        {
            // This commits multi-byte text when using an IME, or after every keypress for western keyboards
            if (auto* owner = getOwner (self))
            {
                if (auto* target = owner->findCurrentTextInputTarget())
                {
                    const auto newText = nsStringToDrx ([aString isKindOfClass: [NSAttributedString class]] ? [aString string] : aString);

                    if (newText.isNotEmpty())
                    {
                        target->setHighlightedRegion ([&]
                        {
                            // To test this, try i64-pressing 'e' to bring up the accent popup,
                            // then select one of the accented options.
                            if (replacementRange.location != NSNotFound)
                                return nsRangeToDrx (replacementRange);

                            // To test this, try entering the characters 'a b <esc>' with the 2-Set
                            // Korean IME. The input client should receive three calls to setMarkedText:
                            // followed by a call to insertText:
                            // The final call to insertText should overwrite the currently-marked
                            // text, and reset the composition string.
                            if (owner->stringBeingComposed.isNotEmpty())
                                return Range<i32>::withStartAndLength (owner->startOfMarkedTextInTextInputTarget,
                                                                       owner->stringBeingComposed.length());

                            return target->getHighlightedRegion();
                        }());

                        target->insertTextAtCaret (newText);
                        target->setTemporaryUnderlining ({});
                    }
                }
                else
                    jassertfalse; // The system should not attempt to insert text when there is no active TextInputTarget

                owner->stringBeingComposed.clear();
            }
        });

        addMethod (@selector (doCommandBySelector:), [] (id self, SEL, SEL sel)
        {
            const auto handled = [&]
            {
                // 'Special' keys, like backspace, return, tab, and escape, are converted to commands by the system.
                // Components still expect to receive these events as key presses, so we send the currently-processed
                // key event (if any).
                if (auto* owner = getOwner (self))
                {
                    owner->viewCannotHandleEvent = [&]
                    {
                        if (auto* e = owner->keyEventBeingHandled.get())
                        {
                            if ([e type] != NSEventTypeKeyDown && [e type] != NSEventTypeKeyUp)
                                return true;

                            return ! ([e type] == NSEventTypeKeyDown ? owner->redirectKeyDown (e)
                                                                     : owner->redirectKeyUp (e));
                        }

                        return true;
                    }();

                    return ! owner->viewCannotHandleEvent;
                }

                return false;
            }();

            if (! handled)
                sendSuperclassMessage<z0> (self, @selector (doCommandBySelector:), sel);
        });

        addMethod (@selector (setMarkedText:selectedRange:replacementRange:), [] (id self,
                                                                                  SEL,
                                                                                  id aString,
                                                                                  const NSRange selectedRange,
                                                                                  const NSRange replacementRange)
        {
            if (auto* owner = getOwner (self))
            {
                if (auto* target = owner->findCurrentTextInputTarget())
                {
                    const auto toInsert = nsStringToDrx ([aString isKindOfClass: [NSAttributedString class]] ? [aString string] : aString);
                    const auto [initialHighlight, marked, finalHighlight] = [&]
                    {
                        if (owner->stringBeingComposed.isNotEmpty())
                        {
                            const auto toReplace = Range<i32>::withStartAndLength (owner->startOfMarkedTextInTextInputTarget,
                                                                                   owner->stringBeingComposed.length());

                            return replacementRange.location != NSNotFound
                                 // There's a composition underway, so replacementRange is relative to the marked text,
                                 // and selectedRange is relative to the inserted string.
                                 ? std::tuple (toReplace,
                                               owner->stringBeingComposed.replaceSection (static_cast<i32> (replacementRange.location),
                                                                                          static_cast<i32> (replacementRange.length),
                                                                                          toInsert),
                                               nsRangeToDrx (selectedRange) + static_cast<i32> (replacementRange.location))
                                 // The replacementRange is invalid, so replace all the marked text.
                                 : std::tuple (toReplace, toInsert, nsRangeToDrx (selectedRange));
                        }

                        if (replacementRange.location != NSNotFound)
                            // There's no string composition in progress, so replacementRange is relative to the start
                            // of the document.
                            return std::tuple (nsRangeToDrx (replacementRange), toInsert, nsRangeToDrx (selectedRange));

                        return std::tuple (target->getHighlightedRegion(), toInsert, nsRangeToDrx (selectedRange));
                    }();

                    owner->stringBeingComposed = marked;
                    owner->startOfMarkedTextInTextInputTarget = initialHighlight.getStart();

                    target->setHighlightedRegion (initialHighlight);
                    target->insertTextAtCaret (marked);
                    target->setTemporaryUnderlining ({ Range<i32>::withStartAndLength (initialHighlight.getStart(), marked.length()) });
                    target->setHighlightedRegion (finalHighlight + owner->startOfMarkedTextInTextInputTarget);
                }
            }
        });

        addMethod (@selector (unmarkText), [] (id self, SEL)
        {
            if (auto* owner = getOwner (self))
            {
                if (owner->stringBeingComposed.isNotEmpty())
                {
                    if (auto* target = owner->findCurrentTextInputTarget())
                    {
                        target->insertTextAtCaret (owner->stringBeingComposed);
                        target->setTemporaryUnderlining ({});
                    }

                    owner->stringBeingComposed.clear();
                }
            }
        });

        addMethod (@selector (hasMarkedText), [] (id self, SEL)
        {
            auto* owner = getOwner (self);
            return owner != nullptr && owner->stringBeingComposed.isNotEmpty();
        });

        addMethod (@selector (attributedSubstringForProposedRange:actualRange:), [] (id self, SEL, NSRange theRange, NSRangePointer actualRange) -> NSAttributedString*
        {
            jassert (theRange.location != NSNotFound);

            if (auto* owner = getOwner (self))
            {
                if (auto* target = owner->findCurrentTextInputTarget())
                {
                    const auto clamped = Range<i32> { 0, target->getTotalNumChars() }.constrainRange (nsRangeToDrx (theRange));

                    if (actualRange != nullptr)
                        *actualRange = juceRangeToNS (clamped);

                    return [[[NSAttributedString alloc] initWithString: juceStringToNS (target->getTextInRange (clamped))] autorelease];
                }
            }

            return nil;
        });

        addMethod (@selector (markedRange), [] (id self, SEL)
        {
            if (auto* owner = getOwner (self))
                if (owner->stringBeingComposed.isNotEmpty())
                    return NSMakeRange (static_cast<NSUInteger> (owner->startOfMarkedTextInTextInputTarget),
                                        static_cast<NSUInteger> (owner->stringBeingComposed.length()));

            return NSMakeRange (NSNotFound, 0);
        });

        addMethod (@selector (selectedRange), [] (id self, SEL)
        {
            if (auto* owner = getOwner (self))
            {
                if (auto* target = owner->findCurrentTextInputTarget())
                {
                    const auto highlight = target->getHighlightedRegion();

                    // The accent-selector popup does not show if the selectedRange location is NSNotFound!
                    return NSMakeRange ((NSUInteger) highlight.getStart(),
                                        (NSUInteger) highlight.getLength());
                }
            }

            return NSMakeRange (NSNotFound, 0);
        });

        addMethod (@selector (firstRectForCharacterRange:actualRange:), [] (id self, SEL, NSRange range, NSRangePointer actualRange)
        {
            if (auto* owner = getOwner (self))
            {
                if (auto* target = owner->findCurrentTextInputTarget())
                {
                    if (auto* comp = dynamic_cast<Component*> (target))
                    {
                        const auto codePointRange = range.location == NSNotFound ? Range<i32>::emptyRange (target->getCaretPosition())
                                                                                 : nsRangeToDrx (range);
                        const auto clamped = Range<i32> { 0, target->getTotalNumChars() }.constrainRange (codePointRange);

                        if (actualRange != nullptr)
                            *actualRange = juceRangeToNS (clamped);

                        const auto rect = codePointRange.isEmpty() ? target->getCaretRectangleForCharIndex (codePointRange.getStart())
                                                                   : target->getTextBounds (codePointRange).getRectangle (0);
                        const auto areaOnDesktop = comp->localAreaToGlobal (rect);

                        return flippedScreenRect (makeCGRect (detail::ScalingHelpers::scaledScreenPosToUnscaled (areaOnDesktop)));
                    }
                }
            }

            return NSZeroRect;
        });

        addMethod (@selector (characterIndexForPoint:), [] (id, SEL, NSPoint) { return NSNotFound; });

        addMethod (@selector (validAttributesForMarkedText), [] (id, SEL) { return [NSArray array]; });

        addMethod (@selector (acceptsFirstResponder), [] (id self, SEL)
        {
            auto* owner = getOwner (self);
            return owner != nullptr && owner->canBecomeKeyWindow();
        });

        addMethod (@selector (prepareForDragOperation:), [] (id, SEL, id<NSDraggingInfo>) { return YES; });

        addMethod (@selector (performDragOperation:), [] (id self, SEL, id<NSDraggingInfo> sender)
        {
            auto* owner = getOwner (self);
            return owner != nullptr && owner->sendDragCallback (&NSViewComponentPeer::handleDragDrop, sender);
        });

        addMethod (@selector (concludeDragOperation:), [] (id, SEL, id<NSDraggingInfo>) {});

        addMethod (@selector (isAccessibilityElement), [] (id, SEL) { return NO; });

        addMethod (@selector (accessibilityChildren), getAccessibilityChildren);

        addMethod (@selector (accessibilityHitTest:), [] (id self, SEL, NSPoint point)
        {
            return [getAccessibleChild (self) accessibilityHitTest: point];
        });

        addMethod (@selector (accessibilityFocusedUIElement), [] (id self, SEL)
        {
            return [getAccessibleChild (self) accessibilityFocusedUIElement];
        });

        // deprecated methods required for backwards compatibility
        addMethod (@selector (accessibilityIsIgnored), [] (id, SEL) { return YES; });

        addMethod (@selector (accessibilityAttributeValue:), [] (id self, SEL, NSString* attribute) -> id
        {
            if ([attribute isEqualToString: NSAccessibilityChildrenAttribute])
                return getAccessibilityChildren (self, {});

            return sendSuperclassMessage<id> (self, @selector (accessibilityAttributeValue:), attribute);
        });

        addMethod (@selector (isFlipped), [] (id, SEL) { return true; });

        addProtocol (@protocol (NSTextInputClient));

        registerClass();
    }

private:
    template <typename Func, typename... Args>
    static z0 callOnOwner (id self, Func&& func, Args&&... args)
    {
        if (auto* owner = getOwner (self))
            (owner->*func) (std::forward<Args> (args)...);
    }

    static z0 mouseDragged   (id self, SEL, NSEvent* ev)               { callOnOwner (self, &NSViewComponentPeer::redirectMouseDrag, ev); }
    static z0 asyncMouseDown (id self, SEL, NSEvent* ev)               { callOnOwner (self, &NSViewComponentPeer::redirectMouseDown, ev); }
    static z0 asyncMouseUp   (id self, SEL, NSEvent* ev)               { callOnOwner (self, &NSViewComponentPeer::redirectMouseUp,   ev); }
    static z0 draggingExited (id self, SEL, id<NSDraggingInfo> sender) { callOnOwner (self, &NSViewComponentPeer::sendDragCallback, &NSViewComponentPeer::handleDragExit, sender); }

    static z0 mouseDown (id self, SEL s, NSEvent* ev)
    {
        if (DRXApplicationBase::isStandaloneApp())
        {
            asyncMouseDown (self, s, ev);
        }
        else
        {
            // In some host situations, the host will stop modal loops from working
            // correctly if they're called from a mouse event, so we'll trigger
            // the event asynchronously..
            [self performSelectorOnMainThread: NSViewComponentPeer::asyncMouseDownSelector
                                   withObject: ev
                                waitUntilDone: NO];
        }
    }

    static z0 mouseUp (id self, SEL s, NSEvent* ev)
    {
        if (DRXApplicationBase::isStandaloneApp())
        {
            asyncMouseUp (self, s, ev);
        }
        else
        {
            // In some host situations, the host will stop modal loops from working
            // correctly if they're called from a mouse event, so we'll trigger
            // the event asynchronously..
            [self performSelectorOnMainThread: NSViewComponentPeer::asyncMouseUpSelector
                                   withObject: ev
                                waitUntilDone: NO];
        }
    }

    static NSDragOperation draggingUpdated (id self, SEL, id<NSDraggingInfo> sender)
    {
        if (auto* owner = getOwner (self))
            if (owner->sendDragCallback (&NSViewComponentPeer::handleDragMove, sender))
                return NSDragOperationGeneric;

        return NSDragOperationNone;
    }

    static NSArray* getAccessibilityChildren (id self, SEL)
    {
        return NSAccessibilityUnignoredChildrenForOnlyChild (getAccessibleChild (self));
    }
};

//==============================================================================
struct DrxNSWindowClass final : public NSViewComponentPeerWrapper<ObjCClass<NSWindow>>
{
    DrxNSWindowClass()  : NSViewComponentPeerWrapper ("DRXWindow_")
    {
        addMethod (@selector (canBecomeKeyWindow), [] (id self, SEL)
        {
            auto* owner = getOwner (self);

            return owner != nullptr
                   && owner->canBecomeKeyWindow()
                   && ! owner->isBlockedByModalComponent();
        });

        addMethod (@selector (canBecomeMainWindow), [] (id self, SEL)
        {
            auto* owner = getOwner (self);

            return owner != nullptr
                   && owner->canBecomeMainWindow()
                   && ! owner->isBlockedByModalComponent();
        });

        addMethod (@selector (becomeKeyWindow), [] (id self, SEL)
        {
            sendSuperclassMessage<z0> (self, @selector (becomeKeyWindow));

            if (auto* owner = getOwner (self))
            {
                jassert (! owner->inBecomeKeyWindow);

                const ScopedValueSetter scope { owner->inBecomeKeyWindow, true };

                if (owner->canBecomeKeyWindow())
                {
                    owner->becomeKeyWindow();
                    return;
                }

                // this fixes a bug causing hidden windows to sometimes become visible when the app regains focus
                if (! owner->getComponent().isVisible())
                    [(NSWindow*) self orderOut: nil];
            }
        });

        addMethod (@selector (resignKeyWindow), [] (id self, SEL)
        {
            sendSuperclassMessage<z0> (self, @selector (resignKeyWindow));

            if (auto* owner = getOwner (self))
                owner->resignKeyWindow();
        });

        addMethod (@selector (windowShouldClose:), [] (id self, SEL, id /*window*/)
        {
            auto* owner = getOwner (self);
            return owner == nullptr || owner->windowShouldClose();
        });

        addMethod (@selector (constrainFrameRect:toScreen:), [] (id self, SEL, NSRect frameRect, NSScreen* screen)
        {
            if (auto* owner = getOwner (self))
            {
                frameRect = sendSuperclassMessage<NSRect, NSRect, NSScreen*> (self, @selector (constrainFrameRect:toScreen:),
                                                                              frameRect, screen);

                frameRect = owner->constrainRect (frameRect);
            }

            return frameRect;
        });

        addMethod (@selector (windowWillResize:toSize:), [] (id self, SEL, NSWindow*, NSSize proposedFrameSize)
        {
            auto* owner = getOwner (self);

            if (owner == nullptr)
                return proposedFrameSize;

            NSRect frameRect = flippedScreenRect ([(NSWindow*) self frame]);
            frameRect.size = proposedFrameSize;

            frameRect = owner->constrainRect (flippedScreenRect (frameRect));

            owner->dismissModals();

            return frameRect.size;
        });

        addMethod (@selector (windowDidExitFullScreen:), [] (id self, SEL, NSNotification*)
        {
            if (auto* owner = getOwner (self))
                owner->resetWindowPresentation();
        });

        addMethod (@selector (windowWillEnterFullScreen:), [] (id self, SEL, NSNotification*)
        {
            if (auto* owner = getOwner (self))
                if (owner->hasNativeTitleBar() && (owner->getStyleFlags() & ComponentPeer::windowIsResizable) == 0)
                    [owner->window setStyleMask: NSWindowStyleMaskBorderless];
        });

        addMethod (@selector (windowWillExitFullScreen:), [] (id self, SEL, NSNotification*)
        {
            // The exit-fullscreen animation looks bad on Monterey if the window isn't resizable...
            if (auto* owner = getOwner (self))
                if (auto* window = owner->window)
                    [window setStyleMask: [window styleMask] | NSWindowStyleMaskResizable];
        });

        addMethod (@selector (windowWillStartLiveResize:), [] (id self, SEL, NSNotification*)
        {
            if (auto* owner = getOwner (self))
                owner->liveResizingStart();
        });

        addMethod (@selector (windowDidEndLiveResize:), [] (id self, SEL, NSNotification*)
        {
            if (auto* owner = getOwner (self))
                owner->liveResizingEnd();
        });

        addMethod (@selector (window:shouldPopUpDocumentPathMenu:), [] (id self, SEL, id /*window*/, NSMenu*)
        {
            if (auto* owner = getOwner (self))
                return owner->windowRepresentsFile;

            return false;
        });

        addMethod (@selector (isFlipped), [] (id, SEL) { return true; });

        addMethod (@selector (windowWillUseStandardFrame:defaultFrame:), [] (id self, SEL, NSWindow* window, NSRect r)
        {
            if (auto* owner = getOwner (self))
            {
                if (auto* constrainer = owner->getConstrainer())
                {
                    if (auto* screen = [window screen])
                    {
                        const auto safeScreenBounds = convertToRectFloat (flippedScreenRect (owner->hasNativeTitleBar() ? r : [screen visibleFrame]));
                        const auto originalBounds = owner->getFrameSize().addedTo (owner->getComponent().getScreenBounds()).toFloat();
                        const auto expanded = originalBounds.withWidth  ((f32) constrainer->getMaximumWidth())
                                                            .withHeight ((f32) constrainer->getMaximumHeight());
                        const auto constrained = expanded.constrainedWithin (safeScreenBounds);

                        return flippedScreenRect (makeCGRect ([&]
                                                              {
                                                                  if (constrained == owner->getBounds().toFloat())
                                                                      return owner->lastSizeBeforeZoom.toFloat();

                                                                  owner->lastSizeBeforeZoom = owner->getBounds().toFloat();
                                                                  return constrained;
                                                              }()));
                    }
                }
            }

            return r;
        });

        addMethod (@selector (windowShouldZoom:toFrame:), [] (id self, SEL, NSWindow*, NSRect)
        {
            if (auto* owner = getOwner (self))
                if (owner->hasNativeTitleBar() && (owner->getStyleFlags() & ComponentPeer::windowIsResizable) == 0)
                    return NO;

            return YES;
        });

        addMethod (@selector (accessibilityTitle), [] (id self, SEL) { return [self title]; });

        addMethod (@selector (accessibilityLabel), [] (id self, SEL) { return [getAccessibleChild (self) accessibilityLabel]; });

        addMethod (@selector (accessibilityRole), [] (id, SEL) { return NSAccessibilityWindowRole; });

        addMethod (@selector (accessibilitySubrole), [] (id self, SEL) -> NSAccessibilitySubrole
        {
            return [getAccessibleChild (self) accessibilitySubrole];
        });

        addMethod (@selector (window:shouldDragDocumentWithEvent:from:withPasteboard:), [] (id self, SEL, id /*window*/, NSEvent*, NSPoint, NSPasteboard*)
        {
            if (auto* owner = getOwner (self))
                return owner->windowRepresentsFile;

            return false;
        });

        addMethod (@selector (toggleFullScreen:), [] (id self, SEL name, id sender)
        {
            if (auto* owner = getOwner (self))
            {
                const auto isFullScreen = owner->isFullScreen();

                if (! isFullScreen)
                    owner->lastSizeBeforeZoom = owner->getBounds().toFloat();

                sendSuperclassMessage<z0> (self, name, sender);

                if (isFullScreen)
                {
                    [NSApp setPresentationOptions: NSApplicationPresentationDefault];
                    owner->setBounds (owner->lastSizeBeforeZoom.toNearestInt(), false);
                }
            }
        });

        addMethod (@selector (accessibilityTopLevelUIElement),      getAccessibilityWindow);
        addMethod (@selector (accessibilityWindow),                 getAccessibilityWindow);

        addProtocol (@protocol (NSWindowDelegate));

        registerClass();
    }

private:
    //==============================================================================
    static id getAccessibilityWindow (id self, SEL) { return self; }
};

NSView* NSViewComponentPeer::createViewInstance()
{
    static DrxNSViewClass cls;
    return cls.createInstance();
}

NSWindow* NSViewComponentPeer::createWindowInstance()
{
    static DrxNSWindowClass cls;
    return cls.createInstance();
}


//==============================================================================
b8 KeyPress::isKeyCurrentlyDown (i32 keyCode)
{
    const auto isDown = [] (i32 k)
    {
        return NSViewComponentPeer::keysCurrentlyDown.find (k) != NSViewComponentPeer::keysCurrentlyDown.cend();
    };

    if (isDown (keyCode))
        return true;

    if (keyCode >= 'A' && keyCode <= 'Z'
         && isDown ((i32) CharacterFunctions::toLowerCase ((t32) keyCode)))
        return true;

    if (keyCode >= 'a' && keyCode <= 'z'
         && isDown ((i32) CharacterFunctions::toUpperCase ((t32) keyCode)))
        return true;

    return false;
}

//==============================================================================
b8 detail::MouseInputSourceList::addSource()
{
    if (sources.size() == 0)
    {
        addSource (0, MouseInputSource::InputSourceType::mouse);
        return true;
    }

    return false;
}

b8 detail::MouseInputSourceList::canUseTouch() const
{
    return false;
}

//==============================================================================
z0 Desktop::setKioskComponent (Component* kioskComp, b8 shouldBeEnabled, b8 allowMenusAndBars)
{
    auto* peer = dynamic_cast<NSViewComponentPeer*> (kioskComp->getPeer());
    jassert (peer != nullptr); // (this should have been checked by the caller)

    if (peer->hasNativeTitleBar())
    {
        if (shouldBeEnabled && ! allowMenusAndBars)
            [NSApp setPresentationOptions: NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar];
        else if (! shouldBeEnabled)
            [NSApp setPresentationOptions: NSApplicationPresentationDefault];

        peer->setFullScreen (shouldBeEnabled);
    }
    else
    {
        if (shouldBeEnabled)
        {
            [NSApp setPresentationOptions: (allowMenusAndBars ? (NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar)
                                                              : (NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar))];

            kioskComp->setBounds (getDisplays().getDisplayForRect (kioskComp->getScreenBounds())->totalArea);
            peer->becomeKeyWindow();
        }
        else
        {
            peer->resetWindowPresentation();
        }
    }
}

z0 Desktop::allowedOrientationsChanged() {}

//==============================================================================
ComponentPeer* Component::createNewPeer (i32 styleFlags, uk windowToAttachTo)
{
    return new NSViewComponentPeer (*this, styleFlags, (NSView*) windowToAttachTo);
}

//==============================================================================
i32k KeyPress::spaceKey        = ' ';
i32k KeyPress::returnKey       = 0x0d;
i32k KeyPress::escapeKey       = 0x1b;
i32k KeyPress::backspaceKey    = 0x7f;
i32k KeyPress::leftKey         = NSLeftArrowFunctionKey;
i32k KeyPress::rightKey        = NSRightArrowFunctionKey;
i32k KeyPress::upKey           = NSUpArrowFunctionKey;
i32k KeyPress::downKey         = NSDownArrowFunctionKey;
i32k KeyPress::pageUpKey       = NSPageUpFunctionKey;
i32k KeyPress::pageDownKey     = NSPageDownFunctionKey;
i32k KeyPress::endKey          = NSEndFunctionKey;
i32k KeyPress::homeKey         = NSHomeFunctionKey;
i32k KeyPress::deleteKey       = NSDeleteFunctionKey;
i32k KeyPress::insertKey       = -1;
i32k KeyPress::tabKey          = 9;
i32k KeyPress::F1Key           = NSF1FunctionKey;
i32k KeyPress::F2Key           = NSF2FunctionKey;
i32k KeyPress::F3Key           = NSF3FunctionKey;
i32k KeyPress::F4Key           = NSF4FunctionKey;
i32k KeyPress::F5Key           = NSF5FunctionKey;
i32k KeyPress::F6Key           = NSF6FunctionKey;
i32k KeyPress::F7Key           = NSF7FunctionKey;
i32k KeyPress::F8Key           = NSF8FunctionKey;
i32k KeyPress::F9Key           = NSF9FunctionKey;
i32k KeyPress::F10Key          = NSF10FunctionKey;
i32k KeyPress::F11Key          = NSF11FunctionKey;
i32k KeyPress::F12Key          = NSF12FunctionKey;
i32k KeyPress::F13Key          = NSF13FunctionKey;
i32k KeyPress::F14Key          = NSF14FunctionKey;
i32k KeyPress::F15Key          = NSF15FunctionKey;
i32k KeyPress::F16Key          = NSF16FunctionKey;
i32k KeyPress::F17Key          = NSF17FunctionKey;
i32k KeyPress::F18Key          = NSF18FunctionKey;
i32k KeyPress::F19Key          = NSF19FunctionKey;
i32k KeyPress::F20Key          = NSF20FunctionKey;
i32k KeyPress::F21Key          = NSF21FunctionKey;
i32k KeyPress::F22Key          = NSF22FunctionKey;
i32k KeyPress::F23Key          = NSF23FunctionKey;
i32k KeyPress::F24Key          = NSF24FunctionKey;
i32k KeyPress::F25Key          = NSF25FunctionKey;
i32k KeyPress::F26Key          = NSF26FunctionKey;
i32k KeyPress::F27Key          = NSF27FunctionKey;
i32k KeyPress::F28Key          = NSF28FunctionKey;
i32k KeyPress::F29Key          = NSF29FunctionKey;
i32k KeyPress::F30Key          = NSF30FunctionKey;
i32k KeyPress::F31Key          = NSF31FunctionKey;
i32k KeyPress::F32Key          = NSF32FunctionKey;
i32k KeyPress::F33Key          = NSF33FunctionKey;
i32k KeyPress::F34Key          = NSF34FunctionKey;
i32k KeyPress::F35Key          = NSF35FunctionKey;

i32k KeyPress::numberPad0              = extendedKeyModifier + 0x20;
i32k KeyPress::numberPad1              = extendedKeyModifier + 0x21;
i32k KeyPress::numberPad2              = extendedKeyModifier + 0x22;
i32k KeyPress::numberPad3              = extendedKeyModifier + 0x23;
i32k KeyPress::numberPad4              = extendedKeyModifier + 0x24;
i32k KeyPress::numberPad5              = extendedKeyModifier + 0x25;
i32k KeyPress::numberPad6              = extendedKeyModifier + 0x26;
i32k KeyPress::numberPad7              = extendedKeyModifier + 0x27;
i32k KeyPress::numberPad8              = extendedKeyModifier + 0x28;
i32k KeyPress::numberPad9              = extendedKeyModifier + 0x29;
i32k KeyPress::numberPadAdd            = extendedKeyModifier + 0x2a;
i32k KeyPress::numberPadSubtract       = extendedKeyModifier + 0x2b;
i32k KeyPress::numberPadMultiply       = extendedKeyModifier + 0x2c;
i32k KeyPress::numberPadDivide         = extendedKeyModifier + 0x2d;
i32k KeyPress::numberPadSeparator      = extendedKeyModifier + 0x2e;
i32k KeyPress::numberPadDecimalPoint   = extendedKeyModifier + 0x2f;
i32k KeyPress::numberPadEquals         = extendedKeyModifier + 0x30;
i32k KeyPress::numberPadDelete         = extendedKeyModifier + 0x31;
i32k KeyPress::playKey                 = extendedKeyModifier + 0x00;
i32k KeyPress::stopKey                 = extendedKeyModifier + 0x01;
i32k KeyPress::fastForwardKey          = extendedKeyModifier + 0x02;
i32k KeyPress::rewindKey               = extendedKeyModifier + 0x03;

} // namespace drx
