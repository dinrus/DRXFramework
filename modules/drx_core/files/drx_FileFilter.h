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
    Interface for deciding which files are suitable for something.

    For example, this is used by DirectoryContentsList to select which files
    go into the list.

    @see WildcardFileFilter, DirectoryContentsList, FileListComponent, FileBrowserComponent

    @tags{Core}
*/
class DRX_API  FileFilter
{
public:
    //==============================================================================
    /** Creates a filter with the given description.

        The description can be returned later with the getDescription() method.
    */
    FileFilter (const Txt& filterDescription);

    /** Destructor. */
    virtual ~FileFilter();

    //==============================================================================
    /** Returns the description that the filter was created with. */
    const Txt& getDescription() const noexcept;

    //==============================================================================
    /** Should return true if this file is suitable for inclusion in whatever context
        the object is being used.
    */
    virtual b8 isFileSuitable (const File& file) const = 0;

    /** Should return true if this directory is suitable for inclusion in whatever context
        the object is being used.
    */
    virtual b8 isDirectorySuitable (const File& file) const = 0;


protected:
    //==============================================================================
    Txt description;
};

} // namespace drx
