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

TabBarButton::TabBarButton (const Txt& name, TabbedButtonBar& bar)
    : Button (name), owner (bar)
{
    setWantsKeyboardFocus (false);
}

TabBarButton::~TabBarButton() {}

i32 TabBarButton::getIndex() const                      { return owner.indexOfTabButton (this); }
Color TabBarButton::getTabBackgroundColor() const     { return owner.getTabBackgroundColor (getIndex()); }
b8 TabBarButton::isFrontTab() const                   { return getToggleState(); }

z0 TabBarButton::paintButton (Graphics& g, const b8 shouldDrawButtonAsHighlighted, const b8 shouldDrawButtonAsDown)
{
    getLookAndFeel().drawTabButton (*this, g, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

z0 TabBarButton::clicked (const ModifierKeys& mods)
{
    if (mods.isPopupMenu())
        owner.popupMenuClickOnTab (getIndex(), getButtonText());
    else
        owner.setCurrentTabIndex (getIndex());
}

b8 TabBarButton::hitTest (i32 mx, i32 my)
{
    auto area = getActiveArea();

    if (owner.isVertical())
    {
        if (isPositiveAndBelow (mx, getWidth())
             && my >= area.getY() + overlapPixels && my < area.getBottom() - overlapPixels)
            return true;
    }
    else
    {
        if (isPositiveAndBelow (my, getHeight())
             && mx >= area.getX() + overlapPixels && mx < area.getRight() - overlapPixels)
            return true;
    }

    Path p;
    getLookAndFeel().createTabButtonShape (*this, p, false, false);

    return p.contains ((f32) (mx - area.getX()),
                       (f32) (my - area.getY()));
}

i32 TabBarButton::getBestTabLength (i32k depth)
{
    return getLookAndFeel().getTabButtonBestWidth (*this, depth);
}

z0 TabBarButton::calcAreas (Rectangle<i32>& extraComp, Rectangle<i32>& textArea) const
{
    auto& lf = getLookAndFeel();
    textArea = getActiveArea();

    auto depth = owner.isVertical() ? textArea.getWidth() : textArea.getHeight();
    auto overlap = lf.getTabButtonOverlap (depth);

    if (overlap > 0)
    {
        if (owner.isVertical())
            textArea.reduce (0, overlap);
        else
            textArea.reduce (overlap, 0);
    }

    if (extraComponent != nullptr)
    {
        extraComp = lf.getTabButtonExtraComponentBounds (*this, textArea, *extraComponent);

        auto orientation = owner.getOrientation();

        if (orientation == TabbedButtonBar::TabsAtLeft || orientation == TabbedButtonBar::TabsAtRight)
        {
            if (extraComp.getCentreY() > textArea.getCentreY())
                textArea.setBottom (jmin (textArea.getBottom(), extraComp.getY()));
            else
                textArea.setTop (jmax (textArea.getY(), extraComp.getBottom()));
        }
        else
        {
            if (extraComp.getCentreX() > textArea.getCentreX())
                textArea.setRight (jmin (textArea.getRight(), extraComp.getX()));
            else
                textArea.setLeft (jmax (textArea.getX(), extraComp.getRight()));
        }
    }
}

Rectangle<i32> TabBarButton::getTextArea() const
{
    Rectangle<i32> extraComp, textArea;
    calcAreas (extraComp, textArea);
    return textArea;
}

Rectangle<i32> TabBarButton::getActiveArea() const
{
    auto r = getLocalBounds();
    auto spaceAroundImage = getLookAndFeel().getTabButtonSpaceAroundImage();
    auto orientation = owner.getOrientation();

    if (orientation != TabbedButtonBar::TabsAtLeft)      r.removeFromRight  (spaceAroundImage);
    if (orientation != TabbedButtonBar::TabsAtRight)     r.removeFromLeft   (spaceAroundImage);
    if (orientation != TabbedButtonBar::TabsAtBottom)    r.removeFromTop    (spaceAroundImage);
    if (orientation != TabbedButtonBar::TabsAtTop)       r.removeFromBottom (spaceAroundImage);

    return r;
}

z0 TabBarButton::setExtraComponent (Component* comp, ExtraComponentPlacement placement)
{
    jassert (extraCompPlacement == beforeText || extraCompPlacement == afterText);
    extraCompPlacement = placement;
    extraComponent.reset (comp);
    addAndMakeVisible (extraComponent.get());
    resized();
}

z0 TabBarButton::childBoundsChanged (Component* c)
{
    if (c == extraComponent.get())
    {
        owner.resized();
        resized();
    }
}

z0 TabBarButton::resized()
{
    if (extraComponent != nullptr)
    {
        Rectangle<i32> extraComp, textArea;
        calcAreas (extraComp, textArea);

        if (! extraComp.isEmpty())
            extraComponent->setBounds (extraComp);
    }
}

//==============================================================================
class TabbedButtonBar::BehindFrontTabComp final : public Component
{
public:
    BehindFrontTabComp (TabbedButtonBar& tb)  : owner (tb)
    {
        setInterceptsMouseClicks (false, false);
    }

    z0 paint (Graphics& g) override
    {
        getLookAndFeel().drawTabAreaBehindFrontButton (owner, g, getWidth(), getHeight());
    }

    z0 enablementChanged() override
    {
        repaint();
    }

    TabbedButtonBar& owner;

    DRX_DECLARE_NON_COPYABLE (BehindFrontTabComp)
};


//==============================================================================
TabbedButtonBar::TabbedButtonBar (Orientation orientationToUse)
    : orientation (orientationToUse)
{
    setInterceptsMouseClicks (false, true);
    behindFrontTab.reset (new BehindFrontTabComp (*this));
    addAndMakeVisible (behindFrontTab.get());
    setFocusContainerType (FocusContainerType::keyboardFocusContainer);
}

TabbedButtonBar::~TabbedButtonBar()
{
    tabs.clear();
    extraTabsButton.reset();
}

//==============================================================================
z0 TabbedButtonBar::setOrientation (const Orientation newOrientation)
{
    orientation = newOrientation;

    for (auto* child : getChildren())
        child->resized();

    resized();
}

TabBarButton* TabbedButtonBar::createTabButton (const Txt& name, i32k /*index*/)
{
    return new TabBarButton (name, *this);
}

z0 TabbedButtonBar::setMinimumTabScaleFactor (f64 newMinimumScale)
{
    minimumScale = newMinimumScale;
    resized();
}

//==============================================================================
z0 TabbedButtonBar::clearTabs()
{
    tabs.clear();
    extraTabsButton.reset();
    setCurrentTabIndex (-1);
}

z0 TabbedButtonBar::addTab (const Txt& tabName,
                              Color tabBackgroundColor,
                              i32 insertIndex)
{
    jassert (tabName.isNotEmpty()); // you have to give them all a name..

    if (tabName.isNotEmpty())
    {
        if (! isPositiveAndBelow (insertIndex, tabs.size()))
            insertIndex = tabs.size();

        auto* currentTab = tabs[currentTabIndex];

        auto* newTab = new TabInfo();
        newTab->name = tabName;
        newTab->colour = tabBackgroundColor;
        newTab->button.reset (createTabButton (tabName, insertIndex));
        jassert (newTab->button != nullptr);

        tabs.insert (insertIndex, newTab);
        currentTabIndex = tabs.indexOf (currentTab);
        addAndMakeVisible (newTab->button.get(), insertIndex);

        resized();

        if (currentTabIndex < 0)
            setCurrentTabIndex (0);
    }
}

z0 TabbedButtonBar::setTabName (i32 tabIndex, const Txt& newName)
{
    if (auto* tab = tabs[tabIndex])
    {
        if (tab->name != newName)
        {
            tab->name = newName;
            tab->button->setButtonText (newName);
            resized();
        }
    }
}

z0 TabbedButtonBar::removeTab (i32k indexToRemove, const b8 animate)
{
    if (isPositiveAndBelow (indexToRemove, tabs.size()))
    {
        auto oldSelectedIndex = currentTabIndex;

        if (indexToRemove == currentTabIndex)
            oldSelectedIndex = -1;
        else if (indexToRemove < oldSelectedIndex)
            --oldSelectedIndex;

        tabs.remove (indexToRemove);

        setCurrentTabIndex (oldSelectedIndex);
        updateTabPositions (animate);
    }
}

z0 TabbedButtonBar::moveTab (i32k currentIndex, i32k newIndex, const b8 animate)
{
    auto* currentTab = tabs[currentTabIndex];
    tabs.move (currentIndex, newIndex);
    currentTabIndex = tabs.indexOf (currentTab);
    updateTabPositions (animate);
}

i32 TabbedButtonBar::getNumTabs() const
{
    return tabs.size();
}

Txt TabbedButtonBar::getCurrentTabName() const
{
    if (auto* tab = tabs [currentTabIndex])
        return tab->name;

    return {};
}

StringArray TabbedButtonBar::getTabNames() const
{
    StringArray names;

    for (auto* t : tabs)
        names.add (t->name);

    return names;
}

z0 TabbedButtonBar::setCurrentTabIndex (i32 newIndex, b8 shouldSendChangeMessage)
{
    if (currentTabIndex != newIndex)
    {
        if (! isPositiveAndBelow (newIndex, tabs.size()))
            newIndex = -1;

        currentTabIndex = newIndex;

        for (i32 i = 0; i < tabs.size(); ++i)
            tabs.getUnchecked (i)->button->setToggleState (i == newIndex, dontSendNotification);

        resized();

        if (shouldSendChangeMessage)
            sendChangeMessage();

        currentTabChanged (newIndex, getCurrentTabName());
    }
}

TabBarButton* TabbedButtonBar::getTabButton (i32k index) const
{
    if (auto* tab = tabs[index])
        return static_cast<TabBarButton*> (tab->button.get());

    return nullptr;
}

i32 TabbedButtonBar::indexOfTabButton (const TabBarButton* button) const
{
    for (i32 i = tabs.size(); --i >= 0;)
        if (tabs.getUnchecked (i)->button.get() == button)
            return i;

    return -1;
}

Rectangle<i32> TabbedButtonBar::getTargetBounds (TabBarButton* button) const
{
    if (button == nullptr || indexOfTabButton (button) == -1)
        return {};

    auto& animator = Desktop::getInstance().getAnimator();

    return animator.isAnimating (button) ? animator.getComponentDestination (button)
                                         : button->getBounds();
}

z0 TabbedButtonBar::lookAndFeelChanged()
{
    extraTabsButton.reset();
    resized();
}

z0 TabbedButtonBar::paint (Graphics& g)
{
    getLookAndFeel().drawTabbedButtonBarBackground (*this, g);
}

z0 TabbedButtonBar::resized()
{
    updateTabPositions (false);
}

//==============================================================================
z0 TabbedButtonBar::updateTabPositions (b8 animate)
{
    auto& lf = getLookAndFeel();

    auto depth = getWidth();
    auto length = getHeight();

    if (! isVertical())
        std::swap (depth, length);

    auto overlap = lf.getTabButtonOverlap (depth) + lf.getTabButtonSpaceAroundImage() * 2;

    auto totalLength = jmax (0, overlap);
    auto numVisibleButtons = tabs.size();

    for (i32 i = 0; i < tabs.size(); ++i)
    {
        auto* tb = tabs.getUnchecked (i)->button.get();

        totalLength += tb->getBestTabLength (depth) - overlap;
        tb->overlapPixels = jmax (0, overlap / 2);
    }

    f64 scale = 1.0;

    if (totalLength > length)
        scale = jmax (minimumScale, length / (f64) totalLength);

    const b8 isTooBig = (i32) (totalLength * scale) > length;
    i32 tabsButtonPos = 0;

    if (isTooBig)
    {
        if (extraTabsButton == nullptr)
        {
            extraTabsButton.reset (lf.createTabBarExtrasButton());
            addAndMakeVisible (extraTabsButton.get());
            extraTabsButton->setAlwaysOnTop (true);
            extraTabsButton->setTriggeredOnMouseDown (true);
            extraTabsButton->onClick = [this] { showExtraItemsMenu(); };
        }

        auto buttonSize = jmin (proportionOfWidth (0.7f), proportionOfHeight (0.7f));
        extraTabsButton->setSize (buttonSize, buttonSize);

        if (isVertical())
        {
            tabsButtonPos = getHeight() - buttonSize / 2 - 1;
            extraTabsButton->setCentrePosition (getWidth() / 2, tabsButtonPos);
        }
        else
        {
            tabsButtonPos = getWidth() - buttonSize / 2 - 1;
            extraTabsButton->setCentrePosition (tabsButtonPos, getHeight() / 2);
        }

        totalLength = 0;

        for (i32 i = 0; i < tabs.size(); ++i)
        {
            auto* tb = tabs.getUnchecked (i)->button.get();
            auto newLength = totalLength + tb->getBestTabLength (depth);

            if (i > 0 && newLength * minimumScale > tabsButtonPos)
            {
                totalLength += overlap;
                break;
            }

            numVisibleButtons = i + 1;
            totalLength = newLength - overlap;
        }

        scale = jmax (minimumScale, tabsButtonPos / (f64) totalLength);
    }
    else
    {
        extraTabsButton.reset();
    }

    i32 pos = 0;

    TabBarButton* frontTab = nullptr;
    auto& animator = Desktop::getInstance().getAnimator();

    for (i32 i = 0; i < tabs.size(); ++i)
    {
        if (auto* tb = getTabButton (i))
        {
            auto bestLength = roundToInt (scale * tb->getBestTabLength (depth));

            if (i < numVisibleButtons)
            {
                auto newBounds = isVertical() ? Rectangle<i32> (0, pos, getWidth(), bestLength)
                                              : Rectangle<i32> (pos, 0, bestLength, getHeight());

                if (animate)
                {
                    animator.animateComponent (tb, newBounds, 1.0f, 200, false, 3.0, 0.0);
                }
                else
                {
                    animator.cancelAnimation (tb, false);
                    tb->setBounds (newBounds);
                }

                tb->toBack();

                if (i == currentTabIndex)
                    frontTab = tb;

                tb->setVisible (true);
            }
            else
            {
                tb->setVisible (false);
            }

            pos += bestLength - overlap;
        }
    }

    behindFrontTab->setBounds (getLocalBounds());

    if (frontTab != nullptr)
    {
        frontTab->toFront (false);
        behindFrontTab->toBehind (frontTab);
    }
}

//==============================================================================
Color TabbedButtonBar::getTabBackgroundColor (i32 tabIndex)
{
    if (auto* tab = tabs[tabIndex])
        return tab->colour;

    return Colors::transparentBlack;
}

z0 TabbedButtonBar::setTabBackgroundColor (i32 tabIndex, Color newColor)
{
    if (auto* tab = tabs [tabIndex])
    {
        if (tab->colour != newColor)
        {
            tab->colour = newColor;
            repaint();
        }
    }
}

z0 TabbedButtonBar::showExtraItemsMenu()
{
    PopupMenu m;

    for (i32 i = 0; i < tabs.size(); ++i)
    {
        auto* tab = tabs.getUnchecked (i);

        if (! tab->button->isVisible())
            m.addItem (PopupMenu::Item (tab->name)
                         .setTicked (i == currentTabIndex)
                         .setAction ([this, i] { setCurrentTabIndex (i); }));
    }

    m.showMenuAsync (PopupMenu::Options()
                        .withDeletionCheck (*this)
                        .withTargetComponent (extraTabsButton.get()));
}

//==============================================================================
z0 TabbedButtonBar::currentTabChanged (i32, const Txt&) {}
z0 TabbedButtonBar::popupMenuClickOnTab (i32, const Txt&) {}

//==============================================================================
std::unique_ptr<AccessibilityHandler> TabbedButtonBar::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
