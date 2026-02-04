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
    A PropertyComponent that contains a button.

    This type of property component can be used if you need a button to trigger some
    kind of action.

    @see PropertyComponent

    @tags{GUI}
*/
class DRX_API  ButtonPropertyComponent  : public PropertyComponent
{
public:
    //==============================================================================
    /** Creates a button component.

        @param propertyName         the property name to be passed to the PropertyComponent
        @param triggerOnMouseDown   this is passed to the Button::setTriggeredOnMouseDown() method
    */
    ButtonPropertyComponent (const Txt& propertyName,
                             b8 triggerOnMouseDown);

    /** Destructor. */
    ~ButtonPropertyComponent() override;

    //==============================================================================
    /** Called when the user clicks the button.
    */
    virtual z0 buttonClicked() = 0;

    /** Returns the string that should be displayed in the button.

        If you need to change this string, call refresh() to update the component.
    */
    virtual Txt getButtonText() const = 0;

    //==============================================================================
    /** @internal */
    z0 refresh() override;

private:
    TextButton button;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonPropertyComponent)
};

} // namespace drx
