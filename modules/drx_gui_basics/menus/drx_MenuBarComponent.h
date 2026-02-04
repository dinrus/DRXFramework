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
    A menu bar component.

    @see MenuBarModel

    @tags{GUI}
*/
class DRX_API  MenuBarComponent  : public Component,
                                    private MenuBarModel::Listener,
                                    private Timer
{
public:
    //==============================================================================
    /** Creates a menu bar.

        @param model    the model object to use to control this bar. You can
                        pass omit the parameter or pass nullptr into this if you like,
                        and set the model later using the setModel() method.
    */
    MenuBarComponent (MenuBarModel* model = nullptr);

    /** Destructor. */
    ~MenuBarComponent() override;

    //==============================================================================
    /** Changes the model object to use to control the bar.

        This can be a null pointer, in which case the bar will be empty. Don't delete the object
        that is passed-in while it's still being used by this MenuBar.
    */
    z0 setModel (MenuBarModel* newModel);

    /** Returns the current menu bar model being used. */
    MenuBarModel* getModel() const noexcept;

    //==============================================================================
    /** Pops up one of the menu items.

        This lets you manually open one of the menus - it could be triggered by a
        key shortcut, for example.
    */
    z0 showMenu (i32 menuIndex);

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 mouseEnter (const MouseEvent&) override;
    /** @internal */
    z0 mouseExit (const MouseEvent&) override;
    /** @internal */
    z0 mouseDown (const MouseEvent&) override;
    /** @internal */
    z0 mouseDrag (const MouseEvent&) override;
    /** @internal */
    z0 mouseUp (const MouseEvent&) override;
    /** @internal */
    z0 mouseMove (const MouseEvent&) override;
    /** @internal */
    z0 handleCommandMessage (i32 commandId) override;
    /** @internal */
    b8 keyPressed (const KeyPress&) override;
    /** @internal */
    z0 menuBarItemsChanged (MenuBarModel*) override;
    /** @internal */
    z0 menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo&) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    class AccessibleItemComponent;

    //==============================================================================
    z0 timerCallback() override;

    i32 getItemAt (Point<i32>);
    z0 setItemUnderMouse (i32);
    z0 setOpenItem (i32);
    z0 updateItemUnderMouse (Point<i32>);
    z0 repaintMenuItem (i32);
    z0 menuDismissed (i32, i32);

    z0 updateItemComponents (const StringArray&);
    i32 indexOfItemComponent (AccessibleItemComponent*) const;

    //==============================================================================
    MenuBarModel* model = nullptr;
    std::vector<std::unique_ptr<AccessibleItemComponent>> itemComponents;

    Point<i32> lastMousePos;
    i32 itemUnderMouse = -1, currentPopupIndex = -1, topLevelIndexDismissed = 0;
    i32 numActiveMenus = 0;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuBarComponent)
};

} // namespace drx
