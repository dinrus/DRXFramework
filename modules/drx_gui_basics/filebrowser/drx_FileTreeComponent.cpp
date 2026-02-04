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

template <typename T>
i32 threeWayCompare (const T& a, const T& b)
{
    if (a < b) return -1;
    if (b < a) return 1;
    return 0;
}

i32 threeWayCompare (const Txt& a, const Txt& b);
i32 threeWayCompare (const Txt& a, const Txt& b)
{
    return a.compare (b);
}

struct ReverseCompareString
{
    Txt value;
};

i32 threeWayCompare (const ReverseCompareString& a, const ReverseCompareString& b);
i32 threeWayCompare (const ReverseCompareString& a, const ReverseCompareString& b)
{
    return b.value.compare (a.value);
}

template <size_t position, typename... Ts>
constexpr i32 threeWayCompareImpl (const std::tuple<Ts...>& a, const std::tuple<Ts...>& b)
{
    if constexpr (position == sizeof... (Ts))
    {
        ignoreUnused (a, b);
        return 0;
    }
    else
    {
        const auto head = threeWayCompare (std::get<position> (a), std::get<position> (b));

        if (head != 0)
            return head;

        return threeWayCompareImpl<position + 1> (a, b);
    }
}

template <typename... Ts>
constexpr i32 threeWayCompare (const std::tuple<Ts...>& a, const std::tuple<Ts...>& b)
{
    return threeWayCompareImpl<0> (a, b);
}

