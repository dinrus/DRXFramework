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
    Represents a dynamically implemented object.

    This class is primarily intended for wrapping scripting language objects,
    but could be used for other purposes.

    An instance of a DynamicObject can be used to store named properties, and
    by subclassing hasMethod() and invokeMethod(), you can give your object
    methods.

    @tags{Core}
*/
class DRX_API  DynamicObject  : public ReferenceCountedObject
{
public:
    //==============================================================================
    DynamicObject();
    DynamicObject (const DynamicObject&);

    using Ptr = ReferenceCountedObjectPtr<DynamicObject>;

    //==============================================================================
    /** Возвращает true, если the object has a property with this name.
        Note that if the property is actually a method, this will return false.
    */
    b8 hasProperty (const Identifier& propertyName) const;

    /** Returns a named property.
        This returns var() if no such property exists.
    */
    const var& getProperty (const Identifier& propertyName) const;

    /** Sets a named property. */
    z0 setProperty (const Identifier& propertyName, const var& newValue);

    /** Removes a named property. */
    z0 removeProperty (const Identifier& propertyName);

    //==============================================================================
    /** Checks whether this object has a property with the given name that has a
        value of type NativeFunction.
    */
    b8 hasMethod (const Identifier& methodName) const;

    /** Invokes a named method on this object.

        The default implementation looks up the named property, and if it's a method
        call, then it invokes it.
    */
    var invokeMethod (Identifier methodName,
                      const var::NativeFunctionArgs& args);

    /** Adds a method to the class.

        This is basically the same as calling setProperty (methodName, (var::NativeFunction) myFunction), but
        helps to avoid accidentally invoking the wrong type of var constructor. It also makes
        the code easier to read.
    */
    z0 setMethod (Identifier methodName, var::NativeFunction function);

    //==============================================================================
    /** Removes all properties and methods from the object. */
    z0 clear();

    /** Returns the NamedValueSet that holds the object's properties. */
    NamedValueSet& getProperties() noexcept                 { return properties; }

    /** Returns the NamedValueSet that holds the object's properties. */
    const NamedValueSet& getProperties() const noexcept     { return properties; }

    /** Calls var::clone() on all the properties that this object contains. */
    z0 cloneAllProperties();

    //==============================================================================
    /** Returns a clone of this object.
        The default implementation of this method just returns a new DynamicObject
        with a (deep) copy of all of its properties. Subclasses can override this to
        implement their own custom copy routines.
    */
    virtual std::unique_ptr<DynamicObject> clone() const;

    //==============================================================================
    /** Writes this object to a text stream in JSON format.
        This method is used by JSON::toString and JSON::writeToStream, and you should
        never need to call it directly, but it's virtual so that custom object types
        can stringify themselves appropriately.
    */
    virtual z0 writeAsJSON (OutputStream&, const JSON::FormatOptions&);

private:
    /** Derived classes may override this function to take additional actions after
        properties are assigned or removed.

        @param name         the name of the property that changed
        @param value        if non-null, the value of the property after assignment
                            if null, indicates that the property was removed
    */
    virtual z0 didModifyProperty ([[maybe_unused]] const Identifier& name,
                                    [[maybe_unused]] const std::optional<var>& value) {}

    //==============================================================================
    NamedValueSet properties;

    DRX_LEAK_DETECTOR (DynamicObject)
};

} // namespace drx
