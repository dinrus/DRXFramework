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

static StringArray readDeadMansPedalFile (const File& file)
{
    StringArray lines;
    file.readLines (lines);
    lines.removeEmptyStrings();
    return lines;
}

PluginDirectoryScanner::PluginDirectoryScanner (KnownPluginList& listToAddTo,
                                                AudioPluginFormat& formatToLookFor,
                                                FileSearchPath directoriesToSearch,
                                                const b8 recursive,
                                                const File& deadMansPedal,
                                                b8 allowPluginsWhichRequireAsynchronousInstantiation)
    : list (listToAddTo),
      format (formatToLookFor),
      deadMansPedalFile (deadMansPedal),
      allowAsync (allowPluginsWhichRequireAsynchronousInstantiation)
{
    directoriesToSearch.removeRedundantPaths();
    setFilesOrIdentifiersToScan (format.searchPathsForPlugins (directoriesToSearch, recursive, allowAsync));
}

PluginDirectoryScanner::~PluginDirectoryScanner()
{
    list.scanFinished();
}

//==============================================================================
z0 PluginDirectoryScanner::setFilesOrIdentifiersToScan (const StringArray& filesOrIdentifiers)
{
    filesOrIdentifiersToScan = filesOrIdentifiers;

    // If any plugins have crashed recently when being loaded, move them to the
    // end of the list to give the others a chance to load correctly..
    for (auto& crashed : readDeadMansPedalFile (deadMansPedalFile))
        for (i32 j = filesOrIdentifiersToScan.size(); --j >= 0;)
            if (crashed == filesOrIdentifiersToScan[j])
                filesOrIdentifiersToScan.move (j, -1);

    applyBlacklistingsFromDeadMansPedal (list, deadMansPedalFile);
    nextIndex.set (filesOrIdentifiersToScan.size());
}

Txt PluginDirectoryScanner::getNextPluginFileThatWillBeScanned() const
{
    return format.getNameOfPluginFromIdentifier (filesOrIdentifiersToScan [nextIndex.get() - 1]);
}

z0 PluginDirectoryScanner::updateProgress()
{
    progress = (1.0f - (f32) nextIndex.get() / (f32) filesOrIdentifiersToScan.size());
}

b8 PluginDirectoryScanner::scanNextFile (b8 dontRescanIfAlreadyInList,
                                           Txt& nameOfPluginBeingScanned)
{
    i32k index = --nextIndex;

    if (index >= 0)
    {
        auto file = filesOrIdentifiersToScan [index];

        if (file.isNotEmpty() && ! (dontRescanIfAlreadyInList && list.isListingUpToDate (file, format)))
        {
            nameOfPluginBeingScanned = format.getNameOfPluginFromIdentifier (file);

            OwnedArray<PluginDescription> typesFound;

            // Add this plugin to the end of the dead-man's pedal list in case it crashes...
            auto crashedPlugins = readDeadMansPedalFile (deadMansPedalFile);
            crashedPlugins.removeString (file);
            crashedPlugins.add (file);
            setDeadMansPedalFile (crashedPlugins);

            list.scanAndAddFile (file, dontRescanIfAlreadyInList, typesFound, format);

            // Managed to load without crashing, so remove it from the dead-man's-pedal..
            crashedPlugins.removeString (file);
            setDeadMansPedalFile (crashedPlugins);

            if (typesFound.size() == 0 && ! list.getBlacklistedFiles().contains (file))
                failedFiles.add (file);
        }
    }

    updateProgress();
    return index > 0;
}

b8 PluginDirectoryScanner::skipNextFile()
{
    updateProgress();
    return --nextIndex > 0;
}

z0 PluginDirectoryScanner::setDeadMansPedalFile (const StringArray& newContents)
{
    if (deadMansPedalFile.getFullPathName().isNotEmpty())
        deadMansPedalFile.replaceWithText (newContents.joinIntoString ("\n"), true, true);
}

z0 PluginDirectoryScanner::applyBlacklistingsFromDeadMansPedal (KnownPluginList& list, const File& file)
{
    // If any plugins have crashed recently when being loaded, move them to the
    // end of the list to give the others a chance to load correctly..
    for (auto& crashedPlugin : readDeadMansPedalFile (file))
        list.addToBlacklist (crashedPlugin);
}

} // namespace drx
