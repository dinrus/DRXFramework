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
    A base class for a component that goes in a PropertyPanel and displays one of
    an item's properties.

    Subclasses of this are used to display a property in various forms, e.g. a
    ChoicePropertyComponent shows its value as a combo box; a SliderPropertyComponent
    shows its value as a slider; a TextPropertyComponent as a text box, etc.

    A subclass must implement the refresh() method which will be called to tell the
    component to update itself, and is also responsible for calling this it when the
    item that it refers to is changed.

    @see PropertyPanel, TextPropertyComponent, SliderPropertyComponent,
         ChoicePropertyComponent, ButtonPropertyComponent, BooleanPropertyComponent

    @tags{GUI}
*/
class DRX_API  PropertyComponent  : public Component,
                                     public SettableTooltipClient
{
public:
    //==============================================================================
    /** Creates a PropertyComponent.

        @param propertyName     the name is stored as this component's name, and is
                                used as the name displayed next to this component in
                                a property panel
        @param preferredHeight  the height that the component should be given - some
                                items may need to be larger than a normal row height.
                                This value can also be set if a subclass changes the
                                preferredHeight member variable.
    */
    PropertyComponent (const Txt& propertyName,
                       i32 preferredHeight = 25);

    /** Destructor. */
    ~PropertyComponent() override;

    //==============================================================================
    /** Returns this item's preferred height.

        This value is specified either in the constructor or by a subclass changing the
        preferredHeight member variable.
    */
    i32 getPreferredHeight() const noexcept                 { return preferredHeight; }

    z0 setPreferredHeight (i32 newHeight) noexcept        { preferredHeight = newHeight; }

    //==============================================================================
    /** Updates the property component if the item it refers to has changed.

        A subclass must implement this method, and other objects may call it to
        force it to refresh itself.

        The subclass should be economical in the amount of work is done, so for
        example it should check whether it really needs to do a repaint rather than
        just doing one every time this method is called, as it may be called when
        the value being displayed hasn't actually changed.
    */
    virtual z0 refresh() = 0;


    /** The default paint method fills the background and draws a label for the
        item's name.

        @see LookAndFeel::drawPropertyComponentBackground(), LookAndFeel::drawPropertyComponentLabel()
    */
    z0 paint (Graphics&) override;

    /** The default resize method positions any child component to the right of this
        one, based on the look and feel's default label size.
    */
    z0 resized() override;

    /** By default, this just repaints the component. */
    z0 enablementChanged() override;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the combo box.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId     = 0x1008300,    /**< The background colour to fill the component with. */
        labelTextColorId      = 0x1008301,    /**< The colour for the property's label text. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual z0 drawPropertyPanelSectionHeader (Graphics&, const Txt& name, b8 isOpen, i32 width, i32 height) = 0;
        virtual z0 drawPropertyComponentBackground (Graphics&, i32 width, i32 height, PropertyComponent&) = 0;
        virtual z0 drawPropertyComponentLabel (Graphics&, i32 width, i32 height, PropertyComponent&) = 0;
        virtual Rectangle<i32> getPropertyComponentContentPosition (PropertyComponent&) = 0;
        virtual i32 getPropertyPanelSectionHeaderHeight (const Txt& sectionTitle) = 0;
    };

protected:
    /** Used by the PropertyPanel to determine how high this component needs to be.
        A subclass can update this value in its constructor but shouldn't alter it later
        as changes won't necessarily be picked up.
    */
    i32 preferredHeight;

private:
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyComponent)
};

} // namespace drx
