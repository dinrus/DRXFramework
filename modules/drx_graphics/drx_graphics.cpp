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

#ifdef DRX_GRAPHICS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_CORE_INCLUDE_COM_SMART_PTR 1
#define DRX_CORE_INCLUDE_JNI_HELPERS 1
#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1

#include "drx_graphics.h"

//==============================================================================
#if DRX_MAC
 #import <QuartzCore/QuartzCore.h>
 #include <CoreImage/CoreImage.h>
 #include <CoreText/CTFont.h>

#elif DRX_WINDOWS
 // get rid of some warnings in Window's own headers
 DRX_BEGIN_IGNORE_WARNINGS_MSVC (4458)

 /* If you hit a compile error trying to include these files, you may need to update
    your version of the Windows SDK to the latest one. The DirectWrite and Direct2D
    headers are in the version 8 SDKs.

    Need Direct2D 1.3 for sprite batching
 */

 #include <d2d1_3.h>
 #include <d3d11_2.h>
 #include <dcomp.h>
 #include <dwrite_3.h>
 #include <dxgi1_3.h>
 #include <processthreadsapi.h>

 #if DRX_ETW_TRACELOGGING
  #include <evntrace.h>
  #include <TraceLoggingProvider.h>

  TRACELOGGING_DEFINE_PROVIDER (DRXTraceLogProvider,
                                "DRXTraceLogProvider",
                                // {6A612E78-284D-4DDB-877A-5F521EB33132}
                                (0x6a612e78, 0x284d, 0x4ddb, 0x87, 0x7a, 0x5f, 0x52, 0x1e, 0xb3, 0x31, 0x32));

#endif

 DRX_END_IGNORE_WARNINGS_MSVC

 #if ! DRX_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment(lib, "Dwrite.lib")
  #pragma comment(lib, "D2d1.lib")
  #pragma comment(lib, "DXGI.lib")
  #pragma comment(lib, "D3D11.lib")
  #pragma comment(lib, "DComp.lib")
  #pragma comment(lib, "dxguid.lib")
 #endif

 #include "native/drx_Direct2DPixelDataPage_windows.h"

#elif DRX_IOS
 #import <QuartzCore/QuartzCore.h>
 #import <CoreText/CoreText.h>

#elif DRX_LINUX || DRX_BSD
 #ifndef DRX_USE_FREETYPE
  #define DRX_USE_FREETYPE 1
 #endif

 #ifndef DRX_USE_FONTCONFIG
  #define DRX_USE_FONTCONFIG 1
 #endif
#elif DRX_ANDROID
 #include <android/font_matcher.h>
#endif

#if DRX_USE_FREETYPE
 #include <ft2build.h>
 #include FT_FREETYPE_H
 #include FT_ADVANCES_H
 #include FT_TRUETYPE_TABLES_H
 #include FT_GLYPH_H

 #ifdef FT_COLOR_H
  #include FT_COLOR_H
 #endif
#endif

#if DRX_USE_FONTCONFIG
 #include <fontconfig/fontconfig.h>
#endif

#undef SIZEOF

#if (DRX_MAC || DRX_IOS) && USE_COREGRAPHICS_RENDERING && DRX_USE_COREIMAGE_LOADER
 #define DRX_USING_COREIMAGE_LOADER 1
#else
 #define DRX_USING_COREIMAGE_LOADER 0
#endif

#if DRX_USE_FREETYPE
 #include <drx_graphics/fonts/harfbuzz/hb-ft.h>
#endif

#if DRX_WINDOWS
 #include <drx_graphics/fonts/harfbuzz/hb-directwrite.h>
#elif DRX_MAC || DRX_IOS
 #include <drx_graphics/fonts/harfbuzz/hb-coretext.h>
#endif

#include <drx_graphics/fonts/harfbuzz/hb-ot.h>

extern "C"
{
#include <drx_graphics/unicode/sheenbidi/Headers/SheenBidi.h>
} // extern "C"

#if DRX_UNIT_TESTS
 #include "fonts/drx_TypefaceTestData.cpp"
#endif

//==============================================================================
#include "drx_core/zip/drx_zlib.h"

