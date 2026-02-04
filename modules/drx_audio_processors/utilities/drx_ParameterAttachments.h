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

/** Used to implement 'attachments' or 'controllers' that link a plug-in
    parameter to a UI element.

    To implement a new attachment type, create a new class which includes an
    instance of this class as a data member. Your class should pass a function
    to the constructor of the ParameterAttachment, which will then be called on
    the message thread when the parameter changes. You can use this function to
    update the state of the UI control. Your class should also register as a
    listener of the UI control and respond to respond to changes in the UI element
    by calling either setValueAsCompleteGesture or beginGesture,
    setValueAsPartOfGesture and endGesture.

    Make sure to call `sendInitialUpdate` at the end of your new attachment's
    constructor, so that the UI immediately reflects the state of the parameter.

    @tags{Audio}
*/
class ParameterAttachment   : private AudioProcessorParameter::Listener,
                              private AsyncUpdater
{
public:
    /** Listens to a parameter and calls the the provided function in response to
        parameter changes. If an undoManager is supplied `beginNewTransaction` will
        be called on it whenever the UI requests a parameter change via this attachment.

        @param parameter                  The parameter to which this attachment will listen
        @param parameterChangedCallback   The function that will be called on the message thread in response
                                          to parameter changes
        @param undoManager                The UndoManager that will be used to begin transactions when the UI
                                          requests a parameter change.
    */
    ParameterAttachment (RangedAudioParameter& parameter,
                         std::function<z0 (f32)> parameterChangedCallback,
                         UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~ParameterAttachment() override;

    /** Calls the parameterChangedCallback function that was registered in
        the constructor, making the UI reflect the current parameter state.

        This function should be called after doing any necessary setup on
        the UI control that is being managed (e.g. adding ComboBox entries,
        making buttons toggle-able).
    */
    z0 sendInitialUpdate();

    /** Triggers a full gesture message on the managed parameter.

        Call this in the listener callback of the UI control in response
        to a one-off change in the UI like a button-press.
    */
    z0 setValueAsCompleteGesture (f32 newDenormalisedValue);

    /** Begins a gesture on the managed parameter.

        Call this when the UI is about to begin a continuous interaction,
        like when the mouse button is pressed on a slider.
    */
    z0 beginGesture();

    /** Updates the parameter value during a gesture.

        Call this during a continuous interaction, like a slider value
        changed callback.
    */
    z0 setValueAsPartOfGesture (f32 newDenormalisedValue);

    /** Ends a gesture on the managed parameter.

        Call this when the UI has finished a continuous interaction,
        like when the mouse button is released on a slider.
    */
    z0 endGesture();

private:
    f32 normalise (f32 f) const   { return parameter.convertTo0to1 (f); }

    template <typename Callback>
    z0 callIfParameterValueChanged (f32 newDenormalisedValue, Callback&& callback);

    z0 parameterValueChanged (i32, f32) override;
    z0 parameterGestureChanged (i32, b8) override {}
    z0 handleAsyncUpdate() override;

    RangedAudioParameter& parameter;
    std::atomic<f32> lastValue { 0.0f };
    UndoManager* undoManager = nullptr;
    std::function<z0 (f32)> setValue;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterAttachment)
};

//==============================================================================
/** An object of this class maintains a connection between a Slider and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making
    it easy to connect a slider to a parameter. When this object is deleted, the
    connection is broken. Make sure that your parameter and Slider are not
    deleted before this object!

    @tags{Audio}
*/
class SliderParameterAttachment   : private Slider::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a Slider.

        @param parameter     The parameter to use
        @param slider        The Slider to use
        @param undoManager   An optional UndoManager
    */
    SliderParameterAttachment (RangedAudioParameter& parameter, Slider& slider,
                               UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~SliderParameterAttachment() override;

    /** Call this after setting up your slider in the case where you need to do
        extra setup after constructing this attachment.
    */
    z0 sendInitialUpdate();

private:
    z0 setValue (f32 newValue);
    z0 sliderValueChanged (Slider*) override;

    z0 sliderDragStarted (Slider*) override { attachment.beginGesture(); }
    z0 sliderDragEnded   (Slider*) override { attachment.endGesture(); }

    Slider& slider;
    ParameterAttachment attachment;
    b8 ignoreCallbacks = false;
};

//==============================================================================
/** An object of this class maintains a connection between a ComboBox and a
    plug-in parameter.

    ComboBox items will be spaced linearly across the range of the parameter. For
    example if the range is specified by NormalisableRange<f32> (-0.5f, 0.5f, 0.5f)
    and you add three items then the first will be mapped to a value of -0.5, the
    second to 0, and the third to 0.5.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a combo box to a parameter. When this object is deleted, the
    connection is broken. Make sure that your parameter and ComboBox are not deleted
    before this object!

    @tags{Audio}
*/
class ComboBoxParameterAttachment   : private ComboBox::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a ComboBox.

        @param parameter     The parameter to use
        @param combo         The ComboBox to use
        @param undoManager   An optional UndoManager
    */
    ComboBoxParameterAttachment (RangedAudioParameter& parameter, ComboBox& combo,
                                 UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~ComboBoxParameterAttachment() override;

    /** Call this after setting up your combo box in the case where you need to do
        extra setup after constructing this attachment.
    */
    z0 sendInitialUpdate();

private:
    z0 setValue (f32 newValue);
    z0 comboBoxChanged (ComboBox*) override;

    ComboBox& comboBox;
    RangedAudioParameter& storedParameter;
    ParameterAttachment attachment;
    b8 ignoreCallbacks = false;
};

