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

#if DRX_DEBUG && ! defined (DRX_DEBUG_XERRORS)
 #define DRX_DEBUG_XERRORS 1

 #if ! defined (DRX_DEBUG_XERRORS_SYNCHRONOUSLY)
  #define DRX_DEBUG_XERRORS_SYNCHRONOUSLY 0
 #endif
#endif

#if DRX_MODULE_AVAILABLE_drx_gui_extra
 #define DRX_X11_SUPPORTS_XEMBED 1
#else
 #define DRX_X11_SUPPORTS_XEMBED 0
#endif

namespace
{
    struct XFreeDeleter
    {
        z0 operator() (uk ptr) const
        {
            if (ptr != nullptr)
                X11Symbols::getInstance()->xFree (ptr);
        }
    };

    template <typename Data>
    std::unique_ptr<Data, XFreeDeleter> makeXFreePtr (Data* raw) { return std::unique_ptr<Data, XFreeDeleter> (raw); }

    template <typename Data, typename Deleter>
    std::unique_ptr<Data, Deleter> makeDeletedPtr (Data* raw, const Deleter& d) { return std::unique_ptr<Data, Deleter> (raw, d); }

    template <typename XValueType>
    struct XValueHolder
    {
        XValueHolder (XValueType&& xv, const std::function<z0(XValueType&)>& cleanup)
            : value (std::move (xv)), cleanupFunc (cleanup)
        {}

        ~XValueHolder()
        {
            cleanupFunc (value);
        }

        XValueType value;
        std::function<z0(XValueType&)> cleanupFunc;
    };
}

//==============================================================================
XWindowSystemUtilities::ScopedXLock::ScopedXLock()
{
    if (auto* xWindow = XWindowSystem::getInstanceWithoutCreating())
        if (auto* d = xWindow->getDisplay())
            X11Symbols::getInstance()->xLockDisplay (d);
}

XWindowSystemUtilities::ScopedXLock::~ScopedXLock()
{
    if (auto* xWindow = XWindowSystem::getInstanceWithoutCreating())
        if (auto* d = xWindow->getDisplay())
            X11Symbols::getInstance()->xUnlockDisplay (d);
}

//==============================================================================
XWindowSystemUtilities::Atoms::Atoms (::Display* display)
{
    protocols                    = getIfExists (display, "WM_PROTOCOLS");
    protocolList [TAKE_FOCUS]    = getIfExists (display, "WM_TAKE_FOCUS");
    protocolList [DELETE_WINDOW] = getIfExists (display, "WM_DELETE_WINDOW");
    protocolList [PING]          = getIfExists (display, "_NET_WM_PING");
    changeState                  = getIfExists (display, "WM_CHANGE_STATE");
    state                        = getIfExists (display, "WM_STATE");
    userTime                     = getCreating (display, "_NET_WM_USER_TIME");
    activeWin                    = getCreating (display, "_NET_ACTIVE_WINDOW");
    pid                          = getCreating (display, "_NET_WM_PID");
    windowType                   = getIfExists (display, "_NET_WM_WINDOW_TYPE");
    windowState                  = getIfExists (display, "_NET_WM_STATE");
    windowStateHidden            = getIfExists (display, "_NET_WM_STATE_HIDDEN");

    XdndAware                    = getCreating (display, "XdndAware");
    XdndEnter                    = getCreating (display, "XdndEnter");
    XdndLeave                    = getCreating (display, "XdndLeave");
    XdndPosition                 = getCreating (display, "XdndPosition");
    XdndStatus                   = getCreating (display, "XdndStatus");
    XdndDrop                     = getCreating (display, "XdndDrop");
    XdndFinished                 = getCreating (display, "XdndFinished");
    XdndSelection                = getCreating (display, "XdndSelection");

    XdndTypeList                 = getCreating (display, "XdndTypeList");
    XdndActionList               = getCreating (display, "XdndActionList");
    XdndActionCopy               = getCreating (display, "XdndActionCopy");
    XdndActionPrivate            = getCreating (display, "XdndActionPrivate");
    XdndActionDescription        = getCreating (display, "XdndActionDescription");

    XembedMsgType                = getCreating (display, "_XEMBED");
    XembedInfo                   = getCreating (display, "_XEMBED_INFO");

    allowedMimeTypes[0]          = getCreating (display, "UTF8_STRING");
    allowedMimeTypes[1]          = getCreating (display, "text/plain;charset=utf-8");
    allowedMimeTypes[2]          = getCreating (display, "text/plain");
    allowedMimeTypes[3]          = getCreating (display, "text/uri-list");

    allowedActions[0]            = getCreating (display, "XdndActionMove");
    allowedActions[1]            = XdndActionCopy;
    allowedActions[2]            = getCreating (display, "XdndActionLink");
    allowedActions[3]            = getCreating (display, "XdndActionAsk");
    allowedActions[4]            = XdndActionPrivate;

    utf8String                   = getCreating (display, "UTF8_STRING");
    clipboard                    = getCreating (display, "CLIPBOARD");
    targets                      = getCreating (display, "TARGETS");
}

Atom XWindowSystemUtilities::Atoms::getIfExists (::Display* display, tukk name)
{
    return X11Symbols::getInstance()->xInternAtom (display, name, True);
}

Atom XWindowSystemUtilities::Atoms::getCreating (::Display* display, tukk name)
{
    return X11Symbols::getInstance()->xInternAtom (display, name, False);
}

Txt XWindowSystemUtilities::Atoms::getName (::Display* display, Atom atom)
{
    if (atom == None)
        return "None";

    return makeXFreePtr (X11Symbols::getInstance()->xGetAtomName (display, atom)).get();
}

b8 XWindowSystemUtilities::Atoms::isMimeTypeFile (::Display* display, Atom atom)
{
    return getName (display, atom).equalsIgnoreCase ("text/uri-list");
}

//==============================================================================
XWindowSystemUtilities::GetXProperty::GetXProperty (::Display* display, Window window, Atom atom,
                                                    i64 offset, i64 length, b8 shouldDelete, Atom requestedType)
{
    success = (X11Symbols::getInstance()->xGetWindowProperty (display, window, atom, offset, length,
                                                              (Bool) shouldDelete, requestedType, &actualType,
                                                              &actualFormat, &numItems, &bytesLeft, &data) == Success)
                && data != nullptr;
}

XWindowSystemUtilities::GetXProperty::~GetXProperty()
{
    if (data != nullptr)
        X11Symbols::getInstance()->xFree (data);
}

//==============================================================================
std::unique_ptr<XWindowSystemUtilities::XSettings> XWindowSystemUtilities::XSettings::createXSettings (::Display* d)
{
    const auto settingsAtom = Atoms::getCreating (d, "_XSETTINGS_SETTINGS");
    const auto settingsWindow = X11Symbols::getInstance()->xGetSelectionOwner (d,
                                                                               Atoms::getCreating (d, "_XSETTINGS_S0"));

    if (settingsWindow == None)
        return {};

    return rawToUniquePtr (new XWindowSystemUtilities::XSettings (d, settingsWindow, settingsAtom));
}

XWindowSystemUtilities::XSettings::XSettings (::Display* d, ::Window settingsWindowIn, Atom settingsAtomIn)
    : display (d), settingsWindow (settingsWindowIn), settingsAtom (settingsAtomIn)
{
    update();
}

XWindowSystemUtilities::XSetting XWindowSystemUtilities::XSettings::getSetting (const Txt& name) const
{
    const auto iter = settings.find (name);

    if (iter != settings.end())
        return iter->second;

    return {};
}

z0 XWindowSystemUtilities::XSettings::update()
{
    const GetXProperty prop { display,
                              settingsWindow,
                              settingsAtom,
                              0L,
                              std::numeric_limits<i64>::max(),
                              false,
                              settingsAtom };

    if (prop.success
        && prop.actualType == settingsAtom
        && prop.actualFormat == 8
        && prop.numItems > 0)
    {
        const auto bytes = (size_t) prop.numItems;
        auto* data = prop.data;
        size_t byteNum = 0;

        const auto increment = [&] (size_t amount)
        {
            data    += amount;
            byteNum += amount;
        };

        struct Header
        {
            CARD8 byteOrder;
            CARD8 padding[3];
            CARD32 serial;
            CARD32 nSettings;
        };

        const auto* header = unalignedPointerCast<const Header*> (data);
        const auto headerSerial = (i32) header->serial;
        increment (sizeof (Header));

        const auto readCARD16 = [&]() -> CARD16
        {
            if (byteNum + sizeof (CARD16) > bytes)
                return {};

            const auto value = header->byteOrder == MSBFirst ? ByteOrder::bigEndianShort (data)
                                                             : ByteOrder::littleEndianShort (data);
            increment (sizeof (CARD16));
            return value;
        };

        const auto readCARD32 = [&]() -> CARD32
        {
            if (byteNum + sizeof (CARD32) > bytes)
                return {};

            const auto value = header->byteOrder == MSBFirst ? ByteOrder::bigEndianInt (data)
                                                             : ByteOrder::littleEndianInt (data);
            increment (sizeof (CARD32));
            return value;
        };

        const auto readString = [&] (size_t nameLen) -> Txt
        {
            const auto padded = (nameLen + 3) & (~(size_t) 3);

            if (byteNum + padded > bytes)
                return {};

            auto* ptr = reinterpret_cast<tukk> (data);
            const Txt result (ptr, nameLen);
            increment (padded);
            return result;
        };

        CARD16 setting = 0;

        while (byteNum < bytes && setting < header->nSettings)
        {
            const auto type = *reinterpret_cast<tukk> (data);
            increment (2);

            const auto name   = readString (readCARD16());
            const auto serial = (i32) readCARD32();

            enum { XSettingsTypeInteger, XSettingsTypeString, XSettingsTypeColor };

            const auto parsedSetting = [&]() -> XSetting
            {
                switch (type)
                {
                    case XSettingsTypeInteger:
                        return { name, (i32) readCARD32() };

                    case XSettingsTypeString:
                        return { name, readString (readCARD32()) };

                    case XSettingsTypeColor:
                        // Order is important, these should be kept as separate statements!
                        const auto r = (u8) readCARD16();
                        const auto g = (u8) readCARD16();
                        const auto b = (u8) readCARD16();
                        const auto a = (u8) readCARD16();
                        return { name, Color { r, g, b, a } };
                }

                return {};
            }();

            if (serial > lastUpdateSerial)
            {
                settings[parsedSetting.name] = parsedSetting;
                listeners.call ([&parsedSetting] (Listener& l) { l.settingChanged (parsedSetting); });
            }

            setting += 1;
        }

        lastUpdateSerial = headerSerial;
    }
}

//==============================================================================
::Window drx_messageWindowHandle;
XContext windowHandleXContext;

#if DRX_X11_SUPPORTS_XEMBED
 b8 drx_handleXEmbedEvent (ComponentPeer*, uk);
 u64 drx_getCurrentFocusWindow (ComponentPeer*);
#endif

struct MotifWmHints
{
    u64 flags       = 0;
    u64 functions   = 0;
    u64 decorations = 0;
    i64          input_mode  = 0;
    u64 status      = 0;
};

//=============================== X11 - Error Handling =========================
namespace X11ErrorHandling
{
    static XErrorHandler   oldErrorHandler   = {};
    static XIOErrorHandler oldIOErrorHandler = {};

    // Usually happens when client-server connection is broken
    static i32 ioErrorHandler (::Display*)
    {
        DBG ("ERROR: connection to X server broken.. terminating.");

        if (DRXApplicationBase::isStandaloneApp())
            MessageManager::getInstance()->stopDispatchLoop();

        return 0;
    }

    static i32 errorHandler ([[maybe_unused]] ::Display* display, [[maybe_unused]] XErrorEvent* event)
    {
       #if DRX_DEBUG_XERRORS
        t8 errorStr[64]   = { 0 };
        t8 requestStr[64] = { 0 };

        X11Symbols::getInstance()->xGetErrorText (display, event->error_code, errorStr, 64);
        X11Symbols::getInstance()->xGetErrorDatabaseText (display, "XRequest", Txt (event->request_code).toUTF8(), "Unknown", requestStr, 64);

        DBG ("ERROR: X returned " << errorStr << " for operation " << requestStr);
       #endif

        return 0;
    }

    static z0 installXErrorHandlers()
    {
        oldIOErrorHandler = X11Symbols::getInstance()->xSetIOErrorHandler (ioErrorHandler);
        oldErrorHandler   = X11Symbols::getInstance()->xSetErrorHandler   (errorHandler);
    }

    static z0 removeXErrorHandlers()
    {
        X11Symbols::getInstance()->xSetIOErrorHandler (oldIOErrorHandler);
        oldIOErrorHandler = {};

        X11Symbols::getInstance()->xSetErrorHandler (oldErrorHandler);
        oldErrorHandler = {};
    }
}

//=============================== X11 - Keys ===================================
namespace Keys
{
    enum MouseButtons
    {
        NoButton = 0,
        LeftButton = 1,
        MiddleButton = 2,
        RightButton = 3,
        WheelUp = 4,
        WheelDown = 5
    };

    static i32 AltMask = 0;
    static i32 NumLockMask = 0;
    static b8 numLock = false;
    static b8 capsLock = false;
    static t8 keyStates [32];
    static constexpr i32 extendedKeyModifier = 0x10000000;
    static b8 modifierKeysAreStale = false;

    static z0 refreshStaleModifierKeys()
    {
        if (modifierKeysAreStale)
        {
            XWindowSystem::getInstance()->getNativeRealtimeModifiers();
            modifierKeysAreStale = false;
        }
    }

    // Call this function when only the mouse keys need to be refreshed e.g. when the event
    // parameter already has information about the keys.
    static z0 refreshStaleMouseKeys()
    {
        if (modifierKeysAreStale)
        {
            const auto oldMods = ModifierKeys::currentModifiers;
            XWindowSystem::getInstance()->getNativeRealtimeModifiers();
            ModifierKeys::currentModifiers = oldMods.withoutMouseButtons()
                                                    .withFlags (ModifierKeys::currentModifiers.withOnlyMouseButtons()
                                                                                              .getRawFlags());
            modifierKeysAreStale = false;
        }
    }
}

