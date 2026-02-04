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

#if TARGET_OS_SIMULATOR && DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
 #warning DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS uses parts of the Metal API that are currently unsupported in the simulator - falling back to DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS=0
 #undef DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
#endif

namespace drx
{

//==============================================================================
static NSArray* getContainerAccessibilityElements (AccessibilityHandler& handler)
{
    const auto children = handler.getChildren();

    NSMutableArray* accessibleChildren = [NSMutableArray arrayWithCapacity: (NSUInteger) children.size()];

    for (auto* childHandler : children)
    {
        id accessibleElement = [&childHandler]
        {
            id nativeChild = static_cast<id> (childHandler->getNativeImplementation());

            if (   ! childHandler->getChildren().empty()
                || AccessibilityHandler::getNativeChildForComponent (childHandler->getComponent()) != nullptr)
            {
                return [nativeChild accessibilityContainer];
            }

            return nativeChild;
        }();

        if (accessibleElement != nil)
            [accessibleChildren addObject: accessibleElement];
    }

    id nativeHandler = static_cast<id> (handler.getNativeImplementation());

    if (auto* view = static_cast<UIView*> (AccessibilityHandler::getNativeChildForComponent (handler.getComponent())))
    {
        [static_cast<UIAccessibilityElement*> (view) setAccessibilityContainer: [nativeHandler accessibilityContainer]];

        [accessibleChildren addObject: view];
    }

    [accessibleChildren addObject: nativeHandler];

    return accessibleChildren;
}

class UIViewComponentPeer;

namespace iOSGlobals
{
class KeysCurrentlyDown
{
public:
    b8 isDown (i32 x) const { return down.find (x) != down.cend(); }

    z0 setDown (i32 x, b8 b)
    {
        if (b)
            down.insert (x);
        else
            down.erase (x);
    }

private:
    std::set<i32> down;
};
static KeysCurrentlyDown keysCurrentlyDown;
static UIViewComponentPeer* currentlyFocusedPeer = nullptr;
} // namespace iOSGlobals

static UIInterfaceOrientation getWindowOrientation()
{
    UIApplication* sharedApplication = [UIApplication sharedApplication];

    if (@available (iOS 13.0, *))
    {
        for (UIScene* scene in [sharedApplication connectedScenes])
            if ([scene isKindOfClass: [UIWindowScene class]])
                return [(UIWindowScene*) scene interfaceOrientation];
    }

    DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
    return [sharedApplication statusBarOrientation];
    DRX_END_IGNORE_DEPRECATION_WARNINGS
}

struct Orientations
{
    static Desktop::DisplayOrientation convertToDrx (UIInterfaceOrientation orientation)
    {
        switch (orientation)
        {
            case UIInterfaceOrientationPortrait:            return Desktop::upright;
            case UIInterfaceOrientationPortraitUpsideDown:  return Desktop::upsideDown;
            case UIInterfaceOrientationLandscapeLeft:       return Desktop::rotatedClockwise;
            case UIInterfaceOrientationLandscapeRight:      return Desktop::rotatedAntiClockwise;
            case UIInterfaceOrientationUnknown:
            default:                                        jassertfalse; // unknown orientation!
        }

        return Desktop::upright;
    }

    static UIInterfaceOrientation convertFromDrx (Desktop::DisplayOrientation orientation)
    {
        switch (orientation)
        {
            case Desktop::upright:                          return UIInterfaceOrientationPortrait;
            case Desktop::upsideDown:                       return UIInterfaceOrientationPortraitUpsideDown;
            case Desktop::rotatedClockwise:                 return UIInterfaceOrientationLandscapeLeft;
            case Desktop::rotatedAntiClockwise:             return UIInterfaceOrientationLandscapeRight;
            case Desktop::allOrientations:
            default:                                        jassertfalse; // unknown orientation!
        }

        return UIInterfaceOrientationPortrait;
    }

    static UIInterfaceOrientationMask getSupportedOrientations()
    {
        NSUInteger allowed = 0;
        auto& d = Desktop::getInstance();

        if (d.isOrientationEnabled (Desktop::upright))              allowed |= UIInterfaceOrientationMaskPortrait;
        if (d.isOrientationEnabled (Desktop::upsideDown))           allowed |= UIInterfaceOrientationMaskPortraitUpsideDown;
        if (d.isOrientationEnabled (Desktop::rotatedClockwise))     allowed |= UIInterfaceOrientationMaskLandscapeLeft;
        if (d.isOrientationEnabled (Desktop::rotatedAntiClockwise)) allowed |= UIInterfaceOrientationMaskLandscapeRight;

        return allowed;
    }
};

enum class MouseEventFlags
{
    none,
    down,
    up,
    upAndCancel,
};

//==============================================================================
} // namespace drx

using namespace drx;

@interface DrxUITextPosition : UITextPosition
{
@public
    i32 index;
}
@end

@implementation DrxUITextPosition

+ (instancetype) withIndex: (i32) indexIn
{
    auto* result = [[DrxUITextPosition alloc] init];
    result->index = indexIn;
    return [result autorelease];
}

@end

//==============================================================================
@interface DrxUITextRange : UITextRange
{
@public
    i32 from, to;
}
@end

@implementation DrxUITextRange

+ (instancetype) withRange: (drx::Range<i32>) range
{
    return [DrxUITextRange from: range.getStart() to: range.getEnd()];
}

+ (instancetype) from: (i32) from to: (i32) to
{
    auto* result = [[DrxUITextRange alloc] init];
    result->from = from;
    result->to = to;
    return [result autorelease];
}

- (UITextPosition*) start
{
    return [DrxUITextPosition withIndex: from];
}

- (UITextPosition*) end
{
    return [DrxUITextPosition withIndex: to];
}

- (Range<i32>) range
{
    return Range<i32>::between (from, to);
}

- (BOOL) isEmpty
{
    return from == to;
}

@end

//==============================================================================
// UITextInputStringTokenizer doesn't handle 'line' granularities correctly by default, hence
// this subclass.
@interface DrxTextInputTokenizer : UITextInputStringTokenizer
{
    UIViewComponentPeer* peer;
}

- (instancetype) initWithPeer: (UIViewComponentPeer*) peer;

@end

//==============================================================================
@interface DrxUITextSelectionRect : UITextSelectionRect
{
    CGRect _rect;
}

@end

@implementation DrxUITextSelectionRect

+ (instancetype) withRect: (CGRect) rect
{
    auto* result = [[DrxUITextSelectionRect alloc] init];
    result->_rect = rect;
    return [result autorelease];
}

- (CGRect) rect { return _rect; }
- (NSWritingDirection) writingDirection { return NSWritingDirectionNatural; }
- (BOOL) containsStart { return NO; }
- (BOOL) containsEnd   { return NO; }
- (BOOL) isVertical    { return NO; }

@end

//==============================================================================
struct CADisplayLinkDeleter
{
    z0 operator() (CADisplayLink* displayLink) const noexcept
    {
        [displayLink invalidate];
        [displayLink release];
    }
};

@interface DrxTextView : UIView <UITextInput>
{
@public
    UIViewComponentPeer* owner;
    id<UITextInputDelegate> delegate;
}

- (instancetype) initWithOwner: (UIViewComponentPeer*) owner;

@end

@interface DrxUIView : UIView<CALayerDelegate>
{
@public
    UIViewComponentPeer* owner;
    std::unique_ptr<CADisplayLink, CADisplayLinkDeleter> displayLink;
}

@end

//==============================================================================
@interface DrxUIViewController : UIViewController
{
}

- (DrxUIViewController*) init;

- (NSUInteger) supportedInterfaceOrientations;
- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation;
- (z0) willRotateToInterfaceOrientation: (UIInterfaceOrientation) toInterfaceOrientation duration: (NSTimeInterval) duration;
- (z0) didRotateFromInterfaceOrientation: (UIInterfaceOrientation) fromInterfaceOrientation;
- (z0) viewWillTransitionToSize: (CGSize) size withTransitionCoordinator: (id<UIViewControllerTransitionCoordinator>) coordinator;
- (BOOL) prefersStatusBarHidden;
- (UIStatusBarStyle) preferredStatusBarStyle;

- (z0) viewDidLoad;
- (z0) viewWillAppear: (BOOL) animated;
- (z0) viewDidAppear: (BOOL) animated;
- (z0) viewWillLayoutSubviews;
- (z0) viewDidLayoutSubviews;
@end

//==============================================================================
@interface DrxUIWindow : UIWindow
{
@private
    UIViewComponentPeer* owner;
}

- (z0) setOwner: (UIViewComponentPeer*) owner;
- (z0) becomeKeyWindow;
@end

