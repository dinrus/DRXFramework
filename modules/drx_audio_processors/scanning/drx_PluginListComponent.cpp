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

class PluginListComponent::TableModel  : public TableListBoxModel
{
public:
    TableModel (PluginListComponent& c, KnownPluginList& l)  : owner (c), list (l) {}

    i32 getNumRows() override
    {
        return list.getNumTypes() + list.getBlacklistedFiles().size();
    }

    z0 paintRowBackground (Graphics& g, i32 /*rowNumber*/, i32 /*width*/, i32 /*height*/, b8 rowIsSelected) override
    {
        const auto defaultColor = owner.findColor (ListBox::backgroundColorId);
        const auto c = rowIsSelected ? defaultColor.interpolatedWith (owner.findColor (ListBox::textColorId), 0.5f)
                                     : defaultColor;

        g.fillAll (c);
    }

    enum
    {
        nameCol = 1,
        typeCol = 2,
        categoryCol = 3,
        manufacturerCol = 4,
        descCol = 5
    };

    z0 paintCell (Graphics& g, i32 row, i32 columnId, i32 width, i32 height, b8 /*rowIsSelected*/) override
    {
        Txt text;
        b8 isBlacklisted = row >= list.getNumTypes();

        if (isBlacklisted)
        {
            if (columnId == nameCol)
                text = list.getBlacklistedFiles() [row - list.getNumTypes()];
            else if (columnId == descCol)
                text = TRANS ("Deactivated after failing to initialise correctly");
        }
        else
        {
            auto desc = list.getTypes()[row];

            switch (columnId)
            {
                case nameCol:         text = desc.name; break;
                case typeCol:         text = desc.pluginFormatName; break;
                case categoryCol:     text = desc.category.isNotEmpty() ? desc.category : "-"; break;
                case manufacturerCol: text = desc.manufacturerName; break;
                case descCol:         text = getPluginDescription (desc); break;

                default: jassertfalse; break;
            }
        }

        if (text.isNotEmpty())
        {
            const auto defaultTextColor = owner.findColor (ListBox::textColorId);
            g.setColor (isBlacklisted ? Colors::red
                                       : columnId == nameCol ? defaultTextColor
                                                             : defaultTextColor.interpolatedWith (Colors::transparentBlack, 0.3f));
            g.setFont (owner.withDefaultMetrics (FontOptions ((f32) height * 0.7f, Font::bold)));
            g.drawFittedText (text, 4, 0, width - 6, height, Justification::centredLeft, 1, 0.9f);
        }
    }

    z0 cellClicked (i32 rowNumber, i32 columnId, const drx::MouseEvent& e) override
    {
        TableListBoxModel::cellClicked (rowNumber, columnId, e);

        if (rowNumber >= 0 && rowNumber < getNumRows() && e.mods.isPopupMenu())
            owner.createMenuForRow (rowNumber).showMenuAsync (PopupMenu::Options().withDeletionCheck (owner));
    }

    z0 deleteKeyPressed (i32) override
    {
        owner.removeSelectedPlugins();
    }

    z0 sortOrderChanged (i32 newSortColumnId, b8 isForwards) override
    {
        switch (newSortColumnId)
        {
            case nameCol:         list.sort (KnownPluginList::sortAlphabetically, isForwards); break;
            case typeCol:         list.sort (KnownPluginList::sortByFormat, isForwards); break;
            case categoryCol:     list.sort (KnownPluginList::sortByCategory, isForwards); break;
            case manufacturerCol: list.sort (KnownPluginList::sortByManufacturer, isForwards); break;
            case descCol:         break;

            default: jassertfalse; break;
        }
    }

    static Txt getPluginDescription (const PluginDescription& desc)
    {
        StringArray items;

        if (desc.descriptiveName != desc.name)
            items.add (desc.descriptiveName);

        items.add (desc.version);

        items.removeEmptyStrings();
        return items.joinIntoString (" - ");
    }

    PluginListComponent& owner;
    KnownPluginList& list;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableModel)
};

