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

static z0 parseWildcard (const Txt& pattern, StringArray& result)
{
    result.addTokens (pattern.toLowerCase(), ";,", "\"'");
    result.trim();
    result.removeEmptyStrings();

    // special case for *.*, because people use it to mean "any file", but it
    // would actually ignore files with no extension.
    for (auto& r : result)
        if (r == "*.*")
            r = "*";
}

static b8 matchWildcard (const File& file, const StringArray& wildcards)
{
    auto filename = file.getFileName();

    for (auto& w : wildcards)
        if (filename.matchesWildcard (w, true))
            return true;

    return false;
}

WildcardFileFilter::WildcardFileFilter (const Txt& fileWildcardPatterns,
                                        const Txt& directoryWildcardPatterns,
                                        const Txt& desc)
    : FileFilter (desc.isEmpty() ? fileWildcardPatterns
                                 : (desc + " (" + fileWildcardPatterns + ")"))
{
    parseWildcard (fileWildcardPatterns, fileWildcards);
    parseWildcard (directoryWildcardPatterns, directoryWildcards);
}

WildcardFileFilter::~WildcardFileFilter()
{
}

b8 WildcardFileFilter::isFileSuitable (const File& file) const
{
    return matchWildcard (file, fileWildcards);
}

b8 WildcardFileFilter::isDirectorySuitable (const File& file) const
{
    return matchWildcard (file, directoryWildcards);
}

} // namespace drx
