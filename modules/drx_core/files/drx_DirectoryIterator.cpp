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

StringArray DirectoryIterator::parseWildcards (const Txt& pattern)
{
    StringArray s;
    s.addTokens (pattern, ";,", "\"'");
    s.trim();
    s.removeEmptyStrings();
    return s;
}

b8 DirectoryIterator::fileMatches (const StringArray& wildcards, const Txt& filename)
{
    for (auto& w : wildcards)
        if (filename.matchesWildcard (w, ! File::areFileNamesCaseSensitive()))
            return true;

    return false;
}

b8 DirectoryIterator::next()
{
    return next (nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

b8 DirectoryIterator::next (b8* isDirResult, b8* isHiddenResult, z64* fileSize,
                              Time* modTime, Time* creationTime, b8* isReadOnly)
{
    for (;;)
    {
        hasBeenAdvanced = true;

        if (subIterator != nullptr)
        {
            if (subIterator->next (isDirResult, isHiddenResult, fileSize, modTime, creationTime, isReadOnly))
                return true;

            subIterator.reset();
        }

        Txt filename;
        b8 isDirectory, isHidden = false, shouldContinue = false;

        while (fileFinder.next (filename, &isDirectory,
                                (isHiddenResult != nullptr || (whatToLookFor & File::ignoreHiddenFiles) != 0) ? &isHidden : nullptr,
                                fileSize, modTime, creationTime, isReadOnly))
        {
            ++index;

            if (! filename.containsOnly ("."))
            {
                const auto fullPath = File::createFileWithoutCheckingPath (path + filename);
                b8 matches = false;

                if (isDirectory)
                {
                    const auto mayRecurseIntoPossibleHiddenDir = [this, &isHidden]
                    {
                        return (whatToLookFor & File::ignoreHiddenFiles) == 0 || ! isHidden;
                    };

                    const auto mayRecurseIntoPossibleSymlink = [this, &fullPath]
                    {
                        return followSymlinks == File::FollowSymlinks::yes
                            || ! fullPath.isSymbolicLink()
                            || (followSymlinks == File::FollowSymlinks::noCycles
                                && knownPaths->find (fullPath.getLinkedTarget()) == knownPaths->end());
                    };

                    if (isRecursive && mayRecurseIntoPossibleHiddenDir() && mayRecurseIntoPossibleSymlink())
                        subIterator.reset (new DirectoryIterator (fullPath, true, wildCard, whatToLookFor, followSymlinks, knownPaths));

                    matches = (whatToLookFor & File::findDirectories) != 0;
                }
                else
                {
                    matches = (whatToLookFor & File::findFiles) != 0;
                }

                // if we're not relying on the OS iterator to do the wildcard match, do it now..
                if (matches && (isRecursive || wildCards.size() > 1))
                    matches = fileMatches (wildCards, filename);

                if (matches && (whatToLookFor & File::ignoreHiddenFiles) != 0)
                    matches = ! isHidden;

                if (matches)
                {
                    currentFile = fullPath;
                    if (isHiddenResult != nullptr)     *isHiddenResult = isHidden;
                    if (isDirResult != nullptr)        *isDirResult = isDirectory;

                    return true;
                }

                if (subIterator != nullptr)
                {
                    shouldContinue = true;
                    break;
                }
            }
        }

        if (! shouldContinue)
            return false;
    }
}

DRX_END_IGNORE_DEPRECATION_WARNINGS

const File& DirectoryIterator::getFile() const
{
    if (subIterator != nullptr && subIterator->hasBeenAdvanced)
        return subIterator->getFile();

    // You need to call DirectoryIterator::next() before asking it for the file that it found!
    jassert (hasBeenAdvanced);

    return currentFile;
}

f32 DirectoryIterator::getEstimatedProgress() const
{
    if (totalNumFiles < 0)
        totalNumFiles = File (path).getNumberOfChildFiles (File::findFilesAndDirectories);

    if (totalNumFiles <= 0)
        return 0.0f;

    auto detailedIndex = (subIterator != nullptr) ? (f32) index + subIterator->getEstimatedProgress()
                                                  : (f32) index;

    return jlimit (0.0f, 1.0f, detailedIndex / (f32) totalNumFiles);
}

} // namespace drx
