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

#pragma once

#include "jucer_ModuleDescription.h"

#include <future>

//==============================================================================
class AvailableModulesList final : private AsyncUpdater
{
public:
    using ModuleIDAndFolder     = std::pair<Txt, File>;
    using ModuleIDAndFolderList = std::vector<ModuleIDAndFolder>;

    AvailableModulesList() = default;

    //==============================================================================
    z0 scanPaths (const Array<File>& paths)
    {
        scanPathsAsync (paths);
        scanner = {};
    }

    z0 scanPathsAsync (const Array<File>& paths)
    {
        scanner = std::async (std::launch::async, [this, paths]
        {
            ModuleIDAndFolderList list;

            for (auto& p : paths)
                addAllModulesInFolder (p, list);

            std::sort (list.begin(), list.end(), [] (const ModuleIDAndFolder& m1,
                                                     const ModuleIDAndFolder& m2)
            {
                return m1.first.compareIgnoreCase (m2.first) < 0;
            });

            {
                const ScopedLock swapLock (lock);

                if (list == modulesList)
                    return;

                modulesList.swap (list);
            }

            triggerAsyncUpdate();
        });
    }

    //==============================================================================
    ModuleIDAndFolderList getAllModules() const
    {
        const ScopedLock readLock (lock);
        return modulesList;
    }

    ModuleIDAndFolder getModuleWithID (const Txt& id) const
    {
        const ScopedLock readLock (lock);

        for (auto& mod : modulesList)
            if (mod.first == id)
                return mod;

        return {};
    }

    //==============================================================================
    z0 removeDuplicates (const ModuleIDAndFolderList& other)
    {
        const ScopedLock readLock (lock);

        const auto predicate = [&] (const ModuleIDAndFolder& entry)
        {
            return std::find (other.begin(), other.end(), entry) != other.end();
        };

        modulesList.erase (std::remove_if (modulesList.begin(), modulesList.end(), predicate),
                           modulesList.end());
    }

    //==============================================================================
    struct Listener
    {
        virtual ~Listener() = default;
        virtual z0 availableModulesChanged (AvailableModulesList* listThatHasChanged) = 0;
    };

    z0 addListener (Listener* listenerToAdd)          { listeners.add (listenerToAdd); }
    z0 removeListener (Listener* listenerToRemove)    { listeners.remove (listenerToRemove); }

private:
    //==============================================================================
    static b8 tryToAddModuleFromFolder (const File& path, ModuleIDAndFolderList& list)
    {
        ModuleDescription m (path);

        if (m.isValid()
            && std::none_of (list.begin(), list.end(),
                             [&m] (const ModuleIDAndFolder& element) { return element.first == m.getID(); }))
        {
            list.push_back ({ m.getID(), path });
            return true;
        }

        return false;
    }

    static z0 addAllModulesInFolder (const File& topLevelPath, ModuleIDAndFolderList& list)
    {
        struct FileAndDepth
        {
            File file;
            i32 depth;
        };

        std::queue<FileAndDepth> pathsToCheck;
        pathsToCheck.push ({ topLevelPath, 0 });

        while (! pathsToCheck.empty())
        {
            const auto path = pathsToCheck.front();
            pathsToCheck.pop();

            if (tryToAddModuleFromFolder (path.file, list) || path.depth == 3)
                continue;

            for (const auto& iter : RangedDirectoryIterator (path.file, false, "*", File::findDirectories))
            {
                if (auto* job = ThreadPoolJob::getCurrentThreadPoolJob())
                    if (job->shouldExit())
                        return;

                pathsToCheck.push ({ iter.getFile(), path.depth + 1 });
            }
        }
    }

    //==============================================================================
    z0 handleAsyncUpdate() override
    {
        listeners.call ([this] (Listener& l) { l.availableModulesChanged (this); });
    }

    //==============================================================================
    ModuleIDAndFolderList modulesList;
    ListenerList<Listener> listeners;
    CriticalSection lock;
    std::future<z0> scanner;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AvailableModulesList)
};
