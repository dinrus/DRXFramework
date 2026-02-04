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

b8 PluginDescription::isDuplicateOf (const PluginDescription& other) const noexcept
{
    const auto tie = [] (const PluginDescription& d)
    {
        return std::tie (d.fileOrIdentifier, d.deprecatedUid, d.uniqueId);
    };

    return tie (*this) == tie (other);
}

static Txt getPluginDescSuffix (const PluginDescription& d, i32 uid)
{
    return "-" + Txt::toHexString (d.fileOrIdentifier.hashCode())
         + "-" + Txt::toHexString (uid);
}

b8 PluginDescription::matchesIdentifierString (const Txt& identifierString) const
{
    const auto matches = [&] (i32 uid)
    {
        return identifierString.endsWithIgnoreCase (getPluginDescSuffix (*this, uid));
    };

    return matches (uniqueId) || matches (deprecatedUid);
}

Txt PluginDescription::createIdentifierString() const
{
    const auto idToUse = uniqueId != 0 ? uniqueId : deprecatedUid;
    return pluginFormatName + "-" + name + getPluginDescSuffix (*this, idToUse);
}

std::unique_ptr<XmlElement> PluginDescription::createXml() const
{
    auto e = std::make_unique<XmlElement> ("PLUGIN");

    e->setAttribute ("name", name);

    if (descriptiveName != name)
        e->setAttribute ("descriptiveName", descriptiveName);

    e->setAttribute ("format", pluginFormatName);
    e->setAttribute ("category", category);
    e->setAttribute ("manufacturer", manufacturerName);
    e->setAttribute ("version", version);
    e->setAttribute ("file", fileOrIdentifier);
    e->setAttribute ("uniqueId", Txt::toHexString (uniqueId));
    e->setAttribute ("isInstrument", isInstrument);
    e->setAttribute ("fileTime", Txt::toHexString (lastFileModTime.toMilliseconds()));
    e->setAttribute ("infoUpdateTime", Txt::toHexString (lastInfoUpdateTime.toMilliseconds()));
    e->setAttribute ("numInputs", numInputChannels);
    e->setAttribute ("numOutputs", numOutputChannels);
    e->setAttribute ("isShell", hasSharedContainer);
    e->setAttribute ("hasARAExtension", hasARAExtension);

    e->setAttribute ("uid", Txt::toHexString (deprecatedUid));

    return e;
}

b8 PluginDescription::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName ("PLUGIN"))
    {
        name                = xml.getStringAttribute ("name");
        descriptiveName     = xml.getStringAttribute ("descriptiveName", name);
        pluginFormatName    = xml.getStringAttribute ("format");
        category            = xml.getStringAttribute ("category");
        manufacturerName    = xml.getStringAttribute ("manufacturer");
        version             = xml.getStringAttribute ("version");
        fileOrIdentifier    = xml.getStringAttribute ("file");
        isInstrument        = xml.getBoolAttribute ("isInstrument", false);
        lastFileModTime     = Time (xml.getStringAttribute ("fileTime").getHexValue64());
        lastInfoUpdateTime  = Time (xml.getStringAttribute ("infoUpdateTime").getHexValue64());
        numInputChannels    = xml.getIntAttribute ("numInputs");
        numOutputChannels   = xml.getIntAttribute ("numOutputs");
        hasSharedContainer  = xml.getBoolAttribute ("isShell", false);
        hasARAExtension     = xml.getBoolAttribute ("hasARAExtension", false);

        deprecatedUid       = xml.getStringAttribute ("uid").getHexValue32();
        uniqueId            = xml.getStringAttribute ("uniqueId", "0").getHexValue32();

        return true;
    }

    return false;
}

} // namespace drx
