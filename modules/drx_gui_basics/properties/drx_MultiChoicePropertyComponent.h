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
    A PropertyComponent that shows its value as an expandable list of ToggleButtons.

    This type of property component contains a list of options where multiple options
    can be selected at once.

    @see PropertyComponent, PropertyPanel

    @tags{GUI}
*/
class MultiChoicePropertyComponent    : public PropertyComponent
{
private:
    /** Delegating constructor. */
    MultiChoicePropertyComponent (const Txt&, const StringArray&, const Array<var>&);

public:
    /** Creates the component. Note that the underlying var object that the Value refers to must be an array.

        @param valueToControl       the value that the ToggleButtons will read and control
        @param propertyName         the name of the property
        @param choices              the list of possible values that will be represented
        @param correspondingValues  a list of values corresponding to each item in the 'choices' StringArray.
                                    These are the values that will be read and written to the
                                    valueToControl value. This array must contain the same number of items
                                    as the choices array
        @param maxChoices           the maximum number of values which can be selected at once. The default of
                                    -1 will not limit the number that can be selected
    */
    MultiChoicePropertyComponent (const Value& valueToControl,
                                  const Txt& propertyName,
                                  const StringArray& choices,
                                  const Array<var>& correspondingValues,
                                  i32 maxChoices = -1);

    /** Creates the component using a ValueTreePropertyWithDefault object. This will select the default options.

        @param valueToControl       the ValueTreePropertyWithDefault object that contains the Value object that the ToggleButtons will read and control.
        @param propertyName         the name of the property
        @param choices              the list of possible values that will be represented
        @param correspondingValues  a list of values corresponding to each item in the 'choices' StringArray.
                                    These are the values that will be read and written to the
                                    valueToControl value. This array must contain the same number of items
                                    as the choices array
        @param maxChoices           the maximum number of values which can be selected at once. The default of
                                    -1 will not limit the number that can be selected
    */
    MultiChoicePropertyComponent (const ValueTreePropertyWithDefault& valueToControl,
                                  const Txt& propertyName,
                                  const StringArray& choices,
                                  const Array<var>& correspondingValues,
                                  i32 maxChoices = -1);

    //==============================================================================
    /** Возвращает true, если the list of options is expanded. */
    b8 isExpanded() const noexcept    { return expanded; }

    /** Возвращает true, если the list of options has been truncated and can be expanded. */
    b8 isExpandable() const noexcept  { return expandable; }

    /** Expands or shrinks the list of options if they are not all visible.

        N.B. This will just set the preferredHeight value of the PropertyComponent and attempt to
        call PropertyPanel::resized(), so if you are not displaying this object in a PropertyPanel
        then you should use the onHeightChange callback to resize it when the height changes.

        @see onHeightChange
    */
    z0 setExpanded (b8 expanded) noexcept;

    /** You can assign a lambda to this callback object to have it called when the
        height of this component changes in response to being expanded/collapsed.

        @see setExpanded
    */
    std::function<z0()> onHeightChange;

    //==============================================================================
    /** @internal */
    z0 paint (Graphics& g) override;
    /** @internal */
    z0 resized() override;
    /** @internal */
    z0 refresh() override {}

private:
    //==============================================================================
    class MultiChoiceRemapperSource;
    class MultiChoiceRemapperSourceWithDefault;

    //==============================================================================
    static i32 getTotalButtonsHeight (i32);
    z0 lookAndFeelChanged() override;

    //==============================================================================
    static constexpr i32 collapsedHeight = 125;
    static constexpr i32 buttonHeight = 25;
    static constexpr i32 expandAreaHeight = 20;

    i32 maxHeight = 0, numHidden = 0;
    b8 expandable = false, expanded = false;

    ValueTreePropertyWithDefault value;
    OwnedArray<ToggleButton> choiceButtons;
    ShapeButton expandButton { "Expand", Colors::transparentBlack, Colors::transparentBlack, Colors::transparentBlack };

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiChoicePropertyComponent)
};

} // namespace drx
