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

#pragma once

namespace drx::etw
{

//==============================================================================
/*
    The following XML can be passed to Windows Performance Recorder (WPR) to enable tracing from
    DRX projects.
    - Save the XML into a file with the name DRX.wprp
    - Run the following command from an admin command prompt to start capture. If you use a Developer
      Command Prompt, wpr.exe should be found on the PATH already.
        - <path to wpr>\wpr.exe -start DRX.wprp
    - Start your DRX project
    - Run the following command from an admin command prompt to stop capture
        - <path to wpr>\wpr.exe -stop TraceCaptureFile.etl description

<?xml version="1.0" encoding="utf-8"?>
<WindowsPerformanceRecorder Version="1.0" Author="Microsoft Corporation" Copyright="Microsoft Corporation" Company="Microsoft Corporation">
  <Profiles>
    <EventCollector Id="EventCollector_DRXTraceLogProvider" Name="DRXTraceLogProvider">
      <BufferSize Value="64" />
      <Buffers Value="4" />
    </EventCollector>

    <EventProvider Id="EventProvider_DRXTraceLogProvider" Name="6A612E78-284D-4DDB-877A-5F521EB33132" />

    <Profile Id="DRXTraceLogProvider.Verbose.File" Name="DRXTraceLogProvider" Description="DRXTraceLogProvider" LoggingMode="File" DetailLevel="Verbose">
      <Collectors>
        <EventCollectorId Value="EventCollector_DRXTraceLogProvider">
          <EventProviders>
            <EventProviderId Value="EventProvider_DRXTraceLogProvider" />
          </EventProviders>
        </EventCollectorId>
      </Collectors>
    </Profile>

    <Profile Id="DRXTraceLogProvider.Light.File" Name="DRXTraceLogProvider" Description="DRXTraceLogProvider" Base="DRXTraceLogProvider.Verbose.File" LoggingMode="File" DetailLevel="Light" />
    <Profile Id="DRXTraceLogProvider.Verbose.Memory" Name="DRXTraceLogProvider" Description="DRXTraceLogProvider" Base="DRXTraceLogProvider.Verbose.File" LoggingMode="Memory" DetailLevel="Verbose" />
    <Profile Id="DRXTraceLogProvider.Light.Memory" Name="DRXTraceLogProvider" Description="DRXTraceLogProvider" Base="DRXTraceLogProvider.Verbose.File" LoggingMode="Memory" DetailLevel="Light" />

  </Profiles>
</WindowsPerformanceRecorder>

*/

//==============================================================================
enum
{
    paintKeyword            = 1 << 0,
    sizeKeyword             = 1 << 1,
    graphicsKeyword         = 1 << 2,
    crucialKeyword          = 1 << 3,
    threadPaintKeyword      = 1 << 4,
    messageKeyword          = 1 << 5,
    direct2dKeyword         = 1 << 6,
    softwareRendererKeyword = 1 << 7,
    resourcesKeyword        = 1 << 8,
    componentKeyword        = 1 << 9,
    spriteKeyword           = 1 << 10
};

enum : zu64
{
    direct2dHwndPaintStart,
    direct2dHwndPaintEnd,
    endDraw,
    present1SwapChainStart,
    present1SwapChainEnd,
    swapChainThreadEvent,
    waitForVBlankDone,
    callVBlankListeners,
    resize,
    createResource,
    presentIdleFrame,
    direct2dImagePaintStart,
    direct2dImagePaintEnd,
    startD2DFrame,
    flush,
    startGDIFrame,
    startGDIImage,
    endGDIFrame,
    createLowLevelGraphicsContext,
    createDeviceResources,
    createSwapChain,
    createSwapChainBuffer,
    createPeer,
    mapBitmap,
    unmapBitmap,
    createDirect2DBitmapFromImage,
    createDirect2DBitmap,
    setOrigin,
    addTransform,
    clipToRectangle,
    clipToRectangleList,
    excludeClipRectangle,
    clipToPath,
    clipToImageAlpha,
    saveState,
    restoreState,
    beginTransparencyLayer,
    endTransparencyLayer,
    setFill,
    setOpacity,
    setInterpolationQuality,
    fillRect,
    fillRectReplace,
    fillRectList,
    drawRectTranslated,
    drawRectTransformed,
    drawRect,
    fillPath,
    strokePath,
    drawPath,
    drawImage,
    drawLine,
    setFont,
    drawGlyph,
    drawGlyphRun,
    drawTextLayout,
    drawRoundedRectangle,
    fillRoundedRectangle,
    drawEllipse,
    fillEllipse,
    filledGeometryRealizationCacheHit,
    filledGeometryRealizationCreated,
    strokedGeometryRealizationCacheHit,
    strokedGeometryRealizationCreated,
    releaseGeometryRealization,
    gradientCacheHit,
    gradientCreated,
    releaseGradient,
    nativeDropShadow,
    nativeGlowEffect,
    resetToDefaultState,
    reduceClipRegionRectangle,
    reduceClipRegionRectangleList,
    reduceClipRegionImage,
    reduceClipRegionPath,
    excludeClipRegion,
    fillAll,
    repaint,
    paintComponentAndChildren,
    paintWithinParentContext,
    createSpriteBatch,
    setSprites,
    addSprites,
    drawSprites,
};

#if DRX_WINDOWS && DRX_ETW_TRACELOGGING
    DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wc++98-compat-extra-semi", "-Wmissing-prototypes", "-Wgnu-zero-variadic-macro-arguments")
    TRACELOGGING_DECLARE_PROVIDER (DRXTraceLogProvider);
    DRX_END_IGNORE_WARNINGS_GCC_LIKE

