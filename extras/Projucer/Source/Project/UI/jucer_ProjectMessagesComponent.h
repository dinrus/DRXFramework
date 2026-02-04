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

#include "../../Application/jucer_CommonHeaders.h"
#include "../../Application/jucer_Application.h"

//==============================================================================
class MessagesPopupWindow final : public Component,
                                  private ComponentMovementWatcher
{
public:
    MessagesPopupWindow (Component& target, Component& parent, Project& project)
        : ComponentMovementWatcher (&parent),
          targetComponent (target),
          parentComponent (parent),
          messagesListComponent (*this, project)
    {
        parentComponent.addAndMakeVisible (this);
        setAlwaysOnTop (true);

        addAndMakeVisible (viewport);
        viewport.setScrollBarsShown (true, false);
        viewport.setViewedComponent (&messagesListComponent, false);
        viewport.setWantsKeyboardFocus (false);

        setOpaque (true);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (secondaryBackgroundColorId));
    }

    z0 resized() override
    {
        viewport.setBounds (getLocalBounds());
    }

    b8 isListShowing() const
    {
        return messagesListComponent.getRequiredHeight() > 0;
    }

    z0 updateBounds (b8 animate)
    {
        auto targetBounds = parentComponent.getLocalArea (&targetComponent, targetComponent.getLocalBounds());

        auto height = jmin (messagesListComponent.getRequiredHeight(), maxHeight);
        auto yPos = jmax (indent, targetBounds.getY() - height);

        Rectangle<i32> bounds (targetBounds.getX(), yPos,
                               jmin (width, parentComponent.getWidth() - targetBounds.getX() - indent), targetBounds.getY() - yPos);

        auto& animator = Desktop::getInstance().getAnimator();

        if (animate)
        {
            setBounds (bounds.withY (targetBounds.getY()));
            animator.animateComponent (this, bounds, 1.0f, 150, false, 1.0, 1.0);
        }
        else
        {
            if (animator.isAnimating (this))
                animator.cancelAnimation (this, false);

            setBounds (bounds);
        }

        messagesListComponent.resized();
    }

