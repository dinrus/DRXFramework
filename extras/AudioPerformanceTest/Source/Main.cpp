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
#include "MainComponent.h"

//==============================================================================
class AudioPerformanceTestApplication final : public DRXApplication
{
public:
    //==============================================================================
    AudioPerformanceTestApplication() {}

    const Txt getApplicationName() override       { return ProjectInfo::projectName; }
    const Txt getApplicationVersion() override    { return ProjectInfo::versionString; }
    b8 moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    z0 initialise (const Txt&) override
    {
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    z0 shutdown() override
    {
        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    z0 systemRequestedQuit() override
    {
        quit();
    }

    //==============================================================================
    class MainWindow final : public DocumentWindow
    {
    public:
        explicit MainWindow (Txt name)
            : DocumentWindow (name, Colors::lightgrey, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainContentComponent(), true);
            setResizable (false, false);

           #if DRX_IOS || DRX_ANDROID
            setFullScreen (true);
           #else
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        z0 closeButtonPressed() override
        {
            DRXApplication::getInstance()->systemRequestedQuit();
        }

    private:
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_DRX_APPLICATION (AudioPerformanceTestApplication)
