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
#include "../UI/GraphEditorPanel.h"
#include "InternalPlugins.h"
#include "../UI/MainHostWindow.h"
#include "IOConfigurationWindow.h"


//==============================================================================
struct NumberedBoxes final : public TableListBox,
                        private TableListBoxModel,
                        private Button::Listener
{
    struct Listener
    {
        virtual ~Listener() {}

        virtual z0 addColumn()    = 0;
        virtual z0 removeColumn() = 0;
        virtual z0 columnSelected (i32 columnId) = 0;
    };

    enum
    {
        plusButtonColumnId  = 128,
        minusButtonColumnId = 129
    };

    //==============================================================================
    NumberedBoxes (Listener& listenerToUse, b8 canCurrentlyAddColumn, b8 canCurrentlyRemoveColumn)
        : TableListBox ("NumberedBoxes", this),
          listener (listenerToUse),
          canAddColumn (canCurrentlyAddColumn),
          canRemoveColumn (canCurrentlyRemoveColumn)
    {
        auto& tableHeader = getHeader();

        for (i32 i = 0; i < 16; ++i)
            tableHeader.addColumn (Txt (i + 1), i + 1, 40);

        setHeaderHeight (0);
        setRowHeight (40);
        getHorizontalScrollBar().setAutoHide (false);
    }

    z0 setSelected (i32 columnId)
    {
        if (auto* button = dynamic_cast<TextButton*> (getCellComponent (columnId, 0)))
            button->setToggleState (true, NotificationType::dontSendNotification);
    }

    z0 setCanAddColumn (b8 canCurrentlyAdd)
    {
        if (canCurrentlyAdd != canAddColumn)
        {
            canAddColumn = canCurrentlyAdd;

            if (auto* button = dynamic_cast<TextButton*> (getCellComponent (plusButtonColumnId, 0)))
                button->setEnabled (true);
        }
    }

    z0 setCanRemoveColumn (b8 canCurrentlyRemove)
    {
        if (canCurrentlyRemove != canRemoveColumn)
        {
            canRemoveColumn = canCurrentlyRemove;

            if (auto* button = dynamic_cast<TextButton*> (getCellComponent (minusButtonColumnId, 0)))
                button->setEnabled (true);
        }
    }

private:
    //==============================================================================
    Listener& listener;
    b8 canAddColumn, canRemoveColumn;

    //==============================================================================
    i32 getNumRows() override                                             { return 1; }
    z0 paintCell (Graphics&, i32, i32, i32, i32, b8) override         {}
    z0 paintRowBackground (Graphics& g, i32, i32, i32, b8) override   { g.fillAll (Colors::grey); }

    Component* refreshComponentForCell (i32, i32 columnId, b8,
                                        Component* existingComponentToUpdate) override
    {
        auto* textButton = dynamic_cast<TextButton*> (existingComponentToUpdate);

        if (textButton == nullptr)
            textButton = new TextButton();

        textButton->setButtonText (getButtonName (columnId));
        textButton->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight |
                                       Button::ConnectedOnTop  | Button::ConnectedOnBottom);

        const b8 isPlusMinusButton = (columnId == plusButtonColumnId || columnId == minusButtonColumnId);

        if (isPlusMinusButton)
        {
            textButton->setEnabled (columnId == plusButtonColumnId ? canAddColumn : canRemoveColumn);
        }
        else
        {
            textButton->setRadioGroupId (1, NotificationType::dontSendNotification);
            textButton->setClickingTogglesState (true);

            auto busColor = Colors::green.withRotatedHue (static_cast<f32> (columnId) / 5.0f);
            textButton->setColor (TextButton::buttonColorId, busColor);
            textButton->setColor (TextButton::buttonOnColorId, busColor.withMultipliedBrightness (2.0f));
        }

        textButton->addListener (this);

        return textButton;
    }

    //==============================================================================
    Txt getButtonName (i32 columnId)
    {
        if (columnId == plusButtonColumnId)  return "+";
        if (columnId == minusButtonColumnId) return "-";

        return Txt (columnId);
    }

    z0 buttonClicked (Button* btn) override
    {
        auto text = btn->getButtonText();

        if (text == "+") listener.addColumn();
        if (text == "-") listener.removeColumn();
    }

    z0 buttonStateChanged (Button* btn) override
    {
        auto text = btn->getButtonText();

        if (text == "+" || text == "-")
            return;

        if (btn->getToggleState())
            listener.columnSelected (text.getIntValue());
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NumberedBoxes)
};

