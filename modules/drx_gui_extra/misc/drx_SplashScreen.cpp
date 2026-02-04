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

SplashScreen::SplashScreen (const Txt& title, const Image& image, b8 useDropShadow)
    : Component (title),
      backgroundImage (image),
      clickCountToDelete (0)
{
    // You must supply a valid image here!
    jassert (backgroundImage.isValid());

    setOpaque (! backgroundImage.hasAlphaChannel());

   #if DRX_IOS || DRX_ANDROID
    const b8 useFullScreen = true;
   #else
    const b8 useFullScreen = false;
   #endif

    makeVisible (image.getWidth(), image.getHeight(), useDropShadow, useFullScreen);
}

SplashScreen::SplashScreen (const Txt& title, i32 width, i32 height, b8 useDropShadow)
    : Component (title),
      clickCountToDelete (0)
{
    makeVisible (width, height, useDropShadow, false);
}

z0 SplashScreen::makeVisible (i32 w, i32 h, b8 useDropShadow, b8 fullscreen)
{
    clickCountToDelete = Desktop::getInstance().getMouseButtonClickCounter();
    creationTime = Time::getCurrentTime();

    const Rectangle<i32> screenSize = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
    i32k width  = (fullscreen ? screenSize.getWidth()   : w);
    i32k height = (fullscreen ? screenSize.getHeight()  : h);

    setAlwaysOnTop (true);
    setVisible (true);
    centreWithSize (width, height);
    addToDesktop (useDropShadow ? ComponentPeer::windowHasDropShadow : 0);

    if (fullscreen)
        getPeer()->setFullScreen (true);

    toFront (false);
}

SplashScreen::~SplashScreen() {}

z0 SplashScreen::deleteAfterDelay (RelativeTime timeout, b8 removeOnMouseClick)
{
    // Note that this method must be safe to call from non-GUI threads
    if (! removeOnMouseClick)
        clickCountToDelete = std::numeric_limits<i32>::max();

    minimumVisibleTime = timeout;

    startTimer (50);
}

z0 SplashScreen::paint (Graphics& g)
{
    g.setOpacity (1.0f);
    g.drawImage (backgroundImage, getLocalBounds().toFloat(), RectanglePlacement (RectanglePlacement::fillDestination));
}

z0 SplashScreen::timerCallback()
{
    if (Time::getCurrentTime() > creationTime + minimumVisibleTime
         || Desktop::getInstance().getMouseButtonClickCounter() > clickCountToDelete)
        delete this;
}

} // namespace drx
