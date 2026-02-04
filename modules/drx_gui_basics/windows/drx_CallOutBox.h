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
    A box with a small arrow that can be used as a temporary pop-up window to show
    extra controls when a button or other component is clicked.

    Using one of these is similar to having a popup menu attached to a button or
    other component - but it looks fancier, and has an arrow that can indicate the
    object that it applies to.

    The class works best when shown modally, but obviously running modal loops is
    evil and must never be done, so the launchAsynchronously method is provided as
    a handy way of launching an instance of a CallOutBox and automatically managing
    its lifetime, e.g.

    @code
    z0 mouseUp (const MouseEvent&)
    {
        auto content = std::make_unique<FoobarContentComp>();
        content->setSize (300, 300);

        auto& myBox = CallOutBox::launchAsynchronously (std::move (content),
                                                        getScreenBounds(),
                                                        nullptr);
    }
    @endcode

    The call-out will resize and position itself when the content changes size.

    @tags{GUI}
*/
class DRX_API  CallOutBox    : public Component,
                                private Timer
{
public:
    //==============================================================================
    /** Creates a CallOutBox.

        @param contentComponent     the component to display inside the call-out. This should
                                    already have a size set (although the call-out will also
                                    update itself when the component's size is changed later).
                                    Obviously this component must not be deleted until the
                                    call-out box has been deleted.
        @param areaToPointTo        the area that the call-out's arrow should point towards. If
                                    a parentComponent is supplied, then this is relative to that
                                    parent; otherwise, it's a global screen coord.
        @param parentComponent      if not a nullptr, this is the component to add the call-out to.
                                    If this is a nullptr, the call-out will be added to the desktop.
    */
    CallOutBox (Component& contentComponent,
                Rectangle<i32> areaToPointTo,
                Component* parentComponent);

    //==============================================================================
    /** Changes the base width of the arrow. */
    z0 setArrowSize (f32 newSize);

    /** Updates the position and size of the box.

        You shouldn't normally need to call this, unless you need more precise control over the
        layout.

        @param newAreaToPointTo     the rectangle to make the box's arrow point to
        @param newAreaToFitIn       the area within which the box's position should be constrained
    */
    z0 updatePosition (const Rectangle<i32>& newAreaToPointTo,
                         const Rectangle<i32>& newAreaToFitIn);


    /** This will launch a callout box containing the given content, pointing to the
        specified target component.

        This method will create and display a callout, returning immediately, after which
        the box will continue to run modally until the user clicks on some other component, at
        which point it will be dismissed and deleted automatically.

        It returns a reference to the newly-created box so that you can customise it, but don't
        keep a pointer to it, as it'll be deleted at some point when it gets closed.

        @param contentComponent     the component to display inside the call-out. This should
                                    already have a size set (although the call-out will also
                                    update itself when the component's size is changed later).
        @param areaToPointTo        the area that the call-out's arrow should point towards. If
                                    a parentComponent is supplied, then this is relative to that
                                    parent; otherwise, it's a global screen coord.
        @param parentComponent      if not a nullptr, this is the component to add the call-out to.
                                    If this is a nullptr, the call-out will be added to the desktop.
    */
    static CallOutBox& launchAsynchronously (std::unique_ptr<Component> contentComponent,
                                             Rectangle<i32> areaToPointTo,
                                             Component* parentComponent);

    /** Posts a message which will dismiss the callout box asynchronously.
        NB: it's safe to call this method from any thread.
    */
    z0 dismiss();

    /** Determines whether the mouse events for clicks outside the calloutbox are
        consumed, or allowed to arrive at the other component that they were aimed at.

        By default this is false, so that when you click on something outside the calloutbox,
        that event will also be sent to the component that was clicked on. If you set it to
        true, then the first click will always just dismiss the box and not be sent to
        anything else.
    */
    z0 setDismissalMouseClicksAreAlwaysConsumed (b8 shouldAlwaysBeConsumed) noexcept;

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawCallOutBoxBackground (CallOutBox&, Graphics&, const Path&, Image&) = 0;
        virtual i32 getCallOutBoxBorderSize (const CallOutBox&) = 0;
        virtual f32 getCallOutBoxCornerSize (const CallOutBox&) = 0;
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 moved() override;
    /** @internal */
    z0 childBoundsChanged (Component*) override;
    /** @internal */
    b8 hitTest (i32 x, i32 y) override;
    /** @internal */
    z0 inputAttemptWhenModal() override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 handleCommandMessage (i32) override;
    /** @internal */
    i32 getBorderSize() const noexcept;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Component& content;
    Path outline;
    Point<f32> targetPoint;
    Rectangle<i32> availableArea, targetArea;
    Image background;
    f32 arrowSize = 16.0f;
    b8 dismissalMouseClicksAreAlwaysConsumed = false;

    Time creationTime;

    z0 refreshPath();
    z0 timerCallback() override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CallOutBox)
};

} // namespace drx
