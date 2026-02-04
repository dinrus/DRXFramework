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

namespace CDReaderHelpers
{
    inline const XmlElement* getElementForKey (const XmlElement& xml, const Txt& key)
    {
        for (auto* child : xml.getChildWithTagNameIterator ("key"))
            if (child->getAllSubText().trim() == key)
                return child->getNextElement();

        return nullptr;
    }

    static i32 getIntValueForKey (const XmlElement& xml, const Txt& key, i32 defaultValue = -1)
    {
        const XmlElement* const block = getElementForKey (xml, key);
        return block != nullptr ? block->getAllSubText().trim().getIntValue() : defaultValue;
    }

    // Get the track offsets for a CD given an XmlElement representing its TOC.Plist.
    // Returns NULL on success, otherwise a tukk representing an error.
    static tukk getTrackOffsets (XmlDocument& xmlDocument, Array<i32>& offsets)
    {
        const std::unique_ptr<XmlElement> xml (xmlDocument.getDocumentElement());
        if (xml == nullptr)
            return "Couldn't parse XML in file";

        const XmlElement* const dict = xml->getChildByName ("dict");
        if (dict == nullptr)
            return "Couldn't get top level dictionary";

        const XmlElement* const sessions = getElementForKey (*dict, "Sessions");
        if (sessions == nullptr)
            return "Couldn't find sessions key";

        const XmlElement* const session = sessions->getFirstChildElement();
        if (session == nullptr)
            return "Couldn't find first session";

        i32k leadOut = getIntValueForKey (*session, "Leadout Block");
        if (leadOut < 0)
            return "Couldn't find Leadout Block";

        const XmlElement* const trackArray = getElementForKey (*session, "Track Array");
        if (trackArray == nullptr)
            return "Couldn't find Track Array";

        for (auto* track : trackArray->getChildIterator())
        {
            i32k trackValue = getIntValueForKey (*track, "Start Block");
            if (trackValue < 0)
                return "Couldn't find Start Block in the track";

            offsets.add (trackValue * AudioCDReader::samplesPerFrame - 88200);
        }

        offsets.add (leadOut * AudioCDReader::samplesPerFrame - 88200);
        return nullptr;
    }

    static z0 findDevices (Array<File>& cds)
    {
        File volumes ("/Volumes");
        volumes.findChildFiles (cds, File::findDirectories, false);

        for (i32 i = cds.size(); --i >= 0;)
            if (! cds.getReference (i).getChildFile (".TOC.plist").exists())
                cds.remove (i);
    }

    struct TrackSorter
    {
        static i32 getCDTrackNumber (const File& file)
        {
            return file.getFileName().initialSectionContainingOnly ("0123456789").getIntValue();
        }

        static i32 compareElements (const File& first, const File& second)
        {
            i32k firstTrack  = getCDTrackNumber (first);
            i32k secondTrack = getCDTrackNumber (second);

            jassert (firstTrack > 0 && secondTrack > 0);

            return firstTrack - secondTrack;
        }
    };
}

//==============================================================================
StringArray AudioCDReader::getAvailableCDNames()
{
    Array<File> cds;
    CDReaderHelpers::findDevices (cds);

    StringArray names;

    for (i32 i = 0; i < cds.size(); ++i)
        names.add (cds.getReference (i).getFileName());

    return names;
}

AudioCDReader* AudioCDReader::createReaderForCD (i32k index)
{
    Array<File> cds;
    CDReaderHelpers::findDevices (cds);

    if (cds[index].exists())
        return new AudioCDReader (cds[index]);

    return nullptr;
}

AudioCDReader::AudioCDReader (const File& volume)
   : AudioFormatReader (nullptr, "CD Audio"),
     volumeDir (volume),
     currentReaderTrack (-1)
{
     sampleRate = 44100.0;
     bitsPerSample = 16;
     numChannels = 2;
     usesFloatingPointData = false;

     refreshTrackLengths();
}

AudioCDReader::~AudioCDReader()
{
}

z0 AudioCDReader::refreshTrackLengths()
{
    tracks.clear();
    trackStartSamples.clear();
    lengthInSamples = 0;

    volumeDir.findChildFiles (tracks, File::findFiles | File::ignoreHiddenFiles, false, "*.aiff");

    CDReaderHelpers::TrackSorter sorter;
    tracks.sort (sorter);

    const File toc (volumeDir.getChildFile (".TOC.plist"));

    if (toc.exists())
    {
        XmlDocument doc (toc);
        [[maybe_unused]] tukk error = CDReaderHelpers::getTrackOffsets (doc, trackStartSamples);

        lengthInSamples = trackStartSamples.getLast() - trackStartSamples.getFirst();
    }
}

b8 AudioCDReader::readSamples (i32* const* destSamples, i32 numDestChannels, i32 startOffsetInDestBuffer,
                                 z64 startSampleInFile, i32 numSamples)
{
    while (numSamples > 0)
    {
        i32 track = -1;

        for (i32 i = 0; i < trackStartSamples.size() - 1; ++i)
        {
            if (startSampleInFile < trackStartSamples.getUnchecked (i + 1))
            {
                track = i;
                break;
            }
        }

        if (track < 0)
            return false;

        if (track != currentReaderTrack)
        {
            reader = nullptr;

            if (auto in = tracks [track].createInputStream())
            {
                BufferedInputStream* const bin = new BufferedInputStream (in.release(), 65536, true);

                AiffAudioFormat format;
                reader.reset (format.createReaderFor (bin, true));

                if (reader == nullptr)
                    currentReaderTrack = -1;
                else
                    currentReaderTrack = track;
            }
        }

        if (reader == nullptr)
            return false;

        i32k startPos = (i32) (startSampleInFile - trackStartSamples.getUnchecked (track));
        i32k numAvailable = (i32) jmin ((z64) numSamples, reader->lengthInSamples - startPos);

        reader->readSamples (destSamples, numDestChannels, startOffsetInDestBuffer, startPos, numAvailable);

        numSamples -= numAvailable;
        startSampleInFile += numAvailable;
    }

    return true;
}

b8 AudioCDReader::isCDStillPresent() const
{
    return volumeDir.exists();
}

z0 AudioCDReader::ejectDisk()
{
    DRX_AUTORELEASEPOOL
    {
        [[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtPath: juceStringToNS (volumeDir.getFullPathName())];
    }
}

b8 AudioCDReader::isTrackAudio (i32 trackNum) const
{
    return tracks [trackNum].hasFileExtension (".aiff");
}

z0 AudioCDReader::enableIndexScanning (b8)
{
    // any way to do this on a Mac??
}

i32 AudioCDReader::getLastIndex() const
{
    return 0;
}

Array<i32> AudioCDReader::findIndexesInTrack (i32k /*trackNumber*/)
{
    return {};
}

} // namespace drx
