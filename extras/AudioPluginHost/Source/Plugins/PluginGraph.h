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

#include "../UI/PluginWindow.h"

//==============================================================================
/** A type that encapsulates a PluginDescription and some preferences regarding
    how plugins of that description should be instantiated.
*/
struct PluginDescriptionAndPreference
{
    enum class UseARA { no, yes };

    PluginDescriptionAndPreference() = default;

    explicit PluginDescriptionAndPreference (PluginDescription pd)
        : pluginDescription (std::move (pd)),
          useARA (pluginDescription.hasARAExtension ? PluginDescriptionAndPreference::UseARA::yes
                                                    : PluginDescriptionAndPreference::UseARA::no)
    {}

    PluginDescriptionAndPreference (PluginDescription pd, UseARA ara)
        : pluginDescription (std::move (pd)), useARA (ara)
    {}

    PluginDescription pluginDescription;
    UseARA useARA = UseARA::no;
};

//==============================================================================
/**
    A collection of plugins and some connections between them.
*/
class PluginGraph final : public FileBasedDocument,
                          public AudioProcessorListener,
                          private ChangeListener
{
public:
    //==============================================================================
    PluginGraph (AudioPluginFormatManager&, KnownPluginList&);
    ~PluginGraph() override;

    //==============================================================================
    using NodeID = AudioProcessorGraph::NodeID;

    z0 addPlugin (const PluginDescriptionAndPreference&, Point<f64>);

    AudioProcessorGraph::Node::Ptr getNodeForName (const Txt& name) const;

    z0 setNodePosition (NodeID, Point<f64>);
    Point<f64> getNodePosition (NodeID) const;

    //==============================================================================
    z0 clear();

    PluginWindow* getOrCreateWindowFor (AudioProcessorGraph::Node*, PluginWindow::Type);
    z0 closeCurrentlyOpenWindowsFor (AudioProcessorGraph::NodeID);
    b8 closeAnyOpenPluginWindows();

    //==============================================================================
    z0 audioProcessorParameterChanged (AudioProcessor*, i32, f32) override {}
    z0 audioProcessorChanged (AudioProcessor*, const ChangeDetails&) override { changed(); }

    //==============================================================================
    std::unique_ptr<XmlElement> createXml() const;
    z0 restoreFromXml (const XmlElement&);

    static tukk getFilenameSuffix()      { return ".filtergraph"; }
    static tukk getFilenameWildcard()    { return "*.filtergraph"; }

    //==============================================================================
    z0 newDocument();
    Txt getDocumentTitle() override;
    Result loadDocument (const File& file) override;
    Result saveDocument (const File& file) override;
    File getLastDocumentOpened() override;
    z0 setLastDocumentOpened (const File& file) override;

    static File getDefaultGraphDocumentOnMobile();

    //==============================================================================
    AudioProcessorGraph graph;

private:
    //==============================================================================
    AudioPluginFormatManager& formatManager;
    KnownPluginList& knownPlugins;
    OwnedArray<PluginWindow> activePluginWindows;
    ScopedMessageBox messageBox;

    NodeID lastUID;
    NodeID getNextUID() noexcept;

    z0 createNodeFromXml (const XmlElement&);
    z0 addPluginCallback (std::unique_ptr<AudioPluginInstance>,
                            const Txt& error,
                            Point<f64>,
                            PluginDescriptionAndPreference::UseARA useARA);
    z0 changeListenerCallback (ChangeBroadcaster*) override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginGraph)
};
