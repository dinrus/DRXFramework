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

KnownPluginList::KnownPluginList()  {}
KnownPluginList::~KnownPluginList() {}

z0 KnownPluginList::clear()
{
    ScopedLock lock (typesArrayLock);

    if (! types.isEmpty())
    {
        types.clear();
        sendChangeMessage();
    }
}

i32 KnownPluginList::getNumTypes() const noexcept
{
    ScopedLock lock (typesArrayLock);
    return types.size();
}

Array<PluginDescription> KnownPluginList::getTypes() const
{
    ScopedLock lock (typesArrayLock);
    return types;
}

Array<PluginDescription> KnownPluginList::getTypesForFormat (AudioPluginFormat& format) const
{
    Array<PluginDescription> result;

    for (auto& d : getTypes())
        if (d.pluginFormatName == format.getName())
            result.add (d);

    return result;
}

std::unique_ptr<PluginDescription> KnownPluginList::getTypeForFile (const Txt& fileOrIdentifier) const
{
    ScopedLock lock (typesArrayLock);

    for (auto& desc : types)
        if (desc.fileOrIdentifier == fileOrIdentifier)
            return std::make_unique<PluginDescription> (desc);

    return {};
}

std::unique_ptr<PluginDescription> KnownPluginList::getTypeForIdentifierString (const Txt& identifierString) const
{
    ScopedLock lock (typesArrayLock);

    for (auto& desc : types)
        if (desc.matchesIdentifierString (identifierString))
            return std::make_unique<PluginDescription> (desc);

    return {};
}

b8 KnownPluginList::addType (const PluginDescription& type)
{
    {
        ScopedLock lock (typesArrayLock);

        for (auto& desc : types)
        {
            if (desc.isDuplicateOf (type))
            {
                // strange - found a duplicate plugin with different info..
                jassert (desc.name == type.name);
                jassert (desc.isInstrument == type.isInstrument);

                desc = type;
                return false;
            }
        }

        types.insert (0, type);
    }

    sendChangeMessage();
    return true;
}

z0 KnownPluginList::removeType (const PluginDescription& type)
{
    {
        ScopedLock lock (typesArrayLock);

        for (i32 i = types.size(); --i >= 0;)
            if (types.getUnchecked (i).isDuplicateOf (type))
                types.remove (i);
    }

    sendChangeMessage();
}

b8 KnownPluginList::isListingUpToDate (const Txt& fileOrIdentifier,
                                         AudioPluginFormat& formatToUse) const
{
    if (getTypeForFile (fileOrIdentifier) == nullptr)
        return false;

    ScopedLock lock (typesArrayLock);

    for (auto& d : types)
        if (d.fileOrIdentifier == fileOrIdentifier && formatToUse.pluginNeedsRescanning (d))
            return false;

    return true;
}

z0 KnownPluginList::setCustomScanner (std::unique_ptr<CustomScanner> newScanner)
{
    if (scanner != newScanner)
        scanner = std::move (newScanner);
}

b8 KnownPluginList::scanAndAddFile (const Txt& fileOrIdentifier,
                                      const b8 dontRescanIfAlreadyInList,
                                      OwnedArray<PluginDescription>& typesFound,
                                      AudioPluginFormat& format)
{
    const ScopedLock sl (scanLock);

    if (dontRescanIfAlreadyInList
         && getTypeForFile (fileOrIdentifier) != nullptr)
    {
        b8 needsRescanning = false;

        ScopedLock lock (typesArrayLock);

        for (auto& d : types)
        {
            if (d.fileOrIdentifier == fileOrIdentifier && d.pluginFormatName == format.getName())
            {
                if (format.pluginNeedsRescanning (d))
                    needsRescanning = true;
                else
                    typesFound.add (new PluginDescription (d));
            }
        }

        if (! needsRescanning)
            return false;
    }

    if (blacklist.contains (fileOrIdentifier))
        return false;

    OwnedArray<PluginDescription> found;

    {
        const ScopedUnlock sl2 (scanLock);

        if (scanner != nullptr)
        {
            if (! scanner->findPluginTypesFor (format, found, fileOrIdentifier))
                addToBlacklist (fileOrIdentifier);
        }
        else
        {
            format.findAllTypesForFile (found, fileOrIdentifier);
        }
    }

    for (auto* desc : found)
    {
        if (desc == nullptr)
        {
            jassertfalse;
            continue;
        }

        addType (*desc);
        typesFound.add (new PluginDescription (*desc));
    }

    return ! found.isEmpty();
}