i32k KeyPress::spaceKey              = XK_space         & 0xff;
i32k KeyPress::returnKey             = XK_Return        & 0xff;
i32k KeyPress::escapeKey             = XK_Escape        & 0xff;
i32k KeyPress::backspaceKey          = XK_BackSpace     & 0xff;
i32k KeyPress::leftKey               = (XK_Left         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::rightKey              = (XK_Right        & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::upKey                 = (XK_Up           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::downKey               = (XK_Down         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::pageUpKey             = (XK_Page_Up      & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::pageDownKey           = (XK_Page_Down    & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::endKey                = (XK_End          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::homeKey               = (XK_Home         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::insertKey             = (XK_Insert       & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::deleteKey             = (XK_Delete       & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::tabKey                = XK_Tab           & 0xff;
i32k KeyPress::F1Key                 = (XK_F1           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F2Key                 = (XK_F2           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F3Key                 = (XK_F3           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F4Key                 = (XK_F4           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F5Key                 = (XK_F5           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F6Key                 = (XK_F6           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F7Key                 = (XK_F7           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F8Key                 = (XK_F8           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F9Key                 = (XK_F9           & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F10Key                = (XK_F10          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F11Key                = (XK_F11          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F12Key                = (XK_F12          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F13Key                = (XK_F13          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F14Key                = (XK_F14          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F15Key                = (XK_F15          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F16Key                = (XK_F16          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F17Key                = (XK_F17          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F18Key                = (XK_F18          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F19Key                = (XK_F19          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F20Key                = (XK_F20          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F21Key                = (XK_F21          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F22Key                = (XK_F22          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F23Key                = (XK_F23          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F24Key                = (XK_F24          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F25Key                = (XK_F25          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F26Key                = (XK_F26          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F27Key                = (XK_F27          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F28Key                = (XK_F28          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F29Key                = (XK_F29          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F30Key                = (XK_F30          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F31Key                = (XK_F31          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F32Key                = (XK_F32          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F33Key                = (XK_F33          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F34Key                = (XK_F34          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::F35Key                = (XK_F35          & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad0            = (XK_KP_0         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad1            = (XK_KP_1         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad2            = (XK_KP_2         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad3            = (XK_KP_3         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad4            = (XK_KP_4         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad5            = (XK_KP_5         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad6            = (XK_KP_6         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad7            = (XK_KP_7         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad8            = (XK_KP_8         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPad9            = (XK_KP_9         & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPadAdd          = (XK_KP_Add       & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPadSubtract     = (XK_KP_Subtract  & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPadMultiply     = (XK_KP_Multiply  & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPadDivide       = (XK_KP_Divide    & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPadSeparator    = (XK_KP_Separator & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPadDecimalPoint = (XK_KP_Decimal   & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPadEquals       = (XK_KP_Equal     & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::numberPadDelete       = (XK_KP_Delete    & 0xff) | Keys::extendedKeyModifier;
i32k KeyPress::playKey               = ((i32) 0xffeeff00)       | Keys::extendedKeyModifier;
i32k KeyPress::stopKey               = ((i32) 0xffeeff01)       | Keys::extendedKeyModifier;
i32k KeyPress::fastForwardKey        = ((i32) 0xffeeff02)       | Keys::extendedKeyModifier;
i32k KeyPress::rewindKey             = ((i32) 0xffeeff03)       | Keys::extendedKeyModifier;

static z0 updateKeyStates (i32 keycode, b8 press) noexcept
{
    auto keybyte = keycode >> 3;
    auto keybit = (1 << (keycode & 7));

    if (press)
        Keys::keyStates [keybyte] |= keybit;
    else
        Keys::keyStates [keybyte] &= ~keybit;
}

static z0 updateKeyModifiers (i32 status) noexcept
{
    i32 keyMods = 0;

    if ((status & ShiftMask)     != 0) keyMods |= ModifierKeys::shiftModifier;
    if ((status & ControlMask)   != 0) keyMods |= ModifierKeys::ctrlModifier;
    if ((status & Keys::AltMask) != 0) keyMods |= ModifierKeys::altModifier;

    ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withOnlyMouseButtons().withFlags (keyMods);

    Keys::numLock  = ((status & Keys::NumLockMask) != 0);
    Keys::capsLock = ((status & LockMask)          != 0);
}

static b8 updateKeyModifiersFromSym (KeySym sym, b8 press) noexcept
{
    i32 modifier = 0;
    b8 isModifier = true;

    switch (sym)
    {
        case XK_Shift_L:
        case XK_Shift_R:   modifier = ModifierKeys::shiftModifier; break;

        case XK_Control_L:
        case XK_Control_R: modifier = ModifierKeys::ctrlModifier; break;

        case XK_Alt_L:
        case XK_Alt_R:     modifier = ModifierKeys::altModifier; break;

        case XK_Num_Lock:
            if (press)
                Keys::numLock = ! Keys::numLock;

            break;

        case XK_Caps_Lock:
            if (press)
                Keys::capsLock = ! Keys::capsLock;

            break;

        case XK_Scroll_Lock:
            break;

        default:
            isModifier = false;
            break;
    }

    ModifierKeys::currentModifiers = press ? ModifierKeys::currentModifiers.withFlags (modifier)
                                           : ModifierKeys::currentModifiers.withoutFlags (modifier);

    return isModifier;
}

enum
{
    KeyPressEventType = 2
};

//================================== X11 - Shm =================================
#if DRX_USE_XSHM
 namespace XSHMHelpers
 {
     static i32 trappedErrorCode = 0;

     extern "C" i32 errorTrapHandler (Display*, XErrorEvent* err);
     extern "C" i32 errorTrapHandler (Display*, XErrorEvent* err)
     {
         trappedErrorCode = err->error_code;
         return 0;
     }

     static b8 isShmAvailable (::Display* display)
     {
         static b8 isChecked = false;
         static b8 isAvailable = false;

         if (! isChecked)
         {
             isChecked = true;

             if (display != nullptr)
             {
                 i32 major, minor;
                 Bool pixmaps;

                 XWindowSystemUtilities::ScopedXLock xLock;

                 if (X11Symbols::getInstance()->xShmQueryVersion (display, &major, &minor, &pixmaps))
                 {
                     trappedErrorCode = 0;
                     auto oldHandler = X11Symbols::getInstance()->xSetErrorHandler (errorTrapHandler);

                     XShmSegmentInfo segmentInfo;
                     zerostruct (segmentInfo);

                     if (auto* xImage = X11Symbols::getInstance()->xShmCreateImage (display,
                                                                                    X11Symbols::getInstance()->xDefaultVisual (display, X11Symbols::getInstance()->xDefaultScreen (display)),
                                                                                    24, ZPixmap, nullptr, &segmentInfo, 50, 50))
                     {
                         if ((segmentInfo.shmid = shmget (IPC_PRIVATE,
                                                          (size_t) (xImage->bytes_per_line * xImage->height),
                                                          IPC_CREAT | 0777)) >= 0)
                         {
                             segmentInfo.shmaddr = (tuk) shmat (segmentInfo.shmid, nullptr, 0);

                             if (segmentInfo.shmaddr != (uk) -1)
                             {
                                 segmentInfo.readOnly = False;
                                 xImage->data = segmentInfo.shmaddr;
                                 X11Symbols::getInstance()->xSync (display, False);

                                 if (X11Symbols::getInstance()->xShmAttach (display, &segmentInfo) != 0)
                                 {
                                     X11Symbols::getInstance()->xShmDetach (display, &segmentInfo);
                                     X11Symbols::getInstance()->xSync (display, False);

                                     isAvailable = true;
                                 }
                             }

                             X11Symbols::getInstance()->xFlush (display);
                             X11Symbols::getInstance()->xDestroyImage (xImage);

                             shmdt (segmentInfo.shmaddr);
                         }

                         shmctl (segmentInfo.shmid, IPC_RMID, nullptr);

                         X11Symbols::getInstance()->xSetErrorHandler (oldHandler);

                         if (trappedErrorCode != 0)
                             isAvailable = false;
                     }
                 }
             }
         }

         return isAvailable;
     }
 }
#endif

//=============================== X11 - Render =================================
#if DRX_USE_XRENDER
 namespace XRender
 {
     static b8 isAvailable (::Display* display)
     {
         i32 major, minor;
         return X11Symbols::getInstance()->xRenderQueryVersion (display, &major, &minor);
     }

     static b8 hasCompositingWindowManager (::Display* display)
     {
         return display != nullptr
                 && X11Symbols::getInstance()->xGetSelectionOwner (display,
                                                                   XWindowSystemUtilities::Atoms::getCreating (display, "_NET_WM_CM_S0")) != 0;
     }

     static XRenderPictFormat* findPictureFormat (::Display* display)
     {
         XWindowSystemUtilities::ScopedXLock xLock;

         if (isAvailable (display))
         {
             if (auto* pictFormat = X11Symbols::getInstance()->xRenderFindStandardFormat (display, PictStandardARGB32))
             {
                 XRenderPictFormat desiredFormat;
                 desiredFormat.type = PictTypeDirect;
                 desiredFormat.depth = 32;

                 desiredFormat.direct.alphaMask = 0xff;
                 desiredFormat.direct.redMask   = 0xff;
                 desiredFormat.direct.greenMask = 0xff;
                 desiredFormat.direct.blueMask  = 0xff;

                 desiredFormat.direct.alpha = 24;
                 desiredFormat.direct.red   = 16;
                 desiredFormat.direct.green = 8;
                 desiredFormat.direct.blue  = 0;

                 pictFormat = X11Symbols::getInstance()->xRenderFindFormat (display,
                                                                            PictFormatType | PictFormatDepth
                                                                             | PictFormatRedMask | PictFormatRed
                                                                             | PictFormatGreenMask | PictFormatGreen
                                                                             | PictFormatBlueMask | PictFormatBlue
                                                                             | PictFormatAlphaMask | PictFormatAlpha,
                                                                            &desiredFormat,
                                                                            0);

                 return pictFormat;
             }
         }

         return nullptr;
     }
 }
#endif

//================================ X11 - Visuals ===============================
namespace Visuals
{
    static Visual* findVisualWithDepth (::Display* display, i32 desiredDepth)
    {
        XWindowSystemUtilities::ScopedXLock xLock;

        Visual* visual = nullptr;
        i32 numVisuals = 0;
        auto desiredMask = VisualNoMask;
        XVisualInfo desiredVisual;

        desiredVisual.screen = X11Symbols::getInstance()->xDefaultScreen (display);
        desiredVisual.depth = desiredDepth;

        desiredMask = VisualScreenMask | VisualDepthMask;

        if (desiredDepth == 32)
        {
            desiredVisual.c_class    = TrueColor;
            desiredVisual.red_mask   = 0x00FF0000;
            desiredVisual.green_mask = 0x0000FF00;
            desiredVisual.blue_mask  = 0x000000FF;
            desiredVisual.bits_per_rgb = 8;

            desiredMask |= VisualClassMask;
            desiredMask |= VisualRedMaskMask;
            desiredMask |= VisualGreenMaskMask;
            desiredMask |= VisualBlueMaskMask;
            desiredMask |= VisualBitsPerRGBMask;
        }

        if (auto xvinfos = makeXFreePtr (X11Symbols::getInstance()->xGetVisualInfo (display, desiredMask, &desiredVisual, &numVisuals)))
        {
            for (i32 i = 0; i < numVisuals; i++)
            {
                if (xvinfos.get()[i].depth == desiredDepth)
                {
                    visual = xvinfos.get()[i].visual;
                    break;
                }
            }
        }

        return visual;
    }

    static Visual* findVisualFormat (::Display* display, i32 desiredDepth, i32& matchedDepth)
    {
        Visual* visual = nullptr;

        if (desiredDepth == 32)
        {
           #if DRX_USE_XSHM
            if (XSHMHelpers::isShmAvailable (display))
            {
               #if DRX_USE_XRENDER
                if (XRender::isAvailable (display))
                {
                    if (XRender::findPictureFormat (display) != nullptr)
                    {
                        i32 numVisuals = 0;
                        XVisualInfo desiredVisual;
                        desiredVisual.screen = X11Symbols::getInstance()->xDefaultScreen (display);
                        desiredVisual.depth = 32;
                        desiredVisual.bits_per_rgb = 8;

                        if (auto xvinfos = makeXFreePtr (X11Symbols::getInstance()->xGetVisualInfo (display,
                                                                                                    VisualScreenMask | VisualDepthMask | VisualBitsPerRGBMask,
                                                                                                    &desiredVisual, &numVisuals)))
                        {
                            for (i32 i = 0; i < numVisuals; ++i)
                            {
                                auto pictVisualFormat = X11Symbols::getInstance()->xRenderFindVisualFormat (display, xvinfos.get()[i].visual);

                                if (pictVisualFormat != nullptr
                                     && pictVisualFormat->type == PictTypeDirect
                                     && pictVisualFormat->direct.alphaMask)
                                {
                                    visual = xvinfos.get()[i].visual;
                                    matchedDepth = 32;
                                    break;
                                }
                            }
                        }
                    }
                }
               #endif
                if (visual == nullptr)
                {
                    visual = findVisualWithDepth (display, 32);

                    if (visual != nullptr)
                        matchedDepth = 32;
                }
            }
           #endif
        }

        if (visual == nullptr && desiredDepth >= 24)
        {
            visual = findVisualWithDepth (display, 24);

            if (visual != nullptr)
                matchedDepth = 24;
        }

        if (visual == nullptr && desiredDepth >= 16)
        {
            visual = findVisualWithDepth (display, 16);

            if (visual != nullptr)
                matchedDepth = 16;
        }

        return visual;
    }
}

//================================= X11 - Bitmap ===============================
class XBitmapImage final : public ImagePixelData
{
public:
    explicit XBitmapImage (XImage* image)
        : ImagePixelData (image->depth == 24 ? Image::RGB : Image::ARGB, image->width, image->height),
          xImage (image),
          imageDepth ((u32) xImage->depth)
    {
        pixelStride = xImage->bits_per_pixel / 8;
        lineStride = xImage->bytes_per_line;
        imageData = reinterpret_cast<u8*> (xImage->data);
    }

    XBitmapImage (Image::PixelFormat format, i32 w, i32 h,
                  b8 clearImage, u32 imageDepth_, Visual* visual)
        : ImagePixelData (format, w, h),
          imageDepth (imageDepth_)
    {
        jassert (format == Image::RGB || format == Image::ARGB);

        pixelStride = (format == Image::RGB) ? 3 : 4;
        lineStride = ((w * pixelStride + 3) & ~3);

        XWindowSystemUtilities::ScopedXLock xLock;

       #if DRX_USE_XSHM
        usingXShm = false;

        if ((imageDepth > 16) && XSHMHelpers::isShmAvailable (display))
        {
            zerostruct (segmentInfo);

            segmentInfo.shmid = -1;
            segmentInfo.shmaddr = (t8 *) -1;
            segmentInfo.readOnly = False;

            xImage.reset (X11Symbols::getInstance()->xShmCreateImage (display, visual, imageDepth, ZPixmap, nullptr,
                                                                      &segmentInfo, (u32) w, (u32) h));

            if (xImage != nullptr)
            {
                if ((segmentInfo.shmid = shmget (IPC_PRIVATE,
                                                 (size_t) (xImage->bytes_per_line * xImage->height),
                                                 IPC_CREAT | 0777)) >= 0)
                {
                    if (segmentInfo.shmid != -1)
                    {
                        segmentInfo.shmaddr = (tuk) shmat (segmentInfo.shmid, nullptr, 0);

                        if (segmentInfo.shmaddr != (uk) -1)
                        {
                            segmentInfo.readOnly = False;

                            xImage->data = segmentInfo.shmaddr;
                            imageData = (u8*) segmentInfo.shmaddr;

                            if (X11Symbols::getInstance()->xShmAttach (display, &segmentInfo) != 0)
                                usingXShm = true;
                            else
                                jassertfalse;
                        }
                        else
                        {
                            shmctl (segmentInfo.shmid, IPC_RMID, nullptr);
                        }
                    }
                }
            }
        }

        if (! isUsingXShm())
       #endif
        {
            imageDataAllocated.allocate ((size_t) (lineStride * h), format == Image::ARGB && clearImage);
            imageData = imageDataAllocated;

            xImage.reset ((XImage*) ::calloc (1, sizeof (XImage)));

            xImage->width = w;
            xImage->height = h;
            xImage->xoffset = 0;
            xImage->format = ZPixmap;
            xImage->data = (tuk) imageData;
            xImage->byte_order = X11Symbols::getInstance()->xImageByteOrder (display);
            xImage->bitmap_unit = X11Symbols::getInstance()->xBitmapUnit (display);
            xImage->bitmap_bit_order = X11Symbols::getInstance()->xBitmapBitOrder (display);
            xImage->bitmap_pad = 32;
            xImage->depth = pixelStride * 8;
            xImage->bytes_per_line = lineStride;
            xImage->bits_per_pixel = pixelStride * 8;
            xImage->red_mask   = 0x00FF0000;
            xImage->green_mask = 0x0000FF00;
            xImage->blue_mask  = 0x000000FF;

            if (imageDepth == 16)
            {
                i32 pixStride = 2;
                auto stride = ((w * pixStride + 3) & ~3);

                imageData16Bit.malloc (stride * h);
                xImage->data = imageData16Bit;
                xImage->bitmap_pad = 16;
                xImage->depth = pixStride * 8;
                xImage->bytes_per_line = stride;
                xImage->bits_per_pixel = pixStride * 8;
                xImage->red_mask   = visual->red_mask;
                xImage->green_mask = visual->green_mask;
                xImage->blue_mask  = visual->blue_mask;
            }

            if (! X11Symbols::getInstance()->xInitImage (xImage.get()))
                jassertfalse;
        }
    }

    ~XBitmapImage() override
    {
        XWindowSystemUtilities::ScopedXLock xLock;

        if (gc != None)
            X11Symbols::getInstance()->xFreeGC (display, gc);

       #if DRX_USE_XSHM
        if (isUsingXShm())
        {
            X11Symbols::getInstance()->xShmDetach (display, &segmentInfo);

            X11Symbols::getInstance()->xFlush (display);

            shmdt (segmentInfo.shmaddr);
            shmctl (segmentInfo.shmid, IPC_RMID, nullptr);
        }
        else
       #endif
        {
            xImage->data = nullptr;
        }
    }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        sendDataChangeMessage();
        return std::make_unique<LowLevelGraphicsSoftwareRenderer> (Image (this));
    }

    z0 initialiseBitmapData (Image::BitmapData& bitmap, i32 x, i32 y,
                               Image::BitmapData::ReadWriteMode mode) override
    {
        const auto offset = (size_t) (x * pixelStride + y * lineStride);
        bitmap.data = imageData + offset;
        bitmap.size = (size_t) (lineStride * height) - offset;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    ImagePixelData::Ptr clone() override
    {
        jassertfalse;
        return nullptr;
    }

    std::unique_ptr<ImageType> createType() const override     { return std::make_unique<NativeImageType>(); }

    z0 blitToWindow (::Window window, i32 dx, i32 dy, u32 dw, u32 dh, i32 sx, i32 sy)
    {
        XWindowSystemUtilities::ScopedXLock xLock;

       #if DRX_USE_XSHM
        if (isUsingXShm())
            XWindowSystem::getInstance()->addPendingPaintForWindow (window);
       #endif

        if (gc == None)
        {
            XGCValues gcvalues;
            gcvalues.foreground = None;
            gcvalues.background = None;
            gcvalues.function = GXcopy;
            gcvalues.plane_mask = AllPlanes;
            gcvalues.clip_mask = None;
            gcvalues.graphics_exposures = False;

            gc = X11Symbols::getInstance()->xCreateGC (display, window,
                                                       GCBackground | GCForeground | GCFunction | GCPlaneMask | GCClipMask | GCGraphicsExposures,
                                                       &gcvalues);
        }

        if (imageDepth == 16)
        {
            auto rMask   = (u32) xImage->red_mask;
            auto gMask   = (u32) xImage->green_mask;
            auto bMask   = (u32) xImage->blue_mask;
            auto rShiftL = (u32) jmax (0,  getShiftNeeded (rMask));
            auto rShiftR = (u32) jmax (0, -getShiftNeeded (rMask));
            auto gShiftL = (u32) jmax (0,  getShiftNeeded (gMask));
            auto gShiftR = (u32) jmax (0, -getShiftNeeded (gMask));
            auto bShiftL = (u32) jmax (0,  getShiftNeeded (bMask));
            auto bShiftR = (u32) jmax (0, -getShiftNeeded (bMask));

            Image::BitmapData srcData (Image (this), Image::BitmapData::readOnly);

            for (i32 y = sy; y < sy + (i32) dh; ++y)
            {
                auto* p = srcData.getPixelPointer (sx, y);

                for (i32 x = sx; x < sx + (i32) dw; ++x)
                {
                    auto* pixel = (PixelRGB*) p;
                    p += srcData.pixelStride;

                    X11Symbols::getInstance()->xPutPixel (xImage.get(), x, y,
                                                              (((((u32) pixel->getRed())   << rShiftL) >> rShiftR) & rMask)
                                                            | (((((u32) pixel->getGreen()) << gShiftL) >> gShiftR) & gMask)
                                                            | (((((u32) pixel->getBlue())  << bShiftL) >> bShiftR) & bMask));
                }
            }
        }

        // blit results to screen.
       #if DRX_USE_XSHM
        if (isUsingXShm())
            X11Symbols::getInstance()->xShmPutImage (display, (::Drawable) window, gc, xImage.get(), sx, sy, dx, dy, dw, dh, True);
        else
       #endif
            X11Symbols::getInstance()->xPutImage (display, (::Drawable) window, gc, xImage.get(), sx, sy, dx, dy, dw, dh);
    }

    #if DRX_USE_XSHM
     b8 isUsingXShm() const noexcept       { return usingXShm; }
    #endif

private:
    //==============================================================================
    struct Deleter
    {
        z0 operator() (XImage* img) const noexcept
        {
            X11Symbols::getInstance()->xDestroyImage (img);
        }
    };

    std::unique_ptr<XImage, Deleter> xImage;
    u32k imageDepth;
    HeapBlock<u8> imageDataAllocated;
    HeapBlock<t8> imageData16Bit;
    i32 pixelStride, lineStride;
    u8* imageData = nullptr;
    GC gc = None;
    ::Display* display = XWindowSystem::getInstance()->getDisplay();

   #if DRX_USE_XSHM
    XShmSegmentInfo segmentInfo;
    b8 usingXShm;
   #endif

    static i32 getShiftNeeded (u32k mask) noexcept
    {
        for (i32 i = 32; --i >= 0;)
            if (((mask >> i) & 1) != 0)
                return i - 7;

        jassertfalse;
        return 0;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XBitmapImage)
};

//=============================== X11 - Displays ===============================
namespace DisplayHelpers
{
    static f64 getDisplayDPI (::Display* display, i32 index)
    {
        auto widthMM  = X11Symbols::getInstance()->xDisplayWidthMM  (display, index);
        auto heightMM = X11Symbols::getInstance()->xDisplayHeightMM (display, index);

        if (widthMM > 0 && heightMM > 0)
            return (((X11Symbols::getInstance()->xDisplayWidth (display, index) * 25.4) / widthMM)
                    + ((X11Symbols::getInstance()->xDisplayHeight (display, index) * 25.4) / heightMM)) / 2.0;

        return 96.0;
    }

    static f64 getDisplayScale (const Txt& name, f64 dpi)
    {
        if (auto* xSettings = XWindowSystem::getInstance()->getXSettings())
        {
            auto windowScalingFactorSetting = xSettings->getSetting (XWindowSystem::getWindowScalingFactorSettingName());

            if (windowScalingFactorSetting.isValid()
                && windowScalingFactorSetting.integerValue > 0)
            {
                return (f64) windowScalingFactorSetting.integerValue;
            }
        }

        if (name.isNotEmpty())
        {
            // Ubuntu and derived distributions now save a per-display scale factor as a configuration
            // variable. This can be changed in the Monitor system settings panel.
            ChildProcess dconf;

            if (File ("/usr/bin/dconf").existsAsFile()
                && dconf.start ("/usr/bin/dconf read /com/ubuntu/user-interface/scale-factor", ChildProcess::wantStdOut))
            {
                if (dconf.waitForProcessToFinish (200))
                {
                    auto jsonOutput = dconf.readAllProcessOutput().replaceCharacter ('\'', '"');

                    if (dconf.getExitCode() == 0 && jsonOutput.isNotEmpty())
                    {
                        auto jsonVar = JSON::parse (jsonOutput);

                        if (auto* object = jsonVar.getDynamicObject())
                        {
                            auto scaleFactorVar = object->getProperty (name);

                            if (! scaleFactorVar.isVoid())
                            {
                                auto scaleFactor = ((f64) scaleFactorVar) / 8.0;

                                if (scaleFactor > 0.0)
                                    return scaleFactor;
                            }
                        }
                    }
                }
            }
        }

        {
            // Other gnome based distros now use gsettings for a global scale factor
            ChildProcess gsettings;

            if (File ("/usr/bin/gsettings").existsAsFile()
                && gsettings.start ("/usr/bin/gsettings get org.gnome.desktop.interface scaling-factor", ChildProcess::wantStdOut))
            {
                if (gsettings.waitForProcessToFinish (200))
                {
                    auto gsettingsOutput = StringArray::fromTokens (gsettings.readAllProcessOutput(), true);

                    if (gsettingsOutput.size() >= 2 && gsettingsOutput[1].length() > 0)
                    {
                        auto scaleFactor = gsettingsOutput[1].getDoubleValue();

                        if (scaleFactor > 0.0)
                            return scaleFactor;

                        return 1.0;
                    }
                }
            }
        }

        // If no scale factor is set by GNOME or Ubuntu then calculate from monitor dpi
        // We use the same approach as chromium which simply divides the dpi by 96
        // and then rounds the result
        return round (dpi / 96.0);
    }

   #if DRX_USE_XINERAMA
    static Array<XineramaScreenInfo> xineramaQueryDisplays (::Display* display)
    {
        i32 major_opcode, first_event, first_error;

        if (X11Symbols::getInstance()->xQueryExtension (display, "XINERAMA", &major_opcode, &first_event, &first_error)
            && (X11Symbols::getInstance()->xineramaIsActive (display) != 0))
        {
            i32 numScreens;

            if (auto xinfo = makeXFreePtr (X11Symbols::getInstance()->xineramaQueryScreens (display, &numScreens)))
                return { xinfo.get(), numScreens };
        }

        return {};
    }
   #endif
}

//=============================== X11 - Pixmap =================================
namespace PixmapHelpers
{
    static Pixmap createColorPixmapFromImage (::Display* display, const Image& image)
    {
        XWindowSystemUtilities::ScopedXLock xLock;

        auto width  = (u32) image.getWidth();
        auto height = (u32) image.getHeight();
        HeapBlock<u32> colour (width * height);
        i32 index = 0;

        for (i32 y = 0; y < (i32) height; ++y)
            for (i32 x = 0; x < (i32) width; ++x)
                colour[index++] = image.getPixelAt (x, y).getARGB();

        auto ximage = makeXFreePtr (X11Symbols::getInstance()->xCreateImage (display, (Visual*) CopyFromParent, 24, ZPixmap,
                                                                             0, reinterpret_cast<tukk> (colour.getData()),
                                                                             width, height, 32, 0));

        auto pixmap = X11Symbols::getInstance()->xCreatePixmap (display,
                                                                X11Symbols::getInstance()->xDefaultRootWindow (display),
                                                                width, height, 24);

        XValueHolder<GC> gc (X11Symbols::getInstance()->xCreateGC (display, pixmap, 0, nullptr),
                             [&display] (GC& g) { X11Symbols::getInstance()->xFreeGC (display, g); });
        X11Symbols::getInstance()->xPutImage (display, pixmap, gc.value, ximage.get(), 0, 0, 0, 0, width, height);

        return pixmap;
    }

    static Pixmap createMaskPixmapFromImage (::Display* display, const Image& image)
    {
        XWindowSystemUtilities::ScopedXLock xLock;

        auto width  = (u32) image.getWidth();
        auto height = (u32) image.getHeight();
        auto stride = (width + 7) >> 3;
        HeapBlock<t8> mask;
        mask.calloc (stride * height);

        auto msbfirst = (X11Symbols::getInstance()->xBitmapBitOrder (display) == MSBFirst);

        for (u32 y = 0; y < height; ++y)
        {
            for (u32 x = 0; x < width; ++x)
            {
                auto bit = (t8) (1 << (msbfirst ? (7 - (x & 7)) : (x & 7)));
                auto offset = y * stride + (x >> 3);

                if (image.getPixelAt ((i32) x, (i32) y).getAlpha() >= 128)
                    mask[offset] |= bit;
            }
        }

        return X11Symbols::getInstance()->xCreatePixmapFromBitmapData (display, X11Symbols::getInstance()->xDefaultRootWindow (display),
                                                                       mask.getData(), width, height, 1, 0, 1);
    }
}

//=============================== X11 - Clipboard ==============================
namespace ClipboardHelpers
{
    //==============================================================================
    // Read the content of a window property as either a locale-dependent string or an utf8 string
    // works only for strings shorter than 1000000 bytes
    static Txt readWindowProperty (::Display* display, Window window, Atom atom)
    {
        if (display != nullptr)
        {
            XWindowSystemUtilities::GetXProperty prop (display, window, atom, 0L, 100000, false, AnyPropertyType);

            if (prop.success)
            {
                if (prop.actualType == XWindowSystem::getInstance()->getAtoms().utf8String && prop.actualFormat == 8)
                    return Txt::fromUTF8 ((tukk) prop.data, (i32) prop.numItems);

                if (prop.actualType == XA_STRING && prop.actualFormat == 8)
                    return Txt ((tukk) prop.data, prop.numItems);
            }
        }

        return {};
    }

    //==============================================================================
    // Send a SelectionRequest to the window owning the selection and waits for its answer (with a timeout) */
    static b8 requestSelectionContent (::Display* display, Txt& selectionContent, Atom selection, Atom requestedFormat)
    {
        auto property_name = X11Symbols::getInstance()->xInternAtom (display, "DRX_SEL", false);

        // The selection owner will be asked to set the DRX_SEL property on the
        // drx_messageWindowHandle with the selection content
        X11Symbols::getInstance()->xConvertSelection (display, selection, requestedFormat, property_name,
                                                      drx_messageWindowHandle, CurrentTime);

        i32 count = 50; // will wait at most for 200 ms

        while (--count >= 0)
        {
            XEvent event;

            if (X11Symbols::getInstance()->xCheckTypedWindowEvent (display, drx_messageWindowHandle, SelectionNotify, &event))
            {
                if (event.xselection.property == property_name)
                {
                    jassert (event.xselection.requestor == drx_messageWindowHandle);

                    selectionContent = readWindowProperty (display, event.xselection.requestor, event.xselection.property);
                    return true;
                }

                return false;  // the format we asked for was denied.. (event.xselection.property == None)
            }

            // not very elegant.. we could do a select() or something like that...
            // however clipboard content requesting is inherently slow on x11, it
            // often takes 50ms or more so...
            Thread::sleep (4);
        }

        return false;
    }

    //==============================================================================
    // Called from the event loop in drx_Messaging_linux in response to SelectionRequest events
    static z0 handleSelection (XSelectionRequestEvent& evt)
    {
        // the selection content is sent to the target window as a window property
        XSelectionEvent reply;
        reply.type = SelectionNotify;
        reply.display = evt.display;
        reply.requestor = evt.requestor;
        reply.selection = evt.selection;
        reply.target = evt.target;
        reply.property = None; // == "fail"
        reply.time = evt.time;

        HeapBlock<t8> data;
        i32 propertyFormat = 0;
        size_t numDataItems = 0;

        const auto& atoms = XWindowSystem::getInstance()->getAtoms();

        if (evt.selection == XA_PRIMARY || evt.selection == atoms.clipboard)
        {
            if (evt.target == XA_STRING || evt.target == atoms.utf8String)
            {
                auto localContent = XWindowSystem::getInstance()->getLocalClipboardContent();

                // Translate to utf8
                numDataItems = localContent.getNumBytesAsUTF8();
                auto numBytesRequiredToStore = numDataItems + 1;
                data.calloc (numBytesRequiredToStore);
                localContent.copyToUTF8 (data, numBytesRequiredToStore);
                propertyFormat = 8;   // bits per item
            }
            else if (evt.target == atoms.targets)
            {
                // Another application wants to know what we are able to send

                numDataItems = 2;
                data.calloc (numDataItems * sizeof (Atom));

                // Atoms are flagged as 32-bit irrespective of sizeof (Atom)
                propertyFormat = 32;

                auto* dataAtoms = unalignedPointerCast<Atom*> (data.getData());

                dataAtoms[0] = atoms.utf8String;
                dataAtoms[1] = XA_STRING;

                evt.target = XA_ATOM;
            }
        }
        else
        {
            DBG ("requested unsupported clipboard");
        }

        if (data != nullptr)
        {
            const size_t maxReasonableSelectionSize = 1000000;

            // for very big chunks of data, we should use the "INCR" protocol , which is a pain in the *ss
            if (evt.property != None && numDataItems < maxReasonableSelectionSize)
            {
                X11Symbols::getInstance()->xChangeProperty (evt.display, evt.requestor,
                                                            evt.property, evt.target,
                                                            propertyFormat, PropModeReplace,
                                                            reinterpret_cast<u8k*> (data.getData()), (i32) numDataItems);
                reply.property = evt.property; // " == success"
            }
        }

        X11Symbols::getInstance()->xSendEvent (evt.display, evt.requestor, 0, NoEventMask, (XEvent*) &reply);
    }
}

//==============================================================================
ComponentPeer* getPeerFor (::Window windowH)
{
    if (windowH == 0)
        return nullptr;

    if (auto* display = XWindowSystem::getInstance()->getDisplay())
    {
        XWindowSystemUtilities::ScopedXLock xLock;

        if (XPointer peer = nullptr;
            X11Symbols::getInstance()->xFindContext (display,
                                                     static_cast<XID> (windowH),
                                                     windowHandleXContext,
                                                     &peer) == 0)
        {
            return unalignedPointerCast<ComponentPeer*> (peer);
        }
    }

    return nullptr;
}

//==============================================================================
static std::unordered_map<LinuxComponentPeer*, X11DragState> dragAndDropStateMap;

XWindowSystem::XWindowSystem()
{
    xIsAvailable = X11Symbols::getInstance()->loadAllSymbols();

    if (! xIsAvailable)
        return;

    if (DRXApplicationBase::isStandaloneApp())
    {
        // Initialise xlib for multiple thread support
        static b8 initThreadCalled = false;

        if (! initThreadCalled)
        {
            if (! X11Symbols::getInstance()->xInitThreads())
            {
                // This is fatal!  Print error and closedown
                Logger::outputDebugString ("Failed to initialise xlib thread support.");
                Process::terminate();

                return;
            }

            initThreadCalled = true;
        }

        X11ErrorHandling::installXErrorHandlers();
    }

    if (! initialiseXDisplay())
    {
        if (DRXApplicationBase::isStandaloneApp())
            X11ErrorHandling::removeXErrorHandlers();

        X11Symbols::deleteInstance();
        xIsAvailable = false;
    }
}

XWindowSystem::~XWindowSystem()
{
    if (xIsAvailable)
    {
        destroyXDisplay();

        if (DRXApplicationBase::isStandaloneApp())
            X11ErrorHandling::removeXErrorHandlers();
    }

    X11Symbols::deleteInstance();
    clearSingletonInstance();
}

//==============================================================================
static i32 getAllEventsMask (b8 ignoresMouseClicks)
{
    return NoEventMask | KeyPressMask | KeyReleaseMask
             | EnterWindowMask | LeaveWindowMask | PointerMotionMask | KeymapStateMask
             | ExposureMask | StructureNotifyMask | FocusChangeMask | PropertyChangeMask
             | (ignoresMouseClicks ? 0 : (ButtonPressMask | ButtonReleaseMask));
}

::Window XWindowSystem::createWindow (::Window parentToAddTo, LinuxComponentPeer* peer) const
{
    if (! xIsAvailable)
    {
        // can't open a window on a system that doesn't have X11 installed!
        jassertfalse;
        return 0;
    }

    auto styleFlags = peer->getStyleFlags();

    XWindowSystemUtilities::ScopedXLock xLock;

    auto root = X11Symbols::getInstance()->xRootWindow (display, X11Symbols::getInstance()->xDefaultScreen (display));

    auto visualAndDepth = displayVisuals->getBestVisualForWindow ((styleFlags & ComponentPeer::windowIsSemiTransparent) != 0);

    auto colormap = X11Symbols::getInstance()->xCreateColormap (display, root, visualAndDepth.visual, AllocNone);
    X11Symbols::getInstance()->xInstallColormap (display, colormap);

    // Set up the window attributes
    XSetWindowAttributes swa;
    swa.border_pixel = 0;
    swa.background_pixmap = None;
    swa.colormap = colormap;
    swa.override_redirect = ((styleFlags & ComponentPeer::windowIsTemporary) != 0) ? True : False;
    swa.event_mask = getAllEventsMask (styleFlags & ComponentPeer::windowIgnoresMouseClicks);

    auto windowH = X11Symbols::getInstance()->xCreateWindow (display, parentToAddTo != 0 ? parentToAddTo : root,
                                                             0, 0, 1, 1,
                                                             0, visualAndDepth.depth, InputOutput, visualAndDepth.visual,
                                                             CWBorderPixel | CWColormap | CWBackPixmap | CWEventMask | CWOverrideRedirect,
                                                             &swa);

    // Set the window context to identify the window handle object
    if (! peer->setWindowAssociation (windowH))
    {
        // Failed
        jassertfalse;

        Logger::outputDebugString ("Failed to create context information for window.\n");
        X11Symbols::getInstance()->xDestroyWindow (display, windowH);

        return 0;
    }

    // Set window manager hints
    if (auto wmHints = makeXFreePtr (X11Symbols::getInstance()->xAllocWMHints()))
    {
        wmHints->flags = InputHint | StateHint;
        wmHints->input = True;
        wmHints->initial_state = NormalState;
        X11Symbols::getInstance()->xSetWMHints (display, windowH, wmHints.get());
    }

    // Set class hint
    if (auto* app = DRXApplicationBase::getInstance())
    {
        if (auto classHint = makeXFreePtr (X11Symbols::getInstance()->xAllocClassHint()))
        {
            auto appName = app->getApplicationName();
            classHint->res_name  = (tuk) appName.getCharPointer().getAddress();
            classHint->res_class = (tuk) appName.getCharPointer().getAddress();

            X11Symbols::getInstance()->xSetClassHint (display, windowH, classHint.get());
        }
    }

    // Set the window type
    setWindowType (windowH, styleFlags);

    // Define decoration
    if ((styleFlags & ComponentPeer::windowHasTitleBar) == 0)
        removeWindowDecorations (windowH);
    else
        addWindowButtons (windowH, styleFlags);

    // Associate the PID, allowing to be shut down when something goes wrong
    auto pid = (u64) getpid();
    xchangeProperty (windowH, atoms.pid, XA_CARDINAL, 32, &pid, 1);

    // Set window manager protocols
    xchangeProperty (windowH, atoms.protocols, XA_ATOM, 32, atoms.protocolList, 2);

    // Set drag and drop flags
    xchangeProperty (windowH, atoms.XdndTypeList, XA_ATOM, 32, atoms.allowedMimeTypes, numElementsInArray (atoms.allowedMimeTypes));
    xchangeProperty (windowH, atoms.XdndActionList, XA_ATOM, 32, atoms.allowedActions, numElementsInArray (atoms.allowedActions));
    xchangeProperty (windowH, atoms.XdndActionDescription, XA_STRING, 8, "", 0);

    auto dndVersion = XWindowSystemUtilities::Atoms::DndVersion;
    xchangeProperty (windowH, atoms.XdndAware, XA_ATOM, 32, &dndVersion, 1);

    u64 info[2] = { 0, 1 };
    xchangeProperty (windowH, atoms.XembedInfo, atoms.XembedInfo, 32, (u8*) info, 2);

    return windowH;
}

z0 XWindowSystem::destroyWindow (::Window windowH)
{
    auto* peer = dynamic_cast<LinuxComponentPeer*> (getPeerFor (windowH));

    if (peer == nullptr)
    {
        jassertfalse;
        return;
    }

   #if DRX_X11_SUPPORTS_XEMBED
    drx_handleXEmbedEvent (peer, nullptr);
   #endif

    deleteIconPixmaps (windowH);
    dragAndDropStateMap.erase (peer);

    XWindowSystemUtilities::ScopedXLock xLock;

    peer->clearWindowAssociation();

    X11Symbols::getInstance()->xDestroyWindow (display, windowH);

    // Wait for it to complete and then remove any events for this
    // window from the event queue.
    X11Symbols::getInstance()->xSync (display, false);

    XEvent event;
    while (X11Symbols::getInstance()->xCheckWindowEvent (display, windowH,
                                                         getAllEventsMask (peer->getStyleFlags() & ComponentPeer::windowIgnoresMouseClicks),
                                                         &event) == True)
    {}

   #if DRX_USE_XSHM
    if (XSHMHelpers::isShmAvailable (display))
        shmPaintsPendingMap.erase (windowH);
   #endif
}

//==============================================================================
z0 XWindowSystem::setTitle (::Window windowH, const Txt& title) const
{
    jassert (windowH != 0);

    XTextProperty nameProperty{};
    tuk strings[] = { const_cast<tuk> (title.toRawUTF8()) };

    XWindowSystemUtilities::ScopedXLock xLock;

    if (X11Symbols::getInstance()->xutf8TextListToTextProperty (display,
                                                                strings,
                                                                numElementsInArray (strings),
                                                                XUTF8StringStyle,
                                                                &nameProperty) >= 0)
    {
        X11Symbols::getInstance()->xSetWMName (display, windowH, &nameProperty);
        X11Symbols::getInstance()->xSetWMIconName (display, windowH, &nameProperty);

        X11Symbols::getInstance()->xFree (nameProperty.value);
    }
}


z0 XWindowSystem::setIcon (::Window windowH, const Image& newIcon) const
{
    jassert (windowH != 0);

    auto dataSize = newIcon.getWidth() * newIcon.getHeight() + 2;
    HeapBlock<u64> data (dataSize);

    i32 index = 0;
    data[index++] = (u64) newIcon.getWidth();
    data[index++] = (u64) newIcon.getHeight();

    for (i32 y = 0; y < newIcon.getHeight(); ++y)
        for (i32 x = 0; x < newIcon.getWidth(); ++x)
            data[index++] = (u64) newIcon.getPixelAt (x, y).getARGB();

    XWindowSystemUtilities::ScopedXLock xLock;
    xchangeProperty (windowH, XWindowSystemUtilities::Atoms::getCreating (display, "_NET_WM_ICON"),
                     XA_CARDINAL, 32, data.getData(), dataSize);

    deleteIconPixmaps (windowH);

    auto wmHints = makeXFreePtr (X11Symbols::getInstance()->xGetWMHints (display, windowH));

    if (wmHints == nullptr)
        wmHints = makeXFreePtr (X11Symbols::getInstance()->xAllocWMHints());

    if (wmHints != nullptr)
    {
        wmHints->flags |= IconPixmapHint | IconMaskHint;
        wmHints->icon_pixmap = PixmapHelpers::createColorPixmapFromImage (display, newIcon);
        wmHints->icon_mask = PixmapHelpers::createMaskPixmapFromImage (display, newIcon);

        X11Symbols::getInstance()->xSetWMHints (display, windowH, wmHints.get());
    }

    X11Symbols::getInstance()->xSync (display, False);
}

z0 XWindowSystem::setVisible (::Window windowH, b8 shouldBeVisible) const
{
    jassert (windowH != 0);

    XWindowSystemUtilities::ScopedXLock xLock;

    if (shouldBeVisible)
        X11Symbols::getInstance()->xMapWindow (display, windowH);
    else
        X11Symbols::getInstance()->xUnmapWindow (display, windowH);
}

z0 XWindowSystem::setBounds (::Window windowH, Rectangle<i32> newBounds, b8 isFullScreen) const
{
    jassert (windowH != 0);

    if (auto* peer = getPeerFor (windowH))
    {
        if (peer->isFullScreen() && ! isFullScreen)
        {
            // When transitioning back from fullscreen, we might need to remove
            // the FULLSCREEN window property
            Atom fs = XWindowSystemUtilities::Atoms::getIfExists (display, "_NET_WM_STATE_FULLSCREEN");

            if (fs != None)
            {
                auto root = X11Symbols::getInstance()->xRootWindow (display, X11Symbols::getInstance()->xDefaultScreen (display));

                XClientMessageEvent clientMsg;
                clientMsg.display = display;
                clientMsg.window = windowH;
                clientMsg.type = ClientMessage;
                clientMsg.format = 32;
                clientMsg.message_type = atoms.windowState;
                clientMsg.data.l[0] = 0;  // Remove
                clientMsg.data.l[1] = (i64) fs;
                clientMsg.data.l[2] = 0;
                clientMsg.data.l[3] = 1;  // Normal Source

                XWindowSystemUtilities::ScopedXLock xLock;
                X11Symbols::getInstance()->xSendEvent (display, root, false,
                                                       SubstructureRedirectMask | SubstructureNotifyMask,
                                                       (XEvent*) &clientMsg);
            }
        }

        updateConstraints (windowH, *peer);

        XWindowSystemUtilities::ScopedXLock xLock;

        if (auto hints = makeXFreePtr (X11Symbols::getInstance()->xAllocSizeHints()))
        {
            hints->flags  = USSize | USPosition;
            hints->x      = newBounds.getX();
            hints->y      = newBounds.getY();
            hints->width  = newBounds.getWidth();
            hints->height = newBounds.getHeight();
            X11Symbols::getInstance()->xSetWMNormalHints (display, windowH, hints.get());
        }

        const auto nativeWindowBorder = [&]() -> BorderSize<i32>
        {
            if (const auto& frameSize = peer->getFrameSizeIfPresent())
                return frameSize->multipliedBy (peer->getPlatformScaleFactor());

            return {};
        }();

        X11Symbols::getInstance()->xMoveResizeWindow (display, windowH,
                                                      newBounds.getX() - nativeWindowBorder.getLeft(),
                                                      newBounds.getY() - nativeWindowBorder.getTop(),
                                                      (u32) newBounds.getWidth(),
                                                      (u32) newBounds.getHeight());
    }
}

z0 XWindowSystem::startHostManagedResize (::Window windowH,
                                            ResizableBorderComponent::Zone zone)
{
    const auto moveResize = XWindowSystemUtilities::Atoms::getIfExists (display, "_NET_WM_MOVERESIZE");

    if (moveResize == None)
        return;

    XWindowSystemUtilities::ScopedXLock xLock;

    X11Symbols::getInstance()->xUngrabPointer (display, CurrentTime);

    const auto root = X11Symbols::getInstance()->xRootWindow (display, X11Symbols::getInstance()->xDefaultScreen (display));
    const auto mouseDown = getCurrentMousePosition();

    XClientMessageEvent clientMsg;
    clientMsg.display = display;
    clientMsg.window = windowH;
    clientMsg.type = ClientMessage;
    clientMsg.format = 32;
    clientMsg.message_type = moveResize;
    clientMsg.data.l[0] = (i64) mouseDown.x;
    clientMsg.data.l[1] = (i64) mouseDown.y;
    clientMsg.data.l[2] = [&]
    {
        // It's unclear which header is supposed to contain these
        static constexpr auto _NET_WM_MOVERESIZE_SIZE_TOPLEFT      = 0;
        static constexpr auto _NET_WM_MOVERESIZE_SIZE_TOP          = 1;
        static constexpr auto _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     = 2;
        static constexpr auto _NET_WM_MOVERESIZE_SIZE_RIGHT        = 3;
        static constexpr auto _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  = 4;
        static constexpr auto _NET_WM_MOVERESIZE_SIZE_BOTTOM       = 5;
        static constexpr auto _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   = 6;
        static constexpr auto _NET_WM_MOVERESIZE_SIZE_LEFT         = 7;
        static constexpr auto _NET_WM_MOVERESIZE_MOVE              = 8;

        using F = ResizableBorderComponent::Zone::Zones;

        switch (zone.getZoneFlags())
        {
            case F::top | F::left:      return _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
            case F::top:                return _NET_WM_MOVERESIZE_SIZE_TOP;
            case F::top | F::right:     return _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
            case F::right:              return _NET_WM_MOVERESIZE_SIZE_RIGHT;
            case F::bottom | F::right:  return _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
            case F::bottom:             return _NET_WM_MOVERESIZE_SIZE_BOTTOM;
            case F::bottom | F::left:   return _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
            case F::left:               return _NET_WM_MOVERESIZE_SIZE_LEFT;
        }

        return _NET_WM_MOVERESIZE_MOVE;
    }();
    clientMsg.data.l[3] = 0;
    clientMsg.data.l[4] = 1;

    X11Symbols::getInstance()->xSendEvent (display,
                                           root,
                                           false,
                                           SubstructureRedirectMask | SubstructureNotifyMask,
                                           unalignedPointerCast<XEvent*> (&clientMsg));
}

z0 XWindowSystem::updateConstraints (::Window windowH) const
{
    if (auto* peer = getPeerFor (windowH))
        updateConstraints (windowH, *peer);
}

z0 XWindowSystem::updateConstraints (::Window windowH, ComponentPeer& peer) const
{
    XWindowSystemUtilities::ScopedXLock xLock;

    if (auto hints = makeXFreePtr (X11Symbols::getInstance()->xAllocSizeHints()))
    {
        if ((peer.getStyleFlags() & ComponentPeer::windowIsResizable) == 0)
        {
            hints->min_width  = hints->max_width  = peer.getBounds().getWidth();
            hints->min_height = hints->max_height = peer.getBounds().getHeight();
            hints->flags = PMinSize | PMaxSize;
        }
        else if (auto* c = peer.getConstrainer())
        {
            const auto windowBorder = [&]() -> BorderSize<i32>
            {
                if (const auto& frameSize = peer.getFrameSizeIfPresent())
                    return *frameSize;

                return {};
            }();

            const auto factor       = peer.getPlatformScaleFactor();
            const auto leftAndRight = windowBorder.getLeftAndRight();
            const auto topAndBottom = windowBorder.getTopAndBottom();
            hints->min_width  = jmax (1, (i32) (factor * c->getMinimumWidth())  - leftAndRight);
            hints->max_width  = jmax (1, (i32) (factor * c->getMaximumWidth())  - leftAndRight);
            hints->min_height = jmax (1, (i32) (factor * c->getMinimumHeight()) - topAndBottom);
            hints->max_height = jmax (1, (i32) (factor * c->getMaximumHeight()) - topAndBottom);
            hints->flags = PMinSize | PMaxSize;
        }

        X11Symbols::getInstance()->xSetWMNormalHints (display, windowH, hints.get());
    }
}

b8 XWindowSystem::contains (::Window windowH, Point<i32> localPos) const
{
    ::Window root, child;
    i32 wx, wy;
    u32 ww, wh, bw, bitDepth;

    XWindowSystemUtilities::ScopedXLock xLock;

    return X11Symbols::getInstance()->xGetGeometry (display, (::Drawable) windowH, &root, &wx, &wy, &ww, &wh, &bw, &bitDepth)
          && X11Symbols::getInstance()->xTranslateCoordinates (display, windowH, windowH, localPos.getX(), localPos.getY(), &wx, &wy, &child)
          && child == None;
}

ComponentPeer::OptionalBorderSize XWindowSystem::getBorderSize (::Window windowH) const
{
    jassert (windowH != 0);

    XWindowSystemUtilities::ScopedXLock xLock;
    auto hints = XWindowSystemUtilities::Atoms::getIfExists (display, "_NET_FRAME_EXTENTS");

    if (hints != None)
    {
        XWindowSystemUtilities::GetXProperty prop (display, windowH, hints, 0, 4, false, XA_CARDINAL);

        if (prop.success && prop.actualFormat == 32)
        {
            auto data = prop.data;
            std::array<u64, 4> sizes;

            for (auto& size : sizes)
            {
                memcpy (&size, data, sizeof (u64));
                data += sizeof (u64);
            }

            return ComponentPeer::OptionalBorderSize ({ (i32) sizes[2], (i32) sizes[0], (i32) sizes[3], (i32) sizes[1] });
        }
    }

    return {};
}

Rectangle<i32> XWindowSystem::getWindowBounds (::Window windowH, ::Window parentWindow)
{
    jassert (windowH != 0);

    Window root, child;
    i32 wx = 0, wy = 0;
    u32 ww = 0, wh = 0, bw, bitDepth;

    XWindowSystemUtilities::ScopedXLock xLock;

    if (X11Symbols::getInstance()->xGetGeometry (display, (::Drawable) windowH, &root, &wx, &wy, &ww, &wh, &bw, &bitDepth))
    {
        i32 rootX = 0, rootY = 0;

        if (! X11Symbols::getInstance()->xTranslateCoordinates (display, windowH, root, 0, 0, &rootX, &rootY, &child))
            rootX = rootY = 0;

        if (parentWindow == 0)
        {
            wx = rootX;
            wy = rootY;
        }
        else
        {
            // XGetGeometry returns wx and wy relative to the parent window's origin.
            // XTranslateCoordinates returns rootX and rootY relative to the root window.
            parentScreenPosition = Point<i32> (rootX - wx, rootY - wy);
        }
    }

    return { wx, wy, (i32) ww, (i32) wh };
}

Point<i32> XWindowSystem::getPhysicalParentScreenPosition() const
{
    return parentScreenPosition;
}

z0 XWindowSystem::setMinimised (::Window windowH, b8 shouldBeMinimised) const
{
    jassert (windowH != 0);

    if (shouldBeMinimised)
    {
        auto root = X11Symbols::getInstance()->xRootWindow (display, X11Symbols::getInstance()->xDefaultScreen (display));

        XClientMessageEvent clientMsg;
        clientMsg.display = display;
        clientMsg.window = windowH;
        clientMsg.type = ClientMessage;
        clientMsg.format = 32;
        clientMsg.message_type = atoms.changeState;
        clientMsg.data.l[0] = IconicState;

        XWindowSystemUtilities::ScopedXLock xLock;
        X11Symbols::getInstance()->xSendEvent (display, root, false, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*) &clientMsg);
    }
}

b8 XWindowSystem::isMinimised (::Window w) const
{
    return isHidden (w);
}

z0 XWindowSystem::setMaximised (::Window windowH, b8 shouldBeMaximised) const
{
    const auto root = X11Symbols::getInstance()->xRootWindow (display, X11Symbols::getInstance()->xDefaultScreen (display));

    XEvent ev;
    ev.xclient.window = windowH;
    ev.xclient.type   = ClientMessage;
    ev.xclient.format = 32;
    ev.xclient.message_type = XWindowSystemUtilities::Atoms::getCreating (display, "_NET_WM_STATE");
    ev.xclient.data.l[0] = shouldBeMaximised;
    ev.xclient.data.l[1] = (i64) XWindowSystemUtilities::Atoms::getCreating (display, "_NET_WM_STATE_MAXIMIZED_HORZ");
    ev.xclient.data.l[2] = (i64) XWindowSystemUtilities::Atoms::getCreating (display, "_NET_WM_STATE_MAXIMIZED_VERT");
    ev.xclient.data.l[3] = 1;
    ev.xclient.data.l[4] = 0;

    XWindowSystemUtilities::ScopedXLock xLock;
    X11Symbols::getInstance()->xSendEvent (display, root, false, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

z0 XWindowSystem::toFront (::Window windowH, b8) const
{
    jassert (windowH != 0);

    XWindowSystemUtilities::ScopedXLock xLock;
    XEvent ev;
    ev.xclient.type = ClientMessage;
    ev.xclient.serial = 0;
    ev.xclient.send_event = True;
    ev.xclient.message_type = atoms.activeWin;
    ev.xclient.window = windowH;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = 2;
    ev.xclient.data.l[1] = getUserTime (windowH);
    ev.xclient.data.l[2] = 0;
    ev.xclient.data.l[3] = 0;
    ev.xclient.data.l[4] = 0;

    X11Symbols::getInstance()->xSendEvent (display, X11Symbols::getInstance()->xRootWindow (display, X11Symbols::getInstance()->xDefaultScreen (display)),
                                           False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);

    X11Symbols::getInstance()->xSync (display, False);
}

z0 XWindowSystem::toBehind (::Window windowH, ::Window otherWindow) const
{
    jassert (windowH != 0 && otherWindow != 0);

    const auto topLevelA = findTopLevelWindowOf (windowH);
    const auto topLevelB = findTopLevelWindowOf (otherWindow);

    Window newStack[] = { topLevelA, topLevelB };

    XWindowSystemUtilities::ScopedXLock xLock;
    X11Symbols::getInstance()->xRestackWindows (display, newStack, numElementsInArray (newStack));
}

b8 XWindowSystem::isFocused (::Window windowH) const
{
    jassert (windowH != 0);

    i32 revert = 0;
    Window focusedWindow = 0;
    XWindowSystemUtilities::ScopedXLock xLock;
    X11Symbols::getInstance()->xGetInputFocus (display, &focusedWindow, &revert);

    if (focusedWindow == PointerRoot)
        return false;

    return isParentWindowOf (windowH, focusedWindow);
}

::Window XWindowSystem::getFocusWindow (::Window windowH) const
{
    jassert (windowH != 0);

   #if DRX_X11_SUPPORTS_XEMBED
    if (auto w = (::Window) drx_getCurrentFocusWindow (dynamic_cast<LinuxComponentPeer*> (getPeerFor (windowH))))
        return w;
   #endif

    return windowH;
}

b8 XWindowSystem::grabFocus (::Window windowH) const
{
    jassert (windowH != 0);

    XWindowAttributes atts;
    XWindowSystemUtilities::ScopedXLock xLock;

    if (windowH != 0
        && X11Symbols::getInstance()->xGetWindowAttributes (display, windowH, &atts)
        && atts.map_state == IsViewable
        && ! isFocused (windowH))
    {
        X11Symbols::getInstance()->xSetInputFocus (display, getFocusWindow (windowH), RevertToParent, (::Time) getUserTime (windowH));
        return true;
    }

    return false;
}

b8 XWindowSystem::canUseSemiTransparentWindows() const
{
   #if DRX_USE_XRENDER
    if (XRender::hasCompositingWindowManager (display))
    {
        i32 matchedDepth = 0, desiredDepth = 32;

        return Visuals::findVisualFormat (display, desiredDepth, matchedDepth) != nullptr
                && matchedDepth == desiredDepth;
    }
   #endif

    return false;
}

b8 XWindowSystem::canUseARGBImages() const
{
    static b8 canUseARGB = false;

   #if DRX_USE_XSHM
    static b8 checked = false;

    if (! checked)
    {
        if (XSHMHelpers::isShmAvailable (display))
        {
            XWindowSystemUtilities::ScopedXLock xLock;
            XShmSegmentInfo segmentinfo;

            auto testImage = X11Symbols::getInstance()->xShmCreateImage (display,
                                                                         X11Symbols::getInstance()->xDefaultVisual (display, X11Symbols::getInstance()->xDefaultScreen (display)),
                                                                         24, ZPixmap, nullptr, &segmentinfo, 64, 64);

            canUseARGB = testImage != nullptr && testImage->bits_per_pixel == 32;
            X11Symbols::getInstance()->xDestroyImage (testImage);
        }
        else
        {
            canUseARGB = false;
        }

        checked = true;
    }
   #endif

    return canUseARGB;
}

b8 XWindowSystem::isDarkModeActive() const
{
    const auto themeName = [this]() -> Txt
    {
        if (xSettings != nullptr)
        {
            const auto themeNameSetting = xSettings->getSetting (getThemeNameSettingName());

            if (themeNameSetting.isValid()
                && themeNameSetting.stringValue.isNotEmpty())
            {
                return themeNameSetting.stringValue;
            }
        }

        ChildProcess gsettings;

        if (File ("/usr/bin/gsettings").existsAsFile()
            && gsettings.start ("/usr/bin/gsettings get org.gnome.desktop.interface gtk-theme", ChildProcess::wantStdOut))
        {
            if (gsettings.waitForProcessToFinish (200))
                return gsettings.readAllProcessOutput();
        }

        return {};
    }();

    return (themeName.isNotEmpty()
          && (themeName.containsIgnoreCase ("dark") || themeName.containsIgnoreCase ("black")));
}

Image XWindowSystem::createImage (b8 isSemiTransparent, i32 width, i32 height, b8 argb) const
{
    auto visualAndDepth = displayVisuals->getBestVisualForWindow (isSemiTransparent);

   #if DRX_USE_XSHM
    return Image (new XBitmapImage (argb ? Image::ARGB : Image::RGB,
   #else
    return Image (new XBitmapImage (Image::RGB,
   #endif
                                    (width + 31) & ~31,
                                    (height + 31) & ~31,
                                    false, (u32) visualAndDepth.depth, visualAndDepth.visual));
}

z0 XWindowSystem::blitToWindow (::Window windowH, Image image, Rectangle<i32> destinationRect, Rectangle<i32> totalRect) const
{
    jassert (windowH != 0);

    auto* xbitmap = static_cast<XBitmapImage*> (image.getPixelData().get());

    xbitmap->blitToWindow (windowH,
                           destinationRect.getX(), destinationRect.getY(),
                           (u32) destinationRect.getWidth(),
                           (u32) destinationRect.getHeight(),
                           destinationRect.getX() - totalRect.getX(), destinationRect.getY() - totalRect.getY());
}

z0 XWindowSystem::processPendingPaintsForWindow (::Window windowH)
{
   #if DRX_USE_XSHM
    if (! XSHMHelpers::isShmAvailable (display))
        return;

    if (getNumPaintsPendingForWindow (windowH) > 0)
    {
        XWindowSystemUtilities::ScopedXLock xLock;

        XEvent evt;
        while (X11Symbols::getInstance()->xCheckTypedWindowEvent (display, windowH, shmCompletionEvent, &evt))
            removePendingPaintForWindow (windowH);
    }
   #endif
}

i32 XWindowSystem::getNumPaintsPendingForWindow (::Window windowH)
{
   #if DRX_USE_XSHM
    if (XSHMHelpers::isShmAvailable (display))
        return shmPaintsPendingMap[windowH];
   #endif

    return 0;
}

z0 XWindowSystem::addPendingPaintForWindow (::Window windowH)
{
   #if DRX_USE_XSHM
    if (XSHMHelpers::isShmAvailable (display))
        ++shmPaintsPendingMap[windowH];
   #endif
}

z0 XWindowSystem::removePendingPaintForWindow (::Window windowH)
{
   #if DRX_USE_XSHM
    if (XSHMHelpers::isShmAvailable (display))
        --shmPaintsPendingMap[windowH];
   #endif
}

z0 XWindowSystem::setScreenSaverEnabled (b8 enabled) const
{
    using tXScreenSaverSuspend = z0 (*) (Display*, Bool);
    static tXScreenSaverSuspend xScreenSaverSuspend = nullptr;

    if (xScreenSaverSuspend == nullptr)
        if (uk h = dlopen ("libXss.so.1", RTLD_GLOBAL | RTLD_NOW))
            xScreenSaverSuspend = (tXScreenSaverSuspend) dlsym (h, "XScreenSaverSuspend");

    XWindowSystemUtilities::ScopedXLock xLock;

    NullCheckedInvocation::invoke (xScreenSaverSuspend, display, ! enabled);
}

Point<f32> XWindowSystem::getCurrentMousePosition() const
{
    Window root, child;
    i32 x, y, winx, winy;
    u32 mask;

    XWindowSystemUtilities::ScopedXLock xLock;

    if (X11Symbols::getInstance()->xQueryPointer (display,
                                                  X11Symbols::getInstance()->xRootWindow (display,
                                                                                          X11Symbols::getInstance()->xDefaultScreen (display)),
                                                  &root, &child,
                                                  &x, &y, &winx, &winy, &mask) == False)
    {
        x = y = -1;
    }

    return { (f32) x, (f32) y };
}

z0 XWindowSystem::setMousePosition (Point<f32> pos) const
{
    XWindowSystemUtilities::ScopedXLock xLock;

    auto root = X11Symbols::getInstance()->xRootWindow (display,
                                                        X11Symbols::getInstance()->xDefaultScreen (display));

    X11Symbols::getInstance()->xWarpPointer (display, None, root, 0, 0, 0, 0,
                                             roundToInt (pos.getX()), roundToInt (pos.getY()));
}

Cursor XWindowSystem::createCustomMouseCursorInfo (const Image& image, Point<i32> hotspot) const
{
    if (display == nullptr)
        return {};

    XWindowSystemUtilities::ScopedXLock xLock;

    auto imageW = (u32) image.getWidth();
    auto imageH = (u32) image.getHeight();
    auto hotspotX = hotspot.x;
    auto hotspotY = hotspot.y;

   #if DRX_USE_XCURSOR
    if (auto xcImage = makeDeletedPtr (X11Symbols::getInstance()->xcursorImageCreate ((i32) imageW, (i32) imageH),
                                       [] (XcursorImage* i) { X11Symbols::getInstance()->xcursorImageDestroy (i); }))
    {
        xcImage->xhot = (XcursorDim) hotspotX;
        xcImage->yhot = (XcursorDim) hotspotY;
        auto* dest = xcImage->pixels;

        for (i32 y = 0; y < (i32) imageH; ++y)
            for (i32 x = 0; x < (i32) imageW; ++x)
                *dest++ = image.getPixelAt (x, y).getARGB();

        auto result = X11Symbols::getInstance()->xcursorImageLoadCursor (display, xcImage.get());

        if (result != Cursor{})
            return result;
    }
   #endif

    auto root = X11Symbols::getInstance()->xRootWindow (display,
                                                        X11Symbols::getInstance()->xDefaultScreen (display));

    u32 cursorW, cursorH;
    if (! X11Symbols::getInstance()->xQueryBestCursor (display, root, imageW, imageH, &cursorW, &cursorH))
        return {};

    Image im (Image::ARGB, (i32) cursorW, (i32) cursorH, true);

    {
        Graphics g (im);

        if (imageW > cursorW || imageH > cursorH)
        {
            hotspotX = (hotspotX * (i32) cursorW) / (i32) imageW;
            hotspotY = (hotspotY * (i32) cursorH) / (i32) imageH;

            g.drawImage (image, Rectangle<f32> ((f32) imageW, (f32) imageH),
                         RectanglePlacement::xLeft | RectanglePlacement::yTop | RectanglePlacement::onlyReduceInSize);
        }
        else
        {
            g.drawImageAt (image, 0, 0);
        }
    }

    auto stride = (cursorW + 7) >> 3;
    HeapBlock<t8> maskPlane, sourcePlane;
    maskPlane.calloc (stride * cursorH);
    sourcePlane.calloc (stride * cursorH);

    auto msbfirst = (X11Symbols::getInstance()->xBitmapBitOrder (display) == MSBFirst);

    for (auto y = (i32) cursorH; --y >= 0;)
    {
        for (auto x = (i32) cursorW; --x >= 0;)
        {
            auto mask   = (t8) (1 << (msbfirst ? (7 - (x & 7)) : (x & 7)));
            auto offset = (u32) y * stride + ((u32) x >> 3);

            auto c = im.getPixelAt (x, y);

            if (c.getAlpha() >= 128)        maskPlane[offset]   |= mask;
            if (c.getBrightness() >= 0.5f)  sourcePlane[offset] |= mask;
        }
    }

    auto xFreePixmap = [this] (Pixmap& p) { X11Symbols::getInstance()->xFreePixmap (display, p); };
    XValueHolder<Pixmap> sourcePixmap (X11Symbols::getInstance()->xCreatePixmapFromBitmapData (display, root, sourcePlane.getData(), cursorW, cursorH, 0xffff, 0, 1), xFreePixmap);
    XValueHolder<Pixmap> maskPixmap   (X11Symbols::getInstance()->xCreatePixmapFromBitmapData (display, root, maskPlane.getData(),   cursorW, cursorH, 0xffff, 0, 1), xFreePixmap);

    XColor white, black;
    black.red = black.green = black.blue = 0;
    white.red = white.green = white.blue = 0xffff;

    return X11Symbols::getInstance()->xCreatePixmapCursor (display, sourcePixmap.value, maskPixmap.value, &white, &black,
                                                           (u32) hotspotX, (u32) hotspotY);
}

z0 XWindowSystem::deleteMouseCursor (Cursor cursorHandle) const
{
    if (cursorHandle != Cursor{} && display != nullptr)
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        X11Symbols::getInstance()->xFreeCursor (display, (Cursor) cursorHandle);
    }
}

static Cursor createDraggingHandCursor()
{
    constexpr u8 dragHandData[] = {
        71,73,70,56,57,97,16,0,16,0,145,2,0,0,0,0,255,255,255,0,0,0,0,0,0,33,249,4,1,0,0,2,0,44,0,0,0,0,16,0,16,0,
        0,2,52,148,47,0,200,185,16,130,90,12,74,139,107,84,123,39,132,117,151,116,132,146,248,60,209,138,98,22,203,
        114,34,236,37,52,77,217, 247,154,191,119,110,240,193,128,193,95,163,56,60,234,98,135,2,0,59
    };

    auto image = ImageFileFormat::loadFrom (dragHandData, (size_t) numElementsInArray (dragHandData));
    return XWindowSystem::getInstance()->createCustomMouseCursorInfo (std::move (image), { 8, 7 });
}

Cursor XWindowSystem::createStandardMouseCursor (MouseCursor::StandardCursorType type) const
{
    if (display == nullptr)
        return None;

    u32 shape;

    switch (type)
    {
        case MouseCursor::NormalCursor:
        case MouseCursor::ParentCursor:                  return None; // Use parent cursor
        case MouseCursor::NoCursor:                      return XWindowSystem::createCustomMouseCursorInfo (Image (Image::ARGB, 16, 16, true), {});

        case MouseCursor::WaitCursor:                    shape = XC_watch; break;
        case MouseCursor::IBeamCursor:                   shape = XC_xterm; break;
        case MouseCursor::PointingHandCursor:            shape = XC_hand2; break;
        case MouseCursor::LeftRightResizeCursor:         shape = XC_sb_h_double_arrow; break;
        case MouseCursor::UpDownResizeCursor:            shape = XC_sb_v_double_arrow; break;
        case MouseCursor::UpDownLeftRightResizeCursor:   shape = XC_fleur; break;
        case MouseCursor::TopEdgeResizeCursor:           shape = XC_top_side; break;
        case MouseCursor::BottomEdgeResizeCursor:        shape = XC_bottom_side; break;
        case MouseCursor::LeftEdgeResizeCursor:          shape = XC_left_side; break;
        case MouseCursor::RightEdgeResizeCursor:         shape = XC_right_side; break;
        case MouseCursor::TopLeftCornerResizeCursor:     shape = XC_top_left_corner; break;
        case MouseCursor::TopRightCornerResizeCursor:    shape = XC_top_right_corner; break;
        case MouseCursor::BottomLeftCornerResizeCursor:  shape = XC_bottom_left_corner; break;
        case MouseCursor::BottomRightCornerResizeCursor: shape = XC_bottom_right_corner; break;
        case MouseCursor::CrosshairCursor:               shape = XC_crosshair; break;
        case MouseCursor::DraggingHandCursor:            return createDraggingHandCursor();

        case MouseCursor::CopyingCursor:
        {
            constexpr u8 copyCursorData[] = {
                71,73,70,56,57,97,21,0,21,0,145,0,0,0,0,0,255,255,255,0,128,128,255,255,255,33,249,4,1,0,0,3,0,44,0,0,0,0,
                21,0,21,0,0,2,72,4,134,169,171,16,199,98,11,79,90,71,161,93,56,111,78,133,218,215,137,31,82,154,100,200,
                86,91,202,142,12,108,212,87,235,174,15,54,214,126,237,226,37,96,59,141,16,37,18,201,142,157,230,204,51,112,
                252,114,147,74,83,5,50,68,147,208,217,16,71,149,252,124,5,0,59,0,0
            };

            auto image = ImageFileFormat::loadFrom (copyCursorData, (size_t) numElementsInArray (copyCursorData));
            return createCustomMouseCursorInfo (std::move (image), { 1, 3 });
        }

        case MouseCursor::NumStandardCursorTypes:
        default:
        {
            jassertfalse;
            return None;
        }
    }

    XWindowSystemUtilities::ScopedXLock xLock;

    return X11Symbols::getInstance()->xCreateFontCursor (display, shape);
}

z0 XWindowSystem::showCursor (::Window windowH, Cursor cursorHandle) const
{
    jassert (windowH != 0);

    XWindowSystemUtilities::ScopedXLock xLock;
    X11Symbols::getInstance()->xDefineCursor (display, windowH, (Cursor) cursorHandle);
}

b8 XWindowSystem::isKeyCurrentlyDown (i32 keyCode) const
{
    i32 keysym;

    if (keyCode & Keys::extendedKeyModifier)
    {
        keysym = 0xff00 | (keyCode & 0xff);
    }
    else
    {
        keysym = keyCode;

        if (keysym == (XK_Tab & 0xff)
            || keysym == (XK_Return & 0xff)
            || keysym == (XK_Escape & 0xff)
            || keysym == (XK_BackSpace & 0xff))
        {
            keysym |= 0xff00;
        }
    }

    XWindowSystemUtilities::ScopedXLock xLock;

    auto keycode = X11Symbols::getInstance()->xKeysymToKeycode (display, (KeySym) keysym);
    auto keybyte = keycode >> 3;
    auto keybit = (1 << (keycode & 7));

    return (Keys::keyStates [keybyte] & keybit) != 0;
}

ModifierKeys XWindowSystem::getNativeRealtimeModifiers() const
{
    ::Window root, child;
    i32 x, y, winx, winy;
    u32 mask;
    i32 mouseMods = 0, keyboardMods = 0, keyboardClearMods = 0;

    XWindowSystemUtilities::ScopedXLock xLock;

    if (X11Symbols::getInstance()->xQueryPointer (display,
                                                  X11Symbols::getInstance()->xRootWindow (display,
                                                                                          X11Symbols::getInstance()->xDefaultScreen (display)),
                                                  &root, &child, &x, &y, &winx, &winy, &mask) != False)
    {
        if ((mask & Button1Mask) != 0)  mouseMods |= ModifierKeys::leftButtonModifier;
        if ((mask & Button2Mask) != 0)  mouseMods |= ModifierKeys::middleButtonModifier;
        if ((mask & Button3Mask) != 0)  mouseMods |= ModifierKeys::rightButtonModifier;

        ((mask & ShiftMask)     != 0 ? keyboardMods : keyboardClearMods) |= ModifierKeys::shiftModifier;
        ((mask & ControlMask)   != 0 ? keyboardMods : keyboardClearMods) |= ModifierKeys::ctrlModifier;
    }

    ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons()
                                                                   .withFlags (mouseMods)
                                                                   .withoutFlags (keyboardClearMods)
                                                                   .withFlags (keyboardMods);

    // We are keeping track of the state of modifier keys and mouse buttons with the assumption that
    // for every mouse down we are going to receive a mouse up etc.
    //
    // This assumption is broken when getNativeRealtimeModifiers() is called. If for example we call
    // this function when the mouse cursor is in another application and the mouse button happens to
    // be down, then its represented state in currentModifiers may remain down indefinitely, since
    // we aren't going to receive an event when it's released.
    //
    // We mark this state in this variable, and we can restore synchronization when our window
    // receives an event again.
    Keys::modifierKeysAreStale = true;

    return ModifierKeys::currentModifiers;
}

static b8 hasWorkAreaData (const XWindowSystemUtilities::GetXProperty& prop)
{
    return prop.success
        && prop.actualType == XA_CARDINAL
        && prop.actualFormat == 32
        && prop.numItems == 4
        && prop.data != nullptr;
}

static Rectangle<i32> getWorkArea (const XWindowSystemUtilities::GetXProperty& prop)
{
    if (hasWorkAreaData (prop))
    {
        auto* positionData = prop.data;
        std::array<i64, 4> position;

        for (auto& p : position)
        {
            memcpy (&p, positionData, sizeof (i64));
            positionData += sizeof (i64);
        }

        return { (i32) position[0], (i32) position[1],
                 (i32) position[2], (i32) position[3] };
    }

    return {};
}

Array<Displays::Display> XWindowSystem::findDisplays (f32 masterScale) const
{
    Array<Displays::Display> displays;
    auto workAreaHints = XWindowSystemUtilities::Atoms::getIfExists (display, "_NET_WORKAREA");

   #if DRX_USE_XRANDR
    if (workAreaHints != None)
    {
        i32 major_opcode, first_event, first_error;

        if (X11Symbols::getInstance()->xQueryExtension (display, "RANDR", &major_opcode, &first_event, &first_error))
        {
            auto numMonitors = X11Symbols::getInstance()->xScreenCount (display);
            auto mainDisplay = X11Symbols::getInstance()->xRRGetOutputPrimary (display, X11Symbols::getInstance()->xRootWindow (display, 0));

            for (i32 i = 0; i < numMonitors; ++i)
            {
                auto rootWindow = X11Symbols::getInstance()->xRootWindow (display, i);
                XWindowSystemUtilities::GetXProperty prop (display, rootWindow, workAreaHints, 0, 4, false, XA_CARDINAL);

                if (! hasWorkAreaData (prop))
                    continue;

                if (auto screens = makeDeletedPtr (X11Symbols::getInstance()->xRRGetScreenResources (display, rootWindow),
                                                   [] (XRRScreenResources* srs) { X11Symbols::getInstance()->xRRFreeScreenResources (srs); }))
                {
                    for (i32 j = 0; j < screens->noutput; ++j)
                    {
                        if (screens->outputs[j])
                        {
                            // Xrandr on the raspberry pi fails to determine the main display (mainDisplay == 0)!
                            // Detect this edge case and make the first found display the main display
                            if (! mainDisplay)
                                mainDisplay = screens->outputs[j];

                            if (auto output = makeDeletedPtr (X11Symbols::getInstance()->xRRGetOutputInfo (display, screens.get(), screens->outputs[j]),
                                                              [] (XRROutputInfo* oi) { X11Symbols::getInstance()->xRRFreeOutputInfo (oi); }))
                            {
                                if (output->crtc)
                                {
                                    if (auto crtc = makeDeletedPtr (X11Symbols::getInstance()->xRRGetCrtcInfo (display, screens.get(), output->crtc),
                                                                    [] (XRRCrtcInfo* ci) { X11Symbols::getInstance()->xRRFreeCrtcInfo (ci); }))
                                    {
                                        Displays::Display d;
                                        d.totalArea = { crtc->x, crtc->y, (i32) crtc->width, (i32) crtc->height };
                                        d.isMain = (mainDisplay == screens->outputs[j]) && (i == 0);
                                        d.dpi = DisplayHelpers::getDisplayDPI (display, 0);

                                        d.verticalFrequencyHz = [&]() -> std::optional<f64>
                                        {
                                            if (crtc->mode != None)
                                            {
                                                if (auto it = std::find_if (screens->modes,
                                                                            screens->modes + screens->nmode,
                                                                            [&crtc] (const auto& m) { return m.id == crtc->mode; });
                                                    it != screens->modes + screens->nmode)
                                                {
                                                    return (f64) it->dotClock / ((f64) it->hTotal * (f64) it->vTotal);
                                                }
                                            }

                                            return {};
                                        }();

                                        // The raspberry pi returns a zero sized display, so we need to guard for divide-by-zero
                                        if (output->mm_width > 0 && output->mm_height > 0)
                                            d.dpi = ((static_cast<f64> (crtc->width)  * 25.4 * 0.5) / static_cast<f64> (output->mm_width))
                                                  + ((static_cast<f64> (crtc->height) * 25.4 * 0.5) / static_cast<f64> (output->mm_height));

                                        auto scale = DisplayHelpers::getDisplayScale (output->name, d.dpi);
                                        scale = (scale <= 0.1 || ! DRXApplicationBase::isStandaloneApp()) ? 1.0 : scale;

                                        d.scale = masterScale * scale;

                                        if (d.isMain)
                                            displays.insert (0, d);
                                        else
                                            displays.add (d);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (! displays.isEmpty() && ! displays.getReference (0).isMain)
                displays.getReference (0).isMain = true;
        }
    }

    if (displays.isEmpty())
   #endif
   #if DRX_USE_XINERAMA
    {
        auto screens = DisplayHelpers::xineramaQueryDisplays (display);
        auto numMonitors = screens.size();

        for (i32 index = 0; index < numMonitors; ++index)
        {
            for (auto j = numMonitors; --j >= 0;)
            {
                if (screens[j].screen_number == index)
                {
                    Displays::Display d;
                    d.totalArea = { screens[j].x_org, screens[j].y_org,
                                    screens[j].width, screens[j].height };
                    d.isMain = (index == 0);
                    d.scale = masterScale;
                    d.dpi = DisplayHelpers::getDisplayDPI (display, 0); // (all screens share the same DPI)

                    displays.add (d);
                }
            }
        }
    }

    if (displays.isEmpty())
   #endif
    {
        if (workAreaHints != None)
        {
            auto numMonitors = X11Symbols::getInstance()->xScreenCount (display);

            for (i32 i = 0; i < numMonitors; ++i)
            {
                XWindowSystemUtilities::GetXProperty prop (display,
                                                           X11Symbols::getInstance()->xRootWindow (display, i),
                                                           workAreaHints, 0, 4, false, XA_CARDINAL);

                auto workArea = getWorkArea (prop);

                if (! workArea.isEmpty())
                {
                    Displays::Display d;

                    d.totalArea = workArea;
                    d.isMain = displays.isEmpty();
                    d.scale = masterScale;
                    d.dpi = DisplayHelpers::getDisplayDPI (display, i);

                    displays.add (d);
                }
            }
        }

        if (displays.isEmpty())
        {
            Displays::Display d;
            d.totalArea = { X11Symbols::getInstance()->xDisplayWidth  (display, X11Symbols::getInstance()->xDefaultScreen (display)),
                            X11Symbols::getInstance()->xDisplayHeight (display, X11Symbols::getInstance()->xDefaultScreen (display)) };
            d.isMain = true;
            d.scale = masterScale;
            d.dpi = DisplayHelpers::getDisplayDPI (display, 0);

            displays.add (d);
        }
    }

    for (auto& d : displays)
        d.userArea = d.totalArea; // DRX currently does not support requesting the user area on Linux

    return displays;
}

::Window XWindowSystem::createKeyProxy (::Window windowH)
{
    jassert (windowH != 0);

    XSetWindowAttributes swa;
    swa.event_mask = KeyPressMask | KeyReleaseMask | FocusChangeMask;

    auto keyProxy = X11Symbols::getInstance()->xCreateWindow (display, windowH,
                                                              -1, -1, 1, 1, 0, 0,
                                                              InputOnly, CopyFromParent,
                                                              CWEventMask,
                                                              &swa);

    X11Symbols::getInstance()->xMapWindow (display, keyProxy);

    return keyProxy;
}

z0 XWindowSystem::deleteKeyProxy (::Window keyProxy) const
{
    jassert (keyProxy != 0);

    X11Symbols::getInstance()->xDestroyWindow (display, keyProxy);
    X11Symbols::getInstance()->xSync (display, false);

    XEvent event;
    while (X11Symbols::getInstance()->xCheckWindowEvent (display, keyProxy, getAllEventsMask (false), &event) == True)
    {}
}

b8 XWindowSystem::externalDragFileInit (LinuxComponentPeer* peer, const StringArray& files, b8, std::function<z0()>&& callback) const
{
    auto& dragState = dragAndDropStateMap[peer];

    if (dragState.isDragging())
        return false;

    StringArray uriList;

    for (auto& f : files)
    {
        if (f.matchesWildcard ("?*://*", false))
            uriList.add (f);
        else
            uriList.add ("file://" + f);
    }

    return dragState.externalDragInit ((::Window) peer->getNativeHandle(), false, uriList.joinIntoString ("\r\n"), std::move (callback));
}

b8 XWindowSystem::externalDragTextInit (LinuxComponentPeer* peer, const Txt& text, std::function<z0()>&& callback) const
{
    auto& dragState = dragAndDropStateMap[peer];

    if (dragState.isDragging())
        return false;

    return dragState.externalDragInit ((::Window) peer->getNativeHandle(), true, text, std::move (callback));
}

z0 XWindowSystem::copyTextToClipboard (const Txt& clipText)
{
    localClipboardContent = clipText;

    X11Symbols::getInstance()->xSetSelectionOwner (display, XA_PRIMARY,      drx_messageWindowHandle, CurrentTime);
    X11Symbols::getInstance()->xSetSelectionOwner (display, atoms.clipboard, drx_messageWindowHandle, CurrentTime);
}

Txt XWindowSystem::getTextFromClipboard() const
{
    /* 1) try to read from the "CLIPBOARD" selection first (the "high
       level" clipboard that is supposed to be filled by ctrl-C
       etc). When a clipboard manager is running, the content of this
       selection is preserved even when the original selection owner
       exits.

       2) and then try to read from "PRIMARY" selection (the "legacy" selection
       filled by good old x11 apps such as xterm)
    */

    auto getContentForSelection = [this] (Atom selectionAtom) -> Txt
    {
        auto selectionOwner = X11Symbols::getInstance()->xGetSelectionOwner (display, selectionAtom);

        if (selectionOwner == None)
            return {};

        if (selectionOwner == drx_messageWindowHandle)
            return localClipboardContent;

        Txt content;

        if (! ClipboardHelpers::requestSelectionContent (display, content, selectionAtom, atoms.utf8String))
            ClipboardHelpers::requestSelectionContent (display, content, selectionAtom, XA_STRING);

        return content;
    };

    auto content = getContentForSelection (atoms.clipboard);

    if (content.isEmpty())
        content = getContentForSelection (XA_PRIMARY);

    return content;
}

//==============================================================================
::Window XWindowSystem::findTopLevelWindowOf (::Window w) const
{
    if (w == 0)
        return 0;

    Window* windowList = nullptr;
    u32 windowListSize = 0;
    Window parent, root;

    XWindowSystemUtilities::ScopedXLock xLock;
    const auto result = X11Symbols::getInstance()->xQueryTree (display, w, &root, &parent, &windowList, &windowListSize);
    const auto deleter = makeXFreePtr (windowList);

    if (result == 0)
        return 0;

    if (parent == root)
        return w;

    return findTopLevelWindowOf (parent);
}

b8 XWindowSystem::isParentWindowOf (::Window windowH, ::Window possibleChild) const
{
    if (windowH == 0 || possibleChild == 0)
        return false;

    if (possibleChild == windowH)
        return true;

    Window* windowList = nullptr;
    u32 windowListSize = 0;
    Window parent, root;

    XWindowSystemUtilities::ScopedXLock xLock;
    const auto result = X11Symbols::getInstance()->xQueryTree (display, possibleChild, &root, &parent, &windowList, &windowListSize);
    const auto deleter = makeXFreePtr (windowList);

    if (result == 0 || parent == root)
        return false;

    return isParentWindowOf (windowH, parent);
}

b8 XWindowSystem::isFrontWindow (::Window windowH) const
{
    jassert (windowH != 0);

    Window* windowList = nullptr;
    u32 windowListSize = 0;

    XWindowSystemUtilities::ScopedXLock xLock;
    Window parent;
    auto root = X11Symbols::getInstance()->xRootWindow (display, X11Symbols::getInstance()->xDefaultScreen (display));

    const auto queryResult = X11Symbols::getInstance()->xQueryTree (display, root, &root, &parent, &windowList, &windowListSize);
    const auto deleter = makeXFreePtr (windowList);

    if (queryResult == 0)
        return false;

    for (i32 i = (i32) windowListSize; --i >= 0;)
    {
        if (auto* peer = dynamic_cast<LinuxComponentPeer*> (getPeerFor (windowList[i])))
            return peer == dynamic_cast<LinuxComponentPeer*> (getPeerFor (windowH));
    }

    return false;
}

z0 XWindowSystem::xchangeProperty (::Window windowH, Atom property, Atom type, i32 format, ukk data, i32 numElements) const
{
    jassert (windowH != 0);

    X11Symbols::getInstance()->xChangeProperty (display, windowH, property, type, format, PropModeReplace, (u8k*) data, numElements);
}

z0 XWindowSystem::removeWindowDecorations (::Window windowH) const
{
    jassert (windowH != 0);

    auto hints = XWindowSystemUtilities::Atoms::getIfExists (display, "_MOTIF_WM_HINTS");

    if (hints != None)
    {
        MotifWmHints motifHints;
        zerostruct (motifHints);

        motifHints.flags = 2; /* MWM_HINTS_DECORATIONS */
        motifHints.decorations = 0;

        XWindowSystemUtilities::ScopedXLock xLock;
        xchangeProperty (windowH, hints, hints, 32, &motifHints, 4);
    }

    hints = XWindowSystemUtilities::Atoms::getIfExists (display, "_WIN_HINTS");

    if (hints != None)
    {
        i64 gnomeHints = 0;

        XWindowSystemUtilities::ScopedXLock xLock;
        xchangeProperty (windowH, hints, hints, 32, &gnomeHints, 1);
    }

    hints = XWindowSystemUtilities::Atoms::getIfExists (display, "KWM_WIN_DECORATION");

    if (hints != None)
    {
        i64 kwmHints = 2; /*KDE_tinyDecoration*/

        XWindowSystemUtilities::ScopedXLock xLock;
        xchangeProperty (windowH, hints, hints, 32, &kwmHints, 1);
    }

    hints = XWindowSystemUtilities::Atoms::getIfExists (display, "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

    if (hints != None)
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        xchangeProperty (windowH, atoms.windowType, XA_ATOM, 32, &hints, 1);
    }
}

static z0 addAtomIfExists (b8 condition, tukk key, ::Display* display, std::vector<Atom>& atoms)
{
    if (condition)
    {
        auto atom = XWindowSystemUtilities::Atoms::getIfExists (display, key);

        if (atom != None)
            atoms.push_back (atom);
    }
}

z0 XWindowSystem::addWindowButtons (::Window windowH, i32 styleFlags) const
{
    jassert (windowH != 0);

    XWindowSystemUtilities::ScopedXLock xLock;
    auto motifAtom = XWindowSystemUtilities::Atoms::getIfExists (display, "_MOTIF_WM_HINTS");

    if (motifAtom != None)
    {
        MotifWmHints motifHints;
        zerostruct (motifHints);

        motifHints.flags = 1 | 2; /* MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS */
        motifHints.decorations = 2 /* MWM_DECOR_BORDER */ | 8 /* MWM_DECOR_TITLE */ | 16; /* MWM_DECOR_MENU */

        motifHints.functions = 4 /* MWM_FUNC_MOVE */;

        if ((styleFlags & ComponentPeer::windowHasCloseButton) != 0)
            motifHints.functions |= 32; /* MWM_FUNC_CLOSE */

        if ((styleFlags & ComponentPeer::windowHasMinimiseButton) != 0)
        {
            motifHints.functions |= 8; /* MWM_FUNC_MINIMIZE */
            motifHints.decorations |= 0x20; /* MWM_DECOR_MINIMIZE */
        }

        if ((styleFlags & ComponentPeer::windowHasMaximiseButton) != 0)
        {
            motifHints.functions |= 0x10; /* MWM_FUNC_MAXIMIZE */
            motifHints.decorations |= 0x40; /* MWM_DECOR_MAXIMIZE */
        }

        if ((styleFlags & ComponentPeer::windowIsResizable) != 0)
        {
            motifHints.functions |= 2; /* MWM_FUNC_RESIZE */
            motifHints.decorations |= 0x4; /* MWM_DECOR_RESIZEH */
        }

        xchangeProperty (windowH, motifAtom, motifAtom, 32, &motifHints, 5);
    }

    auto actionsAtom = XWindowSystemUtilities::Atoms::getIfExists (display, "_NET_WM_ALLOWED_ACTIONS");

    if (actionsAtom != None)
    {
        std::vector<Atom> netHints;

        addAtomIfExists ((styleFlags & ComponentPeer::windowIsResizable)       != 0, "_NET_WM_ACTION_RESIZE",     display, netHints);
        addAtomIfExists ((styleFlags & ComponentPeer::windowHasMaximiseButton) != 0, "_NET_WM_ACTION_FULLSCREEN", display, netHints);
        addAtomIfExists ((styleFlags & ComponentPeer::windowHasMinimiseButton) != 0, "_NET_WM_ACTION_MINIMIZE",   display, netHints);
        addAtomIfExists ((styleFlags & ComponentPeer::windowHasCloseButton)    != 0, "_NET_WM_ACTION_CLOSE",      display, netHints);

        auto numHints = (i32) netHints.size();

        if (numHints > 0)
            xchangeProperty (windowH, actionsAtom, XA_ATOM, 32, netHints.data(), numHints);
    }
}

z0 XWindowSystem::setWindowType (::Window windowH, i32 styleFlags) const
{
    jassert (windowH != 0);

    if (atoms.windowType != None)
    {
        auto hint = (styleFlags & ComponentPeer::windowIsTemporary) != 0
                    || ((styleFlags & ComponentPeer::windowHasDropShadow) == 0 && Desktop::canUseSemiTransparentWindows())
                        ? XWindowSystemUtilities::Atoms::getIfExists (display, "_NET_WM_WINDOW_TYPE_COMBO")
                        : XWindowSystemUtilities::Atoms::getIfExists (display, "_NET_WM_WINDOW_TYPE_NORMAL");

        if (hint != None)
            xchangeProperty (windowH, atoms.windowType, XA_ATOM, 32, &hint, 1);
    }

    if (atoms.windowState != None)
    {
        std::vector<Atom> netStateHints;

        addAtomIfExists ((styleFlags & ComponentPeer::windowAppearsOnTaskbar) == 0, "_NET_WM_STATE_SKIP_TASKBAR", display, netStateHints);
        addAtomIfExists (getPeerFor (windowH)->getComponent().isAlwaysOnTop(),      "_NET_WM_STATE_ABOVE",        display, netStateHints);

        auto numHints = (i32) netStateHints.size();

        if (numHints > 0)
            xchangeProperty (windowH, atoms.windowState, XA_ATOM, 32, netStateHints.data(), numHints);
    }
}

z0 XWindowSystem::initialisePointerMap()
{
    auto numButtons = X11Symbols::getInstance()->xGetPointerMapping (display, nullptr, 0);
    pointerMap[2] = pointerMap[3] = pointerMap[4] = Keys::NoButton;

    if (numButtons == 2)
    {
        pointerMap[0] = Keys::LeftButton;
        pointerMap[1] = Keys::RightButton;
    }
    else if (numButtons >= 3)
    {
        pointerMap[0] = Keys::LeftButton;
        pointerMap[1] = Keys::MiddleButton;
        pointerMap[2] = Keys::RightButton;

        if (numButtons >= 5)
        {
            pointerMap[3] = Keys::WheelUp;
            pointerMap[4] = Keys::WheelDown;
        }
    }
}

z0 XWindowSystem::deleteIconPixmaps (::Window windowH) const
{
    jassert (windowH != 0);

    XWindowSystemUtilities::ScopedXLock xLock;

    if (auto wmHints = makeXFreePtr (X11Symbols::getInstance()->xGetWMHints (display, windowH)))
    {
        if ((wmHints->flags & IconPixmapHint) != 0)
        {
            wmHints->flags &= ~IconPixmapHint;
            X11Symbols::getInstance()->xFreePixmap (display, wmHints->icon_pixmap);
        }

        if ((wmHints->flags & IconMaskHint) != 0)
        {
            wmHints->flags &= ~IconMaskHint;
            X11Symbols::getInstance()->xFreePixmap (display, wmHints->icon_mask);
        }

        X11Symbols::getInstance()->xSetWMHints (display, windowH, wmHints.get());
    }
}

// Alt and Num lock are not defined by standard X modifier constants: check what they're mapped to
z0 XWindowSystem::updateModifierMappings() const
{
    XWindowSystemUtilities::ScopedXLock xLock;
    auto altLeftCode = X11Symbols::getInstance()->xKeysymToKeycode (display, XK_Alt_L);
    auto numLockCode = X11Symbols::getInstance()->xKeysymToKeycode (display, XK_Num_Lock);

    Keys::AltMask = 0;
    Keys::NumLockMask = 0;

    if (auto mapping = makeDeletedPtr (X11Symbols::getInstance()->xGetModifierMapping (display),
                                       [] (XModifierKeymap* mk) { X11Symbols::getInstance()->xFreeModifiermap (mk); }))
    {
        for (i32 modifierIdx = 0; modifierIdx < 8; ++modifierIdx)
        {
            for (i32 keyIndex = 0; keyIndex < mapping->max_keypermod; ++keyIndex)
            {
                auto key = mapping->modifiermap[(modifierIdx * mapping->max_keypermod) + keyIndex];

                if (key == altLeftCode)
                    Keys::AltMask = 1 << modifierIdx;
                else if (key == numLockCode)
                    Keys::NumLockMask = 1 << modifierIdx;
            }
        }
    }
}

i64 XWindowSystem::getUserTime (::Window windowH) const
{
    jassert (windowH != 0);

    XWindowSystemUtilities::GetXProperty prop (display, windowH, atoms.userTime, 0, 65536, false, XA_CARDINAL);

    if (! prop.success)
        return 0;

    i64 result = 0;
    std::memcpy (&result, prop.data, sizeof (i64));

    return result;
}

z0 XWindowSystem::initialiseXSettings()
{
    xSettings = XWindowSystemUtilities::XSettings::createXSettings (display);

    if (xSettings != nullptr)
        X11Symbols::getInstance()->xSelectInput (display,
                                                 xSettings->getSettingsWindow(),
                                                 StructureNotifyMask | PropertyChangeMask);
}

XWindowSystem::DisplayVisuals::DisplayVisuals (::Display* xDisplay)
{
    auto findVisualWithDepthOrNull = [&] (i32 desiredDepth) -> Visual*
    {
        i32 matchedDepth = 0;
        auto* visual = Visuals::findVisualFormat (xDisplay, desiredDepth, matchedDepth);

        if (desiredDepth == matchedDepth)
            return visual;

        return nullptr;
    };

    visual16Bit = findVisualWithDepthOrNull (16);
    visual24Bit = findVisualWithDepthOrNull (24);
    visual32Bit = findVisualWithDepthOrNull (32);
}

XWindowSystem::VisualAndDepth XWindowSystem::DisplayVisuals::getBestVisualForWindow (b8 isSemiTransparent) const
{
    if (isSemiTransparent && visual32Bit != nullptr)
        return { visual32Bit, 32 };

    if (visual24Bit != nullptr)
        return { visual24Bit, 24 };

    if (visual32Bit != nullptr)
        return { visual32Bit, 32 };

    // No visual available!
    jassert (visual16Bit != nullptr);

    return { visual16Bit, 16 };
}

b8 XWindowSystem::DisplayVisuals::isValid() const noexcept
{
    return (visual32Bit != nullptr || visual24Bit != nullptr || visual16Bit != nullptr);
}

//==============================================================================
b8 XWindowSystem::initialiseXDisplay()
{
    jassert (display == nullptr);

    Txt displayName (getenv ("DISPLAY"));

    if (displayName.isEmpty())
        displayName = ":0.0";

    // it seems that on some systems XOpenDisplay will occasionally
    // fail the first time, but succeed on a second attempt..
    for (i32 retries = 2; --retries >= 0;)
    {
        display = X11Symbols::getInstance()->xOpenDisplay (displayName.toUTF8());

        if (display != nullptr)
            break;
    }

    // No X Server running
    if (display == nullptr)
        return false;

   #if DRX_DEBUG_XERRORS_SYNCHRONOUSLY
    X11Symbols::getInstance()->xSynchronize (display, True);
   #endif

    // Create a context to store user data associated with Windows we create
    windowHandleXContext = (XContext) X11Symbols::getInstance()->xrmUniqueQuark();

    // Create our message window (this will never be mapped)
    auto screen = X11Symbols::getInstance()->xDefaultScreen (display);
    auto root = X11Symbols::getInstance()->xRootWindow (display, screen);
    X11Symbols::getInstance()->xSelectInput (display, root, SubstructureNotifyMask);

    // We're only interested in client messages for this window, which are always sent
    XSetWindowAttributes swa;
    swa.event_mask = NoEventMask;
    drx_messageWindowHandle = X11Symbols::getInstance()->xCreateWindow (display, root,
                                                                         0, 0, 1, 1, 0, 0, InputOnly,
                                                                         X11Symbols::getInstance()->xDefaultVisual (display, screen),
                                                                         CWEventMask, &swa);

    X11Symbols::getInstance()->xSync (display, False);

    atoms = XWindowSystemUtilities::Atoms (display);

    initialisePointerMap();
    updateModifierMappings();
    initialiseXSettings();

   #if DRX_USE_XSHM
    if (XSHMHelpers::isShmAvailable (display))
        shmCompletionEvent = X11Symbols::getInstance()->xShmGetEventBase (display) + ShmCompletion;
   #endif

    displayVisuals = std::make_unique<DisplayVisuals> (display);

    if (! displayVisuals->isValid())
    {
        Logger::outputDebugString ("ERROR: System doesn't support 32, 24 or 16 bit RGB display.\n");
        return false;
    }

    // Setup input event handler
    LinuxEventLoop::registerFdCallback (X11Symbols::getInstance()->xConnectionNumber (display),
                                        [this] (i32)
                                        {
                                            do
                                            {
                                                XEvent evt;

                                                {
                                                    XWindowSystemUtilities::ScopedXLock xLock;

                                                    if (! X11Symbols::getInstance()->xPending (display))
                                                        return;

                                                    X11Symbols::getInstance()->xNextEvent (display, &evt);
                                                }

                                                if (evt.type == SelectionRequest && evt.xany.window == drx_messageWindowHandle)
                                                {
                                                    ClipboardHelpers::handleSelection (evt.xselectionrequest);
                                                }
                                                else if (evt.xany.window != drx_messageWindowHandle)
                                                {
                                                    windowMessageReceive (evt);
                                                }

                                            } while (display != nullptr);
                                        });

    return true;
}

z0 XWindowSystem::destroyXDisplay()
{
    if (xIsAvailable)
    {
        jassert (display != nullptr);

        {
            XWindowSystemUtilities::ScopedXLock xLock;

            X11Symbols::getInstance()->xDestroyWindow (display, drx_messageWindowHandle);
            drx_messageWindowHandle = 0;
            X11Symbols::getInstance()->xSync (display, True);
        }

        LinuxEventLoop::unregisterFdCallback (X11Symbols::getInstance()->xConnectionNumber (display));

        {
            XWindowSystemUtilities::ScopedXLock xLock;
            X11Symbols::getInstance()->xCloseDisplay (display);
            display = nullptr;
            displayVisuals = nullptr;
        }
    }
}

//==============================================================================
::Window drx_createKeyProxyWindow (ComponentPeer* peer);
::Window drx_createKeyProxyWindow (ComponentPeer* peer)
{
    return XWindowSystem::getInstance()->createKeyProxy ((::Window) peer->getNativeHandle());
}

z0 drx_deleteKeyProxyWindow (::Window keyProxy);
z0 drx_deleteKeyProxyWindow (::Window keyProxy)
{
    XWindowSystem::getInstance()->deleteKeyProxy (keyProxy);
}

//==============================================================================
template <typename EventType>
static Point<f32> getLogicalMousePos (const EventType& e, f64 scaleFactor) noexcept
{
    return Point<f32> ((f32) e.x, (f32) e.y) / scaleFactor;
}

static z64 getEventTime (::Time t)
{
    static z64 eventTimeOffset = 0x12345678;
    auto thisMessageTime = (z64) t;

    if (eventTimeOffset == 0x12345678)
        eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;

    return eventTimeOffset + thisMessageTime;
}

template <typename EventType>
static z64 getEventTime (const EventType& t)
{
    return getEventTime (t.time);
}

z0 XWindowSystem::handleWindowMessage (LinuxComponentPeer* peer, XEvent& event) const
{
    switch (event.xany.type)
    {
        case KeyPressEventType:     handleKeyPressEvent        (peer, event.xkey);                     break;
        case KeyRelease:            handleKeyReleaseEvent      (peer, event.xkey);                     break;
        case ButtonPress:           handleButtonPressEvent     (peer, event.xbutton);                  break;
        case ButtonRelease:         handleButtonReleaseEvent   (peer, event.xbutton);                  break;
        case MotionNotify:          handleMotionNotifyEvent    (peer, event.xmotion);                  break;
        case EnterNotify:           handleEnterNotifyEvent     (peer, event.xcrossing);                break;
        case LeaveNotify:           handleLeaveNotifyEvent     (peer, event.xcrossing);                break;
        case FocusIn:               handleFocusInEvent         (peer);                                 break;
        case FocusOut:              handleFocusOutEvent        (peer);                                 break;
        case Expose:                handleExposeEvent          (peer, event.xexpose);                  break;
        case MappingNotify:         handleMappingNotify        (event.xmapping);                       break;
        case ClientMessage:         handleClientMessageEvent   (peer, event.xclient, event);           break;
        case SelectionNotify:       dragAndDropStateMap[peer].handleDragAndDropSelection (event);      break;
        case ConfigureNotify:       handleConfigureNotifyEvent (peer, event.xconfigure);               break;
        case ReparentNotify:
        case GravityNotify:         handleGravityNotify (peer);                                        break;
        case SelectionClear:        dragAndDropStateMap[peer].handleExternalSelectionClear();          break;
        case SelectionRequest:      dragAndDropStateMap[peer].handleExternalSelectionRequest (event);  break;
        case PropertyNotify:        propertyNotifyEvent        (peer, event.xproperty);                break;

        case CirculateNotify:
        case CreateNotify:
        case DestroyNotify:
        case UnmapNotify:
            break;

        case MapNotify:
            peer->handleBroughtToFront();
            break;

        default:
           #if DRX_USE_XSHM
            if (XSHMHelpers::isShmAvailable (display))
            {
                XWindowSystemUtilities::ScopedXLock xLock;

                if (event.xany.type == shmCompletionEvent)
                    XWindowSystem::getInstance()->removePendingPaintForWindow ((::Window) peer->getNativeHandle());
            }
           #endif
            break;
    }
}

z0 XWindowSystem::handleKeyPressEvent (LinuxComponentPeer* peer, XKeyEvent& keyEvent) const
{
    auto oldMods = ModifierKeys::currentModifiers;
    Keys::refreshStaleModifierKeys();

    t8 utf8 [64] = { 0 };
    t32 unicodeChar = 0;
    i32 keyCode = 0;
    b8 keyDownChange = false;
    KeySym sym;

    {
        XWindowSystemUtilities::ScopedXLock xLock;
        updateKeyStates ((i32) keyEvent.keycode, true);

        Txt oldLocale (::setlocale (LC_ALL, nullptr));
        ::setlocale (LC_ALL, "");
        X11Symbols::getInstance()->xLookupString (&keyEvent, utf8, sizeof (utf8), &sym, nullptr);

        if (oldLocale.isNotEmpty())
            ::setlocale (LC_ALL, oldLocale.toRawUTF8());

        unicodeChar = *CharPointer_UTF8 (utf8);
        keyCode = (i32) unicodeChar;

        if (keyCode < 0x20)
            keyCode = (i32) X11Symbols::getInstance()->xkbKeycodeToKeysym (display, (::KeyCode) keyEvent.keycode, 0,
                                                                           ModifierKeys::currentModifiers.isShiftDown() ? 1 : 0);

        keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, true);
    }

    b8 keyPressed = false;

    if ((sym & 0xff00) == 0xff00 || keyCode == XK_ISO_Left_Tab)
    {
        switch (sym)  // Translate keypad
        {
            case XK_KP_Add:         keyCode = XK_plus;      break;
            case XK_KP_Subtract:    keyCode = XK_hyphen;    break;
            case XK_KP_Divide:      keyCode = XK_slash;     break;
            case XK_KP_Multiply:    keyCode = XK_asterisk;  break;
            case XK_KP_Enter:       keyCode = XK_Return;    break;
            case XK_KP_Insert:      keyCode = XK_Insert;    break;
            case XK_Delete:
            case XK_KP_Delete:      keyCode = XK_Delete;    break;
            case XK_KP_Left:        keyCode = XK_Left;      break;
            case XK_KP_Right:       keyCode = XK_Right;     break;
            case XK_KP_Up:          keyCode = XK_Up;        break;
            case XK_KP_Down:        keyCode = XK_Down;      break;
            case XK_KP_Home:        keyCode = XK_Home;      break;
            case XK_KP_End:         keyCode = XK_End;       break;
            case XK_KP_Page_Down:   keyCode = XK_Page_Down; break;
            case XK_KP_Page_Up:     keyCode = XK_Page_Up;   break;

            case XK_KP_0:           keyCode = XK_0;         break;
            case XK_KP_1:           keyCode = XK_1;         break;
            case XK_KP_2:           keyCode = XK_2;         break;
            case XK_KP_3:           keyCode = XK_3;         break;
            case XK_KP_4:           keyCode = XK_4;         break;
            case XK_KP_5:           keyCode = XK_5;         break;
            case XK_KP_6:           keyCode = XK_6;         break;
            case XK_KP_7:           keyCode = XK_7;         break;
            case XK_KP_8:           keyCode = XK_8;         break;
            case XK_KP_9:           keyCode = XK_9;         break;

            default:                break;
        }

        switch (keyCode)
        {
            case XK_Left:
            case XK_Right:
            case XK_Up:
            case XK_Down:
            case XK_Page_Up:
            case XK_Page_Down:
            case XK_End:
            case XK_Home:
            case XK_Delete:
            case XK_Insert:
                keyPressed = true;
                keyCode = (keyCode & 0xff) | Keys::extendedKeyModifier;
                break;

            case XK_Tab:
            case XK_Return:
            case XK_Escape:
            case XK_BackSpace:
                keyPressed = true;
                keyCode &= 0xff;
                break;

            case XK_ISO_Left_Tab:
                keyPressed = true;
                keyCode = XK_Tab & 0xff;
                break;

            default:
                if (sym >= XK_F1 && sym <= XK_F35)
                {
                    keyPressed = true;
                    keyCode = static_cast<i32> ((sym & 0xff) | Keys::extendedKeyModifier);
                }
                break;
        }
    }

    if (utf8[0] != 0 || ((sym & 0xff00) == 0 && sym >= 8))
        keyPressed = true;

    if (oldMods != ModifierKeys::currentModifiers)
        peer->handleModifierKeysChange();

    if (keyDownChange)
        peer->handleKeyUpOrDown (true);

    if (keyPressed)
        peer->handleKeyPress (keyCode, unicodeChar);
}

z0 XWindowSystem::handleKeyReleaseEvent (LinuxComponentPeer* peer, const XKeyEvent& keyEvent) const
{
    auto isKeyReleasePartOfAutoRepeat = [&]() -> b8
    {
        if (X11Symbols::getInstance()->xPending (display))
        {
            XEvent e;
            X11Symbols::getInstance()->xPeekEvent (display, &e);

            // Look for a subsequent key-down event with the same timestamp and keycode
            return e.type           == KeyPressEventType
                  && e.xkey.keycode == keyEvent.keycode
                  && e.xkey.time    == keyEvent.time;
        }

        return false;
    }();

    if (! isKeyReleasePartOfAutoRepeat)
    {
        updateKeyStates ((i32) keyEvent.keycode, false);
        KeySym sym;

        {
            XWindowSystemUtilities::ScopedXLock xLock;
            sym = X11Symbols::getInstance()->xkbKeycodeToKeysym (display, (::KeyCode) keyEvent.keycode, 0, 0);
        }

        auto oldMods = ModifierKeys::currentModifiers;
        auto keyDownChange = (sym != NoSymbol) && ! updateKeyModifiersFromSym (sym, false);

        if (oldMods != ModifierKeys::currentModifiers)
            peer->handleModifierKeysChange();

        if (keyDownChange)
            peer->handleKeyUpOrDown (false);
    }
}

z0 XWindowSystem::handleWheelEvent (LinuxComponentPeer* peer, const XButtonPressedEvent& buttonPressEvent, f32 amount) const
{
    MouseWheelDetails wheel;
    wheel.deltaX = 0.0f;
    wheel.deltaY = amount;
    wheel.isReversed = false;
    wheel.isSmooth = false;
    wheel.isInertial = false;

    peer->handleMouseWheel (MouseInputSource::InputSourceType::mouse, getLogicalMousePos (buttonPressEvent, peer->getPlatformScaleFactor()),
                            getEventTime (buttonPressEvent), wheel);
}

z0 XWindowSystem::handleButtonPressEvent (LinuxComponentPeer* peer, const XButtonPressedEvent& buttonPressEvent, i32 buttonModifierFlag) const
{
    ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (buttonModifierFlag);
    peer->toFront (true);
    peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse, getLogicalMousePos (buttonPressEvent, peer->getPlatformScaleFactor()),
                            ModifierKeys::currentModifiers, MouseInputSource::defaultPressure,
                            MouseInputSource::defaultOrientation, getEventTime (buttonPressEvent), {});
}

z0 XWindowSystem::handleButtonPressEvent (LinuxComponentPeer* peer, const XButtonPressedEvent& buttonPressEvent) const
{
    updateKeyModifiers ((i32) buttonPressEvent.state);

    auto mapIndex = (u32) (buttonPressEvent.button - Button1);

    if (mapIndex < (u32) numElementsInArray (pointerMap))
    {
        switch (pointerMap[mapIndex])
        {
            case Keys::WheelUp:         handleWheelEvent (peer, buttonPressEvent,  50.0f / 256.0f); break;
            case Keys::WheelDown:       handleWheelEvent (peer, buttonPressEvent, -50.0f / 256.0f); break;
            case Keys::LeftButton:      handleButtonPressEvent (peer, buttonPressEvent, ModifierKeys::leftButtonModifier); break;
            case Keys::RightButton:     handleButtonPressEvent (peer, buttonPressEvent, ModifierKeys::rightButtonModifier); break;
            case Keys::MiddleButton:    handleButtonPressEvent (peer, buttonPressEvent, ModifierKeys::middleButtonModifier); break;
            default: break;
        }
    }
}

z0 XWindowSystem::handleButtonReleaseEvent (LinuxComponentPeer* peer, const XButtonReleasedEvent& buttonRelEvent) const
{
    updateKeyModifiers ((i32) buttonRelEvent.state);

    if (peer->getParentWindow() != 0)
        peer->updateWindowBounds();

    auto mapIndex = (u32) (buttonRelEvent.button - Button1);

    if (mapIndex < (u32) numElementsInArray (pointerMap))
    {
        switch (pointerMap[mapIndex])
        {
            case Keys::LeftButton:      ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutFlags (ModifierKeys::leftButtonModifier);   break;
            case Keys::RightButton:     ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutFlags (ModifierKeys::rightButtonModifier);  break;
            case Keys::MiddleButton:    ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutFlags (ModifierKeys::middleButtonModifier); break;
            default: break;
        }
    }

    auto& dragState = dragAndDropStateMap[peer];

    if (dragState.isDragging())
        dragState.handleExternalDragButtonReleaseEvent();

    peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse, getLogicalMousePos (buttonRelEvent, peer->getPlatformScaleFactor()),
                            ModifierKeys::currentModifiers, MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation, getEventTime (buttonRelEvent));
}

z0 XWindowSystem::handleMotionNotifyEvent (LinuxComponentPeer* peer, const XPointerMovedEvent& movedEvent) const
{
    updateKeyModifiers ((i32) movedEvent.state);
    Keys::refreshStaleMouseKeys();

    auto& dragState = dragAndDropStateMap[peer];

    if (dragState.isDragging())
        dragState.handleExternalDragMotionNotify();

    peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse, getLogicalMousePos (movedEvent, peer->getPlatformScaleFactor()),
                            ModifierKeys::currentModifiers, MouseInputSource::defaultPressure,
                            MouseInputSource::defaultOrientation, getEventTime (movedEvent));
}

z0 XWindowSystem::handleEnterNotifyEvent (LinuxComponentPeer* peer, const XEnterWindowEvent& enterEvent) const
{
    if (peer->getParentWindow() != 0)
        peer->updateWindowBounds();

    if (! ModifierKeys::currentModifiers.isAnyMouseButtonDown())
    {
        updateKeyModifiers ((i32) enterEvent.state);
        peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse, getLogicalMousePos (enterEvent, peer->getPlatformScaleFactor()),
                                ModifierKeys::currentModifiers, MouseInputSource::defaultPressure,
                                MouseInputSource::defaultOrientation, getEventTime (enterEvent));
    }
}

z0 XWindowSystem::handleLeaveNotifyEvent (LinuxComponentPeer* peer, const XLeaveWindowEvent& leaveEvent) const
{
    // Suppress the normal leave if we've got a pointer grab, or if
    // it's a bogus one caused by clicking a mouse button when running
    // in a Window manager
    if (((! ModifierKeys::currentModifiers.isAnyMouseButtonDown()) && leaveEvent.mode == NotifyNormal)
         || leaveEvent.mode == NotifyUngrab)
    {
        updateKeyModifiers ((i32) leaveEvent.state);
        peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse, getLogicalMousePos (leaveEvent, peer->getPlatformScaleFactor()),
                                ModifierKeys::currentModifiers, MouseInputSource::defaultPressure,
                                MouseInputSource::defaultOrientation, getEventTime (leaveEvent));
    }
}

z0 XWindowSystem::handleFocusInEvent (LinuxComponentPeer* peer) const
{
    peer->isActiveApplication = true;

    if (isFocused ((::Window) peer->getNativeHandle()) && ! peer->focused)
    {
        peer->focused = true;
        peer->handleFocusGain();
    }
}

z0 XWindowSystem::handleFocusOutEvent (LinuxComponentPeer* peer) const
{
    if (! isFocused ((::Window) peer->getNativeHandle()) && peer->focused)
    {
        peer->focused = false;
        peer->isActiveApplication = false;

        peer->handleFocusLoss();
    }
}

z0 XWindowSystem::handleExposeEvent (LinuxComponentPeer* peer, XExposeEvent& exposeEvent) const
{
    // Batch together all pending expose events
    XEvent nextEvent;
    XWindowSystemUtilities::ScopedXLock xLock;

    // if we have opengl contexts then just repaint them all
    // regardless if this is really necessary
    peer->repaintOpenGLContexts();

    auto windowH = (::Window) peer->getNativeHandle();

    if (exposeEvent.window != windowH)
    {
        Window child;
        X11Symbols::getInstance()->xTranslateCoordinates (display, exposeEvent.window, windowH,
                                                          exposeEvent.x, exposeEvent.y, &exposeEvent.x, &exposeEvent.y,
                                                          &child);
    }

    // exposeEvent is in local window local coordinates so do not convert with
    // physicalToScaled, but rather use currentScaleFactor
    auto currentScaleFactor = peer->getPlatformScaleFactor();

    peer->repaint (Rectangle<i32> (exposeEvent.x, exposeEvent.y,
                                   exposeEvent.width, exposeEvent.height) / currentScaleFactor);

    while (X11Symbols::getInstance()->xEventsQueued (display, QueuedAfterFlush) > 0)
    {
        X11Symbols::getInstance()->xPeekEvent (display, &nextEvent);

        if (nextEvent.type != Expose || nextEvent.xany.window != exposeEvent.window)
            break;

        X11Symbols::getInstance()->xNextEvent (display, &nextEvent);
        auto& nextExposeEvent = (XExposeEvent&) nextEvent.xexpose;

        peer->repaint (Rectangle<i32> (nextExposeEvent.x, nextExposeEvent.y,
                                       nextExposeEvent.width, nextExposeEvent.height) / currentScaleFactor);
    }
}

z0 XWindowSystem::dismissBlockingModals (LinuxComponentPeer* peer) const
{
    if (peer->getComponent().isCurrentlyBlockedByAnotherModalComponent())
        if (auto* currentModalComp = Component::getCurrentlyModalComponent())
            if (auto* otherPeer = currentModalComp->getPeer())
                if ((otherPeer->getStyleFlags() & ComponentPeer::windowIsTemporary) != 0)
                    currentModalComp->inputAttemptWhenModal();
}

z0 XWindowSystem::handleConfigureNotifyEvent (LinuxComponentPeer* peer, XConfigureEvent& confEvent) const
{
    const ScopedValueSetter<b8> scope { peer->inConfigureNotifyHandler, true };

    peer->updateWindowBounds();
    peer->updateBorderSize();
    peer->handleMovedOrResized();

    // if the native title bar is dragged, need to tell any active menus, etc.
    if ((peer->getStyleFlags() & ComponentPeer::windowHasTitleBar) != 0)
        dismissBlockingModals (peer);

    auto windowH = (::Window) peer->getNativeHandle();

    if (confEvent.window == windowH && confEvent.above != 0 && isFrontWindow (windowH))
        peer->handleBroughtToFront();
}

z0 XWindowSystem::handleGravityNotify (LinuxComponentPeer* peer) const
{
    peer->updateWindowBounds();
    peer->updateBorderSize();
    peer->handleMovedOrResized();
}

b8 XWindowSystem::isIconic (Window w) const
{
    jassert (w != 0);

    XWindowSystemUtilities::ScopedXLock xLock;
    XWindowSystemUtilities::GetXProperty prop (display, w, atoms.state, 0, 64, false, atoms.state);

    if (prop.success && prop.actualType == atoms.state
        && prop.actualFormat == 32 && prop.numItems > 0)
    {
        u64 state;
        memcpy (&state, prop.data, sizeof (u64));

        return state == IconicState;
    }

    return false;
}

b8 XWindowSystem::isHidden (Window w) const
{
    XWindowSystemUtilities::ScopedXLock xLock;
    XWindowSystemUtilities::GetXProperty prop (display, w, atoms.windowState, 0, 128, false, XA_ATOM);

    if (! (prop.success && prop.actualFormat == 32 && prop.actualType == XA_ATOM))
        return false;

    const auto* data = unalignedPointerCast<const i64*> (prop.data);
    const auto end = data + prop.numItems;

    return std::find (data, end, atoms.windowStateHidden) != end;
}

z0 XWindowSystem::propertyNotifyEvent (LinuxComponentPeer* peer, const XPropertyEvent& event) const
{
    if ((event.atom == atoms.state && isIconic (event.window))
        || (event.atom == atoms.windowState && isHidden (event.window)))
    {
        dismissBlockingModals (peer);
    }

    if (event.atom == XWindowSystemUtilities::Atoms::getIfExists (display, "_NET_FRAME_EXTENTS"))
        peer->updateBorderSize();
}

z0 XWindowSystem::handleMappingNotify (XMappingEvent& mappingEvent) const
{
    if (mappingEvent.request != MappingPointer)
    {
        // Deal with modifier/keyboard mapping
        XWindowSystemUtilities::ScopedXLock xLock;
        X11Symbols::getInstance()->xRefreshKeyboardMapping (&mappingEvent);
        updateModifierMappings();
    }
}

z0 XWindowSystem::handleClientMessageEvent (LinuxComponentPeer* peer, XClientMessageEvent& clientMsg, XEvent& event) const
{
    if (clientMsg.message_type == atoms.protocols && clientMsg.format == 32)
    {
        auto atom = (Atom) clientMsg.data.l[0];

        if (atom == atoms.protocolList [XWindowSystemUtilities::Atoms::PING])
        {
            auto root = X11Symbols::getInstance()->xRootWindow (display, X11Symbols::getInstance()->xDefaultScreen (display));

            clientMsg.window = root;

            X11Symbols::getInstance()->xSendEvent (display, root, False, NoEventMask, &event);
            X11Symbols::getInstance()->xFlush (display);
        }
        else if (atom == atoms.protocolList [XWindowSystemUtilities::Atoms::TAKE_FOCUS])
        {
            if ((peer->getStyleFlags() & ComponentPeer::windowIgnoresKeyPresses) == 0)
            {
                XWindowAttributes atts;

                XWindowSystemUtilities::ScopedXLock xLock;
                if (clientMsg.window != 0
                     && X11Symbols::getInstance()->xGetWindowAttributes (display, clientMsg.window, &atts))
                {
                    if (atts.map_state == IsViewable)
                    {
                        auto windowH = (::Window) peer->getNativeHandle();

                        X11Symbols::getInstance()->xSetInputFocus (display, (clientMsg.window == windowH ? getFocusWindow (windowH)
                                                                                                         : clientMsg.window),
                                                                   RevertToParent, (::Time) clientMsg.data.l[1]);
                    }
                }
            }
        }
        else if (atom == atoms.protocolList [XWindowSystemUtilities::Atoms::DELETE_WINDOW])
        {
            peer->handleUserClosingWindow();
        }
    }
    else if (clientMsg.message_type == atoms.XdndEnter)
    {
        dragAndDropStateMap[peer].handleDragAndDropEnter (clientMsg, peer);
    }
    else if (clientMsg.message_type == atoms.XdndLeave)
    {
        dragAndDropStateMap[peer].handleDragAndDropExit();
    }
    else if (clientMsg.message_type == atoms.XdndPosition)
    {
        dragAndDropStateMap[peer].handleDragAndDropPosition (clientMsg, peer);
    }
    else if (clientMsg.message_type == atoms.XdndDrop)
    {
        dragAndDropStateMap[peer].handleDragAndDropDrop (clientMsg, peer);
    }
    else if (clientMsg.message_type == atoms.XdndStatus)
    {
        dragAndDropStateMap[peer].handleExternalDragAndDropStatus (clientMsg);
    }
    else if (clientMsg.message_type == atoms.XdndFinished)
    {
        dragAndDropStateMap[peer].externalResetDragAndDrop();
    }
    else if (clientMsg.message_type == atoms.XembedMsgType && clientMsg.format == 32)
    {
        handleXEmbedMessage (peer, clientMsg);
    }
}

z0 XWindowSystem::handleXEmbedMessage (LinuxComponentPeer* peer, XClientMessageEvent& clientMsg) const
{
    switch (clientMsg.data.l[1])
    {
        case 0:   // XEMBED_EMBEDDED_NOTIFY
            peer->setParentWindow ((::Window) clientMsg.data.l[3]);
            peer->updateWindowBounds();
            peer->getComponent().setBounds (peer->getBounds());
            break;
        case 4:   // XEMBED_FOCUS_IN
            handleFocusInEvent (peer);
            break;
        case 5:   // XEMBED_FOCUS_OUT
            handleFocusOutEvent (peer);
            break;

        default:
            break;
    }
}

//==============================================================================
z0 XWindowSystem::dismissBlockingModals (LinuxComponentPeer* peer, const XConfigureEvent& configure) const
{
    if (peer == nullptr)
        return;

    const auto peerHandle = peer->getWindowHandle();

    if (configure.window != peerHandle && isParentWindowOf (configure.window, peerHandle))
        dismissBlockingModals (peer);
}

z0 XWindowSystem::windowMessageReceive (XEvent& event)
{
    if (event.xany.window != None)
    {
       #if DRX_X11_SUPPORTS_XEMBED
        if (! drx_handleXEmbedEvent (nullptr, &event))
       #endif
        {
            auto* instance = XWindowSystem::getInstance();

            if (auto* xSettings = instance->getXSettings())
            {
                if (event.xany.window == xSettings->getSettingsWindow())
                {
                    if (event.xany.type == PropertyNotify)
                        xSettings->update();
                    else if (event.xany.type == DestroyNotify)
                        instance->initialiseXSettings();

                    return;
                }
            }

            if (auto* peer = dynamic_cast<LinuxComponentPeer*> (getPeerFor (event.xany.window)))
            {
                XWindowSystem::getInstance()->handleWindowMessage (peer, event);
                return;
            }

            if (event.type != ConfigureNotify)
                return;

            for (auto i = ComponentPeer::getNumPeers(); --i >= 0;)
                instance->dismissBlockingModals (dynamic_cast<LinuxComponentPeer*> (ComponentPeer::getPeer (i)),
                                                 event.xconfigure);
        }
    }
    else if (event.xany.type == KeymapNotify)
    {
        auto& keymapEvent = (const XKeymapEvent&) event.xkeymap;
        memcpy (Keys::keyStates, keymapEvent.key_vector, 32);
    }
}

//==============================================================================
Image createSnapshotOfNativeWindow (uk window)
{
    ::Window root;
    i32 wx, wy;
    u32 ww, wh, bw, bitDepth;

    XWindowSystemUtilities::ScopedXLock xLock;

    const auto display = XWindowSystem::getInstance()->getDisplay();

    if (! X11Symbols::getInstance()->xGetGeometry (display, (::Drawable) window, &root, &wx, &wy, &ww, &wh, &bw, &bitDepth))
        return {};

    const auto scale = []
    {
        if (auto* d = Desktop::getInstance().getDisplays().getPrimaryDisplay())
            return d->scale;

        return 1.0;
    }();

    auto image = Image { new XBitmapImage { X11Symbols::getInstance()->xGetImage (display,
                                                                                  (::Drawable) window,
                                                                                  0,
                                                                                  0,
                                                                                  ww,
                                                                                  wh,
                                                                                  AllPlanes,
                                                                                  ZPixmap) } };
    return image.rescaled ((i32) ((f64) ww / scale), (i32) ((f64) wh / scale));
}

} // namespace drx
