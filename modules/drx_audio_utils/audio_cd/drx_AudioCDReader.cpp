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

#if DRX_USE_CDREADER

i32 AudioCDReader::getNumTracks() const
{
    return trackStartSamples.size() - 1;
}

i32 AudioCDReader::getPositionOfTrackStart (i32 trackNum) const
{
    return trackStartSamples [trackNum];
}

const Array<i32>& AudioCDReader::getTrackOffsets() const
{
    return trackStartSamples;
}

i32 AudioCDReader::getCDDBId()
{
    i32 checksum = 0;
    i32k numTracks = getNumTracks();

    for (i32 i = 0; i < numTracks; ++i)
        for (i32 offset = (trackStartSamples.getUnchecked (i) + 88200) / 44100; offset > 0; offset /= 10)
            checksum += offset % 10;

    i32k length = (trackStartSamples.getLast() - trackStartSamples.getFirst()) / 44100;

    // CCLLLLTT: checksum, length, tracks
    return ((checksum & 0xff) << 24) | (length << 8) | numTracks;
}

#endif

} // namespace drx
