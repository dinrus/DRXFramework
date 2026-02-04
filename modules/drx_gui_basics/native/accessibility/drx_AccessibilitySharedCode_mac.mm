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
struct AccessibleObjCClassDeleter
{
    template <typename ElementType>
    z0 operator() (ElementType* element) const
    {
        juceFreeAccessibilityPlatformSpecificData (element);

        object_setInstanceVariable (element, "handler", nullptr);
        [element release];
    }
};

template <typename Base>
class AccessibleObjCClass : public ObjCClass<Base>
{
public:
    using Holder = std::unique_ptr<Base, AccessibleObjCClassDeleter>;

protected:
    AccessibleObjCClass() : AccessibleObjCClass ("DRXAccessibilityElement_") {}

    explicit AccessibleObjCClass (tukk name)  : ObjCClass<Base> (name)
    {
        ObjCClass<Base>::template addIvar<AccessibilityHandler*> ("handler");
    }

    //==============================================================================
    static AccessibilityHandler* getHandler (id self)
    {
        return getIvar<AccessibilityHandler*> (self, "handler");
    }

    template <typename MemberFn>
    static auto getInterface (id self, MemberFn fn) noexcept -> decltype ((std::declval<AccessibilityHandler>().*fn)())
    {
        if (auto* handler = getHandler (self))
            return (handler->*fn)();

        return nullptr;
    }

    static AccessibilityTextInterface*  getTextInterface  (id self) noexcept  { return getInterface (self, &AccessibilityHandler::getTextInterface); }
    static AccessibilityValueInterface* getValueInterface (id self) noexcept  { return getInterface (self, &AccessibilityHandler::getValueInterface); }
    static AccessibilityTableInterface* getTableInterface (id self) noexcept  { return getInterface (self, &AccessibilityHandler::getTableInterface); }
    static AccessibilityCellInterface*  getCellInterface  (id self) noexcept  { return getInterface (self, &AccessibilityHandler::getCellInterface); }

    static b8 hasEditableText (AccessibilityHandler& handler) noexcept
    {
        return handler.getRole() == AccessibilityRole::editableText
            && handler.getTextInterface() != nullptr
            && ! handler.getTextInterface()->isReadOnly();
    }

    static id getAccessibilityValueFromInterfaces (const AccessibilityHandler& handler)
    {
        if (auto* textInterface = handler.getTextInterface())
            return juceStringToNS (textInterface->getText ({ 0, textInterface->getTotalNumCharacters() }));

        if (auto* valueInterface = handler.getValueInterface())
            return juceStringToNS (valueInterface->getCurrentValueAsString());

        return nil;
    }

    //==============================================================================
    static BOOL getIsAccessibilityElement (id self, SEL)
    {
        if (auto* handler = getHandler (self))
            return ! handler->isIgnored() && handler->getRole() != AccessibilityRole::window;

        return NO;
    }

    static z0 setAccessibilityValue (id self, SEL, NSString* value)
    {
        if (auto* handler = getHandler (self))
        {
            if (hasEditableText (*handler))
            {
                handler->getTextInterface()->setText (nsStringToDrx (value));
                return;
            }

            if (auto* valueInterface = handler->getValueInterface())
                if (! valueInterface->isReadOnly())
                    valueInterface->setValueAsString (nsStringToDrx (value));
        }
    }

    static BOOL performActionIfSupported (id self, AccessibilityActionType actionType)
    {
        if (auto* handler = getHandler (self))
            if (handler->getActions().invoke (actionType))
                return YES;

        return NO;
    }

    static BOOL accessibilityPerformPress (id self, SEL)
    {
        if (auto* handler = getHandler (self))
            if (handler->getCurrentState().isCheckable() && handler->getActions().invoke (AccessibilityActionType::toggle))
                return YES;

        return performActionIfSupported (self, AccessibilityActionType::press);
    }

    static BOOL accessibilityPerformIncrement (id self, SEL)
    {
        if (auto* valueInterface = getValueInterface (self))
        {
            if (! valueInterface->isReadOnly())
            {
                auto range = valueInterface->getRange();

                if (range.isValid())
                {
                    valueInterface->setValue (jlimit (range.getMinimumValue(),
                                                      range.getMaximumValue(),
                                                      valueInterface->getCurrentValue() + range.getInterval()));
                    return YES;
                }
            }
        }

        return NO;
    }

    static BOOL accessibilityPerformDecrement (id self, SEL)
    {
        if (auto* valueInterface = getValueInterface (self))
        {
            if (! valueInterface->isReadOnly())
            {
                auto range = valueInterface->getRange();

                if (range.isValid())
                {
                    valueInterface->setValue (jlimit (range.getMinimumValue(),
                                                      range.getMaximumValue(),
                                                      valueInterface->getCurrentValue() - range.getInterval()));
                    return YES;
                }
            }
        }

        return NO;
    }

    static NSString* getAccessibilityTitle (id self, SEL)
    {
        if (auto* handler = getHandler (self))
        {
            auto title = handler->getTitle();

            if (title.isEmpty() && handler->getComponent().isOnDesktop())
                title = detail::AccessibilityHelpers::getApplicationOrPluginName();

            NSString* nsString = juceStringToNS (title);

            if (nsString != nil && [[self accessibilityValue] isEqual: nsString])
                return @"";

            return nsString;
        }

        return nil;
    }

    static NSString* getAccessibilityHelp (id self, SEL)
    {
        if (auto* handler = getHandler (self))
            return juceStringToNS (handler->getHelp());

        return nil;
    }

    static BOOL getIsAccessibilityModal (id self, SEL)
    {
        if (auto* handler = getHandler (self))
            return handler->getComponent().isCurrentlyModal();

        return NO;
    }

    static NSInteger getAccessibilityRowCount (id self, SEL)
    {
        if (auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (getHandler (self), &AccessibilityHandler::getTableInterface))
            if (auto* tableInterface = tableHandler->getTableInterface())
                return tableInterface->getNumRows();

        return 0;
    }

    static NSInteger getAccessibilityColumnCount (id self, SEL)
    {
        if (auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (getHandler (self), &AccessibilityHandler::getTableInterface))
            if (auto* tableInterface = tableHandler->getTableInterface())
                return tableInterface->getNumColumns();

        return 0;
    }

    template <typename Getter>
    static NSRange getCellDimensions (id self, Getter getter)
    {
        const auto notFound = NSMakeRange (NSNotFound, 0);

        auto* handler = getHandler (self);

        if (handler == nullptr)
            return notFound;

        auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (getHandler (self), &AccessibilityHandler::getTableInterface);

        if (tableHandler == nullptr)
            return notFound;

        auto* tableInterface = tableHandler->getTableInterface();

        if (tableInterface == nullptr)
            return notFound;

        const auto result = (tableInterface->*getter) (*handler);

        if (! result.hasValue())
            return notFound;

        return NSMakeRange ((NSUInteger) result->begin, (NSUInteger) result->num);
    }

    static NSRange getAccessibilityRowIndexRange (id self, SEL)
    {
        return getCellDimensions (self, &AccessibilityTableInterface::getRowSpan);
    }

    static NSRange getAccessibilityColumnIndexRange (id self, SEL)
    {
        return getCellDimensions (self, &AccessibilityTableInterface::getColumnSpan);
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibleObjCClass)
};

} // namespace drx
