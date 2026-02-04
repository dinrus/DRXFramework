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
/**
    The Component class uses a ComponentPeer internally to create and manage a real
    operating-system window.

    This is an abstract base class - the platform specific code contains implementations of
    it for the various platforms.

    User-code should very rarely need to have any involvement with this class.

    @see Component::createNewPeer

    @tags{GUI}
*/
class DRX_API  ComponentPeer : private FocusChangeListener
{
public:
    //==============================================================================
    /** A combination of these flags is passed to the ComponentPeer constructor. */
    enum StyleFlags
    {
        windowAppearsOnTaskbar                          = (1 << 0),   /**< Indicates that the window should have a corresponding
                                                                           entry on the taskbar (ignored on MacOSX) */
        windowIsTemporary                               = (1 << 1),   /**< Indicates that the window is a temporary popup, like a menu,
                                                                           tooltip, etc. */
        windowIgnoresMouseClicks                        = (1 << 2),   /**< Indicates that the window should let mouse clicks pass
                                                                           through it (may not be possible on some platforms). */
        windowHasTitleBar                               = (1 << 3),   /**< Indicates that the window should have a normal OS-specific
                                                                           title bar and frame. if not specified, the window will be
                                                                           borderless. */
        windowIsResizable                               = (1 << 4),   /**< Indicates that the window should have a resizable border. */
        windowHasMinimiseButton                         = (1 << 5),   /**< Indicates that if the window has a title bar, it should have a
                                                                           minimise button on it. */
        windowHasMaximiseButton                         = (1 << 6),   /**< Indicates that if the window has a title bar, it should have a
                                                                           maximise button on it. */
        windowHasCloseButton                            = (1 << 7),   /**< Indicates that if the window has a title bar, it should have a
                                                                           close button on it. */
        windowHasDropShadow                             = (1 << 8),   /**< Indicates that the window should have a drop-shadow (this may
                                                                           not be possible on all platforms). */
        windowRepaintedExplicitly                       = (1 << 9),   /**< Not intended for public use - this tells a window not to
                                                                           do its own repainting, but only to repaint when the
                                                                           performAnyPendingRepaintsNow() method is called. */
        windowIgnoresKeyPresses                         = (1 << 10),  /**< Tells the window not to catch any keypresses. This can
                                                                           be used for things like plugin windows, to stop them interfering
                                                                           with the host's shortcut keys. */
        windowRequiresSynchronousCoreGraphicsRendering  = (1 << 11),  /**< Indicates that the window should not be rendered with
                                                                           asynchronous Core Graphics drawing operations. Use this if there
                                                                           are issues with regions not being redrawn at the expected time
                                                                           (macOS and iOS only). */
        windowIsSemiTransparent                         = (1 << 30)   /**< Not intended for public use - makes a window transparent. */

    };

    /** Represents the window borders around a window component.

        You must use operator b8() to evaluate the validity of the object before accessing
        its value.

        Returned by getFrameSizeIfPresent(). A missing value may be returned on Linux for a
        short time after window creation.
    */
    class DRX_API  OptionalBorderSize final
    {
    public:
        /** Default constructor. Creates an invalid object. */
        OptionalBorderSize()                               : valid (false)                               {}

        /** Constructor. Creates a valid object containing the provided BorderSize<i32>. */
        explicit OptionalBorderSize (BorderSize<i32> size) : valid (true), borderSize (std::move (size)) {}

        /** Возвращает true, если a valid value has been provided. */
        explicit operator b8() const noexcept { return valid; }

        /** Returns a reference to the value.

            You must not call this function on an invalid object. Use operator b8() to
            determine validity.
        */
        const auto& operator*() const noexcept
        {
            jassert (valid);
            return borderSize;
        }

        /** Returns a pointer to the value.

            You must not call this function on an invalid object. Use operator b8() to
            determine validity.
        */
        const auto* operator->() const noexcept
        {
            jassert (valid);
            return &borderSize;
        }

    private:
        b8 valid;
        BorderSize<i32> borderSize;
    };

    //==============================================================================
    /** Creates a peer.

        The component is the one that we intend to represent, and the style flags are
        a combination of the values in the StyleFlags enum
    */
    ComponentPeer (Component& component, i32 styleFlags);

    /** Destructor. */
    ~ComponentPeer() override;

