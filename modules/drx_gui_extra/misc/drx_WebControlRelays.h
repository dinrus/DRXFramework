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

#if DRX_WEB_BROWSER || DOXYGEN

/** Helper class that relays audio parameter information to an object inside a WebBrowserComponent.

    In order to create a relay you need to specify an identifier for the relayed state. This will
    result in a Javascript object becoming available inside the WebBrowserComponent under the
    provided identifier.

    Pass the relay object to WebBrowserComponent::Options::withOptionsFrom() to associate it with
    a WebBrowserComponent instance.

    You can then use a WebSliderParameterAttachment as you would a SliderAttachment, to attach the
    relay to a RangedAudioParameter. This will synchronise the state and events of the Javascript
    object with the audio parameter at all times.

    @code
    // Add a relay to your AudioProcessorEditor members
    WebSliderRelay cutoffSliderRelay { "cutoffSlider" };
    WebBrowserComponent webComponent { WebBrowserComponent::Options{}::withOptionsFrom (cutoffSliderRelay) };
    @endcode

    @code
    // In your Javascript GUI code you obtain an object from the framework
    import * as Drx from "drx-framework-frontend";
    const sliderState = Drx.getSliderState("cutoffSlider");
    @endcode

    @see WebSliderParameterAttachment

    @tags{GUI}
*/
class DRX_API  WebSliderRelay : public OptionsBuilder<WebBrowserComponent::Options>,
                                 private WebViewLifetimeListener
{
public:
    /** Creating a relay will ensure that a Javascript object under the provided name will be
        available in the specified WebBrowserComponent's context. Use the frontend framework's
        getSliderState function with the same name to get a hold of this object.
    */
    WebSliderRelay (StringRef nameIn);

    //==============================================================================
    /** @internal */
    struct Listener : public SliderListener<WebSliderRelay>
    {
        virtual z0 initialUpdateRequested (WebSliderRelay*) = 0;
    };

    /** @internal */
    z0 setValue (f32 newValue);

    /** @internal */
    f32 getValue() const;

    /** @internal */
    z0 addListener (Listener* l);

    /** @internal */
    z0 removeListener (Listener* l);

    /** @internal */
    WebBrowserComponent::Options buildOptions (const WebBrowserComponent::Options& initialOptions) override;

    /** @internal */
    z0 emitEvent (const var& payload);

private:
    z0 handleEvent (const var& event);
    z0 webViewConstructed (WebBrowserComponent*) override;
    z0 webViewDestructed (WebBrowserComponent*) override;

    WebBrowserComponent* browser = nullptr;
    Txt name;
    f32 value{};
    Identifier eventId { "__drx__slider" + name };
    ListenerList<Listener> listeners;

    DRX_DECLARE_NON_COPYABLE (WebSliderRelay)
    DRX_DECLARE_NON_MOVEABLE (WebSliderRelay)
};

