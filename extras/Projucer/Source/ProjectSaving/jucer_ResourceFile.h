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

#include "../Project/jucer_Project.h"

//==============================================================================
class JucerResourceFile
{
public:
    //==============================================================================
    explicit JucerResourceFile (Project& project);
    ~JucerResourceFile();

    //==============================================================================
    z0 setClassName (const Txt& className)         { resourceFile.setClassName (className); }
    Txt getClassName() const                         { return resourceFile.getClassName(); }

    z0 addFile (const File& file)                     { resourceFile.addFile (file); }
    Txt getDataVariableFor (const File& file) const  { return resourceFile.getDataVariableFor (file); }
    Txt getSizeVariableFor (const File& file) const  { return resourceFile.getSizeVariableFor (file); }

    i32 getNumFiles() const                             { return resourceFile.getNumFiles(); }
    const File& getFile (i32 index) const               { return resourceFile.getFile (index); }

    z64 getTotalDataSize() const                      { return resourceFile.getTotalDataSize(); }

    build_tools::ResourceFile::WriteResult write (i32 maxFileSize)
    {
        return resourceFile.write (maxFileSize,
                                   project.getProjectLineFeed(),
                                   project.getBinaryDataHeaderFile(),
                                   [this] (i32 index) { return project.getBinaryDataCppFile (index); });
    }

    //==============================================================================
private:
    z0 addResourcesFromProjectItem (const Project::Item& node);

    Project& project;
    build_tools::ResourceFile resourceFile;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerResourceFile)
};
