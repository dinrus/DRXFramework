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
/**
    Maintains an ongoing measurement of the proportion of time which is being
    spent inside an audio callback.

    @tags{Audio}
*/
class DRX_API  AudioProcessLoadMeasurer
{
public:
    /** */
    AudioProcessLoadMeasurer();

    /** Destructor. */
    ~AudioProcessLoadMeasurer();

    //==============================================================================
    /** Resets the state. */
    z0 reset();

    /** Resets the counter, in preparation for use with the given sample rate and block size. */
    z0 reset (f64 sampleRate, i32 blockSize);

    /** Returns the current load as a proportion 0 to 1.0 */
    f64 getLoadAsProportion() const;

    /** Returns the current load as a percentage 0 to 100.0 */
    f64 getLoadAsPercentage() const;

    /** Returns the number of over- (or under-) runs recorded since the state was reset. */
    i32 getXRunCount() const;

    //==============================================================================
    /** This class measures the time between its construction and destruction and
        adds it to an AudioProcessLoadMeasurer.

        e.g.
        @code
        {
            AudioProcessLoadMeasurer::ScopedTimer timer (myProcessLoadMeasurer);
            myCallback->doTheCallback();
        }
        @endcode

        @tags{Audio}
    */
    struct DRX_API  ScopedTimer
    {
        ScopedTimer (AudioProcessLoadMeasurer&);
        ScopedTimer (AudioProcessLoadMeasurer&, i32 numSamplesInBlock);
        ~ScopedTimer();

    private:
        AudioProcessLoadMeasurer& owner;
        f64 startTime;
        i32 samplesInBlock;

        DRX_DECLARE_NON_COPYABLE (ScopedTimer)
    };

    /** Can be called manually to add the time of a callback to the stats.
        Normally you probably would never call this - it's simpler and more robust to
        use a ScopedTimer to measure the time using an RAII pattern.
    */
    z0 registerBlockRenderTime (f64 millisecondsTaken);

    /** Can be called manually to add the time of a callback to the stats.
        Normally you probably would never call this - it's simpler and more robust to
        use a ScopedTimer to measure the time using an RAII pattern.
    */
    z0 registerRenderTime (f64 millisecondsTaken, i32 numSamples);

private:
    z0 registerRenderTimeLocked (f64, i32);

    SpinLock mutex;
    i32 samplesPerBlock = 0;
    f64 msPerSample = 0;
    std::atomic<f64> cpuUsageProportion { 0 };
    std::atomic<i32> xruns { 0 };
};


} // namespace drx
