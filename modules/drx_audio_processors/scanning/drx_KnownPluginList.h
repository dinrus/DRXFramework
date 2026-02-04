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
    Manages a list of plugin types.

    This can be easily edited, saved and loaded, and used to create instances of
    the plugin types in it.

    @see PluginListComponent

    @tags{Audio}
*/
class DRX_API  KnownPluginList   : public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an empty list. */
    KnownPluginList();

    /** Destructor. */
    ~KnownPluginList() override;

    //==============================================================================
    /** Clears the list. */
    z0 clear();

    /** Adds a type manually from its description. */
    b8 addType (const PluginDescription& type);

    /** Removes a type. */
    z0 removeType (const PluginDescription& type);

    /** Returns the number of types currently in the list. */
    i32 getNumTypes() const noexcept;

    /** Returns a copy of the current list. */
    Array<PluginDescription> getTypes() const;

    /** Returns the subset of plugin types for a given format. */
    Array<PluginDescription> getTypesForFormat (AudioPluginFormat&) const;

    /** Looks for a type in the list which comes from this file. */
    std::unique_ptr<PluginDescription> getTypeForFile (const Txt& fileOrIdentifier) const;

    /** Looks for a type in the list which matches a plugin type ID.

        The identifierString parameter must have been created by
        PluginDescription::createIdentifierString().
    */
    std::unique_ptr<PluginDescription> getTypeForIdentifierString (const Txt& identifierString) const;

    /** Looks for all types that can be loaded from a given file, and adds them
        to the list.

        If dontRescanIfAlreadyInList is true, then the file will only be loaded and
        re-tested if it's not already in the list, or if the file's modification
        time has changed since the list was created. If dontRescanIfAlreadyInList is
        false, the file will always be reloaded and tested.

        Возвращает true, если any new types were added, and all the types found in this
        file (even if it was already known and hasn't been re-scanned) get returned
        in the array.
    */
    b8 scanAndAddFile (const Txt& possiblePluginFileOrIdentifier,
                         b8 dontRescanIfAlreadyInList,
                         OwnedArray<PluginDescription>& typesFound,
                         AudioPluginFormat& formatToUse);

    /** Tells a custom scanner that a scan has finished, and it can release any resources. */
    z0 scanFinished();

    /** Возвращает true, если the specified file is already known about and if it
        hasn't been modified since our entry was created.
    */
    b8 isListingUpToDate (const Txt& possiblePluginFileOrIdentifier,
                            AudioPluginFormat& formatToUse) const;

    /** Scans and adds a bunch of files that might have been dragged-and-dropped.
        If any types are found in the files, their descriptions are returned in the array.
    */
    z0 scanAndAddDragAndDroppedFiles (AudioPluginFormatManager& formatManager,
                                        const StringArray& filenames,
                                        OwnedArray<PluginDescription>& typesFound);

    //==============================================================================
    /** Returns the list of blacklisted files. */
    const StringArray& getBlacklistedFiles() const;

    /** Adds a plugin ID to the black-list. */
    z0 addToBlacklist (const Txt& pluginID);

    /** Removes a plugin ID from the black-list. */
    z0 removeFromBlacklist (const Txt& pluginID);

    /** Clears all the blacklisted files. */
    z0 clearBlacklistedFiles();

    //==============================================================================
    /** Sort methods used to change the order of the plugins in the list.
    */
    enum SortMethod
    {
        defaultOrder = 0,
        sortAlphabetically,
        sortByCategory,
        sortByManufacturer,
        sortByFormat,
        sortByFileSystemLocation,
        sortByInfoUpdateTime
    };

    //==============================================================================
    /** Adds the plug-in types to a popup menu so that the user can select one.

        Depending on the sort method, it may add sub-menus for categories,
        manufacturers, etc.

        Use getIndexChosenByMenu() to find out the type that was chosen.
    */
    static z0 addToMenu (PopupMenu& menu, const Array<PluginDescription>& types,
                           SortMethod sortMethod, const Txt& currentlyTickedPluginID = {});

    /** Converts a menu item index that has been chosen into its index in the list.
        Returns -1 if it's not an ID that was used.
        @see addToMenu
    */
    static i32 getIndexChosenByMenu (const Array<PluginDescription>& types, i32 menuResultCode);

    //==============================================================================
    /** Sorts the list. */
    z0 sort (SortMethod method, b8 forwards);

    //==============================================================================
    /** Creates some XML that can be used to store the state of this list. */
    std::unique_ptr<XmlElement> createXml() const;

    /** Recreates the state of this list from its stored XML format. */
    z0 recreateFromXml (const XmlElement& xml);

    //==============================================================================
    /** A structure that recursively holds a tree of plugins.
        @see KnownPluginList::createTree()
    */
    struct PluginTree
    {
        Txt folder; /**< The name of this folder in the tree */
        OwnedArray<PluginTree> subFolders;
        Array<PluginDescription> plugins;
    };

    /** Creates a PluginTree object representing the list of plug-ins. */
    static std::unique_ptr<PluginTree> createTree (const Array<PluginDescription>& types, SortMethod sortMethod);

    //==============================================================================
    /** Class to define a custom plugin scanner */
    class CustomScanner
    {
    public:
        CustomScanner();
        virtual ~CustomScanner();

        /** Attempts to load the given file and find a list of plugins in it.
            @returns true if the plugin loaded, false if it crashed
        */
        virtual b8 findPluginTypesFor (AudioPluginFormat& format,
                                         OwnedArray<PluginDescription>& result,
                                         const Txt& fileOrIdentifier) = 0;

        /** Called when a scan has finished, to allow clean-up of resources. */
        virtual z0 scanFinished();

        /** Возвращает true, если the current scan should be abandoned.
            Any blocking methods should check this value repeatedly and return if
            if becomes true.
        */
        b8 shouldExit() const noexcept;
    };

    /** Supplies a custom scanner to be used in future scans.
        The KnownPluginList will take ownership of the object passed in.
    */
    z0 setCustomScanner (std::unique_ptr<CustomScanner> newScanner);

    //==============================================================================
   #ifndef DOXYGEN
    // These methods have been deprecated! When getting the list of plugin types you should instead use
    // the getTypes() method which returns a copy of the internal PluginDescription array and can be accessed
    // in a thread-safe way.
    [[deprecated]] PluginDescription* getType (i32 index)  noexcept            { return &types.getReference (index); }
    [[deprecated]] const PluginDescription* getType (i32 index) const noexcept { return &types.getReference (index); }
    [[deprecated]] PluginDescription** begin() noexcept                        { jassertfalse; return nullptr; }
    [[deprecated]] PluginDescription* const* begin() const noexcept            { jassertfalse; return nullptr; }
    [[deprecated]] PluginDescription** end() noexcept                          { jassertfalse; return nullptr; }
    [[deprecated]] PluginDescription* const* end() const noexcept              { jassertfalse; return nullptr; }

    // These methods have been deprecated in favour of their static counterparts. You should call getTypes()
    // to store the plug-in list at a point in time and use it when calling these methods.
    [[deprecated]] z0 addToMenu (PopupMenu& menu, SortMethod sortMethod, const Txt& currentlyTickedPluginID = {}) const;
    [[deprecated]] i32 getIndexChosenByMenu (i32 menuResultCode) const;
    [[deprecated]] std::unique_ptr<PluginTree> createTree (SortMethod sortMethod) const;
   #endif

private:
    //==============================================================================
    Array<PluginDescription> types;
    StringArray blacklist;
    std::unique_ptr<CustomScanner> scanner;
    CriticalSection scanLock, typesArrayLock;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnownPluginList)
};

} // namespace drx
