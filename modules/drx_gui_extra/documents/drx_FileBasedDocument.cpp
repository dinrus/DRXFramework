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
class FileBasedDocument::Pimpl
{
private:
    //==============================================================================
    class SafeParentPointer
    {
    public:
        SafeParentPointer (Pimpl* parent, b8 isAsync)
            : ptr (parent), shouldCheck (isAsync)
        {}

        Pimpl* operator->() const noexcept
        {
            return ptr.get();
        }

        b8 operator== (Pimpl* object) const noexcept   { return ptr.get() == object; }
        b8 operator!= (Pimpl* object) const noexcept   { return ptr.get() != object; }

        b8 shouldExitAsyncCallback() const noexcept
        {
            return shouldCheck && ptr == nullptr;
        }

    private:
        WeakReference<Pimpl> ptr;
        b8 shouldCheck = false;
    };

public:
    //==============================================================================
    Pimpl (FileBasedDocument& parent_,
           const Txt& fileExtension_,
           const Txt& fileWildcard_,
           const Txt& openFileDialogTitle_,
           const Txt& saveFileDialogTitle_)
        : document (parent_),
          fileExtension (fileExtension_),
          fileWildcard (fileWildcard_),
          openFileDialogTitle (openFileDialogTitle_),
          saveFileDialogTitle (saveFileDialogTitle_)
    {
    }

    //==============================================================================
    b8 hasChangedSinceSaved() const
    {
        return changedSinceSave;
    }

    z0 setChangedFlag (b8 hasChanged)
    {
        if (changedSinceSave != hasChanged)
        {
            changedSinceSave = hasChanged;
            document.sendChangeMessage();
        }
    }

    z0 changed()
    {
        changedSinceSave = true;
        document.sendChangeMessage();
    }

    //==============================================================================
    Result loadFrom (const File& newFile, b8 showMessageOnFailure, b8 showWaitCursor = true)
    {
        SafeParentPointer parent { this, false };
        auto result = Result::ok();
        loadFromImpl (parent,
                      newFile,
                      showMessageOnFailure,
                      showWaitCursor,
                      [this] (const File& file, const auto& callback) { callback (document.loadDocument (file)); },
                      [&result] (Result r) { result = r; });
        return result;
    }

    z0 loadFromAsync (const File& newFile,
                        b8 showMessageOnFailure,
                        std::function<z0 (Result)> callback)
    {
        SafeParentPointer parent { this, true };
        loadFromImpl (parent,
                      newFile,
                      showMessageOnFailure,
                      false,
                      [parent] (const File& file, auto cb)
                      {
                          if (parent != nullptr)
                              parent->document.loadDocumentAsync (file, std::move (cb));
                      },
                      std::move (callback));
    }

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED
    Result loadFromUserSpecifiedFile (b8 showMessageOnFailure)
    {
        FileChooser fc (openFileDialogTitle,
                        document.getLastDocumentOpened(),
                        fileWildcard);

        if (fc.browseForFileToOpen())
            return loadFrom (fc.getResult(), showMessageOnFailure);

        return Result::fail (TRANS ("User cancelled"));
    }
   #endif

