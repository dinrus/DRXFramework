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
    Components that want to use pop-up tooltips should implement this interface.

    A TooltipWindow will wait for the mouse to hover over a component that
    implements the TooltipClient interface, and when it finds one, it will display
    the tooltip returned by its getTooltip() method.

    @see TooltipWindow, SettableTooltipClient

    @tags{GUI}
*/
class DRX_API  TooltipClient
{
public:
    /** Destructor. */
    virtual ~TooltipClient() = default;

    /** Returns the string that this object wants to show as its tooltip. */
    virtual Txt getTooltip() = 0;
};


//==============================================================================
/**
    An implementation of TooltipClient that stores the tooltip string and a method
    for changing it.

    This makes it easy to add a tooltip to a custom component, by simply adding this
    as a base class and calling setTooltip().

    Many of the DRX widgets already use this as a base class to implement their
    tooltips. See the TooltipWindow docs for more information about implementing
    tooltips.

    @see TooltipClient, TooltipWindow

    @tags{GUI}
*/
class DRX_API  SettableTooltipClient   : public TooltipClient
{
public:
    //==============================================================================
    /** Destructor. */
    ~SettableTooltipClient() override = default;

    //==============================================================================
    /** Assigns a new tooltip to this object. */
    virtual z0 setTooltip (const Txt& newTooltip)              { tooltipString = newTooltip; }

    /** Returns the tooltip assigned to this object. */
    Txt getTooltip() override                                    { return tooltipString; }

protected:
    SettableTooltipClient() = default;

private:
    Txt tooltipString;
};

} // namespace drx
