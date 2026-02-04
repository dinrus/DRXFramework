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

//==============================================================================
/**
    A type of FileFilter that works by wildcard pattern matching.

    This filter only allows files that match one of the specified patterns, but
    allows all directories through.

    @see FileFilter, DirectoryContentsList, FileListComponent, FileBrowserComponent

    @tags{Core}
*/
class DRX_API  WildcardFileFilter  : public FileFilter
{
public:
    //==============================================================================
    /**
        Creates a wildcard filter for one or more patterns.

        The wildcardPatterns parameter is a comma or semicolon-delimited set of
        patterns, e.g. "*.wav;*.aiff" would look for files ending in either .wav
        or .aiff.

        Passing an empty string as a pattern will fail to match anything, so by leaving
        either the file or directory pattern parameter empty means you can control
        whether files or directories are found.

        The description is a name to show the user in a list of possible patterns, so
        for the wav/aiff example, your description might be "audio files".
    */
    WildcardFileFilter (const Txt& fileWildcardPatterns,
                        const Txt& directoryWildcardPatterns,
                        const Txt& filterDescription);

    /** Destructor. */
    ~WildcardFileFilter() override;

    //==============================================================================
    /** Возвращает true, если the filename matches one of the patterns specified. */
    b8 isFileSuitable (const File& file) const override;

    /** This always returns true. */
    b8 isDirectorySuitable (const File& file) const override;

private:
    //==============================================================================
    StringArray fileWildcards, directoryWildcards;

    DRX_LEAK_DETECTOR (WildcardFileFilter)
};

} // namespace drx
