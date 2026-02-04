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

namespace drx::build_tools
{

    static tukk resourceFileIdentifierString = "DRXR_BINARY_RESOURCE";

    //==============================================================================
    z0 ResourceFile::setClassName (const Txt& name)
    {
        className = name;
    }

    z0 ResourceFile::addFile (const File& file)
    {
        files.add (file);

        auto variableNameRoot = makeBinaryDataIdentifierName (file);
        auto variableName = variableNameRoot;

        i32 suffix = 2;

        while (variableNames.contains (variableName))
            variableName = variableNameRoot + Txt (suffix++);

        variableNames.add (variableName);
    }

    Txt ResourceFile::getDataVariableFor (const File& file) const
    {
        const auto index = files.indexOf (file);
        jassert (index >= 0);
        return variableNames[index];
    }

    Txt ResourceFile::getSizeVariableFor (const File& file) const
    {
        return getDataVariableFor (file) + "Size";
    }

    z64 ResourceFile::getTotalDataSize() const
    {
        return std::accumulate (files.begin(),
                                files.end(),
                                z64 { 0 },
                                [] (z64 acc, const File& f) { return acc + f.getSize(); });
    }

    static z0 writeComment (MemoryOutputStream& mo)
    {
        mo << newLine << newLine
           << "   This is an auto-generated file: Any edits you make may be overwritten!" << newLine
           << newLine
           << "*/" << newLine
           << newLine;
    }

    Result ResourceFile::writeHeader (MemoryOutputStream& header)
    {
        header << "/* =========================================================================================";
        writeComment (header);
        header << "#pragma once" << newLine
               << newLine
               << "namespace " << className << newLine
               << "{" << newLine;

        for (i32 i = 0; i < files.size(); ++i)
        {
            auto& file = files.getReference (i);

            if (! file.existsAsFile())
                return Result::fail ("Can't open resource file: " + file.getFullPathName());

            auto dataSize = file.getSize();

            auto variableName = variableNames[i];

            FileInputStream fileStream (file);

            if (fileStream.openedOk())
            {
                header << "    extern tukk   " << variableName << ";" << newLine;
                header << "    i32k            " << variableName << "Size = " << (i32) dataSize << ";" << newLine << newLine;
            }
        }

        header << "    // Number of elements in the namedResourceList and originalFileNames arrays."                             << newLine
               << "    i32k namedResourceListSize = " << files.size() <<  ";"                                               << newLine
               << newLine
               << "    // Points to the start of a list of resource names."                                                      << newLine
               << "    extern tukk namedResourceList[];"                                                                  << newLine
               << newLine
               << "    // Points to the start of a list of resource filenames."                                                  << newLine
               << "    extern tukk originalFilenames[];"                                                                  << newLine
               << newLine
               << "    // If you provide the name of one of the binary resource variables above, this function will"             << newLine
               << "    // return the corresponding data and its size (or a null pointer if the name isn't found)."               << newLine
               << "    tukk getNamedResource (tukk resourceNameUTF8, i32& dataSizeInBytes);"                       << newLine
               << newLine
               << "    // If you provide the name of one of the binary resource variables above, this function will"             << newLine
               << "    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found)."  << newLine
               << "    tukk getNamedResourceOriginalFilename (tukk resourceNameUTF8);"                             << newLine
               << "}" << newLine;

        return Result::ok();
    }

