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

#pragma once


//==============================================================================
class ModuleDescription
{
public:
    ModuleDescription() = default;

    ModuleDescription (const File& folder)
       : moduleFolder (folder),
         moduleInfo (parseDRXHeaderMetadata (getHeader()))
    {
    }

    b8 isValid() const                    { return getID().isNotEmpty(); }

    Txt getID() const                    { return moduleInfo [Ids::ID_uppercase].toString(); }
    Txt getVendor() const                { return moduleInfo [Ids::vendor].toString(); }
    Txt getVersion() const               { return moduleInfo [Ids::version].toString(); }
    Txt getName() const                  { return moduleInfo [Ids::name].toString(); }
    Txt getDescription() const           { return moduleInfo [Ids::description].toString(); }
    Txt getLicense() const               { return moduleInfo [Ids::license].toString(); }
    Txt getMinimumCppStandard() const    { return moduleInfo [Ids::minimumCppStandard].toString(); }
    Txt getPreprocessorDefs() const      { return moduleInfo [Ids::defines].toString(); }
    Txt getExtraSearchPaths() const      { return moduleInfo [Ids::searchpaths].toString(); }
    var getModuleInfo() const               { return moduleInfo; }
    File getModuleFolder() const            { return moduleFolder; }

    File getFolder() const
    {
        jassert (moduleFolder != File());

        return moduleFolder;
    }

    File getHeader() const
    {
        if (moduleFolder != File())
        {
            static tukk extensions[] = { ".h", ".hpp", ".hxx" };

            for (auto e : extensions)
            {
                auto header = moduleFolder.getChildFile (moduleFolder.getFileName() + e);

                if (header.existsAsFile())
                    return header;
            }
        }

        return {};
    }

    StringArray getDependencies() const
    {
        auto moduleDependencies = StringArray::fromTokens (moduleInfo ["dependencies"].toString(), " \t;,", "\"'");
        moduleDependencies.trim();
        moduleDependencies.removeEmptyStrings();

        return moduleDependencies;
    }

private:
    File moduleFolder;
    var moduleInfo;
    URL url;
};