/** Helper class that relays audio parameter information to an object inside a WebBrowserComponent.

    In order to create a relay you need to specify an identifier for the relayed state. This will
    result in a Javascript object becoming available inside the WebBrowserComponent under the
    provided identifier.

    Pass the relay object to WebBrowserComponent::Options::withOptionsFrom() to associate it with
    a WebBrowserComponent instance.

    You can then use a WebToggleButtonParameterAttachment as you would a ButtonParameterAttachment,
    to attach the relay to a RangedAudioParameter. This will synchronise the state and events of
    the Javascript object with the audio parameter at all times.

    @code
    // Add a relay to your AudioProcessorEditor members
    WebToggleButtonRelay muteToggleRelay { "muteToggle" };
    WebBrowserComponent webComponent { WebBrowserComponent::Options{}::withOptionsFrom (muteToggleRelay) };
    @endcode

    @code
    // In your Javascript GUI code you obtain an object from the framework
    import * as Drx from "drx-framework-frontend";
    const checkboxState = Drx.getToggleState("muteToggle");
    @endcode

    @see WebToggleButtonParameterAttachment

    @tags{GUI}
*/
class DRX_API  WebToggleButtonRelay  : public OptionsBuilder<WebBrowserComponent::Options>,
                                        private WebViewLifetimeListener
{
public:
    /** Creating a relay will ensure that a Javascript object under the provided name will be
        available in the specified WebBrowserComponent's context. Use the frontend framework's
        getToggleState function with the same name to get a hold of this object.
    */
    WebToggleButtonRelay (StringRef nameIn);

    //==============================================================================
    /** @internal */
    struct Listener
    {
        virtual ~Listener()                          = default;
        virtual z0 toggleStateChanged (b8)       = 0;
        virtual z0 initialUpdateRequested()        = 0;
    };

    /** @internal */
    z0 setToggleState (b8 newState);

    /** @internal */
    z0 addListener (Listener* l);

    /** @internal */
    z0 removeListener (Listener* l);

    /** @internal */
    WebBrowserComponent::Options buildOptions (const WebBrowserComponent::Options& initialOptions) override;

    /** @internal */
    z0 emitEvent (const var& payload);

private:
    z0 handleEvent (const var& event);
    z0 webViewConstructed (WebBrowserComponent*) override;
    z0 webViewDestructed (WebBrowserComponent*) override;

    WebBrowserComponent* browser = nullptr;
    Txt name;
    Identifier eventId { "__drx__toggle" + name };
    ListenerList<Listener> listeners;

    DRX_DECLARE_NON_COPYABLE (WebToggleButtonRelay)
    DRX_DECLARE_NON_MOVEABLE (WebToggleButtonRelay)
};

/** Helper class that relays audio parameter information to an object inside a WebBrowserComponent.

    In order to create a relay you need to specify an identifier for the relayed state. This will
    result in a Javascript object becoming available inside the WebBrowserComponent under the
    provided identifier.

    Pass the relay object to WebBrowserComponent::Options::withOptionsFrom() to associate it with
    a WebBrowserComponent instance.

    You can then use a WebComboBoxParameterAttachment as you would a ComboBoxParameterAttachment,
    to attach the relay to a RangedAudioParameter. This will synchronise the state and events of
    the Javascript object with the audio parameter at all times.

    @code
    // Add a relay to your AudioProcessorEditor members
    WebComboBoxRelay filterTypeComboRelay { "filterTypeCombo" };
    WebBrowserComponent webComponent { WebBrowserComponent::Options{}::withOptionsFrom (filterTypeComboRelay) };
    @endcode

    @code
    // In your Javascript GUI code you obtain an object from the framework
    import * as Drx from "drx-framework-frontend";
    const comboBoxState = Drx.getComboBoxState("filterTypeCombo");
    @endcode

    @see WebComboBoxParameterAttachment

    @tags{GUI}
*/
class DRX_API  WebComboBoxRelay  : public OptionsBuilder<WebBrowserComponent::Options>,
                                    private WebViewLifetimeListener
{
public:
    /** Creating a relay will ensure that a Javascript object under the provided name will be
        available in the specified WebBrowserComponent's context. Use the frontend framework's
        getComboBoxState function with the same name to get a hold of this object.
    */
    WebComboBoxRelay (StringRef nameIn);

    //==============================================================================
    /** @internal */
    struct Listener
    {
        virtual ~Listener()                          = default;
        virtual z0 valueChanged (f32)            = 0;
        virtual z0 initialUpdateRequested()        = 0;
    };

    /** @internal */
    z0 setValue (f32 newValue);

    /** @internal */
    z0 addListener (Listener* l);

    /** @internal */
    z0 removeListener (Listener* l);

    /** @internal */
    WebBrowserComponent::Options buildOptions (const WebBrowserComponent::Options& initialOptions) override;

    /** @internal */
    z0 emitEvent (const var& payload);

private:
    z0 handleEvent (const var& event);
    z0 webViewConstructed (WebBrowserComponent*) override;
    z0 webViewDestructed (WebBrowserComponent*) override;

    WebBrowserComponent* browser = nullptr;
    Txt name;
    Identifier eventId { "__drx__comboBox" + name };
    ListenerList<Listener> listeners;

    DRX_DECLARE_NON_COPYABLE (WebComboBoxRelay)
    DRX_DECLARE_NON_MOVEABLE (WebComboBoxRelay)
};

#endif

}
