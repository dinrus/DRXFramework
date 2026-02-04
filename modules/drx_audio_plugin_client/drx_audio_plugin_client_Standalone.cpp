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

#include <drx_core/system/drx_TargetPlatform.h>

#if DrxPlugin_Build_Standalone

#if ! DRX_MODULE_AVAILABLE_drx_audio_utils
 #error To compile AudioUnitv3 and/or Standalone plug-ins, you need to add the drx_audio_utils and drx_audio_devices modules!
#endif

#include <drx_core/system/drx_TargetPlatform.h>
#include <drx_audio_plugin_client/detail/drx_CheckSettingMacros.h>

#include <drx_audio_plugin_client/detail/drx_IncludeSystemHeaders.h>
#include <drx_audio_plugin_client/detail/drx_IncludeModuleHeaders.h>
#include <drx_gui_basics/native/drx_WindowsHooks_windows.h>
#include <drx_audio_plugin_client/detail/drx_PluginUtilities.h>

#include <drx_audio_devices/drx_audio_devices.h>
#include <drx_gui_extra/drx_gui_extra.h>
#include <drx_audio_utils/drx_audio_utils.h>

// You can set this flag in your build if you need to specify a different
// standalone DRXApplication class for your app to use. If you don't
// set it then by default we'll just create a simple one as below.
#if ! DRX_USE_CUSTOM_PLUGIN_STANDALONE_APP

#include <drx_audio_plugin_client/Standalone/drx_StandaloneFilterWindow.h>

namespace drx
{

//==============================================================================
class StandaloneFilterApp final : public DRXApplication
{
public:
    StandaloneFilterApp()
    {
        PropertiesFile::Options options;

        options.applicationName     = CharPointer_UTF8 (DrxPlugin_Name);
        options.filenameSuffix      = ".settings";
        options.osxLibrarySubFolder = "Application Support";
       #if DRX_LINUX || DRX_BSD
        options.folderName          = "~/.config";
       #else
        options.folderName          = "";
       #endif

        appProperties.setStorageParameters (options);
    }

    const Txt getApplicationName() override              { return CharPointer_UTF8 (DrxPlugin_Name); }
    const Txt getApplicationVersion() override           { return DrxPlugin_VersionString; }
    b8 moreThanOneInstanceAllowed() override              { return true; }
    z0 anotherInstanceStarted (const Txt&) override    {}

    virtual StandaloneFilterWindow* createWindow()
    {
        if (Desktop::getInstance().getDisplays().displays.isEmpty())
        {
            // No displays are available, so no window will be created!
            jassertfalse;
            return nullptr;
        }

        return new StandaloneFilterWindow (getApplicationName(),
                                           LookAndFeel::getDefaultLookAndFeel().findColor (ResizableWindow::backgroundColorId),
                                           createPluginHolder());
    }

    virtual std::unique_ptr<StandalonePluginHolder> createPluginHolder()
    {
        constexpr auto autoOpenMidiDevices =
       #if (DRX_ANDROID || DRX_IOS) && ! DRX_DONT_AUTO_OPEN_MIDI_DEVICES_ON_MOBILE
                true;
       #else
                false;
       #endif


       #ifdef DrxPlugin_PreferredChannelConfigurations
        constexpr StandalonePluginHolder::PluginInOuts channels[] { DrxPlugin_PreferredChannelConfigurations };
        const Array<StandalonePluginHolder::PluginInOuts> channelConfig (channels, drx::numElementsInArray (channels));
       #else
        const Array<StandalonePluginHolder::PluginInOuts> channelConfig;
       #endif

        return std::make_unique<StandalonePluginHolder> (appProperties.getUserSettings(),
                                                         false,
                                                         Txt{},
                                                         nullptr,
                                                         channelConfig,
                                                         autoOpenMidiDevices);
    }

    //==============================================================================
    z0 initialise (const Txt&) override
    {
        mainWindow = rawToUniquePtr (createWindow());

        if (mainWindow != nullptr)
        {
           #if DRX_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE
            Desktop::getInstance().setKioskModeComponent (mainWindow.get(), false);
           #endif

            mainWindow->setVisible (true);
        }
        else
        {
            pluginHolder = createPluginHolder();
        }
    }

    z0 shutdown() override
    {
        pluginHolder = nullptr;
        mainWindow = nullptr;
        appProperties.saveIfNeeded();
    }

    //==============================================================================
    z0 systemRequestedQuit() override
    {
        if (pluginHolder != nullptr)
            pluginHolder->savePluginState();

        if (mainWindow != nullptr)
            mainWindow->pluginHolder->savePluginState();

        if (ModalComponentManager::getInstance()->cancelAllModalComponents())
        {
            Timer::callAfterDelay (100, []()
            {
                if (auto app = DRXApplicationBase::getInstance())
                    app->systemRequestedQuit();
            });
        }
        else
        {
            quit();
        }
    }

protected:
    ApplicationProperties appProperties;
    std::unique_ptr<StandaloneFilterWindow> mainWindow;

private:
    std::unique_ptr<StandalonePluginHolder> pluginHolder;
};

} // namespace drx

#if DrxPlugin_Build_Standalone && DRX_IOS

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-prototypes")

using namespace drx;

b8 DRX_CALLTYPE drx_isInterAppAudioConnected()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->isInterAppAudioConnected();

    return false;
}

z0 DRX_CALLTYPE drx_switchToHostApplication()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        holder->switchToHostApplication();
}

Image DRX_CALLTYPE drx_getIAAHostIcon (i32 size)
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->getIAAHostIcon (size);

    return Image();
}

DRX_END_IGNORE_WARNINGS_GCC_LIKE

#endif

#endif

#if DRX_USE_CUSTOM_PLUGIN_STANDALONE_APP
 extern drx::DRXApplicationBase* drx_CreateApplication();

 #if DRX_IOS
  extern uk drx_GetIOSCustomDelegateClass();
 #endif

#else
 DRX_CREATE_APPLICATION_DEFINE (drx::StandaloneFilterApp)
#endif

#if ! DRX_USE_CUSTOM_PLUGIN_STANDALONE_ENTRYPOINT
 DRX_MAIN_FUNCTION_DEFINITION
#endif

#endif
