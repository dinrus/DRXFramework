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
namespace ReturnHelpers
{
    template <typename Type>
    Type returnDefaultConstructedAnyType()               { return {}; }

    template <>
    inline z0 returnDefaultConstructedAnyType<z0>()  {}
}

#define DRX_GENERATE_FUNCTION_WITH_DEFAULT(functionName, objectName, args, returnType) \
    using functionName      = returnType (*) args; \
    functionName objectName = [] args -> returnType  { return ReturnHelpers::returnDefaultConstructedAnyType<returnType>(); };


//==============================================================================
class DRX_API  X11Symbols
{
public:
    //==============================================================================
    b8 loadAllSymbols();

    //==============================================================================
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XAllocClassHint, xAllocClassHint,
                                         (),
                                         XClassHint*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XAllocSizeHints, xAllocSizeHints,
                                         (),
                                         XSizeHints*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XAllocWMHints, xAllocWMHints,
                                         (),
                                         XWMHints*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XBitmapBitOrder, xBitmapBitOrder,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XBitmapUnit, xBitmapUnit,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XChangeActivePointerGrab, xChangeActivePointerGrab,
                                         (::Display*, u32, Cursor, ::Time),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XChangeProperty, xChangeProperty,
                                         (::Display*, ::Window, Atom, Atom, i32, i32, u8k*, i32),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCheckTypedWindowEvent, xCheckTypedWindowEvent,
                                         (::Display*, ::Window, i32, XEvent*),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCheckWindowEvent, xCheckWindowEvent,
                                         (::Display*, ::Window, i64, XEvent*),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XClearArea, xClearArea,
                                         (::Display*, ::Window, i32, i32, u32, u32, Bool),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCloseDisplay, xCloseDisplay,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XConnectionNumber, xConnectionNumber,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XConvertSelection, xConvertSelection,
                                         (::Display*, Atom, Atom, Atom, ::Window, ::Time),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCreateColormap, xCreateColormap,
                                         (::Display*, ::Window, Visual*, i32),
                                         Colormap)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCreateFontCursor, xCreateFontCursor,
                                         (::Display*, u32),
                                         Cursor)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCreateGC, xCreateGC,
                                         (::Display*, ::Drawable, u64, XGCValues*),
                                         GC)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCreateImage, xCreateImage,
                                         (::Display*, Visual*, u32, i32, i32, tukk, u32, u32, i32, i32),
                                         XImage*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCreatePixmap, xCreatePixmap,
                                         (::Display*, ::Drawable, u32, u32, u32),
                                         Pixmap)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCreatePixmapCursor, xCreatePixmapCursor,
                                         (::Display*, Pixmap, Pixmap, XColor*, XColor*, u32, u32),
                                         Cursor)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCreatePixmapFromBitmapData, xCreatePixmapFromBitmapData,
                                         (::Display*, ::Drawable, tukk, u32, u32, u64, u64, u32),
                                         Pixmap)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XCreateWindow, xCreateWindow,
                                         (::Display*, ::Window, i32, i32, u32, u32, u32, i32, u32, Visual*, u64, XSetWindowAttributes*),
                                         ::Window)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDefaultRootWindow, xDefaultRootWindow,
                                         (::Display*),
                                         ::Window)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDefaultScreen, xDefaultScreen,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDefaultScreenOfDisplay, xDefaultScreenOfDisplay,
                                         (::Display*),
                                         Screen*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDefaultVisual, xDefaultVisual,
                                         (::Display*, i32),
                                         Visual*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDefineCursor, xDefineCursor,
                                         (::Display*, ::Window, Cursor),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDeleteContext, xDeleteContext,
                                         (::Display*, XID, XContext),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDeleteProperty, xDeleteProperty,
                                         (::Display*, Window, Atom),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDestroyImage, xDestroyImage,
                                         (XImage*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDestroyWindow, xDestroyWindow,
                                         (::Display*, ::Window),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDisplayHeight, xDisplayHeight,
                                         (::Display*, i32),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDisplayHeightMM, xDisplayHeightMM,
                                         (::Display*, i32),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDisplayWidth, xDisplayWidth,
                                         (::Display*, i32),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XDisplayWidthMM, xDisplayWidthMM,
                                         (::Display*, i32),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XEventsQueued, xEventsQueued,
                                         (::Display*, i32),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XFindContext, xFindContext,
                                         (::Display*, XID, XContext, XPointer*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XFlush, xFlush,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XFree, xFree,
                                         (uk),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XFreeCursor, xFreeCursor,
                                         (::Display*, Cursor),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XFreeColormap ,xFreeColormap,
                                         (::Display*, Colormap),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XFreeGC, xFreeGC,
                                         (::Display*, GC),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XFreeModifiermap, xFreeModifiermap,
                                         (XModifierKeymap*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XFreePixmap, xFreePixmap,
                                         (::Display*, Pixmap),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetAtomName, xGetAtomName,
                                         (::Display*, Atom),
                                         tuk)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetErrorDatabaseText, xGetErrorDatabaseText,
                                         (::Display*, tukk, tukk, tukk, tukk, i32),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetErrorText, xGetErrorText,
                                         (::Display*, i32, tukk, i32),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetGeometry, xGetGeometry,
                                         (::Display*, ::Drawable, ::Window*, i32*, i32*, u32*, u32*, u32*, u32*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetImage, xGetImage,
                                         (::Display*, ::Drawable, i32, i32, u32, u32, u64, i32),
                                         XImage*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetInputFocus, xGetInputFocus,
                                         (::Display*, ::Window*, i32*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetModifierMapping, xGetModifierMapping,
                                         (::Display*),
                                         XModifierKeymap*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetPointerMapping, xGetPointerMapping,
                                         (::Display*, u8[], i32),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetSelectionOwner, xGetSelectionOwner,
                                         (::Display*, Atom),
                                         ::Window)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetVisualInfo, xGetVisualInfo,
                                         (::Display*, i64, XVisualInfo*, i32*),
                                         XVisualInfo*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetWMHints, xGetWMHints,
                                         (::Display*, ::Window),
                                         XWMHints*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetWindowAttributes, xGetWindowAttributes,
                                         (::Display*, ::Window, XWindowAttributes*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGetWindowProperty, xGetWindowProperty,
                                         (::Display*, ::Window, Atom, i64, i64, Bool, Atom, Atom*, i32*, u64*, u64*, u8**),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGrabPointer, xGrabPointer,
                                         (::Display*, ::Window, Bool, u32, i32, i32, ::Window, Cursor, ::Time),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XGrabServer, xGrabServer,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XImageByteOrder, xImageByteOrder,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XInitImage, xInitImage,
                                         (XImage*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XInitThreads, xInitThreads,
                                         (),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XInstallColormap, xInstallColormap,
                                         (::Display*, Colormap),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XInternAtom, xInternAtom,
                                         (::Display*, tukk, Bool),
                                         Atom)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XkbKeycodeToKeysym, xkbKeycodeToKeysym,
                                         (::Display*, KeyCode, u32, u32),
                                         KeySym)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XKeysymToKeycode, xKeysymToKeycode,
                                         (::Display*, KeySym),
                                         KeyCode)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XListProperties, xListProperties,
                                         (::Display*, Window, i32*),
                                         Atom*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XLockDisplay, xLockDisplay,
                                         (::Display*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XLookupString, xLookupString,
                                         (XKeyEvent*, tukk, i32, KeySym*, XComposeStatus*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XMapRaised, xMapRaised,
                                         (::Display*, ::Window),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XMapWindow, xMapWindow,
                                         (::Display*, ::Window),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XMoveResizeWindow, xMoveResizeWindow,
                                         (::Display*, ::Window, i32, i32, u32, u32),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XNextEvent, xNextEvent,
                                         (::Display*, XEvent*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XOpenDisplay, xOpenDisplay,
                                         (tukk),
                                         ::Display*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XPeekEvent, xPeekEvent,
                                         (::Display*, XEvent*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XPending, xPending,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XPutImage, xPutImage,
                                         (::Display*, ::Drawable, GC, XImage*, i32, i32, i32, i32, u32, u32),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XPutPixel, xPutPixel,
                                         (XImage*, i32, i32, u64),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XQueryBestCursor, xQueryBestCursor,
                                         (::Display*, ::Drawable, u32, u32, u32*, u32*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XQueryExtension, xQueryExtension,
                                         (::Display*, tukk, i32*, i32*, i32*),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XQueryPointer, xQueryPointer,
                                         (::Display*, ::Window, ::Window*, ::Window*, i32*, i32*, i32*, i32*, u32*),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XQueryTree, xQueryTree,
                                         (::Display*, ::Window, ::Window*, ::Window*, ::Window**, u32*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRefreshKeyboardMapping, xRefreshKeyboardMapping,
                                         (XMappingEvent*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XReparentWindow, xReparentWindow,
                                         (::Display*, ::Window, ::Window, i32, i32),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XResizeWindow, xResizeWindow,
                                         (::Display*, Window, u32, u32),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRestackWindows, xRestackWindows,
                                         (::Display*, ::Window[], i32),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRootWindow, xRootWindow,
                                         (::Display*, i32),
                                         ::Window)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSaveContext, xSaveContext,
                                         (::Display*, XID, XContext, XPointer),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XScreenCount, xScreenCount,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XScreenNumberOfScreen, xScreenNumberOfScreen,
                                         (Screen*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSelectInput, xSelectInput,
                                         (::Display*, ::Window, i64),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSendEvent, xSendEvent,
                                         (::Display*, ::Window, Bool, i64, XEvent*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetClassHint, xSetClassHint,
                                         (::Display*, ::Window, XClassHint*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetErrorHandler, xSetErrorHandler,
                                         (XErrorHandler),
                                         XErrorHandler)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetIOErrorHandler, xSetIOErrorHandler,
                                         (XIOErrorHandler),
                                         XIOErrorHandler)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetInputFocus, xSetInputFocus,
                                         (::Display*, ::Window, i32, ::Time),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetSelectionOwner, xSetSelectionOwner,
                                         (::Display*, Atom, ::Window, ::Time),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetWMHints, xSetWMHints,
                                         (::Display*, ::Window, XWMHints*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetWMIconName, xSetWMIconName,
                                         (::Display*, ::Window, XTextProperty*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetWMName, xSetWMName,
                                         (::Display*, ::Window, XTextProperty*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSetWMNormalHints, xSetWMNormalHints,
                                         (::Display*, ::Window, XSizeHints*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XStringListToTextProperty, xStringListToTextProperty,
                                         (tuk*, i32, XTextProperty*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (Xutf8TextListToTextProperty, xutf8TextListToTextProperty,
                                         (::Display*, tuk*, i32, XICCEncodingStyle, XTextProperty*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSync, xSync,
                                         (::Display*, Bool),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XSynchronize, xSynchronize,
                                         (::Display*, Bool),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XTranslateCoordinates, xTranslateCoordinates,
                                         (::Display*, ::Window, ::Window, i32, i32, i32*, i32*, ::Window*),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XrmUniqueQuark, xrmUniqueQuark,
                                         (),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XUngrabPointer, xUngrabPointer,
                                         (::Display*, ::Time),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XUngrabServer, xUngrabServer,
                                         (::Display*),
                                         i32)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XUnlockDisplay, xUnlockDisplay,
                                         (::Display*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XUnmapWindow, xUnmapWindow,
                                         (::Display*, ::Window),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XWarpPointer, xWarpPointer,
                                         (::Display*, ::Window, ::Window, i32, i32, u32, u32, i32, i32),
                                         z0)
   #if DRX_USE_XCURSOR
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XcursorImageCreate, xcursorImageCreate,
                                         (i32, i32),
                                         XcursorImage*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XcursorImageLoadCursor, xcursorImageLoadCursor,
                                         (::Display*, XcursorImage*),
                                         Cursor)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XcursorImageDestroy, xcursorImageDestroy,
                                         (XcursorImage*),
                                         z0)
   #endif
   #if DRX_USE_XINERAMA
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XineramaIsActive, xineramaIsActive,
                                         (::Display*),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XineramaQueryScreens, xineramaQueryScreens,
                                         (::Display*, i32*),
                                         XineramaScreenInfo*)
   #endif
   #if DRX_USE_XRENDER
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRenderQueryVersion, xRenderQueryVersion,
                                         (::Display*, i32*, i32*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRenderFindStandardFormat, xRenderFindStandardFormat,
                                         (Display*, i32),
                                         XRenderPictFormat*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRenderFindFormat, xRenderFindFormat,
                                         (Display*, u64, XRenderPictFormat*, i32),
                                         XRenderPictFormat*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRenderFindVisualFormat, xRenderFindVisualFormat,
                                         (Display*, Visual*),
                                         XRenderPictFormat*)
   #endif
   #if DRX_USE_XRANDR
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRRGetScreenResources, xRRGetScreenResources,
                                         (::Display*, Window),
                                         XRRScreenResources*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRRFreeScreenResources, xRRFreeScreenResources,
                                         (XRRScreenResources*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRRGetOutputInfo, xRRGetOutputInfo,
                                         (::Display*, XRRScreenResources*, RROutput),
                                         XRROutputInfo*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRRFreeOutputInfo, xRRFreeOutputInfo,
                                         (XRROutputInfo*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRRGetCrtcInfo, xRRGetCrtcInfo,
                                         (::Display*, XRRScreenResources*, RRCrtc),
                                         XRRCrtcInfo*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRRFreeCrtcInfo, xRRFreeCrtcInfo,
                                         (XRRCrtcInfo*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XRRGetOutputPrimary, xRRGetOutputPrimary,
                                         (::Display*, ::Window),
                                         RROutput)
   #endif
   #if DRX_USE_XSHM
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XShmAttach, xShmAttach,
                                         (::Display*, XShmSegmentInfo*),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XShmCreateImage, xShmCreateImage,
                                         (::Display*, Visual*, u32, i32, tukk, XShmSegmentInfo*, u32, u32),
                                         XImage*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XShmDetach, xShmDetach,
                                         (::Display*, XShmSegmentInfo*),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XShmGetEventBase, xShmGetEventBase,
                                         (::Display*),
                                         Status)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XShmPutImage, xShmPutImage,
                                         (::Display*, ::Drawable, GC, XImage*, i32, i32, i32, i32, u32, u32, b8),
                                         Bool)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (XShmQueryVersion, xShmQueryVersion,
                                         (::Display*, i32*, i32*, Bool*),
                                         Bool)
   #endif

    //==============================================================================
    DRX_DECLARE_SINGLETON_INLINE (X11Symbols, false)

private:
    X11Symbols() = default;

    ~X11Symbols()
    {
        clearSingletonInstance();
    }

    //==============================================================================
    DynamicLibrary xLib { "libX11.so.6" }, xextLib { "libXext.so.6" };

   #if DRX_USE_XCURSOR
    DynamicLibrary xcursorLib  { "libXcursor.so.1" };
   #endif
   #if DRX_USE_XINERAMA
    DynamicLibrary xineramaLib { "libXinerama.so.1" };
   #endif
   #if DRX_USE_XRENDER
    DynamicLibrary xrenderLib  { "libXrender.so.1" };
   #endif
   #if DRX_USE_XRANDR
    DynamicLibrary xrandrLib   { "libXrandr.so.2" };
   #endif

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (X11Symbols)
};

} // namespace drx