    #define DRX_WRITE_TRACE_LOG_VA(code, keyword, ...) \
        TraceLoggingWrite (::drx::etw::DRXTraceLogProvider, \
                           #code, \
                           TraceLoggingLevel (TRACE_LEVEL_INFORMATION), \
                           TraceLoggingKeyword ((UINT64) keyword), \
                           __VA_ARGS__, \
                           TraceLoggingValue ((u16) code, "code"))
    #define DRX_WRITE_TRACE_LOG(code, keyword) \
        TraceLoggingWrite (::drx::etw::DRXTraceLogProvider, \
                           #code, \
                           TraceLoggingLevel (TRACE_LEVEL_INFORMATION), \
                           TraceLoggingKeyword ((UINT64) keyword), \
                           TraceLoggingValue ((u16) code, "code"))
#else
    #define DRX_WRITE_TRACE_LOG_VA(code, keyword, ...)
    #define DRX_WRITE_TRACE_LOG(code, keyword)
#endif

template <typename Number>
auto toVector (const Rectangle<Number>& r)
{
    return std::vector { r.getX(), r.getY(), r.getWidth(), r.getHeight() };
}

template <typename Number>
auto toVector (const RectangleList<Number>& list)
{
    std::vector<Number> result;

    for (const auto& r : list)
        result.insert (result.end(), { r.getX(), r.getY(), r.getWidth(), r.getHeight() });

    return result;
}

//==============================================================================
#if DRX_WINDOWS && DRX_ETW_TRACELOGGING
    #define DRX_SCOPED_TRACE_EVENT_FRAME(code, keyword, frameNumber) \
        const ScopeGuard scopedEvent_ ## __LINE__ { [start = ::drx::Time::getHighResolutionTicks(), frame = (frameNumber)] \
        {                                                    \
            const auto ticks = ::drx::Time::getHighResolutionTicks() - start; \
            DRX_WRITE_TRACE_LOG_VA (code, keyword, TraceLoggingValue (ticks), TraceLoggingValue (frame)); \
        } };

    #define DRX_SCOPED_TRACE_EVENT_FRAME_RECT_F32(code, keyword, frameNumber, rectIn) \
        const ScopeGuard scopedEvent_ ## __LINE__ { [start = ::drx::Time::getHighResolutionTicks(), frame = (frameNumber), rect = (rectIn)] \
        { \
            const auto ticks = ::drx::Time::getHighResolutionTicks() - start;       \
            const std::vector<f32> vec = ::drx::etw::toVector (rect); \
            DRX_WRITE_TRACE_LOG_VA (code,                                             \
                                     keyword,                                          \
                                     TraceLoggingValue (ticks),                        \
                                     TraceLoggingValue (frame),                        \
                                     TraceLoggingFloat32Array (vec.data(), (UINT16) vec.size(), "rect")); \
        } };

    #define DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32(code, keyword, frameNumber, rectIn) \
        const ScopeGuard scopedEvent_ ## __LINE__ { [start = ::drx::Time::getHighResolutionTicks(), frame = (frameNumber), rect = (rectIn)] \
        { \
            const auto ticks = ::drx::Time::getHighResolutionTicks() - start;       \
            const std::vector<INT32> vec = ::drx::etw::toVector (rect); \
            DRX_WRITE_TRACE_LOG_VA (code,                                             \
                                     keyword,                                          \
                                     TraceLoggingValue (ticks),                        \
                                     TraceLoggingValue (frame),                        \
                                     TraceLoggingInt32Array (vec.data(), (UINT16) vec.size(), "rect")); \
        } };
