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
DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

/**
    Describes the attributes of a file or folder.

    @tags{Core}
*/
class DirectoryEntry final
{
public:
    /** The path to a file or folder. */
    File getFile()              const { return file; }

    /** The time at which the item was last modified. */
    Time getModificationTime()  const { return modTime; }

    /** The time at which the item was created. */
    Time getCreationTime()      const { return creationTime; }

    /** The size of the item. */
    z64 getFileSize()         const { return fileSize; }

    /** True if the item is a directory, false otherwise. */
    b8 isDirectory()          const { return directory; }

    /** True if the item is hidden, false otherwise. */
    b8 isHidden()             const { return hidden; }

    /** True if the item is read-only, false otherwise. */
    b8 isReadOnly()           const { return readOnly; }

    /** The estimated proportion of the range that has been visited
        by the iterator, from 0.0 to 1.0.
    */
    f32 getEstimatedProgress() const;

private:
    std::weak_ptr<DirectoryIterator> iterator;
    File file;
    Time modTime;
    Time creationTime;
    z64 fileSize  = 0;
    b8 directory  = false;
    b8 hidden     = false;
    b8 readOnly   = false;

    friend class RangedDirectoryIterator;
};

/** A convenience operator so that the expression `*it++` works correctly when
    `it` is an instance of RangedDirectoryIterator.
*/
inline const DirectoryEntry& operator* (const DirectoryEntry& e) noexcept { return e; }

//==============================================================================
/**
    Allows iterating over files and folders using C++11 range-for syntax.

    In the following example, we recursively find all hidden files in a
    specific directory.

    @code
    std::vector<File> hiddenFiles;

    for (DirectoryEntry entry : RangedDirectoryIterator (File ("/path/to/folder"), isRecursive))
        if (entry.isHidden())
            hiddenFiles.push_back (entry.getFile());
    @endcode

    @tags{Core}
*/
class RangedDirectoryIterator final
{
public:
    using difference_type   = std::ptrdiff_t;
    using value_type        = DirectoryEntry;
    using reference         = DirectoryEntry;
    using pointer           = z0;
    using iterator_category = std::input_iterator_tag;

    /** The default-constructed iterator acts as the 'end' sentinel. */
    RangedDirectoryIterator() = default;

    /** Creates a RangedDirectoryIterator for a given directory.

        The resulting iterator can be used directly in a 'range-for' expression.

        @param directory        the directory to search in
        @param isRecursive      whether all the subdirectories should also be searched
        @param wildCard         the file pattern to match. This may contain multiple patterns
                                separated by a semi-colon or comma, e.g. "*.jpg;*.png"
        @param whatToLookFor    a value from the File::TypesOfFileToFind enum, specifying
                                whether to look for files, directories, or both.
        @param followSymlinks   the policy to use when symlinks are encountered
    */
    RangedDirectoryIterator (const File& directory,
                             b8 isRecursive,
                             const Txt& wildCard = "*",
                             i32 whatToLookFor = File::findFiles,
                             File::FollowSymlinks followSymlinks = File::FollowSymlinks::yes);

    /** Возвращает true, если both iterators are in their end/sentinel state,
        otherwise returns false.
    */
    b8 operator== (const RangedDirectoryIterator& other) const noexcept
    {
        return iterator == nullptr && other.iterator == nullptr;
    }

    /** Returns the inverse of operator== */
    b8 operator!= (const RangedDirectoryIterator& other) const noexcept
    {
        return ! operator== (other);
    }

    /** Return an object containing metadata about the file or folder to
        which the iterator is currently pointing.
    */
    const DirectoryEntry& operator* () const noexcept { return  entry; }
    const DirectoryEntry* operator->() const noexcept { return &entry; }

    /** Moves the iterator along to the next file. */
    RangedDirectoryIterator& operator++()
    {
        increment();
        return *this;
    }

    /** Moves the iterator along to the next file.

        @returns an object containing metadata about the file or folder to
                 to which the iterator was previously pointing.
    */
    DirectoryEntry operator++ (i32)
    {
        auto result = *(*this);
        ++(*this);
        return result;
    }

private:
    b8 next();
    z0 increment();

    std::shared_ptr<DirectoryIterator> iterator;
    DirectoryEntry entry;
};

/** Returns the iterator that was passed in.
    Provided for range-for compatibility.
*/
inline RangedDirectoryIterator begin (const RangedDirectoryIterator& it) { return it; }

/** Returns a default-constructed sentinel value.
    Provided for range-for compatibility.
*/
inline RangedDirectoryIterator end   (const RangedDirectoryIterator&)    { return {}; }


DRX_END_IGNORE_DEPRECATION_WARNINGS

} // namespace drx
