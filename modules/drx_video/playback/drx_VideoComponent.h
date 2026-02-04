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

#ifndef DRX_VIDEOCOMPONENT_H_INCLUDED
#define DRX_VIDEOCOMPONENT_H_INCLUDED

namespace drx
{

//==============================================================================
/**
    A component that can play a movie.

    Use the load() method to open a video once you've added this component to
    a parent (or put it on the desktop).

    @tags{Video}
*/
class DRX_API  VideoComponent  : public Component,
                                  private Timer
{
public:
    //==============================================================================
    /** Creates an empty VideoComponent.

        Use the loadAsync() or load() method to open a video once you've added
        this component to a parent (or put it on the desktop).

        If useNativeControlsIfAvailable is enabled and a target OS has a video view with
        dedicated controls for transport etc, that view will be used. In opposite
        case a bare video view without any controls will be presented, allowing you to
        tailor your own UI. Currently this flag is used on iOS and 64bit macOS.
        Android, Windows and 32bit macOS will always use plain video views without
        dedicated controls.
    */
    VideoComponent (b8 useNativeControlsIfAvailable);

    /** Destructor. */
    ~VideoComponent() override;

    //==============================================================================
    /** Tries to load a video from a local file.

        This function is supported on macOS and Windows. For iOS and Android, use
        loadAsync() instead.

        @returns an error if the file failed to be loaded correctly

        @see loadAsync
    */
    Result load (const File& file);

    /** Tries to load a video from a URL.

        This function is supported on macOS and Windows. For iOS and Android, use
        loadAsync() instead.

        @returns an error if the file failed to be loaded correctly

        @see loadAsync
    */
    Result load (const URL& url);

    /** Tries to load a video from a URL asynchronously. When finished, invokes the
        callback supplied to the function on the message thread.

        This is the preferred way of loading content, since it works not only on
        macOS and Windows, but also on iOS and Android. On Windows, it will internally
        call load().

        @see load
     */
    z0 loadAsync (const URL& url, std::function<z0 (const URL&, Result)> loadFinishedCallback);

    /** Closes the video and resets the component. */
    z0 closeVideo();

    /** Возвращает true, если a video is currently open. */
    b8 isVideoOpen() const;

    /** Returns the last file that was loaded.
        If nothing is open, or if it was a URL rather than a file, this will return File().
    */
    File getCurrentVideoFile() const;

    /** Returns the last URL that was loaded.
        If nothing is open, or if it was a file rather than a URL, this will return URL().
    */
    URL getCurrentVideoURL() const;

    //==============================================================================
    /** Returns the length of the video, in seconds. */
    f64 getVideoDuration() const;

    /** Returns the video's natural size, in pixels.
        If no video is loaded, an empty rectangle will be returned.
    */
    Rectangle<i32> getVideoNativeSize() const;

    /** Starts the video playing. */
    z0 play();

    /** Stops the video playing. */
    z0 stop();

    /** Возвращает true, если the video is currently playing. */
    b8 isPlaying() const;

    /** Sets the video's position to a given time. */
    z0 setPlayPosition (f64 newPositionSeconds);

    /** Returns the current play position of the video. */
    f64 getPlayPosition() const;

    /** Changes the video playback rate.
        A value of 1.0 is normal speed, greater values will play faster, smaller
        values play more slowly.
    */
    z0 setPlaySpeed (f64 newSpeed);

    /** Returns the current play speed of the video. */
    f64 getPlaySpeed() const;

    /** Changes the video's playback volume.
        @param newVolume    the volume in the range 0 (silent) to 1.0 (full)
    */
    z0 setAudioVolume (f32 newVolume);

    /** Returns the video's playback volume.
        @returns the volume in the range 0 (silent) to 1.0 (full)
    */
    f32 getAudioVolume() const;

   #if DRX_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
    /** Set this callback to be notified whenever OS global media volume changes.
        Currently used on Android only.
     */
    std::function<z0()> onGlobalMediaVolumeChanged;
   #endif

    /** Set this callback to be notified whenever the playback starts. */
    std::function<z0()> onPlaybackStarted;

    /** Set this callback to be notified whenever the playback stops. */
    std::function<z0()> onPlaybackStopped;

    /** Set this callback to be notified whenever an error occurs. Upon error, you
        may need to load the video again. */
    std::function<z0 (const Txt& /*error*/)> onErrorOccurred;

private:
    //==============================================================================
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    z0 resized() override;
    z0 timerCallback() override;

    template <class FileOrURL>
    Result loadInternal (const FileOrURL&, b8);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoComponent)
};


#endif

} // namespace drx