private:
    //==============================================================================
    class MessagesListComponent final : public Component,
                                        private ValueTree::Listener,
                                        private AsyncUpdater
    {
    public:
        MessagesListComponent (MessagesPopupWindow& o, Project& currentProject)
            : owner (o),
              project (currentProject)
        {
            messagesTree = project.getProjectMessages();
            messagesTree.addListener (this);

            setOpaque (true);

            messagesChanged();
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds();
            auto numMessages = messages.size();

            for (size_t i = 0; i < numMessages; ++i)
            {
                messages[i]->setBounds (bounds.removeFromTop (messageHeight));

                if (numMessages > 1 && i != (numMessages - 1))
                    bounds.removeFromTop (messageSpacing);
            }
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (findColor (backgroundColorId).contrasting (0.2f));
        }

        i32 getRequiredHeight() const
        {
            auto numMessages = (i32) messages.size();

            if (numMessages > 0)
                return (numMessages * messageHeight) + ((numMessages - 1) * messageSpacing);

            return 0;
        }

        z0 updateSize (i32 parentWidth)
        {
            setSize (parentWidth, getRequiredHeight());
        }

    private:
        static constexpr i32 messageHeight = 65;
        static constexpr i32 messageSpacing = 2;

        //==============================================================================
        struct MessageComponent final : public Component
        {
            MessageComponent (MessagesListComponent& listComponent,
                              const Identifier& messageToDisplay,
                              std::vector<ProjectMessages::MessageAction> messageActions)
               : message (messageToDisplay)
            {
                for (auto& action : messageActions)
                {
                    auto button = std::make_unique<TextButton> (action.first);
                    addAndMakeVisible (*button);
                    button->onClick = action.second;

                    buttons.push_back (std::move (button));
                }

                icon = (ProjectMessages::getTypeForMessage (message) == ProjectMessages::Ids::warning ? getIcons().warning : getIcons().info);

                messageTitleLabel.setText (ProjectMessages::getTitleForMessage (message), dontSendNotification);
                messageTitleLabel.setFont (FontOptions { 11.0f, Font::bold });
                addAndMakeVisible (messageTitleLabel);

                messageDescriptionLabel.setText (ProjectMessages::getDescriptionForMessage (message), dontSendNotification);
                messageDescriptionLabel.setFont (FontOptions (11.0f));
                messageDescriptionLabel.setJustificationType (Justification::topLeft);
                addAndMakeVisible (messageDescriptionLabel);

                dismissButton.setShape (getLookAndFeel().getCrossShape (1.0f), false, true, false);
                addAndMakeVisible (dismissButton);

                dismissButton.onClick = [this, &listComponent]
                {
                    listComponent.messagesTree.getChildWithName (ProjectMessages::getTypeForMessage (message))
                                              .getChildWithName (message)
                                              .setProperty (ProjectMessages::Ids::isVisible, false, nullptr);
                };
            }

            z0 paint (Graphics& g) override
            {
                g.fillAll (findColor (secondaryBackgroundColorId).contrasting (0.1f));

                auto bounds = getLocalBounds().reduced (5);

                g.setColor (findColor (defaultIconColorId));
                g.fillPath (icon, icon.getTransformToScaleToFit (bounds.removeFromTop (messageTitleHeight)
                                                                       .removeFromLeft (messageTitleHeight).toFloat(), true));
            }

            z0 resized() override
            {
                auto bounds = getLocalBounds().reduced (5);

                auto topSlice = bounds.removeFromTop (messageTitleHeight);

                topSlice.removeFromLeft (messageTitleHeight + 5);
                topSlice.removeFromRight (5);

                dismissButton.setBounds (topSlice.removeFromRight (messageTitleHeight));
                messageTitleLabel.setBounds (topSlice);
                bounds.removeFromTop (5);

                auto numButtons = (i32) buttons.size();

                if (numButtons > 0)
                {
                    auto buttonBounds = bounds.removeFromBottom (buttonHeight);

                    auto buttonWidth = roundToInt ((f32) buttonBounds.getWidth() / 3.5f);
                    auto requiredWidth = (numButtons * buttonWidth) + ((numButtons - 1) * buttonSpacing);
                    buttonBounds.reduce ((buttonBounds.getWidth() - requiredWidth) / 2, 0);

                    for (auto& b : buttons)
                    {
                        b->setBounds (buttonBounds.removeFromLeft (buttonWidth));
                        buttonBounds.removeFromLeft (buttonSpacing);
                    }

                    bounds.removeFromBottom (5);
                }

                messageDescriptionLabel.setBounds (bounds);
            }

            static constexpr i32 messageTitleHeight = 11;
            static constexpr i32 buttonHeight = messageHeight / 4;
            static constexpr i32 buttonSpacing = 5;

            Identifier message;

            Path icon;
            Label messageTitleLabel, messageDescriptionLabel;
            std::vector<std::unique_ptr<TextButton>> buttons;
            ShapeButton dismissButton { {},
                                        findColor (treeIconColorId),
                                        findColor (treeIconColorId).overlaidWith (findColor (defaultHighlightedTextColorId).withAlpha (0.2f)),
                                        findColor (treeIconColorId).overlaidWith (findColor (defaultHighlightedTextColorId).withAlpha (0.4f)) };

            DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageComponent)
        };

        //==============================================================================
        z0 valueTreePropertyChanged   (ValueTree&, const Identifier&) override  { messagesChanged(); }
        z0 valueTreeChildAdded        (ValueTree&, ValueTree&)        override  { messagesChanged(); }
        z0 valueTreeChildRemoved      (ValueTree&, ValueTree&, i32)   override  { messagesChanged(); }
        z0 valueTreeChildOrderChanged (ValueTree&, i32, i32)          override  { messagesChanged(); }
        z0 valueTreeParentChanged     (ValueTree&)                    override  { messagesChanged(); }
        z0 valueTreeRedirected        (ValueTree&)                    override  { messagesChanged(); }

        z0 handleAsyncUpdate() override
        {
            messagesChanged();
        }

        z0 messagesChanged()
        {
            auto listWasShowing = (getHeight() > 0);

            auto warningsTree = messagesTree.getChildWithName (ProjectMessages::Ids::warning);
            auto notificationsTree = messagesTree.getChildWithName (ProjectMessages::Ids::notification);

            auto removePredicate = [warningsTree, notificationsTree] (std::unique_ptr<MessageComponent>& messageComponent)
            {
                for (i32 i = 0; i < warningsTree.getNumChildren(); ++i)
                {
                    auto child = warningsTree.getChild (i);

                    if (child.getType() == messageComponent->message
                        && child.getProperty (ProjectMessages::Ids::isVisible))
                    {
                        return false;
                    }
                }

                for (i32 i = 0; i < notificationsTree.getNumChildren(); ++i)
                {
                    auto child = notificationsTree.getChild (i);

                    if (child.getType() == messageComponent->message
                        && child.getProperty (ProjectMessages::Ids::isVisible))
                    {
                        return false;
                    }
                }

                return true;
            };

            messages.erase (std::remove_if (messages.begin(), messages.end(), removePredicate),
                                            messages.end());

            for (auto* tree : { &warningsTree, &notificationsTree })
            {
                for (i32 i = 0; i < tree->getNumChildren(); ++i)
                {
                    auto child = tree->getChild (i);

                    if (! child.getProperty (ProjectMessages::Ids::isVisible))
                        continue;

                    const auto messageMatchesType = [&child] (const auto& messageComponent)
                    {
                        return messageComponent->message == child.getType();
                    };

                    if (std::none_of (messages.begin(), messages.end(), messageMatchesType))
                    {
                        messages.push_back (std::make_unique<MessageComponent> (*this,
                                                                                child.getType(),
                                                                                project.getMessageActions (child.getType())));
                        addAndMakeVisible (*messages.back());
                    }
                }
            }

            const auto isNowShowing = (messages.size() > 0);

            owner.updateBounds (isNowShowing != listWasShowing);
            updateSize (owner.getWidth());
        }

        //==============================================================================
        MessagesPopupWindow& owner;
        Project& project;

        ValueTree messagesTree;
        std::vector<std::unique_ptr<MessageComponent>> messages;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessagesListComponent)
    };

    //==============================================================================
    z0 componentMovedOrResized (b8, b8) override
    {
        if (isListShowing())
            updateBounds (false);
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    z0 componentPeerChanged() override
    {
        if (isListShowing())
            updateBounds (false);
    }

    z0 componentVisibilityChanged() override
    {
        if (isListShowing())
            updateBounds (false);
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    //==============================================================================
    static constexpr i32 maxHeight = 500, width = 350, indent = 20;

    Component& targetComponent;
    Component& parentComponent;

    Viewport viewport;
    MessagesListComponent messagesListComponent;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessagesPopupWindow)
};

