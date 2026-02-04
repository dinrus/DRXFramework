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

#if DRX_MODULE_AVAILABLE_drx_audio_plugin_client
 #include <drx_audio_plugin_client/AAX/drx_AAX_Modifier_Injector.h>
#endif

namespace drx
{

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-function-type")

#undef GetSystemMetrics // multimon overrides this for some reason and causes a mess..

// these are in the windows SDK, but need to be repeated here for GCC..
#ifndef GET_APPCOMMAND_LPARAM
 #define GET_APPCOMMAND_LPARAM(lParam)     ((short) (HIWORD (lParam) & ~FAPPCOMMAND_MASK))

 #define FAPPCOMMAND_MASK                  0xF000
 #define APPCOMMAND_MEDIA_NEXTTRACK        11
 #define APPCOMMAND_MEDIA_PREVIOUSTRACK    12
 #define APPCOMMAND_MEDIA_STOP             13
 #define APPCOMMAND_MEDIA_PLAY_PAUSE       14
#endif

#ifndef WM_APPCOMMAND
 #define WM_APPCOMMAND                     0x0319
#endif

z0 drx_repeatLastProcessPriority();

using CheckEventBlockedByModalComps = b8 (*) (const MSG&);
extern CheckEventBlockedByModalComps isEventBlockedByModalComps;

static b8 shouldDeactivateTitleBar = true;

uk getUser32Function (tukk);

#if DRX_DEBUG
 i32 numActiveScopedDpiAwarenessDisablers = 0;
 extern HWND drx_messageWindowHandle;
#endif

struct ScopedDeviceContext
{
    explicit ScopedDeviceContext (HWND h)
        : hwnd (h), dc (GetDC (hwnd))
    {
    }

    ~ScopedDeviceContext()
    {
        ReleaseDC (hwnd, dc);
    }

    HWND hwnd;
    HDC dc;

    DRX_DECLARE_NON_COPYABLE (ScopedDeviceContext)
    DRX_DECLARE_NON_MOVEABLE (ScopedDeviceContext)
};

//==============================================================================
#ifndef WM_TOUCH
 enum
 {
     WM_TOUCH         = 0x0240,
     TOUCHEVENTF_MOVE = 0x0001,
     TOUCHEVENTF_DOWN = 0x0002,
     TOUCHEVENTF_UP   = 0x0004
 };

 typedef HANDLE HTOUCHINPUT;
 typedef HANDLE HGESTUREINFO;

 struct TOUCHINPUT
 {
     LONG         x;
     LONG         y;
     HANDLE       hSource;
     DWORD        dwID;
     DWORD        dwFlags;
     DWORD        dwMask;
     DWORD        dwTime;
     ULONG_PTR    dwExtraInfo;
     DWORD        cxContact;
     DWORD        cyContact;
 };

 struct GESTUREINFO
 {
     UINT         cbSize;
     DWORD        dwFlags;
     DWORD        dwID;
     HWND         hwndTarget;
     POINTS       ptsLocation;
     DWORD        dwInstanceID;
     DWORD        dwSequenceID;
     ULONGLONG    ullArguments;
     UINT         cbExtraArgs;
 };
#endif

#ifndef WM_NCPOINTERUPDATE
 enum
 {
     WM_NCPOINTERUPDATE       = 0x241,
     WM_NCPOINTERDOWN         = 0x242,
     WM_NCPOINTERUP           = 0x243,
     WM_POINTERUPDATE         = 0x245,
     WM_POINTERDOWN           = 0x246,
     WM_POINTERUP             = 0x247,
     WM_POINTERENTER          = 0x249,
     WM_POINTERLEAVE          = 0x24A,
     WM_POINTERACTIVATE       = 0x24B,
     WM_POINTERCAPTURECHANGED = 0x24C,
     WM_TOUCHHITTESTING       = 0x24D,
     WM_POINTERWHEEL          = 0x24E,
     WM_POINTERHWHEEL         = 0x24F,
     WM_POINTERHITTEST        = 0x250
 };

 enum
 {
     PT_TOUCH = 0x00000002,
     PT_PEN   = 0x00000003
 };

 enum POINTER_BUTTON_CHANGE_TYPE
 {
     POINTER_CHANGE_NONE,
     POINTER_CHANGE_FIRSTBUTTON_DOWN,
     POINTER_CHANGE_FIRSTBUTTON_UP,
     POINTER_CHANGE_SECONDBUTTON_DOWN,
     POINTER_CHANGE_SECONDBUTTON_UP,
     POINTER_CHANGE_THIRDBUTTON_DOWN,
     POINTER_CHANGE_THIRDBUTTON_UP,
     POINTER_CHANGE_FOURTHBUTTON_DOWN,
     POINTER_CHANGE_FOURTHBUTTON_UP,
     POINTER_CHANGE_FIFTHBUTTON_DOWN,
     POINTER_CHANGE_FIFTHBUTTON_UP
 };

 enum
 {
     PEN_MASK_NONE      = 0x00000000,
     PEN_MASK_PRESSURE  = 0x00000001,
     PEN_MASK_ROTATION  = 0x00000002,
     PEN_MASK_TILT_X    = 0x00000004,
     PEN_MASK_TILT_Y    = 0x00000008
 };

 enum
 {
     TOUCH_MASK_NONE        = 0x00000000,
     TOUCH_MASK_CONTACTAREA = 0x00000001,
     TOUCH_MASK_ORIENTATION = 0x00000002,
     TOUCH_MASK_PRESSURE    = 0x00000004
 };

 enum
 {
     POINTER_FLAG_NONE           = 0x00000000,
     POINTER_FLAG_NEW            = 0x00000001,
     POINTER_FLAG_INRANGE        = 0x00000002,
     POINTER_FLAG_INCONTACT      = 0x00000004,
     POINTER_FLAG_FIRSTBUTTON    = 0x00000010,
     POINTER_FLAG_SECONDBUTTON   = 0x00000020,
     POINTER_FLAG_THIRDBUTTON    = 0x00000040,
     POINTER_FLAG_FOURTHBUTTON   = 0x00000080,
     POINTER_FLAG_FIFTHBUTTON    = 0x00000100,
     POINTER_FLAG_PRIMARY        = 0x00002000,
     POINTER_FLAG_CONFIDENCE     = 0x00004000,
     POINTER_FLAG_CANCELED       = 0x00008000,
     POINTER_FLAG_DOWN           = 0x00010000,
     POINTER_FLAG_UPDATE         = 0x00020000,
     POINTER_FLAG_UP             = 0x00040000,
     POINTER_FLAG_WHEEL          = 0x00080000,
     POINTER_FLAG_HWHEEL         = 0x00100000,
     POINTER_FLAG_CAPTURECHANGED = 0x00200000,
     POINTER_FLAG_HASTRANSFORM   = 0x00400000
 };

 typedef DWORD  POINTER_INPUT_TYPE;
 typedef UINT32 POINTER_FLAGS;
 typedef UINT32 PEN_FLAGS;
 typedef UINT32 PEN_MASK;
 typedef UINT32 TOUCH_FLAGS;
 typedef UINT32 TOUCH_MASK;

 struct POINTER_INFO
 {
     POINTER_INPUT_TYPE         pointerType;
     UINT32                     pointerId;
     UINT32                     frameId;
     POINTER_FLAGS              pointerFlags;
     HANDLE                     sourceDevice;
     HWND                       hwndTarget;
     POINT                      ptPixelLocation;
     POINT                      ptHimetricLocation;
     POINT                      ptPixelLocationRaw;
     POINT                      ptHimetricLocationRaw;
     DWORD                      dwTime;
     UINT32                     historyCount;
     INT32                      InputData;
     DWORD                      dwKeyStates;
     UINT64                     PerformanceCount;
     POINTER_BUTTON_CHANGE_TYPE ButtonChangeType;
 };

 struct POINTER_TOUCH_INFO
 {
     POINTER_INFO    pointerInfo;
     TOUCH_FLAGS     touchFlags;
     TOUCH_MASK      touchMask;
     RECT            rcContact;
     RECT            rcContactRaw;
     UINT32          orientation;
     UINT32          pressure;
 };

 struct POINTER_PEN_INFO
 {
     POINTER_INFO    pointerInfo;
     PEN_FLAGS       penFlags;
     PEN_MASK        penMask;
     UINT32          pressure;
     UINT32          rotation;
     INT32           tiltX;
     INT32           tiltY;
 };

 #define GET_POINTERID_WPARAM(wParam)    (LOWORD(wParam))
#endif

#ifndef MONITOR_DPI_TYPE
 enum Monitor_DPI_Type
 {
     MDT_Effective_DPI = 0,
     MDT_Angular_DPI   = 1,
     MDT_Raw_DPI       = 2,
     MDT_Default       = MDT_Effective_DPI
 };
#endif

#ifndef DPI_AWARENESS
 enum DPI_Awareness
 {
     DPI_Awareness_Invalid           = -1,
     DPI_Awareness_Unaware           = 0,
     DPI_Awareness_System_Aware      = 1,
     DPI_Awareness_Per_Monitor_Aware = 2
 };
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
 #define USER_DEFAULT_SCREEN_DPI 96
#endif

#ifndef _DPI_AWARENESS_CONTEXTS_
 typedef HANDLE DPI_AWARENESS_CONTEXT;

 #define DPI_AWARENESS_CONTEXT_UNAWARE              ((DPI_AWARENESS_CONTEXT) - 1)
 #define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((DPI_AWARENESS_CONTEXT) - 2)
 #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((DPI_AWARENESS_CONTEXT) - 3)
 #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT) - 4)
#endif

// Some versions of the Windows 10 SDK define _DPI_AWARENESS_CONTEXTS_ but not
// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
 #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT) - 4)
#endif

//==============================================================================
using RegisterTouchWindowFunc    = BOOL (WINAPI*) (HWND, ULONG);
using GetTouchInputInfoFunc      = BOOL (WINAPI*) (HTOUCHINPUT, UINT, TOUCHINPUT*, i32);
using CloseTouchInputHandleFunc  = BOOL (WINAPI*) (HTOUCHINPUT);
using GetGestureInfoFunc         = BOOL (WINAPI*) (HGESTUREINFO, GESTUREINFO*);

static RegisterTouchWindowFunc   registerTouchWindow   = nullptr;
static GetTouchInputInfoFunc     getTouchInputInfo     = nullptr;
static CloseTouchInputHandleFunc closeTouchInputHandle = nullptr;
static GetGestureInfoFunc        getGestureInfo        = nullptr;

static b8 hasCheckedForMultiTouch = false;

static b8 canUseMultiTouch()
{
    if (registerTouchWindow == nullptr && ! hasCheckedForMultiTouch)
    {
        hasCheckedForMultiTouch = true;

        registerTouchWindow   = (RegisterTouchWindowFunc)   getUser32Function ("RegisterTouchWindow");
        getTouchInputInfo     = (GetTouchInputInfoFunc)     getUser32Function ("GetTouchInputInfo");
        closeTouchInputHandle = (CloseTouchInputHandleFunc) getUser32Function ("CloseTouchInputHandle");
        getGestureInfo        = (GetGestureInfoFunc)        getUser32Function ("GetGestureInfo");
    }

    return registerTouchWindow != nullptr;
}

//==============================================================================
using GetPointerTypeFunc       =  BOOL (WINAPI*) (UINT32, POINTER_INPUT_TYPE*);
using GetPointerTouchInfoFunc  =  BOOL (WINAPI*) (UINT32, POINTER_TOUCH_INFO*);
using GetPointerPenInfoFunc    =  BOOL (WINAPI*) (UINT32, POINTER_PEN_INFO*);

static GetPointerTypeFunc      getPointerTypeFunction = nullptr;
static GetPointerTouchInfoFunc getPointerTouchInfo    = nullptr;
static GetPointerPenInfoFunc   getPointerPenInfo      = nullptr;

static b8 canUsePointerAPI = false;

static z0 checkForPointerAPI()
{
    getPointerTypeFunction = (GetPointerTypeFunc)      getUser32Function ("GetPointerType");
    getPointerTouchInfo    = (GetPointerTouchInfoFunc) getUser32Function ("GetPointerTouchInfo");
    getPointerPenInfo      = (GetPointerPenInfoFunc)   getUser32Function ("GetPointerPenInfo");

    canUsePointerAPI = (getPointerTypeFunction != nullptr
                     && getPointerTouchInfo    != nullptr
                     && getPointerPenInfo      != nullptr);
}

//==============================================================================
using SetProcessDPIAwareFunc                   = BOOL                  (WINAPI*) ();
using SetProcessDPIAwarenessContextFunc        = BOOL                  (WINAPI*) (DPI_AWARENESS_CONTEXT);
using SetProcessDPIAwarenessFunc               = HRESULT               (WINAPI*) (DPI_Awareness);
using SetThreadDPIAwarenessContextFunc         = DPI_AWARENESS_CONTEXT (WINAPI*) (DPI_AWARENESS_CONTEXT);
using GetDPIForWindowFunc                      = UINT                  (WINAPI*) (HWND);
using GetDPIForMonitorFunc                     = HRESULT               (WINAPI*) (HMONITOR, Monitor_DPI_Type, UINT*, UINT*);
using GetSystemMetricsForDpiFunc               = i32                   (WINAPI*) (i32, UINT);
using GetProcessDPIAwarenessFunc               = HRESULT               (WINAPI*) (HANDLE, DPI_Awareness*);
using GetWindowDPIAwarenessContextFunc         = DPI_AWARENESS_CONTEXT (WINAPI*) (HWND);
using GetThreadDPIAwarenessContextFunc         = DPI_AWARENESS_CONTEXT (WINAPI*) ();
using GetAwarenessFromDpiAwarenessContextFunc  = DPI_Awareness         (WINAPI*) (DPI_AWARENESS_CONTEXT);
using EnableNonClientDPIScalingFunc            = BOOL                  (WINAPI*) (HWND);

static SetProcessDPIAwareFunc                  setProcessDPIAware                  = nullptr;
static SetProcessDPIAwarenessContextFunc       setProcessDPIAwarenessContext       = nullptr;
static SetProcessDPIAwarenessFunc              setProcessDPIAwareness              = nullptr;
static SetThreadDPIAwarenessContextFunc        setThreadDPIAwarenessContext        = nullptr;
static GetDPIForMonitorFunc                    getDPIForMonitor                    = nullptr;
static GetDPIForWindowFunc                     getDPIForWindow                     = nullptr;
static GetProcessDPIAwarenessFunc              getProcessDPIAwareness              = nullptr;
static GetWindowDPIAwarenessContextFunc        getWindowDPIAwarenessContext        = nullptr;
static GetThreadDPIAwarenessContextFunc        getThreadDPIAwarenessContext        = nullptr;
static GetAwarenessFromDpiAwarenessContextFunc getAwarenessFromDPIAwarenessContext = nullptr;
static EnableNonClientDPIScalingFunc           enableNonClientDPIScaling           = nullptr;

static b8 hasCheckedForDPIAwareness = false;

static z0 loadDPIAwarenessFunctions()
{
    setProcessDPIAware = (SetProcessDPIAwareFunc) getUser32Function ("SetProcessDPIAware");

    constexpr auto shcore = "SHCore.dll";
    LoadLibraryA (shcore);
    const auto shcoreModule = GetModuleHandleA (shcore);

    if (shcoreModule == nullptr)
        return;

    getDPIForMonitor                    = (GetDPIForMonitorFunc) GetProcAddress (shcoreModule, "GetDpiForMonitor");
    setProcessDPIAwareness              = (SetProcessDPIAwarenessFunc) GetProcAddress (shcoreModule, "SetProcessDpiAwareness");

   #if DRX_WIN_PER_MONITOR_DPI_AWARE
    getDPIForWindow                     = (GetDPIForWindowFunc) getUser32Function ("GetDpiForWindow");
    getProcessDPIAwareness              = (GetProcessDPIAwarenessFunc) GetProcAddress (shcoreModule, "GetProcessDpiAwareness");
    getWindowDPIAwarenessContext        = (GetWindowDPIAwarenessContextFunc) getUser32Function ("GetWindowDpiAwarenessContext");
    setThreadDPIAwarenessContext        = (SetThreadDPIAwarenessContextFunc) getUser32Function ("SetThreadDpiAwarenessContext");
    getThreadDPIAwarenessContext        = (GetThreadDPIAwarenessContextFunc) getUser32Function ("GetThreadDpiAwarenessContext");
    getAwarenessFromDPIAwarenessContext = (GetAwarenessFromDpiAwarenessContextFunc) getUser32Function ("GetAwarenessFromDpiAwarenessContext");
    setProcessDPIAwarenessContext       = (SetProcessDPIAwarenessContextFunc) getUser32Function ("SetProcessDpiAwarenessContext");
    enableNonClientDPIScaling           = (EnableNonClientDPIScalingFunc) getUser32Function ("EnableNonClientDpiScaling");
   #endif
}

static z0 setDPIAwareness()
{
    if (hasCheckedForDPIAwareness)
        return;

    hasCheckedForDPIAwareness = true;

    if (! DRXApplicationBase::isStandaloneApp())
        return;

    loadDPIAwarenessFunctions();

    if (setProcessDPIAwarenessContext != nullptr
        && setProcessDPIAwarenessContext (DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
        return;

    if (setProcessDPIAwareness != nullptr && enableNonClientDPIScaling != nullptr
        && SUCCEEDED (setProcessDPIAwareness (DPI_Awareness::DPI_Awareness_Per_Monitor_Aware)))
        return;

    if (setProcessDPIAwareness != nullptr && getDPIForMonitor != nullptr
        && SUCCEEDED (setProcessDPIAwareness (DPI_Awareness::DPI_Awareness_System_Aware)))
        return;

    NullCheckedInvocation::invoke (setProcessDPIAware);
}

static b8 isPerMonitorDPIAwareProcess()
{
   #if ! DRX_WIN_PER_MONITOR_DPI_AWARE
    return false;
   #else
    static b8 dpiAware = []() -> b8
    {
        setDPIAwareness();

        if (! DRXApplication::isStandaloneApp())
            return false;

        if (getProcessDPIAwareness == nullptr)
            return false;

        DPI_Awareness context;
        getProcessDPIAwareness (nullptr, &context);

        return context == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware;
    }();

    return dpiAware;
   #endif
}

static b8 isPerMonitorDPIAwareWindow ([[maybe_unused]] HWND nativeWindow)
{
   #if ! DRX_WIN_PER_MONITOR_DPI_AWARE
    return false;
   #else
    setDPIAwareness();

    if (getWindowDPIAwarenessContext != nullptr
        && getAwarenessFromDPIAwarenessContext != nullptr)
    {
        return (getAwarenessFromDPIAwarenessContext (getWindowDPIAwarenessContext (nativeWindow))
                  == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware);
    }

    return isPerMonitorDPIAwareProcess();
   #endif
}

static b8 isPerMonitorDPIAwareThread (GetThreadDPIAwarenessContextFunc getThreadDPIAwarenessContextIn = getThreadDPIAwarenessContext,
                                        GetAwarenessFromDpiAwarenessContextFunc getAwarenessFromDPIAwarenessContextIn = getAwarenessFromDPIAwarenessContext)
{
   #if ! DRX_WIN_PER_MONITOR_DPI_AWARE
    return false;
   #else
    setDPIAwareness();

    if (getThreadDPIAwarenessContextIn != nullptr
        && getAwarenessFromDPIAwarenessContextIn != nullptr)
    {
        return (getAwarenessFromDPIAwarenessContextIn (getThreadDPIAwarenessContextIn())
                  == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware);
    }

    return isPerMonitorDPIAwareProcess();
   #endif
}

static f64 getGlobalDPI()
{
    setDPIAwareness();

    ScopedDeviceContext deviceContext { nullptr };
    return (GetDeviceCaps (deviceContext.dc, LOGPIXELSX) + GetDeviceCaps (deviceContext.dc, LOGPIXELSY)) / 2.0;
}

//==============================================================================
class ScopedSuspendResumeNotificationRegistration
{
    static auto& getFunctions()
    {
        struct Functions
        {
            using Register = HPOWERNOTIFY (WINAPI*) (HANDLE, DWORD);
            using Unregister = BOOL (WINAPI*) (HPOWERNOTIFY);

            Register registerNotification = (Register) getUser32Function ("RegisterSuspendResumeNotification");
            Unregister unregisterNotification = (Unregister) getUser32Function ("UnregisterSuspendResumeNotification");

            b8 isValid() const { return registerNotification != nullptr && unregisterNotification != nullptr; }

            Functions() = default;
            DRX_DECLARE_NON_COPYABLE (Functions)
            DRX_DECLARE_NON_MOVEABLE (Functions)
        };

        static const Functions functions;
        return functions;
    }

public:
    ScopedSuspendResumeNotificationRegistration() = default;

    explicit ScopedSuspendResumeNotificationRegistration (HWND window)
        : handle (getFunctions().isValid()
                      ? getFunctions().registerNotification (window, DEVICE_NOTIFY_WINDOW_HANDLE)
                      : nullptr)
    {}

private:
    struct Destructor
    {
        z0 operator() (HPOWERNOTIFY ptr) const
        {
            if (ptr != nullptr)
                getFunctions().unregisterNotification (ptr);
        }
    };

    std::unique_ptr<std::remove_pointer_t<HPOWERNOTIFY>, Destructor> handle;
};

//==============================================================================
class ScopedThreadDPIAwarenessSetter::NativeImpl
{
public:
    static auto& getFunctions()
    {
        struct Functions
        {
            SetThreadDPIAwarenessContextFunc setThreadAwareness             = (SetThreadDPIAwarenessContextFunc) getUser32Function ("SetThreadDpiAwarenessContext");
            GetWindowDPIAwarenessContextFunc getWindowAwareness             = (GetWindowDPIAwarenessContextFunc) getUser32Function ("GetWindowDpiAwarenessContext");
            GetThreadDPIAwarenessContextFunc getThreadAwareness             = (GetThreadDPIAwarenessContextFunc) getUser32Function ("GetThreadDpiAwarenessContext");
            GetAwarenessFromDpiAwarenessContextFunc getAwarenessFromContext = (GetAwarenessFromDpiAwarenessContextFunc) getUser32Function ("GetAwarenessFromDpiAwarenessContext");

            b8 isLoaded() const noexcept
            {
                return setThreadAwareness != nullptr
                    && getWindowAwareness != nullptr
                    && getThreadAwareness != nullptr
                    && getAwarenessFromContext != nullptr;
            }

            Functions() = default;
            DRX_DECLARE_NON_COPYABLE (Functions)
            DRX_DECLARE_NON_MOVEABLE (Functions)
        };

        static const Functions functions;
        return functions;
    }

    explicit NativeImpl (HWND nativeWindow [[maybe_unused]])
    {
       #if DRX_WIN_PER_MONITOR_DPI_AWARE
        if (const auto& functions = getFunctions(); functions.isLoaded())
        {
            auto dpiAwareWindow = (functions.getAwarenessFromContext (functions.getWindowAwareness (nativeWindow))
                                   == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware);

            auto dpiAwareThread = (functions.getAwarenessFromContext (functions.getThreadAwareness())
                                   == DPI_Awareness::DPI_Awareness_Per_Monitor_Aware);

            if (dpiAwareWindow && ! dpiAwareThread)
                oldContext = functions.setThreadAwareness (DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
            else if (! dpiAwareWindow && dpiAwareThread)
                oldContext = functions.setThreadAwareness (DPI_AWARENESS_CONTEXT_UNAWARE);
        }
       #endif
    }

    ~NativeImpl()
    {
        if (oldContext != nullptr)
            getFunctions().setThreadAwareness (oldContext);
    }

private:
    DPI_AWARENESS_CONTEXT oldContext = nullptr;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeImpl)
    DRX_DECLARE_NON_MOVEABLE (NativeImpl)
};

ScopedThreadDPIAwarenessSetter::ScopedThreadDPIAwarenessSetter (uk nativeWindow)
{
    pimpl = std::make_unique<NativeImpl> ((HWND) nativeWindow);
}

ScopedThreadDPIAwarenessSetter::~ScopedThreadDPIAwarenessSetter() = default;

ScopedThreadDPIAwarenessSetter::ScopedThreadDPIAwarenessSetter (ScopedThreadDPIAwarenessSetter&&) noexcept = default;
ScopedThreadDPIAwarenessSetter& ScopedThreadDPIAwarenessSetter::operator= (ScopedThreadDPIAwarenessSetter&&) noexcept = default;

static auto& getScopedDPIAwarenessDisablerFunctions()
{
    struct Functions
    {
        GetThreadDPIAwarenessContextFunc localGetThreadDpiAwarenessContext = (GetThreadDPIAwarenessContextFunc) getUser32Function ("GetThreadDpiAwarenessContext");
        GetAwarenessFromDpiAwarenessContextFunc localGetAwarenessFromDpiAwarenessContextFunc = (GetAwarenessFromDpiAwarenessContextFunc) getUser32Function ("GetAwarenessFromDpiAwarenessContext");
        SetThreadDPIAwarenessContextFunc localSetThreadDPIAwarenessContext = (SetThreadDPIAwarenessContextFunc) getUser32Function ("SetThreadDpiAwarenessContext");

        Functions() = default;
        DRX_DECLARE_NON_COPYABLE (Functions)
        DRX_DECLARE_NON_MOVEABLE (Functions)
    };

    static const Functions functions;
    return functions;
}

ScopedDPIAwarenessDisabler::ScopedDPIAwarenessDisabler()
{
    const auto& functions = getScopedDPIAwarenessDisablerFunctions();

    if (! isPerMonitorDPIAwareThread (functions.localGetThreadDpiAwarenessContext, functions.localGetAwarenessFromDpiAwarenessContextFunc))
        return;

    if (auto* localSetThreadDPIAwarenessContext = functions.localSetThreadDPIAwarenessContext)
    {
        previousContext = localSetThreadDPIAwarenessContext (DPI_AWARENESS_CONTEXT_UNAWARE);

       #if DRX_DEBUG
        ++numActiveScopedDpiAwarenessDisablers;
       #endif
    }
}

ScopedDPIAwarenessDisabler::~ScopedDPIAwarenessDisabler()
{
    if (previousContext != nullptr)
    {
        if (auto* localSetThreadDPIAwarenessContext = getScopedDPIAwarenessDisablerFunctions().localSetThreadDPIAwarenessContext)
            localSetThreadDPIAwarenessContext ((DPI_AWARENESS_CONTEXT) previousContext);

       #if DRX_DEBUG
        --numActiveScopedDpiAwarenessDisablers;
       #endif
    }
}

//==============================================================================
using SettingChangeCallbackFunc = z0 (*)(z0);
extern SettingChangeCallbackFunc settingChangeCallback;

//==============================================================================
static const Displays::Display* getCurrentDisplayFromScaleFactor (HWND hwnd);

template <typename ValueType>
static Rectangle<ValueType> convertPhysicalScreenRectangleToLogical (Rectangle<ValueType> r, HWND h) noexcept
{
    if (isPerMonitorDPIAwareWindow (h))
        return Desktop::getInstance().getDisplays().physicalToLogical (r, getCurrentDisplayFromScaleFactor (h));

    return r;
}

template <typename ValueType>
static Rectangle<ValueType> convertLogicalScreenRectangleToPhysical (Rectangle<ValueType> r, HWND h) noexcept
{
    if (isPerMonitorDPIAwareWindow (h))
        return Desktop::getInstance().getDisplays().logicalToPhysical (r, getCurrentDisplayFromScaleFactor (h));

    return r;
}

static Point<i32> convertPhysicalScreenPointToLogical (Point<i32> p, HWND h) noexcept
{
    if (isPerMonitorDPIAwareWindow (h))
        return Desktop::getInstance().getDisplays().physicalToLogical (p, getCurrentDisplayFromScaleFactor (h));

    return p;
}

static Point<i32> convertLogicalScreenPointToPhysical (Point<i32> p, HWND h) noexcept
{
    if (isPerMonitorDPIAwareWindow (h))
        return Desktop::getInstance().getDisplays().logicalToPhysical (p, getCurrentDisplayFromScaleFactor (h));

    return p;
}

DRX_API f64 getScaleFactorForWindow (HWND h);
DRX_API f64 getScaleFactorForWindow (HWND h)
{
    // NB. Using a local function here because we need to call this method from the plug-in wrappers
    // which don't load the DPI-awareness functions on startup
    static auto localGetDPIForWindow = (GetDPIForWindowFunc) getUser32Function ("GetDpiForWindow");

    if (localGetDPIForWindow != nullptr)
        return (f64) localGetDPIForWindow (h) / USER_DEFAULT_SCREEN_DPI;

    return 1.0;
}

static RECT getWindowScreenRect (HWND hwnd)
{
    ScopedThreadDPIAwarenessSetter setter { hwnd };

    RECT rect;
    GetWindowRect (hwnd, &rect);
    return rect;
}

static RECT getWindowClientRect (HWND hwnd)
{
    auto rect = getWindowScreenRect (hwnd);

    if (auto parentH = GetParent (hwnd))
    {
        ScopedThreadDPIAwarenessSetter setter { hwnd };
        MapWindowPoints (HWND_DESKTOP, parentH, (LPPOINT) &rect, 2);
    }

    return rect;
}

static z0 setWindowZOrder (HWND hwnd, HWND insertAfter)
{
    SetWindowPos (hwnd, insertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
}

//==============================================================================
RTL_OSVERSIONINFOW getWindowsVersionInfo();

f64 Desktop::getDefaultMasterScale()
{
    if (! DRXApplicationBase::isStandaloneApp() || isPerMonitorDPIAwareProcess())
        return 1.0;

    return getGlobalDPI() / USER_DEFAULT_SCREEN_DPI;
}

b8 Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
}

class Desktop::NativeDarkModeChangeDetectorImpl
{
public:
    NativeDarkModeChangeDetectorImpl()
    {
        const auto winVer = getWindowsVersionInfo();

        if (winVer.dwMajorVersion >= 10 && winVer.dwBuildNumber >= 17763)
        {
            const auto uxtheme = "uxtheme.dll";
            LoadLibraryA (uxtheme);
            const auto uxthemeModule = GetModuleHandleA (uxtheme);

            if (uxthemeModule != nullptr)
            {
                shouldAppsUseDarkMode = (ShouldAppsUseDarkModeFunc) GetProcAddress (uxthemeModule, MAKEINTRESOURCEA (132));

                if (shouldAppsUseDarkMode != nullptr)
                    darkModeEnabled = shouldAppsUseDarkMode() && ! isHighContrast();
            }
        }
    }

    ~NativeDarkModeChangeDetectorImpl()
    {
        UnhookWindowsHookEx (hook);
    }

    b8 isDarkModeEnabled() const noexcept  { return darkModeEnabled; }

private:
    static b8 isHighContrast()
    {
        HIGHCONTRASTW highContrast {};

        if (SystemParametersInfoW (SPI_GETHIGHCONTRAST, sizeof (highContrast), &highContrast, false))
            return highContrast.dwFlags & HCF_HIGHCONTRASTON;

        return false;
    }

    static LRESULT CALLBACK callWndProc (i32 nCode, WPARAM wParam, LPARAM lParam)
    {
        auto* params = reinterpret_cast<CWPSTRUCT*> (lParam);

        if (nCode >= 0
            && params != nullptr
            && params->message == WM_SETTINGCHANGE
            && params->lParam != 0
            && CompareStringOrdinal (reinterpret_cast<LPWCH> (params->lParam), -1, L"ImmersiveColorSet", -1, true) == CSTR_EQUAL)
        {
            Desktop::getInstance().nativeDarkModeChangeDetectorImpl->colourSetChanged();
        }

        return CallNextHookEx ({}, nCode, wParam, lParam);
    }

    z0 colourSetChanged()
    {
        if (shouldAppsUseDarkMode != nullptr)
        {
            const auto wasDarkModeEnabled = std::exchange (darkModeEnabled, shouldAppsUseDarkMode() && ! isHighContrast());

            if (darkModeEnabled != wasDarkModeEnabled)
                Desktop::getInstance().darkModeChanged();
        }
    }

    using ShouldAppsUseDarkModeFunc = b8 (WINAPI*)();
    ShouldAppsUseDarkModeFunc shouldAppsUseDarkMode = nullptr;

    b8 darkModeEnabled = false;
    HHOOK hook { SetWindowsHookEx (WH_CALLWNDPROC,
                                   callWndProc,
                                   (HINSTANCE) drx::Process::getCurrentModuleInstanceHandle(),
                                   GetCurrentThreadId()) };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeDarkModeChangeDetectorImpl)
};

std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
{
    return std::make_unique<NativeDarkModeChangeDetectorImpl>();
}

b8 Desktop::isDarkModeActive() const
{
    return nativeDarkModeChangeDetectorImpl->isDarkModeEnabled();
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return upright;
}

z64 getMouseEventTime();
z64 getMouseEventTime()
{
    static z64 eventTimeOffset = 0;
    static LONG lastMessageTime = 0;
    const LONG thisMessageTime = GetMessageTime();

    if (thisMessageTime < lastMessageTime || lastMessageTime == 0)
    {
        lastMessageTime = thisMessageTime;
        eventTimeOffset = Time::currentTimeMillis() - thisMessageTime;
    }

    return eventTimeOffset + thisMessageTime;
}

//==============================================================================
i32k extendedKeyModifier               = 0x10000;

i32k KeyPress::spaceKey                = VK_SPACE;
i32k KeyPress::returnKey               = VK_RETURN;
i32k KeyPress::escapeKey               = VK_ESCAPE;
i32k KeyPress::backspaceKey            = VK_BACK;
i32k KeyPress::deleteKey               = VK_DELETE         | extendedKeyModifier;
i32k KeyPress::insertKey               = VK_INSERT         | extendedKeyModifier;
i32k KeyPress::tabKey                  = VK_TAB;
i32k KeyPress::leftKey                 = VK_LEFT           | extendedKeyModifier;
i32k KeyPress::rightKey                = VK_RIGHT          | extendedKeyModifier;
i32k KeyPress::upKey                   = VK_UP             | extendedKeyModifier;
i32k KeyPress::downKey                 = VK_DOWN           | extendedKeyModifier;
i32k KeyPress::homeKey                 = VK_HOME           | extendedKeyModifier;
i32k KeyPress::endKey                  = VK_END            | extendedKeyModifier;
i32k KeyPress::pageUpKey               = VK_PRIOR          | extendedKeyModifier;
i32k KeyPress::pageDownKey             = VK_NEXT           | extendedKeyModifier;
i32k KeyPress::F1Key                   = VK_F1             | extendedKeyModifier;
i32k KeyPress::F2Key                   = VK_F2             | extendedKeyModifier;
i32k KeyPress::F3Key                   = VK_F3             | extendedKeyModifier;
i32k KeyPress::F4Key                   = VK_F4             | extendedKeyModifier;
i32k KeyPress::F5Key                   = VK_F5             | extendedKeyModifier;
i32k KeyPress::F6Key                   = VK_F6             | extendedKeyModifier;
i32k KeyPress::F7Key                   = VK_F7             | extendedKeyModifier;
i32k KeyPress::F8Key                   = VK_F8             | extendedKeyModifier;
i32k KeyPress::F9Key                   = VK_F9             | extendedKeyModifier;
i32k KeyPress::F10Key                  = VK_F10            | extendedKeyModifier;
i32k KeyPress::F11Key                  = VK_F11            | extendedKeyModifier;
i32k KeyPress::F12Key                  = VK_F12            | extendedKeyModifier;
i32k KeyPress::F13Key                  = VK_F13            | extendedKeyModifier;
i32k KeyPress::F14Key                  = VK_F14            | extendedKeyModifier;
i32k KeyPress::F15Key                  = VK_F15            | extendedKeyModifier;
i32k KeyPress::F16Key                  = VK_F16            | extendedKeyModifier;
i32k KeyPress::F17Key                  = VK_F17            | extendedKeyModifier;
i32k KeyPress::F18Key                  = VK_F18            | extendedKeyModifier;
i32k KeyPress::F19Key                  = VK_F19            | extendedKeyModifier;
i32k KeyPress::F20Key                  = VK_F20            | extendedKeyModifier;
i32k KeyPress::F21Key                  = VK_F21            | extendedKeyModifier;
i32k KeyPress::F22Key                  = VK_F22            | extendedKeyModifier;
i32k KeyPress::F23Key                  = VK_F23            | extendedKeyModifier;
i32k KeyPress::F24Key                  = VK_F24            | extendedKeyModifier;
i32k KeyPress::F25Key                  = 0x31000;          // Windows doesn't support F-keys 25 or higher
i32k KeyPress::F26Key                  = 0x31001;
i32k KeyPress::F27Key                  = 0x31002;
i32k KeyPress::F28Key                  = 0x31003;
i32k KeyPress::F29Key                  = 0x31004;
i32k KeyPress::F30Key                  = 0x31005;
i32k KeyPress::F31Key                  = 0x31006;
i32k KeyPress::F32Key                  = 0x31007;
i32k KeyPress::F33Key                  = 0x31008;
i32k KeyPress::F34Key                  = 0x31009;
i32k KeyPress::F35Key                  = 0x3100a;

i32k KeyPress::numberPad0              = VK_NUMPAD0        | extendedKeyModifier;
i32k KeyPress::numberPad1              = VK_NUMPAD1        | extendedKeyModifier;
i32k KeyPress::numberPad2              = VK_NUMPAD2        | extendedKeyModifier;
i32k KeyPress::numberPad3              = VK_NUMPAD3        | extendedKeyModifier;
i32k KeyPress::numberPad4              = VK_NUMPAD4        | extendedKeyModifier;
i32k KeyPress::numberPad5              = VK_NUMPAD5        | extendedKeyModifier;
i32k KeyPress::numberPad6              = VK_NUMPAD6        | extendedKeyModifier;
i32k KeyPress::numberPad7              = VK_NUMPAD7        | extendedKeyModifier;
i32k KeyPress::numberPad8              = VK_NUMPAD8        | extendedKeyModifier;
i32k KeyPress::numberPad9              = VK_NUMPAD9        | extendedKeyModifier;
i32k KeyPress::numberPadAdd            = VK_ADD            | extendedKeyModifier;
i32k KeyPress::numberPadSubtract       = VK_SUBTRACT       | extendedKeyModifier;
i32k KeyPress::numberPadMultiply       = VK_MULTIPLY       | extendedKeyModifier;
i32k KeyPress::numberPadDivide         = VK_DIVIDE         | extendedKeyModifier;
i32k KeyPress::numberPadSeparator      = VK_SEPARATOR      | extendedKeyModifier;
i32k KeyPress::numberPadDecimalPoint   = VK_DECIMAL        | extendedKeyModifier;
i32k KeyPress::numberPadEquals         = 0x92 /*VK_OEM_NEC_EQUAL*/  | extendedKeyModifier;
i32k KeyPress::numberPadDelete         = VK_DELETE         | extendedKeyModifier;
i32k KeyPress::playKey                 = 0x30000;
i32k KeyPress::stopKey                 = 0x30001;
i32k KeyPress::fastForwardKey          = 0x30002;
i32k KeyPress::rewindKey               = 0x30003;


//==============================================================================
class WindowsBitmapImage final : public ImagePixelData
{
public:
    using Ptr = ReferenceCountedObjectPtr<WindowsBitmapImage>;

    WindowsBitmapImage (const Image::PixelFormat format,
                        i32k w, i32k h, const b8 clearImage)
        : ImagePixelData (format, w, h)
    {
        jassert (format == Image::RGB || format == Image::ARGB);

        static b8 alwaysUse32Bits = isGraphicsCard32Bit(); // NB: for 32-bit cards, it's faster to use a 32-bit image.

        pixelStride = (alwaysUse32Bits || format == Image::ARGB) ? 4 : 3;
        lineStride = -((w * pixelStride + 3) & ~3);

        zerostruct (bitmapInfo);
        bitmapInfo.bV4Size     = sizeof (BITMAPV4HEADER);
        bitmapInfo.bV4Width    = w;
        bitmapInfo.bV4Height   = h;
        bitmapInfo.bV4Planes   = 1;
        bitmapInfo.bV4CSType   = 1;
        bitmapInfo.bV4BitCount = (u16) (pixelStride * 8);

        if (format == Image::ARGB)
        {
            bitmapInfo.bV4AlphaMask      = 0xff000000;
            bitmapInfo.bV4RedMask        = 0xff0000;
            bitmapInfo.bV4GreenMask      = 0xff00;
            bitmapInfo.bV4BlueMask       = 0xff;
            bitmapInfo.bV4V4Compression  = BI_BITFIELDS;
        }
        else
        {
            bitmapInfo.bV4V4Compression  = BI_RGB;
        }

        {
            ScopedDeviceContext deviceContext { nullptr };
            hdc = CreateCompatibleDC (deviceContext.dc);
        }

        SetMapMode (hdc, MM_TEXT);

        hBitmap = CreateDIBSection (hdc, (BITMAPINFO*) &(bitmapInfo), DIB_RGB_COLORS,
                                    (uk*) &bitmapData, nullptr, 0);

        if (hBitmap != nullptr)
            previousBitmap = SelectObject (hdc, hBitmap);

        if (format == Image::ARGB && clearImage)
            zeromem (bitmapData, (size_t) std::abs (h * lineStride));

        imageData = bitmapData - (lineStride * (h - 1));
    }

    ~WindowsBitmapImage() override
    {
        SelectObject (hdc, previousBitmap); // Selecting the previous bitmap before deleting the DC avoids a warning in BoundsChecker
        DeleteDC (hdc);
        DeleteObject (hBitmap);
    }

    std::unique_ptr<ImageType> createType() const override
    {
        // This type only exists to return a type ID that's different to the SoftwareImageType's ID,
        // so that `SoftwareImageType{}.convert (windowsBitmapImage)` works.
        // If we return SoftwareImageType here, then SoftwareImageType{}.convert() will compare the
        // type IDs and assume the source image is already of the correct type.
        struct Type : public ImageType
        {
            i32 getTypeID() const override { return ByteOrder::makeInt ('w', 'b', 'i', 't'); }
            ImagePixelData::Ptr create (Image::PixelFormat, i32, i32, b8) const override { return {}; }
            Image convert (const Image&) const override { return {}; }
        };

        return std::make_unique<Type>();
    }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        sendDataChangeMessage();
        return std::make_unique<LowLevelGraphicsSoftwareRenderer> (Image (this));
    }

    z0 initialiseBitmapData (Image::BitmapData& bitmap, i32 x, i32 y, Image::BitmapData::ReadWriteMode mode) override
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
        Image newImage { SoftwareImageType{}.create (pixelFormat, width, height, pixelFormat != Image::RGB) };

        {
            Graphics g (newImage);
            g.drawImageAt (Image { *this }, 0, 0);
        }

        return newImage.getPixelData();
    }

