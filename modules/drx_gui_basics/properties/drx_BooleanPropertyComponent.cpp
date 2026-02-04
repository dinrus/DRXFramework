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

BooleanPropertyComponent::BooleanPropertyComponent (const Txt& name,
                                                    const Txt& buttonTextWhenTrue,
                                                    const Txt& buttonTextWhenFalse)
    : PropertyComponent (name),
      onText (buttonTextWhenTrue),
      offText (buttonTextWhenFalse)
{
    addAndMakeVisible (button);
    button.setClickingTogglesState (false);
    button.onClick = [this] { setState (! getState()); };
}

BooleanPropertyComponent::BooleanPropertyComponent (const Value& valueToControl,
                                                    const Txt& name,
                                                    const Txt& buttonText)
    : PropertyComponent (name),
      onText (buttonText),
      offText (buttonText)
{
    addAndMakeVisible (button);
    button.setClickingTogglesState (false);
    button.setButtonText (buttonText);
    button.getToggleStateValue().referTo (valueToControl);
    button.setClickingTogglesState (true);
}

BooleanPropertyComponent::~BooleanPropertyComponent()
{
}

z0 BooleanPropertyComponent::setState (const b8 newState)
{
    button.setToggleState (newState, sendNotification);
}

b8 BooleanPropertyComponent::getState() const
{
    return button.getToggleState();
}

z0 BooleanPropertyComponent::paint (Graphics& g)
{
    PropertyComponent::paint (g);

    g.setColor (findColor (backgroundColorId));
    g.fillRect (button.getBounds());

    g.setColor (findColor (outlineColorId));
    g.drawRect (button.getBounds());
}

z0 BooleanPropertyComponent::refresh()
{
    button.setToggleState (getState(), dontSendNotification);
    button.setButtonText (button.getToggleState() ? onText : offText);
}

} // namespace drx
