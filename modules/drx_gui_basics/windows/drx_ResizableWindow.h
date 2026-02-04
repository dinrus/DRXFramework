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
    A base class for top-level windows that can be dragged around and resized.

    To add content to the window, use its setContentOwned() or setContentNonOwned() methods
    to give it a component that will remain positioned inside it (leaving a gap around
    the edges for a border).

    It's not advisable to add child components directly to a ResizableWindow: put them
    inside your content component instead. And overriding methods like resized(), moved(), etc
    is also not recommended - instead override these methods for your content component.
    (If for some obscure reason you do need to override these methods, always remember to
    call the super-class's resized() method too, otherwise it'll fail to lay out the window
    decorations correctly).

    By default resizing isn't enabled - use the setResizable() method to enable it and
    to choose the style of resizing to use.

    @see TopLevelWindow

    @tags{GUI}
*/
class DRX_API  ResizableWindow  : public TopLevelWindow
{
public:
    //==============================================================================
    /** Creates a ResizableWindow.

        This constructor doesn't specify a background colour, so the LookAndFeel's default
        background colour will be used.

        @param name                 the name to give the component
        @param addToDesktop         if true, the window will be automatically added to the
                                    desktop; if false, you can use it as a child component
    */
    ResizableWindow (const Txt& name,
                     b8 addToDesktop);

    /** Creates a ResizableWindow.

        @param name                 the name to give the component
        @param backgroundColor     the colour to use for filling the window's background.
        @param addToDesktop         if true, the window will be automatically added to the
                                    desktop; if false, you can use it as a child component
    */
    ResizableWindow (const Txt& name,
                     Color backgroundColor,
                     b8 addToDesktop);

    /** Destructor.
        If a content component has been set with setContentOwned(), it will be deleted.
    */
    ~ResizableWindow() override;

    //==============================================================================
    /** Returns the colour currently being used for the window's background.

        As a convenience the window will fill itself with this colour, but you
        can override the paint() method if you need more customised behaviour.

        This method is the same as retrieving the colour for ResizableWindow::backgroundColorId.

        @see setBackgroundColor
    */
    Color getBackgroundColor() const noexcept;

    /** Changes the colour currently being used for the window's background.

        As a convenience the window will fill itself with this colour, but you
        can override the paint() method if you need more customised behaviour.

        Note that the opaque state of this window is altered by this call to reflect
        the opacity of the colour passed-in. On window systems which can't support
        semi-transparent windows this might cause problems, (though it's unlikely you'll
        be using this class as a base for a semi-transparent component anyway).

        You can also use the ResizableWindow::backgroundColorId colour id to set
        this colour.

        @see getBackgroundColor
    */
    z0 setBackgroundColor (Color newColor);

    //==============================================================================
    /** Make the window resizable or fixed.

        @param shouldBeResizable            whether it's resizable at all
        @param useBottomRightCornerResizer  if true, it'll add a ResizableCornerComponent at the
                                            bottom-right; if false, it'll use a ResizableBorderComponent
                                            around the edge
        @see setResizeLimits, isResizable
    */
    z0 setResizable (b8 shouldBeResizable,
                       b8 useBottomRightCornerResizer);

    /** Возвращает true, если resizing is enabled.
        @see setResizable
    */
    b8 isResizable() const noexcept;

    /** This sets the maximum and minimum sizes for the window.

        If the window's current size is outside these limits, it will be resized to
        make sure it's within them.

        A direct call to setBounds() will bypass any constraint checks, but when the
        window is dragged by the user or resized by other indirect means, the constrainer
        will limit the numbers involved.

        @see setResizable, setFixedAspectRatio
    */
    z0 setResizeLimits (i32 newMinimumWidth,
                          i32 newMinimumHeight,
                          i32 newMaximumWidth,
                          i32 newMaximumHeight) noexcept;

    /** Can be used to enable or disable user-dragging of the window. */
    z0 setDraggable (b8 shouldBeDraggable) noexcept;

    /** Возвращает true, если the window can be dragged around by the user. */
    b8 isDraggable() const noexcept                               { return canDrag; }

    /** Returns the bounds constrainer object that this window is using.
        You can access this to change its properties.
    */
    ComponentBoundsConstrainer* getConstrainer() noexcept           { return constrainer; }

