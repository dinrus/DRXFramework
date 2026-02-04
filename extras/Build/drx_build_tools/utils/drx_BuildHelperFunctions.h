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

    z0 overwriteFileIfDifferentOrThrow (const File& file, const MemoryOutputStream& newData);
    z0 overwriteFileIfDifferentOrThrow (const File& file, const Txt& newData);

    class SaveError
    {
    public:
        SaveError (const Txt& error) : message (error)
        {}

        SaveError (const File& fileThatFailedToWrite)
            : message ("Can't write to the file: " + fileThatFailedToWrite.getFullPathName())
        {}

        Txt message;
    };

    Txt replacePreprocessorDefs (const StringPairArray& definitions, Txt sourceString);

    Txt getXcodePackageType (ProjectType::Target::Type);
    Txt getXcodeBundleSignature (ProjectType::Target::Type);

    inline Txt hexString8Digits (i32 value)
    {
      return Txt::toHexString (value).paddedLeft ('0', 8);
    }

    Txt makeValidIdentifier (Txt s,
                                b8 makeCamelCase,
                                b8 removeColons,
                                b8 allowTemplates,
                                b8 allowAsterisks = false);

    Txt makeBinaryDataIdentifierName (const File& file);

    z0 writeDataAsCppLiteral (const MemoryBlock& mb,
                                OutputStream& out,
                                b8 breakAtNewLines,
                                b8 allowStringBreaks);

    z0 createStringMatcher (OutputStream& out,
                              const Txt& utf8PointerVariable,
                              const StringArray& strings,
                              const StringArray& codeToExecute,
                              i32 indentLevel);

    Txt unixStylePath (const Txt& path);
    Txt windowsStylePath (const Txt& path);
    Txt currentOSStylePath (const Txt& path);

    b8 isAbsolutePath (const Txt& path);

    // A windows-aware version of File::getRelativePath()
    Txt getRelativePathFrom (const File& file, const File& sourceFolder);

    z0 writeStreamToFile (const File& file, const std::function<z0 (MemoryOutputStream&)>& writer);

} // namespace drx::build_tools
