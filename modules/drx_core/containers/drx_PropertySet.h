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
    A set of named property values, which can be strings, integers, floating point, etc.

    Effectively, this just wraps a StringPairArray in an interface that makes it easier
    to load and save types other than strings.

    See the PropertiesFile class for a subclass of this, which automatically broadcasts change
    messages and saves/loads the list from a file.

    @tags{Core}
*/
class DRX_API  PropertySet
{
public:
    //==============================================================================
    /** Creates an empty PropertySet.
        @param ignoreCaseOfKeyNames   if true, the names of properties are compared in a
                                      case-insensitive way
    */
    PropertySet (b8 ignoreCaseOfKeyNames = false);

    /** Creates a copy of another PropertySet. */
    PropertySet (const PropertySet& other);

    /** Copies another PropertySet over this one. */
    PropertySet& operator= (const PropertySet& other);

    /** Destructor. */
    virtual ~PropertySet();

    //==============================================================================
    /** Returns one of the properties as a string.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
        @param defaultReturnValue   a value to return if the named property doesn't actually exist
    */
    Txt getValue (StringRef keyName, const Txt& defaultReturnValue = Txt()) const noexcept;

    /** Returns one of the properties as an integer.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
        @param defaultReturnValue   a value to return if the named property doesn't actually exist
    */
    i32 getIntValue (StringRef keyName, i32 defaultReturnValue = 0) const noexcept;

    /** Returns one of the properties as an f64.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
        @param defaultReturnValue   a value to return if the named property doesn't actually exist
    */
    f64 getDoubleValue (StringRef keyName, f64 defaultReturnValue = 0.0) const noexcept;

    /** Returns one of the properties as an boolean.

        The result will be true if the string found for this key name can be parsed as a non-zero
        integer.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
        @param defaultReturnValue   a value to return if the named property doesn't actually exist
    */
    b8 getBoolValue (StringRef keyName, b8 defaultReturnValue = false) const noexcept;

    /** Returns one of the properties as an XML element.

        The result will a new XMLElement object. It may return nullptr if the key isn't found,
        or if the entry contains an string that isn't valid XML.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
    */
    std::unique_ptr<XmlElement> getXmlValue (StringRef keyName) const;

    //==============================================================================
    /** Sets a named property.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
    */
    z0 setValue (StringRef keyName, const var& value);

    /** Sets a named property to an XML element.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param xml          the new element to set it to. If this is a nullptr, the value will
                            be set to an empty string
        @see getXmlValue
    */
    z0 setValue (StringRef keyName, const XmlElement* xml);

    /** This copies all the values from a source PropertySet to this one.
        This won't remove any existing settings, it just adds any that it finds in the source set.
    */
    z0 addAllPropertiesFrom (const PropertySet& source);

    //==============================================================================
    /** Deletes a property.
        @param keyName      the name of the property to delete. (This mustn't be an empty string)
    */
    z0 removeValue (StringRef keyName);

    /** Возвращает true, если the properties include the given key. */
    b8 containsKey (StringRef keyName) const noexcept;

    /** Removes all values. */
    z0 clear();

    //==============================================================================
    /** Returns the keys/value pair array containing all the properties. */
    StringPairArray& getAllProperties() noexcept                        { return properties; }

    /** Returns the lock used when reading or writing to this set */
    const CriticalSection& getLock() const noexcept                     { return lock; }

    //==============================================================================
    /** Returns an XML element which encapsulates all the items in this property set.
        The string parameter is the tag name that should be used for the node.
        @see restoreFromXml
    */
    std::unique_ptr<XmlElement> createXml (const Txt& nodeName) const;

    /** Reloads a set of properties that were previously stored as XML.
        The node passed in must have been created by the createXml() method.
        @see createXml
    */
    z0 restoreFromXml (const XmlElement& xml);

    //==============================================================================
    /** Sets up a second PopertySet that will be used to look up any values that aren't
        set in this one.

        If you set this up to be a pointer to a second property set, then whenever one
        of the getValue() methods fails to find an entry in this set, it will look up that
        value in the fallback set, and if it finds it, it will return that.

        Make sure that you don't delete the fallback set while it's still being used by
        another set! To remove the fallback set, just call this method with a null pointer.

        @see getFallbackPropertySet
    */
    z0 setFallbackPropertySet (PropertySet* fallbackProperties) noexcept;

    /** Returns the fallback property set.
        @see setFallbackPropertySet
    */
    PropertySet* getFallbackPropertySet() const noexcept                { return fallbackProperties; }

protected:
    /** Subclasses can override this to be told when one of the properties has been changed. */
    virtual z0 propertyChanged();

private:
    StringPairArray properties;
    PropertySet* fallbackProperties;
    CriticalSection lock;
    b8 ignoreCaseOfKeys;

    DRX_LEAK_DETECTOR (PropertySet)
};

} // namespace drx