//==============================================================================
class PluginListComponent::Scanner final : private Timer
{
public:
    Scanner (PluginListComponent& plc, AudioPluginFormat& format, const StringArray& filesOrIdentifiers,
             PropertiesFile* properties, b8 allowPluginsWhichRequireAsynchronousInstantiation, i32 threads,
             const Txt& title, const Txt& text)
        : owner (plc),
          formatToScan (format),
          filesOrIdentifiersToScan (filesOrIdentifiers),
          propertiesToUse (properties),
          pathChooserWindow (TRANS ("Select folders to scan..."), Txt(), MessageBoxIconType::NoIcon),
          progressWindow (title, text, MessageBoxIconType::NoIcon),
          numThreads (threads),
          allowAsync (allowPluginsWhichRequireAsynchronousInstantiation)
    {
        const auto blacklisted = owner.list.getBlacklistedFiles();
        initiallyBlacklistedFiles = std::set<Txt> (blacklisted.begin(), blacklisted.end());

        FileSearchPath path (formatToScan.getDefaultLocationsToSearch());

        // You need to use at least one thread when scanning plug-ins asynchronously
        jassert (! allowAsync || (numThreads > 0));

        // If the filesOrIdentifiersToScan argument isn't empty, we should only scan these
        // If the path is empty, then paths aren't used for this format.
        if (filesOrIdentifiersToScan.isEmpty() && path.getNumPaths() > 0)
        {
           #if ! DRX_IOS
            if (propertiesToUse != nullptr)
                path = getLastSearchPath (*propertiesToUse, formatToScan);
           #endif

            pathList.setSize (500, 300);
            pathList.setPath (path);

            pathChooserWindow.addCustomComponent (&pathList);
            pathChooserWindow.addButton (TRANS ("Scan"),   1, KeyPress (KeyPress::returnKey));
            pathChooserWindow.addButton (TRANS ("Cancel"), 0, KeyPress (KeyPress::escapeKey));

            pathChooserWindow.enterModalState (true,
                                               ModalCallbackFunction::forComponent (startScanCallback,
                                                                                    &pathChooserWindow, this),
                                               false);
        }
        else
        {
            startScan();
        }
    }

    ~Scanner() override
    {
        if (pool != nullptr)
        {
            pool->removeAllJobs (true, 60000);
            pool.reset();
        }
    }

private:
    PluginListComponent& owner;
    AudioPluginFormat& formatToScan;
    StringArray filesOrIdentifiersToScan;
    PropertiesFile* propertiesToUse;
    std::unique_ptr<PluginDirectoryScanner> scanner;
    AlertWindow pathChooserWindow, progressWindow;
    FileSearchPathListComponent pathList;
    Txt pluginBeingScanned;
    f64 progress = 0;
    i32k numThreads;
    b8 allowAsync, timerReentrancyCheck = false;
    std::atomic<b8> finished { false };
    std::unique_ptr<ThreadPool> pool;
    std::set<Txt> initiallyBlacklistedFiles;
    ScopedMessageBox messageBox;

    static z0 startScanCallback (i32 result, AlertWindow* alert, Scanner* scanner)
    {
        if (alert != nullptr && scanner != nullptr)
        {
            if (result != 0)
                scanner->warnUserAboutStupidPaths();
            else
                scanner->finishedScan();
        }
    }

    // Try to dissuade people from to scanning their entire C: drive, or other system folders.
    z0 warnUserAboutStupidPaths()
    {
        for (i32 i = 0; i < pathList.getPath().getNumPaths(); ++i)
        {
            auto f = pathList.getPath().getRawString (i);

            if (File::isAbsolutePath (f) && isStupidPath (File (f)))
            {
                auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                                       TRANS ("Plugin Scanning"),
                                                                       TRANS ("If you choose to scan folders that contain non-plugin files, "
                                                                              "then scanning may take a i64 time, and can cause crashes when "
                                                                              "attempting to load unsuitable files.")
                                                                         + newLine
                                                                         + TRANS ("Are you sure you want to scan the folder \"XYZ\"?")
                                                                            .replace ("XYZ", f),
                                                                       TRANS ("Scan"));
                messageBox = AlertWindow::showScopedAsync (options, [this] (i32 result)
                {
                    if (result != 0)
                        startScan();
                    else
                        finishedScan();
                });

                return;
            }
        }