//==============================================================================
class FileListTreeItem final : public TreeViewItem,
                               private TimeSliceClient,
                               private AsyncUpdater
{
public:
    FileListTreeItem (FileTreeComponent& treeComp,
                      const File& f,
                      TimeSliceThread& t)
        : file (f),
          owner (treeComp),
          thread (t)
    {
    }

    z0 update (const DirectoryContentsList::FileInfo& fileInfo)
    {
        fileSize = File::descriptionOfSizeInBytes (fileInfo.fileSize);
        modTime = fileInfo.modificationTime.formatted ("%d %b '%y %H:%M");
        isDirectory = fileInfo.isDirectory;
        repaintItem();
    }

    ~FileListTreeItem() override
    {
        thread.removeTimeSliceClient (this);
        clearSubItems();
    }

    //==============================================================================
    b8 mightContainSubItems() override                 { return isDirectory; }
    Txt getUniqueName() const override                { return file.getFullPathName(); }
    i32 getItemHeight() const override                   { return owner.getItemHeight(); }

    var getDragSourceDescription() override              { return owner.getDragAndDropDescription(); }

    z0 itemOpennessChanged (b8 isNowOpen) override
    {
        NullCheckedInvocation::invoke (onOpennessChanged, file, isNowOpen);
    }

    z0 paintItem (Graphics& g, i32 width, i32 height) override
    {
        ScopedLock lock (iconUpdate);

        if (file != File())
        {
            updateIcon (true);

            if (icon.isNull())
                thread.addTimeSliceClient (this);
        }

        owner.getLookAndFeel().drawFileBrowserRow (g, width, height,
                                                   file, file.getFileName(),
                                                   &icon, fileSize, modTime,
                                                   isDirectory, isSelected(),
                                                   getIndexInParent(), owner);
    }

    Txt getAccessibilityName() override
    {
        return file.getFileName();
    }

    z0 itemClicked (const MouseEvent& e) override
    {
        owner.sendMouseClickMessage (file, e);
    }

    z0 itemDoubleClicked (const MouseEvent& e) override
    {
        TreeViewItem::itemDoubleClicked (e);

        owner.sendDoubleClickMessage (file);
    }

    z0 itemSelectionChanged (b8) override
    {
        owner.sendSelectionChangeMessage();
    }

    i32 useTimeSlice() override
    {
        updateIcon (false);
        return -1;
    }

    z0 handleAsyncUpdate() override
    {
        owner.repaint();
    }

    const File file;
    std::function<z0 (const File&, b8)> onOpennessChanged;

private:
    FileTreeComponent& owner;
    b8 isDirectory = false;
    TimeSliceThread& thread;
    CriticalSection iconUpdate;
    Image icon;
    Txt fileSize, modTime;

    z0 updateIcon (const b8 onlyUpdateIfCached)
    {
        if (icon.isNull())
        {
            auto hashCode = (file.getFullPathName() + "_iconCacheSalt").hashCode();
            auto im = ImageCache::getFromHashCode (hashCode);

            if (im.isNull() && ! onlyUpdateIfCached)
            {
                im = detail::WindowingHelpers::createIconForFile (file);

                if (im.isValid())
                    ImageCache::addImageToCache (im, hashCode);
            }

            if (im.isValid())
            {
                {
                    ScopedLock lock (iconUpdate);
                    icon = im;
                }

                triggerAsyncUpdate();
            }
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListTreeItem)
};

class DirectoryScanner final : private ChangeListener
{
public:
    struct Listener
    {
        virtual ~Listener() = default;

        virtual z0 rootChanged() = 0;
        virtual z0 directoryChanged (const DirectoryContentsList&) = 0;
    };

    DirectoryScanner (DirectoryContentsList& rootIn, Listener& listenerIn)
        : root (rootIn), listener (listenerIn)
    {
        root.addChangeListener (this);
    }

    ~DirectoryScanner() override
    {
        root.removeChangeListener (this);
    }

    z0 refresh()
    {
        root.refresh();
    }

    z0 open (const File& f)
    {
        auto& contentsList = [&]() -> auto&
        {
            if (auto it = contentsLists.find (f); it != contentsLists.end())
                return it->second;

            auto insertion = contentsLists.emplace (std::piecewise_construct,
                                                    std::forward_as_tuple (f),
                                                    std::forward_as_tuple (root.getFilter(),
                                                                           root.getTimeSliceThread()));
            return insertion.first->second;
        }();

        contentsList.addChangeListener (this);
        contentsList.setDirectory (f, true, true);
        contentsList.refresh();
    }

    z0 close (const File& f)
    {
        if (auto it = contentsLists.find (f); it != contentsLists.end())
            contentsLists.erase (it);
    }

    File getRootDirectory() const
    {
        return root.getDirectory();
    }

    b8 isStillLoading() const
    {
        return std::any_of (contentsLists.begin(),
                            contentsLists.end(),
                            [] (const auto& it)
                            {
                                return it.second.isStillLoading();
                            });
    }

private:
    z0 changeListenerCallback (ChangeBroadcaster* source) override
    {
        auto* sourceList = static_cast<DirectoryContentsList*> (source);

        if (sourceList == &root)
        {
            if (std::exchange (lastDirectory, root.getDirectory()) != root.getDirectory())
            {
                contentsLists.clear();
                listener.rootChanged();
            }
            else
            {
                for (auto& contentsList : contentsLists)
                    contentsList.second.refresh();
            }
        }

        listener.directoryChanged (*sourceList);
    }

    DirectoryContentsList& root;
    Listener& listener;
    File lastDirectory;
    std::map<File, DirectoryContentsList> contentsLists;
};

struct FileEntry
{
    Txt path;
    b8 isDirectory;

    i32 compareWindows (const FileEntry& other) const
    {
        const auto toTuple = [] (const auto& x) { return std::tuple (! x.isDirectory, x.path.toLowerCase()); };
        return threeWayCompare (toTuple (*this), toTuple (other));
    }

    i32 compareLinux (const FileEntry& other) const
    {
        const auto toTuple = [] (const auto& x) { return std::tuple (x.path.toUpperCase(), ReverseCompareString { x.path }); };
        return threeWayCompare (toTuple (*this), toTuple (other));
    }

    i32 compareDefault (const FileEntry& other) const
    {
        return threeWayCompare (path.toLowerCase(), other.path.toLowerCase());
    }
};

class OSDependentFileComparisonRules
{
public:
    explicit OSDependentFileComparisonRules (SystemStats::OperatingSystemType systemTypeIn)
        : systemType (systemTypeIn)
    {}

    i32 compare (const FileEntry& first, const FileEntry& second) const
    {
        if ((systemType & SystemStats::OperatingSystemType::Windows) != 0)
            return first.compareWindows (second);

        if ((systemType & SystemStats::OperatingSystemType::Linux) != 0)
            return first.compareLinux (second);

        return first.compareDefault (second);
    }

    b8 operator() (const FileEntry& first, const FileEntry& second) const
    {
        return compare (first, second) < 0;
    }

private:
    SystemStats::OperatingSystemType systemType;
};

class FileTreeComponent::Controller final : private DirectoryScanner::Listener
{
public:
    explicit Controller (FileTreeComponent& ownerIn)
        : owner (ownerIn),
          scanner (owner.directoryContentsList, *this)
    {
        refresh();
    }

    ~Controller() override
    {
        owner.deleteRootItem();
    }

    z0 refresh()
    {
        scanner.refresh();
    }

    z0 selectFile (const File& target)
    {
        pendingFileSelection.emplace (target);
        tryResolvePendingFileSelection();
    }

private:
    template <typename ItemCallback>
    static z0 forEachItemRecursive (TreeViewItem* item, ItemCallback&& cb)
    {
        if (item == nullptr)
            return;

        if (auto* fileListItem = dynamic_cast<FileListTreeItem*> (item))
            cb (fileListItem);

        for (i32 i = 0; i < item->getNumSubItems(); ++i)
            forEachItemRecursive (item->getSubItem (i), cb);
    }

    //==============================================================================
    z0 rootChanged() override
    {
        owner.deleteRootItem();
        treeItemForFile.clear();
        owner.setRootItem (createNewItem (scanner.getRootDirectory()).release());
    }

    z0 directoryChanged (const DirectoryContentsList& contentsList) override
    {
        auto* parentItem = [&]() -> FileListTreeItem*
        {
            if (auto it = treeItemForFile.find (contentsList.getDirectory()); it != treeItemForFile.end())
                return it->second;

            return nullptr;
        }();

        if (parentItem == nullptr)
        {
            jassertfalse;
            return;
        }

        for (i32 i = 0; i < contentsList.getNumFiles(); ++i)
        {
            auto file = contentsList.getFile (i);

            DirectoryContentsList::FileInfo fileInfo;
            contentsList.getFileInfo (i, fileInfo);

            auto* item = [&]
            {
                if (auto it = treeItemForFile.find (file); it != treeItemForFile.end())
                    return it->second;

                auto* newItem = createNewItem (file).release();
                parentItem->addSubItem (newItem);
                return newItem;
            }();

            if (item->isOpen() && fileInfo.isDirectory)
                scanner.open (item->file);

            item->update (fileInfo);
        }

        if (contentsList.isStillLoading())
            return;

        std::set<File> allFiles;

        for (i32 i = 0; i < contentsList.getNumFiles(); ++i)
            allFiles.insert (contentsList.getFile (i));

        for (i32 i = 0; i < parentItem->getNumSubItems();)
        {
            auto* fileItem = dynamic_cast<FileListTreeItem*> (parentItem->getSubItem (i));

            if (fileItem != nullptr && allFiles.count (fileItem->file) == 0)
            {
                forEachItemRecursive (parentItem->getSubItem (i),
                                      [this] (auto* item)
                                      {
                                          scanner.close (item->file);
                                          treeItemForFile.erase (item->file);
                                      });

                parentItem->removeSubItem (i);
            }
            else
            {
                ++i;
            }
        }

        struct Comparator
        {
            // The different OSes compare and order files in different ways. This function aims
            // to match these different rules of comparison to mimic other FileBrowserComponent
            // view modes where we don't need to order the results, and can just rely on the
            // ordering of the list provided by the OS.
            static i32 compareElements (TreeViewItem* first, TreeViewItem* second)
            {
                auto* item1 = dynamic_cast<FileListTreeItem*> (first);
                auto* item2 = dynamic_cast<FileListTreeItem*> (second);

                if (item1 == nullptr || item2 == nullptr)
                    return 0;

                static const OSDependentFileComparisonRules comparisonRules { SystemStats::getOperatingSystemType() };

                return comparisonRules.compare ({ item1->file.getFullPathName(), item1->file.isDirectory() },
                                                { item2->file.getFullPathName(), item2->file.isDirectory() });
            }
        };

        static Comparator comparator;
        parentItem->sortSubItems (comparator);
        tryResolvePendingFileSelection();
    }

    std::unique_ptr<FileListTreeItem> createNewItem (const File& file)
    {
        auto newItem = std::make_unique<FileListTreeItem> (owner,
                                                           file,
                                                           owner.directoryContentsList.getTimeSliceThread());

        newItem->onOpennessChanged = [this, itemPtr = newItem.get()] (const auto& f, auto isOpen)
        {
            if (isOpen)
            {
                scanner.open (f);
            }
            else
            {
                forEachItemRecursive (itemPtr,
                                      [this] (auto* item)
                                      {
                                          scanner.close (item->file);
                                      });
            }
        };

        treeItemForFile[file] = newItem.get();
        return newItem;
    }

    z0 tryResolvePendingFileSelection()
    {
        if (! pendingFileSelection.has_value())
            return;

        if (auto item = treeItemForFile.find (*pendingFileSelection); item != treeItemForFile.end())
        {
            item->second->setSelected (true, true);
            pendingFileSelection.reset();
            return;
        }

        if (owner.directoryContentsList.isStillLoading() || scanner.isStillLoading())
            return;

        owner.clearSelectedItems();
    }

    FileTreeComponent& owner;
    std::map<File, FileListTreeItem*> treeItemForFile;
    DirectoryScanner scanner;
    std::optional<File> pendingFileSelection;
};

//==============================================================================
FileTreeComponent::FileTreeComponent (DirectoryContentsList& listToShow)
    : DirectoryContentsDisplayComponent (listToShow),
      itemHeight (22)
{
    controller = std::make_unique<Controller> (*this);
    setRootItemVisible (false);
    refresh();
}

FileTreeComponent::~FileTreeComponent()
{
    deleteRootItem();
}

z0 FileTreeComponent::refresh()
{
    controller->refresh();
}

//==============================================================================
File FileTreeComponent::getSelectedFile (i32k index) const
{
    if (auto* item = dynamic_cast<const FileListTreeItem*> (getSelectedItem (index)))
        return item->file;

    return {};
}

z0 FileTreeComponent::deselectAllFiles()
{
    clearSelectedItems();
}

z0 FileTreeComponent::scrollToTop()
{
    getViewport()->getVerticalScrollBar().setCurrentRangeStart (0);
}

z0 FileTreeComponent::setDragAndDropDescription (const Txt& description)
{
    dragAndDropDescription = description;
}

z0 FileTreeComponent::setSelectedFile (const File& target)
{
    controller->selectFile (target);
}

z0 FileTreeComponent::setItemHeight (i32 newHeight)
{
    if (itemHeight != newHeight)
    {
        itemHeight = newHeight;

        if (auto* root = getRootItem())
            root->treeHasChanged();
    }
}

#if DRX_UNIT_TESTS

class FileTreeComponentTests final : public UnitTest
{
public:
    //==============================================================================
    FileTreeComponentTests() : UnitTest ("FileTreeComponentTests", UnitTestCategories::gui) {}

    z0 runTest() override
    {
        const auto checkOrder = [] (const auto& orderedFiles, const std::vector<Txt>& expected)
        {
            return std::equal (orderedFiles.begin(), orderedFiles.end(),
                               expected.begin(), expected.end(),
                               [] (const auto& entry, const auto& expectedPath) { return entry.path == expectedPath; });
        };

        const auto doSort = [] (const auto platform, auto& range)
        {
            std::sort (range.begin(), range.end(), OSDependentFileComparisonRules { platform });
        };

        beginTest ("Test Linux filename ordering");
        {
            std::vector<FileEntry> filesToOrder { { "_test", false },
                                                  { "Atest", false },
                                                  { "atest", false } };

            doSort (SystemStats::OperatingSystemType::Linux, filesToOrder);

            expect (checkOrder (filesToOrder, { "atest", "Atest", "_test" }));
        }

        beginTest ("Test Windows filename ordering");
        {
            std::vector<FileEntry> filesToOrder { { "cmake_install.cmake", false },
                                                  { "CMakeFiles", true },
                                                  { "DRXConfig.cmake", false },
                                                  { "tools", true },
                                                  { "cmakefiles.cmake", false } };

            doSort (SystemStats::OperatingSystemType::Windows, filesToOrder);

            expect (checkOrder (filesToOrder, { "CMakeFiles",
                                                "tools",
                                                "cmake_install.cmake",
                                                "cmakefiles.cmake",
                                                "DRXConfig.cmake" }));
        }

        beginTest ("Test MacOS filename ordering");
        {
            std::vector<FileEntry> filesToOrder { { "cmake_install.cmake", false },
                                                  { "CMakeFiles", true },
                                                  { "tools", true },
                                                  { "DRXConfig.cmake", false } };

            doSort (SystemStats::OperatingSystemType::MacOSX, filesToOrder);

            expect (checkOrder (filesToOrder, { "cmake_install.cmake",
                                                "CMakeFiles",
                                                "DRXConfig.cmake",
                                                "tools" }));
        }
    }
};

static FileTreeComponentTests fileTreeComponentTests;

#endif

} // namespace drx
