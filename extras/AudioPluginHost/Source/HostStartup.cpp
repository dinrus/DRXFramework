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
#include "UI/MainHostWindow.h"
#include "Plugins/InternalPlugins.h"

#if ! (DRX_PLUGINHOST_VST || DRX_PLUGINHOST_VST3 || DRX_PLUGINHOST_AU)
 #error "If you're building the audio plugin host, you probably want to enable VST and/or AU support"
#endif

class PluginScannerSubprocess final : private ChildProcessWorker,
                                      private AsyncUpdater
{
public:
    PluginScannerSubprocess()
    {
        formatManager.addDefaultFormats();
    }

    using ChildProcessWorker::initialiseFromCommandLine;

private:
    z0 handleMessageFromCoordinator (const MemoryBlock& mb) override
    {
        if (mb.isEmpty())
            return;

        const std::lock_guard<std::mutex> lock (mutex);

        if (const auto results = doScan (mb); ! results.isEmpty())
        {
            sendResults (results);
        }
        else
        {
            pendingBlocks.emplace (mb);
            triggerAsyncUpdate();
        }
    }

    z0 handleConnectionLost() override
    {
        DRXApplicationBase::quit();
    }

    z0 handleAsyncUpdate() override
    {
        for (;;)
        {
            const std::lock_guard<std::mutex> lock (mutex);

            if (pendingBlocks.empty())
                return;

            sendResults (doScan (pendingBlocks.front()));
            pendingBlocks.pop();
        }
    }

    OwnedArray<PluginDescription> doScan (const MemoryBlock& block)
    {
        MemoryInputStream stream { block, false };
        const auto formatName = stream.readString();
        const auto identifier = stream.readString();

        PluginDescription pd;
        pd.fileOrIdentifier = identifier;
        pd.uniqueId = pd.deprecatedUid = 0;

        const auto matchingFormat = [&]() -> AudioPluginFormat*
        {
            for (auto* format : formatManager.getFormats())
                if (format->getName() == formatName)
                    return format;

            return nullptr;
        }();

        OwnedArray<PluginDescription> results;

        if (matchingFormat != nullptr
            && (MessageManager::getInstance()->isThisTheMessageThread()
                || matchingFormat->requiresUnblockedMessageThreadDuringCreation (pd)))
        {
            matchingFormat->findAllTypesForFile (results, identifier);
        }

        return results;
    }

    z0 sendResults (const OwnedArray<PluginDescription>& results)
    {
        XmlElement xml ("LIST");

        for (const auto& desc : results)
            xml.addChildElement (desc->createXml().release());

        const auto str = xml.toString();
        sendMessageToCoordinator ({ str.toRawUTF8(), str.getNumBytesAsUTF8() });
    }

    std::mutex mutex;
    std::queue<MemoryBlock> pendingBlocks;
    AudioPluginFormatManager formatManager;
};

//==============================================================================
class PluginHostApp final : public DRXApplication,
                            private AsyncUpdater
{
public:
    PluginHostApp() = default;

    z0 initialise (const Txt& commandLine) override
    {
        auto scannerSubprocess = std::make_unique<PluginScannerSubprocess>();

        if (scannerSubprocess->initialiseFromCommandLine (commandLine, processUID))
        {
            storedScannerSubprocess = std::move (scannerSubprocess);
            return;
        }

        // initialise our settings file..

        PropertiesFile::Options options;
        options.applicationName     = "Drx Audio Plugin Host";
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Preferences";

        appProperties.reset (new ApplicationProperties());
        appProperties->setStorageParameters (options);

        mainWindow.reset (new MainHostWindow());

        commandManager.registerAllCommandsForTarget (this);
        commandManager.registerAllCommandsForTarget (mainWindow.get());

        mainWindow->menuItemsChanged();

        // Important note! We're going to use an async update here so that if we need
        // to re-open a file and instantiate some plugins, it will happen AFTER this
        // initialisation method has returned.
        // On Windows this probably won't make a difference, but on OSX there's a subtle event loop
        // issue that can happen if a plugin runs one of those irritating modal dialogs while it's
        // being loaded. If that happens inside this method, the OSX event loop seems to be in some
        // kind of special "initialisation" mode and things get confused. But if we load the plugin
        // later when the normal event loop is running, everything's fine.
        triggerAsyncUpdate();
    }

    z0 handleAsyncUpdate() override
    {
        File fileToOpen;

       #if DRX_ANDROID || DRX_IOS
        fileToOpen = PluginGraph::getDefaultGraphDocumentOnMobile();
       #else
        for (i32 i = 0; i < getCommandLineParameterArray().size(); ++i)
        {
            fileToOpen = File::getCurrentWorkingDirectory().getChildFile (getCommandLineParameterArray()[i]);

            if (fileToOpen.existsAsFile())
                break;
        }
       #endif

        if (! fileToOpen.existsAsFile())
        {
            RecentlyOpenedFilesList recentFiles;
            recentFiles.restoreFromString (getAppProperties().getUserSettings()->getValue ("recentFilterGraphFiles"));

            if (recentFiles.getNumFiles() > 0)
                fileToOpen = recentFiles.getFile (0);
        }

        if (fileToOpen.existsAsFile())
            if (auto* graph = mainWindow->graphHolder.get())
                if (auto* ioGraph = graph->graph.get())
                    ioGraph->loadFrom (fileToOpen, true);
    }

    z0 shutdown() override
    {
        mainWindow = nullptr;
        appProperties = nullptr;
        LookAndFeel::setDefaultLookAndFeel (nullptr);
    }

    z0 suspended() override
    {
       #if DRX_ANDROID || DRX_IOS
        if (auto graph = mainWindow->graphHolder.get())
            if (auto ioGraph = graph->graph.get())
                ioGraph->saveDocument (PluginGraph::getDefaultGraphDocumentOnMobile());
       #endif
    }

    z0 systemRequestedQuit() override
    {
        if (mainWindow != nullptr)
            mainWindow->tryToQuitApplication();
        else
            DRXApplicationBase::quit();
    }

    b8 backButtonPressed() override
    {
        if (mainWindow->graphHolder != nullptr)
            mainWindow->graphHolder->hideLastSidePanel();

        return true;
    }

    const Txt getApplicationName() override       { return "Drx Plug-In Host"; }
    const Txt getApplicationVersion() override    { return ProjectInfo::versionString; }
    b8 moreThanOneInstanceAllowed() override       { return true; }

    ApplicationCommandManager commandManager;
    std::unique_ptr<ApplicationProperties> appProperties;

private:
    std::unique_ptr<MainHostWindow> mainWindow;
    std::unique_ptr<PluginScannerSubprocess> storedScannerSubprocess;
};

