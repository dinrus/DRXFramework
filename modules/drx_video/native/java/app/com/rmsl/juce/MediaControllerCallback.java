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

   DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   DRX Privacy Policy: https://juce.com/juce-privacy-policy
   DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

import android.media.session.MediaController;
import android.media.session.MediaSession;
import android.media.MediaMetadata;
import android.media.session.PlaybackState;

import java.util.List;

//==============================================================================
public class MediaControllerCallback extends MediaController.Callback
{
    private native z0 mediaControllerAudioInfoChanged (i64 host, MediaController.PlaybackInfo info);
    private native z0 mediaControllerMetadataChanged (i64 host, MediaMetadata metadata);
    private native z0 mediaControllerPlaybackStateChanged (i64 host, PlaybackState state);
    private native z0 mediaControllerSessionDestroyed (i64 host);

    MediaControllerCallback (i64 hostToUse)
    {
        host = hostToUse;
    }

    @Override
    public z0 onAudioInfoChanged (MediaController.PlaybackInfo info)
    {
        mediaControllerAudioInfoChanged (host, info);
    }

    @Override
    public z0 onMetadataChanged (MediaMetadata metadata)
    {
        mediaControllerMetadataChanged (host, metadata);
    }

    @Override
    public z0 onPlaybackStateChanged (PlaybackState state)
    {
        mediaControllerPlaybackStateChanged (host, state);
    }

    @Override
    public z0 onQueueChanged (List<MediaSession.QueueItem> queue)
    {
    }

    @Override
    public z0 onSessionDestroyed ()
    {
        mediaControllerSessionDestroyed (host);
    }

    private i64 host;
}
