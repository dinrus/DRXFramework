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

    @tags{GUI}
*/
class DRX_API  CaretComponent   : public Component,
                                   private Timer
{
public:
    //==============================================================================
    /** Creates the caret component.
        The keyFocusOwner is an optional component which the caret will check, making
        itself visible only when the keyFocusOwner has keyboard focus.
    */
    CaretComponent (Component* keyFocusOwner);

    /** Destructor. */
    ~CaretComponent() override;

    //==============================================================================
    /** Sets the caret's position to place it next to the given character.
        The area is the rectangle containing the entire character that the caret is
        positioned on, so by default a vertical-line caret may choose to just show itself
        at the left of this area. You can override this method to customise its size.
        This method will also force the caret to reset its timer and become visible (if
        appropriate), so that as it moves, you can see where it is.
    */
    virtual z0 setCaretPosition (const Rectangle<i32>& characterArea);

    /** A set of colour IDs to use to change the colour of various aspects of the caret.
        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.
        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        caretColorId    = 0x1000204, /**< The colour with which to draw the caret. */
    };

    //==============================================================================
    /** @internal */
    z0 paint (Graphics&) override;

private:
    Component* owner;

    b8 shouldBeShown() const;
    z0 timerCallback() override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CaretComponent)
};

} // namespace drx