    static z0 updateLayeredWindow (HDC sourceHdc, HWND hwnd, Point<i32> pt, f32 constantAlpha)
    {
        const auto windowBounds = getWindowScreenRect (hwnd);

        auto p = D2DUtilities::toPOINT (pt);
        POINT pos = { windowBounds.left, windowBounds.top };
        SIZE size = { windowBounds.right - windowBounds.left,
                      windowBounds.bottom - windowBounds.top };

        BLENDFUNCTION bf { AC_SRC_OVER, 0, (BYTE) (255.0f * constantAlpha), AC_SRC_ALPHA };

        UpdateLayeredWindow (hwnd, nullptr, &pos, &size, sourceHdc, &p, 0, &bf, ULW_ALPHA);
    }

    z0 updateLayeredWindow (HWND hwnd, Point<i32> pt, f32 constantAlpha) const noexcept
    {
        updateLayeredWindow (hdc, hwnd, pt, constantAlpha);
    }

    z0 blitToDC (HDC dc, i32 x, i32 y) const noexcept
    {
        SetMapMode (dc, MM_TEXT);

        StretchDIBits (dc,
                       x, y, width, height,
                       0, 0, width, height,
                       bitmapData, (const BITMAPINFO*) &bitmapInfo,
                       DIB_RGB_COLORS, SRCCOPY);
    }

    HBITMAP getHBITMAP() const { return hBitmap; }
    HDC getHDC() const { return hdc; }

private:
    HBITMAP hBitmap;
    HGDIOBJ previousBitmap;
    BITMAPV4HEADER bitmapInfo;
    HDC hdc;
    u8* bitmapData;
    i32 pixelStride, lineStride;
    u8* imageData;

    static b8 isGraphicsCard32Bit()
    {
        ScopedDeviceContext deviceContext { nullptr };
        return GetDeviceCaps (deviceContext.dc, BITSPIXEL) > 24;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsBitmapImage)
};

//==============================================================================
namespace IconConverters
{
    struct IconDestructor
    {
        z0 operator() (HICON ptr) const { if (ptr != nullptr) DestroyIcon (ptr); }
    };

    using IconPtr = std::unique_ptr<std::remove_pointer_t<HICON>, IconDestructor>;

    static Image createImageFromHICON (HICON icon)
    {
        if (icon == nullptr)
            return {};

        struct ScopedICONINFO final : public ICONINFO
        {
            ScopedICONINFO()
            {
                hbmColor = nullptr;
                hbmMask = nullptr;
            }

            ~ScopedICONINFO()
            {
                if (hbmColor != nullptr)
                    ::DeleteObject (hbmColor);

                if (hbmMask != nullptr)
                    ::DeleteObject (hbmMask);
            }
        };

        ScopedICONINFO info;

        if (! ::GetIconInfo (icon, &info))
            return {};

        BITMAP bm;

        if (! (::GetObject (info.hbmColor, sizeof (BITMAP), &bm)
               && bm.bmWidth > 0 && bm.bmHeight > 0))
            return {};

        ScopedDeviceContext deviceContext { nullptr };

        if (auto* dc = ::CreateCompatibleDC (deviceContext.dc))
        {
            BITMAPV5HEADER header = {};
            header.bV5Size = sizeof (BITMAPV5HEADER);
            header.bV5Width = bm.bmWidth;
            header.bV5Height = -bm.bmHeight;
            header.bV5Planes = 1;
            header.bV5Compression = BI_RGB;
            header.bV5BitCount = 32;
            header.bV5RedMask = 0x00FF0000;
            header.bV5GreenMask = 0x0000FF00;
            header.bV5BlueMask = 0x000000FF;
            header.bV5AlphaMask = 0xFF000000;
            header.bV5CSType = 0x57696E20; // 'Win '
            header.bV5Intent = LCS_GM_IMAGES;

            u32* bitmapImageData = nullptr;

            if (auto* dib = ::CreateDIBSection (deviceContext.dc, (BITMAPINFO*) &header, DIB_RGB_COLORS,
                                                (uk*) &bitmapImageData, nullptr, 0))
            {
                auto oldObject = ::SelectObject (dc, dib);

                auto numPixels = bm.bmWidth * bm.bmHeight;
                auto numColorComponents = (size_t) numPixels * 4;

                // Windows icon data comes as two layers, an XOR mask which contains the bulk
                // of the image data and an AND mask which provides the transparency. Annoyingly
                // the XOR mask can also contain an alpha channel, in which case the transparency
                // mask should not be applied, but there's no way to find out a priori if the XOR
                // mask contains an alpha channel.

                HeapBlock<b8> opacityMask (numPixels);
                memset (bitmapImageData, 0, numColorComponents);
                ::DrawIconEx (dc, 0, 0, icon, bm.bmWidth, bm.bmHeight, 0, nullptr, DI_MASK);

                for (i32 i = 0; i < numPixels; ++i)
                    opacityMask[i] = (bitmapImageData[i] == 0);

                Image result = Image (Image::ARGB, bm.bmWidth, bm.bmHeight, true);
                Image::BitmapData imageData (result, Image::BitmapData::readWrite);

                memset (bitmapImageData, 0, numColorComponents);
                ::DrawIconEx (dc, 0, 0, icon, bm.bmWidth, bm.bmHeight, 0, nullptr, DI_NORMAL);
                memcpy (imageData.data, bitmapImageData, numColorComponents);

                auto imageHasAlphaChannel = [&imageData, numPixels]()
                {
                    for (i32 i = 0; i < numPixels; ++i)
                        if (imageData.data[i * 4] != 0)
                            return true;

                    return false;
                };

                if (! imageHasAlphaChannel())
                    for (i32 i = 0; i < numPixels; ++i)
                        imageData.data[i * 4] = opacityMask[i] ? 0xff : 0x00;

                ::SelectObject (dc, oldObject);
                ::DeleteObject (dib);
                ::DeleteDC (dc);

                return result;
            }

            ::DeleteDC (dc);
        }

        return {};
    }

    HICON createHICONFromImage (const Image& image, const BOOL isIcon, i32 hotspotX, i32 hotspotY);
    HICON createHICONFromImage (const Image& image, const BOOL isIcon, i32 hotspotX, i32 hotspotY)
    {
        auto nativeBitmap = new WindowsBitmapImage (Image::ARGB, image.getWidth(), image.getHeight(), true);
        Image bitmap (nativeBitmap);

        {
            Graphics g (bitmap);
            g.drawImageAt (image, 0, 0);
        }

        auto mask = CreateBitmap (image.getWidth(), image.getHeight(), 1, 1, nullptr);

        ICONINFO info;
        info.fIcon = isIcon;
        info.xHotspot = (DWORD) hotspotX;
        info.yHotspot = (DWORD) hotspotY;
        info.hbmMask = mask;
        info.hbmColor = nativeBitmap->getHBITMAP();

        auto hi = CreateIconIndirect (&info);
        DeleteObject (mask);
        return hi;
    }
} // namespace IconConverters

//==============================================================================
DRX_IUNKNOWNCLASS (ITipInvocation, "37c994e7-432b-4834-a2f7-dce1f13b834b")
{
    static CLSID getCLSID() noexcept   { return { 0x4ce576fa, 0x83dc, 0x4f88, { 0x95, 0x1c, 0x9d, 0x07, 0x82, 0xb4, 0xe3, 0x76 } }; }

    DRX_COMCALL Toggle (HWND) = 0;
};

} // namespace drx

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (drx::ITipInvocation, 0x37c994e7, 0x432b, 0x4834, 0xa2, 0xf7, 0xdc, 0xe1, 0xf1, 0x3b, 0x83, 0x4b)
#endif

namespace drx
{

//==============================================================================
struct HSTRING_PRIVATE;
typedef HSTRING_PRIVATE* HSTRING;

struct IInspectable : public IUnknown
{
    DRX_COMCALL GetIids (ULONG* ,IID**) = 0;
    DRX_COMCALL GetRuntimeClassName (HSTRING*) = 0;
    DRX_COMCALL GetTrustLevel (uk) = 0;
};

DRX_COMCLASS (IUIViewSettingsInterop, "3694dbf9-8f68-44be-8ff5-195c98ede8a6")  : public IInspectable
{
    DRX_COMCALL GetForWindow (HWND, REFIID, uk*) = 0;
};

DRX_COMCLASS (IUIViewSettings, "c63657f6-8850-470d-88f8-455e16ea2c26")  : public IInspectable
{
    enum UserInteractionMode
    {
        Mouse = 0,
        Touch = 1
    };

    DRX_COMCALL GetUserInteractionMode (UserInteractionMode*) = 0;
};

} // namespace drx

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (drx::IUIViewSettingsInterop, 0x3694dbf9, 0x8f68, 0x44be, 0x8f, 0xf5, 0x19, 0x5c, 0x98, 0xed, 0xe8, 0xa6)
__CRT_UUID_DECL (drx::IUIViewSettings,        0xc63657f6, 0x8850, 0x470d, 0x88, 0xf8, 0x45, 0x5e, 0x16, 0xea, 0x2c, 0x26)
#endif

