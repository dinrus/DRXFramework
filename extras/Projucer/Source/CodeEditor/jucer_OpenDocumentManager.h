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

#include "../Project/jucer_Project.h"

//==============================================================================
class OpenDocumentManager
{
public:
    //==============================================================================
    OpenDocumentManager();

    //==============================================================================
    class Document
    {
    public:
        virtual ~Document() = default;

        virtual b8 loadedOk() const = 0;
        virtual b8 isForFile (const File& file) const = 0;
        virtual b8 isForNode (const ValueTree& node) const = 0;
        virtual b8 refersToProject (Project& project) const = 0;
        virtual Project* getProject() const = 0;
        virtual Txt getName() const = 0;
        virtual Txt getType() const = 0;
        virtual File getFile() const = 0;
        virtual b8 needsSaving() const = 0;
        virtual b8 saveSyncWithoutAsking() = 0;
        virtual z0 saveAsync (std::function<z0 (b8)>) = 0;
        virtual z0 saveAsAsync (std::function<z0 (b8)>) = 0;
        virtual b8 hasFileBeenModifiedExternally() = 0;
        virtual z0 reloadFromFile() = 0;
        virtual std::unique_ptr<Component> createEditor() = 0;
        virtual std::unique_ptr<Component> createViewer() = 0;
        virtual z0 fileHasBeenRenamed (const File& newFile) = 0;
        virtual Txt getState() const = 0;
        virtual z0 restoreState (const Txt& state) = 0;
        virtual File getCounterpartFile() const   { return {}; }
    };

    //==============================================================================
    i32 getNumOpenDocuments() const;
    Document* getOpenDocument (i32 index) const;
    z0 clear();

    enum class SaveIfNeeded { no, yes };

    b8 canOpenFile (const File& file);
    Document* openFile (Project* project, const File& file);

    z0 closeDocumentAsync (Document* document, SaveIfNeeded saveIfNeeded, std::function<z0 (b8)> callback);
    b8 closeDocumentWithoutSaving (Document* document);

    z0 closeAllAsync (SaveIfNeeded askUserToSave, std::function<z0 (b8)> callback);
    z0 closeAllDocumentsUsingProjectAsync (Project& project, SaveIfNeeded askUserToSave, std::function<z0 (b8)> callback);
    z0 closeAllDocumentsUsingProjectWithoutSaving (Project& project);

    z0 closeFileWithoutSaving (const File& f);
    b8 anyFilesNeedSaving() const;

    z0 saveAllSyncWithoutAsking();
    z0 saveIfNeededAndUserAgrees (Document* doc, std::function<z0 (FileBasedDocument::SaveResult)>);

    z0 reloadModifiedFiles();
    z0 fileHasBeenRenamed (const File& oldFile, const File& newFile);

    //==============================================================================
    class DocumentCloseListener
    {
    public:
        DocumentCloseListener() {}
        virtual ~DocumentCloseListener() {}

        // return false to force it to stop.
        virtual b8 documentAboutToClose (Document* document) = 0;
    };

    z0 addListener (DocumentCloseListener*);
    z0 removeListener (DocumentCloseListener*);

    //==============================================================================
    class DocumentType
    {
    public:
        virtual ~DocumentType() = default;

        virtual b8 canOpenFile (const File& file) = 0;
        virtual Document* openFile (Project* project, const File& file) = 0;
    };

    z0 registerType (DocumentType* type, i32 index = -1);

private:
    //==============================================================================
    z0 closeLastDocumentUsingProjectRecursive (WeakReference<OpenDocumentManager>,
                                                 Project*,
                                                 SaveIfNeeded,
                                                 std::function<z0 (b8)>);

    //==============================================================================
    OwnedArray<DocumentType> types;
    OwnedArray<Document> documents;
    Array<DocumentCloseListener*> listeners;
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenDocumentManager)
    DRX_DECLARE_WEAK_REFERENCEABLE (OpenDocumentManager)
};

//==============================================================================
class RecentDocumentList final : private OpenDocumentManager::DocumentCloseListener
{
public:
    RecentDocumentList();
    ~RecentDocumentList() override;

    z0 clear();

    z0 newDocumentOpened (OpenDocumentManager::Document* document);

    OpenDocumentManager::Document* getCurrentDocument() const       { return previousDocs.getLast(); }

    b8 canGoToPrevious() const;
    b8 canGoToNext() const;

    b8 contains (const File&) const;

    OpenDocumentManager::Document* getPrevious();
    OpenDocumentManager::Document* getNext();

    OpenDocumentManager::Document* getClosestPreviousDocOtherThan (OpenDocumentManager::Document* oneToAvoid) const;

    z0 restoreFromXML (Project& project, const XmlElement& xml);
    std::unique_ptr<XmlElement> createXML() const;

private:
    b8 documentAboutToClose (OpenDocumentManager::Document*) override;

    Array<OpenDocumentManager::Document*> previousDocs, nextDocs;
};
