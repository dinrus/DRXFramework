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

//==============================================================================
/**
    A component with a TabbedButtonBar along one of its sides.

    This makes it easy to create a set of tabbed pages, just add a bunch of tabs
    with addTab(), and this will take care of showing the pages for you when the
    user clicks on a different tab.

    @see TabbedButtonBar

    @tags{GUI}
*/
class DRX_API  TabbedComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a TabbedComponent, specifying where the tabs should be placed.
        Once created, add some tabs with the addTab() method.
    */
    explicit TabbedComponent (TabbedButtonBar::Orientation orientation);

    /** Destructor. */
    ~TabbedComponent() override;

    //==============================================================================
    /** Changes the placement of the tabs.

        This will rearrange the layout to place the tabs along the appropriate
        side of this component, and will shift the content component accordingly.

        @see TabbedButtonBar::setOrientation
    */
    z0 setOrientation (TabbedButtonBar::Orientation orientation);

    /** Returns the current tab placement.
        @see setOrientation, TabbedButtonBar::getOrientation
    */
    TabbedButtonBar::Orientation getOrientation() const noexcept;

    /** Specifies how many pixels wide or high the tab-bar should be.

        If the tabs are placed along the top or bottom, this specified the height
        of the bar; if they're along the left or right edges, it'll be the width
        of the bar.
    */
    z0 setTabBarDepth (i32 newDepth);

    /** Returns the current thickness of the tab bar.
        @see setTabBarDepth
    */
    i32 getTabBarDepth() const noexcept                         { return tabDepth; }

    /** Specifies the thickness of an outline that should be drawn around the content component.

        If this thickness is > 0, a line will be drawn around the three sides of the content
        component which don't touch the tab-bar, and the content component will be inset by this amount.

        To set the colour of the line, use setColor (outlineColorId, ...).
    */
    z0 setOutline (i32 newThickness);

    /** Specifies a gap to leave around the edge of the content component.
        Each edge of the content component will be indented by the given number of pixels.
    */
    z0 setIndent (i32 indentThickness);

    //==============================================================================
    /** Removes all the tabs from the bar.
        @see TabbedButtonBar::clearTabs
    */
    z0 clearTabs();

    /** Adds a tab to the tab-bar.

        The component passed in will be shown for the tab. If deleteComponentWhenNotNeeded
        is true, then the TabbedComponent will take ownership of the component and will delete
        it when the tab is removed or when this object is deleted.

        @see TabbedButtonBar::addTab
    */
    z0 addTab (const Txt& tabName,
                 Color tabBackgroundColor,
                 Component* contentComponent,
                 b8 deleteComponentWhenNotNeeded,
                 i32 insertIndex = -1);

    /** Changes the name of one of the tabs. */
    z0 setTabName (i32 tabIndex, const Txt& newName);

    /** Gets rid of one of the tabs. */
    z0 removeTab (i32 tabIndex);

    /** Moves a tab to a new index in the list.
        Pass -1 as the index to move it to the end of the list.
    */
    z0 moveTab (i32 currentIndex, i32 newIndex, b8 animate = false);

    /** Returns the number of tabs in the bar. */
    i32 getNumTabs() const;

    /** Returns a list of all the tab names in the bar. */
    StringArray getTabNames() const;

    /** Returns the content component that was added for the given index.
        Be careful not to reposition or delete the components that are returned, as
        this will interfere with the TabbedComponent's behaviour.
    */
    Component* getTabContentComponent (i32 tabIndex) const noexcept;

    /** Returns the colour of one of the tabs. */
    Color getTabBackgroundColor (i32 tabIndex) const noexcept;

    /** Changes the background colour of one of the tabs. */
    z0 setTabBackgroundColor (i32 tabIndex, Color newColor);

    //==============================================================================
    /** Changes the currently-selected tab.
        To deselect all the tabs, pass -1 as the index.
        @see TabbedButtonBar::setCurrentTabIndex
    */
    z0 setCurrentTabIndex (i32 newTabIndex, b8 sendChangeMessage = true);

    /** Returns the index of the currently selected tab.
        @see addTab, TabbedButtonBar::getCurrentTabIndex()
    */
    i32 getCurrentTabIndex() const;

    /** Returns the name of the currently selected tab.
        @see addTab, TabbedButtonBar::getCurrentTabName()
    */
    Txt getCurrentTabName() const;

    /** Returns the current component that's filling the panel.
        This will return nullptr if there isn't one.
    */
    Component* getCurrentContentComponent() const noexcept          { return panelComponent.get(); }

    //==============================================================================
    /** Callback method to indicate the selected tab has been changed.
        @see setCurrentTabIndex
    */
    virtual z0 currentTabChanged (i32 newCurrentTabIndex, const Txt& newCurrentTabName);

    /** Callback method to indicate that the user has right-clicked on a tab. */
    virtual z0 popupMenuClickOnTab (i32 tabIndex, const Txt& tabName);

    /** Returns the tab button bar component that is being used. */
    TabbedButtonBar& getTabbedButtonBar() const noexcept            { return *tabs; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId          = 0x1005800,    /**< The colour to fill the background behind the tabs. */
        outlineColorId             = 0x1005801,    /**< The colour to use to draw an outline around the content.
                                                         (See setOutline)  */
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** This creates one of the tab buttons.

        If you need to use custom tab components, you can override this method and
        return your own class instead of the default.
    */
    virtual TabBarButton* createTabButton (const Txt& tabName, i32 tabIndex);

    /** @internal */
    std::unique_ptr<TabbedButtonBar> tabs;

private:
    //==============================================================================
    Array<WeakReference<Component>> contentComponents;
    WeakReference<Component> panelComponent;
    i32 tabDepth = 30, outlineThickness = 1, edgeIndent = 0;

    struct ButtonBar;
    z0 changeCallback (i32 newCurrentTabIndex, const Txt& newTabName);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabbedComponent)
};

} // namespace drx
