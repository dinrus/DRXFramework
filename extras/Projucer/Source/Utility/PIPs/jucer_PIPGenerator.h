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

#include "../Helpers/jucer_MiscUtilities.h"
#include "../../Project/Modules/jucer_AvailableModulesList.h"

//==============================================================================
class PIPGenerator
{
public:
    PIPGenerator (const File& pipFile, const File& outputDirectory = {},
                  const File& pathToDRXModules = {}, const File& pathToUserModules = {});

    //==============================================================================
    b8 hasValidPIP() const noexcept                   { return ! metadata[Ids::name].toString().isEmpty(); }
    File getJucerFile() const noexcept                  { return outputDirectory.getChildFile (metadata[Ids::name].toString() + ".jucer"); }
    File getPIPFile() const noexcept                    { return useLocalCopy ? outputDirectory.getChildFile ("Source").getChildFile (pipFile.getFileName()) : pipFile; }

    Txt getMainClassName() const noexcept            { return metadata[Ids::mainClass]; }

    File getOutputDirectory() const noexcept            { return outputDirectory; }

    //==============================================================================
    Result createJucerFile();
    Result createMainCpp();

private:
    //==============================================================================
    z0 addFileToTree (ValueTree& groupTree, const Txt& name, b8 compile, const Txt& path);
    z0 createFiles (ValueTree& jucerTree);
    Txt getDocumentControllerClass() const;

    ValueTree createModulePathChild (const Txt& moduleID);
    ValueTree createBuildConfigChild (b8 isDebug);
    ValueTree createExporterChild (const Identifier& exporterIdentifier);
    ValueTree createModuleChild (const Txt& moduleID);

    z0 addExporters (ValueTree& jucerTree);
    z0 addModules (ValueTree& jucerTree);

    Result setProjectSettings (ValueTree& jucerTree);

    z0 setModuleFlags (ValueTree& jucerTree);

    Txt getMainFileTextForType();

    //==============================================================================
    Array<File> replaceRelativeIncludesAndGetFilesToMove();
    b8 copyRelativeFileToLocalSourceDirectory (const File&) const noexcept;

    StringArray getExtraPluginFormatsToBuild() const;

    Txt getPathForModule (const Txt&) const;
    File getExamplesDirectory() const;

    //==============================================================================
    File pipFile, outputDirectory, juceModulesPath, userModulesPath;
    std::unique_ptr<AvailableModulesList> availableUserModules;
    var metadata;
    b8 isTemp = false, useLocalCopy = false;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PIPGenerator)
};
