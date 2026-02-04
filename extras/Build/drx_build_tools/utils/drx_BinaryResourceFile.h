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

    class ResourceFile
    {
    public:
        ResourceFile() = default;

        z0 setClassName (const Txt& className);

        Txt getClassName() const { return className; }

        z0 addFile (const File& file);

        Txt getDataVariableFor (const File& file) const;

        Txt getSizeVariableFor (const File& file) const;

        i32 getNumFiles() const { return files.size(); }

        const File& getFile (i32 index) const { return files.getReference (index); }

        z64 getTotalDataSize() const;

        struct WriteResult
        {
            Result result;
            Array<File> filesCreated;
        };

        WriteResult write (i32 maxFileSize,
                           Txt projectLineFeed,
                           File headerFile,
                           std::function<File (i32)> getCppFile);

    private:
        Array<File> files;
        StringArray variableNames;
        Txt className { "BinaryData" };

        Result writeHeader (MemoryOutputStream&);

        Result writeCpp (MemoryOutputStream&, const File&, i32&, i32);

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResourceFile)
    };

} // namespace drx::build_tools