    z0 loadFromUserSpecifiedFileAsync (const b8 showMessageOnFailure, std::function<z0 (Result)> callback)
    {
        asyncFc = std::make_unique<FileChooser> (openFileDialogTitle,
                                                 document.getLastDocumentOpened(),
                                                 fileWildcard);

        asyncFc->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                              [this, showMessageOnFailure, cb = std::move (callback)] (const FileChooser& fc)
        {
            auto chosenFile = fc.getResult();

            if (chosenFile == File{})
            {
                NullCheckedInvocation::invoke (cb, Result::fail (TRANS ("User cancelled")));
                return;
            }

            WeakReference<Pimpl> parent { this };
            loadFromAsync (chosenFile, showMessageOnFailure, [parent, cb] (Result result)
            {
                if (parent != nullptr)
                    NullCheckedInvocation::invoke (cb, result);
            });

            asyncFc = nullptr;
        });
    }

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED
    FileBasedDocument::SaveResult save (b8 askUserForFileIfNotSpecified,
                                        b8 showMessageOnFailure)
    {
        return saveAs (documentFile,
                       false,
                       askUserForFileIfNotSpecified,
                       showMessageOnFailure);
    }
   #endif

    z0 saveAsync (b8 askUserForFileIfNotSpecified,
                    b8 showMessageOnFailure,
                    std::function<z0 (SaveResult)> callback)
    {
        saveAsAsync (documentFile,
                     false,
                     askUserForFileIfNotSpecified,
                     showMessageOnFailure,
                     std::move (callback));
    }

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED
    FileBasedDocument::SaveResult saveIfNeededAndUserAgrees()
    {
        SafeParentPointer parent { this, false };
        SaveResult result;
        saveIfNeededAndUserAgreesImpl (parent,
                                       [&result] (SaveResult r) { result = r; },
                                       AskToSaveChangesSync { *this },
                                       SaveSync { *this });
        return result;
    }
   #endif

    z0 saveIfNeededAndUserAgreesAsync (std::function<z0 (SaveResult)> callback)
    {
        SafeParentPointer parent { this, true };

        saveIfNeededAndUserAgreesImpl (parent,
                                       std::move (callback),
                                       [] (SafeParentPointer ptr, auto cb)
                                       {
                                           if (ptr != nullptr)
                                               ptr->askToSaveChangesAsync (ptr, std::move (cb));
                                       },
                                       [parent] (b8 askUserForFileIfNotSpecified,
                                                 b8 showMessageOnFailure,
                                                 auto cb)
                                       {
                                           if (parent != nullptr)
                                               parent->saveAsync (askUserForFileIfNotSpecified,
                                                                  showMessageOnFailure,
                                                                  std::move (cb));
                                       });
    }

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED
    FileBasedDocument::SaveResult saveAs (const File& newFile,
                                          b8 warnAboutOverwritingExistingFiles,
                                          b8 askUserForFileIfNotSpecified,
                                          b8 showMessageOnFailure,
                                          b8 showWaitCursor = true)
    {
        SafeParentPointer parent { this, false };
        SaveResult result{};
        saveAsSyncImpl (parent,
                        newFile,
                        warnAboutOverwritingExistingFiles,
                        askUserForFileIfNotSpecified,
                        showMessageOnFailure,
                        [&result] (SaveResult r) { result = r; },
                        showWaitCursor);
        return result;
    }
    #endif

    z0 saveAsAsync (const File& newFile,
                      b8 warnAboutOverwritingExistingFiles,
                      b8 askUserForFileIfNotSpecified,
                      b8 showMessageOnFailure,
                      std::function<z0 (SaveResult)> callback)
    {
        SafeParentPointer parent { this, true };
        saveAsAsyncImpl (parent,
                         newFile,
                         warnAboutOverwritingExistingFiles,
                         askUserForFileIfNotSpecified,
                         showMessageOnFailure,
                         std::move (callback),
                         false);
    }

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED
    FileBasedDocument::SaveResult saveAsInteractive (b8 warnAboutOverwritingExistingFiles)
    {
        SafeParentPointer parent { this, false };
        SaveResult result{};
        saveAsInteractiveSyncImpl (parent,
                                   warnAboutOverwritingExistingFiles,
                                   [&result] (SaveResult r) { result = r; });
        return result;
    }
   #endif

    z0 saveAsInteractiveAsync (b8 warnAboutOverwritingExistingFiles,
                                 std::function<z0 (SaveResult)> callback)
    {
        SafeParentPointer parent { this, true };
        saveAsInteractiveAsyncImpl (parent,
                                    warnAboutOverwritingExistingFiles,
                                    std::move (callback));
    }

    //==============================================================================
    const File& getFile() const
    {
        return documentFile;
    }

    z0 setFile (const File& newFile)
    {
        if (documentFile != newFile)
        {
            documentFile = newFile;
            changed();
        }
    }

    //==============================================================================
    const Txt& getFileExtension() const
    {
        return fileExtension;
    }