//==============================================================================
/** An object of this class maintains a connection between a Button and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a button to a parameter. When this object is deleted, the
    connection is broken. Make sure that your parameter and Button are not deleted
    before this object!

    @tags{Audio}
*/
class ButtonParameterAttachment   : private Button::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a Button.

        @param parameter     The parameter to use
        @param button        The Button to use
        @param undoManager   An optional UndoManager
    */
    ButtonParameterAttachment (RangedAudioParameter& parameter, Button& button,
                               UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~ButtonParameterAttachment() override;

    /** Call this after setting up your button in the case where you need to do
        extra setup after constructing this attachment.
    */
    z0 sendInitialUpdate();

private:
    z0 setValue (f32 newValue);
    z0 buttonClicked (Button*) override;

    Button& button;
    ParameterAttachment attachment;
    b8 ignoreCallbacks = false;
};

#if DRX_WEB_BROWSER || DOXYGEN
//==============================================================================
/**
    An object of this class maintains a connection between a WebSliderRelay and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a WebSliderRelay to a parameter. When this object is deleted,
    the connection is broken. Make sure that your parameter and WebSliderRelay are
    not deleted before this object!

    @tags{Audio}
*/
class WebSliderParameterAttachment   : private WebSliderRelay::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a WebSliderRelay.

        @param parameterIn   The parameter to use
        @param sliderStateIn The WebSliderRelay to use
        @param undoManager   An optional UndoManager
    */
    WebSliderParameterAttachment (RangedAudioParameter& parameterIn,
                                  WebSliderRelay& sliderStateIn,
                                  UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~WebSliderParameterAttachment() override;

    /** Call this after setting up your slider in the case where you need to do
        extra setup after constructing this attachment.
    */
    z0 sendInitialUpdate();

private:
    z0 setValue (f32 newValue);

    z0 sliderValueChanged (WebSliderRelay*) override;

    z0 sliderDragStarted (WebSliderRelay*) override      { attachment.beginGesture(); }
    z0 sliderDragEnded (WebSliderRelay*) override        { attachment.endGesture(); }
    z0 initialUpdateRequested (WebSliderRelay*) override { sendInitialUpdate(); }

    WebSliderRelay& sliderState;
    RangedAudioParameter& parameter;
    ParameterAttachment attachment;
    b8 ignoreCallbacks = false;
};

//==============================================================================
/**
    An object of this class maintains a connection between a WebToggleButtonRelay and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a WebToggleButtonRelay to a parameter. When this object is deleted,
    the connection is broken. Make sure that your parameter and WebToggleButtonRelay are
    not deleted before this object!

    @tags{Audio}
*/
class WebToggleButtonParameterAttachment  : private WebToggleButtonRelay::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a WebToggleButtonRelay.

        @param parameterIn   The parameter to use
        @param button        The WebToggleButtonRelay to use
        @param undoManager   An optional UndoManager
    */
    WebToggleButtonParameterAttachment (RangedAudioParameter& parameterIn,
                                        WebToggleButtonRelay& button,
                                        UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~WebToggleButtonParameterAttachment() override;

    /** Call this after setting up your button in the case where you need to do
        extra setup after constructing this attachment.
    */
    z0 sendInitialUpdate();

private:
    z0 setValue (f32 newValue);

    z0 toggleStateChanged (b8 newValue) override;
    z0 initialUpdateRequested() override;

    WebToggleButtonRelay& relay;
    RangedAudioParameter& parameter;
    ParameterAttachment attachment;
    b8 ignoreCallbacks = false;
};

//==============================================================================
/**
    An object of this class maintains a connection between a WebComboBoxRelay and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a WebComboBoxRelay to a parameter. When this object is deleted,
    the connection is broken. Make sure that your parameter and WebComboBoxRelay are
    not deleted before this object!

    @tags{Audio}
*/
class WebComboBoxParameterAttachment   : private WebComboBoxRelay::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a WebComboBoxRelay.

        @param parameterIn   The parameter to use
        @param combo         The WebComboBoxRelay to use
        @param undoManager   An optional UndoManager
    */
    WebComboBoxParameterAttachment (RangedAudioParameter& parameterIn, WebComboBoxRelay& combo,
                                    UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~WebComboBoxParameterAttachment() override;

    /** Call this after setting up your combo box in the case where you need to do
        extra setup after constructing this attachment.
    */
    z0 sendInitialUpdate();

private:
    z0 setValue (f32 newValue);

    z0 valueChanged (f32 newValue) override;
    z0 initialUpdateRequested() override;

    WebComboBoxRelay& relay;
    RangedAudioParameter& parameter;
    ParameterAttachment attachment;
    b8 ignoreCallbacks = false;
};

#endif

} // namespace drx