//==============================================================================
class IOConfigurationWindow::InputOutputConfig final : public Component,
                                                       private Button::Listener,
                                                       private NumberedBoxes::Listener
{
public:
    InputOutputConfig (IOConfigurationWindow& parent, b8 direction)
        : owner (parent),
          ioTitle ("ioLabel", direction ? "Input Configuration" : "Output Configuration"),
          ioBuses (*this, false, false),
          isInput (direction)
    {
        ioTitle.setFont (ioTitle.getFont().withStyle (Font::bold));
        nameLabel.setFont (nameLabel.getFont().withStyle (Font::bold));
        layoutLabel.setFont (layoutLabel.getFont().withStyle (Font::bold));
        enabledToggle.setClickingTogglesState (true);

        enabledToggle.addListener (this);

        addAndMakeVisible (layoutLabel);
        addAndMakeVisible (layouts);
        addAndMakeVisible (enabledToggle);
        addAndMakeVisible (ioTitle);
        addAndMakeVisible (nameLabel);
        addAndMakeVisible (name);
        addAndMakeVisible (ioBuses);

        updateBusButtons();
        updateBusLayout();
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
    }

    z0 resized() override
    {
        auto r = getLocalBounds().reduced (10);

        ioTitle.setBounds (r.removeFromTop (14));
        r.reduce (10, 0);
        r.removeFromTop (16);

        ioBuses.setBounds (r.removeFromTop (60));

        {
            auto label = r.removeFromTop (24);
            nameLabel.setBounds (label.removeFromLeft (100));
            enabledToggle.setBounds (label.removeFromRight (80));
            name.setBounds (label);
        }

        {
            auto label = r.removeFromTop (24);
            layoutLabel.setBounds (label.removeFromLeft (100));
            layouts.setBounds (label);
        }
    }

private:
    z0 updateBusButtons()
    {
        if (auto* plugin = owner.getAudioProcessor())
        {
            auto& header = ioBuses.getHeader();
            header.removeAllColumns();

            i32k n = plugin->getBusCount (isInput);

            for (i32 i = 0; i < n; ++i)
                header.addColumn ("", i + 1, 40);

            header.addColumn ("+", NumberedBoxes::plusButtonColumnId,  20);
            header.addColumn ("-", NumberedBoxes::minusButtonColumnId, 20);

            ioBuses.setCanAddColumn    (plugin->canAddBus    (isInput));
            ioBuses.setCanRemoveColumn (plugin->canRemoveBus (isInput));
        }

        ioBuses.setSelected (currentBus + 1);
    }

    z0 updateBusLayout()
    {
        if (auto* plugin = owner.getAudioProcessor())
        {
            if (auto* bus = plugin->getBus (isInput, currentBus))
            {
                name.setText (bus->getName(), NotificationType::dontSendNotification);

                // supported layouts have changed
                layouts.clear (dontSendNotification);
                auto* menu = layouts.getRootMenu();

                auto itemId = 1;
                auto selectedId = -1;

                for (auto i = 1; i <= AudioChannelSet::maxChannelsOfNamedLayout; ++i)
                {
                    for (const auto& set : AudioChannelSet::channelSetsWithNumberOfChannels (i))
                    {
                        if (bus->isLayoutSupported (set))
                        {
                            menu->addItem (PopupMenu::Item { set.getDescription() }
                                               .setAction ([this, set] { applyBusLayout (set); })
                                               .setID (itemId));
                        }

                        if (bus->getCurrentLayout() == set)
                            selectedId = itemId;

                        ++itemId;
                    }
                }

                layouts.setSelectedId (selectedId);

                const b8 canBeDisabled = bus->isNumberOfChannelsSupported (0);

                if (canBeDisabled != enabledToggle.isEnabled())
                    enabledToggle.setEnabled (canBeDisabled);

                enabledToggle.setToggleState (bus->isEnabled(), NotificationType::dontSendNotification);
            }
        }
    }

    //==============================================================================
    z0 applyBusLayout (const AudioChannelSet& set)
    {
        if (auto* p = owner.getAudioProcessor())
        {
            if (auto* bus = p->getBus (isInput, currentBus))
            {
                if (bus->setCurrentLayoutWithoutEnabling (set))
                {
                    if (auto* config = owner.getConfig (! isInput))
                        config->updateBusLayout();

                    owner.update();
                }
            }
        }
    }

    z0 buttonClicked (Button*) override {}

    z0 buttonStateChanged (Button* btn) override
    {
        if (btn == &enabledToggle && enabledToggle.isEnabled())
        {
            if (auto* p = owner.getAudioProcessor())
            {
                if (auto* bus = p->getBus (isInput, currentBus))
                {
                    if (bus->isEnabled() != enabledToggle.getToggleState())
                    {
                        b8 success = enabledToggle.getToggleState() ? bus->enable()
                                                                      : bus->setCurrentLayout (AudioChannelSet::disabled());

                        if (success)
                        {
                            updateBusLayout();

                            if (auto* config = owner.getConfig (! isInput))
                                config->updateBusLayout();

                            owner.update();
                        }
                        else
                        {
                            enabledToggle.setToggleState (! enabledToggle.getToggleState(),
                                                          NotificationType::dontSendNotification);
                        }
                    }
                }
            }
        }
    }

    //==============================================================================
    z0 addColumn() override
    {
        if (auto* p = owner.getAudioProcessor())
        {
            if (p->canAddBus (isInput))
            {
                if (p->addBus (isInput))
                {
                    updateBusButtons();
                    updateBusLayout();

                    if (auto* config = owner.getConfig (! isInput))
                    {
                        config->updateBusButtons();
                        config->updateBusLayout();
                    }
                }

                owner.update();
            }
        }
    }

    z0 removeColumn() override
    {
        if (auto* p = owner.getAudioProcessor())
        {
            if (p->getBusCount (isInput) > 1 && p->canRemoveBus (isInput))
            {
                if (p->removeBus (isInput))
                {
                    currentBus = jmin (p->getBusCount (isInput) - 1, currentBus);

                    updateBusButtons();
                    updateBusLayout();

                    if (auto* config = owner.getConfig (! isInput))
                    {
                        config->updateBusButtons();
                        config->updateBusLayout();
                    }

                    owner.update();
                }
            }
        }
    }

    z0 columnSelected (i32 columnId) override
    {
        i32k newBus = columnId - 1;

        if (currentBus != newBus)
        {
            currentBus = newBus;
            ioBuses.setSelected (currentBus + 1);
            updateBusLayout();
        }
    }

    //==============================================================================
    IOConfigurationWindow& owner;
    Label ioTitle, name;
    Label nameLabel { "nameLabel", "Bus Name:" };
    Label layoutLabel { "layoutLabel", "Channel Layout:" };
    ToggleButton enabledToggle { "Enabled" };
    ComboBox layouts;
    NumberedBoxes ioBuses;
    b8 isInput;
    i32 currentBus = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputOutputConfig)
};


