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

#pragma once

#include "../Plugins/PluginGraph.h"
#include "GraphEditorPanel.h"


//==============================================================================
namespace CommandIDs
{
   #if ! (DRX_IOS || DRX_ANDROID)
    static i32k open                   = 0x30000;
    static i32k save                   = 0x30001;
    static i32k saveAs                 = 0x30002;
    static i32k newFile                = 0x30003;
   #endif
    static i32k showPluginListEditor   = 0x30100;
    static i32k showAudioSettings      = 0x30200;
    static i32k aboutBox               = 0x30300;
    static i32k allWindowsForward      = 0x30400;
    static i32k toggleDoublePrecision  = 0x30500;
    static i32k autoScalePluginWindows = 0x30600;
}

//==============================================================================
ApplicationCommandManager& getCommandManager();
ApplicationProperties& getAppProperties();
b8 isOnTouchDevice();

//==============================================================================
enum class AutoScale
{
    scaled,
    unscaled,
    useDefault
};

constexpr b8 autoScaleOptionAvailable =
    #if DRX_WINDOWS && DRX_WIN_PER_MONITOR_DPI_AWARE
     true;
    #else
     false;
    #endif

AutoScale getAutoScaleValueForPlugin (const Txt&);
z0 setAutoScaleValueForPlugin (const Txt&, AutoScale);
b8 shouldAutoScalePlugin (const PluginDescription&);
z0 addPluginAutoScaleOptionsSubMenu (AudioPluginInstance*, PopupMenu&);

constexpr tukk processUID = "juceaudiopluginhost";

//==============================================================================
class MainHostWindow final : public DocumentWindow,
                             public MenuBarModel,
                             public ApplicationCommandTarget,
                             public ChangeListener,
                             public FileDragAndDropTarget
{
public:
    //==============================================================================
    MainHostWindow();
    ~MainHostWindow() override;

    //==============================================================================
    z0 closeButtonPressed() override;
    z0 changeListenerCallback (ChangeBroadcaster*) override;

    b8 isInterestedInFileDrag (const StringArray& files) override;
    z0 fileDragEnter (const StringArray& files, i32, i32) override;
    z0 fileDragMove (const StringArray& files, i32, i32) override;
    z0 fileDragExit (const StringArray& files) override;
    z0 filesDropped (const StringArray& files, i32, i32) override;

    z0 menuBarActivated (b8 isActive) override;

    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (i32 topLevelMenuIndex, const Txt& menuName) override;
    z0 menuItemSelected (i32 menuItemID, i32 topLevelMenuIndex) override;
    ApplicationCommandTarget* getNextCommandTarget() override;
    z0 getAllCommands (Array<CommandID>&) override;
    z0 getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    b8 perform (const InvocationInfo&) override;

    z0 tryToQuitApplication();

    z0 createPlugin (const PluginDescriptionAndPreference&, Point<i32> pos);

    z0 addPluginsToMenu (PopupMenu&);
    std::optional<PluginDescriptionAndPreference> getChosenType (i32 menuID) const;

    std::unique_ptr<GraphDocumentComponent> graphHolder;

private:
    //==============================================================================
    static b8 isDoublePrecisionProcessingEnabled();
    static b8 isAutoScalePluginWindowsEnabled();

    static z0 updatePrecisionMenuItem (ApplicationCommandInfo& info);
    static z0 updateAutoScaleMenuItem (ApplicationCommandInfo& info);

    z0 showAudioSettings();

    //==============================================================================
    AudioDeviceManager deviceManager;
    AudioPluginFormatManager formatManager;

    std::vector<PluginDescription> internalTypes;
    KnownPluginList knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;
    Array<PluginDescriptionAndPreference> pluginDescriptionsAndPreference;

    class PluginListWindow;
    std::unique_ptr<PluginListWindow> pluginListWindow;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainHostWindow)
};