//==============================================================================
//==============================================================================
namespace drx
{

struct UIViewPeerControllerReceiver
{
    virtual ~UIViewPeerControllerReceiver() = default;
    virtual z0 setViewController (UIViewController*) = 0;
};

//==============================================================================
class UIViewComponentPeer final : public ComponentPeer,
                                  public UIViewPeerControllerReceiver
{
public:
    UIViewComponentPeer (Component&, i32 windowStyleFlags, UIView* viewToAttachTo);
    ~UIViewComponentPeer() override;

    //==============================================================================
    uk getNativeHandle() const override                  { return view; }
    z0 setVisible (b8 shouldBeVisible) override;
    z0 setTitle (const Txt& title) override;
    z0 setBounds (const Rectangle<i32>&, b8 isNowFullScreen) override;

    z0 setViewController (UIViewController* newController) override
    {
        jassert (controller == nullptr);
        controller = [newController retain];
    }

    Rectangle<i32> getBounds() const override                 { return getBounds (! isSharedWindow); }
    Rectangle<i32> getBounds (b8 global) const;
    Point<f32> localToGlobal (Point<f32> relativePosition) override;
    Point<f32> globalToLocal (Point<f32> screenPosition) override;
    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;
    z0 setAlpha (f32 newAlpha) override;
    z0 setMinimised (b8) override                         {}
    b8 isMinimised() const override                         { return false; }
    b8 isShowing() const override                           { return true; }
    z0 setFullScreen (b8 shouldBeFullScreen) override;
    b8 isFullScreen() const override                        { return fullScreen; }
    b8 contains (Point<i32> localPos, b8 trueIfInAChildWindow) const override;
    OptionalBorderSize getFrameSizeIfPresent() const override { return {}; }
    BorderSize<i32> getFrameSize() const override             { return BorderSize<i32>(); }
    b8 setAlwaysOnTop (b8 alwaysOnTop) override;
    z0 toFront (b8 makeActiveWindow) override;
    z0 toBehind (ComponentPeer* other) override;
    z0 setIcon (const Image& newIcon) override;
    StringArray getAvailableRenderingEngines() override       { return StringArray ("CoreGraphics Renderer"); }

    z0 displayLinkCallback (f64 timestampSec);

    z0 drawRect (CGRect);
    z0 drawRectWithContext (CGContextRef, CGRect);
    b8 canBecomeKeyWindow();

    //==============================================================================
    z0 viewFocusGain();
    z0 viewFocusLoss();
    b8 isFocused() const override;
    z0 grabFocus() override;
    z0 textInputRequired (Point<i32>, TextInputTarget&) override;
    z0 dismissPendingTextInput() override;
    z0 closeInputMethodContext() override;

    z0 updateScreenBounds();

    z0 handleTouches (UIEvent*, MouseEventFlags);

    API_AVAILABLE (ios (13.0)) z0 onHover (UIHoverGestureRecognizer*);
    z0 onScroll (UIPanGestureRecognizer*);

    Range<i32> getMarkedTextRange() const
    {
        return Range<i32>::withStartAndLength (startOfMarkedTextInTextInputTarget,
                                               stringBeingComposed.length());
    }

    enum class UnderlineRegion
    {
        none,
        underCompositionRange
    };

    z0 replaceMarkedRangeWithText (TextInputTarget* target,
                                     const Txt& text,
                                     UnderlineRegion underline)
    {
        if (stringBeingComposed.isNotEmpty())
            target->setHighlightedRegion (getMarkedTextRange());

        target->insertTextAtCaret (text);

        const auto underlineRanges = underline == UnderlineRegion::underCompositionRange
                                   ? Array { Range<i32>::withStartAndLength (startOfMarkedTextInTextInputTarget,
                                                                             text.length()) }
                                   : Array<Range<i32>>{};
        target->setTemporaryUnderlining (underlineRanges);

        stringBeingComposed = text;
    }

    //==============================================================================
    z0 repaint (const Rectangle<i32>& area) override;
    z0 performAnyPendingRepaintsNow() override;

    //==============================================================================
    UIWindow* window = nil;
    DrxUIView* view = nil;
    UIViewController* controller = nil;
    const b8 isSharedWindow, isAppex;
    Txt stringBeingComposed;
    i32 startOfMarkedTextInTextInputTarget = 0;
    b8 fullScreen = false, insideDrawRect = false;
    NSUniquePtr<DrxTextView> hiddenTextInput { [[DrxTextView alloc] initWithOwner: this] };
    NSUniquePtr<DrxTextInputTokenizer> tokenizer { [[DrxTextInputTokenizer alloc] initWithPeer: this] };

    static z64 getMouseTime (NSTimeInterval timestamp) noexcept
    {
        return (Time::currentTimeMillis() - Time::getMillisecondCounter())
             + (z64) (timestamp * 1000.0);
    }

    static z64 getMouseTime (UIEvent* e) noexcept
    {
        return getMouseTime ([e timestamp]);
    }

    static NSString* getDarkModeNotificationName()
    {
        return @"ViewDarkModeChanged";
    }

    static MultiTouchMapper<UITouch*> currentTouches;

    static UIKeyboardType getUIKeyboardType (TextInputTarget::VirtualKeyboardType type) noexcept
    {
        switch (type)
        {
            case TextInputTarget::textKeyboard:          return UIKeyboardTypeDefault;
            case TextInputTarget::numericKeyboard:       return UIKeyboardTypeNumbersAndPunctuation;
            case TextInputTarget::decimalKeyboard:       return UIKeyboardTypeNumbersAndPunctuation;
            case TextInputTarget::urlKeyboard:           return UIKeyboardTypeURL;
            case TextInputTarget::emailAddressKeyboard:  return UIKeyboardTypeEmailAddress;
            case TextInputTarget::phoneNumberKeyboard:   return UIKeyboardTypePhonePad;
            case TextInputTarget::passwordKeyboard:      return UIKeyboardTypeASCIICapable;
        }

        jassertfalse;
        return UIKeyboardTypeDefault;
    }

   #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    std::unique_ptr<CoreGraphicsMetalLayerRenderer> metalRenderer;
   #endif

    RectangleList<f32> deferredRepaints;

private:
    z0 appStyleChanged() override
    {
        [controller setNeedsStatusBarAppearanceUpdate];
    }

    //==============================================================================
    class AsyncRepaintMessage final : public CallbackMessage
    {
    public:
        UIViewComponentPeer* const peer;
        const Rectangle<i32> rect;

        AsyncRepaintMessage (UIViewComponentPeer* const p, const Rectangle<i32>& r)
            : peer (p), rect (r)
        {
        }

        z0 messageCallback() override
        {
            if (ComponentPeer::isValidPeer (peer))
                peer->repaint (rect);
        }
    };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIViewComponentPeer)
};

static UIViewComponentPeer* getViewPeer (DrxUIViewController* c)
{
    if (DrxUIView* juceView = (DrxUIView*) [c view])
        return juceView->owner;

    jassertfalse;
    return nullptr;
}

static z0 sendScreenBoundsUpdate (DrxUIViewController* c)
{
    if (auto* peer = getViewPeer (c))
        peer->updateScreenBounds();
}

static b8 isKioskModeView (DrxUIViewController* c)
{
    if (auto* peer = getViewPeer (c))
        return Desktop::getInstance().getKioskModeComponent() == &(peer->getComponent());

    return false;
}

MultiTouchMapper<UITouch*> UIViewComponentPeer::currentTouches;

} // namespace drx

//==============================================================================
//==============================================================================
@implementation DrxUIViewController

- (DrxUIViewController*) init
{
    self = [super init];

    return self;
}

- (NSUInteger) supportedInterfaceOrientations
{
    return Orientations::getSupportedOrientations();
}

- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation
{
    return Desktop::getInstance().isOrientationEnabled (Orientations::convertToDrx (interfaceOrientation));
}

- (z0) willRotateToInterfaceOrientation: (UIInterfaceOrientation) toInterfaceOrientation
                                 duration: (NSTimeInterval) duration
{
    ignoreUnused (toInterfaceOrientation, duration);

    [UIView setAnimationsEnabled: NO]; // disable this because it goes the wrong way and looks like crap.
}

- (z0) didRotateFromInterfaceOrientation: (UIInterfaceOrientation) fromInterfaceOrientation
{
    ignoreUnused (fromInterfaceOrientation);
    sendScreenBoundsUpdate (self);
    [UIView setAnimationsEnabled: YES];
}

- (z0) viewWillTransitionToSize: (CGSize) size withTransitionCoordinator: (id<UIViewControllerTransitionCoordinator>) coordinator
{
    [super viewWillTransitionToSize: size withTransitionCoordinator: coordinator];
    [coordinator animateAlongsideTransition: nil completion: ^z0 (id<UIViewControllerTransitionCoordinatorContext>)
    {
        sendScreenBoundsUpdate (self);
    }];
}

- (BOOL) prefersStatusBarHidden
{
    if (isKioskModeView (self))
        return true;

    return [[[NSBundle mainBundle] objectForInfoDictionaryKey: @"UIStatusBarHidden"] boolValue];
}

 - (BOOL) prefersHomeIndicatorAutoHidden
 {
     return isKioskModeView (self);
 }

- (UIStatusBarStyle) preferredStatusBarStyle
{
    if (@available (iOS 13.0, *))
    {
        if (auto* peer = getViewPeer (self))
        {
            switch (peer->getAppStyle())
            {
                case ComponentPeer::Style::automatic:
                    return UIStatusBarStyleDefault;
                case ComponentPeer::Style::light:
                    return UIStatusBarStyleDarkContent;
                case ComponentPeer::Style::dark:
                    return UIStatusBarStyleLightContent;
            }
        }
    }

    return UIStatusBarStyleDefault;
}

