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
    A PropertyComponent that shows its value as a slider.

    @see PropertyComponent, Slider

    @tags{GUI}
*/
class DRX_API  SliderPropertyComponent   : public PropertyComponent
{
protected:
    //==============================================================================
    /** Creates the property component.

        The ranges, interval and skew factor are passed to the Slider component.

        If you need to customise the slider in other ways, your constructor can
        access the slider member variable and change it directly.
    */
    SliderPropertyComponent (const Txt& propertyName,
                             f64 rangeMin,
                             f64 rangeMax,
                             f64 interval,
                             f64 skewFactor = 1.0,
                             b8 symmetricSkew = false);

public:
    //==============================================================================
    /** Creates the property component.

        The ranges, interval and skew factor are passed to the Slider component.

        If you need to customise the slider in other ways, your constructor can
        access the slider member variable and change it directly.

        Note that if you call this constructor then you must use the Value to interact with
        the value, and you can't override the class with your own setValue or getValue methods.
        If you want to use those methods, call the other constructor instead.
    */
    SliderPropertyComponent (const Value& valueToControl,
                             const Txt& propertyName,
                             f64 rangeMin,
                             f64 rangeMax,
                             f64 interval,
                             f64 skewFactor = 1.0,
                             b8 symmetricSkew = false);

    /** Destructor. */
    ~SliderPropertyComponent() override;


    //==============================================================================
    /** Called when the user moves the slider to change its value.

        Your subclass must use this method to update whatever item this property
        represents.
    */
    virtual z0 setValue (f64 newValue);

    /** Returns the value that the slider should show. */
    virtual f64 getValue() const;


    //==============================================================================
    /** @internal */
    z0 refresh() override;

protected:
    /** The slider component being used in this component.
        Your subclass has access to this in case it needs to customise it in some way.
    */
    Slider slider;

private:
    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderPropertyComponent)
};

} // namespace drx