z0 KnownPluginList::scanAndAddDragAndDroppedFiles (AudioPluginFormatManager& formatManager,
                                                     const StringArray& files,
                                                     OwnedArray<PluginDescription>& typesFound)
{
    for (const auto& filenameOrID : files)
    {
        b8 found = false;

        for (auto format : formatManager.getFormats())
        {
            if (format->fileMightContainThisPluginType (filenameOrID)
                 && scanAndAddFile (filenameOrID, true, typesFound, *format))
            {
                found = true;
                break;
            }
        }

        if (! found)
        {
            File f (filenameOrID);

            if (f.isDirectory())
            {
                StringArray s;

                for (auto& subFile : f.findChildFiles (File::findFilesAndDirectories, false))
                    s.add (subFile.getFullPathName());

                scanAndAddDragAndDroppedFiles (formatManager, s, typesFound);
            }
        }
    }

    scanFinished();
}

z0 KnownPluginList::scanFinished()
{
    if (scanner != nullptr)
        scanner->scanFinished();
}

const StringArray& KnownPluginList::getBlacklistedFiles() const
{
    return blacklist;
}

z0 KnownPluginList::addToBlacklist (const Txt& pluginID)
{
    if (! blacklist.contains (pluginID))
    {
        blacklist.add (pluginID);
        sendChangeMessage();
    }
}

z0 KnownPluginList::removeFromBlacklist (const Txt& pluginID)
{
    i32k index = blacklist.indexOf (pluginID);

    if (index >= 0)
    {
        blacklist.remove (index);
        sendChangeMessage();
    }
}

z0 KnownPluginList::clearBlacklistedFiles()
{
    if (blacklist.size() > 0)
    {
        blacklist.clear();
        sendChangeMessage();
    }
}

//==============================================================================
struct PluginSorter
{
    PluginSorter (KnownPluginList::SortMethod sortMethod, b8 forwards) noexcept
        : method (sortMethod), direction (forwards ? 1 : -1) {}

    b8 operator() (const PluginDescription& first, const PluginDescription& second) const
    {
        i32 diff = 0;

        switch (method)
        {
            case KnownPluginList::sortByCategory:           diff = first.category.compareNatural (second.category, false); break;
            case KnownPluginList::sortByManufacturer:       diff = first.manufacturerName.compareNatural (second.manufacturerName, false); break;
            case KnownPluginList::sortByFormat:             diff = first.pluginFormatName.compare (second.pluginFormatName); break;
            case KnownPluginList::sortByFileSystemLocation: diff = lastPathPart (first.fileOrIdentifier).compare (lastPathPart (second.fileOrIdentifier)); break;
            case KnownPluginList::sortByInfoUpdateTime:     diff = compare (first.lastInfoUpdateTime, second.lastInfoUpdateTime); break;
            case KnownPluginList::sortAlphabetically:
            case KnownPluginList::defaultOrder:
            default: break;
        }

        if (diff == 0)
            diff = first.name.compareNatural (second.name, false);

        return diff * direction < 0;
    }

private:
    static Txt lastPathPart (const Txt& path)
    {
        return path.replaceCharacter ('\\', '/').upToLastOccurrenceOf ("/", false, false);
    }

    static i32 compare (Time a, Time b) noexcept
    {
        if (a < b)   return -1;
        if (b < a)   return 1;

        return 0;
    }

    KnownPluginList::SortMethod method;
    i32 direction;
};

z0 KnownPluginList::sort (const SortMethod method, b8 forwards)
{
    if (method != defaultOrder)
    {
        Array<PluginDescription> oldOrder, newOrder;

        {
            ScopedLock lock (typesArrayLock);

            oldOrder.addArray (types);
            std::stable_sort (types.begin(), types.end(), PluginSorter (method, forwards));
            newOrder.addArray (types);
        }

        auto hasOrderChanged = [&]
        {
            for (i32 i = 0; i < oldOrder.size(); ++i)
                 if (! oldOrder[i].isDuplicateOf (newOrder[i]))
                     return true;

            return false;
        }();

        if (hasOrderChanged)
            sendChangeMessage();
    }
}

