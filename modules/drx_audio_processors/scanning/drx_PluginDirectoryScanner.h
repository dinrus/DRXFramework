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
    Scans a directory for plugins, and adds them to a KnownPluginList.

    To use one of these, create it and call scanNextFile() repeatedly, until
    it returns false.

    @tags{Audio}
*/
class DRX_API  PluginDirectoryScanner
{
public:
    //==============================================================================
    /**
        Creates a scanner.

        @param listToAddResultsTo       this will get the new types added to it.
        @param formatToLookFor          this is the type of format that you want to look for
        @param directoriesToSearch      the path to search
        @param searchRecursively        true to search recursively
        @param deadMansPedalFile        if this isn't File(), then it will be used as a file
                                        to store the names of any plugins that crash during
                                        initialisation. If there are any plugins listed in it,
                                        then these will always be scanned after all other possible
                                        files have been tried - in this way, even if there's a few
                                        dodgy plugins in your path, then a couple of rescans
                                        will still manage to find all the proper plugins.
                                        It's probably best to choose a file in the user's
                                        application data directory (alongside your app's
                                        settings file) for this. The file format it uses
                                        is just a list of filenames of the modules that
                                        failed.
       @param allowPluginsWhichRequireAsynchronousInstantiation
                                        If this is false then the scanner will exclude plug-ins
                                        asynchronous creation - such as AUv3 plug-ins.
    */
    PluginDirectoryScanner (KnownPluginList& listToAddResultsTo,
                            AudioPluginFormat& formatToLookFor,
                            FileSearchPath directoriesToSearch,
                            b8 searchRecursively,
                            const File& deadMansPedalFile,
                            b8 allowPluginsWhichRequireAsynchronousInstantiation = false);

    /** Destructor. */
    ~PluginDirectoryScanner();

    //==============================================================================
    /** Sets a specific list of filesOrIdentifiersToScan to scan.
        N.B. This list must match the format passed to the constructor.
        @see AudioPluginFormat::searchPathsForPlugins
    */
    z0 setFilesOrIdentifiersToScan (const StringArray& filesOrIdentifiersToScan);

    /** Tries the next likely-looking file.

        If dontRescanIfAlreadyInList is true, then the file will only be loaded and
        re-tested if it's not already in the list, or if the file's modification
        time has changed since the list was created. If dontRescanIfAlreadyInList is
        false, the file will always be reloaded and tested.
        The nameOfPluginBeingScanned will be updated to the name of the plugin being
        scanned before the scan starts.

        Returns false when there are no more files to try.
    */
    b8 scanNextFile (b8 dontRescanIfAlreadyInList,
                       Txt& nameOfPluginBeingScanned);

    /** Skips over the next file without scanning it.
        Returns false when there are no more files to try.
    */
    b8 skipNextFile();

    /** Returns the description of the plugin that will be scanned during the next
        call to scanNextFile().

        This is handy if you want to show the user which file is currently getting
        scanned.
    */
    Txt getNextPluginFileThatWillBeScanned() const;

    /** Returns the estimated progress, between 0 and 1. */
    f32 getProgress() const                                       { return progress; }

    /** This returns a list of all the filenames of things that looked like being
        a plugin file, but which failed to open for some reason.
    */
    const StringArray& getFailedFiles() const noexcept              { return failedFiles; }

    /** Reads the given dead-mans-pedal file and applies its contents to the list. */
    static z0 applyBlacklistingsFromDeadMansPedal (KnownPluginList& listToApplyTo,
                                                     const File& deadMansPedalFile);

private:
    //==============================================================================
    KnownPluginList& list;
    AudioPluginFormat& format;
    StringArray filesOrIdentifiersToScan;
    File deadMansPedalFile;
    StringArray failedFiles;
    Atomic<i32> nextIndex;
    std::atomic<f32> progress { 0.0f };
    const b8 allowAsync;

    z0 updateProgress();
    z0 setDeadMansPedalFile (const StringArray& newContents);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginDirectoryScanner)
};

} // namespace drx