- (z0) viewDidLoad
{
    sendScreenBoundsUpdate (self);
    [super viewDidLoad];
}

- (z0) viewWillAppear: (BOOL) animated
{
    sendScreenBoundsUpdate (self);
    [super viewWillAppear:animated];
}

- (z0) viewDidAppear: (BOOL) animated
{
    sendScreenBoundsUpdate (self);
    [super viewDidAppear:animated];
}

- (z0) viewWillLayoutSubviews
{
    sendScreenBoundsUpdate (self);
}

- (z0) viewDidLayoutSubviews
{
    sendScreenBoundsUpdate (self);
}

@end

@implementation DrxUIView

- (DrxUIView*) initWithOwner: (UIViewComponentPeer*) peer
                    withFrame: (CGRect) frame
{
    [super initWithFrame: frame];
    owner = peer;

   #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (@available (iOS 13.0, *))
    {
        auto* layer = (CAMetalLayer*) [self layer];
        layer.device = MTLCreateSystemDefaultDevice();
        layer.framebufferOnly = NO;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

        if (owner != nullptr)
            layer.opaque = owner->getComponent().isOpaque();

        layer.presentsWithTransaction = YES;
        layer.needsDisplayOnBoundsChange = true;
        layer.presentsWithTransaction = true;
        layer.delegate = self;

        layer.allowsNextDrawableTimeout = NO;
    }
   #endif

    displayLink.reset ([CADisplayLink displayLinkWithTarget: self
                                                   selector: @selector (displayLinkCallback:)]);
    [displayLink.get() addToRunLoop: [NSRunLoop mainRunLoop]
                            forMode: NSDefaultRunLoopMode];

    [self addSubview: owner->hiddenTextInput.get()];

    if (@available (iOS 13.4, *))
    {
        auto hoverRecognizer = [[[UIHoverGestureRecognizer alloc] initWithTarget: self action: @selector (onHover:)] autorelease];
        [hoverRecognizer setCancelsTouchesInView: NO];
        [hoverRecognizer setRequiresExclusiveTouchType: YES];
        [self addGestureRecognizer: hoverRecognizer];

        auto panRecognizer = [[[UIPanGestureRecognizer alloc] initWithTarget: self action: @selector (onScroll:)] autorelease];
        [panRecognizer setCancelsTouchesInView: NO];
        [panRecognizer setRequiresExclusiveTouchType: YES];
        [panRecognizer setAllowedScrollTypesMask: UIScrollTypeMaskAll];
        [panRecognizer setMaximumNumberOfTouches: 0];
        [self addGestureRecognizer: panRecognizer];
    }

    return self;
}

- (z0) dealloc
{
    [owner->hiddenTextInput.get() removeFromSuperview];
    displayLink = nullptr;

    [super dealloc];
}

//==============================================================================
+ (Class) layerClass
{
   #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (@available (iOS 13, *))
        return [CAMetalLayer class];
   #endif

    return [CALayer class];
}

- (z0) displayLinkCallback: (CADisplayLink*) dl
{
    if (owner != nullptr)
        owner->displayLinkCallback (dl.targetTimestamp);
}

#if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
- (CALayer*) makeBackingLayer
{
    auto* layer = [CAMetalLayer layer];

    layer.device = MTLCreateSystemDefaultDevice();
    layer.framebufferOnly = NO;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

    if (owner != nullptr)
        layer.opaque = owner->getComponent().isOpaque();

    layer.presentsWithTransaction = YES;
    layer.needsDisplayOnBoundsChange = true;
    layer.presentsWithTransaction = true;
    layer.delegate = self;

    layer.allowsNextDrawableTimeout = NO;

    return layer;
}

- (z0) displayLayer: (CALayer*) layer
{
    if (owner != nullptr)
    {
        owner->deferredRepaints = owner->metalRenderer->drawRectangleList (static_cast<CAMetalLayer*> (layer),
                                                                           (f32) [self contentScaleFactor],
                                                                           [self] (auto&&... args) { owner->drawRectWithContext (args...); },
                                                                           std::move (owner->deferredRepaints),
                                                                           false);
    }
}
#endif

//==============================================================================
- (z0) drawRect: (CGRect) r
{
    if (owner != nullptr)
        owner->drawRect (r);
}

//==============================================================================
- (z0) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event
{
    ignoreUnused (touches);

    if (owner != nullptr)
        owner->handleTouches (event, MouseEventFlags::down);
}

- (z0) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
    ignoreUnused (touches);

    if (owner != nullptr)
        owner->handleTouches (event, MouseEventFlags::none);
}

- (z0) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
    ignoreUnused (touches);

    if (owner != nullptr)
        owner->handleTouches (event, MouseEventFlags::up);
}

- (z0) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event
{
    if (owner != nullptr)
        owner->handleTouches (event, MouseEventFlags::upAndCancel);

    [self touchesEnded: touches withEvent: event];
}

- (z0) onHover: (UIHoverGestureRecognizer*) gesture API_AVAILABLE (ios (13))
{
    if (owner != nullptr)
        owner->onHover (gesture);
}

- (z0) onScroll: (UIPanGestureRecognizer*) gesture
{
    if (owner != nullptr)
        owner->onScroll (gesture);
}

static std::optional<i32> getKeyCodeForSpecialCharacterString (StringRef characters)
{
    static const auto map = [&]
    {
        std::map<Txt, i32> result
        {
            { nsStringToDrx (UIKeyInputUpArrow),       KeyPress::upKey },
            { nsStringToDrx (UIKeyInputDownArrow),     KeyPress::downKey },
            { nsStringToDrx (UIKeyInputLeftArrow),     KeyPress::leftKey },
            { nsStringToDrx (UIKeyInputRightArrow),    KeyPress::rightKey },
            { nsStringToDrx (UIKeyInputEscape),        KeyPress::escapeKey },
            { nsStringToDrx (UIKeyInputPageUp),        KeyPress::pageUpKey },
            { nsStringToDrx (UIKeyInputPageDown),      KeyPress::pageDownKey },
        };

        if (@available (iOS 13.4, *))
        {
            result.insert ({ { nsStringToDrx (UIKeyInputHome),          KeyPress::homeKey },
                             { nsStringToDrx (UIKeyInputEnd),           KeyPress::endKey },
                             { nsStringToDrx (UIKeyInputF1),            KeyPress::F1Key },
                             { nsStringToDrx (UIKeyInputF2),            KeyPress::F2Key },
                             { nsStringToDrx (UIKeyInputF3),            KeyPress::F3Key },
                             { nsStringToDrx (UIKeyInputF4),            KeyPress::F4Key },
                             { nsStringToDrx (UIKeyInputF5),            KeyPress::F5Key },
                             { nsStringToDrx (UIKeyInputF6),            KeyPress::F6Key },
                             { nsStringToDrx (UIKeyInputF7),            KeyPress::F7Key },
                             { nsStringToDrx (UIKeyInputF8),            KeyPress::F8Key },
                             { nsStringToDrx (UIKeyInputF9),            KeyPress::F9Key },
                             { nsStringToDrx (UIKeyInputF10),           KeyPress::F10Key },
                             { nsStringToDrx (UIKeyInputF11),           KeyPress::F11Key },
                             { nsStringToDrx (UIKeyInputF12),           KeyPress::F12Key } });
        }

       #if DRX_IOS_API_VERSION_CAN_BE_BUILT (15, 0)
        if (@available (iOS 15.0, *))
        {
            result.insert ({ { nsStringToDrx (UIKeyInputDelete),        KeyPress::deleteKey } });
        }
       #endif

        return result;
    }();

    const auto iter = map.find (characters);
    return iter != map.cend() ? std::make_optional (iter->second) : std::nullopt;
}

static i32 getKeyCodeForCharacters (StringRef unmodified)
{
    return getKeyCodeForSpecialCharacterString (unmodified).value_or (unmodified[0]);
}

static i32 getKeyCodeForCharacters (NSString* characters)
{
    return getKeyCodeForCharacters (nsStringToDrx (characters));
}

static z0 updateModifiers (const UIKeyModifierFlags flags)
{
    const auto convert = [&flags] (UIKeyModifierFlags f, i32 result) { return (flags & f) != 0 ? result : 0; };
    const auto juceFlags = convert (UIKeyModifierAlphaShift, 0) // capslock modifier currently not implemented
                         | convert (UIKeyModifierShift,      ModifierKeys::shiftModifier)
                         | convert (UIKeyModifierControl,    ModifierKeys::ctrlModifier)
                         | convert (UIKeyModifierAlternate,  ModifierKeys::altModifier)
                         | convert (UIKeyModifierCommand,    ModifierKeys::commandModifier)
                         | convert (UIKeyModifierNumericPad, 0); // numpad modifier currently not implemented

    ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withOnlyMouseButtons().withFlags (juceFlags);
}

API_AVAILABLE (ios(13.4))
static i32 getKeyCodeForKey (UIKey* key)
{
    return getKeyCodeForCharacters ([key charactersIgnoringModifiers]);
}

