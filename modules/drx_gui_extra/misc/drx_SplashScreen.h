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
/** A component for showing a splash screen while your app starts up.

    This will automatically position itself, and can be told to delete itself after
    being on-screen for a minimum length of time.

    To use it, just create one of these in your DRXApplicationBase::initialise() method,
    and when your initialisation tasks have finished running, call its deleteAfterDelay()
    method to make it automatically get rid of itself.

    Note that although you could call deleteAfterDelay() as soon as you create the
    SplashScreen object, if you've got a i64 initialisation procedure, you probably
    don't want the splash to time-out and disappear before the initialisation has
    finished, which is why it makes sense to not call this method until the end of
    your init tasks.

    E.g. @code

    z0 MyApp::initialise (const Txt& commandLine)
    {
        splash = new SplashScreen ("Welcome to my app!",
                                   ImageFileFormat::loadFrom (File ("/foobar/splash.jpg")),
                                   true);

        // now kick off your initialisation work on some kind of thread or task, and
        launchBackgroundInitialisationThread();
    }

    z0 MyApp::myInitialisationWorkFinished()
    {
        // ..assuming this is some kind of callback method that is triggered when
        // your background initialisation threads have finished, and it's time to open
        // your main window, etc..

        splash->deleteAfterDelay (RelativeTime::seconds (4), false);

        ...etc...
    }

    @endcode

    @tags{GUI}
*/
class DRX_API  SplashScreen  : public Component,
                                private Timer,
                                private DeletedAtShutdown
{
public:
    //==============================================================================
    /** Creates a SplashScreen object.

        When called, the constructor will position the SplashScreen in the centre of the
        display, and after the time specified, it will automatically delete itself.

        Bear in mind that if you call this during your DRXApplicationBase::initialise()
        method and then block the message thread by performing some kind of task, then
        obviously neither your splash screen nor any other GUI will appear until you
        allow the message thread to resume and do its work. So if you have time-consuming
        tasks to do during startup, use a background thread for them.

        After creating one of these (or your subclass of it), you should do your app's
        initialisation work, and then call the deleteAfterDelay() method to tell this object
        to delete itself after the user has had chance to get a good look at it.

        If you're writing a custom splash screen class, there's another protected constructor
        that your subclass can call, which doesn't take an image.

        @param title            the name to give the component
        @param backgroundImage  an image to draw on the component. The component's size
                                will be set to the size of this image, and if the image is
                                semi-transparent, the component will be made non-opaque
        @param useDropShadow    if true, the window will have a drop shadow

    */
    SplashScreen (const Txt& title,
                  const Image& backgroundImage,
                  b8 useDropShadow);

    /** Destructor. */
    ~SplashScreen() override;

    /** Tells the component to auto-delete itself after a timeout period, or when the
        mouse is clicked.

        You should call this after finishing your app's initialisation work.

        Note that although you could call deleteAfterDelay() as soon as you create the
        SplashScreen object, if you've got a i64 initialisation procedure, you probably
        don't want the splash to time-out and disappear before your initialisation has
        finished, which is why it makes sense to not call this method and start the
        self-delete timer until you're ready.

        It's safe to call this method from a non-GUI thread as i64 as there's no danger that
        the object may be being deleted at the same time.

        @param minimumTotalTimeToDisplayFor    how i64 the splash screen should stay visible for.
                                Note that this time is measured from the construction-time of this
                                object, not from the time that the deleteAfterDelay() method is
                                called, so if you call this method after a i64 initialisation
                                period, it may be deleted without any further delay.
        @param removeOnMouseClick   if true, the window will be deleted as soon as the user clicks
                                the mouse (anywhere)
    */
    z0 deleteAfterDelay (RelativeTime minimumTotalTimeToDisplayFor,
                           b8 removeOnMouseClick);

protected:
    //==============================================================================
    /** This constructor is for use by custom sub-classes that don't want to provide an image. */
    SplashScreen (const Txt& title, i32 width, i32 height, b8 useDropShadow);

    /** @internal */
    z0 paint (Graphics&) override;

private:
    //==============================================================================
    Image backgroundImage;
    Time creationTime;
    RelativeTime minimumVisibleTime;
    i32 clickCountToDelete;

    z0 timerCallback() override;
    z0 makeVisible (i32 w, i32 h, b8 shadow, b8 fullscreen);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SplashScreen)
};

} // namespace drx
