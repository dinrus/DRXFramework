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
namespace XWindowSystemUtilities
{
    //==============================================================================
    /** A handy struct that uses XLockDisplay and XUnlockDisplay to lock the X server
        using RAII.

        @tags{GUI}
    */
    struct ScopedXLock
    {
        ScopedXLock();
        ~ScopedXLock();
    };

    //==============================================================================
    /** Gets a specified window property and stores its associated data, freeing it
        on deletion.

        @tags{GUI}
    */
    struct GetXProperty
    {
        GetXProperty (::Display* display, ::Window windowH, Atom property,
                      i64 offset, i64 length, b8 shouldDelete, Atom requestedType);
        ~GetXProperty();

        b8 success = false;
        u8* data = nullptr;
        u64 numItems = 0, bytesLeft = 0;
        Atom actualType;
        i32 actualFormat = -1;
    };

    //==============================================================================
    /** Initialises and stores some atoms for the display.

        @tags{GUI}
    */
    struct Atoms
    {
        enum ProtocolItems
        {
            TAKE_FOCUS = 0,
            DELETE_WINDOW = 1,
            PING = 2
        };

        Atoms() = default;
        explicit Atoms (::Display*);

        static Atom getIfExists (::Display*, tukk name);
        static Atom getCreating (::Display*, tukk name);

        static Txt getName (::Display*, Atom);
        static b8 isMimeTypeFile (::Display*, Atom);

        static constexpr u64 DndVersion = 3;

        Atom protocols, protocolList[3], changeState, state, userTime, activeWin, pid, windowType, windowState, windowStateHidden,
             XdndAware, XdndEnter, XdndLeave, XdndPosition, XdndStatus, XdndDrop, XdndFinished, XdndSelection,
             XdndTypeList, XdndActionList, XdndActionDescription, XdndActionCopy, XdndActionPrivate,
             XembedMsgType, XembedInfo, allowedActions[5], allowedMimeTypes[4], utf8String, clipboard, targets;
    };

    //==============================================================================
    /** Represents a setting according to the XSETTINGS specification.

        @tags{GUI}
    */
    struct XSetting
    {
        enum class Type
        {
            integer,
            string,
            colour,
            invalid
        };

        XSetting() = default;

        XSetting (const Txt& n, i32 v)            : name (n), type (Type::integer), integerValue (v)  {}
        XSetting (const Txt& n, const Txt& v)  : name (n), type (Type::string),  stringValue (v)   {}
        XSetting (const Txt& n, const Color& v)  : name (n), type (Type::colour),  colourValue (v)   {}

        b8 isValid() const noexcept  { return type != Type::invalid; }

        Txt name;
        Type type = Type::invalid;

        i32 integerValue = -1;
        Txt stringValue;
        Color colourValue;
    };

    /** Parses and stores the X11 settings for a display according to the XSETTINGS
        specification.

        @tags{GUI}
    */
    class XSettings
    {
    public:
        static std::unique_ptr<XSettings> createXSettings (::Display*);

        //==============================================================================
        z0 update();
        ::Window getSettingsWindow() const noexcept  { return settingsWindow; }

        XSetting getSetting (const Txt& settingName) const;

        //==============================================================================
        struct Listener
        {
            virtual ~Listener() = default;
            virtual z0 settingChanged (const XSetting& settingThatHasChanged) = 0;
        };

        z0 addListener (Listener* listenerToAdd)        { listeners.add (listenerToAdd); }
        z0 removeListener (Listener* listenerToRemove)  { listeners.remove (listenerToRemove); }

    private:
        ::Display* display = nullptr;
        ::Window settingsWindow = None;
        Atom settingsAtom;

        i32 lastUpdateSerial = -1;

        std::unordered_map<Txt, XSetting> settings;
        ListenerList<Listener> listeners;

        XSettings (::Display*, Atom, ::Window);

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XSettings)
    };
}

//==============================================================================
class LinuxComponentPeer;

