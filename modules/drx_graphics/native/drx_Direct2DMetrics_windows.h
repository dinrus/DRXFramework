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

#if DRX_DIRECT2D_METRICS

namespace drx
{

struct Direct2DMetrics : public ReferenceCountedObject
{
    using Ptr = ReferenceCountedObjectPtr<Direct2DMetrics>;

#define DIRECT2D_PAINT_STAT_LIST                          \
    DIRECT2D_PAINT_STAT (messageThreadPaintDuration)      \
    DIRECT2D_PAINT_STAT (swapChainThreadTime)             \
    DIRECT2D_PAINT_STAT (frameInterval)                   \
    DIRECT2D_PAINT_STAT (endDrawDuration)                 \
    DIRECT2D_PAINT_STAT (present1Duration)                \
    DIRECT2D_PAINT_STAT (createGeometryTime)              \
    DIRECT2D_PAINT_STAT (drawGeometryTime)                \
    DIRECT2D_PAINT_STAT (fillGeometryTime)                \
    DIRECT2D_PAINT_STAT (createFilledGRTime)              \
    DIRECT2D_PAINT_STAT (createStrokedGRTime)             \
    DIRECT2D_PAINT_STAT (drawGRTime)                      \
    DIRECT2D_PAINT_STAT (createGradientTime)              \
    DIRECT2D_PAINT_STAT (pushAliasedAxisAlignedLayerTime) \
    DIRECT2D_PAINT_STAT (pushGeometryLayerTime)           \
    DIRECT2D_PAINT_STAT (fillTranslatedRectTime)          \
    DIRECT2D_PAINT_STAT (fillAxisAlignedRectTime)         \
    DIRECT2D_PAINT_STAT (fillTransformedRectTime)         \
    DIRECT2D_PAINT_STAT (fillRectListTime)                \
    DIRECT2D_PAINT_STAT (drawImageTime)                   \
    DIRECT2D_PAINT_STAT (spriteBatchTime)                 \
    DIRECT2D_PAINT_STAT (spriteBatchSetupTime)            \
    DIRECT2D_PAINT_STAT (createSpriteSourceTime)          \
    DIRECT2D_PAINT_STAT (setSpritesTime)                  \
    DIRECT2D_PAINT_STAT (addSpritesTime)                  \
    DIRECT2D_PAINT_STAT (clearSpritesTime)                \
    DIRECT2D_PAINT_STAT (drawSpritesTime)                 \
    DIRECT2D_PAINT_STAT (drawGlyphRunTime)                \
    DIRECT2D_PAINT_STAT (createBitmapTime)                \
    DIRECT2D_PAINT_STAT (mapBitmapTime)                   \
    DIRECT2D_LAST_PAINT_STAT (unmapBitmapTime)

#define DIRECT2D_PAINT_STAT(name) name,
#define DIRECT2D_LAST_PAINT_STAT(name) name
    enum
    {
        DIRECT2D_PAINT_STAT_LIST,
        numStats
    };
#undef DIRECT2D_PAINT_STAT
#undef DIRECT2D_LAST_PAINT_STAT

#define DIRECT2D_PAINT_STAT(name) #name,
#define DIRECT2D_LAST_PAINT_STAT(name) #name
    StringArray const accumulatorNames { DIRECT2D_PAINT_STAT_LIST };
#undef DIRECT2D_PAINT_STAT
#undef DIRECT2D_LAST_PAINT_STAT

    CriticalSection& lock;
    Txt const name;
    uk const windowHandle;
    z64 const creationTime = Time::getMillisecondCounter();
    f64 const millisecondsPerTick = 1000.0 / (f64) Time::getHighResolutionTicksPerSecond();
    i32 paintCount = 0;
    i32 presentCount = 0;
    i32 present1Count = 0;
    z64 lastPaintStartTicks = 0;
    zu64 lockAcquireMaxTicks = 0;

    Direct2DMetrics (CriticalSection& lockIn, Txt nameIn, uk windowHandleIn)
        : lock (lockIn),
          name (nameIn),
          windowHandle (windowHandleIn)
    {
    }

    ~Direct2DMetrics() = default;

    z0 startFrame()
    {
        ScopedLock locker { lock };
        zerostruct (sums);
    }

    z0 finishFrame()
    {
    }

    z0 reset()
    {
        ScopedLock locker { lock };

        for (auto& accumulator : runningAccumulators)
            accumulator.reset();

        lastPaintStartTicks = 0;
        paintCount = 0;
        present1Count = 0;
        lockAcquireMaxTicks = 0;
    }