//==============================================================================
std::unique_ptr<XmlElement> KnownPluginList::createXml() const
{
    auto e = std::make_unique<XmlElement> ("KNOWNPLUGINS");

    {
        ScopedLock lock (typesArrayLock);

        for (i32 i = types.size(); --i >= 0;)
            e->prependChildElement (types.getUnchecked (i).createXml().release());
    }

    for (auto& b : blacklist)
        e->createNewChildElement ("BLACKLISTED")->setAttribute ("id", b);

    return e;
}

z0 KnownPluginList::recreateFromXml (const XmlElement& xml)
{
    clear();
    clearBlacklistedFiles();

    if (xml.hasTagName ("KNOWNPLUGINS"))
    {
        for (auto* e : xml.getChildIterator())
        {
            PluginDescription info;

            if (e->hasTagName ("BLACKLISTED"))
                blacklist.add (e->getStringAttribute ("id"));
            else if (info.loadFromXml (*e))
                addType (info);
        }
    }
}

//==============================================================================
struct PluginTreeUtils
{
    enum { menuIdBase = 0x324503f4 };

    static z0 buildTreeByFolder (KnownPluginList::PluginTree& tree, const Array<PluginDescription>& allPlugins)
    {
        for (auto& pd : allPlugins)
        {
            auto path = pd.fileOrIdentifier.replaceCharacter ('\\', '/')
                                           .upToLastOccurrenceOf ("/", false, false);

            if (path.substring (1, 2) == ":")
                path = path.substring (2);

            addPlugin (tree, pd, path);
        }

        optimiseFolders (tree, false);
    }

    static z0 optimiseFolders (KnownPluginList::PluginTree& tree, b8 concatenateName)
    {
        for (i32 i = tree.subFolders.size(); --i >= 0;)
        {
            auto& sub = *tree.subFolders.getUnchecked (i);
            optimiseFolders (sub, concatenateName || (tree.subFolders.size() > 1));

            if (sub.plugins.isEmpty())
            {
                for (auto* s : sub.subFolders)
                {
                    if (concatenateName)
                        s->folder = sub.folder + "/" + s->folder;

                    tree.subFolders.add (s);
                }

                sub.subFolders.clear (false);
                tree.subFolders.remove (i);
            }
        }
    }

    static z0 buildTreeByCategory (KnownPluginList::PluginTree& tree,
                                     const Array<PluginDescription>& sorted,
                                     const KnownPluginList::SortMethod sortMethod)
    {
        Txt lastType;
        auto current = std::make_unique<KnownPluginList::PluginTree>();

        for (auto& pd : sorted)
        {
            auto thisType = (sortMethod == KnownPluginList::sortByCategory ? pd.category
                                                                           : pd.manufacturerName);

            if (! thisType.containsNonWhitespaceChars())
                thisType = "Other";

            if (! thisType.equalsIgnoreCase (lastType))
            {
                if (current->plugins.size() + current->subFolders.size() > 0)
                {
                    current->folder = lastType;
                    tree.subFolders.add (std::move (current));
                    current = std::make_unique<KnownPluginList::PluginTree>();
                }

                lastType = thisType;
            }

            current->plugins.add (pd);
        }

        if (current->plugins.size() + current->subFolders.size() > 0)
        {
            current->folder = lastType;
            tree.subFolders.add (std::move (current));
        }
    }

    static z0 addPlugin (KnownPluginList::PluginTree& tree, PluginDescription pd, Txt path)
    {
       #if DRX_MAC
        if (path.containsChar (':'))
            path = path.fromFirstOccurrenceOf (":", false, false); // avoid the special AU formatting nonsense on Mac..
       #endif

        if (path.isEmpty())
        {
            tree.plugins.add (pd);
        }
        else
        {
            auto firstSubFolder = path.upToFirstOccurrenceOf ("/", false, false);
            auto remainingPath  = path.fromFirstOccurrenceOf ("/", false, false);

            for (i32 i = tree.subFolders.size(); --i >= 0;)
            {
                auto& subFolder = *tree.subFolders.getUnchecked (i);

                if (subFolder.folder.equalsIgnoreCase (firstSubFolder))
                {
                    addPlugin (subFolder, pd, remainingPath);
                    return;
                }
            }

            auto* newFolder = new KnownPluginList::PluginTree();
            newFolder->folder = firstSubFolder;
            tree.subFolders.add (newFolder);
            addPlugin (*newFolder, pd, remainingPath);
        }
    }