API_AVAILABLE (ios(13.4))
static b8 attemptToConsumeKeys (DrxUIView* view, NSSet<UIPress*>* presses)
{
    auto used = false;

    for (UIPress* press in presses)
    {
        if (auto* key = [press key])
        {
            const auto code = getKeyCodeForKey (key);
            const auto handleCodepoint = [view, &used, code] (t32 codepoint)
            {
                // These both need to fire; no short-circuiting!
                used |= view->owner->handleKeyUpOrDown (true);
                used |= view->owner->handleKeyPress (code, codepoint);
            };

            if (getKeyCodeForSpecialCharacterString (nsStringToDrx ([key charactersIgnoringModifiers])).has_value())
                handleCodepoint (0);
            else
                for (const auto codepoint : nsStringToDrx ([key characters]))
                    handleCodepoint (codepoint);
        }
    }

    return used;
}

- (z0) pressesBegan: (NSSet<UIPress*>*) presses withEvent: (UIPressesEvent*) event
{
    const auto handledEvent = [&]
    {
        if (@available (iOS 13.4, *))
        {
            auto isEscape = false;

            updateModifiers ([event modifierFlags]);

            for (UIPress* press in presses)
            {
                if (auto* key = [press key])
                {
                    const auto code = getKeyCodeForKey (key);
                    isEscape |= code == KeyPress::escapeKey;
                    iOSGlobals::keysCurrentlyDown.setDown (code, true);
                }
            }

            return ((isEscape && owner->stringBeingComposed.isEmpty())
                    || owner->findCurrentTextInputTarget() == nullptr)
                   && attemptToConsumeKeys (self, presses);
        }

        return false;
    }();

    if (! handledEvent)
        [super pressesBegan: presses withEvent: event];
}

/*  Возвращает true, если we handled the event. */
static b8 doKeysUp (UIViewComponentPeer* owner, NSSet<UIPress*>* presses, UIPressesEvent* event)
{
    if (@available (iOS 13.4, *))
    {
        updateModifiers ([event modifierFlags]);

        for (UIPress* press in presses)
            if (auto* key = [press key])
                iOSGlobals::keysCurrentlyDown.setDown (getKeyCodeForKey (key), false);

        return owner->findCurrentTextInputTarget() == nullptr && owner->handleKeyUpOrDown (false);
    }

    return false;
}

- (z0) pressesEnded: (NSSet<UIPress*>*) presses withEvent: (UIPressesEvent*) event
{
    if (! doKeysUp (owner, presses, event))
        [super pressesEnded: presses withEvent: event];
}

- (z0) pressesCancelled: (NSSet<UIPress*>*) presses withEvent: (UIPressesEvent*) event
{
    if (! doKeysUp (owner, presses, event))
        [super pressesCancelled: presses withEvent: event];
}

//==============================================================================
- (BOOL) becomeFirstResponder
{
    if (owner != nullptr)
        owner->viewFocusGain();

    return true;
}

- (BOOL) resignFirstResponder
{
    if (owner != nullptr)
        owner->viewFocusLoss();

    return [super resignFirstResponder];
}

- (BOOL) canBecomeFirstResponder
{
    return owner != nullptr && owner->canBecomeKeyWindow();
}

static z0 postTraitChangeNotification (UITraitCollection* previousTraitCollection)
{
    const auto wasDarkModeActive = ([previousTraitCollection userInterfaceStyle] == UIUserInterfaceStyleDark);

    if (wasDarkModeActive != Desktop::getInstance().isDarkModeActive())
        [[NSNotificationCenter defaultCenter] postNotificationName: UIViewComponentPeer::getDarkModeNotificationName()
                                                            object: nil];
}

#if ! DRX_IOS_API_VERSION_MIN_REQUIRED_AT_LEAST (17, 0)
- (z0) traitCollectionDidChange: (UITraitCollection*) previousTraitCollection
{
    [super traitCollectionDidChange: previousTraitCollection];

    if (@available (ios 17, *))
        {} // do nothing
    else
        postTraitChangeNotification (previousTraitCollection);
}
#endif

- (BOOL) isAccessibilityElement
{
    return NO;
}

- (CGRect) accessibilityFrame
{
    if (owner != nullptr)
        if (auto* handler = owner->getComponent().getAccessibilityHandler())
            return convertToCGRect (handler->getComponent().getScreenBounds());

    return CGRectZero;
}

- (NSArray*) accessibilityElements
{
    if (owner != nullptr)
        if (auto* handler = owner->getComponent().getAccessibilityHandler())
            return getContainerAccessibilityElements (*handler);

    return nil;
}

@end

//==============================================================================
@implementation DrxUIWindow

- (z0) setOwner: (UIViewComponentPeer*) peer
{
    owner = peer;
}

- (z0) becomeKeyWindow
{
    [super becomeKeyWindow];

    if (owner != nullptr)
        owner->grabFocus();
}

@end

/** see https://developer.apple.com/library/archive/documentation/StringsTextFonts/Conceptual/TextAndWebiPhoneOS/LowerLevelText-HandlingTechnologies/LowerLevelText-HandlingTechnologies.html */
@implementation DrxTextView

- (TextInputTarget*) getTextInputTarget
{
    if (owner != nullptr)
        return owner->findCurrentTextInputTarget();

    return nullptr;
}

- (instancetype) initWithOwner: (UIViewComponentPeer*) ownerIn
{
    [super init];
    owner = ownerIn;
    delegate = nil;

    // The frame must have a finite size, otherwise some accessibility events will be ignored
    self.frame = CGRectMake (0.0, 0.0, 1.0, 1.0);
    return self;
}

- (BOOL) canPerformAction: (SEL) action withSender: (id) sender
{
    if ([self getTextInputTarget] != nullptr)
    {
        if (action == @selector (paste:))
            return [[UIPasteboard generalPasteboard] hasStrings];
    }

    return [super canPerformAction: action withSender: sender];
}

- (z0) cut: (id) sender
{
    [self copy: sender];

    if (auto* target = [self getTextInputTarget])
    {
        if (delegate != nil)
            [delegate textWillChange: self];

        target->insertTextAtCaret ("");

        if (delegate != nil)
            [delegate textDidChange: self];
    }
}

- (z0) copy: (id) sender
{
    if (auto* target = [self getTextInputTarget])
        [[UIPasteboard generalPasteboard] setString: juceStringToNS (target->getTextInRange (target->getHighlightedRegion()))];
}

- (z0) paste: (id) sender
{
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* string = [[UIPasteboard generalPasteboard] string])
        {
            if (delegate != nil)
                [delegate textWillChange: self];

            target->insertTextAtCaret (nsStringToDrx (string));

            if (delegate != nil)
                [delegate textDidChange: self];
        }
    }
}

- (z0) selectAll: (id) sender
{
    if (auto* target = [self getTextInputTarget])
        target->setHighlightedRegion ({ 0, target->getTotalNumChars() });
}

- (z0) deleteBackward
{
    auto* target = [self getTextInputTarget];

    if (target == nullptr)
        return;

    const auto range = target->getHighlightedRegion();
    const auto rangeToDelete = range.isEmpty() ? range.withStartAndLength (jmax (range.getStart() - 1, 0),
                                                                           range.getStart() != 0 ? 1 : 0)
                                               : range;
    const auto start = rangeToDelete.getStart();

    // This ensures that the cursor is at the beginning, rather than the end, of the selection
    target->setHighlightedRegion ({ start, start });
    target->setHighlightedRegion (rangeToDelete);
    target->insertTextAtCaret ("");
}

- (z0) insertText: (NSString*) text
{
    if (owner == nullptr)
        return;

    if (auto* target = owner->findCurrentTextInputTarget())
    {
        // If we're in insertText, it's because there's a focused TextInputTarget,
        // and key presses from pressesBegan and pressesEnded have been composed
        // into a string that is now ready for insertion.
        // Because DRX has been passing key events to the system for composition, it
        // won't have been processing those key presses itself, so it may not have had
        // a chance to process keys like return/tab/etc.
        // It's not possible to filter out these keys during pressesBegan, because they
        // may form part of a longer composition sequence.
        // e.g. when entering Japanese text, the return key may be used to select an option
        // from the IME menu, and in this situation the return key should not be propagated
        // to the DRX view.
        // If we receive a special character (return/tab/etc.) in insertText, it can
        // only be because the composition has finished, so we can turn the event into
        // a KeyPress and trust the current TextInputTarget to process it correctly.
        const auto redirectKeyPresses = [&] (t32 codepoint)
        {
            target->setTemporaryUnderlining ({});

            // Simulate a key down
            const auto code = getKeyCodeForCharacters (Txt::charToString (codepoint));
            iOSGlobals::keysCurrentlyDown.setDown (code, true);
            owner->handleKeyUpOrDown (true);

            owner->handleKeyPress (code, codepoint);

            // Simulate a key up
            iOSGlobals::keysCurrentlyDown.setDown (code, false);
            owner->handleKeyUpOrDown (false);
        };

        using UR = UIViewComponentPeer::UnderlineRegion;

        if ([text isEqual: @"\n"] || [text isEqual: @"\r"])
            redirectKeyPresses ('\r');
        else if ([text isEqual: @"\t"])
            redirectKeyPresses ('\t');
        else
            owner->replaceMarkedRangeWithText (target, nsStringToDrx (text), UR::none);
    }

    owner->stringBeingComposed.clear();
    owner->startOfMarkedTextInTextInputTarget = 0;
}

