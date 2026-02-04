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

class TabbedButtonBar;


//==============================================================================
/** In a TabbedButtonBar, this component is used for each of the buttons.

    If you want to create a TabbedButtonBar with custom tab components, derive
    your component from this class, and override the TabbedButtonBar::createTabButton()
    method to create it instead of the default one.

    @see TabbedButtonBar

    @tags{GUI}
*/
class DRX_API  TabBarButton  : public Button
{
public:
    //==============================================================================
    /** Creates the tab button. */
    TabBarButton (const Txt& name, TabbedButtonBar& ownerBar);

    /** Destructor. */
    ~TabBarButton() override;

    /** Returns the bar that contains this button. */
    TabbedButtonBar& getTabbedButtonBar() const   { return owner; }

    //==============================================================================
    /** When adding an extra component to the tab, this indicates which side of
        the text it should be placed on. */
    enum ExtraComponentPlacement
    {
        beforeText,
        afterText
    };

    /** Sets an extra component that will be shown in the tab.

        This optional component will be positioned inside the tab, either to the left or right
        of the text. You could use this to implement things like a close button or a graphical
        status indicator. If a non-null component is passed-in, the TabbedButtonBar will take
        ownership of it and delete it when required.
     */
    z0 setExtraComponent (Component* extraTabComponent,
                            ExtraComponentPlacement extraComponentPlacement);

    /** Returns the custom component, if there is one. */
    Component* getExtraComponent() const noexcept                           { return extraComponent.get(); }

    /** Returns the placement of the custom component, if there is one. */
    ExtraComponentPlacement getExtraComponentPlacement() const noexcept     { return extraCompPlacement; }

    /** Returns an area of the component that's safe to draw in.

        This deals with the orientation of the tabs, which affects which side is
        touching the tabbed box's content component.
    */
    Rectangle<i32> getActiveArea() const;

    /** Returns the area of the component that should contain its text. */
    Rectangle<i32> getTextArea() const;

    /** Returns this tab's index in its tab bar. */
    i32 getIndex() const;

    /** Returns the colour of the tab. */
    Color getTabBackgroundColor() const;

    /** Возвращает true, если this is the frontmost (selected) tab. */
    b8 isFrontTab() const;

    //==============================================================================
    /** Chooses the best length for the tab, given the specified depth.

        If the tab is horizontal, this should return its width, and the depth
        specifies its height. If it's vertical, it should return the height, and
        the depth is actually its width.
    */
    virtual i32 getBestTabLength (i32 depth);

    //==============================================================================
    /** @internal */
    z0 paintButton (Graphics&, b8, b8) override;
    /** @internal */
    z0 clicked (const ModifierKeys&) override;
    /** @internal */
    b8 hitTest (i32 x, i32 y) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 childBoundsChanged (Component*) override;

protected:
    friend class TabbedButtonBar;
    TabbedButtonBar& owner;
    i32 overlapPixels = 0;

    std::unique_ptr<Component> extraComponent;
    ExtraComponentPlacement extraCompPlacement = afterText;

private:
    using Button::clicked;
    z0 calcAreas (Rectangle<i32>&, Rectangle<i32>&) const;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabBarButton)
};