        startScan();
    }

    static b8 isStupidPath (const File& f)
    {
        Array<File> roots;
        File::findFileSystemRoots (roots);

        if (roots.contains (f))
            return true;

        File::SpecialLocationType pathsThatWouldBeStupidToScan[]
            = { File::globalApplicationsDirectory,
                File::userHomeDirectory,
                File::userDocumentsDirectory,
                File::userDesktopDirectory,
                File::tempDirectory,
                File::userMusicDirectory,
                File::userMoviesDirectory,
                File::userPicturesDirectory };

        for (auto location : pathsThatWouldBeStupidToScan)
        {
            auto sillyFolder = File::getSpecialLocation (location);

            if (f == sillyFolder || sillyFolder.isAChildOf (f))
                return true;
        }

        return false;
    }

    z0 startScan()
    {
        pathChooserWindow.setVisible (false);

        scanner.reset (new PluginDirectoryScanner (owner.list, formatToScan, pathList.getPath(),
                                                   true, owner.deadMansPedalFile, allowAsync));

        if (! filesOrIdentifiersToScan.isEmpty())
        {
            scanner->setFilesOrIdentifiersToScan (filesOrIdentifiersToScan);
        }
        else if (propertiesToUse != nullptr)
        {
            setLastSearchPath (*propertiesToUse, formatToScan, pathList.getPath());
            propertiesToUse->saveIfNeeded();
        }

        progressWindow.addButton (TRANS ("Cancel"), 0, KeyPress (KeyPress::escapeKey));
        progressWindow.addProgressBarComponent (progress);
        progressWindow.enterModalState();

        if (numThreads > 0)
        {
            pool.reset (new ThreadPool (ThreadPoolOptions{}.withNumberOfThreads (numThreads)));

            for (i32 i = numThreads; --i >= 0;)
                pool->addJob (new ScanJob (*this), true);
        }

        startTimer (20);
    }

    z0 finishedScan()
    {
        const auto blacklisted = owner.list.getBlacklistedFiles();
        std::set<Txt> allBlacklistedFiles (blacklisted.begin(), blacklisted.end());

        std::vector<Txt> newBlacklistedFiles;
        std::set_difference (allBlacklistedFiles.begin(), allBlacklistedFiles.end(),
                             initiallyBlacklistedFiles.begin(), initiallyBlacklistedFiles.end(),
                             std::back_inserter (newBlacklistedFiles));

        owner.scanFinished (scanner != nullptr ? scanner->getFailedFiles() : StringArray(),
                            newBlacklistedFiles);
    }

    z0 timerCallback() override
    {
        if (timerReentrancyCheck)
            return;

        progress = scanner->getProgress();

        if (pool == nullptr)
        {
            const ScopedValueSetter<b8> setter (timerReentrancyCheck, true);

            if (doNextScan())
                startTimer (20);
        }

        if (! progressWindow.isCurrentlyModal())
            finished = true;

        if (finished)
            finishedScan();
        else
            progressWindow.setMessage (TRANS ("Testing") + ":\n\n" + pluginBeingScanned);
    }

    b8 doNextScan()
    {
        if (scanner->scanNextFile (true, pluginBeingScanned))
            return true;

        finished = true;
        return false;
    }

    struct ScanJob final : public ThreadPoolJob
    {
        ScanJob (Scanner& s)  : ThreadPoolJob ("pluginscan"), scanner (s) {}

        JobStatus runJob() override
        {
            while (scanner.doNextScan() && ! shouldExit())
            {}

            return jobHasFinished;
        }

        Scanner& scanner;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScanJob)
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Scanner)
};

//==============================================================================
PluginListComponent::PluginListComponent (AudioPluginFormatManager& manager, KnownPluginList& listToEdit,
                                          const File& deadMansPedal, PropertiesFile* const props,
                                          b8 allowPluginsWhichRequireAsynchronousInstantiation)
    : formatManager (manager),
      list (listToEdit),
      deadMansPedalFile (deadMansPedal),
      optionsButton (TRANS ("Options...")),
      propertiesToUse (props),
      allowAsync (allowPluginsWhichRequireAsynchronousInstantiation),
      numThreads (allowAsync ? 1 : 0)
{
    tableModel.reset (new TableModel (*this, listToEdit));

    TableHeaderComponent& header = table.getHeader();

    header.addColumn (TRANS ("Name"),         TableModel::nameCol,         200, 100, 700, TableHeaderComponent::defaultFlags | TableHeaderComponent::sortedForwards);
    header.addColumn (TRANS ("Format"),       TableModel::typeCol,         80, 80, 80,    TableHeaderComponent::notResizable);
    header.addColumn (TRANS ("Category"),     TableModel::categoryCol,     100, 100, 200);
    header.addColumn (TRANS ("Manufacturer"), TableModel::manufacturerCol, 200, 100, 300);
    header.addColumn (TRANS ("Description"),  TableModel::descCol,         300, 100, 500, TableHeaderComponent::notSortable);

    table.setHeaderHeight (22);
    table.setRowHeight (20);
    table.setModel (tableModel.get());
    table.setMultipleSelectionEnabled (true);
    addAndMakeVisible (table);

    addAndMakeVisible (optionsButton);
    optionsButton.onClick = [this]
    {
        createOptionsMenu().showMenuAsync (PopupMenu::Options()
                                              .withDeletionCheck (*this)
                                              .withTargetComponent (optionsButton));
    };

    optionsButton.setTriggeredOnMouseDown (true);

    setSize (400, 600);
    list.addChangeListener (this);
    updateList();
    table.getHeader().reSortTable();

    PluginDirectoryScanner::applyBlacklistingsFromDeadMansPedal (list, deadMansPedalFile);
    deadMansPedalFile.deleteFile();
}