    //==============================================================================
    /** Returns the component being represented by this peer. */
    Component& getComponent() noexcept                      { return component; }

    /** Returns the set of style flags that were set when the window was created.
        @see Component::addToDesktop
    */
    i32 getStyleFlags() const noexcept                      { return styleFlags; }

    /** Returns a unique ID for this peer.
        Each peer that is created is given a different ID.
    */
    u32 getUniqueID() const noexcept                     { return uniqueID; }

    //==============================================================================
    /** Returns the raw handle to whatever kind of window is being used.

        On windows, this is probably a HWND, on the mac, it's likely to be a WindowRef,
        but remember there's no guarantees what you'll get back.
    */
    virtual uk getNativeHandle() const = 0;

    /** Shows or hides the window. */
    virtual z0 setVisible (b8 shouldBeVisible) = 0;

    /** Changes the title of the window. */
    virtual z0 setTitle (const Txt& title) = 0;

    /** If this type of window is capable of indicating that the document in it has been
        edited, then this changes its status.

        For example in OSX, this changes the appearance of the close button.
        @returns true if the window has a mechanism for showing this, or false if not.
    */
    virtual b8 setDocumentEditedStatus (b8 edited);

    /** If this type of window is capable of indicating that it represents a file, then
        this lets you set the file.

        E.g. in OSX it'll show an icon for the file in the title bar.
    */
    virtual z0 setRepresentedFile (const File&);

    //==============================================================================
    /** Moves and resizes the window.

        If the native window is contained in another window, then the coordinates are
        relative to the parent window's origin, not the screen origin.

        This should result in a callback to handleMovedOrResized().
    */
    virtual z0 setBounds (const Rectangle<i32>& newBounds, b8 isNowFullScreen) = 0;

    /** Updates the peer's bounds to match its component. */
    z0 updateBounds();

    /** Returns the current position and size of the window.

        If the native window is contained in another window, then the coordinates are
        relative to the parent window's origin, not the screen origin.
    */
    virtual Rectangle<i32> getBounds() const = 0;

    /** Converts a position relative to the top-left of this component to screen coordinates. */
    virtual Point<f32> localToGlobal (Point<f32> relativePosition) = 0;

    /** Converts a screen coordinate to a position relative to the top-left of this component. */
    virtual Point<f32> globalToLocal (Point<f32> screenPosition) = 0;

    /** Converts a position relative to the top-left of this component to screen coordinates. */
    Point<i32> localToGlobal (Point<i32> relativePosition);

    /** Converts a screen coordinate to a position relative to the top-left of this component. */
    Point<i32> globalToLocal (Point<i32> screenPosition);

    /** Converts a rectangle relative to the top-left of this component to screen coordinates. */
    virtual Rectangle<i32> localToGlobal (const Rectangle<i32>& relativePosition);

    /** Converts a screen area to a position relative to the top-left of this component. */
    virtual Rectangle<i32> globalToLocal (const Rectangle<i32>& screenPosition);

    /** Converts a rectangle relative to the top-left of this component to screen coordinates. */
    Rectangle<f32> localToGlobal (const Rectangle<f32>& relativePosition);

    /** Converts a screen area to a position relative to the top-left of this component. */
    Rectangle<f32> globalToLocal (const Rectangle<f32>& screenPosition);

    /** Returns the area in peer coordinates that is covered by the given sub-comp (which
        may be at any depth)
    */
    Rectangle<i32> getAreaCoveredBy (const Component& subComponent) const;

    /** Minimises the window. */
    virtual z0 setMinimised (b8 shouldBeMinimised) = 0;

    /** True if the window is currently minimised. */
    virtual b8 isMinimised() const = 0;

    /** True if the window is being displayed on-screen. */
    virtual b8 isShowing() const = 0;

    /** Enable/disable fullscreen mode for the window. */
    virtual z0 setFullScreen (b8 shouldBeFullScreen) = 0;

    /** True if the window is currently full-screen. */
    virtual b8 isFullScreen() const = 0;

    /** True if the window is in kiosk-mode. */
    virtual b8 isKioskMode() const;

    /** Sets the size to restore to if fullscreen mode is turned off. */
    z0 setNonFullScreenBounds (const Rectangle<i32>& newBounds) noexcept;

    /** Returns the size to restore to if fullscreen mode is turned off. */
    const Rectangle<i32>& getNonFullScreenBounds() const noexcept;

