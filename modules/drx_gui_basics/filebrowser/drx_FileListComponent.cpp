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
FileListComponent::FileListComponent (DirectoryContentsList& listToShow)
    : ListBox ({}, this),
      DirectoryContentsDisplayComponent (listToShow),
      lastDirectory (listToShow.getDirectory())
{
    setTitle ("Files");
    directoryContentsList.addChangeListener (this);
}

FileListComponent::~FileListComponent()
{
    directoryContentsList.removeChangeListener (this);
}

i32 FileListComponent::getNumSelectedFiles() const
{
    return getNumSelectedRows();
}

File FileListComponent::getSelectedFile (i32 index) const
{
    return directoryContentsList.getFile (getSelectedRow (index));
}

z0 FileListComponent::deselectAllFiles()
{
    deselectAllRows();
}

z0 FileListComponent::scrollToTop()
{
    getVerticalScrollBar().setCurrentRangeStart (0);
}

z0 FileListComponent::setSelectedFile (const File& f)
{
    if (! directoryContentsList.isStillLoading())
    {
        for (i32 i = directoryContentsList.getNumFiles(); --i >= 0;)
        {
            if (directoryContentsList.getFile (i) == f)
            {
                fileWaitingToBeSelected = File();

                updateContent();
                selectRow (i);
                return;
            }
        }
    }

    deselectAllRows();
    fileWaitingToBeSelected = f;
}

//==============================================================================
z0 FileListComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateContent();

    if (lastDirectory != directoryContentsList.getDirectory())
    {
        fileWaitingToBeSelected = File();
        lastDirectory = directoryContentsList.getDirectory();
        deselectAllRows();
    }

    if (fileWaitingToBeSelected != File())
        setSelectedFile (fileWaitingToBeSelected);
}

//==============================================================================
class FileListComponent::ItemComponent final : public Component,
                                               public TooltipClient,
                                               private TimeSliceClient,
                                               private AsyncUpdater
{
public:
    ItemComponent (FileListComponent& fc, TimeSliceThread& t)
        : owner (fc), thread (t)
    {
    }

    ~ItemComponent() override
    {
        thread.removeTimeSliceClient (this);
    }

    //==============================================================================
    z0 paint (Graphics& g) override
    {
        getLookAndFeel().drawFileBrowserRow (g, getWidth(), getHeight(),
                                             file, file.getFileName(),
                                             &icon, fileSize, modTime,
                                             isDirectory, highlighted,
                                             index, owner);
    }

    z0 mouseDown (const MouseEvent& e) override
    {
        owner.selectRowsBasedOnModifierKeys (index, e.mods, true);
        owner.sendMouseClickMessage (file, e);
    }

    z0 mouseDoubleClick (const MouseEvent&) override
    {
        owner.sendDoubleClickMessage (file);
    }

    z0 update (const File& root, const DirectoryContentsList::FileInfo* fileInfo,
                 i32 newIndex, b8 nowHighlighted)
    {
        thread.removeTimeSliceClient (this);

        if (nowHighlighted != highlighted || newIndex != index)
        {
            index = newIndex;
            highlighted = nowHighlighted;
            repaint();
        }

        File newFile;
        Txt newFileSize, newModTime;

        if (fileInfo != nullptr)
        {
            newFile = root.getChildFile (fileInfo->filename);
            newFileSize = File::descriptionOfSizeInBytes (fileInfo->fileSize);
            newModTime = fileInfo->modificationTime.formatted ("%d %b '%y %H:%M");
        }

        if (newFile != file
             || fileSize != newFileSize
             || modTime != newModTime)
        {
            file = newFile;
            fileSize = newFileSize;
            modTime = newModTime;
            icon = Image();
            isDirectory = fileInfo != nullptr && fileInfo->isDirectory;

            repaint();
        }

        if (file != File() && icon.isNull() && ! isDirectory)
        {
            updateIcon (true);

            if (! icon.isValid())
                thread.addTimeSliceClient (this);
        }
    }

    i32 useTimeSlice() override
    {
        updateIcon (false);
        return -1;
    }

    z0 handleAsyncUpdate() override
    {
        repaint();
    }

    Txt getTooltip() override
    {
        return owner.getTooltipForRow (index);
    }

private:
    //==============================================================================
    FileListComponent& owner;
    TimeSliceThread& thread;
    File file;
    Txt fileSize, modTime;
    Image icon;
    i32 index = 0;
    b8 highlighted = false, isDirectory = false;

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

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
                icon = im;
                triggerAsyncUpdate();
            }
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};

//==============================================================================
i32 FileListComponent::getNumRows()
{
    return directoryContentsList.getNumFiles();
}

Txt FileListComponent::getNameForRow (i32 rowNumber)
{
    return directoryContentsList.getFile (rowNumber).getFileName();
}

z0 FileListComponent::paintListBoxItem (i32, Graphics&, i32, i32, b8)
{
}

Component* FileListComponent::refreshComponentForRow (i32 row, b8 isSelected, Component* existingComponentToUpdate)
{
    jassert (existingComponentToUpdate == nullptr || dynamic_cast<ItemComponent*> (existingComponentToUpdate) != nullptr);

    auto comp = static_cast<ItemComponent*> (existingComponentToUpdate);

    if (comp == nullptr)
        comp = new ItemComponent (*this, directoryContentsList.getTimeSliceThread());

    DirectoryContentsList::FileInfo fileInfo;
    comp->update (directoryContentsList.getDirectory(),
                  directoryContentsList.getFileInfo (row, fileInfo) ? &fileInfo : nullptr,
                  row, isSelected);

    return comp;
}

z0 FileListComponent::selectedRowsChanged (i32 /*lastRowSelected*/)
{
    sendSelectionChangeMessage();
}

z0 FileListComponent::deleteKeyPressed (i32 /*currentSelectedRow*/)
{
}

z0 FileListComponent::returnKeyPressed (i32 currentSelectedRow)
{
    sendDoubleClickMessage (directoryContentsList.getFile (currentSelectedRow));
}

} // namespace drx