namespace drx
{
struct UWPUIViewSettings
{
    UWPUIViewSettings()
    {
        ComBaseModule dll (L"api-ms-win-core-winrt-l1-1-0");

        if (dll.h != nullptr)
        {
            roInitialize           = (RoInitializeFuncPtr)           ::GetProcAddress (dll.h, "RoInitialize");
            roGetActivationFactory = (RoGetActivationFactoryFuncPtr) ::GetProcAddress (dll.h, "RoGetActivationFactory");
            createHString          = (WindowsCreateStringFuncPtr)    ::GetProcAddress (dll.h, "WindowsCreateString");
            deleteHString          = (WindowsDeleteStringFuncPtr)    ::GetProcAddress (dll.h, "WindowsDeleteString");

            if (roInitialize == nullptr || roGetActivationFactory == nullptr
                 || createHString == nullptr || deleteHString == nullptr)
                return;

            auto status = roInitialize (1);

            if (status != S_OK && status != S_FALSE && (u32) status != 0x80010106L)
                return;

            LPCWSTR uwpClassName = L"Windows.UI.ViewManagement.UIViewSettings";
            HSTRING uwpClassId = nullptr;

            if (createHString (uwpClassName, (::UINT32) wcslen (uwpClassName), &uwpClassId) != S_OK
                 || uwpClassId == nullptr)
                return;

            DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            status = roGetActivationFactory (uwpClassId, __uuidof (IUIViewSettingsInterop),
                                             (uk*) viewSettingsInterop.resetAndGetPointerAddress());
            DRX_END_IGNORE_WARNINGS_GCC_LIKE
            deleteHString (uwpClassId);

            if (status != S_OK || viewSettingsInterop == nullptr)
                return;

            // move dll into member var
            comBaseDLL = std::move (dll);
        }
    }

private:
    //==============================================================================
    struct ComBaseModule
    {
        ComBaseModule() = default;
        ComBaseModule (LPCWSTR libraryName) : h (::LoadLibrary (libraryName)) {}
        ComBaseModule (ComBaseModule&& o) : h (o.h) { o.h = nullptr; }
        ~ComBaseModule() { release(); }

        z0 release() { if (h != nullptr) ::FreeLibrary (h); h = nullptr; }
        ComBaseModule& operator= (ComBaseModule&& o) { release(); h = o.h; o.h = nullptr; return *this; }

        HMODULE h = {};
    };

    using RoInitializeFuncPtr           = HRESULT (WINAPI*) (i32);
    using RoGetActivationFactoryFuncPtr = HRESULT (WINAPI*) (HSTRING, REFIID, uk*);
    using WindowsCreateStringFuncPtr    = HRESULT (WINAPI*) (LPCWSTR,UINT32, HSTRING*);
    using WindowsDeleteStringFuncPtr    = HRESULT (WINAPI*) (HSTRING);

    ComBaseModule comBaseDLL;
    ComSmartPtr<IUIViewSettingsInterop> viewSettingsInterop;

    RoInitializeFuncPtr roInitialize;
    RoGetActivationFactoryFuncPtr roGetActivationFactory;
    WindowsCreateStringFuncPtr createHString;
    WindowsDeleteStringFuncPtr deleteHString;
};

//==============================================================================
/*  This is an interface to functionality that is implemented differently depending on the rendering
    backend, currently either GDI or Direct2D on Windows.

    This isn't public, so feel free to add or remove functions if necessary.
    In general, it's best to keep things consistent between renderers, so try to make changes
    in the HWNDComponentPeer rather than in implementations of RenderContext wherever possible.
    However, any behaviour that is only required in specific renderers should be added to the
    RenderContext implementations of those renderers.
*/
struct RenderContext
{
    virtual ~RenderContext() = default;

    /*  The name of the renderer backend.
        This must be unique - no two backends may share the same name.
        The name may be displayed to the user, so it should be descriptive.
    */
    virtual tukk getName() const = 0;

    /*  The following functions will all be called by the peer to update the state of the renderer. */
    virtual z0 updateConstantAlpha() = 0;
    virtual z0 handlePaintMessage() = 0;
    virtual z0 repaint (const Rectangle<i32>& area) = 0;
    virtual z0 dispatchDeferredRepaints() = 0;
    virtual z0 performAnyPendingRepaintsNow() = 0;
    virtual z0 onVBlank() = 0;
    virtual z0 handleShowWindow() = 0;

    /*  Gets a snapshot of whatever the render context is currently showing. */
    virtual Image createSnapshot() = 0;
};

//==============================================================================
class HWNDComponentPeer final : public ComponentPeer
                              , private ComponentPeer::VBlankListener
                              , private Timer
                              #if DRX_MODULE_AVAILABLE_drx_audio_plugin_client
                              , public ModifierKeyReceiver
                              #endif
{
public:
    //==============================================================================
    HWNDComponentPeer (Component& comp,
                       i32 windowStyleFlags,
                       HWND parent,
                       b8 nonRepainting,
                       i32 engine)
        : ComponentPeer (comp, windowStyleFlags),
          dontRepaint (nonRepainting),
          parentToAddTo (parent)
    {
        getNativeRealtimeModifiers = getMouseModifiers;

        // CreateWindowEx needs to be called from the message thread
        callFunctionIfNotLocked (&createWindowCallback, this);

        // Complete the window initialisation on the calling thread
        setTitle (component.getName());
        updateShadower();

        updateCurrentMonitorAndRefreshVBlankDispatcher (ForceRefreshDispatcher::yes);

        if (parentToAddTo != nullptr)
        {
            monitorUpdateTimer.emplace ([this]
                                        {
                                            updateCurrentMonitorAndRefreshVBlankDispatcher (ForceRefreshDispatcher::yes);
                                            monitorUpdateTimer->startTimer (1000);
                                        });
        }

        suspendResumeRegistration = ScopedSuspendResumeNotificationRegistration { hwnd };

        setCurrentRenderingEngine (engine);
    }

    ~HWNDComponentPeer() override
    {
        // Clean up that needs to happen on the calling thread
        suspendResumeRegistration = {};

        VBlankDispatcher::getInstance()->removeListener (*this);

        // do this first to avoid messages arriving for this window before it's destroyed
        DrxWindowIdentifier::setAsDRXWindow (hwnd, false);

        if (isAccessibilityActive)
            WindowsAccessibility::revokeUIAMapEntriesForWindow (hwnd);

        shadower = nullptr;
        currentTouches.deleteAllTouchesForPeer (this);

        // Destroy the window from the message thread
        callFunctionIfNotLocked (&destroyWindowCallback, this);

        // And one last little bit of cleanup
        if (dropTarget != nullptr)
        {
            dropTarget->peerIsDeleted = true;
            dropTarget->Release();
            dropTarget = nullptr;
        }
    }

    //==============================================================================
    auto getHWND() const { return hwnd; }
    uk getNativeHandle() const override    { return hwnd; }

    z0 setVisible (b8 shouldBeVisible) override
    {
        const ScopedValueSetter<b8> scope (shouldIgnoreModalDismiss, true);

        ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);

        if (shouldBeVisible)
            InvalidateRect (hwnd, nullptr, 0);
        else
            lastPaintTime = 0;
    }

    z0 setTitle (const Txt& title) override
    {
        // Unfortunately some ancient bits of win32 mean you can only perform this operation from the message thread.
        DRX_ASSERT_MESSAGE_THREAD

        SetWindowText (hwnd, title.toWideCharPointer());
    }

    z0 repaintNowIfTransparent()
    {
        if (getTransparencyKind() == TransparencyKind::perPixel
            && lastPaintTime > 0
            && Time::getMillisecondCounter() > lastPaintTime + 30)
        {
            handlePaintMessage();
        }
    }

    std::optional<BorderSize<i32>> getCustomBorderSize() const
    {
        if (hasTitleBar() || (styleFlags & windowIsTemporary) != 0 || isFullScreen())
            return {};

        return BorderSize<i32> { 0, 0, 0, 0 };
    }

    std::optional<BorderSize<i32>> findPhysicalBorderSize() const
    {
        if (const auto custom = getCustomBorderSize())
            return *custom;

        ScopedThreadDPIAwarenessSetter setter { hwnd };

        WINDOWINFO info{};
        info.cbSize = sizeof (info);

        if (! GetWindowInfo (hwnd, &info))
            return {};

        // Sometimes GetWindowInfo returns bogus information when called in the middle of restoring
        // the window
        if (info.rcWindow.left <= -32000 && info.rcWindow.top <= -32000)
            return {};

        return BorderSize<i32> { info.rcClient.top - info.rcWindow.top,
                                 info.rcClient.left - info.rcWindow.left,
                                 info.rcWindow.bottom - info.rcClient.bottom,
                                 info.rcWindow.right - info.rcClient.right };
    }

    z0 setBounds (const Rectangle<i32>& bounds, b8 isNowFullScreen) override
    {
        // If we try to set new bounds while handling an existing position change,
        // Windows may get confused about our current scale and size.
        // This can happen when moving a window between displays, because the mouse-move
        // generator in handlePositionChanged can cause the window to move again.
        if (inHandlePositionChanged)
            return;

        if (isNowFullScreen != isFullScreen())
            setFullScreen (isNowFullScreen);

        const ScopedValueSetter<b8> scope (shouldIgnoreModalDismiss, true);

        const auto borderSize = findPhysicalBorderSize().value_or (BorderSize<i32>{});
        auto newBounds = borderSize.addedTo ([&]
        {
            ScopedThreadDPIAwarenessSetter setter { hwnd };

            if (! isPerMonitorDPIAwareWindow (hwnd))
                return bounds;

            if (inDpiChange)
                return convertLogicalScreenRectangleToPhysical (bounds, hwnd);

            return convertLogicalScreenRectangleToPhysical (bounds, hwnd)
                    .withPosition (Desktop::getInstance().getDisplays().logicalToPhysical (bounds.getTopLeft()));
        }());

        if (getTransparencyKind() == TransparencyKind::perPixel)
        {
            if (auto parentHwnd = GetParent (hwnd))
            {
                auto parentRect = convertPhysicalScreenRectangleToLogical (D2DUtilities::toRectangle (getWindowScreenRect (parentHwnd)), hwnd);
                newBounds.translate (parentRect.getX(), parentRect.getY());
            }
        }

        const auto oldBounds = [this]
        {
            ScopedThreadDPIAwarenessSetter setter { hwnd };
            RECT result;
            GetWindowRect (hwnd, &result);
            return D2DUtilities::toRectangle (result);
        }();

        const b8 hasMoved = (oldBounds.getPosition() != bounds.getPosition());
        const b8 hasResized = (oldBounds.getWidth() != bounds.getWidth()
                                  || oldBounds.getHeight() != bounds.getHeight());

        DWORD flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
        if (! hasMoved)    flags |= SWP_NOMOVE;
        if (! hasResized)  flags |= SWP_NOSIZE;

        SetWindowPos (hwnd,
                      nullptr,
                      newBounds.getX(),
                      newBounds.getY(),
                      newBounds.getWidth(),
                      newBounds.getHeight(),
                      flags);

        if (hasResized && isValidPeer (this))
        {
            repaintNowIfTransparent();
        }
    }

    Rectangle<i32> getBounds() const override
    {
        if (parentToAddTo == nullptr)
        {
            if (hasTitleBar())
            {
                // Depending on the desktop scale factor, the physical size of the window may not map to
                // an integral client-area size.
                // In this case, we always round the width and height of the client area up to the next
                // integer.
                // This means that we may end up clipping off up to one logical pixel under the physical
                // window border, but this is preferable to displaying an uninitialised/unpainted
                // region of the client area.
                const auto physicalBorder = findPhysicalBorderSize().value_or (BorderSize<i32>{});

                const auto physicalBounds = D2DUtilities::toRectangle (getWindowScreenRect (hwnd));
                const auto physicalClient = physicalBorder.subtractedFrom (physicalBounds);
                const auto logicalClient = convertPhysicalScreenRectangleToLogical (physicalClient.toFloat(), hwnd);
                const auto snapped = logicalClient.withPosition (logicalClient.getPosition().roundToInt().toFloat()).getSmallestIntegerContainer();
                return snapped;
            }

            const auto logicalClient = convertPhysicalScreenRectangleToLogical (getClientRectInScreen(), hwnd);
            return logicalClient;
        }

        auto localBounds = D2DUtilities::toRectangle (getWindowClientRect (hwnd));

        if (isPerMonitorDPIAwareWindow (hwnd))
            return (localBounds.toDouble() / getPlatformScaleFactor()).toNearestInt();

        return localBounds;
    }

    Point<i32> getScreenPosition() const
    {
        return convertPhysicalScreenPointToLogical (getClientRectInScreen().getPosition(), hwnd);
    }

    Point<f32> localToGlobal (Point<f32> relativePosition) override  { return relativePosition + getScreenPosition().toFloat(); }
    Point<f32> globalToLocal (Point<f32> screenPosition) override    { return screenPosition   - getScreenPosition().toFloat(); }

    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;

    enum class TransparencyKind
    {
        perPixel,
        constant,
        opaque,
    };

    TransparencyKind getTransparencyKind() const
    {
        return transparencyKind;
    }

    z0 setAlpha (f32) override
    {
        setLayeredWindow();

        if (renderContext != nullptr)
            renderContext->updateConstantAlpha();
    }

    z0 setMinimised (b8 shouldBeMinimised) override
    {
        const ScopedValueSetter<b8> scope (shouldIgnoreModalDismiss, true);

        if (shouldBeMinimised != isMinimised())
            ShowWindow (hwnd, shouldBeMinimised ? SW_MINIMIZE : SW_RESTORE);
    }

    b8 isMinimised() const override
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof (WINDOWPLACEMENT);
        GetWindowPlacement (hwnd, &wp);

        return wp.showCmd == SW_SHOWMINIMIZED;
    }

    b8 isShowing() const override
    {
        return IsWindowVisible (hwnd) && ! isMinimised();
    }

    z0 setFullScreen (b8 shouldBeFullScreen) override
    {
        const ScopedValueSetter<b8> scope (shouldIgnoreModalDismiss, true);

        setMinimised (false);

        if (isFullScreen() != shouldBeFullScreen)
        {
            if (constrainer != nullptr)
                constrainer->resizeStart();

            const WeakReference<Component> deletionChecker (&component);

            if (shouldBeFullScreen)
            {
                ShowWindow (hwnd, SW_SHOWMAXIMIZED);
            }
            else
            {
                auto boundsCopy = lastNonFullscreenBounds;

                ShowWindow (hwnd, SW_SHOWNORMAL);

                if (! boundsCopy.isEmpty())
                    setBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, boundsCopy), false);
            }

            if (deletionChecker != nullptr)
                handleMovedOrResized();

            if (constrainer != nullptr)
                constrainer->resizeEnd();
        }
    }

    b8 isFullScreen() const override
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof (wp);
        GetWindowPlacement (hwnd, &wp);

        return wp.showCmd == SW_SHOWMAXIMIZED;
    }

    Rectangle<i32> getClientRectInScreen() const
    {
        ScopedThreadDPIAwarenessSetter setter { hwnd };

        RECT rect{};
        GetClientRect (hwnd, &rect);
        auto points = readUnaligned<std::array<POINT, 2>> (&rect);
        MapWindowPoints (hwnd, nullptr, points.data(), (UINT) points.size());
        const auto result = readUnaligned<RECT> (&points);

        return D2DUtilities::toRectangle (result);
    }

    b8 contains (Point<i32> localPos, b8 trueIfInAChildWindow) const override
    {
        auto r = convertPhysicalScreenRectangleToLogical (D2DUtilities::toRectangle (getWindowScreenRect (hwnd)), hwnd);

        if (! r.withZeroOrigin().contains (localPos))
            return false;

        const auto screenPos = convertLogicalScreenPointToPhysical (localPos + getScreenPosition(), hwnd);

        auto w = WindowFromPoint (D2DUtilities::toPOINT (screenPos));
        return w == hwnd || (trueIfInAChildWindow && (IsChild (hwnd, w) != 0));
    }

    OptionalBorderSize getFrameSizeIfPresent() const override
    {
        return ComponentPeer::OptionalBorderSize { getFrameSize() };
    }

    BorderSize<i32> getFrameSize() const override
    {
        return findPhysicalBorderSize().value_or (BorderSize<i32>{}).multipliedBy (1.0 / scaleFactor);
    }

    b8 setAlwaysOnTop (b8 alwaysOnTop) override
    {
        const b8 oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        setWindowZOrder (hwnd, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST);

        shouldDeactivateTitleBar = oldDeactivate;

        if (shadower != nullptr)
            handleBroughtToFront();

        return true;
    }

    z0 toFront (b8 makeActive) override
    {
        const ScopedValueSetter<b8> scope (shouldIgnoreModalDismiss, true);

        setMinimised (false);

        const b8 oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        callFunctionIfNotLocked (makeActive ? &toFrontCallback1 : &toFrontCallback2, hwnd);

        shouldDeactivateTitleBar = oldDeactivate;

        if (! makeActive)
        {
            // in this case a broughttofront call won't have occurred, so do it now..
            handleBroughtToFront();
        }
    }

    z0 toBehind (ComponentPeer* other) override
    {
        const ScopedValueSetter<b8> scope (shouldIgnoreModalDismiss, true);

        if (auto* otherPeer = dynamic_cast<HWNDComponentPeer*> (other))
        {
            setMinimised (false);

            // Must be careful not to try to put a topmost window behind a normal one, or Windows
            // promotes the normal one to be topmost!
            if (component.isAlwaysOnTop() == otherPeer->getComponent().isAlwaysOnTop())
                setWindowZOrder (hwnd, otherPeer->hwnd);
            else if (otherPeer->getComponent().isAlwaysOnTop())
                setWindowZOrder (hwnd, HWND_TOP);
        }
        else
        {
            jassertfalse; // wrong type of window?
        }
    }

    b8 isFocused() const override
    {
        return callFunctionIfNotLocked (&getFocusCallback, nullptr) == (uk) hwnd;
    }

    z0 grabFocus() override
    {
        const ScopedValueSetter<b8> scope (shouldIgnoreModalDismiss, true);

        const b8 oldDeactivate = shouldDeactivateTitleBar;
        shouldDeactivateTitleBar = ((styleFlags & windowIsTemporary) == 0);

        callFunctionIfNotLocked (&setFocusCallback, hwnd);

        shouldDeactivateTitleBar = oldDeactivate;
    }

    z0 textInputRequired (Point<i32>, TextInputTarget&) override
    {
        if (! hasCreatedCaret)
            hasCreatedCaret = CreateCaret (hwnd, (HBITMAP) 1, 0, 0);

        if (hasCreatedCaret)
        {
            SetCaretPos (0, 0);
            ShowCaret (hwnd);
        }

        ImmAssociateContext (hwnd, nullptr);

        // MSVC complains about the nullptr argument, but the docs for this
        // function say that the second argument is ignored when the third
        // argument is IACE_DEFAULT.
        DRX_BEGIN_IGNORE_WARNINGS_MSVC (6387)
        ImmAssociateContextEx (hwnd, nullptr, IACE_DEFAULT);
        DRX_END_IGNORE_WARNINGS_MSVC
    }

    z0 closeInputMethodContext() override
    {
        imeHandler.handleSetContext (hwnd, false);
    }

    z0 dismissPendingTextInput() override
    {
        closeInputMethodContext();

        ImmAssociateContext (hwnd, nullptr);

        if (std::exchange (hasCreatedCaret, false))
            DestroyCaret();
    }

    z0 repaint (const Rectangle<i32>& area) override
    {
        if (renderContext != nullptr)
            renderContext->repaint ((area.toDouble() * getPlatformScaleFactor()).getSmallestIntegerContainer());
    }

    z0 dispatchDeferredRepaints()
    {
        if (renderContext != nullptr)
            renderContext->dispatchDeferredRepaints();
    }

    z0 performAnyPendingRepaintsNow() override
    {
        if (renderContext != nullptr)
            renderContext->performAnyPendingRepaintsNow();
    }

    Image createSnapshot()
    {
        if (renderContext != nullptr)
            return renderContext->createSnapshot();

        return {};
    }

    //==============================================================================
    z0 onVBlank (f64 timestampSec) override
    {
        callVBlankListeners (timestampSec);
        dispatchDeferredRepaints();

        if (renderContext != nullptr)
            renderContext->onVBlank();
    }

    //==============================================================================
    static HWNDComponentPeer* getOwnerOfWindow (HWND h) noexcept
    {
        if (h != nullptr && DrxWindowIdentifier::isDRXWindow (h))
            return (HWNDComponentPeer*) GetWindowLongPtr (h, 8);

        return nullptr;
    }

    //==============================================================================
    b8 isInside (HWND h) const noexcept
    {
        return GetAncestor (hwnd, GA_ROOT) == h;
    }

    //==============================================================================
    static b8 isKeyDown (i32k key) noexcept  { return (GetAsyncKeyState (key) & 0x8000) != 0; }

    static z0 updateKeyModifiers() noexcept
    {
        i32 keyMods = 0;
        if (isKeyDown (VK_SHIFT))   keyMods |= ModifierKeys::shiftModifier;
        if (isKeyDown (VK_CONTROL)) keyMods |= ModifierKeys::ctrlModifier;
        if (isKeyDown (VK_MENU))    keyMods |= ModifierKeys::altModifier;

        // workaround: Windows maps AltGr to left-Ctrl + right-Alt.
        if (isKeyDown (VK_RMENU) && !isKeyDown (VK_RCONTROL))
        {
            keyMods = (keyMods & ~ModifierKeys::ctrlModifier) | ModifierKeys::altModifier;
        }

        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withOnlyMouseButtons().withFlags (keyMods);
    }

    static z0 updateModifiersFromWParam (const WPARAM wParam)
    {
        i32 mouseMods = 0;
        if (wParam & MK_LBUTTON)   mouseMods |= ModifierKeys::leftButtonModifier;
        if (wParam & MK_RBUTTON)   mouseMods |= ModifierKeys::rightButtonModifier;
        if (wParam & MK_MBUTTON)   mouseMods |= ModifierKeys::middleButtonModifier;

        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (mouseMods);
        updateKeyModifiers();
    }

    //==============================================================================
    b8 dontRepaint;
    static ModifierKeys modifiersAtLastCallback;

    //==============================================================================
    struct FileDropTarget final : public ComBaseClassHelper<IDropTarget>
    {
        FileDropTarget (HWNDComponentPeer& p)   : peer (p) {}

        DRX_COMRESULT DragEnter (IDataObject* pDataObject, DWORD grfKeyState, POINTL mousePos, DWORD* pdwEffect) override
        {
            auto hr = updateFileList (pDataObject);

            if (FAILED (hr))
                return hr;

            return DragOver (grfKeyState, mousePos, pdwEffect);
        }

        DRX_COMRESULT DragLeave() override
        {
            if (peerIsDeleted)
                return S_FALSE;

            peer.handleDragExit (dragInfo);
            return S_OK;
        }

        DRX_COMRESULT DragOver (DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect) override
        {
            if (peerIsDeleted)
                return S_FALSE;

            dragInfo.position = getMousePos (mousePos).roundToInt();
            *pdwEffect = peer.handleDragMove (dragInfo) ? (DWORD) DROPEFFECT_COPY
                                                        : (DWORD) DROPEFFECT_NONE;
            return S_OK;
        }

        DRX_COMRESULT Drop (IDataObject* pDataObject, DWORD /*grfKeyState*/, POINTL mousePos, DWORD* pdwEffect) override
        {
            auto hr = updateFileList (pDataObject);

            if (FAILED (hr))
                return hr;

            dragInfo.position = getMousePos (mousePos).roundToInt();
            *pdwEffect = peer.handleDragDrop (dragInfo) ? (DWORD) DROPEFFECT_COPY
                                                        : (DWORD) DROPEFFECT_NONE;
            return S_OK;
        }

        HWNDComponentPeer& peer;
        DragInfo dragInfo;
        b8 peerIsDeleted = false;

    private:
        Point<f32> getMousePos (POINTL mousePos) const
        {
            const auto originalPos = D2DUtilities::toPoint ({ mousePos.x, mousePos.y });
            const auto logicalPos = convertPhysicalScreenPointToLogical (originalPos, peer.hwnd);
            return detail::ScalingHelpers::screenPosToLocalPos (peer.component, logicalPos.toFloat());
        }

        struct DroppedData
        {
            DroppedData (IDataObject* dataObject, CLIPFORMAT type)
            {
                FORMATETC format = { type, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

                if (SUCCEEDED (error = dataObject->GetData (&format, &medium)) && medium.hGlobal != nullptr)
                {
                    dataSize = GlobalSize (medium.hGlobal);
                    data = GlobalLock (medium.hGlobal);
                }
            }

            ~DroppedData()
            {
                if (data != nullptr && medium.hGlobal != nullptr)
                    GlobalUnlock (medium.hGlobal);
            }

            HRESULT error;
            STGMEDIUM medium { TYMED_HGLOBAL, { nullptr }, nullptr };
            uk data = {};
            SIZE_T dataSize;
        };

        z0 parseFileList (HDROP dropFiles)
        {
            dragInfo.files.clearQuick();

            std::vector<TCHAR> nameBuffer;

            const auto numFiles = DragQueryFile (dropFiles, ~(UINT) 0, nullptr, 0);

            for (UINT i = 0; i < numFiles; ++i)
            {
                const auto bufferSize = DragQueryFile (dropFiles, i, nullptr, 0);
                nameBuffer.clear();
                nameBuffer.resize (bufferSize + 1, 0); // + 1 for the null terminator

                [[maybe_unused]] const auto readCharacters = DragQueryFile (dropFiles, i, nameBuffer.data(), (UINT) nameBuffer.size());
                jassert (readCharacters == bufferSize);

                dragInfo.files.add (Txt (nameBuffer.data()));
            }
        }

        HRESULT updateFileList (IDataObject* const dataObject)
        {
            if (peerIsDeleted)
                return S_FALSE;

            dragInfo.clear();

            {
                DroppedData fileData (dataObject, CF_HDROP);

                if (SUCCEEDED (fileData.error))
                {
                    parseFileList (static_cast<HDROP> (fileData.data));
                    return S_OK;
                }
            }

            DroppedData textData (dataObject, CF_UNICODETEXT);

            if (SUCCEEDED (textData.error))
            {
                dragInfo.text = Txt (CharPointer_UTF16 ((const WCHAR*) textData.data),
                                        CharPointer_UTF16 ((const WCHAR*) addBytesToPointer (textData.data, textData.dataSize)));
                return S_OK;
            }

            return textData.error;
        }

        DRX_DECLARE_NON_COPYABLE (FileDropTarget)
    };

    static b8 offerKeyMessageToDRXWindow (const MSG& msg)
    {
        // If this isn't a keyboard message, let the host deal with it.

        constexpr UINT messages[] { WM_KEYDOWN, WM_SYSKEYDOWN, WM_KEYUP, WM_SYSKEYUP, WM_CHAR, WM_SYSCHAR };

        if (std::find (std::begin (messages), std::end (messages), msg.message) == std::end (messages))
            return false;

        auto* peer = getOwnerOfWindow (msg.hwnd);
        auto* focused = Component::getCurrentlyFocusedComponent();

        if (focused == nullptr || peer == nullptr || focused->getPeer() != peer)
            return false;

        auto* hwnd = static_cast<HWND> (peer->getNativeHandle());

        if (hwnd == nullptr)
            return false;

        ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { hwnd };

        // If we've been sent a text character, process it as text.

        if (msg.message == WM_CHAR || msg.message == WM_SYSCHAR)
            return peer->doKeyChar ((i32) msg.wParam, msg.lParam);

        // The event was a keypress, rather than a text character

        if (peer->findCurrentTextInputTarget() != nullptr)
        {
            // If there's a focused text input target, we want to attempt "real" text input with an
            // IME, and we want to prevent the host from eating keystrokes (spaces etc.).

            TranslateMessage (&msg);

            // TranslateMessage may post WM_CHAR back to the window, so we remove those messages
            // from the queue before the host gets to see them.
            // This will dispatch pending WM_CHAR messages, so we may end up reentering
            // offerKeyMessageToDRXWindow and hitting the WM_CHAR case above.
            // We always return true if WM_CHAR is posted so that the keypress is not forwarded
            // to the host. Otherwise, the host may call TranslateMessage again on this message,
            // resulting in duplicate WM_CHAR messages being posted.

            MSG peeked{};
            if (PeekMessage (&peeked, hwnd, WM_CHAR, WM_DEADCHAR, PM_REMOVE)
                || PeekMessage (&peeked, hwnd, WM_SYSCHAR, WM_SYSDEADCHAR, PM_REMOVE))
            {
                return true;
            }

            // If TranslateMessage didn't add a WM_CHAR to the queue, fall back to processing the
            // event as a plain keypress
        }

        // There's no text input target, or the key event wasn't translated, so we'll just see if we
        // can use the plain keystroke event

        if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
            return peer->doKeyDown (msg.wParam);

        return peer->doKeyUp (msg.wParam);
    }

    f64 getPlatformScaleFactor() const noexcept override
    {
       #if ! DRX_WIN_PER_MONITOR_DPI_AWARE
        return 1.0;
       #else
        if (! isPerMonitorDPIAwareWindow (hwnd))
            return 1.0;

        if (auto* parentHWND = GetParent (hwnd))
        {
            if (auto* parentPeer = getOwnerOfWindow (parentHWND))
                return parentPeer->getPlatformScaleFactor();

            if (getDPIForWindow != nullptr)
                return getScaleFactorForWindow (parentHWND);
        }

        return scaleFactor;
       #endif
    }

    static z0 getLastError()
    {
        TCHAR messageBuffer[256] = {};

        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr,
                       GetLastError(),
                       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                       messageBuffer,
                       (DWORD) numElementsInArray (messageBuffer) - 1,
                       nullptr);

        DBG (messageBuffer);
        jassertfalse;
    }

    b8 hasTitleBar() const                 { return (styleFlags & windowHasTitleBar) != 0; }
    f64 getScaleFactor() const            { return scaleFactor; }

