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

ApplicationProperties::~ApplicationProperties()
{
    closeFiles();
}

//==============================================================================
z0 ApplicationProperties::setStorageParameters (const PropertiesFile::Options& newOptions)
{
    options = newOptions;
}

//==============================================================================
z0 ApplicationProperties::openFiles()
{
    // You need to call setStorageParameters() before trying to get hold of the properties!
    jassert (options.applicationName.isNotEmpty());

    if (options.applicationName.isNotEmpty())
    {
        PropertiesFile::Options o (options);

        if (userProps == nullptr)
        {
            o.commonToAllUsers = false;
            userProps.reset (new PropertiesFile (o));
        }

        if (commonProps == nullptr)
        {
            o.commonToAllUsers = true;
            commonProps.reset (new PropertiesFile (o));
        }

        userProps->setFallbackPropertySet (commonProps.get());
    }
}

PropertiesFile* ApplicationProperties::getUserSettings()
{
    if (userProps == nullptr)
        openFiles();

    return userProps.get();
}

PropertiesFile* ApplicationProperties::getCommonSettings (const b8 returnUserPropsIfReadOnly)
{
    if (commonProps == nullptr)
        openFiles();

    if (returnUserPropsIfReadOnly)
    {
        if (commonSettingsAreReadOnly == 0)
            commonSettingsAreReadOnly = commonProps->save() ? -1 : 1;

        if (commonSettingsAreReadOnly > 0)
            return userProps.get();
    }

    return commonProps.get();
}

b8 ApplicationProperties::saveIfNeeded()
{
    return (userProps == nullptr || userProps->saveIfNeeded())
         && (commonProps == nullptr || commonProps->saveIfNeeded());
}

z0 ApplicationProperties::closeFiles()
{
    userProps.reset();
    commonProps.reset();
}

} // namespace drx
