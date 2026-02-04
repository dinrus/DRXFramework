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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 DRX Module Format.md file.


 BEGIN_DRX_MODULE_DECLARATION

  ID:                 drx_graphics
  vendor:             drx
  version:            8.0.7
  name:               DRX graphics classes
  description:        Classes for 2D vector graphics, image loading/saving, font handling, etc.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_events
  OSXFrameworks:      Cocoa QuartzCore
  iOSFrameworks:      CoreGraphics CoreImage CoreText QuartzCore
  linuxPackages:      freetype2 fontconfig

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_GRAPHICS_H_INCLUDED

#include <drx_core/drx_core.h>
#include <drx_events/drx_events.h>

//==============================================================================
/** Config: DRX_USE_COREIMAGE_LOADER

    On OSX, enabling this flag means that the CoreImage codecs will be used to load
    PNG/JPEG/GIF files. It is enabled by default, but you may want to disable it if
    you'd rather use libpng, libjpeg, etc.
*/
#ifndef DRX_USE_COREIMAGE_LOADER
 #define DRX_USE_COREIMAGE_LOADER 1
#endif

/** Config: DRX_DISABLE_COREGRAPHICS_FONT_SMOOTHING

    Setting this flag will turn off CoreGraphics font smoothing on macOS, which some people
    find makes the text too 'fat' for their taste.
*/
#ifndef DRX_DISABLE_COREGRAPHICS_FONT_SMOOTHING
 #define DRX_DISABLE_COREGRAPHICS_FONT_SMOOTHING 0
#endif

#ifndef DRX_INCLUDE_PNGLIB_CODE
 #define DRX_INCLUDE_PNGLIB_CODE 1
#endif

#ifndef DRX_INCLUDE_JPEGLIB_CODE
 #define DRX_INCLUDE_JPEGLIB_CODE 1
#endif

#ifndef USE_COREGRAPHICS_RENDERING
 #define USE_COREGRAPHICS_RENDERING 1
#endif

//==============================================================================
namespace drx
{
    class Image;
    class AffineTransform;
    class Path;
    class Font;
    class Graphics;
    class FillType;
    class LowLevelGraphicsContext;
}

#include "geometry/drx_AffineTransform.h"
#include "geometry/drx_Point.h"
#include "geometry/drx_Line.h"
#include "geometry/drx_Rectangle.h"
#include "geometry/drx_Parallelogram.h"
#include "placement/drx_Justification.h"
#include "geometry/drx_Path.h"
#include "geometry/drx_RectangleList.h"
#include "colour/drx_PixelFormats.h"
#include "colour/drx_Color.h"
#include "colour/drx_ColorGradient.h"
#include "colour/drx_Colors.h"
#include "geometry/drx_BorderSize.h"
#include "geometry/drx_EdgeTable.h"
#include "geometry/drx_PathIterator.h"
#include "geometry/drx_PathStrokeType.h"
#include "placement/drx_RectanglePlacement.h"
#include "images/drx_ImageCache.h"
#include "images/drx_ImageConvolutionKernel.h"
#include "images/drx_ImageFileFormat.h"
#include "contexts/drx_GraphicsContext.h"
#include "images/drx_Image.h"
#include "colour/drx_FillType.h"
#include "fonts/drx_Typeface.h"
#include "fonts/drx_FontOptions.h"
#include "fonts/drx_Font.h"
#include "detail/drx_Ranges.h"
#include "detail/drx_SimpleShapedText.h"
#include "detail/drx_JustifiedText.h"
#include "detail/drx_ShapedText.h"
#include "fonts/drx_AttributedString.h"
#include "fonts/drx_GlyphArrangement.h"
#include "fonts/drx_TextLayout.h"
#include "contexts/drx_LowLevelGraphicsContext.h"
#include "images/drx_ScaledImage.h"
#include "fonts/drx_LruCache.h"
#include "native/drx_RenderingHelpers.h"
#include "contexts/drx_LowLevelGraphicsSoftwareRenderer.h"
#include "effects/drx_ImageEffectFilter.h"
#include "effects/drx_DropShadowEffect.h"
#include "effects/drx_GlowEffect.h"
#include "detail/drx_Unicode.h"

#if DRX_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS && (DRX_MAC || DRX_IOS)
 #include "native/drx_CoreGraphicsHelpers_mac.h"
 #include "native/drx_CoreGraphicsContext_mac.h"
#endif