PluginListComponent::~PluginListComponent()
{
    list.removeChangeListener (this);
}

z0 PluginListComponent::setOptionsButtonText (const Txt& newText)
{
    optionsButton.setButtonText (newText);
    resized();
}

z0 PluginListComponent::setScanDialogText (const Txt& title, const Txt& content)
{
    dialogTitle = title;
    dialogText = content;
}

z0 PluginListComponent::setNumberOfThreadsForScanning (i32 num)
{
    numThreads = num;
}

z0 PluginListComponent::resized()
{
    auto r = getLocalBounds().reduced (2);

    if (optionsButton.isVisible())
    {
        optionsButton.setBounds (r.removeFromBottom (24));
        optionsButton.changeWidthToFitText (24);
        r.removeFromBottom (3);
    }

    table.setBounds (r);
}

z0 PluginListComponent::changeListenerCallback (ChangeBroadcaster*)
{
    table.getHeader().reSortTable();
    updateList();
}

z0 PluginListComponent::updateList()
{
    table.updateContent();
    table.repaint();
}

z0 PluginListComponent::removeSelectedPlugins()
{
    auto selected = table.getSelectedRows();

    for (i32 i = table.getNumRows(); --i >= 0;)
        if (selected.contains (i))
            removePluginItem (i);
}

z0 PluginListComponent::setTableModel (TableListBoxModel* model)
{
    table.setModel (nullptr);
    tableModel.reset (model);
    table.setModel (tableModel.get());

    table.getHeader().reSortTable();
    table.updateContent();
    table.repaint();
}

static b8 canShowFolderForPlugin (KnownPluginList& list, i32 index)
{
    return File::createFileWithoutCheckingPath (list.getTypes()[index].fileOrIdentifier).exists();
}

static z0 showFolderForPlugin (KnownPluginList& list, i32 index)
{
    if (canShowFolderForPlugin (list, index))
        File (list.getTypes()[index].fileOrIdentifier).revealToUser();
}

z0 PluginListComponent::removeMissingPlugins()
{
    auto types = list.getTypes();

    for (i32 i = types.size(); --i >= 0;)
    {
        auto type = types.getUnchecked (i);

        if (! formatManager.doesPluginStillExist (type))
            list.removeType (type);
    }
}

z0 PluginListComponent::removePluginItem (i32 index)
{
    if (index < list.getNumTypes())
        list.removeType (list.getTypes()[index]);
    else
        list.removeFromBlacklist (list.getBlacklistedFiles() [index - list.getNumTypes()]);
}

PopupMenu PluginListComponent::createOptionsMenu()
{
    PopupMenu menu;
    menu.addItem (PopupMenu::Item (TRANS ("Clear list"))
                    .setAction ([this] { list.clear(); }));

    menu.addSeparator();

    for (auto format : formatManager.getFormats())
        if (format->canScanForPlugins())
            menu.addItem (PopupMenu::Item (TRANS ("Remove all XFMTX plug-ins").replace ("XFMTX", format->getName()))
                            .setEnabled (! list.getTypesForFormat (*format).isEmpty())
                            .setAction ([this, format]
                                        {
                                            for (auto& pd : list.getTypesForFormat (*format))
                                                list.removeType (pd);
                                        }));

    menu.addSeparator();

    menu.addItem (PopupMenu::Item (TRANS ("Remove selected plug-in from list"))
                    .setEnabled (table.getNumSelectedRows() > 0)
                    .setAction ([this] { removeSelectedPlugins(); }));

    menu.addItem (PopupMenu::Item (TRANS ("Remove any plug-ins whose files no longer exist"))
                    .setAction ([this] { removeMissingPlugins(); }));

    menu.addSeparator();

    auto selectedRow = table.getSelectedRow();

    menu.addItem (PopupMenu::Item (TRANS ("Show folder containing selected plug-in"))
                    .setEnabled (canShowFolderForPlugin (list, selectedRow))
                    .setAction ([this, selectedRow] { showFolderForPlugin (list, selectedRow); }));

    menu.addSeparator();

    for (auto format : formatManager.getFormats())
        if (format->canScanForPlugins())
            menu.addItem (PopupMenu::Item (TRANS ("Scan for new or updated XFMTX plug-ins").replace ("XFMTX", format->getName()))
                            .setAction ([this, format]  { scanFor (*format); }));

    return menu;
}