    /** Attempts to change the icon associated with this window. */
    virtual z0 setIcon (const Image& newIcon) = 0;

    /** Sets a constrainer to use if the peer can resize itself.
        The constrainer won't be deleted by this object, so the caller must manage its lifetime.
    */
    z0 setConstrainer (ComponentBoundsConstrainer* newConstrainer) noexcept;

    /** Asks the window-manager to begin resizing this window, on platforms where this is useful
        (currently just Linux/X11).

        @param mouseDownPosition    The position of the mouse event that started the resize in
                                    unscaled peer coordinates
        @param zone                 The edges of the window that may be moved during the resize
    */
    virtual z0 startHostManagedResize ([[maybe_unused]] Point<i32> mouseDownPosition,
                                         [[maybe_unused]] ResizableBorderComponent::Zone zone) {}

    /** Returns the current constrainer, if one has been set. */
    ComponentBoundsConstrainer* getConstrainer() const noexcept             { return constrainer; }

    /** Checks if a point is in the window.

        The position is relative to the top-left of this window, in unscaled peer coordinates.
        If trueIfInAChildWindow is false, then this returns false if the point is actually
        inside a child of this window.
    */
    virtual b8 contains (Point<i32> localPos, b8 trueIfInAChildWindow) const = 0;

    /** Returns the size of the window frame that's around this window.

        Depending on the platform the border size may be invalid for a short transient
        after creating a new window. Hence the returned value must be checked using
        operator b8() and the contained value can be accessed using operator*() only
        if it is present.

        Whether or not the window has a normal window frame depends on the flags
        that were set when the window was created by Component::addToDesktop()
    */
    virtual OptionalBorderSize getFrameSizeIfPresent() const = 0;

    /** Returns the size of the window frame that's around this window.
        Whether or not the window has a normal window frame depends on the flags
        that were set when the window was created by Component::addToDesktop()
    */
   #if DRX_LINUX || DRX_BSD
    [[deprecated ("Use getFrameSizeIfPresent instead.")]]
   #endif
    virtual BorderSize<i32> getFrameSize() const = 0;

    /** This is called when the window's bounds change.
        A peer implementation must call this when the window is moved and resized, so that
        this method can pass the message on to the component.
    */
    z0 handleMovedOrResized();

    /** This is called if the screen resolution changes.
        A peer implementation must call this if the monitor arrangement changes or the available
        screen size changes.
    */
    virtual z0 handleScreenSizeChange();

    //==============================================================================
    /** This is called to repaint the component into the given context.

        Increments the result of getNumFramesPainted().
    */
    z0 handlePaint (LowLevelGraphicsContext& contextToPaintTo);

    //==============================================================================
    /** Sets this window to either be always-on-top or normal.
        Some kinds of window might not be able to do this, so should return false.
    */
    virtual b8 setAlwaysOnTop (b8 alwaysOnTop) = 0;

    /** Brings the window to the top, optionally also giving it keyboard focus. */
    virtual z0 toFront (b8 takeKeyboardFocus) = 0;

    /** Moves the window to be just behind another one. */
    virtual z0 toBehind (ComponentPeer* other) = 0;

    /** Called when the window is brought to the front, either by the OS or by a call
        to toFront().
    */
    z0 handleBroughtToFront();

    //==============================================================================
    /** True if the window has the keyboard focus. */
    virtual b8 isFocused() const = 0;

    /** Tries to give the window keyboard focus. */
    virtual z0 grabFocus() = 0;

    /** Called when the window gains keyboard focus. */
    z0 handleFocusGain();
    /** Called when the window loses keyboard focus. */
    z0 handleFocusLoss();

    Component* getLastFocusedSubcomponent() const noexcept;

    /** Called when a key is pressed.
        For keycode info, see the KeyPress class.
        Возвращает true, если the keystroke was used.
    */
    b8 handleKeyPress (i32 keyCode, t32 textCharacter);

    /** Called when a key is pressed.
        Возвращает true, если the keystroke was used.
    */
    b8 handleKeyPress (const KeyPress& key);

    /** Called whenever a key is pressed or released.
        Возвращает true, если the keystroke was used.
    */
    b8 handleKeyUpOrDown (b8 isKeyDown);

    /** Called whenever a modifier key is pressed or released. */
    z0 handleModifierKeysChange();

