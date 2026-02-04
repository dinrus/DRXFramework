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

AudioCDReader::AudioCDReader()
    : AudioFormatReader (0, "CD Audio")
{
}

StringArray AudioCDReader::getAvailableCDNames()
{
    StringArray names;
    return names;
}

AudioCDReader* AudioCDReader::createReaderForCD (i32k)
{
    return nullptr;
}

AudioCDReader::~AudioCDReader()
{
}

z0 AudioCDReader::refreshTrackLengths()
{
}

b8 AudioCDReader::readSamples (i32* const*, i32, i32,
                                 z64, i32)
{
    return false;
}

b8 AudioCDReader::isCDStillPresent() const
{
    return false;
}

b8 AudioCDReader::isTrackAudio (i32) const
{
    return false;
}

z0 AudioCDReader::enableIndexScanning (b8)
{
}

i32 AudioCDReader::getLastIndex() const
{
    return 0;
}

Array<i32> AudioCDReader::findIndexesInTrack (i32k)
{
    return {};
}

} // namespace drx