private:
    HWND hwnd, parentToAddTo;
    std::unique_ptr<DropShadower> shadower;
    u32 lastPaintTime = 0;
    ULONGLONG lastMagnifySize = 0;
    b8 isDragging = false, isMouseOver = false,
         hasCreatedCaret = false, constrainerIsResizing = false, sizing = false;
    IconConverters::IconPtr currentWindowIcon;
    FileDropTarget* dropTarget = nullptr;
    UWPUIViewSettings uwpViewSettings;
    TransparencyKind transparencyKind = TransparencyKind::opaque;
   #if DRX_MODULE_AVAILABLE_drx_audio_plugin_client
    ModifierKeyProvider* modProvider = nullptr;
   #endif

    f64 scaleFactor = 1.0;
    b8 inDpiChange = 0, inHandlePositionChanged = 0;
    HMONITOR currentMonitor = nullptr;

    b8 isAccessibilityActive = false;

    //==============================================================================
    static MultiTouchMapper<DWORD> currentTouches;

    //==============================================================================
    class WindowClassHolder final : private DeletedAtShutdown
    {
    public:
        WindowClassHolder()
        {
            // this name has to be different for each app/dll instance because otherwise poor old Windows can
            // get a bit confused (even despite it not being a process-global window class).
            Txt windowClassName ("DRX_");
            windowClassName << Txt::toHexString (Time::currentTimeMillis());

            auto moduleHandle = (HINSTANCE) Process::getCurrentModuleInstanceHandle();

            TCHAR moduleFile[1024] = {};
            GetModuleFileName (moduleHandle, moduleFile, 1024);

            WNDCLASSEX wcex = {};
            wcex.cbSize         = sizeof (wcex);
            wcex.lpfnWndProc    = (WNDPROC) windowProc;
            wcex.lpszClassName  = windowClassName.toWideCharPointer();
            wcex.cbWndExtra     = 32;
            wcex.hInstance      = moduleHandle;

            for (const auto& [index, field, ptr] : { std::tuple { 0, &wcex.hIcon,   &iconBig },
                                                     std::tuple { 1, &wcex.hIconSm, &iconSmall } })
            {
                auto iconNum = static_cast<WORD> (index);
                ptr->reset (*field = ExtractAssociatedIcon (moduleHandle, moduleFile, &iconNum));
            }

            atom = RegisterClassEx (&wcex);
            jassert (atom != 0);

            isEventBlockedByModalComps = checkEventBlockedByModalComps;
        }

        ~WindowClassHolder()
        {
            if (ComponentPeer::getNumPeers() == 0)
                UnregisterClass (getWindowClassName(), (HINSTANCE) Process::getCurrentModuleInstanceHandle());

            clearSingletonInstance();
        }

        LPCTSTR getWindowClassName() const noexcept     { return (LPCTSTR) (pointer_sized_uint) atom; }

        DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (WindowClassHolder)

    private:
        ATOM atom;

        static b8 isHWNDBlockedByModalComponents (HWND h)
        {
            for (i32 i = Desktop::getInstance().getNumComponents(); --i >= 0;)
                if (auto* c = Desktop::getInstance().getComponent (i))
                    if ((! c->isCurrentlyBlockedByAnotherModalComponent())
                          && IsChild ((HWND) c->getWindowHandle(), h))
                        return false;

            return true;
        }

        static b8 checkEventBlockedByModalComps (const MSG& m)
        {
            if (Component::getNumCurrentlyModalComponents() == 0 || DrxWindowIdentifier::isDRXWindow (m.hwnd))
                return false;

            switch (m.message)
            {
                case WM_MOUSEMOVE:
                case WM_NCMOUSEMOVE:
                case 0x020A: /* WM_MOUSEWHEEL */
                case 0x020E: /* WM_MOUSEHWHEEL */
                case WM_KEYUP:
                case WM_SYSKEYUP:
                case WM_CHAR:
                case WM_APPCOMMAND:
                case WM_LBUTTONUP:
                case WM_MBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MOUSEACTIVATE:
                case WM_NCMOUSEHOVER:
                case WM_MOUSEHOVER:
                case WM_TOUCH:
                case WM_POINTERUPDATE:
                case WM_NCPOINTERUPDATE:
                case WM_POINTERWHEEL:
                case WM_POINTERHWHEEL:
                case WM_POINTERUP:
                case WM_POINTERACTIVATE:
                    return isHWNDBlockedByModalComponents (m.hwnd);
                case WM_NCLBUTTONDOWN:
                case WM_NCLBUTTONDBLCLK:
                case WM_NCRBUTTONDOWN:
                case WM_NCRBUTTONDBLCLK:
                case WM_NCMBUTTONDOWN:
                case WM_NCMBUTTONDBLCLK:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONDBLCLK:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONDBLCLK:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONDBLCLK:
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                case WM_NCPOINTERDOWN:
                case WM_POINTERDOWN:
                    if (isHWNDBlockedByModalComponents (m.hwnd))
                    {
                        if (auto* modal = Component::getCurrentlyModalComponent (0))
                            modal->inputAttemptWhenModal();

                        return true;
                    }
                    break;

                default:
                    break;
            }

            return false;
        }

        IconConverters::IconPtr iconBig, iconSmall;

        DRX_DECLARE_NON_COPYABLE (WindowClassHolder)
    };

    //==============================================================================
    static uk createWindowCallback (uk userData)
    {
        static_cast<HWNDComponentPeer*> (userData)->createWindowOnMessageThread();
        return nullptr;
    }

    z0 createWindowOnMessageThread()
    {
        DWORD exstyle = 0;
        DWORD type = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        const auto titled = (styleFlags & windowHasTitleBar) != 0;
        const auto hasClose = (styleFlags & windowHasCloseButton) != 0;
        const auto hasMin = (styleFlags & windowHasMinimiseButton) != 0;
        const auto hasMax = (styleFlags & windowHasMaximiseButton) != 0;
        const auto appearsOnTaskbar = (styleFlags & windowAppearsOnTaskbar) != 0;
        const auto resizable = (styleFlags & windowIsResizable) != 0;
        const auto usesDropShadow = windowUsesNativeShadow();

        if (parentToAddTo != nullptr)
        {
            type |= WS_CHILD;
        }
        else
        {
            if (titled || usesDropShadow)
            {
                type |= usesDropShadow ? WS_CAPTION : 0;
                type |= titled ? (WS_OVERLAPPED | WS_CAPTION) : WS_POPUP;
                type |= hasClose ? (WS_SYSMENU | WS_CAPTION) : 0;
                type |= hasMin ? (WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU) : 0;
                type |= hasMax ? (WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU) : 0;
                type |= resizable ? WS_THICKFRAME : 0;
            }
            else
            {
                // Transparent windows need WS_POPUP and not WS_OVERLAPPED | WS_CAPTION, otherwise
                // the top corners of the window will get rounded unconditionally.
                // Unfortunately, this disables nice mouse handling for the caption area.
                type |= WS_POPUP;
            }

            exstyle |= appearsOnTaskbar ? WS_EX_APPWINDOW : WS_EX_TOOLWINDOW;
        }

        hwnd = CreateWindowEx (exstyle, WindowClassHolder::getInstance()->getWindowClassName(),
                               L"", type, 0, 0, 0, 0, parentToAddTo, nullptr,
                               (HINSTANCE) Process::getCurrentModuleInstanceHandle(), nullptr);

        if (! titled && usesDropShadow)
        {
            // The choice of margins is very particular.
            // - Using 0 for all values disables the system decoration (shadow etc.) completely.
            // - Using -1 for all values breaks the software renderer, because the client content
            //   gets blended with the system-drawn controls.
            //   It looks OK most of the time with the D2D renderer, but can look very ugly during
            //   resize because the native window controls still get drawn under the client area.
            // - Using 1 for all values looks the way we want for both renderers, but seems to
            //   prevent the Windows 11 maximize-button flyout from appearing (?).
            // - Using 1 for left and right, and 0 for top and bottom shows the system shadow and
            //   maximize-button flyout.
            static constexpr MARGINS margins { 1, 1, 0, 0 };
            ::DwmExtendFrameIntoClientArea (hwnd, &margins);
            ::SetWindowPos (hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
        }

       #if DRX_DEBUG
        // The DPI-awareness context of this window and DRX's hidden message window are different.
        // You normally want these to match otherwise timer events and async messages will happen
        // in a different context to normal HWND messages which can cause issues with UI scaling.
        jassert (isPerMonitorDPIAwareWindow (hwnd) == isPerMonitorDPIAwareWindow (drx_messageWindowHandle)
                   || numActiveScopedDpiAwarenessDisablers > 0);
       #endif

        if (hwnd != nullptr)
        {
            SetWindowLongPtr (hwnd, 0, 0);
            SetWindowLongPtr (hwnd, 8, (LONG_PTR) this);
            DrxWindowIdentifier::setAsDRXWindow (hwnd, true);

            if (dropTarget == nullptr)
            {
                HWNDComponentPeer* peer = nullptr;

                if (dontRepaint)
                    peer = getOwnerOfWindow (parentToAddTo);

                if (peer == nullptr)
                    peer = this;

                dropTarget = new FileDropTarget (*peer);
            }

            RegisterDragDrop (hwnd, dropTarget);

            if (canUseMultiTouch())
                registerTouchWindow (hwnd, 0);

            setDPIAwareness();

            if (isPerMonitorDPIAwareThread())
                scaleFactor = getScaleFactorForWindow (hwnd);

            setMessageFilter();
            checkForPointerAPI();

            // This is needed so that our plugin window gets notified of WM_SETTINGCHANGE messages
            // and can respond to display scale changes
            if (! DRXApplication::isStandaloneApp())
                settingChangeCallback = ComponentPeer::forceDisplayUpdate;

            // Calling this function here is (for some reason) necessary to make Windows
            // correctly enable the menu items that we specify in the wm_initmenu message.
            GetSystemMenu (hwnd, false);

            setAlpha (component.getAlpha());
        }
        else
        {
            getLastError();
        }
    }

    static BOOL CALLBACK revokeChildDragDropCallback (HWND hwnd, LPARAM)    { RevokeDragDrop (hwnd); return TRUE; }

    static uk destroyWindowCallback (uk userData)
    {
        static_cast<HWNDComponentPeer*> (userData)->destroyWindowOnMessageThread();
        return nullptr;
    }

    z0 destroyWindowOnMessageThread() noexcept
    {
        if (IsWindow (hwnd))
        {
            RevokeDragDrop (hwnd);

            // NB: we need to do this before DestroyWindow() as child HWNDs will be invalid after
            EnumChildWindows (hwnd, revokeChildDragDropCallback, 0);

            DestroyWindow (hwnd);
        }
    }

    static uk toFrontCallback1 (uk h)
    {
        BringWindowToTop ((HWND) h);
        return nullptr;
    }

    static uk toFrontCallback2 (uk h)
    {
        setWindowZOrder ((HWND) h, HWND_TOP);
        return nullptr;
    }

    static uk setFocusCallback (uk h)
    {
        SetFocus ((HWND) h);
        return nullptr;
    }

    static uk getFocusCallback (uk)
    {
        return GetFocus();
    }

    b8 isOpaque() const
    {
        return component.isOpaque();
    }

    b8 windowUsesNativeShadow() const
    {
        return hasTitleBar()
            || (   (0 != (styleFlags & windowHasDropShadow))
                && (0 == (styleFlags & windowIsSemiTransparent))
                && (0 == (styleFlags & windowIsTemporary)));
    }

    z0 updateShadower()
    {
        if (! component.isCurrentlyModal()
            && (styleFlags & windowHasDropShadow) != 0
            && ! windowUsesNativeShadow())
        {
            shadower = component.getLookAndFeel().createDropShadowerForComponent (component);

            if (shadower != nullptr)
                shadower->setOwner (&component);
        }
    }

    z0 setIcon (const Image& newIcon) override
    {
        if (IconConverters::IconPtr hicon { IconConverters::createHICONFromImage (newIcon, TRUE, 0, 0) })
        {
            SendMessage (hwnd, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM> (hicon.get()));
            SendMessage (hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM> (hicon.get()));
            currentWindowIcon = std::move (hicon);
        }
    }

    z0 setMessageFilter()
    {
        using ChangeWindowMessageFilterExFunc = BOOL (WINAPI*) (HWND, UINT, DWORD, PVOID);

        static auto changeMessageFilter = (ChangeWindowMessageFilterExFunc) getUser32Function ("ChangeWindowMessageFilterEx");

        if (changeMessageFilter != nullptr)
        {
            changeMessageFilter (hwnd, WM_DROPFILES, 1 /*MSGFLT_ALLOW*/, nullptr);
            changeMessageFilter (hwnd, WM_COPYDATA, 1 /*MSGFLT_ALLOW*/, nullptr);
            changeMessageFilter (hwnd, 0x49, 1 /*MSGFLT_ALLOW*/, nullptr);
        }
    }

    TransparencyKind computeTransparencyKind() const
    {
        if (! hasTitleBar() && ! component.isOpaque())
            return TransparencyKind::perPixel;

        // If you hit this assertion, you're trying to create a window with a native titlebar
        // and per-pixel transparency. If you want a semi-transparent window, then remove the
        // native title bar. Otherwise, ensure that the window's component is opaque.
        jassert (! hasTitleBar() || component.isOpaque());

        if (component.getAlpha() < 1.0f)
            return TransparencyKind::constant;

        return TransparencyKind::opaque;
    }

    z0 setLayeredWindow()
    {
        const auto old = std::exchange (transparencyKind, computeTransparencyKind());

        if (old == getTransparencyKind())
            return;

        const auto prev = GetWindowLongPtr (hwnd, GWL_EXSTYLE);

        // UpdateLayeredWindow will fail if SetLayeredWindowAttributes has previously been called
        // without unsetting and resetting the layering style bit.
        // UpdateLayeredWindow is used for perPixel windows; SetLayeredWindowAttributes is used for
        // windows with a constant alpha but otherwise "opaque" contents (i.e. component.isOpaque()
        // returns true but component.getAlpha() is less than 1.0f).
        if (getTransparencyKind() == TransparencyKind::perPixel)
            SetWindowLongPtr (hwnd, GWL_EXSTYLE, prev & ~WS_EX_LAYERED);

        const auto newStyle = getTransparencyKind() == TransparencyKind::opaque
                              ? (prev & ~WS_EX_LAYERED)
                              : (prev | WS_EX_LAYERED);

        SetWindowLongPtr (hwnd, GWL_EXSTYLE, newStyle);
        RedrawWindow (hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }

    //==============================================================================
    z0 handlePaintMessage()
    {
        if (renderContext != nullptr)
            renderContext->handlePaintMessage();

        lastPaintTime = Time::getMillisecondCounter();
    }

    //==============================================================================
    z0 doMouseEvent (Point<f32> position, f32 pressure, f32 orientation = 0.0f, ModifierKeys mods = ModifierKeys::currentModifiers)
    {
        handleMouseEvent (MouseInputSource::InputSourceType::mouse, position, mods, pressure, orientation, getMouseEventTime());
    }

    StringArray getAvailableRenderingEngines() override;
    i32 getCurrentRenderingEngine() const override;
    z0 setCurrentRenderingEngine (i32 e) override;

    b8 isTouchEvent() noexcept
    {
        if (registerTouchWindow == nullptr)
            return false;

        // Relevant info about touch/pen detection flags:
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms703320(v=vs.85).aspx
        // http://www.petertissen.de/?p=4

        return ((u32) GetMessageExtraInfo() & 0xFFFFFF80 /*SIGNATURE_MASK*/) == 0xFF515780 /*MI_WP_SIGNATURE*/;
    }

    static b8 areOtherTouchSourcesActive()
    {
        for (auto& ms : Desktop::getInstance().getMouseSources())
            if (ms.isDragging() && (ms.getType() == MouseInputSource::InputSourceType::touch
                                     || ms.getType() == MouseInputSource::InputSourceType::pen))
                return true;

        return false;
    }

    enum class WindowArea
    {
        nonclient,
        client,
    };

    std::optional<LRESULT> doMouseMove (const LPARAM lParam, b8 isMouseDownEvent, WindowArea area)
    {
        // Check if the mouse has moved since being pressed in the caption area.
        // If it has, then we defer to DefWindowProc to handle the mouse movement.
        // Allowing DefWindowProc to handle WM_NCLBUTTONDOWN directly will pause message
        // processing (and therefore painting) when the mouse is clicked in the caption area,
        // which is why we wait until the mouse is *moved* before asking the system to take over.
        // Letting the system handle the move is important for things like Aero Snap to work.
        if (area == WindowArea::nonclient && captionMouseDown.has_value() && *captionMouseDown != lParam)
        {
            captionMouseDown.reset();

            // When clicking and dragging on the caption area, a new modal loop is started
            // inside DefWindowProc. This modal loop appears to consume some mouse events,
            // without forwarding them back to our own window proc. In particular, we never
            // get to see the WM_NCLBUTTONUP event with the HTCAPTION argument, or any other
            // kind of mouse-up event to signal that the loop exited, so
            // ModifierKeys::currentModifiers gets left in the wrong state. As a workaround, we
            // manually update the modifier keys after DefWindowProc exits, and update the
            // capture state if necessary.
            const auto result = DefWindowProc (hwnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
            getMouseModifiers();
            releaseCaptureIfNecessary();
            return result;
        }

        ModifierKeys modsToSend (ModifierKeys::currentModifiers);

        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return {};

        const auto position = area == WindowArea::client ? getPointFromLocalLParam (lParam)
                                                         : getLocalPointFromScreenLParam (lParam);

        if (! isMouseOver)
        {
            isMouseOver = true;

            // This avoids a rare stuck-button problem when focus is lost unexpectedly, but must
            // not be called as part of a move, in case it's actually a mouse-drag from another
            // app which ends up here when we get focus before the mouse is released..
            if (isMouseDownEvent)
                NullCheckedInvocation::invoke (getNativeRealtimeModifiers);

            updateKeyModifiers();
            updateModifiersFromModProvider();

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof (tme);
            tme.dwFlags = TME_LEAVE | (area == WindowArea::nonclient ? TME_NONCLIENT : 0);
            tme.hwndTrack = hwnd;
            tme.dwHoverTime = 0;

            if (! TrackMouseEvent (&tme))
                jassertfalse;

            if (area == WindowArea::client)
                Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
        }
        else if (! isDragging)
        {
            if (! contains (position.roundToInt(), false))
                return {};
        }

        static u32 lastMouseTime = 0;
        auto now = Time::getMillisecondCounter();

        if (! Desktop::getInstance().getMainMouseSource().isDragging())
            modsToSend = modsToSend.withoutMouseButtons();

        if (now >= lastMouseTime)
        {
            lastMouseTime = now;
            doMouseEvent (position, MouseInputSource::defaultPressure,
                          MouseInputSource::defaultOrientation, modsToSend);
        }

        return {};
    }

    z0 updateModifiersFromModProvider() const
    {
       #if DRX_MODULE_AVAILABLE_drx_audio_plugin_client
        if (modProvider != nullptr)
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withFlags (modProvider->getWin32Modifiers());
       #endif
    }

    z0 updateModifiersWithMouseWParam (const WPARAM wParam) const
    {
        updateModifiersFromWParam (wParam);
        updateModifiersFromModProvider();
    }

    z0 releaseCaptureIfNecessary() const
    {
        if (! ModifierKeys::currentModifiers.isAnyMouseButtonDown() && hwnd == GetCapture())
            ReleaseCapture();
    }

    z0 doMouseDown (LPARAM lParam, const WPARAM wParam)
    {
        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return;

        if (GetCapture() != hwnd)
            SetCapture (hwnd);

        doMouseMove (lParam, true, WindowArea::client);

        if (isValidPeer (this))
        {
            updateModifiersWithMouseWParam (wParam);

            isDragging = true;

            doMouseEvent (getPointFromLocalLParam (lParam), MouseInputSource::defaultPressure);
        }
    }

    z0 doMouseUp (Point<f32> position, const WPARAM wParam, b8 adjustCapture = true)
    {
        // this will be handled by WM_TOUCH
        if (isTouchEvent() || areOtherTouchSourcesActive())
            return;

        updateModifiersWithMouseWParam (wParam);

        const b8 wasDragging = std::exchange (isDragging, false);

        // release the mouse capture if the user has released all buttons
        if (adjustCapture)
            releaseCaptureIfNecessary();

        // NB: under some circumstances (e.g. f64-clicking a native title bar), a mouse-up can
        // arrive without a mouse-down, so in that case we need to avoid sending a message.
        if (wasDragging)
            doMouseEvent (position, MouseInputSource::defaultPressure);
    }

    z0 doCaptureChanged()
    {
        if (constrainerIsResizing)
        {
            if (constrainer != nullptr)
                constrainer->resizeEnd();

            constrainerIsResizing = false;
        }

        if (isDragging)
            doMouseUp (getCurrentMousePos(), (WPARAM) 0, false);
    }

    z0 doMouseExit()
    {
        isMouseOver = false;

        if (! areOtherTouchSourcesActive())
            doMouseEvent (getCurrentMousePos(), MouseInputSource::defaultPressure);
    }

    std::tuple<ComponentPeer*, Point<f32>> findPeerUnderMouse()
    {
        auto currentMousePos = getPOINTFromLParam ((LPARAM) GetMessagePos());

        auto* peer = getOwnerOfWindow (WindowFromPoint (currentMousePos));

        if (peer == nullptr)
            peer = this;

        return std::tuple (peer, peer->globalToLocal (convertPhysicalScreenPointToLogical (D2DUtilities::toPoint (currentMousePos), hwnd).toFloat()));
    }

    static MouseInputSource::InputSourceType getPointerType (WPARAM wParam)
    {
        if (getPointerTypeFunction != nullptr)
        {
            POINTER_INPUT_TYPE pointerType;

            if (getPointerTypeFunction (GET_POINTERID_WPARAM (wParam), &pointerType))
            {
                if (pointerType == 2)
                    return MouseInputSource::InputSourceType::touch;

                if (pointerType == 3)
                    return MouseInputSource::InputSourceType::pen;
            }
        }

        return MouseInputSource::InputSourceType::mouse;
    }

    b8 doMouseWheel (const WPARAM wParam, const b8 isVertical)
    {
        updateKeyModifiers();
        const f32 amount = jlimit (-1000.0f, 1000.0f, 0.5f * (short) HIWORD (wParam));

        MouseWheelDetails wheel;
        wheel.deltaX = isVertical ? 0.0f : amount / -256.0f;
        wheel.deltaY = isVertical ? amount / 256.0f : 0.0f;
        wheel.isReversed = false;
        wheel.isSmooth = false;
        wheel.isInertial = false;

        // From Windows 10 onwards, mouse events are sent first to the window under the mouse, not
        // the window with focus, despite what the MSDN docs might say.
        // This is the behaviour we want; if we're receiving a scroll event, we can assume it
        // should be processed by the current peer.
        const auto localPos = getLocalPointFromScreenLParam ((LPARAM) GetMessagePos());
        handleMouseWheel (getPointerType (wParam), localPos, getMouseEventTime(), wheel);
        return true;
    }

    b8 doGestureEvent (LPARAM lParam)
    {
        GESTUREINFO gi;
        zerostruct (gi);
        gi.cbSize = sizeof (gi);

        if (getGestureInfo != nullptr && getGestureInfo ((HGESTUREINFO) lParam, &gi))
        {
            updateKeyModifiers();

            if (const auto [peer, localPos] = findPeerUnderMouse(); peer != nullptr)
            {
                switch (gi.dwID)
                {
                    case 3: /*GID_ZOOM*/
                        if (gi.dwFlags != 1 /*GF_BEGIN*/ && lastMagnifySize > 0)
                            peer->handleMagnifyGesture (MouseInputSource::InputSourceType::touch, localPos, getMouseEventTime(),
                                                        (f32) ((f64) gi.ullArguments / (f64) lastMagnifySize));

                        lastMagnifySize = gi.ullArguments;
                        return true;

                    case 4: /*GID_PAN*/
                    case 5: /*GID_ROTATE*/
                    case 6: /*GID_TWOFINGERTAP*/
                    case 7: /*GID_PRESSANDTAP*/
                    default:
                        break;
                }
            }
        }

        return false;
    }

    LRESULT doTouchEvent (i32k numInputs, HTOUCHINPUT eventHandle)
    {
        if ((styleFlags & windowIgnoresMouseClicks) != 0)
            if (auto* parent = getOwnerOfWindow (GetParent (hwnd)))
                if (parent != this)
                    return parent->doTouchEvent (numInputs, eventHandle);

        HeapBlock<TOUCHINPUT> inputInfo (numInputs);

        if (getTouchInputInfo (eventHandle, (UINT) numInputs, inputInfo, sizeof (TOUCHINPUT)))
        {
            for (i32 i = 0; i < numInputs; ++i)
            {
                auto flags = inputInfo[i].dwFlags;

                if ((flags & (TOUCHEVENTF_DOWN | TOUCHEVENTF_MOVE | TOUCHEVENTF_UP)) != 0)
                    if (! handleTouchInput (inputInfo[i], (flags & TOUCHEVENTF_DOWN) != 0, (flags & TOUCHEVENTF_UP) != 0))
                        return 0;  // abandon method if this window was deleted by the callback
            }
        }

        closeTouchInputHandle (eventHandle);
        return 0;
    }

    b8 handleTouchInput (const TOUCHINPUT& touch, const b8 isDown, const b8 isUp,
                           const f32 touchPressure = MouseInputSource::defaultPressure,
                           const f32 orientation = 0.0f)
    {
        auto isCancel = false;

        const auto touchIndex = currentTouches.getIndexOfTouch (this, touch.dwID);
        const auto time = getMouseEventTime();
        const auto pos = globalToLocal (convertPhysicalScreenPointToLogical (D2DUtilities::toPoint ({ roundToInt (touch.x / 100.0f),
                                                                                                      roundToInt (touch.y / 100.0f) }), hwnd).toFloat());
        const auto pressure = touchPressure;
        auto modsToSend = ModifierKeys::currentModifiers;

        if (isDown)
        {
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
            modsToSend = ModifierKeys::currentModifiers;

            // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
            handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend.withoutMouseButtons(),
                              pressure, orientation, time, {}, touchIndex);

            if (! isValidPeer (this)) // (in case this component was deleted by the event)
                return false;
        }
        else if (isUp)
        {
            modsToSend = modsToSend.withoutMouseButtons();
            ModifierKeys::currentModifiers = modsToSend;
            currentTouches.clearTouch (touchIndex);

            if (! currentTouches.areAnyTouchesActive())
                isCancel = true;
        }
        else
        {
            modsToSend = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        }

        handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend,
                          pressure, orientation, time, {}, touchIndex);

        if (! isValidPeer (this))
            return false;

        if (isUp)
        {
            handleMouseEvent (MouseInputSource::InputSourceType::touch, MouseInputSource::offscreenMousePos, ModifierKeys::currentModifiers.withoutMouseButtons(),
                              pressure, orientation, time, {}, touchIndex);

            if (! isValidPeer (this))
                return false;

            if (isCancel)
            {
                currentTouches.clear();
                ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons();
            }
        }

        return true;
    }

    b8 handlePointerInput (WPARAM wParam, LPARAM lParam, const b8 isDown, const b8 isUp)
    {
        if (! canUsePointerAPI)
            return false;

        auto pointerType = getPointerType (wParam);

        if (pointerType == MouseInputSource::InputSourceType::touch)
        {
            POINTER_TOUCH_INFO touchInfo;

            if (! getPointerTouchInfo (GET_POINTERID_WPARAM (wParam), &touchInfo))
                return false;

            const auto pressure = touchInfo.touchMask & TOUCH_MASK_PRESSURE ? static_cast<f32> (touchInfo.pressure)
                                                                            : MouseInputSource::defaultPressure;
            const auto orientation = touchInfo.touchMask & TOUCH_MASK_ORIENTATION ? degreesToRadians (static_cast<f32> (touchInfo.orientation))
                                                                                  : MouseInputSource::defaultOrientation;

            if (! handleTouchInput (emulateTouchEventFromPointer (touchInfo.pointerInfo.ptPixelLocationRaw, wParam),
                                    isDown, isUp, pressure, orientation))
                return false;
        }
        else if (pointerType == MouseInputSource::InputSourceType::pen)
        {
            POINTER_PEN_INFO penInfo;

            if (! getPointerPenInfo (GET_POINTERID_WPARAM (wParam), &penInfo))
                return false;

            const auto pressure = (penInfo.penMask & PEN_MASK_PRESSURE) ? (f32) penInfo.pressure / 1024.0f : MouseInputSource::defaultPressure;

            if (! handlePenInput (penInfo, globalToLocal (convertPhysicalScreenPointToLogical (D2DUtilities::toPoint (getPOINTFromLParam (lParam)), hwnd).toFloat()),
                                  pressure, isDown, isUp))
                return false;
        }
        else
        {
            return false;
        }

        return true;
    }

    TOUCHINPUT emulateTouchEventFromPointer (POINT p, WPARAM wParam)
    {
        TOUCHINPUT touchInput;

        touchInput.dwID = GET_POINTERID_WPARAM (wParam);
        touchInput.x = p.x * 100;
        touchInput.y = p.y * 100;

        return touchInput;
    }

    b8 handlePenInput (POINTER_PEN_INFO penInfo, Point<f32> pos, const f32 pressure, b8 isDown, b8 isUp)
    {
        const auto time = getMouseEventTime();
        ModifierKeys modsToSend (ModifierKeys::currentModifiers);
        PenDetails penDetails;

        penDetails.rotation = (penInfo.penMask & PEN_MASK_ROTATION) ? degreesToRadians (static_cast<f32> (penInfo.rotation)) : MouseInputSource::defaultRotation;
        penDetails.tiltX = (penInfo.penMask & PEN_MASK_TILT_X) ? (f32) penInfo.tiltX / 90.0f : MouseInputSource::defaultTiltX;
        penDetails.tiltY = (penInfo.penMask & PEN_MASK_TILT_Y) ? (f32) penInfo.tiltY / 90.0f : MouseInputSource::defaultTiltY;

        auto pInfoFlags = penInfo.pointerInfo.pointerFlags;

        if ((pInfoFlags & POINTER_FLAG_FIRSTBUTTON) != 0)
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        else if ((pInfoFlags & POINTER_FLAG_SECONDBUTTON) != 0)
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::rightButtonModifier);

        if (isDown)
        {
            modsToSend = ModifierKeys::currentModifiers;

            // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
            handleMouseEvent (MouseInputSource::InputSourceType::pen, pos, modsToSend.withoutMouseButtons(),
                              pressure, MouseInputSource::defaultOrientation, time, penDetails);

            if (! isValidPeer (this)) // (in case this component was deleted by the event)
                return false;
        }
        else if (isUp || ! (pInfoFlags & POINTER_FLAG_INCONTACT))
        {
            modsToSend = modsToSend.withoutMouseButtons();
            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons();
        }

        handleMouseEvent (MouseInputSource::InputSourceType::pen, pos, modsToSend, pressure,
                          MouseInputSource::defaultOrientation, time, penDetails);

        if (! isValidPeer (this)) // (in case this component was deleted by the event)
            return false;

        if (isUp)
        {
            handleMouseEvent (MouseInputSource::InputSourceType::pen, MouseInputSource::offscreenMousePos, ModifierKeys::currentModifiers,
                              pressure, MouseInputSource::defaultOrientation, time, penDetails);

            if (! isValidPeer (this))
                return false;
        }

        return true;
    }

    //==============================================================================
    z0 sendModifierKeyChangeIfNeeded()
    {
        if (modifiersAtLastCallback != ModifierKeys::currentModifiers)
        {
            modifiersAtLastCallback = ModifierKeys::currentModifiers;
            handleModifierKeysChange();
        }
    }

    b8 doKeyUp (const WPARAM key)
    {
        updateKeyModifiers();

        switch (key)
        {
            case VK_SHIFT:
            case VK_CONTROL:
            case VK_MENU:
            case VK_CAPITAL:
            case VK_LWIN:
            case VK_RWIN:
            case VK_APPS:
            case VK_NUMLOCK:
            case VK_SCROLL:
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_LCONTROL:
            case VK_LMENU:
            case VK_RCONTROL:
            case VK_RMENU:
                sendModifierKeyChangeIfNeeded();
        }

        return handleKeyUpOrDown (false)
                || Component::getCurrentlyModalComponent() != nullptr;
    }

    b8 doKeyDown (const WPARAM key)
    {
        updateKeyModifiers();
        b8 used = false;

        switch (key)
        {
            case VK_SHIFT:
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL:
            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU:
            case VK_LWIN:
            case VK_RWIN:
            case VK_CAPITAL:
            case VK_NUMLOCK:
            case VK_SCROLL:
            case VK_APPS:
                used = handleKeyUpOrDown (true);
                sendModifierKeyChangeIfNeeded();
                break;

            case VK_LEFT:
            case VK_RIGHT:
            case VK_UP:
            case VK_DOWN:
            case VK_PRIOR:
            case VK_NEXT:
            case VK_HOME:
            case VK_END:
            case VK_DELETE:
            case VK_INSERT:
            case VK_F1:
            case VK_F2:
            case VK_F3:
            case VK_F4:
            case VK_F5:
            case VK_F6:
            case VK_F7:
            case VK_F8:
            case VK_F9:
            case VK_F10:
            case VK_F11:
            case VK_F12:
            case VK_F13:
            case VK_F14:
            case VK_F15:
            case VK_F16:
            case VK_F17:
            case VK_F18:
            case VK_F19:
            case VK_F20:
            case VK_F21:
            case VK_F22:
            case VK_F23:
            case VK_F24:
                used = handleKeyUpOrDown (true);
                used = handleKeyPress (extendedKeyModifier | (i32) key, 0) || used;
                break;

            default:
                used = handleKeyUpOrDown (true);

                {
                    MSG msg;
                    if (! PeekMessage (&msg, hwnd, WM_CHAR, WM_DEADCHAR, PM_NOREMOVE))
                    {
                        // if there isn't a WM_CHAR or WM_DEADCHAR message pending, we need to
                        // manually generate the key-press event that matches this key-down.
                        const UINT keyChar  = MapVirtualKey ((UINT) key, 2);
                        const UINT scanCode = MapVirtualKey ((UINT) key, 0);
                        BYTE keyState[256];
                        [[maybe_unused]] const auto state = GetKeyboardState (keyState);

                        WCHAR text[16] = { 0 };
                        if (ToUnicode ((UINT) key, scanCode, keyState, text, 8, 0) != 1)
                            text[0] = 0;

                        used = handleKeyPress ((i32) LOWORD (keyChar), (t32) text[0]) || used;
                    }
                }

                break;
        }

        return used || (Component::getCurrentlyModalComponent() != nullptr);
    }

    b8 doKeyChar (i32 key, const LPARAM flags)
    {
        updateKeyModifiers();

        auto textChar = (t32) key;
        i32k virtualScanCode = (flags >> 16) & 0xff;

        if (key >= '0' && key <= '9')
        {
            switch (virtualScanCode)  // check for a numeric keypad scan-code
            {
                case 0x52:
                case 0x4f:
                case 0x50:
                case 0x51:
                case 0x4b:
                case 0x4c:
                case 0x4d:
                case 0x47:
                case 0x48:
                case 0x49:
                    key = (key - '0') + KeyPress::numberPad0;
                    break;
                default:
                    break;
            }
        }
        else
        {
            // convert the scan code to an unmodified character code..
            const UINT virtualKey = MapVirtualKey ((UINT) virtualScanCode, 1);
            UINT keyChar = MapVirtualKey (virtualKey, 2);

            keyChar = LOWORD (keyChar);

            if (keyChar != 0)
                key = (i32) keyChar;

            // avoid sending junk text characters for some control-key combinations
            if (textChar < ' ' && ModifierKeys::currentModifiers.testFlags (ModifierKeys::ctrlModifier | ModifierKeys::altModifier))
                textChar = 0;
        }

        return handleKeyPress (key, textChar);
    }

    z0 forwardMessageToParent (UINT message, WPARAM wParam, LPARAM lParam) const
    {
        if (HWND parentH = GetParent (hwnd))
            PostMessage (parentH, message, wParam, lParam);
    }

    b8 doAppCommand (const LPARAM lParam)
    {
        i32 key = 0;

        switch (GET_APPCOMMAND_LPARAM (lParam))
        {
            case APPCOMMAND_MEDIA_PLAY_PAUSE:       key = KeyPress::playKey; break;
            case APPCOMMAND_MEDIA_STOP:             key = KeyPress::stopKey; break;
            case APPCOMMAND_MEDIA_NEXTTRACK:        key = KeyPress::fastForwardKey; break;
            case APPCOMMAND_MEDIA_PREVIOUSTRACK:    key = KeyPress::rewindKey; break;
            default: break;
        }

        if (key != 0)
        {
            updateKeyModifiers();

            if (hwnd == GetActiveWindow())
                return handleKeyPress (key, 0);
        }

        return false;
    }

    b8 isConstrainedNativeWindow() const
    {
        return constrainer != nullptr && ! isKioskMode();
    }

    LRESULT handleSizeConstraining (RECT& r, const WPARAM wParam)
    {
        if (isConstrainedNativeWindow())
        {
            const auto movingTop    = wParam == WMSZ_TOP    || wParam == WMSZ_TOPLEFT    || wParam == WMSZ_TOPRIGHT;
            const auto movingLeft   = wParam == WMSZ_LEFT   || wParam == WMSZ_TOPLEFT    || wParam == WMSZ_BOTTOMLEFT;
            const auto movingBottom = wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT;
            const auto movingRight  = wParam == WMSZ_RIGHT  || wParam == WMSZ_TOPRIGHT   || wParam == WMSZ_BOTTOMRIGHT;

            const auto requestedPhysicalBounds = D2DUtilities::toRectangle (r);
            const auto modifiedPhysicalBounds = getConstrainedBounds (requestedPhysicalBounds,
                                                                      movingTop,
                                                                      movingLeft,
                                                                      movingBottom,
                                                                      movingRight);

            if (! modifiedPhysicalBounds.has_value())
                return TRUE;

            r = D2DUtilities::toRECT (*modifiedPhysicalBounds);
        }

        return TRUE;
    }

    LRESULT handlePositionChanging (WINDOWPOS& wp)
    {
        if (isConstrainedNativeWindow() && ! isFullScreen())
        {
            if ((wp.flags & (SWP_NOMOVE | SWP_NOSIZE)) != (SWP_NOMOVE | SWP_NOSIZE)
                 && (wp.x > -32000 && wp.y > -32000)
                 && ! Component::isMouseButtonDownAnywhere())
            {
                const auto requestedPhysicalBounds = D2DUtilities::toRectangle ({ wp.x, wp.y, wp.x + wp.cx, wp.y + wp.cy });

                if (const auto modifiedPhysicalBounds = getConstrainedBounds (requestedPhysicalBounds, false, false, false, false))
                {
                    wp.x  = modifiedPhysicalBounds->getX();
                    wp.y  = modifiedPhysicalBounds->getY();
                    wp.cx = modifiedPhysicalBounds->getWidth();
                    wp.cy = modifiedPhysicalBounds->getHeight();
                }
            }
        }

        if (((wp.flags & SWP_SHOWWINDOW) != 0 && ! component.isVisible()))
            component.setVisible (true);
        else if (((wp.flags & SWP_HIDEWINDOW) != 0 && component.isVisible()))
            component.setVisible (false);

        return 0;
    }

    std::optional<Rectangle<i32>> getConstrainedBounds (Rectangle<i32> proposed,
                                                        b8 top,
                                                        b8 left,
                                                        b8 bottom,
                                                        b8 right) const
    {
        const auto physicalBorder = findPhysicalBorderSize();

        if (! physicalBorder.has_value())
            return {};

        const auto logicalBorder = getFrameSize();

        // The constrainer expects to operate in logical coordinate space.
        // Additionally, the ComponentPeer can only report the current frame size as an integral
        // number of logical pixels, but at fractional scale factors it may not be possible to
        // express the logical frame size accurately as an integer.
        // To work around this, we replace the physical borders with the currently-reported logical
        // border size before invoking the constrainer.
        // After the constrainer returns, we substitute in the other direction, replacing logical
        // borders with physical.
        const auto requestedPhysicalBounds = proposed;
        const auto requestedPhysicalClient = physicalBorder->subtractedFrom (requestedPhysicalBounds);
        const auto requestedLogicalClient = detail::ScalingHelpers::unscaledScreenPosToScaled (
                component,
                convertPhysicalScreenRectangleToLogical (requestedPhysicalClient, hwnd));
        const auto requestedLogicalBounds = logicalBorder.addedTo (requestedLogicalClient);

        const auto originalLogicalBounds = logicalBorder.addedTo (component.getBounds());

        auto modifiedLogicalBounds = requestedLogicalBounds;

        constrainer->checkBounds (modifiedLogicalBounds,
                                  originalLogicalBounds,
                                  Desktop::getInstance().getDisplays().getTotalBounds (true),
                                  top,
                                  left,
                                  bottom,
                                  right);

        const auto modifiedLogicalClient = logicalBorder.subtractedFrom (modifiedLogicalBounds);
        const auto modifiedPhysicalClient = convertLogicalScreenRectangleToPhysical (
                detail::ScalingHelpers::scaledScreenPosToUnscaled (component, modifiedLogicalClient).toFloat(),
                hwnd);

        const auto closestIntegralSize = modifiedPhysicalClient
                .withPosition (requestedPhysicalClient.getPosition().toFloat())
                .getLargestIntegerWithin();

        const auto withSnappedPosition = [&]
        {
            auto modified = closestIntegralSize;

            if (left || right)
            {
                modified = left ? modified.withRightX (requestedPhysicalClient.getRight())
                                : modified.withX (requestedPhysicalClient.getX());
            }

            if (top || bottom)
            {
                modified = top ? modified.withBottomY (requestedPhysicalClient.getBottom())
                               : modified.withY (requestedPhysicalClient.getY());
            }

            return modified;
        }();

        return physicalBorder->addedTo (withSnappedPosition);
    }

    enum class ForceRefreshDispatcher
    {
        no,
        yes
    };

    static z0 updateVBlankDispatcherForAllPeers (ForceRefreshDispatcher force = ForceRefreshDispatcher::no)
    {
        // There's an edge case where only top-level windows seem to get WM_SETTINGCHANGE
        // messages, which means that if we have a plugin that opens its own top-level/desktop
        // window, then the extra window might get a SETTINGCHANGE but the plugin window may not.
        // If we only update the vblank dispatcher for windows that get a SETTINGCHANGE, we might
        // miss child windows, and those windows won't be able to repaint.

        for (auto i = getNumPeers(); --i >= 0;)
            if (auto* peer = static_cast<HWNDComponentPeer*> (getPeer (i)))
                peer->updateCurrentMonitorAndRefreshVBlankDispatcher (force);
    }

    z0 updateCurrentMonitorAndRefreshVBlankDispatcher (ForceRefreshDispatcher force = ForceRefreshDispatcher::no)
    {
        auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONULL);

        if (std::exchange (currentMonitor, monitor) != monitor || force == ForceRefreshDispatcher::yes)
            VBlankDispatcher::getInstance()->updateDisplay (*this, currentMonitor);
    }

    b8 handlePositionChanged()
    {
        auto pos = getCurrentMousePos();

        if (contains (pos.roundToInt(), false))
        {
            const ScopedValueSetter<b8> scope (inHandlePositionChanged, true);

            if (! areOtherTouchSourcesActive())
                doMouseEvent (pos, MouseInputSource::defaultPressure);

            if (! isValidPeer (this))
                return true;
        }

        handleMovedOrResized();
        updateCurrentMonitorAndRefreshVBlankDispatcher();

        return ! dontRepaint; // to allow non-accelerated openGL windows to draw themselves correctly.
    }

    //==============================================================================
    LRESULT handleDPIChanging (i32 newDPI, RECT newRect)
    {
        // Sometimes, windows that should not be automatically scaled (secondary windows in plugins)
        // are sent WM_DPICHANGED. The size suggested by the OS is incorrect for our unscaled
        // window, so we should ignore it.
        if (! isPerMonitorDPIAwareWindow (hwnd))
            return 0;

        const auto newScale = (f64) newDPI / USER_DEFAULT_SCREEN_DPI;

        if (approximatelyEqual (scaleFactor, newScale))
            return 0;

        scaleFactor = newScale;

        {
            const ScopedValueSetter<b8> setter (inDpiChange, true);
            SetWindowPos (hwnd,
                          nullptr,
                          newRect.left,
                          newRect.top,
                          newRect.right  - newRect.left,
                          newRect.bottom - newRect.top,
                          SWP_NOZORDER | SWP_NOACTIVATE);
        }

        // This is to handle reentrancy. If responding to a DPI change triggers further DPI changes,
        // we should only notify listeners and resize windows once all of the DPI changes have
        // resolved.
        if (inDpiChange)
        {
            // Danger! Re-entrant call to handleDPIChanging.
            // Please report this issue on the DRX forum, along with instructions
            // so that a DRX developer can reproduce the issue.
            jassertfalse;
            return 0;
        }

        updateShadower();
        InvalidateRect (hwnd, nullptr, FALSE);

        scaleFactorListeners.call ([this] (ScaleFactorListener& l) { l.nativeScaleFactorChanged (scaleFactor); });

        return 0;
    }

    //==============================================================================
    z0 handleAppActivation (const WPARAM wParam)
    {
        modifiersAtLastCallback = -1;
        updateKeyModifiers();

        if (isMinimised())
        {
            component.repaint();
            handleMovedOrResized();

            if (! isValidPeer (this))
                return;
        }

        auto* underMouse = component.getComponentAt (component.getMouseXYRelative());

        if (underMouse == nullptr)
            underMouse = &component;

        if (underMouse->isCurrentlyBlockedByAnotherModalComponent())
        {
            if (LOWORD (wParam) == WA_CLICKACTIVE)
                Component::getCurrentlyModalComponent()->inputAttemptWhenModal();
            else
                ModalComponentManager::getInstance()->bringModalComponentsToFront();
        }
        else
        {
            handleBroughtToFront();
        }
    }

    z0 handlePowerBroadcast (WPARAM wParam)
    {
        if (auto* app = DRXApplicationBase::getInstance())
        {
            switch (wParam)
            {
                case PBT_APMSUSPEND:                app->suspended(); break;

                case PBT_APMQUERYSUSPENDFAILED:
                case PBT_APMRESUMECRITICAL:
                case PBT_APMRESUMESUSPEND:
                case PBT_APMRESUMEAUTOMATIC:        app->resumed(); break;

                default: break;
            }
        }
    }

    z0 handleLeftClickInNCArea (WPARAM wParam)
    {
        if (sendInputAttemptWhenModalMessage())
            return;

        switch (wParam)
        {
        case HTBOTTOM:
        case HTBOTTOMLEFT:
        case HTBOTTOMRIGHT:
        case HTGROWBOX:
        case HTLEFT:
        case HTRIGHT:
        case HTTOP:
        case HTTOPLEFT:
        case HTTOPRIGHT:
            if (isConstrainedNativeWindow())
            {
                constrainerIsResizing = true;
                constrainer->resizeStart();
            }
            return;

        default:
            break;
        }
    }

    z0 initialiseSysMenu (HMENU menu) const
    {
        if (! hasTitleBar())
        {
            if (isFullScreen())
            {
                EnableMenuItem (menu, SC_RESTORE,  MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem (menu, SC_MOVE,     MF_BYCOMMAND | MF_GRAYED);
            }
            else if (! isMinimised())
            {
                EnableMenuItem (menu, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
            }
        }
    }

    z0 doSettingChange()
    {
        forceDisplayUpdate();

        auto* dispatcher = VBlankDispatcher::getInstance();
        dispatcher->reconfigureDisplays();
        updateVBlankDispatcherForAllPeers (ForceRefreshDispatcher::yes);
    }

    //==============================================================================
   #if DRX_MODULE_AVAILABLE_drx_audio_plugin_client
    z0 setModifierKeyProvider (ModifierKeyProvider* provider) override
    {
        modProvider = provider;
    }

    z0 removeModifierKeyProvider() override
    {
        modProvider = nullptr;
    }
   #endif

    static LRESULT CALLBACK windowProc (HWND h, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // Ensure that non-client areas are scaled for per-monitor DPI awareness v1 - can't
        // do this in peerWindowProc as we have no window at this point
        if (message == WM_NCCREATE)
            NullCheckedInvocation::invoke (enableNonClientDPIScaling, h);

        if (auto* peer = getOwnerOfWindow (h))
        {
            jassert (isValidPeer (peer));
            return peer->peerWindowProc (h, message, wParam, lParam);
        }

        return DefWindowProcW (h, message, wParam, lParam);
    }

    static uk callFunctionIfNotLocked (MessageCallbackFunction* callback, uk userData)
    {
        auto& mm = *MessageManager::getInstance();

        if (mm.currentThreadHasLockedMessageManager())
            return callback (userData);

        return mm.callFunctionOnMessageThread (callback, userData);
    }

    static POINT getPOINTFromLParam (LPARAM lParam) noexcept
    {
        return { GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam) };
    }

    Point<f32> getLocalPointFromScreenLParam (LPARAM lParam)
    {
        const auto globalPos = D2DUtilities::toPoint (getPOINTFromLParam (lParam));
        return globalToLocal (convertPhysicalScreenPointToLogical (globalPos, hwnd).toFloat());
    }

    Point<f32> getPointFromLocalLParam (LPARAM lParam) noexcept
    {
        const auto p = D2DUtilities::toPoint (getPOINTFromLParam (lParam));

        if (! isPerMonitorDPIAwareWindow (hwnd))
            return p.toFloat();

        // LPARAM is relative to this window's top-left but may be on a different monitor so we need to calculate the
        // physical screen position and then convert this to local logical coordinates
        auto r = getWindowScreenRect (hwnd);
        const auto windowBorder = findPhysicalBorderSize().value_or (BorderSize<i32>{});
        const auto offset = p
                          + Point { (i32) r.left, (i32) r.top }
                          + Point { windowBorder.getLeft(), windowBorder.getTop() };
        return globalToLocal (Desktop::getInstance().getDisplays().physicalToLogical (offset).toFloat());
    }

    Point<f32> getCurrentMousePos() noexcept
    {
        return globalToLocal (convertPhysicalScreenPointToLogical (D2DUtilities::toPoint (getPOINTFromLParam ((LPARAM) GetMessagePos())), hwnd).toFloat());
    }

    static ModifierKeys getMouseModifiers()
    {
        updateKeyModifiers();

        i32 mouseMods = 0;
        if (isKeyDown (VK_LBUTTON))  mouseMods |= ModifierKeys::leftButtonModifier;
        if (isKeyDown (VK_RBUTTON))  mouseMods |= ModifierKeys::rightButtonModifier;
        if (isKeyDown (VK_MBUTTON))  mouseMods |= ModifierKeys::middleButtonModifier;

        ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (mouseMods);

        return ModifierKeys::currentModifiers;
    }

    std::optional<LRESULT> onNcLButtonDown (WPARAM wParam, LPARAM lParam)
    {
        handleLeftClickInNCArea (wParam);

        switch (wParam)
        {
            case HTCLOSE:
            case HTMAXBUTTON:
            case HTMINBUTTON:
                // The default implementation in DefWindowProc for these functions has some
                // unwanted behaviour. Specifically, it seems to draw some ugly grey buttons over
                // our custom nonclient area, just for one frame.
                // To avoid this, we handle the message ourselves. The actual handling happens
                // in WM_NCLBUTTONUP.
                return 0;

            case HTCAPTION:
                // The default click-in-caption handler appears to block the message loop until a
                // mouse move is detected, which prevents the view from repainting. We want to keep
                // painting, so log the click ourselves and only defer to DefWindowProc once the
                // mouse moves with the button held.
                captionMouseDown = lParam;
                return 0;
        }

        return {};
    }

    LRESULT peerWindowProc (HWND h, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            //==============================================================================
            case WM_NCHITTEST:
            {
                if ((styleFlags & windowIgnoresMouseClicks) != 0)
                    return HTTRANSPARENT;

                if (! hasTitleBar() && (styleFlags & windowIsTemporary) == 0 && parentToAddTo == nullptr)
                {
                    if ((styleFlags & windowIsResizable) != 0)
                        if (const auto result = DefWindowProc (h, message, wParam, lParam); HTSIZEFIRST <= result && result <= HTSIZELAST)
                            return result;

                    const auto physicalPoint = D2DUtilities::toPoint (getPOINTFromLParam (lParam));
                    const auto logicalPoint = convertPhysicalScreenPointToLogical (physicalPoint, hwnd);
                    const auto localPoint = globalToLocal (logicalPoint.toFloat());
                    const auto componentPoint = detail::ScalingHelpers::unscaledScreenPosToScaled (component, localPoint);

                    const auto kind = component.findControlAtPoint (componentPoint);

                    using Kind = Component::WindowControlKind;
                    switch (kind)
                    {
                        case Kind::caption:         return HTCAPTION;
                        case Kind::minimise:        return HTMINBUTTON;
                        case Kind::maximise:        return HTMAXBUTTON;
                        case Kind::close:           return HTCLOSE;
                        case Kind::sizeTop:         return HTTOP;
                        case Kind::sizeLeft:        return HTLEFT;
                        case Kind::sizeRight:       return HTRIGHT;
                        case Kind::sizeBottom:      return HTBOTTOM;
                        case Kind::sizeTopLeft:     return HTTOPLEFT;
                        case Kind::sizeTopRight:    return HTTOPRIGHT;
                        case Kind::sizeBottomLeft:  return HTBOTTOMLEFT;
                        case Kind::sizeBottomRight: return HTBOTTOMRIGHT;

                        case Kind::client:
                            break;
                    }

                    // For a bordered window, Windows would normally let you resize by hovering just
                    // outside the client area (over the drop shadow).
                    // When we disable the border by doing nothing in WM_NCCALCSIZE, the client
                    // size will match the total window size.
                    // It seems that, when there's no nonclient area, Windows won't send us
                    // WM_NCHITTEST when hovering the window shadow.
                    // We only start getting NCHITTEST messages once the cursor is inside the client
                    // area.
                    // The upshot of all this is that we need to emulate the resizable border
                    // ourselves, but inside the window.
                    // Other borderless apps (1Password, Spotify, VS Code) seem to do the same thing,
                    // and if Microsoft's own VS Code doesn't have perfect mouse handling I don't
                    // think we can be expected to either!

                    if ((styleFlags & windowIsResizable) != 0)
                    {
                        const ScopedThreadDPIAwarenessSetter scope { hwnd };

                        const auto cursor = getPOINTFromLParam (lParam);
                        RECT client{};
                        ::GetWindowRect (h, &client);

                        const auto dpi = GetDpiForWindow (hwnd);
                        const auto padding = GetSystemMetricsForDpi (SM_CXPADDEDBORDER, dpi);
                        const auto borderX = GetSystemMetricsForDpi (SM_CXFRAME, dpi) + padding;
                        const auto borderY = GetSystemMetricsForDpi (SM_CYFRAME, dpi) + padding;

                        const auto left   = cursor.x < client.left + borderX;
                        const auto right  = client.right - borderX < cursor.x;
                        const auto top    = cursor.y < client.top + borderY;
                        const auto bottom = client.bottom - borderY < cursor.y;

                        enum Bits
                        {
                            bitL = 1 << 0,
                            bitR = 1 << 1,
                            bitT = 1 << 2,
                            bitB = 1 << 3,
                        };

                        const auto positionMask = (left   ? bitL : 0)
                                                | (right  ? bitR : 0)
                                                | (top    ? bitT : 0)
                                                | (bottom ? bitB : 0);

                        switch (positionMask)
                        {
                            case bitL: return HTLEFT;
                            case bitR: return HTRIGHT;
                            case bitT: return HTTOP;
                            case bitB: return HTBOTTOM;

                            case bitT | bitL: return HTTOPLEFT;
                            case bitT | bitR: return HTTOPRIGHT;
                            case bitB | bitL: return HTBOTTOMLEFT;
                            case bitB | bitR: return HTBOTTOMRIGHT;
                        }
                    }

                    return HTCLIENT;
                }

                break;
            }

            //==============================================================================
            case WM_PAINT:
                handlePaintMessage();
                return 0;

            case WM_NCPAINT:
                // this must be done, even with native titlebars, or there are rendering artifacts.
                handlePaintMessage();
                // Even if we're *not* using a native titlebar (i.e. extending into the nonclient area)
                // the system needs to handle the NCPAINT to draw rounded corners and shadows.
                break;

            case WM_ERASEBKGND:
                if (hasTitleBar())
                    break;

                return 1;

            case WM_NCCALCSIZE:
            {
                // If we're using the native titlebar, then the default window proc behaviour will
                // do the right thing.
                if (hasTitleBar())
                    break;

                auto* param = (RECT*) lParam;

                // If we're not using a native titlebar, and the window is maximised, then the
                // proposed window may be a bit bigger than the available space. Remove the padding
                // so that the client area exactly fills the available space.
                if (isFullScreen())
                {
                    const auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONULL);

                    if (monitor == nullptr)
                        return 0;

                    MONITORINFOEX info{};
                    info.cbSize = sizeof (info);
                    GetMonitorInfo (monitor, &info);

                    const auto padX = info.rcMonitor.left - param->left;
                    const auto padY = info.rcMonitor.top - param->top;

                    param->left  += padX;
                    param->right -= padX;

                    param->top    += padY;
                    param->bottom -= padY;
                }

                return 0;
            }

            //==============================================================================
            case WM_POINTERUPDATE:
                if (handlePointerInput (wParam, lParam, false, false))
                    return 0;
                break;

            case WM_POINTERDOWN:
                if (handlePointerInput (wParam, lParam, true, false))
                    return 0;
                break;

            case WM_POINTERUP:
                if (handlePointerInput (wParam, lParam, false, true))
                    return 0;
                break;

            //==============================================================================
            case WM_NCMOUSEMOVE:
            case WM_MOUSEMOVE:
                return doMouseMove (lParam, false, message == WM_MOUSEMOVE ? WindowArea::client : WindowArea::nonclient).value_or (0);

            case WM_POINTERLEAVE:
            case WM_NCMOUSELEAVE:
            case WM_MOUSELEAVE:
                doMouseExit();
                return 0;

            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
                doMouseDown (lParam, wParam);
                return 0;

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
                doMouseUp (getPointFromLocalLParam (lParam), wParam);
                return 0;

            case WM_POINTERWHEEL:
            case WM_MOUSEWHEEL:
                if (doMouseWheel (wParam, true))
                    return 0;
                break;

            case WM_POINTERHWHEEL:
            case WM_MOUSEHWHEEL:
                if (doMouseWheel (wParam, false))
                    return 0;
                break;

            case WM_CAPTURECHANGED:
                doCaptureChanged();
                return 0;

            case WM_TOUCH:
                if (getTouchInputInfo != nullptr)
                    return doTouchEvent ((i32) wParam, (HTOUCHINPUT) lParam);

                break;

            case 0x119: /* WM_GESTURE */
                if (doGestureEvent (lParam))
                    return 0;

                break;

            //==============================================================================
            case WM_ENTERSIZEMOVE:
                sizing = true;
                break;

            case WM_EXITSIZEMOVE:
                sizing = false;
                break;

            case WM_SIZING:
                sizing = true;
                return handleSizeConstraining (*(RECT*) lParam, wParam);

            case WM_MOVING:
                return handleSizeConstraining (*(RECT*) lParam, 0);

            case WM_WINDOWPOSCHANGING:
                if (hasTitleBar() && sizing)
                    break;

                return handlePositionChanging (*(WINDOWPOS*) lParam);

            case 0x2e0: /* WM_DPICHANGED */  return handleDPIChanging ((i32) HIWORD (wParam), *(RECT*) lParam);

            case WM_WINDOWPOSCHANGED:
            {
                const WINDOWPOS& wPos = *reinterpret_cast<WINDOWPOS*> (lParam);

                if ((wPos.flags & SWP_NOMOVE) != 0 && (wPos.flags & SWP_NOSIZE) != 0)
                    startTimer (100);
                else if (handlePositionChanged())
                    return 0;
            }
            break;

            //==============================================================================
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                if (doKeyDown (wParam))
                    return 0;

                forwardMessageToParent (message, wParam, lParam);
                break;

            case WM_KEYUP:
            case WM_SYSKEYUP:
                if (doKeyUp (wParam))
                    return 0;

                forwardMessageToParent (message, wParam, lParam);
                break;

            case WM_CHAR:
                if (doKeyChar ((i32) wParam, lParam))
                    return 0;

                forwardMessageToParent (message, wParam, lParam);
                break;

            case WM_APPCOMMAND:
                if (doAppCommand (lParam))
                    return TRUE;

                break;

            case WM_MENUCHAR: // triggered when alt+something is pressed
                return MNC_CLOSE << 16; // (avoids making the default system beep)

            //==============================================================================
            case WM_SETFOCUS:
                /*  When the HWND receives Focus from the system it sends a
                    UIA_AutomationFocusChangedEventId notification redirecting the focus to the HWND
                    itself. This is a built-in behaviour of the HWND.

                    This means that whichever DRX managed provider was active before the entire
                    window lost and then regained the focus, loses its focused state, and the
                    window's root element will become focused under which all DRX managed providers
                    can be found.

                    This needs to be reflected on currentlyFocusedHandler so that the DRX
                    accessibility mechanisms can detect that the root window got the focus, and send
                    another FocusChanged event to the system to redirect focus to a DRX managed
                    provider if necessary.
                */
                AccessibilityHandler::clearCurrentlyFocusedHandler();
                updateKeyModifiers();
                handleFocusGain();
                break;

            case WM_KILLFOCUS:
                if (hasCreatedCaret)
                {
                    hasCreatedCaret = false;
                    DestroyCaret();
                }

                handleFocusLoss();

                if (auto* modal = Component::getCurrentlyModalComponent())
                    if (auto* peer = modal->getPeer())
                        if ((peer->getStyleFlags() & windowIsTemporary) != 0)
                            sendInputAttemptWhenModalMessage();

                break;

            case WM_ACTIVATEAPP:
                // Windows does weird things to process priority when you swap apps,
                // so this forces an update when the app is brought to the front
                if (wParam != FALSE)
                    drx_repeatLastProcessPriority();
                else
                    Desktop::getInstance().setKioskModeComponent (nullptr); // turn kiosk mode off if we lose focus

                detail::TopLevelWindowManager::checkCurrentlyFocusedTopLevelWindow();
                modifiersAtLastCallback = -1;
                return 0;

            case WM_ACTIVATE:
                if (LOWORD (wParam) == WA_ACTIVE || LOWORD (wParam) == WA_CLICKACTIVE)
                {
                    handleAppActivation (wParam);
                    return 0;
                }

                break;

            case WM_NCACTIVATE:
                // while a temporary window is being shown, prevent Windows from deactivating the
                // title bars of our main windows.
                if (wParam == 0 && ! shouldDeactivateTitleBar)
                    wParam = TRUE; // change this and let it get passed to the DefWindowProc.

                break;

            case WM_POINTERACTIVATE:
            case WM_MOUSEACTIVATE:
                if (! component.getMouseClickGrabsKeyboardFocus())
                    return MA_NOACTIVATE;

                break;

            case WM_SHOWWINDOW:
                if (wParam != 0)
                {
                    component.setVisible (true);
                    handleBroughtToFront();

                    if (renderContext != nullptr)
                        renderContext->handleShowWindow();
                }

                break;

            case WM_CLOSE:
                if (! component.isCurrentlyBlockedByAnotherModalComponent())
                    handleUserClosingWindow();

                return 0;

           #if DRX_REMOVE_COMPONENT_FROM_DESKTOP_ON_WM_DESTROY
            case WM_DESTROY:
                getComponent().removeFromDesktop();
                return 0;
           #endif

            case WM_QUERYENDSESSION:
                if (auto* app = DRXApplicationBase::getInstance())
                {
                    app->systemRequestedQuit();
                    return MessageManager::getInstance()->hasStopMessageBeenSent();
                }
                return TRUE;

            case WM_POWERBROADCAST:
                handlePowerBroadcast (wParam);
                break;

            case WM_SYNCPAINT:
                return 0;

            case WM_DISPLAYCHANGE:
                InvalidateRect (h, nullptr, 0);
                // intentional fall-through...
                DRX_FALLTHROUGH
            case WM_SETTINGCHANGE:  // note the fall-through in the previous case!
                doSettingChange();
                break;

            case WM_INITMENU:
                initialiseSysMenu ((HMENU) wParam);
                break;

            case WM_SYSCOMMAND:
                switch (wParam & 0xfff0)
                {
                case SC_CLOSE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    PostMessage (h, WM_CLOSE, 0, 0);
                    return 0;

                case SC_KEYMENU:
                   #if ! DRX_WINDOWS_ALT_KEY_TRIGGERS_MENU
                    // This test prevents a press of the ALT key from triggering the ancient top-left window menu.
                    // By default we suppress this behaviour because it's unlikely that more than a tiny subset of
                    // our users will actually want it, and it causes problems if you're trying to use the ALT key
                    // as a modifier for mouse actions. If you really need the old behaviour, then just define
                    // DRX_WINDOWS_ALT_KEY_TRIGGERS_MENU=1 in your app.
                    if ((lParam >> 16) <= 0) // Values above zero indicate that a mouse-click triggered the menu
                        return 0;
                   #endif

                    // (NB mustn't call sendInputAttemptWhenModalMessage() here because of very obscure
                    // situations that can arise if a modal loop is started from an alt-key keypress).
                    if (h == GetCapture())
                        ReleaseCapture();

                    break;

                case SC_MAXIMIZE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    setFullScreen (true);
                    return 0;

                case SC_MINIMIZE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    setMinimised (true);
                    return 0;

                case SC_RESTORE:
                    if (sendInputAttemptWhenModalMessage())
                        return 0;

                    if (hasTitleBar())
                    {
                        if (isFullScreen())
                        {
                            setFullScreen (false);
                            return 0;
                        }
                    }
                    else
                    {
                        if (isMinimised())
                            setMinimised (false);
                        else if (isFullScreen())
                            setFullScreen (false);

                        return 0;
                    }
                    break;
                }

                break;

            case WM_NCPOINTERDOWN:
                handleLeftClickInNCArea (HIWORD (wParam));
                break;

            case WM_NCLBUTTONDOWN:
            {
                if (auto result = onNcLButtonDown (wParam, lParam))
                    return *result;

                break;
            }

            case WM_NCLBUTTONUP:
                switch (wParam)
                {
                    case HTCLOSE:
                        if ((styleFlags & windowHasCloseButton) != 0 && ! sendInputAttemptWhenModalMessage())
                        {
                            if (hasTitleBar())
                                PostMessage (h, WM_CLOSE, 0, 0);
                            else
                                component.windowControlClickedClose();
                        }
                        return 0;

                    case HTMAXBUTTON:
                        if ((styleFlags & windowHasMaximiseButton) != 0 && ! sendInputAttemptWhenModalMessage())
                        {
                            if (hasTitleBar())
                                setFullScreen (! isFullScreen());
                            else
                                component.windowControlClickedMaximise();
                        }
                        return 0;

                    case HTMINBUTTON:
                        if ((styleFlags & windowHasMinimiseButton) != 0 && ! sendInputAttemptWhenModalMessage())
                        {
                            if (hasTitleBar())
                                setMinimised (true);
                            else
                                component.windowControlClickedMinimise();
                        }
                        return 0;
                }
                break;

            case WM_NCRBUTTONDOWN:
            case WM_NCMBUTTONDOWN:
                sendInputAttemptWhenModalMessage();
                return 0;

            case WM_IME_SETCONTEXT:
                imeHandler.handleSetContext (h, wParam == TRUE);
                lParam &= ~(LPARAM) ISC_SHOWUICOMPOSITIONWINDOW;
                break;

            case WM_IME_STARTCOMPOSITION:  imeHandler.handleStartComposition (*this); return 0;
            case WM_IME_ENDCOMPOSITION:    imeHandler.handleEndComposition (*this, h); return 0;
            case WM_IME_COMPOSITION:       imeHandler.handleComposition (*this, h, lParam); return 0;

            case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS;

            case WM_GETOBJECT:
            {
                if (static_cast<i64> (lParam) == WindowsAccessibility::getUiaRootObjectId())
                {
                    if (auto* handler = component.getAccessibilityHandler())
                    {
                        LRESULT res = 0;

                        if (WindowsAccessibility::handleWmGetObject (handler, wParam, lParam, &res))
                        {
                            isAccessibilityActive = true;
                            return res;
                        }
                    }
                }

                break;
            }
            default:
                break;
        }

        return DefWindowProc (h, message, wParam, lParam);
    }

    b8 sendInputAttemptWhenModalMessage()
    {
        if (! component.isCurrentlyBlockedByAnotherModalComponent())
            return false;

        if (auto* current = Component::getCurrentlyModalComponent())
            if (auto* owner = getOwnerOfWindow ((HWND) current->getWindowHandle()))
                if (! owner->shouldIgnoreModalDismiss)
                    current->inputAttemptWhenModal();

        return true;
    }

    //==============================================================================
    struct IMEHandler
    {
        IMEHandler()
        {
            reset();
        }

        z0 handleSetContext (HWND hWnd, const b8 windowIsActive)
        {
            if (compositionInProgress && ! windowIsActive)
            {
                if (HIMC hImc = ImmGetContext (hWnd))
                {
                    ImmNotifyIME (hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
                    ImmReleaseContext (hWnd, hImc);
                }

                // If the composition is still in progress, calling ImmNotifyIME may call back
                // into handleComposition to let us know that the composition has finished.
                // We need to set compositionInProgress *after* calling handleComposition, so that
                // the text replaces the current selection, rather than being inserted after the
                // caret.
                compositionInProgress = false;
            }
        }

        z0 handleStartComposition (ComponentPeer& owner)
        {
            reset();

            if (auto* target = owner.findCurrentTextInputTarget())
                target->insertTextAtCaret (Txt());
        }

        z0 handleEndComposition (ComponentPeer& owner, HWND hWnd)
        {
            if (compositionInProgress)
            {
                // If this occurs, the user has cancelled the composition, so clear their changes..
                if (auto* target = owner.findCurrentTextInputTarget())
                {
                    target->setHighlightedRegion (compositionRange);
                    target->insertTextAtCaret (Txt());
                    compositionRange.setLength (0);

                    target->setHighlightedRegion (Range<i32>::emptyRange (compositionRange.getEnd()));
                    target->setTemporaryUnderlining ({});
                }

                if (auto hImc = ImmGetContext (hWnd))
                {
                    ImmNotifyIME (hImc, NI_CLOSECANDIDATE, 0, 0);
                    ImmReleaseContext (hWnd, hImc);
                }
            }

            reset();
        }

        z0 handleComposition (ComponentPeer& owner, HWND hWnd, const LPARAM lParam)
        {
            if (auto* target = owner.findCurrentTextInputTarget())
            {
                if (auto hImc = ImmGetContext (hWnd))
                {
                    if (compositionRange.getStart() < 0)
                        compositionRange = Range<i32>::emptyRange (target->getHighlightedRegion().getStart());

                    if ((lParam & GCS_RESULTSTR) != 0) // (composition has finished)
                    {
                        replaceCurrentSelection (target, getCompositionString (hImc, GCS_RESULTSTR),
                                                 Range<i32>::emptyRange (-1));

                        reset();
                        target->setTemporaryUnderlining ({});
                    }
                    else if ((lParam & GCS_COMPSTR) != 0) // (composition is still in-progress)
                    {
                        replaceCurrentSelection (target, getCompositionString (hImc, GCS_COMPSTR),
                                                 getCompositionSelection (hImc, lParam));

                        target->setTemporaryUnderlining (getCompositionUnderlines (hImc, lParam));
                        compositionInProgress = true;
                    }

                    moveCandidateWindowToLeftAlignWithSelection (hImc, owner, target);
                    ImmReleaseContext (hWnd, hImc);
                }
            }
        }

    private:
        //==============================================================================
        Range<i32> compositionRange; // The range being modified in the TextInputTarget
        b8 compositionInProgress;

        //==============================================================================
        z0 reset()
        {
            compositionRange = Range<i32>::emptyRange (-1);
            compositionInProgress = false;
        }

        Txt getCompositionString (HIMC hImc, const DWORD type) const
        {
            jassert (hImc != HIMC{});

            const auto stringSizeBytes = ImmGetCompositionString (hImc, type, nullptr, 0);

            if (stringSizeBytes > 0)
            {
                HeapBlock<TCHAR> buffer;
                buffer.calloc ((size_t) stringSizeBytes / sizeof (TCHAR) + 1);
                ImmGetCompositionString (hImc, type, buffer, (DWORD) stringSizeBytes);
                return Txt (buffer.get());
            }

            return {};
        }

        i32 getCompositionCaretPos (HIMC hImc, LPARAM lParam, const Txt& currentIMEString) const
        {
            jassert (hImc != HIMC{});

            if ((lParam & CS_NOMOVECARET) != 0)
                return compositionRange.getStart();

            if ((lParam & GCS_CURSORPOS) != 0)
            {
                i32k localCaretPos = ImmGetCompositionString (hImc, GCS_CURSORPOS, nullptr, 0);
                return compositionRange.getStart() + jmax (0, localCaretPos);
            }

            return compositionRange.getStart() + currentIMEString.length();
        }

        // Get selected/highlighted range while doing composition:
        // returned range is relative to beginning of TextInputTarget, not composition string
        Range<i32> getCompositionSelection (HIMC hImc, LPARAM lParam) const
        {
            jassert (hImc != HIMC{});
            i32 selectionStart = 0;
            i32 selectionEnd = 0;

            if ((lParam & GCS_COMPATTR) != 0)
            {
                // Get size of attributes array:
                i32k attributeSizeBytes = ImmGetCompositionString (hImc, GCS_COMPATTR, nullptr, 0);

                if (attributeSizeBytes > 0)
                {
                    // Get attributes (8 bit flag per character):
                    HeapBlock<t8> attributes (attributeSizeBytes);
                    ImmGetCompositionString (hImc, GCS_COMPATTR, attributes, (DWORD) attributeSizeBytes);

                    selectionStart = 0;

                    for (selectionStart = 0; selectionStart < attributeSizeBytes; ++selectionStart)
                        if (attributes[selectionStart] == ATTR_TARGET_CONVERTED || attributes[selectionStart] == ATTR_TARGET_NOTCONVERTED)
                            break;

                    for (selectionEnd = selectionStart; selectionEnd < attributeSizeBytes; ++selectionEnd)
                        if (attributes[selectionEnd] != ATTR_TARGET_CONVERTED && attributes[selectionEnd] != ATTR_TARGET_NOTCONVERTED)
                            break;
                }
            }

            return Range<i32> (selectionStart, selectionEnd) + compositionRange.getStart();
        }

        z0 replaceCurrentSelection (TextInputTarget* const target, const Txt& newContent, Range<i32> newSelection)
        {
            if (compositionInProgress)
                target->setHighlightedRegion (compositionRange);

            target->insertTextAtCaret (newContent);
            compositionRange.setLength (newContent.length());

            if (newSelection.getStart() < 0)
                newSelection = Range<i32>::emptyRange (compositionRange.getEnd());

            target->setHighlightedRegion (newSelection);
        }

        Array<Range<i32>> getCompositionUnderlines (HIMC hImc, LPARAM lParam) const
        {
            Array<Range<i32>> result;

            if (hImc != HIMC{} && (lParam & GCS_COMPCLAUSE) != 0)
            {
                auto clauseDataSizeBytes = ImmGetCompositionString (hImc, GCS_COMPCLAUSE, nullptr, 0);

                if (clauseDataSizeBytes > 0)
                {
                    const auto numItems = (size_t) clauseDataSizeBytes / sizeof (u32);
                    HeapBlock<u32> clauseData (numItems);

                    if (ImmGetCompositionString (hImc, GCS_COMPCLAUSE, clauseData, (DWORD) clauseDataSizeBytes) > 0)
                        for (size_t i = 0; i + 1 < numItems; ++i)
                            result.add (Range<i32> ((i32) clauseData[i], (i32) clauseData[i + 1]) + compositionRange.getStart());
                }
            }

            return result;
        }

        z0 moveCandidateWindowToLeftAlignWithSelection (HIMC hImc, ComponentPeer& peer, TextInputTarget* target) const
        {
            if (auto* targetComp = dynamic_cast<Component*> (target))
            {
                const auto screenPos = targetComp->localPointToGlobal (target->getCaretRectangle().getBottomLeft());
                const auto relativePos = peer.globalToLocal (screenPos) * peer.getPlatformScaleFactor();

                CANDIDATEFORM pos { 0, CFS_CANDIDATEPOS, D2DUtilities::toPOINT (relativePos), { 0, 0, 0, 0 } };
                ImmSetCandidateWindow (hImc, &pos);
            }
        }

        DRX_DECLARE_NON_COPYABLE (IMEHandler)
    };

    z0 timerCallback() override
    {
        handlePositionChanged();
        stopTimer();
    }

    static b8 isAncestor (HWND outer, HWND inner)
    {
        if (outer == nullptr || inner == nullptr)
            return false;

        if (outer == inner)
            return true;

        return isAncestor (outer, GetAncestor (inner, GA_PARENT));
    }

    z0 windowShouldDismissModals (HWND originator)
    {
        if (shouldIgnoreModalDismiss)
            return;

        if (isAncestor (originator, hwnd))
            sendInputAttemptWhenModalMessage();
    }

    // Unfortunately SetWindowsHookEx only allows us to register a static function as a hook.
    // To get around this, we keep a static list of listeners which are interested in
    // top-level window events, and notify all of these listeners from the callback.
    class TopLevelModalDismissBroadcaster
    {
    public:
        TopLevelModalDismissBroadcaster()
            : hook (SetWindowsHookEx (WH_CALLWNDPROC,
                                      callWndProc,
                                      (HINSTANCE) drx::Process::getCurrentModuleInstanceHandle(),
                                      GetCurrentThreadId()))
        {}

        ~TopLevelModalDismissBroadcaster() noexcept
        {
            UnhookWindowsHookEx (hook);
        }

    private:
        static z0 processMessage (i32 nCode, const CWPSTRUCT* info)
        {
            if (nCode < 0 || info == nullptr)
                return;

            constexpr UINT events[] { WM_MOVE,
                                      WM_SIZE,
                                      WM_WINDOWPOSCHANGING,
                                      WM_NCPOINTERDOWN,
                                      WM_NCLBUTTONDOWN,
                                      WM_NCRBUTTONDOWN,
                                      WM_NCMBUTTONDOWN };

            if (std::find (std::begin (events), std::end (events), info->message) == std::end (events))
                return;

            if (info->message == WM_WINDOWPOSCHANGING)
            {
                const auto* windowPos = reinterpret_cast<const WINDOWPOS*> (info->lParam);
                const auto windowPosFlags = windowPos->flags;

                constexpr auto maskToCheck = SWP_NOMOVE | SWP_NOSIZE;

                // This undocumented bit seems to get set when minimising/maximising windows with Win+D.
                // If we attempt to dismiss modals while this bit is set, we might end up bringing
                // modals to the front, which in turn may attempt to un-minimise them.
                constexpr auto SWP_STATECHANGED = 0x8000;

                if ((windowPosFlags & maskToCheck) == maskToCheck || (windowPosFlags & SWP_STATECHANGED) != 0)
                    return;
            }

            // windowMayDismissModals could affect the number of active ComponentPeer instances
            for (auto i = ComponentPeer::getNumPeers(); --i >= 0;)
                if (i < ComponentPeer::getNumPeers())
                    if (auto* hwndPeer = dynamic_cast<HWNDComponentPeer*> (ComponentPeer::getPeer (i)))
                        hwndPeer->windowShouldDismissModals (info->hwnd);
        }

        static LRESULT CALLBACK callWndProc (i32 nCode, WPARAM wParam, LPARAM lParam)
        {
            processMessage (nCode, reinterpret_cast<CWPSTRUCT*> (lParam));
            return CallNextHookEx ({}, nCode, wParam, lParam);
        }

        HHOOK hook;
    };

    SharedResourcePointer<TopLevelModalDismissBroadcaster> modalDismissBroadcaster;
    IMEHandler imeHandler;
    b8 shouldIgnoreModalDismiss = false;

    ScopedSuspendResumeNotificationRegistration suspendResumeRegistration;
    std::optional<TimedCallback> monitorUpdateTimer;

    std::unique_ptr<RenderContext> renderContext;
    std::optional<LPARAM> captionMouseDown;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HWNDComponentPeer)
};

