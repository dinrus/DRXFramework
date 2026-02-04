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
    A PropertyComponent that contains an on/off toggle button.

    This type of property component can be used if you have a boolean value to
    toggle on/off.

    @see PropertyComponent

    @tags{GUI}
*/
class DRX_API  BooleanPropertyComponent  : public PropertyComponent
{
protected:
    //==============================================================================
    /** Creates a button component.

        If you use this constructor, you must override the getState() and setState()
        methods.

        @param propertyName         the property name to be passed to the PropertyComponent
        @param buttonTextWhenTrue   the text shown in the button when the value is true
        @param buttonTextWhenFalse  the text shown in the button when the value is false
    */
    BooleanPropertyComponent (const Txt& propertyName,
                              const Txt& buttonTextWhenTrue,
                              const Txt& buttonTextWhenFalse);

public:
    /** Creates a button component.

        Note that if you call this constructor then you must use the Value to interact with the
        button state, and you can't override the class with your own setState or getState methods.
        If you want to use getState and setState, call the other constructor instead.

        @param valueToControl       a Value object that this property should refer to.
        @param propertyName         the property name to be passed to the PropertyComponent
        @param buttonText           the text shown in the ToggleButton component
    */
    BooleanPropertyComponent (const Value& valueToControl,
                              const Txt& propertyName,
                              const Txt& buttonText);

    /** Destructor. */
    ~BooleanPropertyComponent() override;

    //==============================================================================
    /** Called to change the state of the boolean value. */
    virtual z0 setState (b8 newState);

    /** Must return the current value of the property. */
    virtual b8 getState() const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId          = 0x100e801,    /**< The colour to fill the background of the button area. */
        outlineColorId             = 0x100e803,    /**< The colour to use to draw an outline around the text area. */
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;
    /** @internal */
    z0 refresh() override;

private:
    ToggleButton button;
    Txt onText, offText;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BooleanPropertyComponent)
};

} // namespace drx