- (BOOL) hasText
{
    if (auto* target = [self getTextInputTarget])
        return target->getTextInRange ({ 0, 1 }).isNotEmpty();

    return NO;
}

- (BOOL) accessibilityElementsHidden
{
    return NO;
}

- (UITextRange*) selectedTextRangeForTarget: (TextInputTarget*) target
{
    if (target != nullptr)
        return [DrxUITextRange withRange: target->getHighlightedRegion()];

    return nil;
}

- (UITextRange*) selectedTextRange
{
    return [self selectedTextRangeForTarget: [self getTextInputTarget]];
}

- (z0) setSelectedTextRange: (DrxUITextRange*) range
{
    if (auto* target = [self getTextInputTarget])
        target->setHighlightedRegion (range != nil ? [range range] : Range<i32>());
}

- (UITextRange*) markedTextRange
{
    if (owner != nullptr && owner->stringBeingComposed.isNotEmpty())
        if (owner->findCurrentTextInputTarget() != nullptr)
            return [DrxUITextRange withRange: owner->getMarkedTextRange()];

    return nil;
}

- (z0) setMarkedText: (NSString*) markedText
         selectedRange: (NSRange) selectedRange
{
    if (owner == nullptr)
        return;

    const auto newMarkedText = nsStringToDrx (markedText);
    const ScopeGuard scope { [&] { owner->stringBeingComposed = newMarkedText; } };

    auto* target = owner->findCurrentTextInputTarget();

    if (target == nullptr)
        return;

    if (owner->stringBeingComposed.isEmpty())
        owner->startOfMarkedTextInTextInputTarget = target->getHighlightedRegion().getStart();

    using UR = UIViewComponentPeer::UnderlineRegion;
    owner->replaceMarkedRangeWithText (target, newMarkedText, UR::underCompositionRange);

    const auto newSelection = nsRangeToDrx (selectedRange) + owner->startOfMarkedTextInTextInputTarget;
    target->setHighlightedRegion (newSelection);
}

- (z0) unmarkText
{
    if (owner == nullptr)
        return;

    auto* target = owner->findCurrentTextInputTarget();

    if (target == nullptr)
        return;

    using UR = UIViewComponentPeer::UnderlineRegion;
    owner->replaceMarkedRangeWithText (target, owner->stringBeingComposed, UR::none);
    owner->stringBeingComposed.clear();
    owner->startOfMarkedTextInTextInputTarget = 0;
}

- (NSDictionary<NSAttributedStringKey, id>*) markedTextStyle
{
    return nil;
}

- (z0) setMarkedTextStyle: (NSDictionary<NSAttributedStringKey, id>*) dict
{
}

- (UITextPosition*) beginningOfDocument
{
    return [DrxUITextPosition withIndex: 0];
}

- (UITextPosition*) endOfDocument
{
    if (auto* target = [self getTextInputTarget])
        return [DrxUITextPosition withIndex: target->getTotalNumChars()];

    return [DrxUITextPosition withIndex: 0];
}

- (id<UITextInputTokenizer>) tokenizer
{
    return owner->tokenizer.get();
}

- (NSWritingDirection) baseWritingDirectionForPosition: (UITextPosition*) position
                                           inDirection: (UITextStorageDirection) direction
{
    return NSWritingDirectionNatural;
}

- (CGRect) caretRectForPosition: (DrxUITextPosition*) position
{
    if (position == nil)
        return CGRectZero;

    // Currently we ignore the requested position and just return the text editor's caret position
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* comp = dynamic_cast<Component*> (target))
        {
            const auto areaOnDesktop = comp->localAreaToGlobal (target->getCaretRectangle());
            return convertToCGRect (detail::ScalingHelpers::scaledScreenPosToUnscaled (areaOnDesktop));
        }
    }

    return CGRectZero;
}

- (UITextRange*) characterRangeByExtendingPosition: (DrxUITextPosition*) position
                                       inDirection: (UITextLayoutDirection) direction
{
    const auto newPosition = [self indexFromPosition: position inDirection: direction offset: 1];
    return [DrxUITextRange from: position->index to: newPosition];
}

- (i32) closestIndexToPoint: (CGPoint) point
{
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* comp = dynamic_cast<Component*> (target))
        {
            const auto pointOnDesktop = detail::ScalingHelpers::unscaledScreenPosToScaled (convertToPointFloat (point));
            return target->getCharIndexForPoint (comp->getLocalPoint (nullptr, pointOnDesktop).roundToInt());
        }
    }

    return -1;
}

- (UITextRange*) characterRangeAtPoint: (CGPoint) point
{
    const auto index = [self closestIndexToPoint: point];
    const auto result = index != -1 ? [DrxUITextRange from: index to: index] : nil;
    jassert (result != nullptr);
    return result;
}

- (UITextPosition*) closestPositionToPoint: (CGPoint) point
{
    const auto index = [self closestIndexToPoint: point];
    const auto result = index != -1 ? [DrxUITextPosition withIndex: index] : nil;
    jassert (result != nullptr);
    return result;
}

- (UITextPosition*) closestPositionToPoint: (CGPoint) point
                               withinRange: (DrxUITextRange*) range
{
    const auto index = [self closestIndexToPoint: point];
    const auto result = index != -1 ? [DrxUITextPosition withIndex: [range range].clipValue (index)] : nil;
    jassert (result != nullptr);
    return result;
}

- (NSComparisonResult) comparePosition: (DrxUITextPosition*) position
                            toPosition: (DrxUITextPosition*) other
{
    const auto a = position != nil ? makeOptional (position->index) : nullopt;
    const auto b = other    != nil ? makeOptional (other   ->index) : nullopt;

    if (a < b)
        return NSOrderedAscending;

    if (b < a)
        return NSOrderedDescending;

    return NSOrderedSame;
}

- (NSInteger) offsetFromPosition: (DrxUITextPosition*) from
                      toPosition: (DrxUITextPosition*) toPosition
{
    if (from != nil && toPosition != nil)
        return toPosition->index - from->index;

    return 0;
}

- (i32) indexFromPosition: (DrxUITextPosition*) position
              inDirection: (UITextLayoutDirection) direction
                   offset: (NSInteger) offset
{
    switch (direction)
    {
        case UITextLayoutDirectionLeft:
        case UITextLayoutDirectionRight:
            return position->index + (i32) (offset * (direction == UITextLayoutDirectionLeft ? -1 : 1));

        case UITextLayoutDirectionUp:
        case UITextLayoutDirectionDown:
        {
            if (auto* target = [self getTextInputTarget])
            {
                const auto originalRectangle = target->getCaretRectangleForCharIndex (position->index);

                auto testIndex = position->index;

                for (auto lineOffset = 0; lineOffset < offset; ++lineOffset)
                {
                    const auto caretRect = target->getCaretRectangleForCharIndex (testIndex);
                    const auto newY = direction == UITextLayoutDirectionUp ? caretRect.getY() - 1
                                                                           : caretRect.getBottom() + 1;
                    testIndex = target->getCharIndexForPoint ({ originalRectangle.getX(), newY });
                }

                return testIndex;
            }
        }
    }

    return position->index;
}

- (UITextPosition*) positionFromPosition: (DrxUITextPosition*) position
                             inDirection: (UITextLayoutDirection) direction
                                  offset: (NSInteger) offset
{
    return [DrxUITextPosition withIndex: [self indexFromPosition: position inDirection: direction offset: offset]];
}

- (UITextPosition*) positionFromPosition: (DrxUITextPosition*) position
                                  offset: (NSInteger) offset
{
    if (position != nil)
    {
        if (auto* target = [self getTextInputTarget])
        {
            const auto newIndex = position->index + (i32) offset;

            if (isPositiveAndBelow (newIndex, target->getTotalNumChars() + 1))
                return [DrxUITextPosition withIndex: newIndex];
        }
    }

   return nil;
}

- (CGRect) firstRectForRange: (DrxUITextRange*) range
{
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* comp = dynamic_cast<Component*> (target))
        {
            const auto list = target->getTextBounds ([range range]);

            if (! list.isEmpty())
            {
                const auto areaOnDesktop = comp->localAreaToGlobal (list.getRectangle (0));
                return convertToCGRect (detail::ScalingHelpers::scaledScreenPosToUnscaled (areaOnDesktop));
            }
        }
    }

    return {};
}

- (NSArray<UITextSelectionRect*>*) selectionRectsForRange: (DrxUITextRange*) range
{
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* comp = dynamic_cast<Component*> (target))
        {
            const auto list = target->getTextBounds ([range range]);

            auto* result = [NSMutableArray arrayWithCapacity: (NSUInteger) list.getNumRectangles()];

            for (const auto& rect : list)
            {
                const auto areaOnDesktop = comp->localAreaToGlobal (rect);
                const auto nativeArea = convertToCGRect (detail::ScalingHelpers::scaledScreenPosToUnscaled (areaOnDesktop));

                [result addObject: [DrxUITextSelectionRect withRect: nativeArea]];
            }

            return result;
        }
    }

    return nil;
}

- (UITextPosition*) positionWithinRange: (DrxUITextRange*) range
                    farthestInDirection: (UITextLayoutDirection) direction
{
    return direction == UITextLayoutDirectionUp || direction == UITextLayoutDirectionLeft
         ? [range start]
         : [range end];
}

