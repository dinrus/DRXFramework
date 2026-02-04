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

#include <DrxHeader.h>

namespace
{
    Txt getBroadcastIPAddress()
    {
        return IPAddress::getLocalAddress().toString().upToLastOccurrenceOf (".", false, false) + ".255";
    }

    static i32k masterPortNumber = 9001;  // the UDP port the master sends on / the clients receive.
    static i32k clientPortNumber = 9002;  // the UDP port the clients send on / the master receives.

    static const Txt canvasStateOSCAddress = "/drx/nfd/canvasState";
    static const Txt newClientOSCAddress   = "/drx/nfd/newClient";
    static const Txt userInputOSCAddress   = "/drx/nfd/userInput";
}

#include "SharedCanvas.h"
#include "ClientComponent.h"
#include "Demos.h"
#include "MasterComponent.h"


//==============================================================================
class NetworkGraphicsDemoApplication final : public DRXApplication
{
public:
    NetworkGraphicsDemoApplication()  : properties (getPropertyFileOptions())
    {}

    const Txt getApplicationName() override           { return ProjectInfo::projectName; }
    const Txt getApplicationVersion() override        { return ProjectInfo::versionString; }
    b8 moreThanOneInstanceAllowed() override           { return true; }
    z0 anotherInstanceStarted (const Txt&) override {}

    //==============================================================================
    z0 initialise (const Txt& commandLine) override
    {
       #if ! DRX_IOS && ! DRX_ANDROID
        // Run as the master if we have a command-line flag "master" or if the exe itself
        // has been renamed to include the word "master"..
        b8 isMaster = commandLine.containsIgnoreCase ("master")
                          || File::getSpecialLocation (File::currentApplicationFile)
                                .getFileName().containsIgnoreCase ("master");

        if (isMaster)
            mainWindows.add (new MainWindow (properties));
       #endif

        mainWindows.add (new MainWindow (properties, 0));

        Desktop::getInstance().setScreenSaverEnabled (false);
    }

    z0 shutdown() override
    {
        mainWindows.clear();
        properties.saveIfNeeded();
    }

    z0 systemRequestedQuit() override
    {
        quit();
    }

    //==============================================================================
    struct MainWindow final : public DocumentWindow
    {
        explicit MainWindow (PropertiesFile& props)
            : DocumentWindow ("DRX Networked Graphics Demo - Master", Colors::white, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MasterContentComponent (props), true);
            setBounds (100, 50, getWidth(), getHeight());
            setResizable (true, false);
            setVisible (true);

            glContext.attachTo (*this);
        }

        MainWindow (PropertiesFile& props, i32 windowIndex)
            : DocumentWindow ("DRX Networked Graphics Demo", Colors::black, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new ClientCanvasComponent (props, windowIndex), true);
            setBounds (500, 100, getWidth(), getHeight());
            setResizable (true, false);
            setVisible (true);

           #if ! DRX_IOS
            glContext.attachTo (*this);
           #endif

           #if DRX_IOS || DRX_ANDROID
            setFullScreen (true);
           #endif
        }

        ~MainWindow() override
        {
            glContext.detach();
        }

        z0 closeButtonPressed() override
        {
            DRXApplication::getInstance()->systemRequestedQuit();
        }

        OpenGLContext glContext;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    static PropertiesFile::Options getPropertyFileOptions()
    {
        PropertiesFile::Options o;
        o.applicationName = "DRX Network Graphics Demo";
        o.filenameSuffix = ".settings";
        o.folderName = "DRX Network Graphics Demo";
        o.osxLibrarySubFolder = "Application Support/DRX Network Graphics Demo";
        o.millisecondsBeforeSaving = 2000;
        return o;
    }

    PropertiesFile properties;
    OwnedArray<MainWindow> mainWindows;
};


//==============================================================================
// This macro generates the main() routine that launches the app.
START_DRX_APPLICATION (NetworkGraphicsDemoApplication)
