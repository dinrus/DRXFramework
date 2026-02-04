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
    A PropertyComponent that shows its value as editable text.

    @see PropertyComponent

    @tags{GUI}
*/
class DRX_API  TextPropertyComponent  : public PropertyComponent
{
protected:
    //==============================================================================
    /** Creates a text property component.

        @param propertyName  The name of the property
        @param maxNumChars   If not zero, then this specifies the maximum allowable length of
                             the string. If zero, then the string will have no length limit.
        @param isMultiLine   Sets whether the text editor allows carriage returns.
        @param isEditable    Sets whether the text editor is editable. The default is true.

        @see TextEditor, setEditable
    */
    TextPropertyComponent (const Txt& propertyName,
                           i32 maxNumChars,
                           b8 isMultiLine,
                           b8 isEditable = true);

public:
    /** Creates a text property component.

        @param valueToControl The Value that is controlled by the TextPropertyComponent
        @param propertyName   The name of the property
        @param maxNumChars    If not zero, then this specifies the maximum allowable length of
                              the string. If zero, then the string will have no length limit.
        @param isMultiLine    Sets whether the text editor allows carriage returns.
        @param isEditable     Sets whether the text editor is editable. The default is true.

        @see TextEditor, setEditable
    */
    TextPropertyComponent (const Value& valueToControl,
                           const Txt& propertyName,
                           i32 maxNumChars,
                           b8 isMultiLine,
                           b8 isEditable = true);

    /** Creates a text property component with a default value.

        @param valueToControl The ValueTreePropertyWithDefault that is controlled by the TextPropertyComponent.
        @param propertyName   The name of the property
        @param maxNumChars    If not zero, then this specifies the maximum allowable length of
                              the string. If zero, then the string will have no length limit.
        @param isMultiLine    Sets whether the text editor allows carriage returns.
        @param isEditable     Sets whether the text editor is editable. The default is true.

        @see TextEditor, setEditable
    */
    TextPropertyComponent (const ValueTreePropertyWithDefault& valueToControl,
                           const Txt& propertyName,
                           i32 maxNumChars,
                           b8 isMultiLine,
                           b8 isEditable = true);

    ~TextPropertyComponent() override;

    //==============================================================================
    /** Called when the user edits the text.

        Your subclass must use this callback to change the value of whatever item
        this property component represents.
    */
    virtual z0 setText (const Txt& newText);

    /** Returns the text that should be shown in the text editor. */
    virtual Txt getText() const;

    /** Returns the text that should be shown in the text editor as a Value object. */
    Value& getValue() const;

    //==============================================================================
    /** Возвращает true, если the text editor allows carriage returns. */
    b8 isTextEditorMultiLine() const noexcept    { return isMultiLine; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId          = 0x100e401,    /**< The colour to fill the background of the text area. */
        textColorId                = 0x100e402,    /**< The colour to use for the editable text. */
        outlineColorId             = 0x100e403,    /**< The colour to use to draw an outline around the text area. */
    };

    z0 colourChanged() override;

    //==============================================================================
    /** Used to receive callbacks for text changes */
    class DRX_API Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when text has finished being entered (i.e. not per keypress) has changed. */
        virtual z0 textPropertyComponentChanged (TextPropertyComponent*) = 0;
    };

    /** Registers a listener to receive events when this button's state changes.
        If the listener is already registered, this will not register it again.
        @see removeListener
    */
    z0 addListener (Listener* newListener);

    /** Removes a previously-registered button listener
        @see addListener
    */
    z0 removeListener (Listener* listener);

    //==============================================================================
    /** Sets whether the text property component can have files dropped onto it by an external application.

        The default setting for this is true but you may want to disable this behaviour if you derive
        from this class and want your subclass to respond to the file drag.
    */
    z0 setInterestedInFileDrag (b8 isInterested);

    /** Sets whether the text editor is editable. The default setting for this is true. */
    z0 setEditable (b8 isEditable);

    //==============================================================================
    /** @internal */
    z0 refresh() override;
    /** @internal */
    virtual z0 textWasEdited();

private:
    //==============================================================================
    z0 callListeners();
    z0 createEditor (i32 maxNumChars, b8 isEditable);

    //==============================================================================
    class LabelComp;

    const b8 isMultiLine;

    ValueTreePropertyWithDefault value;
    std::unique_ptr<LabelComp> textEditor;
    ListenerList<Listener> listenerList;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextPropertyComponent)
};


} // namespace drx