    /** Sets the bounds-constrainer object to use for resizing and dragging this window.

        A pointer to the object you pass in will be kept, but it won't be deleted
        by this object, so it's the caller's responsibility to manage it.

        If you pass a nullptr, then no constraints will be placed on the positioning of the window.
    */
    z0 setConstrainer (ComponentBoundsConstrainer* newConstrainer);

    /** Calls the window's setBounds method, after first checking these bounds
        with the current constrainer.
        @see setConstrainer
    */
    z0 setBoundsConstrained (const Rectangle<i32>& newBounds);


    //==============================================================================
    /** Возвращает true, если the window is currently in full-screen mode.
        @see setFullScreen
    */
    b8 isFullScreen() const;

    /** Puts the window into full-screen mode, or restores it to its normal size.

        If true, the window will become full-screen; if false, it will return to the
        last size it was before being made full-screen.

        @see isFullScreen
    */
    z0 setFullScreen (b8 shouldBeFullScreen);

    /** Возвращает true, если the window is currently minimised.
        @see setMinimised
    */
    b8 isMinimised() const;

    /** Minimises the window, or restores it to its previous position and size.

        When being un-minimised, it'll return to the last position and size it
        was in before being minimised.

        @see isMinimised
    */
    z0 setMinimised (b8 shouldMinimise);

    /** Возвращает true, если the window has been placed in kiosk-mode.
        @see Desktop::setKioskComponent
    */
    b8 isKioskMode() const;

    //==============================================================================
    /** Returns a string which encodes the window's current size and position.

        This string will encapsulate the window's size, position, and whether it's
        in full-screen mode. It's intended for letting your application save and
        restore a window's position.

        Use the restoreWindowStateFromString() to restore from a saved state.

        @see restoreWindowStateFromString
    */
    Txt getWindowStateAsString();

    /** Restores the window to a previously-saved size and position.

        This restores the window's size, position and full-screen status from an
        string that was previously created with the getWindowStateAsString()
        method.

        @returns false if the string wasn't a valid window state
        @see getWindowStateAsString
    */
    b8 restoreWindowStateFromString (const Txt& previousState);


    //==============================================================================
    /** Returns the current content component.

        This will be the component set by setContentOwned() or setContentNonOwned, or
        nullptr if none has yet been specified.

        @see setContentOwned, setContentNonOwned
    */
    Component* getContentComponent() const noexcept                 { return contentComponent; }

    /** Changes the current content component.

        This sets a component that will be placed in the centre of the ResizableWindow,
        (leaving a space around the edge for the border).

        You should never add components directly to a ResizableWindow (or any of its subclasses)
        with addChildComponent(). Instead, add them to the content component.

        @param newContentComponent  the new component to use - this component will be deleted when it's
                                    no longer needed (i.e. when the window is deleted or a new content
                                    component is set for it). To set a component that this window will not
                                    delete, call setContentNonOwned() instead.
        @param resizeToFitWhenContentChangesSize  if true, then the ResizableWindow will maintain its size
                                    such that it always fits around the size of the content component. If false,
                                    the new content will be resized to fit the current space available.
    */
    z0 setContentOwned (Component* newContentComponent,
                          b8 resizeToFitWhenContentChangesSize);

    /** Changes the current content component.

        This sets a component that will be placed in the centre of the ResizableWindow,
        (leaving a space around the edge for the border).

        You should never add components directly to a ResizableWindow (or any of its subclasses)
        with addChildComponent(). Instead, add them to the content component.

        @param newContentComponent  the new component to use - this component will NOT be deleted by this
                                    component, so it's the caller's responsibility to manage its lifetime (it's
                                    ok to delete it while this window is still using it). To set a content
                                    component that the window will delete, call setContentOwned() instead.
        @param resizeToFitWhenContentChangesSize  if true, then the ResizableWindow will maintain its size
                                    such that it always fits around the size of the content component. If false,
                                    the new content will be resized to fit the current space available.
    */
    z0 setContentNonOwned (Component* newContentComponent,
                             b8 resizeToFitWhenContentChangesSize);

    /** Removes the current content component.
        If the previous content component was added with setContentOwned(), it will also be deleted. If
        it was added with setContentNonOwned(), it will simply be removed from this component.
    */
    z0 clearContentComponent();

