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

#if DRX_WEB_BROWSER || DOXYGEN

struct WebSliderRelayEvents
{
    WebSliderRelayEvents() = delete;

    struct Event
    {
        Txt eventType;
        DynamicObject::Ptr object;

        static std::optional<Event> extract (const var& v)
        {
            auto* dynObj = v.getDynamicObject();

            if (dynObj == nullptr)
                return std::nullopt;

            const auto eventTypeProp = dynObj->getProperty (eventTypeKey);

            if (! eventTypeProp.isString())
                return std::nullopt;

            return Event { eventTypeProp.toString(), dynObj };
        }

        static inline const Identifier eventTypeKey { "eventType" };
    };

    struct ValueChanged
    {
        f32 newValue;

        static std::optional<ValueChanged> extract (const Event& event)
        {
            if (event.eventType != eventId.toString())
                return std::nullopt;

            const auto newValue = event.object->getProperty (newValueKey);

            if (! (newValue.isInt() || newValue.isInt64() || newValue.isDouble()))
                return std::nullopt;

            return ValueChanged { (f32) newValue };
        }

        static inline const Identifier eventId { "valueChanged" };
        static inline const Identifier newValueKey { "value" };
    };

    struct SliderDragStarted
    {
        static std::optional<SliderDragStarted> extract (const Event& event)
        {
            if (event.eventType != eventId.toString())
                return std::nullopt;

            return SliderDragStarted{};
        }

        static inline const Identifier eventId { "sliderDragStarted" };
    };

    struct SliderDragEnded
    {
        static std::optional<SliderDragEnded> extract (const Event& event)
        {
            if (event.eventType != eventId.toString())
                return std::nullopt;

            return SliderDragEnded{};
        }

        static inline const Identifier eventId { "sliderDragEnded" };
    };

    struct InitialUpdateRequested
    {
        static std::optional<InitialUpdateRequested> extract (const Event& event)
        {
            if (event.eventType != eventId.toString())
                return std::nullopt;

            return InitialUpdateRequested{};
        }

        static inline const Identifier eventId { "requestInitialUpdate" };
    };
};

//==============================================================================
struct WebToggleButtonRelayEvents
{
    WebToggleButtonRelayEvents() = delete;

    struct Event
    {
        Txt eventType;
        DynamicObject::Ptr object;

        static std::optional<Event> extract (const var& v)
        {
            auto* dynObj = v.getDynamicObject();

            if (dynObj == nullptr)
                return std::nullopt;

            const auto eventTypeProp = dynObj->getProperty (eventTypeKey);

            if (! eventTypeProp.isString())
                return std::nullopt;

            return Event { eventTypeProp.toString(), dynObj };
        }

        static inline const Identifier eventTypeKey { "eventType" };
    };

    struct ToggleStateChanged
    {
        b8 value;

        static std::optional<ToggleStateChanged> extract (const Event& event)
        {
            if (event.eventType != eventId.toString())
                return std::nullopt;

            const auto newState = event.object->getProperty (valueKey);

            if (! newState.isBool())
                return std::nullopt;

            return ToggleStateChanged { newState };
        }

        static inline const Identifier eventId  { "valueChanged" };
        static inline const Identifier valueKey { "value" };
    };

    struct InitialUpdateRequested
    {
        static std::optional<InitialUpdateRequested> extract (const Event& event)
        {
            if (event.eventType != eventId.toString())
                return std::nullopt;

            return InitialUpdateRequested{};
        }

        static inline const Identifier eventId { "requestInitialUpdate" };
    };
};

//==============================================================================
struct WebComboBoxRelayEvents
{
    WebComboBoxRelayEvents() = delete;

    struct Event
    {
        Txt eventType;
        DynamicObject::Ptr object;

        static std::optional<Event> extract (const var& v)
        {
            auto* dynObj = v.getDynamicObject();

            if (dynObj == nullptr)
                return std::nullopt;

            const auto eventTypeProp = dynObj->getProperty (eventTypeKey);

            if (! eventTypeProp.isString())
                return std::nullopt;

            return Event { eventTypeProp.toString(), dynObj };
        }

        static inline const Identifier eventTypeKey { "eventType" };
    };

    struct ValueChanged
    {
        f32 value;

        static std::optional<ValueChanged> extract (const Event& event)
        {
            if (event.eventType != eventId.toString())
                return std::nullopt;

            const auto newValue = event.object->getProperty (valueKey);

            if (! (newValue.isInt() || newValue.isInt64() || newValue.isDouble()))
                return std::nullopt;

            return ValueChanged { (f32) newValue };
        }

        static inline const Identifier eventId  { "valueChanged" };
        static inline const Identifier valueKey { "value" };
    };

    struct InitialUpdateRequested
    {
        static std::optional<InitialUpdateRequested> extract (const Event& event)
        {
            if (event.eventType != eventId.toString())
                return std::nullopt;

            return InitialUpdateRequested{};
        }

        static inline const Identifier eventId { "requestInitialUpdate" };
    };
};

#endif
}