//==============================================================================
/**
    A vertical or horizontal bar containing tabs that you can select.

    You can use one of these to generate things like a dialog box that has
    tabbed pages you can flip between. Attach a ChangeListener to the
    button bar to be told when the user changes the page.

    An easier method than doing this is to use a TabbedComponent, which
    contains its own TabbedButtonBar and which takes care of the layout
    and other housekeeping.

    @see TabbedComponent

    @tags{GUI}
*/
class DRX_API  TabbedButtonBar  : public Component,
                                   public ChangeBroadcaster
{
public:
    //==============================================================================
    /** The placement of the tab-bar
        @see setOrientation, getOrientation
    */
    enum Orientation
    {
        TabsAtTop,
        TabsAtBottom,
        TabsAtLeft,
        TabsAtRight
    };

    //==============================================================================
    /** Creates a TabbedButtonBar with a given orientation.
        You can change the orientation later if you need to.
    */
    TabbedButtonBar (Orientation orientation);

    /** Destructor. */
    ~TabbedButtonBar() override;

    //==============================================================================
    /** Changes the bar's orientation.

        This won't change the bar's actual size - you'll need to do that yourself,
        but this determines which direction the tabs go in, and which side they're
        stuck to.
    */
    z0 setOrientation (Orientation orientation);

    /** Returns the bar's current orientation.
        @see setOrientation
    */
    Orientation getOrientation() const noexcept         { return orientation; }

    /** Возвращает true, если the orientation is TabsAtLeft or TabsAtRight. */
    b8 isVertical() const noexcept                    { return orientation == TabsAtLeft || orientation == TabsAtRight; }

    /** Returns the thickness of the bar, which may be its width or height, depending on the orientation. */
    i32 getThickness() const noexcept                   { return isVertical() ? getWidth() : getHeight(); }

    /** Changes the minimum scale factor to which the tabs can be compressed when trying to
        fit a lot of tabs on-screen.
    */
    z0 setMinimumTabScaleFactor (f64 newMinimumScale);

    //==============================================================================
    /** Deletes all the tabs from the bar.
        @see addTab
    */
    z0 clearTabs();

    /** Adds a tab to the bar.
        Tabs are added in left-to-right reading order.
        If this is the first tab added, it'll also be automatically selected.
    */
    z0 addTab (const Txt& tabName,
                 Color tabBackgroundColor,
                 i32 insertIndex);

    /** Changes the name of one of the tabs. */
    z0 setTabName (i32 tabIndex, const Txt& newName);

    /** Gets rid of one of the tabs. */
    z0 removeTab (i32 tabIndex, b8 animate = false);

    /** Moves a tab to a new index in the list.
        Pass -1 as the index to move it to the end of the list.
    */
    z0 moveTab (i32 currentIndex, i32 newIndex, b8 animate = false);

    /** Returns the number of tabs in the bar. */
    i32 getNumTabs() const;

    /** Returns a list of all the tab names in the bar. */
    StringArray getTabNames() const;

    /** Changes the currently selected tab.
        This will send a change message and cause a synchronous callback to
        the currentTabChanged() method. (But if the given tab is already selected,
        nothing will be done).

        To deselect all the tabs, use an index of -1.
    */
    z0 setCurrentTabIndex (i32 newTabIndex, b8 sendChangeMessage = true);

    /** Returns the name of the currently selected tab.
        This could be an empty string if none are selected.
    */
    Txt getCurrentTabName() const;

    /** Returns the index of the currently selected tab.
        This could return -1 if none are selected.
    */
    i32 getCurrentTabIndex() const noexcept             { return currentTabIndex; }

    /** Returns the button for a specific tab.
        The button that is returned may be deleted later by this component, so don't hang
        on to the pointer that is returned. A null pointer may be returned if the index is
        out of range.
    */
    TabBarButton* getTabButton (i32 index) const;

    /** Returns the index of a TabBarButton if it belongs to this bar. */
    i32 indexOfTabButton (const TabBarButton* button) const;

    /** Returns the final bounds of this button if it is currently being animated. */
    Rectangle<i32> getTargetBounds (TabBarButton* button) const;

    //==============================================================================
    /** Callback method to indicate the selected tab has been changed.
        @see setCurrentTabIndex
    */
    virtual z0 currentTabChanged (i32 newCurrentTabIndex,
                                    const Txt& newCurrentTabName);

    /** Callback method to indicate that the user has right-clicked on a tab. */
    virtual z0 popupMenuClickOnTab (i32 tabIndex, const Txt& tabName);

    /** Returns the colour of a tab.
        This is the colour that was specified in addTab().
    */
    Color getTabBackgroundColor (i32 tabIndex);

    /** Changes the background colour of a tab.
        @see addTab, getTabBackgroundColor
    */
    z0 setTabBackgroundColor (i32 tabIndex, Color newColor);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        tabOutlineColorId              = 0x1005812,    /**< The colour to use to draw an outline around the tabs.  */
        tabTextColorId                 = 0x1005813,    /**< The colour to use to draw the tab names. If this isn't specified,
                                                             the look and feel will choose an appropriate colour. */
        frontOutlineColorId            = 0x1005814,    /**< The colour to use to draw an outline around the currently-selected tab.  */
        frontTextColorId               = 0x1005815,    /**< The colour to use to draw the currently-selected tab name. If
                                                             this isn't specified, the look and feel will choose an appropriate
                                                             colour. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        window drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual i32 getTabButtonSpaceAroundImage() = 0;
        virtual i32 getTabButtonOverlap (i32 tabDepth) = 0;
        virtual i32 getTabButtonBestWidth (TabBarButton&, i32 tabDepth) = 0;
        virtual Rectangle<i32> getTabButtonExtraComponentBounds (const TabBarButton&, Rectangle<i32>& textArea, Component& extraComp) = 0;

        virtual z0 drawTabButton (TabBarButton&, Graphics&, b8 isMouseOver, b8 isMouseDown) = 0;
        virtual Font getTabButtonFont (TabBarButton&, f32 height) = 0;
        virtual z0 drawTabButtonText (TabBarButton&, Graphics&, b8 isMouseOver, b8 isMouseDown) = 0;
        virtual z0 drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics&) = 0;
        virtual z0 drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, i32 w, i32 h) = 0;

        virtual z0 createTabButtonShape (TabBarButton&, Path& path,  b8 isMouseOver, b8 isMouseDown) = 0;
        virtual z0 fillTabButtonShape (TabBarButton&, Graphics&, const Path& path, b8 isMouseOver, b8 isMouseDown) = 0;

        virtual Button* createTabBarExtrasButton() = 0;
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
    /** This creates one of the tabs.

        If you need to use custom tab components, you can override this method and
        return your own class instead of the default.
    */
    virtual TabBarButton* createTabButton (const Txt& tabName, i32 tabIndex);

private:
    struct TabInfo
    {
        std::unique_ptr<TabBarButton> button;
        Txt name;
        Color colour;
    };

    OwnedArray<TabInfo> tabs;

    Orientation orientation;
    f64 minimumScale = 0.7;
    i32 currentTabIndex = -1;

    class BehindFrontTabComp;
    std::unique_ptr<BehindFrontTabComp> behindFrontTab;
    std::unique_ptr<Button> extraTabsButton;

    z0 showExtraItemsMenu();
    z0 updateTabPositions (b8 animate);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabbedButtonBar)
};

} // namespace drx
