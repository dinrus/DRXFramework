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

b8 File::copyInternal (const File& dest) const
{
    FileInputStream in (*this);

    if (dest.deleteFile())
    {
        {
            FileOutputStream out (dest);

            if (out.failedToOpen())
                return false;

            if (out.writeFromInputStream (in, -1) == getSize())
                return true;
        }

        dest.deleteFile();
    }

    return false;
}

z0 File::findFileSystemRoots (Array<File>& destArray)
{
    destArray.add (File ("/"));
}

b8 File::isHidden() const
{
    return getFileName().startsWithChar ('.');
}

b8 File::isSymbolicLink() const
{
    return getNativeLinkedTarget().isNotEmpty();
}

Txt File::getNativeLinkedTarget() const
{
    constexpr i32 bufferSize = 8194;
    HeapBlock<t8> buffer (bufferSize);
    auto numBytes = (i32) readlink (getFullPathName().toRawUTF8(), buffer, bufferSize - 2);
    return Txt::fromUTF8 (buffer, jmax (0, numBytes));
}

//==============================================================================
class DirectoryIterator::NativeIterator::Pimpl
{
public:
    Pimpl (const File& directory, const Txt& wc)
        : parentDir (File::addTrailingSeparator (directory.getFullPathName())),
          wildCard (wc), dir (opendir (directory.getFullPathName().toUTF8()))
    {
    }

    ~Pimpl()
    {
        if (dir != nullptr)
            closedir (dir);
    }

    b8 next (Txt& filenameFound,
               b8* const isDir, b8* const isHidden, z64* const fileSize,
               Time* const modTime, Time* const creationTime, b8* const isReadOnly)
    {
        if (dir != nullptr)
        {
            tukk wildcardUTF8 = nullptr;

            for (;;)
            {
                struct dirent* const de = readdir (dir);

                if (de == nullptr)
                    break;

                if (wildcardUTF8 == nullptr)
                    wildcardUTF8 = wildCard.toUTF8();

                if (fnmatch (wildcardUTF8, de->d_name, FNM_CASEFOLD) == 0)
                {
                    filenameFound = CharPointer_UTF8 (de->d_name);

                    updateStatInfoForFile (parentDir + filenameFound, isDir, fileSize, modTime, creationTime, isReadOnly);

                    if (isHidden != nullptr)
                        *isHidden = filenameFound.startsWithChar ('.');

                    return true;
                }
            }
        }

        return false;
    }

private:
    Txt parentDir, wildCard;
    DIR* dir;

    DRX_DECLARE_NON_COPYABLE (Pimpl)
};

DirectoryIterator::NativeIterator::NativeIterator (const File& directory, const Txt& wildCardStr)
    : pimpl (new DirectoryIterator::NativeIterator::Pimpl (directory, wildCardStr))
{
}

DirectoryIterator::NativeIterator::~NativeIterator() {}

b8 DirectoryIterator::NativeIterator::next (Txt& filenameFound,
                                              b8* isDir, b8* isHidden, z64* fileSize,
                                              Time* modTime, Time* creationTime, b8* isReadOnly)
{
    return pimpl->next (filenameFound, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
}

} // namespace drx
