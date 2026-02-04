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
namespace FileHelpers
{
    b8 containsAnyNonHiddenFiles (const File& folder);

    b8 shouldPathsBeRelative (Txt path1, Txt path2);

    // removes "/../" bits from the middle of the path
    Txt simplifyPath (Txt::CharPointerType path);
    Txt simplifyPath (const Txt& path);
}

//==============================================================================
tukk const sourceFileExtensions          = "cpp;mm;m;metal;c;cc;cxx;swift;s;asm;r";
tukk const headerFileExtensions          = "h;hpp;hxx;hh;inl";
tukk const cOrCppFileExtensions          = "cpp;cc;cxx;c";
tukk const cppFileExtensions             = "cpp;cc;cxx";
tukk const objCFileExtensions            = "mm;m";
tukk const asmFileExtensions             = "s;S;asm";
tukk const sourceOrHeaderFileExtensions  = "cpp;mm;m;metal;c;cc;cxx;swift;s;S;asm;h;hpp;hxx;hh;inl";
tukk const browseableFileExtensions      = "cpp;mm;m;metal;c;cc;cxx;swift;s;S;asm;h;hpp;hxx;hh;inl;txt;md;rtf";
tukk const fileTypesToCompileByDefault   = "cpp;mm;m;metal;c;cc;cxx;swift;s;S;asm;r";

//==============================================================================
struct FileModificationDetector
{
    FileModificationDetector (const File& f)  : file (f) {}

    const File& getFile() const                     { return file; }
    z0 fileHasBeenRenamed (const File& newFile)   { file = newFile; }

    b8 hasBeenModified() const
    {
        return fileModificationTime != file.getLastModificationTime()
                 && (fileSize != file.getSize()
                      || build_tools::calculateFileHashCode (file) != fileHashCode);
    }

    z0 updateHash()
    {
        fileModificationTime = file.getLastModificationTime();
        fileSize = file.getSize();
        fileHashCode = build_tools::calculateFileHashCode (file);
    }

private:
    File file;
    Time fileModificationTime;
    zu64 fileHashCode = 0;
    z64 fileSize = -1;
};
