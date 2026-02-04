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
    A type of button designed to go on a toolbar.

    This simple button can have two Drawable objects specified - one for normal
    use and another one (optionally) for the button's "on" state if it's a
    toggle button.

    @see Toolbar, ToolbarItemFactory, ToolbarItemComponent, Drawable, Button

    @tags{GUI}
*/
class DRX_API  ToolbarButton   : public ToolbarItemComponent
{
public:
    //==============================================================================
    /** Creates a ToolbarButton.

        @param itemId       the ID for this toolbar item type. This is passed through to the
                            ToolbarItemComponent constructor
        @param labelText    the text to display on the button (if the toolbar is using a style
                            that shows text labels). This is passed through to the
                            ToolbarItemComponent constructor
        @param normalImage  a drawable object that the button should use as its icon. The object
                            that is passed-in here will be kept by this object and will be
                            deleted when no longer needed or when this button is deleted.
        @param toggledOnImage  a drawable object that the button can use as its icon if the button
                            is in a toggled-on state (see the Button::getToggleState() method). If
                            nullptr is passed-in here, then the normal image will be used instead,
                            regardless of the toggle state. The object that is passed-in here will be
                            owned by this object and will be deleted when no longer needed or when
                            this button is deleted.
    */
    ToolbarButton (i32 itemId,
                   const Txt& labelText,
                   std::unique_ptr<Drawable> normalImage,
                   std::unique_ptr<Drawable> toggledOnImage);

    /** Destructor. */
    ~ToolbarButton() override;


    //==============================================================================
    /** @internal */
    b8 getToolbarItemSizes (i32 toolbarDepth, b8 isToolbarVertical, i32& preferredSize,
                              i32& minSize, i32& maxSize) override;
    /** @internal */
    z0 paintButtonArea (Graphics&, i32 width, i32 height, b8 isMouseOver, b8 isMouseDown) override;
    /** @internal */
    z0 contentAreaChanged (const Rectangle<i32>&) override;
    /** @internal */
    z0 buttonStateChanged() override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 enablementChanged() override;

private:
    //==============================================================================
    std::unique_ptr<Drawable> normalImage, toggledOnImage;
    Drawable* currentImage = nullptr;

    z0 updateDrawable();
    Drawable* getImageToUse() const;
    z0 setCurrentImage (Drawable*);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarButton)
};

} // namespace drx
