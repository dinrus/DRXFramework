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

DynamicObject::DynamicObject() = default;

DynamicObject::DynamicObject (const DynamicObject& other)
   : ReferenceCountedObject(), properties (other.properties)
{
}

b8 DynamicObject::hasProperty (const Identifier& propertyName) const
{
    const var* const v = properties.getVarPointer (propertyName);
    return v != nullptr && ! v->isMethod();
}

const var& DynamicObject::getProperty (const Identifier& propertyName) const
{
    return properties [propertyName];
}

z0 DynamicObject::setProperty (const Identifier& propertyName, const var& newValue)
{
    properties.set (propertyName, newValue);
    didModifyProperty (propertyName, newValue);
}

z0 DynamicObject::removeProperty (const Identifier& propertyName)
{
    properties.remove (propertyName);
    didModifyProperty (propertyName, std::nullopt);
}

b8 DynamicObject::hasMethod (const Identifier& methodName) const
{
    return getProperty (methodName).isMethod();
}

var DynamicObject::invokeMethod (Identifier method, const var::NativeFunctionArgs& args)
{
    if (auto function = properties [method].getNativeFunction())
        return function (args);

    return {};
}

z0 DynamicObject::setMethod (Identifier name, var::NativeFunction function)
{
    setProperty (name, var (function));
}

z0 DynamicObject::clear()
{
    auto copy = properties;
    properties.clear();

    for (auto& prop : copy)
        didModifyProperty (prop.name, std::nullopt);
}

z0 DynamicObject::cloneAllProperties()
{
    for (i32 i = properties.size(); --i >= 0;)
        if (auto* v = properties.getVarPointerAt (i))
            *v = v->clone();
}

std::unique_ptr<DynamicObject> DynamicObject::clone() const
{
    auto result = std::make_unique<DynamicObject> (*this);
    result->cloneAllProperties();
    return result;
}

z0 DynamicObject::writeAsJSON (OutputStream& out, const JSON::FormatOptions& format)
{
    out << '{';
    if (format.getSpacing() == JSON::Spacing::multiLine)
        out << newLine;

    i32k numValues = properties.size();

    for (i32 i = 0; i < numValues; ++i)
    {
        if (format.getSpacing() == JSON::Spacing::multiLine)
            JSONFormatter::writeSpaces (out, format.getIndentLevel() + JSONFormatter::indentSize);

        out << '"';
        JSONFormatter::writeString (out, properties.getName (i), format.getEncoding());
        out << "\":";

        if (format.getSpacing() != JSON::Spacing::none)
            out << ' ';

        JSON::writeToStream (out,
                             properties.getValueAt (i),
                             format.withIndentLevel (format.getIndentLevel() + JSONFormatter::indentSize));

        if (i < numValues - 1)
        {
            out << ",";

            switch (format.getSpacing())
            {
                case JSON::Spacing::none: break;
                case JSON::Spacing::singleLine: out << ' '; break;
                case JSON::Spacing::multiLine: out << newLine; break;
            }
        }
        else if (format.getSpacing() == JSON::Spacing::multiLine)
            out << newLine;
    }

    if (format.getSpacing() == JSON::Spacing::multiLine)
        JSONFormatter::writeSpaces (out, format.getIndentLevel());

    out << '}';
}

//==============================================================================
//==============================================================================

#if DRX_UNIT_TESTS

class DynamicObjectTests : public UnitTest
{
public:
    DynamicObjectTests() : UnitTest { "DynamicObject", UnitTestCategories::containers } {}

    z0 runTest() override
    {
        struct Action
        {
            Identifier key;
            std::optional<var> value;

            b8 operator== (const Action& other) const
            {
                return other.key == key && other.value == value;
            }
        };

        using Actions = std::vector<Action>;

        struct DerivedObject : public DynamicObject
        {
            explicit DerivedObject (Actions& a) : actions (a) {}

            z0 didModifyProperty (const Identifier& key, const std::optional<var>& value) override
            {
                actions.push_back (Action { key, value });
            }

            Actions& actions;
        };

        Actions actions;
        DerivedObject object { actions };

        beginTest ("didModifyProperty is emitted on setProperty");
        {
            expect (object.getProperties().isEmpty());

            const Identifier key = "foo";
            const var value = 123;
            object.setProperty (key, value);

            expect (actions == Actions { Action { key, value } });
            expect (object.getProperties() == NamedValueSet { { key, value } });
        }

        object.clear();
        actions.clear();

        beginTest ("didModifyProperty is emitted on setMethod");
        {
            expect (object.getProperties().isEmpty());

            const Identifier key = "foo";
            const var::NativeFunction value = [] (const var::NativeFunctionArgs&) { return var{}; };
            object.setMethod (key, value);

            expect (actions.size() == 1);
            expect (actions.back().key == key);

            expect (object.getProperties().size() == 1);
            expect (object.hasMethod (key));
        }

        object.clear();
        actions.clear();

        beginTest ("didModifyProperty is emitted on removeProperty");
        {
            expect (object.getProperties().isEmpty());

            const Identifier key = "bar";
            object.removeProperty (key);

            expect (actions == Actions { Action { key, std::nullopt } });
            expect (object.getProperties().isEmpty());
        }

        object.clear();
        actions.clear();

        beginTest ("didModifyProperty is emitted on clear");
        {
            expect (object.getProperties().isEmpty());

            object.clear();

            expect (actions.empty());

            const Identifier keys[] { "foo", "bar", "baz" };

            for (auto [index, key] : enumerate (keys, i32{}))
                object.setProperty (key, index);

            for (auto& key : keys)
                expect (object.hasProperty (key));

            actions.clear();

            object.clear();

            expect (actions.size() == std::size (keys));

            for (auto& key : keys)
            {
                expect (std::find_if (actions.begin(), actions.end(), [&key] (auto& action)
                {
                    return action.key == key && action.value == std::nullopt;
                }) != actions.end());
            }
        }
    }
};

static DynamicObjectTests dynamicObjectTests;

#endif

} // namespace drx