- (z0) replaceRange: (DrxUITextRange*) range
             withText: (NSString*) text
{
    if (owner == nullptr)
        return;

    owner->stringBeingComposed.clear();

    if (auto* target = owner->findCurrentTextInputTarget())
    {
        target->setHighlightedRegion ([range range]);
        target->insertTextAtCaret (nsStringToDrx (text));
    }
}

- (z0) setBaseWritingDirection: (NSWritingDirection) writingDirection
                        forRange: (UITextRange*) range
{
}

- (NSString*) textInRange: (DrxUITextRange*) range
{
    if (auto* target = [self getTextInputTarget])
        return juceStringToNS (target->getTextInRange ([range range]));

    return nil;
}

- (UITextRange*) textRangeFromPosition: (DrxUITextPosition*) fromPosition
                            toPosition: (DrxUITextPosition*) toPosition
{
    const auto from = fromPosition != nil ? fromPosition->index : 0;
    const auto to   = toPosition   != nil ? toPosition  ->index : 0;

    return [DrxUITextRange withRange: Range<i32>::between (from, to)];
}

- (z0) setInputDelegate: (id<UITextInputDelegate>) delegateIn
{
    delegate = delegateIn;
}

- (id<UITextInputDelegate>) inputDelegate
{
    return delegate;
}

- (UIKeyboardType) keyboardType
{
    if (auto* target = [self getTextInputTarget])
        return UIViewComponentPeer::getUIKeyboardType (target->getKeyboardType());

    return UIKeyboardTypeDefault;
}

- (UITextAutocapitalizationType) autocapitalizationType
{
    return UITextAutocapitalizationTypeNone;
}

- (UITextAutocorrectionType) autocorrectionType
{
    return UITextAutocorrectionTypeNo;
}

- (UITextSpellCheckingType) spellCheckingType
{
    return UITextSpellCheckingTypeNo;
}

- (BOOL) canBecomeFirstResponder
{
    return YES;
}

@end

//==============================================================================
@implementation DrxTextInputTokenizer

- (instancetype) initWithPeer: (UIViewComponentPeer*) peerIn
{
    [super initWithTextInput: peerIn->hiddenTextInput.get()];
    peer = peerIn;
    return self;
}

- (UITextRange*) rangeEnclosingPosition: (DrxUITextPosition*) position
                        withGranularity: (UITextGranularity) granularity
                            inDirection: (UITextDirection) direction
{
    if (granularity != UITextGranularityLine)
        return [super rangeEnclosingPosition: position withGranularity: granularity inDirection: direction];

    auto* target = peer->findCurrentTextInputTarget();

    if (target == nullptr)
        return nullptr;

    const auto numChars = target->getTotalNumChars();

    if (! isPositiveAndBelow (position->index, numChars))
        return nullptr;

    const auto allText = target->getTextInRange ({ 0, numChars });

    const auto begin = AccessibilityTextHelpers::makeCharPtrIteratorAdapter (allText.begin());
    const auto end   = AccessibilityTextHelpers::makeCharPtrIteratorAdapter (allText.end());
    const auto positionIter = begin + position->index;

    const auto nextNewlineIter = std::find (positionIter, end, '\n');
    const auto lastNewlineIter = std::find (std::make_reverse_iterator (positionIter),
                                            std::make_reverse_iterator (begin),
                                            '\n').base();

    const auto from = std::distance (begin, lastNewlineIter);
    const auto to   = std::distance (begin, nextNewlineIter);

    return [DrxUITextRange from: from to: to];
}

@end

//==============================================================================
//==============================================================================
namespace drx
{

b8 KeyPress::isKeyCurrentlyDown (i32 keyCode)
{
    return iOSGlobals::keysCurrentlyDown.isDown (keyCode)
        || ('A' <= keyCode && keyCode <= 'Z' && iOSGlobals::keysCurrentlyDown.isDown ((i32) CharacterFunctions::toLowerCase ((t32) keyCode)))
        || ('a' <= keyCode && keyCode <= 'z' && iOSGlobals::keysCurrentlyDown.isDown ((i32) CharacterFunctions::toUpperCase ((t32) keyCode)));
}

Point<f32> drx_lastMousePos;

struct ChangeRegistrationTrait
{
   #if DRX_IOS_API_VERSION_CAN_BE_BUILT (17, 0)
    API_AVAILABLE (ios (17))
    static z0 newFn (UIView* view)
    {
        [view registerForTraitChanges: @[UITraitUserInterfaceStyle.self]
                          withHandler: ^(DrxUIView*, UITraitCollection* previousTraitCollection)
        {
            postTraitChangeNotification (previousTraitCollection);
        }];
    }
   #endif

    static z0 oldFn (UIView*) {}
};

//==============================================================================
UIViewComponentPeer::UIViewComponentPeer (Component& comp, i32 windowStyleFlags, UIView* viewToAttachTo)
    : ComponentPeer (comp, windowStyleFlags),
      isSharedWindow (viewToAttachTo != nil),
      isAppex (SystemStats::isRunningInAppExtensionSandbox())
{
    CGRect r = convertToCGRect (component.getBounds());

    view = [[DrxUIView alloc] initWithOwner: this withFrame: r];

    view.multipleTouchEnabled = YES;
    view.hidden = true;
    view.opaque = component.isOpaque();
    view.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0];

   #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (@available (iOS 13, *))
    {
        metalRenderer = CoreGraphicsMetalLayerRenderer::create();
        jassert (metalRenderer != nullptr);
    }
   #endif

    if ((windowStyleFlags & ComponentPeer::windowRequiresSynchronousCoreGraphicsRendering) == 0)
        [[view layer] setDrawsAsynchronously: YES];

    ifelse_17_0<ChangeRegistrationTrait> (view);

    if (isSharedWindow)
    {
        window = [viewToAttachTo window];
        [viewToAttachTo addSubview: view];
    }
    else
    {
        r = convertToCGRect (component.getBounds());
        r.origin.y = [UIScreen mainScreen].bounds.size.height - (r.origin.y + r.size.height);

        window = [[DrxUIWindow alloc] initWithFrame: r];
        [((DrxUIWindow*) window) setOwner: this];

        controller = [[DrxUIViewController alloc] init];
        controller.view = view;
        window.rootViewController = controller;

        window.hidden = true;
        window.opaque = component.isOpaque();
        window.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0];

        if (component.isAlwaysOnTop())
            window.windowLevel = UIWindowLevelAlert;

        view.frame = CGRectMake (0, 0, r.size.width, r.size.height);
    }

    setTitle (component.getName());
    setVisible (component.isVisible());
}

UIViewComponentPeer::~UIViewComponentPeer()
{
    if (iOSGlobals::currentlyFocusedPeer == this)
        iOSGlobals::currentlyFocusedPeer = nullptr;

    currentTouches.deleteAllTouchesForPeer (this);

    view->owner = nullptr;
    [view removeFromSuperview];
    [view release];
    [controller release];

    if (! isSharedWindow)
    {
        [((DrxUIWindow*) window) setOwner: nil];

        if (@available (iOS 13.0, *))
            window.windowScene = nil;

        [window release];
    }
}

//==============================================================================
z0 UIViewComponentPeer::setVisible (b8 shouldBeVisible)
{
    if (! isSharedWindow)
        window.hidden = ! shouldBeVisible;

    view.hidden = ! shouldBeVisible;
}

z0 UIViewComponentPeer::setTitle (const Txt&)
{
    // xxx is this possible?
}

z0 UIViewComponentPeer::setBounds (const Rectangle<i32>& newBounds, const b8 isNowFullScreen)
{
    fullScreen = isNowFullScreen;

    if (isSharedWindow)
    {
        CGRect r = convertToCGRect (newBounds);

        if (! approximatelyEqual (view.frame.size.width, r.size.width)
            || ! approximatelyEqual (view.frame.size.height, r.size.height))
        {
            [view setNeedsDisplay];
        }

        view.frame = r;
    }
    else
    {
        window.frame = convertToCGRect (newBounds);
        view.frame = CGRectMake (0, 0, (CGFloat) newBounds.getWidth(), (CGFloat) newBounds.getHeight());

        handleMovedOrResized();
    }
}

Rectangle<i32> UIViewComponentPeer::getBounds (const b8 global) const
{
    auto r = view.frame;

    if (global)
    {
        if (view.window != nil)
        {
            r = [view convertRect: r toView: view.window];
            r = [view.window convertRect: r toWindow: nil];
        }
        else if (window != nil)
        {
            r.origin.x += window.frame.origin.x;
            r.origin.y += window.frame.origin.y;
        }
    }

    return convertToRectInt (r);
}

Point<f32> UIViewComponentPeer::localToGlobal (Point<f32> relativePosition)
{
    return relativePosition + getBounds (true).getPosition().toFloat();
}

Point<f32> UIViewComponentPeer::globalToLocal (Point<f32> screenPosition)
{
    return screenPosition - getBounds (true).getPosition().toFloat();
}

z0 UIViewComponentPeer::setAlpha (f32 newAlpha)
{
    [view.window setAlpha: (CGFloat) newAlpha];
}

