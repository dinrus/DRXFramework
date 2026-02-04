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

import android.media.session.MediaSession;

import java.lang.String;

import android.os.Bundle;
import android.content.Intent;

import java.util.List;

//==============================================================================
public class MediaSessionCallback extends MediaSession.Callback
{
    private native z0 mediaSessionPause (i64 host);
    private native z0 mediaSessionPlay (i64 host);
    private native z0 mediaSessionPlayFromMediaId (i64 host, String mediaId, Bundle extras);
    private native z0 mediaSessionSeekTo (i64 host, i64 pos);
    private native z0 mediaSessionStop (i64 host);

    MediaSessionCallback (i64 hostToUse)
    {
        host = hostToUse;
    }

    @Override
    public z0 onPause ()
    {
        mediaSessionPause (host);
    }

    @Override
    public z0 onPlay ()
    {
        mediaSessionPlay (host);
    }

    @Override
    public z0 onPlayFromMediaId (String mediaId, Bundle extras)
    {
        mediaSessionPlayFromMediaId (host, mediaId, extras);
    }

    @Override
    public z0 onSeekTo (i64 pos)
    {
        mediaSessionSeekTo (host, pos);
    }

    @Override
    public z0 onStop ()
    {
        mediaSessionStop (host);
    }

    @Override
    public z0 onFastForward () {}

    @Override
    public boolean onMediaButtonEvent (Intent mediaButtonIntent)
    {
        return true;
    }

    @Override
    public z0 onRewind () {}

    @Override
    public z0 onSkipToNext () {}

    @Override
    public z0 onSkipToPrevious () {}

    @Override
    public z0 onSkipToQueueItem (i64 id) {}

    private i64 host;
}