static PluginHostApp& getApp()                    { return *dynamic_cast<PluginHostApp*> (DRXApplication::getInstance()); }

ApplicationProperties& getAppProperties()         { return *getApp().appProperties; }
ApplicationCommandManager& getCommandManager()    { return getApp().commandManager; }

b8 isOnTouchDevice()
{
    static b8 isTouch = Desktop::getInstance().getMainMouseSource().isTouch();
    return isTouch;
}

//==============================================================================
static AutoScale autoScaleFromString (StringRef str)
{
    if (str.isEmpty())                     return AutoScale::useDefault;
    if (str == CharPointer_ASCII { "0" })  return AutoScale::scaled;
    if (str == CharPointer_ASCII { "1" })  return AutoScale::unscaled;

    jassertfalse;
    return AutoScale::useDefault;
}

static tukk autoScaleToString (AutoScale autoScale)
{
    if (autoScale == AutoScale::scaled)    return "0";
    if (autoScale == AutoScale::unscaled)  return "1";

    return {};
}

AutoScale getAutoScaleValueForPlugin (const Txt& identifier)
{
    if (identifier.isNotEmpty())
    {
        auto plugins = StringArray::fromLines (getAppProperties().getUserSettings()->getValue ("autoScalePlugins"));
        plugins.removeEmptyStrings();

        for (auto& plugin : plugins)
        {
            auto fromIdentifier = plugin.fromFirstOccurrenceOf (identifier, false, false);

            if (fromIdentifier.isNotEmpty())
                return autoScaleFromString (fromIdentifier.fromFirstOccurrenceOf (":", false, false));
        }
    }

    return AutoScale::useDefault;
}

z0 setAutoScaleValueForPlugin (const Txt& identifier, AutoScale s)
{
    auto plugins = StringArray::fromLines (getAppProperties().getUserSettings()->getValue ("autoScalePlugins"));
    plugins.removeEmptyStrings();

    auto index = [identifier, plugins]
    {
        auto it = std::find_if (plugins.begin(), plugins.end(),
                                [&] (const Txt& str) { return str.startsWith (identifier); });

        return (i32) std::distance (plugins.begin(), it);
    }();

    if (s == AutoScale::useDefault && index != plugins.size())
    {
        plugins.remove (index);
    }
    else
    {
        auto str = identifier + ":" + autoScaleToString (s);

        if (index != plugins.size())
            plugins.getReference (index) = str;
        else
            plugins.add (str);
    }

    getAppProperties().getUserSettings()->setValue ("autoScalePlugins", plugins.joinIntoString ("\n"));
}

static b8 isAutoScaleAvailableForPlugin (const PluginDescription& description)
{
    return autoScaleOptionAvailable
          && (description.pluginFormatName.containsIgnoreCase ("VST")
              || description.pluginFormatName.containsIgnoreCase ("LV2"));
}

b8 shouldAutoScalePlugin (const PluginDescription& description)
{
    if (! isAutoScaleAvailableForPlugin (description))
        return false;

    const auto scaleValue = getAutoScaleValueForPlugin (description.fileOrIdentifier);

    return (scaleValue == AutoScale::scaled
              || (scaleValue == AutoScale::useDefault
                    && getAppProperties().getUserSettings()->getBoolValue ("autoScalePluginWindows")));
}

z0 addPluginAutoScaleOptionsSubMenu (AudioPluginInstance* pluginInstance,
                                       PopupMenu& menu)
{
    if (pluginInstance == nullptr)
        return;

    auto description = pluginInstance->getPluginDescription();

    if (! isAutoScaleAvailableForPlugin (description))
        return;

    auto identifier = description.fileOrIdentifier;

    PopupMenu autoScaleMenu;

    autoScaleMenu.addItem ("Default",
                           true,
                           getAutoScaleValueForPlugin (identifier) == AutoScale::useDefault,
                           [identifier] { setAutoScaleValueForPlugin (identifier, AutoScale::useDefault); });

    autoScaleMenu.addItem ("Enabled",
                           true,
                           getAutoScaleValueForPlugin (identifier) == AutoScale::scaled,
                           [identifier] { setAutoScaleValueForPlugin (identifier, AutoScale::scaled); });

    autoScaleMenu.addItem ("Disabled",
                           true,
                           getAutoScaleValueForPlugin (identifier) == AutoScale::unscaled,
                           [identifier] { setAutoScaleValueForPlugin (identifier, AutoScale::unscaled); });

    menu.addSubMenu ("Auto-scale window", autoScaleMenu);
}

// This kicks the whole thing off..
START_DRX_APPLICATION (PluginHostApp)