MultiTouchMapper<DWORD> HWNDComponentPeer::currentTouches;
ModifierKeys HWNDComponentPeer::modifiersAtLastCallback;

ComponentPeer* Component::createNewPeer (i32 styleFlags, uk parentHWND)
{
    return new HWNDComponentPeer { *this, styleFlags, (HWND) parentHWND, false, 1 };
}

Image createSnapshotOfNativeWindow (uk nativeWindowHandle)
{
    i32 numDesktopComponents = Desktop::getInstance().getNumComponents();

    for (i32 index = 0; index < numDesktopComponents; ++index)
    {
        auto component = Desktop::getInstance().getComponent (index);

        if (auto peer = dynamic_cast<HWNDComponentPeer*> (component->getPeer()))
            if (peer->getNativeHandle() == nativeWindowHandle)
                return peer->createSnapshot();
    }

    return {};
}

class GDIRenderContext : public RenderContext
{
public:
    static constexpr auto name = "Software Renderer";

    explicit GDIRenderContext (HWNDComponentPeer& peerIn)
        : peer (peerIn)
    {
        RedrawWindow (peer.getHWND(),
                      nullptr,
                      nullptr,
                      RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }

    tukk getName() const override { return name; }

    z0 updateConstantAlpha() override
    {
        InvalidateRect (peer.getHWND(), nullptr, false);
    }

    z0 handlePaintMessage() override
    {
        HRGN rgn = CreateRectRgn (0, 0, 0, 0);
        i32k regionType = GetUpdateRgn (peer.getHWND(), rgn, false);

        PAINTSTRUCT paintStruct;
        HDC dc = BeginPaint (peer.getHWND(), &paintStruct); // Note this can immediately generate a WM_NCPAINT
                                                            // message and become re-entrant, but that's OK

        // If something in a paint handler calls, e.g. a message box, this can become reentrant and
        // corrupt the image it's using to paint into, so do a check here.
        static b8 reentrant = false;

        if (! reentrant)
        {
            const ScopedValueSetter<b8> setter (reentrant, true, false);

            if (peer.dontRepaint)
                peer.getComponent().handleCommandMessage (0); // (this triggers a repaint in the openGL context)
            else
                performPaint (dc, rgn, regionType, paintStruct);
        }

        DeleteObject (rgn);
        EndPaint (peer.getHWND(), &paintStruct);

       #if DRX_MSVC
        _fpreset(); // because some graphics cards can unmask FP exceptions
       #endif
    }

    z0 repaint (const Rectangle<i32>& area) override
    {
        DRX_TRACE_EVENT_INT_RECT (etw::repaint, etw::paintKeyword, area);
        deferredRepaints.add (area);
    }

    z0 dispatchDeferredRepaints() override
    {
        for (auto deferredRect : deferredRepaints)
        {
            auto r = D2DUtilities::toRECT (deferredRect);
            InvalidateRect (peer.getHWND(), &r, FALSE);
        }

        deferredRepaints.clear();
    }

    z0 performAnyPendingRepaintsNow() override
    {
        if (! peer.getComponent().isVisible())
            return;

        dispatchDeferredRepaints();

        WeakReference localRef (&peer.getComponent());
        MSG m;

        if (peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::perPixel
            || PeekMessage (&m, peer.getHWND(), WM_PAINT, WM_PAINT, PM_REMOVE))
        {
            if (localRef != nullptr) // (the PeekMessage call can dispatch messages, which may delete this comp)
                handlePaintMessage();
        }
    }

    Image createSnapshot() override
    {
        return peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::perPixel
             ? createSnapshotOfLayeredWindow()
             : createSnapshotOfNormalWindow();
    }

    z0 onVBlank() override {}

    z0 handleShowWindow() override {}

private:
    // If we've called UpdateLayeredWindow to display the window contents, retrieving the
    // contents of the window DC will fail.
    // Instead, we produce a fresh render of the window into a temporary image.
    // Child windows will not be included.
    Image createSnapshotOfLayeredWindow() const
    {
        const auto rect = peer.getClientRectInScreen();
        Image result { Image::ARGB, rect.getWidth(), rect.getHeight(), true, SoftwareImageType{} };

        {
            auto context = peer.getComponent()
                               .getLookAndFeel()
                               .createGraphicsContext (result, {}, rect.withZeroOrigin());

            context->addTransform (AffineTransform::scale ((f32) peer.getPlatformScaleFactor()));
            peer.handlePaint (*context);
        }

        return result;
    }

    // If UpdateLayeredWindow hasn't been called, then we can blit the window contents directly
    // from the window's DC.
    Image createSnapshotOfNormalWindow() const
    {
        auto hwnd = peer.getHWND();

        auto r = convertPhysicalScreenRectangleToLogical (D2DUtilities::toRectangle (getWindowScreenRect (hwnd)), hwnd);
        const auto w = r.getWidth();
        const auto h = r.getHeight();

        WindowsBitmapImage::Ptr nativeBitmap = new WindowsBitmapImage (Image::RGB, w, h, true);
        Image bitmap (nativeBitmap);

        ScopedDeviceContext deviceContext { hwnd };

        const auto hdc = nativeBitmap->getHDC();

        if (isPerMonitorDPIAwareProcess())
        {
            auto scale = getScaleFactorForWindow (hwnd);
            auto prevStretchMode = SetStretchBltMode (hdc, HALFTONE);
            SetBrushOrgEx (hdc, 0, 0, nullptr);

            StretchBlt (hdc, 0, 0, w, h,
                        deviceContext.dc, 0, 0, roundToInt (w * scale), roundToInt (h * scale),
                        SRCCOPY);

            SetStretchBltMode (hdc, prevStretchMode);
        }
        else
        {
            BitBlt (hdc, 0, 0, w, h, deviceContext.dc, 0, 0, SRCCOPY);
        }

        return SoftwareImageType().convert (bitmap);
    }

    z0 performPaint (HDC dc, HRGN rgn, i32 regionType, PAINTSTRUCT& paintStruct)
    {
        i32 x = paintStruct.rcPaint.left;
        i32 y = paintStruct.rcPaint.top;
        i32 w = paintStruct.rcPaint.right - x;
        i32 h = paintStruct.rcPaint.bottom - y;

        const auto perPixelTransparent = peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::perPixel;

        if (perPixelTransparent)
        {
            // it's not possible to have a transparent window with a title bar at the moment!
            jassert (! peer.hasTitleBar());

            auto r = getWindowScreenRect (peer.getHWND());
            x = y = 0;
            w = r.right - r.left;
            h = r.bottom - r.top;
        }

        if (w > 0 && h > 0)
        {
            Image& offscreenImage = offscreenImageGenerator.getImage (perPixelTransparent, w, h);

            RectangleList<i32> contextClip;
            const Rectangle<i32> clipBounds (w, h);

            b8 needToPaintAll = true;

            if (regionType == COMPLEXREGION && ! perPixelTransparent)
            {
                HRGN clipRgn = CreateRectRgnIndirect (&paintStruct.rcPaint);
                CombineRgn (rgn, rgn, clipRgn, RGN_AND);
                DeleteObject (clipRgn);

                alignas (RGNDATA) std::byte rgnData[8192];
                const DWORD res = GetRegionData (rgn, sizeof (rgnData), (RGNDATA*) &rgnData);

                if (res > 0 && res <= sizeof (rgnData))
                {
                    const RGNDATAHEADER* const hdr = &(((const RGNDATA*) &rgnData)->rdh);

                    if (hdr->iType == RDH_RECTANGLES
                        && hdr->rcBound.right - hdr->rcBound.left >= w
                        && hdr->rcBound.bottom - hdr->rcBound.top >= h)
                    {
                        needToPaintAll = false;

                        auto rects = unalignedPointerCast<const RECT*> ((tuk) &rgnData + sizeof (RGNDATAHEADER));

                        for (i32 i = (i32) ((RGNDATA*) &rgnData)->rdh.nCount; --i >= 0;)
                        {
                            if (rects->right <= x + w && rects->bottom <= y + h)
                            {
                                i32k cx = jmax (x, (i32) rects->left);
                                contextClip.addWithoutMerging (Rectangle<i32> (cx - x, rects->top - y,
                                                                               rects->right - cx, rects->bottom - rects->top)
                                                                       .getIntersection (clipBounds));
                            }
                            else
                            {
                                needToPaintAll = true;
                                break;
                            }

                            ++rects;
                        }
                    }
                }
            }

            if (needToPaintAll)
            {
                contextClip.clear();
                contextClip.addWithoutMerging (Rectangle<i32> (w, h));
            }

            ChildWindowClippingInfo childClipInfo = { dc, &peer, &contextClip, Point<i32> (x, y), 0 };
            EnumChildWindows (peer.getHWND(), clipChildWindowCallback, (LPARAM) &childClipInfo);

            if (! contextClip.isEmpty())
            {
                if (perPixelTransparent)
                    for (auto& i : contextClip)
                        offscreenImage.clear (i);

                {
                    auto context = peer.getComponent()
                                       .getLookAndFeel()
                                       .createGraphicsContext (offscreenImage, { -x, -y }, contextClip);

                    context->addTransform (AffineTransform::scale ((f32) peer.getPlatformScaleFactor()));
                    peer.handlePaint (*context);
                }

                auto* image = static_cast<WindowsBitmapImage*> (offscreenImage.getPixelData().get());

                if (perPixelTransparent)
                {
                    image->updateLayeredWindow (peer.getHWND(), { x, y }, peer.getComponent().getAlpha());
                }
                else
                {
                    image->blitToDC (dc, x, y);

                    if (peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::constant)
                        SetLayeredWindowAttributes (peer.getHWND(), {}, (BYTE) (255.0f * peer.getComponent().getAlpha()), LWA_ALPHA);
                }
            }

            if (childClipInfo.savedDC != 0)
                RestoreDC (dc, childClipInfo.savedDC);
        }
    }

    struct ChildWindowClippingInfo
    {
        HDC dc;
        HWNDComponentPeer* peer;
        RectangleList<i32>* clip;
        Point<i32> origin;
        i32 savedDC;
    };

    static BOOL CALLBACK clipChildWindowCallback (HWND hwnd, LPARAM context)
    {
        if (IsWindowVisible (hwnd))
        {
            auto& info = *(ChildWindowClippingInfo*) context;

            if (GetParent (hwnd) == info.peer->getHWND())
            {
                auto clip = D2DUtilities::toRectangle (getWindowClientRect (hwnd));

                info.clip->subtract (clip - info.origin);

                if (info.savedDC == 0)
                    info.savedDC = SaveDC (info.dc);

                ExcludeClipRect (info.dc, clip.getX(), clip.getY(), clip.getRight(), clip.getBottom());
            }
        }

        return TRUE;
    }

    //==============================================================================
    struct TemporaryImage final : private Timer
    {
        TemporaryImage() = default;

        Image& getImage (b8 transparent, i32 w, i32 h)
        {
            auto format = transparent ? Image::ARGB : Image::RGB;

            if ((! image.isValid()) || image.getWidth() < w || image.getHeight() < h || image.getFormat() != format)
                image = Image (new WindowsBitmapImage (format, (w + 31) & ~31, (h + 31) & ~31, false));

            startTimer (3000);
            return image;
        }

        z0 timerCallback() override
        {
            stopTimer();
            image = {};
        }

    private:
        Image image;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemporaryImage)
    };

