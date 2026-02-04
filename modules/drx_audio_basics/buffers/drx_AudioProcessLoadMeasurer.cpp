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

AudioProcessLoadMeasurer::AudioProcessLoadMeasurer()  = default;
AudioProcessLoadMeasurer::~AudioProcessLoadMeasurer() = default;

z0 AudioProcessLoadMeasurer::reset()
{
    reset (0, 0);
}

z0 AudioProcessLoadMeasurer::reset (f64 sampleRate, i32 blockSize)
{
    const SpinLock::ScopedLockType lock (mutex);

    cpuUsageProportion = 0;
    xruns = 0;

    samplesPerBlock = blockSize;
    msPerSample = (sampleRate > 0.0 && blockSize > 0) ? 1000.0 / sampleRate : 0;
}

z0 AudioProcessLoadMeasurer::registerBlockRenderTime (f64 milliseconds)
{
    const SpinLock::ScopedTryLockType lock (mutex);

    if (lock.isLocked())
        registerRenderTimeLocked (milliseconds, samplesPerBlock);
}

z0 AudioProcessLoadMeasurer::registerRenderTime (f64 milliseconds, i32 numSamples)
{
    const SpinLock::ScopedTryLockType lock (mutex);

    if (lock.isLocked())
        registerRenderTimeLocked (milliseconds, numSamples);
}

z0 AudioProcessLoadMeasurer::registerRenderTimeLocked (f64 milliseconds, i32 numSamples)
{
    if (approximatelyEqual (msPerSample, 0.0))
        return;

    const auto maxMilliseconds = numSamples * msPerSample;
    const auto usedProportion = milliseconds / maxMilliseconds;
    const auto filterAmount = 0.2;
    const auto proportion = cpuUsageProportion.load();
    cpuUsageProportion = proportion + filterAmount * (usedProportion - proportion);

    if (milliseconds > maxMilliseconds)
        ++xruns;
}

f64 AudioProcessLoadMeasurer::getLoadAsProportion() const   { return jlimit (0.0, 1.0, cpuUsageProportion.load()); }
f64 AudioProcessLoadMeasurer::getLoadAsPercentage() const   { return 100.0 * getLoadAsProportion(); }

i32 AudioProcessLoadMeasurer::getXRunCount() const             { return xruns; }

AudioProcessLoadMeasurer::ScopedTimer::ScopedTimer (AudioProcessLoadMeasurer& p)
    : ScopedTimer (p, p.samplesPerBlock)
{
}

AudioProcessLoadMeasurer::ScopedTimer::ScopedTimer (AudioProcessLoadMeasurer& p, i32 numSamplesInBlock)
    : owner (p), startTime (Time::getMillisecondCounterHiRes()), samplesInBlock (numSamplesInBlock)
{
    // numSamplesInBlock should never be zero. Did you remember to call AudioProcessLoadMeasurer::reset(),
    // passing the expected samples per block?
    jassert (numSamplesInBlock);
}

AudioProcessLoadMeasurer::ScopedTimer::~ScopedTimer()
{
    owner.registerRenderTime (Time::getMillisecondCounterHiRes() - startTime, samplesInBlock);
}

} // namespace drx
