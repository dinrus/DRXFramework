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

namespace TabbedComponentHelpers
{
    const Identifier deleteComponentId ("deleteByTabComp_");

    static z0 deleteIfNecessary (Component* comp)
    {
        if (comp != nullptr && (b8) comp->getProperties() [deleteComponentId])
            delete comp;
    }

    static Rectangle<i32> getTabArea (Rectangle<i32>& content, BorderSize<i32>& outline,
                                      TabbedButtonBar::Orientation orientation, i32 tabDepth)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtTop:    outline.setTop (0);     return content.removeFromTop (tabDepth);
            case TabbedButtonBar::TabsAtBottom: outline.setBottom (0);  return content.removeFromBottom (tabDepth);
            case TabbedButtonBar::TabsAtLeft:   outline.setLeft (0);    return content.removeFromLeft (tabDepth);
            case TabbedButtonBar::TabsAtRight:  outline.setRight (0);   return content.removeFromRight (tabDepth);
            default: jassertfalse; break;
        }

        return Rectangle<i32>();
    }
}

//==============================================================================
struct TabbedComponent::ButtonBar final : public TabbedButtonBar
{
    ButtonBar (TabbedComponent& tabComp, TabbedButtonBar::Orientation o)
        : TabbedButtonBar (o), owner (tabComp)
    {
    }

    z0 currentTabChanged (i32 newCurrentTabIndex, const Txt& newTabName) override
    {
        owner.changeCallback (newCurrentTabIndex, newTabName);
    }

    z0 popupMenuClickOnTab (i32 tabIndex, const Txt& tabName) override
    {
        owner.popupMenuClickOnTab (tabIndex, tabName);
    }

    Color getTabBackgroundColor (i32 tabIndex)
    {
        return owner.tabs->getTabBackgroundColor (tabIndex);
    }

    TabBarButton* createTabButton (const Txt& tabName, i32 tabIndex) override
    {
        return owner.createTabButton (tabName, tabIndex);
    }

    TabbedComponent& owner;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonBar)
};

//==============================================================================
TabbedComponent::TabbedComponent (TabbedButtonBar::Orientation orientation)
{
    tabs.reset (new ButtonBar (*this, orientation));
    addAndMakeVisible (tabs.get());
}

TabbedComponent::~TabbedComponent()
{
    clearTabs();
    tabs.reset();
}

//==============================================================================
z0 TabbedComponent::setOrientation (TabbedButtonBar::Orientation orientation)
{
    tabs->setOrientation (orientation);
    resized();
}

TabbedButtonBar::Orientation TabbedComponent::getOrientation() const noexcept
{
    return tabs->getOrientation();
}

z0 TabbedComponent::setTabBarDepth (i32 newDepth)
{
    if (tabDepth != newDepth)
    {
        tabDepth = newDepth;
        resized();
    }
}

TabBarButton* TabbedComponent::createTabButton (const Txt& tabName, i32 /*tabIndex*/)
{
    return new TabBarButton (tabName, *tabs);
}

//==============================================================================
z0 TabbedComponent::clearTabs()
{
    if (panelComponent != nullptr)
    {
        panelComponent->setVisible (false);
        removeChildComponent (panelComponent.get());
        panelComponent = nullptr;
    }

    tabs->clearTabs();

    for (i32 i = contentComponents.size(); --i >= 0;)
        TabbedComponentHelpers::deleteIfNecessary (contentComponents.getReference (i));

    contentComponents.clear();
}

z0 TabbedComponent::addTab (const Txt& tabName,
                              Color tabBackgroundColor,
                              Component* contentComponent,
                              b8 deleteComponentWhenNotNeeded,
                              i32 insertIndex)
{
    contentComponents.insert (insertIndex, WeakReference<Component> (contentComponent));

    if (deleteComponentWhenNotNeeded && contentComponent != nullptr)
        contentComponent->getProperties().set (TabbedComponentHelpers::deleteComponentId, true);

    tabs->addTab (tabName, tabBackgroundColor, insertIndex);
    resized();
}

z0 TabbedComponent::setTabName (i32 tabIndex, const Txt& newName)
{
    tabs->setTabName (tabIndex, newName);
}