    HWNDComponentPeer& peer;
    TemporaryImage offscreenImageGenerator;
    RectangleList<i32> deferredRepaints;
};

class D2DRenderContext : public RenderContext
{
public:
    static constexpr auto name = "Direct2D";

    explicit D2DRenderContext (HWNDComponentPeer& peerIn)
        : peer (peerIn)
    {
    }

    tukk getName() const override { return name; }

    z0 updateConstantAlpha() override
    {
        const auto transparent = peer.getTransparencyKind() != HWNDComponentPeer::TransparencyKind::opaque;

        if (transparent != direct2DContext->supportsTransparency())
        {
            direct2DContext.reset();
            direct2DContext = getContextForPeer (peer);
        }

        if (direct2DContext->supportsTransparency())
            direct2DContext->updateAlpha();
    }

    z0 handlePaintMessage() override
    {
       #if DRX_DIRECT2D_METRICS
        auto paintStartTicks = Time::getHighResolutionTicks();
       #endif

        updateRegion.findRECTAndValidate (peer.getHWND());

        for (const auto& rect : updateRegion.getRects())
            repaint (D2DUtilities::toRectangle (rect));

       #if DRX_DIRECT2D_METRICS
        lastPaintStartTicks = paintStartTicks;
       #endif
    }

    z0 repaint (const Rectangle<i32>& area) override
    {
        direct2DContext->addDeferredRepaint (area);
    }

