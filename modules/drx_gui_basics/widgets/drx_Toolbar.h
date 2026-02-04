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

class ToolbarItemComponent;
class ToolbarItemFactory;


//==============================================================================
/**
    A toolbar component.

    A toolbar contains a horizontal or vertical strip of ToolbarItemComponents,
    and looks after their order and layout.

    Items (icon buttons or other custom components) are added to a toolbar using a
    ToolbarItemFactory - each type of item is given a unique ID number, and a
    toolbar might contain more than one instance of a particular item type.

    Toolbars can be interactively customised, allowing the user to drag the items
    around, and to drag items onto or off the toolbar, using the ToolbarItemPalette
    component as a source of new items.

    @see ToolbarItemFactory, ToolbarItemComponent, ToolbarItemPalette

    @tags{GUI}
*/
class DRX_API  Toolbar   : public Component,
                            public DragAndDropContainer,
                            public DragAndDropTarget
{
public:
    //==============================================================================
    /** Creates an empty toolbar component.

        To add some icons or other components to your toolbar, you'll need to
        create a ToolbarItemFactory class that can create a suitable set of
        ToolbarItemComponents.

        @see ToolbarItemFactory, ToolbarItemComponents
    */
    Toolbar();

    /** Destructor.

        Any items on the bar will be deleted when the toolbar is deleted.
    */
    ~Toolbar() override;

    //==============================================================================
    /** Changes the bar's orientation.
        @see isVertical
    */
    z0 setVertical (b8 shouldBeVertical);

    /** Возвращает true, если the bar is set to be vertical, or false if it's horizontal.

        You can change the bar's orientation with setVertical().
    */
    b8 isVertical() const noexcept                 { return vertical; }

    /** Returns the depth of the bar.

        If the bar is horizontal, this will return its height; if it's vertical, it
        will return its width.

        @see getLength
    */
    i32 getThickness() const noexcept;

    /** Returns the length of the bar.

        If the bar is horizontal, this will return its width; if it's vertical, it
        will return its height.

        @see getThickness
    */
    i32 getLength() const noexcept;

    //==============================================================================
    /** Deletes all items from the bar.
    */
    z0 clear();

    /** Adds an item to the toolbar.

        The factory's ToolbarItemFactory::createItem() will be called by this method
        to create the component that will actually be added to the bar.

        The new item will be inserted at the specified index (if the index is -1, it
        will be added to the right-hand or bottom end of the bar).

        Once added, the component will be automatically deleted by this object when it
        is no longer needed.

        @see ToolbarItemFactory
    */
    z0 addItem (ToolbarItemFactory& factory,
                  i32 itemId,
                  i32 insertIndex = -1);

    /** Deletes one of the items from the bar. */
    z0 removeToolbarItem (i32 itemIndex);

    /** Removes an item from the bar and returns it. */
    ToolbarItemComponent* removeAndReturnItem (i32 itemIndex);

    /** Returns the number of items currently on the toolbar.

        @see getItemId, getItemComponent
    */
    i32 getNumItems() const noexcept;

    /** Returns the ID of the item with the given index.

        If the index is less than zero or greater than the number of items,
        this will return nullptr.

        @see getNumItems
    */
    i32 getItemId (i32 itemIndex) const noexcept;

    /** Returns the component being used for the item with the given index.

        If the index is less than zero or greater than the number of items,
        this will return nullptr.

        @see getNumItems
    */
    ToolbarItemComponent* getItemComponent (i32 itemIndex) const noexcept;

    /** Clears this toolbar and adds to it the default set of items that the specified
        factory creates.

        @see ToolbarItemFactory::getDefaultItemSet
    */
    z0 addDefaultItems (ToolbarItemFactory& factoryToUse);

    //==============================================================================
    /** Options for the way items should be displayed.
        @see setStyle, getStyle
    */
    enum ToolbarItemStyle
    {
        iconsOnly,       /**< Means that the toolbar should just contain icons. */
        iconsWithText,   /**< Means that the toolbar should have text labels under each icon. */
        textOnly         /**< Means that the toolbar only display text labels for each item. */
    };

    /** Returns the toolbar's current style.
        @see ToolbarItemStyle, setStyle
    */
    ToolbarItemStyle getStyle() const noexcept               { return toolbarStyle; }

    /** Changes the toolbar's current style.
        @see ToolbarItemStyle, getStyle, ToolbarItemComponent::setStyle
    */
    z0 setStyle (const ToolbarItemStyle& newStyle);

    //==============================================================================
    /** Flags used by the showCustomisationDialog() method. */
    enum CustomisationFlags
    {
        allowIconsOnlyChoice            = 1,    /**< If this flag is specified, the customisation dialog can
                                                     show the "icons only" option on its choice of toolbar styles. */
        allowIconsWithTextChoice        = 2,    /**< If this flag is specified, the customisation dialog can
                                                     show the "icons with text" option on its choice of toolbar styles. */
        allowTextOnlyChoice             = 4,    /**< If this flag is specified, the customisation dialog can
                                                     show the "text only" option on its choice of toolbar styles. */
        showResetToDefaultsButton       = 8,    /**< If this flag is specified, the customisation dialog can
                                                     show a button to reset the toolbar to its default set of items. */

        allCustomisationOptionsEnabled = (allowIconsOnlyChoice | allowIconsWithTextChoice | allowTextOnlyChoice | showResetToDefaultsButton)
    };

    /** Pops up a modal dialog box that allows this toolbar to be customised by the user.

        The dialog contains a ToolbarItemPalette and various controls for editing other
        aspects of the toolbar. The dialog box will be opened modally, but the method will
        return immediately.

        The factory is used to determine the set of items that will be shown on the
        palette.

        The optionFlags parameter is a bitwise-or of values from the CustomisationFlags
        enum.

        @see ToolbarItemPalette
    */
    z0 showCustomisationDialog (ToolbarItemFactory& factory,
                                  i32 optionFlags = allCustomisationOptionsEnabled);

    /** Turns on or off the toolbar's editing mode, in which its items can be
        rearranged by the user.

        (In most cases it's easier just to use showCustomisationDialog() instead of
        trying to enable editing directly).

        @see ToolbarItemPalette
    */
    z0 setEditingActive (b8 editingEnabled);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the toolbar.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId          = 0x1003200,  /**< A colour to use to fill the toolbar's background. For
                                                       more control over this, override LookAndFeel::paintToolbarBackground(). */
        separatorColorId           = 0x1003210,  /**< A colour to use to draw the separator lines. */

        buttonMouseOverBackgroundColorId = 0x1003220,  /**< A colour used to paint the background of buttons when the mouse is
                                                             over them. */
        buttonMouseDownBackgroundColorId = 0x1003230,  /**< A colour used to paint the background of buttons when the mouse is
                                                             held down on them. */

        labelTextColorId           = 0x1003240,        /**< A colour to use for drawing the text under buttons
                                                             when the style is set to iconsWithText or textOnly. */

        editingModeOutlineColorId  = 0x1003250,  /**< A colour to use for an outline around buttons when
                                                       the customisation dialog is active and the mouse moves over them. */

        customisationDialogBackgroundColorId = 0x1003260 /**< A colour used to paint the background of the CustomisationDialog. */
    };

    //==============================================================================
    /** Returns a string that represents the toolbar's current set of items.

        This lets you later restore the same item layout using restoreFromString().

        @see restoreFromString
    */
    Txt toString() const;

    /** Restores a set of items that was previously stored in a string by the toString()
        method.

        The factory object is used to create any item components that are needed.

        @see toString
    */
    b8 restoreFromString (ToolbarItemFactory& factoryToUse,
                            const Txt& savedVersion);

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 paintToolbarBackground (Graphics&, i32 width, i32 height, Toolbar&) = 0;

        virtual Button* createToolbarMissingItemsButton (Toolbar&) = 0;

        virtual z0 paintToolbarButtonBackground (Graphics&, i32 width, i32 height,
                                                   b8 isMouseOver, b8 isMouseDown,
                                                   ToolbarItemComponent&) = 0;

        virtual z0 paintToolbarButtonLabel (Graphics&, i32 x, i32 y, i32 width, i32 height,
                                              const Txt& text, ToolbarItemComponent&) = 0;
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    b8 isInterestedInDragSource (const SourceDetails&) override;
    /** @internal */
    z0 itemDragMove (const SourceDetails&) override;
    /** @internal */
    z0 itemDragExit (const SourceDetails&) override;
    /** @internal */
    z0 itemDropped (const SourceDetails&) override;
    /** @internal */
    z0 lookAndFeelChanged() override;
    /** @internal */
    z0 updateAllItemPositions (b8 animate);
    /** @internal */
    static ToolbarItemComponent* createItem (ToolbarItemFactory&, i32 itemId);
    /** @internal */
    static tukk const toolbarDragDescriptor;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    std::unique_ptr<Button> missingItemsButton;
    b8 vertical = false, isEditingActive = false;
    ToolbarItemStyle toolbarStyle = iconsOnly;
    class MissingItemsComponent;
    friend class MissingItemsComponent;
    OwnedArray<ToolbarItemComponent> items;
    class Spacer;
    class CustomisationDialog;

    z0 initMissingItemButton();
    z0 showMissingItems();
    z0 addItemInternal (ToolbarItemFactory& factory, i32 itemId, i32 insertIndex);

    ToolbarItemComponent* getNextActiveComponent (i32 index, i32 delta) const;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Toolbar)
};

} // namespace drx
