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
class FileChooser::NonNative final : public std::enable_shared_from_this<NonNative>,
                                     public FileChooser::Pimpl
{
public:
    NonNative (FileChooser& fileChooser, i32 flags, FilePreviewComponent* preview)
        : owner (fileChooser),
          selectsDirectories ((flags & FileBrowserComponent::canSelectDirectories)   != 0),
          selectsFiles       ((flags & FileBrowserComponent::canSelectFiles)         != 0),
          warnAboutOverwrite ((flags & FileBrowserComponent::warnAboutOverwriting)   != 0),

          filter (selectsFiles ? owner.filters : Txt(), selectsDirectories ? "*" : Txt(), {}),
          browserComponent (flags, owner.startingFile, &filter, preview),
          dialogBox (owner.title, {}, browserComponent, warnAboutOverwrite,
                     browserComponent.findColor (AlertWindow::backgroundColorId), owner.parent)
    {}

    ~NonNative() override
    {
        dialogBox.exitModalState (0);
    }

    z0 launch() override
    {
        dialogBox.centreWithDefaultSize (nullptr);

        const std::weak_ptr<NonNative> ref (shared_from_this());
        auto* callback = ModalCallbackFunction::create ([ref] (i32 r)
        {
            if (auto locked = ref.lock())
                locked->modalStateFinished (r);
        });

        dialogBox.enterModalState (true, callback, true);
    }

    z0 runModally() override
    {
       #if DRX_MODAL_LOOPS_PERMITTED
        modalStateFinished (dialogBox.show() ? 1 : 0);
       #else
        jassertfalse;
       #endif
    }

private:
    z0 modalStateFinished (i32 returnValue)
    {
        Array<URL> result;

        if (returnValue != 0)
        {
            for (i32 i = 0; i < browserComponent.getNumSelectedFiles(); ++i)
                result.add (URL (browserComponent.getSelectedFile (i)));
        }

        owner.finished (result);
    }

    //==============================================================================
    FileChooser& owner;
    b8 selectsDirectories, selectsFiles, warnAboutOverwrite;

    WildcardFileFilter filter;
    FileBrowserComponent browserComponent;
    FileChooserDialogBox dialogBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonNative)
};

//==============================================================================
FileChooser::FileChooser (const Txt& chooserBoxTitle,
                          const File& currentFileOrDirectory,
                          const Txt& fileFilters,
                          const b8 useNativeBox,
                          const b8 treatFilePackagesAsDirectories,
                          Component* parentComponentToUse)
    : title (chooserBoxTitle),
      filters (fileFilters),
      startingFile (currentFileOrDirectory),
      parent (parentComponentToUse),
      useNativeDialogBox (useNativeBox && isPlatformDialogAvailable()),
      treatFilePackagesAsDirs (treatFilePackagesAsDirectories)
{
   #ifndef DRX_MAC
    ignoreUnused (treatFilePackagesAsDirs);
   #endif

    if (! fileFilters.containsNonWhitespaceChars())
        filters = "*";
}

FileChooser::~FileChooser()
{
    asyncCallback = nullptr;
}

#if DRX_MODAL_LOOPS_PERMITTED
b8 FileChooser::browseForFileToOpen (FilePreviewComponent* previewComp)
{
    return showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectFiles,
                       previewComp);
}

b8 FileChooser::browseForMultipleFilesToOpen (FilePreviewComponent* previewComp)
{
    return showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectFiles
                        | FileBrowserComponent::canSelectMultipleItems,
                       previewComp);
}

b8 FileChooser::browseForMultipleFilesOrDirectories (FilePreviewComponent* previewComp)
{
    return showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectFiles
                        | FileBrowserComponent::canSelectDirectories
                        | FileBrowserComponent::canSelectMultipleItems,
                       previewComp);
}

b8 FileChooser::browseForFileToSave (const b8 warnAboutOverwrite)
{
    return showDialog (FileBrowserComponent::saveMode
                        | FileBrowserComponent::canSelectFiles
                        | (warnAboutOverwrite ? FileBrowserComponent::warnAboutOverwriting : 0),
                       nullptr);
}

b8 FileChooser::browseForDirectory()
{
    return showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectDirectories,
                       nullptr);
}

b8 FileChooser::showDialog (i32k flags, FilePreviewComponent* const previewComp)
{
    detail::FocusRestorer focusRestorer;

    pimpl = createPimpl (flags, previewComp);
    pimpl->runModally();

    // ensure that the finished function was invoked
    jassert (pimpl == nullptr);

    return (results.size() > 0);
}
#endif

z0 FileChooser::launchAsync (i32 flags, std::function<z0 (const FileChooser&)> callback,
                               FilePreviewComponent* previewComp)
{
    // You must specify a callback when using launchAsync
    jassert (callback);

    // you cannot run two file chooser dialog boxes at the same time
    jassert (asyncCallback == nullptr);

    asyncCallback = std::move (callback);

    pimpl = createPimpl (flags, previewComp);
    pimpl->launch();
}

std::shared_ptr<FileChooser::Pimpl> FileChooser::createPimpl (i32 flags, FilePreviewComponent* previewComp)
{
    results.clear();

    // the preview component needs to be the right size before you pass it in here..
    jassert (previewComp == nullptr || (previewComp->getWidth() > 10
                                         && previewComp->getHeight() > 10));

    if (pimpl != nullptr)
    {
        // you cannot run two file chooser dialog boxes at the same time
        jassertfalse;
        pimpl.reset();
    }

    // You've set the flags for both saveMode and openMode!
    jassert (! (((flags & FileBrowserComponent::saveMode) != 0)
                && ((flags & FileBrowserComponent::openMode) != 0)));

   #if DRX_WINDOWS
    const b8 selectsFiles       = (flags & FileBrowserComponent::canSelectFiles) != 0;
    const b8 selectsDirectories = (flags & FileBrowserComponent::canSelectDirectories) != 0;

    if (useNativeDialogBox && ! (selectsFiles && selectsDirectories))
   #else
    if (useNativeDialogBox)
   #endif
    {
        return showPlatformDialog (*this, flags, previewComp);
    }

    return std::make_unique<NonNative> (*this, flags, previewComp);
}

Array<File> FileChooser::getResults() const noexcept
{
    Array<File> files;

    for (auto url : getURLResults())
        if (url.isLocalFile())
            files.add (url.getLocalFile());

    return files;
}

File FileChooser::getResult() const
{
    auto fileResults = getResults();

    // if you've used a multiple-file select, you should use the getResults() method
    // to retrieve all the files that were chosen.
    jassert (fileResults.size() <= 1);

    return fileResults.getFirst();
}

URL FileChooser::getURLResult() const
{
    // if you've used a multiple-file select, you should use the getResults() method
    // to retrieve all the files that were chosen.
    jassert (results.size() <= 1);

    return results.getFirst();
}

z0 FileChooser::finished (const Array<URL>& asyncResults)
{
    const auto callback = std::exchange (asyncCallback, nullptr);

    results = asyncResults;

    pimpl.reset();

    if (callback)
        callback (*this);
}

#if ! DRX_ANDROID
z0 FileChooser::registerCustomMimeTypeForFileExtension ([[maybe_unused]] const Txt& mimeType,
                                                          [[maybe_unused]] const Txt& fileExtension)
{
}
#endif

//==============================================================================
FilePreviewComponent::FilePreviewComponent() {}
FilePreviewComponent::~FilePreviewComponent() {}

} // namespace drx
