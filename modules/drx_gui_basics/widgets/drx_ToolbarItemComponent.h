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
    A component that can be used as one of the items in a Toolbar.

    Each of the items on a toolbar must be a component derived from ToolbarItemComponent,
    and these objects are always created by a ToolbarItemFactory - see the ToolbarItemFactory
    class for further info about creating them.

    The ToolbarItemComponent class is actually a button, but can be used to hold non-button
    components too. To do this, set the value of isBeingUsedAsAButton to false when
    calling the constructor, and override contentAreaChanged(), in which you can position
    any sub-components you need to add.

    To add basic buttons without writing a special subclass, have a look at the
    ToolbarButton class.

    @see ToolbarButton, Toolbar, ToolbarItemFactory

    @tags{GUI}
*/
class DRX_API  ToolbarItemComponent  : public Button
{
public:
    //==============================================================================
    /** Constructor.

        @param itemId       the ID of the type of toolbar item which this represents
        @param labelText    the text to display if the toolbar's style is set to
                            Toolbar::iconsWithText or Toolbar::textOnly
        @param isBeingUsedAsAButton     set this to false if you don't want the button
                            to draw itself with button over/down states when the mouse
                            moves over it or clicks
    */
    ToolbarItemComponent (i32 itemId,
                          const Txt& labelText,
                          b8 isBeingUsedAsAButton);

    /** Destructor. */
    ~ToolbarItemComponent() override;

    //==============================================================================
    /** Returns the item type ID that this component represents.
        This value is in the constructor.
    */
    i32 getItemId() const noexcept                                  { return itemId; }

    /** Returns the toolbar that contains this component, or nullptr if it's not currently
        inside one.
    */
    Toolbar* getToolbar() const;

    /** Возвращает true, если this component is currently inside a toolbar which is vertical.
        @see Toolbar::isVertical
    */
    b8 isToolbarVertical() const;

    /** Returns the current style setting of this item.

        Styles are listed in the Toolbar::ToolbarItemStyle enum.
        @see setStyle, Toolbar::getStyle
    */
    Toolbar::ToolbarItemStyle getStyle() const noexcept             { return toolbarStyle; }

    /** Changes the current style setting of this item.

        Styles are listed in the Toolbar::ToolbarItemStyle enum, and are automatically updated
        by the toolbar that holds this item.

        @see setStyle, Toolbar::setStyle
    */
    virtual z0 setStyle (const Toolbar::ToolbarItemStyle& newStyle);

    /** Returns the area of the component that should be used to display the button image or
        other contents of the item.

        This content area may change when the item's style changes, and may leave a space around the
        edge of the component where the text label can be shown.

        @see contentAreaChanged
    */
    Rectangle<i32> getContentArea() const noexcept                  { return contentArea; }

    //==============================================================================
    /** This method must return the size criteria for this item, based on a given toolbar
        size and orientation.

        The preferredSize, minSize and maxSize values must all be set by your implementation
        method. If the toolbar is horizontal, these will be the width of the item; for a vertical
        toolbar, they refer to the item's height.

        The preferredSize is the size that the component would like to be, and this must be
        between the min and max sizes. For a fixed-size item, simply set all three variables to
        the same value.

        The toolbarThickness parameter tells you the depth of the toolbar - the same as calling
        Toolbar::getThickness().

        The isToolbarVertical parameter tells you whether the bar is oriented horizontally or
        vertically.
    */
    virtual b8 getToolbarItemSizes (i32 toolbarThickness,
                                      b8 isToolbarVertical,
                                      i32& preferredSize,
                                      i32& minSize,
                                      i32& maxSize) = 0;

    /** Your subclass should use this method to draw its content area.

        The graphics object that is passed-in will have been clipped and had its origin
        moved to fit the content area as specified get getContentArea(). The width and height
        parameters are the width and height of the content area.

        If the component you're writing isn't a button, you can just do nothing in this method.
    */
    virtual z0 paintButtonArea (Graphics& g,
                                  i32 width, i32 height,
                                  b8 isMouseOver, b8 isMouseDown) = 0;

    /** Callback to indicate that the content area of this item has changed.

        This might be because the component was resized, or because the style changed and
        the space needed for the text label is different.

        See getContentArea() for a description of what the area is.
    */
    virtual z0 contentAreaChanged (const Rectangle<i32>& newBounds) = 0;


    //==============================================================================
    /** Editing modes.
        These are used by setEditingMode(), but will be rarely needed in user code.
    */
    enum ToolbarEditingMode
    {
        normalMode = 0,     /**< Means that the component is active, inside a toolbar. */
        editableOnToolbar,  /**< Means that the component is on a toolbar, but the toolbar is in
                                 customisation mode, and the items can be dragged around. */
        editableOnPalette   /**< Means that the component is on an new-item palette, so it can be
                                 dragged onto a toolbar to add it to that bar.*/
    };

    /** Changes the editing mode of this component.

        This is used by the ToolbarItemPalette and related classes for making the items draggable,
        and is unlikely to be of much use in end-user-code.
    */
    z0 setEditingMode (ToolbarEditingMode newMode);

    /** Returns the current editing mode of this component.

        This is used by the ToolbarItemPalette and related classes for making the items draggable,
        and is unlikely to be of much use in end-user-code.
    */
    ToolbarEditingMode getEditingMode() const noexcept                  { return mode; }


    //==============================================================================
    /** @internal */
    z0 paintButton (Graphics&, b8 isMouseOver, b8 isMouseDown) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    friend class Toolbar;
    friend class detail::ToolbarItemDragAndDropOverlayComponent;

    i32k itemId;
    ToolbarEditingMode mode;
    Toolbar::ToolbarItemStyle toolbarStyle;
    std::unique_ptr<Component> overlayComp;
    i32 dragOffsetX, dragOffsetY;
    b8 isActive, isBeingDragged, isBeingUsedAsAButton;
    Rectangle<i32> contentArea;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarItemComponent)
};

} // namespace drx
