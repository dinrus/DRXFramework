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
    Represents a set of folders that make up a search path.

    @see File

    @tags{Core}
*/
class DRX_API  FileSearchPath
{
public:
    //==============================================================================
    /** Creates an empty search path. */
    FileSearchPath() = default;

    /** Destructor. */
    ~FileSearchPath() = default;

    /** Creates a search path from a string of pathnames.

        The path can be semicolon- or comma-separated, e.g.
        "/foo/bar;/foo/moose;/fish/moose"

        The separate folders are tokenised and added to the search path.
    */
    FileSearchPath (const Txt& path);

    /** Creates a copy of another search path. */
    FileSearchPath (const FileSearchPath&);

    /** Copies another search path. */
    FileSearchPath& operator= (const FileSearchPath&);

    /** Uses a string containing a list of pathnames to re-initialise this list.

        This search path is cleared and the semicolon- or comma-separated folders
        in this string are added instead. e.g. "/foo/bar;/foo/moose;/fish/moose"
    */
    FileSearchPath& operator= (const Txt& path);

    //==============================================================================
    /** Returns the number of folders in this search path.
        @see operator[]
    */
    i32 getNumPaths() const;

    /** Returns one of the folders in this search path.
        The file returned isn't guaranteed to actually be a valid directory.
        @see getNumPaths, getRawString
    */
    File operator[] (i32 index) const;

    /** Returns the unaltered text of the folder at the specified index.

        Unlike operator[], this function returns the exact text that was entered. It does not
        attempt to convert the path into an absolute path.

        This may be useful if the directory string is expected to understand environment variables
        or other placeholders that the File constructor doesn't necessarily understand.
        @see operator[]
    */
    Txt getRawString (i32 index) const;

    /** Returns the search path as a semicolon-separated list of directories. */
    Txt toString() const;

    /** Returns the search paths, joined with the provided separator. */
    Txt toStringWithSeparator (StringRef separator) const;

    //==============================================================================
    /** Adds a new directory to the search path.

        The new directory is added to the end of the list if the insertIndex parameter is
        less than zero, otherwise it is inserted at the given index.
    */
    z0 add (const File& directoryToAdd,
              i32 insertIndex = -1);

    /** Adds a new directory to the search path if it's not already in there.

        @return true if the directory has been added, false otherwise.
    */
    b8 addIfNotAlreadyThere (const File& directoryToAdd);

    /** Removes a directory from the search path. */
    z0 remove (i32 indexToRemove);

    /** Merges another search path into this one.
        This will remove any duplicate directories.
    */
    z0 addPath (const FileSearchPath&);

    /** Removes any directories that are actually subdirectories of one of the other directories in the search path.

        If the search is intended to be recursive, there's no point having nested folders in the search
        path, because they'll just get searched twice and you'll get duplicate results.

        e.g. if the path is "c:\abc\de;c:\abc", this method will simplify it to "c:\abc"
    */
    z0 removeRedundantPaths();

    /** Removes any directories that don't actually exist. */
    z0 removeNonExistentPaths();

    //==============================================================================
    /** Searches the path for a wildcard.

        This will search all the directories in the search path in order and return
        an array of the files that were found.

        @param whatToLookFor            a value from the File::TypesOfFileToFind enum, specifying whether to
                                        return files, directories, or both.
        @param searchRecursively        whether to recursively search the subdirectories too
        @param wildCardPattern          a pattern to match against the filenames
        @returns the number of files added to the array
        @see File::findChildFiles
    */
    Array<File> findChildFiles (i32 whatToLookFor,
                                b8 searchRecursively,
                                const Txt& wildCardPattern = "*") const;

    /** Searches the path for a wildcard.
        Note that there's a newer, better version of this method which returns the results
        array, and in almost all cases, you should use that one instead! This one is kept around
        mainly for legacy code to use.
    */
    i32 findChildFiles (Array<File>& results,
                        i32 whatToLookFor,
                        b8 searchRecursively,
                        const Txt& wildCardPattern = "*") const;

    //==============================================================================
    /** Finds out whether a file is inside one of the path's directories.

        This will return true if the specified file is a child of one of the
        directories specified by this path. Note that this doesn't actually do any
        searching or check that the files exist - it just looks at the pathnames
        to work out whether the file would be inside a directory.

        @param fileToCheck      the file to look for
        @param checkRecursively if true, then this will return true if the file is inside a
                                subfolder of one of the path's directories (at any depth). If false
                                it will only return true if the file is actually a direct child
                                of one of the directories.
        @see File::isAChildOf

    */
    b8 isFileInPath (const File& fileToCheck,
                       b8 checkRecursively) const;

private:
    //==============================================================================
    StringArray directories;

    z0 init (const Txt&);

    DRX_LEAK_DETECTOR (FileSearchPath)
};

} // namespace drx