    z0 dispatchDeferredRepaints() override {}

    z0 performAnyPendingRepaintsNow() override {}

    Image createSnapshot() override
    {
        return direct2DContext->createSnapshot();
    }

    z0 onVBlank() override
    {
        handleDirect2DPaint();
    }

    z0 handleShowWindow() override
    {
        direct2DContext->handleShowWindow();
        handleDirect2DPaint();
    }

private:
    struct WrappedD2DHwndContextBase
    {
        virtual ~WrappedD2DHwndContextBase() = default;
        virtual z0 addDeferredRepaint (Rectangle<i32> area) = 0;
        virtual Image createSnapshot() const = 0;
        virtual z0 handleShowWindow() = 0;
        virtual LowLevelGraphicsContext* startFrame (f32 dpiScale) = 0;
        virtual z0 endFrame() = 0;
        virtual b8 supportsTransparency() const = 0;
        virtual z0 updateAlpha() = 0;
        virtual Direct2DMetrics::Ptr getMetrics() const = 0;
    };

    /** This is a D2D context that uses a swap chain for presentation.
        D2D contexts that use a swapchain can be made transparent using DirectComposition, but this
        ends up causing other problems in DRX, such as:
        - The window redirection bitmap also needs to be disabled, which is a permanent window
          setting, so it can't be enabled on the same window - instead a new window needs to be created.
          This means that dynamically changing a component's alpha level at runtime might force the
          window to be recreated, which is not ideal.
        - We can't just disable the redirection bitmap by default, because it's needed to display
          child windows, notably plugin editors
        - The mouse gets captured inside the entire window bounds, rather than just the non-transparent parts

        To avoid these problems, we only use the swapchain to present opaque windows.
        For transparent windows, we use a different technique - see below.
    */
    class WrappedD2DHwndContext : public WrappedD2DHwndContextBase
    {
    public:
        explicit WrappedD2DHwndContext (HWND hwnd) : ctx (hwnd) {}

        z0 addDeferredRepaint (Rectangle<i32> area) override
        {
            ctx.addDeferredRepaint (area);
        }

        Image createSnapshot() const override
        {
            return ctx.createSnapshot();
        }

        z0 handleShowWindow() override
        {
            ctx.handleShowWindow();
        }

        LowLevelGraphicsContext* startFrame (f32 scale) override
        {
            if (ctx.startFrame (scale))
                return &ctx;

            return nullptr;
        }

        z0 endFrame() override
        {
            ctx.endFrame();
        }

        b8 supportsTransparency() const override
        {
            return false;
        }

        z0 updateAlpha() override
        {
            // This doesn't support transparency, so updating the alpha won't do anything
            jassertfalse;
        }

        Direct2DMetrics::Ptr getMetrics() const override
        {
            return ctx.metrics;
        }

    private:
        Direct2DHwndContext ctx;
    };

    class DxgiBitmapRenderer
    {
    public:
        LowLevelGraphicsContext* startFrame (HWND hwnd, f32 scale, const RectangleList<i32>& dirty)
        {
            RECT r{};
            GetClientRect (hwnd, &r);

            const auto w = r.right - r.left;
            const auto h = r.bottom - r.top;
            const auto size = D2D1::SizeU ((UINT32) w, (UINT32) h);

            const auto lastAdapter = std::exchange (adapter, directX->adapters.getAdapterForHwnd (hwnd));

            const auto needsNewDC = lastAdapter != adapter || deviceContext == nullptr;

            if (needsNewDC)
            {
                deviceContext = Direct2DDeviceContext::create (adapter);
                bitmap = nullptr;
                context = nullptr;
            }

            if (deviceContext == nullptr)
                return nullptr;

            const auto needsNewBitmap = bitmap == nullptr || ! equal (bitmap->GetPixelSize(), size);

            if (needsNewBitmap)
            {
                bitmap = Direct2DBitmap::createBitmap (deviceContext,
                                                       Image::ARGB,
                                                       size,
                                                       D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE);
                context = nullptr;
            }

            if (bitmap == nullptr)
                return nullptr;

            const auto paintAreas = needsNewBitmap ? Rectangle { (i32) w, (i32) h } : dirty;

            if (paintAreas.isEmpty())
                return nullptr;

            if (context == nullptr)
                context = std::make_unique<Direct2DImageContext> (deviceContext, bitmap, paintAreas);

            if (! context->startFrame (scale))
                context = nullptr;

            if (context == nullptr)
                return nullptr;

            context->setFill (Colors::transparentBlack);
            context->fillRect ({ (i32) size.width, (i32) size.height }, true);

            return context.get();
        }

        z0 endFrame()
        {
            if (context != nullptr)
                context->endFrame();
        }

        Image getImage() const
        {
            return Image { new Direct2DPixelData { adapter->direct2DDevice, bitmap } };
        }

        ComSmartPtr<ID2D1Bitmap1> getBitmap() const
        {
            return bitmap;
        }

        Direct2DMetrics::Ptr getMetrics() const
        {
            if (context != nullptr)
                return context->metrics;

            return {};
        }

    private:
        static constexpr b8 equal (D2D1_SIZE_U a, D2D1_SIZE_U b)
        {
            const auto tie = [] (auto& x) { return std::tie (x.width, x.height); };
            return tie (a) == tie (b);
        }

        SharedResourcePointer<DirectX> directX;
        DxgiAdapter::Ptr adapter;
        ComSmartPtr<ID2D1DeviceContext1> deviceContext;
        ComSmartPtr<ID2D1Bitmap1> bitmap;
        std::unique_ptr<Direct2DImageContext> context;
    };

    /*  This wrapper facilitates drawing Direct2D content into a transparent/layered window.

        As an alternative to using DirectComposition, we instead use the older technique of using
        a layered window, and calling UpdateLayeredWindow to set per-pixel alpha on the window.
        This will be slower than going through the swap chain, but means that we can still set
        the alpha level dynamically at runtime, support child windows as before, and support
        per-pixel mouse hit-testing.

        UpdateLayeredWindow is an older API that expects a HDC input containing the image that is
        blitted to the screen. To get an HDC out of Direct2D, we cast a D2D bitmap to IDXGISurface1,
        which exposes a suitable DC. This only works if the target bitmap is constructed with the
        D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE option.
    */
    class WrappedD2DHwndContextTransparent : public WrappedD2DHwndContextBase
    {
    public:
        explicit WrappedD2DHwndContextTransparent (HWNDComponentPeer& p) : peer (p) {}

        z0 addDeferredRepaint (Rectangle<i32> area) override
        {
            deferredRepaints.add (area);
        }

        Image createSnapshot() const override
        {
            DxgiBitmapRenderer renderer;

            if (auto* ctx = renderer.startFrame (peer.getHWND(), (f32) peer.getPlatformScaleFactor(), {}))
            {
                peer.handlePaint (*ctx);
                renderer.endFrame();
            }

            return renderer.getImage();
        }

        z0 handleShowWindow() override {}

        LowLevelGraphicsContext* startFrame (f32 scale) override
        {
            auto* result = bitmapRenderer.startFrame (peer.getHWND(), scale, deferredRepaints);

            if (result != nullptr)
                deferredRepaints.clear();

            return result;
        }

        z0 endFrame() override
        {
            bitmapRenderer.endFrame();
            updateLayeredWindow();
        }

        b8 supportsTransparency() const override
        {
            return true;
        }

        z0 updateAlpha() override
        {
            updateLayeredWindow();
        }

        Direct2DMetrics::Ptr getMetrics() const override
        {
            return bitmapRenderer.getMetrics();
        }

    private:
        z0 updateLayeredWindow()
        {
            const auto bitmap = bitmapRenderer.getBitmap();

            if (bitmap == nullptr)
                return;

            ComSmartPtr<IDXGISurface> surface;
            if (const auto hr = bitmap->GetSurface (surface.resetAndGetPointerAddress());
                FAILED (hr) || surface == nullptr)
            {
                jassertfalse;
                return;
            }

            ComSmartPtr<IDXGISurface1> surface1;
            surface.QueryInterface (surface1);

            if (surface1 == nullptr)
            {
                jassertfalse;
                return;
            }

            HDC hdc{};
            if (const auto hr = surface1->GetDC (false, &hdc); FAILED (hr))
            {
                jassertfalse;
                return;
            }

            const ScopeGuard releaseDC { [&]
                                         {
                                             RECT emptyRect { 0, 0, 0, 0 };
                                             const auto hr = surface1->ReleaseDC (&emptyRect);
                                             jassertquiet (SUCCEEDED (hr));
                                         } };

            if (peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::perPixel)
            {
                WindowsBitmapImage::updateLayeredWindow (hdc, peer.getHWND(), {}, peer.getComponent().getAlpha());
            }
            else
            {
                const ScopedDeviceContext scope { peer.getHWND() };
                const auto size = bitmap->GetPixelSize();
                BitBlt (scope.dc, 0, 0, (i32) size.width, (i32) size.height, hdc, 0, 0, SRCCOPY);

                if (peer.getTransparencyKind() == HWNDComponentPeer::TransparencyKind::constant)
                    SetLayeredWindowAttributes (peer.getHWND(), {}, (BYTE) (255.0f * peer.getComponent().getAlpha()), LWA_ALPHA);
            }
        }