PopupMenu PluginListComponent::createMenuForRow (i32 rowNumber)
{
    PopupMenu menu;

    if (rowNumber >= 0 && rowNumber < tableModel->getNumRows())
    {
        menu.addItem (PopupMenu::Item (TRANS ("Remove plug-in from list"))
                        .setAction ([this, rowNumber] { removePluginItem (rowNumber); }));

        menu.addItem (PopupMenu::Item (TRANS ("Show folder containing plug-in"))
                        .setEnabled (canShowFolderForPlugin (list, rowNumber))
                        .setAction ([this, rowNumber] { showFolderForPlugin (list, rowNumber); }));
    }

    return menu;
}

b8 PluginListComponent::isInterestedInFileDrag (const StringArray& /*files*/)
{
    return true;
}

z0 PluginListComponent::filesDropped (const StringArray& files, i32, i32)
{
    OwnedArray<PluginDescription> typesFound;
    list.scanAndAddDragAndDroppedFiles (formatManager, files, typesFound);
}

FileSearchPath PluginListComponent::getLastSearchPath (PropertiesFile& properties, AudioPluginFormat& format)
{
    auto key = "lastPluginScanPath_" + format.getName();

    if (properties.containsKey (key) && properties.getValue (key, {}).trim().isEmpty())
        properties.removeValue (key);

    return FileSearchPath (properties.getValue (key, format.getDefaultLocationsToSearch().toString()));
}

z0 PluginListComponent::setLastSearchPath (PropertiesFile& properties, AudioPluginFormat& format,
                                             const FileSearchPath& newPath)
{
    auto key = "lastPluginScanPath_" + format.getName();

    if (newPath.getNumPaths() == 0)
        properties.removeValue (key);
    else
        properties.setValue (key, newPath.toString());
}

//==============================================================================
z0 PluginListComponent::scanFor (AudioPluginFormat& format)
{
    scanFor (format, StringArray());
}

z0 PluginListComponent::scanFor (AudioPluginFormat& format, const StringArray& filesOrIdentifiersToScan)
{
    currentScanner.reset (new Scanner (*this, format, filesOrIdentifiersToScan, propertiesToUse, allowAsync, numThreads,
                                       dialogTitle.isNotEmpty() ? dialogTitle : TRANS ("Scanning for plug-ins..."),
                                       dialogText.isNotEmpty()  ? dialogText  : TRANS ("Searching for all possible plug-in files...")));
}

b8 PluginListComponent::isScanning() const noexcept
{
    return currentScanner != nullptr;
}

z0 PluginListComponent::scanFinished (const StringArray& failedFiles,
                                        const std::vector<Txt>& newBlacklistedFiles)
{
    StringArray warnings;

    const auto addWarningText = [&warnings] (const auto& range, const auto& prefix)
    {
        if (range.size() == 0)
            return;

        StringArray names;

        for (auto& f : range)
            names.add (File::createFileWithoutCheckingPath (f).getFileName());

        warnings.add (prefix + ":\n\n" + names.joinIntoString (", "));
    };

    addWarningText (newBlacklistedFiles,  TRANS ("The following files encountered fatal errors during validation"));
    addWarningText (failedFiles,          TRANS ("The following files appeared to be plugin files, but failed to load correctly"));

    currentScanner.reset(); // mustn't delete this before using the failed files array

    if (! warnings.isEmpty())
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                         TRANS ("Scan complete"),
                                                         warnings.joinIntoString ("\n\n"));
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }
}

} // namespace drx
