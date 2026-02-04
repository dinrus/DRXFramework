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
    A component with a set of buttons at the top for changing between pages of
    preferences.

    This is just a handy way of writing a Mac-style preferences panel where you
    have a row of buttons along the top for the different preference categories,
    each button having an icon above its name. Clicking these will show an
    appropriate prefs page below it.

    You can either put one of these inside your own component, or just use the
    showInDialogBox() method to show it in a window and run it modally.

    To use it, just add a set of named pages with the addSettingsPage() method,
    and implement the createComponentForPage() method to create suitable components
    for each of these pages.

    @tags{GUI}
*/
class DRX_API  PreferencesPanel  : public Component
{
public:
    //==============================================================================
    /** Creates an empty panel.

        Use addSettingsPage() to add some pages to it in your constructor.
    */
    PreferencesPanel();

    /** Destructor. */
    ~PreferencesPanel() override;

    //==============================================================================
    /** Creates a page using a set of drawables to define the page's icon.

        Note that the other version of this method is much easier if you're using
        an image instead of a custom drawable.

        @param pageTitle    the name of this preferences page - you'll need to
                            make sure your createComponentForPage() method creates
                            a suitable component when it is passed this name
        @param normalIcon   the drawable to display in the page's button normally
        @param overIcon     the drawable to display in the page's button when the mouse is over
        @param downIcon     the drawable to display in the page's button when the button is down
        @see DrawableButton
    */
    z0 addSettingsPage (const Txt& pageTitle,
                          const Drawable* normalIcon,
                          const Drawable* overIcon,
                          const Drawable* downIcon);

    /** Creates a page using a set of drawables to define the page's icon.

        The other version of this method gives you more control over the icon, but this
        one is much easier if you're just loading it from a file.

        @param pageTitle        the name of this preferences page - you'll need to
                                make sure your createComponentForPage() method creates
                                a suitable component when it is passed this name
        @param imageData        a block of data containing an image file, e.g. a jpeg, png or gif.
                                For this to look good, you'll probably want to use a nice
                                transparent png file.
        @param imageDataSize    the size of the image data, in bytes
    */
    z0 addSettingsPage (const Txt& pageTitle,
                          ukk imageData,
                          i32 imageDataSize);

    /** Utility method to display this panel in a DialogWindow.

        Calling this will create a DialogWindow containing this panel with the
        given size and title, and will run it modally, returning when the user
        closes the dialog box.
    */
    z0 showInDialogBox (const Txt& dialogTitle,
                          i32 dialogWidth,
                          i32 dialogHeight,
                          Color backgroundColor = Colors::white);

    //==============================================================================
    /** Subclasses must override this to return a component for each preferences page.

        The subclass should return a pointer to a new component representing the named
        page, which the panel will then display.

        The panel will delete the component later when the user goes to another page
        or deletes the panel.
    */
    virtual Component* createComponentForPage (const Txt& pageName) = 0;

    //==============================================================================
    /** Changes the current page being displayed. */
    z0 setCurrentPage (const Txt& pageName);

    /** Returns the size of the buttons shown along the top. */
    i32 getButtonSize() const noexcept;

    /** Changes the size of the buttons shown along the top. */
    z0 setButtonSize (i32 newSize);

    //==============================================================================
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 paint (Graphics&) override;

private:
    //==============================================================================
    Txt currentPageName;
    std::unique_ptr<Component> currentPage;
    OwnedArray<DrawableButton> buttons;
    i32 buttonSize;

    z0 clickedPage();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreferencesPanel)
};

} // namespace drx
