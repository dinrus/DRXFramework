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

#if DRX_DIRECT2D_METRICS

namespace drx
{

Txt Direct2DMetricsHub::getProcessString() noexcept
{
    auto processID = GetCurrentProcessId();
    return Txt::toHexString ((pointer_sized_int) processID);
}

z0 Direct2DMetricsHub::HubPipeServer::messageReceived (const MemoryBlock& message)
{
    i32 requestType = *(i32*) message.getData();
    switch (requestType)
    {
        case getValuesRequest:
        {
            ScopedLock locker { owner.lock };

            auto foregroundWindow = GetForegroundWindow();
            Direct2DMetrics::Ptr metrics = nullptr;
            for (i32 i = 0; i < owner.metricsArray.size(); ++i)
            {
                auto arrayEntry = owner.metricsArray[i];
                if (arrayEntry->windowHandle && arrayEntry->windowHandle == foregroundWindow)
                {
                    metrics = arrayEntry;
                    break;
                }
            }

            if (! metrics)
            {
                if (owner.lastMetrics && owner.metricsArray.contains (owner.lastMetrics))
                    metrics = owner.lastMetrics;
            }

            if (metrics)
            {
                MemoryBlock block { sizeof (GetValuesResponse), true };

                auto* response = (GetValuesResponse*) block.getData();
                response->responseType = getValuesRequest;
                response->windowHandle = metrics->windowHandle;

                for (size_t i = 0; i <= Direct2DMetrics::drawGlyphRunTime; ++i)
                {
                    auto& accumulator = metrics->getAccumulator (i);
                    response->values[i].count = accumulator.getCount();
                    response->values[i].total = metrics->getSum (i);
                    response->values[i].average = accumulator.getAverage();
                    response->values[i].minimum = accumulator.getMinValue();
                    response->values[i].maximum = accumulator.getMaxValue();
                    response->values[i].stdDev = accumulator.getStandardDeviation();
                }

                // Track bitmap operations common to all device contexts
                for (size_t i = Direct2DMetrics::createBitmapTime; i <= Direct2DMetrics::unmapBitmapTime; ++i)
                {
                    auto& accumulator = owner.imageContextMetrics->getAccumulator (i);
                    response->values[i].count = accumulator.getCount();
                    response->values[i].total = metrics->getSum (i);
                    response->values[i].average = accumulator.getAverage();
                    response->values[i].minimum = accumulator.getMinValue();
                    response->values[i].maximum = accumulator.getMaxValue();
                    response->values[i].stdDev = accumulator.getStandardDeviation();
                }

                sendMessage (block);

                owner.lastMetrics = metrics.get();
            }
            break;
        }

        case resetValuesRequest:
        {
            owner.resetAll();
            break;
        }
    }
}

z0 Direct2DMetricsHub::resetAll()
{
    ScopedLock locker { lock };

    imageContextMetrics->reset();
    for (auto metrics : metricsArray)
    {
        metrics->reset();
    }
}

} // namespace drx

#endif