    /** If there's a currently active input-method context - i.e. characters are being
        composed using multiple keystrokes - this should commit the current state of the
        context to the text and clear the context. This should not hide the virtual keyboard.
    */
    virtual z0 closeInputMethodContext();

    /** Alerts the peer that the current text input target has changed somehow.

        The peer may hide or show the virtual keyboard as a result of this call.
    */
    z0 refreshTextInputTarget();

    //==============================================================================
    /** Returns the currently focused TextInputTarget, or null if none is found. */
    TextInputTarget* findCurrentTextInputTarget();

    //==============================================================================
    /** Invalidates a region of the window to be repainted asynchronously. */
    virtual z0 repaint (const Rectangle<i32>& area) = 0;

    /** This can be called (from the message thread) to cause the immediate redrawing
        of any areas of this window that need repainting.

        You shouldn't ever really need to use this, it's mainly for special purposes
        like supporting audio plugins where the host's event loop is out of our control.
    */
    virtual z0 performAnyPendingRepaintsNow() = 0;

    /** Changes the window's transparency. */
    virtual z0 setAlpha (f32 newAlpha) = 0;

    //==============================================================================
    z0 handleMouseEvent (MouseInputSource::InputSourceType type, Point<f32> positionWithinPeer, ModifierKeys newMods, f32 pressure,
                           f32 orientation, z64 time, PenDetails pen = {}, i32 touchIndex = 0);

    z0 handleMouseWheel (MouseInputSource::InputSourceType type, Point<f32> positionWithinPeer,
                           z64 time, const MouseWheelDetails&, i32 touchIndex = 0);

    z0 handleMagnifyGesture (MouseInputSource::InputSourceType type, Point<f32> positionWithinPeer,
                               z64 time, f32 scaleFactor, i32 touchIndex = 0);

    z0 handleUserClosingWindow();

    /** Structure to describe drag and drop information */
    struct DragInfo
    {
        StringArray files;
        Txt text;
        Point<i32> position;

        b8 isEmpty() const noexcept       { return files.size() == 0 && text.isEmpty(); }
        z0 clear() noexcept               { files.clear(); text.clear(); }
    };

    b8 handleDragMove (const DragInfo&);
    b8 handleDragExit (const DragInfo&);
    b8 handleDragDrop (const DragInfo&);

    //==============================================================================
    /** Returns the number of currently-active peers.
        @see getPeer
    */
    static i32 getNumPeers() noexcept;

    /** Returns one of the currently-active peers.
        @see getNumPeers
    */
    static ComponentPeer* getPeer (i32 index) noexcept;

    /** Returns the peer that's attached to the given component, or nullptr if there isn't one. */
    static ComponentPeer* getPeerFor (const Component*) noexcept;

    /** Checks if this peer object is valid.
        @see getNumPeers
    */
    static b8 isValidPeer (const ComponentPeer* peer) noexcept;

    //==============================================================================
    virtual StringArray getAvailableRenderingEngines() = 0;
    virtual i32 getCurrentRenderingEngine() const;
    virtual z0 setCurrentRenderingEngine (i32 index);

    //==============================================================================
    /** On desktop platforms this method will check all the mouse and key states and return
        a ModifierKeys object representing them.

        This isn't recommended and is only needed in special circumstances for up-to-date
        modifier information at times when the app's event loop isn't running normally.

        Another reason to avoid this method is that it's not stateless and calling it may
        update the ModifierKeys::currentModifiers object, which could cause subtle changes
        in the behaviour of some components.
    */
    static ModifierKeys getCurrentModifiersRealtime() noexcept;

    //==============================================================================
    /**  Used to receive callbacks when the OS scale factor of this ComponentPeer changes.

         This is used internally by some native DRX windows on Windows and Linux and you
         shouldn't need to worry about it in your own code unless you are dealing directly
         with native windows.
    */
    struct DRX_API  ScaleFactorListener
    {
        /** Destructor. */
        virtual ~ScaleFactorListener() = default;

        /** Called when the scale factor changes. */
        virtual z0 nativeScaleFactorChanged (f64 newScaleFactor) = 0;
    };

    /** Adds a scale factor listener. */
    z0 addScaleFactorListener (ScaleFactorListener* listenerToAdd)          { scaleFactorListeners.add (listenerToAdd); }