private:
    //==============================================================================
    template <typename DoLoadDocument>
    z0 loadFromImpl (SafeParentPointer parent,
                       const File& newFile,
                       b8 showMessageOnFailure,
                       b8 showWaitCursor,
                       DoLoadDocument&& doLoadDocument,
                       std::function<z0 (Result)> completed)
    {
        if (parent.shouldExitAsyncCallback())
            return;

        if (showWaitCursor)
            MouseCursor::showWaitCursor();

        auto oldFile = documentFile;
        documentFile = newFile;

        auto tidyUp = [parent, newFile, oldFile, showMessageOnFailure, showWaitCursor, completed] (Result result)
        {
            if (parent.shouldExitAsyncCallback())
                return;

            parent->documentFile = oldFile;

            if (showWaitCursor)
                MouseCursor::hideWaitCursor();

            if (showMessageOnFailure)
            {
                auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                                 TRANS ("Failed to open file..."),
                                                                 TRANS ("There was an error while trying to load the file: FLNM")
                                                                         .replace ("FLNM", "\n" + newFile.getFullPathName())
                                                                     + "\n\n"
                                                                     + result.getErrorMessage());
                parent->messageBox = AlertWindow::showScopedAsync (options, nullptr);
            }

            NullCheckedInvocation::invoke (completed, result);
        };

        if (newFile.existsAsFile())
        {
            auto afterLoading = [parent,
                                 showWaitCursor,
                                 newFile,
                                 cb = std::move (completed),
                                 tidyUp] (Result result)
            {
                if (result.wasOk())
                {
                    parent->setChangedFlag (false);

                    if (showWaitCursor)
                        MouseCursor::hideWaitCursor();

                    parent->document.setLastDocumentOpened (newFile);
                    NullCheckedInvocation::invoke (cb, result);
                    return;
                }

                tidyUp (result);
            };

            doLoadDocument (newFile, std::move (afterLoading));

            return;
        }

        tidyUp (Result::fail (TRANS ("The file doesn't exist")));
    }

    //==============================================================================
    template <typename DoAskToSaveChanges, typename DoSave>
    z0 saveIfNeededAndUserAgreesImpl (SafeParentPointer parent,
                                        std::function<z0 (SaveResult)> completed,
                                        DoAskToSaveChanges&& doAskToSaveChanges,
                                        DoSave&& doSave)
    {
        if (parent.shouldExitAsyncCallback())
            return;

        if (! hasChangedSinceSaved())
        {
            NullCheckedInvocation::invoke (completed, savedOk);
            return;
        }

        auto afterAsking = [save = std::forward<DoSave> (doSave),
                            cb = std::move (completed)] (SafeParentPointer ptr,
                                                         i32 alertResult)
        {
            if (ptr.shouldExitAsyncCallback())
                return;

            switch (alertResult)
            {
                case 1:  // save changes
                    save (true, true, [ptr, cb] (SaveResult result)
                    {
                        if (ptr.shouldExitAsyncCallback())
                            return;

                        NullCheckedInvocation::invoke (cb, result);
                    });
                    return;

                case 2:  // discard changes
                    NullCheckedInvocation::invoke (cb, savedOk);
                    return;
            }

            NullCheckedInvocation::invoke (cb, userCancelledSave);
        };

        doAskToSaveChanges (parent, std::move (afterAsking));
    }

    //==============================================================================
    MessageBoxOptions getAskToSaveChangesOptions() const
    {
        return MessageBoxOptions::makeOptionsYesNoCancel (MessageBoxIconType::QuestionIcon,
                                                          TRANS ("Closing document..."),
                                                          TRANS ("Do you want to save the changes to \"DCNM\"?")
                                                                  .replace ("DCNM", document.getDocumentTitle()),
                                                          TRANS ("Save"),
                                                          TRANS ("Discard changes"),
                                                          TRANS ("Cancel"));
    }

    z0 askToSaveChangesAsync (SafeParentPointer parent,
                                std::function<z0 (SafeParentPointer, i32)> callback)
    {
        messageBox = AlertWindow::showScopedAsync (getAskToSaveChangesOptions(),
                                                   [parent, cb = std::move (callback)] (i32 alertResult)
                                                   {
                                                       if (parent != nullptr)
                                                           cb (parent, alertResult);
                                                   });
    }

   #if DRX_MODAL_LOOPS_PERMITTED
    i32 askToSaveChangesSync()
    {
        return AlertWindow::show (getAskToSaveChangesOptions());
    }
   #endif

    //==============================================================================
    template <typename DoSaveDocument>
    z0 saveInternal (SafeParentPointer parent,
                       const File& newFile,
                       b8 showMessageOnFailure,
                       b8 showWaitCursor,
                       std::function<z0 (SaveResult)> afterSave,
                       DoSaveDocument&& doSaveDocument)
    {
        if (showWaitCursor)
            MouseCursor::showWaitCursor();

        auto oldFile = documentFile;
        documentFile = newFile;

        doSaveDocument (newFile, [parent,
                                  showMessageOnFailure,
                                  showWaitCursor,
                                  oldFile,
                                  newFile,
                                  after = std::move (afterSave)] (Result result)
        {
            if (parent.shouldExitAsyncCallback())
            {
                if (showWaitCursor)
                    MouseCursor::hideWaitCursor();

                return;
            }

            if (result.wasOk())
            {
                parent->setChangedFlag (false);

                if (showWaitCursor)
                    MouseCursor::hideWaitCursor();

                parent->document.sendChangeMessage(); // because the filename may have changed

                NullCheckedInvocation::invoke (after, savedOk);
                return;
            }

            parent->documentFile = oldFile;

            if (showWaitCursor)
                MouseCursor::hideWaitCursor();

            if (showMessageOnFailure)
            {
                auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                                 TRANS ("Error writing to file..."),
                                                                 TRANS ("An error occurred while trying to save \"DCNM\" to the file: FLNM")
                                                                         .replace ("DCNM", parent->document.getDocumentTitle())
                                                                         .replace ("FLNM", "\n" + newFile.getFullPathName())
                                                                     + "\n\n"
                                                                     + result.getErrorMessage());
                parent->messageBox = AlertWindow::showScopedAsync (options, nullptr);
            }

            parent->document.sendChangeMessage(); // because the filename may have changed
            NullCheckedInvocation::invoke (after, failedToWriteToFile);
        });
    }

    template <typename DoSaveAsInteractive, typename DoAskToOverwriteFile, typename DoSaveDocument>
    z0 saveAsImpl (SafeParentPointer parent,
                     const File& newFile,
                     b8 warnAboutOverwritingExistingFiles,
                     b8 askUserForFileIfNotSpecified,
                     b8 showMessageOnFailure,
                     std::function<z0 (SaveResult)> callback,
                     b8 showWaitCursor,
                     DoSaveAsInteractive&& doSaveAsInteractive,
                     DoAskToOverwriteFile&& doAskToOverwriteFile,
                     DoSaveDocument&& doSaveDocument)
    {
        if (parent.shouldExitAsyncCallback())
            return;

        if (newFile == File())
        {
            if (askUserForFileIfNotSpecified)
            {
                doSaveAsInteractive (parent, true, std::move (callback));
                return;
            }

            // can't save to an unspecified file
            jassertfalse;

            NullCheckedInvocation::invoke (callback, failedToWriteToFile);
            return;
        }

        auto saveInternalHelper = [parent,
                                   callback,
                                   newFile,
                                   showMessageOnFailure,
                                   showWaitCursor,
                                   saveDocument = std::forward<DoSaveDocument> (doSaveDocument)]
        {
            if (! parent.shouldExitAsyncCallback())
                parent->saveInternal (parent,
                                      newFile,
                                      showMessageOnFailure,
                                      showWaitCursor,
                                      callback,
                                      saveDocument);
        };

        if (warnAboutOverwritingExistingFiles && newFile.exists())
        {
            auto afterAsking = [cb = std::move (callback),
                                saveInternalHelper] (SafeParentPointer ptr,
                                                     b8 shouldOverwrite)
            {
                if (ptr.shouldExitAsyncCallback())
                    return;

                if (shouldOverwrite)
                    saveInternalHelper();
                else
                    NullCheckedInvocation::invoke (cb, userCancelledSave);
            };
            doAskToOverwriteFile (parent, newFile, std::move (afterAsking));
            return;
        }

        saveInternalHelper();
    }

    z0 saveAsAsyncImpl (SafeParentPointer parent,
                          const File& newFile,
                          b8 warnAboutOverwritingExistingFiles,
                          b8 askUserForFileIfNotSpecified,
                          b8 showMessageOnFailure,
                          std::function<z0 (SaveResult)> callback,
                          b8 showWaitCursor)
    {
        saveAsImpl (parent,
                    newFile,
                    warnAboutOverwritingExistingFiles,
                    askUserForFileIfNotSpecified,
                    showMessageOnFailure,
                    std::move (callback),
                    showWaitCursor,
                    [] (SafeParentPointer ptr, b8 warnAboutOverwriting, auto cb)
                    {
                        if (ptr != nullptr)
                            ptr->saveAsInteractiveAsyncImpl (ptr, warnAboutOverwriting, std::move (cb));
                    },
                    [] (SafeParentPointer ptr, const File& destination, std::function<z0 (SafeParentPointer, b8)> cb)
                    {
                        if (ptr != nullptr)
                            ptr->askToOverwriteFileAsync (ptr, destination, std::move (cb));
                    },
                    [parent] (const File& destination, std::function<z0 (Result)> cb)
                    {
                        if (parent != nullptr)
                            parent->document.saveDocumentAsync (destination, std::move (cb));
                    });
    }

    //==============================================================================
    z0 saveAsInteractiveAsyncImpl (SafeParentPointer parent,
                                     b8 warnAboutOverwritingExistingFiles,
                                     std::function<z0 (SaveResult)> callback)
    {
        if (parent == nullptr)
            return;

        saveAsInteractiveImpl (parent,
                               warnAboutOverwritingExistingFiles,
                               std::move (callback),
                               [] (SafeParentPointer ptr, b8 warnAboutOverwriting, auto cb)
                               {
                                   if (ptr != nullptr)
                                       ptr->getSaveAsFilenameAsync (ptr, warnAboutOverwriting, std::move (cb));
                               },
                               [] (SafeParentPointer ptr,
                                   const File& newFile,
                                   b8 warnAboutOverwriting,
                                   b8 askUserForFileIfNotSpecified,
                                   b8 showMessageOnFailure,
                                   auto cb,
                                   b8 showWaitCursor)
                               {
                                   if (ptr != nullptr)
                                       ptr->saveAsAsyncImpl (ptr,
                                                             newFile,
                                                             warnAboutOverwriting,
                                                             askUserForFileIfNotSpecified,
                                                             showMessageOnFailure,
                                                             std::move (cb),
                                                             showWaitCursor);
                               },
                               [] (SafeParentPointer ptr, const File& destination, auto cb)
                               {
                                   if (ptr != nullptr)
                                       ptr->askToOverwriteFileAsync (ptr, destination, std::move (cb));
                               });
    }

    //==============================================================================
    MessageBoxOptions getAskToOverwriteFileOptions (const File& newFile) const
    {
        return MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                       TRANS ("File already exists"),
                                                       TRANS ("There's already a file called: FLNM")
                                                               .replace ("FLNM", newFile.getFullPathName())
                                                           + "\n\n"
                                                           + TRANS ("Are you sure you want to overwrite it?"),
                                                       TRANS ("Overwrite"),
                                                       TRANS ("Cancel"));
    }

    z0 askToOverwriteFileAsync (SafeParentPointer parent,
                                  const File& newFile,
                                  std::function<z0 (SafeParentPointer, b8)> callback)
    {
        if (parent == nullptr)
            return;

        messageBox = AlertWindow::showScopedAsync (getAskToOverwriteFileOptions (newFile),
                                                   [parent, cb = std::move (callback)] (i32 r)
                                                   {
                                                       if (parent != nullptr)
                                                           NullCheckedInvocation::invoke (cb, parent, r != 1);
                                                   });
    }

   #if DRX_MODAL_LOOPS_PERMITTED
    b8 askToOverwriteFileSync (const File& newFile)
    {
        return AlertWindow::show (getAskToOverwriteFileOptions (newFile));
    }
   #endif

    //==============================================================================
    z0 getSaveAsFilenameAsync (SafeParentPointer parent,
                                 b8 warnAboutOverwritingExistingFiles,
                                 std::function<z0 (SafeParentPointer, const File&)> callback)
    {
        asyncFc = getInteractiveFileChooser();

        auto flags = FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles;

        if (warnAboutOverwritingExistingFiles)
            flags |= FileBrowserComponent::warnAboutOverwriting;

        asyncFc->launchAsync (flags, [parent, cb = std::move (callback)] (const FileChooser& fc)
        {
            cb (parent, fc.getResult());
        });
    }

    //==============================================================================
    template <typename DoSelectFilename, typename DoSaveAs, typename DoAskToOverwriteFile>
    z0 saveAsInteractiveImpl (SafeParentPointer parent,
                                b8 warnAboutOverwritingExistingFiles,
                                std::function<z0 (SaveResult)> callback,
                                DoSelectFilename&& doSelectFilename,
                                DoSaveAs&& doSaveAs,
                                DoAskToOverwriteFile&& doAskToOverwriteFile)
    {
        doSelectFilename (parent,
                          warnAboutOverwritingExistingFiles,
                          [saveAs = std::forward<DoSaveAs> (doSaveAs),
                           askToOverwriteFile = std::forward<DoAskToOverwriteFile> (doAskToOverwriteFile),
                           cb = std::move (callback)] (SafeParentPointer parentPtr, File chosen)
        {
            if (parentPtr.shouldExitAsyncCallback())
                return;

            if (chosen == File{})
            {
                NullCheckedInvocation::invoke (cb, userCancelledSave);
                return;
            }

            auto updateAndSaveAs = [parentPtr, saveAs, cb] (const File& chosenFile)
            {
                if (parentPtr.shouldExitAsyncCallback())
                    return;

                parentPtr->document.setLastDocumentOpened (chosenFile);
                saveAs (parentPtr, chosenFile, false, false, true, cb, false);
            };

            if (chosen.getFileExtension().isEmpty())
            {
                chosen = chosen.withFileExtension (parentPtr->fileExtension);

                if (chosen.exists())
                {
                    auto afterAsking = [chosen, updateAndSaveAs, cb] (SafeParentPointer overwritePtr,
                                                                            b8 overwrite)
                    {
                        if (overwritePtr.shouldExitAsyncCallback())
                            return;

                        if (overwrite)
                            updateAndSaveAs (chosen);
                        else
                            NullCheckedInvocation::invoke (cb, userCancelledSave);
                    };

                    askToOverwriteFile (parentPtr, chosen, std::move (afterAsking));
                    return;
                }
            }

            updateAndSaveAs (chosen);
        });
    }

    //==============================================================================
    std::unique_ptr<FileChooser> getInteractiveFileChooser()
    {
        auto f = documentFile.existsAsFile() ? documentFile : document.getLastDocumentOpened();

        auto legalFilename = File::createLegalFileName (document.getDocumentTitle());

        if (legalFilename.isEmpty())
            legalFilename = "unnamed";

        f = (f.existsAsFile() || f.getParentDirectory().isDirectory())
            ? f.getSiblingFile (legalFilename)
            : File::getSpecialLocation (File::userDocumentsDirectory).getChildFile (legalFilename);

        f = document.getSuggestedSaveAsFile (f);

        return std::make_unique<FileChooser> (saveFileDialogTitle,
                                              f,
                                              fileWildcard);
    }

    //==============================================================================
   #if DRX_MODAL_LOOPS_PERMITTED
    struct SaveAsInteractiveSyncImpl
    {
        template <typename... Ts>
        z0 operator() (Ts&&... ts) const noexcept
        {
            p.saveAsInteractiveSyncImpl (std::forward<Ts> (ts)...);
        }

        Pimpl& p;
    };

    struct AskToOverwriteFileSync
    {
        template <typename... Ts>
        z0 operator() (Ts&&... ts) const noexcept
        {
            p.askToOverwriteFileSync (std::forward<Ts> (ts)...);
        }

        Pimpl& p;
    };

    struct AskToSaveChangesSync
    {
        template <typename... Ts>
        z0 operator() (Ts&&... ts) const noexcept
        {
            p.askToSaveChangesSync (std::forward<Ts> (ts)...);
        }

        Pimpl& p;
    };

    struct SaveSync
    {
        template <typename... Ts>
        z0 operator() (Ts&&... ts) const noexcept
        {
            p.saveSync (std::forward<Ts> (ts)...);
        }

        Pimpl& p;
    };

    struct GetSaveAsFilenameSync
    {
        template <typename... Ts>
        z0 operator() (Ts&&... ts) const noexcept
        {
            p.getSaveAsFilenameSync (std::forward<Ts> (ts)...);
        }

        Pimpl& p;
    };

    struct SaveAsSyncImpl
    {
        template <typename... Ts>
        z0 operator() (Ts&&... ts) const noexcept
        {
            p.saveAsSyncImpl (std::forward<Ts> (ts)...);
        }

        Pimpl& p;
    };

    //==============================================================================
    z0 saveAsSyncImpl (SafeParentPointer parent,
                         const File& newFile,
                         b8 warnAboutOverwritingExistingFiles,
                         b8 askUserForFileIfNotSpecified,
                         b8 showMessageOnFailure,
                         std::function<z0 (SaveResult)> callback,
                         b8 showWaitCursor)
    {
        saveAsImpl (parent,
                    newFile,
                    warnAboutOverwritingExistingFiles,
                    askUserForFileIfNotSpecified,
                    showMessageOnFailure,
                    std::move (callback),
                    showWaitCursor,
                    SaveAsInteractiveSyncImpl { *this },
                    AskToOverwriteFileSync { *this },
                    [this] (const File& file, const auto& cb) { cb (document.saveDocument (file)); });
    }

    //==============================================================================
    template <typename Callback>
    z0 askToSaveChangesSync (SafeParentPointer parent, Callback&& callback)
    {
        callback (parent, askToSaveChangesSync());
    }

    //==============================================================================
    z0 saveAsInteractiveSyncImpl (SafeParentPointer parent,
                                    b8 warnAboutOverwritingExistingFiles,
                                    std::function<z0 (SaveResult)> callback)
    {
        saveAsInteractiveImpl (parent,
                               warnAboutOverwritingExistingFiles,
                               std::move (callback),
                               GetSaveAsFilenameSync { *this },
                               SaveAsSyncImpl { *this },
                               AskToOverwriteFileSync { *this });
    }

    //==============================================================================
    template <typename Callback>
    z0 askToOverwriteFileSync (SafeParentPointer parent,
                                 const File& newFile,
                                 Callback&& callback)
    {
        callback (parent, askToOverwriteFileSync (newFile));
    }

    //==============================================================================
    template <typename Callback>
    z0 saveSync (b8 askUserForFileIfNotSpecified,
                   b8 showMessageOnFailure,
                   Callback&& callback)
    {
        callback (save (askUserForFileIfNotSpecified, showMessageOnFailure));
    }

    //==============================================================================
    template <typename Callback>
    z0 getSaveAsFilenameSync (SafeParentPointer parent,
                                b8 warnAboutOverwritingExistingFiles,
                                Callback&& callback)
    {
        auto fc = getInteractiveFileChooser();

        if (fc->browseForFileToSave (warnAboutOverwritingExistingFiles))
        {
            callback (parent, fc->getResult());
            return;
        }

        callback (parent, {});
    }
   #endif

    //==============================================================================
    FileBasedDocument& document;

    File documentFile;
    b8 changedSinceSave = false;
    Txt fileExtension, fileWildcard, openFileDialogTitle, saveFileDialogTitle;
    std::unique_ptr<FileChooser> asyncFc;
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
    DRX_DECLARE_WEAK_REFERENCEABLE (Pimpl)
};