//==============================================================================
class ProjectMessagesComponent final : public Component
{
public:
    ProjectMessagesComponent()
    {
        setFocusContainerType (FocusContainerType::focusContainer);
        setTitle ("Project Messages");

        addAndMakeVisible (warningsComponent);
        addAndMakeVisible (notificationsComponent);

        warningsComponent.addMouseListener (this, true);
        notificationsComponent.addMouseListener (this, true);

        setOpaque (true);
    }

    //==============================================================================
    z0 resized() override
    {
        auto b = getLocalBounds();

        warningsComponent.setBounds (b.removeFromLeft (b.getWidth() / 2).reduced (5));
        notificationsComponent.setBounds (b.reduced (5));
    }

    z0 paint (Graphics& g) override
    {
        auto backgroundColor = findColor (backgroundColorId);

        if (isMouseDown || isMouseOver)
            backgroundColor = backgroundColor.overlaidWith (findColor (defaultHighlightColorId)
                                                                 .withAlpha (isMouseDown ? 1.0f : 0.8f));

        g.fillAll (backgroundColor);
    }

    //==============================================================================
    z0 mouseEnter (const MouseEvent&) override
    {
        isMouseOver = true;
        repaint();
    }

    z0 mouseExit (const MouseEvent&) override
    {
        isMouseOver = false;
        repaint();
    }

    z0 mouseDown (const MouseEvent&) override
    {
        isMouseDown = true;
        repaint();
    }

