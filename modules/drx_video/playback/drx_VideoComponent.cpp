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

#if ! (DRX_LINUX || DRX_BSD)

#if DRX_MAC || DRX_IOS
 #include "../native/drx_Video_mac.h"
#elif DRX_WINDOWS
 #include "../native/drx_Video_windows.h"
#elif DRX_ANDROID
 #include "../native/drx_Video_android.h"
#endif

//==============================================================================
VideoComponent::VideoComponent (b8 useNativeControlsIfAvailable)
    : pimpl (new Pimpl (*this, useNativeControlsIfAvailable))
{
    addAndMakeVisible (pimpl.get());
}

VideoComponent::~VideoComponent()
{
    pimpl.reset();
}

Result VideoComponent::load (const File& file)
{
    return loadInternal (file, false);
}

Result VideoComponent::load (const URL& url)
{
    return loadInternal (url, false);
}

z0 VideoComponent::loadAsync (const URL& url, std::function<z0 (const URL&, Result)> callback)
{
    if (callback == nullptr)
    {
        jassertfalse;
        return;
    }

   #if DRX_ANDROID || DRX_IOS || DRX_MAC
    pimpl->loadAsync (url, callback);
   #else
    auto result = loadInternal (url, true);
    callback (url, result);
   #endif
}

z0 VideoComponent::closeVideo()
{
    pimpl->close();
    // Closing on Android is async and resized() will be called internally by pimpl once
    // close operation finished.
   #if ! DRX_ANDROID// TODO DRX_IOS too?
    resized();
   #endif
}

b8 VideoComponent::isVideoOpen() const                    { return pimpl->isOpen(); }

File VideoComponent::getCurrentVideoFile() const            { return pimpl->currentFile; }
URL VideoComponent::getCurrentVideoURL() const              { return pimpl->currentURL; }

f64 VideoComponent::getVideoDuration() const             { return pimpl->getDuration(); }
Rectangle<i32> VideoComponent::getVideoNativeSize() const   { return pimpl->getNativeSize(); }

z0 VideoComponent::play()                                 { pimpl->play(); }
z0 VideoComponent::stop()                                 { pimpl->stop(); }

b8 VideoComponent::isPlaying() const                      { return pimpl->isPlaying(); }

z0 VideoComponent::setPlayPosition (f64 newPos)        { pimpl->setPosition (newPos); }
f64 VideoComponent::getPlayPosition() const              { return pimpl->getPosition(); }

z0 VideoComponent::setPlaySpeed (f64 newSpeed)         { pimpl->setSpeed (newSpeed); }
f64 VideoComponent::getPlaySpeed() const                 { return pimpl->getSpeed(); }

z0 VideoComponent::setAudioVolume (f32 newVolume)       { pimpl->setVolume (newVolume); }
f32 VideoComponent::getAudioVolume() const                { return pimpl->getVolume(); }

z0 VideoComponent::resized()
{
    auto r = getLocalBounds();

    if (isVideoOpen() && ! r.isEmpty())
    {
        auto nativeSize = getVideoNativeSize();

        if (nativeSize.isEmpty())
        {
            // if we've just opened the file and are still waiting for it to
            // figure out the size, start our timer..
            if (! isTimerRunning())
                startTimer (50);
        }
        else
        {
            r = RectanglePlacement (RectanglePlacement::centred).appliedTo (nativeSize, r);
            stopTimer();
        }
    }
    else
    {
        stopTimer();
    }

    pimpl->setBounds (r);
}

z0 VideoComponent::timerCallback()
{
    resized();
}

template <class FileOrURL>
Result VideoComponent::loadInternal (const FileOrURL& fileOrUrl, b8 loadAsync)
{
   #if DRX_ANDROID || DRX_IOS
    ignoreUnused (fileOrUrl, loadAsync);
    // You need to use loadAsync on Android & iOS.
    jassertfalse;
    return Result::fail ("load() не поддерживается on this platform. Use loadAsync() instead.");
   #else
    auto result = pimpl->load (fileOrUrl);

    if (loadAsync)
        startTimer (50);
    else
        resized();

    return result;
   #endif
}

#endif

} // namespace drx