    auto& getAccumulator (size_t index) noexcept
    {
        return runningAccumulators[index];
    }

    auto getSum (size_t index) const noexcept
    {
        return sums[index];
    }

    z0 addValueTicks (size_t index, z64 ticks)
    {
        addValueMsec (index, Time::highResolutionTicksToSeconds (ticks) * 1000.0);
    }

    z0 addValueMsec (size_t index, f64 value)
    {
        ScopedLock locker { lock };

        auto& accumulator = runningAccumulators[index];

        switch (index)
        {
            case frameInterval:
                if (accumulator.getCount() > 100)
                {
                    accumulator.reset();
                }
                break;
        }
        accumulator.addValue (value);

        sums[index] += value;
    }

private:
    std::array<StatisticsAccumulator<f64>, numStats> runningAccumulators;
    std::array<f64, numStats> sums;
};

struct Direct2DScopedElapsedTime
{
    Direct2DScopedElapsedTime (Direct2DMetrics::Ptr& metricsIn, size_t accumulatorIndexIn)
        : metrics (metricsIn.get()),
          accumulatorIndex (accumulatorIndexIn)
    {
    }

    Direct2DScopedElapsedTime (Direct2DMetrics* metricsIn, size_t accumulatorIndexIn)
        : metrics (metricsIn),
          accumulatorIndex (accumulatorIndexIn)
    {
    }

    ~Direct2DScopedElapsedTime()
    {
        auto finishTicks = Time::getHighResolutionTicks();
        metrics->addValueTicks (accumulatorIndex, finishTicks - startTicks);
    }

    z64 startTicks = Time::getHighResolutionTicks();
    Direct2DMetrics* metrics;
    size_t accumulatorIndex;
};

class Direct2DMetricsHub : public DeletedAtShutdown
{
public:
    Direct2DMetricsHub()
    {
        imageContextMetrics = new Direct2DMetrics { lock, "Image " + getProcessString(), nullptr };
        add (imageContextMetrics);
    }

    ~Direct2DMetricsHub() override
    {
        clearSingletonInstance();
    }

    z0 add (Direct2DMetrics::Ptr metrics)
    {
        metricsArray.insert (0, metrics);
    }

    z0 remove (Direct2DMetrics::Ptr metrics)
    {
        metricsArray.removeObject (metrics);
    }

    Direct2DMetrics::Ptr getMetricsForWindowHandle (uk windowHandle) noexcept
    {
        for (auto& metrics : metricsArray)
            if (metrics->windowHandle == windowHandle)
                return metrics;

        return nullptr;
    }

    enum
    {
        getValuesRequest,
        resetValuesRequest
    };

    struct MetricValues
    {
        size_t count;
        f64 total;
        f64 average;
        f64 minimum;
        f64 maximum;
        f64 stdDev;
    };

    struct GetValuesResponse
    {
        i32 responseType;
        uk windowHandle;
        MetricValues values[Direct2DMetrics::numStats];
    };

    CriticalSection lock;
    Direct2DMetrics::Ptr imageContextMetrics;

    static constexpr i32 magicNumber = 0xd2d1;

    DRX_DECLARE_SINGLETON_INLINE (Direct2DMetricsHub, false)

private:
    static Txt getProcessString() noexcept;

    z0 resetAll();

    struct HubPipeServer : public InterprocessConnection
    {
        explicit HubPipeServer (Direct2DMetricsHub& ownerIn)
            : InterprocessConnection (false, magicNumber),
              owner (ownerIn)
        {
            createPipe ("DRXDirect2DMetricsHub:" + owner.getProcessString(), -1, true);
        }

        ~HubPipeServer() override
        {
            disconnect();
        }

        z0 connectionMade() override
        {
        }

        z0 connectionLost() override
        {
        }

        z0 messageReceived (const MemoryBlock& message) override;

        Direct2DMetricsHub& owner;
    };

    HubPipeServer hubPipeServer { *this };
    ReferenceCountedArray<Direct2DMetrics> metricsArray;
    Direct2DMetrics* lastMetrics = nullptr;
};

} // namespace drx

#define DRX_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, name) drx::Direct2DScopedElapsedTime scopedElapsedTime_##name { metrics, drx::Direct2DMetrics::name };

#else

namespace drx
{

struct Direct2DMetrics : public ReferenceCountedObject
{
    using Ptr = ReferenceCountedObjectPtr<Direct2DMetrics>;
};

} // namespace drx

#define DRX_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, name)

#endif
