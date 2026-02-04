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

#define DRX_NATIVE_ACCESSIBILITY_INCLUDED 1

DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

//==============================================================================
struct WindowsAccessibility
{
    WindowsAccessibility() = delete;

    static i64 getUiaRootObjectId()
    {
        return static_cast<i64> (UiaRootObjectId);
    }

    static b8 handleWmGetObject (AccessibilityHandler* handler, WPARAM wParam, LPARAM lParam, LRESULT* res)
    {
        if (isStartingUpOrShuttingDown() || (handler == nullptr || ! isHandlerValid (*handler)))
            return false;

        if (auto* uiaWrapper = WindowsUIAWrapper::getInstance())
        {
            ComSmartPtr<IRawElementProviderSimple> provider;
            handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

            if (! uiaWrapper->isProviderDisconnecting (provider))
                *res = uiaWrapper->returnRawElementProvider ((HWND) handler->getComponent().getWindowHandle(), wParam, lParam, provider);

            return true;
        }

        return false;
    }

    static z0 revokeUIAMapEntriesForWindow (HWND hwnd)
    {
        if (auto* uiaWrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
            uiaWrapper->returnRawElementProvider (hwnd, 0, 0, nullptr);
    }

    static b8 isStartingUpOrShuttingDown()
    {
        if (auto* app = DRXApplicationBase::getInstance())
            if (app->isInitialising())
                return true;

        if (auto* mm = MessageManager::getInstanceWithoutCreating())
            if (mm->hasStopMessageBeenSent())
                return true;

        return false;
    }

    static b8 isHandlerValid (const AccessibilityHandler& handler)
    {
        if (auto* provider = handler.getNativeImplementation())
            return provider->isElementValid();

        return false;
    }

    static b8 areAnyAccessibilityClientsActive()
    {
        const auto areClientsListening = []
        {
            if (auto* uiaWrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
                return uiaWrapper->clientsAreListening() != 0;

            return false;
        };

        const auto isScreenReaderRunning = []
        {
            BOOL isRunning = FALSE;
            SystemParametersInfo (SPI_GETSCREENREADER, 0, (PVOID) &isRunning, 0);

            return isRunning != 0;
        };

        return areClientsListening() || isScreenReaderRunning();
    }
};

//==============================================================================
class AccessibilityHandler::AccessibilityNativeImpl
{
public:
    explicit AccessibilityNativeImpl (AccessibilityHandler& owner)
        : accessibilityElement (becomeComSmartPtrOwner (new AccessibilityNativeHandle (owner)))
    {
        ++providerCount;
    }

    ~AccessibilityNativeImpl()
    {
        ComSmartPtr<IRawElementProviderSimple> provider;
        accessibilityElement->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

        accessibilityElement->invalidateElement();
        --providerCount;

        if (auto* uiaWrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
        {
            uiaWrapper->disconnectProvider (provider);

            if (providerCount == 0 && DRXApplicationBase::isStandaloneApp())
                uiaWrapper->disconnectAllProviders();
        }
    }

    //==============================================================================
    ComSmartPtr<AccessibilityNativeHandle> accessibilityElement;
    static i32 providerCount;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityNativeImpl)
};

i32 AccessibilityHandler::AccessibilityNativeImpl::providerCount = 0;

//==============================================================================
AccessibilityNativeHandle* AccessibilityHandler::getNativeImplementation() const
{
    return nativeImpl->accessibilityElement;
}

template <typename Callback>
z0 getProviderWithCheckedWrapper (const AccessibilityHandler& handler, Callback&& callback)
{
    if (! WindowsAccessibility::areAnyAccessibilityClientsActive()
        || WindowsAccessibility::isStartingUpOrShuttingDown()
        || ! WindowsAccessibility::isHandlerValid (handler))
        return;

    if (auto* uiaWrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
    {
        ComSmartPtr<IRawElementProviderSimple> provider;
        handler.getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));

        callback (uiaWrapper, provider);
    }
}

z0 sendAccessibilityAutomationEvent (const AccessibilityHandler& handler, EVENTID event)
{
    jassert (event != EVENTID{});

    getProviderWithCheckedWrapper (handler, [event] (WindowsUIAWrapper* uiaWrapper, ComSmartPtr<IRawElementProviderSimple>& provider)
    {
        uiaWrapper->raiseAutomationEvent (provider, event);
    });
}

z0 sendAccessibilityPropertyChangedEvent (const AccessibilityHandler& handler, PROPERTYID property, VARIANT newValue)
{
    jassert (property != PROPERTYID{});

    getProviderWithCheckedWrapper (handler, [property, newValue] (WindowsUIAWrapper* uiaWrapper, ComSmartPtr<IRawElementProviderSimple>& provider)
    {
        VARIANT oldValue;
        VariantHelpers::clear (&oldValue);

        uiaWrapper->raiseAutomationPropertyChangedEvent (provider, property, oldValue, newValue);
    });
}

z0 detail::AccessibilityHelpers::notifyAccessibilityEvent (const AccessibilityHandler& handler, Event eventType)
{
    if (eventType == Event::elementCreated
        || eventType == Event::elementDestroyed)
    {
        if (auto* parent = handler.getParent())
            sendAccessibilityAutomationEvent (*parent, UIA_LayoutInvalidatedEventId);

        return;
    }

    if (eventType == Event::windowOpened
        || eventType == Event::windowClosed)
    {
        if (auto* peer = handler.getComponent().getPeer())
            if ((peer->getStyleFlags() & ComponentPeer::windowHasTitleBar) == 0)
                return;
    }

    auto event = [eventType]() -> EVENTID
    {
        switch (eventType)
        {
            case Event::focusChanged:           return UIA_AutomationFocusChangedEventId;
            case Event::windowOpened:           return UIA_Window_WindowOpenedEventId;
            case Event::windowClosed:           return UIA_Window_WindowClosedEventId;
            case Event::elementCreated:
            case Event::elementDestroyed:
            case Event::elementMovedOrResized:  break;
        }

        return {};
    }();

    if (event != EVENTID{})
        sendAccessibilityAutomationEvent (handler, event);
}

z0 AccessibilityHandler::notifyAccessibilityEvent (AccessibilityEvent eventType) const
{
    if (eventType == AccessibilityEvent::titleChanged)
    {
        VARIANT newValue;
        VariantHelpers::setString (getTitle(), &newValue);

        sendAccessibilityPropertyChangedEvent (*this, UIA_NamePropertyId, newValue);
        return;
    }

    if (eventType == AccessibilityEvent::valueChanged)
    {
        if (auto* valueInterface = getValueInterface())
        {
            const auto propertyType = getRole() == AccessibilityRole::slider ? UIA_RangeValueValuePropertyId
                                                                             : UIA_ValueValuePropertyId;

            const auto value = getRole() == AccessibilityRole::slider
                               ? VariantHelpers::getWithValue (valueInterface->getCurrentValue())
                               : VariantHelpers::getWithValue (valueInterface->getCurrentValueAsString());

            sendAccessibilityPropertyChangedEvent (*this, propertyType, value);
        }

        return;
    }

    auto event = [eventType]() -> EVENTID
    {
        switch (eventType)
        {
            case AccessibilityEvent::textSelectionChanged:  return UIA_Text_TextSelectionChangedEventId;
            case AccessibilityEvent::textChanged:           return UIA_Text_TextChangedEventId;
            case AccessibilityEvent::structureChanged:      return UIA_StructureChangedEventId;
            case AccessibilityEvent::rowSelectionChanged:   return UIA_SelectionItem_ElementSelectedEventId;
            case AccessibilityEvent::titleChanged:
            case AccessibilityEvent::valueChanged:          break;
        }

        return {};
    }();

    if (event != EVENTID{})
        sendAccessibilityAutomationEvent (*this, event);
}

struct SpVoiceWrapper final : public DeletedAtShutdown
{
    SpVoiceWrapper()
    {
        [[maybe_unused]] auto hr = voice.CoCreateInstance (CLSID_SpVoice);

        jassert (SUCCEEDED (hr));
    }

    ~SpVoiceWrapper() override
    {
        clearSingletonInstance();
    }

    ComSmartPtr<ISpVoice> voice;

    DRX_DECLARE_SINGLETON_INLINE (SpVoiceWrapper, false)
};


z0 AccessibilityHandler::postAnnouncement (const Txt& announcementString, AnnouncementPriority priority)
{
    if (! areAnyAccessibilityClientsActive())
        return;

    if (auto* sharedVoice = SpVoiceWrapper::getInstance())
    {
        auto voicePriority = [priority]
        {
            switch (priority)
            {
                case AnnouncementPriority::low:    return SPVPRI_OVER;
                case AnnouncementPriority::medium: return SPVPRI_NORMAL;
                case AnnouncementPriority::high:   return SPVPRI_ALERT;
            }

            jassertfalse;
            return SPVPRI_OVER;
        }();

        sharedVoice->voice->SetPriority (voicePriority);
        sharedVoice->voice->Speak (announcementString.toWideCharPointer(), SPF_ASYNC, nullptr);
    }
}

b8 AccessibilityHandler::areAnyAccessibilityClientsActive()
{
    return WindowsAccessibility::areAnyAccessibilityClientsActive();
}

DRX_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace drx