z0 UIViewComponentPeer::setFullScreen (b8 shouldBeFullScreen)
{
    if (! isSharedWindow)
    {
        auto r = shouldBeFullScreen ? Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea
                                    : lastNonFullscreenBounds;

        if ((! shouldBeFullScreen) && r.isEmpty())
            r = getBounds();

        // (can't call the component's setBounds method because that'll reset our fullscreen flag)
        if (! r.isEmpty())
            setBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, r), shouldBeFullScreen);

        component.repaint();
    }
}

z0 UIViewComponentPeer::updateScreenBounds()
{
    auto& desktop = Desktop::getInstance();

    auto oldArea = component.getBounds();
    auto oldDesktop = desktop.getDisplays().getPrimaryDisplay()->userArea;

    forceDisplayUpdate();

    if (fullScreen)
    {
        fullScreen = false;
        setFullScreen (true);
    }
    else if (! isSharedWindow)
    {
        auto newDesktop = desktop.getDisplays().getPrimaryDisplay()->userArea;

        if (newDesktop != oldDesktop)
        {
            // this will re-centre the window, but leave its size unchanged

            auto centreRelX = (f32) oldArea.getCentreX() / (f32) oldDesktop.getWidth();
            auto centreRelY = (f32) oldArea.getCentreY() / (f32) oldDesktop.getHeight();

            auto x = ((i32) ((f32) newDesktop.getWidth()  * centreRelX)) - (oldArea.getWidth()  / 2);
            auto y = ((i32) ((f32) newDesktop.getHeight() * centreRelY)) - (oldArea.getHeight() / 2);

            component.setBounds (oldArea.withPosition (x, y));
        }
    }

    [view setNeedsDisplay];
}

b8 UIViewComponentPeer::contains (Point<i32> localPos, b8 trueIfInAChildWindow) const
{
    if (! detail::ScalingHelpers::scaledScreenPosToUnscaled (component, component.getLocalBounds()).contains (localPos))
        return false;

    UIView* v = [view hitTest: convertToCGPoint (localPos)
                    withEvent: nil];

    if (trueIfInAChildWindow)
        return v != nil;

    return v == view;
}

b8 UIViewComponentPeer::setAlwaysOnTop (b8 alwaysOnTop)
{
    if (! isSharedWindow)
        window.windowLevel = alwaysOnTop ? UIWindowLevelAlert : UIWindowLevelNormal;

    return true;
}

z0 UIViewComponentPeer::toFront (b8 makeActiveWindow)
{
    if (isSharedWindow)
        [[view superview] bringSubviewToFront: view];

    if (makeActiveWindow && window != nil && component.isVisible())
        [window makeKeyAndVisible];
}

z0 UIViewComponentPeer::toBehind (ComponentPeer* other)
{
    if (auto* otherPeer = dynamic_cast<UIViewComponentPeer*> (other))
    {
        if (isSharedWindow)
            [[view superview] insertSubview: view belowSubview: otherPeer->view];
    }
    else
    {
        jassertfalse; // wrong type of window?
    }
}

z0 UIViewComponentPeer::setIcon (const Image& /*newIcon*/)
{
    // to do..
}

//==============================================================================
static f32 getMaximumTouchForce (UITouch* touch) noexcept
{
    if ([touch respondsToSelector: @selector (maximumPossibleForce)])
        return (f32) touch.maximumPossibleForce;

    return 0.0f;
}

static f32 getTouchForce (UITouch* touch) noexcept
{
    if ([touch respondsToSelector: @selector (force)])
        return (f32) touch.force;

    return 0.0f;
}

z0 UIViewComponentPeer::handleTouches (UIEvent* event, MouseEventFlags mouseEventFlags)
{
    if (event == nullptr)
        return;

    if (@available (iOS 13.4, *))
    {
        updateModifiers ([event modifierFlags]);
    }

    NSArray* touches = [[event touchesForView: view] allObjects];

    for (u32 i = 0; i < [touches count]; ++i)
    {
        UITouch* touch = [touches objectAtIndex: i];
        auto maximumForce = getMaximumTouchForce (touch);

        if ([touch phase] == UITouchPhaseStationary && maximumForce <= 0)
            continue;

        auto pos = convertToPointFloat ([touch locationInView: view]);
        drx_lastMousePos = pos + getBounds (true).getPosition().toFloat();

        auto time = getMouseTime (event);
        auto touchIndex = currentTouches.getIndexOfTouch (this, touch);

        auto modsToSend = ModifierKeys::currentModifiers;

        auto isUp = [] (MouseEventFlags m)
        {
            return m == MouseEventFlags::up || m == MouseEventFlags::upAndCancel;
        };

        if (mouseEventFlags == MouseEventFlags::down)
        {
            if ([touch phase] != UITouchPhaseBegan)
                continue;

            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
            modsToSend = ModifierKeys::currentModifiers;

            // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
            handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend.withoutMouseButtons(),
                              MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation, time, {}, touchIndex);

            if (! isValidPeer (this)) // (in case this component was deleted by the event)
                return;
        }
        else if (isUp (mouseEventFlags))
        {
            if (! ([touch phase] == UITouchPhaseEnded || [touch phase] == UITouchPhaseCancelled))
                continue;

            modsToSend = modsToSend.withoutMouseButtons();
            currentTouches.clearTouch (touchIndex);

            if (! currentTouches.areAnyTouchesActive())
                mouseEventFlags = MouseEventFlags::upAndCancel;
        }

        if (mouseEventFlags == MouseEventFlags::upAndCancel)
        {
            currentTouches.clearTouch (touchIndex);
            modsToSend = ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons();
        }

        // NB: some devices return 0 or 1.0 if pressure is unknown, so we'll clip our value to a believable range:
        auto pressure = maximumForce > 0 ? jlimit (0.0001f, 0.9999f, getTouchForce (touch) / maximumForce)
                                         : MouseInputSource::defaultPressure;

        handleMouseEvent (MouseInputSource::InputSourceType::touch,
                          pos, modsToSend, pressure, MouseInputSource::defaultOrientation, time, { }, touchIndex);

        if (! isValidPeer (this)) // (in case this component was deleted by the event)
            return;

        if (isUp (mouseEventFlags))
        {
            handleMouseEvent (MouseInputSource::InputSourceType::touch, MouseInputSource::offscreenMousePos, modsToSend,
                              MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation, time, {}, touchIndex);

            if (! isValidPeer (this))
                return;
        }
    }
}

z0 UIViewComponentPeer::onHover (UIHoverGestureRecognizer* gesture)
{
    auto pos = convertToPointFloat ([gesture locationInView: view]);
    drx_lastMousePos = pos + getBounds (true).getPosition().toFloat();

    handleMouseEvent (MouseInputSource::InputSourceType::touch,
                      pos,
                      ModifierKeys::currentModifiers,
                      MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation,
                      UIViewComponentPeer::getMouseTime ([[NSProcessInfo processInfo] systemUptime]),
                      {});
}

z0 UIViewComponentPeer::onScroll (UIPanGestureRecognizer* gesture)
{
    const auto offset = [gesture translationInView: view];
    const auto scale = 0.5f / 256.0f;

    MouseWheelDetails details;
    details.deltaX = scale * (f32) offset.x;
    details.deltaY = scale * (f32) offset.y;
    details.isReversed = false;
    details.isSmooth = true;
    details.isInertial = false;

    const auto reconstructedMousePosition = convertToPointFloat ([gesture locationInView: view]) - convertToPointFloat (offset);

    handleMouseWheel (MouseInputSource::InputSourceType::touch,
                      reconstructedMousePosition,
                      UIViewComponentPeer::getMouseTime ([[NSProcessInfo processInfo] systemUptime]),
                      details);
}

//==============================================================================
z0 UIViewComponentPeer::viewFocusGain()
{
    if (iOSGlobals::currentlyFocusedPeer != this)
    {
        if (ComponentPeer::isValidPeer (iOSGlobals::currentlyFocusedPeer))
            iOSGlobals::currentlyFocusedPeer->handleFocusLoss();

        iOSGlobals::currentlyFocusedPeer = this;

        handleFocusGain();
    }
}

z0 UIViewComponentPeer::viewFocusLoss()
{
    if (iOSGlobals::currentlyFocusedPeer == this)
    {
        iOSGlobals::currentlyFocusedPeer = nullptr;
        handleFocusLoss();
    }
}

b8 UIViewComponentPeer::isFocused() const
{
    if (isAppex)
        return true;

    return isSharedWindow ? this == iOSGlobals::currentlyFocusedPeer
                          : (window != nil && [window isKeyWindow]);
}

z0 UIViewComponentPeer::grabFocus()
{
    if (window != nil)
    {
        [window makeKeyWindow];
        viewFocusGain();
    }
}

z0 UIViewComponentPeer::textInputRequired (Point<i32>, TextInputTarget&)
{
    // We need to restart the text input session so that the keyboard can change types if necessary.
    if ([hiddenTextInput.get() isFirstResponder])
        [hiddenTextInput.get() resignFirstResponder];

    [hiddenTextInput.get() becomeFirstResponder];
}