        HWNDComponentPeer& peer;

        DxgiBitmapRenderer bitmapRenderer;
        RectangleList<i32> deferredRepaints;
    };

    z0 handleDirect2DPaint()
    {
       #if DRX_DIRECT2D_METRICS
        auto paintStartTicks = Time::getHighResolutionTicks();
       #endif

        // Use the ID2D1DeviceContext to paint a swap chain buffer, then tell the swap chain to present
        // the next buffer.
        //
        // Direct2DLowLevelGraphicsContext::startFrame checks if there are any areas to be painted and if the
        // renderer is ready to go; if so, startFrame allocates any needed Direct2D resources,
        // and calls ID2D1DeviceContext::BeginDraw
        //
        // handlePaint() makes various calls into the Direct2DLowLevelGraphicsContext which in turn calls
        // the appropriate ID2D1DeviceContext functions to draw rectangles, clip, set the fill color, etc.
        //
        // Direct2DLowLevelGraphicsContext::endFrame calls ID2D1DeviceContext::EndDraw to finish painting
        // and then tells the swap chain to present the next swap chain back buffer.
        if (auto* ctx = direct2DContext->startFrame ((f32) peer.getPlatformScaleFactor()))
        {
            peer.handlePaint (*ctx);
            direct2DContext->endFrame();
        }

       #if DRX_DIRECT2D_METRICS
        if (lastPaintStartTicks > 0)
        {
            if (auto metrics = direct2DContext->getMetrics())
            {
                metrics->addValueTicks (Direct2DMetrics::messageThreadPaintDuration,
                                        Time::getHighResolutionTicks() - paintStartTicks);
                metrics->addValueTicks (Direct2DMetrics::frameInterval,
                                        paintStartTicks - lastPaintStartTicks);
            }
        }
        lastPaintStartTicks = paintStartTicks;
       #endif
    }

    static std::unique_ptr<WrappedD2DHwndContextBase> getContextForPeer (HWNDComponentPeer& peer)
    {
        if (peer.getTransparencyKind() != HWNDComponentPeer::TransparencyKind::opaque)
            return std::make_unique<WrappedD2DHwndContextTransparent> (peer);

        return std::make_unique<WrappedD2DHwndContext> (peer.getHWND());
    }

    HWNDComponentPeer& peer;

    std::unique_ptr<WrappedD2DHwndContextBase> direct2DContext = getContextForPeer (peer);
    UpdateRegion updateRegion;

   #if DRX_ETW_TRACELOGGING
    struct ETWEventProvider
    {
        ETWEventProvider()
        {
            const auto hr = TraceLoggingRegister (::drx::etw::DRXTraceLogProvider);
            jassertquiet (SUCCEEDED (hr));
        }

        ~ETWEventProvider()
        {
            TraceLoggingUnregister (::drx::etw::DRXTraceLogProvider);
        }
    };

    SharedResourcePointer<ETWEventProvider> etwEventProvider;
   #endif

   #if DRX_DIRECT2D_METRICS
    z64 lastPaintStartTicks = 0;
   #endif
};

using Constructor = std::unique_ptr<RenderContext> (*) (HWNDComponentPeer&);
struct ContextDescriptor
{
    tukk name = nullptr;
    Constructor construct = nullptr;
};

template <typename... T>
inline constexpr ContextDescriptor contextDescriptorList[]
{
    {
        T::name,
        [] (HWNDComponentPeer& p) -> std::unique_ptr<RenderContext> { return std::make_unique<T> (p); }
    }...
};

// To add a new rendering backend, implement RenderContext for that backend, and then append the backend to this typelist
inline constexpr auto& contextDescriptors = contextDescriptorList<GDIRenderContext, D2DRenderContext>;

z0 HWNDComponentPeer::setCurrentRenderingEngine (i32 e)
{
    if (isPositiveAndBelow (e, std::size (contextDescriptors)) && (renderContext == nullptr || getCurrentRenderingEngine() != e))
    {
        // Reset the old context before creating the new context, because some context resources
        // can only be created once per window.
        renderContext.reset();
        renderContext = contextDescriptors[e].construct (*this);
    }
}

StringArray HWNDComponentPeer::getAvailableRenderingEngines()
{
    StringArray results;

    for (const auto& d : contextDescriptors)
        results.add (d.name);

    return results;
}

i32 HWNDComponentPeer::getCurrentRenderingEngine() const
{
    jassert (renderContext != nullptr);

    for (const auto [index, d] : enumerate (contextDescriptors, i32{}))
        if (d.name == renderContext->getName())
            return index;

    return -1;
}

DRX_API ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component& component, Component* parentComponent);
DRX_API ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component& component, Component* parentComponent)
{
    if (auto parentPeer = parentComponent->getPeer())
    {
        // Explicitly set the top-level window to software renderer mode in case
        // this is switching from Direct2D to OpenGL
        //
        // HWNDComponentPeer and Direct2DComponentPeer rely on virtual methods for initialization; hence the call to
        // embeddedWindowPeer->initialise() after creating the peer
        i32 styleFlags = ComponentPeer::windowIgnoresMouseClicks;
        return new HWNDComponentPeer (component,
                                      styleFlags,
                                      (HWND) parentPeer->getNativeHandle(),
                                      true, /* nonRepainting*/
                                      0);
    }

    return nullptr;
}

//==============================================================================
b8 KeyPress::isKeyCurrentlyDown (i32k keyCode)
{
    const auto k = [&]
    {
        if ((keyCode & extendedKeyModifier) != 0)
            return keyCode & (extendedKeyModifier - 1);

        const auto vk = BYTE (VkKeyScan ((WCHAR) keyCode) & 0xff);
        return vk != (BYTE) -1 ? vk : keyCode;
    }();

    return HWNDComponentPeer::isKeyDown (k);
}

//==============================================================================
static DWORD getProcess (HWND hwnd)
{
    DWORD result = 0;
    GetWindowThreadProcessId (hwnd, &result);
    return result;
}

/*  Возвращает true, если the viewComponent is embedded into a window
    owned by the foreground process.
*/
b8 detail::WindowingHelpers::isEmbeddedInForegroundProcess (Component* c)
{
    if (c == nullptr)
        return false;

    auto* peer = c->getPeer();
    auto* hwnd = peer != nullptr ? static_cast<HWND> (peer->getNativeHandle()) : nullptr;

    if (hwnd == nullptr)
        return true;

    const auto fgProcess    = getProcess (GetForegroundWindow());
    const auto ownerProcess = getProcess (GetAncestor (hwnd, GA_ROOTOWNER));
    return fgProcess == ownerProcess;
}

b8 DRX_CALLTYPE Process::isForegroundProcess()
{
    if (auto fg = GetForegroundWindow())
        return getProcess (fg) == GetCurrentProcessId();

    return true;
}

// N/A on Windows as far as I know.
z0 DRX_CALLTYPE Process::makeForegroundProcess() {}
z0 DRX_CALLTYPE Process::hide() {}

//==============================================================================
b8 detail::MouseInputSourceList::addSource()
{
    auto numSources = sources.size();

    if (numSources == 0 || canUseMultiTouch())
    {
        addSource (numSources, numSources == 0 ? MouseInputSource::InputSourceType::mouse
                                               : MouseInputSource::InputSourceType::touch);
        return true;
    }

    return false;
}

b8 detail::MouseInputSourceList::canUseTouch() const
{
    return canUseMultiTouch();
}

Point<f32> MouseInputSource::getCurrentRawMousePosition()
{
    POINT mousePos;
    GetCursorPos (&mousePos);

    auto p = D2DUtilities::toPoint (mousePos);

    if (isPerMonitorDPIAwareThread())
        p = Desktop::getInstance().getDisplays().physicalToLogical (p);

    return p.toFloat();
}

z0 MouseInputSource::setRawMousePosition (Point<f32> newPosition)
{
    auto newPositionInt = newPosition.roundToInt();

   #if DRX_WIN_PER_MONITOR_DPI_AWARE
    if (isPerMonitorDPIAwareThread())
        newPositionInt = Desktop::getInstance().getDisplays().logicalToPhysical (newPositionInt);
   #endif

    auto point = D2DUtilities::toPOINT (newPositionInt);
    SetCursorPos (point.x, point.y);
}

//==============================================================================
class ScreenSaverDefeater final : public Timer
{
public:
    ScreenSaverDefeater()
    {
        startTimer (10000);
        timerCallback();
    }

    z0 timerCallback() override
    {
        if (Process::isForegroundProcess())
        {
            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.mouseData = MOUSEEVENTF_MOVE;

            SendInput (1, &input, sizeof (INPUT));
        }
    }
};

static std::unique_ptr<ScreenSaverDefeater> screenSaverDefeater;

z0 Desktop::setScreenSaverEnabled (const b8 isEnabled)
{
    if (isEnabled)
        screenSaverDefeater = nullptr;
    else if (screenSaverDefeater == nullptr)
        screenSaverDefeater.reset (new ScreenSaverDefeater());
}

b8 Desktop::isScreenSaverEnabled()
{
    return screenSaverDefeater == nullptr;
}

//==============================================================================
z0 LookAndFeel::playAlertSound()
{
    MessageBeep (MB_OK);
}

//==============================================================================
z0 SystemClipboard::copyTextToClipboard (const Txt& text)
{
    if (OpenClipboard (nullptr) != 0)
    {
        if (EmptyClipboard() != 0)
        {
            auto bytesNeeded = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer()) + 4;

            if (bytesNeeded > 0)
            {
                if (auto bufH = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, bytesNeeded + sizeof (WCHAR)))
                {
                    if (auto* data = static_cast<WCHAR*> (GlobalLock (bufH)))
                    {
                        text.copyToUTF16 (data, bytesNeeded);
                        GlobalUnlock (bufH);

                        SetClipboardData (CF_UNICODETEXT, bufH);
                    }
                }
            }
        }

        CloseClipboard();
    }
}

Txt SystemClipboard::getTextFromClipboard()
{
    Txt result;

    if (OpenClipboard (nullptr) != 0)
    {
        if (auto bufH = GetClipboardData (CF_UNICODETEXT))
        {
            if (auto* data = (const WCHAR*) GlobalLock (bufH))
            {
                result = Txt (data, (size_t) (GlobalSize (bufH) / sizeof (WCHAR)));
                GlobalUnlock (bufH);
            }
        }

        CloseClipboard();
    }

    return result;
}

//==============================================================================
z0 Desktop::setKioskComponent (Component* kioskModeComp, b8 enableOrDisable, b8 /*allowMenusAndBars*/)
{
    if (auto* tlw = dynamic_cast<TopLevelWindow*> (kioskModeComp))
        tlw->setUsingNativeTitleBar (! enableOrDisable);

    if (kioskModeComp != nullptr && enableOrDisable)
        kioskModeComp->setBounds (getDisplays().getDisplayForRect (kioskModeComp->getScreenBounds())->totalArea);
}

z0 Desktop::allowedOrientationsChanged() {}

//==============================================================================
static const Displays::Display* getCurrentDisplayFromScaleFactor (HWND hwnd)
{
    Array<const Displays::Display*> candidateDisplays;

    const auto scaleToLookFor = [&]
    {
        if (auto* peer = HWNDComponentPeer::getOwnerOfWindow (hwnd))
            return peer->getPlatformScaleFactor();

        return getScaleFactorForWindow (hwnd);
    }();

    auto globalScale = Desktop::getInstance().getGlobalScaleFactor();

    for (auto& d : Desktop::getInstance().getDisplays().displays)
        if (approximatelyEqual (d.scale / globalScale, scaleToLookFor))
            candidateDisplays.add (&d);

    if (candidateDisplays.size() > 0)
    {
        if (candidateDisplays.size() == 1)
            return candidateDisplays[0];

        const auto bounds = [&]
        {
            if (auto* peer = HWNDComponentPeer::getOwnerOfWindow (hwnd))
                return peer->getComponent().getTopLevelComponent()->getBounds();

            return Desktop::getInstance().getDisplays().physicalToLogical (D2DUtilities::toRectangle (getWindowScreenRect (hwnd)));
        }();

        const Displays::Display* retVal = nullptr;
        i32 maxArea = -1;

        for (auto* d : candidateDisplays)
        {
            auto intersection = d->totalArea.getIntersection (bounds);
            auto area = intersection.getWidth() * intersection.getHeight();

            if (area > maxArea)
            {
                maxArea = area;
                retVal = d;
            }
        }

        if (retVal != nullptr)
            return retVal;
    }

    return Desktop::getInstance().getDisplays().getPrimaryDisplay();
}

//==============================================================================
struct MonitorInfo
{
    MonitorInfo (b8 main, RECT totalArea, RECT workArea, f64 d, std::optional<f64> frequency) noexcept
        : isMain (main),
          totalAreaRect (totalArea),
          workAreaRect (workArea),
          dpi (d),
          verticalFrequencyHz (frequency)
    {
    }

    b8 isMain;
    RECT totalAreaRect, workAreaRect;
    f64 dpi;
    std::optional<f64> verticalFrequencyHz;
};

static BOOL CALLBACK enumMonitorsProc (HMONITOR hm, HDC, LPRECT, LPARAM userInfo)
{
    MONITORINFOEX info = {};
    info.cbSize = sizeof (info);
    GetMonitorInfo (hm, &info);

    auto isMain = (info.dwFlags & 1 /* MONITORINFOF_PRIMARY */) != 0;
    auto dpi = 0.0;

    if (getDPIForMonitor != nullptr)
    {
        UINT dpiX = 0, dpiY = 0;

        if (SUCCEEDED (getDPIForMonitor (hm, MDT_Default, &dpiX, &dpiY)))
            dpi = (dpiX + dpiY) / 2.0;
    }

    // Call EnumDisplayDevices and EnumDisplaySettings to get the refresh rate of the monitor
    BOOL ok = TRUE;
    std::optional<f64> frequency;
    for (u32 deviceNumber = 0; ok; ++deviceNumber)
    {
        DISPLAY_DEVICEW displayDevice{};
        displayDevice.cb = sizeof (displayDevice);
        ok = EnumDisplayDevicesW (nullptr, deviceNumber, &displayDevice, 0);
        if (ok && (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
        {
            DEVMODE displaySettings{};
            ok = EnumDisplaySettingsW (displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &displaySettings);
            if (ok)
            {
                if (Txt { displayDevice.DeviceName } == Txt { info.szDevice })
                {
                    frequency = (f64) displaySettings.dmDisplayFrequency;
                    break;
                }
            }
        }
    }

    ((Array<MonitorInfo>*) userInfo)->add ({ isMain, info.rcMonitor, info.rcWork, dpi, frequency });
    return TRUE;
}

z0 Displays::findDisplays (f32 masterScale)
{
    setDPIAwareness();

    Array<MonitorInfo> monitors;
    EnumDisplayMonitors (nullptr, nullptr, &enumMonitorsProc, (LPARAM) &monitors);

    auto globalDPI = getGlobalDPI();

    if (monitors.size() == 0)
    {
        auto windowRect = getWindowScreenRect (GetDesktopWindow());
        monitors.add ({ true, windowRect, windowRect, globalDPI, std::optional<f64>{} });
    }

    // make sure the first in the list is the main monitor
    for (i32 i = 1; i < monitors.size(); ++i)
        if (monitors.getReference (i).isMain)
            monitors.swap (i, 0);

    for (auto& monitor : monitors)
    {
        Display d;

        d.isMain = monitor.isMain;
        d.dpi = monitor.dpi;

        if (approximatelyEqual (d.dpi, 0.0))
        {
            d.dpi = globalDPI;
            d.scale = masterScale;
        }
        else
        {
            d.scale = (d.dpi / USER_DEFAULT_SCREEN_DPI) * (masterScale / Desktop::getDefaultMasterScale());
        }

        d.totalArea = D2DUtilities::toRectangle (monitor.totalAreaRect);
        d.userArea  = D2DUtilities::toRectangle (monitor.workAreaRect);

        displays.add (d);
    }

   #if DRX_WIN_PER_MONITOR_DPI_AWARE
    if (isPerMonitorDPIAwareThread())
        updateToLogical();
    else
   #endif
    {
        for (auto& d : displays)
        {
            d.totalArea /= masterScale;
            d.userArea  /= masterScale;
        }
    }
}

//==============================================================================
static auto extractFileHICON (const File& file)
{
    WORD iconNum = 0;
    WCHAR name[MAX_PATH * 2];
    file.getFullPathName().copyToUTF16 (name, sizeof (name));

    return IconConverters::IconPtr { ExtractAssociatedIcon ((HINSTANCE) Process::getCurrentModuleInstanceHandle(),
                                                            name,
                                                            &iconNum) };
}

Image detail::WindowingHelpers::createIconForFile (const File& file)
{
    if (const auto icon = extractFileHICON (file))
        return IconConverters::createImageFromHICON (icon.get());

    return {};
}

//==============================================================================
class MouseCursor::PlatformSpecificHandle
{
public:
    explicit PlatformSpecificHandle (const MouseCursor::StandardCursorType type)
        : impl (makeHandle (type)) {}

    explicit PlatformSpecificHandle (const detail::CustomMouseCursorInfo& info)
        : impl (makeHandle (info)) {}

    static z0 showInWindow (PlatformSpecificHandle* handle, ComponentPeer* peer)
    {
        SetCursor ([&]
        {
            if (handle != nullptr && handle->impl != nullptr && peer != nullptr)
                return handle->impl->getCursor (*peer);

            return LoadCursor (nullptr, IDC_ARROW);
        }());
    }

private:
    struct Impl
    {
        virtual ~Impl() = default;
        virtual HCURSOR getCursor (ComponentPeer&) = 0;
    };

    class BuiltinImpl : public Impl
    {
    public:
        explicit BuiltinImpl (HCURSOR cursorIn)
            : cursor (cursorIn) {}

        HCURSOR getCursor (ComponentPeer&) override { return cursor; }

    private:
        HCURSOR cursor;
    };

    class ImageImpl : public Impl
    {
    public:
        explicit ImageImpl (const detail::CustomMouseCursorInfo& infoIn) : info (infoIn) {}

        HCURSOR getCursor (ComponentPeer& peer) override
        {
            DRX_ASSERT_MESSAGE_THREAD;

            static auto getCursorSize = getCursorSizeForPeerFunction();

            const auto size = getCursorSize (peer);
            const auto iter = cursorsBySize.find (size);

            if (iter != cursorsBySize.end())
                return iter->second.get();

            const auto logicalSize = info.image.getScaledBounds();
            const auto scale = (f32) size / (f32) unityCursorSize;
            const auto physicalSize = logicalSize * scale;

            const auto& image = info.image.getImage();
            const auto rescaled = image.rescaled (roundToInt ((f32) physicalSize.getWidth()),
                                                  roundToInt ((f32) physicalSize.getHeight()));

            const auto effectiveScale = rescaled.getWidth() / logicalSize.getWidth();

            const auto hx = jlimit (0, rescaled.getWidth(),  roundToInt ((f32) info.hotspot.x * effectiveScale));
            const auto hy = jlimit (0, rescaled.getHeight(), roundToInt ((f32) info.hotspot.y * effectiveScale));

            return cursorsBySize.emplace (size, CursorPtr { IconConverters::createHICONFromImage (rescaled, false, hx, hy) }).first->second.get();
        }

    private:
        struct CursorDestructor
        {
            z0 operator() (HCURSOR ptr) const { if (ptr != nullptr) DestroyCursor (ptr); }
        };

        using CursorPtr = std::unique_ptr<std::remove_pointer_t<HCURSOR>, CursorDestructor>;

        const detail::CustomMouseCursorInfo info;
        std::map<i32, CursorPtr> cursorsBySize;
    };

    static auto getCursorSizeForPeerFunction() -> i32 (*) (ComponentPeer&)
    {
        static const auto getDpiForMonitor = []() -> GetDPIForMonitorFunc
        {
            constexpr auto library = "SHCore.dll";
            LoadLibraryA (library);

            if (auto* handle = GetModuleHandleA (library))
                return (GetDPIForMonitorFunc) GetProcAddress (handle, "GetDpiForMonitor");

            return nullptr;
        }();

        static const auto getSystemMetricsForDpi = []() -> GetSystemMetricsForDpiFunc
        {
            constexpr auto library = "User32.dll";
            LoadLibraryA (library);

            if (auto* handle = GetModuleHandleA (library))
                return (GetSystemMetricsForDpiFunc) GetProcAddress (handle, "GetSystemMetricsForDpi");

            return nullptr;
        }();

        if (getDpiForMonitor == nullptr || getSystemMetricsForDpi == nullptr)
            return [] (ComponentPeer&) { return unityCursorSize; };

        return [] (ComponentPeer& p)
        {
            const ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { p.getNativeHandle() };

            UINT dpiX = 0, dpiY = 0;

            if (auto* monitor = MonitorFromWindow ((HWND) p.getNativeHandle(), MONITOR_DEFAULTTONULL))
                if (SUCCEEDED (getDpiForMonitor (monitor, MDT_Default, &dpiX, &dpiY)))
                    return getSystemMetricsForDpi (SM_CXCURSOR, dpiX);

            return unityCursorSize;
        };
    }

    static constexpr auto unityCursorSize = 32;

    static std::unique_ptr<Impl> makeHandle (const detail::CustomMouseCursorInfo& info)
    {
        return std::make_unique<ImageImpl> (info);
    }

    static std::unique_ptr<Impl> makeHandle (const MouseCursor::StandardCursorType type)
    {
        LPCTSTR cursorName = IDC_ARROW;

        switch (type)
        {
            case NormalCursor:
            case ParentCursor:                  break;
            case NoCursor:                      return std::make_unique<BuiltinImpl> (nullptr);
            case WaitCursor:                    cursorName = IDC_WAIT; break;
            case IBeamCursor:                   cursorName = IDC_IBEAM; break;
            case PointingHandCursor:            cursorName = MAKEINTRESOURCE (32649); break;
            case CrosshairCursor:               cursorName = IDC_CROSS; break;

            case LeftRightResizeCursor:
            case LeftEdgeResizeCursor:
            case RightEdgeResizeCursor:         cursorName = IDC_SIZEWE; break;

            case UpDownResizeCursor:
            case TopEdgeResizeCursor:
            case BottomEdgeResizeCursor:        cursorName = IDC_SIZENS; break;

            case TopLeftCornerResizeCursor:
            case BottomRightCornerResizeCursor: cursorName = IDC_SIZENWSE; break;

            case TopRightCornerResizeCursor:
            case BottomLeftCornerResizeCursor:  cursorName = IDC_SIZENESW; break;

            case UpDownLeftRightResizeCursor:   cursorName = IDC_SIZEALL; break;

            case DraggingHandCursor:
            {
                static u8k dragHandData[]
                    { 71,73,70,56,57,97,16,0,16,0,145,2,0,0,0,0,255,255,255,0,0,0,0,0,0,33,249,4,1,0,0,2,0,44,0,0,0,0,16,0,
                      16,0,0,2,52,148,47,0,200,185,16,130,90,12,74,139,107,84,123,39,132,117,151,116,132,146,248,60,209,138,
                      98,22,203,114,34,236,37,52,77,217,247,154,191,119,110,240,193,128,193,95,163,56,60,234,98,135,2,0,59 };

                return makeHandle ({ ScaledImage (ImageFileFormat::loadFrom (dragHandData, sizeof (dragHandData))), { 8, 7 } });
            }

            case CopyingCursor:
            {
                static u8k copyCursorData[]
                    { 71,73,70,56,57,97,21,0,21,0,145,0,0,0,0,0,255,255,255,0,128,128,255,255,255,33,249,4,1,0,0,3,0,44,0,0,0,0,21,0,
                      21,0,0,2,72,4,134,169,171,16,199,98,11,79,90,71,161,93,56,111,78,133,218,215,137,31,82,154,100,200,86,91,202,142,
                      12,108,212,87,235,174, 15,54,214,126,237,226,37,96,59,141,16,37,18,201,142,157,230,204,51,112,252,114,147,74,83,
                      5,50,68,147,208,217,16,71,149,252,124,5,0,59,0,0 };

                return makeHandle ({ ScaledImage (ImageFileFormat::loadFrom (copyCursorData, sizeof (copyCursorData))), { 1, 3 } });
            }

            case NumStandardCursorTypes: DRX_FALLTHROUGH
            default:
                jassertfalse; break;
        }

        return std::make_unique<BuiltinImpl> ([&]
        {
            if (auto* c = LoadCursor (nullptr, cursorName))
                return c;

            return LoadCursor (nullptr, IDC_ARROW);
        }());
    }

    std::unique_ptr<Impl> impl;
};

//==============================================================================
DRX_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
DRX_COMCLASS (DrxIVirtualDesktopManager, "a5cd92ff-29be-454c-8d04-d82879fb3f1b") : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
         __RPC__in HWND topLevelWindow,
         __RPC__out BOOL * onCurrentDesktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
         __RPC__in HWND topLevelWindow,
         __RPC__out GUID * desktopId) = 0;

    virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
         __RPC__in HWND topLevelWindow,
         __RPC__in REFGUID desktopId) = 0;
};

DRX_COMCLASS (DrxVirtualDesktopManager, "aa509086-5ca9-4c25-8f95-589d3c07b48a");

} // namespace drx

#ifdef __CRT_UUID_DECL
__CRT_UUID_DECL (drx::DrxIVirtualDesktopManager, 0xa5cd92ff, 0x29be, 0x454c, 0x8d, 0x04, 0xd8, 0x28, 0x79, 0xfb, 0x3f, 0x1b)
__CRT_UUID_DECL (drx::DrxVirtualDesktopManager,  0xaa509086, 0x5ca9, 0x4c25, 0x8f, 0x95, 0x58, 0x9d, 0x3c, 0x07, 0xb4, 0x8a)
#endif

b8 drx::detail::WindowingHelpers::isWindowOnCurrentVirtualDesktop (uk x)
{
    if (x == nullptr)
        return false;

    ComSmartPtr<DrxIVirtualDesktopManager> manager;

    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
    manager.CoCreateInstance (__uuidof (DrxVirtualDesktopManager), CLSCTX_ALL);
    DRX_END_IGNORE_WARNINGS_GCC_LIKE

    if (manager == nullptr)
        return true;

    BOOL current = false;

    if (FAILED (manager->IsWindowOnCurrentVirtualDesktop (static_cast<HWND> (x), &current)))
        return true;

    return current != false;
}