//==============================================================================
FileBasedDocument::FileBasedDocument (const Txt& fileExtension,
                                      const Txt& fileWildcard,
                                      const Txt& openFileDialogTitle,
                                      const Txt& saveFileDialogTitle)
    : pimpl (new Pimpl (*this,
                        fileExtension,
                        fileWildcard,
                        openFileDialogTitle,
                        saveFileDialogTitle))
{
}

FileBasedDocument::~FileBasedDocument() = default;

//==============================================================================
b8 FileBasedDocument::hasChangedSinceSaved() const
{
    return pimpl->hasChangedSinceSaved();
}

z0 FileBasedDocument::setChangedFlag (b8 hasChanged)
{
    pimpl->setChangedFlag (hasChanged);
}

z0 FileBasedDocument::changed()
{
    pimpl->changed();
}

//==============================================================================
Result FileBasedDocument::loadFrom (const File& fileToLoadFrom,
                                    b8 showMessageOnFailure,
                                    b8 showWaitCursor)
{
    return pimpl->loadFrom (fileToLoadFrom, showMessageOnFailure, showWaitCursor);
}

z0 FileBasedDocument::loadFromAsync (const File& fileToLoadFrom,
                                       b8 showMessageOnFailure,
                                       std::function<z0 (Result)> callback)
{
    pimpl->loadFromAsync (fileToLoadFrom, showMessageOnFailure, std::move (callback));
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
Result FileBasedDocument::loadFromUserSpecifiedFile (b8 showMessageOnFailure)
{
    return pimpl->loadFromUserSpecifiedFile (showMessageOnFailure);
}
#endif

z0 FileBasedDocument::loadFromUserSpecifiedFileAsync (const b8 showMessageOnFailure,
                                                        std::function<z0 (Result)> callback)
{
    pimpl->loadFromUserSpecifiedFileAsync (showMessageOnFailure, std::move (callback));
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
FileBasedDocument::SaveResult FileBasedDocument::save (b8 askUserForFileIfNotSpecified,
                                                       b8 showMessageOnFailure)
{
    return pimpl->save (askUserForFileIfNotSpecified, showMessageOnFailure);
}
#endif

z0 FileBasedDocument::saveAsync (b8 askUserForFileIfNotSpecified,
                                   b8 showMessageOnFailure,
                                   std::function<z0 (SaveResult)> callback)
{
    pimpl->saveAsync (askUserForFileIfNotSpecified, showMessageOnFailure, std::move (callback));
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
FileBasedDocument::SaveResult FileBasedDocument::saveIfNeededAndUserAgrees()
{
    return pimpl->saveIfNeededAndUserAgrees();
}
#endif

z0 FileBasedDocument::saveIfNeededAndUserAgreesAsync (std::function<z0 (SaveResult)> callback)
{
    pimpl->saveIfNeededAndUserAgreesAsync (std::move (callback));
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
FileBasedDocument::SaveResult FileBasedDocument::saveAs (const File& newFile,
                                                         b8 warnAboutOverwritingExistingFiles,
                                                         b8 askUserForFileIfNotSpecified,
                                                         b8 showMessageOnFailure,
                                                         b8 showWaitCursor)
{
    return pimpl->saveAs (newFile,
                          warnAboutOverwritingExistingFiles,
                          askUserForFileIfNotSpecified,
                          showMessageOnFailure,
                          showWaitCursor);
}
#endif

z0 FileBasedDocument::saveAsAsync (const File& newFile,
                                     b8 warnAboutOverwritingExistingFiles,
                                     b8 askUserForFileIfNotSpecified,
                                     b8 showMessageOnFailure,
                                     std::function<z0 (SaveResult)> callback)
{
    pimpl->saveAsAsync (newFile,
                        warnAboutOverwritingExistingFiles,
                        askUserForFileIfNotSpecified,
                        showMessageOnFailure,
                        std::move (callback));
}

//==============================================================================
#if DRX_MODAL_LOOPS_PERMITTED
FileBasedDocument::SaveResult FileBasedDocument::saveAsInteractive (b8 warnAboutOverwritingExistingFiles)
{
    return pimpl->saveAsInteractive (warnAboutOverwritingExistingFiles);
}
#endif

z0 FileBasedDocument::saveAsInteractiveAsync (b8 warnAboutOverwritingExistingFiles,
                                                std::function<z0 (SaveResult)> callback)
{
    pimpl->saveAsInteractiveAsync (warnAboutOverwritingExistingFiles, std::move (callback));
}

//==============================================================================
const File& FileBasedDocument::getFile() const
{
    return pimpl->getFile();
}

z0 FileBasedDocument::setFile (const File& newFile)
{
    pimpl->setFile (newFile);
}

//==============================================================================
z0 FileBasedDocument::loadDocumentAsync (const File& file, std::function<z0 (Result)> callback)
{
    const auto result = loadDocument (file);
    NullCheckedInvocation::invoke (callback, result);
}

z0 FileBasedDocument::saveDocumentAsync (const File& file, std::function<z0 (Result)> callback)
{
    const auto result = saveDocument (file);
    NullCheckedInvocation::invoke (callback, result);
}

File FileBasedDocument::getSuggestedSaveAsFile (const File& defaultFile)
{
    return defaultFile.withFileExtension (pimpl->getFileExtension()).getNonexistentSibling (true);
}

} // namespace drx