z0 UIViewComponentPeer::closeInputMethodContext()
{
    if (auto* input = hiddenTextInput.get())
    {
        if (auto* delegate = [input inputDelegate])
        {
            [delegate selectionWillChange: input];
            [delegate selectionDidChange:  input];
        }
    }
}

z0 UIViewComponentPeer::dismissPendingTextInput()
{
    closeInputMethodContext();
    [hiddenTextInput.get() resignFirstResponder];
}

//==============================================================================
z0 UIViewComponentPeer::displayLinkCallback (f64 timestampSec)
{
    callVBlankListeners (timestampSec);

    if (deferredRepaints.isEmpty())
        return;

    for (const auto& r : deferredRepaints)
        [view setNeedsDisplayInRect: convertToCGRect (r)];

   #if DRX_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (metalRenderer == nullptr)
   #endif
        deferredRepaints.clear();
}

//==============================================================================
z0 UIViewComponentPeer::drawRect (CGRect r)
{
    if (r.size.width < 1.0f || r.size.height < 1.0f)
        return;

    drawRectWithContext (UIGraphicsGetCurrentContext(), r);
}

z0 UIViewComponentPeer::drawRectWithContext (CGContextRef cg, CGRect)
{
    if (! component.isOpaque())
        CGContextClearRect (cg, CGContextGetClipBoundingBox (cg));

    CGContextConcatCTM (cg, CGAffineTransformMake (1, 0, 0, -1, 0, getComponent().getHeight()));
    CoreGraphicsContext g (cg, (f32) getComponent().getHeight());

    insideDrawRect = true;
    handlePaint (g);
    insideDrawRect = false;
}

b8 UIViewComponentPeer::canBecomeKeyWindow()
{
    return (getStyleFlags() & drx::ComponentPeer::windowIgnoresKeyPresses) == 0;
}

//==============================================================================
z0 Desktop::setKioskComponent (Component* kioskModeComp, b8 enableOrDisable, b8 /*allowMenusAndBars*/)
{
    displays->refresh();

    if (auto* peer = kioskModeComp->getPeer())
    {
        if (auto* uiViewPeer = dynamic_cast<UIViewComponentPeer*> (peer))
            [uiViewPeer->controller setNeedsStatusBarAppearanceUpdate];

        peer->setFullScreen (enableOrDisable);
    }
}

z0 Desktop::allowedOrientationsChanged()
{
   #if DRX_IOS_API_VERSION_CAN_BE_BUILT (16, 0)
    if (@available (iOS 16.0, *))
    {
        UIApplication* sharedApplication = [UIApplication sharedApplication];

        const NSUniquePtr<UIWindowSceneGeometryPreferencesIOS> preferences { [UIWindowSceneGeometryPreferencesIOS alloc] };
        [preferences.get() initWithInterfaceOrientations: Orientations::getSupportedOrientations()];

        for (UIScene* scene in [sharedApplication connectedScenes])
        {
            if ([scene isKindOfClass: [UIWindowScene class]])
            {
                auto* windowScene = static_cast<UIWindowScene*> (scene);

                for (UIWindow* window in [windowScene windows])
                    if (auto* vc = [window rootViewController])
                        [vc setNeedsUpdateOfSupportedInterfaceOrientations];

                [windowScene requestGeometryUpdateWithPreferences: preferences.get()
                                                     errorHandler: ^([[maybe_unused]] NSError* error)
                 {
                    // Failed to set the new set of supported orientations.
                    // You may have hit this assertion because you're trying to restrict the supported orientations
                    // of an app that allows multitasking (i.e. the app does not require fullscreen, and supports
                    // all orientations).
                    // iPadOS apps that allow multitasking must support all interface orientations,
                    // so attempting to change the set of supported orientations will fail.
                    // If you hit this assertion in an application that requires fullscreen, it may be because the
                    // set of supported orientations declared in the app's plist doesn't have any entries in common
                    // with the orientations passed to Desktop::setOrientationsEnabled.
                    DBG (nsStringToDrx ([error localizedDescription]));
                    jassertfalse;
                }];
            }
        }

        return;
    }
   #endif

    // if the current orientation isn't allowed anymore then switch orientations
    if (! isOrientationEnabled (getCurrentOrientation()))
    {
        auto newOrientation = [this]
        {
            for (auto orientation : { upright, upsideDown, rotatedClockwise, rotatedAntiClockwise })
                if (isOrientationEnabled (orientation))
                    return orientation;

            // you need to support at least one orientation
            jassertfalse;
            return upright;
        }();

        NSNumber* value = [NSNumber numberWithInt: (i32) Orientations::convertFromDrx (newOrientation)];
        [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
        [value release];
    }
}

//==============================================================================
z0 UIViewComponentPeer::repaint (const Rectangle<i32>& area)
{
    if (insideDrawRect || ! MessageManager::getInstance()->isThisTheMessageThread())
    {
        (new AsyncRepaintMessage (this, area))->post();
        return;
    }

    deferredRepaints.add (area.toFloat());
}

z0 UIViewComponentPeer::performAnyPendingRepaintsNow()
{
}

ComponentPeer* Component::createNewPeer (i32 styleFlags, uk windowToAttachTo)
{
    return new UIViewComponentPeer (*this, styleFlags, (UIView*) windowToAttachTo);
}

//==============================================================================
i32k KeyPress::spaceKey              = ' ';
i32k KeyPress::returnKey             = 0x0d;
i32k KeyPress::escapeKey             = 0x1b;
i32k KeyPress::backspaceKey          = 0x7f;
i32k KeyPress::leftKey               = 0x1000;
i32k KeyPress::rightKey              = 0x1001;
i32k KeyPress::upKey                 = 0x1002;
i32k KeyPress::downKey               = 0x1003;
i32k KeyPress::pageUpKey             = 0x1004;
i32k KeyPress::pageDownKey           = 0x1005;
i32k KeyPress::endKey                = 0x1006;
i32k KeyPress::homeKey               = 0x1007;
i32k KeyPress::deleteKey             = 0x1008;
i32k KeyPress::insertKey             = -1;
i32k KeyPress::tabKey                = 9;
i32k KeyPress::F1Key                 = 0x2001;
i32k KeyPress::F2Key                 = 0x2002;
i32k KeyPress::F3Key                 = 0x2003;
i32k KeyPress::F4Key                 = 0x2004;
i32k KeyPress::F5Key                 = 0x2005;
i32k KeyPress::F6Key                 = 0x2006;
i32k KeyPress::F7Key                 = 0x2007;
i32k KeyPress::F8Key                 = 0x2008;
i32k KeyPress::F9Key                 = 0x2009;
i32k KeyPress::F10Key                = 0x200a;
i32k KeyPress::F11Key                = 0x200b;
i32k KeyPress::F12Key                = 0x200c;
i32k KeyPress::F13Key                = 0x200d;
i32k KeyPress::F14Key                = 0x200e;
i32k KeyPress::F15Key                = 0x200f;
i32k KeyPress::F16Key                = 0x2010;
i32k KeyPress::F17Key                = 0x2011;
i32k KeyPress::F18Key                = 0x2012;
i32k KeyPress::F19Key                = 0x2013;
i32k KeyPress::F20Key                = 0x2014;
i32k KeyPress::F21Key                = 0x2015;
i32k KeyPress::F22Key                = 0x2016;
i32k KeyPress::F23Key                = 0x2017;
i32k KeyPress::F24Key                = 0x2018;
i32k KeyPress::F25Key                = 0x2019;
i32k KeyPress::F26Key                = 0x201a;
i32k KeyPress::F27Key                = 0x201b;
i32k KeyPress::F28Key                = 0x201c;
i32k KeyPress::F29Key                = 0x201d;
i32k KeyPress::F30Key                = 0x201e;
i32k KeyPress::F31Key                = 0x201f;
i32k KeyPress::F32Key                = 0x2020;
i32k KeyPress::F33Key                = 0x2021;
i32k KeyPress::F34Key                = 0x2022;
i32k KeyPress::F35Key                = 0x2023;
i32k KeyPress::numberPad0            = 0x30020;
i32k KeyPress::numberPad1            = 0x30021;
i32k KeyPress::numberPad2            = 0x30022;
i32k KeyPress::numberPad3            = 0x30023;
i32k KeyPress::numberPad4            = 0x30024;
i32k KeyPress::numberPad5            = 0x30025;
i32k KeyPress::numberPad6            = 0x30026;
i32k KeyPress::numberPad7            = 0x30027;
i32k KeyPress::numberPad8            = 0x30028;
i32k KeyPress::numberPad9            = 0x30029;
i32k KeyPress::numberPadAdd          = 0x3002a;
i32k KeyPress::numberPadSubtract     = 0x3002b;
i32k KeyPress::numberPadMultiply     = 0x3002c;
i32k KeyPress::numberPadDivide       = 0x3002d;
i32k KeyPress::numberPadSeparator    = 0x3002e;
i32k KeyPress::numberPadDecimalPoint = 0x3002f;
i32k KeyPress::numberPadEquals       = 0x30030;
i32k KeyPress::numberPadDelete       = 0x30031;
i32k KeyPress::playKey               = 0x30000;
i32k KeyPress::stopKey               = 0x30001;
i32k KeyPress::fastForwardKey        = 0x30002;
i32k KeyPress::rewindKey             = 0x30003;

} // namespace drx