class XWindowSystem  : public DeletedAtShutdown
{
public:
    //==============================================================================
    ::Window createWindow (::Window parentWindow, LinuxComponentPeer*) const;
    z0 destroyWindow    (::Window);

    z0 setTitle (::Window, const Txt&) const;
    z0 setIcon (::Window , const Image&) const;
    z0 setVisible (::Window, b8 shouldBeVisible) const;
    z0 setBounds (::Window, Rectangle<i32>, b8 fullScreen) const;
    z0 updateConstraints (::Window) const;

    ComponentPeer::OptionalBorderSize getBorderSize (::Window) const;
    Rectangle<i32> getWindowBounds (::Window, ::Window parentWindow);
    Point<i32> getPhysicalParentScreenPosition() const;

    b8 contains (::Window, Point<i32> localPos) const;

    z0 setMinimised (::Window, b8 shouldBeMinimised) const;
    b8 isMinimised  (::Window) const;

    z0 setMaximised (::Window, b8 shouldBeMinimised) const;

    z0 toFront  (::Window, b8 makeActive) const;
    z0 toBehind (::Window, ::Window otherWindow) const;

    b8 isFocused (::Window) const;
    b8 grabFocus (::Window) const;

    b8 canUseSemiTransparentWindows() const;
    b8 canUseARGBImages() const;
    b8 isDarkModeActive() const;

    i32 getNumPaintsPendingForWindow (::Window);
    z0 processPendingPaintsForWindow (::Window);
    z0 addPendingPaintForWindow (::Window);
    z0 removePendingPaintForWindow (::Window);

    Image createImage (b8 isSemiTransparentWindow, i32 width, i32 height, b8 argb) const;
    z0 blitToWindow (::Window, Image, Rectangle<i32> destinationRect, Rectangle<i32> totalRect) const;

    z0 setScreenSaverEnabled (b8 enabled) const;

    Point<f32> getCurrentMousePosition() const;
    z0 setMousePosition (Point<f32> pos) const;

    Cursor createCustomMouseCursorInfo (const Image&, Point<i32> hotspot) const;
    z0 deleteMouseCursor (Cursor cursorHandle) const;
    Cursor createStandardMouseCursor (MouseCursor::StandardCursorType) const;
    z0 showCursor (::Window, Cursor cursorHandle) const;

    b8 isKeyCurrentlyDown (i32 keyCode) const;
    ModifierKeys getNativeRealtimeModifiers() const;

    Array<Displays::Display> findDisplays (f32 masterScale) const;

    ::Window createKeyProxy (::Window);
    z0 deleteKeyProxy (::Window) const;

    b8 externalDragFileInit (LinuxComponentPeer*, const StringArray& files, b8 canMove, std::function<z0()>&& callback) const;
    b8 externalDragTextInit (LinuxComponentPeer*, const Txt& text, std::function<z0()>&& callback) const;

    z0 copyTextToClipboard (const Txt&);
    Txt getTextFromClipboard() const;
    Txt getLocalClipboardContent() const noexcept  { return localClipboardContent; }

    ::Display* getDisplay() const noexcept                            { return display; }
    const XWindowSystemUtilities::Atoms& getAtoms() const noexcept    { return atoms; }
    XWindowSystemUtilities::XSettings* getXSettings() const noexcept  { return xSettings.get(); }

    b8 isX11Available() const noexcept  { return xIsAvailable; }

    z0 startHostManagedResize (::Window window,
                                 ResizableBorderComponent::Zone zone);

    static Txt getWindowScalingFactorSettingName()  { return "Gdk/WindowScalingFactor"; }
    static Txt getThemeNameSettingName()            { return "Net/ThemeName"; }

    //==============================================================================
    z0 handleWindowMessage (LinuxComponentPeer*, XEvent&) const;
    b8 isParentWindowOf (::Window, ::Window possibleChild) const;

    //==============================================================================
    DRX_DECLARE_SINGLETON_INLINE (XWindowSystem, false)

private:
    XWindowSystem();
    ~XWindowSystem();

    //==============================================================================
    struct VisualAndDepth
    {
        Visual* visual;
        i32 depth;
    };

