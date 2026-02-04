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
    A component that is positioned on either the left- or right-hand side of its parent,
    containing a header and some content. This sort of component is typically used for
    navigation and forms in mobile applications.

    When triggered with the showOrHide() method, the SidePanel will animate itself to its
    new position. This component also contains some logic to reactively resize and dismiss
    itself when the user drags it.

    @tags{GUI}
*/
class SidePanel    : public Component,
                     private ComponentListener,
                     private ChangeListener
{
public:
    //==============================================================================
    /** Creates a SidePanel component.

        @param title               the text to use for the SidePanel's title bar
        @param width               the width of the SidePanel
        @param positionOnLeft      if true, the SidePanel will be positioned on the left of its parent component and
                                   if false, the SidePanel will be positioned on the right of its parent component
        @param contentComponent    the component to add to this SidePanel - this content will take up the full
                                   size of the SidePanel, minus the height of the title bar. You can pass nullptr
                                   to this if you like and set the content component later using the setContent() method
        @param deleteComponentWhenNoLongerNeeded    if true, the component will be deleted automatically when
                                   the SidePanel is deleted or when a different component is added. If false,
                                   the caller must manage the lifetime of the component
    */
    SidePanel (StringRef title, i32 width, b8 positionOnLeft,
               Component* contentComponent = nullptr,
               b8 deleteComponentWhenNoLongerNeeded = true);

    /** Destructor */
    ~SidePanel() override;

    //==============================================================================
    /** Sets the component that this SidePanel will contain.

        This will add the given component to this SidePanel and position it below the title bar.

        (Don't add or remove any child components directly using the normal
        Component::addChildComponent() methods).

        @param newContentComponent   the component to add to this SidePanel, or nullptr to remove
                                     the current component.
        @param deleteComponentWhenNoLongerNeeded    if true, the component will be deleted automatically when
                                   the SidePanel is deleted or when a different component is added. If false,
                                   the caller must manage the lifetime of the component

        @see getContent
    */
    z0 setContent (Component* newContentComponent,
                     b8 deleteComponentWhenNoLongerNeeded = true);

    /** Returns the component that's currently being used inside the SidePanel.

        @see setViewedComponent
    */
    Component* getContent() const noexcept    { return contentComponent.get(); }

    /** Sets a custom component to be used for the title bar of this SidePanel, replacing
        the default. You can pass a nullptr to revert to the default title bar.

        @param titleBarComponentToUse  the component to use as the title bar, or nullptr to use
                                       the default
        @param keepDismissButton       if false the specified component will take up the full width of
                                       the title bar including the dismiss button but if true, the default
                                       dismiss button will be kept
        @param deleteComponentWhenNoLongerNeeded  if true, the component will be deleted automatically when
                                       the SidePanel is deleted or when a different component is added. If false,
                                       the caller must manage the lifetime of the component

        @see getTitleBarComponent
    */
    z0 setTitleBarComponent (Component* titleBarComponentToUse,
                               b8 keepDismissButton,
                               b8 deleteComponentWhenNoLongerNeeded = true);

    /** Returns the component that is currently being used as the title bar of the SidePanel.

        @see setTitleBarComponent
    */
    Component* getTitleBarComponent() const noexcept    { return titleBarComponent.get(); }

    /** Shows or hides the SidePanel.

        This will animate the SidePanel to either its full width or to be hidden on the
        left- or right-hand side of its parent component depending on the value of positionOnLeft
        that was passed to the constructor.

        @param show    if true, this will show the SidePanel and if false the SidePanel will be hidden
    */
    z0 showOrHide (b8 show);

    //==============================================================================
    /** Возвращает true, если the SidePanel is currently showing. */
    b8 isPanelShowing() const noexcept               { return isShowing; }

    /** Возвращает true, если the SidePanel is positioned on the left of its parent. */
    b8 isPanelOnLeft() const noexcept                { return isOnLeft; }

    /** Sets the width of the shadow that will be drawn on the side of the panel. */
    z0 setShadowWidth (i32 newWidth) noexcept        { shadowWidth = newWidth; }

    /** Returns the width of the shadow that will be drawn on the side of the panel. */
    i32 getShadowWidth() const noexcept                { return shadowWidth; }

    /** Sets the height of the title bar at the top of the SidePanel. */
    z0 setTitleBarHeight (i32 newHeight) noexcept    { titleBarHeight = newHeight; }

    /** Returns the height of the title bar at the top of the SidePanel. */
    i32 getTitleBarHeight() const noexcept             { return titleBarHeight; }

    /** Returns the text that is displayed in the title bar at the top of the SidePanel. */
    Txt getTitleText() const noexcept               { return titleLabel.getText(); }

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        SidePanel drawing functionality.
     */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual Font getSidePanelTitleFont (SidePanel&) = 0;
        virtual Justification getSidePanelTitleJustification (SidePanel&) = 0;
        virtual Path getSidePanelDismissButtonShape (SidePanel&) = 0;
    };

    /** A set of colour IDs to use to change the colour of various aspects of the SidePanel.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColor          = 0x100f001,
        titleTextColor           = 0x100f002,
        shadowBaseColor          = 0x100f003,
        dismissButtonNormalColor = 0x100f004,
        dismissButtonOverColor   = 0x100f005,
        dismissButtonDownColor   = 0x100f006
    };

    //==============================================================================
    /** You can assign a lambda to this callback object and it will be called when the panel is moved. */
    std::function<z0()> onPanelMove;

    /** You can assign a lambda to this callback object and it will be called when the panel is shown or hidden. */
    std::function<z0 (b8)> onPanelShowHide;

    //==============================================================================
    /** @internal */
    z0 moved() override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 paint (Graphics& g) override;
    /** @internal */
    z0 parentHierarchyChanged() override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Component* parent = nullptr;
    OptionalScopedPointer<Component> contentComponent;
    OptionalScopedPointer<Component> titleBarComponent;

    Label titleLabel;
    ShapeButton dismissButton { "dismissButton", Colors::lightgrey, Colors::lightgrey, Colors::white };

    Rectangle<i32> shadowArea;

    b8 isOnLeft = false;
    b8 isShowing = false;

    i32 panelWidth = 0;
    i32 shadowWidth = 15;
    i32 titleBarHeight = 40;

    Rectangle<i32> startingBounds;
    b8 shouldResize = false;
    i32 amountMoved = 0;

    b8 shouldShowDismissButton = true;

    //==============================================================================
    z0 lookAndFeelChanged() override;
    z0 componentMovedOrResized (Component&, b8 wasMoved, b8 wasResized) override;
    z0 changeListenerCallback (ChangeBroadcaster*) override;

    Rectangle<i32> calculateBoundsInParent (Component&) const;
    z0 calculateAndRemoveShadowBounds (Rectangle<i32>& bounds);

    b8 isMouseEventInThisOrChildren (Component*);

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SidePanel)
};

} // namespace drx