IOConfigurationWindow::IOConfigurationWindow (AudioProcessor& p)
   : AudioProcessorEditor (&p),
     title ("title", p.getName())
{
    setOpaque (true);

    title.setFont (title.getFont().withStyle (Font::bold));
    addAndMakeVisible (title);

    {
        ScopedLock renderLock (p.getCallbackLock());
        p.suspendProcessing (true);
        p.releaseResources();
    }

    if (p.getBusCount (true)  > 0 || p.canAddBus (true))
    {
        inConfig.reset (new InputOutputConfig (*this, true));
        addAndMakeVisible (inConfig.get());
    }

    if (p.getBusCount (false) > 0 || p.canAddBus (false))
    {
        outConfig.reset (new InputOutputConfig (*this, false));
        addAndMakeVisible (outConfig.get());
    }

    currentLayout = p.getBusesLayout();
    setSize (400, (inConfig != nullptr && outConfig != nullptr ? 160 : 0) + 200);
}

IOConfigurationWindow::~IOConfigurationWindow()
{
    if (auto* graph = getGraph())
    {
        if (auto* p = getAudioProcessor())
        {
            ScopedLock renderLock (graph->getCallbackLock());

            graph->suspendProcessing (true);
            graph->releaseResources();

            p->prepareToPlay (graph->getSampleRate(), graph->getBlockSize());
            p->suspendProcessing (false);

            graph->prepareToPlay (graph->getSampleRate(), graph->getBlockSize());
            graph->suspendProcessing (false);
        }
    }
}

z0 IOConfigurationWindow::paint (Graphics& g)
{
     g.fillAll (getLookAndFeel().findColor (ResizableWindow::backgroundColorId));
}

z0 IOConfigurationWindow::resized()
{
    auto r = getLocalBounds().reduced (10);

    title.setBounds (r.removeFromTop (14));
    r.reduce (10, 0);

    if (inConfig != nullptr)
        inConfig->setBounds (r.removeFromTop (160));

    if (outConfig != nullptr)
        outConfig->setBounds (r.removeFromTop (160));
}

z0 IOConfigurationWindow::update()
{
    auto nodeID = getNodeID();

    if (auto* graph = getGraph())
        if (nodeID != AudioProcessorGraph::NodeID())
            graph->disconnectNode (nodeID);

    if (auto* graphEditor = getGraphEditor())
        if (auto* panel = graphEditor->graphPanel.get())
            panel->updateComponents();
}

AudioProcessorGraph::NodeID IOConfigurationWindow::getNodeID() const
{
    if (auto* graph = getGraph())
        for (auto* node : graph->getNodes())
            if (node->getProcessor() == getAudioProcessor())
                return node->nodeID;

    return {};
}

MainHostWindow* IOConfigurationWindow::getMainWindow() const
{
    auto& desktop = Desktop::getInstance();

    for (i32 i = desktop.getNumComponents(); --i >= 0;)
        if (auto* mainWindow = dynamic_cast<MainHostWindow*> (desktop.getComponent (i)))
            return mainWindow;

    return nullptr;
}

GraphDocumentComponent* IOConfigurationWindow::getGraphEditor() const
{
    if (auto* mainWindow = getMainWindow())
        return mainWindow->graphHolder.get();

    return nullptr;
}

AudioProcessorGraph* IOConfigurationWindow::getGraph() const
{
    if (auto* graphEditor = getGraphEditor())
        if (auto* panel = graphEditor->graph.get())
            return &panel->graph;

    return nullptr;
}
