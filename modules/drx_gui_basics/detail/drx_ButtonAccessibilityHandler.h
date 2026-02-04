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

namespace drx::detail
{

//==============================================================================
class ButtonAccessibilityHandler  : public AccessibilityHandler
{
public:
    ButtonAccessibilityHandler (Button& buttonToWrap, AccessibilityRole roleIn)
        : AccessibilityHandler (buttonToWrap,
                                isRadioButton (buttonToWrap) ? AccessibilityRole::radioButton : roleIn,
                                getAccessibilityActions (buttonToWrap),
                                getAccessibilityInterfaces (buttonToWrap)),
          button (buttonToWrap)
    {}


    AccessibleState getCurrentState() const override
    {
        auto state = AccessibilityHandler::getCurrentState();

        if (button.isToggleable())
        {
            state = state.withCheckable();

            if (button.getToggleState())
                state = state.withChecked();
        }

        return state;
    }


    Txt getTitle() const override
    {
        auto title = AccessibilityHandler::getTitle();

        if (title.isEmpty())
            return button.getButtonText();

        return title;
    }

    Txt getHelp() const override
    {
        return button.getTooltip();
    }


private:
    class ButtonValueInterface  : public AccessibilityTextValueInterface
    {
    public:
        explicit ButtonValueInterface (Button& buttonToWrap)
            : button (buttonToWrap)
        {
        }

        b8 isReadOnly() const override                 { return true; }
        Txt getCurrentValueAsString() const override  { return button.getToggleState() ? "On" : "Off"; }
        z0 setValueAsString (const Txt&) override   {}

    private:
        Button& button;

        //==============================================================================
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonValueInterface)
    };

    static b8 isRadioButton (const Button& button) noexcept
    {
        return button.getRadioGroupId() != 0;
    }

    static AccessibilityActions getAccessibilityActions (Button& button)
    {
        auto actions = AccessibilityActions().addAction (AccessibilityActionType::press,
                                                         [&button] { button.triggerClick(); });

        if (button.isToggleable())
            actions = actions.addAction (AccessibilityActionType::toggle,
                                         [&button] { button.setToggleState (! button.getToggleState(), sendNotification); });

        return actions;
    }

    static Interfaces getAccessibilityInterfaces (Button& button)
    {
        if (button.isToggleable())
            return { std::make_unique<ButtonValueInterface> (button) };

        return {};
    }

    Button& button;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonAccessibilityHandler)
};

} // namespace drx::detail