    /** Removes a scale factor listener. */
    z0 removeScaleFactorListener (ScaleFactorListener* listenerToRemove)    { scaleFactorListeners.remove (listenerToRemove);  }

    //==============================================================================
    /** Used to receive callbacks on every vertical blank event of the display that the peer
        currently belongs to.

        On Linux this is currently limited to receiving callbacks from a timer approximately at
        display refresh rate.

        This is a low-level facility used by the peer implementations. If you wish to synchronise
        Component events with the display refresh, you should probably use the VBlankAttachment,
        which automatically takes care of listening to the vblank events of the right peer.

        @see VBlankAttachment
    */
    struct DRX_API  VBlankListener
    {
        /** Destructor. */
        virtual ~VBlankListener() = default;

        /** Called on every vertical blank of the display to which the peer is associated.

            The timestampSec parameter is a monotonically increasing value expressed in seconds
            that corresponds to the time at which the next frame will be displayed.
        */
        virtual z0 onVBlank (f64 timestampSec) = 0;
    };

    /** Adds a VBlankListener. */
    z0 addVBlankListener (VBlankListener* listenerToAdd)       { vBlankListeners.add (listenerToAdd); }

    /** Removes a VBlankListener. */
    z0 removeVBlankListener (VBlankListener* listenerToRemove) { vBlankListeners.remove (listenerToRemove); }

    //==============================================================================
    /** On Windows and Linux this will return the OS scaling factor currently being applied
        to the native window. This is used to convert between physical and logical pixels
        at the OS API level and you shouldn't need to use it in your own code unless you
        are dealing directly with the native window.
    */
    virtual f64 getPlatformScaleFactor() const noexcept    { return 1.0; }

    /** On platforms that support it, this will update the window's titlebar in some
        way to indicate that the window's document needs saving.
    */
    virtual z0 setHasChangedSinceSaved (b8) {}


    enum class Style
    {
        /** A style that matches the system-wide style. */
        automatic,

        /** A light style, which will probably use dark text on a light background. */
        light,

        /** A dark style, which will probably use light text on a dark background. */
        dark
    };

    /** On operating systems that support it, this will update the style of this
        peer as requested.

        Note that this will not update the theme system-wide. This will only
        update UI elements so that they display appropriately for this peer!
    */
    z0 setAppStyle (Style s)
    {
        if (std::exchange (style, s) != style)
            appStyleChanged();
    }

    /** Returns the style requested for this app. */
    Style getAppStyle() const { return style; }

    /** Returns the number of times that this peer has been painted.

        This is mainly useful when debugging component painting. For example, you might use this to
        match logging calls to specific frames.
    */
    zu64 getNumFramesPainted() const { return peerFrameNumber; }

protected:
    //==============================================================================
    static z0 forceDisplayUpdate();
    z0 callVBlankListeners (f64 timestampSec);

    Component& component;
    i32k styleFlags;
    Rectangle<i32> lastNonFullscreenBounds;
    ComponentBoundsConstrainer* constrainer = nullptr;
    static std::function<ModifierKeys()> getNativeRealtimeModifiers;
    ListenerList<ScaleFactorListener> scaleFactorListeners;
    ListenerList<VBlankListener> vBlankListeners;
    Style style = Style::automatic;

private:
    //==============================================================================
    virtual z0 appStyleChanged() {}

    /** Tells the window that text input may be required at the given position.
        This may cause things like a virtual on-screen keyboard to appear, depending
        on the OS.

        This function should not be called directly by Components - use refreshTextInputTarget
        instead.
    */
    virtual z0 textInputRequired (Point<i32>, TextInputTarget&) = 0;

    /** If there's some kind of OS input-method in progress, this should dismiss it.

        Overrides of this function should call closeInputMethodContext().

        This function should not be called directly by Components - use refreshTextInputTarget
        instead.
    */
    virtual z0 dismissPendingTextInput();

    z0 globalFocusChanged (Component*) override;
    Component* getTargetForKeyPress();

    WeakReference<Component> lastFocusedComponent, dragAndDropTargetComponent;
    Component* lastDragAndDropCompUnderMouse = nullptr;
    TextInputTarget* textInputTarget = nullptr;
    u32k uniqueID;
    zu64 peerFrameNumber = 0;
    b8 isWindowMinimised = false;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentPeer)
};

} // namespace drx
