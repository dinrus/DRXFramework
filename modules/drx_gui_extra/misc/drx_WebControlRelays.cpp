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

#if DRX_WEB_BROWSER

WebSliderRelay::WebSliderRelay (StringRef nameIn)
    : name (nameIn)
{
}

z0 WebSliderRelay::setValue (f32 newValue)
{
    using namespace detail;

    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (WebSliderRelayEvents::Event::eventTypeKey,
                         WebSliderRelayEvents::ValueChanged::eventId.toString());
    object->setProperty (WebSliderRelayEvents::ValueChanged::newValueKey, newValue);

    value = newValue;
    emitEvent (object.get());
}

f32 WebSliderRelay::getValue() const
{
    return value;
}

z0 WebSliderRelay::addListener (Listener* l)
{
    listeners.add (l);
}

z0 WebSliderRelay::removeListener (Listener* l)
{
    listeners.remove (l);
}

WebBrowserComponent::Options WebSliderRelay::buildOptions (const WebBrowserComponent::Options& initialOptions)
{
    return initialOptions
        .withEventListener (eventId, [this] (auto object) { handleEvent (object); })
        .withInitialisationData ("__drx__sliders", name)
        .withWebViewLifetimeListener (this);
}

z0 WebSliderRelay::emitEvent (const var& payload)
{
    if (browser != nullptr)
        browser->emitEventIfBrowserIsVisible (eventId, payload);
}

z0 WebSliderRelay::webViewConstructed (WebBrowserComponent* browserIn)
{
    browser = browserIn;
    listeners.call (&Listener::initialUpdateRequested, this);
}

z0 WebSliderRelay::webViewDestructed (WebBrowserComponent*)
{
    browser = nullptr;
}

z0 WebSliderRelay::handleEvent (const var& event)
{
    using namespace detail;

    if (const auto sliderEvent = WebSliderRelayEvents::Event::extract (event))
    {
        if (const auto valueChanged = WebSliderRelayEvents::ValueChanged::extract (*sliderEvent))
        {
            if (! approximatelyEqual (std::exchange (value, valueChanged->newValue), valueChanged->newValue))
                listeners.call ([this] (Listener& l) { l.sliderValueChanged (this); });

            return;
        }

        if (const auto dragStarted = WebSliderRelayEvents::SliderDragStarted::extract (*sliderEvent))
        {
            listeners.call ([this] (Listener& l) { l.sliderDragStarted (this); });
            return;
        }

        if (const auto dragEnded = WebSliderRelayEvents::SliderDragEnded::extract (*sliderEvent))
        {
            listeners.call ([this] (Listener& l) { l.sliderDragEnded (this); });
            return;
        }

        if (const auto initialUpdate =
                WebSliderRelayEvents::InitialUpdateRequested::extract (*sliderEvent))
        {
            listeners.call ([this] (Listener& l) { l.initialUpdateRequested (this); });
            return;
        }
    }

    const auto s = JSON::toString (event);
    jassertfalse;
}

//==============================================================================
WebToggleButtonRelay::WebToggleButtonRelay (StringRef nameIn)
    : name (nameIn)
{
}

z0 WebToggleButtonRelay::setToggleState (b8 newState)
{
    using namespace detail;

    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (WebToggleButtonRelayEvents::Event::eventTypeKey,
                         WebToggleButtonRelayEvents::ToggleStateChanged::eventId.toString());
    object->setProperty (WebToggleButtonRelayEvents::ToggleStateChanged::valueKey, newState);

    emitEvent (object.get());
}

z0 WebToggleButtonRelay::addListener (Listener* l)
{
    listeners.add (l);
}

z0 WebToggleButtonRelay::removeListener (Listener* l)
{
    listeners.remove (l);
}

WebBrowserComponent::Options WebToggleButtonRelay::buildOptions (const WebBrowserComponent::Options& initialOptions)
{
    return initialOptions
              .withEventListener (eventId, [this] (auto object) { handleEvent (object); })
              .withInitialisationData ("__drx__toggles", name)
              .withWebViewLifetimeListener (this);
}

z0 WebToggleButtonRelay::emitEvent (const var& payload)
{
    if (browser != nullptr)
        browser->emitEventIfBrowserIsVisible (eventId, payload);
}

z0 WebToggleButtonRelay::webViewConstructed (WebBrowserComponent* browserIn)
{
    browser = browserIn;
    listeners.call (&Listener::initialUpdateRequested);
}

z0 WebToggleButtonRelay::webViewDestructed (WebBrowserComponent*)
{
    browser = nullptr;
}

z0 WebToggleButtonRelay::handleEvent (const var& event)
{
    using namespace detail;

    if (const auto buttonEvent = WebToggleButtonRelayEvents::Event::extract (event))
    {
        if (const auto toggleStateChanged = WebToggleButtonRelayEvents::ToggleStateChanged::extract (*buttonEvent))
        {
            listeners.call ([&toggleStateChanged] (Listener& l)
                            { l.toggleStateChanged (toggleStateChanged->value); });
            return;
        }

        if (const auto initialUpdate = WebToggleButtonRelayEvents::InitialUpdateRequested::extract (*buttonEvent))
        {
            listeners.call ([] (Listener& l) { l.initialUpdateRequested(); });
            return;
        }
    }

    const auto s = JSON::toString (event);
    jassertfalse;
}

//==============================================================================
WebComboBoxRelay::WebComboBoxRelay (StringRef nameIn)
    : name (nameIn)
{
}

z0 WebComboBoxRelay::setValue (f32 newValue)
{
    using namespace detail;

    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (WebComboBoxRelayEvents::Event::eventTypeKey,
                         WebComboBoxRelayEvents::ValueChanged::eventId.toString());
    object->setProperty (WebComboBoxRelayEvents::ValueChanged::valueKey, newValue);

    emitEvent (object.get());
}

z0 WebComboBoxRelay::addListener (Listener* l)
{
    listeners.add (l);
}

z0 WebComboBoxRelay::removeListener (Listener* l)
{
    listeners.remove (l);
}

WebBrowserComponent::Options WebComboBoxRelay::buildOptions (const WebBrowserComponent::Options& initialOptions)
{
    return initialOptions
        .withEventListener (eventId, [this] (auto object) { handleEvent (object); })
        .withInitialisationData ("__drx__comboBoxes", name)
        .withWebViewLifetimeListener (this);
}

z0 WebComboBoxRelay::emitEvent (const var& payload)
{
    if (browser != nullptr)
        browser->emitEventIfBrowserIsVisible (eventId, payload);
}

z0 WebComboBoxRelay::webViewConstructed (WebBrowserComponent* browserIn)
{
    browser = browserIn;
    listeners.call (&Listener::initialUpdateRequested);
}

z0 WebComboBoxRelay::webViewDestructed (WebBrowserComponent*)
{
    browser = nullptr;
}

z0 WebComboBoxRelay::handleEvent (const var& event)
{
    using namespace detail;

    if (const auto buttonEvent = WebComboBoxRelayEvents::Event::extract (event))
    {
        if (const auto valueChanged = WebComboBoxRelayEvents::ValueChanged::extract (*buttonEvent))
        {
            listeners.call ([&valueChanged] (Listener& l)
                            { l.valueChanged (valueChanged->value); });
            return;
        }

        if (const auto initialUpdate = WebComboBoxRelayEvents::InitialUpdateRequested::extract (*buttonEvent))
        {
            listeners.call ([] (Listener& l) { l.initialUpdateRequested(); });
            return;
        }
    }

    const auto s = JSON::toString (event);
    jassertfalse;
}

#endif

} // namespace drx