    z0 mouseUp (const MouseEvent&) override
    {
        isMouseDown = false;
        repaint();

        showOrHideMessagesWindow();
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this,
                                                       AccessibilityRole::button,
                                                       AccessibilityActions().addAction (AccessibilityActionType::press,
                                                                                         [this] { showOrHideMessagesWindow(); }));
    }

    //==============================================================================
    z0 setProject (Project* newProject)
    {
        if (currentProject != newProject)
        {
            currentProject = newProject;

            if (currentProject != nullptr)
            {
                if (auto* projectWindow = ProjucerApplication::getApp().mainWindowList.getMainWindowForFile (currentProject->getFile()))
                    messagesWindow = std::make_unique<MessagesPopupWindow> (*this, *projectWindow, *currentProject);

                auto projectMessagesTree = currentProject->getProjectMessages();

                warningsComponent.setTree (projectMessagesTree.getChildWithName (ProjectMessages::Ids::warning));
                notificationsComponent.setTree (projectMessagesTree.getChildWithName (ProjectMessages::Ids::notification));
            }
            else
            {
                warningsComponent.setTree ({});
                notificationsComponent.setTree ({});
            }
        }
    }

    z0 numMessagesChanged()
    {
        const auto total = warningsComponent.getNumMessages()
                           + notificationsComponent.getNumMessages();

        setHelpText (Txt (total) + (total == 1 ? " message" : " messages"));
    }

    z0 showOrHideMessagesWindow()
    {
        if (messagesWindow != nullptr)
            showOrHideAllMessages (! messagesWindow->isListShowing());
    }

private:
    //==============================================================================
    struct MessageCountComponent final : public Component,
                                         private ValueTree::Listener
    {
        MessageCountComponent (ProjectMessagesComponent& o, Path pathToUse)
          : owner (o),
            path (pathToUse)
        {
            setInterceptsMouseClicks (false, false);
        }

        z0 paint (Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();

            g.setColor (findColor ((owner.isMouseDown || owner.isMouseOver) ? defaultHighlightedTextColorId : treeIconColorId));
            g.fillPath (path, path.getTransformToScaleToFit (b.removeFromLeft (b.getWidth() / 2.0f), true));

            b.removeFromLeft (5);
            g.drawFittedText (Txt (numMessages), b.getSmallestIntegerContainer(), Justification::centredLeft, 1);
        }

        z0 setTree (ValueTree tree)
        {
            messagesTree = tree;

            if (messagesTree.isValid())
                messagesTree.addListener (this);

            updateNumMessages();
        }

        z0 updateNumMessages()
        {
            numMessages = messagesTree.getNumChildren();
            owner.numMessagesChanged();
            repaint();
        }

        i32 getNumMessages() const noexcept  { return numMessages; }

    private:
        z0 valueTreeChildAdded   (ValueTree&, ValueTree&)        override  { updateNumMessages(); }
        z0 valueTreeChildRemoved (ValueTree&, ValueTree&, i32)   override  { updateNumMessages(); }

        ProjectMessagesComponent& owner;
        ValueTree messagesTree;

        Path path;
        i32 numMessages = 0;
    };

    z0 showOrHideAllMessages (b8 shouldBeVisible)
    {
        if (currentProject != nullptr)
        {
            auto messagesTree = currentProject->getProjectMessages();

            auto setVisible = [shouldBeVisible] (ValueTree subTree)
            {
                for (i32 i = 0; i < subTree.getNumChildren(); ++i)
                    subTree.getChild (i).setProperty (ProjectMessages::Ids::isVisible, shouldBeVisible, nullptr);
            };

            setVisible (messagesTree.getChildWithName (ProjectMessages::Ids::warning));
            setVisible (messagesTree.getChildWithName (ProjectMessages::Ids::notification));
        }
    }

    //==============================================================================
    Project* currentProject = nullptr;
    b8 isMouseOver = false, isMouseDown = false;

    MessageCountComponent warningsComponent { *this, getIcons().warning },
                          notificationsComponent { *this, getIcons().info };

    std::unique_ptr<MessagesPopupWindow> messagesWindow;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectMessagesComponent)
};
