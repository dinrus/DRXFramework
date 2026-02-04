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

PropertySet::PropertySet (b8 ignoreCaseOfKeyNames)
    : properties (ignoreCaseOfKeyNames),
      fallbackProperties (nullptr),
      ignoreCaseOfKeys (ignoreCaseOfKeyNames)
{
}

PropertySet::PropertySet (const PropertySet& other)
    : properties (other.properties),
      fallbackProperties (other.fallbackProperties),
      ignoreCaseOfKeys (other.ignoreCaseOfKeys)
{
}

PropertySet& PropertySet::operator= (const PropertySet& other)
{
    properties = other.properties;
    fallbackProperties = other.fallbackProperties;
    ignoreCaseOfKeys = other.ignoreCaseOfKeys;

    propertyChanged();
    return *this;
}

PropertySet::~PropertySet()
{
}

z0 PropertySet::clear()
{
    const ScopedLock sl (lock);

    if (properties.size() > 0)
    {
        properties.clear();
        propertyChanged();
    }
}

Txt PropertySet::getValue (StringRef keyName, const Txt& defaultValue) const noexcept
{
    const ScopedLock sl (lock);
    auto index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

    if (index >= 0)
        return properties.getAllValues() [index];

    return fallbackProperties != nullptr ? fallbackProperties->getValue (keyName, defaultValue)
                                         : defaultValue;
}

i32 PropertySet::getIntValue (StringRef keyName, i32 defaultValue) const noexcept
{
    const ScopedLock sl (lock);
    auto index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

    if (index >= 0)
        return properties.getAllValues() [index].getIntValue();

    return fallbackProperties != nullptr ? fallbackProperties->getIntValue (keyName, defaultValue)
                                         : defaultValue;
}

f64 PropertySet::getDoubleValue (StringRef keyName, f64 defaultValue) const noexcept
{
    const ScopedLock sl (lock);
    auto index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

    if (index >= 0)
        return properties.getAllValues()[index].getDoubleValue();

    return fallbackProperties != nullptr ? fallbackProperties->getDoubleValue (keyName, defaultValue)
                                         : defaultValue;
}

b8 PropertySet::getBoolValue (StringRef keyName, b8 defaultValue) const noexcept
{
    const ScopedLock sl (lock);
    auto index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

    if (index >= 0)
        return properties.getAllValues() [index].getIntValue() != 0;

    return fallbackProperties != nullptr ? fallbackProperties->getBoolValue (keyName, defaultValue)
                                         : defaultValue;
}

std::unique_ptr<XmlElement> PropertySet::getXmlValue (StringRef keyName) const
{
    return parseXML (getValue (keyName));
}

z0 PropertySet::setValue (StringRef keyName, const var& v)
{
    jassert (keyName.isNotEmpty()); // shouldn't use an empty key name!

    if (keyName.isNotEmpty())
    {
        auto value = v.toString();
        const ScopedLock sl (lock);
        auto index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

        if (index < 0 || properties.getAllValues() [index] != value)
        {
            properties.set (keyName, value);
            propertyChanged();
        }
    }
}

z0 PropertySet::removeValue (StringRef keyName)
{
    if (keyName.isNotEmpty())
    {
        const ScopedLock sl (lock);
        auto index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

        if (index >= 0)
        {
            properties.remove (keyName);
            propertyChanged();
        }
    }
}

z0 PropertySet::setValue (StringRef keyName, const XmlElement* xml)
{
    setValue (keyName, xml == nullptr ? var()
                                      : var (xml->toString (XmlElement::TextFormat().singleLine().withoutHeader())));
}

b8 PropertySet::containsKey (StringRef keyName) const noexcept
{
    const ScopedLock sl (lock);
    return properties.getAllKeys().contains (keyName, ignoreCaseOfKeys);
}

z0 PropertySet::addAllPropertiesFrom (const PropertySet& source)
{
    const ScopedLock sl (source.getLock());

    for (i32 i = 0; i < source.properties.size(); ++i)
        setValue (source.properties.getAllKeys() [i],
                  source.properties.getAllValues() [i]);
}

z0 PropertySet::setFallbackPropertySet (PropertySet* fallbackProperties_) noexcept
{
    const ScopedLock sl (lock);
    fallbackProperties = fallbackProperties_;
}

std::unique_ptr<XmlElement> PropertySet::createXml (const Txt& nodeName) const
{
    auto xml = std::make_unique<XmlElement> (nodeName);

    const ScopedLock sl (lock);

    for (i32 i = 0; i < properties.getAllKeys().size(); ++i)
    {
        auto e = xml->createNewChildElement ("VALUE");
        e->setAttribute ("name", properties.getAllKeys()[i]);
        e->setAttribute ("val", properties.getAllValues()[i]);
    }

    return xml;
}

z0 PropertySet::restoreFromXml (const XmlElement& xml)
{
    const ScopedLock sl (lock);
    clear();

    for (auto* e : xml.getChildWithTagNameIterator ("VALUE"))
    {
        if (e->hasAttribute ("name")
             && e->hasAttribute ("val"))
        {
            properties.set (e->getStringAttribute ("name"),
                            e->getStringAttribute ("val"));
        }
    }

    if (properties.size() > 0)
        propertyChanged();
}

z0 PropertySet::propertyChanged()
{
}

} // namespace drx