#else
    #define DRX_SCOPED_TRACE_EVENT_FRAME(code, keyword, frameNumber)
    #define DRX_SCOPED_TRACE_EVENT_FRAME_RECT_F32(code, keyword, frameNumber, rectIn)
    #define DRX_SCOPED_TRACE_EVENT_FRAME_RECT_I32(code, keyword, frameNumber, rectIn)
#endif

//==============================================================================
#define DRX_TRACE_LOG_D2D_PAINT_CALL(code, frameNumber) \
    DRX_WRITE_TRACE_LOG_VA (code, etw::paintKeyword | etw::direct2dKeyword, TraceLoggingValue ((UINT64) frameNumber, "frame"))

#define DRX_TRACE_LOG_DRX_VBLANK_THREAD_EVENT \
    DRX_WRITE_TRACE_LOG (etw::waitForVBlankDone, etw::softwareRendererKeyword)

#define DRX_TRACE_LOG_DRX_VBLANK_CALL_LISTENERS \
    DRX_WRITE_TRACE_LOG (etw::callVBlankListeners, etw::softwareRendererKeyword)

#define DRX_TRACE_LOG_D2D_RESIZE(message) \
    DRX_WRITE_TRACE_LOG_VA (etw::resize, etw::paintKeyword | etw::direct2dKeyword, TraceLoggingValue (message, "message"))

#define DRX_TRACE_LOG_D2D_IMAGE_MAP_DATA \
    DRX_WRITE_TRACE_LOG (etw::mapBitmap, etw::direct2dKeyword)

#define DRX_TRACE_LOG_D2D_IMAGE_UNMAP_DATA \
    DRX_WRITE_TRACE_LOG (etw::unmapBitmap, etw::direct2dKeyword)

#define DRX_TRACE_LOG_PAINT_COMPONENT_AND_CHILDREN(depth) \
    DRX_WRITE_TRACE_LOG_VA (etw::paintComponentAndChildren, etw::paintKeyword, TraceLoggingValue (depth, "depth"))

#define DRX_TRACE_LOG_PAINT_CALL(code, frameNumber) \
    DRX_WRITE_TRACE_LOG_VA (code, etw::softwareRendererKeyword, TraceLoggingValue ((UINT64) frameNumber, "frame"))

#if DRX_ETW_TRACELOGGING
    #define DRX_TRACE_EVENT_INT_RECT_LIST(code, keyword, frameNumber, rect) \
        { \
            const std::vector<INT32> vec = ::drx::etw::toVector (rect); \
            DRX_WRITE_TRACE_LOG (code,                              \
                                  etw::softwareRendererKeyword,      \
                                  TraceLoggingValue ((UINT64) frameNumber, "frame"), \
                                  TraceLoggingInt32Array (vec.data(), (UINT16) vec.size(), "rect")); \
        }

    #define DRX_TRACE_EVENT_INT_RECT(code, keyword, rect) \
        { \
            const std::vector<INT32> vec = ::drx::etw::toVector (rect); \
            DRX_WRITE_TRACE_LOG (code,                              \
                                  etw::softwareRendererKeyword,      \
                                  TraceLoggingInt32Array (vec.data(), (UINT16) vec.size(), "rect")); \
        }
#else
    #define DRX_TRACE_EVENT_INT_RECT_LIST(code, keyword, frameNumber, rect)
    #define DRX_TRACE_EVENT_INT_RECT(code, keyword, rect)
#endif

} // namespace drx::etw