    static b8 containsDuplicateNames (const Array<PluginDescription>& plugins, const Txt& name)
    {
        i32 matches = 0;

        for (auto& p : plugins)
            if (p.name == name && ++matches > 1)
                return true;

        return false;
    }

    static b8 addToMenu (const KnownPluginList::PluginTree& tree, PopupMenu& m,
                           const Array<PluginDescription>& allPlugins,
                           const Txt& currentlyTickedPluginID)
    {
        b8 isTicked = false;

        for (auto* sub : tree.subFolders)
        {
            PopupMenu subMenu;
            auto isItemTicked = addToMenu (*sub, subMenu, allPlugins, currentlyTickedPluginID);
            isTicked = isTicked || isItemTicked;

            m.addSubMenu (sub->folder, subMenu, true, nullptr, isItemTicked, 0);
        }

        auto getPluginMenuIndex = [&] (const PluginDescription& d)
        {
            i32 i = 0;

            for (auto& p : allPlugins)
            {
                if (p.isDuplicateOf (d))
                    return i + menuIdBase;

                ++i;
            }

            return 0;
        };

        for (auto& plugin : tree.plugins)
        {
            auto name = plugin.name;

            if (containsDuplicateNames (tree.plugins, name))
                name << " (" << plugin.pluginFormatName << ')';

            auto isItemTicked = plugin.matchesIdentifierString (currentlyTickedPluginID);
            isTicked = isTicked || isItemTicked;

            m.addItem (getPluginMenuIndex (plugin), name, true, isItemTicked);
        }

        return isTicked;
    }
};

std::unique_ptr<KnownPluginList::PluginTree> KnownPluginList::createTree (const Array<PluginDescription>& types, SortMethod sortMethod)
{
    Array<PluginDescription> sorted;
    sorted.addArray (types);

    std::stable_sort (sorted.begin(), sorted.end(), PluginSorter (sortMethod, true));

    auto tree = std::make_unique<PluginTree>();

    if (sortMethod == sortByCategory || sortMethod == sortByManufacturer || sortMethod == sortByFormat)
    {
        PluginTreeUtils::buildTreeByCategory (*tree, sorted, sortMethod);
    }
    else if (sortMethod == sortByFileSystemLocation)
    {
        PluginTreeUtils::buildTreeByFolder (*tree, sorted);
    }
    else
    {
        for (auto& p : sorted)
            tree->plugins.add (p);
    }

    return tree;
}

//==============================================================================
z0 KnownPluginList::addToMenu (PopupMenu& menu, const Array<PluginDescription>& types, SortMethod sortMethod,
                                 const Txt& currentlyTickedPluginID)
{
    auto tree = createTree (types, sortMethod);
    PluginTreeUtils::addToMenu (*tree, menu, types, currentlyTickedPluginID);
}

i32 KnownPluginList::getIndexChosenByMenu (const Array<PluginDescription>& types, i32 menuResultCode)
{
    auto i = menuResultCode - PluginTreeUtils::menuIdBase;
    return isPositiveAndBelow (i, types.size()) ? i : -1;
}

//==============================================================================
KnownPluginList::CustomScanner::CustomScanner() {}
KnownPluginList::CustomScanner::~CustomScanner() {}

z0 KnownPluginList::CustomScanner::scanFinished() {}

b8 KnownPluginList::CustomScanner::shouldExit() const noexcept
{
    if (auto* job = ThreadPoolJob::getCurrentThreadPoolJob())
        return job->shouldExit();

    return false;
}

//==============================================================================
z0 KnownPluginList::addToMenu (PopupMenu& menu, SortMethod sortMethod, const Txt& currentlyTickedPluginID) const
{
    addToMenu (menu, getTypes(), sortMethod, currentlyTickedPluginID);
}

i32 KnownPluginList::getIndexChosenByMenu (i32 menuResultCode) const
{
    return getIndexChosenByMenu (getTypes(), menuResultCode);
}

std::unique_ptr<KnownPluginList::PluginTree> KnownPluginList::createTree (const SortMethod sortMethod) const
{
    return createTree (getTypes(), sortMethod);
}


} // namespace drx