#include "fonts/drx_FunctionPointerDestructor.h"
#include "native/drx_EventTracing.h"

#include "unicode/drx_UnicodeGenerated.cpp"
#include "unicode/drx_UnicodeUtils.cpp"
#include "unicode/drx_UnicodeLine.cpp"
#include "unicode/drx_UnicodeScript.cpp"
#include "unicode/drx_UnicodeBidi.cpp"
#include "unicode/drx_Unicode.cpp"
#include "colour/drx_Color.cpp"
#include "colour/drx_ColorGradient.cpp"
#include "colour/drx_Colors.cpp"
#include "colour/drx_FillType.cpp"
#include "geometry/drx_AffineTransform.cpp"
#include "geometry/drx_EdgeTable.cpp"
#include "geometry/drx_Path.cpp"
#include "geometry/drx_PathIterator.cpp"
#include "geometry/drx_PathStrokeType.cpp"
#include "placement/drx_RectanglePlacement.cpp"
#include "contexts/drx_GraphicsContext.cpp"
#include "contexts/drx_LowLevelGraphicsSoftwareRenderer.cpp"
#include "images/drx_Image.cpp"
#include "images/drx_ImageCache.cpp"
#include "images/drx_ImageConvolutionKernel.cpp"
#include "images/drx_ImageFileFormat.cpp"
#include "image_formats/drx_GIFLoader.cpp"
#include "image_formats/drx_JPEGLoader.cpp"
#include "image_formats/drx_PNGLoader.cpp"
#include "fonts/drx_AttributedString.cpp"
#include "fonts/drx_Typeface.cpp"
#include "fonts/drx_FontOptions.cpp"
#include "fonts/drx_Font.cpp"
#include "detail/drx_Ranges.cpp"
#include "detail/drx_SimpleShapedText.cpp"
#include "detail/drx_JustifiedText.cpp"
#include "detail/drx_ShapedText.cpp"
#include "fonts/drx_GlyphArrangement.cpp"
#include "fonts/drx_TextLayout.cpp"
#include "effects/drx_DropShadowEffect.cpp"
#include "effects/drx_GlowEffect.cpp"

#if DRX_UNIT_TESTS
 #include "geometry/drx_Parallelogram_test.cpp"
 #include "geometry/drx_Rectangle_test.cpp"
#endif

#if DRX_USE_FREETYPE
 #include "fonts/drx_TypefaceFileCache.h"
 #include "native/drx_Fonts_freetype.cpp"
#endif

//==============================================================================
#if DRX_MAC || DRX_IOS
 #include "native/drx_Fonts_mac.mm"
 #include "native/drx_CoreGraphicsContext_mac.mm"
 #include "native/drx_IconHelpers_mac.cpp"

#elif DRX_WINDOWS
 #include "native/drx_Direct2DMetrics_windows.h"
 #include "native/drx_Direct2DGraphicsContext_windows.h"
 #include "native/drx_Direct2DHwndContext_windows.h"
 #include "native/drx_DirectX_windows.h"
 #include "native/drx_Direct2DImage_windows.h"
 #include "native/drx_Direct2DImageContext_windows.h"

 #include "native/drx_DirectWriteTypeface_windows.cpp"
 #include "native/drx_IconHelpers_windows.cpp"
 #include "native/drx_Direct2DHelpers_windows.cpp"
 #include "native/drx_Direct2DResources_windows.cpp"
 #include "native/drx_Direct2DGraphicsContext_windows.cpp"
 #include "native/drx_Direct2DHwndContext_windows.cpp"
 #include "native/drx_Direct2DImageContext_windows.cpp"
 #include "native/drx_Direct2DImage_windows.cpp"
 #include "native/drx_Direct2DMetrics_windows.cpp"

#elif DRX_LINUX || DRX_BSD
 #include "native/drx_Fonts_linux.cpp"
 #include "native/drx_IconHelpers_linux.cpp"

#elif DRX_ANDROID
 #include "fonts/drx_TypefaceFileCache.h"
 #include "native/drx_GraphicsContext_android.cpp"
 #include "native/drx_Fonts_android.cpp"
 #include "native/drx_IconHelpers_android.cpp"

#endif
