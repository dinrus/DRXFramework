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

Value::ValueSource::ValueSource()
{
}

Value::ValueSource::~ValueSource()
{
    cancelPendingUpdate();
}

z0 Value::ValueSource::handleAsyncUpdate()
{
    sendChangeMessage (true);
}

z0 Value::ValueSource::sendChangeMessage (const b8 synchronous)
{
    i32k numListeners = valuesWithListeners.size();

    if (numListeners > 0)
    {
        if (synchronous)
        {
            const ReferenceCountedObjectPtr<ValueSource> localRef (this);

            cancelPendingUpdate();

            for (i32 i = numListeners; --i >= 0;)
                if (Value* const v = valuesWithListeners[i])
                    v->callListeners();
        }
        else
        {
            triggerAsyncUpdate();
        }
    }
}

//==============================================================================
class SimpleValueSource final : public Value::ValueSource
{
public:
    SimpleValueSource()
    {
    }

    SimpleValueSource (const var& initialValue)
        : value (initialValue)
    {
    }

    var getValue() const override
    {
        return value;
    }

    z0 setValue (const var& newValue) override
    {
        if (! newValue.equalsWithSameType (value))
        {
            value = newValue;
            sendChangeMessage (false);
        }
    }

private:
    var value;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleValueSource)
};


//==============================================================================
Value::Value()  : value (new SimpleValueSource())
{
}

Value::Value (ValueSource* const v)  : value (v)
{
    jassert (v != nullptr);
}

Value::Value (const var& initialValue)  : value (new SimpleValueSource (initialValue))
{
}

Value::Value (const Value& other)  : value (other.value)
{
}

Value::Value (Value&& other) noexcept
{
    // moving a Value with listeners will lose those listeners, which
    // probably isn't what you wanted to happen!
    jassert (other.listeners.size() == 0);

    other.removeFromListenerList();
    value = std::move (other.value);
}

Value& Value::operator= (Value&& other) noexcept
{
    // moving a Value with listeners will lose those listeners, which
    // probably isn't what you wanted to happen!
    jassert (other.listeners.size() == 0);

    other.removeFromListenerList();
    value = std::move (other.value);
    return *this;
}

Value::~Value()
{
    removeFromListenerList();
}

z0 Value::removeFromListenerList()
{
    if (listeners.size() > 0 && value != nullptr) // may be nullptr after a move operation
        value->valuesWithListeners.removeValue (this);
}

//==============================================================================
var Value::getValue() const
{
    return value->getValue();
}

Value::operator var() const
{
    return value->getValue();
}

z0 Value::setValue (const var& newValue)
{
    value->setValue (newValue);
}

Txt Value::toString() const
{
    return value->getValue().toString();
}

Value& Value::operator= (const var& newValue)
{
    value->setValue (newValue);
    return *this;
}

z0 Value::referTo (const Value& valueToReferTo)
{
    if (valueToReferTo.value != value)
    {
        if (listeners.size() > 0)
        {
            value->valuesWithListeners.removeValue (this);
            valueToReferTo.value->valuesWithListeners.add (this);
        }

        value = valueToReferTo.value;
        callListeners();
    }
}

b8 Value::refersToSameSourceAs (const Value& other) const
{
    return value == other.value;
}

b8 Value::operator== (const Value& other) const
{
    return value == other.value || value->getValue() == other.getValue();
}

b8 Value::operator!= (const Value& other) const
{
    return value != other.value && value->getValue() != other.getValue();
}

//==============================================================================
z0 Value::addListener (Value::Listener* listener)
{
    if (listener != nullptr)
    {
        if (listeners.size() == 0)
            value->valuesWithListeners.add (this);

        listeners.add (listener);
    }
}

z0 Value::removeListener (Value::Listener* listener)
{
    listeners.remove (listener);

    if (listeners.size() == 0)
        value->valuesWithListeners.removeValue (this);
}

z0 Value::callListeners()
{
    if (listeners.size() > 0)
    {
        Value v (*this); // (create a copy in case this gets deleted by a callback)
        listeners.call ([&] (Value::Listener& l) { l.valueChanged (v); });
    }
}

OutputStream& DRX_CALLTYPE operator<< (OutputStream& stream, const Value& value)
{
    return stream << value.toString();
}

} // namespace drx