z0 TabbedComponent::removeTab (i32 tabIndex)
{
    if (isPositiveAndBelow (tabIndex, contentComponents.size()))
    {
        TabbedComponentHelpers::deleteIfNecessary (contentComponents.getReference (tabIndex).get());
        contentComponents.remove (tabIndex);
        tabs->removeTab (tabIndex);
    }
}

z0 TabbedComponent::moveTab (i32 currentIndex, i32 newIndex, b8 animate)
{
    contentComponents.move (currentIndex, newIndex);
    tabs->moveTab (currentIndex, newIndex, animate);
}

i32 TabbedComponent::getNumTabs() const
{
    return tabs->getNumTabs();
}

StringArray TabbedComponent::getTabNames() const
{
    return tabs->getTabNames();
}

Component* TabbedComponent::getTabContentComponent (i32 tabIndex) const noexcept
{
    return contentComponents[tabIndex].get();
}

Color TabbedComponent::getTabBackgroundColor (i32 tabIndex) const noexcept
{
    return tabs->getTabBackgroundColor (tabIndex);
}

z0 TabbedComponent::setTabBackgroundColor (i32 tabIndex, Color newColor)
{
    tabs->setTabBackgroundColor (tabIndex, newColor);

    if (getCurrentTabIndex() == tabIndex)
        repaint();
}

z0 TabbedComponent::setCurrentTabIndex (i32 newTabIndex, b8 sendChangeMessage)
{
    tabs->setCurrentTabIndex (newTabIndex, sendChangeMessage);
}

i32 TabbedComponent::getCurrentTabIndex() const
{
    return tabs->getCurrentTabIndex();
}

Txt TabbedComponent::getCurrentTabName() const
{
    return tabs->getCurrentTabName();
}

z0 TabbedComponent::setOutline (i32 thickness)
{
    outlineThickness = thickness;
    resized();
    repaint();
}

z0 TabbedComponent::setIndent (i32 indentThickness)
{
    edgeIndent = indentThickness;
    resized();
    repaint();
}

z0 TabbedComponent::paint (Graphics& g)
{
    g.fillAll (findColor (backgroundColorId));

    auto content = getLocalBounds();
    BorderSize<i32> outline (outlineThickness);
    TabbedComponentHelpers::getTabArea (content, outline, getOrientation(), tabDepth);

    g.reduceClipRegion (content);
    g.fillAll (tabs->getTabBackgroundColor (getCurrentTabIndex()));

    if (outlineThickness > 0)
    {
        RectangleList<i32> rl (content);
        rl.subtract (outline.subtractedFrom (content));

        g.reduceClipRegion (rl);
        g.fillAll (findColor (outlineColorId));
    }
}

z0 TabbedComponent::resized()
{
    auto content = getLocalBounds();
    BorderSize<i32> outline (outlineThickness);

    tabs->setBounds (TabbedComponentHelpers::getTabArea (content, outline, getOrientation(), tabDepth));
    content = BorderSize<i32> (edgeIndent).subtractedFrom (outline.subtractedFrom (content));

    for (auto& c : contentComponents)
        if (auto comp = c.get())
            comp->setBounds (content);
}

z0 TabbedComponent::lookAndFeelChanged()
{
    for (auto& c : contentComponents)
        if (auto comp = c.get())
          comp->lookAndFeelChanged();
}

z0 TabbedComponent::changeCallback (i32 newCurrentTabIndex, const Txt& newTabName)
{
    auto* newPanelComp = getTabContentComponent (getCurrentTabIndex());

    if (newPanelComp != panelComponent)
    {
        if (panelComponent != nullptr)
        {
            panelComponent->setVisible (false);
            removeChildComponent (panelComponent);
        }

        panelComponent = newPanelComp;

        if (panelComponent != nullptr)
        {
            // do these ops as two stages instead of addAndMakeVisible() so that the
            // component has always got a parent when it gets the visibilityChanged() callback
            addChildComponent (panelComponent);
            panelComponent->sendLookAndFeelChange();
            panelComponent->setVisible (true);
            panelComponent->toFront (true);
        }

        repaint();
    }

    resized();
    currentTabChanged (newCurrentTabIndex, newTabName);
}

z0 TabbedComponent::currentTabChanged (i32, const Txt&) {}
z0 TabbedComponent::popupMenuClickOnTab (i32, const Txt&) {}

//==============================================================================
std::unique_ptr<AccessibilityHandler> TabbedComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
