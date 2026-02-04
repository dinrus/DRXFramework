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

/*
  ==============================================================================

   Utility to turn a bunch of binary files into a .cpp file and .h file full of
   data so they can be built directly into an executable.

   Use this code at your own risk! It carries no warranty!

  ==============================================================================
*/

#include <DrxHeader.h>


//==============================================================================
static i32 addFile (const File& file,
                    const Txt& classname,
                    OutputStream& headerStream,
                    OutputStream& cppStream)
{
    MemoryBlock mb;
    file.loadFileAsData (mb);

    const Txt name (file.getFileName()
                           .replaceCharacter (' ', '_')
                           .replaceCharacter ('.', '_')
                           .retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789"));

    std::cout << "Adding " << name << ": "
              << (i32) mb.getSize() << " bytes" << std::endl;

    headerStream << "    extern tukk  " << name << ";\r\n"
                    "    i32k           " << name << "Size = "
                 << (i32) mb.getSize() << ";\r\n\r\n";

    static i32 tempNum = 0;

    cppStream << "static u8k temp" << ++tempNum << "[] = {";

    size_t i = 0;
    u8k* const data = (u8k*) mb.getData();

    while (i < mb.getSize() - 1)
    {
        if ((i % 40) != 39)
            cppStream << (i32) data[i] << ",";
        else
            cppStream << (i32) data[i] << ",\r\n  ";

        ++i;
    }

    cppStream << (i32) data[i] << ",0,0};\r\n";

    cppStream << "tukk " << classname << "::" << name
              << " = (tukk) temp" << tempNum << ";\r\n\r\n";

    return (i32) mb.getSize();
}

static b8 isHiddenFile (const File& f, const File& root)
{
    return f.getFileName().endsWithIgnoreCase (".scc")
         || f.getFileName() == ".svn"
         || f.getFileName().startsWithChar ('.')
         || (f.getSize() == 0 && ! f.isDirectory())
         || (f.getParentDirectory() != root && isHiddenFile (f.getParentDirectory(), root));
}


//==============================================================================
i32 main (i32 argc, tuk argv[])
{
    std::cout << std::endl << " BinaryBuilder!  Visit www.drx.com for more info." << std::endl;

    if (argc < 4 || argc > 5)
    {
        std::cout << " Usage: BinaryBuilder  sourcedirectory targetdirectory targetclassname [optional wildcard pattern]\n\n"
                     " BinaryBuilder will find all files in the source directory, and encode them\n"
                     " into two files called (targetclassname).cpp and (targetclassname).h, which it\n"
                     " will write into the target directory supplied.\n\n"
                     " Any files in sub-directories of the source directory will be put into the\n"
                     " resultant class, but #ifdef'ed out using the name of the sub-directory (hard to\n"
                     " explain, but obvious when you try it...)\n";

        return 0;
    }

    const File sourceDirectory (File::getCurrentWorkingDirectory()
                                     .getChildFile (Txt (argv[1]).unquoted()));

    if (! sourceDirectory.isDirectory())
    {
        std::cout << "Source directory doesn't exist: "
                  << sourceDirectory.getFullPathName()
                  << std::endl << std::endl;

        return 0;
    }

    const File destDirectory (File::getCurrentWorkingDirectory()
                                   .getChildFile (Txt (argv[2]).unquoted()));

    if (! destDirectory.isDirectory())
    {
        std::cout << "Destination directory doesn't exist: "
                  << destDirectory.getFullPathName() << std::endl << std::endl;

        return 0;
    }

    Txt className (argv[3]);
    className = className.trim();

    const File headerFile (destDirectory.getChildFile (className).withFileExtension (".h"));
    const File cppFile    (destDirectory.getChildFile (className).withFileExtension (".cpp"));

    std::cout << "Creating " << headerFile.getFullPathName()
              << " and " << cppFile.getFullPathName()
              << " from files in " << sourceDirectory.getFullPathName()
              << "..." << std::endl << std::endl;

    auto files = sourceDirectory.findChildFiles (File::findFiles, true,
                                                 (argc > 4) ? argv[4] : "*");

    if (files.isEmpty())
    {
        std::cout << "Didn't find any source files in: "
                  << sourceDirectory.getFullPathName() << std::endl << std::endl;
        return 0;
    }

    headerFile.deleteFile();
    cppFile.deleteFile();

    std::unique_ptr<OutputStream> header (headerFile.createOutputStream());

    if (header == nullptr)
    {
        std::cout << "Couldn't open "
                  << headerFile.getFullPathName() << " for writing" << std::endl << std::endl;
        return 0;
    }

    std::unique_ptr<OutputStream> cpp (cppFile.createOutputStream());

    if (cpp == nullptr)
    {
        std::cout << "Couldn't open "
                  << cppFile.getFullPathName() << " for writing" << std::endl << std::endl;
        return 0;
    }

    *header << "/* (Auto-generated binary data file). */\r\n\r\n"
               "#pragma once\r\n\r\n"
               "namespace " << className << "\r\n"
               "{\r\n";

    *cpp << "/* (Auto-generated binary data file). */\r\n\r\n"
            "#include \"" << className << ".h\"\r\n\r\n";

    i32 totalBytes = 0;

    for (i32 i = 0; i < files.size(); ++i)
    {
        const File file (files[i]);

        // (avoid source control files and hidden files..)
        if (! isHiddenFile (file, sourceDirectory))
        {
            if (file.getParentDirectory() != sourceDirectory)
            {
                *header << "  #ifdef " << file.getParentDirectory().getFileName().toUpperCase() << "\r\n";
                *cpp << "#ifdef " << file.getParentDirectory().getFileName().toUpperCase() << "\r\n";

                totalBytes += addFile (file, className, *header, *cpp);

                *header << "  #endif\r\n";
                *cpp << "#endif\r\n";
            }
            else
            {
                totalBytes += addFile (file, className, *header, *cpp);
            }
        }
    }

    *header << "}\r\n";

    header = nullptr;
    cpp = nullptr;

    std::cout << std::endl << " Total size of binary data: " << totalBytes << " bytes" << std::endl;

    return 0;
}