    /** Changes the window so that the content component ends up with the specified size.

        This is basically a setSize call on the window, but which adds on the borders,
        so you can specify the content component's target size.
    */
    z0 setContentComponentSize (i32 width, i32 height);

    /** Returns the width of the frame to use around the window.
        @see getContentComponentBorder
    */
    virtual BorderSize<i32> getBorderThickness() const;

    /** Returns the insets to use when positioning the content component.
        @see getBorderThickness
    */
    virtual BorderSize<i32> getContentComponentBorder() const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the window.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId          = 0x1005700,  /**< A colour to use to fill the window's background. */
    };

    //==============================================================================
   #ifndef DOXYGEN
    [[deprecated ("use setContentOwned and setContentNonOwned instead.")]]
    z0 setContentComponent (Component* newContentComponent,
                              b8 deleteOldOne = true,
                              b8 resizeToFit = false);
   #endif

    using TopLevelWindow::addToDesktop;

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        window drawing functionality.
    */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        //==============================================================================
        virtual z0 drawCornerResizer (Graphics&, i32 w, i32 h, b8 isMouseOver, b8 isMouseDragging) = 0;
        virtual z0 drawResizableFrame (Graphics&, i32 w, i32 h, const BorderSize<i32>&) = 0;

        virtual z0 fillResizableWindowBackground (Graphics&, i32 w, i32 h, const BorderSize<i32>&, ResizableWindow&) = 0;
        virtual z0 drawResizableWindowBorder (Graphics&, i32 w, i32 h, const BorderSize<i32>& border, ResizableWindow&) = 0;
    };

protected:
    /** @internal */
    z0 paint (Graphics&) override;
    /** (if overriding this, make sure you call ResizableWindow::moved() in your subclass) */
    z0 moved() override;
    /** (if overriding this, make sure you call ResizableWindow::resized() in your subclass) */
    z0 resized() override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    z0 childBoundsChanged (Component*) override;
    /** @internal */
    z0 parentSizeChanged() override;
    /** @internal */
    z0 visibilityChanged() override;
    /** @internal */
    z0 activeWindowStatusChanged() override;
    /** @internal */
    i32 getDesktopWindowStyleFlags() const override;

   #if DRX_DEBUG
    /** Overridden to warn people about adding components directly to this component
        instead of using setContentOwned().

        If you know what you're doing and are sure you really want to add a component, specify
        a base-class method call to Component::addAndMakeVisible(), to side-step this warning.
    */
    z0 addChildComponent (Component*, i32 zOrder = -1);
    /** Overridden to warn people about adding components directly to this component
        instead of using setContentOwned().

        If you know what you're doing and are sure you really want to add a component, specify
        a base-class method call to Component::addAndMakeVisible(), to side-step this warning.
    */
    z0 addAndMakeVisible (Component*, i32 zOrder = -1);
   #endif

    std::unique_ptr<ResizableCornerComponent> resizableCorner;
    std::unique_ptr<ResizableBorderComponent> resizableBorder;

    //==============================================================================
    // The parameters for these methods have changed - please update your code!
    z0 getBorderThickness (i32& left, i32& top, i32& right, i32& bottom);
    z0 getContentComponentBorder (i32& left, i32& top, i32& right, i32& bottom);

private:
    //==============================================================================
    Component::SafePointer<Component> contentComponent;
    b8 ownsContentComponent = false;
    b8 resizeToFitContent = false;
    b8 fullscreen = false;
    b8 canDrag = true;
    b8 dragStarted = false;
    b8 resizable = false;
    ComponentDragger dragger;
    Rectangle<i32> lastNonFullScreenPos;
    ComponentBoundsConstrainer defaultConstrainer;
    ComponentBoundsConstrainer* constrainer = nullptr;
   #if DRX_DEBUG
    b8 hasBeenResized = false;
   #endif

    z0 initialise (b8 addToDesktop);
    z0 updateLastPosIfNotFullScreen();
    z0 updateLastPosIfShowing();
    z0 setContent (Component*, b8 takeOwnership, b8 resizeToFit);
    z0 updatePeerConstrainer();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizableWindow)
};

} // namespace drx
