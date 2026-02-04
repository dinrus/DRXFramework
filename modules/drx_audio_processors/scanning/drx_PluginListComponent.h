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
    A component displaying a list of plugins, with options to scan for them,
    add, remove and sort them.

    @tags{Audio}
*/
class DRX_API  PluginListComponent   : public Component,
                                        public FileDragAndDropTarget,
                                        private ChangeListener
{
public:
    //==============================================================================
    /**
        Creates the list component.

        For info about the deadMansPedalFile, see the PluginDirectoryScanner constructor.
        The properties file, if supplied, is used to store the user's last search paths.
    */
    PluginListComponent (AudioPluginFormatManager& formatManager,
                         KnownPluginList& listToRepresent,
                         const File& deadMansPedalFile,
                         PropertiesFile* propertiesToUse,
                         b8 allowPluginsWhichRequireAsynchronousInstantiation = false);

    /** Destructor. */
    ~PluginListComponent() override;

    /** Changes the text in the panel's options button. */
    z0 setOptionsButtonText (const Txt& newText);

    /** Returns a pop-up menu that contains all the options for scanning and updating the list. */
    PopupMenu createOptionsMenu();

    /** Returns a menu that can be shown if a row is right-clicked, containing actions
        like "remove plugin" or "show folder" etc.
    */
    PopupMenu createMenuForRow (i32 rowNumber);

    /** Changes the text in the progress dialog box that is shown when scanning. */
    z0 setScanDialogText (const Txt& textForProgressWindowTitle,
                            const Txt& textForProgressWindowDescription);

    /** Sets how many threads to simultaneously scan for plugins.
     If this is 0, then all scanning happens on the message thread (this is the default when
     allowPluginsWhichRequireAsynchronousInstantiation is false). If
     allowPluginsWhichRequireAsynchronousInstantiation is true then numThreads must not
     be zero (it is one by default). */
    z0 setNumberOfThreadsForScanning (i32 numThreads);

    /** Returns the last search path stored in a given properties file for the specified format. */
    static FileSearchPath getLastSearchPath (PropertiesFile&, AudioPluginFormat&);

    /** Stores a search path in a properties file for the given format. */
    static z0 setLastSearchPath (PropertiesFile&, AudioPluginFormat&, const FileSearchPath&);

    /** Triggers an asynchronous scan for the given format. */
    z0 scanFor (AudioPluginFormat&);

    /** Triggers an asynchronous scan for the given format and scans only the given files or identifiers.
        @see AudioPluginFormat::searchPathsForPlugins
    */
    z0 scanFor (AudioPluginFormat&, const StringArray& filesOrIdentifiersToScan);

    /** Возвращает true, если there's currently a scan in progress. */
    b8 isScanning() const noexcept;

    /** Removes the plugins currently selected in the table. */
    z0 removeSelectedPlugins();

    /** Sets a custom table model to be used.
        This will take ownership of the model and delete it when no longer needed.
     */
    z0 setTableModel (TableListBoxModel*);

    /** Returns the table used to display the plugin list. */
    TableListBox& getTableListBox() noexcept            { return table; }

    /** Returns the button used to display the options menu - you can make this invisible
        if you want to hide it and use some other method for showing the menu.
    */
    TextButton& getOptionsButton()                      { return optionsButton; }

    /** @internal */
    z0 resized() override;

private:
    //==============================================================================
    AudioPluginFormatManager& formatManager;
    KnownPluginList& list;
    File deadMansPedalFile;
    TableListBox table;
    TextButton optionsButton;
    PropertiesFile* propertiesToUse;
    Txt dialogTitle, dialogText;
    b8 allowAsync;
    i32 numThreads;

    class TableModel;
    std::unique_ptr<TableListBoxModel> tableModel;

    class Scanner;
    std::unique_ptr<Scanner> currentScanner;

    ScopedMessageBox messageBox;

    z0 scanFinished (const StringArray&, const std::vector<Txt>&);
    z0 updateList();
    z0 removeMissingPlugins();
    z0 removePluginItem (i32 index);

    b8 isInterestedInFileDrag (const StringArray&) override;
    z0 filesDropped (const StringArray&, i32, i32) override;
    z0 changeListenerCallback (ChangeBroadcaster*) override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListComponent)
};

} // namespace drx