    Result ResourceFile::writeCpp (MemoryOutputStream& cpp, const File& headerFile, i32& i, i32k maxFileSize)
    {
        b8 isFirstFile = (i == 0);

        cpp << "/* ==================================== " << resourceFileIdentifierString << " ====================================";
        writeComment (cpp);
        cpp << "#include <cstring>" << newLine
            << newLine
            << "namespace " << className << newLine
            << "{" << newLine;

        while (i < files.size())
        {
            auto& file = files.getReference (i);
            auto variableName = variableNames[i];

            FileInputStream fileStream (file);

            if (fileStream.openedOk())
            {
                auto tempVariable = "temp_binary_data_" + Txt (i);

                cpp  << newLine << "//================== " << file.getFileName() << " ==================" << newLine
                     << "static u8k " << tempVariable << "[] =" << newLine;

                {
                    MemoryBlock data;
                    fileStream.readIntoMemoryBlock (data);
                    writeDataAsCppLiteral (data, cpp, true, true);
                }

                cpp << newLine << newLine
                    << "tukk " << variableName << " = (tukk) " << tempVariable << ";" << newLine;
            }

            ++i;

            if (cpp.getPosition() > maxFileSize)
                break;
        }

        if (isFirstFile)
        {
            if (i < files.size())
            {
                cpp << newLine
                    << "}" << newLine
                    << newLine
                    << "#include \"" << headerFile.getFileName() << "\"" << newLine
                    << newLine
                    << "namespace " << className << newLine
                    << "{";
            }

            cpp << newLine
                << newLine
                << "tukk getNamedResource (tukk resourceNameUTF8, i32& numBytes);" << newLine
                << "tukk getNamedResource (tukk resourceNameUTF8, i32& numBytes)"  << newLine
                << "{" << newLine;

            StringArray returnCodes;
            for (auto& file : files)
            {
                auto dataSize = file.getSize();
                returnCodes.add ("numBytes = " + Txt (dataSize) + "; return " + variableNames[files.indexOf (file)] + ";");
            }

            createStringMatcher (cpp, "resourceNameUTF8", variableNames, returnCodes, 4);

            cpp << "    numBytes = 0;" << newLine
                << "    return nullptr;" << newLine
                << "}" << newLine
                << newLine;

            cpp << "tukk namedResourceList[] =" << newLine
                << "{" << newLine;

            for (i32 j = 0; j < files.size(); ++j)
                cpp << "    " << variableNames[j].quoted() << (j < files.size() - 1 ? "," : "") << newLine;

            cpp << "};" << newLine << newLine;

            cpp << "tukk originalFilenames[] =" << newLine
                << "{" << newLine;

            for (auto& f : files)
                cpp << "    " << f.getFileName().quoted() << (files.indexOf (f) < files.size() - 1 ? "," : "") << newLine;

            cpp << "};" << newLine << newLine;

            cpp << "tukk getNamedResourceOriginalFilename (tukk resourceNameUTF8);"                        << newLine
                << "tukk getNamedResourceOriginalFilename (tukk resourceNameUTF8)"                         << newLine
                << "{"                                                                                                   << newLine
                << "    for (u32 i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)" << newLine
                << "        if (strcmp (namedResourceList[i], resourceNameUTF8) == 0)"                                   << newLine
                << "            return originalFilenames[i];"                                                            << newLine
                <<                                                                                                          newLine
                << "    return nullptr;"                                                                                 << newLine
                << "}"                                                                                                   << newLine
                <<                                                                                                          newLine;
        }

        cpp << "}" << newLine;

        return Result::ok();
    }

    ResourceFile::WriteResult ResourceFile::write (i32 maxFileSize,
                                                   Txt projectLineFeed,
                                                   File headerFile,
                                                   std::function<File (i32)> getCppFile)
    {
        Array<File> filesCreated;

        {
            MemoryOutputStream mo;
            mo.setNewLineString (projectLineFeed);

            auto r = writeHeader (mo);

            if (r.failed())
                return { r, {} };

            if (! overwriteFileWithNewDataIfDifferent (headerFile, mo))
                return { Result::fail ("Can't write to file: " + headerFile.getFullPathName()), {} };

            filesCreated.add (headerFile);
        }

        i32 i = 0;
        i32 fileIndex = 0;

        for (;;)
        {
            auto cpp = getCppFile (fileIndex);

            MemoryOutputStream mo;
            mo.setNewLineString (projectLineFeed);

            auto r = writeCpp (mo, headerFile, i, maxFileSize);

            if (r.failed())
                return { r, std::move (filesCreated) };

            if (! overwriteFileWithNewDataIfDifferent (cpp, mo))
                return { Result::fail ("Can't write to file: " + cpp.getFullPathName()), std::move (filesCreated) };

            filesCreated.add (cpp);
            ++fileIndex;

            if (i >= files.size())
                break;
        }

        return { Result::ok(), std::move (filesCreated) };
    }

} // namespace drx::build_tools