    struct DisplayVisuals
    {
        explicit DisplayVisuals (::Display*);

        VisualAndDepth getBestVisualForWindow (b8) const;
        b8 isValid() const noexcept;

        Visual* visual16Bit = nullptr;
        Visual* visual24Bit = nullptr;
        Visual* visual32Bit = nullptr;
    };

    b8 initialiseXDisplay();
    z0 destroyXDisplay();

    //==============================================================================
    ::Window getFocusWindow (::Window) const;

    b8 isFrontWindow (::Window) const;

    //==============================================================================
    z0 xchangeProperty (::Window, Atom, Atom, i32, ukk, i32) const;

    z0 removeWindowDecorations (::Window) const;
    z0 addWindowButtons        (::Window, i32) const;
    z0 setWindowType           (::Window, i32) const;

    z0 initialisePointerMap();
    z0 deleteIconPixmaps (::Window) const;
    z0 updateModifierMappings() const;

    i64 getUserTime (::Window) const;
    b8 isHidden (Window) const;
    b8 isIconic (Window) const;

    z0 initialiseXSettings();

    //==============================================================================
    z0 handleKeyPressEvent        (LinuxComponentPeer*, XKeyEvent&) const;
    z0 handleKeyReleaseEvent      (LinuxComponentPeer*, const XKeyEvent&) const;
    z0 handleWheelEvent           (LinuxComponentPeer*, const XButtonPressedEvent&, f32) const;
    z0 handleButtonPressEvent     (LinuxComponentPeer*, const XButtonPressedEvent&, i32) const;
    z0 handleButtonPressEvent     (LinuxComponentPeer*, const XButtonPressedEvent&) const;
    z0 handleButtonReleaseEvent   (LinuxComponentPeer*, const XButtonReleasedEvent&) const;
    z0 handleMotionNotifyEvent    (LinuxComponentPeer*, const XPointerMovedEvent&) const;
    z0 handleEnterNotifyEvent     (LinuxComponentPeer*, const XEnterWindowEvent&) const;
    z0 handleLeaveNotifyEvent     (LinuxComponentPeer*, const XLeaveWindowEvent&) const;
    z0 handleFocusInEvent         (LinuxComponentPeer*) const;
    z0 handleFocusOutEvent        (LinuxComponentPeer*) const;
    z0 handleExposeEvent          (LinuxComponentPeer*, XExposeEvent&) const;
    z0 handleConfigureNotifyEvent (LinuxComponentPeer*, XConfigureEvent&) const;
    z0 handleGravityNotify        (LinuxComponentPeer*) const;
    z0 propertyNotifyEvent        (LinuxComponentPeer*, const XPropertyEvent&) const;
    z0 handleMappingNotify        (XMappingEvent&) const;
    z0 handleClientMessageEvent   (LinuxComponentPeer*, XClientMessageEvent&, XEvent&) const;
    z0 handleXEmbedMessage        (LinuxComponentPeer*, XClientMessageEvent&) const;

    z0 dismissBlockingModals      (LinuxComponentPeer*) const;
    z0 dismissBlockingModals      (LinuxComponentPeer*, const XConfigureEvent&) const;
    z0 updateConstraints          (::Window, ComponentPeer&) const;

    ::Window findTopLevelWindowOf (::Window) const;

    static z0 windowMessageReceive (XEvent&);

    //==============================================================================
    b8 xIsAvailable = false;

    XWindowSystemUtilities::Atoms atoms;
    ::Display* display = nullptr;
    std::unique_ptr<DisplayVisuals> displayVisuals;
    std::unique_ptr<XWindowSystemUtilities::XSettings> xSettings;

   #if DRX_USE_XSHM
    std::map<::Window, i32> shmPaintsPendingMap;
   #endif

    i32 shmCompletionEvent = 0;
    i32 pointerMap[5] = {};
    Txt localClipboardContent;

    Point<i32> parentScreenPosition;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XWindowSystem)
};

} // namespace drx
